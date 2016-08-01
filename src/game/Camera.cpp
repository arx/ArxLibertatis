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

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

void EERIE_TRANSFORM::updateFromAngle(const Anglef &angle) {
	float pitch = glm::radians(angle.getPitch());
	xcos = std::cos(pitch);
	xsin = std::sin(pitch);
	float yaw = glm::radians(angle.getYaw());
	ycos = std::cos(yaw);
	ysin = std::sin(yaw);
	float roll = glm::radians(angle.getRoll());
	zcos = std::cos(roll);
	zsin = std::sin(roll);
	
	glm::mat4 translation = glm::translate(-pos);
	worldToView = toRotationMatrix(angle) * translation;
}

MASTER_CAMERA_STRUCT MasterCamera;


static void rotPoint(Vec3f *in, Vec3f *out, EERIE_TRANSFORM &transform) {

	Vec3f tmp;

	ZRotatePoint(in, out, transform.zcos, transform.zsin);
	XRotatePoint(out, &tmp, transform.xcos, transform.xsin);
	YRotatePoint(&tmp, out, transform.ycos, -transform.ysin);
}

static glm::mat4 Util_LookAt(Vec3f vFrom, Vec3f vView, Vec3f vWorldUp) {
	
	glm::mat4x4 mat;
	
	// Normalize the z basis vector
	float fLength = glm::length(vView);
	if (fLength < 1e-6f)
		return glm::mat4(1);

	// Get the dot product, and calculate the projection of the z basis
	// vector onto the up vector. The projection is the y basis vector.
	float fDotProduct = glm::dot(vWorldUp, vView);

	Vec3f vUp = vWorldUp - vView * fDotProduct;

	// If this vector has near-zero length because the input specified a
	// bogus up vector, let's try a default up vector
	if(1e-6f > (fLength = glm::length(vUp)))
	{
		vUp = Vec3f_Y_AXIS - vView * vView.y;

		// If we still have near-zero length, resort to a different axis.
		if(1e-6f > (fLength = glm::length(vUp)))
		{
			vUp = Vec3f_Z_AXIS - vView * vView.z;

			if(1e-6f > (fLength = glm::length(vUp)))
				return glm::mat4(1);
		}
	}

	// Normalize the y basis vector
	vUp /= fLength;

	// The x basis vector is found simply with the cross product of the y
	// and z basis vectors
	Vec3f vRight = glm::cross(vUp, vView);

	// Start building the matrix. The first three rows contains the basis
	// vectors used to rotate the view to point at the lookat point
	mat[0][0] = vRight.x;
	mat[0][1] = vUp.x;
	mat[0][2] = vView.x;
	mat[1][0] = vRight.y;
	mat[1][1] = vUp.y;
	mat[1][2] = vView.y;
	mat[2][0] = vRight.z;
	mat[2][1] = vUp.z;
	mat[2][2] = vView.z;

	// Do the translation values (rotations are still about the eyepoint)
	mat[3][0] = -glm::dot(vFrom, vRight);
	mat[3][1] = -glm::dot(vFrom, vUp);
	mat[3][2] = -glm::dot(vFrom, vView);
	mat[3][3] = 1.0f;
	
	return mat;
}

static glm::mat4 Util_SetViewMatrix(EERIE_TRANSFORM &transform) {

	Vec3f vFrom(transform.pos.x, -transform.pos.y, transform.pos.z);
	Vec3f vTout(0.0f, 0.0f, 1.0f);

	Vec3f vView;
	rotPoint(&vTout, &vView, transform);
	
	Vec3f up(0.f, 1.f, 0.f);
	Vec3f vWorldUp;
	rotPoint(&up, &vWorldUp, transform);

	return Util_LookAt(vFrom, vView, vWorldUp);
}

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
	cam->ProjectionMatrix[1][1] = h;
	cam->ProjectionMatrix[2][2] = Q;
	cam->ProjectionMatrix[3][2] = (-Q * nearDist);
	cam->ProjectionMatrix[2][3] = 1.f;
	cam->ProjectionMatrix[3][3] = 0.f;
	GRenderer->SetProjectionMatrix(cam->ProjectionMatrix);
	
	glm::mat4 tempViewMatrix = Util_SetViewMatrix(cam->orgTrans);
	GRenderer->SetViewMatrix(tempViewMatrix);

	cam->ProjectionMatrix[0][0] *= width * .5f;
	cam->ProjectionMatrix[1][1] *= height * .5f;
	cam->ProjectionMatrix[2][2] = -(farDist * nearDist) / frustumDepth;	//HYPERBOLIC
	cam->ProjectionMatrix[3][2] = Q;

	GRenderer->SetViewport(Rect(static_cast<s32>(width), static_cast<s32>(height)));
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
