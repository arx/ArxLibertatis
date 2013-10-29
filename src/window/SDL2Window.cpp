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

#include <boost/foreach.hpp>

#include "core/Config.h"
#include "graphics/opengl/OpenGLRenderer.h"
#include "input/SDL2InputBackend.h"
#include "io/log/Logger.h"
#include "math/Rectangle.h"
#include "platform/CrashHandler.h"
#include "platform/Platform.h"

#define SDL_DISPLAY 0 // TODO don't hardcode this!

SDL2Window * SDL2Window::mainWindow = NULL;

SDL2Window::SDL2Window() : window(NULL), context(0) { }

SDL2Window::~SDL2Window() {
	
	if(renderer) {
		onRendererShutdown();
		delete renderer, renderer = NULL;
	}
	
	arx_assert_msg(m_handlers.empty(), "Window is still being used!");
	
	if(context) {
		SDL_GL_DeleteContext(context);
	}
	
	if(window) {
		SDL_DestroyWindow(window);
	}
	
	if(mainWindow) {
		SDL_Quit(), mainWindow = NULL;
	}
	
}

bool SDL2Window::initializeFramework() {
	
	arx_assert_msg(mainWindow == NULL, "SDL only supports one window"); // TODO it supports multiple windows now!
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
	
	mainWindow = this;
	
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
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	
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
		
		if(!window) {
			window = SDL_CreateWindow(
				title.c_str(),
				SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				size.x, size.y,
				getSDLFlagsForMode(size, fullscreen) | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
			);
			if(!window) {
				continue;
			}
		}
		
		context = SDL_GL_CreateContext(window);
		if(context) {
			break;
		}
		
	}
	
	SDL_GL_SetSwapInterval(config.video.vsync ? 1 : 0); // TODO support -1
	
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
			if(!SDL_GetClosestDisplayMode(SDL_DISPLAY, &requested, &sdlmode)) {
				return false;
			}
			if(SDL_SetWindowDisplayMode(window, &sdlmode) < 0) {
				return false;
			}
		}
	} else {
		SDL_SetWindowSize(window, mode.resolution.x, mode.resolution.y);
	}
	
	if(SDL_SetWindowFullscreen(window, getSDLFlagsForMode(mode.resolution, makeFullscreen)) < 0) {
		return false;
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
	SDL_GetWindowSize(window, &w, &h);
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
	
	if(!isFullscreen_ && size == getSize()) {
		return;
	}
	
	setMode(DisplayMode(size, 0), false);
}

int SDLCALL SDL2Window::eventFilter(void * userdata, SDL_Event * event) {
	
	ARX_UNUSED(userdata);
	
	// TODO support multiple windows!
	if(mainWindow && event->type == SDL_QUIT) {
		return (mainWindow->onClose()) ? 1 : 0;
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
		
		BOOST_FOREACH(EventHandler * handler, m_handlers) {
			handler->onEvent(event);
		}
		
	}
	
	reinitializeRenderer();
}

Vec2i SDL2Window::getCursorPosition() const {
	int cursorPosX, cursorPosY;
	SDL_GetMouseState(&cursorPosX, &cursorPosY);
	return Vec2i(cursorPosX, cursorPosY);
}

void SDL2Window::showFrame() {
	SDL_GL_SwapWindow(window);
}

void SDL2Window::hide() {
	SDL_HideWindow(window);
	onShow(false);
}

void SDL2Window::addEventHandler(EventHandler * handler) {
	m_handlers.push_back(handler);
}

void SDL2Window::removeEventHandler(SDL2Window::EventHandler * handler) {
	EventHandlers::iterator it = std::find(m_handlers.begin(), m_handlers.end(), handler);
	if(it != m_handlers.end()) {
		m_handlers.erase(it);
	}
}
