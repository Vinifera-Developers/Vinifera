/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          DSHOWRENDERER.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         DirectShow video renderers.
 *
 *  @license       Vinifera is free software: you can redistribute it and/or
 *                 modify it under the terms of the GNU General Public License
 *                 as published by the Free Software Foundation, either version
 *                 3 of the License, or (at your option) any later version.
 *
 *                 Vinifera is distributed in the hope that it will be
 *                 useful, but WITHOUT ANY WARRANTY; without even the implied
 *                 warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *                 PURPOSE. See the GNU General Public License for more details.
 *
 *                 You should have received a copy of the GNU General Public
 *                 License along with this program.
 *                 If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/
#include "dshowrenderer.h"
#include "dshowutil.h"
#include "debughandler.h"
#include "asserthandler.h"
#include <Mfidl.h>


/**
 *  Initialize the VMR-7 for windowless mode. 
 */
HRESULT InitWindowlessVMR( 
    IBaseFilter *pVMR,              // Pointer to the VMR
    HWND hWnd,                      // Clipping window
    IVMRWindowlessControl **ppWC    // Receives a pointer to the VMR.
    ) 
{ 

    IVMRFilterConfig* pConfig = nullptr; 
    IVMRWindowlessControl *pWC = nullptr;

    /**
     *  Set the rendering mode.  
     */
    HRESULT hr = pVMR->QueryInterface(IID_PPV_ARGS(&pConfig)); 
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pConfig->SetRenderingMode(VMRMode_Windowless); 
    if (FAILED(hr))
    {
        goto done;
    }
    
    /**
     *  Query for the windowless control interface.
     */
    hr = pVMR->QueryInterface(IID_PPV_ARGS(&pWC));
    if (FAILED(hr))
    {
        goto done;
    }
    
    /**
     *  Set the clipping window.
     */
    hr = pWC->SetVideoClippingWindow(hWnd);
    if (FAILED(hr))
    {
        goto done;
    }
    
    /**
     *  Preserve aspect ratio by letter-boxing.
     */
    hr = pWC->SetAspectRatioMode(VMR_ARMODE_LETTER_BOX);
    if (FAILED(hr))
    {
        goto done;
    }

    /**
     *  Return the IVMRWindowlessControl pointer to the caller.
     */
    *ppWC = pWC;
    (*ppWC)->AddRef();

done:
    SafeRelease(&pConfig);
    SafeRelease(&pWC);
    return hr; 
} 



// Initialize the VMR-9 for windowless mode. 
/**
 *  
 */
HRESULT InitWindowlessVMR9( 
    IBaseFilter *pVMR,              // Pointer to the VMR
    HWND hWnd,                      // Clipping window
    IVMRWindowlessControl9 **ppWC   // Receives a pointer to the VMR.
    ) 
{ 

    IVMRFilterConfig9 * pConfig = nullptr; 
    IVMRWindowlessControl9 *pWC = nullptr;

    /**
     *  Set the rendering mode. 
     */ 
    HRESULT hr = pVMR->QueryInterface(IID_PPV_ARGS(&pConfig)); 
    if (FAILED(hr)) {
        goto done;
    }

    hr = pConfig->SetRenderingMode(VMR9Mode_Windowless); 
    if (FAILED(hr)) {
        goto done;
    }

    /**
     *  Query for the windowless control interface.
     */
    hr = pVMR->QueryInterface(IID_PPV_ARGS(&pWC));
    if (FAILED(hr)) {
        goto done;
    }

    /**
     *  Set the clipping window.
     */
    hr = pWC->SetVideoClippingWindow(hWnd);
    if (FAILED(hr)) {
        goto done;
    }

    /**
     *  Preserve aspect ratio by letter-boxing.
     */
    hr = pWC->SetAspectRatioMode(VMR9ARMode_LetterBox);
    if (FAILED(hr)) {
        goto done;
    }

    /**
     *  Return the IVMRWindowlessControl pointer to the caller.
     */
    *ppWC = pWC;
    (*ppWC)->AddRef();

done:
    SafeRelease(&pConfig);
    SafeRelease(&pWC);
    return hr; 
} 


/**
 *  Initialize the EVR filter.
 */
HRESULT InitializeEVR( 
    IBaseFilter *pEVR,              // Pointer to the EVR
    HWND hWnd,                      // Clipping window
    IMFVideoDisplayControl **ppDisplayControl
    ) 
{ 
    IMFGetService *pGS = nullptr;
    IMFVideoDisplayControl *pDisplay = nullptr;

    HRESULT hr = pEVR->QueryInterface(IID_PPV_ARGS(&pGS)); 
    if (FAILED(hr)) {
        goto done;
    }

    hr = pGS->GetService(MR_VIDEO_RENDER_SERVICE, IID_PPV_ARGS(&pDisplay));
    if (FAILED(hr)) {
        goto done;
    }

    /**
     *  Set the clipping window.
     */
    hr = pDisplay->SetVideoWindow(hWnd);
    if (FAILED(hr)) {
        goto done;
    }

    /**
     *  Preserve aspect ratio by letter-boxing.
     */
    hr = pDisplay->SetAspectRatioMode(MFVideoARMode_PreservePicture);
    if (FAILED(hr)) {
        goto done;
    }

    /**
     *  Return the IMFVideoDisplayControl pointer to the caller.
     */
    *ppDisplayControl = pDisplay;
    (*ppDisplayControl)->AddRef();

done:
    SafeRelease(&pGS);
    SafeRelease(&pDisplay);
    return hr; 
} 


/**
 *  VMR-7 Wrapper
 */
VMR7::VMR7() :
    m_pWindowless(nullptr)
{

}


VMR7::~VMR7()
{
    SafeRelease(&m_pWindowless);
}


BOOL VMR7::HasVideo() const 
{ 
    return (m_pWindowless != nullptr); 
}


HRESULT VMR7::AddToGraph(IGraphBuilder *pGraph, HWND hWnd)
{
    IBaseFilter *pVMR = nullptr;

    HRESULT hr = AddFilterByCLSID(pGraph, CLSID_VideoMixingRenderer, 
        &pVMR, L"VMR-7");

    if (SUCCEEDED(hr)) {
        /**
         *  Set windowless mode on the VMR. This must be done before the VMR
         *  is connected.
         */
        hr = InitWindowlessVMR(pVMR, hWnd, &m_pWindowless);
    }
    SafeRelease(&pVMR);
    return hr;
}

HRESULT VMR7::FinalizeGraph(IGraphBuilder *pGraph)
{
    if (m_pWindowless == nullptr) {
        return S_OK;
    }

    IBaseFilter *pFilter = nullptr;

    HRESULT hr = m_pWindowless->QueryInterface(IID_PPV_ARGS(&pFilter));
    if (FAILED(hr)) {
        goto done;
    }

    BOOL bRemoved;
    hr = RemoveUnconnectedRenderer(pGraph, pFilter, &bRemoved);

    /**
     *  If we removed the VMR, then we also need to release our 
     *  pointer to the VMR's windowless control interface.
     */
    if (bRemoved) {
        SafeRelease(&m_pWindowless);
    }

done:
    SafeRelease(&pFilter);
    return hr;
}


HRESULT VMR7::UpdateVideoWindow(HWND hWnd, const LPRECT prc)
{
    if (m_pWindowless == nullptr) {
        return S_OK; // no-op
    }

    if (prc) {
        return m_pWindowless->SetVideoPosition(nullptr, prc);
    } else {
        RECT rc;
        GetClientRect(hWnd, &rc);
        return m_pWindowless->SetVideoPosition(nullptr, &rc);
    }
}


HRESULT VMR7::Repaint(HWND hWnd, HDC hdc)
{
    if (m_pWindowless)
    {
        return m_pWindowless->RepaintVideo(hWnd, hdc);
    } else {
        return S_OK;
    }
}


HRESULT VMR7::DisplayModeChanged()
{
    if (m_pWindowless) {
        return m_pWindowless->DisplayModeChanged();
    } else {
        return S_OK;
    }
}


/**
 *  VMR-9 Wrapper
 */
VMR9::VMR9() : 
    m_pWindowless(nullptr)
{

}


BOOL VMR9::HasVideo() const 
{ 
    return (m_pWindowless != nullptr); 
}


VMR9::~VMR9()
{
    SafeRelease(&m_pWindowless);
}


HRESULT VMR9::AddToGraph(IGraphBuilder *pGraph, HWND hWnd)
{
    IBaseFilter *pVMR = nullptr;

    HRESULT hr = AddFilterByCLSID(pGraph, CLSID_VideoMixingRenderer9, 
        &pVMR, L"VMR-9");
    if (SUCCEEDED(hr)) {

        /**
         *  Set windowless mode on the VMR. This must be done before the VMR 
         *  is connected.
         */
        hr = InitWindowlessVMR9(pVMR, hWnd, &m_pWindowless);
    }
    SafeRelease(&pVMR);
    return hr;
}


HRESULT VMR9::FinalizeGraph(IGraphBuilder *pGraph)
{
    if (m_pWindowless == nullptr) {
        return S_OK;
    }

    IBaseFilter *pFilter = nullptr;

    HRESULT hr = m_pWindowless->QueryInterface(IID_PPV_ARGS(&pFilter));
    if (FAILED(hr)) {
        goto done;
    }

    BOOL bRemoved;
    hr = RemoveUnconnectedRenderer(pGraph, pFilter, &bRemoved);

    /**
     *  If we removed the VMR, then we also need to release our 
     *  pointer to the VMR's windowless control interface.
     */
    if (bRemoved)
    {
        SafeRelease(&m_pWindowless);
    }

done:
    SafeRelease(&pFilter);
    return hr;
}


HRESULT VMR9::UpdateVideoWindow(HWND hWnd, const LPRECT prc)
{
    if (m_pWindowless == nullptr) {
        return S_OK; // no-op
    }

    if (prc) {
        return m_pWindowless->SetVideoPosition(nullptr, prc);
    } else {

        RECT rc;
        GetClientRect(hWnd, &rc);
        return m_pWindowless->SetVideoPosition(nullptr, &rc);
    }
}


HRESULT VMR9::Repaint(HWND hWnd, HDC hdc)
{
    if (m_pWindowless) {
        return m_pWindowless->RepaintVideo(hWnd, hdc);
    } else {
        return S_OK;
    }
}


HRESULT VMR9::DisplayModeChanged()
{
    if (m_pWindowless) {
        return m_pWindowless->DisplayModeChanged();
    } else {
        return S_OK;
    }
}


/**
 *  EVR Wrapper
 */
EVR::EVR() :
    m_pEVR(nullptr),
    VideoDisplay(nullptr)
{

}


EVR::~EVR()
{
    SafeRelease(&m_pEVR);
    SafeRelease(&VideoDisplay);
}


BOOL EVR::HasVideo() const 
{ 
    return (VideoDisplay != nullptr); 
}


HRESULT EVR::AddToGraph(IGraphBuilder *pGraph, HWND hWnd)
{
    IBaseFilter *pEVR = nullptr;

    HRESULT hr = AddFilterByCLSID(pGraph, CLSID_EnhancedVideoRenderer, 
        &pEVR, L"EVR");

    if (FAILED(hr)) {
        DEBUG_ERROR("DirectShow(EVR): AddFilterByCLSID failed!\n");
        goto done;
    }

    hr = InitializeEVR(pEVR, hWnd, &VideoDisplay);
    if (FAILED(hr)) {
        DEBUG_ERROR("DirectShow(EVR): InitializeEVR failed!\n");
        goto done;
    }

    /**
     *  NOTE: Because IMFVideoDisplayControl is a service interface,
     *  you cannot QI the pointer to get back the IBaseFilter pointer.
     *  Therefore, we need to cache the IBaseFilter pointer.
     */
    m_pEVR = pEVR;
    m_pEVR->AddRef();

done:
    SafeRelease(&pEVR);
    return hr;
}


HRESULT EVR::FinalizeGraph(IGraphBuilder *pGraph)
{
    if (m_pEVR == nullptr) {
        return S_OK;
    }

    BOOL bRemoved;
    HRESULT hr = RemoveUnconnectedRenderer(pGraph, m_pEVR, &bRemoved);
    if (bRemoved) {
        SafeRelease(&m_pEVR);
        SafeRelease(&VideoDisplay);
    }
    return hr;
}


HRESULT EVR::UpdateVideoWindow(HWND hWnd, const LPRECT prc)
{
    if (VideoDisplay == nullptr) {
        return S_OK; // no-op
    }

    if (prc) {
        return VideoDisplay->SetVideoPosition(nullptr, prc);
    } else {

        RECT rc;
        GetClientRect(hWnd, &rc);
        return VideoDisplay->SetVideoPosition(nullptr, &rc);
    }
}


HRESULT EVR::Repaint(HWND hWnd, HDC hdc)
{
    if (VideoDisplay) {
        return VideoDisplay->RepaintVideo();
    } else {
        return S_OK;
    }
}


HRESULT EVR::DisplayModeChanged()
{
    /**
     *  The EVR does not require any action in response to WM_DISPLAYCHANGE.
     */
    return S_OK;
}
