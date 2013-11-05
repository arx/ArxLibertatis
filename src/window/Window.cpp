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

#include "window/Window.h"

#include <algorithm>

#include <boost/foreach.hpp>

void Window::Listener::onCreateWindow( const Window & /*window*/ ) {}
bool Window::Listener::onCloseWindow( const Window & /*window*/ ) { return true; }
void Window::Listener::onDestroyWindow( const Window & /*window*/ ) {}
void Window::Listener::onMoveWindow( const Window & /*window*/ ) {}
void Window::Listener::onResizeWindow( const Window & /*window*/ ) {}
void Window::Listener::onMinimizeWindow( const Window & /*window*/ ) {}
void Window::Listener::onMaximizeWindow( const Window & /*window*/ ) {}
void Window::Listener::onRestoreWindow( const Window & /*window*/ ) {}
void Window::Listener::onToggleFullscreen( const Window & /*window*/ ) {}
void Window::Listener::onWindowGotFocus( const Window & /*window*/ ) {}
void Window::Listener::onWindowLostFocus( const Window & /*window*/ ) {}
void Window::Listener::onPaintWindow( const Window & /*window*/ ) {}

Window::Window()
	: m_position(0, 0)
	, m_size(640, 480)
	, isMinimized_(false)
	, isMaximized_(false)
	, isVisible_(false)
	, isFullscreen_(false)
	, hasFocus_(false) { }

Window::~Window() { }

void Window::addListener(Listener * listener) {
	listeners.push_back(listener);
}

void Window::removeListener(Listener * listener) {
	Listeners::iterator it = std::find(listeners.begin(), listeners.end(), listener);
	if(it != listeners.end()) {
		listeners.erase(it);
	}
}

bool Window::onClose() {
	BOOST_FOREACH(Listener * listener, listeners) {
		bool shouldClose = listener->onCloseWindow(*this);
		if(!shouldClose) {
			return false;
		}
	}
	return true;
}

void Window::onCreate() {
	BOOST_FOREACH(Listener * listener, listeners) {
		listener->onCreateWindow(*this);
	}
}

void Window::onDestroy() {
	BOOST_FOREACH(Listener * listener, listeners) {
		listener->onDestroyWindow(*this);
	}
}

void Window::onMove(s32 x, s32 y) {
	m_position = Vec2i(x, y);
	BOOST_FOREACH(Listener * listener, listeners) {
		listener->onMoveWindow(*this);
	}
}

void Window::onResize(const Vec2i & size) {
	m_size = size;
	BOOST_FOREACH(Listener * listener, listeners) {
		listener->onResizeWindow(*this);
	}
}

void Window::onMinimize() {
	isMinimized_ = true, isMaximized_ = false;
	BOOST_FOREACH(Listener * listener, listeners) {
		listener->onMinimizeWindow(*this);
	}
}
	
void Window::onMaximize() {
	isMinimized_ = false, isMaximized_ = true;
	BOOST_FOREACH(Listener * listener, listeners) {
		listener->onMaximizeWindow(*this);
	}
}

void Window::onRestore() {
	isMinimized_ = false, isMaximized_ = false;
	BOOST_FOREACH(Listener * listener, listeners) {
		listener->onRestoreWindow(*this);
	}
}

void Window::onShow(bool isVisible) {
	isVisible_ = isVisible;
}
	
void Window::onToggleFullscreen() {
	BOOST_FOREACH(Listener * listener, listeners) {
		listener->onToggleFullscreen(*this);
	}
}
	
void Window::onFocus(bool hasFocus) {
	hasFocus_ = hasFocus;
	if(hasFocus) {
		BOOST_FOREACH(Listener * listener, listeners) {
			listener->onWindowGotFocus(*this);
		}
	} else {
		BOOST_FOREACH(Listener * listener, listeners) {
			listener->onWindowLostFocus(*this);
		}
	}
}

void Window::onPaint() {
	BOOST_FOREACH(Listener * listener, listeners) {
		listener->onPaintWindow(*this);
	}
}
