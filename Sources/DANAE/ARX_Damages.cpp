/*
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
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
// ARX_Damages
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Damages management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>

#include "ARX_Damages.h"
#include "HERMESMain.h"

#include "EERIEpoly.h"
#include "EERIELinkedObj.h"
#include "EERIELight.h"
#include "EERIEDRAW.h"
#include "EERIEPoly.h"

#include "ARX_Player.h"
#include "ARX_NPC.h"
#include "ARX_Sound.h"
#include "ARX_Speech.h"
#include "ARX_Collisions.h"
#include "ARX_Particles.h"
#include "ARX_Equipment.h"
#include "ARX_Interface.h"
#include "ARX_Paths.h"
#include "ARX_script.h"
#include "ARX_time.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

extern long REFUSE_GAME_RETURN;


typedef struct
{
	long	exist;
	float	x;
	float	y;
	float	size;
	TextureContainer * tc;
	long	duration;
	unsigned long starttime;
} SCREEN_SPLATS;

#define MAX_SCREEN_SPLATS 12
DAMAGE_INFO	damages[MAX_DAMAGES];
extern long ParticleCount;
extern EERIE_3D PUSH_PLAYER_FORCE;

float Blood_Pos = 0.f;
long Blood_Duration = 0;
SCREEN_SPLATS ssplat[MAX_SCREEN_SPLATS];
void ARX_DAMAGES_IgnitIO(INTERACTIVE_OBJ * io, float dmg)
{
	if ((!io)
	        ||	(io->ioflags & IO_INVULNERABILITY))
		return;

	if ((io->ignition <= 0.f) && (io->ignition + dmg > 1.f))
	{
		SendIOScriptEvent(io, SM_ENTERZONE, "COOK_S", NULL);
	}

	if (io->ioflags & IO_FIX) io->ignition += dmg * DIV10;
	else if (io->ioflags & IO_ITEM) io->ignition += dmg * DIV8;
	else if (io->ioflags & IO_NPC) io->ignition += dmg * DIV4;
}
void ARX_DAMAGES_SCREEN_SPLATS_Init()
{
	for (long i = 0; i < MAX_SCREEN_SPLATS; i++)
		ssplat[i].exist = 0;
}
 
extern TextureContainer * bloodsplat[6];
void ARX_DAMAGES_SCREEN_SPLATS_Add(EERIE_3D * pos, float dmgs)
{
	return;
	long j = ARX_PARTICLES_GetFree();

	if ((j != -1) && (!ARXPausedTimer))
	{
		D3DCOLOR col = inter.iobj[0]->_npcdata->blood_color;
		D3DTLVERTEX in, out;
		in.sx = pos->x;
		in.sy = pos->y;
		in.sz = pos->z;
		EERIETreatPoint(&in, &out);


		if (out.sx < 0)
			out.sx = 0;
		else if (out.sx > DANAESIZX)
			out.sx = ARX_CLEAN_WARN_CAST_D3DVALUE(DANAESIZX);




		if (out.sy < 0)
			out.sy = 0;
		else if (out.sy > DANAESIZY)
			out.sy = ARX_CLEAN_WARN_CAST_D3DVALUE(DANAESIZY);



		float power;
		power = (dmgs * DIV60) + 0.9f;
		float r, g, b;
		r = (float)((long)((col >> 16) & 255)) * DIV255;
		g = (float)((long)((col >> 8) & 255)) * DIV255;
		b = (float)((long)((col) & 255)) * DIV255;
		ParticleCount++;
		PARTICLE_DEF * pd = &particle[j];
		pd->special			=	PARTICLE_SUB2 | SUBSTRACT;
		pd->exist			=	TRUE;
		pd->zdec			=	0;
		pd->ov.x			=	out.sx;
		pd->ov.y			=	out.sy;
		pd->ov.z			=	0.0000001f;
		pd->move.x			=	0.f;
		pd->move.y			=	0.f;
		pd->move.z			=	0.f;
		pd->scale.x			=	1.8f;
		pd->scale.y			=	1.8f;
		pd->scale.z			=	1.f;
		pd->timcreation		=	lARXTime;
		pd->tolive			=	1800 + (unsigned long)(rnd() * 400.f);
		long num;
		F2L((float)(rnd() * 6.f), &num);

		if (num < 0) num = 0;
		else if (num > 5) num = 5;

		pd->tc = bloodsplat[num];
		pd->r = r;
		pd->g = g;
		pd->b = b;
		pd->siz = 3.5f * power * 40 * Xratio;
		pd->type = PARTICLE_2D;
	}
}

void ARX_DAMAGE_Reset_Blood_Info()
{
	Blood_Pos = 0.f;
	Blood_Duration = 0;
}

void ARX_DAMAGE_Show_Hit_Blood(LPDIRECT3DDEVICE7 pd3dDevice)
{
	D3DCOLOR color;
	static float Last_Blood_Pos = 0.f;
	static long duration;

	if (Blood_Pos > 2.f) // end of blood flash
	{
		Blood_Pos = 0.f;
		duration = 0;
	}
	else if (Blood_Pos > 1.f)
	{
		pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO);
		pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCCOLOR);
		SETALPHABLEND(pd3dDevice, TRUE);
		SETZWRITE(pd3dDevice, FALSE);

		if (player.poison > 1.f)
			color = D3DRGB(Blood_Pos - 1.f, 1.f, Blood_Pos - 1.f);
		else
			color = D3DRGB(1.f, Blood_Pos - 1.f, Blood_Pos - 1.f);

		EERIEDrawBitmap(pd3dDevice, 0.f, 0.f, (float)DANAESIZX, (float)DANAESIZY, 0.00009f, NULL, color);
		SETZWRITE(pd3dDevice, TRUE);
		SETALPHABLEND(pd3dDevice, FALSE);
	}
	else if (Blood_Pos > 0.f)
	{
		pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO);
		pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCCOLOR);
		SETALPHABLEND(pd3dDevice, TRUE);
		SETZWRITE(pd3dDevice, FALSE);

		if (player.poison > 1.f)
			color = D3DRGB(1.f - Blood_Pos, 1.f, 1.f - Blood_Pos);
		else
			color = D3DRGB(1.f, 1.f - Blood_Pos, 1.f - Blood_Pos);

		EERIEDrawBitmap(pd3dDevice, 0.f, 0.f, (float)DANAESIZX, (float)DANAESIZY, 0.00009f, NULL, color);
		SETALPHABLEND(pd3dDevice, FALSE);
		SETZWRITE(pd3dDevice, TRUE);
	}

	if (Blood_Pos > 0.f)
	{
		if (Blood_Pos > 1.f)
		{
			if ((Last_Blood_Pos <= 1.f)) 
			{
				Blood_Pos = 1.0001f;
				duration = 0;
			}

			if (duration > Blood_Duration)
				Blood_Pos += (float)FrameDiff * DIV300;

			duration += ARX_CLEAN_WARN_CAST_LONG(FrameDiff);
		}
		else Blood_Pos += (float)FrameDiff * DIV40;
	}

	Last_Blood_Pos = Blood_Pos;
}

//*************************************************************************************
//*************************************************************************************
float ARX_DAMAGES_DamagePlayer(float dmg, long type, long source, EERIE_3D * pos)
{
	if (player.playerflags & PLAYERFLAGS_INVULNERABILITY)
		return 0;

	float damagesdone = 0.f;

	if (player.life == 0.f) return damagesdone;

	if (dmg > player.life) damagesdone = dmg;
	else damagesdone = player.life;

	inter.iobj[0]->dmg_sum += dmg;

	if (ARXTime > inter.iobj[0]->ouch_time + 500)
	{
		INTERACTIVE_OBJ * oes = EVENT_SENDER;

		if (ValidIONum(source))
			EVENT_SENDER = inter.iobj[source];
		else
			EVENT_SENDER = NULL;

		inter.iobj[0]->ouch_time = ARXTimeUL();
		char tex[32];
		sprintf(tex, "%5.2f", inter.iobj[0]->dmg_sum);
		SendIOScriptEvent(inter.iobj[0], SM_OUCH, tex, NULL);
		EVENT_SENDER = oes;
		float power = inter.iobj[0]->dmg_sum / player.maxlife * 220.f;
		AddQuakeFX(power * 3.5f, 500 + power * 3, rnd() * 100.f + power + 200, 0);
		inter.iobj[0]->dmg_sum = 0.f;
	}

	if (dmg > 0.f)
	{
		if (ValidIONum(source))
		{
			INTERACTIVE_OBJ * pio = NULL;

			if (inter.iobj[source]->ioflags & IO_NPC)
			{
				pio = (INTERACTIVE_OBJ *)inter.iobj[source]->_npcdata->weapon;

				if ((pio) && ((pio->poisonous == 0) || (pio->poisonous_count == 0)))
					pio = NULL;
			}

			if (!pio) pio = inter.iobj[source];

			if (pio && pio->poisonous && (pio->poisonous_count != 0))
			{
				if (rnd() * 100.f > player.resist_poison)
				{
					player.poison += pio->poisonous;
				}

				if (pio->poisonous_count != -1)
					pio->poisonous_count--;
			}
		}

		long alive;

		if (player.life > 0) alive = 1;
		else alive = 0;

		if (!BLOCK_PLAYER_CONTROLS)
			player.life -= dmg;

		if (player.life <= 0.f)
		{
			player.life = 0.f;

			if (alive) 
			{
				REFUSE_GAME_RETURN = 1;
				ARX_PLAYER_BecomesDead();

				if ((type & DAMAGE_TYPE_FIRE) || (type & DAMAGE_TYPE_FAKEFIRE))
				{
					ARX_SOUND_PlayInterface(SND_PLAYER_DEATH_BY_FIRE);
				}

				SendIOScriptEvent(inter.iobj[0], SM_DIE, "", NULL);

				for (long i = 1; i < inter.nbmax; i++)
				{
					INTERACTIVE_OBJ * ioo = inter.iobj[i];

					if ((ioo) && (ioo->ioflags & IO_NPC))
					{
						if ((ioo->targetinfo == 0) || (ioo->targetinfo == TARGET_PLAYER))
						{
							EVENT_SENDER = inter.iobj[0];
							char killer[256];
							memset(killer, 0, 256);

							if (source == 0) strcpy(killer, "PLAYER");
							else if (source <= -1) strcpy(killer, "NONE");
							else if (ValidIONum(source)
							         &&	(inter.iobj[source]->filename)
							         &&	(inter.iobj[source]->filename[0] != 0))
							{
								char temp[256];
								strcpy(temp, GetName(inter.iobj[source]->filename));
								sprintf(killer, "%s_%04d", temp, inter.iobj[source]->ident);
							}

							SendIOScriptEvent(inter.iobj[i], 0, killer, "TARGET_DEATH");
						}
					}
				}
			}
		}

		if (player.maxlife <= 0.f) return damagesdone;

		float t = dmg / player.maxlife;

		if (Blood_Pos == 0.f)
		{
			Blood_Pos = 0.000001f;
			F2L(100 + (float)(t * 200.f), &Blood_Duration);
		}
		else
		{
			long temp;
			F2L((float)(t * 800.f), &temp);
			Blood_Duration += temp;
		}
	}

	// revient les barres
	ResetPlayerInterface();

	return damagesdone;
}

void ARX_DAMAGES_HealPlayer(float dmg)
{
	if (player.life == 0.f) return;

	if (dmg > 0.f)
	{
		if (!BLOCK_PLAYER_CONTROLS)
			player.life += dmg;

		if (player.life > player.Full_maxlife) player.life = player.Full_maxlife;
	}
}
void ARX_DAMAGES_HealInter(INTERACTIVE_OBJ * io, float dmg)
{
	if (io == NULL) return;

	if (!(io->ioflags & IO_NPC)) return;

	if (io->_npcdata->life <= 0.f) return;

	if (io == inter.iobj[0]) ARX_DAMAGES_HealPlayer(dmg);

	if (dmg > 0.f)
	{
		io->_npcdata->life += dmg;

		if (io->_npcdata->life > io->_npcdata->maxlife) io->_npcdata->life = io->_npcdata->maxlife;
	}
}
void ARX_DAMAGES_HealManaPlayer(float dmg)
{
	if (player.life == 0.f) return;

	if (dmg > 0.f)
	{
		player.mana += dmg;

		if (player.mana > player.Full_maxmana) player.mana = player.Full_maxmana;
	}
}
void ARX_DAMAGES_HealManaInter(INTERACTIVE_OBJ * io, float dmg)
{
	if (io == NULL) return;

	if (!(io->ioflags & IO_NPC)) return;

	if (io == inter.iobj[0]) ARX_DAMAGES_HealManaPlayer(dmg);

	if (io->_npcdata->life <= 0.f) return;

	if (dmg > 0.f)
	{
		io->_npcdata->mana += dmg;

		if (io->_npcdata->mana > io->_npcdata->maxmana) io->_npcdata->mana = io->_npcdata->maxmana;
	}
}
float ARX_DAMAGES_DrainMana(INTERACTIVE_OBJ * io, float dmg)
{
	if (io == NULL) return 0;

	if (!(io->ioflags & IO_NPC)) return 0;

	if (io == inter.iobj[0])
	{
		if (player.playerflags & PLAYERFLAGS_NO_MANA_DRAIN)
			return 0;

		if (player.mana >= dmg)
		{
			player.mana -= dmg;
			return dmg;
		}

		float d = player.mana;
		player.mana = 0;
		return d;
	}

	if (io->_npcdata->mana >= dmg)
	{
		io->_npcdata->mana -= dmg;
		return dmg;
	}

	float d = io->_npcdata->mana;
	io->_npcdata->mana = 0;
	return d;
}
//*************************************************************************************
//*************************************************************************************
void ARX_DAMAGES_DamageFIX(INTERACTIVE_OBJ * io, float dmg, long source, long flags, EERIE_3D * pos)
{
	if ((!io)
	        ||	(!io->show)
	        ||	(!(io->ioflags & IO_FIX))
	        ||	(io->ioflags & IO_INVULNERABILITY)
	        ||	(!io->script.data))
		return;

	io->dmg_sum += dmg;

	if (ValidIONum(source))
		EVENT_SENDER = inter.iobj[source];
	else
		EVENT_SENDER = NULL;

	if (ARXTime > io->ouch_time + 500)
	{
		io->ouch_time = ARXTimeUL();
		char tex[32];
		sprintf(tex, "%5.2f", io->dmg_sum);
		SendIOScriptEvent(io, SM_OUCH, tex, NULL);
		io->dmg_sum = 0.f;
	}

	if (rnd() * 100.f > io->durability) io->durability -= dmg * DIV2; //1.f;

	if (io->durability <= 0.f)
	{
		io->durability = 0.f;
		SendIOScriptEvent(io, SM_BREAK, "");
	}
	else
	{
		char dmm[32];

		if (EVENT_SENDER == inter.iobj[0])
		{
			if (flags & 1)
			{
				sprintf(dmm, "%f SPELL", dmg);
			}
			else switch	(ARX_EQUIPMENT_GetPlayerWeaponType())
				{
					case WEAPON_BARE:
						sprintf(dmm, "%f BARE", dmg);
						break;
					case WEAPON_DAGGER:
						sprintf(dmm, "%f DAGGER", dmg);
						break;
					case WEAPON_1H:
						sprintf(dmm, "%f 1H", dmg);
						break;
					case WEAPON_2H:
						sprintf(dmm, "%f 2H", dmg);
						break;
					case WEAPON_BOW:
						sprintf(dmm, "%f ARROW", dmg);
						break;
					default:
						sprintf(dmm, "%f", dmg);
						break;
				}
		}
		else sprintf(dmm, "%f", dmg);

		if (SendIOScriptEvent(io, SM_HIT, dmm) != ACCEPT) return;
	}
}
extern INTERACTIVE_OBJ * FlyingOverIO;
extern MASTER_CAMERA_STRUCT MasterCamera;
void ARX_DAMAGES_ForceDeath(INTERACTIVE_OBJ * io_dead, INTERACTIVE_OBJ * io_killer)
{
	if (!stricmp(io_dead->mainevent, "DEAD"))
		return;

	INTERACTIVE_OBJ * old_sender = EVENT_SENDER;
	EVENT_SENDER = io_killer;

	if (io_dead == DRAGINTER)
		Set_DragInter(NULL);

	if (io_dead == FlyingOverIO)
		FlyingOverIO = NULL;

	if ((MasterCamera.exist & 1) && (MasterCamera.io == io_dead))
		MasterCamera.exist = 0;

	if ((MasterCamera.exist & 2) && (MasterCamera.want_io == io_dead))
		MasterCamera.exist = 0;

	//Kill all speeches
	if (ValidDynLight(io_dead->dynlight))
		DynLight[io_dead->dynlight].exist = 0;

	io_dead->dynlight = -1;

	if (ValidDynLight(io_dead->halo.dynlight))
		DynLight[io_dead->halo.dynlight].exist = 0;

	io_dead->halo.dynlight = -1;
	ARX_NPC_Behaviour_Reset(io_dead);

	ARX_SPEECH_ReleaseIOSpeech(io_dead);

	//Kill all Timers...
	ARX_SCRIPT_Timer_Clear_By_IO(io_dead);

	if (stricmp(io_dead->mainevent, "DEAD"))
		NotifyIOEvent(io_dead, SM_DIE, "");

	if (!ValidIOAddress(io_dead))
		return;

	ARX_SCRIPT_SetMainEvent(io_dead, "DEAD");

	if (EEDistance3D(&io_dead->pos, &ACTIVECAM->pos) > 3200)
	{
		io_dead->animlayer[0].ctime = 9999999;
		io_dead->lastanimtime = 0;
	}

	char killer[256];
	killer[0] = 0;

	if (io_dead->ioflags & IO_NPC)
		io_dead->_npcdata->weaponinhand = 0;

	ARX_INTERACTIVE_DestroyDynamicInfo(io_dead);

	if (io_killer == inter.iobj[0]) strcpy(killer, "PLAYER");
	else
	{
		if (io_killer)
		{
			char temp[256];
			strcpy(temp, GetName(io_killer->filename));
			sprintf(killer, "%s_%04d", temp, io_killer->ident);
		}
	}

	for (long i = 1; i < inter.nbmax; i++)
	{
		INTERACTIVE_OBJ * ioo = inter.iobj[i];

		if (ioo == io_dead)
			continue;

		if ((ioo) && (ioo->ioflags & IO_NPC))
		{
			if (ValidIONum(ioo->targetinfo))
				if (inter.iobj[ioo->targetinfo] == io_dead)
				{
					EVENT_SENDER = io_dead; 
					Stack_SendIOScriptEvent(inter.iobj[i], 0, killer, "TARGET_DEATH");
					ioo->targetinfo = TARGET_NONE;
					ioo->_npcdata->reachedtarget = 0;
				}

			if (ValidIONum(ioo->_npcdata->pathfind.truetarget))
				if (inter.iobj[ioo->_npcdata->pathfind.truetarget] == io_dead)
				{
					EVENT_SENDER = io_dead; 
					Stack_SendIOScriptEvent(inter.iobj[i], 0, killer, "TARGET_DEATH");
					ioo->_npcdata->pathfind.truetarget = TARGET_NONE;
					ioo->_npcdata->reachedtarget = 0;
				}
		}
	}

	IO_UnlinkAllLinkedObjects(io_dead);
	io_dead->animlayer[1].cur_anim = NULL;
	io_dead->animlayer[2].cur_anim = NULL;
	io_dead->animlayer[3].cur_anim = NULL;

	if (io_dead->ioflags & IO_NPC)
	{
		io_dead->_npcdata->life = 0;

		if (io_dead->_npcdata->weapon != NULL)
		{
			INTERACTIVE_OBJ * ioo = (INTERACTIVE_OBJ *)io_dead->_npcdata->weapon;

			if (ValidIOAddress(ioo))
			{
				ioo->show = SHOW_FLAG_IN_SCENE;
				ioo->ioflags |= IO_NO_NPC_COLLIDE;
				ioo->pos.x = ioo->obj->vertexlist3[ioo->obj->origin].v.x;
				ioo->pos.y = ioo->obj->vertexlist3[ioo->obj->origin].v.y;
				ioo->pos.z = ioo->obj->vertexlist3[ioo->obj->origin].v.z;
				ioo->velocity.x = 0.f;
				ioo->velocity.y = 13.f;
				ioo->velocity.z = 0.f;
				ioo->stopped = 0;
			}
		}
	}

	EVENT_SENDER = old_sender;
}
void ARX_DAMAGES_PushIO(INTERACTIVE_OBJ * io_target, long source, float power)
{
	if ((power > 0.f)
	        &&	(ValidIONum(source)))
	{
		power *= DIV20;
		INTERACTIVE_OBJ * io = inter.iobj[source];
		EERIE_3D vect;
		vect.x = io_target->pos.x - io->pos.x;
		vect.y = io_target->pos.y - io->pos.y;
		vect.z = io_target->pos.z - io->pos.z;
		Vector_Normalize(&vect);
		vect.x *= power;
		vect.y *= power;
		vect.z *= power;

		if (io_target == inter.iobj[0])
		{
			PUSH_PLAYER_FORCE.x = vect.x;
			PUSH_PLAYER_FORCE.y = vect.y;
			PUSH_PLAYER_FORCE.z = vect.z;
		}
		else
		{
			io_target->move.x += vect.x;
			io_target->move.y += vect.y;
			io_target->move.z += vect.z;
		}
	}
}

float ARX_DAMAGES_DealDamages(long target, float dmg, long source, long flags, EERIE_3D * pos)
{
	if ((!ValidIONum(target))
	        ||	(!ValidIONum(source)))
		return 0;

	INTERACTIVE_OBJ * io_target = inter.iobj[target];
	INTERACTIVE_OBJ * io_source = inter.iobj[source];
	float damagesdone;

	if (flags & DAMAGE_TYPE_PER_SECOND)
	{
		dmg = dmg * _framedelay * DIV1000;
	}

	if (target == 0)
	{
		if (flags & DAMAGE_TYPE_POISON)
		{
			if (rnd() * 100.f > player.resist_poison)
			{
				damagesdone = dmg;
				player.poison += damagesdone;
			}
			else damagesdone = 0;

			goto dodamage;
		}

		if (flags & DAMAGE_TYPE_DRAIN_MANA)
			damagesdone = ARX_DAMAGES_DrainMana(io_target, dmg);
		else
		{
			ARX_DAMAGES_DamagePlayerEquipment(dmg);
			damagesdone = ARX_DAMAGES_DamagePlayer(dmg, flags, source, pos);
		}

	dodamage:
		;

		if (flags & DAMAGE_TYPE_FIRE)
		{
			ARX_DAMAGES_IgnitIO(io_target, damagesdone);
		}

		if (flags & DAMAGE_TYPE_DRAIN_LIFE)
		{
			ARX_DAMAGES_HealInter(io_source, damagesdone);
		}

		if (flags & DAMAGE_TYPE_DRAIN_MANA)
		{
			ARX_DAMAGES_HealManaInter(io_source, damagesdone);
		}

		if (flags & DAMAGE_TYPE_PUSH)
		{
			ARX_DAMAGES_PushIO(io_target, source, damagesdone * DIV2);
		}

		if ((flags & DAMAGE_TYPE_MAGICAL)
		        && !(flags & (DAMAGE_TYPE_FIRE | DAMAGE_TYPE_COLD)))
		{
			damagesdone -= player.Full_resist_magic * DIV100 * damagesdone;
			damagesdone = __max(0, damagesdone);
		}

		return damagesdone;
	}
	else
	{
		if (io_target->ioflags & IO_NPC)
		{
			if (flags & DAMAGE_TYPE_POISON)
			{
				if (rnd() * 100.f > io_target->_npcdata->resist_poison)
				{
					damagesdone = dmg;
					io_target->_npcdata->poisonned += damagesdone;
				}
				else damagesdone = 0;

				goto dodamage2;
			}

			if (flags & DAMAGE_TYPE_FIRE)
			{
				if (rnd() * 100.f <= io_target->_npcdata->resist_fire)
					dmg = 0;

				ARX_DAMAGES_IgnitIO(io_target, dmg);
			}

			if (flags & DAMAGE_TYPE_DRAIN_MANA)
			{
				damagesdone = ARX_DAMAGES_DrainMana(io_target, dmg);
			}
			else damagesdone = ARX_DAMAGES_DamageNPC(io_target, dmg, source, 1, pos);

		dodamage2:
			;

			if (flags & DAMAGE_TYPE_DRAIN_LIFE)
			{
				ARX_DAMAGES_HealInter(io_source, damagesdone);
			}

			if (flags & DAMAGE_TYPE_DRAIN_MANA)
			{
				ARX_DAMAGES_HealManaInter(io_source, damagesdone);
			}

			if (flags & DAMAGE_TYPE_PUSH)
			{
				ARX_DAMAGES_PushIO(io_target, source, damagesdone * DIV2);
			}

			if ((flags & DAMAGE_TYPE_MAGICAL)
			        && !(flags & (DAMAGE_TYPE_FIRE | DAMAGE_TYPE_COLD)))
			{
				damagesdone -= io_target->_npcdata->resist_magic * DIV100 * damagesdone;
				damagesdone = __max(0, damagesdone);
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
float ARX_DAMAGES_DamageNPC(INTERACTIVE_OBJ * io, float dmg, long source, long flags, EERIE_3D * pos) //,INTERACTIVE_OBJ * source)
{
	if ((!io)
	        ||	(!io->show)
	        ||	(io->ioflags & IO_INVULNERABILITY)
	        ||	(!(io->ioflags & IO_NPC)))
		return 0.f;

	float damagesdone = 0.f;

	if (io->_npcdata->life <= 0.f)
	{
		if ((source != 0)
		        ||	((source == 0) &&	(player.equiped[EQUIP_SLOT_WEAPON] > 0)))
		{
			if ((dmg >= io->_npcdata->maxlife * 0.4f) && pos)
				ARX_NPC_TryToCutSomething(io, pos);

			return damagesdone;
		}

		return damagesdone;
	}

	io->dmg_sum += dmg;

	if (ARXTime > io->ouch_time + 500)
	{
		if (ValidIONum(source))
		{
			EVENT_SENDER = inter.iobj[source];

		}
		else
			EVENT_SENDER = NULL;

		io->ouch_time = ARXTimeUL();
		char tex[32];

		if (EVENT_SENDER && (EVENT_SENDER->summoner == 0))
		{
			EVENT_SENDER = inter.iobj[0];
			sprintf(tex, "%5.2f SUMMONED", io->dmg_sum);
		}
		else
			sprintf(tex, "%5.2f", io->dmg_sum);

		SendIOScriptEvent(io, SM_OUCH, tex, NULL);
		io->dmg_sum = 0.f;
		long n = ARX_SPELLS_GetSpellOn(io, SPELL_CONFUSE);

		if (n >= 0)
		{
			spells[n].tolive = 0;
		}
	}

	if (dmg >= 0.f)
	{
		if (ValidIONum(source))
		{
			INTERACTIVE_OBJ * pio = NULL;

			if (source == 0)
			{
				if ((player.equiped[EQUIP_SLOT_WEAPON] != 0)
				        &&	ValidIONum(player.equiped[EQUIP_SLOT_WEAPON]))
				{
					pio = inter.iobj[player.equiped[EQUIP_SLOT_WEAPON]];

					if ((pio) && ((pio->poisonous == 0) || (pio->poisonous_count == 0))
					        || (flags & 1))
						pio = NULL;
				}
			}
			else
			{
				if (inter.iobj[source]->ioflags & IO_NPC)
				{
					pio = (INTERACTIVE_OBJ *)inter.iobj[source]->_npcdata->weapon;

					if ((pio) && ((pio->poisonous == 0) || (pio->poisonous_count == 0)))
						pio = NULL;
				}
			}

			if (!pio) pio = inter.iobj[source];

			if (pio && pio->poisonous && (pio->poisonous_count != 0))
			{
				if (rnd() * 100.f > io->_npcdata->resist_poison)
				{
					io->_npcdata->poisonned += pio->poisonous;
				}

				if (pio->poisonous_count != -1)
					pio->poisonous_count--;
			}
		}

		if (io->script.data != NULL)
		{
			if (source >= 0)
			{
				if (ValidIONum(source))
					EVENT_SENDER = inter.iobj[source]; 
				else
					EVENT_SENDER = NULL;

				char dmm[256];

				if (EVENT_SENDER == inter.iobj[0])
				{
					if (flags & 1)
					{
						sprintf(dmm, "%f SPELL", dmg);
					}
					else switch	(ARX_EQUIPMENT_GetPlayerWeaponType())
						{
							case WEAPON_BARE:
								sprintf(dmm, "%f BARE", dmg);
								break;
							case WEAPON_DAGGER:
								sprintf(dmm, "%f DAGGER", dmg);
								break;
							case WEAPON_1H:
								sprintf(dmm, "%f 1H", dmg);
								break;
							case WEAPON_2H:
								sprintf(dmm, "%f 2H", dmg);
								break;
							case WEAPON_BOW:
								sprintf(dmm, "%f ARROW", dmg);
								break;
							default:
								sprintf(dmm, "%f", dmg);
								break;
						}
				}
				else sprintf(dmm, "%f", dmg);

				if ((EVENT_SENDER)
				        &&	(EVENT_SENDER->summoner == 0))
				{
					EVENT_SENDER = inter.iobj[0];
					sprintf(dmm, "%f SUMMONED", dmg);
				}

				if (SendIOScriptEvent(io, SM_HIT, dmm) != ACCEPT) return damagesdone;

			}
		}

		damagesdone = __min(dmg, io->_npcdata->life);
		io->_npcdata->life -= dmg;

		bHitFlash = true;

		if (io->_npcdata->life <= 0)
		{
			fHitFlash = 0;
		}
		else
		{
			fHitFlash = io->_npcdata->life / io->_npcdata->maxlife;
		}

		ulHitFlash = 0;

		if (io->_npcdata->life <= 0.f)
		{
			io->_npcdata->life = 0.f;

			if ((source != 0)
			        ||	((source == 0) &&	(player.equiped[EQUIP_SLOT_WEAPON] > 0)))
			{
				if ((dmg >= io->_npcdata->maxlife * DIV2) && pos)
					ARX_NPC_TryToCutSomething(io, pos);
			}

			if (ValidIONum(source))
			{
				long xp = io->_npcdata->xpvalue;
				ARX_DAMAGES_ForceDeath(io, inter.iobj[source]);

				if (source == 0)
					ARX_PLAYER_Modify_XP(xp);
			}
			else ARX_DAMAGES_ForceDeath(io, NULL);
		}
	}

	return damagesdone;
}

//*************************************************************************************
//*************************************************************************************
void ARX_DAMAGES_Reset()
{
	memset(damages, 0, sizeof(DAMAGE_INFO)*MAX_DAMAGES);
}

//*************************************************************************************
//*************************************************************************************
long ARX_DAMAGES_GetFree()
{
	for (long i = 0; i < MAX_DAMAGES; i++)
	{
		if (!damages[i].exist)
		{
			damages[i].radius = 100.f;
			damages[i].start_time = ARXTimeUL(); 
			damages[i].duration = 1000;
			damages[i].area = 0;
			damages[i].flags = 0;
			damages[i].type = 0;
			damages[i].special = 0;
			damages[i].special_ID = 0;
			damages[i].lastupd = 0;
			damages[i].active = 1;

			for (long j = 0; j < 10; j++)
				damages[i].except[j] = -1;

			return i;
		}
	}

	return -1;
}
long InExceptList(long dmg, long num)
{
	for (long j = 0; j < 10; j++)
		if (damages[dmg].except[j] == num) return 1;

	return 0;
}

//*************************************************************************************
//*************************************************************************************
void ARX_DAMAGES_AddVisual(DAMAGE_INFO * di, EERIE_3D * pos, float dmg, INTERACTIVE_OBJ * io)
{

	if (di->type & DAMAGE_TYPE_FAKEFIRE)
	{
		long num = -1;

		if (io != NULL) num = ((long)(float)(rnd() * (io->obj->nbvertex / 4 - 1))) * 4 + 1;

		unsigned long tim = ARXTimeUL();

		if (di->lastupd + 200 < tim)
		{
			di->lastupd = tim;

			if (di->type & DAMAGE_TYPE_MAGICAL)
				ARX_SOUND_PlaySFX(SND_SPELL_MAGICAL_HIT, pos, 0.8F + 0.4F * rnd());
			else
				ARX_SOUND_PlaySFX(SND_SPELL_FIRE_HIT, pos, 0.8F + 0.4F * rnd());
		}

		for (long k = 0 ; k < 14 ; k++)
		{
			long j = ARX_PARTICLES_GetFree();

			if ((j != -1) && (!ARXPausedTimer))
			{
				ParticleCount++;
				particle[j].exist	= TRUE;
				particle[j].zdec	= 0;

				if (io != NULL)
				{
					ARX_CHECK_NOT_NEG(num);

					particle[j].ov.x = io->obj->vertexlist3[num].v.x + rnd() * 10.f - 5.f;;
					particle[j].ov.y = io->obj->vertexlist3[num].v.y + rnd() * 10.f - 5.f;;
					particle[j].ov.z = io->obj->vertexlist3[num].v.z + rnd() * 10.f - 5.f;;
					//	}
				}
				else
				{
					particle[j].ov.x = pos->x + rnd() * 100.f - 50.f;
					particle[j].ov.y = pos->y + rnd() * 100.f - 50.f;
					particle[j].ov.z = pos->z + rnd() * 100.f - 50.f;
				}

				particle[j].siz = dmg;

				if (particle[j].siz < 5.f) particle[j].siz = 5.f;
				else if (particle[j].siz > 15.f) particle[j].siz = 15.f;

				particle[j].scale.x			= -10.f;
				particle[j].scale.y			= -10.f;
				particle[j].scale.z			= -10.f;
				particle[j].timcreation		= lARXTime;
				particle[j].special		   |= ROTATING | MODULATE_ROTATION;
				particle[j].special		   |= FIRE_TO_SMOKE;
				particle[j].tolive			= 500 + (unsigned long)(rnd() * 400.f);

				if (di->type & DAMAGE_TYPE_MAGICAL)
				{
					particle[j].move.x	= 1.f - 2.f * rnd();
					particle[j].move.y	= 2.f - 16.f * rnd();
					particle[j].move.z	= 1.f - 2.f * rnd();
					particle[j].r		= 0.3f;
					particle[j].g		= 0.3f;
					particle[j].b		= 0.8f;
				}
				else
				{
					particle[j].move.x	= 1.f - 2.f * rnd();
					particle[j].move.y	= 2.f - 16.f * rnd();
					particle[j].move.z	= 1.f - 2.f * rnd();
					particle[j].r		= 0.5f;
					particle[j].g		= 0.5f;
					particle[j].b		= 0.5f;
				}

				particle[j].tc		= TC_fire2;
				particle[j].fparam	= 0.1f - rnd() * 0.2f;
			}
		}
	}
}
//*************************************************************************************
// source = -1 no source but valid pos
// source = 0  player
// source > 0  IO
//*************************************************************************************
void ARX_DAMAGES_UpdateDamage(long j, float tim)
{
	float dmg, dist;
	EERIE_3D sub;

	if (damages[j].exist)
	{
		if (!damages[j].active) return;

		if (damages[j].flags & DAMAGE_FLAG_FOLLOW_SOURCE)
		{
			if (damages[j].source == 0)
			{
				damages[j].pos.x = player.pos.x;
				damages[j].pos.y = player.pos.y;
				damages[j].pos.z = player.pos.z;
			}
			else
			{
				if (ValidIONum(damages[j].source))
				{
					damages[j].pos.x = inter.iobj[damages[j].source]->pos.x;
					damages[j].pos.y = inter.iobj[damages[j].source]->pos.y;
					damages[j].pos.z = inter.iobj[damages[j].source]->pos.z;
				}
			}
		}

		if (damages[j].flags & DAMAGE_NOT_FRAME_DEPENDANT)
			dmg = damages[j].damages;
		else if (damages[j].duration == -1) dmg = damages[j].damages;
		else
		{
			float FD = (float)FrameDiff;

			if (tim > damages[j].start_time + damages[j].duration)
				FD -= damages[j].start_time + damages[j].duration - tim;

			dmg = damages[j].damages * FD * DIV1000;
		}

		long validsource = ValidIONum(damages[j].source);
		float divradius = 1.f / damages[j].radius;

		// checking for IO damages
		for (long i = 0; i < inter.nbmax; i++)
		{
			INTERACTIVE_OBJ * io = inter.iobj[i];

			if ((io)
			        &&	(io->GameFlags & GFLAG_ISINTREATZONE)
			        &&	(io->show == SHOW_FLAG_IN_SCENE)
			        &&	(!InExceptList(j, i))
			        && ( (damages[j].source != i)
			            || ((damages[j].source == i)
			                && (!(damages[j].flags & DAMAGE_FLAG_DONT_HURT_SOURCE))))
			   )
			{
				if (io->ioflags & IO_NPC) 
				{
					if ((i != 0) && (damages[j].source != 0)
					        && validsource && (HaveCommonGroup(io, inter.iobj[damages[j].source])))
						continue;

					EERIE_SPHERE sphere;
					sphere.origin.x = damages[j].pos.x;
					sphere.origin.y = damages[j].pos.y;
					sphere.origin.z = damages[j].pos.z;
					sphere.radius = damages[j].radius - 10.f;

					if (CheckIOInSphere(&sphere, i, IIS_NO_NOCOL))
					{
						sub.x = io->pos.x;
						sub.y = io->pos.y - 60.f;
						sub.z = io->pos.z;
						dist = EEDistance3D(&damages[j].pos, &sub);

						if (damages[j].type & DAMAGE_TYPE_FIELD)
						{
							if (ARXTime > io->collide_door_time + 500) 
							{
								EVENT_SENDER = NULL;
								io->collide_door_time = ARXTimeUL(); 
								char param[64];
								param[0] = 0;

								if (damages[j].type & DAMAGE_TYPE_FIRE)
									strcpy(param, "FIRE");

								if (damages[j].type & DAMAGE_TYPE_COLD)
									strcpy(param, "COLD");

								SendIOScriptEvent(io, SM_COLLIDE_FIELD, param, NULL);

							}
						}

						switch (damages[j].area)
						{
							case DAMAGE_AREA:
							{
								float ratio = (damages[j].radius - dist) * divradius;

								if (ratio > 1.f) ratio = 1.f;
								else if (ratio < 0.f) ratio = 0.f;

								dmg = dmg * ratio + 1.f;
							}
							break;
							case DAMAGE_AREAHALF:
							{
								float ratio = (damages[j].radius - (dist * DIV2)) * divradius;

								if (ratio > 1.f) ratio = 1.f;
								else if (ratio < 0.f) ratio = 0.f;

								dmg = dmg * ratio + 1.f;
							}
							break;
						}

						if (dmg <= 0.f) continue;

						if (damages[j].flags & DAMAGE_FLAG_ADD_VISUAL_FX)
						{
							if ((inter.iobj[i]->ioflags & IO_NPC)
							        &&	(inter.iobj[i]->_npcdata->life > 0.f))
								ARX_DAMAGES_AddVisual(&damages[j], &sub, dmg, inter.iobj[i]);
						}

						if (damages[j].type & DAMAGE_TYPE_DRAIN_MANA)
						{
							float manadrained;

							if (i == 0)
							{
								manadrained = __min(dmg, player.mana);
								player.mana -= manadrained;
							}
							else
							{
								manadrained = dmg;

								if ((io) && (io->_npcdata))
								{
									manadrained = __min(dmg, io->_npcdata->mana);
									io->_npcdata->mana -= manadrained;
								}
							}

							if (damages[j].source == 0)
							{
								player.mana = __min(player.mana + manadrained, player.Full_maxmana);
							}
							else
							{
								if (ValidIONum(damages[j].source) && (inter.iobj[damages[j].source]->_npcdata))
								{
									inter.iobj[damages[j].source]->_npcdata->mana = __min(inter.iobj[damages[j].source]->_npcdata->mana + manadrained, inter.iobj[damages[j].source]->_npcdata->maxmana);
								}
							}
						}
						else
						{
							float damagesdone;

							if (i == 0)
							{

								if (damages[j].type & DAMAGE_TYPE_POISON)
								{
									if (rnd() * 100.f > player.resist_poison)
									{
										// Failed Saving Throw
										damagesdone = dmg; 
										player.poison += damagesdone;

									}
									else damagesdone = 0;
								}
								else
								{
									if ((damages[j].type & DAMAGE_TYPE_MAGICAL)
									        &&	(!(damages[j].type & DAMAGE_TYPE_FIRE))
									        &&	(!(damages[j].type & DAMAGE_TYPE_COLD))
									   )
									{
										dmg -= player.Full_resist_magic * DIV100 * dmg;
										dmg = __max(0, dmg);
									}

									if (damages[j].type & DAMAGE_TYPE_FIRE)
									{
										dmg = ARX_SPELLS_ApplyFireProtection(inter.iobj[0], dmg);
										ARX_DAMAGES_IgnitIO(inter.iobj[0], dmg);
									}

									if (damages[j].type & DAMAGE_TYPE_COLD)
									{
										dmg = ARX_SPELLS_ApplyColdProtection(inter.iobj[0], dmg);
									}

									damagesdone = ARX_DAMAGES_DamagePlayer(dmg, damages[j].type, damages[j].source, &damages[j].pos);
								}
							}
							else
							{
								if ((inter.iobj[i]->ioflags & IO_NPC)
								        && (damages[j].type & DAMAGE_TYPE_POISON))
								{
									if (rnd() * 100.f > inter.iobj[i]->_npcdata->resist_poison)
									{
										// Failed Saving Throw
										damagesdone = dmg; 
										inter.iobj[i]->_npcdata->poisonned += damagesdone;
									}
									else damagesdone = 0;
								}
								else
								{
									if (damages[j].type & DAMAGE_TYPE_FIRE)
									{
										dmg = ARX_SPELLS_ApplyFireProtection(inter.iobj[i], dmg);
										ARX_DAMAGES_IgnitIO(inter.iobj[i], dmg);
									}

									if ((damages[j].type & DAMAGE_TYPE_MAGICAL)
									        && (!(damages[j].type & DAMAGE_TYPE_FIRE))
									        && (!(damages[j].type & DAMAGE_TYPE_COLD))
									   )
									{
										dmg -= inter.iobj[i]->_npcdata->resist_magic * DIV100 * dmg;
										dmg = __max(0, dmg);
									}

									if (damages[j].type & DAMAGE_TYPE_COLD)
									{
										dmg = ARX_SPELLS_ApplyColdProtection(inter.iobj[i], dmg);
									}

									damagesdone = ARX_DAMAGES_DamageNPC(inter.iobj[i], dmg, damages[j].source, 1, &damages[j].pos);
								}

								if ((damagesdone > 0) && (damages[j].flags & DAMAGE_SPAWN_BLOOD))
								{
									EERIE_3D vector;
									vector.x = damages[j].pos.x - inter.iobj[i]->pos.x;
									vector.y = (damages[j].pos.y - inter.iobj[i]->pos.y) * DIV2;
									vector.z = damages[j].pos.z - inter.iobj[i]->pos.z;
									float t = 1.f / TRUEVector_Magnitude(&vector);
									vector.x *= t;
									vector.y *= t;
									vector.z *= t;
									ARX_PARTICLES_Spawn_Blood(&damages[j].pos, &vector, damagesdone, damages[j].source);
								}
							}

							if (damages[j].type & DAMAGE_TYPE_DRAIN_LIFE) 
							{
								if (ValidIONum(damages[j].source))
									ARX_DAMAGES_HealInter(inter.iobj[damages[j].source], damagesdone);
							}
						}
					}

				}
				else if ((io->ioflags & IO_FIX)
				         &&	(!(damages[j].type & DAMAGE_TYPE_NO_FIX)))
				{
					EERIE_SPHERE sphere;
					sphere.origin.x = damages[j].pos.x;
					sphere.origin.y = damages[j].pos.y;
					sphere.origin.z = damages[j].pos.z;
					sphere.radius = damages[j].radius + 15.f;

					if (CheckIOInSphere(&sphere, i))
					{
						ARX_DAMAGES_DamageFIX(io, dmg, damages[j].source, 1);
					}
				}
			}
		}

		if (damages[j].duration == -1) damages[j].exist = FALSE;
		else if (tim > damages[j].start_time + damages[j].duration)
			damages[j].exist = FALSE;
	}
}
void ARX_DAMAGES_UpdateAll()
{
	for (long j = 0; j < MAX_DAMAGES; j++)
		ARX_DAMAGES_UpdateDamage(j, ARXTime);
}
BOOL SphereInIO(INTERACTIVE_OBJ * io, EERIE_3D * pos, float radius)
{
	if (io == NULL) return FALSE;

	if (io->obj == NULL) return FALSE;

	long step;

	if (io->obj->nbvertex < 150) step = 1;
	else if (io->obj->nbvertex < 300) step = 2;
	else if (io->obj->nbvertex < 600) step = 4;
	else if (io->obj->nbvertex < 1200) step = 6;
	else step = 7;

	for (long i = 0; i < io->obj->nbvertex; i += step)
	{
		if (EEDistance3D(pos, &io->obj->vertexlist3[i].v) <= radius)
		{
			return TRUE;
		}
	}

	return FALSE;
}
BOOL ARX_DAMAGES_TryToDoDamage(EERIE_3D * pos, float dmg, float radius, long source)
{
	BOOL ret = FALSE;

	for (long i = 0; i < inter.nbmax; i++)
	{
		INTERACTIVE_OBJ * io = inter.iobj[i];

		if (io != NULL)

			if (inter.iobj[i]->GameFlags & GFLAG_ISINTREATZONE)
				if (io->show == SHOW_FLAG_IN_SCENE)
					if (source != i)
					{
						float threshold;
						float rad = radius + 5.f;

						if (io->ioflags & IO_FIX)
						{
							threshold = 510;
							rad += 10.f;
						}
						else if (io->ioflags & IO_NPC)
							threshold = 250;
						else threshold = 350;

						if (EEDistance3D(pos, &io->pos) < threshold)
							if (SphereInIO(io, pos, rad))
							{
								if (io->ioflags & IO_NPC)
								{
									if (ValidIONum(source))
										ARX_EQUIPMENT_ComputeDamages(inter.iobj[source], NULL, io, 1.f);

									ret = TRUE;
								}

								if (io->ioflags & IO_FIX)
								{
									ARX_DAMAGES_DamageFIX(io, dmg, source, 0);
									ret = TRUE;
								}
							}
					}
	}

	return ret;
}

// mode=1 ON mode=0  OFF
// flag & 1 no lights;
// flag & 2 Only affects small sources
//*************************************************************************************
//*************************************************************************************
void CheckForIgnition(EERIE_3D * pos, float radius, long mode, long flag)
{
	long i;
	float dist;

	if (!(flag & 1))
		for (i = 0; i < MAX_LIGHTS; i++)
		{
			EERIE_LIGHT * el = GLight[i];

			if (el == NULL) continue;

			if ((el->extras & EXTRAS_EXTINGUISHABLE)
			        &&
			        ((el->extras & EXTRAS_SEMIDYNAMIC)
			         || (el->extras & EXTRAS_SPAWNFIRE)
			         || (el->extras & EXTRAS_SPAWNSMOKE)))
			{
				if ((el->extras & EXTRAS_FIREPLACE) && (flag & 2))
					continue;

				dist = EEDistance3D(pos, &el->pos);

				if (dist <= radius) 
				{
					if (mode)
					{
						if (!(el->extras & EXTRAS_NO_IGNIT))
							el->status = 1;
					}
					else
					{
						el->status = 0;
					}
				}

			}
		}

	for (i = 0; i < inter.nbmax; i++)
	{
		INTERACTIVE_OBJ * io = inter.iobj[i];

		if ((io)
		        &&	(io->show == 1)
		        &&	(io->obj)
		        &&  !(io->ioflags & IO_UNDERWATER)
		        &&	(io->obj->fastaccess.fire >= 0)
		   )
		{
			float dist = EEDistance3D(pos, &io->obj->vertexlist3[io->obj->fastaccess.fire].v);

			if (dist < radius)
			{

				if ((mode) && (io->ignition <= 0) && (io->obj->fastaccess.fire >= 0))
				{
					io->ignition = 1;
				}
				else if ((!mode) && (io->ignition > 0))
				{
					if (io->obj->fastaccess.fire >= 0)
					{
						io->ignition = 0; 

						if (ValidDynLight(io->ignit_light))
							DynLight[io->ignit_light].exist = 0;

						io->ignit_light = -1;

						if (io->ignit_sound != ARX_SOUND_INVALID_RESOURCE)
						{
							ARX_SOUND_Stop(io->ignit_sound);
							io->ignit_sound = ARX_SOUND_INVALID_RESOURCE;
						}
					}
					else if (!(flag & 2))
						io->ignition = 0.00001f;
				}
			}
		}
	}
}

void PushPlayer(EERIE_3D * pos, float intensity)
{
	
}
//*************************************************************************************
//*************************************************************************************
BOOL DoSphericDamage(EERIE_3D * pos, float dmg, float radius, long flags, long typ, long numsource)
{
	BOOL damagesdone = FALSE;
	EERIE_3D sub;
	sub.x = player.pos.x;
	sub.y = player.pos.y + 90.f;
	sub.z = player.pos.z;
	float dist;
	dist = EEDistance3D(pos, &sub);

	if (radius <= 0.f) return damagesdone;

	float rad = 1.f / radius;
	long validsource = ValidIONum(numsource);

	for (long i = 0; i < inter.nbmax; i++)
	{
		INTERACTIVE_OBJ * ioo = inter.iobj[i];

		if ((ioo) && (i != numsource) && (ioo->obj))
		{
			if ((i != 0) && (numsource != 0)
			        && validsource && (HaveCommonGroup(ioo, inter.iobj[numsource])))
				continue;

			if ((ioo->ioflags & IO_CAMERA) || (ioo->ioflags & IO_MARKER)) continue;

			long count = 0;
			long count2 = 0;
			float mindist = FLT_MAX;

			for (long k = 0; k < ioo->obj->nbvertex; k += 1)
			{
				if (ioo->obj->nbvertex < 120)
				{
					for (long kk = 0; kk < ioo->obj->nbvertex; kk += 1)
					{
						if (kk != k)
						{
							EERIE_3D posi;
							posi.x = (inter.iobj[i]->obj->vertexlist3[k].v.x + inter.iobj[i]->obj->vertexlist3[kk].v.x) * DIV2;
							posi.y = (inter.iobj[i]->obj->vertexlist3[k].v.y + inter.iobj[i]->obj->vertexlist3[kk].v.y) * DIV2;
							posi.z = (inter.iobj[i]->obj->vertexlist3[k].v.z + inter.iobj[i]->obj->vertexlist3[kk].v.z) * DIV2;
							dist = EEDistance3D(pos, &posi);

							if (dist <= radius)
							{
								count2++;

								if (dist < mindist) mindist = dist;
							}
						}
					}
				}

				{
					dist = EEDistance3D(pos, &inter.iobj[i]->obj->vertexlist3[k].v);

					if (dist <= radius)
					{
						count++;

						if (dist < mindist) mindist = dist;
					}
				}
			}

			float ratio = ((float)count / ((float)ioo->obj->nbvertex * DIV2));

			if (count2 > count)
				ratio = ((float)count2 / ((float)ioo->obj->nbvertex * DIV2));

			if (ratio > 2.f) ratio = 2.f;

			if (ioo->ioflags & IO_NPC)
			{
				if (mindist <= radius + 30.f)
				{
					switch (flags)
					{
						case DAMAGE_AREA:
							dmg = dmg * (radius + 30 - mindist) * rad;
							break;
						case DAMAGE_AREAHALF:
							dmg = dmg * (radius + 30 - mindist * DIV2) * rad;
							break;
					}

					if (i == 0)
					{
						if (typ & DAMAGE_TYPE_FIRE)
						{
							dmg = ARX_SPELLS_ApplyFireProtection(ioo, dmg);
							ARX_DAMAGES_IgnitIO(inter.iobj[0], dmg);
						}

						if (typ & DAMAGE_TYPE_COLD)
						{
							dmg = ARX_SPELLS_ApplyColdProtection(ioo, dmg);
						}

						ARX_DAMAGES_DamagePlayer(dmg, typ, numsource, pos);
						ARX_DAMAGES_DamagePlayerEquipment(dmg);
						EERIE_3D vector;
						float div = 1.f / dist;
						vector.x = (sub.x - pos->x) * div;
						vector.y = (sub.y - pos->y) * div;
						vector.z = (sub.z - pos->z) * div;
						PushPlayer(&vector, (radius - dist) / radius);
					}
					else
					{
						if (typ & DAMAGE_TYPE_FIRE)
						{
							dmg = ARX_SPELLS_ApplyFireProtection(ioo, dmg * ratio);
							ARX_DAMAGES_IgnitIO(ioo, dmg);
						}

						if (typ & DAMAGE_TYPE_COLD)
						{
							dmg = ARX_SPELLS_ApplyColdProtection(ioo, dmg * ratio);
						}

						ARX_DAMAGES_DamageNPC(ioo, dmg * ratio, numsource, 1, pos);
					}

					if (dmg > 1) damagesdone = TRUE;
				}
			}
			else
			{
				if (mindist <= radius + 30.f)
				{
					if (typ & DAMAGE_TYPE_FIRE)
					{
						dmg = ARX_SPELLS_ApplyFireProtection(ioo, dmg * ratio);
						ARX_DAMAGES_IgnitIO(inter.iobj[i], dmg);
					}

					if (typ & DAMAGE_TYPE_COLD)
					{
						dmg = ARX_SPELLS_ApplyColdProtection(ioo, dmg * ratio);
					}

					if (inter.iobj[i]->ioflags & IO_FIX)
						ARX_DAMAGES_DamageFIX(inter.iobj[i], dmg * ratio, numsource, 1);

					if (dmg > 0.2f) damagesdone = TRUE;
				}
			}
		}
	}

	if (typ & DAMAGE_TYPE_FIRE)
		CheckForIgnition(pos, radius, 1);

	return damagesdone;
}
void ARX_DAMAGES_DurabilityRestore(INTERACTIVE_OBJ * io, float percent)
{
	if (!io) return;

	if (io->durability <= 0) return;

	if (io->durability == io->max_durability) return;

	if (percent >= 100.f)
	{
		io->durability = io->max_durability;
	}
	else
	{
		float ratio			= percent * DIV100;
		float to_restore	= (io->max_durability - io->durability) * ratio;
		float v				= rnd() * 100.f - percent;

		if (v <= 0.f)
		{
			float mloss = 1.f;

			if (io->ioflags & IO_ITEM)
			{

				ARX_CHECK_LONG(io->_itemdata->price / io->max_durability);
				io->_itemdata->price -= ARX_CLEAN_WARN_CAST_LONG(io->_itemdata->price / io->max_durability);

			}

			io->max_durability -= mloss;
		}
		else
		{
			if (v > 50.f)
				v = 50.f;

			v *= DIV100;
			float mloss = io->max_durability * v;

			if (io->ioflags & IO_ITEM)
			{
				io->_itemdata->price -= ARX_CLEAN_WARN_CAST_LONG(io->_itemdata->price * v);
			}

			io->max_durability -= mloss;
		}

		io->durability += to_restore;

		if (io->durability > io->max_durability) io->durability = io->max_durability;

		if (io->max_durability <= 0.f)
			ARX_DAMAGES_DurabilityLoss(io, 100);
	}

}
void ARX_DAMAGES_DurabilityCheck(INTERACTIVE_OBJ * io, float ratio)
{
	if (!io) return;

	if (rnd() * 100.f > io->durability)
	{
		ARX_DAMAGES_DurabilityLoss(io, ratio);
	}
}
void ARX_DAMAGES_DurabilityLoss(INTERACTIVE_OBJ * io, float loss)
{
	if (!io) return;

	io->durability -= loss;

	if (io->durability <= 0)
	{
		SendIOScriptEvent(io, SM_BREAK, "", NULL);
	}
}
void ARX_DAMAGES_DamagePlayerEquipment(float damages)
{
	float ratio = damages * DIV20;

	if (ratio > 1.f) ratio = 1.f;

	for (long i = 0; i < MAX_EQUIPED; i++)
	{
		if (player.equiped[i] != 0)
		{
			INTERACTIVE_OBJ * todamage = inter.iobj[player.equiped[i]];
			ARX_DAMAGES_DurabilityCheck(todamage, ratio);
		}
	}
}
float ARX_DAMAGES_ComputeRepairPrice(INTERACTIVE_OBJ * torepair, INTERACTIVE_OBJ * blacksmith)
{
	if ((!torepair) || (!blacksmith)) return -1.f;

	if (!(torepair->ioflags & IO_ITEM)) return -1.f;

	if (torepair->max_durability <= 0.f) return -1.f;

	if (torepair->durability == torepair->max_durability) return -1.f;

	float ratio = (torepair->max_durability - torepair->durability) / torepair->max_durability;
	float price = torepair->_itemdata->price * ratio;

	if (blacksmith->shop_multiply != 0.f)
		price *= blacksmith->shop_multiply;

	if ((price > 0.f) &&
	        (price < 1.f)) price = 1.f;

	return price;
}
