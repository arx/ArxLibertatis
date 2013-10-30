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

#ifndef ARX_WINDOW_SDL2WINDOW_H
#define ARX_WINDOW_SDL2WINDOW_H

#include <vector>

#include <SDL.h>

#include "window/RenderWindow.h"

class SDL2Window : public RenderWindow {
	
public:
	
	class EventHandler {
	public:
		virtual void onEvent(const SDL_Event & event) = 0;
	};
	
	SDL2Window();
	virtual ~SDL2Window();
	
	bool initializeFramework();
	bool initialize(const std::string & title, Vec2i size, bool fullscreen,
	                unsigned depth = 0);
	void setFullscreenMode(Vec2i resolution, unsigned depth = 0);
	void setWindowSize(Vec2i size);
	void tick();
	
	void showFrame();
	
	void hide();
	
	void addEventHandler(EventHandler * handler);
	void removeEventHandler(EventHandler * handler);
	
	SDL_Window * getSDLWindow() { return m_window; }
	
private:
	
	bool setMode(DisplayMode mode, bool fullscreen);
	void updateSize();
	void cleanupRenderer(bool wasOrIsFullscreen);
	void reinitializeRenderer();
	
	static int SDLCALL eventFilter(void * userdata, SDL_Event * event);
	
	bool m_fullscreenDesktop;
	SDL_Window * m_window;
	SDL_GLContext m_glcontext;
	
	typedef std::vector<EventHandler *> EventHandlers;
	EventHandlers m_handlers;
	
	static SDL2Window * s_mainWindow;
	
};

#endif // ARX_WINDOW_SDL2WINDOW_H
