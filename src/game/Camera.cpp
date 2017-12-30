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

#include "game/Camera.h"

#include <glm/gtc/matrix_transform.hpp>

#include "graphics/Math.h"
#include "graphics/Renderer.h"


MASTER_CAMERA_STRUCT MasterCamera;

static void EERIE_CreateMatriceProj(float width, float height, EERIE_CAMERA * cam) {

	float fov = focalToFov(cam->focal);
	
	const float nearDist = 1.f;
	const float farDist = cam->cdepth;
	const float frustumDepth = farDist - nearDist;
	
	float aspect = height / width;
	float w = aspect * (glm::cos(fov / 2) / glm::sin(fov / 2));
	float h =   1.0f  * (glm::cos(fov / 2) / glm::sin(fov / 2));
	float Q = farDist / frustumDepth;
	
	glm::mat4x4 projectionMatrix;
	projectionMatrix[0][0] = w;
	projectionMatrix[1][1] = -h;
	projectionMatrix[2][2] = Q;
	projectionMatrix[3][2] = -Q * nearDist;
	projectionMatrix[2][3] = 1.f;
	projectionMatrix[3][3] = 0.f;
	GRenderer->SetProjectionMatrix(projectionMatrix);
	
	glm::mat4x4 ndcToScreen = glm::scale(glm::mat4x4(1), Vec3f(width * 0.5f, -height * 0.5f, 1.f));
	
	cam->ProjectionMatrix = ndcToScreen * projectionMatrix;
	
}

void PrepareCamera(EERIE_CAMERA * cam, const Rect & size) {
	
	cam->m_worldToView = glm::translate(toRotationMatrix(cam->angle), -cam->m_pos);
	GRenderer->SetViewMatrix(cam->m_worldToView);
	
	cam->m_mod = Vec2f(cam->center);
	
	EERIE_CreateMatriceProj(static_cast<float>(size.width()),
	                        static_cast<float>(size.height()),
	                        cam);
	
	GRenderer->SetViewport(size);
	
}

EERIE_CAMERA * ACTIVECAM = NULL;

void SetActiveCamera(EERIE_CAMERA * cam)
{
	if (ACTIVECAM != cam) ACTIVECAM = cam;
}
