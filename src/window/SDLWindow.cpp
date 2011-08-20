
#include "window/SDLWindow.h"

#if ARX_PLATFORM == ARX_PLATFORM_WIN32
// TODO don't include this for linux as it includes Xlib.h, which has symbols/defines that conflict with our own! (Window, FillSolid, None)
#include <SDL_syswm.h>
#endif

#include "graphics/opengl/OpenGLRenderer.h"
#include "input/SDLInputBackend.h"
#include "io/Logger.h"
#include "math/Rectangle.h"

SDLWindow * SDLWindow::mainWindow = NULL;

SDLWindow::SDLWindow() : window(NULL), input(NULL) { }

SDLWindow::~SDLWindow() {
	
	if(renderer) {
		onRendererShutdown();
		delete renderer, renderer = NULL;
	}
	
	if(mainWindow) {
		SDL_Quit(), mainWindow = NULL;
	}
	
	window = NULL;
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
	
	m_Size = Vec2i::ZERO;
	depth = 0;
	
	if(!setMode(DisplayMode(size, fullscreen ? _depth : 0), fullscreen)) {
		return false;
	}
	
	m_IsFullscreen = fullscreen;
	
	SDL_WM_SetCaption(title.c_str(), title.c_str());
	
	const SDL_version * ver = SDL_Linked_Version();
	LogInfo << "Using SDL " << int(ver->major) << '.' << int(ver->minor) << '.' << int(ver->patch);
	
	OnCreate();
	
	renderer = new OpenGLRenderer;
	renderer->Initialize();
	
	updateSize();
	
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
	
	Uint32 flags = SDL_ANYFORMAT | SDL_OPENGL | SDL_HWSURFACE;
	flags |= (fullscreen) ? SDL_FULLSCREEN : SDL_RESIZABLE;
	SDL_Surface * win = SDL_SetVideoMode(mode.resolution.x, mode.resolution.y, mode.depth, flags);
	if(!win) {
		return false;
	}
	
	window = win;
	return true;
}

void SDLWindow::updateSize() {
	
	const SDL_VideoInfo * vid = SDL_GetVideoInfo();
	
	DisplayMode oldMode(m_Size, depth);
	
	m_Size = Vec2i(vid->current_w, vid->current_h);
	depth = vid->vfmt->BitsPerPixel;
	
	if(m_Size != oldMode.resolution) {
		
		// Finally, set the viewport for the newly created device
		renderer->SetViewport(Rect(m_Size.x, m_Size.y));
		
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
		OnToggleFullscreen();
	}
	
	updateSize();
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
		OnToggleFullscreen();
		
		updateSize();
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
				if(event.active.type == SDL_APPINPUTFOCUS) {
					// ignored
				} else if(event.active.type == SDL_APPACTIVE) {
					if(event.active.gain) {
						setMode(DisplayMode(Vec2i(event.resize.w, event.resize.h), depth), m_IsFullscreen);
						updateSize();
						OnRestore();
					} else {
						OnMinimize();
					}
				} else if(event.active.type == SDL_APPMOUSEFOCUS) {
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
				setMode(DisplayMode(Vec2i(event.resize.w, event.resize.h), depth), m_IsFullscreen);
				updateSize();
				break;
			}
			
			case SDL_VIDEOEXPOSE: {
				OnPaint();
				SDL_UpdateRect(window, 0, 0, m_Size.x, m_Size.y);
				break;
			}
			
		}
		
	}
	
}

bool SDLWindow::showFrame() {
	
	SDL_GL_SwapBuffers();
	
	if(SDL_Flip(window) != 0) {
		LogError << "Failed to update screen: " << SDL_GetError();
		return false;
	}
	
	return true;
}

void SDLWindow::restoreSurfaces() {
	// nothing to do?
}

void SDLWindow::evictManagedTextures() {
	// nothing to do?
}
