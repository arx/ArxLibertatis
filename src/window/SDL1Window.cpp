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

#include <algorithm>
#include <sstream>

#include "graphics/opengl/OpenGLRenderer.h"
#include "input/SDL1InputBackend.h"
#include "io/log/Logger.h"
#include "math/Rectangle.h"
#include "platform/CrashHandler.h"
#include "platform/Platform.h"

SDL1Window * SDL1Window::s_mainWindow = NULL;

SDL1Window::SDL1Window()
	: m_initialized(false)
	, m_desktopMode(Vec2i(640, 480))
	, m_input(NULL)
	{
	m_renderer = new OpenGLRenderer;
}

SDL1Window::~SDL1Window() {
	
	if(m_input) {
		delete m_input;
	}
	
	if(m_renderer) {
		delete m_renderer, m_renderer = NULL;
	}
	
	if(s_mainWindow) {
		SDL_Quit(), s_mainWindow = NULL;
	}
	
}

bool SDL1Window::initializeFramework() {
	
	arx_assert_msg(s_mainWindow == NULL, "SDL only supports one window");
	arx_assert(m_displayModes.empty());
	
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
	
	u32 flags = SDL_FULLSCREEN | SDL_ANYFORMAT | SDL_OPENGL | SDL_HWSURFACE;
	SDL_Rect ** modes = SDL_ListModes(NULL, flags);
	if(modes == (SDL_Rect **)(-1)) {
		
		// Any mode is supported, add some standard modes.
		
#define ADD_MODE(x, y) \
		if(m_desktopMode.resolution != Vec2i(x, y)) { \
			m_displayModes.push_back(Vec2i(x, y)); \
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
		
		m_displayModes.push_back(m_desktopMode);
		
	} else if(modes) {
		for(; *modes; modes++) {
			m_displayModes.push_back(Vec2i((*modes)->w, (*modes)->h));
		}
	} else {
		return false;
	}
	
	std::sort(m_displayModes.begin(), m_displayModes.end());
	m_displayModes.erase(std::unique(m_displayModes.begin(), m_displayModes.end()),
	                     m_displayModes.end());
	
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

bool SDL1Window::initialize() {
	
	arx_assert(!m_displayModes.empty());
	
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	
	// We need an accelerated OpenGL context or we'll likely fail later
	// However, this attribute may have the opposite effect with SDL < 1.2.15 with some
	// drivers - only enable it for new enough SDL versions.
	const SDL_version * ver = SDL_Linked_Version();
	if(SDL_VERSIONNUM(ver->major, ver->minor, ver->patch) >= SDL_VERSIONNUM(1, 2, 15)) {
		SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	}
	
	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, m_vsync);
	
	for(int msaa = m_maxMSAALevel; msaa > 0; msaa--) {
		bool lastTry = (msaa == 1);
		
		SDL_ClearError();
		
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, msaa > 1 ? 1 : 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, msaa > 1 ? msaa : 0);
		
		if(!setMode(m_size, m_fullscreen)) {
			if(lastTry) {
				LogError << "Could not initialize window: " << SDL_GetError();
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
	
	m_initialized = true;
	
	setTitle(m_title);
	
	SDL_ShowCursor(SDL_DISABLE);
	
	m_renderer->initialize();
	
	onCreate();
	onToggleFullscreen(m_fullscreen);
	updateSize(true);
	
	onShow(true);
	onFocus(true);
	
	return true;
}

void SDL1Window::setTitle(const std::string & title) {
	if(m_initialized) {
		SDL_WM_SetCaption(title.c_str(), title.c_str());
	}
	m_title = title;
}

bool SDL1Window::setVSync(int vsync) {
	if(m_initialized) {
		// Cannot change vsync after init
		return false;
	}
	m_vsync = (vsync != 0) ? 1 : 0;
	return true;
}

bool SDL1Window::setMode(DisplayMode mode, bool fullscreen) {
	
	if(fullscreen && mode.resolution == Vec2i_ZERO) {
		mode = m_desktopMode;
	}
	
	Uint32 flags = SDL_ANYFORMAT | SDL_OPENGL | SDL_HWSURFACE;
	flags |= (fullscreen) ? SDL_FULLSCREEN : SDL_RESIZABLE;
	if(SDL_SetVideoMode(mode.resolution.x, mode.resolution.y, 0, flags) == NULL) {
		return false;
	}
	
	return true;
}

void SDL1Window::changeMode(DisplayMode mode, bool makeFullscreen) {
	
	if(!m_initialized) {
		m_size = mode.resolution;
		m_fullscreen = makeFullscreen;
		return;
	}
	
	if(m_fullscreen == makeFullscreen && m_size == mode.resolution) {
		return;
	}
	
	bool wasFullscreen = m_fullscreen;
	
	m_renderer->beforeResize(m_fullscreen || makeFullscreen);
	
	if(!setMode(mode, makeFullscreen)) {
		return;
	}
	
	if(wasFullscreen != makeFullscreen) {
		onToggleFullscreen(makeFullscreen);
	}
	
	updateSize();
	
	tick();
}

void SDL1Window::updateSize(bool force) {
	
	Vec2i oldSize = m_size;
	
	const SDL_VideoInfo * vid = SDL_GetVideoInfo();
	m_size = Vec2i(vid->current_w, vid->current_h);
	
	if(force || m_size != oldSize) {
		m_renderer->afterResize();
		m_renderer->SetViewport(Rect(m_size.x, m_size.y));
		onResize(m_size);
	}
}

void SDL1Window::setFullscreenMode(const DisplayMode & mode) {
	changeMode(mode, true);
}

void SDL1Window::setWindowSize(const Vec2i & size) {
	changeMode(size, false);
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
				int quitkey = SDLK_q, quitmod = KMOD_META;
				#else
				int quitkey = SDLK_F4, quitmod = KMOD_ALT;
				#endif
				if(event.key.keysym.sym == quitkey
				   && (event.key.keysym.mod & quitmod) != KMOD_NONE) {
					SDL_Event quitevent;
					quitevent.type = SDL_QUIT;
					SDL_PushEvent(&quitevent);
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
				if(!m_fullscreen) {
					changeMode(Vec2i(event.resize.w, event.resize.h), false);
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
	
	if(!m_renderer->isInitialized()) {
		updateSize();
		m_renderer->afterResize();
		m_renderer->SetViewport(Rect(m_size.x, m_size.y));
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
