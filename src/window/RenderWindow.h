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

#ifndef ARX_WINDOW_RENDERWINDOW_H
#define ARX_WINDOW_RENDERWINDOW_H

#include <vector>
#include <algorithm>

#include "window/Window.h"

class Renderer;

class RenderWindow : public Window {
	
public:
	
	struct DisplayMode {
		
		Vec2i resolution;
		
		DisplayMode() { }
		DisplayMode(const DisplayMode & o) : resolution(o.resolution) { }
		DisplayMode(Vec2i res) : resolution(res) { }
		bool operator<(const DisplayMode & other) const;
		bool operator==(const DisplayMode & other) const {
			return resolution == other.resolution;
		}
		
	};
	
	typedef std::vector<DisplayMode> DisplayModes;
	
	RenderWindow()
		: m_minTextureUnits(1)
		, m_maxMSAALevel(1)
		, m_vsync(1)
		, renderer(NULL)
		{ }
	
	virtual ~RenderWindow() { }
	
	class RendererListener {
		
	public:
		
		virtual ~RendererListener() { }
		
		virtual void onRendererInit(RenderWindow &) { }
		virtual void onRendererShutdown(RenderWindow &) { }
		
	};
	
	/*!
	 * Initialize the framework.
	 * This needs to be called before anything else!
	 */
	virtual bool initializeFramework() = 0;
	
	/*!
	 * Set the minimum number of texture units required.
	 * Must be set before calling @ref initialize().
	 */
	void setMinTextureUnits(int units) { m_minTextureUnits = units; }
	
	/*!
	 * Set the maximum MSAA level to use.
	 * Actual level may be lower if the requested one is not supported by the HW.
	 * Must be set before calling @ref initialize().
	 */
	void setMaxMSAALevel(int msaa) { m_maxMSAALevel = std::max(1, msaa); }
	
	/*!
	 * Enebly or disable vsync.
	 * May not have any effect when called after @ref initialize().
	 *
	 * @param vsync 1 to enable vsync, 0 to disable or -1 to allow late swaps to
	 *              happen immediately if supported.
	 *
	 * @return true if the vsync setting was successfully changed.
	 */
	virtual bool setVSync(int vsync) = 0;
	
	Renderer * getRenderer() { return renderer; }
	
	//! Get a sorted list of supported fullscreen display modes.
	const DisplayModes & getDisplayModes() { return displayModes; }
	
	void addRenderListener(RendererListener * listener);
	void removeRenderListener(RendererListener * listener);
	
	virtual void showFrame() = 0;
	
protected:
	
	int m_minTextureUnits;
	int m_maxMSAALevel;
	int m_vsync;
	
	Renderer * renderer;
	DisplayModes displayModes; //! Available fullscreen modes.
	
	void onRendererInit();
	void onRendererShutdown();
	
private:
	
	typedef std::vector<RendererListener *> RendererListeners;
	
	RendererListeners renderListeners; //! Listeners for renderer events
	
};

#endif // ARX_WINDOW_RENDERWINDOW_H
