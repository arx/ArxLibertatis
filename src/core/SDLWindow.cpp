
#include "core/SDLWindow.h"

#include "graphics/opengl/OpenGLRenderer.h"
#include "io/Logger.h"

static SDLWindow * mainWindow = NULL;

SDLWindow::SDLWindow() : window(NULL) {
	mainWindow = this;
}

SDLWindow::~SDLWindow() {
	
	if(renderer) {
		onRendererShutdown();
		delete renderer, renderer = NULL;
	}
	
	if(window) {
		SDL_Quit(), window = NULL;
	}
}

bool SDLWindow::Init(const std::string & title, int width, int height, bool visible, bool fullscreen) {
	
	
	ARX_UNUSED(title), ARX_UNUSED(visible);
	
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		LogError << "Failed to initialize SDL: " << SDL_GetError();
		return false;
	}
	
	SDL_SetEventFilter(eventFilter);
	
	SDL_EventState(SDL_ACTIVEEVENT, SDL_ENABLE);
	SDL_EventState(SDL_KEYDOWN, SDL_ENABLE);
	SDL_EventState(SDL_KEYUP, SDL_ENABLE);
	SDL_EventState(SDL_MOUSEMOTION, SDL_ENABLE);
	SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_ENABLE);
	SDL_EventState(SDL_MOUSEBUTTONUP, SDL_ENABLE);
	SDL_EventState(SDL_JOYAXISMOTION, SDL_ENABLE);
	SDL_EventState(SDL_JOYBALLMOTION, SDL_ENABLE);
	SDL_EventState(SDL_JOYHATMOTION, SDL_ENABLE);
	SDL_EventState(SDL_JOYBUTTONDOWN, SDL_ENABLE);
	SDL_EventState(SDL_JOYBUTTONUP, SDL_ENABLE);
	SDL_EventState(SDL_QUIT, SDL_ENABLE);
	SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
	SDL_EventState(SDL_VIDEORESIZE, SDL_ENABLE);
	SDL_EventState(SDL_VIDEOEXPOSE, SDL_ENABLE);
	SDL_EventState(SDL_USEREVENT, SDL_IGNORE);
	
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	if(!setMode(Vec2i(width, height), fullscreen)) {
		SDL_Quit();
		return false;
	}
	
	OnCreate();
	
	renderer = new OpenGLRenderer;
	renderer->Initialize();
	
	onRendererInit();
	
	OnResize(window->w, window->h);
	if(fullscreen) {
		OnMakeFullscreen();
	}
	
	OnShow(true);
	
	return true; // TODO implement
}

bool SDLWindow::setMode(Vec2i size, bool fullscreen) {
	
	Uint32 flags = SDL_ANYFORMAT | SDL_OPENGL | SDL_DOUBLEBUF;
	flags |= (fullscreen) ? SDL_FULLSCREEN : SDL_RESIZABLE;
	SDL_Surface * win = SDL_SetVideoMode(size.x, size.y, 32, flags);
	if(!win) {
		SDL_Quit();
		return false;
	}
	
	window = win;
	return true;
}

void * SDLWindow::GetHandle() {
	return NULL; // TODO implement
}

void SDLWindow::SetFullscreen(bool fullscreen) {
	
	if(fullscreen == IsFullScreen()) {
		return;
	}
	
	setMode(GetSize(), fullscreen);
	
	if(fullscreen) {
		OnMakeFullscreen();
	}
}

void SDLWindow::SetSize(Vec2i size) {
	
	if(size == GetSize()) {
		return;
	}
	
	setMode(size, IsFullScreen());
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
					OnFocus(event.active.gain == 1);
				} else if(event.active.type == SDL_APPACTIVE) {
					if(event.active.gain) {
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
				// TODO input event
				break;
			}
			
			case SDL_QUIT: {
				OnDestroy();
				break;
			}
			
			case SDL_VIDEORESIZE: {
				OnResize(event.resize.w, event.resize.h);
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
