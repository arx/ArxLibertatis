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

#ifndef ARX_GRAPHICS_BASEGRAPHICSTYPES_H
#define ARX_GRAPHICS_BASEGRAPHICSTYPES_H

#include "math/Rectangle.h"
#include "math/Vector.h"

struct Cylinder {
	Vec3f origin;
	float radius;
	float height;
	
	Cylinder()
		: origin(0.f)
		, radius(0.f)
		, height(0.f)
	{}
	
	Cylinder(const Vec3f & origin_, float radius_, float height_)
		: origin(origin_)
		, radius(radius_)
		, height(height_)
	{}
};

struct Sphere {
	
	Vec3f origin;
	float radius;
	
	Sphere()
		: origin(0.f)
		, radius(0.f)
	{}
	
	Sphere(const Vec3f & origin_, float radius_)
		: origin(origin_)
		, radius(radius_)
	{}
	
	bool contains(const Vec3f & pos) const {
		return closerThan(pos, origin, radius);
	}
	
};

struct EERIE_2D_BBOX {
	
	Vec2f min;
	Vec2f max;
	
	EERIE_2D_BBOX() : min(0.f), max(0.f) { }
	
	void reset() {
		min = Vec2f(32000);
		max = Vec2f(-32000);
	}
	
	void add(const Vec3f & pos) {
		min = glm::min(min, Vec2f(pos));
		max = glm::max(max, Vec2f(pos));
	}
	
	bool valid() const {
		return (min.x <= max.x && min.y <= max.y);
	}
	
	Rectf toRect() {
		return Rectf(min.x, min.y, max.x, max.y);
	}
	
};

struct EERIE_3D_BBOX {
	
	Vec3f min;
	Vec3f max;
	
	EERIE_3D_BBOX() : min(0.f), max(0.f) { }
	
	EERIE_3D_BBOX(const Vec3f & min_, const Vec3f & max_) : min(min_), max(max_) { }
	
	void reset() {
		min = Vec3f(99999999.f);
		max = Vec3f(-99999999.f);
	}
	
	void add(const Vec3f & pos) {
		min = glm::min(min, pos);
		max = glm::max(max, pos);
	}
	
	bool valid() const {
		return (min.x <= max.x && min.y <= max.y && min.z <= max.z);
	}
	
};

#endif // ARX_GRAPHICS_BASEGRAPHICSTYPES_H
