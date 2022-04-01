/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          DSHOWRENDERER.H
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
#pragma once

#include "always.h"
#include <dshow.h>
#include <d3d9.h>
#include <Vmr9.h>
#include <Evr.h>


/**
 * 
 *  Heavily based on Windows SDK sample code.
 * 
 */


/**
 *  Abstract class to manage the video renderer filter.
 *  Specific implementations handle the VMR-7, VMR-9, or EVR filter.
 */
class BaseVideoRenderer
{
    public:
        BaseVideoRenderer() {}
        virtual ~BaseVideoRenderer() {}

        virtual BOOL HasVideo() const = 0;
        virtual HRESULT AddToGraph(IGraphBuilder *pGraph, HWND hwnd) = 0;
        virtual HRESULT FinalizeGraph(IGraphBuilder *pGraph) = 0;
        virtual HRESULT UpdateVideoWindow(HWND hwnd, const LPRECT prc) = 0;
        virtual HRESULT Repaint(HWND hwnd, HDC hdc) = 0;
        virtual HRESULT DisplayModeChanged() = 0;
};


/**
 *  Manages the VMR-7 (Video Mixing Renderer Filter 7) video renderer filter.
 */
class VMR7 : public BaseVideoRenderer
{
    public:
        VMR7();
        ~VMR7();

        BOOL HasVideo() const;
        HRESULT AddToGraph(IGraphBuilder *pGraph, HWND hwnd);
        HRESULT FinalizeGraph(IGraphBuilder *pGraph);
        HRESULT UpdateVideoWindow(HWND hwnd, const LPRECT prc);
        HRESULT Repaint(HWND hwnd, HDC hdc);
        HRESULT DisplayModeChanged();
        
    public:
        IVMRWindowlessControl *m_pWindowless;
};


/**
 *  Manages the VMR-9 (Video Mixing Renderer Filter 9) video renderer filter.
 */
class VMR9 : public BaseVideoRenderer
{
    public:
        VMR9();
        ~VMR9();

        BOOL HasVideo() const;
        HRESULT AddToGraph(IGraphBuilder *pGraph, HWND hwnd);
        HRESULT FinalizeGraph(IGraphBuilder *pGraph);
        HRESULT UpdateVideoWindow(HWND hwnd, const LPRECT prc);
        HRESULT Repaint(HWND hwnd, HDC hdc);
        HRESULT DisplayModeChanged();

    public:
        IVMRWindowlessControl9 *m_pWindowless;
};


/**
 *  Manages the EVR (Enhanced) video renderer filter.
 */
class EVR : public BaseVideoRenderer
{
    public:
        EVR();
        ~EVR();

        BOOL HasVideo() const;
        HRESULT AddToGraph(IGraphBuilder *pGraph, HWND hwnd);
        HRESULT FinalizeGraph(IGraphBuilder *pGraph);
        HRESULT UpdateVideoWindow(HWND hwnd, const LPRECT prc);
        HRESULT Repaint(HWND hwnd, HDC hdc);
        HRESULT DisplayModeChanged();

    public:
        IBaseFilter *m_pEVR;
        IMFVideoDisplayControl *VideoDisplay;
};


HRESULT InitializeEVR(IBaseFilter *pEVR, HWND hwnd, IMFVideoDisplayControl ** ppWc); 
HRESULT InitWindowlessVMR9(IBaseFilter *pVMR, HWND hwnd, IVMRWindowlessControl9 ** ppWc); 
HRESULT InitWindowlessVMR(IBaseFilter *pVMR, HWND hwnd, IVMRWindowlessControl** ppWc); 
