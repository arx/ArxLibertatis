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

#include "window/SDL1Window.h"

#include <algorithm>
#include <sstream>

#include "boost/foreach.hpp"

#include "gui/Credits.h"
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
	, m_gamma(1.f)
	, m_gammaOverridden(false)
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
		restoreGamma();
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
	LogInfo << "Using SDL " << runtimeVersion.str();
	CrashHandler::setVariable("SDL version (runtime)", runtimeVersion.str());
	credits::setLibraryCredits("windowing", "SDL " + runtimeVersion.str());
	
	const SDL_VideoInfo * vid = SDL_GetVideoInfo();
	m_desktopMode.resolution.x = vid->current_w;
	m_desktopMode.resolution.y = vid->current_h;
	
	u32 flags = SDL_FULLSCREEN | SDL_ANYFORMAT | SDL_OPENGL | SDL_HWSURFACE;
	SDL_Rect ** modes = SDL_ListModes(NULL, flags);
	if(modes == (SDL_Rect **)(-1)) {
		
		// Any mode is supported, add some standard modes.
		
		Vec2i standardModes[] = {
			
			// 4:3
			Vec2i(640, 480), // VGA
			Vec2i(800, 600), // SVGA
			Vec2i(1024, 768), // XGA
			Vec2i(1280, 960), // SXGA-
			Vec2i(1600, 1200), // UXGA
			
			// 5:4
			Vec2i(1280, 1024), // SXGA
			
			// 16:9
			Vec2i(1280, 720), // 720p
			Vec2i(1600, 900), // 900p
			Vec2i(1920, 1080), // 1080p
			Vec2i(2048, 1152), // 2K
			Vec2i(4096, 2304), // 4K
			// ~16:9
			Vec2i(1360, 768), // "HD"
			Vec2i(1366, 768), // "HD"
			
			// 16:10
			Vec2i(1024, 640), // laptops
			Vec2i(1280, 800), // WXGA
			Vec2i(1440, 900), // WXGA+
			Vec2i(1680, 1050), // WSXGA+
			Vec2i(1920, 1200), // WUXGA
			
			
		};
		
		BOOST_FOREACH(Vec2i mode, standardModes) {
			if(m_desktopMode.resolution != mode) { \
				m_displayModes.push_back(mode); \
			}
		}
		
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
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	
	
#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	// Used on Windows to prevent software opengl fallback.
	// The linux situation:
	// Causes SDL to require visuals without caveats.
	// On linux some drivers only supply multisample capable GLX Visuals
	// with a GLX_NON_CONFORMANT_VISUAL_EXT caveat.
	// see: https://www.opengl.org/registry/specs/EXT/visual_rating.txt
	
	
	// We need an accelerated OpenGL context or we'll likely fail later
	// However, this attribute may have the opposite effect with SDL < 1.2.15 with some
	// drivers - only enable it for new enough SDL versions.
	const SDL_version * ver = SDL_Linked_Version();
	if(SDL_VERSIONNUM(ver->major, ver->minor, ver->patch) >= SDL_VERSIONNUM(1, 2, 15)) {
		SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	}
#endif
	
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
		int msaaEnabled, msaaValue;
		SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &msaaEnabled);
		SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &msaaValue);
		if(!lastTry) {
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
				m_renderer->initialize(); // Log hardware information
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
	
	setGamma(m_gamma);
	
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

void SDL1Window::restoreGamma() {
	if(m_gammaOverridden) {
		SDL_SetGamma(1.f, 1.f, 1.f);
		SDL_SetGammaRamp(m_gammaRed, m_gammaGreen, m_gammaBlue);
		m_gammaOverridden = false;
	}
}

bool SDL1Window::setGamma(float gamma) {
	if(m_initialized && m_fullscreen) {
		if(!m_gammaOverridden) {
			m_gammaOverridden = (SDL_GetGammaRamp(m_gammaRed, m_gammaGreen, m_gammaBlue) == 0);
		}
		if(SDL_SetGamma(gamma, gamma, gamma) != 0) {
			return false;
		}
	}
	m_gamma = gamma;
	return true;
}

bool SDL1Window::setMode(DisplayMode mode, bool fullscreen) {
	
	if(fullscreen && mode.resolution == Vec2i(0)) {
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
	
	if(!makeFullscreen && wasFullscreen) {
		restoreGamma();
	}
	
	if(wasFullscreen != makeFullscreen) {
		onToggleFullscreen(makeFullscreen);
	}
	
	if(makeFullscreen) {
		setGamma(m_gamma);
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
				// should be sent when Command+Q is pressed on macOS or ALT-F4 on other platforms
				// but it doesn't look like it's working as expected...
				#if ARX_PLATFORM == ARX_PLATFORM_MACOS
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

void SDL1Window::setMinimizeOnFocusLost(bool enabled) {
	ARX_UNUSED(enabled);
	// Not supported
}

Window::MinimizeSetting SDL1Window::willMinimizeOnFocusLost() {
	return AlwaysEnabled;
}

std::string SDL1Window::getClipboardText() {
	// Clipboard not supported by SDL 1
	return std::string();
}

void SDL1Window::setClipboardText(const std::string & text) {
	// Clipboard not supported by SDL 1
	ARX_UNUSED(text);
}

void SDL1Window::allowScreensaver(bool allowed) {
	// Toggling screensaver not supported by SDL 1
	ARX_UNUSED(allowed);
}

InputBackend * SDL1Window::getInputBackend() {
	if(!m_input) {
		m_input = new SDL1InputBackend();
	}
	return m_input;
}
