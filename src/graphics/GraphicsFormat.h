
#ifndef ARX_GRAHPHICS_GRAPHICSFORMAT_H
#define ARX_GRAHPHICS_GRAPHICSFORMAT_H

#include "graphics/GraphicsTypes.h"


#pragma pack(push,1)


struct SavedColor {
	
	f32 r;
	f32 g;
	f32 b;
	
	inline SavedColor & operator=(const Color3f & o) {
		r = o.r, g = o.g, b = o.b;
		return *this;
	}
	
	inline operator Color3f() {
		Color3f a;
		a.r = r, a.g = g, a.b = b;
		return a;
	}
	
};

struct SavedVec3 {
	
	f32 x;
	f32 y;
	f32 z;
	
	inline operator Vec3f() {
		Vec3f a;
		a.x = x, a.y = y, a.z = z;
		return a;
	}
	
	inline SavedVec3 & operator=(const Vec3f & b) {
		x = b.x, y = b.y, z = b.z;
		return *this;
	}
	
};

struct SavedAnglef {
	
	f32 a;
	f32 b;
	f32 g;
	
	inline operator Anglef() {
		Anglef r;
		r.a = a, r.b = b, r.g  =g;
		return r;
	}
	
	inline SavedAnglef & operator=(const Anglef & o) {
		a = o.a, b = o.b, g = o.g;
		return *this;
	}
	
};

struct SavedTextureVertex {
	
	SavedVec3 pos;
	f32 rhw;
	u32 color;
	u32 specular;
	f32 tu;
	f32 tv;
	
	inline operator TexturedVertex() {
		TexturedVertex a;
		a.sx = pos.x, a.sy = pos.y, a.sz = pos.z;
		a.rhw = rhw;
		a.color = color;
		a.specular = specular;
		a.tu = tu;
		a.tv = tv;
		return a;
	}
	
	inline SavedTextureVertex & operator=(const TexturedVertex & b) {
		pos.x = b.sx, pos.y = b.sy, pos.z = b.sz;
		rhw = b.rhw;
		color = b.color;
		specular = b.specular;
		tu = b.tu;
		tv = b.tv;
		return *this;
	}
	
};


#pragma pack(pop)


#endif // ARX_GRAHPHICS_GRAPHICSFORMAT_H
