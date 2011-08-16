
#ifndef ARX_WINDOW_RENDERWINDOW_H
#define ARX_WINDOW_RENDERWINDOW_H

#include <vector>

#include "window/Window.h"

class Renderer;

class RenderWindow : public Window {
	
public:
	
	typedef std::vector<Vec2i> DisplayModes;
	
	inline RenderWindow() : renderer(NULL), vsync(false) { };
	virtual ~RenderWindow() { };
	
	class RendererListener {
		
	public:
		
		virtual void onRendererInit(RenderWindow &) { };
		virtual void onRendererShutdown(RenderWindow &) { };
		
	};
	
	inline Renderer * getRenderer() { return renderer; }
	
	inline const DisplayModes & getDisplayModes() { return displayModes; }
	
	void addListener(RendererListener * listener);
	void removeListener(RendererListener * listener);
	
	virtual bool showFrame() = 0;
	virtual void restoreSurfaces() = 0;
	
	inline virtual void setVSync(bool _vsync) { vsync = _vsync; }
	inline bool isVSync() { return vsync; };
	
	virtual void evictManagedTextures() = 0;
	
protected:
	
	Renderer * renderer;
	DisplayModes displayModes; //! Available fullscreen modes.
	
	void onRendererInit();
	void onRendererShutdown();
	
private:
	
	typedef std::vector<RendererListener *> RendererListeners;
	
	RendererListeners renderListeners; //! Listeners for renderer events
	
	bool vsync;
	
};

#endif // ARX_WINDOW_RENDERWINDOW_H
