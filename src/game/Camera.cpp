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

	cam->ProjectionMatrix = glm::mat4x4();
	cam->ProjectionMatrix[0][0] = w;
	cam->ProjectionMatrix[1][1] = -h;
	cam->ProjectionMatrix[2][2] = Q;
	cam->ProjectionMatrix[3][2] = -Q * nearDist;
	cam->ProjectionMatrix[2][3] = 1.f;
	cam->ProjectionMatrix[3][3] = 0.f;
	GRenderer->SetProjectionMatrix(cam->ProjectionMatrix);
	GRenderer->SetViewMatrix(cam->orgTrans.worldToView);

	cam->ProjectionMatrix[0][0] *= width * .5f;
	cam->ProjectionMatrix[1][1] *= -height * .5f;
	
	GRenderer->SetViewport(Rect(static_cast<s32>(width), static_cast<s32>(height)));
}

void SP_PrepareCamera(EERIE_CAMERA * cam) {
	
	cam->orgTrans.worldToView = glm::translate(toRotationMatrix(cam->angle), -cam->orgTrans.pos);
	
	cam->orgTrans.mod = Vec2f(cam->center + cam->clip.topLeft());
}

void PrepareCamera(EERIE_CAMERA * cam, const Rect & size) {
	
	SP_PrepareCamera(cam);
	EERIE_CreateMatriceProj(static_cast<float>(size.width()),
	                        static_cast<float>(size.height()),
	                        cam);
}

EERIE_CAMERA * ACTIVECAM = NULL;

void SetActiveCamera(EERIE_CAMERA * cam)
{
	if (ACTIVECAM != cam) ACTIVECAM = cam;
}
