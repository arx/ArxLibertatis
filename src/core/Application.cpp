/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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
//   Didier Pedreno  (ScreenSaver Problem Fix)
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved

#include "core/Application.h"

#include "core/GameTime.h"

#include "window/RenderWindow.h"

Application * mainApp = 0;
float FPS;


Application::Application() : m_MainWindow(NULL) {
	m_bReady = true;
	m_RunLoop = true;
}

Application::~Application() {
	arx_assert(!m_MainWindow);
}

void Application::shutdown() {
	delete m_MainWindow, m_MainWindow = NULL;
}

void Application::quit() {
	m_RunLoop = false;
}

void CalcFPS(bool reset) {
	
	static PlatformInstant fLastTime = 0;
	static u32 dwFrames  = 0;

	if(reset) {
		dwFrames = 0;
		fLastTime = 0;
		FPS = 7.f * FPS;
	} else {
		// Keep track of the time lapse and frame count
		PlatformInstant fTime = g_platformTime.frameStart();
		++dwFrames;

		float tmp = toS(fTime - fLastTime);

		// Update the frame rate once per second
		if(tmp > 1.f) {
			FPS = float(dwFrames) / tmp;
			fLastTime = fTime;
			dwFrames  = 0;
		}
	}
}
