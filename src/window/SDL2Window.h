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

#ifndef ARX_WINDOW_SDL2WINDOW_H
#define ARX_WINDOW_SDL2WINDOW_H

#include <SDL.h>

#include "platform/Platform.h"
#include "window/RenderWindow.h"

class SDL2InputBackend;

class SDL2Window arx_final : public RenderWindow {
	
public:
	
	SDL2Window();
	virtual ~SDL2Window();
	
	bool initializeFramework();
	void setTitle(const std::string & title);
	bool setVSync(int vsync);
	void setFullscreenMode(const DisplayMode & mode);
	void setWindowSize(const Vec2i & size);
	bool setGamma(float gamma = 1.f);
	bool initialize();
	void tick();
	
	void showFrame();
	
	void hide();
	
	void setMinimizeOnFocusLost(bool enabled);
	MinimizeSetting willMinimizeOnFocusLost();
	
	std::string getClipboardText();
	void setClipboardText(const std::string & text);
	
	void allowScreensaver(bool allowed);
	
	InputBackend * getInputBackend();
	
private:
	
	int createWindowAndGLContext(const char * profile);
	
	void changeMode(DisplayMode mode, bool fullscreen);
	void updateSize(bool force = false);
	
	void restoreGamma();
	
	static int SDLCALL eventFilter(void * userdata, SDL_Event * event);
	
	SDL_Window * m_window;
	SDL_GLContext m_glcontext;
	
	SDL2InputBackend * m_input;
	
	MinimizeSetting m_minimizeOnFocusLost;
	MinimizeSetting m_allowScreensaver;
	
	float m_gamma;
	bool m_gammaOverridden;
	u16 m_gammaRed[256];
	u16 m_gammaGreen[256];
	u16 m_gammaBlue[256];
	
	static SDL2Window * s_mainWindow;
	
	friend class SDL2InputBackend;
};

#endif // ARX_WINDOW_SDL2WINDOW_H
