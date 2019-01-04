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

#include "window/SDL2Window.h"

#include <algorithm>
#include <sstream>
#include <cstdlib>

#include "Configure.h"

#ifdef ARX_DEBUG
#include <signal.h>
#endif

#include "platform/Platform.h"

#if ARX_PLATFORM == ARX_PLATFORM_WIN32
#include <windows.h>
#include <shellapi.h>
#endif

#if ARX_HAVE_DLSYM
#include <dlfcn.h>
#endif

#if ARX_PLATFORM != ARX_PLATFORM_WIN32
#define SDL_PROTOTYPES_ONLY 1
#endif
#include <SDL_syswm.h>

#include "core/Config.h"
#include "core/Version.h"
#include "gui/Credits.h"
#include "graphics/opengl/GLDebug.h"
#include "graphics/opengl/OpenGLRenderer.h"
#include "input/SDL2InputBackend.h"
#include "io/log/Logger.h"
#include "math/Rectangle.h"
#include "platform/CrashHandler.h"
#include "platform/Environment.h"
#include "platform/WindowsUtils.h"
#include "platform/profiler/Profiler.h"
#include "window/SDL2X11Util.h"

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
	ARX_SDL_SYSWM_ANDROID,
	ARX_SDL_SYSWM_VIVANTE,
	ARX_SDL_SYSWM_OS2,
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
	, m_allowScreensaver(AlwaysDisabled)
	, m_gamma(1.f)
	, m_gammaOverridden(false)
{
	m_renderer = new OpenGLRenderer;
}

SDL2Window::~SDL2Window() {
	
	delete m_input;
	
	if(m_renderer) {
		delete m_renderer, m_renderer = NULL;
	}
	
	if(m_glcontext) {
		SDL_GL_DeleteContext(m_glcontext);
	}
	
	if(m_window) {
		restoreGamma();
		SDL_DestroyWindow(m_window);
	}
	
	if(s_mainWindow) {
		SDL_Quit(), s_mainWindow = NULL;
	}
	
}

#ifndef SDL_HINT_VIDEO_ALLOW_SCREENSAVER // SDL 2.0.2+
#define SDL_HINT_VIDEO_ALLOW_SCREENSAVER "SDL_VIDEO_ALLOW_SCREENSAVER"
#endif
#ifndef SDL_HINT_NO_SIGNAL_HANDLERS // SDL 2.0.4+
#define SDL_HINT_NO_SIGNAL_HANDLERS "SDL_NO_SIGNAL_HANDLERS"
#endif
#ifndef SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH // SDL 2.0.5+
#define SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH "SDL_MOUSE_FOCUS_CLICKTHROUGH"
#endif

static Window::MinimizeSetting getInitialSDLSetting(const char * hint, Window::MinimizeSetting def) {
	const char * setting = SDL_GetHint(hint);
	if(!setting) {
		return def;
	}
	return (*setting == '0') ? Window::AlwaysDisabled : Window::AlwaysEnabled;
}

bool SDL2Window::initializeFramework() {
	
	#if defined(ARX_DEBUG)
	SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
	#endif
	
	SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
	
	m_minimizeOnFocusLost = getInitialSDLSetting(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, Enabled);
	m_allowScreensaver = getInitialSDLSetting(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, Disabled);
	
	arx_assert_msg(s_mainWindow == NULL, "SDL only supports one window"); // TODO it supports multiple windows now!
	arx_assert(m_displayModes.empty());
	
	const char * headerVersion = ARX_STR(SDL_MAJOR_VERSION) "." ARX_STR(SDL_MINOR_VERSION)
	                             "." ARX_STR(SDL_PATCHLEVEL);
	CrashHandler::setVariable("SDL version (headers)", headerVersion);
	
	#if ARX_PLATFORM != ARX_PLATFORM_WIN32 && ARX_PLATFORM != ARX_PLATFORM_MACOS
	platform::EnvironmentOverride overrrides[] = {
		/*
		 * We want the X11 WM_CLASS to match the .desktop file and icon name,
		 * but SDL does not let us set it directly.
		 * This is also used by SDL's Wayland backend!
		 */
		{ "SDL_VIDEO_X11_WMCLASS",  arx_icon_name.c_str() },
	};
	platform::EnvironmentLock environment(overrrides);
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
	SDL_EventState(SDL_DROPFILE,    SDL_ENABLE);
	SDL_EventState(SDL_SYSWMEVENT,  SDL_IGNORE);
	SDL_EventState(SDL_USEREVENT,   SDL_IGNORE);
	
	return true;
}

static Uint32 getSDLFlagsForMode(const Vec2i & size, bool fullscreen) {
	
	Uint32 flags = 0;
	
	if(fullscreen) {
		if(size == Vec2i(0)) {
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		} else {
			flags |= SDL_WINDOW_FULLSCREEN;
		}
	}
	
	return flags;
}

int SDL2Window::createWindowAndGLContext(const char * profile) {
	
	int x = SDL_WINDOWPOS_UNDEFINED, y = SDL_WINDOWPOS_UNDEFINED;
	Uint32 windowFlags = getSDLFlagsForMode(m_size, m_fullscreen);
	windowFlags |= SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;
	
	for(int msaa = m_maxMSAALevel; true; msaa--) {
		bool lastTry = (msaa == 0);
		
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
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, msaa > 0 ? 24 : 16);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE,   msaa > 0 ? 8 : 3);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, msaa > 0 ? 8 : 3);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,  msaa > 0 ? 8 : 2);
		
		m_window = SDL_CreateWindow(m_title.c_str(), x, y, m_size.x, m_size.y, windowFlags);
		if(!m_window) {
			if(lastTry) {
				LogError << "Could not create " << profile << " window: " << SDL_GetError();
				return 0;
			}
			continue;
		}
		
		m_glcontext = SDL_GL_CreateContext(m_window);
		if(!m_glcontext) {
			if(lastTry) {
				LogError << "Could not create " << profile << " context: " << SDL_GetError();
				return 0;
			}
			continue;
		}
		
		// Verify that the MSAA setting matches what was requested
		if(msaa > 1) {
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
		if(glGetError() != GL_NO_ERROR || texunits < m_minTextureUnits) {
			if(lastTry) {
				m_renderer->initialize(); // Log hardware information
				LogError << "Not enough " << profile << " texture units available: have " << texunits
				         << ", need at least " << m_minTextureUnits;
				return 0;
			}
			continue;
		}
		
		return std::max(msaa, 1);
	}
	
}

bool SDL2Window::initialize() {
	
	arx_assert(!m_displayModes.empty());
	
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	// Used on Windows to prevent software opengl fallback.
	// The linux situation:
	// Causes SDL to require visuals without caveats.
	// On linux some drivers only supply multisample capable GLX Visuals
	// with a GLX_NON_CONFORMANT_VISUAL_EXT caveat.
	// see: https://www.opengl.org/registry/specs/EXT/visual_rating.txt
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	#endif
	
	if(gldebug::isEnabled()) {
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	} else {
		#if SDL_VERSION_ATLEAST(2, 0, 6)
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_NO_ERROR, 1);
		#endif
	}
	
	bool autoRenderer = (config.video.renderer == "auto");
	
	int samples = 0;
	for(int i = 0; i < 2 && samples == 0; i++) {
		bool first = (i == 0);
		
		bool matched = false;
		
		if(samples == 0 && first == (autoRenderer || config.video.renderer == "OpenGL")) {
			matched = true;
			// TODO core profile are not supported yet
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
			samples = createWindowAndGLContext("Desktop OpenGL");
		}
		
		#if ARX_HAVE_EPOXY
		if(samples == 0 && first == (autoRenderer || config.video.renderer == "OpenGL ES")) {
			matched = true;
			// TODO OpenGL ES 2.0+ is not supported yet
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
			samples = createWindowAndGLContext("OpenGL ES");
		}
		#endif
		
		if(first && !matched) {
			LogError << "Unknown renderer: " << config.video.renderer;
		}
	}
	
	if(samples == 0) {
		return false;
	}
	
	// All good
	{
		const char * windowSystem = "(unknown)";
		{
		  ARX_SDL_SysWMinfo info;
			info.version.major = 2;
			info.version.minor = 0;
			info.version.patch = 6;
			if(SDL_GetWindowWMInfo(m_window, reinterpret_cast<SDL_SysWMinfo *>(&info))) {
				switch(info.subsystem) {
					case ARX_SDL_SYSWM_UNKNOWN:   break;
					case ARX_SDL_SYSWM_WINDOWS:   windowSystem = "Windows"; break;
					case ARX_SDL_SYSWM_X11:       windowSystem = "X11"; break;
					case ARX_SDL_SYSWM_DIRECTFB:  windowSystem = "DirectFB"; break;
					case ARX_SDL_SYSWM_COCOA:     windowSystem = "Cocoa"; break;
					case ARX_SDL_SYSWM_UIKIT:     windowSystem = "UIKit"; break;
					case ARX_SDL_SYSWM_WAYLAND:   windowSystem = "Wayland"; break;
					case ARX_SDL_SYSWM_MIR:       windowSystem = "Mir"; break;
					case ARX_SDL_SYSWM_WINRT:     windowSystem = "WinRT"; break;
					case ARX_SDL_SYSWM_ANDROID:   windowSystem = "Android"; break;
					case ARX_SDL_SYSWM_VIVANTE:   windowSystem = "Vivante"; break;
					case ARX_SDL_SYSWM_OS2:       windowSystem = "OS2"; break;
					default: LogWarning << "Unknown SDL video backend: " << info.subsystem;
				}
				#if ARX_PLATFORM != ARX_PLATFORM_WIN32 && ARX_PLATFORM != ARX_PLATFORM_MACOS
				#if ARX_HAVE_EPOXY
				const char * wrangler = "libepoxy";
				#else
				const char * wrangler = "GLEW";
				#endif
				switch(info.subsystem) {
					case ARX_SDL_SYSWM_X11: {
						SDL_version ver;
						SDL_GetVersion(&ver);
						if(ver.major == 2 && ver.minor == 0 && ver.patch < 9) {
							// Work around a bug causing dbus-daemon memory usage to continually rise while AL is running
							// if the org.gnome.ScreenSaver service does not exist.
							if(m_allowScreensaver != AlwaysDisabled && m_allowScreensaver != AlwaysEnabled) {
								SDL_EnableScreenSaver();
								m_allowScreensaver = AlwaysEnabled;
							}
						}
						#if ARX_HAVE_GL_STATIC || !ARX_HAVE_DLSYM || !defined(RTLD_DEFAULT)
						const bool haveGLX = ARX_HAVE_GLX;
						#elif ARX_HAVE_EPOXY
						const bool haveGLX = (dlsym(RTLD_DEFAULT, "epoxy_has_glx") != NULL);
						#else
						const bool haveGLX = (dlsym(RTLD_DEFAULT, "glxewInit") != NULL);
						#endif
						if(!haveGLX) {
							LogWarning << "SDL is using the X11 video backend but " << wrangler
							           << " was built without GLX support";
							LogWarning << "Try setting the SDL_VIDEODRIVER=wayland environment variable";
						}
						break;
					}
					case ARX_SDL_SYSWM_WAYLAND:
					case ARX_SDL_SYSWM_MIR: {
						#if ARX_HAVE_GL_STATIC || !ARX_HAVE_DLSYM || !defined(RTLD_DEFAULT)
						const bool haveEGL = ARX_HAVE_EGL;
						#elif ARX_HAVE_EPOXY
						const bool haveEGL = (dlsym(RTLD_DEFAULT, "epoxy_has_egl") != NULL);
						#else
						const bool haveEGL = (dlsym(RTLD_DEFAULT, "eglewInit") != NULL);
						#endif
						if(!haveEGL) {
							LogWarning << "SDL is using the " << windowSystem << " video backend but " << wrangler
							           << " was built without EGL support";
							LogWarning << "Try setting the SDL_VIDEODRIVER=x11 environment variable";
						}
						break;
					}
					default: break;
				}
				#endif
			}
		}
		
		int red = 0, green = 0, blue = 0, alpha = 0, depth = 0, doublebuffer = 0;
		SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &red);
		SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &green);
		SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &blue);
		SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &alpha);
		SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &depth);
		SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &doublebuffer);
		LogInfo << "Window: " << windowSystem << " r:" << red << " g:" << green << " b:" << blue
		        << " a:" << alpha << " depth:" << depth << " aa:" << samples << "x"
		        << " doublebuffer:" << doublebuffer;
	}
	
	// Use the executable icon for the window
	#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	u64 nativeWindow = 0;
	{
		SDL_SysWMinfo info;
		SDL_VERSION(&info.version);
		if(SDL_GetWindowWMInfo(m_window, &info) && info.subsystem == SDL_SYSWM_WINDOWS) {
			nativeWindow = u64(info.info.win.window);
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
	#elif ARX_PLATFORM != ARX_PLATFORM_MACOS
	u64 nativeWindow = SDL2X11_getNativeWindowHandle(m_window);
	#else
	u64 nativeWindow = 0;
	#endif
	CrashHandler::setWindow(nativeWindow);
	
	setVSync(m_vsync);
	
	SDL_ShowWindow(m_window);
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

void SDL2Window::restoreGamma() {
	if(m_gammaOverridden) {
		SDL_SetWindowGammaRamp(m_window, m_gammaRed, m_gammaGreen, m_gammaBlue);
		m_gammaOverridden = false;
	}
}

bool SDL2Window::setGamma(float gamma) {
	if(m_window && m_fullscreen) {
		if(!m_gammaOverridden) {
			m_gammaOverridden = (SDL_GetWindowGammaRamp(m_window, m_gammaRed, m_gammaGreen, m_gammaBlue) == 0);
		}
		if(SDL_SetWindowBrightness(m_window, gamma) != 0) {
			return false;
		}
	}
	m_gamma = gamma;
	return true;
}

void SDL2Window::changeMode(DisplayMode mode, bool fullscreen) {
	
	if(!m_window) {
		m_size = mode.resolution;
		m_fullscreen = fullscreen;
		return;
	}
	
	if(m_fullscreen == fullscreen && m_size == mode.resolution) {
		return;
	}
	
	bool wasFullscreen = m_fullscreen;
	
	m_renderer->beforeResize(false);
	
	if(fullscreen) {
		if(wasFullscreen) {
			// SDL will not update the window size with the new mode if already fullscreen
			SDL_SetWindowFullscreen(m_window, 0);
		}
		if(mode.resolution != Vec2i(0)) {
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
	
	Uint32 flags = getSDLFlagsForMode(mode.resolution, fullscreen);
	if(SDL_SetWindowFullscreen(m_window, flags) < 0) {
		return;
	}
	
	if(!fullscreen) {
		if(wasFullscreen) {
			restoreGamma();
			SDL_RestoreWindow(m_window);
		}
		SDL_SetWindowSize(m_window, mode.resolution.x, mode.resolution.y);
	}
	
	if(wasFullscreen != fullscreen) {
		onToggleFullscreen(fullscreen);
	}
	
	if(fullscreen) {
		setGamma(m_gamma);
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
			
			case SDL_DROPFILE: {
				onDroppedFile(event.drop.file);
				SDL_free(event.drop.file);
				return;
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
	ARX_PROFILE_FUNC();
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

std::string SDL2Window::getClipboardText() {
	char * text = SDL_GetClipboardText();
	std::string result;
	if(text) {
		result = text;
		SDL_free(text);
	}
	return result;
}

void SDL2Window::setClipboardText(const std::string & text) {
	SDL_SetClipboardText(text.c_str());
}

void SDL2Window::allowScreensaver(bool allowed) {
	
	if(m_allowScreensaver == AlwaysDisabled || m_allowScreensaver == AlwaysEnabled) {
		return;
	}
	
	MinimizeSetting setting = allowed ? Enabled : Disabled;
	if(m_allowScreensaver != setting) {
		if(allowed) {
			SDL_EnableScreenSaver();
		} else {
			SDL_DisableScreenSaver();
		}
		m_allowScreensaver = setting;
	}
}

InputBackend * SDL2Window::getInputBackend() {
	if(!m_input) {
		m_input = new SDL2InputBackend(this);
	}
	return m_input;
}
