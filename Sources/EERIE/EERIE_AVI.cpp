/*
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
//-----------------------------------------------------------------------------
// Arkane Studios
//-----------------------------------------------------------------------------

#include <windows.h>
#include <amstream.h>
#include <control.h>
#include <uuids.h>
#include <amvideo.h>
#include <edevdefs.h>
#include <dshow.h>

#include "../danae/arx_menu2.h"			// CDirectInput

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

//-----------------------------------------------------------------------------
#pragma comment (lib, "amstrmid")

//-----------------------------------------------------------------------------
#define CLASSNAME "VideoWindow"
#define WM_GRAPHNOTIFY  WM_USER+13

//-----------------------------------------------------------------------------
extern CDirectInput * pGetInfoDirectInput;
extern long EDITMODE;
extern float Xratio;
extern float Yratio;

//-----------------------------------------------------------------------------
IGraphBuilder  * pGraph = NULL;
IMediaControl  * pMediaControl = NULL;
IVideoWindow  *  pVidWin = NULL;
IBasicVideo	*	pBasicVideo = NULL;
IMediaEvent	*	pMediaEvent = NULL;
bool			bSkipVideoIntro = false;
bool			bGameNotFirstLaunch = false;

//-----------------------------------------------------------------------------
void CleanUp(void)
{
	pMediaControl->Stop();
	pMediaEvent->Release();
	pMediaEvent = NULL;
	pMediaControl->Release();
	pVidWin->Release();
	pBasicVideo->Release();
	pGraph->Release();
}


//-----------------------------------------------------------------------------
bool PlayFile(HWND hWnd, char * file)
{
	HRESULT hr;
	// Create the filter graph manager.
	CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
	                 IID_IGraphBuilder, (void **)&pGraph);
	pGraph->QueryInterface(IID_IMediaControl, (void **)&pMediaControl);
	pGraph->QueryInterface(IID_IVideoWindow, (void **)&pVidWin);
	pGraph->QueryInterface(IID_IMediaEvent, (void **)&pMediaEvent);
	pGraph->QueryInterface(IID_IBasicVideo, (void **)&pBasicVideo);

	// Build the graph.
	WCHAR wFile[MAX_PATH];

	MultiByteToWideChar(CP_ACP, 0, file, -1, wFile, MAX_PATH);

	if (SUCCEEDED(hr = pGraph->RenderFile(wFile, NULL)))
	{
		//Set the video window.
		pVidWin->put_Owner((OAHWND)hWnd);
		pVidWin->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS);
	
		long w, h;
		RECT grc;
		GetClientRect(hWnd, &grc);
	
		pVidWin->SetWindowPosition(0, 0, grc.right, grc.bottom);
		w = grc.right;
		h = grc.bottom;

		char te[256];
		sprintf(te, "%d - %d", w, h);

		long lLeft, lTop;
		lLeft = 0;
		lTop = 0;
		pBasicVideo->SetDestinationPosition(lLeft, lTop, w, h);
		pBasicVideo->SetSourcePosition(lLeft, lTop, w, h);

		HDC hDC = GetDC(hWnd);
		FillRect(hDC, &grc, (HBRUSH) GetStockObject(BLACK_BRUSH));
		ReleaseDC(hWnd, hDC);

		// Run the graph.
		pMediaControl->Run();
		long l = 0;

		for (int i = 0; i < 256; i++)
		{
			pGetInfoDirectInput->iOneTouch[i] = 0;
		}

		while (!l && !bSkipVideoIntro)
		{
			if (bGameNotFirstLaunch || EDITMODE)
			{
				pGetInfoDirectInput->GetInput();

				for (int i = 0; i < 256; i++)
				{
					if (pGetInfoDirectInput->iOneTouch[i] > 0)
						bSkipVideoIntro = true;
				}
			}

			pMediaEvent->WaitForCompletion(0, &l); //INFINITE
		}

		CleanUp();

		return true;
	}
	else
	{
		CleanUp();
		return false;
	}
}

//-----------------------------------------------------------------------------
bool LaunchAVI(HWND hWnd, char * dest)
{
	CoInitialize(NULL);
	bool ret = PlayFile(hWnd, dest);
	CoUninitialize();
	return ret;
}
