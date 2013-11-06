/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_CORE_ARXGAME_H
#define ARX_CORE_ARXGAME_H

#include <string>

#include "core/Application.h"
#include "graphics/Renderer.h"
#include "window/Window.h"
#include "window/RenderWindow.h"

class Font;

class ArxGame : public Application, public Window::Listener, public Renderer::Listener {
	
protected:
	
	virtual bool initialize();
	virtual bool initWindow();
	virtual bool initInput();
	virtual bool initSound();
	bool initGameData();
	bool addPaks();
	
	void doFrame();
	void update();
	void render();	

	void manageKeyMouse();
	void manageEditorControls();
	void managePlayerControls();
	void updateAllInterface();
	void drawAllInterfac(); //TODO rename this
	void drawAllInterface();
	void drawAllInterfaceFinish();

	void update2DFX();
	void goFor2DFX();

	bool beforeRun();
		
public:
	
	ArxGame();
	virtual ~ArxGame();
	
	bool create();
	virtual void run();

	bool initDeviceObjects();
	bool finalCleanup();
	virtual void cleanup3DEnvironment();
	
	/*!
	 * Writes text to the window
	 * @param x The x coordinate for the text
	 * @param y The y coordinate for the text
	 * @param str The string of text to be written
	 */
	virtual void outputText(int x, int y, const std::string & str);
	virtual void outputTextGrid(float x, float y, const std::string &text, const Color &color);
	
private:
	void updateTime();
	void updateInput();

	// Camera stuff
	void updateFirstPersonCamera();
	void updateConversationCamera();
	void speechControlledCinematic();
	void handlePlayerDeath();
	void handleCameraController();
	void updateActiveCamera();

	bool isInMenu() const;
	bool isInCinematic() const;

	void renderMenu();
	void renderCinematic();

	void updateLevel();
	void renderLevel();

	
	virtual void onWindowGotFocus(const Window & window);
	virtual void onResizeWindow(const Window & window);
	virtual void onDestroyWindow(const Window & window);
	virtual void onToggleFullscreen(const Window & window);
	
	bool wasResized;
	
	void onRendererInit(Renderer &);
	void onRendererShutdown(Renderer &);
	
	bool initWindow(RenderWindow * window);
	
	void setWindowSize(bool fullscreen);
};

#endif // ARX_CORE_ARXGAME_H

