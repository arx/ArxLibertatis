#include "core/Win32Window.h"
#include "core/Resource.h"

#include "io/Logger.h"

#include <windows.h>
#include <map>

WNDCLASS      Win32Window::m_WindowClass;
bool       Win32Window::m_WindowClassInitialized = false;
std::map<HWND,Win32Window*>  Win32Window::m_WindowsMap;

extern long FINAL_COMMERCIAL_DEMO;
extern long FINAL_COMMERCIAL_GAME;

Win32Window::Win32Window()
	: m_hWnd(NULL)
	, m_HijackedWindowProc(0) {
}

Win32Window::~Win32Window() {
}

bool Win32Window::InitWindowClass() {
	if( m_WindowClassInitialized )
		return true;

	memset( &m_WindowClass, 0, sizeof(m_WindowClass) );
	
	// Fill all the info for our window class.
	m_WindowClass.style   = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	m_WindowClass.lpfnWndProc = Win32Window::WindowProc;
	m_WindowClass.hInstance  = (HINSTANCE)GetModuleHandle(NULL);
	m_WindowClass.hCursor  = LoadCursor(NULL, IDC_ARROW);
	m_WindowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	m_WindowClass.lpszMenuName = MAKEINTRESOURCE(NULL);
	m_WindowClass.lpszClassName = "ARX_WINDOW_CLASS";
	m_WindowClass.hIcon   = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MAIN));

	// Register our window class.
	m_WindowClassInitialized = RegisterClass(&m_WindowClass) != 0;

	return m_WindowClassInitialized;
}

bool Win32Window::Init(const std::string& Title, int Width, int Height, bool bVisible, bool bFullscreen) {
	bool init;

	init = Window::Init(Title, Width, Height, bVisible, bFullscreen);
	if(!init)
		return false;

	init = InitWindowClass();
	if(!init)
		return false;

	DWORD windowStyle = WS_OVERLAPPEDWINDOW;       // Define Our Window Style
	DWORD windowExtendedStyle = WS_EX_APPWINDOW;      // Define The Window's Extended Style

	RECT rcWnd;

	SetRect(&rcWnd, 0, 0, m_Size.x, m_Size.y);
	init = AdjustWindowRectEx( &rcWnd,
								 windowStyle,
								 GetMenu(m_hWnd) != NULL,
								 windowExtendedStyle ) == TRUE;

	// Bound the window size to the desktop
	HWND hWndDesktop = GetDesktopWindow();
	RECT rcDesktop;
	GetWindowRect(hWndDesktop, &rcDesktop);
	LONG wndWidth = rcWnd.right - rcWnd.left;
	LONG wndHeight = rcWnd.bottom - rcWnd.top;
	LONG maxWidth = rcDesktop.right - rcDesktop.left;
	LONG maxHeight = rcDesktop.bottom - rcDesktop.top;
	wndWidth = std::min(wndWidth, maxWidth);
	wndHeight = std::min(wndHeight, maxHeight);

	// Create a window using our window class.
	m_hWnd = CreateWindowEx( windowExtendedStyle,
							m_WindowClass.lpszClassName,
							"",
							(m_IsFullscreen ?  (WS_POPUP | WS_VISIBLE) : WS_OVERLAPPEDWINDOW ),
							CW_USEDEFAULT,
							CW_USEDEFAULT,
							wndWidth,
							wndHeight,
							0,
							NULL,
							(HINSTANCE)GetModuleHandle(NULL),
							this );
	if(!m_hWnd) {
		LogError << "Couldn't create window";
		return false;
	}

	init = SetWindowText( m_hWnd, m_Title.c_str() ) == TRUE;
	if(!init)
		LogWarning << "Couldn't change the window's title";

	if( m_IsVisible )
		ShowWindow( m_hWnd, SW_SHOW );

	return true;
}

LRESULT CALLBACK Win32Window::WindowProc( HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam ) {
	static Win32Window* currentWindow = NULL;
	
	if( !currentWindow || currentWindow->m_hWnd != hWnd )
	{
		std::map<HWND,Win32Window*>::iterator it = m_WindowsMap.find(hWnd);
		if( it != m_WindowsMap.end() )
			currentWindow = it->second;
	}

	if( currentWindow == NULL && iMsg != WM_NCCREATE )
		return DefWindowProc(hWnd, iMsg, wParam, lParam);

	bool bProcessed = false;

	switch(iMsg)
	{
	// Sent prior to the WM_CREATE message when a window is first created.
	case WM_NCCREATE:
		currentWindow = (Win32Window*)((CREATESTRUCT*)lParam)->lpCreateParams;
		m_WindowsMap[hWnd] = currentWindow;
		return 1;

	// Sent when an application requests that a window be created.
	case WM_CREATE:
		currentWindow->OnCreate();
		bProcessed = true;
		break;

	// Paint the window's client area.
	case WM_PAINT:
		currentWindow->OnPaint();
		break;

	// Sent after a window has been moved.
	case WM_MOVE:
		currentWindow->OnMove( (short)LOWORD(lParam), (short)HIWORD(lParam) );
		bProcessed = true;
		break;

	// Sent to a window after its size has changed.
	case WM_SIZE:
	{
		switch( wParam )
		{
		case SIZE_MINIMIZED:
			currentWindow->OnMinimize();
			break;
		case SIZE_MAXIMIZED:
			currentWindow->OnMaximize();
			break;
		case SIZE_RESTORED:
			currentWindow->OnRestore();
			break;
		}

		currentWindow->OnResize( (short)LOWORD(lParam), (short)HIWORD(lParam) );
		bProcessed = true;
		break;
	}

	// Sent to both the window being activated and the window being deactivated.
	case WM_ACTIVATE:
		currentWindow->OnFocus( LOWORD(wParam) != WA_INACTIVE );
		bProcessed = true;
		break;

	// Sent when the window is about to be hidden or shown.
	case WM_SHOWWINDOW:
		currentWindow->OnShow( wParam == TRUE );
		bProcessed = true;
		break;

	// Sent just before a window is destroyed.
	// Informs a window that its nonclient area is being destroyed.
	// Window is about to be destroyed, clean up window-specific data objects.
	case WM_DESTROY:
		currentWindow->OnDestroy();
		bProcessed = true;
		break;

	// Sent as a signal that a window or an application should terminate.
	case WM_CLOSE:
		if( !currentWindow->OnClose() )
			bProcessed = true;
		break;

	// To avoid screensaver / monitorpower interference
	case WM_SYSCOMMAND:
		if ((wParam & 0xFFF0)== SC_SCREENSAVE || (wParam & 0xFFF0)== SC_MONITORPOWER)
			bProcessed = true;
		break;

	case WM_SETCURSOR:
		// Prevent a cursor in fullscreen mode
		if (currentWindow->HasFocus() && currentWindow->IsFullScreen())
		{
			SetCursor(NULL);
			return 1;
		}
		break;
	}

	// If the window proc was hijacked, always forward messages to the original WindowProc
	if( currentWindow->m_HijackedWindowProc != NULL )
	{
		return CallWindowProc(currentWindow->m_HijackedWindowProc, hWnd, iMsg, wParam, lParam);
	}
	else
	{
		// If processed, return 0, otherwise, send all the other messages to the default WindowProc.
		if( bProcessed )
			return 0;
		else
			return DefWindowProc(hWnd, iMsg, wParam, lParam);
	}
}

void Win32Window::Tick() {
	MSG msg;

	if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg ); // Send message to the WindowProc.
	}
}

void* Win32Window::GetHandle() {
	return m_hWnd;
}

void Win32Window::SetFullscreen(bool bFullscreen) {
	if(bFullscreen == m_IsFullscreen)
		return; // Nothing to do here!

	if (!bFullscreen)
	{
		// Coming from fullscreen mode, so restore window properties
		SetWindowLong(m_hWnd, GWL_STYLE, dwSavedStyle);
		SetWindowPos(m_hWnd, HWND_NOTOPMOST, rcSaved.left, rcSaved.top,
						(rcSaved.right - rcSaved.left),
						(rcSaved.bottom - rcSaved.top), SWP_SHOWWINDOW);
	}
	else
	{
		// Going to fullscreen mode, save/set window properties as needed
		dwSavedStyle = GetWindowLong(m_hWnd, GWL_STYLE);
		GetWindowRect(m_hWnd, &rcSaved);

		if (FINAL_COMMERCIAL_DEMO || FINAL_COMMERCIAL_GAME)
			SetWindowLong(m_hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
		else
			SetWindowLong(m_hWnd, GWL_STYLE, WS_POPUP | WS_SYSMENU | WS_VISIBLE);
	}

	m_IsFullscreen = bFullscreen;
}

void Win32Window::SetSize(Vec2i Size) {
	RECT rRect;
	RECT rRect2;
	GetClientRect(m_hWnd, &rRect);
	GetWindowRect(m_hWnd, &rRect2);
	int dx = (rRect2.right - rRect2.left) - (rRect.right - rRect.left);
	int dy = (rRect2.bottom - rRect2.top) - (rRect.bottom - rRect.top);
	SetWindowPos(m_hWnd,
					HWND_TOP,
					rRect2.left,
					rRect2.top,
					Size.x + dx,
					Size.y + dy,
					SWP_SHOWWINDOW);
}
