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

void Util_SetViewMatrix(EERIEMATRIX &mat, EERIE_TRANSFORM &transform) {

	Vec3f vFrom(transform.pos.x, -transform.pos.y, transform.pos.z);
	Vec3f vTout(0.0f, 0.0f, 1.0f);

	Vec3f vView;
	vView.y = -(vTout.z * transform.xsin);
	vView.z = -(vTout.z * transform.xcos);
	vView.x =  (vView.z * transform.ysin);
	vView.z = -(vView.z * transform.ycos);

	Vec3f vWorldUp(0.f, 1.f, 0.f);

	// Normalize the z basis vector
	float fLength = glm::length(vView);
	if (fLength < 1e-6f)
		return;

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
				return;
		}
	}

	// Normalize the y basis vector
	vUp /= fLength;

	// The x basis vector is found simply with the cross product of the y
	// and z basis vectors
	Vec3f vRight = glm::cross(vUp, vView);

	// Start building the matrix. The first three rows contains the basis
	// vectors used to rotate the view to point at the lookat point
	mat._11 = vRight.x;
	mat._12 = vUp.x;
	mat._13 = vView.x;
	mat._21 = vRight.y;
	mat._22 = vUp.y;
	mat._23 = vView.y;
	mat._31 = vRight.z;
	mat._32 = vUp.z;
	mat._33 = vView.z;

	// Do the translation values (rotations are still about the eyepoint)
	mat._41 = -glm::dot(vFrom, vRight);
	mat._42 = -glm::dot(vFrom, vUp);
	mat._43 = -glm::dot(vFrom, vView);
	mat._44 = 1.0f;
}
