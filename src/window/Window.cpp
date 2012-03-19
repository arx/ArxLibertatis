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

#include "window/Window.h"

#include <algorithm>

void Window::Listener::onCreateWindow( const Window& /*pWindow*/ ) {}
bool Window::Listener::onCloseWindow( const Window& /*pWindow*/ ) { return true; }
void Window::Listener::onDestroyWindow( const Window& /*pWindow*/ ) {}
void Window::Listener::onMoveWindow( const Window& /*pWindow*/ ) {}
void Window::Listener::onResizeWindow( const Window& /*pWindow*/ ) {}
void Window::Listener::onMinimizeWindow( const Window& /*pWindow*/ ) {}
void Window::Listener::onMaximizeWindow( const Window& /*pWindow*/ ) {}
void Window::Listener::onRestoreWindow( const Window& /*pWindow*/ ) {}
void Window::Listener::onToggleFullscreen( const Window& /*pWindow*/ ) {}
void Window::Listener::onWindowGotFocus( const Window& /*pWindow*/ ) {}
void Window::Listener::onWindowLostFocus( const Window& /*pWindow*/ ) {}
void Window::Listener::onPaintWindow( const Window& /*pWindow*/ ) {}

Window::Window()
	: m_Position(0, 0)
	, m_Size(640, 480)
	, m_IsMinimized(false)
	, m_IsMaximized(false)
	, m_IsVisible(false)
	, m_IsFullscreen(false)
	, m_HasFocus(false) {
}

Window::~Window() {
}

void Window::addWindowListener( Window::Listener* pListener ) {
	m_Listeners.push_back(pListener);
}

void Window::removeWindowListener( Window::Listener* pListener ) {
	std::list<Listener*>::iterator it = std::find( m_Listeners.begin(), m_Listeners.end(), pListener );

	if( it != m_Listeners.end() )
		m_Listeners.erase(it);
}

bool Window::onClose() {
	bool bShouldClose = true;
	for( std::list<Listener*>::iterator it = m_Listeners.begin(); it != m_Listeners.end() && bShouldClose; ++it )
		bShouldClose = (*it)->onCloseWindow( *this );

	return bShouldClose;
}
	
void Window::onCreate() {
	for( std::list<Listener*>::iterator it = m_Listeners.begin(); it != m_Listeners.end(); ++it )
		(*it)->onCreateWindow( *this );
}

void Window::onDestroy() {
	for( std::list<Listener*>::iterator it = m_Listeners.begin(); it != m_Listeners.end(); ++it )
		(*it)->onDestroyWindow( *this );
}

void Window::onMove( s32 pPosX, s32 pPosY ) {
	m_Position.x = pPosX;
	m_Position.y = pPosY;

	for( std::list<Listener*>::iterator it = m_Listeners.begin(); it != m_Listeners.end(); ++it )
		(*it)->onMoveWindow( *this );
}

void Window::onResize( s32 pWidth, s32 pHeight ) {
	m_Size.x = pWidth;
	m_Size.y = pHeight;

	for( std::list<Listener*>::iterator it = m_Listeners.begin(); it != m_Listeners.end(); ++it )
		(*it)->onResizeWindow( *this );
}

void Window::onMinimize() {
	m_IsMinimized = true;
	m_IsMaximized = false;
	
	for( std::list<Listener*>::iterator it = m_Listeners.begin(); it != m_Listeners.end(); ++it )
		(*it)->onMinimizeWindow( *this );
}
	
void Window::onMaximize() {
	m_IsMinimized = false;
	m_IsMaximized = true;
	
	for( std::list<Listener*>::iterator it = m_Listeners.begin(); it != m_Listeners.end(); ++it )
		(*it)->onMaximizeWindow( *this );
}

void Window::onRestore() {
	m_IsMinimized = false;
	m_IsMaximized = false;
	
	for( std::list<Listener*>::iterator it = m_Listeners.begin(); it != m_Listeners.end(); ++it )
		(*it)->onRestoreWindow( *this );
}

void Window::onShow( bool bVisible ) {
	m_IsVisible = bVisible;
}
	
void Window::onToggleFullscreen() {
	for( std::list<Listener*>::iterator it = m_Listeners.begin(); it != m_Listeners.end(); ++it )
		(*it)->onToggleFullscreen( *this );
}
	
void Window::onFocus(bool bHasFocus) {
	m_HasFocus = bHasFocus;
	
	if(bHasFocus) {
		for( std::list<Listener*>::iterator it = m_Listeners.begin(); it != m_Listeners.end(); ++it )
			(*it)->onWindowGotFocus( *this );
	} else {
		for( std::list<Listener*>::iterator it = m_Listeners.begin(); it != m_Listeners.end(); ++it )
			(*it)->onWindowLostFocus( *this );
	}
}

void Window::onPaint() {
	for( std::list<Listener*>::iterator it = m_Listeners.begin(); it != m_Listeners.end(); ++it )
		(*it)->onPaintWindow( *this );
}
