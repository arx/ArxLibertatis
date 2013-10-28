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
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved

#include "game/Damage.h"

#include <cstring>
#include <cstdio>
#include <algorithm>
#include <limits>
#include <string>
#include <vector>

#include "ai/Paths.h"

#include "core/GameTime.h"
#include "core/Core.h"

#include "game/EntityManager.h"
#include "game/Equipment.h"
#include "game/Inventory.h"
#include "game/Item.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "game/Spells.h"

#include "gui/Speech.h"
#include "gui/Interface.h"

#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Color.h"
#include "graphics/Draw.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"
#include "graphics/Renderer.h"
#include "graphics/Vertex.h"
#include "graphics/data/Mesh.h"
#include "graphics/particle/ParticleEffects.h"

#include "io/resource/ResourcePath.h"

#include "math/Random.h"

#include "physics/Collisions.h"

#include "scene/GameSound.h"
#include "scene/Light.h"
#include "scene/Interactive.h"

#include "script/Script.h"

class TextureContainer;

using std::min;
using std::max;

extern long REFUSE_GAME_RETURN;

DAMAGE_INFO	damages[MAX_DAMAGES];
extern Vec3f PUSH_PLAYER_FORCE;

float Blood_Pos = 0.f;
long Blood_Duration = 0;

static void ARX_DAMAGES_IgnitIO(Entity * io, float dmg)
{
	if(!io || (io->ioflags & IO_INVULNERABILITY))
		return;

	if(io->ignition <= 0.f && io->ignition + dmg > 1.f)
		SendIOScriptEvent(io, SM_ENTERZONE, "cook_s");

	if(io->ioflags & IO_FIX)
		io->ignition += dmg * ( 1.0f / 10 );
	else if(io->ioflags & IO_ITEM)
		io->ignition += dmg * ( 1.0f / 8 );
	else if(io->ioflags & IO_NPC)
		io->ignition += dmg * ( 1.0f / 4 );
}
 
void ARX_DAMAGE_Reset_Blood_Info()
{
	Blood_Pos = 0.f;
	Blood_Duration = 0;
}

void ARX_DAMAGE_Show_Hit_Blood()
{
	Color color;
	static float Last_Blood_Pos = 0.f;
	static long duration;

	if(Blood_Pos > 2.f) { // end of blood flash
		Blood_Pos = 0.f;
		duration = 0;
	} else if (Blood_Pos > 1.f) {
		GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendSrcColor);
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		GRenderer->SetRenderState(Renderer::DepthWrite, false);

		if(player.poison > 1.f)
			color = Color3f(Blood_Pos - 1.f, 1.f, Blood_Pos - 1.f).to<u8>();
		else
			color = Color3f(1.f, Blood_Pos - 1.f, Blood_Pos - 1.f).to<u8>();

		EERIEDrawBitmap(g_size, 0.00009f, NULL, color);
		GRenderer->SetRenderState(Renderer::DepthWrite, true);
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	} else if(Blood_Pos > 0.f) {
		GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendSrcColor);
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		GRenderer->SetRenderState(Renderer::DepthWrite, false);

		if(player.poison > 1.f)
			color = Color3f(1.f - Blood_Pos, 1.f, 1.f - Blood_Pos).to<u8>();
		else
			color = Color3f(1.f, 1.f - Blood_Pos, 1.f - Blood_Pos).to<u8>();

		EERIEDrawBitmap(g_size, 0.00009f, NULL, color);
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		GRenderer->SetRenderState(Renderer::DepthWrite, true);
	}

	if(Blood_Pos > 0.f) {
		if(Blood_Pos > 1.f) {
			if(Last_Blood_Pos <= 1.f) {
				Blood_Pos = 1.0001f;
				duration = 0;
			}

			if(duration > Blood_Duration)
				Blood_Pos += (float)framedelay * ( 1.0f / 300 );

			duration += static_cast<long>(framedelay);
		}
		else
			Blood_Pos += (float)framedelay * ( 1.0f / 40 );
	}

	Last_Blood_Pos = Blood_Pos;
}

float ARX_DAMAGES_DamagePlayer(float dmg, DamageType type, long source) {
	if (player.playerflags & PLAYERFLAGS_INVULNERABILITY)
		return 0;

	float damagesdone = 0.f;

	if(player.life == 0.f)
		return damagesdone;

	if(dmg > player.life)
		damagesdone = dmg;
	else
		damagesdone = player.life;

	entities.player()->dmg_sum += dmg;

	if(float(arxtime) > entities.player()->ouch_time + 500) {
		Entity * oes = EVENT_SENDER;

		if(ValidIONum(source))
			EVENT_SENDER = entities[source];
		else
			EVENT_SENDER = NULL;

		entities.player()->ouch_time = (unsigned long)(arxtime);
		char tex[32];
		sprintf(tex, "%5.2f", entities.player()->dmg_sum);
		SendIOScriptEvent( entities.player(), SM_OUCH, tex );
		EVENT_SENDER = oes;
		float power = entities.player()->dmg_sum / player.maxlife * 220.f;
		AddQuakeFX(power * 3.5f, 500 + power * 3, rnd() * 100.f + power + 200, 0);
		entities.player()->dmg_sum = 0.f;
	}

	if(dmg > 0.f) {
		if(ValidIONum(source)) {
			Entity * pio = NULL;

			if(entities[source]->ioflags & IO_NPC) {
				pio = entities[source]->_npcdata->weapon;

				if(pio && (pio->poisonous == 0 || pio->poisonous_count == 0))
					pio = NULL;
			}

			if(!pio)
				pio = entities[source];

			if(pio && pio->poisonous && pio->poisonous_count != 0) {
				if(rnd() * 100.f > player.resist_poison) {
					player.poison += pio->poisonous;
				}

				if(pio->poisonous_count != -1)
					pio->poisonous_count--;
			}
		}

		long alive;

		if(player.life > 0)
			alive = 1;
		else
			alive = 0;

		if(!BLOCK_PLAYER_CONTROLS)
			player.life -= dmg;

		if(player.life <= 0.f) {
			player.life = 0.f;

			if(alive) {
				//REFUSE_GAME_RETURN = 1;
				ARX_PLAYER_BecomesDead();

				if((type & DAMAGE_TYPE_FIRE) || (type & DAMAGE_TYPE_FAKEFIRE)) {
					ARX_SOUND_PlayInterface(SND_PLAYER_DEATH_BY_FIRE);
				}

				SendIOScriptEvent(entities.player(), SM_DIE);

				for(size_t i = 1; i < entities.size(); i++) {
					Entity * ioo = entities[i];
					if(ioo && (ioo->ioflags & IO_NPC)) {
						if(ioo->targetinfo == TARGET_PLAYER) {
							EVENT_SENDER = entities.player();
							std::string killer;
							if(source == 0) {
								killer = "player";
							} else if(source <= -1) {
								killer = "none";
							} else if(ValidIONum(source)) {
								killer = entities[source]->long_name();
							}
							SendIOScriptEvent(entities[i], SM_NULL, killer, "target_death");
						}
					}
				}
			}
		}

		if(player.maxlife <= 0.f)
			return damagesdone;

		float t = dmg / player.maxlife;

		if(Blood_Pos == 0.f) {
			Blood_Pos = 0.000001f;
			Blood_Duration = 100 + (t * 200.f);
		} else {
			long temp = t * 800.f;
			Blood_Duration += temp;
		}
	}

	// revient les barres
	ResetPlayerInterface();

	return damagesdone;
}

void ARX_DAMAGES_HealPlayer(float dmg)
{
	if(player.life == 0.f)
		return;

	if(dmg > 0.f) {
		if(!BLOCK_PLAYER_CONTROLS)
			player.life += dmg;

		if(player.life > player.Full_maxlife)
			player.life = player.Full_maxlife;
	}
}

void ARX_DAMAGES_HealInter(Entity * io, float dmg)
{
	if(!io || !(io->ioflags & IO_NPC))
		return;

	if(io->_npcdata->life <= 0.f)
		return;

	if(io == entities.player())
		ARX_DAMAGES_HealPlayer(dmg);

	if(dmg > 0.f) {
		io->_npcdata->life += dmg;

		if(io->_npcdata->life > io->_npcdata->maxlife)
			io->_npcdata->life = io->_npcdata->maxlife;
	}
}

void ARX_DAMAGES_HealManaPlayer(float dmg)
{
	if(player.life == 0.f)
		return;

	if(dmg > 0.f) {
		player.mana += dmg;

		if(player.mana > player.Full_maxmana)
			player.mana = player.Full_maxmana;
	}
}

void ARX_DAMAGES_HealManaInter(Entity * io, float dmg)
{
	if(!io || !(io->ioflags & IO_NPC))
		return;

	if(io == entities.player())
		ARX_DAMAGES_HealManaPlayer(dmg);

	if(io->_npcdata->life <= 0.f)
		return;

	if(dmg > 0.f) {
		io->_npcdata->mana += dmg;

		if(io->_npcdata->mana > io->_npcdata->maxmana)
			io->_npcdata->mana = io->_npcdata->maxmana;
	}
}

float ARX_DAMAGES_DrainMana(Entity * io, float dmg)
{
	if(!io || !(io->ioflags & IO_NPC))
		return 0;

	if(io == entities.player()) {
		if(player.playerflags & PLAYERFLAGS_NO_MANA_DRAIN)
			return 0;

		if(player.mana >= dmg) {
			player.mana -= dmg;
			return dmg;
		}

		float d = player.mana;
		player.mana = 0;
		return d;
	}

	if(io->_npcdata->mana >= dmg) {
		io->_npcdata->mana -= dmg;
		return dmg;
	}

	float d = io->_npcdata->mana;
	io->_npcdata->mana = 0;
	return d;
}

void ARX_DAMAGES_DamageFIX(Entity * io, float dmg, long source, long flags)
{
	if ((!io)
	        ||	(!io->show)
	        ||	(!(io->ioflags & IO_FIX))
	        ||	(io->ioflags & IO_INVULNERABILITY)
	        ||	(!io->script.data))
		return;

	io->dmg_sum += dmg;

	if (ValidIONum(source))
		EVENT_SENDER = entities[source];
	else
		EVENT_SENDER = NULL;

	if(float(arxtime) > io->ouch_time + 500) {
		io->ouch_time = (unsigned long)(arxtime);
		char tex[32];
		sprintf(tex, "%5.2f", io->dmg_sum);
		SendIOScriptEvent(io, SM_OUCH, tex);
		io->dmg_sum = 0.f;
	}

	if(rnd() * 100.f > io->durability)
		io->durability -= dmg * ( 1.0f / 2 ); //1.f;

	if(io->durability <= 0.f) {
		io->durability = 0.f;
		SendIOScriptEvent(io, SM_BREAK);
	} else {
		char dmm[32];

		if(EVENT_SENDER == entities.player()) {
			if(flags & 1) {
				sprintf(dmm, "%f spell", dmg);
			}
			else
				switch(ARX_EQUIPMENT_GetPlayerWeaponType())
				{
					case WEAPON_BARE:
						sprintf(dmm, "%f bare", dmg);
						break;
					case WEAPON_DAGGER:
						sprintf(dmm, "%f dagger", dmg);
						break;
					case WEAPON_1H:
						sprintf(dmm, "%f 1h", dmg);
						break;
					case WEAPON_2H:
						sprintf(dmm, "%f 2h", dmg);
						break;
					case WEAPON_BOW:
						sprintf(dmm, "%f arrow", dmg);
						break;
					default:
						sprintf(dmm, "%f", dmg);
						break;
				}
		}
		else
			sprintf(dmm, "%f", dmg);

		if(SendIOScriptEvent(io, SM_HIT, dmm) != ACCEPT)
			return;
	}
}

extern Entity * FlyingOverIO;
extern MASTER_CAMERA_STRUCT MasterCamera;

void ARX_DAMAGES_ForceDeath(Entity * io_dead, Entity * io_killer) {
	
	if(io_dead->mainevent == "dead") {
		return;
	}

	Entity * old_sender = EVENT_SENDER;
	EVENT_SENDER = io_killer;

	if(io_dead == DRAGINTER)
		Set_DragInter(NULL);

	if(io_dead == FlyingOverIO)
		FlyingOverIO = NULL;

	if((MasterCamera.exist & 1) && (MasterCamera.io == io_dead))
		MasterCamera.exist = 0;

	if((MasterCamera.exist & 2) && (MasterCamera.want_io == io_dead))
		MasterCamera.exist = 0;

	//Kill all speeches
	if(ValidDynLight(io_dead->dynlight))
		DynLight[io_dead->dynlight].exist = 0;

	io_dead->dynlight = -1;

	if(ValidDynLight(io_dead->halo.dynlight))
		DynLight[io_dead->halo.dynlight].exist = 0;

	io_dead->halo.dynlight = -1;
	ARX_NPC_Behaviour_Reset(io_dead);

	ARX_SPEECH_ReleaseIOSpeech(io_dead);

	//Kill all Timers...
	ARX_SCRIPT_Timer_Clear_By_IO(io_dead);

	if(io_dead->mainevent != "dead") {
		if(SendIOScriptEvent(io_dead, SM_DIE) != REFUSE && ValidIOAddress(io_dead)) {
			io_dead->infracolor = Color3f::blue;
		}
	}
	
	if (!ValidIOAddress(io_dead))
		return;

	ARX_SCRIPT_SetMainEvent(io_dead, "dead");

	if(fartherThan(io_dead->pos, ACTIVECAM->orgTrans.pos, 3200.f)) {
		io_dead->animlayer[0].ctime = 9999999;
		io_dead->animBlend.lastanimtime = 0;
	}

	std::string killer;

	if(io_dead->ioflags & IO_NPC)
		io_dead->_npcdata->weaponinhand = 0;

	ARX_INTERACTIVE_DestroyDynamicInfo(io_dead);

	if(io_killer == entities.player()) {
		killer = "player";
	} else {
		if(io_killer)
			killer = io_killer->long_name();
	}

	for(size_t i = 1; i < entities.size(); i++) {
		Entity * ioo = entities[i];

		if(ioo == io_dead)
			continue;

		if(ioo && (ioo->ioflags & IO_NPC)) {
			if(ValidIONum(ioo->targetinfo))
				if(entities[ioo->targetinfo] == io_dead) {
					EVENT_SENDER = io_dead; 
					Stack_SendIOScriptEvent(entities[i], SM_NULL, killer, "target_death");
					ioo->targetinfo = TARGET_NONE;
					ioo->_npcdata->reachedtarget = 0;
				}

			if(ValidIONum(ioo->_npcdata->pathfind.truetarget))
				if(entities[ioo->_npcdata->pathfind.truetarget] == io_dead) {
					EVENT_SENDER = io_dead; 
					Stack_SendIOScriptEvent(entities[i], SM_NULL, killer, "target_death");
					ioo->_npcdata->pathfind.truetarget = TARGET_NONE;
					ioo->_npcdata->reachedtarget = 0;
				}
		}
	}

	IO_UnlinkAllLinkedObjects(io_dead);
	io_dead->animlayer[1].cur_anim = NULL;
	io_dead->animlayer[2].cur_anim = NULL;
	io_dead->animlayer[3].cur_anim = NULL;

	if(io_dead->ioflags & IO_NPC) {
		io_dead->_npcdata->life = 0;

		if(io_dead->_npcdata->weapon) {
			Entity * ioo = io_dead->_npcdata->weapon;
			if(ValidIOAddress(ioo)) {
				ioo->show = SHOW_FLAG_IN_SCENE;
				ioo->ioflags |= IO_NO_NPC_COLLIDE;
				ioo->pos = ioo->obj->vertexlist3[ioo->obj->origin].v;
				ioo->velocity = Vec3f(0.f, 13.f, 0.f);
				ioo->stopped = 0;
			}
		}
	}

	EVENT_SENDER = old_sender;
}

void ARX_DAMAGES_PushIO(Entity * io_target, long source, float power)
{
	if(power > 0.f && ValidIONum(source)) {
		power *= ( 1.0f / 20 );
		Entity * io = entities[source];
		Vec3f vect = io_target->pos - io->pos;
		fnormalize(vect);
		vect *= power;
		if(io_target == entities.player()) {
			PUSH_PLAYER_FORCE = vect; // TODO why not +=?
		} else {
			io_target->move += vect;
		}
	}
}

float ARX_DAMAGES_DealDamages(long target, float dmg, long source, DamageType flags, Vec3f * pos)
{
	if(!ValidIONum(target) || !ValidIONum(source))
		return 0;

	Entity * io_target = entities[target];
	Entity * io_source = entities[source];
	float damagesdone;

	if(flags & DAMAGE_TYPE_PER_SECOND) {
		dmg = dmg * framedelay * ( 1.0f / 1000 );
	}

	if(target == 0) {
		if(flags & DAMAGE_TYPE_POISON) {
			if(rnd() * 100.f > player.resist_poison) {
				damagesdone = dmg;
				player.poison += damagesdone;
			} else {
				damagesdone = 0;
			}
		} else {
			if(flags & DAMAGE_TYPE_DRAIN_MANA) {
				damagesdone = ARX_DAMAGES_DrainMana(io_target, dmg);
			} else {
				ARX_DAMAGES_DamagePlayerEquipment(dmg);
				damagesdone = ARX_DAMAGES_DamagePlayer(dmg, flags, source);
			}
		}

		if(flags & DAMAGE_TYPE_FIRE)
			ARX_DAMAGES_IgnitIO(io_target, damagesdone);

		if(flags & DAMAGE_TYPE_DRAIN_LIFE)
			ARX_DAMAGES_HealInter(io_source, damagesdone);

		if(flags & DAMAGE_TYPE_DRAIN_MANA)
			ARX_DAMAGES_HealManaInter(io_source, damagesdone);

		if(flags & DAMAGE_TYPE_PUSH)
			ARX_DAMAGES_PushIO(io_target, source, damagesdone * ( 1.0f / 2 ));

		if((flags & DAMAGE_TYPE_MAGICAL) && !(flags & (DAMAGE_TYPE_FIRE | DAMAGE_TYPE_COLD))) {
			damagesdone -= player.Full_resist_magic * ( 1.0f / 100 ) * damagesdone;
			damagesdone = max(0.0f, damagesdone);
		}

		return damagesdone;
	} else {
		if(io_target->ioflags & IO_NPC) {
			if(flags & DAMAGE_TYPE_POISON) {
				if(rnd() * 100.f > io_target->_npcdata->resist_poison) {
					damagesdone = dmg;
					io_target->_npcdata->poisonned += damagesdone;
				} else {
					damagesdone = 0;
				}
			} else {
				if(flags & DAMAGE_TYPE_FIRE) {
					if(rnd() * 100.f <= io_target->_npcdata->resist_fire)
						dmg = 0;

					ARX_DAMAGES_IgnitIO(io_target, dmg);
				}

				if(flags & DAMAGE_TYPE_DRAIN_MANA) {
					damagesdone = ARX_DAMAGES_DrainMana(io_target, dmg);
				} else {
					damagesdone = ARX_DAMAGES_DamageNPC(io_target, dmg, source, 1, pos);
				}
			}

			if(flags & DAMAGE_TYPE_DRAIN_LIFE)
				ARX_DAMAGES_HealInter(io_source, damagesdone);

			if(flags & DAMAGE_TYPE_DRAIN_MANA)
				ARX_DAMAGES_HealManaInter(io_source, damagesdone);

			if(flags & DAMAGE_TYPE_PUSH)
				ARX_DAMAGES_PushIO(io_target, source, damagesdone * ( 1.0f / 2 ));

			if((flags & DAMAGE_TYPE_MAGICAL) && !(flags & (DAMAGE_TYPE_FIRE | DAMAGE_TYPE_COLD))) {
				damagesdone -= io_target->_npcdata->resist_magic * ( 1.0f / 100 ) * damagesdone;
				damagesdone = max(0.0f, damagesdone);
			}

			return damagesdone;
		}
	}

	return 0;
}

extern bool bHitFlash;
extern float fHitFlash;
extern unsigned long ulHitFlash;

//*************************************************************************************
// flags & 1 == spell damage
//*************************************************************************************
float ARX_DAMAGES_DamageNPC(Entity * io, float dmg, long source, long flags, Vec3f * pos) {
	
	if(!io || !io->show || (io->ioflags & IO_INVULNERABILITY) || !(io->ioflags & IO_NPC))
		return 0.f;

	float damagesdone = 0.f;

	if(io->_npcdata->life <= 0.f) {
		if(source != 0 || (source == 0 && player.equiped[EQUIP_SLOT_WEAPON] > 0)) {
			if(dmg >= io->_npcdata->maxlife * 0.4f && pos)
				ARX_NPC_TryToCutSomething(io, pos);

			return damagesdone;
		}

		return damagesdone;
	}

	io->dmg_sum += dmg;

	if(float(arxtime) > io->ouch_time + 500) {
		if(ValidIONum(source))
			EVENT_SENDER = entities[source];
		else
			EVENT_SENDER = NULL;

		io->ouch_time = (unsigned long)(arxtime);
		char tex[32];

		if(EVENT_SENDER && EVENT_SENDER->summoner == 0) {
			EVENT_SENDER = entities.player();
			sprintf(tex, "%5.2f summoned", io->dmg_sum);
		} else {
			sprintf(tex, "%5.2f", io->dmg_sum);
		}

		SendIOScriptEvent(io, SM_OUCH, tex);
		io->dmg_sum = 0.f;
		long n = ARX_SPELLS_GetSpellOn(io, SPELL_CONFUSE);

		if(n >= 0)
			spells[n].tolive = 0;
	}

	if(dmg >= 0.f) {
		if(ValidIONum(source)) {
			Entity * pio = NULL;

			if(source == 0) {
				if(player.equiped[EQUIP_SLOT_WEAPON] != 0 && ValidIONum(player.equiped[EQUIP_SLOT_WEAPON])) {
					pio = entities[player.equiped[EQUIP_SLOT_WEAPON]];

					if((pio && (pio->poisonous == 0 || pio->poisonous_count == 0)) || (flags & 1)) {
						pio = NULL;
					}
				}
			} else {
				if(entities[source]->ioflags & IO_NPC) {
					pio = entities[source]->_npcdata->weapon;
					if(pio && (pio->poisonous == 0 || pio->poisonous_count == 0)) {
						pio = NULL;
					}
				}
			}

			if(!pio)
				pio = entities[source];

			if(pio && pio->poisonous && (pio->poisonous_count != 0)) {
				if(rnd() * 100.f > io->_npcdata->resist_poison) {
					io->_npcdata->poisonned += pio->poisonous;
				}

				if (pio->poisonous_count != -1)
					pio->poisonous_count--;
			}
		}

		if(io->script.data != NULL) {
			if(source >= 0) {
				if(ValidIONum(source))
					EVENT_SENDER = entities[source]; 
				else
					EVENT_SENDER = NULL;

				char dmm[256];

				if(EVENT_SENDER == entities.player()) {
					if(flags & 1) {
						sprintf(dmm, "%f spell", dmg);
					}
					else
						switch (ARX_EQUIPMENT_GetPlayerWeaponType()) {
							case WEAPON_BARE:
								sprintf(dmm, "%f bare", dmg);
								break;
							case WEAPON_DAGGER:
								sprintf(dmm, "%f dagger", dmg);
								break;
							case WEAPON_1H:
								sprintf(dmm, "%f 1h", dmg);
								break;
							case WEAPON_2H:
								sprintf(dmm, "%f 2h", dmg);
								break;
							case WEAPON_BOW:
								sprintf(dmm, "%f arrow", dmg);
								break;
							default:
								sprintf(dmm, "%f", dmg);
								break;
						}
				}
				else
					sprintf(dmm, "%f", dmg);

				if(EVENT_SENDER && EVENT_SENDER->summoner == 0) {
					EVENT_SENDER = entities.player();
					sprintf(dmm, "%f summoned", dmg);
				}

				if(SendIOScriptEvent(io, SM_HIT, dmm) != ACCEPT)
					return damagesdone;
			}
		}

		damagesdone = min(dmg, io->_npcdata->life);
		io->_npcdata->life -= dmg;

		bHitFlash = true;

		if(io->_npcdata->life <= 0) {
			fHitFlash = 0;
		} else {
			fHitFlash = io->_npcdata->life / io->_npcdata->maxlife;
		}

		ulHitFlash = 0;

		if(io->_npcdata->life <= 0.f) {
			io->_npcdata->life = 0.f;

			if(source != 0 || (source == 0 && player.equiped[EQUIP_SLOT_WEAPON] > 0)) {
				if((dmg >= io->_npcdata->maxlife * ( 1.0f / 2 )) && pos)
					ARX_NPC_TryToCutSomething(io, pos);
			}

			if(ValidIONum(source)) {
				long xp = io->_npcdata->xpvalue;
				ARX_DAMAGES_ForceDeath(io, entities[source]);

				if(source == 0)
					ARX_PLAYER_Modify_XP(xp);
			}
			else ARX_DAMAGES_ForceDeath(io, NULL);
		}
	}

	return damagesdone;
}

void ARX_DAMAGES_Reset()
{
	memset(damages, 0, sizeof(DAMAGE_INFO)*MAX_DAMAGES);
}

long ARX_DAMAGES_GetFree()
{
	for(size_t i = 0; i < MAX_DAMAGES; i++) {
		if(!damages[i].exist) {
			damages[i].radius = 100.f;
			damages[i].start_time = (unsigned long)(arxtime); 
			damages[i].duration = 1000;
			damages[i].area = DAMAGE_AREA;
			damages[i].flags = 0;
			damages[i].type = 0;
			damages[i].special = 0;
			damages[i].special_ID = 0;
			damages[i].lastupd = 0;
			damages[i].active = 1;

			for(long j = 0; j < 10; j++)
				damages[i].except[j] = -1;

			return i;
		}
	}

	return -1;
}

long InExceptList(long dmg, long num)
{
	for(long j = 0; j < 10; j++)
		if(damages[dmg].except[j] == num)
			return 1;

	return 0;
}

void ARX_DAMAGES_AddVisual(DAMAGE_INFO * di, Vec3f * pos, float dmg, Entity * io) {
	
	if(!(di->type & DAMAGE_TYPE_FAKEFIRE)) {
		return;
	}
	
	long num = -1;
	if(io) {
		num = Random::get(0, io->obj->vertexlist.size() / 4 - 1) * 4 + 1;
	}
	
	unsigned long tim = (unsigned long)(arxtime);
	if(di->lastupd + 200 < tim) {
		di->lastupd = tim;
		if(di->type & DAMAGE_TYPE_MAGICAL) {
			ARX_SOUND_PlaySFX(SND_SPELL_MAGICAL_HIT, pos, 0.8F + 0.4F * rnd());
		} else {
			ARX_SOUND_PlaySFX(SND_SPELL_FIRE_HIT, pos, 0.8F + 0.4F * rnd());
		}
	}
	
	for(long k = 0 ; k < 14 ; k++) {
		
		PARTICLE_DEF * pd = createParticle();
		if(!pd) {
			break;
		}
		
		if(io) {
			arx_assert(num >= 0);
			pd->ov = io->obj->vertexlist3[num].v + randomVec(-5.f, 5.f);
		} else {
			pd->ov = *pos + randomVec(-50.f, 50.f);
		}
		pd->siz = clamp(dmg, 5.f, 15.f);
		pd->scale = Vec3f(-10.f);
		pd->special = ROTATING | MODULATE_ROTATION | FIRE_TO_SMOKE;
		pd->tolive = Random::get(500, 900);
		pd->move = Vec3f(1.f - 2.f * rnd(), 2.f - 16.f * rnd(), 1.f - 2.f * rnd());
		if(di->type & DAMAGE_TYPE_MAGICAL) {
			pd->rgb = Color3f(0.3f, 0.3f, 0.8f);
		} else {
			pd->rgb = Color3f::gray(0.5f);
		}
		pd->tc = TC_fire2;
		pd->fparam = 0.1f - rnd() * 0.2f;
	}
}

// source = -1 no source but valid pos
// source = 0  player
// source > 0  IO
void ARX_DAMAGES_UpdateDamage(long j, float tim) {
	
	Vec3f sub;

	if(damages[j].exist) {
		if(!damages[j].active)
			return;
		
		if(damages[j].flags & DAMAGE_FLAG_FOLLOW_SOURCE) {
			if(damages[j].source == 0) {
				damages[j].pos = player.pos;
			} else if (ValidIONum(damages[j].source)) {
				damages[j].pos = entities[damages[j].source]->pos;
			}
		}
		
		float dmg;
		if (damages[j].flags & DAMAGE_NOT_FRAME_DEPENDANT)
			dmg = damages[j].damages;
		else if(damages[j].duration == -1)
			dmg = damages[j].damages;
		else
		{
			float FD = (float)framedelay;

			if(tim > damages[j].start_time + damages[j].duration)
				FD -= damages[j].start_time + damages[j].duration - tim;

			dmg = damages[j].damages * FD * ( 1.0f / 1000 );
		}

		bool validsource = ValidIONum(damages[j].source);
		float divradius = 1.f / damages[j].radius;

		// checking for IO damages
		for(size_t i = 0; i < entities.size(); i++) {
			Entity * io = entities[i];

			if ((io)
			        &&	(io->gameFlags & GFLAG_ISINTREATZONE)
			        &&	(io->show == SHOW_FLAG_IN_SCENE)
			        &&	(!InExceptList(j, i))
			        && ( (damages[j].source != long(i))
			            || ((damages[j].source == long(i))
			                && (!(damages[j].flags & DAMAGE_FLAG_DONT_HURT_SOURCE))))
			   )
			{
				if(io->ioflags & IO_NPC) {
					if ((i != 0) && (damages[j].source != 0)
					        && validsource && (HaveCommonGroup(io, entities[damages[j].source])))
						continue;

					EERIE_SPHERE sphere;
					sphere.origin = damages[j].pos;
					sphere.radius = damages[j].radius - 10.f;

					if(CheckIOInSphere(&sphere, i, true)) {
						sub.x = io->pos.x;
						sub.y = io->pos.y - 60.f;
						sub.z = io->pos.z;
						float dist = fdist(damages[j].pos, sub);

						if(damages[j].type & DAMAGE_TYPE_FIELD) {
							if(float(arxtime) > io->collide_door_time + 500) {
								EVENT_SENDER = NULL;
								io->collide_door_time = (unsigned long)(arxtime); 
								char param[64];
								param[0] = 0;

								if (damages[j].type & DAMAGE_TYPE_FIRE)
									strcpy(param, "fire");

								if (damages[j].type & DAMAGE_TYPE_COLD)
									strcpy(param, "cold");

								SendIOScriptEvent(io, SM_COLLIDE_FIELD, param);

							}
						}

						switch(damages[j].area) {
							case DAMAGE_AREA:
							{
								float ratio = (damages[j].radius - dist) * divradius;

								ratio = clamp(ratio, 0.f, 1.f);

								dmg = dmg * ratio + 1.f;
							}
							break;
							case DAMAGE_AREAHALF:
							{
								float ratio = (damages[j].radius - (dist * ( 1.0f / 2 ))) * divradius;

								ratio = clamp(ratio, 0.f, 1.f);

								dmg = dmg * ratio + 1.f;
							}
							break;
							case DAMAGE_FULL: break;
						}

						if(dmg <= 0.f)
							continue;

						if(damages[j].flags & DAMAGE_FLAG_ADD_VISUAL_FX) {
							if ((entities[i]->ioflags & IO_NPC)
							        &&	(entities[i]->_npcdata->life > 0.f))
								ARX_DAMAGES_AddVisual(&damages[j], &sub, dmg, entities[i]);
						}

						if(damages[j].type & DAMAGE_TYPE_DRAIN_MANA) {
							float manadrained;

							if(i == 0) {
								manadrained = min(dmg, player.mana);
								player.mana -= manadrained;
							} else {
								manadrained = dmg;

								if(io && io->_npcdata) {
									manadrained = min(dmg, io->_npcdata->mana);
									io->_npcdata->mana -= manadrained;
								}
							}

							if (damages[j].source == 0) {
								player.mana = min(player.mana + manadrained, player.Full_maxmana);
							} else {
								if(ValidIONum(damages[j].source) && (entities[damages[j].source]->_npcdata)) {
									entities[damages[j].source]->_npcdata->mana = min(entities[damages[j].source]->_npcdata->mana + manadrained, entities[damages[j].source]->_npcdata->maxmana);
								}
							}
						} else {
							float damagesdone;

							if(i == 0) {
								if(damages[j].type & DAMAGE_TYPE_POISON) {
									if(rnd() * 100.f > player.resist_poison) {
										// Failed Saving Throw
										damagesdone = dmg; 
										player.poison += damagesdone;

									}
									else
										damagesdone = 0;
								} else {
									if ((damages[j].type & DAMAGE_TYPE_MAGICAL)
									        &&	(!(damages[j].type & DAMAGE_TYPE_FIRE))
									        &&	(!(damages[j].type & DAMAGE_TYPE_COLD))
									   )
									{
										dmg -= player.Full_resist_magic * ( 1.0f / 100 ) * dmg;
										dmg = max(0.0f, dmg);
									}

									if(damages[j].type & DAMAGE_TYPE_FIRE) {
										dmg = ARX_SPELLS_ApplyFireProtection(entities.player(), dmg);
										ARX_DAMAGES_IgnitIO(entities.player(), dmg);
									}

									if(damages[j].type & DAMAGE_TYPE_COLD) {
										dmg = ARX_SPELLS_ApplyColdProtection(entities.player(), dmg);
									}

									damagesdone = ARX_DAMAGES_DamagePlayer(dmg, damages[j].type, damages[j].source);
								}
							} else {
								if ((entities[i]->ioflags & IO_NPC)
								        && (damages[j].type & DAMAGE_TYPE_POISON))
								{
									if(rnd() * 100.f > entities[i]->_npcdata->resist_poison) {
										// Failed Saving Throw
										damagesdone = dmg; 
										entities[i]->_npcdata->poisonned += damagesdone;
									}
									else damagesdone = 0;
								} else {
									if(damages[j].type & DAMAGE_TYPE_FIRE) {
										dmg = ARX_SPELLS_ApplyFireProtection(entities[i], dmg);
										ARX_DAMAGES_IgnitIO(entities[i], dmg);
									}

									if ((damages[j].type & DAMAGE_TYPE_MAGICAL)
									        && (!(damages[j].type & DAMAGE_TYPE_FIRE))
									        && (!(damages[j].type & DAMAGE_TYPE_COLD))
									   )
									{
										dmg -= entities[i]->_npcdata->resist_magic * ( 1.0f / 100 ) * dmg;
										dmg = max(0.0f, dmg);
									}

									if(damages[j].type & DAMAGE_TYPE_COLD) {
										dmg = ARX_SPELLS_ApplyColdProtection(entities[i], dmg);
									}

									damagesdone = ARX_DAMAGES_DamageNPC(entities[i], dmg, damages[j].source, 1, &damages[j].pos);
								}

								if(damagesdone > 0 && (damages[j].flags & DAMAGE_SPAWN_BLOOD)) {
									ARX_PARTICLES_Spawn_Blood(&damages[j].pos, damagesdone, damages[j].source);
								}
							}

							if(damages[j].type & DAMAGE_TYPE_DRAIN_LIFE) {
								if(ValidIONum(damages[j].source))
									ARX_DAMAGES_HealInter(entities[damages[j].source], damagesdone);
							}
						}
					}
				} else if((io->ioflags & IO_FIX) && !(damages[j].type & DAMAGE_TYPE_NO_FIX)) {
					EERIE_SPHERE sphere;
					sphere.origin = damages[j].pos;
					sphere.radius = damages[j].radius + 15.f;

					if(CheckIOInSphere(&sphere, i)) {
						ARX_DAMAGES_DamageFIX(io, dmg, damages[j].source, 1);
					}
				}
			}
		}

		if(damages[j].duration == -1)
			damages[j].exist = false;
		else if(tim > damages[j].start_time + damages[j].duration)
			damages[j].exist = false;
	}
}

void ARX_DAMAGES_UpdateAll()
{
	for (size_t j = 0; j < MAX_DAMAGES; j++)
		ARX_DAMAGES_UpdateDamage(j, arxtime);
}

bool SphereInIO(Entity * io, Vec3f * pos, float radius)
{
	if(!io || !io->obj)
		return false;

	long step;

	if (io->obj->vertexlist.size() < 150) step = 1;
	else if (io->obj->vertexlist.size() < 300) step = 2;
	else if (io->obj->vertexlist.size() < 600) step = 4;
	else if (io->obj->vertexlist.size() < 1200) step = 6;
	else step = 7;

	for(size_t i = 0; i < io->obj->vertexlist.size(); i += step) {
		if(!fartherThan(*pos, io->obj->vertexlist3[i].v, radius)) {
			return true;
		}
	}
	
	return false;
}

bool ARX_DAMAGES_TryToDoDamage(Vec3f * pos, float dmg, float radius, long source)
{
	bool ret = false;

	for(size_t i = 0; i < entities.size(); i++) {
		Entity * io = entities[i];

		if(io != NULL
		   && (entities[i]->gameFlags & GFLAG_ISINTREATZONE)
		   && io->show == SHOW_FLAG_IN_SCENE
		   && source != long(i)
		) {
			float threshold;
			float rad = radius + 5.f;

			if(io->ioflags & IO_FIX) {
				threshold = 510;
				rad += 10.f;
			} else if(io->ioflags & IO_NPC) {
				threshold = 250;
			} else {
				threshold = 350;
			}

			if(closerThan(*pos, io->pos, threshold) && SphereInIO(io, pos, rad)) {
				if(io->ioflags & IO_NPC) {
					if(ValidIONum(source))
						ARX_EQUIPMENT_ComputeDamages(entities[source], io, 1.f);

					ret = true;
				}

				if(io->ioflags & IO_FIX) {
					ARX_DAMAGES_DamageFIX(io, dmg, source, 0);
					ret = true;
				}
			}
		}
	}

	return ret;
}

void CheckForIgnition(Vec3f * pos, float radius, bool mode, long flag) {
	
	if(!(flag & 1))
		for(size_t i = 0; i < MAX_LIGHTS; i++) {
			EERIE_LIGHT * el = GLight[i];

			if(el == NULL)
				continue;

			if((el->extras & EXTRAS_EXTINGUISHABLE) && (el->extras & (EXTRAS_SEMIDYNAMIC | EXTRAS_SPAWNFIRE | EXTRAS_SPAWNSMOKE)))
			{
				if((el->extras & EXTRAS_FIREPLACE) && (flag & 2))
					continue;

				if(!fartherThan(*pos, el->pos, radius)) {
					if(mode) {
						if (!(el->extras & EXTRAS_NO_IGNIT))
							el->status = 1;
					} else {
						el->status = 0;
					}
				}

			}
		}

	for(size_t i = 0; i < entities.size(); i++) {
		Entity * io = entities[i];

		if(io && io->show == 1 && io->obj && !(io->ioflags & IO_UNDERWATER) && io->obj->fastaccess.fire >= 0) {
			
			if(closerThan(*pos, io->obj->vertexlist3[io->obj->fastaccess.fire].v, radius)) {

				if(mode && io->ignition <= 0 && io->obj->fastaccess.fire >= 0) {
					io->ignition = 1;
				} else if(!mode && io->ignition > 0) {
					if(io->obj->fastaccess.fire >= 0) {
						io->ignition = 0; 

						if(ValidDynLight(io->ignit_light))
							DynLight[io->ignit_light].exist = 0;

						io->ignit_light = -1;

						if(io->ignit_sound != audio::INVALID_ID) {
							ARX_SOUND_Stop(io->ignit_sound);
							io->ignit_sound = audio::INVALID_ID;
						}
					}
					else if(!(flag & 2))
						io->ignition = 0.00001f;
				}
			}
		}
	}
}

bool DoSphericDamage(Vec3f * pos, float dmg, float radius, DamageArea flags, DamageType typ, long numsource)
{
	bool damagesdone = false;
	Vec3f sub;
	sub.x = player.pos.x;
	sub.y = player.pos.y + 90.f;
	sub.z = player.pos.z;

	if(radius <= 0.f)
		return damagesdone;

	float rad = 1.f / radius;
	bool validsource = ValidIONum(numsource);

	for(size_t i = 0; i < entities.size(); i++) {
		Entity * ioo = entities[i];

		if((ioo) && (long(i) != numsource) && (ioo->obj)) {
			if ((i != 0) && (numsource != 0)
			        && validsource && (HaveCommonGroup(ioo, entities[numsource])))
				continue;

			if((ioo->ioflags & IO_CAMERA) || (ioo->ioflags & IO_MARKER))
				continue;

			long count = 0;
			long count2 = 0;
			float mindist = std::numeric_limits<float>::max();

			for(size_t k = 0; k < ioo->obj->vertexlist.size(); k += 1) {
				if(ioo->obj->vertexlist.size() < 120) {
					for(size_t kk = 0; kk < ioo->obj->vertexlist.size(); kk += 1) {
						if(kk != k) {
							Vec3f posi = (entities[i]->obj->vertexlist3[k].v
							              + entities[i]->obj->vertexlist3[kk].v) * 0.5f;
							float dist = fdist(*pos, posi);
							if(dist <= radius) {
								count2++;
								if(dist < mindist)
									mindist = dist;
							}
						}
					}
				}

				{
					float dist = fdist(*pos, entities[i]->obj->vertexlist3[k].v);

					if(dist <= radius) {
						count++;

						if(dist < mindist)
							mindist = dist;
					}
				}
			}

			float ratio = ((float)count / ((float)ioo->obj->vertexlist.size() * ( 1.0f / 2 )));

			if(count2 > count)
				ratio = ((float)count2 / ((float)ioo->obj->vertexlist.size() * ( 1.0f / 2 )));

			if(ratio > 2.f)
				ratio = 2.f;

			if(ioo->ioflags & IO_NPC) {
				if(mindist <= radius + 30.f) {
					switch (flags) {
						case DAMAGE_AREA:
							dmg = dmg * (radius + 30 - mindist) * rad;
							break;
						case DAMAGE_AREAHALF:
							dmg = dmg * (radius + 30 - mindist * ( 1.0f / 2 )) * rad;
							break;
						case DAMAGE_FULL: break;
					}

					if(i == 0) {
						if(typ & DAMAGE_TYPE_FIRE) {
							dmg = ARX_SPELLS_ApplyFireProtection(ioo, dmg);
							ARX_DAMAGES_IgnitIO(entities.player(), dmg);
						}

						if(typ & DAMAGE_TYPE_COLD) {
							dmg = ARX_SPELLS_ApplyColdProtection(ioo, dmg);
						}

						ARX_DAMAGES_DamagePlayer(dmg, typ, numsource);
						ARX_DAMAGES_DamagePlayerEquipment(dmg);
					} else {
						if(typ & DAMAGE_TYPE_FIRE) {
							dmg = ARX_SPELLS_ApplyFireProtection(ioo, dmg * ratio);
							ARX_DAMAGES_IgnitIO(ioo, dmg);
						}

						if(typ & DAMAGE_TYPE_COLD) {
							dmg = ARX_SPELLS_ApplyColdProtection(ioo, dmg * ratio);
						}

						ARX_DAMAGES_DamageNPC(ioo, dmg * ratio, numsource, 1, pos);
					}

					if(dmg > 1)
						damagesdone = true;
				}
			} else {
				if(mindist <= radius + 30.f) {
					if(typ & DAMAGE_TYPE_FIRE) {
						dmg = ARX_SPELLS_ApplyFireProtection(ioo, dmg * ratio);
						ARX_DAMAGES_IgnitIO(entities[i], dmg);
					}

					if(typ & DAMAGE_TYPE_COLD) {
						dmg = ARX_SPELLS_ApplyColdProtection(ioo, dmg * ratio);
					}

					if(entities[i]->ioflags & IO_FIX)
						ARX_DAMAGES_DamageFIX(entities[i], dmg * ratio, numsource, 1);

					if(dmg > 0.2f)
						damagesdone = true;
				}
			}
		}
	}

	if (typ & DAMAGE_TYPE_FIRE)
		CheckForIgnition(pos, radius, 1);

	return damagesdone;
}

void ARX_DAMAGES_DurabilityRestore(Entity * io, float percent)
{
	if(!io)
		return;

	if (io->durability <= 0) return;

	if (io->durability == io->max_durability) return;

	if (percent >= 100.f) {
		io->durability = io->max_durability;
	} else {
		float ratio			= percent * ( 1.0f / 100 );
		float to_restore	= (io->max_durability - io->durability) * ratio;
		float v				= rnd() * 100.f - percent;

		if (v <= 0.f) {
			float mloss = 1.f;

			if(io->ioflags & IO_ITEM) {
				io->_itemdata->price -= checked_range_cast<long>(io->_itemdata->price / io->max_durability);
			}

			io->max_durability -= mloss;
		} else {
			if (v > 50.f)
				v = 50.f;

			v *= ( 1.0f / 100 );
			float mloss = io->max_durability * v;

			if(io->ioflags & IO_ITEM) {
				io->_itemdata->price -= static_cast<long>(io->_itemdata->price * v);
			}

			io->max_durability -= mloss;
		}

		io->durability += to_restore;

		if(io->durability > io->max_durability)
			io->durability = io->max_durability;

		if(io->max_durability <= 0.f)
			ARX_DAMAGES_DurabilityLoss(io, 100);
	}

}

void ARX_DAMAGES_DurabilityCheck(Entity * io, float ratio)
{
	if(!io)
		return;

	if(rnd() * 100.f > io->durability) {
		ARX_DAMAGES_DurabilityLoss(io, ratio);
	}
}

void ARX_DAMAGES_DurabilityLoss(Entity * io, float loss)
{
	if(!io)
		return;

	io->durability -= loss;

	if(io->durability <= 0) {
		SendIOScriptEvent(io, SM_BREAK);
	}
}

void ARX_DAMAGES_DamagePlayerEquipment(float damages)
{
	float ratio = damages * ( 1.0f / 20 );

	if(ratio > 1.f)
		ratio = 1.f;

	for(long i = 0; i < MAX_EQUIPED; i++) {
		if(player.equiped[i] != 0) {
			Entity * todamage = entities[player.equiped[i]];
			ARX_DAMAGES_DurabilityCheck(todamage, ratio);
		}
	}
}

float ARX_DAMAGES_ComputeRepairPrice(Entity * torepair, Entity * blacksmith)
{
	if(!torepair || !blacksmith) return -1.f;

	if(!(torepair->ioflags & IO_ITEM)) return -1.f;

	if(torepair->max_durability <= 0.f) return -1.f;

	if(torepair->durability == torepair->max_durability) return -1.f;

	float ratio = (torepair->max_durability - torepair->durability) / torepair->max_durability;
	float price = torepair->_itemdata->price * ratio;

	if(blacksmith->shop_multiply != 0.f)
		price *= blacksmith->shop_multiply;

	if(price > 0.f && price < 1.f)
		price = 1.f;

	return price;
}
