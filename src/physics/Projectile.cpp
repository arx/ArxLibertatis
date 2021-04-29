/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
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

#include <boost/foreach.hpp>

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

#include "graphics/Raycast.h"
#include "graphics/Renderer.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/Spark.h"
#include "graphics/effects/PolyBoom.h"
#include "graphics/effects/Trail.h"

#include "physics/Collisions.h"

#include "util/Flags.h"


const size_t MAX_THROWN_OBJECTS = 100;

static Projectile g_projectiles[MAX_THROWN_OBJECTS];

static bool IsPointInField(const Vec3f & pos) {

	for(size_t i = 0; i < MAX_SPELLS; i++) {
		const SpellBase * spell = spells[SpellHandle(i)];

		if(spell && spell->m_type == SPELL_CREATE_FIELD) {
			const CreateFieldSpell * sp = static_cast<const CreateFieldSpell *>(spell);
			
			Entity * pfrm = entities.get(sp->m_entity);
			if(pfrm) {
				Cylinder cyl = Cylinder(pos + Vec3f(0.f, 17.5f, 0.f), 35.f, -35.f);
				
				if(CylinderPlatformCollide(cyl, pfrm)) {
					return true;
				}
			}
		}
	}

	return false;
}

static void ARX_THROWN_OBJECT_Kill(size_t num) {
	if(num < MAX_THROWN_OBJECTS) {
		g_projectiles[num].obj = NULL;
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
	
	GameInstant latest_time = g_gameTime.now();
	size_t oldest = 0;
	
	for(size_t i = 0; i < MAX_THROWN_OBJECTS; i++) {
		if(!g_projectiles[i].obj) {
			return i;
		}
		if(g_projectiles[i].creation_time < latest_time) {
			         oldest = i;
			latest_time = g_projectiles[i].creation_time;
		}
	}
	
	ARX_THROWN_OBJECT_Kill(oldest);
	
	return oldest;
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
	projectile.m_trail->Update(g_gameTime.lastFrameDuration());
	
	projectile.creation_time = g_gameTime.now();
	projectile.flags = 0;
	
	if(source == EntityHandle_Player) {
		Entity * tio = entities.get(player.equiped[EQUIP_SLOT_WEAPON]);
		if(tio) {
			if(tio->ioflags & IO_FIERY)
				projectile.flags |= ATO_FIERY;
		}
	}
}

static float ARX_THROWN_ComputeDamages(const Projectile & projectile, EntityHandle target) {
	
	Entity * io_target = entities[target];
	
	SendIOScriptEvent(entities.player(), io_target, SM_AGGRESSION);
	
	float distance = fdist(projectile.position, projectile.initial_position);
	float distance_modifier = 1.f;
	const float distance_limit = 1000.f;
	if(distance < distance_limit * 2.f) {
		distance_modifier = distance / distance_limit;
		if(distance_modifier < 0.5f)
			distance_modifier = 0.5f;
	} else {
		distance_modifier = 2.f;
	}
	
	float critical = 1.f;
	if(Random::getf(0.f, 100.f) <= (player.m_attributeFull.dexterity - 9.f) * 2.f
	                               + player.m_skillFull.projectile * 0.2f) {
		if(SendIOScriptEvent(NULL, entities.player(), SM_CRITICAL, "bow") != REFUSE) {
			critical = 1.5f;
		}
	}
	
	float backstab = 1.f;
	if(io_target->_npcdata->npcflags & NPCFLAG_BACKSTAB) {
		if(Random::getf(0.f, 100.f) <= player.m_skillFull.stealth) {
			if(SendIOScriptEvent(NULL, entities.player(), SM_BACKSTAB, "bow") != REFUSE) {
				backstab = 1.5f;
			}
		}
	}
	
	float ac, absorb;
	if(target == EntityHandle_Player) {
		ac = player.m_miscFull.armorClass;
		absorb = player.m_skillFull.defense * .5f;
	} else {
		ac = ARX_INTERACTIVE_GetArmorClass(io_target);
		absorb = io_target->_npcdata->absorb;
	}
	
	std::string _amat = "flesh";
	const std::string * amat = &_amat;
	if(!io_target->armormaterial.empty()) {
		amat = &io_target->armormaterial;
	}
	if(io_target == entities.player()) {
		Entity * io = entities.get(player.equiped[EQUIP_SLOT_ARMOR]);
		if(io) {
			if(!io->armormaterial.empty()) {
				amat = &io->armormaterial;
			}
		}
	}
	
	float power = std::min(projectile.damages * 0.05f, 1.f) * 0.15f + 0.85f;
	ARX_SOUND_PlayCollision(*amat, "dagger", power, 1.f, projectile.position, entities.player());
	
	float dmgs = projectile.damages * critical * backstab * (1.f - absorb * 0.01f) * distance_modifier;
	
	if(dmgs <= 0.f || Random::getf(0.f, 100.f) < ac - projectile.damages) {
		return 0.f;
	}
	
	return dmgs;
}

static void CheckExp(const Projectile & projectile) {
	
	if((projectile.flags & ATO_FIERY) && !(projectile.flags & ATO_UNDERWATER)) {
		const Vec3f & pos = projectile.position;
		
		spawnFireHitParticle(pos, 0);
		PolyBoomAddScorch(pos);
		LaunchFireballBoom(pos, 10);
		DoSphericDamage(Sphere(pos, 50.f), 4.f * 2, DAMAGE_AREA, DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL, EntityHandle_Player);
		ARX_SOUND_PlaySFX(g_snd.SPELL_FIRE_HIT, &pos);
		ARX_NPC_SpawnAudibleSound(pos, entities.player());
		
		EERIE_LIGHT * light = dynLightCreate();
		if(light && g_gameTime.lastFrameDuration() > 0) {
			light->intensity = 3.9f;
			light->fallstart = 400.f;
			light->fallend   = 440.f;
			light->rgb = Color3f(1.f, .8f, .6f) - randomColor3f() * Color3f(.2f, .2f, .2f);
			light->pos = pos;
			light->ex_flaresize = 40.f;
			light->duration = GameDurationMs(1500);
		}
	}
}

void ARX_THROWN_OBJECT_Render() {
	
	for(size_t i = 0; i < MAX_THROWN_OBJECTS; i++) {
		Projectile & projectile = g_projectiles[i];
		
		if(!projectile.obj) {
			continue;
		}
		
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

static void ARX_THROWN_OBJECT_ManageProjectile(size_t i, GameDuration timeDelta) {
	
	float timeDeltaMs = toMsf(timeDelta);
	
	Projectile & projectile = g_projectiles[i];
	if(!projectile.obj) {
		return;
	}
	
	// Is Object Visible & Near ?
	
	if(!ACTIVEBKG->isInActiveTile(projectile.position)) {
		// Projectile got outside of the world
		ARX_THROWN_OBJECT_Kill(i);
		return;
	}
	
	if(projectile.velocity == 0.f) {
		if(projectile.m_trail) {
			projectile.m_trail->Update(timeDelta);
			if(projectile.m_trail->emtpy()) {
				delete projectile.m_trail;
				projectile.m_trail = NULL;
			}
		}
		return;
	}
	
	TransformInfo t(projectile.position, projectile.quat);
	DrawEERIEInter_ModelTransform(projectile.obj, t);
	
	if((projectile.flags & ATO_FIERY) && !(projectile.flags & ATO_UNDERWATER)) {
		EERIE_LIGHT * light = dynLightCreate();
		if(light && g_gameTime.lastFrameDuration() > 0) {
			light->intensity = 1.f;
			light->fallstart = 100.f;
			light->fallend   = 240.f;
			light->rgb = Color3f(1.f, .8f, .6f) - randomColor3f() * Color3f(.2f, .2f, .2f);
			light->pos = projectile.position;
			light->ex_flaresize = 40.f;
			light->extras |= EXTRAS_FLARE;
			light->duration = GameDurationMsf(g_framedelay * 0.5f);
		}
		createObjFireParticles(projectile.obj, 6, 2, 180);
	}
	
	if(projectile.m_trail) {
		projectile.m_trail->SetNextPosition(projectile.position);
		projectile.m_trail->Update(timeDelta);
	}
	
	float mod = timeDeltaMs * projectile.velocity;
	Vec3f original_pos = projectile.position;
	projectile.position.x += projectile.vector.x * mod;
	float gmod = 1.f - projectile.velocity;
	
	gmod = glm::clamp(gmod, 0.f, 1.f);
	
	projectile.position.y += projectile.vector.y * mod + (timeDeltaMs * gmod);
	projectile.position.z += projectile.vector.z * mod;
	
	CheckForIgnition(Sphere(original_pos, 10.f), false, 2);
	
	{
		Vec3f wpos = projectile.position;
		wpos.y += 20.f;
		EERIEPOLY * ep = EEIsUnderWater(wpos);
		if(projectile.flags & ATO_UNDERWATER) {
			if(!ep) {
				projectile.flags &= ~ATO_UNDERWATER;
				ARX_SOUND_PlaySFX(g_snd.PLOUF, &projectile.position);
			}
		} else if(ep) {
			projectile.flags |= ATO_UNDERWATER;
			ARX_SOUND_PlaySFX(g_snd.PLOUF, &projectile.position);
		}
	}
	
	// Check for collision MUST be done after DRAWING !!!!
	size_t nbact = projectile.obj->actionlist.size();
	
	for(size_t j = 0; j < nbact; j++) {
		
		float rad = GetHitValue(projectile.obj->actionlist[j].name);
		if(rad == -1.f) {
			continue;
		}
		rad *= .5f;
		
		const Vec3f v0 = actionPointPosition(projectile.obj, projectile.obj->actionlist[j].idx);
		
		Vec3f dest = original_pos + projectile.vector * 90.f;
		Vec3f orgn = original_pos - projectile.vector * 20.f;
		RaycastResult result = RaycastLine(orgn, dest, POLY_WATER | POLY_TRANS | POLY_NOCOL);
		if(result || IsPointInField(v0)) {
			
			ParticleSparkSpawn(v0, result ? 14 : 24, SpawnSparkType_Default);
			CheckExp(projectile);
			
			if(ValidIONum(projectile.source)) {
				ARX_NPC_SpawnAudibleSound(v0, entities[projectile.source]);
			}
			
			projectile.velocity = 0.f;
			
			if(ValidIONum(projectile.source)) {
				std::string bkg_material = "earth";
				if(result.hit && result.hit->tex && !result.hit->tex->m_texName.empty()) {
					bkg_material = GetMaterialString(result.hit->tex->m_texName);
				}
				ARX_SOUND_PlayCollision("dagger", bkg_material, 1.f, 1.f, v0, entities[projectile.source]);
			}
			
			projectile.position = original_pos;
			
			if(!result) {
				ARX_THROWN_OBJECT_Kill(i);
			}
			
			return;
		}
		
		for(size_t k = 1; k <= 12; k++) {
			float precision = float(k) * 0.5f;
			
			Sphere sphere;
			sphere.origin = v0 + projectile.vector * precision * 4.5f;
			sphere.radius = rad + 3.f;
			
			std::vector<EntityHandle> sphereContent;
			if(!CheckEverythingInSphere(sphere, projectile.source, EntityHandle(), sphereContent)) {
				continue;
			}
			
			bool need_kill = false;
			for(size_t jj = 0; jj < sphereContent.size(); jj++) {
				
				if(!ValidIONum(sphereContent[jj]) && sphereContent[jj] != projectile.source) {
					continue;
				}
				
				Entity & target = *entities[sphereContent[jj]];
				
				if(target.ioflags & IO_NPC) {
					
					long hitpoint = -1;
					float curdist = 999999.f;
					for(size_t ii = 0 ; ii < target.obj->facelist.size() ; ii++) {
						
						if(target.obj->facelist[ii].facetype & POLY_HIDE) {
							continue;
						}
						
						unsigned short vid = target.obj->facelist[ii].vid[0];
						float d = glm::distance(sphere.origin, target.obj->vertexWorldPositions[vid].v);
						if(d < curdist) {
							hitpoint = target.obj->facelist[ii].vid[0];
							curdist = d;
						}
						
					}
					
					if(projectile.source == EntityHandle_Player) {
						
						float damages = ARX_THROWN_ComputeDamages(projectile, sphereContent[jj]);
						if(damages > 0.f) {
							
							arx_assert(hitpoint >= 0);
							Color color = target._npcdata->blood_color;
							Vec3f pos = target.obj->vertexWorldPositions[hitpoint].v;
							
							if(target.ioflags & IO_NPC) {
								target._npcdata->SPLAT_TOT_NB = 0;
								ARX_PARTICLES_Spawn_Blood2(original_pos, damages, color, &target);
							}
							
							ARX_PARTICLES_Spawn_Blood2(pos, damages, color, &target);
							ARX_DAMAGES_DamageNPC(&target, damages, projectile.source, false, &pos);
							
							if(Random::getf(0.f, 100.f) > target._npcdata->resist_poison) {
								target._npcdata->poisonned += projectile.poisonous;
							}
							
							CheckExp(projectile);
							
						} else {
							ParticleSparkSpawn(v0, 14, SpawnSparkType_Default);
							ARX_NPC_SpawnAudibleSound(v0, entities[projectile.source]);
						}
						
					}
					
				} else { // not NPC
					
					if((target.ioflags & IO_FIX) && ValidIONum(projectile.source)) {
						ARX_DAMAGES_DamageFIX(&target, 0.1f, projectile.source, false);
					}
					
					ParticleSparkSpawn(v0, 14, SpawnSparkType_Default);
					
					if(ValidIONum(projectile.source)) {
						ARX_NPC_SpawnAudibleSound(v0, entities[projectile.source]);
					}
					
					CheckExp(projectile);
					
				}
				
				projectile.velocity = 0.f;
				
				need_kill = true;
				
			}
			
			if(need_kill) {
				ARX_THROWN_OBJECT_Kill(i);
				return;
			}
			
		}
		
	}
	
}

void ARX_THROWN_OBJECT_Manage(GameDuration timeDelta) {
	for(size_t i = 0; i < MAX_THROWN_OBJECTS; i++) {
		ARX_THROWN_OBJECT_ManageProjectile(i, timeDelta);
	}
}
