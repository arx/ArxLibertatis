/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#include "physics/Physics.h"

#include <stddef.h>

#include <boost/foreach.hpp>

#include "graphics/GraphicsTypes.h"
#include "graphics/data/Mesh.h"
#include "math/Vector.h"

#include "core/GameTime.h"

#include "game/NPC.h"
#include "game/EntityManager.h"
#include "game/magic/spells/SpellsLvl06.h"

#include "io/log/Logger.h"

#include "physics/Collisions.h"

#include "scene/GameSound.h"
#include "scene/Interactive.h"


// Creation of the physics box... quite cabalistic and extensive func...
// Need to put a (really) smarter algorithm in there...
void EERIE_PHYSICS_BOX_Create(EERIE_3DOBJ * obj)
{
	if (!obj) return;

	EERIE_PHYSICS_BOX_Release(obj);

	if (obj->vertexlist.empty()) return;

	PHYSICS_BOX_DATA * pbox = new PHYSICS_BOX_DATA();
	
	pbox->stopcount = 0;
	
	Vec3f cubmin = Vec3f(std::numeric_limits<float>::max());
	Vec3f cubmax = Vec3f(-std::numeric_limits<float>::max());
	
	for(size_t k = 0; k < obj->vertexlist.size(); k++) {
		if(k != obj->origin) {
			cubmin = glm::min(cubmin, obj->vertexlist[k].v);
			cubmax = glm::max(cubmax, obj->vertexlist[k].v);
		}
	}
	
	pbox->vert[0].pos = cubmin + (cubmax - cubmin) * .5f;
	pbox->vert[13].pos = pbox->vert[0].pos;
	pbox->vert[13].pos.y = cubmin.y;
	pbox->vert[14].pos = pbox->vert[0].pos;
	pbox->vert[14].pos.y = cubmax.y;
	
	for(size_t k = 1; k < pbox->vert.size() - 2; k++) {
		pbox->vert[k].pos.x = pbox->vert[0].pos.x;
		pbox->vert[k].pos.z = pbox->vert[0].pos.z;
		if(k < 5) {
			pbox->vert[k].pos.y = cubmin.y;
		} else if(k < 9) {
			pbox->vert[k].pos.y = pbox->vert[0].pos.y;
		} else {
			pbox->vert[k].pos.y = cubmax.y;
		}
	}
	
	float diff = cubmax.y - cubmin.y;
	
	if(diff < 12.f) {
		
		cubmax.y += 8.f;
		cubmin.y -= 8.f;
		
		for(size_t k = 1; k < pbox->vert.size() - 2; k++) {
			pbox->vert[k].pos.x = pbox->vert[0].pos.x;
			pbox->vert[k].pos.z = pbox->vert[0].pos.z;
			if(k < 5) {
				pbox->vert[k].pos.y = cubmin.y;
			} else if(k < 9) {
				pbox->vert[k].pos.y = pbox->vert[0].pos.y;
			} else {
				pbox->vert[k].pos.y = cubmax.y;
			}
		}
		
		pbox->vert[14].pos.y = cubmax.y;
		pbox->vert[13].pos.y = cubmin.y;
		float RATI = diff * (1.0f / 8);
		
		for(size_t k = 0; k < obj->vertexlist.size(); k++) {
			
			if(k == obj->origin) {
				continue;
			}
			Vec3f curr = obj->vertexlist[k].v;
			
			size_t SEC = 1;
			pbox->vert[SEC].pos.x = std::min(pbox->vert[SEC].pos.x, curr.x);
			pbox->vert[SEC].pos.z = std::min(pbox->vert[SEC].pos.z, curr.z);
			pbox->vert[SEC + 1].pos.x = std::min(pbox->vert[SEC + 1].pos.x, curr.x);
			pbox->vert[SEC + 1].pos.z = std::max(pbox->vert[SEC + 1].pos.z, curr.z);
			pbox->vert[SEC + 2].pos.x = std::max(pbox->vert[SEC + 2].pos.x, curr.x);
			pbox->vert[SEC + 2].pos.z = std::max(pbox->vert[SEC + 2].pos.z, curr.z);
			pbox->vert[SEC + 3].pos.x = std::max(pbox->vert[SEC + 3].pos.x, curr.x);
			pbox->vert[SEC + 3].pos.z = std::min(pbox->vert[SEC + 3].pos.z, curr.z);
			
			SEC = 5;
			pbox->vert[SEC].pos.x = std::min(pbox->vert[SEC].pos.x, curr.x - RATI);
			pbox->vert[SEC].pos.z = std::min(pbox->vert[SEC].pos.z, curr.z - RATI);
			pbox->vert[SEC + 1].pos.x = std::min(pbox->vert[SEC + 1].pos.x, curr.x - RATI);
			pbox->vert[SEC + 1].pos.z = std::max(pbox->vert[SEC + 1].pos.z, curr.z + RATI);
			pbox->vert[SEC + 2].pos.x = std::max(pbox->vert[SEC + 2].pos.x, curr.x + RATI);
			pbox->vert[SEC + 2].pos.z = std::max(pbox->vert[SEC + 2].pos.z, curr.z + RATI);
			pbox->vert[SEC + 3].pos.x = std::max(pbox->vert[SEC + 3].pos.x, curr.x + RATI);
			pbox->vert[SEC + 3].pos.z = std::min(pbox->vert[SEC + 3].pos.z, curr.z - RATI);
			
			SEC = 9;
			pbox->vert[SEC].pos.x = std::min(pbox->vert[SEC].pos.x, curr.x);
			pbox->vert[SEC].pos.z = std::min(pbox->vert[SEC].pos.z, curr.z);
			pbox->vert[SEC + 1].pos.x = std::min(pbox->vert[SEC + 1].pos.x, curr.x);
			pbox->vert[SEC + 1].pos.z = std::max(pbox->vert[SEC + 1].pos.z, curr.z);
			pbox->vert[SEC + 2].pos.x = std::max(pbox->vert[SEC + 2].pos.x, curr.x);
			pbox->vert[SEC + 2].pos.z = std::max(pbox->vert[SEC + 2].pos.z, curr.z);
			pbox->vert[SEC + 3].pos.x = std::max(pbox->vert[SEC + 3].pos.x, curr.x);
			pbox->vert[SEC  + 3].pos.z = std::min(pbox->vert[SEC + 3].pos.z, curr.z);
			
		}
		
	} else {
		
		float cut = (cubmax.y - cubmin.y) * (1.0f / 3);
		float ysec2 = cubmin.y + cut * 2.f;
		float ysec1 = cubmin.y + cut;

		for (size_t k = 0; k < obj->vertexlist.size(); k++)
		{
			if (k == obj->origin) continue;

			Vec3f curr = obj->vertexlist[k].v;
			size_t SEC;

			if(curr.y < ysec1) {
				SEC = 1;
			} else if(curr.y < ysec2) {
				SEC = 5;
			} else {
				SEC = 9;
			}
			
			pbox->vert[SEC].pos.x = std::min(pbox->vert[SEC].pos.x, curr.x);
			pbox->vert[SEC].pos.z = std::min(pbox->vert[SEC].pos.z, curr.z);
			pbox->vert[SEC + 1].pos.x = std::min(pbox->vert[SEC + 1].pos.x, curr.x);
			pbox->vert[SEC + 1].pos.z = std::max(pbox->vert[SEC + 1].pos.z, curr.z);
			pbox->vert[SEC + 2].pos.x = std::max(pbox->vert[SEC + 2].pos.x, curr.x);
			pbox->vert[SEC + 2].pos.z = std::max(pbox->vert[SEC + 2].pos.z, curr.z);
			pbox->vert[SEC + 3].pos.x = std::max(pbox->vert[SEC + 3].pos.x, curr.x);
			pbox->vert[SEC + 3].pos.z = std::min(pbox->vert[SEC + 3].pos.z, curr.z);
			
		}
	}
	
	for(size_t k = 0; k < 4; k++) {
		if(glm::abs(pbox->vert[5 + k].pos.x - pbox->vert[0].pos.x) < 2.f) {
			pbox->vert[5 + k].pos.x = (pbox->vert[1 + k].pos.x + pbox->vert[9 + k].pos.x) * 0.5f;
		}
		if(glm::abs(pbox->vert[5 + k].pos.z - pbox->vert[0].pos.z) < 2.f) {
			pbox->vert[5 + k].pos.z = (pbox->vert[1 + k].pos.z + pbox->vert[9 + k].pos.z) * 0.5f;
		}
	}
	
	pbox->radius = 0.f;

	for(size_t k = 0; k < pbox->vert.size(); k++) {
		PhysicsParticle & pv = pbox->vert[k];
		
		float distt = glm::distance(pv.pos, pbox->vert[0].pos);
		
		if(distt > 20.f) {
			pv.pos.x = (pv.pos.x - pbox->vert[0].pos.x) * 0.5f + pbox->vert[0].pos.x;
			pv.pos.z = (pv.pos.z - pbox->vert[0].pos.z) * 0.5f + pbox->vert[0].pos.z;
		}
		
		pv.initpos = pv.pos;
		
		if(k != 0) {
			float d = glm::distance(pbox->vert[0].pos, pv.pos);
			pbox->radius = std::max(pbox->radius, d);
		}
	}
	
	float surface = 0.f;
	for(size_t i = 0; i < obj->facelist.size(); i++) {
		const Vec3f & p0 = obj->vertexlist[obj->facelist[i].vid[0]].v;
		const Vec3f & p1 = obj->vertexlist[obj->facelist[i].vid[1]].v;
		const Vec3f & p2 = obj->vertexlist[obj->facelist[i].vid[2]].v;
		surface += glm::distance((p0 + p1) * .5f, p2) * glm::distance(p0, p1) * .5f;
	}
	pbox->surface = surface;
	
	obj->pbox = pbox;
}

// Releases physic box data from an object
void EERIE_PHYSICS_BOX_Release(EERIE_3DOBJ * obj) {
	
	if(!obj || !obj->pbox) {
		return;
	}
	
	delete obj->pbox;
	obj->pbox = NULL;
}

// Used to launch an object into the physical world...
void EERIE_PHYSICS_BOX_Launch(EERIE_3DOBJ * obj, const Vec3f & pos, const Anglef & angle, const Vec3f & vect)
{
	arx_assert(obj);
	arx_assert(obj->pbox);
	
	float ratio = obj->pbox->surface * (1.0f / 10000);
	ratio = glm::clamp(ratio, 0.f, 0.8f);
	ratio = 1.f - (ratio * (1.0f / 4));
	
	for(size_t i = 0; i < obj->pbox->vert.size(); i++) {
		PhysicsParticle * pv = &obj->pbox->vert[i];
		pv->pos = pv->initpos;
		
		pv->pos = VRotateY(pv->pos, MAKEANGLE(270.f - angle.getYaw()));
		pv->pos = VRotateX(pv->pos, -angle.getPitch());
		pv->pos = VRotateZ(pv->pos, angle.getRoll());
		pv->pos += pos;

		pv->force = Vec3f(0.f);
		pv->velocity = vect * (250.f * ratio);
		pv->mass = 0.4f + ratio * 0.1f;
	}
	
	obj->pbox->active = 1;
	obj->pbox->stopcount = 0;
	obj->pbox->storedtiming = 0;
}


static const float VELOCITY_THRESHOLD = 400.f;

template <size_t N>
static void ComputeForces(boost::array<PhysicsParticle, N> & particles) {
	
	const Vec3f PHYSICS_Gravity(0.f, 65.f, 0.f);
	const float PHYSICS_Damping = 0.5f;

	float lastmass = 1.f;
	float div = 1.f;

	for(size_t k = 0; k < particles.size(); k++) {

		PhysicsParticle * pv = &particles[k];

		// Reset Force
		pv->force = Vec3f(0.f);

		// Apply Gravity
		if(pv->mass > 0.f) {

			// need to be precomputed...
			if(lastmass != pv->mass) {
				div = 1.f / pv->mass;
				lastmass = pv->mass;
			}

			pv->force += (PHYSICS_Gravity * div);
		}

		// Apply Damping
		pv->force += pv->velocity * -PHYSICS_Damping;
	}
}

//! Calculate new Positions and Velocities given a deltatime
//! \param DeltaTime that has passed since last iteration
template <size_t N>
static void RK4Integrate(boost::array<PhysicsParticle, N> & particles, float DeltaTime) {
	
	float halfDeltaT, sixthDeltaT;
	halfDeltaT = DeltaTime * .5f; // some time values i will need
	sixthDeltaT = (1.0f / 6);
	
	boost::array<boost::array<PhysicsParticle, N>, 5> m_TempSys;
	
	for(size_t jj = 0; jj < 4; jj++) {

		arx_assert(particles.size() == m_TempSys[jj + 1].size());
		m_TempSys[jj + 1] = particles;

		if(jj == 3) {
			halfDeltaT = DeltaTime;
		}

		for(size_t kk = 0; kk < particles.size(); kk++) {

			PhysicsParticle & source = particles[kk];
			PhysicsParticle & accum1 = m_TempSys[jj + 1][kk];
			PhysicsParticle & target = m_TempSys[0][kk];

			accum1.force = source.force * (source.mass * halfDeltaT);
			accum1.velocity = source.velocity * halfDeltaT;

			// determine the new velocity for the particle over 1/2 time
			target.velocity = source.velocity + accum1.force;
			target.mass = source.mass;

			// set the new position
			target.pos = source.pos + accum1.velocity;
		}

		ComputeForces(m_TempSys[0]); // compute the new forces
	}

	for(size_t kk = 0; kk < particles.size(); kk++) {
		PhysicsParticle & particle = particles[kk];
		
		PhysicsParticle & accum1 = m_TempSys[1][kk];
		PhysicsParticle & accum2 = m_TempSys[2][kk];
		PhysicsParticle & accum3 = m_TempSys[3][kk];
		PhysicsParticle & accum4 = m_TempSys[4][kk];
		
		Vec3f dv = accum1.force + ((accum2.force + accum3.force) * 2.f) + accum4.force;
		Vec3f dp = accum1.velocity + ((accum2.velocity + accum3.velocity) * 2.f) + accum4.velocity;
		
		particle.velocity += (dv * sixthDeltaT);
		particle.pos += (dp * sixthDeltaT * 1.2f); // TODO what is this 1.2 factor doing here ?
	}
}

static bool IsObjectInField(const PHYSICS_BOX_DATA & pbox) {

	for(size_t i = 0; i < MAX_SPELLS; i++) {
		const SpellBase * spell = spells[SpellHandle(i)];

		if(spell && spell->m_type == SPELL_CREATE_FIELD) {
			const CreateFieldSpell * sp = static_cast<const CreateFieldSpell *>(spell);
			
			Entity * pfrm = entities.get(sp->m_entity);
			if(pfrm) {
				Cylinder cyl = Cylinder(Vec3f(0.f), 35.f, -35.f);
				
				for(size_t k = 0; k < pbox.vert.size(); k++) {
					const PhysicsParticle * pv = &pbox.vert[k];
					cyl.origin = pv->pos + Vec3f(0.f, 17.5f, 0.f);
					if(CylinderPlatformCollide(cyl, pfrm)) {
						return true;
					}
				}
			}
		}
	}

	return false;
}


// Checks is a triangle of a physical object is colliding a triangle
static bool IsObjectVertexCollidingTriangle(const PHYSICS_BOX_DATA & pbox, Vec3f * verts)
{
	EERIE_TRI t1, t2;
	bool ret = false;
	std::copy(verts, verts + 2, t2.v);

	const boost::array<PhysicsParticle, 15> & vert = pbox.vert;

	Vec3f center = (verts[0] + verts[1] + verts[2]) * (1.0f / 3);
	float rad = fdist(center, verts[0]);

	{
		size_t nn = 0;

		for (; nn < pbox.vert.size(); nn++)
		{
			if(!fartherThan(center, vert[nn].pos, std::max(60.0f, rad + 25))) {
				nn = 1000;
			}
		}

		if (nn < 1000)
			return false;
	}
	
	// top
	t1.v[0] = vert[1].pos;
	t1.v[1] = vert[2].pos;
	t1.v[2] = vert[3].pos;
	
	if(Triangles_Intersect(t1, t2)) {
		return true;
	}
	
	// bottom
	t1.v[0] = vert[10].pos;
	t1.v[1] = vert[9].pos;
	t1.v[2] = vert[11].pos;
	
	if(Triangles_Intersect(t1, t2)) {
		return true;
	}
	
	// up / front
	t1.v[0] = vert[1].pos;
	t1.v[1] = vert[4].pos;
	t1.v[2] = vert[5].pos;
	
	if(Triangles_Intersect(t1, t2)) {
		return true;
	}
	
	// down / front
	t1.v[0] = vert[5].pos;
	t1.v[1] = vert[8].pos;
	t1.v[2] = vert[9].pos;
	
	if(Triangles_Intersect(t1, t2)) {
		return true;
	}
	
	// up / back
	t1.v[0] = vert[3].pos;
	t1.v[1] = vert[2].pos;
	t1.v[2] = vert[7].pos;
	
	if(Triangles_Intersect(t1, t2)) {
		return true;
	}
	
	// down / back
	t1.v[0] = vert[7].pos;
	t1.v[1] = vert[6].pos;
	t1.v[2] = vert[11].pos;
	
	if(Triangles_Intersect(t1, t2)) {
		return true;
	}

	// up / left
	t1.v[0] = vert[6].pos;
	t1.v[1] = vert[2].pos;
	t1.v[2] = vert[1].pos;

	if(Triangles_Intersect(t1, t2)) {
		return true;
	}

	// down / left
	t1.v[0] = vert[10].pos;
	t1.v[1] = vert[6].pos;
	t1.v[2] = vert[5].pos;

	if(Triangles_Intersect(t1, t2)) {
		return true;
	}

	// up / right
	t1.v[0] = vert[4].pos;
	t1.v[1] = vert[3].pos;
	t1.v[2] = vert[7].pos;

	if(Triangles_Intersect(t1, t2)) {
		return true;
	}

	// down / right
	t1.v[0] = vert[8].pos;
	t1.v[1] = vert[7].pos;
	t1.v[2] = vert[11].pos;

	if(Triangles_Intersect(t1, t2)) {
		return true;
	}

	return ret;
}

static bool IsObjectVertexCollidingPoly(const PHYSICS_BOX_DATA & pbox, const EERIEPOLY & ep) {
	
	Vec3f pol[3];
	pol[0] = ep.v[0].p;
	pol[1] = ep.v[1].p;
	pol[2] = ep.v[2].p;
	
	if(ep.type & POLY_QUAD) {
		
		if(IsObjectVertexCollidingTriangle(pbox, pol)) {
			return true;
		}
		
		pol[1] = ep.v[2].p;
		pol[2] = ep.v[3].p;
		
		return IsObjectVertexCollidingTriangle(pbox, pol);
	}
	
	return IsObjectVertexCollidingTriangle(pbox, pol);
}

static Material polyTypeToCollisionMaterial(const EERIEPOLY & ep) {
	if(ep.type & POLY_METAL) {
		return MATERIAL_METAL;
	}
	if(ep.type & POLY_WOOD) {
		return MATERIAL_WOOD;
	}
	if(ep.type & POLY_STONE) {
		return MATERIAL_STONE;
	}
	if(ep.type & POLY_GRAVEL) {
		return MATERIAL_GRAVEL;
	}
	if(ep.type & POLY_WATER) {
		return MATERIAL_WATER;
	}
	if(ep.type & POLY_EARTH) {
		return MATERIAL_EARTH;
	}
	return MATERIAL_STONE;
}

static bool IsFULLObjectVertexInValidPosition(const PHYSICS_BOX_DATA & pbox, EERIEPOLY * & collisionPoly) {

	float rad = pbox.radius;
	
	// TODO copy-paste background tiles
	int tilex = int(pbox.vert[0].pos.x * ACTIVEBKG->m_mul.x);
	int tilez = int(pbox.vert[0].pos.z * ACTIVEBKG->m_mul.y);
	int radius = std::min(1, short(rad * 0.01f) + 1);
	
	int minx = std::max(tilex - radius, 0);
	int maxx = std::min(tilex + radius, ACTIVEBKG->m_size.x - 1);
	int minz = std::max(tilez - radius, 0);
	int maxz = std::min(tilez + radius, ACTIVEBKG->m_size.y - 1);
	
	for(int z = minz; z <= maxz; z++)
	for(int x = minx; x <= maxx; x++) {
		BackgroundTileData & eg = ACTIVEBKG->m_tileData[x][z];
		BOOST_FOREACH(EERIEPOLY & ep, eg.polydata) {
			
			if(ep.area > 190.f
			   && !(ep.type & POLY_WATER)
			   && !(ep.type & POLY_TRANS)
			   && !(ep.type & POLY_NOCOL)
			) {
				if(fartherThan(ep.center, pbox.vert[0].pos, rad + 75.f))
					continue;
				
				for(size_t kk = 0; kk < pbox.vert.size(); kk++) {
					float radd = 4.f;

					if(!fartherThan(pbox.vert[kk].pos, ep.center, radd)
					   || !fartherThan(pbox.vert[kk].pos, ep.v[0].p, radd)
					   || !fartherThan(pbox.vert[kk].pos, ep.v[1].p, radd)
					   || !fartherThan(pbox.vert[kk].pos, ep.v[2].p, radd)
					   || !fartherThan(pbox.vert[kk].pos, (ep.v[0].p + ep.v[1].p) * .5f, radd)
					   || !fartherThan(pbox.vert[kk].pos, (ep.v[2].p + ep.v[1].p) * .5f, radd)
					   || !fartherThan(pbox.vert[kk].pos, (ep.v[0].p + ep.v[2].p) * .5f, radd)
					) {
						collisionPoly = &ep;
						return false;
					}
					
					// Last addon
					for(size_t kl = 1; kl < pbox.vert.size(); kl++) {
						if(kl != kk) {
							Vec3f pos = (pbox.vert[kk].pos + pbox.vert[kl].pos) * .5f;
							
							if(!fartherThan(pos, ep.center, radd)
							   || !fartherThan(pos, ep.v[0].p, radd)
							   || !fartherThan(pos, ep.v[1].p, radd)
							   || !fartherThan(pos, ep.v[2].p, radd)
							   || !fartherThan(pos, (ep.v[0].p + ep.v[1].p) * .5f, radd)
							   || !fartherThan(pos, (ep.v[2].p + ep.v[1].p) * .5f, radd)
							   || !fartherThan(pos, (ep.v[0].p + ep.v[2].p) * .5f, radd)
							) {
								collisionPoly = &ep;
								return false;
							}
						}
					}
				}
				
				if(IsObjectVertexCollidingPoly(pbox, ep)) {
					collisionPoly = &ep;
					return false;
				}
			}
			
		}
	}

	return true;
}

static bool ARX_INTERACTIVE_CheckFULLCollision(const PHYSICS_BOX_DATA & pbox, Entity & source) {
	
	for(size_t i = 0; i < treatio.size(); i++) {
		
		if(treatio[i].show != SHOW_FLAG_IN_SCENE || (treatio[i].ioflags & IO_NO_COLLISIONS)) {
			continue;
		}
		
		Entity * io = treatio[i].io;
		if(!io || io == &source || !io->obj || io == entities.player()
		   || treatio[i].io->index() == source.no_collide
		   || (io->ioflags & (IO_CAMERA | IO_MARKER | IO_ITEM))
		   || io->usepath
		   || ((io->ioflags & IO_NPC) && (source.ioflags & IO_NO_NPC_COLLIDE))
		   || !closerThan(io->pos, pbox.vert[0].pos, 600.f)
		   || !In3DBBoxTolerance(pbox.vert[0].pos, io->bbox3D, pbox.radius)) {
			continue;
		}
		
		if((io->ioflags & IO_NPC) && io->_npcdata->lifePool.current > 0.f) {
			for(size_t kk = 0; kk < pbox.vert.size(); kk++) {
				if(PointInCylinder(io->physics.cyl, pbox.vert[kk].pos)) {
					return true;
				}
			}
		} else if(io->ioflags & IO_FIX) {
			
			size_t step;
			const size_t nbv = io->obj->vertexlist.size();
			Sphere sp;
			sp.radius = 28.f;
			
			if(nbv < 500) {
				step = 1;
				sp.radius = 36.f;
			} else if(nbv < 900) {
				step = 2;
			} else if(nbv < 1500) {
				step = 4;
			} else {
				step = 6;
			}
			
			std::vector<EERIE_VERTEX> & vlist = io->obj->vertexWorldPositions;
			
			if(io->gameFlags & GFLAG_PLATFORM) {
				for(size_t kk = 0; kk < pbox.vert.size(); kk++) {
					Sphere sphere;
					sphere.origin = pbox.vert[kk].pos;
					sphere.radius = 30.f;
					float miny, maxy;
					miny = io->bbox3D.min.y;
					maxy = io->bbox3D.max.y;

					if(maxy <= sphere.origin.y + sphere.radius || miny >= sphere.origin.y) {
						if(In3DBBoxTolerance(sphere.origin, io->bbox3D, sphere.radius)) {
							// TODO why ignore the z components?
							if(closerThan(Vec2f(io->pos.x, io->pos.z), Vec2f(sphere.origin.x, sphere.origin.z), 440.f + sphere.radius)) {

								EERIEPOLY ep;
								ep.type = 0;

								for(size_t ii = 0; ii < io->obj->facelist.size(); ii++) {
									float cx = 0;
									float cz = 0;
									
									for(long idx = 0 ; idx < 3 ; idx++) {
										ep.v[idx].p = io->obj->vertexWorldPositions[io->obj->facelist[ii].vid[idx]].v;
										cx += ep.v[idx].p.x;
										cz += ep.v[idx].p.z;
									}
									
									cx *= 1.0f / 3;
									cz *= 1.0f / 3;
									
									for(int k = 0; k < 3; k++) {
										ep.v[k].p.x = (ep.v[k].p.x - cx) * 3.5f + cx;
										ep.v[k].p.z = (ep.v[k].p.z - cz) * 3.5f + cz;
									}
									
									if(PointIn2DPolyXZ(&ep, sphere.origin.x, sphere.origin.z))
										return true;
								}
							}
						}
					}
				}
			}


			for(size_t ii = 1; ii < nbv; ii += step) {
				if(ii != io->obj->origin) {
					sp.origin = vlist[ii].v;

					for(size_t kk = 0; kk < pbox.vert.size(); kk++) {
						if(sp.contains(pbox.vert[kk].pos)) {
							if((io->gameFlags & GFLAG_DOOR)) {
								GameDuration elapsed = g_gameTime.now() - io->collide_door_time;
								if(elapsed > GameDurationMs(500)) {
									io->collide_door_time = g_gameTime.now();
									SendIOScriptEvent(&source, io, SM_COLLIDE_DOOR);
									io->collide_door_time = g_gameTime.now();
									SendIOScriptEvent(io, &source, SM_COLLIDE_DOOR);
								}
							}
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}


static void ARX_TEMPORARY_TrySound(Entity & source, Material collisionMaterial, float volume) {
	
	if(source.ioflags & IO_BODY_CHUNK)
		return;
	
	GameInstant now = g_gameTime.now();
	
	if(now > source.soundtime) {
		
		source.soundcount++;
		
		if(source.soundcount < 5) {
			Material material;
			if(EEIsUnderWater(source.pos))
				material = MATERIAL_WATER;
			else if(source.material)
				material = source.material;
			else
				material = MATERIAL_STONE;
			
			if(volume > 1.f)
				volume = 1.f;
			
			ARX_SOUND_PlayCollision(material, collisionMaterial, volume, 1.f, source.pos, &source);
			
			source.soundtime = now + GameDurationMs(100);
		}
	}
}

bool EERIE_PHYSICS_BOX_IsValidPosition(const Vec3f & pos) {
	
	BackgroundTileData * tile = getFastBackgroundData(pos.x, pos.z);
	if(!tile) {
		// Position is outside the world grid
		return false;
	}
	
	if(tile->polyin.empty()) {
		// Position is in an empty tile
		return false;
	}
	
	if(pos.y > tile->maxy) {
		// Position is below the lowest part of the tile
		return false;
	}
	
	return true;
}

static void ARX_EERIE_PHYSICS_BOX_Compute(PHYSICS_BOX_DATA & pbox, float framediff, Entity & source) {

	Vec3f oldpos[32];
	
	for(size_t kk = 0; kk < pbox.vert.size(); kk++) {
		PhysicsParticle * pv = &pbox.vert[kk];
		oldpos[kk] = pv->pos;
		pv->velocity.x = glm::clamp(pv->velocity.x, -VELOCITY_THRESHOLD, VELOCITY_THRESHOLD);
		pv->velocity.y = glm::clamp(pv->velocity.y, -VELOCITY_THRESHOLD, VELOCITY_THRESHOLD);
		pv->velocity.z = glm::clamp(pv->velocity.z, -VELOCITY_THRESHOLD, VELOCITY_THRESHOLD);
	}

	RK4Integrate(pbox.vert, framediff);
	
	EERIEPOLY * collisionPoly = NULL;
	
	bool invalidPosition = false;
	for(size_t i = 0; i < pbox.vert.size(); i += 2) {
		if(!EERIE_PHYSICS_BOX_IsValidPosition(pbox.vert[i].pos - Vec3f(0.f, 10.f, 0.f))) {
			// This indicaties that entity-world collisions are broken
			LogWarning << "Entity " << source.idString() << " escaped the world";
			invalidPosition = true;
			break;
		}
	}
	
	if(   !IsFULLObjectVertexInValidPosition(pbox, collisionPoly)
	   || ARX_INTERACTIVE_CheckFULLCollision(pbox, source)
	   || invalidPosition
	   || IsObjectInField(pbox)
	) {
		
		if(!(source.ioflags & IO_BODY_CHUNK)) {
			Material collisionMat = MATERIAL_STONE;
			if(collisionPoly) {
				collisionMat = polyTypeToCollisionMaterial(*collisionPoly);
			}
			
			Vec3f velocity = pbox.vert[0].velocity;
			
			float power = (glm::abs(velocity.x) + glm::abs(velocity.y) + glm::abs(velocity.z)) * .01f;
			
			ARX_TEMPORARY_TrySound(source, collisionMat, 0.4f + power);
		}

		if(!collisionPoly) {
			for(size_t k = 0; k < pbox.vert.size(); k++) {
				PhysicsParticle * pv = &pbox.vert[k];
				pv->velocity *= Vec3f(-0.3f, -0.4f, -0.3f);
				pv->pos = oldpos[k];
			}
		} else {
			for(size_t k = 0; k < pbox.vert.size(); k++) {
				PhysicsParticle * pv = &pbox.vert[k];

				float t = glm::dot(collisionPoly->norm, pv->velocity);
				pv->velocity -= collisionPoly->norm * (2.f * t);

				pv->velocity *= Vec3f(0.3f, 0.4f, 0.3f);
				pv->pos = oldpos[k];
			}
		}
		
		pbox.stopcount += 1;
	} else {
		pbox.stopcount -= 2;

		if(pbox.stopcount < 0)
			pbox.stopcount = 0;
	}
}

void ARX_PHYSICS_BOX_ApplyModel(PHYSICS_BOX_DATA & pbox, float framediff, float rubber, Entity & source) {
	
	if(pbox.active == 2) {
		return;
	}
	
	if(framediff == 0.f) {
		return;
	}
	
	float timing = pbox.storedtiming + framediff * rubber * 0.0055f;
	float t_threshold = 0.18f;
	
	if(timing < t_threshold) {
		pbox.storedtiming = timing;
		return;
	}
	
	while(timing >= t_threshold) {
		ComputeForces(pbox.vert);
		ARX_EERIE_PHYSICS_BOX_Compute(pbox, std::min(0.11f, timing * 10), source);
		timing -= t_threshold;
	}
	
	pbox.storedtiming = timing;
	
	if(pbox.stopcount < 16) {
		return;
	}
	
	pbox.active = 2;
	pbox.stopcount = 0;
	
	source.soundcount = 0;
	source.soundtime = g_gameTime.now() + GameDurationMs(2000);
}
