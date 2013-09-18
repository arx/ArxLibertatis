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

#ifndef ARX_PHYSICS_BULLET_BULLETPHYSICSBACKEND_H
#define ARX_PHYSICS_BULLET_BULLETPHYSICSBACKEND_H

#include <vector>

#include <BulletCollision/btBulletCollisionCommon.h>
#include <BulletDynamics/btBulletDynamicsCommon.h>

#include "physics/bullet/BulletTypes.h"
#include "physics/bullet/DebugDrawer.h"
#include "physics/bullet/ArrowPhysics.h"

class BulletPhysicsBackend {
public:
	BulletPhysicsBackend();
	~BulletPhysicsBackend();
	
	void AddBackground();
	
	void DrawBackgroundMesh();
	
	void Step(float time);
	
	void ThrowArrow(Projectile & obj);
	
	void CreateObject(EERIE_3DOBJ * obj);
	void LaunchObject(EERIE_3DOBJ *obj, const Vec3f &pos, const Anglef &angle, const Vec3f &velocity, float mass);
	
private:
	btTriangleMesh * m_backgroundMesh;
	btRigidBody * m_groundRigidBody;
	
	std::vector<ArrowPhysics *> arrows;
	
	
	btDiscreteDynamicsWorld             * m_dynamicsWorld;
	btSequentialImpulseConstraintSolver * m_solver;
	btCollisionDispatcher               * m_dispatcher;
	btDefaultCollisionConfiguration     * m_collisionConfiguration;
	btBroadphaseInterface               * m_broadphase;
	
	ArxBulletDebugDrawer * debugDrawer;
};

extern BulletPhysicsBackend * g_bulletPhysics;

#endif
