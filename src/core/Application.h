/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */
/* Based on:
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
// Code: Cyril Meynier
//   Sébastien Scieux (Zbuffer)
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved

#ifndef  ARX_CORE_APPLICATION_H
#define  ARX_CORE_APPLICATION_H

#include <string>

#include "graphics/Color.h"
#include "platform/Flags.h"

class RenderWindow;

enum HideFlag {
	HIDE_BACKGROUND = (1<<0),
	HIDE_NPC        = (1<<1),
	HIDE_FIXINTER   = (1<<2),
	HIDE_ITEMS      = (1<<3),
	HIDE_PARTICLES  = (1<<4),
	HIDE_INTERFACE  = (1<<5),
	HIDE_NODES      = (1<<6),
	HIDE_CAMERAS    = (1<<7)
};
DECLARE_FLAGS(HideFlag, HideFlags);
DECLARE_FLAGS_OPERATORS(HideFlags);

enum ViewModeFlag {
	VIEWMODE_WIRE           = (1<<0),
	VIEWMODE_NORMALS        = (1<<1),
	VIEWMODE_FLAT           = (1<<2),
	VIEWMODE_NOLIGHTSOURCES = (1<<3),
	VIEWMODE_INFOTEXT       = (1<<4)
};
DECLARE_FLAGS(ViewModeFlag, ViewModeFlags);
DECLARE_FLAGS_OPERATORS(ViewModeFlags);

enum LightModeFlag {
	MODE_STATICLIGHT  = (1<<0),
	MODE_DEPTHCUEING  = (1<<1),
	MODE_DYNAMICLIGHT = (1<<2),
	MODE_NORMALS      = (1<<3),
	MODE_RAYLAUNCH    = (1<<4),
	MODE_SMOOTH       = (1<<5)
};
DECLARE_FLAGS(LightModeFlag, LightMode);
DECLARE_FLAGS_OPERATORS(LightMode);

struct PROJECT {
	
	PROJECT()
		:
		  compatibility(0),
		  improve(0), detectliving(0), improvespeed(0),
		  telekinesis(0), demo(0),
		  hide(0), TextureSize(0),
		  torch(Color3f::black) {
	}
	
	long compatibility;
	long improve;
	long detectliving;
	long improvespeed;
	long telekinesis;
	long demo;
	HideFlags hide;
	long TextureSize;
	Color3f torch;
};

extern PROJECT Project;
extern float FPS;
extern LightMode ModeLight;

extern ViewModeFlags ViewMode;

extern long EERIEMouseButton, EERIEMouseGrab;

class Application {

protected:
	
	RenderWindow * m_MainWindow;
	
	/* Virtual functions to be overriden for the 3D scene in the Application */
	virtual bool DeleteDeviceObjects() { return true; }
	virtual bool FrameMove() { return true; }
	virtual bool BeforeRun() { return true; }
	
public:
	
	virtual bool Initialize();
	
private:
	
	virtual bool InitConfig();
	virtual bool InitWindow() = 0;
	virtual bool InitInput() = 0;
	virtual bool InitSound() = 0;
	
public:
	
	RenderWindow * GetWindow() { return m_MainWindow; }
	
	bool m_bReady;
	
	// Class constructor
	Application();
	
	virtual ~Application();
	
	void EvictManagedTextures();
	
	/* Virtual functions which may be overridden for specific implementations */
	
	/**
	 * Writes text to the window
	 * @param x The x coordinate for the text
	 * @param y The y coordinate for the text
	 * @param str The string of text to be written
	*/
	virtual void OutputText( int, int, const std::string& ) {}
	
	virtual void Run() = 0;
	virtual void Pause(bool bPause);
	virtual bool Render() { return true; }
	virtual bool FinalCleanup() = 0;
	virtual void Cleanup3DEnvironment() = 0;
	
	virtual void setFullscreen(bool fullscreen) = 0;
	
};

extern Application * mainApp;

void CalcFPS(bool reset = false);
void SetZBias(int newZBias);

#endif // ARX_CORE_APPLICATION_H
