/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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
	D3DGAMMARAMP initialGammaRamp;		// Gamma ramp to restore
};

#endif // ARX_WINDOW_D3D9WINDOW_H
