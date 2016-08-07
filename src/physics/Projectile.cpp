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

#include "core/Core.h"
#include "core/GameTime.h"

#include "game/Equipment.h"
#include "game/Player.h"
#include "game/NPC.h"
#include "game/EntityManager.h"
#include "game/magic/spells/SpellsLvl06.h"

#include "scene/Interactive.h"
#include "scene/GameSound.h"
#include "scene/Light.h"

#include "animation/AnimationRender.h"

#include "graphics/Renderer.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/Spark.h"
#include "graphics/effects/PolyBoom.h"
#include "graphics/effects/Trail.h"

#include "physics/Collisions.h"

#include "util/Flags.h"


const size_t MAX_THROWN_OBJECTS = 100;

Projectile g_projectiles[MAX_THROWN_OBJECTS];

static bool IsPointInField(const Vec3f & pos) {

	for(size_t i = 0; i < MAX_SPELLS; i++) {
		const SpellBase * spell = spells[SpellHandle(i)];

		if(spell && spell->m_type == SPELL_CREATE_FIELD) {
			const CreateFieldSpell * sp = static_cast<const CreateFieldSpell *>(spell);
			
			if(ValidIONum(sp->m_entity)) {
				Entity * pfrm = entities[sp->m_entity];
				
				Cylinder cyl = Cylinder(pos + Vec3f(0.f, 17.5f, 0.f), 35.f, -35.f);
				
				if(CylinderPlatformCollide(cyl, pfrm)) {
					return true;
				}
			}
		}
	}

	return false;
}

static void ARX_THROWN_OBJECT_Kill(long num) {
	if(num >= 0 && size_t(num) < MAX_THROWN_OBJECTS) {
		g_projectiles[num].flags = 0;
		delete g_projectiles[num].m_trail;
		g_projectiles[num].m_trail = NULL;
	}
}

void ARX_THROWN_OBJECT_KillAll() {
	for(size_t i = 0; i < MAX_THROWN_OBJECTS; i++) {
		ARX_THROWN_OBJECT_Kill(i);
	}
}

static long ARX_THROWN_OBJECT_GetFree() {
	
	ArxInstant latest_time = arxtime.now();
	long latest_obj = -1;

	for(size_t i = 0; i < MAX_THROWN_OBJECTS; i++) {
		if(g_projectiles[i].flags & ATO_EXIST) {
			if(g_projectiles[i].creation_time < latest_time) {
				latest_obj = i;
				latest_time = g_projectiles[i].creation_time;
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
		
	Projectile & projectile = g_projectiles[num];
	
	projectile.damages = damages;
	projectile.position = position;
	projectile.initial_position = position;
	projectile.vector = vect;
	projectile.quat = quat;
	projectile.source = source;
	projectile.obj = arrowobj;
	projectile.velocity = velocity;
	projectile.poisonous = poison;
	
	projectile.m_trail = new ArrowTrail();
	projectile.m_trail->SetNextPosition(projectile.position);
	projectile.m_trail->Update(g_framedelay);
	
	projectile.creation_time = arxtime.now();
	projectile.flags |= ATO_EXIST | ATO_MOVING;
	
	if(source == EntityHandle_Player
	   && ValidIONum(player.equiped[EQUIP_SLOT_WEAPON])
	) {
		Entity * tio = entities[player.equiped[EQUIP_SLOT_WEAPON]];
		
		if(tio->ioflags & IO_FIERY)
			projectile.flags |= ATO_FIERY;
	}
}

static float ARX_THROWN_ComputeDamages(const Projectile & projectile, EntityHandle source,
                                       EntityHandle target) {
	
	float distance_limit = 1000.f;
	Entity * io_target = entities[target];
	Entity * io_source = entities[source];

	SendIOScriptEvent(io_target, SM_AGGRESSION);

	float distance = fdist(projectile.position, projectile.initial_position);
	float distance_modifier = 1.f;

	if(distance < distance_limit * 2.f) {
		distance_modifier = distance / distance_limit;

		if(distance_modifier < 0.5f)
			distance_modifier = 0.5f;
	} else {
		distance_modifier = 2.f;
	}

	float attack, dmgs, backstab, ac;

	backstab = 1.f;
	bool critical = false;

	if(source == EntityHandle_Player) {
		attack = projectile.damages;

		if(Random::getf(0.f, 100.f) <= float(player.m_attributeFull.dexterity - 9) * 2.f
						   + float(player.m_skillFull.projectile * 0.2f)) {
			if(SendIOScriptEvent(io_source, SM_CRITICAL, "bow") != REFUSE)
				critical = true;
		}

		dmgs = attack;

		if(io_target->_npcdata->npcflags & NPCFLAG_BACKSTAB) {
			if(Random::getf(0.f, 100.f) <= player.m_skillFull.stealth) {
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

	if(target == EntityHandle_Player) {
		ac = player.m_miscFull.armorClass;
		absorb = player.m_skillFull.defense * .5f;
	} else {
		ac = ARX_INTERACTIVE_GetArmorClass(io_target);
		absorb = io_target->_npcdata->absorb;
	}

	std::string _amat = "flesh";
	const std::string * amat = &_amat;

	const char * wmat = "dagger";

	if(!io_target->armormaterial.empty()) {
		amat = &io_target->armormaterial;
	}

	if(io_target == entities.player()) {
		if(ValidIONum(player.equiped[EQUIP_SLOT_ARMOR])) {
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

	ARX_SOUND_PlayCollision(*amat, wmat, power, 1.f, projectile.position, io_source);

	dmgs *= backstab;
	dmgs -= dmgs * (absorb * ( 1.0f / 100 ));

	float chance = 100.f - (ac - attack);
	float dice = Random::getf(0.f, 100.f);

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

static EERIEPOLY * CheckArrowPolyCollision(const Vec3f & start, const Vec3f & end) {
	
	EERIE_TRI pol;
	pol.v[0] = start;
	pol.v[2] = end - Vec3f(2.f, 15.f, 2.f);
	pol.v[1] = end;

	// TODO copy-paste background tiles
	int tilex = int(end.x * ACTIVEBKG->Xmul);
	int tilez = int(end.z * ACTIVEBKG->Zmul);
	int radius = 2;
	
	int minx = std::max(tilex - radius, 0);
	int maxx = std::min(tilex + radius, ACTIVEBKG->Xsize - 1);
	int minz = std::max(tilez - radius, 0);
	int maxz = std::min(tilez + radius, ACTIVEBKG->Zsize - 1);
	
	for(int z = minz; z <= maxz; z++)
	for(int x = minx; x <= maxx; x++) {
		const EERIE_BKG_INFO & feg = ACTIVEBKG->fastdata[x][z];
		for(long l = 0; l < feg.nbpolyin; l++) {
			EERIEPOLY * ep = feg.polyin[l];

			if(ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL)) {
				continue;
			}

			EERIE_TRI pol2;
			pol2.v[0] = ep->v[0].p;
			pol2.v[1] = ep->v[1].p;
			pol2.v[2] = ep->v[2].p;

			if(Triangles_Intersect(pol2, pol)) {
				return ep;
			}

			if(ep->type & POLY_QUAD) {
				pol2.v[0] = ep->v[1].p;
				pol2.v[1] = ep->v[3].p;
				pol2.v[2] = ep->v[2].p;
				if(Triangles_Intersect(pol2, pol)) {
					return ep;
				}
			}

		}
	}

	return NULL;
}

static void CheckExp(const Projectile & projectile) {
	
	if((projectile.flags & ATO_FIERY) && !(projectile.flags & ATO_UNDERWATER)) {
		const Vec3f & pos = projectile.position;
		
		spawnFireHitParticle(pos, 0);
		PolyBoomAddScorch(pos);
		LaunchFireballBoom(pos, 10);
		DoSphericDamage(Sphere(pos, 50.f), 4.f * 2, DAMAGE_AREA, DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL, EntityHandle_Player);
		ARX_SOUND_PlaySFX(SND_SPELL_FIRE_HIT, &pos);
		ARX_NPC_SpawnAudibleSound(pos, entities.player());
		
		EERIE_LIGHT * light = dynLightCreate();
		if(light && g_framedelay > 0) {
			light->intensity = 3.9f;
			light->fallstart = 400.f;
			light->fallend   = 440.f;
			light->rgb = Color3f(1.f, .8f, .6f) - randomColor3f() * Color3f(.2f, .2f, .2f);
			light->pos = pos;
			light->ex_flaresize = 40.f;
			light->duration = ArxDurationMs(1500);
		}
	}
}

void ARX_THROWN_OBJECT_Render() {

	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetRenderState(Renderer::DepthTest, true);

	for(size_t i = 0; i < MAX_THROWN_OBJECTS; i++) {
		Projectile & projectile = g_projectiles[i];
		if(!(projectile.flags & ATO_EXIST))
			continue;

		TransformInfo t(projectile.position, projectile.quat);
		// Object has to be retransformed because arrows share the same object
		DrawEERIEInter_ModelTransform(projectile.obj, t);
		DrawEERIEInter_ViewProjectTransform(projectile.obj);
		DrawEERIEInter_Render(projectile.obj, t, NULL);

		if(projectile.m_trail) {
			projectile.m_trail->Render();
		}
	}
}

void ARX_THROWN_OBJECT_Manage(float time_offset)
{
	for(size_t i = 0; i < MAX_THROWN_OBJECTS; i++) {
		Projectile & projectile = g_projectiles[i];
		if(!(projectile.flags & ATO_EXIST))
			continue;

		{
		// Is Object Visible & Near ?

		EERIE_BKG_INFO * bkgData = getFastBackgroundData(projectile.position.x, projectile.position.z);

		if(!bkgData || !bkgData->treat) {
			continue;
		}

		// Now render object !
		if(!projectile.obj)
			continue;

		TransformInfo t(projectile.position, projectile.quat);
		DrawEERIEInter_ModelTransform(projectile.obj, t);

		if((projectile.flags & ATO_FIERY) && (projectile.flags & ATO_MOVING)
		   && !(projectile.flags & ATO_UNDERWATER)) {

			EERIE_LIGHT * light = dynLightCreate();
			if(light && g_framedelay > 0) {
				light->intensity = 1.f;
				light->fallstart = 100.f;
				light->fallend   = 240.f;
				light->rgb = Color3f(1.f, .8f, .6f) - randomColor3f() * Color3f(.2f, .2f, .2f);
				light->pos = projectile.position;
				light->ex_flaresize = 40.f;
				light->extras |= EXTRAS_FLARE;
				light->duration = ArxDurationMs(g_framedelay * 0.5f);
			}
			
			createObjFireParticles(projectile.obj, 6, 2, 180);
		}

		if(projectile.m_trail) {
			projectile.m_trail->SetNextPosition(projectile.position);
			projectile.m_trail->Update(time_offset);
		}

		if(projectile.flags & ATO_MOVING) {
			long need_kill = 0;
			float mod = time_offset * projectile.velocity;
			Vec3f original_pos = projectile.position;
			projectile.position.x += projectile.vector.x * mod;
			float gmod = 1.f - projectile.velocity;

			gmod = glm::clamp(gmod, 0.f, 1.f);

			projectile.position.y += projectile.vector.y * mod + (time_offset * gmod);
			projectile.position.z += projectile.vector.z * mod;

			CheckForIgnition(Sphere(original_pos, 10.f), 0, 2);

			Vec3f wpos = projectile.position;
			wpos.y += 20.f;
			EERIEPOLY * ep = EEIsUnderWater(wpos);

			if(projectile.flags & ATO_UNDERWATER) {
				if(!ep) {
					projectile.flags &= ~ATO_UNDERWATER;
					ARX_SOUND_PlaySFX(SND_PLOUF, &projectile.position);
				}
			} else if(ep) {
				projectile.flags |= ATO_UNDERWATER;
				ARX_SOUND_PlaySFX(SND_PLOUF, &projectile.position);
			}

			// Check for collision MUST be done after DRAWING !!!!
			long nbact = projectile.obj->actionlist.size();

			for(long j = 0; j < nbact; j++) {
				float rad = GetHitValue(projectile.obj->actionlist[j].name);

				if(rad == -1)
					continue;

				rad *= .5f;

				const Vec3f v0 = actionPointPosition(projectile.obj, projectile.obj->actionlist[j].idx);
				Vec3f dest = original_pos + projectile.vector * 95.f;
				Vec3f orgn = original_pos - projectile.vector * 25.f;
				EERIEPOLY * ep = CheckArrowPolyCollision(orgn, dest);

				if(ep) {
					ParticleSparkSpawn(v0, 14, SpawnSparkType_Default);
					CheckExp(projectile);

					if(ValidIONum(projectile.source))
						ARX_NPC_SpawnAudibleSound(v0, entities[projectile.source]);

					projectile.flags &= ~ATO_MOVING;
					projectile.velocity = 0.f;
					
					std::string bkg_material = "earth";

					if(ep && ep->tex && !ep->tex->m_texName.empty())
						bkg_material = GetMaterialString(ep->tex->m_texName);

					if(ValidIONum(projectile.source)) {
						char weapon_material[64] = "dagger";
						
						ARX_SOUND_PlayCollision(weapon_material, bkg_material, 1.f, 1.f, v0, entities[projectile.source]);
					}

					projectile.position = original_pos;
					j = 200;
				} else if(IsPointInField(v0)) {
					ParticleSparkSpawn(v0, 24, SpawnSparkType_Default);
					CheckExp(projectile);

					if (ValidIONum(projectile.source))
						ARX_NPC_SpawnAudibleSound(v0, entities[projectile.source]);

					projectile.flags &= ~ATO_MOVING;
					projectile.velocity = 0.f;
					
					if(ValidIONum(projectile.source)) {
						char weapon_material[64] = "dagger";
						char bkg_material[64] = "earth";
						
						ARX_SOUND_PlayCollision(weapon_material, bkg_material, 1.f, 1.f, v0, entities[projectile.source]);
					}

					projectile.position = original_pos;
					j = 200;
					need_kill = 1;
				} else {
					for(float precision = 0.5f; precision <= 6.f; precision += 0.5f) {
						Sphere sphere;
						sphere.origin = v0 + projectile.vector * precision * 4.5f;
						sphere.radius = rad + 3.f;

						std::vector<EntityHandle> sphereContent;

						if(CheckEverythingInSphere(sphere, projectile.source, EntityHandle(), sphereContent)) {
							for(size_t jj = 0; jj < sphereContent.size(); jj++) {

								if(ValidIONum(sphereContent[jj])
										&& sphereContent[jj] != projectile.source)
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

										if(projectile.source == EntityHandle_Player) {
											float damages = ARX_THROWN_ComputeDamages(projectile, projectile.source, sphereContent[jj]);

											if(damages > 0.f) {
												arx_assert(hitpoint >= 0);

												if(target->ioflags & IO_NPC) {
													target->_npcdata->SPLAT_TOT_NB = 0;
													ARX_PARTICLES_Spawn_Blood2(original_pos, damages, color, target);
												}

												ARX_PARTICLES_Spawn_Blood2(pos, damages, color, target);
												ARX_DAMAGES_DamageNPC(target, damages, projectile.source, false, &pos);

												if(Random::getf(0.f, 100.f) > target->_npcdata->resist_poison) {
													target->_npcdata->poisonned += projectile.poisonous;
												}

												CheckExp(projectile);
											} else {
												ParticleSparkSpawn(v0, 14, SpawnSparkType_Default);
												ARX_NPC_SpawnAudibleSound(v0, entities[projectile.source]);
											}
										}
									} else {
										// not NPC
										if(target->ioflags & IO_FIX) {
											if(ValidIONum(projectile.source))
												ARX_DAMAGES_DamageFIX(target, 0.1f, projectile.source, false);
										}

										ParticleSparkSpawn(v0, 14, SpawnSparkType_Default);

										if(ValidIONum(projectile.source))
											ARX_NPC_SpawnAudibleSound(v0, entities[projectile.source]);

										CheckExp(projectile);
									}

									// Need to deal damages !
									projectile.flags &= ~ATO_MOVING;
									projectile.velocity = 0.f;
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
