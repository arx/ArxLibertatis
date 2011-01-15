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
// ARX_Equipment
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX_Equipment
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>

#include "ARX_Equipment.h"
#include "HERMESMain.h"
#include "EERIEMath.h"
#include "EERIEObject.h"
#include "EERIEMeshTweak.h"
#include "ARX_NPC.h"
#include "ARX_Sound.h"
#include "ARX_Collisions.h"
#include "ARX_Particles.h"
#include "ARX_Damages.h"
#include "ARX_interactive.h"
#include "ARX_interface.h"
#include "ARX_script.h"

#include "EERIELinkedObj.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>



typedef struct
{
	char name[64];
} EQUIP_INFO;

#define SP_SPARKING 1
#define SP_BLOODY	2

extern float PLAYER_BASE_HEIGHT;
extern long TRUEFIGHT;
extern long GAME_EDITOR;
extern EERIE_3D PUSH_PLAYER_FORCE;
extern long HERO_SHOW_1ST;
extern long EXTERNALVIEW;

extern EERIE_3DOBJ * arrowobj;

EQUIP_INFO equipinfo[IO_EQUIPITEM_ELEMENT_Number];

//***********************************************************************************************
// Returns the object type flag corresponding to a string
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/29)
//***********************************************************************************************
unsigned long ARX_EQUIPMENT_GetObjectTypeFlag(char * temp)
{
	if (!temp) return 0;

	char c = temp[0];

	if ((temp[0] >= 'A') && (temp[0] <= 'Z'))
		c = _tolower(temp[0]);

	switch (c)
	{
		case 'w':
			return OBJECT_TYPE_WEAPON;
		case 'd':
			return OBJECT_TYPE_DAGGER;
		case '1':
			return OBJECT_TYPE_1H;
		case '2':
			return OBJECT_TYPE_2H;
		case 'b':
			return OBJECT_TYPE_BOW;
		case 's':
			return OBJECT_TYPE_SHIELD;
		case 'f':
			return OBJECT_TYPE_FOOD;
		case 'g':
			return OBJECT_TYPE_GOLD;
		case 'r':
			return OBJECT_TYPE_RING;
		case 'a':
			return OBJECT_TYPE_ARMOR;
		case 'h':
			return OBJECT_TYPE_HELMET;
		case 'l':
			return OBJECT_TYPE_LEGGINGS;
	}

	return 0;
}
//***********************************************************************************************
// Releases Equiped Id from player
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/29)
//***********************************************************************************************
void ARX_EQUIPMENT_Release(long id)
{
	if (id)
	{
		for (long i = 0; i < MAX_EQUIPED; i++)
		{
			if (player.equiped[i] == id)
			{
				player.equiped[i] = 0;
			}
		}
	}
}

//***********************************************************************************************
// Releases all id equipments in IO io
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/29)
//***********************************************************************************************
void ARX_EQUIPMENT_ReleaseAll(INTERACTIVE_OBJ * io)
{
	if (io)
		ARX_EQUIPMENT_ReleaseEquipItem(io);
}
extern long EXITING;
//***********************************************************************************************
// Recreates player mesh from scratch
//***********************************************************************************************
void ARX_EQUIPMENT_RecreatePlayerMesh()
{
	if (EXITING) return;

	INTERACTIVE_OBJ * io = inter.iobj[0];

	if (io == NULL) return;

	if (io->obj != hero)
	{
		ReleaseEERIE3DObj(io->obj);
	}

	char tex[256];
	char tex1[256];
	sprintf(tex1, "%sGraph\\Obj3D\\Textures\\", Project.workingdir);
	MakeDir(tex, "graph\\Obj3D\\Interactive\\NPC\\human_base\\human_base.teo");
	io->obj = TheoToEerie_Fast(tex1, tex, TTE_NO_PHYSICS_BOX | TTE_NPC);
	
	long sel_ = -1;
	long i;
	char pathh[256];

	if ((player.equiped[EQUIP_SLOT_HELMET] != 0)
	        &&	ValidIONum(player.equiped[EQUIP_SLOT_HELMET]))
	{
		INTERACTIVE_OBJ * tweaker = inter.iobj[player.equiped[EQUIP_SLOT_HELMET]];

		if (tweaker)
		{
			if (tweaker->tweakerinfo->filename[0] != 0)
			{
				sprintf(pathh, "%s\\Graph\\Obj3D\\Interactive\\NPC\\human_base\\tweaks\\%s", Project.workingdir, tweaker->tweakerinfo->filename);
				EERIE_MESH_TWEAK_Do(io, TWEAK_HEAD, pathh);
			}

			if ((tweaker->tweakerinfo->skintochange[0] != 0) && (tweaker->tweakerinfo->skinchangeto[0] != 0))
			{
				char path[256];
				sprintf(path, "Graph\\Obj3D\\Textures\\%s.bmp", tweaker->tweakerinfo->skinchangeto);
				TextureContainer * temp = MakeTCFromFile(path, 1);

				if (temp) temp->Restore(GDevice);

				long mapidx = ObjectAddMap(io->obj, temp);

				// retreives head sel
				for (i = 0; i < io->obj->nbselections; i++)
				{
					if (!stricmp(io->obj->selections[i].name, "head"))
					{
						sel_ = i;
						break;
					}
				}

				long textochange = -1;

				for (i = 0; i < io->obj->nbmaps; i++)
				{
					if (!stricmp(tweaker->tweakerinfo->skintochange, GetName(io->obj->texturecontainer[i]->m_strName)))
						textochange = i;
				}

				if ((sel_ != -1) && (textochange != -1))
					for (i = 0; i < io->obj->nbfaces; i++)
					{
						if ((IsInSelection(io->obj, io->obj->facelist[i].vid[0], sel_) != -1)
						        &&	(IsInSelection(io->obj, io->obj->facelist[i].vid[1], sel_) != -1)
						        &&	(IsInSelection(io->obj, io->obj->facelist[i].vid[2], sel_) != -1))
						{
							if (io->obj->facelist[i].texid == textochange)
								io->obj->facelist[i].texid = (short)mapidx;
						}
					}
			}
		}
	}

	if ((player.equiped[EQUIP_SLOT_ARMOR] != 0)
	        &&	ValidIONum(player.equiped[EQUIP_SLOT_ARMOR]))
	{
		INTERACTIVE_OBJ * tweaker = inter.iobj[player.equiped[EQUIP_SLOT_ARMOR]];

		if (tweaker)
		{
			if (tweaker->tweakerinfo->filename[0] != 0)
			{
				sprintf(pathh, "%s\\Graph\\Obj3D\\Interactive\\NPC\\human_base\\tweaks\\%s", Project.workingdir, tweaker->tweakerinfo->filename);
				EERIE_MESH_TWEAK_Do(io, TWEAK_TORSO, pathh);
			}

			if ((tweaker->tweakerinfo->skintochange[0] != 0) && (tweaker->tweakerinfo->skinchangeto[0] != 0))
			{
				char path[256];
				sprintf(path, "Graph\\Obj3D\\Textures\\%s.bmp", tweaker->tweakerinfo->skinchangeto);
				TextureContainer * temp = MakeTCFromFile(path, 1);

				if (temp) temp->Restore(GDevice);

				long mapidx = ObjectAddMap(io->obj, temp);

				// retreives head sel
				for (i = 0; i < io->obj->nbselections; i++)
				{
					if (!stricmp(io->obj->selections[i].name, "chest"))
					{
						sel_ = i;
						break;
					}
				}

				long textochange = -1;

				for (i = 0; i < io->obj->nbmaps; i++)
				{
					if (!stricmp(tweaker->tweakerinfo->skintochange, GetName(io->obj->texturecontainer[i]->m_strName)))
						textochange = i;
				}

				if ((sel_ != -1) && (textochange != -1))
					for (i = 0; i < io->obj->nbfaces; i++)
					{
						if ((IsInSelection(io->obj, io->obj->facelist[i].vid[0], sel_) != -1)
						        &&	(IsInSelection(io->obj, io->obj->facelist[i].vid[1], sel_) != -1)
						        &&	(IsInSelection(io->obj, io->obj->facelist[i].vid[2], sel_) != -1))
						{
							if (io->obj->facelist[i].texid == textochange)
								io->obj->facelist[i].texid = (short)mapidx;
						}
					}
			}
		}
	}

	if ((player.equiped[EQUIP_SLOT_LEGGINGS] != 0)
	        &&	ValidIONum(player.equiped[EQUIP_SLOT_LEGGINGS]))
	{
		INTERACTIVE_OBJ * tweaker = inter.iobj[player.equiped[EQUIP_SLOT_LEGGINGS]];

		if (tweaker)
		{
			if (tweaker->tweakerinfo->filename[0] != 0)
			{
				sprintf(pathh, "%s\\Graph\\Obj3D\\Interactive\\NPC\\human_base\\tweaks\\%s", Project.workingdir, tweaker->tweakerinfo->filename);
				EERIE_MESH_TWEAK_Do(io, TWEAK_LEGS, pathh);
			}

			if ((tweaker->tweakerinfo->skintochange[0] != 0) && (tweaker->tweakerinfo->skinchangeto[0] != 0))
			{
				char path[256];
				sprintf(path, "Graph\\Obj3D\\Textures\\%s.bmp", tweaker->tweakerinfo->skinchangeto);
				TextureContainer * temp = MakeTCFromFile(path, 1);

				if (temp) temp->Restore(GDevice);

				long mapidx = ObjectAddMap(io->obj, temp);

				// retreives head sel
				for (i = 0; i < io->obj->nbselections; i++)
				{
					if (!stricmp(io->obj->selections[i].name, "leggings"))
					{
						sel_ = i;
						break;
					}
				}

				long textochange = -1;

				for (i = 0; i < io->obj->nbmaps; i++)
				{
					if (!stricmp(tweaker->tweakerinfo->skintochange, GetName(io->obj->texturecontainer[i]->m_strName)))
						textochange = i;
				}

				if ((sel_ != -1) && (textochange != -1))
					for (i = 0; i < io->obj->nbfaces; i++)
					{
						if ((IsInSelection(io->obj, io->obj->facelist[i].vid[0], sel_) != -1)
						        &&	(IsInSelection(io->obj, io->obj->facelist[i].vid[1], sel_) != -1)
						        &&	(IsInSelection(io->obj, io->obj->facelist[i].vid[2], sel_) != -1))
						{
							if (io->obj->facelist[i].texid == textochange)
								io->obj->facelist[i].texid = (short)mapidx;
						}
					}
			}
		}
	}


	INTERACTIVE_OBJ * target = inter.iobj[0];
	INTERACTIVE_OBJ * toequip = NULL;

	if (!target) return;

	for (i = 0; i < MAX_EQUIPED; i++)
	{
		if ((player.equiped[i] != 0)
		        &&	ValidIONum(player.equiped[i]))
		{
			toequip = inter.iobj[player.equiped[i]];

			if (toequip)
			{
				if (toequip->type_flags & (OBJECT_TYPE_DAGGER
				                           |	OBJECT_TYPE_1H
				                           |	OBJECT_TYPE_2H
				                           |	OBJECT_TYPE_BOW))
				{
					if (player.Interface & INTER_COMBATMODE)	
					{
						ARX_EQUIPMENT_AttachPlayerWeaponToHand();
					}
					else
					{
						EERIE_LINKEDOBJ_LinkObjectToObject(target->obj, toequip->obj, "WEAPON_ATTACH", "PRIMARY_ATTACH", toequip); //
					}
				}
				else if (toequip->type_flags & OBJECT_TYPE_SHIELD)
				{
					if (player.equiped[EQUIP_SLOT_SHIELD] != 0)
					{
						EERIE_LINKEDOBJ_LinkObjectToObject(target->obj, toequip->obj, "SHIELD_ATTACH", "SHIELD_ATTACH", toequip);
					}

				}
			}
		}
	}

	ARX_PLAYER_Restore_Skin();
	HERO_SHOW_1ST = -1;

	if (EXTERNALVIEW)
	{
		ARX_INTERACTIVE_Show_Hide_1st(inter.iobj[0], 0);
	}
	else 
	{
		ARX_INTERACTIVE_Show_Hide_1st(inter.iobj[0], 1);
	}

	ARX_INTERACTIVE_HideGore(inter.iobj[0], 1);
	EERIE_Object_Precompute_Fast_Access(hero);
	EERIE_Object_Precompute_Fast_Access(inter.iobj[0]->obj);

	ARX_INTERACTIVE_RemoveGoreOnIO(inter.iobj[0]); 
}

void ARX_EQUIPMENT_UnEquipAllPlayer()
{
	for (long i = 0; i < MAX_EQUIPED; i++)
	{
		if ((player.equiped[i]) && (ValidIONum(player.equiped[i])))
		{
			ARX_EQUIPMENT_UnEquip(inter.iobj[0], inter.iobj[player.equiped[i]]);
		}
	}

	ARX_PLAYER_ComputePlayerFullStats();
}


bool ARX_EQUIPMENT_IsPlayerEquip(INTERACTIVE_OBJ * _pIO)
{
	INTERACTIVE_OBJ * io = inter.iobj[0];

	if (io == NULL) return false;

	if (io != inter.iobj[0]) return false;

	for (long i = 0; i < MAX_EQUIPED; i++)
	{
		if ((player.equiped[i] != 0) &&
		        ValidIONum(player.equiped[i]))
		{
			INTERACTIVE_OBJ * toequip = inter.iobj[player.equiped[i]];

			if (toequip == _pIO)
			{
				return true;
			}
		}
	}

	return false;
}


//***********************************************************************************************
// flags & 1 == destroyed !
//***********************************************************************************************
void ARX_EQUIPMENT_UnEquip(INTERACTIVE_OBJ * target, INTERACTIVE_OBJ * tounequip, long flags)
{
	if (target == NULL) return;

	if (tounequip == NULL) return;

	if (target != inter.iobj[0]) return;

	for (long i = 0; i < MAX_EQUIPED; i++)
	{
		if ((player.equiped[i] != 0)
		        &&	ValidIONum(player.equiped[i])
		        &&	(inter.iobj[player.equiped[i]] == tounequip))
		{
			EERIE_LINKEDOBJ_UnLinkObjectFromObject(target->obj, tounequip->obj);
			ARX_EQUIPMENT_Release(player.equiped[i]);
			target->bbox1.x = 9999;
			target->bbox2.x = -9999;

			if (!flags & 1)
			{
				if (DRAGINTER == NULL)
				{
					ARX_SOUND_PlayInterface(SND_INVSTD);
					Set_DragInter(tounequip);
				}
				else if (!CanBePutInInventory(tounequip))
				{
					PutInFrontOfPlayer(tounequip, 1); 
				}
			}

			EVENT_SENDER = tounequip;
			SendIOScriptEvent(inter.iobj[0], SM_EQUIPOUT, "", NULL);
			EVENT_SENDER = inter.iobj[0];
			SendIOScriptEvent(tounequip, SM_EQUIPOUT, "", NULL);
		}
	}

	if ((tounequip->type_flags & OBJECT_TYPE_HELMET)
	        ||	(tounequip->type_flags & OBJECT_TYPE_ARMOR)
	        ||	(tounequip->type_flags & OBJECT_TYPE_LEGGINGS))
		ARX_EQUIPMENT_RecreatePlayerMesh();
}
//***********************************************************************************************
//***********************************************************************************************
void ARX_EQUIPMENT_AttachPlayerWeaponToHand()
{
	INTERACTIVE_OBJ * target = inter.iobj[0];
	INTERACTIVE_OBJ * toequip = NULL;

	if (!target) return;

	for (long i = 0; i < MAX_EQUIPED; i++)
	{
		if ((player.equiped[i] != 0)
		        &&	ValidIONum(player.equiped[i]))
		{
			toequip = inter.iobj[player.equiped[i]];

			if (toequip)
			{
				if ((toequip->type_flags & OBJECT_TYPE_DAGGER)
				        ||	(toequip->type_flags & OBJECT_TYPE_1H)
				        ||	(toequip->type_flags & OBJECT_TYPE_2H)
				        ||	(toequip->type_flags & OBJECT_TYPE_BOW)
				   )
				{
					EERIE_LINKEDOBJ_UnLinkObjectFromObject(target->obj, toequip->obj);
					EERIE_LINKEDOBJ_LinkObjectToObject(target->obj, toequip->obj, "PRIMARY_ATTACH", "PRIMARY_ATTACH", toequip); //
					return;
				}
			}
		}
	}
}
//***********************************************************************************************
//***********************************************************************************************
void ARX_EQUIPMENT_AttachPlayerWeaponToBack()
{
	INTERACTIVE_OBJ * target = inter.iobj[0];
	INTERACTIVE_OBJ * toequip = NULL;

	if (!target) return;

	for (long i = 0; i < MAX_EQUIPED; i++)
	{
		if ((player.equiped[i] != 0)
		        &&	ValidIONum(player.equiped[i]))
		{
			toequip = inter.iobj[player.equiped[i]];

			if (toequip)
			{
				if ((toequip->type_flags & OBJECT_TYPE_DAGGER)
				        ||	(toequip->type_flags & OBJECT_TYPE_1H)
				        ||	(toequip->type_flags & OBJECT_TYPE_2H)
				        ||	(toequip->type_flags & OBJECT_TYPE_BOW)
				   )
				{
					if (toequip->type_flags & OBJECT_TYPE_BOW)
					{
						EERIE_LINKEDOBJ_UnLinkObjectFromObject(target->obj, toequip->obj);
						EERIE_LINKEDOBJ_LinkObjectToObject(target->obj, toequip->obj, "WEAPON_ATTACH", "TEST", toequip); //
						return;
					}

					EERIE_LINKEDOBJ_UnLinkObjectFromObject(target->obj, toequip->obj);
					EERIE_LINKEDOBJ_LinkObjectToObject(target->obj, toequip->obj, "WEAPON_ATTACH", "PRIMARY_ATTACH", toequip); //
					return;
				}
			}
		}
	}
}
//***********************************************************************************************
//***********************************************************************************************
long ARX_EQUIPMENT_GetPlayerWeaponType()
{
	INTERACTIVE_OBJ * io = inter.iobj[0];

	if (!io) return WEAPON_BARE;

	if ((player.equiped[EQUIP_SLOT_WEAPON] != 0)
	        &&	ValidIONum(player.equiped[EQUIP_SLOT_WEAPON]))
	{
		INTERACTIVE_OBJ * toequip = inter.iobj[player.equiped[EQUIP_SLOT_WEAPON]];

		if (toequip)
		{
			if (toequip->type_flags & OBJECT_TYPE_DAGGER)	return WEAPON_DAGGER;

			if (toequip->type_flags & OBJECT_TYPE_1H)		return WEAPON_1H;

			if (toequip->type_flags & OBJECT_TYPE_2H)		return WEAPON_2H;

			if (toequip->type_flags & OBJECT_TYPE_BOW)		return WEAPON_BOW;
		}
	}

	return WEAPON_BARE;
}
//***********************************************************************************************
//***********************************************************************************************
void ARX_EQUIPMENT_LaunchPlayerUnReadyWeapon()
{
	INTERACTIVE_OBJ * io = inter.iobj[0];

	if (!io) return;

	ANIM_HANDLE * anim = io->anims[ANIM_BARE_UNREADY];
	long type = ARX_EQUIPMENT_GetPlayerWeaponType();

	switch (type)
	{
		case WEAPON_DAGGER:
			anim = io->anims[ANIM_DAGGER_UNREADY_PART_1];
			break;
		case WEAPON_1H:
			anim = io->anims[ANIM_1H_UNREADY_PART_1];
			break;
		case WEAPON_2H:
			anim = io->anims[ANIM_2H_UNREADY_PART_1];
			break;
		case WEAPON_BOW:
		{
			anim = io->anims[ANIM_MISSILE_UNREADY_PART_1];

			if (arrowobj) 
			{
				EERIE_LINKEDOBJ_UnLinkObjectFromObject(io->obj, arrowobj);
			}
		}
		break;
		default:
			anim = io->anims[ANIM_BARE_UNREADY];
			break;
	}

	AcquireLastAnim(io);
	ANIM_Set(&io->animlayer[1], anim);
}
//***********************************************************************************************
//***********************************************************************************************
float ARX_EQUIPMENT_ComputeDamages(INTERACTIVE_OBJ * io_source, INTERACTIVE_OBJ * io_weapon, INTERACTIVE_OBJ * io_target, float ratioaim, EERIE_3D * position)
{
	EVENT_SENDER = io_source;
	SendIOScriptEvent(io_target, SM_AGGRESSION, "", NULL);

	if ((!io_source)
	        ||	(!io_target))
		return 0.f;

	if (!(io_target->ioflags & IO_NPC))
	{
		if (io_target->ioflags & IO_FIX)
		{
			if (io_source == inter.iobj[0])
				ARX_DAMAGES_DamageFIX(io_target, player.Full_damages, 0, 0);
			else if (io_source->ioflags & IO_NPC)
				ARX_DAMAGES_DamageFIX(io_target, io_source->_npcdata->damages, GetInterNum(io_source), 0);
			else
				ARX_DAMAGES_DamageFIX(io_target, 1, GetInterNum(io_source), 0);
		}

		return 0.f;
	}

	float attack, ac, damages;
	float backstab = 1.f;

	char wmat[64];
	char amat[64];

	strcpy(wmat, "BARE");
	strcpy(amat, "FLESH");

	BOOL critical = FALSE;

	if (io_source == inter.iobj[0]) 
	{
		if ((player.equiped[EQUIP_SLOT_WEAPON] != 0)
		        &&	ValidIONum(player.equiped[EQUIP_SLOT_WEAPON]))
		{
			INTERACTIVE_OBJ * io = inter.iobj[player.equiped[EQUIP_SLOT_WEAPON]];

			if ((io) && (io->weaponmaterial))
			{
				strcpy(wmat, io->weaponmaterial);
			}
		}

		attack = player.Full_damages;

		if (rnd() * 100 <= (float)(player.Full_Attribute_Dexterity - 9) * 2.f + (float)(player.Full_Skill_Close_Combat * DIV5))
		{
			if (SendIOScriptEvent(io_source, SM_CRITICAL, "", NULL) != REFUSE)
				critical = TRUE;
		}
		else critical = FALSE;

		damages = attack * ratioaim; 

		if (io_target->_npcdata->npcflags & NPCFLAG_BACKSTAB)
		{
			if (rnd() * 100.f <= player.Full_Skill_Stealth * DIV2)
			{
				if (SendIOScriptEvent(io_source, SM_BACKSTAB, "", NULL) != REFUSE)
					backstab = 1.5f; 
			}
		}
	}
	else
	{
		if (!(io_source->ioflags & IO_NPC)) return 0.f; // no NPC source...

		if (io_source->weaponmaterial)
		{
			strcpy(wmat, io_source->weaponmaterial);
		}

		if (io_source->_npcdata->weapon != NULL)
		{
			INTERACTIVE_OBJ * iow = (INTERACTIVE_OBJ *)io_source->_npcdata->weapon;

			if (iow->weaponmaterial)
				strcpy(wmat, iow->weaponmaterial);
		}

		attack = io_source->_npcdata->tohit;

		if (GAME_EDITOR)
			damages = io_source->_npcdata->damages * ratioaim;
		else damages = io_source->_npcdata->damages * ratioaim * (rnd() * DIV2 + 0.5f);

		long value = ARX_SPELLS_GetSpellOn(io_source, SPELL_CURSE);

		if (value >= 0)
		{
			damages *= (spells[value].caster_level * 0.05f);
		}

		if (rnd() * 100 <= io_source->_npcdata->critical) 
		{
			if (SendIOScriptEvent(io_source, SM_CRITICAL, "", NULL) != REFUSE)
				critical = TRUE;
		}
		else critical = FALSE;

		if (rnd() * 100.f <= (float)io_source->_npcdata->backstab_skill)
		{
			if (SendIOScriptEvent(io_source, SM_BACKSTAB, "", NULL) != REFUSE)
				backstab = 1.5f; 
		}
	}

	float absorb;

	if (io_target == inter.iobj[0])
	{
		ac = player.Full_armor_class;
		absorb = player.Full_Skill_Defense * DIV2;
	}
	else
	{
		ac = ARX_INTERACTIVE_GetArmorClass(io_target);
		absorb = io_target->_npcdata->absorb;
		long value = ARX_SPELLS_GetSpellOn(io_target, SPELL_CURSE);

		if (value >= 0)
		{
			float modif = (spells[value].caster_level * 0.05f);
			ac *= modif;
			absorb *= modif;
		}
	}


	if (io_target->armormaterial)
	{
		strcpy(amat, io_target->armormaterial);
	}

	if (io_target == inter.iobj[0])
	{
		if ((player.equiped[EQUIP_SLOT_ARMOR] > 0)
		        &&	ValidIONum(player.equiped[EQUIP_SLOT_ARMOR]))
		{
			INTERACTIVE_OBJ * io = inter.iobj[player.equiped[EQUIP_SLOT_ARMOR]];

			if ((io) && (io->armormaterial))
			{
				strcpy(amat, io->armormaterial);
			}
		}
	}

	float dmgs = damages * backstab;
	dmgs -= dmgs * (absorb * DIV100);

	EERIE_3D pos;
	pos.x = io_target->pos.x;
	pos.y = io_target->pos.y;
	pos.z = io_target->pos.z;
	float power;
	power = dmgs * DIV20;

	if (power > 1.f) power = 1.f;

	if (!stricmp(wmat, "BARE"))
		power = power * 0.1f + 0.9f;
	else power = power * 0.1f + 0.9f;

	ARX_SOUND_PlayCollision(amat, wmat, power, 1.f, &pos, io_source);


	float chance = 100.f - (ac - attack); 
	float dice = rnd() * 100.f;

	if (dice <= chance) 
	{
		strcpy(amat, "FLESH");
		ARX_SOUND_PlayCollision(amat, wmat, power, 1.f, &pos, io_source);

		EERIE_3D pos;
		pos.x = io_target->pos.x;
		pos.y = io_target->pos.y + io_target->physics.cyl.height * DIV2;
		pos.z = io_target->pos.z;


		if (dmgs > 0.f)
		{
			if (critical)
			{
				dmgs *= 1.5f; 
			}

			if (io_target == inter.iobj[0])
			{
				EERIE_3D ppos;
				ppos.x = io_source->pos.x - player.pos.x;
				ppos.y = io_source->pos.y - player.pos.y - PLAYER_BASE_HEIGHT;
				ppos.z = io_source->pos.z - player.pos.z;
				Vector_Normalize(&ppos);

				//------- player push START
				EERIE_3D push;
				Vector_Copy(&push, &ppos);
				push.x *= -dmgs * DIV11;
				push.y *= -dmgs * DIV30;
				push.z *= -dmgs * DIV11;
				PUSH_PLAYER_FORCE.x += push.x;
				PUSH_PLAYER_FORCE.y += push.y;
				PUSH_PLAYER_FORCE.z += push.z;
				//------- player push END

				ppos.x *= 60.f;
				ppos.y *= 60.f;
				ppos.z *= 60.f;
				ppos.x += ACTIVECAM->pos.x;
				ppos.y += ACTIVECAM->pos.y;
				ppos.z += ACTIVECAM->pos.z;
				ARX_DAMAGES_SCREEN_SPLATS_Add(&ppos, dmgs);
				ARX_DAMAGES_DamagePlayer(dmgs, 0, GetInterNum(io_source), &io_target->pos);
				ARX_DAMAGES_DamagePlayerEquipment(dmgs);
			}
			else
			{

				EERIE_3D ppos;
				ppos.x = io_source->pos.x - io_target->pos.x;
				ppos.y = io_source->pos.y - io_target->pos.y;
				ppos.z = io_source->pos.z - io_target->pos.z;

				if (io_target == inter.iobj[0]) ppos.y -= PLAYER_BASE_HEIGHT;

				Vector_Normalize(&ppos);

				//------- player NPC START
				EERIE_3D push;
				Vector_Copy(&push, &ppos);
				push.x *= -dmgs;
				push.y *= -dmgs;
				push.z *= -dmgs;
				io_target->forcedmove.x += push.x;
				io_target->forcedmove.y += push.y;
				io_target->forcedmove.z += push.z;

				//------- player NPC END
				if (position)
					ARX_DAMAGES_DamageNPC(io_target, dmgs, GetInterNum(io_source), 0, position);
				else
					ARX_DAMAGES_DamageNPC(io_target, dmgs, GetInterNum(io_source), 0, &io_target->pos);
			}
		}

		return dmgs;
	}


	return 0.f;
}
//***********************************************************************************************
// flags & 1 = blood spawn only
//-----------------------------------------------------------------------------------------------
//***********************************************************************************************
BOOL ARX_EQUIPMENT_Strike_Check(INTERACTIVE_OBJ * io_source, INTERACTIVE_OBJ * io_weapon, float ratioaim, long flags, long targ)
{
	if (TRUEFIGHT) ratioaim = 1.f;

	BOOL ret = FALSE;
	long source = GetInterNum(io_source);
	long weapon = GetInterNum(io_weapon);
	EERIE_SPHERE sphere;

	EERIE_3D * v0;
	EXCEPTIONS_LIST_Pos = 0;
	float rad;

	long nbact = io_weapon->obj->nbaction;
	float drain_life = ARX_EQUIPMENT_GetSpecialValue(io_weapon, IO_SPECIAL_ELEM_DRAIN_LIFE);
	float paralyse = ARX_EQUIPMENT_GetSpecialValue(io_weapon, IO_SPECIAL_ELEM_PARALYZE);

	for (long j = 0; j < nbact; j++)
	{
		if (!ValidIONum(weapon)) return FALSE;

		rad = GetHitValue(io_weapon->obj->actionlist[j].name);

		if (rad == -1) continue;

		v0 = &io_weapon->obj->vertexlist3[io_weapon->obj->actionlist[j].idx].v;
		sphere.origin.x = v0->x;
		sphere.origin.y = v0->y;
		sphere.origin.z = v0->z;
	
		sphere.radius = rad; 

		if (source != 0) sphere.radius += 15.f;

		if (CheckEverythingInSphere(&sphere, source, targ))
		{
			for (long jj = 0; jj < MAX_IN_SPHERE_Pos; jj++)
			{
				float dmgs = 0.f;

				if (ValidIONum(EVERYTHING_IN_SPHERE[jj])
				        && (!(inter.iobj[EVERYTHING_IN_SPHERE[jj]]->ioflags & IO_BODY_CHUNK)))
				{
					long HIT_SPARK = 0;
					EXCEPTIONS_LIST[EXCEPTIONS_LIST_Pos] = EVERYTHING_IN_SPHERE[jj];
					EXCEPTIONS_LIST_Pos++;

					if (EXCEPTIONS_LIST_Pos >= MAX_IN_SPHERE) EXCEPTIONS_LIST_Pos--;

					INTERACTIVE_OBJ * target = inter.iobj[EVERYTHING_IN_SPHERE[jj]];
			
					EERIE_3D	pos;
					D3DCOLOR	color		=	ARX_OPAQUE_WHITE;
					long		hitpoint	=	-1;
					float		curdist		=	999999.f;
					
					EERIE_3D vector;
					vector.x = sphere.origin.x - target->pos.x;
					vector.y = (sphere.origin.y - target->pos.y) * DIV2;
					vector.z = sphere.origin.z - target->pos.z;
					float t = 1.f / TRUEVector_Magnitude(&vector);
					vector.x *= t;
					vector.y *= t;
					vector.z *= t;

					for (long ii = 0; ii < target->obj->nbfaces; ii++)
					{
						if (target->obj->facelist[ii].facetype & POLY_HIDE) continue;

						float dist = TRUEEEDistance3D(&sphere.origin, &target->obj->vertexlist3[target->obj->facelist[ii].vid[0]].v);

						if (dist < curdist)
						{
							hitpoint = target->obj->facelist[ii].vid[0];
							curdist = dist;
						}
					}

					if (hitpoint >= 0)
					{
						if (target->ioflags & IO_NPC)
							color = target->_npcdata->blood_color;
						else color = 0xFFFFFFFF;

						Vector_Copy(&pos, &target->obj->vertexlist3[hitpoint].v);
					}
					else ARX_CHECK_NO_ENTRY(); 
					

					if (!(flags & 1))
					{
						EERIE_3D posi;

						if (hitpoint >= 0)
						{
							Vector_Copy(&posi, &target->obj->vertexlist3[hitpoint].v);
							dmgs = ARX_EQUIPMENT_ComputeDamages(io_source, io_weapon, target, ratioaim, &posi);

						}
						else
						{
							dmgs = ARX_EQUIPMENT_ComputeDamages(io_source, io_weapon, target, ratioaim);

						}

						if (target->ioflags & IO_NPC)
						{
							ret = TRUE;
							target->spark_n_blood = 0;
							target->_npcdata->SPLAT_TOT_NB = 0;

							if (drain_life > 0.f)
							{
								float life_gain = __min(dmgs, drain_life);
								life_gain = __min(life_gain, target->_npcdata->life);
								life_gain = __max(life_gain, 0.f);
								ARX_DAMAGES_HealInter(io_source, life_gain);
							}

							if (paralyse > 0.f)
							{
								float ptime = __min(dmgs * 1000.f, paralyse);
								ARX_SPELLS_Launch(SPELL_PARALYSE, weapon, SPELLCAST_FLAG_NOMANA | SPELLCAST_FLAG_NOCHECKCANCAST
								                  , 5, EVERYTHING_IN_SPHERE[jj], (long)(ptime));
							}
						}

						if (io_source == inter.iobj[0])
						{
							ARX_DAMAGES_DurabilityCheck(io_weapon, 0.2f);
						}
					}

					if ((dmgs > 0.f) || ((target->ioflags & IO_NPC) && (target->spark_n_blood == SP_BLOODY)))
					{
						if (target->ioflags & IO_NPC)
						{
							target->spark_n_blood = SP_BLOODY;

							if (!(flags & 1))
							{
								ARX_PARTICLES_Spawn_Splat(&pos, dmgs, color, hitpoint, target);

								EERIE_SPHERE sp;
								float power;
								power = (dmgs * DIV40) + 0.7f;
								EERIE_3D vect;
								vect.x = target->obj->vertexlist3[hitpoint].v.x - io_source->pos.x;
								vect.y = 0;
								vect.z = target->obj->vertexlist3[hitpoint].v.z - io_source->pos.z;
								Vector_Normalize(&vect);
								sp.origin.x = target->obj->vertexlist3[hitpoint].v.x + vect.x * 30.f;
								sp.origin.y = target->obj->vertexlist3[hitpoint].v.y;
								sp.origin.z = target->obj->vertexlist3[hitpoint].v.z + vect.z * 30.f;
								sp.radius = 3.5f * power * 20;

								if (CheckAnythingInSphere(&sp, 0, 1))
								{
									EERIE_RGB rgb;
									rgb.r = (float)((long)((color >> 16) & 255)) * DIV255;
									rgb.g = (float)((long)((color >> 8) & 255)) * DIV255;
									rgb.b = (float)((long)((color) & 255)) * DIV255;
									SpawnGroundSplat(&sp, &rgb, 30, 1);
								}
							}

							if (target == inter.iobj[0])
								ARX_DAMAGES_SCREEN_SPLATS_Add(&pos, dmgs);

							ARX_PARTICLES_Spawn_Blood2(&pos, dmgs, color, hitpoint, target);

							if (!ValidIONum(weapon)) io_weapon = NULL;
						}
						else
						{
							if (target->ioflags & IO_ITEM)
								ARX_PARTICLES_Spawn_Spark(&pos, rnd() * 3.f, 0);
							else
								ARX_PARTICLES_Spawn_Spark(&pos, rnd() * 30.f, 0);

							ARX_NPC_SpawnAudibleSound(&pos, io_source);

							if (io_source == inter.iobj[0])
								HIT_SPARK = 1;
						}
					}
					else if ((target->ioflags & IO_NPC)
					         &&	((dmgs <= 0.f) || (target->spark_n_blood == SP_SPARKING)))
					{
						long  nb;

						if (target->spark_n_blood == SP_SPARKING) nb = (long)(float)(rnd() * 3.f);
						else nb = 30;

						if (target->ioflags & IO_ITEM)
							nb = 1;

						ARX_PARTICLES_Spawn_Spark(&pos, (float)nb, 0); 
						ARX_NPC_SpawnAudibleSound(&pos, io_source);
						target->spark_n_blood = SP_SPARKING;

						if (!(target->ioflags & IO_NPC))
							HIT_SPARK = 1;
					}
					else if ((dmgs <= 0.f)
					         &&	((target->ioflags & IO_FIX) || (target->ioflags & IO_ITEM)))
					{
						long  nb;

						if (target->spark_n_blood == SP_SPARKING) nb = (long)(float)(rnd() * 3.f);
						else nb = 30;

						if (target->ioflags & IO_ITEM)
							nb = 1;

						ARX_PARTICLES_Spawn_Spark(&pos, (float)nb, 0);
						ARX_NPC_SpawnAudibleSound(&pos, io_source);
						target->spark_n_blood = SP_SPARKING;

						if (!(target->ioflags & IO_NPC))
							HIT_SPARK = 1;
					}

					if (HIT_SPARK)
					{
						if (!(io_source->aflags & IO_NPC_AFLAG_HIT_BACKGROUND))
						{
							ARX_DAMAGES_DurabilityCheck(io_weapon, 1.f);
							io_source->aflags |= IO_NPC_AFLAG_HIT_BACKGROUND;

							if (!ValidIONum(weapon))
							{
								io_weapon = NULL;
							}
							else
							{
								char weapon_material[64] = "";

								if (io_weapon && io_weapon->weaponmaterial) strcpy(weapon_material, io_weapon->weaponmaterial);
								else
									strcpy(weapon_material, "METAL");

								char bkg_material[128];

								if (ARX_MATERIAL_GetNameById(target->material, bkg_material))
									ARX_SOUND_PlayCollision(weapon_material, bkg_material, 1.f, 1.f, &sphere.origin, NULL);
							}
						}
					}

				}
			}
		}


		EERIEPOLY * ep;

		if (ep = CheckBackgroundInSphere(&sphere))
		{
			if (io_source == inter.iobj[0])
			{
				if (!(io_source->aflags & IO_NPC_AFLAG_HIT_BACKGROUND))
				{
					ARX_DAMAGES_DurabilityCheck(io_weapon, 1.f);
					io_source->aflags |= IO_NPC_AFLAG_HIT_BACKGROUND;

					if (!ValidIONum(weapon))
					{
						io_weapon = NULL;
					}
					else
					{
						char weapon_material[64] = "";

						if (io_weapon && io_weapon->weaponmaterial) strcpy(weapon_material, io_weapon->weaponmaterial);
						else
							strcpy(weapon_material, "METAL");

						char bkg_material[64] = "EARTH";

						if (ep &&  ep->tex && ep->tex->m_texName)
							GetMaterialString(ep->tex->m_texName, bkg_material);

						ARX_SOUND_PlayCollision(weapon_material, bkg_material, 1.f, 1.f, &sphere.origin, io_source);
					}
				}
			}

			ARX_PARTICLES_Spawn_Spark(&sphere.origin, rnd() * 10.f, 0);
			ARX_NPC_SpawnAudibleSound(&sphere.origin, io_source);
		}
	}

	return ret;
}

//***********************************************************************************************
//-----------------------------------------------------------------------------------------------
//***********************************************************************************************
void ARX_EQUIPMENT_LaunchPlayerReadyWeapon()
{
	INTERACTIVE_OBJ * io = inter.iobj[0];

	if (!io) return;

	long type = ARX_EQUIPMENT_GetPlayerWeaponType();
	ANIM_HANDLE * anim = NULL;

	switch (type)
	{
		case WEAPON_DAGGER:
			anim = io->anims[ANIM_DAGGER_READY_PART_1];
			break;
		case WEAPON_1H:
			anim = io->anims[ANIM_1H_READY_PART_1];
			break;
		case WEAPON_2H:

			if (player.equiped[EQUIP_SLOT_SHIELD] == 0)
				anim = io->anims[ANIM_2H_READY_PART_1];

			break;
		case WEAPON_BOW:

			if (player.equiped[EQUIP_SLOT_SHIELD] == 0)
				anim = io->anims[ANIM_MISSILE_READY_PART_1];

			break;
		default:
			anim = io->anims[ANIM_BARE_READY];
			break;
	}

	AcquireLastAnim(io);
	ANIM_Set(&io->animlayer[1], anim);
}

//***********************************************************************************************
//-----------------------------------------------------------------------------------------------
//***********************************************************************************************

void ARX_EQUIPMENT_UnEquipPlayerWeapon()
{
	if ((player.equiped[EQUIP_SLOT_WEAPON] != 0)
	        &&	ValidIONum(player.equiped[EQUIP_SLOT_WEAPON]))
	{
		INTERACTIVE_OBJ * pioOldDragInter;
		pioOldDragInter = DRAGINTER;
		DRAGINTER = inter.iobj[player.equiped[EQUIP_SLOT_WEAPON]];

		if (DRAGINTER)
			ARX_SOUND_PlayInterface(SND_INVSTD);

		ARX_EQUIPMENT_UnEquip(inter.iobj[0], inter.iobj[player.equiped[EQUIP_SLOT_WEAPON]]);
		DRAGINTER = pioOldDragInter;
	}

	player.equiped[EQUIP_SLOT_WEAPON] = 0;
}

bool bRing = false;

//***********************************************************************************************
//-----------------------------------------------------------------------------------------------
//***********************************************************************************************
void ARX_EQUIPMENT_Equip(INTERACTIVE_OBJ * target, INTERACTIVE_OBJ * toequip)
{
	if (!target) return;

	if (!toequip) return;

	if (target != inter.iobj[0]) return;

	long validid = -1;

	for (long i = 0; i < inter.nbmax; i++)
	{
		if (inter.iobj[i] == toequip)
		{
			validid = i;
			break;
		}
	}

	if (validid == -1) return;

	RemoveFromAllInventories(toequip);
	toequip->show = SHOW_FLAG_ON_PLAYER; // on player

	if (toequip == DRAGINTER)
		Set_DragInter(NULL);

	if ((toequip->type_flags & OBJECT_TYPE_DAGGER)
	        ||	(toequip->type_flags & OBJECT_TYPE_1H)
	        ||	(toequip->type_flags & OBJECT_TYPE_2H)
	        ||	(toequip->type_flags & OBJECT_TYPE_BOW)
	   )
	{
		if ((player.equiped[EQUIP_SLOT_WEAPON] != 0)
		        &&	ValidIONum(player.equiped[EQUIP_SLOT_WEAPON]))
		{
			ARX_EQUIPMENT_UnEquip(target, inter.iobj[player.equiped[EQUIP_SLOT_WEAPON]]);
		}

		player.equiped[EQUIP_SLOT_WEAPON] = (short)validid;

		if (toequip->type_flags & OBJECT_TYPE_BOW)
		{
			EERIE_LINKEDOBJ_LinkObjectToObject(target->obj, toequip->obj, "WEAPON_ATTACH", "TEST", toequip); //
		}
		else
		{
			EERIE_LINKEDOBJ_LinkObjectToObject(target->obj, toequip->obj, "WEAPON_ATTACH", "PRIMARY_ATTACH", toequip); //
		}

		if ((toequip->type_flags & OBJECT_TYPE_2H) || (toequip->type_flags & OBJECT_TYPE_BOW))
		{
			if (player.equiped[EQUIP_SLOT_SHIELD] != 0)
			{
				ARX_EQUIPMENT_UnEquip(target, inter.iobj[player.equiped[EQUIP_SLOT_SHIELD]]);
			}
		}
	}
	else if (toequip->type_flags & OBJECT_TYPE_SHIELD)
	{
		if ((player.equiped[EQUIP_SLOT_SHIELD] != 0)
		        &&	ValidIONum(player.equiped[EQUIP_SLOT_SHIELD]))
		{
			ARX_EQUIPMENT_UnEquip(target, inter.iobj[player.equiped[EQUIP_SLOT_SHIELD]]);
		}

		player.equiped[EQUIP_SLOT_SHIELD] = (short)validid;
		EERIE_LINKEDOBJ_LinkObjectToObject(target->obj, toequip->obj, "SHIELD_ATTACH", "SHIELD_ATTACH", toequip);

		if ((player.equiped[EQUIP_SLOT_WEAPON] != 0)
		        &&	ValidIONum(player.equiped[EQUIP_SLOT_WEAPON]))
		{
			if ((inter.iobj[player.equiped[EQUIP_SLOT_WEAPON]]->type_flags & OBJECT_TYPE_2H) ||
			        (inter.iobj[player.equiped[EQUIP_SLOT_WEAPON]]->type_flags & OBJECT_TYPE_BOW))
			{
				ARX_EQUIPMENT_UnEquip(target, inter.iobj[player.equiped[EQUIP_SLOT_WEAPON]]);
			}
		}
	}
	else if (toequip->type_flags & OBJECT_TYPE_RING)
	{
		// check first, if not already equiped
		if (!((ValidIONum(player.equiped[EQUIP_SLOT_RING_LEFT]) && (toequip == inter.iobj[player.equiped[EQUIP_SLOT_RING_LEFT]]))
		        ||	(ValidIONum(player.equiped[EQUIP_SLOT_RING_RIGHT]) && (toequip == inter.iobj[player.equiped[EQUIP_SLOT_RING_RIGHT]]))))
		{
			long willequip = -1;

			if (player.equiped[EQUIP_SLOT_RING_LEFT] == 0) willequip = EQUIP_SLOT_RING_LEFT;

			if (player.equiped[EQUIP_SLOT_RING_RIGHT] == 0) willequip = EQUIP_SLOT_RING_RIGHT;

			if (willequip == -1)
			{
				if (bRing)
				{
					if (ValidIONum(player.equiped[EQUIP_SLOT_RING_RIGHT]))
						ARX_EQUIPMENT_UnEquip(target, inter.iobj[player.equiped[EQUIP_SLOT_RING_RIGHT]]);

					willequip = EQUIP_SLOT_RING_RIGHT;
				}
				else
				{
					if (ValidIONum(player.equiped[EQUIP_SLOT_RING_LEFT]))
						ARX_EQUIPMENT_UnEquip(target, inter.iobj[player.equiped[EQUIP_SLOT_RING_LEFT]]);

					willequip = EQUIP_SLOT_RING_LEFT;
				}

				bRing = !bRing;
			}

			player.equiped[willequip] = (short)validid;
		}
	}
	else if (toequip->type_flags & OBJECT_TYPE_ARMOR)
	{
		if ((player.equiped[EQUIP_SLOT_ARMOR] != 0)
		        &&	ValidIONum(player.equiped[EQUIP_SLOT_ARMOR]))
		{
			ARX_EQUIPMENT_UnEquip(target, inter.iobj[player.equiped[EQUIP_SLOT_ARMOR]]);
		}

		player.equiped[EQUIP_SLOT_ARMOR] = (short)validid;
	}
	else if (toequip->type_flags & OBJECT_TYPE_LEGGINGS)
	{
		if ((player.equiped[EQUIP_SLOT_LEGGINGS] != 0)
		        &&	ValidIONum(player.equiped[EQUIP_SLOT_LEGGINGS]))
		{
			ARX_EQUIPMENT_UnEquip(target, inter.iobj[player.equiped[EQUIP_SLOT_LEGGINGS]]);
		}

		player.equiped[EQUIP_SLOT_LEGGINGS] = (short)validid;
	}
	else if (toequip->type_flags & OBJECT_TYPE_HELMET)
	{
		if ((player.equiped[EQUIP_SLOT_HELMET] != 0)
		        &&	(ValidIONum(player.equiped[EQUIP_SLOT_HELMET])))
		{
			ARX_EQUIPMENT_UnEquip(target, inter.iobj[player.equiped[EQUIP_SLOT_HELMET]]);
		}

		player.equiped[EQUIP_SLOT_HELMET] = (short)validid;
	}

	if ((toequip->type_flags & OBJECT_TYPE_HELMET)
	        ||	(toequip->type_flags & OBJECT_TYPE_ARMOR)
	        ||	(toequip->type_flags & OBJECT_TYPE_LEGGINGS))
		ARX_EQUIPMENT_RecreatePlayerMesh();

	ARX_PLAYER_ComputePlayerFullStats();
}
//***********************************************************************************************
// Sets/unsets an object type flag
//-----------------------------------------------------------------------------------------------
//***********************************************************************************************
void ARX_EQUIPMENT_SetObjectType(INTERACTIVE_OBJ * io, char * temp, long val)
{
	// avoid erroneous objects
	if (!io) return;

	// retrieves flag
	unsigned long flagg = ARX_EQUIPMENT_GetObjectTypeFlag(temp);

	if (val)	// add flag
		io->type_flags |= flagg;
	else		// remove flag
		io->type_flags &= ~flagg;
}
//***********************************************************************************************
// Initializes Equipment infos
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/29)
//***********************************************************************************************
void ARX_EQUIPMENT_Init()
{
	// IO_EQUIPITEM_ELEMENT_... are Defined in EERIEPOLY.h
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_STRENGTH].name, "strength");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_DEXTERITY].name, "dexterity");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_CONSTITUTION].name, "constitution");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_MIND].name, "intelligence");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Stealth].name, "stealth");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Mecanism].name, "mecanism");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Intuition].name, "intuition");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Etheral_Link].name, "etheral_link");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Object_Knowledge].name, "object_knowledge");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Casting].name, "casting");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Projectile].name, "projectile");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Close_Combat].name, "close_combat");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Defense].name, "defense");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Armor_Class].name, "armor_class");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Resist_Magic].name, "resist_magic");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Resist_Poison].name, "resist_poison");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Critical_Hit].name, "critical_hit");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Damages].name, "damages");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Duration].name, "duration");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_AimTime].name, "aim_time");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Identify_Value].name, "identify_value");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Life].name, "life");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Mana].name, "mana");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_MaxLife].name, "maxlife");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_MaxMana].name, "maxmana");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_SPECIAL_1].name, "special1");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_SPECIAL_2].name, "special2");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_SPECIAL_3].name, "special3");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_SPECIAL_4].name, "special4");
}
//***********************************************************************************************
// Releases Equipment Structure
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/29)
//***********************************************************************************************
void ARX_EQUIPMENT_ReleaseEquipItem(INTERACTIVE_OBJ * io)
{
	if (!io) return;

	if (!(io->ioflags & IO_ITEM)) return;

	if (io->_itemdata->equipitem)
	{
		free(io->_itemdata->equipitem);
		io->_itemdata->equipitem = NULL;
	}
}
//***********************************************************************************************
// Removes All special equipement properties
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/29)
//***********************************************************************************************
void ARX_EQUIPMENT_Remove_All_Special(INTERACTIVE_OBJ * io)
{
	if (!io) return;

	if (!(io->ioflags & IO_ITEM)) return;

	io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_SPECIAL_1].special = IO_SPECIAL_ELEM_NONE;
	io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_SPECIAL_2].special = IO_SPECIAL_ELEM_NONE;
	io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_SPECIAL_3].special = IO_SPECIAL_ELEM_NONE;
	io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_SPECIAL_4].special = IO_SPECIAL_ELEM_NONE;
}
//***********************************************************************************************
// Sets an equipment property
//-----------------------------------------------------------------------------------------------
//***********************************************************************************************
float ARX_EQUIPMENT_Apply(INTERACTIVE_OBJ * io, long ident, float trueval)
{
	if (io == NULL) return trueval;

	if (io != inter.iobj[0]) return trueval;

	float toadd = 0;

	for (long i = 0; i < MAX_EQUIPED; i++)
	{
		if ((player.equiped[i] != 0)
		        &&	ValidIONum(player.equiped[i]))
		{
			INTERACTIVE_OBJ * toequip = inter.iobj[player.equiped[i]];

			if ((toequip) && (toequip->ioflags & IO_ITEM) && (toequip->_itemdata->equipitem))
			{
				IO_EQUIPITEM_ELEMENT * elem = &toequip->_itemdata->equipitem->elements[ident];

				if (!(elem->flags & 1)) 
					toadd += elem->value;
			}
		}
	}

	return toadd;
}

float ARX_EQUIPMENT_ApplyPercent(INTERACTIVE_OBJ * io, long ident, float trueval)
{
	if (io == NULL) return trueval;

	if (io != inter.iobj[0]) return trueval;

	float toadd = 0;

	for (long i = 0; i < MAX_EQUIPED; i++)
	{
		if ((player.equiped[i] != 0)
		        &&	ValidIONum(player.equiped[i]))
		{
			INTERACTIVE_OBJ * toequip = inter.iobj[player.equiped[i]];

			if ((toequip) && (toequip->ioflags & IO_ITEM) && (toequip->_itemdata->equipitem))
			{
				IO_EQUIPITEM_ELEMENT * elem = &toequip->_itemdata->equipitem->elements[ident];

				if (elem->flags & 1) // percentile value...
				{
					toadd += elem->value;
				}
			}
		}
	}

	return (toadd * trueval * DIV100);
}
 
//***********************************************************************************************
//-----------------------------------------------------------------------------------------------
//***********************************************************************************************

float ARX_EQUIPMENT_GetSpecialValue(INTERACTIVE_OBJ * io, long val)
{
	if ((!io) || !(io->ioflags & IO_ITEM) || !io->_itemdata->equipitem) return -1;

	for (long i = IO_EQUIPITEM_ELEMENT_SPECIAL_1; i <= IO_EQUIPITEM_ELEMENT_SPECIAL_4; i++)
	{
		if (io->_itemdata->equipitem->elements[i].special == val)
		{
			return (io->_itemdata->equipitem->elements[i].value);
		}
	}

	return -1;
}
void ARX_EQUIPMENT_SetEquip(INTERACTIVE_OBJ * io, char * param1, char * param2, float val, short flags)
{
	if (io == NULL) return;

	if (!(io->ioflags & IO_ITEM)) return;

	if (!io->_itemdata->equipitem)
	{
		io->_itemdata->equipitem = (IO_EQUIPITEM *) malloc(sizeof(IO_EQUIPITEM)); //"IOequip"

		if (io->_itemdata->equipitem == NULL) return;

		memset(io->_itemdata->equipitem, 0, sizeof(IO_EQUIPITEM));
		io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Duration].value = 10;
		io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_AimTime].value = 10;
		io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Damages].value = 0;
		io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Identify_Value].value = 0;
	}

	if (!stricmp(param1, "-s"))
	{
		for (long i = IO_EQUIPITEM_ELEMENT_SPECIAL_1; i <= IO_EQUIPITEM_ELEMENT_SPECIAL_4; i++)
		{
			if (io->_itemdata->equipitem->elements[i].special == IO_SPECIAL_ELEM_NONE)
			{
				if (!stricmp(param2, "PARALYSE"))
					io->_itemdata->equipitem->elements[i].special = IO_SPECIAL_ELEM_PARALYZE;
				else if (!stricmp(param2, "DRAINLIFE"))
					io->_itemdata->equipitem->elements[i].special = IO_SPECIAL_ELEM_DRAIN_LIFE;

				io->_itemdata->equipitem->elements[i].value = val;
				io->_itemdata->equipitem->elements[i].flags = flags;
				return;
			}
		}

		return;
	}
	else
		for (long i = 0; i < IO_EQUIPITEM_ELEMENT_Number; i++)
		{
			if (!stricmp(param2, equipinfo[i].name))
			{
				io->_itemdata->equipitem->elements[i].value = val;
				io->_itemdata->equipitem->elements[i].special = 0;
				io->_itemdata->equipitem->elements[i].flags = flags;
				return;
			}
		}
}

//-----------------------------------------------------------------------------
void ARX_EQUIPMENT_IdentifyAll()
{
	INTERACTIVE_OBJ * io = inter.iobj[0];

	if (io == NULL) return;

	if (io != inter.iobj[0]) return;

	for (long i = 0; i < MAX_EQUIPED; i++)
	{
		if ((player.equiped[i] != 0)
		        &&	ValidIONum(player.equiped[i]))
		{
			INTERACTIVE_OBJ * toequip = inter.iobj[player.equiped[i]];

			if ((toequip) && (toequip->ioflags & IO_ITEM) && (toequip->_itemdata->equipitem))
			{
				if (player.Full_Skill_Object_Knowledge + player.Full_Attribute_Mind
				        >= toequip->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Identify_Value].value)
				{
					SendIOScriptEvent(toequip, SM_IDENTIFY, "");
				}
			}
		}
	}
}
float GetHitValue(char * name)
{
	long len = strlen(name);

	if (len < 5) return -1;

	if (((name[0] == 'H') || (name[0] == 'h'))
	        && ((name[1] == 'I') || (name[1] == 'i'))
	        && ((name[2] == 'T') || (name[1] == 't'))
	        && (name[3] == '_'))
	{
		long val = atoi(name + 4);
		return (float)val;
	}

	return -1;
}
