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

#ifndef ARX_GRAPHICS_BASEGRAPHICSTYPES_H
#define ARX_GRAPHICS_BASEGRAPHICSTYPES_H

#include "math/Vector3.h"

struct EERIE_QUAT {
	float x;
	float y;
	float z;
	float w;
};

struct EERIEMATRIX {
	float _11, _12, _13, _14;
	float _21, _22, _23, _24;
	float _31, _32, _33, _34;
	float _41, _42, _43, _44;

	EERIEMATRIX() {
		_11=0, _12=0, _13=0, _14=0;
		_21=0, _22=0, _23=0, _24=0;
		_31=0, _32=0, _33=0, _34=0;
		_41=0, _42=0, _43=0, _44=0;
	}
};

struct EERIE_CYLINDER {
	Vec3f origin;
	float radius;
	float height;
};

struct EERIE_SPHERE {
	
	Vec3f origin;
	float radius;
	
	bool contains(const Vec3f & pos) const {
		return closerThan(pos, origin, radius);
	}
	
};

struct EERIE_3D_BBOX {
	Vec3f min;
	Vec3f max;
};

enum Material {
	MATERIAL_NONE,
	MATERIAL_WEAPON,
	MATERIAL_FLESH,
	MATERIAL_METAL,
	MATERIAL_GLASS,
	MATERIAL_CLOTH,
	MATERIAL_WOOD,
	MATERIAL_EARTH,
	MATERIAL_WATER,
	MATERIAL_ICE,
	MATERIAL_GRAVEL,
	MATERIAL_STONE,
	MATERIAL_FOOT_LARGE,
	MATERIAL_FOOT_BARE,
	MATERIAL_FOOT_SHOE,
	MATERIAL_FOOT_METAL,
	MATERIAL_FOOT_STEALTH
};

#endif // ARX_GRAPHICS_BASEGRAPHICSTYPES_H
