
#ifndef ARX_CORE_SDLWINDOW_H
#define ARX_CORE_SDLWINDOW_H

#include "core/RenderWindow.h"

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
	
	
};

#endif // ARX_CORE_SDLWINDOW_H
