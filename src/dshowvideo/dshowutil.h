/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          DSHOWUTIL.H
 *
 *  @author        CCHyper
 *
 *  @brief         DirectShow utility functions.
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

#include <windows.h>
#include <dshow.h>


template <class T> void SafeRelease(T **ppT) { if (*ppT) { (*ppT)->Release(); *ppT = nullptr; } }


#ifndef SAFE_ARRAY_DELETE
#define SAFE_ARRAY_DELETE(x) { if(x) { delete[] x; x = nullptr; } }
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(x) { delete x; x = nullptr; }
#endif


HRESULT RemoveUnconnectedRenderer(IGraphBuilder *pGraph, IBaseFilter *pRenderer, BOOL *pbRemoved);
HRESULT IsPinConnected(IPin *pPin, BOOL *pResult);
HRESULT IsPinDirection(IPin *pPin, PIN_DIRECTION dir, BOOL *pResult);
HRESULT FindConnectedPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin **ppPin);
HRESULT AddFilterByCLSID(IGraphBuilder *pGraph, REFGUID clsid, IBaseFilter **ppF, LPCWSTR wszName);
