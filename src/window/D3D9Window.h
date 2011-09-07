
#ifndef ARX_WINDOW_D3D9WINDOW_H
#define ARX_WINDOW_D3D9WINDOW_H

#include "window/Win32Window.h"
#include "window/RenderWindow.h"

#include <d3d9.h>

class D3D9Window : public Win32Window {
	
public:
	
	struct DeviceInfo {
		UINT					adapterIdx;
		D3DADAPTER_IDENTIFIER9  adapterIdentifier;
	};
	
	D3D9Window();
	~D3D9Window();
	
	bool initFramework();

	virtual bool init(const std::string & title, Vec2i size, bool fullscreen, unsigned depth = 0);
	
	bool showFrame();
	void restoreSurfaces();
	
	void evictManagedTextures();
	
	inline LPDIRECT3D9 getD3D() { return d3d; }
	inline const DeviceInfo & getInfo() { return *deviceInfo; }
	
	void setFullscreenMode(Vec2i resolution, unsigned depth = 0);
	void setWindowSize(Vec2i size);
	void restoreObjects();

	void setGammaRamp(const u16 * red, const u16 * green, const u16 * blue);
	
private:
	
	// Internal functions for the framework class
	bool initialize(DisplayMode mode);
	void destroyObjects();

private:
	std::vector<DeviceInfo> devices;
	DeviceInfo * deviceInfo;

	LPDIRECT3D9 d3d;					// The Direct3D object
};

#endif // ARX_WINDOW_D3D9WINDOW_H
