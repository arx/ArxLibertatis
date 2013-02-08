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

#ifndef ARX_GAME_CAMERA_H
#define ARX_GAME_CAMERA_H

#include "math/Rectangle.h"
#include "graphics/GraphicsTypes.h"

struct EERIE_TRANSFORM {
	Vec3f pos;
	float ycos;
	float ysin;
	float xsin;
	float xcos;
	float use_focal;
	Vec2f mod;
};

struct EERIE_CAMERA {

	EERIE_TRANSFORM transform;
	Vec3f pos;
	float Ycos;
	float Ysin;
	float Xcos;
	float Xsin;
	float Zcos;
	float Zsin;
	float focal;
	float use_focal;
	float Zmul;
	Vec2f pos2;

	EERIEMATRIX matrix;
	Anglef angle;

	Vec3f d_pos;
	Anglef d_angle;
	Vec3f lasttarget;
	Vec3f lastpos;
	Vec3f translatetarget;
	bool lastinfovalid;
	Vec3f norm;
	Color3f fadecolor;
	Rect clip;
	float clipz0;
	float clipz1;
	Vec2i center;

	float smoothing;
	long Xsnap;
	long Zsnap;
	float Zdiv;

	long clip3D;
	long type;
	Color bkgcolor; // TODO was BGR!
	long nbdrawn;
	float cdepth;

	Anglef size;

};


struct IO_CAMDATA {
	EERIE_CAMERA cam;
};

#endif // ARX_GAME_CAMERA_H
