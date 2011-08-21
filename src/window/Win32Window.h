
#ifndef ARX_WINDOW_WIN32WINDOW_H
#define ARX_WINDOW_WIN32WINDOW_H

#include <map>
#include <windows.h>

#include "window/RenderWindow.h"

class Win32Window : public RenderWindow {
	
public:
	
	Win32Window();
	virtual ~Win32Window();

	virtual bool init(const std::string & title, Vec2i size, bool fullscreen, unsigned depth = 0);
	
	virtual void * GetHandle();
	virtual void setFullscreenMode(Vec2i resolution, unsigned depth = 0);
	virtual void setWindowSize(Vec2i size);
	virtual void Tick();
	
	virtual void hide();
	
private:
	
	static bool InitWindowClass();
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	
	HWND m_hWnd;
	WNDPROC m_HijackedWindowProc;
	
	static WNDCLASS m_WindowClass;
	static bool m_WindowClassInitialized;
	static std::map<HWND,Win32Window*> m_WindowsMap;
	
	DWORD dwSavedStyle;
	
};

#endif // ARX_WINDOW_WIN32WINDOW_H
