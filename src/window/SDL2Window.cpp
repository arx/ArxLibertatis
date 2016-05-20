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
#include <cstdlib>

#include "Configure.h"

#if ARX_HAVE_SETENV || ARX_HAVE_UNSETENV
#include <stdlib.h>
#endif

#ifdef ARX_DEBUG
#include <signal.h>
#endif

#include <boost/scope_exit.hpp>

#include "platform/Platform.h"

#if ARX_PLATFORM == ARX_PLATFORM_WIN32
#include <windows.h>
#include <shellapi.h>
#endif

#if ARX_PLATFORM != ARX_PLATFORM_WIN32
#define SDL_PROTOTYPES_ONLY 1
#endif
#include <SDL_syswm.h>

#include "gui/Credits.h"
#include "graphics/opengl/GLDebug.h"
#include "graphics/opengl/OpenGLRenderer.h"
#include "input/SDL2InputBackend.h"
#include "io/log/Logger.h"
#include "math/Rectangle.h"
#include "platform/CrashHandler.h"
#include "platform/WindowsUtils.h"

// Avoid including SDL_syswm.h without SDL_PROTOTYPES_ONLY on non-Windows systems
// it includes X11 stuff which pullutes the namespace global namespace.
typedef enum {
	ARX_SDL_SYSWM_UNKNOWN,
	ARX_SDL_SYSWM_WINDOWS,
	ARX_SDL_SYSWM_X11,
	ARX_SDL_SYSWM_DIRECTFB,
	ARX_SDL_SYSWM_COCOA,
	ARX_SDL_SYSWM_UIKIT,
	ARX_SDL_SYSWM_WAYLAND,
	ARX_SDL_SYSWM_MIR,
	ARX_SDL_SYSWM_WINRT,
	ARX_SDL_SYSWM_ANDROID
} ARX_SDL_SYSWM_TYPE;
struct ARX_SDL_SysWMinfo {
	SDL_version version;
	ARX_SDL_SYSWM_TYPE subsystem;
	char padding[1024];
};

SDL2Window * SDL2Window::s_mainWindow = NULL;

SDL2Window::SDL2Window()
	: m_window(NULL)
	, m_glcontext(NULL)
	, m_input(NULL)
	, m_minimizeOnFocusLost(AlwaysEnabled)
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
	
	#if defined(ARX_DEBUG) && defined(SDL_HINT_NO_SIGNAL_HANDLERS)
	// SDL 2.0.4+
	SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
	#endif
	
	const char * minimize = SDL_GetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS);
	if(minimize) {
		if(*minimize == '0') {
			m_minimizeOnFocusLost = AlwaysDisabled;
		} else {
			m_minimizeOnFocusLost = AlwaysEnabled;
		}
	} else {
		m_minimizeOnFocusLost = Enabled;
	}
	
	arx_assert(s_mainWindow == NULL, "SDL only supports one window"); // TODO it supports multiple windows now!
	arx_assert(m_displayModes.empty());
	
	const char * headerVersion = ARX_STR(SDL_MAJOR_VERSION) "." ARX_STR(SDL_MINOR_VERSION)
	                             "." ARX_STR(SDL_PATCHLEVEL);
	CrashHandler::setVariable("SDL version (headers)", headerVersion);
	
	#if ARX_PLATFORM != ARX_PLATFORM_WIN32 && ARX_HAVE_SETENV && ARX_HAVE_UNSETENV
	/*
	 * We want the X11 WM_CLASS to be "arx-libertatis" to match the .desktop file,
	 * but SDL does not let us set it directly.
	 */
	const char * oldClass = std::getenv("SDL_VIDEO_X11_WMCLASS");
	if(!oldClass) {
		setenv("SDL_VIDEO_X11_WMCLASS", "arx-libertatis", 1);
	}
	BOOST_SCOPE_EXIT((oldClass)) {
		if(!oldClass) {
			// Don't overrride WM_CLASS for SDL child processes
			unsetenv("SDL_VIDEO_X11_WMCLASS");
		}
	} BOOST_SCOPE_EXIT_END
	#endif
	
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
		LogError << "Failed to initialize SDL: " << SDL_GetError();
		return false;
	}
	
	SDL_version ver;
	SDL_GetVersion(&ver);
	std::ostringstream runtimeVersion;
	runtimeVersion << int(ver.major) << '.' << int(ver.minor) << '.' << int(ver.patch);
	LogInfo << "Using SDL " << runtimeVersion.str();
	CrashHandler::setVariable("SDL version (runtime)", runtimeVersion.str());
	credits::setLibraryCredits("windowing", "SDL " + runtimeVersion.str());
	
	#ifdef ARX_DEBUG
	// No SDL, this is more annoying than helpful!
	if(ver.major == 2 && ver.minor == 0 && ver.patch < 4) {
		// Earlier versions don't support SDL_HINT_NO_SIGNAL_HANDLERS
		#if defined(SIGINT)
		signal(SIGINT, SIG_DFL);
		#endif
		#if defined(SIGTERM)
		signal(SIGTERM, SIG_DFL);
		#endif
	}
	#endif
	
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
	
	#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	// Used on Windows to prevent software opengl fallback.
	// The linux situation:
	// Causes SDL to require visuals without caveats.
	// On linux some drivers only supply multisample capable GLX Visuals
	// with a GLX_NON_CONFORMANT_VISUAL_EXT caveat.
	// see: https://www.opengl.org/registry/specs/EXT/visual_rating.txt
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	#endif
	
	// TODO EGL and core profile are not supported yet
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	
	if(gldebug::isEnabled()) {
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	}
	
	
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
		int msaaEnabled, msaaValue;
		SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &msaaEnabled);
		SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &msaaValue);
		if(!lastTry) {
			if(!msaaEnabled || msaaValue < msaa) {
				continue;
			}
		}
		if(msaaEnabled) {
			m_MSAALevel = msaaValue;
		} else {
			m_MSAALevel = 0;
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
		const char * system = "(unknown)";
		{
		  ARX_SDL_SysWMinfo info;
			info.version.major = 2;
			info.version.minor = 0;
			info.version.patch = 4;
			if(SDL_GetWindowWMInfo(m_window, reinterpret_cast<SDL_SysWMinfo *>(&info))) {
				switch(info.subsystem) {
					case ARX_SDL_SYSWM_UNKNOWN:   break;
					case ARX_SDL_SYSWM_WINDOWS:   system = "Windows"; break;
					case ARX_SDL_SYSWM_X11:       system = "X11"; break;
					#if SDL_VERSION_ATLEAST(2, 0, 3)
					case ARX_SDL_SYSWM_WINRT:     system = "WinRT"; break;
					#endif
					case ARX_SDL_SYSWM_DIRECTFB:  system = "DirectFB"; break;
					case ARX_SDL_SYSWM_COCOA:     system = "Cocoa"; break;
					case ARX_SDL_SYSWM_UIKIT:     system = "UIKit"; break;
					#if SDL_VERSION_ATLEAST(2, 0, 2)
					case ARX_SDL_SYSWM_WAYLAND:   system = "Wayland"; break;
					case ARX_SDL_SYSWM_MIR:       system = "Mir"; break;
					#endif
					#if SDL_VERSION_ATLEAST(2, 0, 4)
					case ARX_SDL_SYSWM_ANDROID:   system = "Android"; break;
					#endif
				}
			}
		}
		
		int red = 0, green = 0, blue = 0, alpha = 0, depth = 0, doublebuffer = 0;
		SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &red);
		SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &green);
		SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &blue);
		SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &alpha);
		SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &depth);
		SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &doublebuffer);
		LogInfo << "Window: " << system << " r:" << red << " g:" << green << " b:" << blue
		        << " a:" << alpha << " depth:" << depth << " aa:" << msaa << "x"
		        << " doublebuffer:" << doublebuffer;
		break;
	}
	
	// Use the executable icon for the window
	#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	{
		SDL_SysWMinfo info;
		SDL_VERSION(&info.version);
		if(SDL_GetWindowWMInfo(m_window, &info) && info.subsystem == SDL_SYSWM_WINDOWS) {
			platform::WideString filename;
			filename.allocate(filename.capacity());
			while(true) {
				DWORD size = GetModuleFileNameW(NULL, filename.data(), filename.size());
				if(size < filename.size()) {
					filename.resize(size);
					break;
				}
				filename.allocate(filename.size() * 2);
			}
			HICON largeIcon = 0;
			HICON smallIcon = 0;
			ExtractIconExW(filename, 0, &largeIcon, &smallIcon, 1);
			if(smallIcon) {
				SendMessage(info.info.win.window, WM_SETICON, ICON_SMALL, LPARAM(smallIcon));
			}
			if(largeIcon) {
				SendMessage(info.info.win.window, WM_SETICON, ICON_BIG, LPARAM(largeIcon));
			}
		}
	}
	#endif
	
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
	
	m_renderer->beforeResize(false);
	
	if(makeFullscreen) {
		if(wasFullscreen) {
			// SDL will not update the window size with the new mode if already fullscreen
			SDL_SetWindowFullscreen(m_window, 0);
		}
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

void SDL2Window::setMinimizeOnFocusLost(bool enabled) {
	if(m_minimizeOnFocusLost != AlwaysDisabled && m_minimizeOnFocusLost != AlwaysEnabled) {
		SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, enabled ? "1" : "0");
		m_minimizeOnFocusLost = enabled ? Enabled : Disabled;
	}
}

Window::MinimizeSetting SDL2Window::willMinimizeOnFocusLost() {
	return m_minimizeOnFocusLost;
}

InputBackend * SDL2Window::getInputBackend() {
	if(!m_input) {
		m_input = new SDL2InputBackend(this);
	}
	return m_input;
}
