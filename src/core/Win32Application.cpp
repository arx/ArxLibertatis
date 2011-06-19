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

#include "core/Win32Application.h"

#include <cstdio>

#include "animation/Cinematic.h"

#include "core/Dialog.h"
#include "core/GameTime.h"
#include "core/Resource.h"

#include "game/Levels.h"
#include "game/Player.h"

#include "graphics/data/Mesh.h"
#include "graphics/Frame.h"
#include "graphics/GraphicsEnum.h"

#include "io/CinematicLoad.h"
#include "io/Logger.h"
#include "io/PakManager.h"

#include "scene/ChangeLevel.h"

extern long ALLOW_CHEATS;
extern long FINAL_COMMERCIAL_GAME;
extern long FINAL_COMMERCIAL_DEMO;
extern long GAME_EDITOR;
extern long PLAY_LOADED_CINEMATIC;
extern char LAST_LAUNCHED_CINE[256];
extern char WILL_LAUNCH_CINE[256];
extern long CHANGE_LEVEL_PROC_RESULT;

extern void DANAE_KillCinematic();
extern void LaunchWaitingCine();

namespace
{

	//*************************************************************************************
	// WndProc()
	//  Static msg handler which passes messages to the application class.
	//*************************************************************************************
	LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (g_pD3DApp)
			// Cast is fine, will always be a Win32Application* if set up for Windows
			return dynamic_cast<Win32Application*>(g_pD3DApp)->MsgProc(hWnd, uMsg, wParam, lParam);

		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

}

//*************************************************************************************
// DANAE()
//  Application constructor. Sets attributes for the app.
//*************************************************************************************
Win32Application::Win32Application() : Application()
{
	m_strWindowTitle  = TEXT("ARX Fatalis");
	m_bAppUseZBuffer  = true;
	m_bAppUseStereo   = false;
	m_bShowStats      = true;
	m_fnConfirmDevice = NULL;
	m_hWnd=NULL;
}

//*************************************************************************************
// Create()
//*************************************************************************************
bool Win32Application::Create() {
	
	HINSTANCE hInst = GetModuleHandle(0);

	HRESULT hr;

	// Enumerate available D3D devices. The callback is used so the app can
	// confirm/reject each enumerated device depending on its capabilities.
	if (FAILED(hr = D3DEnum_EnumerateDevices(m_fnConfirmDevice)))
	{
		DisplayFrameworkError(hr, MSGERR_APPMUSTEXIT);
		return false;
	}

	// Select a device. Ask for a hardware device that renders in a window.
	if (FAILED(hr = D3DEnum_SelectDefaultDevice(&m_pDeviceInfo)))
	{
		DisplayFrameworkError(hr, MSGERR_APPMUSTEXIT);
		return false;
	}

	// Initialize the app's custom scene stuff
	if (FAILED(hr = OneTimeSceneInit()))
	{
		DisplayFrameworkError(hr, MSGERR_APPMUSTEXIT);
		return false;
	}

	// Create a new CD3DFramework class. This class does all of our D3D
	// initialization and manages the common D3D objects.
	if (NULL == (m_pFramework = new CD3DFramework7()))
	{
		DisplayFrameworkError(E_OUTOFMEMORY, MSGERR_APPMUSTEXIT);
		return false;
	}

	//	DisplayFrameworkError( hr, MSGERR_APPMUSTEXIT );
#ifdef BUILD_EDITOR
	if (ToolBar != NULL)
	{
		if (this->ToolBar->Type == EERIE_TOOLBAR_TOP)
		{
			this->m_pFramework->Ystart = 26;
			this->m_pFramework->Xstart = 0;
		}
		else
		{
			this->m_pFramework->Ystart = 0;
			this->m_pFramework->Xstart = 98;
		}
	}
	else
#endif
	{
		this->m_pFramework->Ystart = 0;
		this->m_pFramework->Xstart = 0;
	}


	// Register the window class
	WNDCLASS wndClass = { CS_DBLCLKS, WndProc, 0, 0, hInst,
	                      LoadIcon(hInst, MAKEINTRESOURCE(IDI_MAIN)),
	                      LoadCursor(NULL, IDC_ARROW),
	                      (HBRUSH)GetStockObject(BLACK_BRUSH),
	                      NULL, "D3D Window"
	                    };
	RegisterClass(&wndClass);

	// Create the render window
	DWORD flags = WS_OVERLAPPEDWINDOW;

	if (CreationFlags & WCF_NORESIZE)
	{
		flags |= (WS_DLGFRAME | WS_MINIMIZEBOX);
		flags &= ~WS_CAPTION;
		flags &= ~WS_MAXIMIZEBOX;
		flags &= ~WS_THICKFRAME;
	}
	else if (CreationFlags & WCF_CHILD)
	{
        flags &= ~WS_OVERLAPPEDWINDOW;
		flags |= WS_CHILD;
	}

    long menu;
	if( Fullscreen )
	{
		flags &= ~WS_CAPTION;
		flags &= ~WS_MAXIMIZEBOX;
		flags &= ~WS_THICKFRAME;
		flags &= ~WS_SYSMENU;
		flags &= ~WS_OVERLAPPEDWINDOW;
		menu = 0;
	}
	else
	{
		menu = CreationMenu;
	}

	// We want the client rectangle (3d display) to be a certain size, adjust the window size accordingly
	RECT rcWnd = { 0, 0, WndSizeX, WndSizeY };
	AdjustWindowRectEx(&rcWnd, flags, menu != 0, 0);

	// Bound the window size to fit the desktop
	HWND hWndDesktop = GetDesktopWindow();
	RECT rcDesktop;
	GetWindowRect(hWndDesktop, &rcDesktop);

	LONG wndWidth = rcWnd.right - rcWnd.left;
	LONG wndHeight = rcWnd.bottom - rcWnd.top;
	LONG maxWidth = rcDesktop.right - rcDesktop.left;
	LONG maxHeight = rcDesktop.bottom - rcDesktop.top;
	
	wndWidth = std::min(wndWidth, maxWidth);
	wndHeight = std::min(wndHeight, maxHeight);

	MSGhwnd = m_hWnd = CreateWindow("D3D Window", m_strWindowTitle.c_str(),
		                            flags,
									CW_USEDEFAULT, CW_USEDEFAULT, wndWidth, wndHeight, owner,
		                            LoadMenu(hInst, MAKEINTRESOURCE(menu)),
		                            hInst, 0L);

	if (!m_hWnd) return  false;

	UpdateWindow(m_hWnd);
	ShowWindow(m_hWnd, SW_SHOW);
	

	// Nuky - this isnt used for the game, and I can set WIN32_LEAN_AND_MEAN with it commented
	//// � supprimer au final
	//if (CreationFlags & WCF_ACCEPTFILES)
	//	DragAcceptFiles(m_hWnd, true);

	// Initialize the 3D environment for the app
	if (FAILED(hr = Initialize3DEnvironment()))
	{
		DisplayFrameworkError(hr, MSGERR_APPMUSTEXIT);
		Cleanup3DEnvironment();
		return false;
	}

	// Setup the app so it can support single-stepping
	m_dwBaseTime = dwARX_TIME_Get();

	// The app is ready to go
	m_bReady = true;

#ifdef BUILD_EDITOR
	if (ToolBar != NULL)
	{
		ToolBar->hWnd = CreateToolBar(m_hWnd, hInst);
	}
#endif

	this->m_pFramework->ShowFrame();
	GetZBufferMax();
	return true;
}

//*************************************************************************************
// Run()
// Message-processing loop. Idle time is used to render the scene.
//*************************************************************************************
int Win32Application::Run()
{
	BeforeRun();
	
	// Load keyboard accelerators
	HACCEL hAccel = NULL;

	// Now we're ready to recieve and process Windows messages.
	BOOL bGotMsg;
	MSG  msg;
	PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);

	while (WM_QUIT != msg.message)
	{
		// Use PeekMessage() if the app is active, so we can use idle time to
		// render the scene. Else, use GetMessage() to avoid eating CPU time.
		if (m_bActive)
			bGotMsg = PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE);
		else
			bGotMsg = GetMessage(&msg, NULL, 0U, 0U);

		if (bGotMsg)
		{
			// Translate and dispatch the message
			if (0 == TranslateAccelerator(m_hWnd, hAccel, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			// Render a frame during idle time (no messages are waiting)
			if (m_bActive && m_bReady)
			{
				if (FAILED(Render3DEnvironment()))
					DestroyWindow(m_hWnd);
			}
		}
	}

	return msg.wParam;
}

//*************************************************************************************
// OneTimeSceneInit()
//  Called Once during initial app startup
//*************************************************************************************
bool Win32Application::OneTimeSceneInit()
{
	return true;
}

//*************************************************************************************
// FrameMove()
//  Called once per frame.
//*************************************************************************************
bool Win32Application::FrameMove()
{
	
#ifdef BUILD_EDITOR
	// To disable for demo
	if (	!FINAL_COMMERCIAL_DEMO
		&& !FINAL_COMMERCIAL_GAME
		)
	{
		if (this->kbd.inkey[INKEY_F4])
		{
			this->kbd.inkey[INKEY_F4]=0;
			ARX_TIME_Pause();
			DialogBox( (HINSTANCE)GetWindowLongPtr( this->m_hWnd, GWLP_HINSTANCE ),
						   MAKEINTRESOURCE(IDD_LEVEL_SELECTOR), this->m_hWnd, ChangeLevelProc );

			if (CHANGE_LEVEL_PROC_RESULT!=-1)
			{
				char levelnum[256];
				char levelname[256];
				GetLevelNameByNum(CHANGE_LEVEL_PROC_RESULT,levelnum);
				sprintf(levelname,"LEVEL%s",levelnum);
				char leveltarget[256];
				strcpy(leveltarget,"no");

				ARX_CHECK_LONG( player.angle.b );
				ARX_CHANGELEVEL_Change( levelname, leveltarget, ARX_CLEAN_WARN_CAST_LONG( player.angle.b ), 0 );

			}

			ARX_TIME_UnPause();
		}

		// To Remove For Final Release !!!
		if (ALLOW_CHEATS || GAME_EDITOR)
		{
			if (this->kbd.inkey[INKEY_F2])
			{
				this->kbd.inkey[INKEY_F2]=0;
				ARX_TIME_Pause();
				Pause(true);
				DialogBox( (HINSTANCE)GetWindowLongPtr( this->m_hWnd, GWLP_HINSTANCE ),
					MAKEINTRESOURCE(IDD_OPTIONS), this->m_hWnd, OptionsProc );
				EERIE_LIGHT_ChangeLighting();
				Pause(false);
				ARX_TIME_UnPause();
			}
			else if (this->kbd.inkey[INKEY_F3])
			{
				this->kbd.inkey[INKEY_F3]=0;
				ARX_TIME_Pause();
				Pause(true);
				DialogBox( (HINSTANCE)GetWindowLongPtr( this->m_hWnd, GWLP_HINSTANCE ),
							MAKEINTRESOURCE(IDD_OPTIONS2), this->m_hWnd, OptionsProc_2 );
				Pause(false);
				ARX_TIME_UnPause();
			}
			else if (this->kbd.inkey[INKEY_O])
			{
				this->kbd.inkey[INKEY_O]=0;
			}
		}
	}
#endif // BUILD_EDITOR

	if (WILL_LAUNCH_CINE[0]) // Checks if a cinematic is waiting to be played...
	{
		LaunchWaitingCine();
	}

	return true;
}

