/*
 * Copyright 2011-2017 Arx Libertatis Team (see the AUTHORS file)
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

#include <algorithm>

#include "window/Window.h"

class Renderer;

class RenderWindow : public Window {
	
public:
	
	typedef std::vector<DisplayMode> DisplayModes;
	
	RenderWindow()
		: m_minTextureUnits(1)
		, m_maxMSAALevel(1)
		, m_vsync(1)
		, m_renderer(NULL)
		{ }
	
	virtual ~RenderWindow() { }
	
	/*!
	 * Initialize the framework.
	 * This needs to be called before anything else!
	 */
	virtual bool initializeFramework() = 0;
	
	/*!
	 * Set the minimum number of texture units required.
	 * Must be set before calling \ref initialize().
	 */
	void setMinTextureUnits(int units) { m_minTextureUnits = units; }
	
	/*!
	 * Set the maximum MSAA level to use.
	 * Actual level may be lower if the requested one is not supported by the HW.
	 * Must be set before calling \ref initialize().
	 */
	void setMaxMSAALevel(int msaa) { m_maxMSAALevel = std::max(1, msaa); }
	
	/*!
	 * Enebly or disable vsync.
	 * May not have any effect when called after \ref initialize().
	 *
	 * \param vsync 1 to enable vsync, 0 to disable or -1 to allow late swaps to
	 *              happen immediately if supported.
	 *
	 * \return true if the vsync setting was successfully changed.
	 */
	virtual bool setVSync(int vsync) = 0;
	
	/*!
	 * Set the monitor gamma when in fullscreen mode.
	 */
	virtual bool setGamma(float gamma = 1.f) = 0;
	
	Renderer * getRenderer() { return m_renderer; }
	
	//! Get a sorted list of supported fullscreen display modes.
	const DisplayModes & getDisplayModes() { return m_displayModes; }
	
	virtual void showFrame() = 0;
	
protected:
	
	int m_minTextureUnits;
	int m_maxMSAALevel;
	int m_vsync;
	
	Renderer * m_renderer;
	DisplayModes m_displayModes; //! Available fullscreen modes.
	
};

#endif // ARX_WINDOW_RENDERWINDOW_H
