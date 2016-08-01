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

#include "graphics/GraphicsTypes.h"
#include "math/Rectangle.h"
#include "platform/Alignment.h"

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
	Vec2f mod;

	glm::mat4x4 worldToView;

	void updateFromAngle(const Anglef &angle);
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
	
	glm::mat4x4 ProjectionMatrix;

	void setTargetCamera(const Vec3f &target) {
		setTargetCamera(target.x, target.y, target.z);
	}

	void setTargetCamera(float x, float y, float z)
	{
		if(orgTrans.pos.x == x && orgTrans.pos.y == y && orgTrans.pos.z == z)
			return;

		angle.setPitch((glm::degrees(getAngle(orgTrans.pos.y, orgTrans.pos.z, y, orgTrans.pos.z + glm::distance(Vec2f(x, z), Vec2f(orgTrans.pos.x, orgTrans.pos.z)))))); //alpha entre orgn et dest;
		angle.setYaw((180.f + glm::degrees(getAngle(orgTrans.pos.x, orgTrans.pos.z, x, z)))); //beta entre orgn et dest;
		angle.setRoll(0.f);
	}
	
	ARX_USE_ALIGNED_NEW(EERIE_CAMERA) // for ProjectionMatrix
};


struct IO_CAMDATA {
	EERIE_CAMERA cam;
	ARX_USE_ALIGNED_NEW(IO_CAMDATA) // for cam
};


struct MASTER_CAMERA_STRUCT {
	long exist; // 2== want to change to want_vars...
	Entity * io;
	Entity * want_io;
};

extern MASTER_CAMERA_STRUCT MasterCamera;

void SP_PrepareCamera(EERIE_CAMERA * cam);
void PrepareCamera(EERIE_CAMERA *cam, const Rect & size);

extern EERIE_CAMERA * ACTIVECAM;
void SetActiveCamera(EERIE_CAMERA* cam);

ARX_USE_ALIGNED_ALLOCATOR(EERIE_CAMERA) // for ProjectionMatrix
ARX_USE_ALIGNED_ALLOCATOR(IO_CAMDATA) // for cam

#endif // ARX_GAME_CAMERA_H
