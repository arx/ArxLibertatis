/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#include <list>
#include <string>

#include "math/Vector2.h"

class Window {
	
public:
	
	Window();
	virtual ~Window();
	
	virtual bool init(const std::string & title, Vec2i size, bool fullscreen, unsigned depth = 0) = 0;
	
	/*!
	 * Enter fullscreen and set the given video mode.
	 * If all parameters are zero, the desktop mode is used.
	 */
	virtual void setFullscreenMode(Vec2i resolution, unsigned depth = 0) = 0;
	
	//! Exits fullscreen mode and sets the window size.
	virtual void setWindowSize(Vec2i size) = 0;
	
	virtual void * getHandle() = 0;
	virtual void tick() = 0;
	
	virtual void hide() = 0;
	
	class Listener {
		
	public:
		
		virtual ~Listener() { }
		
		virtual void onCreateWindow(const Window & pWindow);
		virtual void onDestroyWindow(const Window & pWindow);
		virtual bool onCloseWindow(const Window & pWindow);
		virtual void onMoveWindow(const Window & pWindow);
		virtual void onResizeWindow(const Window & pWindow);
		virtual void onMinimizeWindow(const Window & pWindow);
		virtual void onMaximizeWindow(const Window & pWindow);
		virtual void onRestoreWindow(const Window & pWindow);
		virtual void onToggleFullscreen(const Window & pWindow);
		virtual void onWindowGotFocus(const Window & pWindow);
		virtual void onWindowLostFocus(const Window & pWindow);
		virtual void onPaintWindow(const Window & pWindow);
		
	};
	
	void addWindowListener(Listener * pListener);
	void removeWindowListener(Listener * pListener);
	
	inline bool hasFocus() const {
		return m_HasFocus && !m_IsMinimized; // We treat minimized as not having focus
	}
	inline bool isMinimized() const { return m_IsMinimized; }
	inline bool isMaximized() const { return m_IsMaximized; }
	inline bool isVisible() const { return m_IsVisible; }
	
	inline const Vec2i & getSize() const { return m_Size; }
	inline unsigned getDepth() const { return depth; }
	
	inline bool isFullScreen() const { return m_IsFullscreen; }
	
protected:
	
	bool onClose();
	void onDestroy();
	void onMove(s32 pPosX, s32 pPosY);
	void onResize(s32 pWidth, s32 pHeight);
	void onMinimize();
	void onMaximize();
	void onRestore();
	void onShow(bool bShow);
	void onToggleFullscreen();
	void onFocus(bool bHasFocus);
	void onPaint();
	void onCreate();
	
	std::string m_Title; //!< Window title bar caption.
	Vec2i m_Position; //!< Screen position in pixels (relative to the upper left corner)
	Vec2i m_Size; //!< Size in pixels
	bool m_IsMinimized; //!< Is minimized ?
	bool m_IsMaximized; //!< Is maximized ?
	bool m_IsVisible; //!< Is visible ?
	bool m_IsFullscreen; //!< Is fullscreen ?
	bool m_HasFocus; //!< Has focus ?
	unsigned depth;
	
private:
	
	std::list<Listener*> m_Listeners; //! Listeners that will be notified of change in the window properties.
	
};

#endif // ARX_WINDOW_WINDOW_H
