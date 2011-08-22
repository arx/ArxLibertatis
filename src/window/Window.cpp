
#include "window/Window.h"

#include <algorithm>

void Window::Listener::OnCreateWindow( const Window& /*pWindow*/ ) {}
bool Window::Listener::OnCloseWindow( const Window& /*pWindow*/ ) { return true; }
void Window::Listener::OnDestroyWindow( const Window& /*pWindow*/ ) {}
void Window::Listener::OnMoveWindow( const Window& /*pWindow*/ ) {}
void Window::Listener::OnResizeWindow( const Window& /*pWindow*/ ) {}
void Window::Listener::OnMinimizeWindow( const Window& /*pWindow*/ ) {}
void Window::Listener::OnMaximizeWindow( const Window& /*pWindow*/ ) {}
void Window::Listener::OnRestoreWindow( const Window& /*pWindow*/ ) {}
void Window::Listener::OnToggleFullscreen( const Window& /*pWindow*/ ) {}
void Window::Listener::OnWindowGotFocus( const Window& /*pWindow*/ ) {}
void Window::Listener::OnWindowLostFocus( const Window& /*pWindow*/ ) {}
void Window::Listener::OnPaintWindow( const Window& /*pWindow*/ ) {}

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

void Window::AddListener( Window::Listener* pListener ) {
	m_Listeners.push_back(pListener);
}

void Window::RemoveListener( Window::Listener* pListener ) {
	std::list<Listener*>::iterator it = std::find( m_Listeners.begin(), m_Listeners.end(), pListener );

	if( it != m_Listeners.end() )
		m_Listeners.erase(it);
}

bool Window::OnClose() {
	bool bShouldClose = true;
	for( std::list<Listener*>::iterator it = m_Listeners.begin(); it != m_Listeners.end() && bShouldClose; ++it )
		bShouldClose = (*it)->OnCloseWindow( *this );

	return bShouldClose;
}
	
void Window::OnCreate() {
	for( std::list<Listener*>::iterator it = m_Listeners.begin(); it != m_Listeners.end(); ++it )
		(*it)->OnCreateWindow( *this );
}

void Window::OnDestroy() {
	for( std::list<Listener*>::iterator it = m_Listeners.begin(); it != m_Listeners.end(); ++it )
		(*it)->OnDestroyWindow( *this );
}

void Window::OnMove( s32 pPosX, s32 pPosY ) {
	m_Position.x = pPosX;
	m_Position.y = pPosY;

	for( std::list<Listener*>::iterator it = m_Listeners.begin(); it != m_Listeners.end(); ++it )
		(*it)->OnMoveWindow( *this );
}

void Window::OnResize( s32 pWidth, s32 pHeight ) {
	m_Size.x = pWidth;
	m_Size.y = pHeight;

	for( std::list<Listener*>::iterator it = m_Listeners.begin(); it != m_Listeners.end(); ++it )
		(*it)->OnResizeWindow( *this );
}

void Window::OnMinimize() {
	m_IsMinimized = true;
	m_IsMaximized = false;
	
	for( std::list<Listener*>::iterator it = m_Listeners.begin(); it != m_Listeners.end(); ++it )
		(*it)->OnMinimizeWindow( *this );
}
	
void Window::OnMaximize() {
	m_IsMinimized = false;
	m_IsMaximized = true;
	
	for( std::list<Listener*>::iterator it = m_Listeners.begin(); it != m_Listeners.end(); ++it )
		(*it)->OnMaximizeWindow( *this );
}

void Window::OnRestore() {
	m_IsMinimized = false;
	m_IsMaximized = false;
	
	for( std::list<Listener*>::iterator it = m_Listeners.begin(); it != m_Listeners.end(); ++it )
		(*it)->OnRestoreWindow( *this );
}

void Window::OnShow( bool bVisible ) {
	m_IsVisible = bVisible;
}
	
void Window::OnToggleFullscreen() {
	for( std::list<Listener*>::iterator it = m_Listeners.begin(); it != m_Listeners.end(); ++it )
		(*it)->OnToggleFullscreen( *this );
}
	
void Window::OnFocus(bool bHasFocus) {
	m_HasFocus = bHasFocus;
	
	if(bHasFocus) {
		for( std::list<Listener*>::iterator it = m_Listeners.begin(); it != m_Listeners.end(); ++it )
			(*it)->OnWindowGotFocus( *this );
	} else {
		for( std::list<Listener*>::iterator it = m_Listeners.begin(); it != m_Listeners.end(); ++it )
			(*it)->OnWindowLostFocus( *this );
	}
}

void Window::OnPaint() {
	for( std::list<Listener*>::iterator it = m_Listeners.begin(); it != m_Listeners.end(); ++it )
		(*it)->OnPaintWindow( *this );
}
