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

#include <windows.h>

#include "core/Application.h"
#include "core/Window.h"

class ArxGame : public Application, public Window::Listener {
	
protected:
	
	virtual bool Initialize();
	virtual bool InitWindow();
	virtual bool InitGraphics();
	virtual bool InitInput();
	virtual bool InitSound();
	bool InitGameData();
	bool AddPaks();
	
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
	
	// 3D Framework functions
	
	enum APPMSGTYPE {
		MSG_NONE,
		MSGERR_APPMUSTEXIT,
		MSGWARN_SWITCHEDTOSOFTWARE
	};
	
	void DisplayFrameworkError(HRESULT, APPMSGTYPE);
	
	HRESULT Initialize3DEnvironment();
	
	bool Render3DEnvironment();
	
public:
	
	ArxGame();
	bool Create();
	virtual void Run();
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
	virtual void OutputText(int x, int y, const std::string & str);
	
private:
	
	virtual void OnWindowGotFocus(const Window & window);
	virtual void OnWindowLostFocus(const Window & window);
	virtual void OnResizeWindow(const Window & window);
	virtual void OnPaintWindow(const Window & window);
	virtual void OnDestroyWindow(const Window & pWindow);
	
	bool m_RunLoop;
	
};

#endif // WIN32APPLICATION_H

