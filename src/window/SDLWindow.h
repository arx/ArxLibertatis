
#ifndef ARX_WINDOW_SDLWINDOW_H
#define ARX_WINDOW_SDLWINDOW_H

#include <SDL.h>

#include "window/RenderWindow.h"

class SDLInputBackend;

class SDLWindow : public RenderWindow {
	
public:
	
	SDLWindow();
	virtual ~SDLWindow();

	bool Init(const std::string & title, int width, int height, bool visible, bool fullscreen);
	void * GetHandle();
	void SetFullscreen(bool fullscreen);
	void SetSize(Vec2i size);
	void Tick();
	
	bool showFrame();
	void restoreSurfaces();
	
	void evictManagedTextures();
	
private:
	
	bool setMode(Vec2i size, bool fullscreen);
	
	static int SDLCALL eventFilter(const SDL_Event * event);
	
	SDL_Surface * window;
	
	SDLInputBackend * input;
	
	static SDLWindow * mainWindow;
	
	friend class SDLInputBackend;
};

#endif // ARX_WINDOW_SDLWINDOW_H
