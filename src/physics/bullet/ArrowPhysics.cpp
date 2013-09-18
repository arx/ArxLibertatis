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

#include "physics/bullet/ArrowPhysics.h"

#include "io/log/Logger.h"

#include "physics/bullet/BulletTypes.h"

extern EERIE_3DOBJ * arrowobj;


btCompoundShape* shiftTransform(btCompoundShape* boxCompound, btScalar mass, btTransform & shift)
{
	btTransform principal;
	btVector3 principalInertia;
	btScalar* masses = new btScalar[boxCompound->getNumChildShapes()];
	for(int j = 0; j < boxCompound->getNumChildShapes(); j++) {
		//evenly distribute mass
		masses[j]=mass/boxCompound->getNumChildShapes();
	}

	boxCompound->calculatePrincipalAxisTransform(masses,principal,principalInertia);

	///create a new compound with world transform/center of mass properly aligned with the principal axis

	///creation is faster using a new compound to store the shifted children
	btCompoundShape * newBoxCompound = new btCompoundShape();
	for(int i=0; i<boxCompound->getNumChildShapes(); i++) {
		btTransform newChildTransform = principal.inverse() * boxCompound->getChildTransform(i);
		newBoxCompound->addChildShape(newChildTransform, boxCompound->getChildShape(i));
	}

	shift = principal;
	return newBoxCompound;
}


ArrowPhysics::ArrowPhysics(Projectile & obj)
	: m_obj(obj)
{

	btTransform startTransform(btQuaternion(conv(m_obj.quat)), conv(m_obj.position));


	EERIE_3DOBJ *eobj = arrowobj;
/*
	m_mesh = new btConvexHullShape();

	for(size_t i = 0; i < eobj->facelist.size(); i++) {
		EERIE_FACE * eface = &eobj->facelist[i];

		long paf[3];
		paf[0] = eface->vid[0];
		paf[1] = eface->vid[1];
		paf[2] = eface->vid[2];

		Vec3f a = eobj->vertexlist[paf[0]].vert.p;
		Vec3f b = eobj->vertexlist[paf[1]].vert.p;
		Vec3f c = eobj->vertexlist[paf[2]].vert.p;

		m_mesh->addPoint(conv(a));
		m_mesh->addPoint(conv(b));
		m_mesh->addPoint(conv(c));
	}

	btTransform t;
	t.setIdentity();
	btVector3 aabbMin;
	btVector3 aabbMax;
	m_mesh->getAabb(t, aabbMin, aabbMax);
*/

	std::vector<btVector3> actionPoints;

	for(size_t i = 0; i < eobj->actionlist.size(); i++) {
		EERIE_ACTIONLIST & action = eobj->actionlist[i];

		LogInfo << action.name << action.act << action.idx.handleData() << action.sfx;

		Vec3f v = eobj->vertexlist[action.idx.handleData()].v;

		actionPoints.push_back(conv(v));
	}

	btVector3 foo = actionPoints.at(1) - actionPoints.at(0);

	btConeShape * tip = new btConeShapeX(2, 5);
	btTransform tipTransform;
	tipTransform.setIdentity();
	tipTransform.setOrigin(btVector3(96.f, 2.7f, 0.f));
	
	btCylinderShape * shaft = new btCylinderShapeX(btVector3(40, 1, 1));
	btTransform shaftTransform;
	shaftTransform.setIdentity();
	shaftTransform.setOrigin(btVector3(50.f, 2.7f, 0.f));

	btBoxShape * fletching = new btBoxShape(btVector3(5, 2, 2));
	btTransform fletchingTransform;
	fletchingTransform.setIdentity();
	fletchingTransform.setOrigin(btVector3(5, 2.7f, 0.f));
	
	btScalar* masses = new btScalar[3];
	masses[0] = 0.02f;
	masses[1] = 0.009f;
	masses[2] = 0.001f;
	
	btScalar mass = 0.03f;
	btVector3 fallInertia(0,0,0);
	
	btCompoundShape * tempCompound = new btCompoundShape();
	tempCompound->addChildShape(tipTransform, tip);
	tempCompound->addChildShape(shaftTransform, shaft);
	tempCompound->addChildShape(fletchingTransform, fletching);
	//m_collisionShape->calculateLocalInertia(mass, fallInertia);
	
	btTransform principal;
	btVector3 principalInertia;
	tempCompound->calculatePrincipalAxisTransform(masses, principal, principalInertia);
	
	///creation is faster using a new compound to store the shifted children
	btCompoundShape * newBoxCompound = new btCompoundShape();
	for(int i = 0; i < tempCompound->getNumChildShapes(); i++) {
		btTransform newChildTransform = principal.inverse() * tempCompound->getChildTransform(i);
		newBoxCompound->addChildShape(newChildTransform, tempCompound->getChildShape(i));
	}

	delete tempCompound;
	
	m_collisionShape = newBoxCompound;


	//btTransform shift;
	//shift.setIdentity();
	//shaftTransform.setOrigin(-(foo / 2));
	
	//btCompoundShape * compound = new btCompoundShape();
	//compound->addChildShape(shaftTransform, shaft);
	
	
	

	//m_collisionShape = shiftTransform(m_collisionShape, mass, shift);
	

	btTransform dispTransform;
	dispTransform.setIdentity();
	dispTransform.setOrigin(principal.getOrigin() * -1.f);
	

	btDefaultMotionState* motionState = new btDefaultMotionState(startTransform * dispTransform);

	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, m_collisionShape, principalInertia);

	fallRigidBody = new btRigidBody(rbInfo);
	fallRigidBody->setFriction(1.f);
	//fallRigidBody->setDamping(0.f, 0.5f);

	fallRigidBody->setLinearVelocity(conv(m_obj.vector * m_obj.velocity * 200.f));

}

void ArrowPhysics::updateThrownObject() {
	
	btTransform trans;
	fallRigidBody->getMotionState()->getWorldTransform(trans);
	m_obj.position = conv(trans.getOrigin());
	m_obj.quat = conv(trans.getRotation());
}
