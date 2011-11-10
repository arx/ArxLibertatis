/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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
		
		inline DisplayMode() { }
		inline DisplayMode(const DisplayMode & o) : resolution(o.resolution), depth(o.depth) { }
		inline DisplayMode(Vec2i res, unsigned bits) : resolution(res), depth(bits) { }
		bool operator<(const DisplayMode & other) const;
		inline bool operator==(const DisplayMode & other) const {
			return resolution == other.resolution && depth == other.depth;
		}
	};
	
	typedef std::vector<DisplayMode> DisplayModes;
	
	inline RenderWindow() : renderer(NULL) { }
	virtual ~RenderWindow() { }
	
	class RendererListener {
		
	public:
		
		virtual ~RendererListener() { }
		
		virtual void onRendererInit(RenderWindow &) { }
		virtual void onRendererShutdown(RenderWindow &) { }
		
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
	
	virtual void evictManagedTextures() = 0;
	
	virtual void setGammaRamp(const u16 * red, const u16 * green, const u16 * blue) = 0;
	void setGamma(float brightness, float contrast, float gamma);
	
protected:
	
	Renderer * renderer;
	DisplayModes displayModes; //! Available fullscreen modes.
	
	void onRendererInit();
	void onRendererShutdown();
	
private:
	
	typedef std::vector<RendererListener *> RendererListeners;
	
	RendererListeners renderListeners; //! Listeners for renderer events
};

#endif // ARX_WINDOW_RENDERWINDOW_H
