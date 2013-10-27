/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GRAPHICS_GRAPHICSFORMAT_H
#define ARX_GRAPHICS_GRAPHICSFORMAT_H

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
	
	inline operator Color3f() const {
		Color3f a;
		a.r = r, a.g = g, a.b = b;
		return a;
	}
	
};

struct SavedVec3 {
	
	f32 x;
	f32 y;
	f32 z;
	
	inline Vec3f toVec3() const {
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
	
	inline operator Anglef() const {
		return Anglef(a, b, g);
	}
	
	inline SavedAnglef & operator=(const Anglef & o) {
		a = o.getYaw(), b = o.getPitch(), g = o.getRoll();
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
	
	inline operator TexturedVertex() const {
		TexturedVertex a;
		a.p.x = pos.x, a.p.y = pos.y, a.p.z = pos.z;
		a.rhw = rhw;
		a.color = color;
		a.specular = specular;
		a.uv.x = tu;
		a.uv.y = tv;
		return a;
	}
	
	inline SavedTextureVertex & operator=(const TexturedVertex & b) {
		pos.x = b.p.x, pos.y = b.p.y, pos.z = b.p.z;
		rhw = b.rhw;
		color = b.color;
		specular = b.specular;
		tu = b.uv.x;
		tv = b.uv.y;
		return *this;
	}
	
};


#pragma pack(pop)


#endif // ARX_GRAPHICS_GRAPHICSFORMAT_H
