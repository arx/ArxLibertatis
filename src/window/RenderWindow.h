
#ifndef ARX_WINDOW_RENDERWINDOW_H
#define ARX_WINDOW_RENDERWINDOW_H

#include <vector>

#include "window/Window.h"

class Renderer;

class RenderWindow : public Window {
	
public:
	
	struct DisplayMode {
		
		Vec2i resolution;
		unsigned depth;
		
		inline DisplayMode() { };
		inline DisplayMode(const DisplayMode & o) : resolution(o.resolution), depth(o.depth) { };
		inline DisplayMode(Vec2i res, unsigned bits) : resolution(res), depth(bits) { };
		bool operator<(const DisplayMode & other) const;
		inline bool operator==(const DisplayMode & other) const {
			return resolution == other.resolution && depth == other.depth;
		}
	};
	
	typedef std::vector<DisplayMode> DisplayModes;
	
	inline RenderWindow() : renderer(NULL), vsync(false) { };
	virtual ~RenderWindow() { };
	
	class RendererListener {
		
	public:
		
		virtual void onRendererInit(RenderWindow &) { };
		virtual void onRendererShutdown(RenderWindow &) { };
		
	};
	
	/*!
	 * Initialize the framework.
	 * This needs to be called before init() or getDisplayModes()
	 */
	virtual bool initFramework() = 0;
	
	inline Renderer * getRenderer() { return renderer; }
	
	//! Get a sorted list of supported fullscreen display modes.
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
