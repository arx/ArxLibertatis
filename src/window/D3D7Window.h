
#ifndef ARX_WINDOW_D3D7WINDOW_H
#define ARX_WINDOW_D3D7WINDOW_H

#include "window/Win32Window.h"
#include "window/RenderWindow.h"

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
		std::string driverDesc;
		GUID driverGUID;
		
		// For internal use (apps should not need to use these)
		std::vector<DDSURFACEDESC2> modes;
		
	};
	
	D3D7Window();
	~D3D7Window();
	
	bool initFramework();

	virtual bool init(const std::string & title, Vec2i size, bool fullscreen, unsigned depth = 0);
	
	bool showFrame();
	void restoreSurfaces();
	
	void evictManagedTextures();
	
	inline LPDIRECTDRAW7 getDD() { return dd; }
	inline LPDIRECT3D7 getD3D() { return d3d; }
	inline LPDIRECTDRAWSURFACE7 getFront() { return frontBuffer; }
	inline LPDIRECTDRAWSURFACE7 getBack() { return backBuffer; }
	inline LPDIRECT3DDEVICE7 getDevice() { return device; }
	inline const DeviceInfo & getInfo() { return *deviceInfo; }
	
	void setFullscreenMode(Vec2i resolution, unsigned depth = 0);
	void setWindowSize(Vec2i size);
	
	void setGammaRamp(const u16 * red, const u16 * green, const u16 * blue);
	
private:
	
	static BOOL WINAPI driverEnumCallback(GUID *, char *, char *, VOID *, HMONITOR);
	static HRESULT WINAPI modeEnumCallback(DDSURFACEDESC2 *, VOID *);
	static HRESULT WINAPI deviceEnumCallback(char *, char *, D3DDEVICEDESC7 *, VOID *);
	
	// Internal functions for the framework class
	bool initialize(DisplayMode mode);
	bool createZBuffer(GUID * device);
	bool createFullscreenBuffers(DisplayMode mode);
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
	
	LPDIRECTDRAWGAMMACONTROL gammaControl; // gamma control
	DDGAMMARAMP oldGamma; // backup gamma values
	
};

#endif // ARX_WINDOW_D3D7WINDOW_H
