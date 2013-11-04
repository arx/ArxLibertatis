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

#include <sstream>

#include "core/Config.h"
#include "graphics/opengl/OpenGLRenderer.h"
#include "input/SDL2InputBackend.h"
#include "io/log/Logger.h"
#include "math/Rectangle.h"
#include "platform/CrashHandler.h"
#include "platform/Platform.h"

#define SDL_DISPLAY 0 // TODO don't hardcode this!

SDL2Window * SDL2Window::s_mainWindow = NULL;

SDL2Window::SDL2Window() : m_window(NULL), m_glcontext(NULL), m_input(NULL) { }

SDL2Window::~SDL2Window() {
	
	if(m_input) {
		delete m_input;
	}
	
	if(renderer) {
		onRendererShutdown();
		delete renderer, renderer = NULL;
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
	arx_assert(displayModes.empty());
	
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
	
	int modes = SDL_GetNumDisplayModes(SDL_DISPLAY);
	for(int i = 0; i < modes; i++) {
		SDL_DisplayMode mode;
		if(SDL_GetDisplayMode(SDL_DISPLAY, i, &mode) >= 0) {
			displayModes.push_back(DisplayMode(Vec2i(mode.w, mode.h), 32)); // TODO depth
		}
	}

	std::sort(displayModes.begin(), displayModes.end());
	
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

bool SDL2Window::initialize(const std::string & title, Vec2i size, bool fullscreen,
                           unsigned depth) {
	
	ARX_UNUSED(depth); // TODO
	
	arx_assert(!displayModes.empty());
	
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	
	// We need an accelerated OpenGL context or we'll likely fail later
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	// TODO EGL and core profile are not supported yet
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_EGL, 0);
	
	size_ = Vec2i_ZERO;
	depth_ = 0;
	
	for(int msaa = config.video.antialiasing ? 8 : 1; msaa >= 0; msaa--) {
		
		if(msaa > 1) {
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, msaa);
		} else if(msaa > 0) {
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
		} else {
			LogError << "Failed to initialize SDL Window: " << SDL_GetError();
			return false;
		}
		
		if(!m_window) {
			m_window = SDL_CreateWindow(
				title.c_str(),
				SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				size.x, size.y,
				getSDLFlagsForMode(size, fullscreen) | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
			);
			if(!m_window) {
				continue;
			}
		}
		
		m_glcontext = SDL_GL_CreateContext(m_window);
		if(m_glcontext) {
			break;
		}
		
	}
	
	SDL_GL_SetSwapInterval(config.video.vsync ? 1 : 0); // TODO support -1, support changing at runtime
	
	title_ = title;
	isFullscreen_ = fullscreen;
	
	SDL_ShowCursor(SDL_DISABLE);
	
	renderer = new OpenGLRenderer;
	renderer->Initialize();
	
	onCreate();
	updateSize();
	
	onShow(true);
	onFocus(true);
	
	onRendererInit();
	
	return true;
}

void SDL2Window::cleanupRenderer(bool wasOrIsFullscreen) {
	
#if ARX_PLATFORM == ARX_PLATFORM_LINUX || ARX_PLATFORM == ARX_PLATFORM_BSD
	// No re-initialization needed
	ARX_UNUSED(wasOrIsFullscreen);
#else
	
	#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	if(!wasOrIsFullscreen) {
		return;
	}
	#else
	// By default, always reinit to avoid issues on untested platforms
	ARX_UNUSED(wasOrIsFullscreen);
	#endif
	
	if(renderer && reinterpret_cast<OpenGLRenderer *>(renderer)->isInitialized()) {
		onRendererShutdown();
		reinterpret_cast<OpenGLRenderer *>(renderer)->shutdown();
	}
	
#endif
	
}

void SDL2Window::reinitializeRenderer() {
	
	#if ARX_PLATFORM == ARX_PLATFORM_LINUX || ARX_PLATFORM == ARX_PLATFORM_BSD
	// not re-initialization needed
	#else
	
	if(renderer && !reinterpret_cast<OpenGLRenderer *>(renderer)->isInitialized()) {
		reinterpret_cast<OpenGLRenderer *>(renderer)->reinit();
		updateSize();
		renderer->SetViewport(Rect(size_.x, size_.y));
		onRendererInit();
	}
	
	#endif
	
}

bool SDL2Window::setMode(DisplayMode mode, bool makeFullscreen) {
	
	bool wasFullscreen = isFullscreen_;
	
	cleanupRenderer(wasFullscreen || makeFullscreen);
	
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
				return false;
			}
			if(SDL_SetWindowDisplayMode(m_window, &sdlmode) < 0) {
				return false;
			}
		}
	}
	
	Uint32 flags = getSDLFlagsForMode(mode.resolution, makeFullscreen);
	if(SDL_SetWindowFullscreen(m_window, flags) < 0) {
		return false;
	}
	
	if(!makeFullscreen) {
		SDL_SetWindowSize(m_window, mode.resolution.x, mode.resolution.y);
	}
	
	if(wasFullscreen != makeFullscreen) {
		isFullscreen_ = makeFullscreen;
		onToggleFullscreen();
	}
	
	if(makeFullscreen) {
		// SDL regrettably sends resize events when a fullscreen window is minimized.
		// Because of that we ignore all size change events when fullscreen.
		// Instead, handle the size change here.
		updateSize();
	}
	
	tick();
	
	return true;
}

void SDL2Window::updateSize() {
	
	Vec2i oldSize = size_;
	
	int w, h;
	SDL_GetWindowSize(m_window, &w, &h);
	size_ = Vec2i(w, h);
	depth_ = 32; // TODO?
	
	if(size_ != oldSize) {
		if(renderer) {
			reinitializeRenderer();
			renderer->SetViewport(Rect(size_.x, size_.y));
		}
		onResize(size_.x, size_.y);
	}
}

void SDL2Window::setFullscreenMode(Vec2i resolution, unsigned _depth) {
	
	if(isFullscreen_ && size_ == resolution && depth_ == _depth) {
		return;
	}
	
	setMode(DisplayMode(resolution, depth_), true);
}

void SDL2Window::setWindowSize(Vec2i size) {
	
	if(!isFullscreen_ && size == size_) {
		return;
	}
	
	setMode(DisplayMode(size, 0), false);
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
						if(newSize != size_ && !isFullscreen_) {
							cleanupRenderer(false);
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
	
	reinitializeRenderer();
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
