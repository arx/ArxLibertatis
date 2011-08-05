#ifndef	 ARX_CORE_WINDOW_H
#define	 ARX_CORE_WINDOW_H

#include <list>
#include <string>

#include "platform/Flags.h"
#include "math/Vector2.h"

class Window
{
public:
	Window();
	virtual	~Window();

	virtual	bool Init(const	std::string& Title,	int	Width, int Height, bool	bVisible, bool bFullscreen);
	virtual	void SetFullscreen(bool	bFullscreen) = 0;
	virtual	void SetSize(Vec2i Size) = 0;
	virtual	void* GetHandle() =	0;
	virtual	void Tick()	= 0;

	class Listener
	{
	public:
		virtual	void OnCreateWindow(const Window& pWindow);
		virtual	void OnDestroyWindow(const Window& pWindow);
		virtual	bool OnCloseWindow(const Window& pWindow);
		virtual	void OnMoveWindow(const	Window&	pWindow);
		virtual	void OnResizeWindow(const Window& pWindow);
		virtual	void OnMinimizeWindow(const	Window&	pWindow);
		virtual	void OnMaximizeWindow(const	Window&	pWindow);
		virtual	void OnRestoreWindow(const Window& pWindow);
		virtual	void OnMakeWindowFullscreen(const Window& pWindow);
		virtual	void OnWindowGotFocus(const	Window&	pWindow);
		virtual	void OnWindowLostFocus(const Window& pWindow);
		virtual	void OnPaintWindow(const Window& pWindow);
	};

	void AddListener(Listener* pListener);
	void RemoveListener(Listener* pListener);
	
	bool HasFocus()	const;
	bool IsMinimized() const;
	bool IsMaximized() const;
	bool IsVisible() const;

	const Vec2i& GetSize() const;
	bool IsWindowed() const;
	bool IsFullScreen()	const;
		
protected:
	bool OnClose();	   
	void OnDestroy();
	void OnMove(s32	pPosX, s32 pPosY);
	void OnResize(s32 pWidth, s32 pHeight);
	void OnMinimize();	  
	void OnMaximize();
	void OnRestore();
	void OnShow(bool bShow);
	void OnMakeFullscreen();	
	void OnFocus(bool bHasFocus);
	void OnPaint();
	void OnCreate();

protected:
	std::string		m_Title;			//!< Window	title bar caption.
	Vec2i			m_Position;			//!< Screen	position in	pixels (relative to	the	upper left corner)
	Vec2i			m_Size;				//!< Size in pixels
	bool			m_IsMinimized;		//!< Is	minimized ?
	bool			m_IsMaximized;		//!< Is	maximized ?	   
	bool			m_IsVisible;		//!< Is	visible	?
	bool			m_IsFullscreen;		//!< Is	fullscreen ?
	bool			m_HasFocus;			//!< Has focus ?
		
private:
	std::list<Listener*> m_Listeners;	//!	Listeners that will	be notified	of change in the window	properties.
};

#endif // ARX_CORE_APPLICATION_H
