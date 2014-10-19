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
// Code: Cyril Meynier
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved

#include "graphics/GraphicsUtility.h"

#include "graphics/Math.h"

void rotPoint(Vec3f *in, Vec3f *out, EERIE_TRANSFORM &transform) {

	Vec3f tmp;

	ZRotatePoint(in, out, transform.zcos, transform.zsin);
	XRotatePoint(out, &tmp, transform.xcos, transform.xsin);
	YRotatePoint(&tmp, out, transform.ycos, -transform.ysin);
}

glm::mat4 Util_LookAt(Vec3f vFrom, Vec3f vView, Vec3f vWorldUp) {
	
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

void Util_SetViewMatrix(glm::mat4x4 &mat, EERIE_TRANSFORM &transform) {

	Vec3f vFrom(transform.pos.x, -transform.pos.y, transform.pos.z);
	Vec3f vTout(0.0f, 0.0f, 1.0f);

	Vec3f vView;
	rotPoint(&vTout, &vView, transform);
	
	Vec3f up(0.f, 1.f, 0.f);
	Vec3f vWorldUp;
	rotPoint(&up, &vWorldUp, transform);

	mat = Util_LookAt(vFrom, vView, vWorldUp);
}
