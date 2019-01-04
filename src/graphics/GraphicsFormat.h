/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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


#pragma pack(push, 1)


struct SavedColor {
	
	f32 r;
	f32 g;
	f32 b;
	
	SavedColor & operator=(const Color3f & o) {
		r = o.r, g = o.g, b = o.b;
		return *this;
	}
	
	operator Color3f() const {
		Color3f a;
		a.r = r, a.g = g, a.b = b;
		return a;
	}
	
};

struct SavedVec3 {
	
	f32 x;
	f32 y;
	f32 z;
	
	Vec3f toVec3() const {
		return Vec3f(x, y, z);
	}
	
	SavedVec3 & operator=(const Vec3f & b) {
		x = b.x, y = b.y, z = b.z;
		return *this;
	}
	
};

struct SavedAnglef {
	
	f32 a;
	f32 b;
	f32 g;
	
	operator Anglef() const {
		return Anglef(a, b, g);
	}
	
	SavedAnglef & operator=(const Anglef & o) {
		a = o.getPitch(), b = o.getYaw(), g = o.getRoll();
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
	
};


#pragma pack(pop)


#endif // ARX_GRAPHICS_GRAPHICSFORMAT_H
