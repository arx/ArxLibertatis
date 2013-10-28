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
#include "platform/Platform.h"
#include "math/Vector.h"

struct TexturedVertex {
	
	Vec3f p;
	float rhw;
	
	ColorBGRA color;
	ColorBGRA specular;
	
	Vec2f uv;
	
	TexturedVertex() {}

	TexturedVertex(const TexturedVertex & o)
		: p(o.p)
		, rhw(o.rhw)
		, color(o.color)
		, specular(o.specular)
		, uv(o.uv)
	{}

	TexturedVertex(const Vec3f & _p, float _rhw, ColorBGRA _color, ColorBGRA _specular, Vec2f _uv)
		: p(_p)
		, rhw(_rhw)
		, color(_color)
		, specular(_specular)
		, uv(_uv)
	{}
};

template <class Vertex>
class VertexBuffer;

struct SMY_VERTEX {
	Vec3f p;
	ColorBGRA color;
	Vec2f uv;
};

struct SMY_VERTEX3 {
	Vec3f p;
	ColorBGRA color;
	Vec2f uv[3];
};

struct EERIE_VERTEX {
	TexturedVertex vert;
	Vec3f v;
	Vec3f norm;
};

#endif // ARX_GRAPHICS_VERTEX_H
