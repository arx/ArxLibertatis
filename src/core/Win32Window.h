#ifndef	 ARX_CORE_WIN32WINDOW_H
#define	 ARX_CORE_WIN32WINDOW_H

#include "core/Window.h"

#include <map>
#include <windows.h>

class Win32Window :	public Window
{
public:
	Win32Window();
	virtual	~Win32Window();

	virtual	bool Init(const	std::string& Title,	int	Width, int Height, bool	bVisible, bool bFullscreen);
	virtual	void* GetHandle();
	virtual	void SetFullscreen(bool	bFullscreen);
	virtual	void SetSize(Vec2i Size);
	virtual	void Tick();

private:
	static bool	InitWindowClass();
	static LRESULT CALLBACK	WindowProc(	HWND hWnd, UINT	iMsg, WPARAM wParam, LPARAM	lParam );

private:
	HWND								m_hWnd;
	WNDPROC								m_HijackedWindowProc;

	static WNDCLASS						m_WindowClass;
	static bool							m_WindowClassInitialized;
	static std::map<HWND,Win32Window*>	m_WindowsMap;

	DWORD dwSavedStyle;
	RECT  rcSaved;
};

#endif // ARX_CORE_WIN32WINDOW_H
