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

//TODO Remove
#define CAM_SUBJVIEW 0
#define CAM_TOPVIEW 1

struct EERIE_TRANSFORM {
	Vec3f pos;
	float ycos;
	float ysin;
	float xsin;
	float xcos;
	float zcos;
	float zsin;
	float use_focal;
	Vec2f mod;

	void updateFromAngle(const Anglef &angle) {
		float tmp;
		tmp = radians(angle.a);
		xcos = std::cos(tmp);
		xsin = std::sin(tmp);
		tmp = radians(angle.b);
		ycos = std::cos(tmp);
		ysin = std::sin(tmp);
		tmp = radians(angle.g);
		zcos = std::cos(tmp);
		zsin = std::sin(tmp);
	}
};

struct EERIE_CAMERA {

	EERIE_TRANSFORM orgTrans;

	float focal;

	Anglef angle;

	Vec3f d_pos;
	Anglef d_angle;
	Vec3f lasttarget;
	Vec3f lastpos;
	Vec3f translatetarget;
	bool lastinfovalid;
	Color3f fadecolor;
	Rect clip;
	Vec2i center;

	float smoothing;

	Color bkgcolor; // TODO was BGR!
	float cdepth;

	Anglef size;

	void setTargetCamera(const Vec3f &target) {
		setTargetCamera(target.x, target.y, target.z);
	}

	void setTargetCamera(float x, float y, float z)
	{
		if(orgTrans.pos.x == x && orgTrans.pos.y == y && orgTrans.pos.z == z)
			return;

		angle.a = (degrees(getAngle(orgTrans.pos.y, orgTrans.pos.z, y, orgTrans.pos.z + dist(Vec2f(x, z), Vec2f(orgTrans.pos.x, orgTrans.pos.z))))); //alpha entre orgn et dest;
		angle.b = (180.f + degrees(getAngle(orgTrans.pos.x, orgTrans.pos.z, x, z))); //beta entre orgn et dest;
		angle.g = 0.f;
	}
};


struct IO_CAMDATA {
	EERIE_CAMERA cam;
};

#endif // ARX_GAME_CAMERA_H
