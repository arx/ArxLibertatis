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

#ifndef ARX_PHYSICS_BULLET_ARROWPHYSICS_H
#define ARX_PHYSICS_BULLET_ARROWPHYSICS_H

#include <BulletDynamics/btBulletDynamicsCommon.h>
#include <BulletCollision/btBulletCollisionCommon.h>

#include "ai/Paths.h"

#include <graphics/GraphicsTypes.h>
#include <graphics/data/Mesh.h>

#include <physics/Projectile.h>

class ArrowPhysics {
public:
	ArrowPhysics(Projectile & obj);
	
	void updateThrownObject();
	
	btRigidBody* fallRigidBody;
	
private:
	Projectile & m_obj;
	btConvexHullShape * m_mesh;
	
	btCompoundShape * m_collisionShape;
};

/*!
 * TODO
 *
 * Example Arrow:
 *
 * Length: 750mm
 * Weight: 30g
 * V0: 60m/s
 *
 * Balance offset:
 * |---------|---------|
 * <------------------<<
 *       /\
 * From center to tip 10-15% of total length
 */

#endif
