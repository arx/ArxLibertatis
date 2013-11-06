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

#include "window/SDL2Window.h"

#include <algorithm>
#include <sstream>

#include "graphics/opengl/OpenGLRenderer.h"
#include "input/SDL2InputBackend.h"
#include "io/log/Logger.h"
#include "math/Rectangle.h"
#include "platform/CrashHandler.h"
#include "platform/Platform.h"

SDL2Window * SDL2Window::s_mainWindow = NULL;

SDL2Window::SDL2Window()
	: m_window(NULL)
	, m_glcontext(NULL)
	, m_input(NULL)
	{
	m_renderer = new OpenGLRenderer;
}

SDL2Window::~SDL2Window() {
	
	if(m_input) {
		delete m_input;
	}
	
	if(m_renderer) {
		delete m_renderer, m_renderer = NULL;
	}
	
	if(m_glcontext) {
		SDL_GL_DeleteContext(m_glcontext);
	}
	
	if(m_window) {
		SDL_DestroyWindow(m_window);
	}
	
	if(s_mainWindow) {
		SDL_Quit(), s_mainWindow = NULL;
	}
	
}

bool SDL2Window::initializeFramework() {
	
	arx_assert_msg(s_mainWindow == NULL, "SDL only supports one window"); // TODO it supports multiple windows now!
	arx_assert(m_displayModes.empty());
	
	const char * headerVersion = ARX_STR(SDL_MAJOR_VERSION) "." ARX_STR(SDL_MINOR_VERSION)
	                             "." ARX_STR(SDL_PATCHLEVEL);
	CrashHandler::setVariable("SDL version (headers)", headerVersion);
	
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		LogError << "Failed to initialize SDL: " << SDL_GetError();
		return false;
	}
	
	SDL_version ver;
	SDL_GetVersion(&ver);
	std::ostringstream runtimeVersion;
	runtimeVersion << int(ver.major) << '.' << int(ver.minor) << '.' << int(ver.patch);
	CrashHandler::setVariable("SDL version", runtimeVersion.str());
	LogInfo << "Using SDL " << runtimeVersion.str();
	
	int ndisplays = SDL_GetNumVideoDisplays();
	for(int display = 0; display < ndisplays; display++) {
		int modes = SDL_GetNumDisplayModes(display);
		for(int i = 0; i < modes; i++) {
			SDL_DisplayMode mode;
			if(SDL_GetDisplayMode(display, i, &mode) >= 0) {
				m_displayModes.push_back(Vec2i(mode.w, mode.h));
			}
		}
	}
	
	std::sort(m_displayModes.begin(), m_displayModes.end());
	m_displayModes.erase(std::unique(m_displayModes.begin(), m_displayModes.end()),
	                     m_displayModes.end());
	
	s_mainWindow = this;
	
	SDL_SetEventFilter(eventFilter, NULL);
	
	SDL_EventState(SDL_WINDOWEVENT, SDL_ENABLE);
	SDL_EventState(SDL_QUIT,        SDL_ENABLE);
	SDL_EventState(SDL_SYSWMEVENT,  SDL_IGNORE);
	SDL_EventState(SDL_USEREVENT,   SDL_IGNORE);
	
	return true;
}

static Uint32 getSDLFlagsForMode(const Vec2i & size, bool fullscreen) {
	
	Uint32 flags = 0;
	
	if(fullscreen) {
		if(size == Vec2i_ZERO) {
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		} else {
			flags |= SDL_WINDOW_FULLSCREEN;
		}
	}
	
	return flags;
}

bool SDL2Window::initialize() {
	
	arx_assert(!m_displayModes.empty());
	
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	
	// We need an accelerated OpenGL context or we'll likely fail later
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	// TODO EGL and core profile are not supported yet
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_EGL, 0);
	
	int x = SDL_WINDOWPOS_UNDEFINED, y = SDL_WINDOWPOS_UNDEFINED;
	Uint32 windowFlags = getSDLFlagsForMode(m_size, m_fullscreen);
	windowFlags |= SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;
	
	for(int msaa = m_maxMSAALevel; msaa > 0; msaa--) {
		bool lastTry = (msaa == 1);
		
		// Cleanup context and window from previous tries
		if(m_glcontext) {
			SDL_GL_DeleteContext(m_glcontext);
			m_glcontext = NULL;
		}
		if(m_window) {
			SDL_DestroyWindow(m_window);
			m_window = NULL;
		}
		
		SDL_ClearError();
		
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, msaa > 1 ? 1 : 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, msaa > 1 ? msaa : 0);
		
		m_window = SDL_CreateWindow(m_title.c_str(), x, y, m_size.x, m_size.y, windowFlags);
		if(!m_window) {
			if(lastTry) {
				LogError << "Could not create window: " << SDL_GetError();
				return false;
			}
			continue;
		}
		
		m_glcontext = SDL_GL_CreateContext(m_window);
		if(!m_glcontext) {
			if(lastTry) {
				LogError << "Could not create GL context: " << SDL_GetError();
				return false;
			}
			continue;
		}
		
		// Verify that the MSAA setting matches what was requested
		if(!lastTry) {
			int msaaEnabled, msaaValue;
			SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &msaaEnabled);
			SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &msaaValue);
			if(!msaaEnabled || msaaValue < msaa) {
				continue;
			}
		}
		
		// Verify that we actually got an accelerated context
		(void)glGetError(); // clear error flags
		GLint texunits = 0;
		glGetIntegerv(GL_MAX_TEXTURE_UNITS, &texunits);
		if(glGetError() != GL_NO_ERROR || texunits < GLint(m_minTextureUnits)) {
			if(lastTry) {
				LogError << "Not enough GL texture units available: have " << texunits
				         << ", need at least " << m_minTextureUnits;
				return false;
			}
			continue;
		}
		
		// All good
		int red = 0, green = 0, blue = 0, alpha = 0, depth = 0, doublebuffer = 0;
		SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &red);
		SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &green);
		SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &blue);
		SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &alpha);
		SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &depth);
		SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &doublebuffer);
		LogInfo << "Window: r:" << red << " g:" << green << " b:" << blue << " a:" << alpha
		        << " depth:" << depth << " aa:" << msaa << "x doublebuffer:" << doublebuffer;
		break;
	}
	
	setVSync(m_vsync);
	
	SDL_ShowWindow(m_window);
	SDL_ShowCursor(SDL_DISABLE);
	
	m_renderer->initialize();
	
	onCreate();
	onToggleFullscreen(m_fullscreen);
	updateSize(true);
	
	onShow(true);
	onFocus(true);
	
	return true;
}

void SDL2Window::setTitle(const std::string & title) {
	if(m_window) {
		SDL_SetWindowTitle(m_window, title.c_str());
	}
	m_title = title;
}

bool SDL2Window::setVSync(int vsync) {
	if(m_window && SDL_GL_SetSwapInterval(vsync) != 0) {
		if(vsync != 0 && vsync != 1) {
			return setVSync(1);
		}
		return false;
	}
	m_vsync = vsync;
	return true;
}

void SDL2Window::changeMode(DisplayMode mode, bool makeFullscreen) {
	
	if(!m_window) {
		m_size = mode.resolution;
		m_fullscreen = makeFullscreen;
		return;
	}
	
	if(m_fullscreen == makeFullscreen && m_size == mode.resolution) {
		return;
	}
	
	bool wasFullscreen = m_fullscreen;
	
	m_renderer->beforeResize(wasFullscreen || makeFullscreen);
	
	if(makeFullscreen) {
		if(mode.resolution != Vec2i_ZERO) {
			SDL_DisplayMode sdlmode;
			SDL_DisplayMode requested;
			requested.driverdata = NULL;
			requested.format = 0;
			requested.refresh_rate = 0;
			requested.w = mode.resolution.x;
			requested.h = mode.resolution.y;
			int display = SDL_GetWindowDisplayIndex(m_window);
			if(!SDL_GetClosestDisplayMode(display, &requested, &sdlmode)) {
				if(SDL_GetDesktopDisplayMode(display, &sdlmode)) {
					return;
				}
			}
			if(SDL_SetWindowDisplayMode(m_window, &sdlmode) < 0) {
				return;
			}
		}
	}
	
	Uint32 flags = getSDLFlagsForMode(mode.resolution, makeFullscreen);
	if(SDL_SetWindowFullscreen(m_window, flags) < 0) {
		return;
	}
	
	if(!makeFullscreen) {
		SDL_SetWindowSize(m_window, mode.resolution.x, mode.resolution.y);
	}
	
	if(wasFullscreen != makeFullscreen) {
		onToggleFullscreen(makeFullscreen);
	}
	
	if(makeFullscreen) {
		// SDL regrettably sends resize events when a fullscreen window is minimized.
		// Because of that we ignore all size change events when fullscreen.
		// Instead, handle the size change here.
		updateSize();
	}
	
	tick();
}

void SDL2Window::updateSize(bool force) {
	
	Vec2i oldSize = m_size;
	
	int w, h;
	SDL_GetWindowSize(m_window, &w, &h);
	m_size = Vec2i(w, h);
	
	if(force || m_size != oldSize) {
		m_renderer->afterResize();
		m_renderer->SetViewport(Rect(m_size.x, m_size.y));
		onResize(m_size);
	}
}

void SDL2Window::setFullscreenMode(const DisplayMode & mode) {
	changeMode(mode, true);
}

void SDL2Window::setWindowSize(const Vec2i & size) {
	changeMode(size, false);
}

int SDLCALL SDL2Window::eventFilter(void * userdata, SDL_Event * event) {
	
	ARX_UNUSED(userdata);
	
	// TODO support multiple windows!
	if(s_mainWindow && event->type == SDL_QUIT) {
		return (s_mainWindow->onClose()) ? 1 : 0;
	}
	
	return 1;
}

void SDL2Window::tick() {
	
	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		
		switch(event.type) {
			
			case SDL_WINDOWEVENT: {
				switch(event.window.event) {
					
					case SDL_WINDOWEVENT_SHOWN:        onShow(true);   break;
					case SDL_WINDOWEVENT_HIDDEN:       onShow(false);  break;
					case SDL_WINDOWEVENT_EXPOSED:      onPaint();      break;
					case SDL_WINDOWEVENT_MINIMIZED:    onMinimize();   break;
					case SDL_WINDOWEVENT_MAXIMIZED:    onMaximize();   break;
					case SDL_WINDOWEVENT_RESTORED:     onRestore();    break;
					case SDL_WINDOWEVENT_FOCUS_GAINED: onFocus(true);  break;
					case SDL_WINDOWEVENT_FOCUS_LOST:   onFocus(false); break;
					
					case SDL_WINDOWEVENT_MOVED: {
						onMove(event.window.data1, event.window.data2);
						break;
					}
					
					case SDL_WINDOWEVENT_SIZE_CHANGED: {
						Vec2i newSize(event.window.data1, event.window.data2);
						if(newSize != m_size && !m_fullscreen) {
							m_renderer->beforeResize(false);
							updateSize();
						} else {
							// SDL regrettably sends resize events when a fullscreen window
							// is minimized - we'll have none of that!
						}
						break;
					}
					
					case SDL_WINDOWEVENT_CLOSE: {
						// The user has requested to close a single window
						// TODO we only support one main window for now
						break;
					}
					
				}
				break;
			}
			
			#if ARX_PLATFORM == ARX_PLATFORM_WIN32
			case SDL_KEYDOWN: {
				// SDL2 is still eating our ALT+F4 under windows...
				// See bug report here: https://bugzilla.libsdl.org/show_bug.cgi?id=1555
				if(event.key.keysym.sym == SDLK_F4
				   && (event.key.keysym.mod & KMOD_ALT) != KMOD_NONE) {
					SDL_Event quitevent;
					quitevent.type = SDL_QUIT;
					SDL_PushEvent(&quitevent);
				}
				break;
			}
			#endif
			
			case SDL_QUIT: {
				// The user has requested to close the whole program
				// TODO onDestroy() fits SDL_WINDOWEVENT_CLOSE better, but SDL captures Ctrl+C
				// evenst and *only* sends the SDL_QUIT event for them while normal close
				// generates *both* SDL_WINDOWEVENT_CLOSE and SDL_QUIT
				onDestroy();
				return; // abort event loop!
			}
			
		}
		
		if(m_input) {
			m_input->onEvent(event);
		}
		
	}
	
	if(!m_renderer->isInitialized()) {
		updateSize();
		m_renderer->afterResize();
		m_renderer->SetViewport(Rect(m_size.x, m_size.y));
	}
}

void SDL2Window::showFrame() {
	SDL_GL_SwapWindow(m_window);
}

void SDL2Window::hide() {
	SDL_HideWindow(m_window);
	onShow(false);
}

InputBackend * SDL2Window::getInputBackend() {
	if(!m_input) {
		m_input = new SDL2InputBackend(this);
	}
	return m_input;
}
