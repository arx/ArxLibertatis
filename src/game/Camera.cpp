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

#include "game/Camera.h"

#include "graphics/Math.h"
#include "graphics/Renderer.h"
#include "graphics/GraphicsUtility.h"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

void EERIE_TRANSFORM::updateFromAngle(const Anglef &angle) {
	float yaw, pitch, roll;
	yaw = glm::radians(angle.getYaw());
	xcos = std::cos(yaw);
	xsin = std::sin(yaw);
	pitch = glm::radians(angle.getPitch());
	ycos = std::cos(pitch);
	ysin = std::sin(pitch);
	roll = glm::radians(angle.getRoll());
	zcos = std::cos(roll);
	zsin = std::sin(roll);
	
	glm::mat4 translation = glm::translate(-pos);
	worldToView = toRotationMatrix(angle) * translation;
}

MASTER_CAMERA_STRUCT MasterCamera;



float EERIE_TransformOldFocalToNewFocal(float _fOldFocal)
{
	if(_fOldFocal < 200)
		return (-.34f * _fOldFocal + 168.5f);
	else if(_fOldFocal < 300)
		return (-.25f * _fOldFocal + 150.5f);
	else if(_fOldFocal < 400)
		return (-.155f * _fOldFocal + 124.f);
	else if(_fOldFocal < 500)
		return (-.11f * _fOldFocal + 106.f);
	else if(_fOldFocal < 600)
		return (-.075f * _fOldFocal + 88.5f);
	else if(_fOldFocal < 700)
		return (-.055f * _fOldFocal + 76.5f);
	else if(_fOldFocal < 800)
		return (-.045f * _fOldFocal + 69.5f);
	else
		return 33.5f;
}


void EERIE_CreateMatriceProj(float _fWidth, float _fHeight, EERIE_CAMERA * cam) {

	float _fFOV = EERIE_TransformOldFocalToNewFocal(cam->focal);
	float _fZNear = 1.f;
	float _fZFar = cam->cdepth;


	float fAspect = _fHeight / _fWidth;
	float fFOV = glm::radians(_fFOV);
	float fFarPlane = _fZFar;
	float fNearPlane = _fZNear;
	float w = fAspect * (glm::cos(fFOV / 2) / glm::sin(fFOV / 2));
	float h =   1.0f  * (glm::cos(fFOV / 2) / glm::sin(fFOV / 2));
	float Q = fFarPlane / (fFarPlane - fNearPlane);

	memset(&cam->ProjectionMatrix, 0, sizeof(glm::mat4x4));
	cam->ProjectionMatrix[0][0] = w;
	cam->ProjectionMatrix[1][1] = h;
	cam->ProjectionMatrix[2][2] = Q;
	cam->ProjectionMatrix[3][2] = (-Q * fNearPlane);
	cam->ProjectionMatrix[2][3] = 1.f;
	GRenderer->SetProjectionMatrix(cam->ProjectionMatrix);

	glm::mat4x4 tempViewMatrix;
	Util_SetViewMatrix(tempViewMatrix, cam->orgTrans);
	GRenderer->SetViewMatrix(tempViewMatrix);

	cam->ProjectionMatrix[0][0] *= _fWidth * .5f;
	cam->ProjectionMatrix[1][1] *= _fHeight * .5f;
	cam->ProjectionMatrix[2][2] = -(fFarPlane * fNearPlane) / (fFarPlane - fNearPlane);	//HYPERBOLIC
	cam->ProjectionMatrix[3][2] = Q;

	GRenderer->SetViewport(Rect(static_cast<s32>(_fWidth), static_cast<s32>(_fHeight)));
}

void SP_PrepareCamera(EERIE_CAMERA * cam) {
	cam->orgTrans.updateFromAngle(cam->angle);
	cam->orgTrans.mod = Vec2f(cam->center + cam->clip.origin.toVec2());
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
