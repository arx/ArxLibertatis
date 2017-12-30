/*
 * Copyright 2011-2016 Arx Libertatis Team (see the AUTHORS file)
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

struct Camera {
	
	Vec3f m_pos;
	float focal;
	Anglef angle;
	float cdepth;
	
	void lookAt(const Vec3f & target) {
		if(m_pos != target) {
			angle = getLookAtAngle(m_pos, target);
		}
	}
	
	static Anglef getLookAtAngle(const Vec3f & origin, const Vec3f & target);
	
};


struct IO_CAMDATA {
	
	Camera cam;
	
	Vec3f lasttarget;
	Vec3f translatetarget;
	bool lastinfovalid;
	float smoothing;
	
};

struct MASTER_CAMERA_STRUCT {
	
	long exist; // 2== want to change to want_vars...
	Entity * io;
	Entity * want_io;
	
};

extern MASTER_CAMERA_STRUCT MasterCamera;

struct PreparedCamera {
	
	glm::mat4x4 m_worldToView;
	glm::mat4x4 m_viewToScreen;
	
	ARX_USE_ALIGNED_NEW(PreparedCamera) // for matrices
};

extern PreparedCamera g_preparedCamera;

extern Camera * g_camera;

void PrepareCamera(Camera * cam, const Rect & viewport, const Vec2i & projectionCenter);
inline void PrepareCamera(Camera * cam, const Rect & viewport) {
	PrepareCamera(cam, viewport, viewport.center());
}

void SetActiveCamera(Camera * cam);

ARX_USE_ALIGNED_ALLOCATOR(PreparedCamera) // for matrices

#endif // ARX_GAME_CAMERA_H
