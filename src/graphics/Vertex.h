/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
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
/* Based on:
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/

#ifndef ARX_GRAPHICS_VERTEX_H
#define ARX_GRAPHICS_VERTEX_H

#include "graphics/Color.h"
#include "math/Vector.h"
#include "platform/Platform.h"


struct TexturedVertexUntransformed {
	
	Vec3f p = Vec3f(0.f);
	ColorRGBA color;
	Vec2f uv = Vec2f(0.f);
	
	constexpr TexturedVertexUntransformed() arx_noexcept_default
	
	constexpr TexturedVertexUntransformed(const Vec3f & _p, ColorRGBA _color, Vec2f _uv) noexcept
		: p(_p)
		, color(_color)
		, uv(_uv)
	{ }
	
};

struct TexturedVertex {
	
	Vec3f p = Vec3f(0.f);
	float w = 1.f;
	
	ColorRGBA color;
	
	Vec2f uv = Vec2f(0.f);
	
	constexpr TexturedVertex() arx_noexcept_default
	
	constexpr TexturedVertex(const Vec3f & _p, float _w, ColorRGBA _color, Vec2f _uv) noexcept
		: p(_p)
		, w(_w)
		, color(_color)
		, uv(_uv)
	{ }
	
	constexpr TexturedVertex(const Vec4f & _p, ColorRGBA _color, Vec2f _uv) noexcept
		: p(_p)
		, w(_p.w)
		, color(_color)
		, uv(_uv)
	{ }
	
};

template <class Vertex>
class VertexBuffer;

struct SMY_VERTEX {
	
	Vec3f p = Vec3f(0.f);
	ColorRGBA color;
	Vec2f uv = Vec2f(0.f);
	
	constexpr SMY_VERTEX() arx_noexcept_default
	
};

struct SMY_VERTEX3 {
	
	Vec3f p = Vec3f(0.f);
	ColorRGBA color;
	Vec2f uv[3] = { Vec2f(0.f), Vec2f(0.f), Vec2f(0.f) };
	
	constexpr SMY_VERTEX3() arx_noexcept_default
	
};

struct EERIE_VERTEX {
	
	Vec3f v = Vec3f(0.f);
	Vec3f norm = Vec3f(0.f);
	
	constexpr EERIE_VERTEX() arx_noexcept_default
	
};

#endif // ARX_GRAPHICS_VERTEX_H
