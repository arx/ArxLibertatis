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

#include "physics/bullet/BulletPhysicsBackend.h"

#include "BulletCollision/CollisionShapes/btShapeHull.h"

BulletPhysicsBackend * g_bulletPhysics;


BulletPhysicsBackend::BulletPhysicsBackend()
	: m_groundRigidBody(NULL)
{
	m_broadphase = new btDbvtBroadphase();
	
	m_collisionConfiguration = new btDefaultCollisionConfiguration();
	m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
	
	
	m_solver = new btSequentialImpulseConstraintSolver;
	
	m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher,
												  m_broadphase,
												  m_solver,
												  m_collisionConfiguration);
	
	
	
	m_dynamicsWorld->setGravity(btVector3(0, 80, 0));
	
	
	debugDrawer = new ArxBulletDebugDrawer;
	debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe 
							| btIDebugDraw::DBG_DrawText
							| btIDebugDraw::DBG_DrawFeaturesText);
	
	m_dynamicsWorld->setDebugDrawer(debugDrawer);
}

BulletPhysicsBackend::~BulletPhysicsBackend() {
	delete m_dynamicsWorld;
	delete m_solver;
	delete m_dispatcher;
	delete m_collisionConfiguration;
	delete m_broadphase;
}

void BulletPhysicsBackend::AddBackground() {
	
	if(m_groundRigidBody != NULL) {
		m_dynamicsWorld->removeRigidBody(m_groundRigidBody);
		delete m_backgroundMesh;
		delete m_groundRigidBody;
	}
	
	m_backgroundMesh = new btTriangleMesh(false, false);
	
	for(long pz = 0; pz < ACTIVEBKG->m_size.y; pz++) {
		for(long px = 0; px < ACTIVEBKG->m_size.x; px++) {
			
			BackgroundTileData * eg = &ACTIVEBKG->m_tileData[px][pz];
			
			for(long k = 0; k < eg->nbpoly; k++) {
				
				EERIEPOLY * ep = &eg->polydata[k];
				
				if(ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL)) {
					continue;
				}
				
				btVector3 v0(ep->v[0].p.x, ep->v[0].p.y, ep->v[0].p.z);
				btVector3 v1(ep->v[1].p.x, ep->v[1].p.y, ep->v[1].p.z);
				btVector3 v2(ep->v[2].p.x, ep->v[2].p.y, ep->v[2].p.z);
				m_backgroundMesh->addTriangle(v0, v1, v2, true);
				
				if(ep->type & POLY_QUAD) {
					btVector3 v0(ep->v[1].p.x, ep->v[1].p.y, ep->v[1].p.z);
					btVector3 v1(ep->v[3].p.x, ep->v[3].p.y, ep->v[3].p.z);
					btVector3 v2(ep->v[2].p.x, ep->v[2].p.y, ep->v[2].p.z);
					m_backgroundMesh->addTriangle(v0, v1, v2, true);
				}
			}
		}
	}
	
	
	btBvhTriangleMeshShape * groundShape = new btBvhTriangleMeshShape(m_backgroundMesh, false);
	
	btDefaultMotionState* groundMotionState =
					new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)));
	
	btRigidBody::btRigidBodyConstructionInfo
			groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0,0,0));
	
	m_groundRigidBody = new btRigidBody(groundRigidBodyCI);
	
	// Disable debug drawing
	int flags = m_groundRigidBody->getCollisionFlags() | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT;
	m_groundRigidBody->setCollisionFlags(flags);
	
	m_dynamicsWorld->addRigidBody(m_groundRigidBody);
}

void BulletPhysicsBackend::DrawBackgroundMesh() {
	m_dynamicsWorld->debugDrawWorld();
	debugDrawer->renderIt();
}

#include "io/log/Logger.h"

void BulletPhysicsBackend::Step(float time) {
	
	float time_s = time / 1000.f;
	
	int maxSubSteps = 10;
	float fixedTimeStep = 1.f / 60.f;
	float maxValidTimeStep = maxSubSteps * fixedTimeStep;
	
	if(time_s >= maxValidTimeStep) {
		LogDebug("Timestep too large");
	}
	
	m_dynamicsWorld->stepSimulation(time_s, maxSubSteps, fixedTimeStep);
	
	for(size_t i=0;i<arrows.size();i++) {
		ArrowPhysics * arrow = arrows[i];
		arrow->updateThrownObject();
	}
}

void BulletPhysicsBackend::ThrowArrow(Projectile & obj) {
	ArrowPhysics * arrow = new ArrowPhysics(obj);
	arrows.push_back(arrow);
	
	m_dynamicsWorld->addRigidBody(arrow->fallRigidBody);
}

void BulletPhysicsBackend::CreateObject(EERIE_3DOBJ *obj)
{
	btConvexHullShape * shape = new btConvexHullShape();
	
	for(size_t k = 0; k < obj->vertexlist.size(); k++) {
		if(k != obj->origin) {
			const Vec3f & vec = obj->vertexlist[k].v;
			shape->addPoint(conv(vec), false);
		}
	}
	
	shape->recalcLocalAabb();
	
	/*
	{ //create a hull approximation
		btShapeHull * hull = new btShapeHull(shape);
		hull->buildHull(shape->getMargin());
		
		btConvexHullShape * simpleShape = new btConvexHullShape();
		
		for(int i = 0; i < hull->numVertices(); i++) {
			simpleShape->addPoint(hull->getVertexPointer()[i], false);
		}
		
		simpleShape->recalcLocalAabb();
		
		obj->pbox->shape = simpleShape;
	}
	//*/

	obj->pbox->shape = shape;
}

void BulletPhysicsBackend::LaunchObject(EERIE_3DOBJ * obj, const Vec3f & pos, const Anglef & angle, const Vec3f & velocity, float mass)
{
	
	btTransform launchTransform;
	launchTransform.setIdentity();
	launchTransform.setOrigin(conv(pos));
	
	if(!obj->pbox->body) {
		btDefaultMotionState* motionState = new btDefaultMotionState(launchTransform);
		btRigidBody * body = new btRigidBody(1.f, motionState, obj->pbox->shape);
		body->setFriction(1.f);
		body->setDamping(0.1f, 0.1f);
		
		btVector3 inertia;
		body->getCollisionShape()->calculateLocalInertia(mass, inertia);
		body->setMassProps(mass, inertia);
		
		m_dynamicsWorld->addRigidBody(body);
		
		obj->pbox->body = body;
	}
	
	btRigidBody * body = obj->pbox->body;
	
	body->setWorldTransform(launchTransform);
	body->setLinearVelocity(conv(velocity));
	
}
