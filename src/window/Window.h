/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include <ostream>
#include <string>
#include <vector>

#include "math/Vector.h"

class InputBackend;
namespace fs { class path; }

struct DisplayMode {
	
	Vec2i resolution;
	
	DisplayMode() : resolution(0) { }
	/* implicit */ DisplayMode(Vec2i res) : resolution(res) { }
	bool operator<(const DisplayMode & o) const;
	bool operator==(const DisplayMode & o) const {
		return resolution == o.resolution;
	}
	
};

std::ostream & operator<<(std::ostream & os, const DisplayMode & mode);

class Window {
	
public:
	
	Window();
	virtual ~Window();
	
	/*!
	 * Set the window titlebar caption.
	 * May be called before or after \ref initialize()
	 */
	virtual void setTitle(const std::string & title) = 0;
	
	/*!
	 * Enter fullscreen and set the given video mode.
	 * If all parameters are zero, the desktop mode is used.
	 * May be called before or after \ref initialize()
	 */
	virtual void setFullscreenMode(const DisplayMode & mode) = 0;
	
	/*!
	 * Exits fullscreen mode and sets the window size.
	 * May be called before or after \ref initialize()
	 */
	virtual void setWindowSize(const Vec2i & size) = 0;
	
	virtual bool initialize() = 0;
	
	virtual void tick() = 0;
	
	virtual void hide() = 0;
	
	/*!
	 * Minimize the fullscreen windows if they lose focus.
	 * This setting may be overrriden by the environment,
	 * see \ref willMinimizeOnFocusLost().
	 */
	virtual void setMinimizeOnFocusLost(bool enabled) = 0;
	
	enum MinimizeSetting {
		Disabled,
		Enabled,
		AlwaysDisabled,
		AlwaysEnabled
	};
	
	/*!
	 * Return if fullscreen windows if they lose focus.
	 */
	virtual MinimizeSetting willMinimizeOnFocusLost() = 0;
	
	/*!
	 * Get the current text contents of the clipboard.
	 */
	virtual std::string getClipboardText() = 0;
	
	/*!
	 * Get the contents of the clipboard to the given text.
	 */
	virtual void setClipboardText(const std::string & text) = 0;
	
	/*!
	 * Set if the user's screensaver is allowed to start.
	 */
	virtual void allowScreensaver(bool allowed) = 0;
	
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
		virtual void onDroppedFile(const Window & window, const fs::path & path);
		
	};
	
	void addListener(Listener * listener);
	void removeListener(Listener * listener);
	
	bool hasFocus() const {
		return m_focused && !m_minimized; // We treat minimized as not having focus
	}
	bool isMinimized() const { return m_minimized; }
	bool isMaximized() const { return m_maximized; }
	bool isVisible() const { return m_visible; }
	
	const Vec2i & getPosition() const { return m_position; }
	const Vec2i & getSize() const { return m_size; }
	DisplayMode getDisplayMode() const { return DisplayMode(m_size); }
	
	bool isFullScreen() const { return m_fullscreen; }
	
	virtual InputBackend * getInputBackend() = 0;
	
protected:
	
	bool onClose();
	void onDestroy();
	void onMove(s32 x, s32 y);
	void onResize(const Vec2i & size);
	void onMinimize();
	void onMaximize();
	void onRestore();
	void onShow(bool isVisible);
	void onToggleFullscreen(bool fullscreen);
	void onFocus(bool hasFocus);
	void onPaint();
	void onCreate();
	void onDroppedFile(const fs::path & path);
	
	std::string m_title; //!< Window title bar caption.
	Vec2i m_position;    //!< Screen position in pixels (relative to the upper left corner)
	Vec2i m_size;        //!< Size in pixels
	bool m_minimized;    //!< Is minimized ?
	bool m_maximized;    //!< Is maximized ?
	bool m_visible;      //!< Is visible ?
	bool m_fullscreen;   //!< Is fullscreen ?
	bool m_focused;      //!< Has focus ?
	
private:
	
	typedef std::vector<Listener *> Listeners;
	
	//! Listeners that will be notified of change in the window properties.
	Listeners m_listeners;
	
};

#endif // ARX_WINDOW_WINDOW_H
