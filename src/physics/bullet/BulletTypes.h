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

#ifndef ARX_PHYSICS_BULLET_BULLETTYPES_H
#define ARX_PHYSICS_BULLET_BULLETTYPES_H

#include <BulletDynamics/btBulletDynamicsCommon.h>
#include <BulletCollision/btBulletCollisionCommon.h>

#include "graphics/data/Mesh.h"
#include "math/Types.h"

inline btVector3 conv(const Vec3f & V){
	return btVector3(V.x, V.y, V.z);
}

inline Vec3f conv(const btVector3 & V){
	return Vec3f(V.x(), V.y(), V.z());
}

inline Color convColor(const btVector3 & V) {
	return Color(V.x() * 255, V.y() * 255, V.z() * 255);
}

inline btQuaternion conv(const glm::quat & quat){
	return btQuaternion(quat.x, quat.y, quat.z, quat.w);
}

inline glm::quat conv(const btQuaternion & q){
	glm::quat quat;
	quat.x = q.getX();
	quat.y = q.getY();
	quat.z = q.getZ();
	quat.w = q.getW();
	
	return quat;
}


#endif
