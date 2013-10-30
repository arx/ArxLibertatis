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

#include "window/SDL1Window.h"

#include <sstream>

#include "core/Config.h"
#include "graphics/opengl/OpenGLRenderer.h"
#include "input/SDL1InputBackend.h"
#include "io/log/Logger.h"
#include "math/Rectangle.h"
#include "platform/CrashHandler.h"
#include "platform/Platform.h"

SDL1Window * SDL1Window::s_mainWindow = NULL;

SDL1Window::SDL1Window()
	: m_input(NULL)
	{ }

SDL1Window::~SDL1Window() {
	
	if(m_input) {
		delete m_input;
	}
	
	if(renderer) {
		onRendererShutdown();
		delete renderer, renderer = NULL;
	}
	
	if(s_mainWindow) {
		SDL_Quit(), s_mainWindow = NULL;
	}
	
}

bool SDL1Window::initializeFramework() {
	
	arx_assert_msg(s_mainWindow == NULL, "SDL only supports one window");
	arx_assert(displayModes.empty());
	
	const char * headerVersion = ARX_STR(SDL_MAJOR_VERSION) "." ARX_STR(SDL_MINOR_VERSION)
	                             "." ARX_STR(SDL_PATCHLEVEL);
	CrashHandler::setVariable("SDL version (headers)", headerVersion);
	
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) < 0) {
		LogError << "Failed to initialize SDL: " << SDL_GetError();
		return false;
	}
	
	const SDL_version * ver = SDL_Linked_Version();
	std::ostringstream runtimeVersion;
	runtimeVersion << int(ver->major) << '.' << int(ver->minor) << '.' << int(ver->patch);
	CrashHandler::setVariable("SDL version", runtimeVersion.str());
	LogInfo << "Using SDL " << runtimeVersion.str();
	
	const SDL_VideoInfo * vid = SDL_GetVideoInfo();
	
	m_desktopMode.resolution.x = vid->current_w;
	m_desktopMode.resolution.y = vid->current_h;
	m_desktopMode.depth = vid->vfmt->BitsPerPixel;
	
	u32 flags = SDL_FULLSCREEN | SDL_ANYFORMAT | SDL_OPENGL | SDL_HWSURFACE;
	SDL_Rect ** modes = SDL_ListModes(NULL, flags);
	if(modes == (SDL_Rect **)(-1)) {
		
		// Any mode is supported, add some standard modes.
		
#define ADD_MODE(x, y) \
		if(m_desktopMode.resolution != Vec2i(x, y)) { \
			displayModes.push_back(DisplayMode(Vec2i(x, y), m_desktopMode.depth)); \
		}
		
		// 4:3
		ADD_MODE(640, 480) // VGA
		ADD_MODE(800, 600) // SVGA
		ADD_MODE(1024, 768) // XGA
		ADD_MODE(1280, 960) // SXGA-
		ADD_MODE(1600, 1200) // UXGA
		
		// 5:4
		ADD_MODE(1280, 1024) // SXGA
		
		// 16:9
		ADD_MODE(1280, 720) // 720p
		ADD_MODE(1600, 900) // 900p
		ADD_MODE(1920, 1080) // 1080p
		ADD_MODE(2048, 1152) // 2K
		ADD_MODE(4096, 2304) // 4K
		
		// 16:10
		ADD_MODE(1024, 640) // laptops
		ADD_MODE(1280, 800) // WXGA
		ADD_MODE(1440, 900) // WXGA+
		ADD_MODE(1920, 1200) // WUXGA
		
#undef ADD_MODE
		
		displayModes.push_back(m_desktopMode);
		
	} else if(modes) {
		for(; *modes; modes++) {
			DisplayMode mode(Vec2i((*modes)->w, (*modes)->h), m_desktopMode.depth);
			displayModes.push_back(mode);
		}
	} else {
		return false;
	}
	
	std::sort(displayModes.begin(), displayModes.end());
	
	s_mainWindow = this;
	
	SDL_SetEventFilter(eventFilter);
	
	SDL_EventState(SDL_ACTIVEEVENT, SDL_ENABLE);
	SDL_EventState(SDL_QUIT, SDL_ENABLE);
	SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
	SDL_EventState(SDL_VIDEORESIZE, SDL_ENABLE);
	SDL_EventState(SDL_VIDEOEXPOSE, SDL_ENABLE);
	SDL_EventState(SDL_USEREVENT, SDL_IGNORE);
	
	return true;
}

bool SDL1Window::initialize(const std::string & title, Vec2i size, bool fullscreen,
                           unsigned depth) {
	
	arx_assert(!displayModes.empty());
	
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, config.video.vsync ? 1 : 0);
	
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
		
		if(setMode(DisplayMode(size, fullscreen ? depth : 0), fullscreen)) {
			break;
		}
	}
	
	isFullscreen_ = fullscreen;
	
	SDL_WM_SetCaption(title.c_str(), title.c_str());
	title_ = title;
	
	SDL_ShowCursor(SDL_DISABLE);
	
	onCreate();
	
	renderer = new OpenGLRenderer;
	renderer->Initialize();
	
	updateSize(false);
	
	onShow(true);
	onFocus(true);
	
	onRendererInit();
	
	return true;
}

bool SDL1Window::setMode(DisplayMode mode, bool fullscreen) {
	
	if(fullscreen && mode.resolution == Vec2i_ZERO) {
		mode = m_desktopMode;
	} else if(mode.depth == 0) {
		mode.depth = m_desktopMode.depth;
	}
	
	bool needsReinit;
	
#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	needsReinit = isFullscreen_ || fullscreen;
#elif ARX_PLATFORM == ARX_PLATFORM_LINUX || ARX_PLATFORM == ARX_PLATFORM_BSD
	needsReinit = false;
#else
	needsReinit = true; // By default, always reinit to avoid issues on untested platforms
#endif
	
	if(needsReinit) {
		if(renderer && reinterpret_cast<OpenGLRenderer *>(renderer)->isInitialized()) {
			onRendererShutdown();
			reinterpret_cast<OpenGLRenderer *>(renderer)->shutdown();
		}
	}
	
	Uint32 flags = SDL_ANYFORMAT | SDL_OPENGL | SDL_HWSURFACE;
	flags |= (fullscreen) ? SDL_FULLSCREEN : SDL_RESIZABLE;
	SDL_Surface * win = SDL_SetVideoMode(mode.resolution.x, mode.resolution.y,
	                                     mode.depth, flags);
	
	return (win != NULL);
}

void SDL1Window::updateSize(bool reinit) {
	
	const SDL_VideoInfo * vid = SDL_GetVideoInfo();
	
	DisplayMode oldMode(size_, depth_);
	
	size_ = Vec2i(vid->current_w, vid->current_h);
	depth_ = vid->vfmt->BitsPerPixel;
	
	// Finally, set the viewport for the newly created device
	arx_assert(renderer != NULL);
	
#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	// use reinit as-is
#elif ARX_PLATFORM == ARX_PLATFORM_LINUX || ARX_PLATFORM == ARX_PLATFORM_BSD
	reinit = false; // Never needed under Linux & BSD
#else
	reinit = true; // By default, always reinit to avoid issues on untested platforms
#endif
	
	if(reinit && !reinterpret_cast<OpenGLRenderer *>(renderer)->isInitialized()) {
		reinterpret_cast<OpenGLRenderer *>(renderer)->reinit();
		renderer->SetViewport(Rect(size_.x, size_.y));
		onRendererInit();
	} else {
		renderer->SetViewport(Rect(size_.x, size_.y));
	}
	
	if(size_ != oldMode.resolution) {
		onResize(size_.x, size_.y);
	}
}

void SDL1Window::setFullscreenMode(Vec2i resolution, unsigned _depth) {
	
	if(isFullscreen_ && size_ == resolution && depth_ == _depth) {
		return;
	}
	
	if(!setMode(DisplayMode(resolution, depth_), true)) {
		return;
	}
	
	if(!isFullscreen_) {
		isFullscreen_ = true;
		updateSize(true);
		onToggleFullscreen();
	} else {
		updateSize(true);
	}
	
}

void SDL1Window::setWindowSize(Vec2i size) {
	
	if(!isFullscreen_ && size == getSize()) {
		return;
	}
	
	if(!setMode(DisplayMode(size, 0), false)) {
		return;
	}
	
	if(isFullscreen_) {
		isFullscreen_ = false;
		updateSize(true);
		onToggleFullscreen();
	}
}

int SDLCALL SDL1Window::eventFilter(const SDL_Event * event) {
	
	if(s_mainWindow && event->type == SDL_QUIT) {
		return (s_mainWindow->onClose()) ? 1 : 0;
	}
	
	return 1;
}

void SDL1Window::tick() {
	
	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		
		switch(event.type) {
			
			case SDL_ACTIVEEVENT: {
				if(event.active.state & SDL_APPINPUTFOCUS) {
					// ignored
				}
				if(event.active.state & SDL_APPACTIVE) {
					if(event.active.gain) {
						onRestore();
					} else {
						onMinimize();
					}
				}
				break;
			}
			
			case SDL_KEYDOWN: {
				
				// For some reason, release notes from SDL 1.2.12 says a SDL_QUIT message
				// should be sent when Command+Q is pressed on Mac OS or ALT-F4 on other platforms
				// but it doesn't look like it's working as expected...
				#if ARX_PLATFORM == ARX_PLATFORM_MACOSX
				if(event.key.keysym.sym == SDLK_q
					&& (event.key.keysym.mod & KMOD_META) != KMOD_NONE) {
				#else
				if(event.key.keysym.sym == SDLK_F4
					&& (event.key.keysym.mod & KMOD_ALT) != KMOD_NONE) {
				#endif
					onDestroy();
					break;
				}
				
				#if ARX_PLATFORM != ARX_PLATFORM_WIN32
				// The SDL X11 backend always grabs all keys when in fullscreen mode,
				// ufortunately breaking window manager shortcuts.
				// At least provide users with a way to switch to other windows.
				if(event.key.keysym.sym == SDLK_TAB
				   && (event.key.keysym.mod & KMOD_ALT) != KMOD_NONE) {
					SDL_WM_IconifyWindow();
				}
				#endif
				
				break;
			}
			
			case SDL_QUIT: {
				onDestroy();
				break;
			}
			
			case SDL_VIDEORESIZE: {
				Vec2i newSize(event.resize.w, event.resize.h);
				if(newSize != size_ && !isFullscreen_) {
					setMode(DisplayMode(newSize, depth_), false);
					updateSize(false);
				}
				break;
			}
			
			case SDL_VIDEOEXPOSE: {
				onPaint();
				break;
			}
			
		}
		
		if(m_input) {
			m_input->onEvent(event);
		}
		
	}
	
}

void SDL1Window::showFrame() {
	SDL_GL_SwapBuffers();
}

void SDL1Window::hide() {
	SDL_WM_IconifyWindow();
	onShow(false);
}

InputBackend * SDL1Window::getInputBackend() {
	if(!m_input) {
		m_input = new SDL1InputBackend();
	}
	return m_input;
}
