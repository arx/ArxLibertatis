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

#ifndef ARX_GAME_CAMERA_H
#define ARX_GAME_CAMERA_H

#include <glm/glm.hpp>

#include "graphics/GraphicsTypes.h"
#include "math/Rectangle.h"
#include "platform/Alignment.h"

struct Camera {
	
private:
	
	static float imagePlaneHeight() { return 480.f; }
	
public:
	
	Vec3f m_pos;
	float focal;
	Anglef angle;
	float cdepth;
	
	Camera()
		: m_pos(0.f)
		, focal(0.f)
		, cdepth(0.f)
	{ }
	
	float fov() const {
		return focalToFov(focal);
	}
	
	void setFov(const float fov) {
		focal = fovToFocal(fov);
	}
	
	void lookAt(const Vec3f & target) {
		if(m_pos != target) {
			angle = getLookAtAngle(m_pos, target);
		}
	}
	
	//! \param fov vertical angle in radians
	static float fovToFocal(float fov) {
		return imagePlaneHeight() / 2.f / glm::tan(fov / 2.f);
	}
	
	//! \return vertical angle in radians
	static float focalToFov(float focal) {
		return 2.f * glm::atan(imagePlaneHeight() / 2.f / focal);
	}
	
	static Anglef getLookAtAngle(const Vec3f & origin, const Vec3f & target);
	
};

struct IO_CAMDATA {
	
	Camera cam;
	
	Vec3f lasttarget;
	Vec3f translatetarget;
	bool lastinfovalid;
	float smoothing;
	
	IO_CAMDATA()
		: lasttarget(0.f)
		, translatetarget(0.f)
		, lastinfovalid(false)
		, smoothing(0.f)
	{ }
	
};

struct PreparedCamera {
	
	glm::mat4x4 m_worldToView;
	glm::mat4x4 m_viewToClip;
	glm::mat4x4 m_viewToScreen;
	glm::mat4x4 m_worldToScreen;
	
	PreparedCamera()
		: m_worldToView(1.f)
		, m_viewToClip(1.f)
		, m_viewToScreen(1.f)
		, m_worldToScreen(1.f)
	{ }
	
	ARX_USE_ALIGNED_NEW(PreparedCamera) // for matrices
	
};

extern Entity * g_cameraEntity;
extern PreparedCamera g_preparedCamera;
extern Camera * g_camera;
extern Camera g_playerCamera;

void PrepareCamera(Camera * cam, const Rect & viewport, const Vec2i & projectionCenter);
inline void PrepareCamera(Camera * cam, const Rect & viewport) {
	PrepareCamera(cam, viewport, viewport.center());
}

void SetActiveCamera(Camera * cam);

ARX_USE_ALIGNED_ALLOCATOR(PreparedCamera) // for matrices

#endif // ARX_GAME_CAMERA_H
