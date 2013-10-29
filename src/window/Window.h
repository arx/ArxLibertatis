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

#ifndef ARX_WINDOW_WINDOW_H
#define ARX_WINDOW_WINDOW_H

#include <vector>
#include <string>

#include "math/Vector.h"

class Window {
	
public:
	
	Window();
	virtual ~Window();
	
	virtual bool initialize(const std::string & title, Vec2i size, bool fullscreen,
	                        unsigned depth = 0) = 0;
	
	/*!
	 * Enter fullscreen and set the given video mode.
	 * If all parameters are zero, the desktop mode is used.
	 */
	virtual void setFullscreenMode(Vec2i resolution, unsigned depth = 0) = 0;
	
	//! Exits fullscreen mode and sets the window size.
	virtual void setWindowSize(Vec2i size) = 0;
	
	virtual void tick() = 0;
	
	virtual void hide() = 0;

	//! Obtain the cursor position relative to this window.
	virtual Vec2i getCursorPosition() const = 0;
	
	class Listener {
		
	public:
		
		virtual ~Listener() { }
		
		virtual void onCreateWindow(const Window & window);
		virtual void onDestroyWindow(const Window & window);
		virtual bool onCloseWindow(const Window & window);
		virtual void onMoveWindow(const Window & window);
		virtual void onResizeWindow(const Window & window);
		virtual void onMinimizeWindow(const Window & window);
		virtual void onMaximizeWindow(const Window & window);
		virtual void onRestoreWindow(const Window & window);
		virtual void onToggleFullscreen(const Window & window);
		virtual void onWindowGotFocus(const Window & window);
		virtual void onWindowLostFocus(const Window & window);
		virtual void onPaintWindow(const Window & window);
		
	};
	
	void addListener(Listener * listener);
	void removeListener(Listener * listener);
	
	bool hasFocus() const {
		return hasFocus_ && !isMinimized_; // We treat minimized as not having focus
	}
	bool isMinimized() const { return isMinimized_; }
	bool isMaximized() const { return isMaximized_; }
	bool isVisible() const { return isVisible_; }
	
	const Vec2i & getSize() const { return size_; }
	unsigned getDepth() const { return depth_; }
	
	bool isFullScreen() const { return isFullscreen_; }
	
protected:
	
	bool onClose();
	void onDestroy();
	void onMove(s32 x, s32 y);
	void onResize(s32 width, s32 height);
	void onMinimize();
	void onMaximize();
	void onRestore();
	void onShow(bool show);
	void onToggleFullscreen();
	void onFocus(bool hasFocus);
	void onPaint();
	void onCreate();
	
	std::string title_; //!< Window title bar caption.
	Vec2i position_; //!< Screen position in pixels (relative to the upper left corner)
	Vec2i size_; //!< Size in pixels
	bool isMinimized_; //!< Is minimized ?
	bool isMaximized_; //!< Is maximized ?
	bool isVisible_; //!< Is visible ?
	bool isFullscreen_; //!< Is fullscreen ?
	bool hasFocus_; //!< Has focus ?
	unsigned depth_;
	
private:
	
	typedef std::vector<Listener *> Listeners;
	
	//! Listeners that will be notified of change in the window properties.
	Listeners listeners;
	
};

#endif // ARX_WINDOW_WINDOW_H
