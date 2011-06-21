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

#ifndef WIN32APPLICATION_H
#define WIN32APPLICATION_H

#include "Application.h"

#include "windows.h"

class Win32Application : public Application {
	
protected:

	bool OneTimeSceneInit();
	bool DeleteDeviceObjects();
	bool Render();
	bool FrameMove();
	void ManageKeyMouse();
	bool ManageEditorControls();
	void ManagePlayerControls();
	void DrawAllInterface();
	void DrawAllInterfaceFinish();
	void GoFor2DFX();
	bool BeforeRun();
	
	HRESULT(*m_fnConfirmDevice)(DDCAPS *, D3DDEVICEDESC7 *);

	// Overridable power management (APM) functions
	LRESULT OnQuerySuspend(DWORD dwFlags);
	LRESULT OnResumeSuspend(DWORD dwData);

	// 3D Framework functions
	void DisplayFrameworkError(HRESULT, DWORD);
	HRESULT Initialize3DEnvironment();
	HRESULT Render3DEnvironment();


public:
	
	Win32Application();
	virtual bool Create();
	virtual int Run();
	bool InitDeviceObjects();
	bool FinalCleanup();
	virtual bool SwitchFullScreen();
	virtual void Cleanup3DEnvironment();
	virtual bool Change3DEnvironment();
	
	virtual bool UpdateGamma();

	/**
	 * Writes text to the window
	 * @param x The x coordinate for the text
	 * @param y The y coordinate for the text
	 * @param str The string of text to be written
	*/
	virtual void OutputText( int x, int y, const std::string& str );

	virtual LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // WIN32APPLICATION_H

