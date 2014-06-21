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

#include "physics/Projectile.h"

#include <string>

#include "platform/Flags.h"

#include "core/GameTime.h"

#include "game/Equipment.h"
#include "game/Player.h"
#include "game/NPC.h"
#include "game/EntityManager.h"

#include "scene/Interactive.h"
#include "scene/GameSound.h"
#include "scene/Light.h"

#include "animation/AnimationRender.h"

#include "physics/Collisions.h"

#include "graphics/Renderer.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/effects/Trail.h"

using std::string;

extern float framedelay;


enum ThrownObjectFlag {
	ATO_EXIST      = (1<<0),
	ATO_MOVING     = (1<<1),
	ATO_UNDERWATER = (1<<2),
	ATO_FIERY      = (1<<3)
};
DECLARE_FLAGS(ThrownObjectFlag, ThrownObjectFlags)
DECLARE_FLAGS_OPERATORS(ThrownObjectFlags)

struct ARX_THROWN_OBJECT {
	ThrownObjectFlags flags;
	Vec3f vector;
	glm::quat quat;
	Vec3f initial_position;
	float velocity;
	Vec3f position;
	float damages;
	EERIE_3DOBJ * obj;
	EntityHandle source;
	unsigned long creation_time;
	float poisonous;
	Trail * pRuban;
};

const size_t MAX_THROWN_OBJECTS = 100;

ARX_THROWN_OBJECT Thrown[MAX_THROWN_OBJECTS];

static bool IsPointInField(const Vec3f & pos) {

	for(size_t i = 0; i < MAX_SPELLS; i++) {
		const SpellBase * spell = spells[SpellHandle(i)];

		if(spell->m_exist && spell->m_type == SPELL_CREATE_FIELD) {

			if(ValidIONum(spell->m_longinfo_entity)) {
				Entity * pfrm = entities[spell->m_longinfo_entity];
				
				EERIE_CYLINDER cyl;
				cyl.height = -35.f;
				cyl.radius = 35.f;
				cyl.origin = pos + Vec3f(0.f, 17.5f, 0.f);

				if(CylinderPlatformCollide(&cyl, pfrm) != 0.f) {
					return true;
				}
			}
		}
	}

	return false;
}

void ARX_THROWN_OBJECT_Kill(long num) {
	if(num >= 0 && size_t(num) < MAX_THROWN_OBJECTS) {
		Thrown[num].flags = 0;
		delete Thrown[num].pRuban;
		Thrown[num].pRuban = NULL;
	}
}

void ARX_THROWN_OBJECT_KillAll() {
	for(size_t i = 0; i < MAX_THROWN_OBJECTS; i++) {
		ARX_THROWN_OBJECT_Kill(i);
	}
}

long ARX_THROWN_OBJECT_GetFree() {
	unsigned long latest_time = (unsigned long)(arxtime);
	long latest_obj = -1;

	for(size_t i = 0; i < MAX_THROWN_OBJECTS; i++) {
		if(Thrown[i].flags & ATO_EXIST) {
			if(Thrown[i].creation_time < latest_time) {
				latest_obj = i;
				latest_time = Thrown[i].creation_time;
			}
		} else {
			return i;
		}
	}

	if(latest_obj >= 0) {
		ARX_THROWN_OBJECT_Kill(latest_obj);
		return latest_obj;
	}

	return -1;
}

extern EERIE_3DOBJ * arrowobj;

void ARX_THROWN_OBJECT_Throw(EntityHandle source, const Vec3f & position, const Vec3f & vect,
							 const glm::quat & quat, float velocity, float damages, float poison) {
	
	arx_assert(arrowobj);
	
	long num = ARX_THROWN_OBJECT_GetFree();
	if(num < 0)
		return;
		
	ARX_THROWN_OBJECT *thrownObj = &Thrown[num];
	
	thrownObj->damages = damages;
	thrownObj->position = position;
	thrownObj->initial_position = position;
	thrownObj->vector = vect;
	thrownObj->quat = quat;
	thrownObj->source = source;
	thrownObj->obj = arrowobj;
	thrownObj->velocity = velocity;
	thrownObj->poisonous = poison;
	
	thrownObj->pRuban = new ArrowTrail();
	thrownObj->pRuban->SetNextPosition(thrownObj->position);
	thrownObj->pRuban->Update(framedelay);
	
	thrownObj->creation_time = (unsigned long)(arxtime);
	thrownObj->flags |= ATO_EXIST | ATO_MOVING;
	
	if(source == 0
	   && player.equiped[EQUIP_SLOT_WEAPON] != PlayerEntityHandle
	   && ValidIONum(player.equiped[EQUIP_SLOT_WEAPON])
	) {
		Entity * tio = entities[player.equiped[EQUIP_SLOT_WEAPON]];
		
		if(tio->ioflags & IO_FIERY)
			thrownObj->flags |= ATO_FIERY;
	}
}

float ARX_THROWN_ComputeDamages(long thrownum, EntityHandle source, EntityHandle target)
{
	float distance_limit = 1000.f;
	Entity * io_target = entities[target];
	Entity * io_source = entities[source];

	SendIOScriptEvent(io_target, SM_AGGRESSION);

	float distance = fdist(Thrown[thrownum].position, Thrown[thrownum].initial_position);
	float distance_modifier = 1.f;

	if(distance < distance_limit * 2.f) {
		distance_modifier = distance / distance_limit;

		if(distance_modifier < 0.5f)
			distance_modifier = 0.5f;
	} else {
		distance_modifier = 2.f;
	}

	float attack, dmgs, backstab, critical, ac;

	backstab = 1.f;
	critical = false;

	if(source == 0) {
		attack = Thrown[thrownum].damages;

		if(rnd() * 100 <= float(player.m_attributeFull.dexterity - 9) * 2.f
						   + float(player.m_skillFull.projectile * 0.2f)) {
			if(SendIOScriptEvent(io_source, SM_CRITICAL, "bow") != REFUSE)
				critical = true;
		}

		dmgs = attack;

		if(io_target->_npcdata->npcflags & NPCFLAG_BACKSTAB) {
			if(rnd() * 100.f <= player.m_skillFull.stealth) {
				if(SendIOScriptEvent(io_source, SM_BACKSTAB, "bow") != REFUSE)
					backstab = 1.5f;
			}
		}
	} else {
		// TODO treat NPC !!!

		ARX_DEAD_CODE();
		attack = 0;
		dmgs = 0;
	}

	float absorb;

	if(target == 0) {
		ac = player.m_miscFull.armorClass;
		absorb = player.m_skillFull.defense * .5f;
	} else {
		ac = ARX_INTERACTIVE_GetArmorClass(io_target);
		absorb = io_target->_npcdata->absorb;
	}

	char wmat[64];

	string _amat = "flesh";
	const string * amat = &_amat;

	strcpy(wmat, "dagger");

	if(!io_target->armormaterial.empty()) {
		amat = &io_target->armormaterial;
	}

	if(io_target == entities.player()) {
		if(player.equiped[EQUIP_SLOT_ARMOR] > 0) {
			Entity * io = entities[player.equiped[EQUIP_SLOT_ARMOR]];
			if(io && !io->armormaterial.empty()) {
				amat = &io->armormaterial;
			}
		}
	}

	float power;
	power = dmgs * ( 1.0f / 20 );

	if(power > 1.f)
		power = 1.f;

	power = power * 0.15f + 0.85f;

	ARX_SOUND_PlayCollision(*amat, wmat, power, 1.f, Thrown[thrownum].position, io_source);

	dmgs *= backstab;
	dmgs -= dmgs * (absorb * ( 1.0f / 100 ));

	float chance = 100.f - (ac - attack);
	float dice = rnd() * 100.f;

	if(dice <= chance) {
		if(dmgs > 0.f) {
			if(critical)
				dmgs *= 1.5f;

			dmgs *= distance_modifier;
			return dmgs;
		}
	}

	return 0.f;
}

EERIEPOLY * CheckArrowPolyCollision(const Vec3f & start, const Vec3f & end) {

	EERIE_TRI pol;
	pol.v[0] = start;
	pol.v[2] = end - Vec3f(2.f, 15.f, 2.f);
	pol.v[1] = end;

	long px = end.x * ACTIVEBKG->Xmul;
	long pz = end.z * ACTIVEBKG->Zmul;

	long ix = std::max(px - 2, 0L);
	long ax = std::min(px + 2, ACTIVEBKG->Xsize - 1L);
	long iz = std::max(pz - 2, 0L);
	long az = std::min(pz + 2, ACTIVEBKG->Zsize - 1L);

	for(long zz = iz; zz <= az; zz++)
		for(long xx = ix; xx <= ax; xx++) {

		FAST_BKG_DATA * feg = &ACTIVEBKG->fastdata[xx][zz];

		for(long k = 0; k < feg->nbpolyin; k++) {

			EERIEPOLY * ep = feg->polyin[k];

			if(ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL)) {
				continue;
			}

			EERIE_TRI pol2;
			pol2.v[0] = ep->v[0].p;
			pol2.v[1] = ep->v[1].p;
			pol2.v[2] = ep->v[2].p;

			if(Triangles_Intersect(&pol2, &pol)) {
				return ep;
			}

			if(ep->type & POLY_QUAD) {
				pol2.v[0] = ep->v[1].p;
				pol2.v[1] = ep->v[3].p;
				pol2.v[2] = ep->v[2].p;
				if(Triangles_Intersect(&pol2, &pol)) {
					return ep;
				}
			}

		}
	}

	return NULL;
}

void CheckExp(long i) {

	if((Thrown[i].flags & ATO_FIERY) && !(Thrown[i].flags & ATO_UNDERWATER)) {

		ARX_BOOMS_Add(Thrown[i].position);
		LaunchFireballBoom(&Thrown[i].position, 10);
		DoSphericDamage(Thrown[i].position, 4.f * 2, 50.f,
						DAMAGE_AREA, DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL, PlayerEntityHandle);
		ARX_SOUND_PlaySFX(SND_SPELL_FIRE_HIT, &Thrown[i].position);
		ARX_NPC_SpawnAudibleSound(Thrown[i].position, entities.player());
		LightHandle id = GetFreeDynLight();

		if(lightHandleIsValid(id) && framedelay > 0) {
			EERIE_LIGHT * light = lightHandleGet(id);
			
			light->intensity = 3.9f;
			light->fallstart = 400.f;
			light->fallend   = 440.f;
			light->rgb = Color3f(1.f - rnd() * .2f, .8f - rnd() * .2f, .6f - rnd() * .2f);
			light->pos = Thrown[i].position;
			light->ex_flaresize = 40.f;
			light->duration = 1500;
		}
	}
}

extern void createFireParticles(Vec3f &pos, const int particlesToCreate, const int particleDelayFactor);

void ARX_THROWN_OBJECT_Render() {

	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetRenderState(Renderer::DepthTest, true);

	for(size_t i = 0; i < MAX_THROWN_OBJECTS; i++) {
		ARX_THROWN_OBJECT *thrownObj = &Thrown[i];
		if(!(thrownObj->flags & ATO_EXIST))
			continue;

		TransformInfo t(thrownObj->position, thrownObj->quat);
		// Object has to be retransformed because arrows share the same object
		DrawEERIEInter_ModelTransform(thrownObj->obj, t);
		DrawEERIEInter_ViewProjectTransform(thrownObj->obj);
		DrawEERIEInter_Render(thrownObj->obj, t, NULL);

		if(thrownObj->pRuban) {
			thrownObj->pRuban->Render();
		}
	}
}

void ARX_THROWN_OBJECT_Manage(unsigned long time_offset)
{
	for(size_t i = 0; i < MAX_THROWN_OBJECTS; i++) {
		ARX_THROWN_OBJECT *thrownObj = &Thrown[i];
		if(!(thrownObj->flags & ATO_EXIST))
			continue;

		{
		// Is Object Visible & Near ?

		FAST_BKG_DATA * bkgData = getFastBackgroundData(thrownObj->position.x, thrownObj->position.z);

		if(!bkgData || !bkgData->treat) {
			continue;
		}

		// Now render object !
		if(!thrownObj->obj)
			continue;

		TransformInfo t(thrownObj->position, thrownObj->quat);
		DrawEERIEInter_ModelTransform(thrownObj->obj, t);

		if((thrownObj->flags & ATO_FIERY) && (thrownObj->flags & ATO_MOVING)
		   && !(thrownObj->flags & ATO_UNDERWATER)) {

			LightHandle id = GetFreeDynLight();
			if(lightHandleIsValid(id) && framedelay > 0) {
				EERIE_LIGHT * light = lightHandleGet(id);
				
				light->intensity = 1.f;
				light->fallstart = 100.f;
				light->fallend   = 240.f;
				light->rgb = Color3f(1.f - rnd() * .2f, .8f - rnd() * .2f, .6f - rnd() * .2f);
				light->pos = thrownObj->position;
				light->ex_flaresize = 40.f;
				light->extras |= EXTRAS_FLARE;
				light->duration = static_cast<long>(framedelay * 0.5f);
			}

			float p = 3.f;

			while(p > 0.f) {
				p -= 0.5f;

				if(thrownObj->obj) {
					Vec3f pos;
					long notok = 10;
					std::vector<EERIE_FACE>::iterator it;

					while(notok-- > 0) {
						it = Random::getIterator(thrownObj->obj->facelist);
						arx_assert(it != thrownObj->obj->facelist.end());

						if(it->facetype & POLY_HIDE)
							continue;

						notok = -1;
					}

					if(notok < 0) {
						pos = thrownObj->obj->vertexlist3[it->vid[0]].v;

						createFireParticles(pos, 2, 180);
					}
				}
			}
		}

		if(thrownObj->pRuban) {
			thrownObj->pRuban->SetNextPosition(thrownObj->position);
			thrownObj->pRuban->Update(time_offset);
		}

		Vec3f original_pos;

		if(thrownObj->flags & ATO_MOVING) {
			long need_kill = 0;
			float mod = (float)time_offset * thrownObj->velocity;
			original_pos = thrownObj->position;
			thrownObj->position.x += thrownObj->vector.x * mod;
			float gmod = 1.f - thrownObj->velocity;

			gmod = clamp(gmod, 0.f, 1.f);

			thrownObj->position.y += thrownObj->vector.y * mod + (time_offset * gmod);
			thrownObj->position.z += thrownObj->vector.z * mod;

			CheckForIgnition(original_pos, 10.f, 0, 2);

			Vec3f wpos = thrownObj->position;
			wpos.y += 20.f;
			EERIEPOLY * ep = EEIsUnderWater(wpos);

			if(thrownObj->flags & ATO_UNDERWATER) {
				if(!ep) {
					thrownObj->flags &= ~ATO_UNDERWATER;
					ARX_SOUND_PlaySFX(SND_PLOUF, &thrownObj->position);
				}
			} else if(ep) {
				thrownObj->flags |= ATO_UNDERWATER;
				ARX_SOUND_PlaySFX(SND_PLOUF, &thrownObj->position);
			}

			// Check for collision MUST be done after DRAWING !!!!
			long nbact = thrownObj->obj->actionlist.size();

			for(long j = 0; j < nbact; j++) {
				float rad = GetHitValue(thrownObj->obj->actionlist[j].name);

				if(rad == -1)
					continue;

				rad *= .5f;

				const Vec3f v0 = thrownObj->obj->vertexlist3[thrownObj->obj->actionlist[j].idx].v;
				Vec3f dest = original_pos + thrownObj->vector * 95.f;
				Vec3f orgn = original_pos - thrownObj->vector * 25.f;
				EERIEPOLY * ep = CheckArrowPolyCollision(orgn, dest);

				if(ep) {
					ARX_PARTICLES_Spawn_Spark(v0, 14, 0);
					CheckExp(i);

					if(ValidIONum(thrownObj->source))
						ARX_NPC_SpawnAudibleSound(v0, entities[thrownObj->source]);

					thrownObj->flags &= ~ATO_MOVING;
					thrownObj->velocity = 0.f;
					
					string bkg_material = "earth";

					if(ep && ep->tex && !ep->tex->m_texName.empty())
						bkg_material = GetMaterialString(ep->tex->m_texName);

					if(ValidIONum(thrownObj->source)) {
						char weapon_material[64] = "dagger";
						
						ARX_SOUND_PlayCollision(weapon_material, bkg_material, 1.f, 1.f, v0, entities[thrownObj->source]);
					}

					thrownObj->position = original_pos;
					j = 200;
				} else if(IsPointInField(v0)) {
					ARX_PARTICLES_Spawn_Spark(v0, 24, 0);
					CheckExp(i);

					if (ValidIONum(thrownObj->source))
						ARX_NPC_SpawnAudibleSound(v0, entities[thrownObj->source]);

					thrownObj->flags &= ~ATO_MOVING;
					thrownObj->velocity = 0.f;
					
					if(ValidIONum(thrownObj->source)) {
						char weapon_material[64] = "dagger";
						char bkg_material[64] = "earth";
						
						ARX_SOUND_PlayCollision(weapon_material, bkg_material, 1.f, 1.f, v0, entities[thrownObj->source]);
					}

					thrownObj->position = original_pos;
					j = 200;
					need_kill = 1;
				} else {
					for(float precision = 0.5f; precision <= 6.f; precision += 0.5f) {
						Sphere sphere;
						sphere.origin = v0 + thrownObj->vector * precision * 4.5f;
						sphere.radius = rad + 3.f;

						std::vector<EntityHandle> sphereContent;

						if(CheckEverythingInSphere(sphere, thrownObj->source, InvalidEntityHandle, sphereContent)) {
							for(size_t jj = 0; jj < sphereContent.size(); jj++) {

								if(ValidIONum(sphereContent[jj])
										&& sphereContent[jj] != thrownObj->source)
								{

									Entity * target = entities[sphereContent[jj]];

									if(target->ioflags & IO_NPC) {
										Vec3f pos;
										Color color = Color::none;
										long hitpoint = -1;
										float curdist = 999999.f;

										for(size_t ii = 0 ; ii < target->obj->facelist.size() ; ii++) {
											if(target->obj->facelist[ii].facetype & POLY_HIDE)
												continue;

											short vid = target->obj->facelist[ii].vid[0];
											float d = glm::distance(sphere.origin, target->obj->vertexlist3[vid].v);

											if(d < curdist) {
												hitpoint = target->obj->facelist[ii].vid[0];
												curdist = d;
											}
										}

										if(hitpoint >= 0) {
											color = target->_npcdata->blood_color;
											pos = target->obj->vertexlist3[hitpoint].v;
										}

										if(thrownObj->source == 0) {
											float damages = ARX_THROWN_ComputeDamages(i, thrownObj->source, sphereContent[jj]);

											if(damages > 0.f) {
												arx_assert(hitpoint >= 0);

												if(target->ioflags & IO_NPC) {
													target->_npcdata->SPLAT_TOT_NB = 0;
													ARX_PARTICLES_Spawn_Blood2(original_pos, damages, color, target);
												}

												ARX_PARTICLES_Spawn_Blood2(pos, damages, color, target);
												ARX_DAMAGES_DamageNPC(target, damages, thrownObj->source, 0, &pos);

												if(rnd() * 100.f > target->_npcdata->resist_poison) {
													target->_npcdata->poisonned += thrownObj->poisonous;
												}

												CheckExp(i);
											} else {
												ARX_PARTICLES_Spawn_Spark(v0, 14, 0);
												ARX_NPC_SpawnAudibleSound(v0, entities[thrownObj->source]);
											}
										}
									} else {
										// not NPC
										if(target->ioflags & IO_FIX) {
											if(ValidIONum(thrownObj->source))
												ARX_DAMAGES_DamageFIX(target, 0.1f, thrownObj->source, 0);
										}

										ARX_PARTICLES_Spawn_Spark(v0, 14, 0);

										if(ValidIONum(thrownObj->source))
											ARX_NPC_SpawnAudibleSound(v0, entities[thrownObj->source]);

										CheckExp(i);
									}

									// Need to deal damages !
									thrownObj->flags &= ~ATO_MOVING;
									thrownObj->velocity = 0.f;
									need_kill = 1;
									precision = 500.f;
									j = 200;
								}
							}
						}
					}
				}
			}

			if(need_kill)
				ARX_THROWN_OBJECT_Kill(i);
		}
		}
	}
}
