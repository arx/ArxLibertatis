
#ifndef ARX_GRAHPHICS_GRAPHICSFORMAT_H
#define ARX_GRAHPHICS_GRAPHICSFORMAT_H

#include "graphics/GraphicsTypes.h"


#pragma pack(push,1)


struct SavedColor {
	
	f32 r;
	f32 g;
	f32 b;
	
	inline SavedColor & operator=(const EERIE_RGB & o) {
		r = o.r, g = o.g, b = o.b;
		return *this;
	}
	
	inline operator EERIE_RGB() {
		EERIE_RGB a;
		a.r = r, a.g = g, a.b = b;
		return a;
	}
	
};

struct SavedVec3 {
	
	f32 x;
	f32 y;
	f32 z;
	
	inline operator EERIE_3D() {
		EERIE_3D a;
		a.x = x, a.y = y, a.z = z;
		return a;
	}
	
	inline SavedVec3 & operator=(const EERIE_3D & b) {
		x = b.x, y = b.y, z = b.z;
		return *this;
	}
	
};


#pragma pack(pop)


#endif // ARX_GRAHPHICS_GRAPHICSFORMAT_H
