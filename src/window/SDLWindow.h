
#ifndef ARX_WINDOW_SDLWINDOW_H
#define ARX_WINDOW_SDLWINDOW_H

#include <SDL.h>

#include "window/RenderWindow.h"

class SDLInputBackend;

class SDLWindow : public RenderWindow {
	
public:
	
	SDLWindow();
	virtual ~SDLWindow();
	
	bool initFramework();
	bool init(const std::string & title, Vec2i size, bool fullscreen, unsigned depth = 0);
	void * GetHandle();
	void setFullscreenMode(Vec2i resolution, unsigned depth = 0);
	void setWindowSize(Vec2i size);
	void Tick();
	
	bool showFrame();
	void restoreSurfaces();
	
	void evictManagedTextures();
	
	void setGammaRamp(const u16 * red, const u16 * green, const u16 * blue);
	
private:
	
	bool setMode(DisplayMode mode, bool fullscreen);
	void updateSize();
	
	static int SDLCALL eventFilter(const SDL_Event * event);
	
	SDL_Surface * window;
	
	SDLInputBackend * input;
	
	static SDLWindow * mainWindow;
	
	DisplayMode desktopMode;
	
	friend class SDLInputBackend;
};

#endif // ARX_WINDOW_SDLWINDOW_H
