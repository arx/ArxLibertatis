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
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
// EERIEApp
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//
// Updates: (date) (person) (update)
//
// Code:	Cyril Meynier
//			S�bastien Scieux	(Zbuffer)
//			Didier Pedreno		(ScreenSaver Problem Fix)
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include "core/Application.h"

#include <cstdio>

#include "core/Config.h"
#include "core/Resource.h"
#include "core/GameTime.h"

#include "game/Player.h"

#include "gui/Menu.h"
#include "gui/Interface.h"
#include "gui/MenuWidgets.h"

#include "graphics/Frame.h"
#include "graphics/GraphicsEnum.h"
#include "graphics/GraphicsUtility.h"
#include "graphics/data/Mesh.h"

#include "io/FilePath.h"
#include "io/PakManager.h"
#include "io/Logger.h"

using std::max;

//-----------------------------------------------------------------------------
extern long USE_OLD_MOUSE_SYSTEM;
extern long FINAL_RELEASE;
extern bool bGLOBAL_DINPUT_GAME;

extern long FINAL_COMMERCIAL_DEMO;
extern long FINAL_COMMERCIAL_GAME;

//-----------------------------------------------------------------------------
long EERIEMouseButton = 0;
long LastEERIEMouseButton = 0;
long EERIEMouseGrab = 0;

Application* mainApp = 0;
EERIE_CAMERA		* Kam;
LPDIRECT3DDEVICE7	GDevice;
HWND				MSGhwnd = NULL;
float				FPS;
LightMode ModeLight = 0;
ViewModeFlags ViewMode = 0;

static RECT srRect;
static int iCurrZBias;


//*************************************************************************************
// Application()
// Constructor
//*************************************************************************************
Application::Application()
{
	long i;
	d_dlgframe = 0;
	m_pFramework   = NULL;
	m_hWnd         = NULL;
	m_pDD          = NULL;
	m_pD3D         = NULL;

	m_pddsRenderTarget     = NULL;
	m_pddsRenderTargetLeft = NULL;

	m_bActive         = false;
	m_bReady          = false;
	m_bSingleStep     = false;

	m_strWindowTitle  = "EERIE Application";
	m_bAppUseZBuffer  = false;
	m_bAppUseStereo   = false;
	m_bShowStats      = false;
	WndSizeX = 640;
	WndSizeY = 480;
	Fullscreen = 0;
	CreationFlags = 0;
	owner = 0L;
	CreationMenu = 0;
#ifdef BUILD_EDITOR
	ToolBar = NULL;
#endif
	strcpy(StatusText, "");
	memset(&Project, 0, sizeof(PROJECT));
	Project.TextureSize = 0;
	Project.ambient = 0;
	Project.vsync = true;		
	Project.interpolatemouse = 1;

	//Keyboard Init;
	for (i = 0; i < 255; i++)	kbd.inkey[i] = 0;

	kbd.nbkeydown = 0;
	kbd._CAPS = 0;

	zbuffer_max = 1.f;
	zbuffer_max_div = 1.f;

	EERIEWheel = 0;
}


extern long FINAL_COMMERCIAL_DEMO;
extern long FINAL_COMMERCIAL_GAME;


#ifdef BUILD_EDITOR
HWND Application::CreateToolBar(HWND hWndParent, HINSTANCE hInst) {
	
	HWND hWndToolbar;
	long flags;


	flags = WS_CHILD | WS_BORDER | WS_VISIBLE | TBSTYLE_WRAPABLE;

	if (this->ToolBar->Type == EERIE_TOOLBAR_TOP) flags |= CCS_TOP;
	else flags |= CCS_LEFT;

	hWndToolbar = CreateToolbarEx(hWndParent,
	                              flags,
	                              this->ToolBar->CreationToolBar, this->ToolBar->ToolBarNb, //nb buttons in bmp
	                              (HINSTANCE)hInst, this->ToolBar->Bitmap,
	                              (LPCTBBUTTON)this->ToolBar->Buttons, this->ToolBar->ToolBarNb,
	                              16, 15, 16, 15, sizeof(TBBUTTON));

	SetWindowPos(hWndToolbar, HWND_TOP, 0, 90, 2, 500, SWP_SHOWWINDOW);
	ShowWindow(hWndToolbar, SW_SHOW);
	UpdateWindow(hWndToolbar);


	return hWndToolbar;
}
#endif


void Application::EvictManagedTextures()
{
	if (this->m_pD3D)
	{
		this->m_pD3D->EvictManagedTextures();
	}
}

int Application::WinManageMess()
{
	BOOL bGotMsg = true;
	MSG  msg;

	while (bGotMsg)
	{
		if (m_bActive)
			bGotMsg = PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE);
		else
			bGotMsg = GetMessage(&msg, NULL, 0U, 0U);

		if (bGotMsg)
		{
			// Translate and dispatch the message
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return 1;
}




//*************************************************************************************
// Callback for WM_MOUSEMOUVE
//*************************************************************************************
void Application::EERIEMouseUpdate(short x, short y)
{
	if ((m_pFramework) &&
	        (m_pFramework->m_bIsFullscreen) &&
	        (bGLOBAL_DINPUT_GAME)) return;

	// Inactive App: ignore
	if (!this->m_bActive)	return;

	// Updates MouseX & MouseY offsets
	_EERIEMouseXdep += (x - EERIEMouseX);
	_EERIEMouseYdep += (y - EERIEMouseY);

	// Manages MouseGrab (Right-click maintained pressed)

	POINT	pos;
	bool	mod	=	false;

	pos.x = pos.y = 0;

	if (EERIEMouseGrab)
	{
		pos.x = (short)(this->m_pFramework->m_dwRenderWidth >> 1);
		pos.y = (short)(this->m_pFramework->m_dwRenderHeight >> 1);

		if ((x != pos.x) || (y != pos.y)) mod = true;
	}

	if (!((ARXmenu.currentmode == AMCM_NEWQUEST)
	        ||	(player.Interface & INTER_MAP && (Book_Mode != BOOKMODE_MINIMAP))))
		if (mod)
		{
			EERIEMouseX = (long)pos.x;
			EERIEMouseY = (long)pos.y;
			ClientToScreen(this->m_hWnd, &pos);
			SetCursorPos(pos.x, pos.y);
			return;
		}

	EERIEMouseX = x;
	EERIEMouseY = y;
}

//*************************************************************************************
// Pause()
// Called in to toggle the pause state of the app. This function
// brings the GDI surface to the front of the display, so drawing
// output like message boxes and menus may be displayed.
//*************************************************************************************
VOID Application::Pause(bool bPause)
{
	static DWORD dwAppPausedCount = 0L;

	dwAppPausedCount += (bPause ? +1 : -1);
	m_bReady          = (dwAppPausedCount ? false : true);

	// Handle the first pause request (of many, nestable pause requests)
	if (bPause && (1 == dwAppPausedCount))
	{
		// Get a surface for the GDI
		if (m_pFramework)
			m_pFramework->FlipToGDISurface(true);

		// Stop the scene from animating
		m_dwStopTime = dwARX_TIME_Get();
	}

	if (0 == dwAppPausedCount)
	{
		// Restart the scene
		m_dwBaseTime += dwARX_TIME_Get() - m_dwStopTime;
	}
}

//*************************************************************************************
//*************************************************************************************
VOID CalcFPS(bool reset)
{
	static float fFPS      = 0.0f;
	static float fLastTime = 0.0f;
	static DWORD dwFrames  = 0L;

	if (reset)
	{
		dwFrames = 0;
		fLastTime = 0.f;
		FPS = fFPS = 7.f * FPS;
	}
	else
	{
		float tmp;

		// Keep track of the time lapse and frame count
		float fTime = ARX_TIME_Get(false) * 0.001f;   // Get current time in seconds
		++dwFrames;

		tmp = fTime - fLastTime;

		// Update the frame rate once per second
		if (tmp > 1.f)
		{
			FPS = fFPS      = dwFrames / tmp ;
			fLastTime = fTime;
			dwFrames  = 0L;
		}
	}
}

//*************************************************************************************
//*************************************************************************************
void * Application::Lock()
{

	memset((void *)&ddsd, 0, sizeof(ddsd));	
	memset((void *)&ddsd2, 0, sizeof(ddsd2));

	this->ddsd2.dwSize = this->ddsd.dwSize = sizeof(DDSURFACEDESC2);
	this->m_pFramework->m_pddsBackBuffer->GetSurfaceDesc(&ddsd);
	this->m_pFramework->m_pddsZBuffer->GetSurfaceDesc(&ddsd2);

	this->zbuf = (void *)this->ddsd2.lpSurface ;
	this->logical = (void *)this->ddsd.lpSurface ;

	this->zbits = this->ddsd2.ddpfPixelFormat.dwZBufferBitDepth;	
	this->nbbits = this->ddsd.ddpfPixelFormat.dwRGBBitCount;

	dw_zXmodulo = ddsd2.lPitch;
	dw_zmask = ddsd2.ddpfPixelFormat.dwZBitMask;
	w_zdecal = 0;
	unsigned long t = dw_zmask;

	while (!(t & 1))
	{
		w_zdecal++;
		t >>= 1;
	}

	int t2 = 0;
	t = dw_zmask;

	while (t)
	{
		if (t & 0xFF) t2++;

		t >>= 8;
	}

	f_zmul = (float)((1 << (8 * t2)) - 1);	

	return (void *) - 1;
}

//*************************************************************************************
//*************************************************************************************
bool Application::Unlock()
{
	if (zbuf)
	{
		m_pFramework->m_pddsZBuffer->Unlock(&srRect);
		zbuf = NULL;
	}

	return true;
}

//******************************************************************************
// MESSAGE BOXES
//******************************************************************************
//*************************************************************************************
//*************************************************************************************
//TODO(lubosz): is this needed in the game? replace
bool OKBox(const std::string& text, const std::string& title)

{
	int i;
	mainApp->Pause(true);
	i = MessageBox(mainApp->m_hWnd, text.c_str(), title.c_str(), MB_ICONQUESTION | MB_OKCANCEL);
	mainApp->Pause(false);

	if (i == IDCANCEL) return false;

	return true;
}

void SetZBias(int _iZBias)
{
	if (_iZBias < 0)
	{
		_iZBias = 0;
		_iZBias = max(iCurrZBias, -_iZBias);
	}

	if (_iZBias == iCurrZBias) 
		return;

	iCurrZBias = _iZBias;

	GRenderer->SetDepthBias(iCurrZBias);
}
