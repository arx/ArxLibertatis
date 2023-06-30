/*
 * Copyright 2011-2022 Arx Libertatis Team (see the AUTHORS file)
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

#include <memory>
#include <string_view>

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
#include "scene/Tiles.h"

#include "animation/AnimationRender.h"

#include "graphics/Raycast.h"
#include "graphics/Renderer.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/Spark.h"
#include "graphics/effects/Decal.h"
#include "graphics/effects/Trail.h"

#include "physics/Collisions.h"

#include "platform/Platform.h"

#include "util/Flags.h"
#include "util/Range.h"


enum ProjectileFlag : u8 {
	ATO_UNDERWATER = 1 << 0,
	ATO_FIERY      = 1 << 1
};
DECLARE_FLAGS(ProjectileFlag, ProjectileFlags)
DECLARE_FLAGS_OPERATORS(ProjectileFlags)

struct alignas(16) Projectile {
	
	Vec3f vector = Vec3f(0.f);
	float gravity = 0.f;
	Vec3f initial_position = Vec3f(0.f);
	float damages = 0.f;
	Vec3f position = Vec3f(0.f);
	float poisonous = 0.f;
	
	glm::quat quat = quat_identity();
	glm::quat rotation = quat_identity();
	
	EERIE_3DOBJ * obj = nullptr;
	std::unique_ptr<Trail> m_trail;
	
	EntityHandle source;
	VertexId attach;
	ProjectileFlags flags;
	
	Projectile() arx_noexcept_default
	
};

static std::vector<Projectile> g_projectiles;

static bool IsPointInField(const Vec3f & pos) {
	
	for(const Spell & spell : spells.ofType(SPELL_CREATE_FIELD)) {
		if(Entity * field = entities.get(static_cast<const CreateFieldSpell &>(spell).m_entity)) {
			Cylinder cyl = Cylinder(pos + Vec3f(0.f, 17.5f, 0.f), 35.f, -35.f);
			if(isCylinderCollidingWithPlatform(cyl, *field)) {
				return true;
			}
		}
	}
	
	return false;
}

void ARX_THROWN_OBJECT_KillAll() {
	g_projectiles.clear();
}

glm::quat getProjectileQuatFromVector(Vec3f vector) {
	Anglef angle = vectorToAngle(vector);
	return glm::quat(glm::vec3(glm::radians(-angle.getPitch()), glm::radians(-angle.getYaw()), 0.f));
}

void ARX_THROWN_OBJECT_Throw(EntityHandle source, const Vec3f & position, const Vec3f & vect, float gravity,
                             EERIE_3DOBJ * obj, VertexId attach, const glm::quat & rotation,
                             float damages, float poisonous) {
	
	arx_assert(obj);
	
	Projectile & projectile = g_projectiles.emplace_back();
	
	projectile.damages = damages;
	projectile.position = position;
	projectile.initial_position = position;
	projectile.vector = vect;
	projectile.quat = getProjectileQuatFromVector(vect) * rotation;
	projectile.gravity = gravity;
	projectile.source = source;
	projectile.obj = obj;
	projectile.rotation = rotation;
	projectile.attach = attach;
	projectile.poisonous = poisonous;
	
	projectile.m_trail = std::make_unique<ArrowTrail>();
	projectile.m_trail->SetNextPosition(projectile.position);
	projectile.m_trail->Update(g_gameTime.lastFrameDuration());
	
	projectile.flags = 0;
	
	if(source == EntityHandle_Player) {
		Entity * tio = entities.get(player.equiped[EQUIP_SLOT_WEAPON]);
		if(tio && (tio->ioflags & IO_FIERY)) {
			projectile.flags |= ATO_FIERY;
		}
	}
	
}

static float ARX_THROWN_ComputeDamages(const Projectile & projectile, Entity & target) {
	
	SendIOScriptEvent(entities.player(), &target, SM_AGGRESSION);
	
	float distance = fdist(projectile.position, projectile.initial_position);
	float distance_modifier = 1.f;
	const float distance_limit = 1000.f;
	if(distance < distance_limit * 2.f) {
		distance_modifier = distance / distance_limit;
		if(distance_modifier < 0.5f) {
			distance_modifier = 0.5f;
		}
	} else {
		distance_modifier = 2.f;
	}
	
	float critical = 1.f;
	if(Random::getf(0.f, 100.f) <= (player.m_attributeFull.dexterity - 9.f) * 2.f
	                               + player.m_skillFull.projectile * 0.2f) {
		if(SendIOScriptEvent(nullptr, entities.player(), SM_CRITICAL, "bow") != REFUSE) {
			critical = 1.5f;
		}
	}
	
	float backstab = 1.f;
	if(target._npcdata->npcflags & NPCFLAG_BACKSTAB) {
		if(Random::getf(0.f, 100.f) <= player.m_skillFull.stealth) {
			if(SendIOScriptEvent(nullptr, entities.player(), SM_BACKSTAB, "bow") != REFUSE) {
				backstab = 1.5f;
			}
		}
	}
	
	float ac, absorb;
	if(&target == entities.player()) {
		ac = player.m_miscFull.armorClass;
		absorb = player.m_skillFull.defense * .5f;
	} else {
		ac = ARX_INTERACTIVE_GetArmorClass(&target);
		absorb = target._npcdata->absorb;
	}
	
	std::string_view amat = "flesh";
	if(!target.armormaterial.empty()) {
		amat = target.armormaterial;
	}
	if(&target == entities.player()) {
		Entity * io = entities.get(player.equiped[EQUIP_SLOT_ARMOR]);
		if(io) {
			if(!io->armormaterial.empty()) {
				amat = io->armormaterial;
			}
		}
	}
	
	float power = std::min(projectile.damages * 0.05f, 1.f) * 0.15f + 0.85f;
	ARX_SOUND_PlayCollision(amat, "dagger", power, 1.f, projectile.position, entities.player());
	
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
		doSphericDamage(Sphere(pos, 50.f), 4.f * 2, DAMAGE_AREA, nullptr,
		                DAMAGE_TYPE_FAKESPELL | DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL, entities.player());
		ARX_SOUND_PlaySFX(g_snd.SPELL_FIRE_HIT, &pos);
		spawnAudibleSound(pos, *entities.player());
		
		EERIE_LIGHT * light = dynLightCreate();
		if(light && g_gameTime.lastFrameDuration() > 0) {
			light->intensity = 3.9f;
			light->fallstart = 400.f;
			light->fallend   = 440.f;
			light->rgb = Color3f(1.f, .8f, .6f) - randomColor3f() * Color3f(.2f, .2f, .2f);
			light->pos = pos;
			light->ex_flaresize = 40.f;
			light->duration = 1500ms;
		}
	}
}

void ARX_THROWN_OBJECT_Render() {
	
	for(Projectile & projectile : g_projectiles) {
		
		arx_assert(projectile.obj);
		
		TransformInfo t(projectile.position, projectile.quat);
		t.pos = t(projectile.obj->vertexlist[projectile.obj->origin].v
		          - projectile.obj->vertexlist[projectile.attach].v);
		// Object has to be retransformed because arrows share the same object
		DrawEERIEInter_ModelTransform(projectile.obj, t);
		DrawEERIEInter_ViewProjectTransform(projectile.obj);
		DrawEERIEInter_Render(projectile.obj, t, nullptr);
		
		if(projectile.m_trail) {
			projectile.m_trail->Render();
		}
		
	}
	
}

static void ARX_THROWN_OBJECT_ManageProjectile(Projectile & projectile, ShortGameDuration timeDelta) {
	
	arx_assume(timeDelta >= 0 && timeDelta <= GameTime::MaxFrameDuration);
	
	arx_assert(projectile.obj);
	
	float timeDeltaMs = toMsf(timeDelta);
	
	// Is Object Visible & Near ?
	
	if(!g_tiles->isInActiveTile(projectile.position)) {
		// Projectile got outside of the world
		projectile.obj = nullptr;
		return;
	}
	
	if(projectile.vector == Vec3f(0.f)) {
		if(projectile.m_trail) {
			projectile.m_trail->Update(timeDelta);
			if(projectile.m_trail->emtpy()) {
				projectile.m_trail = nullptr;
			}
		}
		return;
	}
	
	TransformInfo t(projectile.position, projectile.quat);
	t.pos = t(projectile.obj->vertexlist[projectile.obj->origin].v
	          - projectile.obj->vertexlist[projectile.attach].v);
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
			light->duration = std::chrono::duration<float, std::milli>(g_framedelay * 0.5f);
		}
		createObjFireParticles(projectile.obj, 6, 2, 180ms);
	}
	
	Vec3f original_pos = projectile.position;
	projectile.position += projectile.vector * timeDeltaMs;
	
	if(projectile.gravity != 0.f) {
		projectile.vector.y += projectile.gravity * timeDeltaMs;
		projectile.quat = getProjectileQuatFromVector(projectile.vector) * projectile.rotation;
	}
	
	igniteLights(Sphere(original_pos, 10.f), false, true);
	igniteEntities(Sphere(original_pos, 10.f), false);
	
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
	for(EERIE_ACTIONLIST & action : projectile.obj->actionlist) {
		
		float rad = GetHitValue(action.name);
		if(rad == -1.f) {
			continue;
		}
		rad *= .5f;
		
		const Vec3f v0 = projectile.obj->vertexWorldPositions[action.idx].v;
		
		RaycastResult result = raycastScene(original_pos, v0, POLY_WATER | POLY_TRANS | POLY_NOCOL);
		if(result || IsPointInField(v0)) {
			
			ParticleSparkSpawn(v0, result ? 14 : 24);
			CheckExp(projectile);
			
			if(Entity * source  = entities.get(projectile.source)) {
				spawnAudibleSound(v0, *source);
				std::string_view bkg_material = "earth";
				if(result.hit && result.hit->tex && !result.hit->tex->m_texName.empty()) {
					bkg_material = GetMaterialString(result.hit->tex->m_texName);
				}
				ARX_SOUND_PlayCollision("dagger", bkg_material, 1.f, 1.f, v0, source);
			}
			
			if(result) {
				// TODO better offset calculation
				projectile.position = original_pos + result.pos - v0;
				projectile.vector = Vec3f(0.f);
			} else {
				projectile.obj = nullptr;
				return;
			}
			
			break;
		}
		
		for(size_t k = 1; k <= 12; k++) {
			float precision = float(k) * 0.5f;
			
			Sphere sphere;
			sphere.origin = v0 + projectile.vector * precision * 4.5f;
			sphere.radius = rad + 3.f;
			
			std::vector<Entity *> sphereContent;
			if(!CheckEverythingInSphere(sphere, entities.get(projectile.source), nullptr, sphereContent)) {
				continue;
			}
			
			bool need_kill = false;
			for(Entity & target : util::dereference(sphereContent)) {
				
				if(target.index() == projectile.source) {
					continue;
				}
				
				if(target.ioflags & IO_NPC) {
					
					VertexId hitpoint;
					float curdist = 999999.f;
					for(size_t ii = 0 ; ii < target.obj->facelist.size() ; ii++) {
						
						if(target.obj->facelist[ii].facetype & POLY_HIDE) {
							continue;
						}
						
						VertexId vid = target.obj->facelist[ii].vid[0];
						float d = glm::distance(sphere.origin, target.obj->vertexWorldPositions[vid].v);
						if(d < curdist) {
							hitpoint = target.obj->facelist[ii].vid[0];
							curdist = d;
						}
						
					}
					
					if(projectile.source == EntityHandle_Player) {
						
						float damages = ARX_THROWN_ComputeDamages(projectile, target);
						if(damages > 0.f) {
							
							Color color = target._npcdata->blood_color;
							Vec3f pos = target.obj->vertexWorldPositions[hitpoint].v;
							
							target._npcdata->SPLAT_TOT_NB = 0;
							ARX_PARTICLES_Spawn_Blood2(original_pos, damages, color, &target);
							
							ARX_PARTICLES_Spawn_Blood2(pos, damages, color, &target);
							damageNpc(target, damages, entities.get(projectile.source), nullptr, DAMAGE_TYPE_METAL, &pos);
							
							if(Random::getf(0.f, 100.f) > target._npcdata->resist_poison) {
								target._npcdata->poisonned += projectile.poisonous;
							}
							
							CheckExp(projectile);
							
						} else {
							spawnAudibleSound(v0, *entities.player());
							ParticleSparkSpawn(v0, 14);
						}
						
					}
					
				} else { // not NPC
					
					if(Entity * source = entities.get(projectile.source)) {
						if(target.ioflags & IO_FIX) {
							damageProp(target, 0.1f, source, nullptr, DAMAGE_TYPE_METAL);
						}
						spawnAudibleSound(v0, *source);
					}
					
					ParticleSparkSpawn(v0, 14);
					
					CheckExp(projectile);
					
				}
				
				projectile.vector = Vec3f(0.f);
				
				need_kill = true;
				
			}
			
			if(need_kill) {
				projectile.obj = nullptr;
				return;
			}
			
		}
		
	}
	
	if(projectile.m_trail) {
		projectile.m_trail->SetNextPosition(projectile.position);
		projectile.m_trail->Update(timeDelta);
	}
	
}

void ARX_THROWN_OBJECT_Manage(ShortGameDuration timeDelta) {
	
	for(Projectile & projectile : g_projectiles) {
		ARX_THROWN_OBJECT_ManageProjectile(projectile, timeDelta);
	}
	
	util::unordered_remove_if(g_projectiles, [](const Projectile & projectile) { return !projectile.obj; });
	
}
