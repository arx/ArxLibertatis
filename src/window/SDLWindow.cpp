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

#include "window/SDLWindow.h"

#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	#undef WIN32_LEAN_AND_MEAN // SDL defines this... Bad SDL!
	#include <SDL_syswm.h>
#endif

#include "core/Config.h"
#include "graphics/opengl/OpenGLRenderer.h"
#include "input/SDLInputBackend.h"
#include "io/log/Logger.h"
#include "math/Rectangle.h"

SDLWindow * SDLWindow::mainWindow = NULL;

SDLWindow::SDLWindow() : input(NULL) { }

SDLWindow::~SDLWindow() {
	
	if(renderer) {
		onRendererShutdown();
		delete renderer, renderer = NULL;
	}
	
	if(mainWindow) {
		SDL_Quit(), mainWindow = NULL;
	}
	
}

bool SDLWindow::initFramework() {
	
	arx_assert_msg(mainWindow == NULL, "SDL only supports one window");
	arx_assert(displayModes.empty());
	
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		LogError << "Failed to initialize SDL: " << SDL_GetError();
		return false;
	}
	
	const SDL_VideoInfo * vid = SDL_GetVideoInfo();
	
	desktopMode.resolution.x = vid->current_w;
	desktopMode.resolution.y = vid->current_h;
	desktopMode.depth = vid->vfmt->BitsPerPixel;
	
	SDL_Rect ** modes = SDL_ListModes(NULL, SDL_FULLSCREEN | SDL_ANYFORMAT | SDL_OPENGL | SDL_HWSURFACE);
	if(modes == (SDL_Rect**)(-1)) {
		
		// Any mode is supported, add some standard modes.
		
#define ADD_MODE(x, y) \
		if(desktopMode.resolution != Vec2i(x, y)) { \
			displayModes.push_back(DisplayMode(Vec2i(x, y), desktopMode.depth)); \
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
		
		displayModes.push_back(desktopMode);
		
	} else if(modes) {
		for(; *modes; modes++) {
			displayModes.push_back(DisplayMode(Vec2i((*modes)->w, (*modes)->h), desktopMode.depth));
		}
	} else {
		return false;
	}
	
	std::sort(displayModes.begin(), displayModes.end());
	
	mainWindow = this;
	
	return true;
}

bool SDLWindow::init(const std::string & title, Vec2i size, bool fullscreen, unsigned _depth) {
	
	arx_assert(!displayModes.empty());
	
	SDL_SetEventFilter(eventFilter);
	
	SDL_EventState(SDL_ACTIVEEVENT, SDL_ENABLE);
	SDL_EventState(SDL_QUIT, SDL_ENABLE);
	SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
	SDL_EventState(SDL_VIDEORESIZE, SDL_ENABLE);
	SDL_EventState(SDL_VIDEOEXPOSE, SDL_ENABLE);
	SDL_EventState(SDL_USEREVENT, SDL_IGNORE);
	
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, config.video.vsync ? 1 : 0);
	
	m_Size = Vec2i::ZERO;
	depth = 0;
	
	for(int msaa = config.video.antialiasing ? 4 : 1; msaa >= 0; msaa--) {
		
		if(msaa > 1) {
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
		} else if(msaa > 0) {
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
		} else {
			LogError << "Failed to initialize SDL Window: " << SDL_GetError();
			return false;
		}
		
		if(setMode(DisplayMode(size, fullscreen ? _depth : 0), fullscreen)) {
			break;
		}
	}
	
	m_IsFullscreen = fullscreen;
	
	SDL_WM_SetCaption(title.c_str(), title.c_str());
	
	SDL_ShowCursor(SDL_DISABLE);
	
	const SDL_version * ver = SDL_Linked_Version();
	LogInfo << "Using SDL " << int(ver->major) << '.' << int(ver->minor) << '.' << int(ver->patch);
	
	OnCreate();
	
	renderer = new OpenGLRenderer;
	renderer->Initialize();
	
	updateSize(false);
	
	OnShow(true);
	OnFocus(true);
	
	onRendererInit();
	
	return true;
}

bool SDLWindow::setMode(DisplayMode mode, bool fullscreen) {
	
	if(fullscreen && mode.resolution == Vec2i::ZERO) {
		mode = desktopMode;
	} else if(mode.depth == 0) {
		mode.depth = desktopMode.depth;
	}
	
#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	if(m_IsFullscreen || fullscreen) {
		if(renderer && reinterpret_cast<OpenGLRenderer *>(renderer)->isInitialized()) {
			onRendererShutdown();
			reinterpret_cast<OpenGLRenderer *>(renderer)->shutdown();
		}
	}
#endif
	
	Uint32 flags = SDL_ANYFORMAT | SDL_OPENGL | SDL_HWSURFACE;
	flags |= (fullscreen) ? SDL_FULLSCREEN : SDL_RESIZABLE;
	SDL_Surface * win = SDL_SetVideoMode(mode.resolution.x, mode.resolution.y, mode.depth, flags);
	
	return (win != NULL);
}

void SDLWindow::updateSize(bool reinit) {
	
	const SDL_VideoInfo * vid = SDL_GetVideoInfo();
	
	DisplayMode oldMode(m_Size, depth);
	
	m_Size = Vec2i(vid->current_w, vid->current_h);
	depth = vid->vfmt->BitsPerPixel;
	
	// Finally, set the viewport for the newly created device
	arx_assert(renderer != NULL);
#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	if(reinit && !reinterpret_cast<OpenGLRenderer *>(renderer)->isInitialized()) {
		reinterpret_cast<OpenGLRenderer *>(renderer)->reinit();
		renderer->SetViewport(Rect(m_Size.x, m_Size.y));
		onRendererInit();
	} else {
		renderer->SetViewport(Rect(m_Size.x, m_Size.y));
	}
#else
	ARX_UNUSED(reinit);
	renderer->SetViewport(Rect(m_Size.x, m_Size.y));
#endif
	
	if(m_Size != oldMode.resolution) {
		OnResize(m_Size.x, m_Size.y);
	}
}

void * SDLWindow::GetHandle() {
	
#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	
	SDL_SysWMinfo wmi;
	SDL_VERSION(&wmi.version);
	
	if(!SDL_GetWMInfo(&wmi)) {
		return NULL;
	}
	
	return wmi.window;
	
#else
	
	// TODO X11 needs more than one pointer (display+window)
	return NULL;
	
#endif
	
}

void SDLWindow::setFullscreenMode(Vec2i resolution, unsigned _depth) {
	
	if(m_IsFullscreen && m_Size == resolution && depth == _depth) {
		return;
	}
	
	if(!setMode(DisplayMode(resolution, depth), true)) {
		return;
	}
	
	if(!m_IsFullscreen) {
		m_IsFullscreen = true;
		updateSize(true);
		OnToggleFullscreen();
	} else {
		updateSize(true);
	}
	
}

void SDLWindow::setWindowSize(Vec2i size) {
	
	if(!m_IsFullscreen && size == GetSize()) {
		return;
	}
	
	if(!setMode(DisplayMode(size, 0), false)) {
		return;
	}
	
	if(m_IsFullscreen) {
		m_IsFullscreen = false;
		updateSize(true);
		OnToggleFullscreen();
	}
}

int SDLCALL SDLWindow::eventFilter(const SDL_Event * event) {
	
	if(mainWindow && event->type == SDL_QUIT) {
		return (mainWindow->OnClose()) ? 1 : 0;
	}
	
	return 1;
}

void SDLWindow::Tick() {
	
	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		
		switch(event.type) {
			
			case SDL_ACTIVEEVENT: {
				if(event.active.state & SDL_APPINPUTFOCUS) {
					// ignored
				}
				if(event.active.state & SDL_APPACTIVE) {
					if(event.active.gain) {
						OnRestore();
					} else {
						OnMinimize();
					}
				}
				if(event.active.state & SDL_APPMOUSEFOCUS) {
					// ignored
				}
				break;
			}
			
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			case SDL_MOUSEMOTION:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			case SDL_JOYAXISMOTION:
			case SDL_JOYBALLMOTION:
			case SDL_JOYHATMOTION:
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP: {
				if(input) {
					input->onInputEvent(event);
				}
				break;
			}
			
			case SDL_QUIT: {
				OnDestroy();
				break;
			}
			
			case SDL_VIDEORESIZE: {
				Vec2i newSize(event.resize.w, event.resize.h);
				if(newSize != m_Size && !m_IsFullscreen) {
					setMode(DisplayMode(newSize, depth), false);
					updateSize(false);
				}
				break;
			}
			
			case SDL_VIDEOEXPOSE: {
				OnPaint();
				break;
			}
			
		}
		
	}
	
}

bool SDLWindow::showFrame() {
	
	SDL_GL_SwapBuffers();
	
	return true;
}

void SDLWindow::restoreSurfaces() {
	// nothing to do?
}

void SDLWindow::evictManagedTextures() {
	// nothing to do?
}

void SDLWindow::setGammaRamp(const u16 * red, const u16 * green, const u16 * blue) {
	SDL_SetGammaRamp(red, green, blue);
}

void SDLWindow::hide() {
	SDL_WM_IconifyWindow();
	OnShow(false);
}
