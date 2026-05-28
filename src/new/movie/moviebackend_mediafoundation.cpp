/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Media Foundation movie decoder backend.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "moviebackend_mediafoundation.h"

#include "ccfile.h"
#include "debughandler.h"

#include <algorithm>
#include <mfapi.h>
#include <mferror.h>
#include <mutex>


using Microsoft::WRL::ComPtr;


namespace
{
    /**
     *  Carries the byte count from a completed synchronous read back
     *  through IMFAsyncResult so EndRead can report bytes_read to the caller.
     */
    class AsyncReadState final : public IUnknown
    {
        public:
            explicit AsyncReadState(ULONG bytes_read) :
                ReferenceCount(1),
                BytesRead(bytes_read)
            {
            }

            HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv) override
            {
                if (!ppv) {
                    return E_POINTER;
                }

                if (riid == IID_IUnknown) {
                    *ppv = static_cast<IUnknown *>(this);
                    AddRef();
                    return S_OK;
                }

                *ppv = nullptr;
                return E_NOINTERFACE;
            }

            ULONG STDMETHODCALLTYPE AddRef() override
            {
                return static_cast<ULONG>(InterlockedIncrement(&ReferenceCount));
            }

            ULONG STDMETHODCALLTYPE Release() override
            {
                const ULONG refcount = static_cast<ULONG>(InterlockedDecrement(&ReferenceCount));
                if (!refcount) {
                    delete this;
                }

                return refcount;
            }

            ULONG BytesRead;

        private:
            ~AsyncReadState() = default;

            volatile long ReferenceCount;
    };


    /**
     *  IMFByteStream adapter over a CCFileClass. BeginRead performs the
     *  read synchronously and completes the async result inline, which is
     *  valid for local file I/O where blocking on disk is acceptable.
     */
    class MovieByteStream final : public IMFByteStream
    {
        public:
            explicit MovieByteStream(CCFileClass *file) :
                ReferenceCount(1),
                File(file),
                Closed(false)
            {
            }

            HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv) override
            {
                if (!ppv) {
                    return E_POINTER;
                }

                if (riid == IID_IUnknown || riid == IID_IMFByteStream) {
                    *ppv = static_cast<IMFByteStream *>(this);
                    AddRef();
                    return S_OK;
                }

                *ppv = nullptr;
                return E_NOINTERFACE;
            }

            ULONG STDMETHODCALLTYPE AddRef() override
            {
                return static_cast<ULONG>(InterlockedIncrement(&ReferenceCount));
            }

            ULONG STDMETHODCALLTYPE Release() override
            {
                const ULONG refcount = static_cast<ULONG>(InterlockedDecrement(&ReferenceCount));
                if (!refcount) {
                    delete this;
                }

                return refcount;
            }

            HRESULT STDMETHODCALLTYPE GetCapabilities(DWORD *capabilities) override
            {
                if (!capabilities) {
                    return E_POINTER;
                }

                *capabilities = MFBYTESTREAM_IS_READABLE | MFBYTESTREAM_IS_SEEKABLE;
                return S_OK;
            }

            HRESULT STDMETHODCALLTYPE GetLength(QWORD *length) override
            {
                if (!length) {
                    return E_POINTER;
                }

                std::lock_guard<std::mutex> lock(Mutex);
                *length = static_cast<QWORD>(File->Size());
                return S_OK;
            }

            HRESULT STDMETHODCALLTYPE SetLength(QWORD) override
            {
                return STG_E_ACCESSDENIED;
            }

            HRESULT STDMETHODCALLTYPE GetCurrentPosition(QWORD *position) override
            {
                if (!position) {
                    return E_POINTER;
                }

                std::lock_guard<std::mutex> lock(Mutex);
                *position = static_cast<QWORD>(File->Tell());
                return S_OK;
            }

            HRESULT STDMETHODCALLTYPE SetCurrentPosition(QWORD position) override
            {
                std::lock_guard<std::mutex> lock(Mutex);
                return File->Seek(static_cast<off_t>(position), FILE_SEEK_START) >= 0 ? S_OK : E_FAIL;
            }

            HRESULT STDMETHODCALLTYPE IsEndOfStream(BOOL *end_of_stream) override
            {
                if (!end_of_stream) {
                    return E_POINTER;
                }

                std::lock_guard<std::mutex> lock(Mutex);
                *end_of_stream = File->Tell() >= File->Size();
                return S_OK;
            }

            HRESULT STDMETHODCALLTYPE Read(BYTE *buffer, ULONG bytes_to_read, ULONG *bytes_read) override
            {
                if (Closed) {
                    return E_HANDLE;
                }

                if (!buffer && bytes_to_read) {
                    return E_POINTER;
                }

                std::lock_guard<std::mutex> lock(Mutex);
                const long result = File->Read(buffer, static_cast<int>(bytes_to_read));
                if (result < 0) {
                    return E_FAIL;
                }

                if (bytes_read) {
                    *bytes_read = static_cast<ULONG>(result);
                }

                return S_OK;
            }

            HRESULT STDMETHODCALLTYPE BeginRead(BYTE *buffer, ULONG bytes_to_read, IMFAsyncCallback *callback, IUnknown *state) override
            {
                if (!callback) {
                    return E_POINTER;
                }

                ULONG bytes_read = 0;
                const HRESULT read_status = Read(buffer, bytes_to_read, &bytes_read);

                ComPtr<IUnknown> async_state;
                async_state.Attach(new (std::nothrow) AsyncReadState(bytes_read));
                if (!async_state) {
                    return E_OUTOFMEMORY;
                }

                ComPtr<IMFAsyncResult> async_result;
                HRESULT hr = MFCreateAsyncResult(async_state.Get(), callback, state, &async_result);
                if (FAILED(hr)) {
                    return hr;
                }

                async_result->SetStatus(read_status);
                return MFInvokeCallback(async_result.Get());
            }

            HRESULT STDMETHODCALLTYPE EndRead(IMFAsyncResult *result, ULONG *bytes_read) override
            {
                if (!result) {
                    return E_POINTER;
                }

                const HRESULT status = result->GetStatus();
                if (FAILED(status)) {
                    return status;
                }

                if (bytes_read) {
                    *bytes_read = 0;
                }

                ComPtr<IUnknown> object;
                HRESULT hr = result->GetObject(&object);
                if (FAILED(hr)) {
                    return hr;
                }

                AsyncReadState *read_state = static_cast<AsyncReadState *>(object.Get());
                if (bytes_read) {
                    *bytes_read = read_state->BytesRead;
                }

                return S_OK;
            }

            HRESULT STDMETHODCALLTYPE Write(const BYTE *, ULONG, ULONG *) override
            {
                return STG_E_ACCESSDENIED;
            }

            HRESULT STDMETHODCALLTYPE BeginWrite(const BYTE *, ULONG, IMFAsyncCallback *, IUnknown *) override
            {
                return STG_E_ACCESSDENIED;
            }

            HRESULT STDMETHODCALLTYPE EndWrite(IMFAsyncResult *, ULONG *) override
            {
                return STG_E_ACCESSDENIED;
            }

            HRESULT STDMETHODCALLTYPE Seek(MFBYTESTREAM_SEEK_ORIGIN seek_origin, LONGLONG seek_offset, DWORD, QWORD *current_position) override
            {
                std::lock_guard<std::mutex> lock(Mutex);

                const FileSeekType origin = (seek_origin == msoBegin) ? FILE_SEEK_START : FILE_SEEK_CURRENT;
                if (File->Seek(static_cast<off_t>(seek_offset), origin) < 0) {
                    return E_FAIL;
                }

                if (current_position) {
                    *current_position = static_cast<QWORD>(File->Tell());
                }

                return S_OK;
            }

            HRESULT STDMETHODCALLTYPE Flush() override
            {
                return S_OK;
            }

            HRESULT STDMETHODCALLTYPE Close() override
            {
                Closed = true;
                return S_OK;
            }

        private:
            ~MovieByteStream() = default;

            volatile long ReferenceCount;
            CCFileClass *File;
            bool Closed;
            std::mutex Mutex;
    };
}


MediaFoundationMovieBackend::MediaFoundationMovieBackend() = default;


MediaFoundationMovieBackend::~MediaFoundationMovieBackend()
{
    if (MediaFoundationStarted) {
        MFShutdown();
    }

    if (ComInitialized) {
        CoUninitialize();
    }
}


void MediaFoundationMovieBackend::Reset()
{
    Reader.Reset();
    ByteStream.Reset();
    File.reset();
    Paused = false;
    Finished = false;
    HasVideoStream = false;
    HasAudioStream = false;
    VideoStreamFinished = false;
    AudioStreamFinished = false;
    VideoStreamIndex = DWORD(-1);
    AudioStreamIndex = DWORD(-1);
    VideoWidth = 0;
    VideoHeight = 0;
    VideoStride = 0;
    AudioRate = 0;
    AudioChannels = 0;
    AudioBits = 0;
    AudioFormat = MOVIE_SAMPLE_INVALID;
}


/**
 *  Initialises COM and Media Foundation, opens the file and configures
 *  video and audio decode streams. Returns false on any failure.
 */
bool MediaFoundationMovieBackend::Open(const char *filename)
{
    Reset();

    if (!filename) {
        return false;
    }

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr) || hr == S_FALSE) {
        ComInitialized = true;
    } else if (hr != RPC_E_CHANGED_MODE) {
        DEBUG_ERROR("Media Foundation backend failed to initialize COM! Error code: 0x{:08x}.\n", hr);
        return false;
    }

    hr = MFStartup(MF_VERSION, MFSTARTUP_FULL);
    if (FAILED(hr)) {
        DEBUG_ERROR("Media Foundation backend failed to initialize Media Foundation! Error code: 0x{:08x}.\n", hr);
        return false;
    }

    MediaFoundationStarted = true;

    File = std::make_unique<CCFileClass>(filename);
    if (!File->Open(FILE_ACCESS_READ)) {
        return false;
    }

    ByteStream.Attach(new (std::nothrow) MovieByteStream(File.get()));
    if (!ByteStream) {
        DEBUG_ERROR("Media Foundation backend failed to create a movie byte stream.\n");
        return false;
    }

    ComPtr<IMFAttributes> attributes;
    hr = MFCreateAttributes(&attributes, 1);
    if (FAILED(hr)) {
        DEBUG_ERROR("Media Foundation backend failed to create source reader attributes! Error code: 0x{:08x}.\n", hr);
        return false;
    }

    attributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);

    hr = MFCreateSourceReaderFromByteStream(ByteStream.Get(), attributes.Get(), &Reader);
    if (FAILED(hr)) {
        DEBUG_ERROR("Media Foundation backend failed to create source reader for \"{}\"! Error code: 0x{:08x}.\n", filename, hr);
        return false;
    }

    Reader->SetStreamSelection(static_cast<DWORD>(MF_SOURCE_READER_ALL_STREAMS), FALSE);

    HasVideoStream = Configure_Video_Stream();
    if (!HasVideoStream) {
        DEBUG_WARNING("Media Foundation backend could not configure a video stream for \"{}\".\n", filename);
        return false;
    }

    HasAudioStream = Configure_Audio_Stream();
    return true;
}


/**
 *  Iterates the source reader's streams to find the first one whose major
 *  type matches major_type, returning its zero-based index in stream_index.
 */
bool MediaFoundationMovieBackend::Find_Stream_Index(const GUID &major_type, DWORD &stream_index) const
{
    stream_index = DWORD(-1);

    for (DWORD candidate = 0;; ++candidate) {
        ComPtr<IMFMediaType> native_type;
        const HRESULT hr = Reader->GetNativeMediaType(candidate, 0, &native_type);
        if (hr == MF_E_INVALIDSTREAMNUMBER) {
            break;
        }

        if (FAILED(hr) || !native_type) {
            continue;
        }

        GUID candidate_major_type = GUID_NULL;
        if (FAILED(native_type->GetGUID(MF_MT_MAJOR_TYPE, &candidate_major_type))) {
            continue;
        }

        if (candidate_major_type == major_type) {
            stream_index = candidate;
            return true;
        }
    }

    return false;
}


/**
 *  Selects the video stream, negotiates NV12 output, then reads back the
 *  final frame size and stride from the configured media type.
 */
bool MediaFoundationMovieBackend::Configure_Video_Stream()
{
    if (!Find_Stream_Index(MFMediaType_Video, VideoStreamIndex)) {
        return false;
    }

    HRESULT hr = Reader->SetStreamSelection(VideoStreamIndex, TRUE);
    if (FAILED(hr)) {
        return false;
    }

    ComPtr<IMFMediaType> output_type;
    hr = MFCreateMediaType(&output_type);
    if (FAILED(hr)) {
        return false;
    }

    output_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    output_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12);

    hr = Reader->SetCurrentMediaType(VideoStreamIndex, nullptr, output_type.Get());
    if (FAILED(hr)) {
        return false;
    }

    ComPtr<IMFMediaType> current_type;
    hr = Reader->GetCurrentMediaType(VideoStreamIndex, &current_type);
    if (FAILED(hr)) {
        return false;
    }

    hr = MFGetAttributeSize(current_type.Get(), MF_MT_FRAME_SIZE, &VideoWidth, &VideoHeight);
    if (FAILED(hr)) {
        return false;
    }

    UINT32 stride_value = 0;
    if (SUCCEEDED(current_type->GetUINT32(MF_MT_DEFAULT_STRIDE, &stride_value))) {
        VideoStride = static_cast<LONG>(stride_value);
    } else {
        LONG stride = 0;
        if (SUCCEEDED(MFGetStrideForBitmapInfoHeader(MFVideoFormat_NV12.Data1, VideoWidth, &stride))) {
            VideoStride = stride;
        } else {
            VideoStride = static_cast<LONG>(VideoWidth);
        }
    }

    DEBUG_INFO("Media Foundation backend video output: NV12 {}x{} stride={}\n",
        VideoWidth,
        VideoHeight,
        VideoStride);

    return true;
}


/**
 *  Selects the audio stream and negotiates 16-bit PCM output at the native
 *  sample rate and channel count. Disables the audio stream on any failure.
 */
bool MediaFoundationMovieBackend::Configure_Audio_Stream()
{
    if (!Find_Stream_Index(MFMediaType_Audio, AudioStreamIndex)) {
        DEBUG_INFO("Media Foundation backend found no audio stream.\n");
        return false;
    }

    HRESULT hr = Reader->SetStreamSelection(AudioStreamIndex, TRUE);
    if (FAILED(hr)) {
        DEBUG_INFO("Media Foundation backend found no selectable audio stream.\n");
        return false;
    }

    ComPtr<IMFMediaType> native_type;
    hr = Reader->GetNativeMediaType(AudioStreamIndex, 0, &native_type);
    if (FAILED(hr)) {
        DEBUG_INFO("Media Foundation backend could not read the native audio type. Error code: 0x{:08x}.\n", hr);
        Reader->SetStreamSelection(AudioStreamIndex, FALSE);
        return false;
    }

    UINT32 native_rate = 0;
    UINT32 native_channels = 0;
    native_type->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &native_rate);
    native_type->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &native_channels);

    if (!native_rate || !native_channels) {
        DEBUG_INFO("Media Foundation backend native audio type is incomplete.\n");
        Reader->SetStreamSelection(AudioStreamIndex, FALSE);
        return false;
    }

    ComPtr<IMFMediaType> output_type;
    hr = MFCreateMediaType(&output_type);
    if (FAILED(hr)) {
        Reader->SetStreamSelection(AudioStreamIndex, FALSE);
        return false;
    }

    output_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    output_type->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    output_type->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
    output_type->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, native_channels);
    output_type->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, native_rate);
    output_type->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, native_channels * 2);
    output_type->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, native_rate * native_channels * 2);
    output_type->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);

    hr = Reader->SetCurrentMediaType(AudioStreamIndex, nullptr, output_type.Get());
    if (FAILED(hr)) {
        DEBUG_INFO("Media Foundation backend could not negotiate 16-bit PCM audio. Error code: 0x{:08x}.\n", hr);
        Reader->SetStreamSelection(AudioStreamIndex, FALSE);
        return false;
    }

    ComPtr<IMFMediaType> current_type;
    hr = Reader->GetCurrentMediaType(AudioStreamIndex, &current_type);
    if (FAILED(hr)) {
        Reader->SetStreamSelection(AudioStreamIndex, FALSE);
        return false;
    }

    current_type->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &AudioBits);
    current_type->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &AudioRate);
    current_type->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &AudioChannels);

    if (!AudioRate || !AudioChannels || !AudioBits) {
        DEBUG_INFO("Media Foundation backend negotiated an incomplete audio type.\n");
        Reader->SetStreamSelection(AudioStreamIndex, FALSE);
        return false;
    }

    if (AudioBits == 16) {
        AudioFormat = MOVIE_SAMPLE_S16;
    } else if (AudioBits == 8) {
        AudioFormat = MOVIE_SAMPLE_U8;
    } else {
        DEBUG_INFO("Media Foundation backend negotiated unsupported audio bit depth: {}.\n", AudioBits);
        Reader->SetStreamSelection(AudioStreamIndex, FALSE);
        return false;
    }

    DEBUG_INFO("Media Foundation backend audio output: {} Hz, {} channels, {} bits.\n", AudioRate, AudioChannels, AudioBits);

    return true;
}


bool MediaFoundationMovieBackend::All_Streams_Finished() const
{
    const bool video_finished = !HasVideoStream || VideoStreamFinished;
    const bool audio_finished = !HasAudioStream || AudioStreamFinished;
    return video_finished && audio_finished;
}


/**
 *  Extracts NV12 Y and UV planes from an IMFSample buffer into
 *  output.VideoFrame, accounting for codec alignment padding.
 */
bool MediaFoundationMovieBackend::Decode_Video_Sample(IMFSample *sample, LONGLONG timestamp, MovieDecodeOutput &output)
{
    ComPtr<IMFMediaBuffer> media_buffer;
    HRESULT hr = sample->ConvertToContiguousBuffer(&media_buffer);
    if (FAILED(hr)) {
        DEBUG_ERROR("Media Foundation backend failed to flatten a video sample! Error code: 0x{:08x}.\n", hr);
        return false;
    }

    /**
     *  Prefer the 2D buffer interface to obtain the actual stride, which
     *  may differ from the media type's default stride when the codec
     *  applies internal alignment padding.
     */
    BYTE *source = nullptr;
    int source_stride = 0;
    DWORD current_length = 0;
    bool locked_2d = false;
    ComPtr<IMF2DBuffer> buffer_2d;

    if (SUCCEEDED(media_buffer.As(&buffer_2d))) {
        LONG stride = 0;
        if (SUCCEEDED(buffer_2d->Lock2D(&source, &stride))) {
            source_stride = std::abs(static_cast<int>(stride));
            locked_2d = true;
            media_buffer->GetCurrentLength(&current_length);
        }
    }

    if (!locked_2d) {
        DWORD max_length = 0;
        hr = media_buffer->Lock(&source, &max_length, &current_length);
        if (FAILED(hr)) {
            DEBUG_ERROR("Media Foundation backend failed to lock a video sample! Error code: 0x{:08x}.\n", hr);
            return false;
        }
        source_stride = std::abs(VideoStride) > 0 ? std::abs(VideoStride) : static_cast<int>(VideoWidth);
    }

    output.VideoFrame.TimestampMs = timestamp / 10000;
    output.VideoFrame.Width = static_cast<int>(VideoWidth);
    output.VideoFrame.Height = static_cast<int>(VideoHeight);
    output.VideoFrame.Format = MOVIE_VIDEO_NV12;

    const int display_width = static_cast<int>(VideoWidth);
    const int display_height = static_cast<int>(VideoHeight);
    const int uv_height = (display_height + 1) / 2;

    /**
     *  The coded frame height may be larger than the display height due to
     *  codec macroblock alignment (e.g. height 360 is padded to 368). The
     *  UV plane starts after the full coded Y plane, not after the display
     *  height rows. For NV12: total = stride * coded_h * 3/2, so the UV
     *  plane begins at total * 2/3.
     */
    const std::size_t uv_offset = (static_cast<std::size_t>(current_length) * 2) / 3;

    const std::size_t last_y_byte = static_cast<std::size_t>(display_height - 1) * source_stride + display_width;
    const std::size_t last_uv_byte = uv_offset + static_cast<std::size_t>(uv_height - 1) * source_stride + display_width;

    if (current_length == 0 || last_y_byte > current_length || last_uv_byte > current_length) {
        if (locked_2d) buffer_2d->Unlock2D(); else media_buffer->Unlock();
        DEBUG_ERROR("Media Foundation backend produced a short NV12 sample.\n");
        return false;
    }

    /**
     *  Copy Y and UV planes row by row into tightly-packed output buffers,
     *  stripping any stride padding the codec may have added.
     */
    output.VideoFrame.Pitch = display_width;
    output.VideoFrame.SecondaryPitch = display_width;
    output.VideoFrame.Pixels.resize(static_cast<std::size_t>(display_width) * display_height);
    output.VideoFrame.SecondaryPixels.resize(static_cast<std::size_t>(display_width) * uv_height);

    for (int row = 0; row < display_height; ++row) {
        std::memcpy(output.VideoFrame.Pixels.data() + static_cast<std::size_t>(row) * display_width,
                    source + static_cast<std::size_t>(row) * source_stride,
                    display_width);
    }

    const BYTE *uv_source = source + uv_offset;
    for (int row = 0; row < uv_height; ++row) {
        std::memcpy(output.VideoFrame.SecondaryPixels.data() + static_cast<std::size_t>(row) * display_width,
                    uv_source + static_cast<std::size_t>(row) * source_stride,
                    display_width);
    }

    if (locked_2d) {
        buffer_2d->Unlock2D();
    } else {
        media_buffer->Unlock();
    }

    output.HasVideoFrame = true;
    return true;
}


/**
 *  Copies raw PCM bytes from an IMFSample buffer into output.AudioChunk.
 */
bool MediaFoundationMovieBackend::Decode_Audio_Sample(IMFSample *sample, LONGLONG timestamp, MovieDecodeOutput &output)
{
    ComPtr<IMFMediaBuffer> buffer;
    HRESULT hr = sample->ConvertToContiguousBuffer(&buffer);
    if (FAILED(hr)) {
        DEBUG_ERROR("Media Foundation backend failed to flatten an audio sample! Error code: 0x{:08x}.\n", hr);
        return false;
    }

    BYTE *source = nullptr;
    DWORD max_length = 0;
    DWORD current_length = 0;

    hr = buffer->Lock(&source, &max_length, &current_length);
    if (FAILED(hr)) {
        DEBUG_ERROR("Media Foundation backend failed to lock an audio sample! Error code: 0x{:08x}.\n", hr);
        return false;
    }

    output.AudioChunk.TimestampMs = timestamp / 10000;
    output.AudioChunk.SampleRate = static_cast<int>(AudioRate);
    output.AudioChunk.Channels = static_cast<int>(AudioChannels);
    output.AudioChunk.Format = AudioFormat;
    output.AudioChunk.Samples.resize(current_length);
    std::memcpy(output.AudioChunk.Samples.data(), source, current_length);

    buffer->Unlock();
    output.HasAudioChunk = true;
    return true;
}


/**
 *  Reads the next sample from the source reader, producing at most one
 *  video frame or one audio chunk per call. Returns false on a read error.
 */
bool MediaFoundationMovieBackend::Pump(MovieDecodeOutput &output)
{
    output.Reset();

    if (!Reader || Finished) {
        output.EndOfStream = true;
        return true;
    }

    if (Paused) {
        return true;
    }

    DWORD stream_index = 0;
    DWORD flags = 0;
    LONGLONG timestamp = 0;
    ComPtr<IMFSample> sample;

    HRESULT hr = Reader->ReadSample(MF_SOURCE_READER_ANY_STREAM, 0, &stream_index, &flags, &timestamp, &sample);
    if (FAILED(hr)) {
        DEBUG_ERROR("Media Foundation backend failed while reading a movie sample! Error code: 0x{:08x}.\n", hr);
        return false;
    }

    if (flags & MF_SOURCE_READERF_ERROR) {
        DEBUG_ERROR("Media Foundation backend encountered a source reader error.\n");
        return false;
    }

    if (flags & MF_SOURCE_READERF_ENDOFSTREAM) {
        if (stream_index == VideoStreamIndex) {
            VideoStreamFinished = true;
        } else if (stream_index == AudioStreamIndex) {
            AudioStreamFinished = true;
        }
    }

    if (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED) {
        DEBUG_WARNING("Media Foundation backend media type changed during playback.\n");
    }

    if (!sample) {
        if (All_Streams_Finished()) {
            Finished = true;
            output.EndOfStream = true;
        }

        return true;
    }

    if (stream_index == VideoStreamIndex) {
        if (!Decode_Video_Sample(sample.Get(), timestamp, output)) {
            return false;
        }
    } else if (stream_index == AudioStreamIndex) {
        if (!Decode_Audio_Sample(sample.Get(), timestamp, output)) {
            return false;
        }
    }

    return true;
}


void MediaFoundationMovieBackend::Pause()
{
    Paused = true;
}


void MediaFoundationMovieBackend::Resume()
{
    Paused = false;
}


void MediaFoundationMovieBackend::Stop()
{
    Finished = true;

    if (Reader) {
        Reader->Flush(static_cast<DWORD>(MF_SOURCE_READER_ALL_STREAMS));
    }
}


bool MediaFoundationMovieBackend::Is_Finished() const
{
    return Finished;
}


int MediaFoundationMovieBackend::Get_Video_Width() const
{
    return static_cast<int>(VideoWidth);
}


int MediaFoundationMovieBackend::Get_Video_Height() const
{
    return static_cast<int>(VideoHeight);
}


const char *MediaFoundationMovieBackend::Get_Name() const
{
    return "Media Foundation";
}


std::unique_ptr<IMovieDecoderBackend> Create_MediaFoundationMovieBackend()
{
    return std::make_unique<MediaFoundationMovieBackend>();
}
