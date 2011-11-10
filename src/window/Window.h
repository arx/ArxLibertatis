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
	
	virtual void * GetHandle() = 0;
	virtual void Tick() = 0;
	
	virtual void hide() = 0;
	
	class Listener {
		
	public:
		
		virtual ~Listener() { }
		
		virtual void OnCreateWindow(const Window & pWindow);
		virtual void OnDestroyWindow(const Window & pWindow);
		virtual bool OnCloseWindow(const Window & pWindow);
		virtual void OnMoveWindow(const Window & pWindow);
		virtual void OnResizeWindow(const Window & pWindow);
		virtual void OnMinimizeWindow(const Window & pWindow);
		virtual void OnMaximizeWindow(const Window & pWindow);
		virtual void OnRestoreWindow(const Window & pWindow);
		virtual void OnToggleFullscreen(const Window & pWindow);
		virtual void OnWindowGotFocus(const Window & pWindow);
		virtual void OnWindowLostFocus(const Window & pWindow);
		virtual void OnPaintWindow(const Window & pWindow);
		
	};
	
	void AddListener(Listener * pListener);
	void RemoveListener(Listener * pListener);
	
	inline bool HasFocus() const {
		return m_HasFocus && !m_IsMinimized; // We treat minimized as not having focus
	}
	inline bool IsMinimized() const { return m_IsMinimized; }
	inline bool IsMaximized() const { return m_IsMaximized; }
	inline bool IsVisible() const { return m_IsVisible; }
	
	inline const Vec2i & GetSize() const { return m_Size; }
	inline unsigned getDepth() const { return depth; }
	
	inline bool IsFullScreen() const { return m_IsFullscreen; }
	
protected:
	
	bool OnClose();
	void OnDestroy();
	void OnMove(s32 pPosX, s32 pPosY);
	void OnResize(s32 pWidth, s32 pHeight);
	void OnMinimize();
	void OnMaximize();
	void OnRestore();
	void OnShow(bool bShow);
	void OnToggleFullscreen();
	void OnFocus(bool bHasFocus);
	void OnPaint();
	void OnCreate();
	
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
