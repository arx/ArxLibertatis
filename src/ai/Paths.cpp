/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

#include "ai/Paths.h"

#include <cstdlib>
#include <cstring>
#include <algorithm>

#include <boost/foreach.hpp>

#include "animation/AnimationRender.h"

#include "core/GameTime.h"
#include "core/Core.h"

#include "game/Spells.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "game/Damage.h"
#include "game/EntityManager.h"
#include "game/Equipment.h"
#include "game/Inventory.h"

#include "graphics/GraphicsModes.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"
#include "graphics/Renderer.h"
#include "graphics/effects/SpellEffects.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/data/Mesh.h"
#include "graphics/data/TextureContainer.h"

#include "io/resource/ResourcePath.h"

#include "math/Random.h"

#include "platform/Platform.h"

#include "physics/Box.h"
#include "physics/Collisions.h"

#include "scene/GameSound.h"
#include "scene/Interactive.h"
#include "scene/Light.h"

#include "script/Script.h"

using std::min;
using std::max;
using std::string;

extern long CHANGE_LEVEL_ICON;
extern float framedelay;
ARX_PATH ** ARXpaths = NULL;
MASTER_CAMERA_STRUCT MasterCamera;
long nbARXpaths = 0;

void ARX_PATH_ComputeBB(ARX_PATH * ap) {
	
	ap->bbmin = Vec3f(9999999999.f);
	ap->bbmax = Vec3f(-9999999999.f);
	
	for(long i = 0; i < ap->nb_pathways; i++) {
		ap->bbmin.x = std::min(ap->bbmin.x, ap->pos.x + ap->pathways[i].rpos.x);
		ap->bbmax.x = std::max(ap->bbmax.x, ap->pos.x + ap->pathways[i].rpos.x);
		ap->bbmin.z = std::min(ap->bbmin.z, ap->pos.z + ap->pathways[i].rpos.z);
		ap->bbmax.z = std::max(ap->bbmax.z, ap->pos.z + ap->pathways[i].rpos.z);
	}
	
	if(ap->height > 0) {
		ap->bbmin.y = ap->pos.y - ap->height;
		ap->bbmax.y = ap->pos.y;
	} else {
		ap->bbmin.y = -99999999.f;
		ap->bbmax.y = 99999999.f;
	}
}

void ARX_PATH_ComputeAllBoundingBoxes()
{
	for(long i = 0; i < nbARXpaths; i++) {
		if(ARXpaths[i]) {
			ARX_PATH_ComputeBB(ARXpaths[i]);
		}
	}
}

long ARX_PATH_IsPosInZone(ARX_PATH * ap, float x, float y, float z)
{
	if(x < ap->bbmin.x
	   || x > ap->bbmax.x
	   || z < ap->bbmin.z
	   || z > ap->bbmax.z
	   || y < ap->bbmin.y
	   || y > ap->bbmax.y
	) {
		return 0;
	}

	int i, j, c = 0;

	x -= ap->pos.x;
	z -= ap->pos.z;

	ARX_PATHWAY * app = ap->pathways;

	for(i = 0, j = ap->nb_pathways - 1; i < ap->nb_pathways; j = i++) {
		Vec3f * pi = &app[i].rpos;
		Vec3f * pj = &app[j].rpos;

		if(((pi->z <= z && z < pj->z) || (pj->z <= z && z < pi->z))
		   && (x < (pj->x - pi->x) *(z - pi->z) / (pj->z - pi->z) + pi->x)
		) {
			c = !c;
		}
	}

	return c;
}

ARX_PATH * ARX_PATH_CheckInZone(Entity * io) {
	if(ARXpaths) {
		Vec3f curpos;
		GetItemWorldPosition(io, &curpos);

		for(long i = 0; i < nbARXpaths; i++) {
			if(ARXpaths[i] && ARXpaths[i]->height != 0) {
				if(ARX_PATH_IsPosInZone(ARXpaths[i], curpos.x, curpos.y, curpos.z))
					return ARXpaths[i];
			}
		}
	}

	return NULL;
}

ARX_PATH * ARX_PATH_CheckPlayerInZone()
{
	if(ARXpaths) {
		for(long i = 0; i < nbARXpaths; i++) {
			if(ARXpaths[i] && ARXpaths[i]->height != 0) {
				if(ARX_PATH_IsPosInZone(ARXpaths[i], player.pos.x, player.pos.y + 160.f, player.pos.z))
					return ARXpaths[i];
			}
		}
	}

	return NULL;
}
long JUST_RELOADED = 0;

void ARX_PATH_UpdateAllZoneInOutInside() {
	
	static size_t count = 1;
	
	long f = clamp(static_cast<long>(framedelay), 10, 50);
	
	if(count >= entities.size()) {
		count = 1;
	}

	if(entities.size() > 1)
		for(long tt = 0; tt < f; tt++) {
			long i = count;
			Entity * io = entities[i];
			

			if(count < entities.size()
			   && io
			   && io->ioflags & (IO_NPC | IO_ITEM)
			   && io->show != SHOW_FLAG_MEGAHIDE
			) {
				arx_assert(io->show != SHOW_FLAG_DESTROYED);
				ARX_PATH * p = ARX_PATH_CheckInZone(io);
				ARX_PATH * op = io->inzone;

				if(op == NULL && p == NULL)
					goto next; // Not in a zone

				if(op == p) { // Stayed inside Zone OP
					if(io->show != io->inzone_show) {
						io->inzone_show = io->show;
						goto entering;
					}
				}
				else if ((op != NULL) && (p == NULL)) // Leaving Zone OP
				{
					SendIOScriptEvent(io, SM_LEAVEZONE, op->name);

					if(!op->controled.empty()) {
						long t = entities.getById(op->controled);

						if(t >= 0) {
							string str = io->idString() + ' ' + op->name;
							SendIOScriptEvent(entities[t], SM_CONTROLLEDZONE_LEAVE, str);
						}
					}
				}
				else if ((op == NULL) && (p != NULL)) // Entering Zone P
				{
					io->inzone_show = io->show;
				entering:

					if(JUST_RELOADED && (p->name == "ingot_maker" || p->name == "mauld_user")) {
						ARX_DEAD_CODE(); // TODO remove JUST_RELOADED global
					} else {
						SendIOScriptEvent(io, SM_ENTERZONE, p->name);

						if(!p->controled.empty()) {
							long t = entities.getById(p->controled);

							if(t >= 0) {
								string params = io->idString() + ' ' + p->name;
								SendIOScriptEvent(entities[t], SM_CONTROLLEDZONE_ENTER, params);
							}
						}
					}
				} else {
					SendIOScriptEvent(io, SM_LEAVEZONE, op->name);

					if(!op->controled.empty()) {
						long t = entities.getById(op->controled);

						if(t >= 0) {
							string str = io->idString() + ' ' + op->name;
							SendIOScriptEvent(entities[t], SM_CONTROLLEDZONE_LEAVE, str);
						}
					}

					io->inzone_show = io->show;
					SendIOScriptEvent(io, SM_ENTERZONE, p->name);

					if(!p->controled.empty()) {
						long t = entities.getById(p->controled);

						if(t >= 0) {
							string str = io->idString() + ' ' + p->name;
							SendIOScriptEvent(entities[t], SM_CONTROLLEDZONE_ENTER, str);
						}
					}
				}

				io->inzone = p;
			}

		next:
			count++;

			if(count >= entities.size())
				count = 1;
		}

	// player check*************************************************
	if(entities.player()) {
		ARX_PATH * p = ARX_PATH_CheckPlayerInZone();
		ARX_PATH * op = (ARX_PATH *)player.inzone;

		if((op == NULL) && (p == NULL))
			goto suite; // Not in a zone

		if(op == p) // Stayed inside Zone OP
		{
		
		}
		else if(op != NULL && p == NULL) // Leaving Zone OP
		{
			SendIOScriptEvent(entities.player(), SM_LEAVEZONE, op->name);
			CHANGE_LEVEL_ICON = -1;

			if(!op->controled.empty()) {
				long t = entities.getById(op->controled);

				if(t >= 0) {
					SendIOScriptEvent(entities[t], SM_CONTROLLEDZONE_LEAVE, "player " + op->name);
				}
			}
		}
		else if ((op == NULL) && (p != NULL)) // Entering Zone P
		{
			SendIOScriptEvent(entities.player(), SM_ENTERZONE, p->name);

			if(p->flags & PATH_AMBIANCE && !p->ambiance.empty())
				ARX_SOUND_PlayZoneAmbiance(p->ambiance, ARX_SOUND_PLAY_LOOPED, p->amb_max_vol * ( 1.0f / 100 ));

			if(p->flags & PATH_FARCLIP) {
				desired.flags |= GMOD_ZCLIP;
				desired.zclip = p->farclip;
			}

			if (p->flags & PATH_REVERB)
			{
			}

			if(p->flags & PATH_RGB) {
				desired.flags |= GMOD_DCOLOR;
				desired.depthcolor = p->rgb;
			}

			if(!p->controled.empty()) {
				long t = entities.getById(p->controled);

				if(t >= 0) {
					SendIOScriptEvent(entities[t], SM_CONTROLLEDZONE_ENTER, "player " + p->name);
				}
			}
		} else {
			if(!op->controled.empty()) {
				long t = entities.getById(op->controled);

				if(t >= 0) {
					SendIOScriptEvent(entities[t], SM_CONTROLLEDZONE_LEAVE, "player " + p->name);
				}
			}

			if(!op->controled.empty()) {
				long t = entities.getById(p->controled);

				if(t >= 0) {
					SendIOScriptEvent(entities[t], SM_CONTROLLEDZONE_ENTER, "player " + p->name);
				}
			}
		}

		player.inzone = p;
	}

	
suite:
	JUST_RELOADED = 0;
}

ARX_PATH::ARX_PATH(const std::string & _name, const Vec3f & _pos)
	: name(_name)
	, initpos(_pos)
	, pos(_pos)
{
	flags = 0;
	nb_pathways = 0;
	pathways = NULL;
	height = 0; // 0 NOT A ZONE
	
	rgb = Color3f::black;
	farclip = 0.f;
	reverb = 0.f;
	amb_max_vol = 0.f;
	bbmin = Vec3f_ZERO;
	bbmax = Vec3f_ZERO;
}

void ARX_PATH_ClearAllUsePath() {
	BOOST_FOREACH(Entity * e, entities) {
		if(e && e->usepath) {
			free(e->usepath), e->usepath = NULL;
		}
	}
}

void ARX_PATH_ClearAllControled() {
	for(long i = 0; i < nbARXpaths; i++) {
		if(ARXpaths[i]) {
			ARXpaths[i]->controled.clear();
		}
	}
}

ARX_PATH * ARX_PATH_GetAddressByName(const string & name) {
	
	// TODO this is almost the same as ARX_PATHS_ExistName()
	
	if(name.empty() || !ARXpaths) {
		return NULL;
	}
	
	for(long i = 0; i < nbARXpaths; i++) {
		if(ARXpaths[i] && ARXpaths[i]->name == name) {
			return ARXpaths[i];
		}
	}
	
	return NULL;
}

void ARX_PATH_ReleaseAllPath() {
	
	ARX_PATH_ClearAllUsePath();
	
	for(long i = 0; i < nbARXpaths; i++) {
		if(ARXpaths[i]) {
			free(ARXpaths[i]->pathways), ARXpaths[i]->pathways = NULL;
			delete ARXpaths[i], ARXpaths[i] = NULL;
		}
	}
	
	free(ARXpaths), ARXpaths = NULL;
	nbARXpaths = 0;
}

ARX_PATH * ARX_PATHS_ExistName(const string & name) {
	
	if(!ARXpaths) {
		return NULL;
	}
	
	for(long i = 0; i < nbARXpaths; i++) {
		if(ARXpaths[i]->name == name) {
			return ARXpaths[i];
		}
	}
	
	return NULL;
}

Vec3f ARX_PATH::interpolateCurve(size_t i, float step) {
	Vec3f p0 = pathways[i + 0].rpos, p1 = pathways[i + 1].rpos, p2 = pathways[i + 2].rpos;
	return pos + p0 * (1 - step) + p1 * (step - square(step)) + p2 * square(step);
}

long ARX_PATHS_Interpolate(ARX_USE_PATH * aup, Vec3f * pos) {
	
	ARX_PATH * ap = aup->path;
	
	// compute Delta Time
	float tim = aup->_curtime - aup->_starttime;
	
	if(tim < 0) {
		return -1;
	}
	
	// set pos to startpos
	*pos = Vec3f_ZERO;
	
	if(tim == 0) {
		return 0;
	}
	
	// we start at reference waypoint 0  (time & rpos = 0 for this waypoint).
	long targetwaypoint = 1;
	aup->aupflags &= ~ARX_USEPATH_FLAG_FINISHED;

	if(ap->pathways) {
		ap->pathways[0]._time = 0;
		ap->pathways[0].rpos = Vec3f_ZERO;
	} else {
		return -1;
	}
	
	// While we have time left, iterate
	while(tim > 0) {
		
		// Path Ended
		if(targetwaypoint > ap->nb_pathways - 1) {
			*pos += ap->pos;
			aup->aupflags |= ARX_USEPATH_FLAG_FINISHED;
			return -2;
		}
		
		// Manages a Bezier block
		if(ap->pathways[targetwaypoint - 1].flag == PATHWAY_BEZIER) {
			
			targetwaypoint += 1;
			float delta = tim - ap->pathways[targetwaypoint]._time;
			
			if(delta >= 0) {
				
				tim = delta;
				
				if(targetwaypoint < ap->nb_pathways) {
					*pos = ap->pathways[targetwaypoint].rpos;
				}
				
				targetwaypoint += 1;
				
			} else {
				
				if(targetwaypoint < ap->nb_pathways) {
					
					if(ap->pathways[targetwaypoint]._time == 0) {
						return targetwaypoint - 1;
					}
					
					float rel = tim / ap->pathways[targetwaypoint]._time;
					*pos = ap->interpolateCurve(targetwaypoint - 2, rel);
				}
				
				return targetwaypoint - 1;
			}
			
		} else {
			
			// Manages a non-Bezier block
			float delta = tim - ap->pathways[targetwaypoint]._time;
			
			if(delta >= 0) {
				
				tim = delta;
				
				if(targetwaypoint < ap->nb_pathways) {
					*pos = ap->pathways[targetwaypoint].rpos;
				}
				
				targetwaypoint++;
				
			} else {
				
				if(targetwaypoint < ap->nb_pathways) {
					
					if(ap->pathways[targetwaypoint]._time == 0) {
						return targetwaypoint - 1;
					}
					
					float rel = tim / ap->pathways[targetwaypoint]._time;
					
					*pos += (ap->pathways[targetwaypoint].rpos - *pos) * rel;
				}
				
				*pos += ap->pos;
				
				return targetwaypoint - 1;
			}
		}
	}

	*pos += ap->pos;
	
	return targetwaypoint;
}

extern bool IsValidPos3(Vec3f * pos);

static EERIEPOLY * LAST_COLLISION_POLY = NULL;
extern long CUR_COLLISION_MATERIAL;

float VELOCITY_THRESHOLD = 850.f;

void ARX_ApplySpring(PHYSVERT * phys, long k, long l, float PHYSICS_constant,
                     float PHYSICS_Damp) {
	
	Vec3f deltaP, deltaV, springforce;
	PHYSVERT * pv_k = &phys[k];
	PHYSVERT * pv_l = &phys[l];
	float Dterm, Hterm;

	float restlength = glm::distance(pv_k->initpos, pv_l->initpos);
	// Computes Spring Magnitude
	deltaP = pv_k->pos - pv_l->pos;
	float dist = glm::length(deltaP); // Magnitude of delta
	dist = std::max(dist, 0.000001f); //TODO workaround for division by zero
	float divdist = 1.f / dist;
	Hterm = (dist - restlength) * PHYSICS_constant;

	deltaV = pv_k->velocity - pv_l->velocity; // Delta Velocity Vector
	Dterm = glm::dot(deltaV, deltaP) * PHYSICS_Damp * divdist; // Damping Term
	Dterm = (-(Hterm + Dterm));
	divdist *= Dterm;
	springforce = deltaP * divdist; // Normalize Distance Vector & Calc Force

	pv_k->force += springforce; // + force on particle 1

	pv_l->force -= springforce; // - force on particle 2
}

void ComputeForces(PHYSVERT * phys, long nb) {
	
	const Vec3f PHYSICS_Gravity(0.f, 65.f, 0.f);
	const float PHYSICS_Damping = 0.5f;
	
	float lastmass = 1.f;
	float div = 1.f;
	
	for(long k = 0; k < nb; k++) {
		
		PHYSVERT * pv = &phys[k];
		
		// Reset Force
		pv->force = pv->inertia;
		
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
	
	for(int k = 0; k < nb; k++) {
		// Now Resolves Spring System
		for(long l = 0; l < nb; l++) {
			if(l != k) {
				ARX_ApplySpring(phys, l, k, 15.f, 0.99f);
			}
		}
	}
}

bool ARX_INTERACTIVE_CheckFULLCollision(EERIE_3DOBJ * obj, long source);

//! Calculate new Positions and Velocities given a deltatime
//! @param DeltaTime that has passed since last iteration
void RK4Integrate(EERIE_3DOBJ * obj, float DeltaTime) {
	
	PHYSVERT * source, * target, * accum1, * accum2, * accum3, * accum4;
	float halfDeltaT, sixthDeltaT;
	halfDeltaT = DeltaTime * .5f; // some time values i will need
	sixthDeltaT = ( 1.0f / 6 );
	
	PHYSVERT m_TempSys[5][32];
	
	for(long jj = 0; jj < 4; jj++) {
		
		arx_assert(size_t(obj->pbox->nb_physvert) <= ARRAY_SIZE(m_TempSys[jj + 1]));
		memcpy(m_TempSys[jj + 1], obj->pbox->vert, sizeof(PHYSVERT) * obj->pbox->nb_physvert);
		
		if(jj == 3) {
			halfDeltaT = DeltaTime;
		}
		
		for(long kk = 0; kk < obj->pbox->nb_physvert; kk++) {
			
			source = &obj->pbox->vert[kk];
			accum1 = &m_TempSys[jj + 1][kk];
			target = &m_TempSys[0][kk];
			
			accum1->force = source->force * (source->mass * halfDeltaT);
			accum1->velocity = source->velocity * halfDeltaT;
			
			// determine the new velocity for the particle over 1/2 time
			target->velocity = source->velocity + accum1->force;
			target->mass = source->mass;
			
			// set the new position
			target->pos = source->pos + accum1->velocity;
		}
		
		ComputeForces(m_TempSys[0], obj->pbox->nb_physvert); // compute the new forces
	}
	
	for(long kk = 0; kk < obj->pbox->nb_physvert; kk++) {
		
		source = &obj->pbox->vert[kk]; // current state of particle
		target = &obj->pbox->vert[kk];
		accum1 = &m_TempSys[1][kk];
		accum2 = &m_TempSys[2][kk];
		accum3 = &m_TempSys[3][kk];
		accum4 = &m_TempSys[4][kk];
		
		// determine the new velocity for the particle using rk4 formula
		Vec3f dv = accum1->force + ((accum2->force + accum3->force) * 2.f) + accum4->force;
		target->velocity = source->velocity + (dv * sixthDeltaT);
		// determine the new position for the particle using rk4 formula
		Vec3f dp = accum1->velocity + ((accum2->velocity + accum3->velocity) * 2.f)
		           + accum4->velocity;
		target->pos = source->pos + (dp * sixthDeltaT * 1.2f);
	}
	
}

static bool IsObjectInField(EERIE_3DOBJ * obj) {
	
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		
		if(spells[i].exist && spells[i].type == SPELL_CREATE_FIELD) {
			
			if(ValidIONum(spells[i].longinfo)) {
				
				Entity * pfrm = entities[spells[i].longinfo];
				EERIE_CYLINDER cyl;
				cyl.height = -35.f;
				cyl.radius = 35.f;
				
				for(long k = 0; k < obj->pbox->nb_physvert; k++) {
					PHYSVERT * pv = &obj->pbox->vert[k];
					cyl.origin = pv->pos + Vec3f(0.f, 17.5f, 0.f);
					if(CylinderPlatformCollide(&cyl, pfrm) != 0.f) {
						return true;
					}
				}
			}
		}
	}
	
	return false;
}

static bool IsObjectVertexCollidingPoly(EERIE_3DOBJ * obj, EERIEPOLY * ep,
                                        long k, long * validd) {
	
	Vec3f pol[3];
	pol[0] = ep->v[0].p;
	pol[1] = ep->v[1].p;
	pol[2] = ep->v[2].p;
	
	if(ep->type & POLY_QUAD) {
		
		if(IsObjectVertexCollidingTriangle(obj, pol, k, validd)) {
			return true;
		}
		
		pol[1] = ep->v[2].p;
		pol[2] = ep->v[3].p;
		
		if(IsObjectVertexCollidingTriangle(obj, pol, k, validd)) {
			return true;
		}
		
		return false;
	}
	
	if(IsObjectVertexCollidingTriangle(obj, pol, k, validd)) {
		return true;
	}
	
	return false;
}

static bool IsFULLObjectVertexInValidPosition(EERIE_3DOBJ * obj) {
	
	bool ret = true;

	float x = obj->pbox->vert[0].pos.x;
	float z = obj->pbox->vert[0].pos.z;
	long px = x * ACTIVEBKG->Xmul;
	long pz = z * ACTIVEBKG->Zmul;

	long n = obj->pbox->radius * ( 1.0f / 100 );
	n = min(1L, n + 1);

	long ix = std::max(px - n, 0L);
	long ax = std::min(px + n, ACTIVEBKG->Xsize - 1L);
	long iz = std::max(pz - n, 0L);
	long az = std::min(pz + n, ACTIVEBKG->Zsize - 1L);

	LAST_COLLISION_POLY = NULL;
	EERIEPOLY * ep;
	EERIE_BKG_INFO * eg;

	float rad = obj->pbox->radius;

	for(pz = iz; pz <= az; pz++)
		for(px = ix; px <= ax; px++) {
			eg = &ACTIVEBKG->Backg[px+pz*ACTIVEBKG->Xsize];

			for(long k = 0; k < eg->nbpoly; k++) {
			
				ep = &eg->polydata[k];

				if(ep->area > 190.f
				   && !(ep->type & POLY_WATER)
				   && !(ep->type & POLY_TRANS)
				   && !(ep->type & POLY_NOCOL)
				) {
					if(fartherThan(ep->center, obj->pbox->vert[0].pos, rad + 75.f))
						continue;

					for(long kk = 0; kk < obj->pbox->nb_physvert; kk++) {
						float radd = 4.f;

						if(!fartherThan(obj->pbox->vert[kk].pos, ep->center, radd)
						   || !fartherThan(obj->pbox->vert[kk].pos, ep->v[0].p, radd)
						   || !fartherThan(obj->pbox->vert[kk].pos, ep->v[1].p, radd)
						   || !fartherThan(obj->pbox->vert[kk].pos, ep->v[2].p, radd)
						   || !fartherThan(obj->pbox->vert[kk].pos, (ep->v[0].p + ep->v[1].p) * .5f, radd)
						   || !fartherThan(obj->pbox->vert[kk].pos, (ep->v[2].p + ep->v[1].p) * .5f, radd)
						   || !fartherThan(obj->pbox->vert[kk].pos, (ep->v[0].p + ep->v[2].p) * .5f, radd)
						) {
							LAST_COLLISION_POLY = ep;

							if (ep->type & POLY_METAL) CUR_COLLISION_MATERIAL = MATERIAL_METAL;
							else if (ep->type & POLY_WOOD) CUR_COLLISION_MATERIAL = MATERIAL_WOOD;
							else if (ep->type & POLY_STONE) CUR_COLLISION_MATERIAL = MATERIAL_STONE;
							else if (ep->type & POLY_GRAVEL) CUR_COLLISION_MATERIAL = MATERIAL_GRAVEL;
							else if (ep->type & POLY_WATER) CUR_COLLISION_MATERIAL = MATERIAL_WATER;
							else if (ep->type & POLY_EARTH) CUR_COLLISION_MATERIAL = MATERIAL_EARTH;
							else CUR_COLLISION_MATERIAL = MATERIAL_STONE;

							return false;
						}

						// Last addon
						for(long kl = 1; kl < obj->pbox->nb_physvert; kl++) {
							if(kl != kk) {
								Vec3f pos = (obj->pbox->vert[kk].pos + obj->pbox->vert[kl].pos) * .5f;

								if(!fartherThan(pos, ep->center, radd)
								   || !fartherThan(pos, ep->v[0].p, radd)
								   || !fartherThan(pos, ep->v[1].p, radd)
								   || !fartherThan(pos, ep->v[2].p, radd)
								   || !fartherThan(pos, (ep->v[0].p + ep->v[1].p) * .5f, radd)
								   || !fartherThan(pos, (ep->v[2].p + ep->v[1].p) * .5f, radd)
								   || !fartherThan(pos, (ep->v[0].p + ep->v[2].p) * .5f, radd)
								) {
									LAST_COLLISION_POLY = ep;

									if (ep->type & POLY_METAL) CUR_COLLISION_MATERIAL = MATERIAL_METAL;
									else if (ep->type & POLY_WOOD) CUR_COLLISION_MATERIAL = MATERIAL_WOOD;
									else if (ep->type & POLY_STONE) CUR_COLLISION_MATERIAL = MATERIAL_STONE;
									else if (ep->type & POLY_GRAVEL) CUR_COLLISION_MATERIAL = MATERIAL_GRAVEL;
									else if (ep->type & POLY_WATER) CUR_COLLISION_MATERIAL = MATERIAL_WATER;
									else if (ep->type & POLY_EARTH) CUR_COLLISION_MATERIAL = MATERIAL_EARTH;
									else CUR_COLLISION_MATERIAL = MATERIAL_STONE;

									return false;
								}
							}
						}
					}

				
					if(IsObjectVertexCollidingPoly(obj, ep, -1, NULL)) {
						
						LAST_COLLISION_POLY = ep;

						if (ep->type & POLY_METAL) CUR_COLLISION_MATERIAL = MATERIAL_METAL;
						else if (ep->type & POLY_WOOD) CUR_COLLISION_MATERIAL = MATERIAL_WOOD;
						else if (ep->type & POLY_STONE) CUR_COLLISION_MATERIAL = MATERIAL_STONE;
						else if (ep->type & POLY_GRAVEL) CUR_COLLISION_MATERIAL = MATERIAL_GRAVEL;
						else if (ep->type & POLY_WATER) CUR_COLLISION_MATERIAL = MATERIAL_WATER;
						else if (ep->type & POLY_EARTH) CUR_COLLISION_MATERIAL = MATERIAL_EARTH;
						else CUR_COLLISION_MATERIAL = MATERIAL_STONE;

						return false;
					}
				}
			}
		}

	return ret;
}

static bool ARX_EERIE_PHYSICS_BOX_Compute(EERIE_3DOBJ * obj, float framediff, long source) {
	
	Vec3f oldpos[32];
	long COUNT = 0;
	COUNT++;

	for(long kk = 0; kk < obj->pbox->nb_physvert; kk++) {
		PHYSVERT *pv = &obj->pbox->vert[kk];
		oldpos[kk] = pv->pos;
		pv->inertia = Vec3f_ZERO;

		pv->velocity.x = clamp(pv->velocity.x, -VELOCITY_THRESHOLD, VELOCITY_THRESHOLD);
		pv->velocity.y = clamp(pv->velocity.y, -VELOCITY_THRESHOLD, VELOCITY_THRESHOLD);
		pv->velocity.z = clamp(pv->velocity.z, -VELOCITY_THRESHOLD, VELOCITY_THRESHOLD);
	}
	
	CUR_COLLISION_MATERIAL = MATERIAL_STONE;
	
	RK4Integrate(obj, framediff);

	EERIE_SPHERE sphere;
	PHYSVERT *pv = &obj->pbox->vert[0];
	sphere.origin = pv->pos;
	sphere.radius = obj->pbox->radius;
	long colidd = 0;

	for(int kk = 0; kk < obj->pbox->nb_physvert; kk += 2) {
		pv = &obj->pbox->vert[kk];

		if(!IsValidPos3(&pv->pos)) {
			colidd = 1;
			break;
		}
	}

	if(!IsFULLObjectVertexInValidPosition(obj)
	   || ARX_INTERACTIVE_CheckFULLCollision(obj, source)
	   || colidd
	   || IsObjectInField(obj)
	) {
		colidd = 1;
		float power = (EEfabs(obj->pbox->vert[0].velocity.x)
		               + EEfabs(obj->pbox->vert[0].velocity.y)
		               + EEfabs(obj->pbox->vert[0].velocity.z)) * .01f;


		if(!(ValidIONum(source) && (entities[source]->ioflags & IO_BODY_CHUNK)))
			ARX_TEMPORARY_TrySound(0.4f + power);

		if(!LAST_COLLISION_POLY) {
			for(long k = 0; k < obj->pbox->nb_physvert; k++) {
				pv = &obj->pbox->vert[k];

				pv->velocity.x *= -0.3f;
				pv->velocity.z *= -0.3f;
				pv->velocity.y *= -0.4f;

				pv->pos = oldpos[k];
			}
		} else {
			for(long k = 0; k < obj->pbox->nb_physvert; k++) {
				pv = &obj->pbox->vert[k];
				
				float t = glm::dot(LAST_COLLISION_POLY->norm, pv->velocity);
				pv->velocity -= LAST_COLLISION_POLY->norm * (2.f * t);
				
				pv->velocity.x *= 0.3f;
				pv->velocity.z *= 0.3f;
				pv->velocity.y *= 0.4f;
				
				pv->pos = oldpos[k];
			}
		}
	}

	if(colidd) {
		obj->pbox->stopcount += 1;
	} else {
		obj->pbox->stopcount -= 2;

		if(obj->pbox->stopcount < 0)
			obj->pbox->stopcount = 0;
	}

	return true;
}

long ARX_PHYSICS_BOX_ApplyModel(EERIE_3DOBJ * obj, float framediff, float rubber, long source) {
	
	VELOCITY_THRESHOLD = 400.f;
	long ret = 0;

	if(!obj || !obj->pbox)
		return ret;

	if(obj->pbox->active == 2)
		return ret;

	if(framediff == 0.f)
		return ret;

	PHYSVERT * pv;

	// Memorizes initpos
	for(long k = 0; k < obj->pbox->nb_physvert; k++) {
		pv = &obj->pbox->vert[k];
		pv->temp = pv->pos;
	}

	float timing = obj->pbox->storedtiming + framediff * rubber * 0.0055f;
	float t_threshold = 0.18f;

	if(timing < t_threshold) {
		obj->pbox->storedtiming = timing;
		return 1;
	} else {
		while(timing >= t_threshold) {
			ComputeForces(obj->pbox->vert, obj->pbox->nb_physvert);

			if(!ARX_EERIE_PHYSICS_BOX_Compute(obj, std::min(0.11f, timing * 10), source))
				ret = 1;

			timing -= t_threshold;
		}

		obj->pbox->storedtiming = timing;
	}

	if(obj->pbox->stopcount < 16)
		return ret;

	obj->pbox->active = 2;
	obj->pbox->stopcount = 0;

	if(ValidIONum(source)) {
		entities[source]->soundcount = 0;
		entities[source]->soundtime = (unsigned long)(arxtime) + 2000;
	}

	return ret;
}

void ARX_PrepareBackgroundNRMLs()
{
	long i, j, k;
	long i2, j2, k2;
	EERIE_BKG_INFO * eg;
	EERIE_BKG_INFO * eg2;
	EERIEPOLY * ep;
	EERIEPOLY * ep2;
	Vec3f nrml;
	Vec3f cur_nrml;
	float count;

	for(j = 0; j < ACTIVEBKG->Zsize; j++)
		for(i = 0; i < ACTIVEBKG->Xsize; i++) {
			eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];

			for(long l = 0; l < eg->nbpoly; l++) {
				ep = &eg->polydata[l];

				long nbvert = (ep->type & POLY_QUAD) ? 4 : 3;

				for(k = 0; k < nbvert; k++) {
					float ttt = 1.f;

					if(k == 3) {
						nrml = ep->norm2;
						count = 1.f;
					} else if(k > 0 && nbvert > 3) {
						nrml = (ep->norm + ep->norm2);
						count = 2.f;
						ttt = .5f;
					} else {
						nrml = ep->norm;
						count = 1.f;
					}

					cur_nrml = nrml * ttt;

					long mii = std::max(i - 4, 0L);
					long mai = std::min(i + 4, ACTIVEBKG->Xsize - 1L);
					long mij = std::max(j - 4, 0L);
					long maj = std::min(j + 4, ACTIVEBKG->Zsize - 1L);

					for(j2 = mij; j2 < maj; j2++)
						for(i2 = mii; i2 < mai; i2++) {
							eg2 = &ACTIVEBKG->Backg[i2+j2*ACTIVEBKG->Xsize];

							for(long kr = 0; kr < eg2->nbpoly; kr++) {
								ep2 = &eg2->polydata[kr];

								long nbvert2 = (ep2->type & POLY_QUAD) ? 4 : 3;

								if(ep != ep2)
									for(k2 = 0; k2 < nbvert2; k2++) {
										if(EEfabs(ep2->v[k2].p.x - ep->v[k].p.x) < 2.f
										   && EEfabs(ep2->v[k2].p.y - ep->v[k].p.y) < 2.f
										   && EEfabs(ep2->v[k2].p.z - ep->v[k].p.z) < 2.f
										) {
											if(k2 == 3) {
												if(LittleAngularDiff(&cur_nrml, &ep2->norm2)) {
													nrml += ep2->norm2;
													count += 1.f;
													nrml += cur_nrml;
													count += 1.f;
												}
											} else if(k2 > 0 && nbvert2 > 3) {
												Vec3f tnrml = (ep2->norm + ep2->norm2) * .5f;
												if(LittleAngularDiff(&cur_nrml, &tnrml)) {
													nrml += tnrml * 2.f;
													count += 2.f;
												}
											} else {
												if(LittleAngularDiff(&cur_nrml, &ep2->norm)) {
													nrml += ep2->norm;
													count += 1.f;
												}
											}
										}
									}
							}
						}

					count = 1.f / count;
					ep->tv[k].p = nrml * count;

				}
			}
		}

	for(j = 0; j < ACTIVEBKG->Zsize; j++)
		for(i = 0; i < ACTIVEBKG->Xsize; i++) {
			eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];

			for(long l = 0; l < eg->nbpoly; l++) {
				ep = &eg->polydata[l];

				long nbvert = (ep->type & POLY_QUAD) ? 4 : 3;

				for(k = 0; k < nbvert; k++) {
					ep->nrml[k] = ep->tv[k].p;
				}

				float d = 0.f;

				for(long ii = 0; ii < nbvert; ii++) {
					d = max(d, glm::distance(ep->center, ep->v[ii].p));
				}

				ep->v[0].rhw = d;
			}
		}

}
