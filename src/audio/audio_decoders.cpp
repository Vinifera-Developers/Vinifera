/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Custom decoders for Miniaudio
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "audio_decoders.h"

#include "audio_debug.h"

#include <algorithm>


/**
 * 
 * MHI SOS ADPCM decoder for Westwood Studios AUD encapsulated files for Miniaudio
 * 
 */
#ifndef MA_NO_AUD

/**
 *  AUD format Format constants
 */
constexpr ma_uint32 MA_AUD_MAGIC = 0x0000DEAF;  // Expected chunk header magic
constexpr ma_uint8 MA_AUD_CODEC_SOS = 99;       // Codec ID for WS/SOS IMA ADPCM
constexpr ma_uint8 MA_AUD_FLAG_STEREO = 1;      // Flag bit for stereo audio
constexpr ma_uint8 MA_AUD_FLAG_16BIT = 2;       // Flag bit for 16-bit samples (otherwise 8-bit assumed)


/**
 *  AUD header and chunk struct
 */
#pragma pack(push, 1) // Header must be packed!
typedef struct {
    ma_uint16 sampleRate; // Sample rate of the audio (e.g., 11025, 22050, 44100 Hz)
    ma_uint32 compSize; // Total size of compressed audio data (in bytes)
    ma_uint32 uncompSize; // Expected size of decompressed PCM data (in bytes)
    ma_uint8 flags; // Bitmask for format options (e.g., stereo/mono, 8/16-bit)
    ma_uint8 compression; // Compression codec ID
} ma_aud_header;
#pragma pack(pop)

#pragma pack(push, 1) // Header must be packed!
typedef struct {
    ma_uint16 compSize; // Size of compressed chunk data following this header (in bytes)
    ma_uint16 decompSize; // Expected output size after decompression (in bytes)
    ma_uint32 magic; // Chunk magic value to validate structure (must match MA_AUD_MAGIC)
} ma_aud_chunk_header;
#pragma pack(pop)


// Pull in the defintions for the SOS codec decoder from TS++
#include "soscodec.h"


/**
 *  The main decoder structure/context for SOS-compressed AUD files using MiniAudio.
 */
typedef struct ma_sos_aud_decoder
{
    ma_data_source_base base;       // Required by MiniAudio. Must be the first member to support polymorphism.

    /**
     *  Compressed input state
     */
    ma_decoder_config config;        // Holds decoder configuration (format, sample rate, etc.)
    ma_uint8* pCompressed;           // Pointer to the full compressed data loaded in memory
    ma_uint64 compressedSize;        // Total size of the compressed buffer in bytes
    ma_uint64 compCursor;            // Current byte offset in the compressed data stream
    ma_uint64 totalFrameCount;       // Total number of PCM frames after full decompression
    ma_uint32 frameSize;             // Size of a single PCM frame in bytes (sample size * channel count)

    ma_aud_header header;            // Original AUD file header, parsed from the stream

    /**
     *  Output format metadata
     */
    ma_format format;                // Sample format (e.g., ma_format_s16)
    ma_uint8 channels;               // Number of audio channels (1 = mono, 2 = stereo)
    ma_uint16 sampleRate;            // Sampling rate (e.g., 22050 Hz)

    /**
     *  SOS codec state
     */
    _SOS_COMPRESS_INFO_2 compInfo;   // Structure used by SOS decoder, includes decoding history and buffers

    /**
     *  Ring buffer for decoded audio (used for streaming)
     */
    ma_rb rbDecodedPCM;              // MiniAudio ring buffer for buffering decompressed PCM frames
    void* rbDecodedPCMData;          // Backing memory for ring buffer data (allocated in init)

} ma_sos_aud_decoder;


/**
 *  Size of the ring buffer in bytes (not samples!). 16384 bytes is enough for ~2048 16-bit stereo samples.
 */
#define AUD_RING_BUFFER_CAPACITY 16384


/**
 * Forward declarations for SOS AUD custom decoder.
 * These are required so we can reference the functions in vtables before their definitions appear.
 */
static ma_result ma_sos_aud_read_pcm_frames(ma_data_source* pDS, void* pOut, ma_uint64 frameCount, ma_uint64* pFramesRead);
static ma_result ma_sos_aud_seek_to_pcm_frame(ma_data_source* pDS, ma_uint64 frameIndex);
static ma_result ma_sos_aud_get_data_format(ma_data_source* pDS, ma_format* pFormat, ma_uint32* pChannels, ma_uint32* pSampleRate, ma_channel* pChannelMap, size_t channelMapCap);
static ma_result ma_sos_aud_get_cursor(ma_data_source* pDS, ma_uint64* pCursor);
static ma_result ma_sos_aud_get_length(ma_data_source* pDS, ma_uint64* pLength);

static ma_result ma_sos_aud_init(void* pUserData, ma_read_proc onRead, ma_seek_proc onSeek, ma_tell_proc onTell, void* pReadSeekTellUserData, const ma_decoding_backend_config* pConfig, const ma_allocation_callbacks* pAllocationCallbacks, ma_data_source** ppBackend);
static void ma_sos_aud_uninit(void* pUserData, ma_data_source* pBackend, const ma_allocation_callbacks* pAllocationCallbacks);


/**
 *  MiniAudio data source virtual function table for the SOS AUD decoder.
 */
static ma_data_source_vtable g_ma_sos_aud_decoder_vtable = {
    ma_sos_aud_read_pcm_frames,
    ma_sos_aud_seek_to_pcm_frame,
    ma_sos_aud_get_data_format,
    ma_sos_aud_get_cursor,
    ma_sos_aud_get_length
};

/**
 *  Decoding backend vtable for the SOS ADPCM decoder.
 *  This registers the decoder with MiniAudio's decoding system.
 */
static ma_decoding_backend_vtable ma_decoding_backend_vtable_sos = {
    ma_sos_aud_init,
    nullptr,
    nullptr,
    nullptr,
    ma_sos_aud_uninit
};


/**
 *  Reads and validates the header of a Westwood-style AUD file.
 */
static ma_result ma_aud_read_header(ma_read_proc onRead, void* pUserData, ma_aud_header* pHeaderOut, ma_allocation_callbacks const* pAllocationCallbacks)
{
    // Validate input arguments
    if (onRead == nullptr || pHeaderOut == nullptr) {
        return MA_INVALID_ARGS;
    }

    size_t bytesRead = 0;

    // Attempt to read the full AUD header structure from the stream
    if (onRead(pUserData, pHeaderOut, sizeof(ma_aud_header), &bytesRead) != MA_SUCCESS || bytesRead != sizeof(ma_aud_header)) {
        return MA_IO_ERROR; // Could not read the full header
    }

    // Reject unsupported or unknown sample rates
    if (pHeaderOut->sampleRate != 11025 &&
        pHeaderOut->sampleRate != 22050 &&
        pHeaderOut->sampleRate != 44100)
    {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_DECODER, "AudioDecoder[AUD]: Unsupported sample-rate: %d!\n", pHeaderOut->sampleRate);
        return MA_ERROR;
    }

    // Log basic header info
    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_DECODER,
        "AudioDecoder[AUD]: Header: sampleRate=%u, compression=%u, flags=0x%02X\n",
        pHeaderOut->sampleRate,
        pHeaderOut->compression,
        pHeaderOut->flags);

    // Reject any unsupported codec types. Only the SOS (Westwood IMA ADPCM) format is supported.
    if (pHeaderOut->compression != MA_AUD_CODEC_SOS) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_DECODER, "AudioDecoder[AUD]: Unsupported codec: %d!\n", pHeaderOut->compression);
        return MA_NOT_IMPLEMENTED;
    }

    // Ensure no unknown flag bits are present
    if (pHeaderOut->flags & ~(MA_AUD_FLAG_STEREO | MA_AUD_FLAG_16BIT)) {
        AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_DECODER,
            "AudioDecoder[AUD]: Unexpected flag bits (0x%02X)!\n", pHeaderOut->flags);
        return MA_ERROR;
    }

    return MA_SUCCESS;
}


/**
 *  Reads PCM frames from the AUD decoder into the output buffer. This function uses a ring buffer
 *  to manage decoded data and decodes new chunks as needed to fulfill the request.
 */
static ma_result ma_sos_aud_read_pcm_frames(
    ma_data_source* pDataSource,
    void* pFramesOut,
    ma_uint64 frameCount,
    ma_uint64* pFramesRead
)
{
    ma_sos_aud_decoder* pDecoder = (ma_sos_aud_decoder*)pDataSource;
    ma_uint8* dst = (ma_uint8*)pFramesOut;
    ma_uint64 totalFramesRead = 0;
    size_t bytesRequested = frameCount * pDecoder->frameSize;

    /**
     *  Step 1: Attempt to read already-decoded PCM data from the ring buffer first.
     */
    {
        size_t availableBytes = ma_rb_available_read(&pDecoder->rbDecodedPCM);
        size_t bytesToRead = std::min(bytesRequested, availableBytes);

        if (bytesToRead > 0) {
            void* pRead;
            ma_rb_acquire_read(&pDecoder->rbDecodedPCM, &bytesToRead, &pRead); // Lock read region
            memcpy(dst, pRead, bytesToRead); // Copy data to output
            ma_rb_commit_read(&pDecoder->rbDecodedPCM, bytesToRead); // Commit read

            dst += bytesToRead;
            bytesRequested -= bytesToRead;
            totalFramesRead += bytesToRead / pDecoder->frameSize;
        }
    }

    /**
     *  Step 2: Decode more chunks as needed until we satisfy the requested number of frames.
     */
    ma_uint8 tempBuffer[4096];  // Temporary buffer for decoded chunk output

    while (bytesRequested > 0 && pDecoder->compCursor < pDecoder->compressedSize) {

        // Ensure we have enough bytes to read a chunk header
        if (pDecoder->compCursor + sizeof(ma_aud_chunk_header) > pDecoder->compressedSize) {
            break;
        }

        // Read the chunk header
        const ma_aud_chunk_header* pChunkHeader = (const ma_aud_chunk_header*)(pDecoder->pCompressed + pDecoder->compCursor);

        // Validate the chunk magic
        if (pChunkHeader->magic != MA_AUD_MAGIC) {
            AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_DECODER, "Invalid chunk magic: 0x%08X\n", pChunkHeader->magic);
            break;
        }

        // Validate chunk boundaries to avoid buffer overrun
        if (pDecoder->compCursor + sizeof(ma_aud_chunk_header) + pChunkHeader->compSize > pDecoder->compressedSize) {
            AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_DECODER, "Chunk buffer overrun at cursor=%llu\n", pDecoder->compCursor);
            break;
        }

        /**
         *  Prepare decompression for this chunk
         */
        pDecoder->compInfo.lpSource = pDecoder->pCompressed + pDecoder->compCursor + sizeof(ma_aud_chunk_header);
        pDecoder->compInfo.lpDest = tempBuffer;
        pDecoder->compInfo.dwCompSize = pChunkHeader->compSize;
        pDecoder->compInfo.dwUnCompSize = pChunkHeader->decompSize;

        long decodedBytes = sosCODEC2DecompressData(&pDecoder->compInfo, (long)pChunkHeader->decompSize);
        if (decodedBytes <= 0 || decodedBytes > (long)sizeof(tempBuffer)) {
            AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_DECODER, "Decompress failed at cursor=%llu\n", pDecoder->compCursor);
            break;
        }

        /**
         *  Clamp 16-bit samples to valid range. Unlikley this will happen, but just to be safe!
         */
        ma_int16* samples = (ma_int16*)tempBuffer;
        ma_uint64 sampleCount = decodedBytes / sizeof(ma_int16);
        for (ma_uint64 i = 0; i < sampleCount; ++i) {
            if (samples[i] > 32767)       samples[i] = 32767;
            else if (samples[i] < -32768) samples[i] = -32768;
        }

        /**
         *  Step 3: Write decoded PCM data into the ring buffer.
         */
        size_t written = 0;
        while (written < (size_t)decodedBytes)
        {
            size_t writeCap = ma_rb_available_write(&pDecoder->rbDecodedPCM);
            size_t writeNow = std::min(writeCap, (size_t)decodedBytes - written);

            if (writeNow == 0) break;  // Ring buffer is full; stop writing

            void* pWrite;
            ma_rb_acquire_write(&pDecoder->rbDecodedPCM, &writeNow, &pWrite);     // Lock write region
            memcpy(pWrite, tempBuffer + written, writeNow);                       // Copy decoded data
            ma_rb_commit_write(&pDecoder->rbDecodedPCM, writeNow);                // Commit write

            written += writeNow;
        }

        // Move past this compressed chunk in the stream
        pDecoder->compCursor += sizeof(ma_aud_chunk_header) + pChunkHeader->compSize;

        /**
         *  Step 4: Attempt to read more from the ring buffer to fulfill the request.
         */
        size_t availableBytes = ma_rb_available_read(&pDecoder->rbDecodedPCM);
        size_t bytesToRead = std::min(bytesRequested, availableBytes);

        if (bytesToRead > 0) {
            void* pRead;
            ma_rb_acquire_read(&pDecoder->rbDecodedPCM, &bytesToRead, &pRead); // Lock read region
            memcpy(dst, pRead, bytesToRead); // Copy to output
            ma_rb_commit_read(&pDecoder->rbDecodedPCM, bytesToRead); // Commit read

            dst += bytesToRead;
            bytesRequested -= bytesToRead;
            totalFramesRead += bytesToRead / pDecoder->frameSize;
        }
    }

    // Report how many frames were actually read
    if (pFramesRead) {
        *pFramesRead = totalFramesRead;
    }

    /**
     *  Signal end-of-stream when the compressed input is exhausted and the
     *  decoded ring buffer is drained.
     */
    if (totalFramesRead < frameCount
        && pDecoder->compCursor >= pDecoder->compressedSize
        && ma_rb_available_read(&pDecoder->rbDecodedPCM) == 0) {
        return MA_AT_END;
    }

    return MA_SUCCESS;
}


/**
 *  Seeks the decoder to a specific PCM frame index.
 *  Currently, only seeking to the beginning (frame 0) is supported.
 */
static ma_result ma_sos_aud_seek_to_pcm_frame(ma_data_source* pDataSource, ma_uint64 frameIndex)
{
    ma_sos_aud_decoder* pDecoder = (ma_sos_aud_decoder*)pDataSource;

    // Seeking to arbitrary frame positions is not implemented!
    // Only support seeking to the start of the stream for now.
    if (frameIndex != 0) {
        return MA_NOT_IMPLEMENTED;
    }

    // Reset the decoder's internal state.
    pDecoder->compCursor = 0;              // Start at the beginning of the compressed buffer.
    pDecoder->compInfo.dwPredicted = 0;    // Reset internal predictor state (SOS-specific).
    pDecoder->compInfo.wIndex = 0;         // Reset index used for ADPCM step table.

    // Reinitialize the decompression stream state.
    sosCODEC2InitStream(&pDecoder->compInfo);

    return MA_SUCCESS;
}


/**
 *  Reports the format of the audio stream: sample format, number of channels, and sample rate.
 *  This function is called by MiniAudio when it wants to know how to configure the playback pipeline.
 */
static ma_result ma_sos_aud_get_data_format(ma_data_source* pDataSource, ma_format* pFormat, ma_uint32* pChannels, ma_uint32* pSampleRate, ma_channel* pChannelMap, size_t channelMapCap)
{
   // This decoder does not currently provide a channel map.
    // MiniAudio will use its default if these are not set.
    (void)pChannelMap;
    (void)channelMapCap;

    ma_sos_aud_decoder* pDecoder = (ma_sos_aud_decoder*)pDataSource;

    if (pFormat) {
        *pFormat = pDecoder->format;
    }
    if (pChannels) {
        *pChannels = pDecoder->channels;
    }
    if (pSampleRate) {
        *pSampleRate = pDecoder->sampleRate;
    }

    return MA_SUCCESS;
}


/**
 *  Reports the current playback position in PCM frames.
 *  This is used by MiniAudio to determine where in the stream the playback is.
 */
static ma_result ma_sos_aud_get_cursor(ma_data_source* pDataSource, ma_uint64* pCursor)
{
    ma_sos_aud_decoder* pDecoder = (ma_sos_aud_decoder*)pDataSource;

    if (pCursor) {
        // Each decoded chunk is typically 2048 bytes (or similar), and each frame is 2 or 4 bytes.
        // Since compCursor is in compressed bytes, we estimate frame position by dividing.
        // Adjust divisor if necessary based on format specifics.
        *pCursor = pDecoder->compCursor / pDecoder->frameSize;
    }

    return MA_SUCCESS;
}


/**
 *  Returns the total number of PCM frames in the stream.
 *  This value is determined from the AUD header on load.
 */
static ma_result ma_sos_aud_get_length(ma_data_source* pDataSource, ma_uint64* pLength)
{
    ma_sos_aud_decoder* pDecoder = (ma_sos_aud_decoder*)pDataSource;

    if (pLength) {
        *pLength = pDecoder->totalFrameCount;  // Precomputed during init from header.uncompsize
    }

    return MA_SUCCESS;
}


/**
 *  Initializes the custom AUD decoder and prepares it for use by MiniAudio.
 */
static ma_result ma_sos_aud_init(
    void* pUserData,
    ma_read_proc onRead,
    ma_seek_proc onSeek,
    ma_tell_proc onTell,
    void* pReadSeekTellUserData,
    const ma_decoding_backend_config* pConfig,
    const ma_allocation_callbacks* pAllocationCallbacks,
    ma_data_source** ppBackend
)
{
    ma_result result;

    // Allocate memory for our custom decoder structure.
    ma_sos_aud_decoder* pDecoder = (ma_sos_aud_decoder*)ma_malloc(sizeof(ma_sos_aud_decoder), pAllocationCallbacks);
    if (pDecoder == NULL) {
        return MA_OUT_OF_MEMORY;
    }

    // Zero the decoder memory for safety.
    memset(pDecoder, 0, sizeof(*pDecoder));

    // Read and validate the AUD file header using helper function.
    ma_aud_header audHeader;
    ma_result headerResult = ma_aud_read_header(onRead, pReadSeekTellUserData, &audHeader, pAllocationCallbacks);
    if (headerResult != MA_SUCCESS) {
        ma_free(pDecoder, pAllocationCallbacks);
        return headerResult;
    }

    // Store the parsed AUD header in our decoder.
    pDecoder->header = audHeader;

    // Derive format information from header flags.
    pDecoder->format = (pDecoder->header.flags & MA_AUD_FLAG_16BIT) ? ma_format_s16 : ma_format_u8;
    pDecoder->channels = (pDecoder->header.flags & MA_AUD_FLAG_STEREO) ? 2 : 1;
    pDecoder->sampleRate = pDecoder->header.sampleRate;

    // Save the compressed size and calculate uncompressed frame info.
    pDecoder->compressedSize = pDecoder->header.compSize;
    pDecoder->frameSize = pDecoder->channels * ma_get_bytes_per_sample(pDecoder->format);;
    pDecoder->totalFrameCount = pDecoder->header.uncompSize / (sizeof(ma_int16) * pDecoder->channels);

    // Reject large files for safety (arbitrary 64MB limit).
    ma_uint64 compSize = pDecoder->header.compSize;
    if (compSize > 64 * 1024 * 1024) return MA_ERROR; // arbitrary 64MB limit

    // Allocate memory to hold the full compressed data stream.
    void* pCompressed = ma_malloc((size_t)compSize, pAllocationCallbacks);
    if (pCompressed == NULL) {
        ma_free(pDecoder, pAllocationCallbacks);
        return MA_OUT_OF_MEMORY;
    }

    // Read the full compressed stream into memory.
    size_t bytesRead = 0;
    if (onRead(pReadSeekTellUserData, pCompressed, (size_t)compSize, &bytesRead) != MA_SUCCESS || bytesRead != compSize) {
        ma_free(pCompressed, pAllocationCallbacks);
        ma_free(pDecoder, pAllocationCallbacks);
        return MA_IO_ERROR;
    }

    // Save compressed data pointer in decoder.
    pDecoder->pCompressed = (ma_uint8*)pCompressed;

    // Setup initial codec decompression info for SOS decoder.
    std::memset(&pDecoder->compInfo, 0, sizeof(pDecoder->compInfo));
    pDecoder->compInfo.lpSource = (ma_uint8 *)pCompressed;
    pDecoder->compInfo.lpDest = NULL; // Will be set during decode
    pDecoder->compCursor = 0;
    pDecoder->compInfo.dwCompSize = pDecoder->header.compSize;
    pDecoder->compInfo.dwUnCompSize = pDecoder->header.uncompSize;
    pDecoder->compInfo.wBitSize = pDecoder->format == ma_format_s16 ? 16 : 8;
    pDecoder->compInfo.wChannels = pDecoder->channels;

    // Call SOS codec init function to prepare for decompression.
    sosCODEC2InitStream(&pDecoder->compInfo);

    // Initialize the MiniAudio data source system with our virtual method table.
    ma_data_source_config dsConfig = ma_data_source_config_init();
    dsConfig.vtable = &g_ma_sos_aud_decoder_vtable;

    result = ma_data_source_init(&dsConfig, &pDecoder->base);
    if (result != MA_SUCCESS) {
        ma_free(pCompressed, pAllocationCallbacks);
        ma_free(pDecoder, pAllocationCallbacks);
        return result;
    }

    // Assign the initialized backend to the caller's output pointer.
    *ppBackend = &pDecoder->base;

    // Allocate a ring buffer to hold decompressed PCM data.
    pDecoder->rbDecodedPCMData = ma_malloc(AUD_RING_BUFFER_CAPACITY, pAllocationCallbacks);
    if (pDecoder->rbDecodedPCMData == NULL) {
        return MA_OUT_OF_MEMORY;
    }

    // Initialize the ring buffer with our allocation.
    result = ma_rb_init(AUD_RING_BUFFER_CAPACITY, pDecoder->rbDecodedPCMData, pAllocationCallbacks, &pDecoder->rbDecodedPCM);
    if (result != MA_SUCCESS) {
        return result;
    }

    return MA_SUCCESS;
}


/**
 *  This function cleans up and releases all resources used by the custom AUD decoder.
 *  It is called by MiniAudio when the decoder is being destroyed.
 */
static void ma_sos_aud_uninit(void* pUserData, ma_data_source* pBackend, const ma_allocation_callbacks* pAllocationCallbacks)
{
    // Cast the backend pointer to our custom decoder type.
    ma_sos_aud_decoder* pDecoder = (ma_sos_aud_decoder*)pBackend;
    if (pDecoder == NULL) {
        return;
    }

    // Free the buffer holding the compressed AUD file data, if allocated.
    if (pDecoder->pCompressed) {
        ma_free(pDecoder->pCompressed, pAllocationCallbacks);
    }

    // Uninitialize the decoded PCM ring buffer and free its backing memory.
    ma_rb_uninit(&pDecoder->rbDecodedPCM);                    // Clean up the ring buffer
    ma_free(pDecoder->rbDecodedPCMData, pAllocationCallbacks);    // Free the ring buffer's allocated memory

    // Uninitialize the MiniAudio data source base (required cleanup step).
    ma_data_source_uninit(&pDecoder->base);

    // Finally, free the decoder structure itself.
    ma_free(pDecoder, pAllocationCallbacks);
}


#endif // MA_NO_SOS


/**
 *  Define our custom decoders here!
 * 
 *  NOTE: This must be the last in the file as the vtables and functions are local to this source file.
 */
/*static*/ const ma_decoding_backend_vtable * ma_custom_backend_vtable[1] = {
#ifndef MA_NO_AUD
    &ma_decoding_backend_vtable_sos     // SOS decoder (that parses the Westwood AUD container)
#endif
};
