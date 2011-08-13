
#ifndef ARX_CORE_D3D7WINDOW_H
#define ARX_CORE_D3D7WINDOW_H

#include "core/Win32Window.h"
#include "core/RenderWindow.h"

#include <d3d.h>

class D3D7Window : public Win32Window {
	
public:
	
	struct DeviceInfo {
		
		// D3D Device info
		std::string name;
		std::string desc;
		D3DDEVICEDESC7 device;
		
		// DDraw Driver info
		std::string driver;
		GUID driverGUID;
		
		// For internal use (apps should not need to use these)
		std::vector<DDSURFACEDESC2> modes;
		
	};
	
	D3D7Window();
	~D3D7Window();

	bool Init(const std::string & Title, int Width, int Height, bool bVisible, bool bFullscreen);
	
	bool showFrame();
	void restoreSurfaces();
	
	void evictManagedTextures();
	
	inline LPDIRECTDRAW7 getDD() { return dd; }
	inline LPDIRECT3D7 getD3D() { return d3d; }
	inline LPDIRECTDRAWSURFACE7 getFront() { return frontBuffer; }
	inline LPDIRECTDRAWSURFACE7 getBack() { return backBuffer; }
	inline LPDIRECT3DDEVICE7 getDevice() { return device; }
	inline const DeviceInfo & getInfo() { return *deviceInfo; }
	
	void SetFullscreen(bool fullscreen);
	void SetSize(Vec2i size);
	
private:
	
	static BOOL WINAPI driverEnumCallback(GUID *, char *, char *, VOID *, HMONITOR);
	static HRESULT WINAPI modeEnumCallback(DDSURFACEDESC2 *, VOID *);
	static HRESULT WINAPI deviceEnumCallback(char *, char *, D3DDEVICEDESC7 *, VOID *);
	
	void enumerate();
	
	// Internal functions for the framework class
	bool initialize();
	bool createZBuffer(GUID * device);
	bool createFullscreenBuffers();
	bool createWindowedBuffers();
	bool createDirectDraw(GUID * driver);
	bool createDirect3D(GUID * device);
	void destroyObjects();
	
	LPDIRECTDRAW7 dd; // The DirectDraw object
	LPDIRECT3D7 d3d; // The Direct3D object
	LPDIRECTDRAWSURFACE7 backBuffer; // The backbuffer surface
	LPDIRECTDRAWSURFACE7 frontBuffer; // The primary surface
	LPDIRECT3DDEVICE7 device;
	
	std::vector<DeviceInfo> devices;
	DeviceInfo * deviceInfo;
	
};

#endif // ARX_CORE_D3D7WINDOW_H
