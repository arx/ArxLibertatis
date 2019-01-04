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

#ifndef ARX_CORE_ARXGAME_H
#define ARX_CORE_ARXGAME_H

#include "core/Application.h"
#include "core/SaveGame.h"

#include "graphics/Renderer.h"

#include "window/Window.h"

class ArxGame : public Application, public Window::Listener, public Renderer::Listener {
	
protected:
	
	virtual bool initialize();
	bool initConfig();
	bool initWindow();
	bool initInput();
	bool initSound();
	bool initGameData();
	bool initGame();
	bool addPaks();
	
	virtual void shutdown();
	void shutdownGame();
	
	void doFrame();
	void render();

	void manageKeyMouse();
	void manageEntityDescription();
	void manageEditorControls();
	void managePlayerControls();
	void updateAllInterface();
	
public:
	
	ArxGame();
	virtual ~ArxGame();
	
	virtual void run();
	
private:
	void updateTime();
	void updateInput();
	
	// Camera stuff
	void updateFirstPersonCamera();
	void speechControlledCinematic();
	void handlePlayerDeath();
	void updateActiveCamera();
	
	void updateLevel();
	void renderLevel();
	
	
	virtual void onWindowGotFocus(const Window & window);
	virtual void onWindowLostFocus(const Window & window);
	virtual void onResizeWindow(const Window & window);
	virtual void onDestroyWindow(const Window & window);
	virtual void onToggleFullscreen(const Window & window);
	virtual void onDroppedFile(const Window & window, const fs::path & path);
	
	bool m_wasResized;
	
	void onRendererInit(Renderer & renderer);
	void onRendererShutdown(Renderer & renderer);
	
	bool initWindow(RenderWindow * window);
	
	void setWindowSize(bool fullscreen);
	
	bool m_gameInitialized;
};

enum InfoPanels {
	InfoPanelNone,
	InfoPanelFramerate,
	InfoPanelFramerateGraph,
	InfoPanelDebug,
	InfoPanelGuiDebug,
	InfoPanelAudio,
	InfoPanelEnumSize
};

extern InfoPanels g_debugInfo;

extern TextureContainer * enviro;

extern SavegameHandle LOADQUEST_SLOT;

#endif // ARX_CORE_ARXGAME_H

