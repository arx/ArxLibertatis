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
// ARX_Paths
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Paths Management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include "ai/Paths.h"

#include <cstdio>
#include <cassert>

#include "animation/Animation.h"

#include "core/Dialog.h"
#include "core/GameTime.h"
#include "core/Core.h"

#include "game/NPC.h"
#include "game/Player.h"
#include "game/Damage.h"
#include "game/Equipment.h"
#include "game/Inventory.h"

#include "graphics/GraphicsModes.h"
#include "graphics/Draw.h"
#include "graphics/effects/SpellEffects.h"
#include "graphics/particle/ParticleEffects.h"

#include "io/FilePath.h"

#include "platform/String.h"

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
extern float FrameDiff;
bool IsPointInField(Vec3f * pos);
ARX_PATH **	ARXpaths = NULL;
ARX_USE_PATH USE_CINEMATICS_PATH;
MASTER_CAMERA_STRUCT MasterCamera;
PathMods ARX_PATHS_HIERARCHYMOVE = 0;
long		nbARXpaths = 0;
long USE_CINEMATICS_CAMERA = 0;

//*************************************************************************************
//*************************************************************************************
#ifdef BUILD_EDITOR
ARX_PATH * ARX_PATHS_AddNew(Vec3f * pos)
{
	char str[64];
	char tex[64];

	if (ARXpaths == NULL)
	{
		ARXpaths = (ARX_PATH **)malloc(sizeof(ARX_PATH *)); 
		nbARXpaths = 1;

		ARXpaths[0] = ARX_PATHS_Create("newpath0001", pos);
		ARX_PATHS_AddPathWay(ARXpaths[0], 0);
		return ARXpaths[0];
	}

	ARXpaths = (ARX_PATH **)realloc(ARXpaths, sizeof(ARX_PATH *) * (nbARXpaths + 1));
	strcpy(tex, "newpath");
	long num = 1;
	sprintf(str, "%s%04ld", tex, num);
	num++;

	while (ARX_PATHS_ExistName(str))
	{
		sprintf(str, "%s%04ld", tex, num);
		num++;
	}

	ARXpaths[nbARXpaths] = ARX_PATHS_Create(str, pos);
	ARX_PATHS_AddPathWay(ARXpaths[nbARXpaths], 0);
	nbARXpaths++;
	return ARXpaths[nbARXpaths-1];
}
//*************************************************************************************
//*************************************************************************************
void ARX_PATHS_RedrawAll()
{
	ARX_PATHS_FlyingOverAP = NULL;
	ARX_PATHS_FlyingOverNum = -1;

	for (long i = 0; i < nbARXpaths; i++)
		ARX_PATHS_DrawPath(ARXpaths[i]);
}
#endif

void ARX_PATH_ComputeBB(ARX_PATH * ap)
{
	ap->bbmin.x = ap->bbmin.y = ap->bbmin.z = 9999999999.f;
	ap->bbmax.x = ap->bbmax.y = ap->bbmax.z = -9999999999.f;

	for (long i = 0; i < ap->nb_pathways; i++)
	{
		ap->bbmin.x = std::min(ap->bbmin.x, ap->pos.x + ap->pathways[i].rpos.x);
		ap->bbmax.x = std::max(ap->bbmax.x, ap->pos.x + ap->pathways[i].rpos.x);

		ap->bbmin.z = std::min(ap->bbmin.z, ap->pos.z + ap->pathways[i].rpos.z);
		ap->bbmax.z = std::max(ap->bbmax.z, ap->pos.z + ap->pathways[i].rpos.z);
	}

	if (ap->height > 0)
	{
		ap->bbmin.y = ap->pos.y - ap->height;
		ap->bbmax.y = ap->pos.y;
	}
	else
	{
		ap->bbmin.y = -99999999.f;
		ap->bbmax.y = 99999999.f;
	}
}
void ARX_PATH_ComputeAllBoundingBoxes()
{
	for (long i = 0; i < nbARXpaths; i++)
	{
		if (ARXpaths[i])
		{
			ARX_PATH_ComputeBB(ARXpaths[i]);
		}
	}
}
long ARX_PATH_IsPosInZone(ARX_PATH * ap, float x, float y, float z)
{
	if (x < ap->bbmin.x) return 0;

	if (x > ap->bbmax.x) return 0;

	if (z < ap->bbmin.z) return 0;

	if (z > ap->bbmax.z) return 0;

	if (y < ap->bbmin.y) return 0;

	if (y > ap->bbmax.y) return 0;

	register int i, j, c = 0;

	x -= ap->pos.x;
	z -= ap->pos.z;

	ARX_PATHWAY * app = ap->pathways;

	for (i = 0, j = ap->nb_pathways - 1; i < ap->nb_pathways; j = i++)
	{
		Vec3f * pi = &app[i].rpos;
		Vec3f * pj = &app[j].rpos;

		if ((((pi->z <= z) && (z < pj->z)) ||
		        ((pj->z <= z) && (z < pi->z))) &&
		        (x < (pj->x - pi->x) *(z - pi->z) / (pj->z - pi->z) + pi->x))
			c = !c;
	}

	return c;
}
ARX_PATH * ARX_PATH_CheckInZone(INTERACTIVE_OBJ * io)
{
	if (ARXpaths)
	{
		Vec3f curpos;
		GetItemWorldPosition(io, &curpos);

		for (long i = 0; i < nbARXpaths; i++)
		{
			if ((ARXpaths[i]) && (ARXpaths[i]->height != 0))
			{
				if (ARX_PATH_IsPosInZone(ARXpaths[i], curpos.x, curpos.y, curpos.z))
					return ARXpaths[i];
			}
		}
	}

	return NULL;
}
ARX_PATH * ARX_PATH_CheckPlayerInZone()
{
	if (ARXpaths)
		for (long i = 0; i < nbARXpaths; i++)
		{
			if ((ARXpaths[i]) && (ARXpaths[i]->height != 0))
			{
				if (ARX_PATH_IsPosInZone(ARXpaths[i], player.pos.x, player.pos.y + 160.f, player.pos.z))
					return ARXpaths[i];
			}
		}

	return NULL;
}
long JUST_RELOADED = 0;

void ARX_PATH_UpdateAllZoneInOutInside()
{
	if (EDITMODE) return;

	static long count = 1;

	long f	=	ARX_CLEAN_WARN_CAST_LONG(FrameDiff);

	if (f < 10) f = 10;

	if (f > 50) f = 50;

	if (count >= inter.nbmax) count = 1;

	if (inter.nbmax > 1)
		for (long tt = 0; tt < f; tt++)
		{
			long i = count;
			INTERACTIVE_OBJ * io = inter.iobj[i];

			if ((count < inter.nbmax) && (io)
			        && (io->ioflags & (IO_NPC | IO_ITEM))
			        && (io->show != SHOW_FLAG_MEGAHIDE)
			        && (io->show != SHOW_FLAG_DESTROYED)

			   )
			{
				ARX_PATH * p = ARX_PATH_CheckInZone(io);
				ARX_PATH * op = io->inzone;

				if ((op == NULL) && (p == NULL)) goto next; // Not in a zone

				if (op == p)	// Stayed inside Zone OP
				{
					if (io->show != io->inzone_show)
					{
						io->inzone_show = io->show;
						goto entering;
					}
				}
				else if ((op != NULL) && (p == NULL)) // Leaving Zone OP
				{
					SendIOScriptEvent(io, SM_LEAVEZONE, op->name); 

					if (op->controled[0] != 0)
					{
						long t = GetTargetByNameTarget(op->controled);

						if (t >= 0)
						{
							string str = io->long_name() + ' ' + op->name;
							SendIOScriptEvent(inter.iobj[t], SM_CONTROLLEDZONE_LEAVE, str);
						}
					}
				}
				else if ((op == NULL) && (p != NULL)) // Entering Zone P
				{
					io->inzone_show = io->show;
				entering:
					;


					if ((JUST_RELOADED)
					        &&	((!strcasecmp(p->name, "ingot_maker")) || (!strcasecmp(p->name, "mauld_user"))))
					{
						ARX_DEAD_CODE(); 
					}
					else
					{
						SendIOScriptEvent(io, SM_ENTERZONE, p->name); 

						if (p->controled[0] != 0)
						{
							long t = GetTargetByNameTarget(p->controled);

							if (t >= 0)
							{
								string params = io->long_name() + ' ' + p->name;
								SendIOScriptEvent(inter.iobj[t], SM_CONTROLLEDZONE_ENTER, params); 
							}
						}
					}
				}
				else 
				{
					SendIOScriptEvent(io, SM_LEAVEZONE, op->name); 

					if (op->controled[0] != 0)
					{
						long t = GetTargetByNameTarget(op->controled);

						if (t >= 0)
						{
							string str = io->long_name() + ' ' + op->name;
							SendIOScriptEvent(inter.iobj[t], SM_CONTROLLEDZONE_LEAVE, str); 
						}
					}

					io->inzone_show = io->show;
					SendIOScriptEvent(io, SM_ENTERZONE, p->name); 

					if (p->controled[0] != 0)
					{
						long t = GetTargetByNameTarget(p->controled);

						if (t >= 0)
						{
							string str = io->long_name() + ' ' + p->name;
							SendIOScriptEvent(inter.iobj[t], SM_CONTROLLEDZONE_ENTER, str);
						}
					}
				}

				io->inzone = p;
			}

		next:
			;
			count++;

			if (count >= inter.nbmax) count = 1;
		}

	//player check*************************************************
	if (inter.iobj[0])
	{
		ARX_PATH * p = ARX_PATH_CheckPlayerInZone();
		ARX_PATH * op = (ARX_PATH *)player.inzone;

		if ((op == NULL) && (p == NULL)) goto suite; // Not in a zone

		if (op == p)	// Stayed inside Zone OP
		{
		
		}
		else if ((op != NULL) && (p == NULL)) // Leaving Zone OP
		{
			SendIOScriptEvent(inter.iobj[0], SM_LEAVEZONE, op->name); 
			CHANGE_LEVEL_ICON = -1;

			if (op->controled[0] != 0)
			{
				long t = GetTargetByNameTarget(op->controled);

				if (t >= 0)
				{
					SendIOScriptEvent(inter.iobj[t], SM_CONTROLLEDZONE_LEAVE, string("player ") + op->name);
				}
			}
		}
		else if ((op == NULL) && (p != NULL)) // Entering Zone P
		{
			SendIOScriptEvent(inter.iobj[0], SM_ENTERZONE, p->name); 

			if (p->flags & PATH_AMBIANCE && p->ambiance[0])
				ARX_SOUND_PlayZoneAmbiance(p->ambiance, ARX_SOUND_PLAY_LOOPED, p->amb_max_vol * ( 1.0f / 100 ));

			if (p->flags & PATH_FARCLIP)
			{
				desired.flags |= GMOD_ZCLIP;
				desired.zclip = p->farclip;
			}

			if (p->flags & PATH_REVERB)
			{
			}

			if (p->flags & PATH_RGB)
			{
				desired.flags |= GMOD_DCOLOR;
				desired.depthcolor = p->rgb;
			}

			if (p->controled[0] != 0)
			{
				long t = GetTargetByNameTarget(p->controled);

				if (t >= 0)
				{
					SendIOScriptEvent(inter.iobj[t], SM_CONTROLLEDZONE_ENTER, string("player ") + p->name);
				}
			}
		}
		else 
		{

			if (op->controled[0] != 0)
			{
				long t = GetTargetByNameTarget(op->controled);

				if (t >= 0)
				{
					SendIOScriptEvent(inter.iobj[t], SM_CONTROLLEDZONE_LEAVE, string("player ") + p->name);
				}
			}

			if (p->controled[0] != 0)
			{
				long t = GetTargetByNameTarget(p->controled);

				if (t >= 0)
				{
					SendIOScriptEvent(inter.iobj[t], SM_CONTROLLEDZONE_ENTER, string("player ") + p->name);
				}
			}
		}

		player.inzone = (void *)p;
	}

	
suite:
	;
	JUST_RELOADED = 0;
}

#ifdef BUILD_EDITOR
//*************************************************************************************
//*************************************************************************************
ARX_PATH * ARX_PATHS_Create(const char * name, Vec3f * pos)
{
	ARX_PATH * ap = (ARX_PATH *)malloc(sizeof(ARX_PATH)); 

	if (ap == NULL) return NULL;

	memset(ap, 0, sizeof(ARX_PATH));
	SAFEstrcpy(ap->name, name, 64);
	ap->initpos.x = ap->pos.x = pos->x;
	ap->initpos.y = ap->pos.y = pos->y;
	ap->initpos.z = ap->pos.z = pos->z;
	InterTreeViewItemAdd(NULL, name, IOTVTYPE_PATH);
	return ap;
}
#endif

//*************************************************************************************
//*************************************************************************************
void ARX_PATH_ClearAllUsePath()
{
	for (long i = 0; i < inter.nbmax; i++)
	{
		if (inter.iobj[i] != NULL)
			if (inter.iobj[i]->usepath != NULL)
			{
				free(inter.iobj[i]->usepath);
				inter.iobj[i]->usepath = NULL;
			}
	}
}
//*************************************************************************************
//*************************************************************************************
void ARX_PATH_ClearAllControled()
{
	for (long i = 0; i < nbARXpaths; i++)
	{
		if (ARXpaths[i])
		{
			ARXpaths[i]->controled[0] = 0;
		}
	}
}
//*************************************************************************************
//*************************************************************************************
#ifdef BUILD_EDITOR
void ARX_PATHS_ChangeName(ARX_PATH * ap, char * newname)
{
	if (ap == NULL) return;

	InterTreeViewItemRemove(NULL, ap->name);
	strcpy(ap->name, newname);
	InterTreeViewItemAdd(NULL, ap->name, IOTVTYPE_PATH);
}
#endif
//*************************************************************************************
//*************************************************************************************
ARX_PATH * ARX_PATH_GetAddressByName( const std::string& name)
{

	if ( !(name).empty() && (ARXpaths) )
		for (long i = 0; i < nbARXpaths; i++)
		{
			if (ARXpaths[i])
			{
				if (!strcasecmp(ARXpaths[i]->name, name)) return ARXpaths[i];
			}
		}

	return NULL;
}
//*************************************************************************************
//*************************************************************************************
void ARX_PATH_ReleaseAllPath()
{
	ARX_PATH_ClearAllUsePath();

	for (long i = 0; i < nbARXpaths; i++)
	{
		if (ARXpaths[i])
		{
#ifdef BUILD_EDITOR
			InterTreeViewItemRemove(NULL, ARXpaths[i]->name);
#endif

			if (ARXpaths[i]->pathways) free(ARXpaths[i]->pathways);

			ARXpaths[i]->pathways = NULL;
			free(ARXpaths[i]);
			ARXpaths[i] = NULL;
		}
	}

	if (ARXpaths) free(ARXpaths);

	ARXpaths = NULL;
	nbARXpaths = 0;
}
//*************************************************************************************
//*************************************************************************************
ARX_PATH * ARX_PATHS_ExistName(char * name)
{
	if (ARXpaths == NULL) return NULL;

	for (long i = 0; i < nbARXpaths; i++)
	{
		if (!strcasecmp(ARXpaths[i]->name, name))
			return ARXpaths[i];
	}

	return NULL;
}
//*************************************************************************************
//*************************************************************************************
#ifdef BUILD_EDITOR
void ARX_PATHS_Delete(ARX_PATH * ap)
{
	ARX_PATH_ClearAllUsePath();

	if (ap == NULL) return;

	InterTreeViewItemRemove(NULL, ap->name);

	for (long i = 0; i < nbARXpaths; i++)
	{
		if (ap == ARXpaths[i])
		{
			while (i < nbARXpaths)
			{
				i++;
				ARXpaths[i-1] = ARXpaths[i];
			}

			nbARXpaths--;

			if (nbARXpaths == 0)
			{
				free(ARXpaths);
				ARXpaths = NULL;
			}
			else ARXpaths = (ARX_PATH **)realloc(ARXpaths, sizeof(ARX_PATH *) * (nbARXpaths));

			break;
		}
	}

	if (ap == ARX_PATHS_SelectedAP)
	{
		ARX_PATHS_SelectedAP = NULL;
		ARX_PATHS_SelectedNum = -1;
	}

	free(ap);
	ap = NULL;
}
#endif
//*************************************************************************************
//*************************************************************************************
#ifdef BUILD_EDITOR
long ARX_PATHS_AddPathWay(ARX_PATH * ap, long insert)
{
	if (ap == NULL) return 0;

	if (insert < 0) insert = 0;
	else if (insert > ap->nb_pathways) insert = ap->nb_pathways;

	if (ap->pathways == NULL)
	{
		ap->pathways = (ARX_PATHWAY *)malloc(sizeof(ARX_PATHWAY)); 
		memset(ap->pathways, 0, sizeof(ARX_PATHWAY));
		ap->nb_pathways = 1;
		return 1;
	}

	ap->pathways = (ARX_PATHWAY *)realloc(ap->pathways, sizeof(ARX_PATHWAY) * (ap->nb_pathways + 1));
	memset(&ap->pathways[ap->nb_pathways], 0, sizeof(ARX_PATHWAY));
	ap->nb_pathways++;

	if (insert == ap->nb_pathways - 1) return ap->nb_pathways;

	for (long i = ap->nb_pathways - 1; i > insert; i--)
	{
		memcpy(&ap->pathways[i], &ap->pathways[i-1], sizeof(ARX_PATHWAY));
	}

	memset(&ap->pathways[insert], 0, sizeof(ARX_PATHWAY));
	return insert + 1;
}
#endif
//*************************************************************************************
//*************************************************************************************
long ARX_PATHS_Interpolate(ARX_USE_PATH * aup, Vec3f * pos)
{
	ARX_PATH * ap = aup->path;

	//compute Delta Time
	float tim = aup->_curtime - aup->_starttime;

	if (tim < 0)	return -1;

	//set pos to startpos
	pos->x = 0.f; 
	pos->y = 0.f; 
	pos->z = 0.f; 

	if (tim == 0) return 0;

	//we start at reference waypoint 0  (time & rpos = 0 for this waypoint).
	long targetwaypoint = 1;
	aup->aupflags &= ~ARX_USEPATH_FLAG_FINISHED;

	if (ap->pathways)
	{
		ap->pathways[0]._time = 0;
		ap->pathways[0].rpos.x = 0;
		ap->pathways[0].rpos.y = 0;
		ap->pathways[0].rpos.z = 0;
	}
	else return -1;

	// While we have time left, iterate
	while (tim > 0)
	{
		// Path Ended
		if (targetwaypoint > ap->nb_pathways - 1)
		{
			pos->x += ap->pos.x;
			pos->y += ap->pos.y;
			pos->z += ap->pos.z;
			aup->aupflags |= ARX_USEPATH_FLAG_FINISHED;
			return -2;
		}

		// Manages a Bezier block
		if (ap->pathways[targetwaypoint-1].flag == PATHWAY_BEZIER)
		{

			targetwaypoint += 1;
			float delta = tim - ap->pathways[targetwaypoint]._time;

			if (delta >= 0)
			{
				tim = delta;

				if (targetwaypoint < ap->nb_pathways)
				{
					pos->x = ap->pathways[targetwaypoint].rpos.x;
					pos->y = ap->pathways[targetwaypoint].rpos.y;
					pos->z = ap->pathways[targetwaypoint].rpos.z;
				}

				targetwaypoint += 1;
			}
			else
			{
				if (targetwaypoint < ap->nb_pathways)
				{
					if (ap->pathways[targetwaypoint]._time == 0)
						return targetwaypoint - 1;
					float	rel = (float)((float)(tim)) / ((float)(ap->pathways[targetwaypoint]._time));

					float mul = rel;
					float mull = mul * mul;
 
					pos->x = mul * (ap->pathways[targetwaypoint-1].rpos.x - ap->pathways[targetwaypoint-2].rpos.x) + (ap->pathways[targetwaypoint].rpos.x - ap->pathways[targetwaypoint-1].rpos.x) * mull;
					pos->y = mul * (ap->pathways[targetwaypoint-1].rpos.y - ap->pathways[targetwaypoint-2].rpos.y) + (ap->pathways[targetwaypoint].rpos.y - ap->pathways[targetwaypoint-1].rpos.y) * mull;
					pos->z = mul * (ap->pathways[targetwaypoint-1].rpos.z - ap->pathways[targetwaypoint-2].rpos.z) + (ap->pathways[targetwaypoint].rpos.z - ap->pathways[targetwaypoint-1].rpos.z) * mull;
					pos->x += ap->pos.x + ap->pathways[targetwaypoint-2].rpos.x;
					pos->y += ap->pos.y + ap->pathways[targetwaypoint-2].rpos.y;
					pos->z += ap->pos.z + ap->pathways[targetwaypoint-2].rpos.z;
				}

				return targetwaypoint - 1;
			}
		}
		else
		{
			// Manages a non-Bezier block
			float delta = tim - ap->pathways[targetwaypoint]._time;

			if (delta >= 0)
			{
				tim = delta;

				if (targetwaypoint < ap->nb_pathways)
				{
					pos->x = ap->pathways[targetwaypoint].rpos.x;
					pos->y = ap->pathways[targetwaypoint].rpos.y;
					pos->z = ap->pathways[targetwaypoint].rpos.z;
				}

				targetwaypoint++;
			}
			else
			{
				if (targetwaypoint < ap->nb_pathways)
				{
					if (ap->pathways[targetwaypoint]._time == 0)
						return targetwaypoint - 1;
					float	rel = (float)((float)(tim)) / ((float)(ap->pathways[targetwaypoint]._time));

					pos->x += (ap->pathways[targetwaypoint].rpos.x - pos->x) * rel;
					pos->y += (ap->pathways[targetwaypoint].rpos.y - pos->y) * rel;
					pos->z += (ap->pathways[targetwaypoint].rpos.z - pos->z) * rel;
				}

				pos->x += ap->pos.x;
				pos->y += ap->pos.y;
				pos->z += ap->pos.z;
				return targetwaypoint - 1;
			}
		}
	}

	pos->x += ap->pos.x;
	pos->y += ap->pos.y;
	pos->z += ap->pos.z;
	return targetwaypoint;
}
//*************************************************************************************
//*************************************************************************************
#ifdef BUILD_EDITOR
void ARX_PATHS_ModifyPathWay(ARX_PATH * ap, long num, PathMods mods, Vec3f * pos, PathwayType flags, unsigned long duration)
{
	if (ap == NULL) return;

	if (num < 1) return;

	if (num > ap->nb_pathways) return;

	long to;

	if (mods & ARX_PATH_HIERARCHY) to = ap->nb_pathways - 1;
	else to = num - 1;

	for (long j = num - 1; j <= to; j++)
	{
		if (mods & ARX_PATH_MOD_FLAGS)
		{
			ap->pathways[j].flag = flags;
		}

		if (mods & ARX_PATH_MOD_POSITION)
		{
			if (j == 0)
			{
				for (long n = 1; n < ap->nb_pathways - 1; n++)
				{
					ap->pathways[n].rpos.x += ap->initpos.x - pos->x;;
					ap->pathways[n].rpos.y += ap->initpos.y - pos->y;;
					ap->pathways[n].rpos.z += ap->initpos.z - pos->z;;
				}

				ap->pos.x = ap->initpos.x = pos->x;
				ap->pos.y = ap->initpos.y = pos->y;
				ap->pos.z = ap->initpos.z = pos->z;
				ap->pathways[j].rpos.x = 0.f;
				ap->pathways[j].rpos.y = 0.f;
				ap->pathways[j].rpos.z = 0.f;
			}
			else
			{
				ap->pathways[j].rpos.x = pos->x;
				ap->pathways[j].rpos.y = pos->y;
				ap->pathways[j].rpos.z = pos->z;
			}
		}

		if (mods & ARX_PATH_MOD_TRANSLATE)
		{
			if (j == 0)
			{
				ap->initpos.x += pos->x;
				ap->initpos.y += pos->y;
				ap->initpos.z += pos->z;
				ap->pos.x = ap->initpos.x;
				ap->pos.y = ap->initpos.y;
				ap->pos.z = ap->initpos.z;

				for (long n = 1; n <= ap->nb_pathways - 1; n++)
				{
					ap->pathways[n].rpos.x -= pos->x;
					ap->pathways[n].rpos.y -= pos->y;
					ap->pathways[n].rpos.z -= pos->z;
				}
			}
			else
			{
				ap->pathways[j].rpos.x += pos->x;
				ap->pathways[j].rpos.y += pos->y;
				ap->pathways[j].rpos.z += pos->z;
			}
		}

		if (mods & ARX_PATH_MOD_TIME)
			ap->pathways[j]._time = ARX_CLEAN_WARN_CAST_FLOAT(duration);
	}

	ARX_PATH_ComputeBB(ap);
}
#endif
//*************************************************************************************
//*************************************************************************************
#ifdef BUILD_EDITOR
void ARX_PATHS_DeletePathWay(ARX_PATH * ap, long del)
{
	if (ap == NULL) return;

	if (ap->pathways == NULL) return;

	if (del < 1) return;

	if (del > ap->nb_pathways) return;

	for (long i = del - 1; i < ap->nb_pathways - 2; i++)
	{
		memcpy(&ap->pathways[i], &ap->pathways[i+1], sizeof(ARX_PATHWAY));
	}

	if ((ARX_PATHS_SelectedNum == del) && (ap == ARX_PATHS_SelectedAP))
	{
		ARX_PATHS_SelectedNum--;

		if ((ARX_PATHS_SelectedNum == 0) && (ARX_PATHS_SelectedAP->nb_pathways == 0)) ARX_PATHS_SelectedAP = NULL;
		else ARX_PATHS_SelectedNum = ARX_PATHS_SelectedAP->nb_pathways - 1;
	}

	if (ap->nb_pathways == 1)
	{
		free(ap->pathways);
		ap->nb_pathways = 0;
		ARX_PATHS_Delete(ap);
		return;
	}

	ap->pathways = (ARX_PATHWAY *)realloc(ap->pathways, sizeof(ARX_PATHWAY) * (ap->nb_pathways - 1));
	ap->nb_pathways--;
}

static void ARX_PATHS_DrawPathWay(Vec3f * pos, float siz, Color color) {
	
	TexturedVertex vert;
	vert.sx = pos->x;
	vert.sy = pos->y;
	vert.sz = pos->z;
	
	EERIEDrawSprite(&vert, siz, EERIE_DRAW_square_particle, color, 2.f);
}
ARX_PATH *	ARX_PATHS_FlyingOverAP = NULL;
long		ARX_PATHS_FlyingOverNum = -1;
ARX_PATH *	ARX_PATHS_SelectedAP = NULL;
long		ARX_PATHS_SelectedNum = -1;
//*************************************************************************************
//*************************************************************************************
void ARX_PATHS_DrawPath(ARX_PATH * ap)
{
	if (ap == NULL) return;

	Vec3f from, to; 
	long selected;

	if (ap == ARX_PATHS_SelectedAP) selected = 1;
	else selected = 0;

	for (long i = 0; i < ap->nb_pathways - 1; i++)
	{
		long flagg = ap->pathways[i].flag;

		if (ap->height != 0) flagg = PATHWAY_STANDARD;

		switch (flagg)
		{
			case PATHWAY_STANDARD:
				from = ap->pos + ap->pathways[i].rpos;
				to = ap->pos + ap->pathways[i + 1].rpos;

				if (ap->height != 0)
				{
					if (selected)
					{
						EERIEDraw3DLine(from, to, Color::white);

						if (ap->height > 0)
						{
							to.y -= ap->height;
							from.y -= ap->height;
							EERIEDraw3DLine(from, to, Color::white);
							to.y += ap->height;
							from.y += ap->height;
						}
					}
					else
					{
						EERIEDraw3DLine(from, to, Color::fromBGRA(0xFFCCCCCC));

						if (ap->height > 0)
						{
							to.y -= ap->height;
							from.y -= ap->height;
							EERIEDraw3DLine(from, to, Color::fromBGRA(0xFFCCCCCC));
							to.y += ap->height;
							from.y += ap->height;
						}
					}

					if (i == ap->nb_pathways - 2)
					{
						from.x = ap->pos.x;
						from.y = ap->pos.y;
						from.z = ap->pos.z;

						if (selected)
						{
							EERIEDraw3DLine(from, to, Color::fromBGRA(0xFFDDDDDD));

							if (ap->height > 0)
							{
								to.y -= ap->height;
								from.y -= ap->height;
								EERIEDraw3DLine(from, to, Color::fromBGRA(0xFFDDDDDD));
								to.y += ap->height;
								from.y += ap->height;
							}
						}
						else
						{
							EERIEDraw3DLine(from, to, Color::fromBGRA(0xFFAAAAAA));

							if (ap->height > 0)
							{
								to.y -= ap->height;
								from.y -= ap->height;
								EERIEDraw3DLine(from, to, Color::fromBGRA(0xFFAAAAAA));
								to.y += ap->height;
								from.y += ap->height;
							}
						}
					}
				}
				else
				{
					if (selected) EERIEDraw3DLine(from, to, Color::yellow);
					else EERIEDraw3DLine(from, to, Color::fromBGRA(0xFFAAAAAA));
				}

				break;
			case PATHWAY_BEZIER: {
				#define BEZIERPrecision 32
				Vec3f lastpos, newpos;
				lastpos.x = ap->pos.x + ap->pathways[i].rpos.x;
				lastpos.y = ap->pos.y + ap->pathways[i].rpos.y;
				lastpos.z = ap->pos.z + ap->pathways[i].rpos.z;
				long time;

				for (time = 1; time <= (long)BEZIERPrecision; time++)
				{
					float mul = (float)time / BEZIERPrecision;
					float mull = mul * mul;
 
					newpos.x = mul * (ap->pathways[i+1].rpos.x - ap->pathways[i].rpos.x) + (ap->pathways[i+2].rpos.x - ap->pathways[i+1].rpos.x) * mull;
					newpos.y = mul * (ap->pathways[i+1].rpos.y - ap->pathways[i].rpos.y) + (ap->pathways[i+2].rpos.y - ap->pathways[i+1].rpos.y) * mull;
					newpos.z = mul * (ap->pathways[i+1].rpos.z - ap->pathways[i].rpos.z) + (ap->pathways[i+2].rpos.z - ap->pathways[i+1].rpos.z) * mull;
					newpos.x += ap->pos.x + ap->pathways[i].rpos.x;
					newpos.y += ap->pos.y + ap->pathways[i].rpos.y;
					newpos.z += ap->pos.z + ap->pathways[i].rpos.z;

					if (selected) EERIEDraw3DLine(lastpos, newpos, Color::green);
					else EERIEDraw3DLine(lastpos, newpos, Color::fromBGRA(0xFFAAAAAA));

					lastpos = newpos;
				}

				i++;
				break;
			}
			case PATHWAY_BEZIER_CONTROLPOINT:
				// There MUST be an ERROR.
				return;
				break;
		}
	}

	for (int i = 0; i < ap->nb_pathways; i++)
	{
		from.x = ap->pos.x + ap->pathways[i].rpos.x;
		from.y = ap->pos.y + ap->pathways[i].rpos.y;
		from.z = ap->pos.z + ap->pathways[i].rpos.z;

		if (ap->height > 0)
		{
			to.x = from.x;
			to.y = from.y - ap->height;
			to.z = from.z;
			EERIEDraw3DLine(from, to, Color::fromBGRA(0xFFAAAAAA));
		}

		if ((ARX_PATHS_SelectedNum == (i + 1)) &&
		        (ap == ARX_PATHS_SelectedAP)) ARX_PATHS_DrawPathWay(&from, 4.f, Color::blue);

		if (i == 0)
		{
			if (selected) ARX_PATHS_DrawPathWay(&from, 3.f, Color::red);
			else ARX_PATHS_DrawPathWay(&from, 3.f, Color::fromBGRA(0xFFAA0000));
		}
		else
			switch (ap->pathways[i].flag)
			{
				case PATHWAY_STANDARD:

					if (selected) ARX_PATHS_DrawPathWay(&from, 2.4f, Color::cyan);
					else ARX_PATHS_DrawPathWay(&from, 2.4f, Color::grayb(0xaa));

					break;
				case PATHWAY_BEZIER:

					if (selected) ARX_PATHS_DrawPathWay(&from, 2.4f, Color::green);
					else ARX_PATHS_DrawPathWay(&from, 2.4f, Color::gray(0xaa));

					break;
				case PATHWAY_BEZIER_CONTROLPOINT: break;
			}

		if ((DANAEMouse.x > SPRmins.x) && (DANAEMouse.x < SPRmaxs.x)
		        && (DANAEMouse.y > SPRmins.y) && (DANAEMouse.y < SPRmaxs.y))
		{
			ARX_PATHS_FlyingOverAP = ap;
			ARX_PATHS_FlyingOverNum = i + 1;
		}
	}
}
#endif // BUILD_EDITOR

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//					THROWN OBJECTS MANAGEMENT
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

ARX_THROWN_OBJECT Thrown[MAX_THROWN_OBJECTS];
long Thrown_Count = 0;
void ARX_THROWN_OBJECT_Kill(long num)
{
	if ((num >= 0) && ((size_t)num < MAX_THROWN_OBJECTS))
	{
		Thrown[num].flags = 0;
		Thrown_Count--;

		if (Thrown[num].pRuban)
		{
			delete Thrown[num].pRuban;
			Thrown[num].pRuban = NULL;
		}
	}
}
void ARX_THROWN_OBJECT_KillAll()
{
	for (size_t i = 0; i < MAX_THROWN_OBJECTS; i++)
	{
		ARX_THROWN_OBJECT_Kill(i);
	}

	Thrown_Count = 0;
}
long ARX_THROWN_OBJECT_GetFree()
{
	unsigned long latest_time = ARXTimeUL();
	long latest_obj = -1;

	for (size_t i = 0; i < MAX_THROWN_OBJECTS; i++)
	{
		if (Thrown[i].flags & ATO_EXIST)
		{
			if (Thrown[i].creation_time < latest_time)
			{
				latest_obj = i;
				latest_time = Thrown[i].creation_time;
			}
		}
		else
		{
			return i;
		}
	}

	if (latest_obj >= 0)
	{
		ARX_THROWN_OBJECT_Kill(latest_obj);
		return latest_obj;
	}

	return -1;
}
extern EERIE_3DOBJ * arrowobj;
long ARX_THROWN_OBJECT_Throw(long source, Vec3f * position, Vec3f * vect, Vec3f * upvect, EERIE_QUAT * quat, float velocity, float damages, float poison)
{
	long num = ARX_THROWN_OBJECT_GetFree();

	if (num >= 0)
	{

		Thrown[num].damages = damages;
		Thrown[num].position = *position;
		Thrown[num].initial_position = *position;
		Thrown[num].vector = *vect;
		Thrown[num].upvect = *upvect;
		Quat_Copy(&Thrown[num].quat, quat);
		Thrown[num].source = source;
		Thrown[num].obj = NULL;
		Thrown[num].velocity = velocity;
		Thrown[num].poisonous = poison;
		Thrown[num].pRuban = new CRuban();
		Thrown[num].pRuban->Create(num, 2000);

		Thrown[num].obj = arrowobj;

		if (Thrown[num].obj)
		{
			Thrown[num].creation_time = ARXTimeUL();
			Thrown[num].flags |= ATO_EXIST | ATO_MOVING;
			Thrown_Count++;
		}

		if ((source == 0)
		        &&	(player.equiped[EQUIP_SLOT_WEAPON] != 0)
		        &&	(ValidIONum(player.equiped[EQUIP_SLOT_WEAPON])))
		{
			INTERACTIVE_OBJ * tio = inter.iobj[player.equiped[EQUIP_SLOT_WEAPON]];

			if (tio->ioflags & IO_FIERY)
				Thrown[num].flags |= ATO_FIERY;
		}

	}

	return num;
}
float ARX_THROWN_ComputeDamages(long thrownum, long source, long target)
{
	float				distance_limit	=	1000.f;
	INTERACTIVE_OBJ *	io_target		=	inter.iobj[target];
	INTERACTIVE_OBJ *	io_source		=	inter.iobj[source];

	SendIOScriptEvent(io_target, SM_AGGRESSION);

	float distance = fdist(Thrown[thrownum].position, Thrown[thrownum].initial_position);
	float distance_modifier =	1.f;

	if (distance < distance_limit * 2.f)
	{
		distance_modifier	=	distance / distance_limit;

		if (distance_modifier < 0.5f)
			distance_modifier = 0.5f;
	}
	else distance_modifier = 2.f;

	float attack, dmgs, backstab, critical, ac;

	backstab	=	1.f;
	critical	=	false;

	if (source == 0)
	{
		attack	=	Thrown[thrownum].damages;

		if (rnd() * 100 <= (float)(player.Full_Attribute_Dexterity - 9) * 2.f + (float)((player.Full_Skill_Projectile) * ( 1.0f / 5 )))
		{
			if (SendIOScriptEvent(io_source, SM_CRITICAL, "bow") != REFUSE)
				critical = true;
		}

		dmgs	=	attack;

		if (io_target->_npcdata->npcflags & NPCFLAG_BACKSTAB)
		{
			if (rnd() * 100.f <= player.Full_Skill_Stealth)
			{
				if (SendIOScriptEvent(io_source, SM_BACKSTAB, "bow") != REFUSE)
					backstab = 1.5f;
			}
		}
	}
	else
	{
		// TODO treat NPC !!!

		ARX_CHECK_NO_ENTRY();
		attack = 0;
		dmgs = 0;

	}

	float	absorb;

	if (target == 0)
	{
		ac		=	player.Full_armor_class;
		absorb	=	player.Full_Skill_Defense * ( 1.0f / 2 );
	}
	else
	{
		ac		=	ARX_INTERACTIVE_GetArmorClass(io_target);
		absorb	=	io_target->_npcdata->absorb;
	}

	char wmat[64];
	char amat[64];

	strcpy(wmat, "dagger");
	strcpy(amat, "flesh");

	if (io_target->armormaterial)
	{
		strcpy(amat, io_target->armormaterial);
	}

	if (io_target == inter.iobj[0])
	{
		if (player.equiped[EQUIP_SLOT_ARMOR] > 0)
		{
			INTERACTIVE_OBJ * io	=	inter.iobj[player.equiped[EQUIP_SLOT_ARMOR]];

			if ((io) && (io->armormaterial))
			{
				strcpy(amat, io->armormaterial);
			}
		}
	}

	float power;
	power	=	dmgs * ( 1.0f / 20 );

	if (power > 1.f) power = 1.f;

	power	=	power * 0.15f + 0.85f;

	ARX_SOUND_PlayCollision(amat, wmat, power, 1.f, &Thrown[thrownum].position, io_source);

	dmgs	*=	backstab;
	dmgs	-=	dmgs * (absorb * ( 1.0f / 100 ));

	float chance	= 100.f - (ac - attack);
	float dice		= rnd() * 100.f;

	if (dice <= chance)  
	{
		if (dmgs > 0.f)
		{
			if (critical)
				dmgs *= 1.5f; 

			dmgs *= distance_modifier;
			return dmgs;
		}
	}

	return 0.f;
}

EERIEPOLY * CheckArrowPolyCollision(Vec3f * start, Vec3f * end)
{
	EERIE_TRI pol;
	EERIE_TRI pol2;

	pol.v[0] = *start;
	pol.v[2] = *end;
	pol.v[2].x -= 2.f;
	pol.v[2].y -= 15.f;
	pol.v[2].z -= 2.f;
	pol.v[1] = *end;
	
	long px, pz;
	px = end->x * ACTIVEBKG->Xmul;
	pz = end->z * ACTIVEBKG->Zmul;

	long ix, ax, iz, az;
	ix = std::max(px - 2, 0L);
	ax = std::min(px + 2, ACTIVEBKG->Xsize - 1L);
	iz = std::max(pz - 2, 0L);
	az = std::min(pz + 2, ACTIVEBKG->Zsize - 1L);
	EERIEPOLY * ep;
	FAST_BKG_DATA * feg;

	for (long zz = iz; zz <= az; zz++)
		for (long xx = ix; xx <= ax; xx++)
		{
			feg = &ACTIVEBKG->fastdata[xx][zz];

			for (long k = 0; k < feg->nbpolyin; k++)
			{
				ep = feg->polyin[k];

				if (ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL))
					continue;

				memcpy(&pol2.v[0], &ep->v[0], sizeof(Vec3f)); // TODO wrong size for struct
				memcpy(&pol2.v[1], &ep->v[1], sizeof(Vec3f));
				memcpy(&pol2.v[2], &ep->v[2], sizeof(Vec3f));

				if (Triangles_Intersect(&pol2, &pol)) return ep;

				if (ep->type & POLY_QUAD)
				{
					memcpy(&pol2.v[0], &ep->v[1], sizeof(Vec3f)); // TODO wrong size for struct
					memcpy(&pol2.v[1], &ep->v[3], sizeof(Vec3f));
					memcpy(&pol2.v[2], &ep->v[2], sizeof(Vec3f));

					if (Triangles_Intersect(&pol2, &pol)) return ep;
				}

			}
		}

	return NULL;
}
void CheckExp(long i)
{
	if ((Thrown[i].flags & ATO_FIERY)
	        &&	!(Thrown[i].flags & ATO_UNDERWATER))
	{
		ARX_BOOMS_Add(&Thrown[i].position);
		LaunchFireballBoom(&Thrown[i].position, 10);
		DoSphericDamage(&Thrown[i].position, 4.f * 2, 50.f, DAMAGE_AREA, DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL, 0);
		ARX_SOUND_PlaySFX(SND_SPELL_FIRE_HIT, &Thrown[i].position);
		ARX_NPC_SpawnAudibleSound(&Thrown[i].position, inter.iobj[0]);
		long id = GetFreeDynLight();

		if ((id != -1) && (FrameDiff > 0))
		{
			DynLight[id].exist = 1;

			DynLight[id].intensity = 3.9f;
			DynLight[id].fallstart = 400.f;
			DynLight[id].fallend   = 440.f;
			DynLight[id].rgb.r = (1.f - rnd() * 0.2f);
			DynLight[id].rgb.g = (0.8f - rnd() * 0.2f);
			DynLight[id].rgb.b = (0.6f - rnd() * 0.2f);
			DynLight[id].pos.x = Thrown[i].position.x;
			DynLight[id].pos.y = Thrown[i].position.y;
			DynLight[id].pos.z = Thrown[i].position.z;
			DynLight[id].ex_flaresize = 40.f; 
			DynLight[id].duration = 1500;
		}
	}
}
extern float fZFogEnd;
extern long FRAME_COUNT;
void ARX_THROWN_OBJECT_Manage(unsigned long time_offset)
{
	if (Thrown_Count <= 0) return;

	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetRenderState(Renderer::DepthTest, true);

	for (size_t i = 0; i < MAX_THROWN_OBJECTS; i++)
	{
		if (Thrown[i].flags & ATO_EXIST)
		{
			// Is Object Visible & Near ?
			if(fartherThan(ACTIVECAM->pos, Thrown[i].position, ACTIVECAM->cdepth * fZFogEnd + 50.f)) {
				continue;
			}

			long xx, yy;
			xx = (Thrown[i].position.x)*ACTIVEBKG->Xmul;
			yy = (Thrown[i].position.z)*ACTIVEBKG->Zmul;

			if (xx < 0)
				continue;

			if (xx >= ACTIVEBKG->Xsize)
				continue;

			if (yy < 0)
				continue;

			if (yy >= ACTIVEBKG->Zsize)
				continue;

			FAST_BKG_DATA * feg = (FAST_BKG_DATA *)&ACTIVEBKG->fastdata[xx][yy];

			if (!feg->treat)
				continue;

			// Now render object !
			if (!Thrown[i].obj)
				continue;

			EERIEMATRIX mat;
			MatrixFromQuat(&mat, &Thrown[i].quat);
			long ccount = FRAME_COUNT;
			FRAME_COUNT = 0;
			DrawEERIEInterMatrix(Thrown[i].obj, &mat, &Thrown[i].position, NULL);

			if ((Thrown[i].flags & ATO_FIERY)
			        &&	(Thrown[i].flags & ATO_MOVING)
			        &&	!(Thrown[i].flags & ATO_UNDERWATER))
			{
				long id = GetFreeDynLight();

				if ((id != -1) && (FrameDiff > 0))
				{
					DynLight[id].exist = 1;

					DynLight[id].intensity = 1.f;
					DynLight[id].fallstart = 100.f;
					DynLight[id].fallend   = 240.f;
					DynLight[id].rgb.r = (1.f - rnd() * 0.2f);
					DynLight[id].rgb.g = (0.8f - rnd() * 0.2f);
					DynLight[id].rgb.b = (0.6f - rnd() * 0.2f);
					DynLight[id].pos.x = Thrown[i].position.x;
					DynLight[id].pos.y = Thrown[i].position.y;
					DynLight[id].pos.z = Thrown[i].position.z;
					DynLight[id].ex_flaresize = 40.f; 
					DynLight[id].extras |= EXTRAS_FLARE;
					DynLight[id].duration	=	ARX_CLEAN_WARN_CAST_LONG(FrameDiff * 0.5f);
				}

				float p = 3.f;

				while (p > 0.f)
				{
					p -= 0.5f;

					if (Thrown[i].obj)
					{
						Vec3f	pos;
						long		notok	=	10;
						size_t num = 0;

						while (notok-- > 0)
						{
							num = (rnd() *(float)Thrown[i].obj->facelist.size());
							assert(num < Thrown[i].obj->facelist.size());

							if (Thrown[i].obj->facelist[num].facetype & POLY_HIDE) continue;

							notok = -1;
						}

						if (notok < 0)
						{
							pos = Thrown[i].obj->vertexlist3[Thrown[i].obj->facelist[num].vid[0]].v;

							for (long nn = 0; nn < 2; nn++)
							{
								long j = ARX_PARTICLES_GetFree();

								if ((j != -1) && (!ARXPausedTimer) && (rnd() < 0.4f))
								{
									ParticleCount++;
									PARTICLE_DEF * pd = &particle[j];
									pd->exist	=	true;
									pd->zdec	=	0;
									pd->ov = pos;
									pd->move.x	=	(2.f - 4.f * rnd());
									pd->move.y	=	(2.f - 22.f * rnd());
									pd->move.z	=	(2.f - 4.f * rnd());
									pd->siz		=	7.f;
									pd->tolive	=	500 + (unsigned long)(rnd() * 1000.f);
									pd->special	=	FIRE_TO_SMOKE | ROTATING | MODULATE_ROTATION;
									pd->tc		=	fire2;//tc;
									pd->fparam	=	0.1f - rnd() * 0.2f;
									pd->scale.x	=	-8.f;
									pd->scale.y	=	-8.f;
									pd->scale.z	=	-8.f;
									pd->timcreation	=	lARXTime;
									pd->rgb = Color3f(0.71f, 0.43f, 0.29f);
									pd->delay	=	nn * 180;
								}
							}

						}

					}
				}
			}

			if (Thrown[i].pRuban)
			{

				Thrown[i].pRuban->Update();

				Thrown[i].pRuban->Render();
			}

			FRAME_COUNT = ccount;
			Vec3f original_pos;

			if (Thrown[i].flags & ATO_MOVING)
			{
				long need_kill = 0;
				float mod = (float)time_offset * Thrown[i].velocity;
				original_pos = Thrown[i].position;
				Thrown[i].position.x += Thrown[i].vector.x * mod;
				float gmod = 1.f - Thrown[i].velocity;

				if (gmod > 1.f) gmod = 1.f;
				else if (gmod < 0.f) gmod = 0.f;

				Thrown[i].position.y += Thrown[i].vector.y * mod + (time_offset * gmod);
				Thrown[i].position.z += Thrown[i].vector.z * mod;

				CheckForIgnition(&original_pos, 10.f, 0, 2);

				Vec3f wpos = Thrown[i].position;
				wpos.y += 20.f;
				EERIEPOLY * ep = EEIsUnderWater(&wpos);

				if (Thrown[i].flags & ATO_UNDERWATER)
				{
					if (ep == NULL)
					{
						Thrown[i].flags &= ~ATO_UNDERWATER;
						ARX_SOUND_PlaySFX(SND_PLOUF, &Thrown[i].position);
					}
				}
				else if (ep != NULL)
				{
					Thrown[i].flags |= ATO_UNDERWATER;
					ARX_SOUND_PlaySFX(SND_PLOUF, &Thrown[i].position);
				}

				// Check for collision MUST be done after DRAWING !!!!
				long nbact = Thrown[i].obj->actionlist.size();

				for (long j = 0; j < nbact; j++)
				{ // TODO iterator
					float rad = -1;
					rad = GetHitValue(Thrown[i].obj->actionlist[j].name);
					rad *= ( 1.0f / 2 );

					if (rad == -1) continue;

					Vec3f * v0;
					v0 = &Thrown[i].obj->vertexlist3[Thrown[i].obj->actionlist[j].idx].v;
					Vec3f orgn, dest;
					dest.x = original_pos.x + Thrown[i].vector.x * 95.f;
					dest.y = original_pos.y + Thrown[i].vector.y * 95.f;
					dest.z = original_pos.z + Thrown[i].vector.z * 95.f;
					orgn.x = original_pos.x - Thrown[i].vector.x * 25.f;
					orgn.y = original_pos.y - Thrown[i].vector.y * 25.f;
					orgn.z = original_pos.z - Thrown[i].vector.z * 25.f;
					EERIEPOLY * ep = CheckArrowPolyCollision(&orgn, &dest); 

					if (ep)
					{
						ARX_PARTICLES_Spawn_Spark(v0, 14, 0); 
						CheckExp(i);

						if (ValidIONum(Thrown[i].source))
							ARX_NPC_SpawnAudibleSound(v0, inter.iobj[Thrown[i].source]);

						Thrown[i].flags &= ~ATO_MOVING;
						Thrown[i].velocity = 0.f;
						char weapon_material[64]	= "dagger";
						string bkg_material = "earth";

						if (ep &&  ep->tex && !ep->tex->m_texName.empty())
							bkg_material = GetMaterialString( ep->tex->m_texName );

						if (ValidIONum(Thrown[i].source))
							ARX_SOUND_PlayCollision(weapon_material, bkg_material, 1.f, 1.f, v0, inter.iobj[Thrown[i].source]);

						Thrown[i].position = original_pos;
						j = 200;

					}
					else if (IsPointInField(v0))
					{
						ARX_PARTICLES_Spawn_Spark(v0, 24, 0);
						CheckExp(i);

						if (ValidIONum(Thrown[i].source))
							ARX_NPC_SpawnAudibleSound(v0, inter.iobj[Thrown[i].source]);

						Thrown[i].flags &= ~ATO_MOVING;
						Thrown[i].velocity = 0.f;
						char weapon_material[64]	= "dagger";
						char bkg_material[64]		= "earth";

						if (ValidIONum(Thrown[i].source))
							ARX_SOUND_PlayCollision(weapon_material, bkg_material, 1.f, 1.f, v0, inter.iobj[Thrown[i].source]);

						Thrown[i].position = original_pos;
						j = 200;
						need_kill = 1;
					}
					else
						for (float precision = 0.5f; precision <= 6.f; precision += 0.5f)
						{
							EERIE_SPHERE sphere;
							sphere.origin.x = v0->x + Thrown[i].vector.x * precision * 4.5f;
							sphere.origin.y = v0->y + Thrown[i].vector.y * precision * 4.5f;
							sphere.origin.z = v0->z + Thrown[i].vector.z * precision * 4.5f;
							sphere.radius = rad + 3.f; 
	
							if (CheckEverythingInSphere(&sphere, Thrown[i].source, -1))
							{
								for (size_t jj = 0; jj < MAX_IN_SPHERE_Pos; jj++)
								{
 
 

									if ((ValidIONum(EVERYTHING_IN_SPHERE[jj])
									        && (EVERYTHING_IN_SPHERE[jj] != Thrown[i].source)))
									{

										INTERACTIVE_OBJ * target = inter.iobj[EVERYTHING_IN_SPHERE[jj]];

										if (target->ioflags & IO_NPC)
										{
											Vec3f	pos;
											Color color = Color::none;
											long		hitpoint	=	-1;
											float		curdist		=	999999.f;

											for (size_t ii = 0 ; ii < target->obj->facelist.size() ; ii++)
											{
												if (target->obj->facelist[ii].facetype & POLY_HIDE) continue;

												float d = dist(sphere.origin, target->obj->vertexlist3[target->obj->facelist[ii].vid[0]].v);

												if (d < curdist)
												{
													hitpoint	=	target->obj->facelist[ii].vid[0];
													curdist		=	d;
												}
											}

											if (hitpoint >= 0)
											{
												color = target->_npcdata->blood_color;
												pos = target->obj->vertexlist3[hitpoint].v;
											}

											if (Thrown[i].source == 0)
											{
												float damages;

												if ((damages = ARX_THROWN_ComputeDamages(i, Thrown[i].source, EVERYTHING_IN_SPHERE[jj])) > 0.f)
												{
													ARX_CHECK(hitpoint >= 0);

													if (target->ioflags & IO_NPC)
													{
														target->_npcdata->SPLAT_TOT_NB = 0;
														ARX_PARTICLES_Spawn_Blood2(original_pos, damages, color, target);
													}

													ARX_PARTICLES_Spawn_Blood2(pos, damages, color, target);
													ARX_DAMAGES_DamageNPC(target, damages, Thrown[i].source, 0, &pos);

													if (rnd() * 100.f > target->_npcdata->resist_poison)
													{
														target->_npcdata->poisonned += Thrown[i].poisonous;
													}

													CheckExp(i);
												}
												else
												{
													ARX_PARTICLES_Spawn_Spark(v0, 14, 0);  //dmgs);
													ARX_NPC_SpawnAudibleSound(v0, inter.iobj[Thrown[i].source]);
												}
											}
										}
										else // not NPC
										{
											if (target->ioflags & IO_FIX)
											{
												if (ValidIONum(Thrown[i].source))
													ARX_DAMAGES_DamageFIX(target, 0.1f, Thrown[i].source, 0);
											}

											ARX_PARTICLES_Spawn_Spark(v0, 14, 0);

											if (ValidIONum(Thrown[i].source))
												ARX_NPC_SpawnAudibleSound(v0, inter.iobj[Thrown[i].source]);

											CheckExp(i);
										}

										// Need to deal damages !
										Thrown[i].flags		&=	~ATO_MOVING;
										Thrown[i].velocity	=	0.f;
										need_kill			=	1;
										precision			=	500.f;
										j					=	200;
									}
								}
							}
						}
				}

				if (need_kill) ARX_THROWN_OBJECT_Kill(i);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// RUBAN
//-----------------------------------------------------------------------------
void CRuban::Create(int _iNumThrow, int _iDuration)
{
	iNumThrow = _iNumThrow;

	key = 1;
	duration = _iDuration;
	currduration = 0;

	nbrubandef = 0;

	int nb = 2048;

	while (nb--)
	{
		truban[nb].actif = 0;
	}

	float col = 0.1f + (rnd() * 0.1f);
	float size = 2.f + (2.f * rnd());
	int taille = 8 + (int)(8.f * rnd());
	AddRubanDef(0, size, taille, col, col, col, 0.f, 0.f, 0.f);

}

//-----------------------------------------------------------------------------
void CRuban::AddRubanDef(int origin, float size, int dec, float r, float g, float b, float r2, float g2, float b2)
{
	if (nbrubandef > 255) return;

	trubandef[nbrubandef].first = -1;
	trubandef[nbrubandef].origin = origin;
	trubandef[nbrubandef].size = size;
	trubandef[nbrubandef].dec = dec;
	trubandef[nbrubandef].r = r;
	trubandef[nbrubandef].g = g;
	trubandef[nbrubandef].b = b;
	trubandef[nbrubandef].r2 = r2;
	trubandef[nbrubandef].g2 = g2;
	trubandef[nbrubandef].b2 = b2;
	nbrubandef++;
}

//-----------------------------------------------------------------------------
int CRuban::GetFreeRuban()
{
	int nb = 2048;

	while (nb--)
	{
		if (!truban[nb].actif) return nb;
	}

	return -1;
}

//-----------------------------------------------------------------------------
void CRuban::AddRuban(int * f, int dec)
{
	int	num;

	num = GetFreeRuban();

	if (num >= 0)
	{
		truban[num].actif = 1;

		truban[num].pos.x = Thrown[iNumThrow].position.x;
		truban[num].pos.y = Thrown[iNumThrow].position.y;
		truban[num].pos.z = Thrown[iNumThrow].position.z;

		if (*f < 0)
		{
			*f = num;
			truban[num].next = -1;
		}
		else
		{
			truban[num].next = *f;
			*f = num;
		}

		int nb = 0, oldnum = 0;

		while (num != -1)
		{
			nb++;
			oldnum = num;
			num = truban[num].next;
		}

		if (nb > dec)
		{

			truban[oldnum].actif = 0;
			num = *f;
			nb -= 2;

			while (nb--)
			{
				num = truban[num].next;
			}

			truban[num].next = -1;
		}
	}
}

//-----------------------------------------------------------------------------
void CRuban::Update() {
	
	int	nb, num;

	if (ARXPausedTimer) return;

	num = 0;
	nb = nbrubandef;

	while (nb--)
	{
		AddRuban(&trubandef[num].first, trubandef[num].dec);
		num++;
	}
}

//-----------------------------------------------------------------------------
void CRuban::DrawRuban(int num, float size, int dec, float r, float g, float b, float r2, float g2, float b2)
{
	int numsuiv;

	float	dsize = size / (float)(dec + 1);
	int		r1 = ((int)(r * 255.f)) << 16;
	int		g1 = ((int)(g * 255.f)) << 16;
	int		b1 = ((int)(b * 255.f)) << 16;
	int		rr2 = ((int)(r2 * 255.f)) << 16;
	int		gg2 = ((int)(g2 * 255.f)) << 16;
	int		bb2 = ((int)(b2 * 255.f)) << 16;
	int		dr = (rr2 - r1) / dec;
	int		dg = (gg2 - g1) / dec;
	int		db = (bb2 - b1) / dec;

	for (;;)
	{
		numsuiv = truban[num].next;

		if ((num >= 0) && (numsuiv >= 0))
		{
			Draw3DLineTex2(truban[num].pos, truban[numsuiv].pos, size, Color(r1 >> 16, g1 >> 16, b1 >> 16, 0), Color((r1 + dr) >> 16, (g1 + dg) >> 16, (b1 + db) >> 16, 0));
			r1 += dr;
			g1 += dg;
			b1 += db;
			size -= dsize;
		}
		else
		{
			break;
		}

		num = numsuiv;
	}
}

//-----------------------------------------------------------------------------
float CRuban::Render()
{
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->ResetTexture(0);

	for (int i = 0; i < nbrubandef; i++)
	{
		this->DrawRuban(trubandef[i].first,
		                trubandef[i].size,
		                trubandef[i].dec,
		                trubandef[i].r, trubandef[i].g, trubandef[i].b,
		                trubandef[i].r2, trubandef[i].g2, trubandef[i].b2) ;
	}

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendZero);

	return 0;
}
extern bool IsValidPos3(Vec3f * pos);

#define FORCE_THRESHOLD 290.f
extern long PHYS_COLLIDER;
extern EERIEPOLY * LAST_COLLISION_POLY;
extern long CUR_COLLISION_MATERIAL;
extern bool IsObjectVertexInValidPosition(EERIE_3DOBJ * obj, long kk, long flags, long source);
 
extern float VELOCITY_THRESHOLD;

void ARX_ApplySpring(PHYSVERT * phys, long k, long l, float PHYSICS_constant, float PHYSICS_Damp)
{
	Vec3f deltaP, deltaV, springforce;
	PHYSVERT * pv_k = &phys[k];
	PHYSVERT * pv_l = &phys[l];
	float Dterm, Hterm;

	float restlength = dist(pv_k->initpos, pv_l->initpos);
	//Computes Spring Magnitude
	deltaP.x = pv_k->pos.x - pv_l->pos.x;		
	deltaP.y = pv_k->pos.y - pv_l->pos.y;	
	deltaP.z = pv_k->pos.z - pv_l->pos.z;		
	float dist = (float)TRUEsqrt(deltaP.x * deltaP.x + deltaP.y * deltaP.y + deltaP.z * deltaP.z); // Magnitude of delta
	float divdist = 1.f / dist;
	Hterm = (dist - restlength) * PHYSICS_constant;	

	deltaV.x = pv_k->velocity.x - pv_l->velocity.x;
	deltaV.y = pv_k->velocity.y - pv_l->velocity.y;
	deltaV.z = pv_k->velocity.z - pv_l->velocity.z;		// Delta Velocity Vector
	Dterm = (deltaV dot deltaP) * PHYSICS_Damp * divdist; // Damping Term
	Dterm = (-(Hterm + Dterm));
	divdist *= Dterm;
	springforce.x = deltaP.x * divdist;	// Normalize Distance Vector
	springforce.y = deltaP.y * divdist;	// & Calc Force
	springforce.z = deltaP.z * divdist;

	pv_k->force.x += springforce.x;	// + force on particle 1
	pv_k->force.y += springforce.y;
	pv_k->force.z += springforce.z;

	pv_l->force.x -= springforce.x;	// - force on particle 2
	pv_l->force.y -= springforce.y;
	pv_l->force.z -= springforce.z;
}

void ComputeForces(PHYSVERT * phys, long nb)
{
	Vec3f PHYSICS_Gravity;
	PHYSICS_Gravity.x = 0.f;
	PHYSICS_Gravity.y = 65.f; 
	PHYSICS_Gravity.z = 0.f;


	float PHYSICS_Damping = 0.5f; 
	float lastmass = 1.f;
	float div = 1.f;

	for (long k = 0; k < nb; k++)
	{
		PHYSVERT * pv = &phys[k];
		// Reset Force
		pv->force.x = pv->inertia.x;
		pv->force.y = pv->inertia.y;
		pv->force.z = pv->inertia.z;

		// Apply Gravity
		if (pv->mass > 0.f)
		{
			//need to be precomputed...
			if (lastmass != pv->mass)
			{
				div = 1.f / pv->mass;
				lastmass = pv->mass;
			}

			pv->force.x += (PHYSICS_Gravity.x * div);
			pv->force.y += (PHYSICS_Gravity.y * div);
			pv->force.z += (PHYSICS_Gravity.z * div);
		}

		// Apply Damping
		pv->force.x += (-PHYSICS_Damping * pv->velocity.x);
		pv->force.y += (-PHYSICS_Damping * pv->velocity.y);
		pv->force.z += (-PHYSICS_Damping * pv->velocity.z);
	}

	for (int k = 0; k < nb; k++)
	{
		// Now Resolves Spring System
		for (long l = 0; l < nb; l++)
		{
			if (l != k) ARX_ApplySpring(phys, l, k, 15.f, 0.99f); //18.f,0.4f);
		}
	}
}
bool ARX_INTERACTIVE_CheckFULLCollision(EERIE_3DOBJ * obj, long source);

 
///////////////////////////////////////////////////////////////////////////////
// Function:	RK4Integrate
// 	Calculate new Positions and Velocities given a deltatime
// 	DeltaTime that has passed since last iteration
///////////////////////////////////////////////////////////////////////////////
void RK4Integrate(EERIE_3DOBJ * obj, float DeltaTime)
{

	/// Local Variables ///////////////////////////////////////////////////////////
	PHYSVERT	* source, *target, *accum1, *accum2, *accum3, *accum4;
	///////////////////////////////////////////////////////////////////////////////
	float		halfDeltaT, sixthDeltaT;
	halfDeltaT = DeltaTime * ( 1.0f / 2 );		// SOME TIME VALUES I WILL NEED
	sixthDeltaT = ( 1.0f / 6 );

	PHYSVERT m_TempSys[5][32];//* pv;


	for (long jj = 0; jj < 4; jj++)
	{
		memcpy(&m_TempSys[jj+1][0], obj->pbox->vert, sizeof(PHYSVERT)*obj->pbox->nb_physvert);

		if (jj == 3)
			halfDeltaT = DeltaTime;

		for (long kk = 0; kk < obj->pbox->nb_physvert; kk++)
		{
			source = &obj->pbox->vert[kk];
			accum1 = &m_TempSys[jj+1][kk];
			target = &m_TempSys[0][kk];


			accum1->force.x = halfDeltaT * source->force.x * source->mass;
			accum1->force.y = halfDeltaT * source->force.y * source->mass;
			accum1->force.z = halfDeltaT * source->force.z * source->mass;

			accum1->velocity.x = halfDeltaT * source->velocity.x;
			accum1->velocity.y = halfDeltaT * source->velocity.y;
			accum1->velocity.z = halfDeltaT * source->velocity.z;
			// DETERMINE THE NEW VELOCITY FOR THE PARTICLE OVER 1/2 TIME
			target->velocity.x = source->velocity.x + (accum1->force.x);
			target->velocity.y = source->velocity.y + (accum1->force.y);
			target->velocity.z = source->velocity.z + (accum1->force.z);

			target->mass = source->mass;

			// SET THE NEW POSITION
			target->pos.x = source->pos.x + (accum1->velocity.x);
			target->pos.y = source->pos.y + (accum1->velocity.y);
			target->pos.z = source->pos.z + (accum1->velocity.z);
		}

		ComputeForces(m_TempSys[0], obj->pbox->nb_physvert); // COMPUTE THE NEW FORCES
	}


	for (long kk = 0; kk < obj->pbox->nb_physvert; kk++)
	{
		source = &obj->pbox->vert[kk];	// CURRENT STATE OF PARTICLE
		target = &obj->pbox->vert[kk];
		accum1 = &m_TempSys[1][kk];
		accum2 = &m_TempSys[2][kk];
		accum3 = &m_TempSys[3][kk];
		accum4 = &m_TempSys[4][kk];

		// DETERMINE THE NEW VELOCITY FOR THE PARTICLE USING RK4 FORMULA
		target->velocity.x = source->velocity.x + ((accum1->force.x + ((accum2->force.x + accum3->force.x) * 2.0f) + accum4->force.x) * sixthDeltaT);
		target->velocity.y = source->velocity.y + ((accum1->force.y + ((accum2->force.y + accum3->force.y) * 2.0f) + accum4->force.y) * sixthDeltaT);
		target->velocity.z = source->velocity.z + ((accum1->force.z + ((accum2->force.z + accum3->force.z) * 2.0f) + accum4->force.z) * sixthDeltaT);
		// DETERMINE THE NEW POSITION FOR THE PARTICLE USING RK4 FORMULA
		target->pos.x = source->pos.x + ((accum1->velocity.x + ((accum2->velocity.x + accum3->velocity.x) * 2.0f) + accum4->velocity.x) * sixthDeltaT * 1.2f);
		target->pos.y = source->pos.y + ((accum1->velocity.y + ((accum2->velocity.y + accum3->velocity.y) * 2.0f) + accum4->velocity.y) * sixthDeltaT * 1.2f);
		target->pos.z = source->pos.z + ((accum1->velocity.z + ((accum2->velocity.z + accum3->velocity.z) * 2.0f) + accum4->velocity.z) * sixthDeltaT * 1.2f);
	}

}
bool IsPointInField(Vec3f * pos)
{
	for (size_t i = 0; i < MAX_SPELLS; i++)
	{
		if ((spells[i].exist)
		        &&	(spells[i].type == SPELL_CREATE_FIELD))
		{
			if (ValidIONum(spells[i].longinfo))
			{
				INTERACTIVE_OBJ * pfrm = inter.iobj[spells[i].longinfo];
				EERIE_CYLINDER cyl;
				cyl.height = -35.f;
				cyl.radius = 35.f;
				cyl.origin.x = pos->x;
				cyl.origin.y = pos->y + 17.5f;
				cyl.origin.z = pos->z;

				if (CylinderPlatformCollide(&cyl, pfrm) != 0.f) return true;
			}
		}
	}

	return false;
}

static bool IsObjectInField(EERIE_3DOBJ * obj) {
	
	for (size_t i = 0; i < MAX_SPELLS; i++)
	{
		if ((spells[i].exist)
		        &&	(spells[i].type == SPELL_CREATE_FIELD))
		{
			if (ValidIONum(spells[i].longinfo))
			{
				INTERACTIVE_OBJ * pfrm = inter.iobj[spells[i].longinfo];
				EERIE_CYLINDER cyl;
				cyl.height = -35.f;
				cyl.radius = 35.f;

				for (long k = 0; k < obj->pbox->nb_physvert; k++)
				{
					PHYSVERT * pv = &obj->pbox->vert[k];
					cyl.origin.x = pv->pos.x;
					cyl.origin.y = pv->pos.y + 17.5f;
					cyl.origin.z = pv->pos.z;

					if (CylinderPlatformCollide(&cyl, pfrm) != 0.f) return true;
				}
			}
		}
	}

	return false;
}
bool IsObjectVertexCollidingPoly(EERIE_3DOBJ * obj, EERIEPOLY * ep, long k, long * validd);

bool _IsObjectVertexCollidingPoly(EERIE_3DOBJ * obj, EERIEPOLY * ep, long k, long * validd)
{
	Vec3f pol[3];
	pol[0].x = ep->v[0].sx;
	pol[0].y = ep->v[0].sy;
	pol[0].z = ep->v[0].sz;
	pol[1].x = ep->v[1].sx;
	pol[1].y = ep->v[1].sy;
	pol[1].z = ep->v[1].sz;
	pol[2].x = ep->v[2].sx;
	pol[2].y = ep->v[2].sy;
	pol[2].z = ep->v[2].sz;

	
	if (ep->type & POLY_QUAD)
	{
		if (IsObjectVertexCollidingTriangle(obj, pol, k, validd)) return true;
		
		pol[1].x = ep->v[2].sx;
		pol[1].y = ep->v[2].sy;
		pol[1].z = ep->v[2].sz;
		pol[2].x = ep->v[3].sx;
		pol[2].y = ep->v[3].sy;
		pol[2].z = ep->v[3].sz;

		if (IsObjectVertexCollidingTriangle(obj, pol, k, validd)) return true;

		return false;
	}

	if (IsObjectVertexCollidingTriangle(obj, pol, k, validd)) return true;

	return false;
}

static bool _IsFULLObjectVertexInValidPosition(EERIE_3DOBJ * obj)
{
	bool ret = true;
	long px, pz;
	float x = obj->pbox->vert[0].pos.x;
	px = x * ACTIVEBKG->Xmul;
	float z = obj->pbox->vert[0].pos.z;
	pz = z * ACTIVEBKG->Zmul;
	long ix, iz, ax, az;
	long n;
	n = obj->pbox->radius * ( 1.0f / 100 );
	n = min(1L, n + 1);
	ix = max(px - n, 0L);
	ax = min(px + n, ACTIVEBKG->Xsize - 1L);
	iz = max(pz - n, 0L);
	az = min(pz + n, ACTIVEBKG->Zsize - 1L);
	LAST_COLLISION_POLY = NULL;
	EERIEPOLY * ep;
	EERIE_BKG_INFO * eg;

	float rad = obj->pbox->radius; 

	for (pz = iz; pz <= az; pz++)
		for (px = ix; px <= ax; px++)
		{
			eg = &ACTIVEBKG->Backg[px+pz*ACTIVEBKG->Xsize];

			for (long k = 0; k < eg->nbpoly; k++)
			{
			
				ep = &eg->polydata[k];

				if ( (ep->area > 190.f)
				    &&	(!(ep->type & (POLY_WATER)))
				    &&	(!(ep->type & (POLY_TRANS)))
				    &&	(!(ep->type & (POLY_NOCOL)))
				)
				{
					if (fartherThan(ep->center, obj->pbox->vert[0].pos, rad + 75.f))
						continue;

					for (long kk = 0; kk < obj->pbox->nb_physvert; kk++)
					{
						float radd = 4.f;

						if (
						    !fartherThan(ep->center, obj->pbox->vert[kk].pos, radd)
						    || !fartherThan(obj->pbox->vert[kk].pos, ep->v[0], radd)
						    || !fartherThan(obj->pbox->vert[kk].pos, ep->v[1], radd)
						    || !fartherThan(obj->pbox->vert[kk].pos, ep->v[2], radd)
						    ||	(Distance3D((ep->v[0].sx + ep->v[1].sx)*( 1.0f / 2 ), (ep->v[0].sy + ep->v[1].sy)*( 1.0f / 2 ), (ep->v[0].sz + ep->v[1].sz)*( 1.0f / 2 ),
						                    obj->pbox->vert[kk].pos.x, obj->pbox->vert[kk].pos.y, obj->pbox->vert[kk].pos.z) <= radd)
						    ||	(Distance3D((ep->v[2].sx + ep->v[1].sx)*( 1.0f / 2 ), (ep->v[2].sy + ep->v[1].sy)*( 1.0f / 2 ), (ep->v[2].sz + ep->v[1].sz)*( 1.0f / 2 ),
						                    obj->pbox->vert[kk].pos.x, obj->pbox->vert[kk].pos.y, obj->pbox->vert[kk].pos.z) <= radd)
						    ||	(Distance3D((ep->v[0].sx + ep->v[2].sx)*( 1.0f / 2 ), (ep->v[0].sy + ep->v[2].sy)*( 1.0f / 2 ), (ep->v[0].sz + ep->v[2].sz)*( 1.0f / 2 ),
						                    obj->pbox->vert[kk].pos.x, obj->pbox->vert[kk].pos.y, obj->pbox->vert[kk].pos.z) <= radd)
						)
						{
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
						for (long kl = 1; kl < obj->pbox->nb_physvert; kl++)
						{
							if (kl != kk)
							{
								Vec3f pos;
								pos.x = (obj->pbox->vert[kk].pos.x + obj->pbox->vert[kl].pos.x) * ( 1.0f / 2 );
								pos.y = (obj->pbox->vert[kk].pos.y + obj->pbox->vert[kl].pos.y) * ( 1.0f / 2 );
								pos.z = (obj->pbox->vert[kk].pos.z + obj->pbox->vert[kl].pos.z) * ( 1.0f / 2 );

								if (
								    !fartherThan(ep->center, pos, radd)
								    || !fartherThan(pos, ep->v[0], radd)
								    || !fartherThan(pos, ep->v[1], radd)
								    || !fartherThan(pos, ep->v[2], radd)
								    ||	(Distance3D((ep->v[0].sx + ep->v[1].sx)*( 1.0f / 2 ), (ep->v[0].sy + ep->v[1].sy)*( 1.0f / 2 ), (ep->v[0].sz + ep->v[1].sz)*( 1.0f / 2 ),
								                    pos.x, pos.y, pos.z) <= radd)
								    ||	(Distance3D((ep->v[2].sx + ep->v[1].sx)*( 1.0f / 2 ), (ep->v[2].sy + ep->v[1].sy)*( 1.0f / 2 ), (ep->v[2].sz + ep->v[1].sz)*( 1.0f / 2 ),
								                    pos.x, pos.y, pos.z) <= radd)
								    ||	(Distance3D((ep->v[0].sx + ep->v[2].sx)*( 1.0f / 2 ), (ep->v[0].sy + ep->v[2].sy)*( 1.0f / 2 ), (ep->v[0].sz + ep->v[2].sz)*( 1.0f / 2 ),
								                    pos.x, pos.y, pos.z) <= radd)
								)
								{
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

				
					if (_IsObjectVertexCollidingPoly(obj, ep, -1, NULL))
					{
						
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

bool ARX_EERIE_PHYSICS_BOX_Compute(EERIE_3DOBJ * obj, float framediff, long source) {
	
	PHYSVERT * pv;
	Vec3f oldpos[32];
	long COUNT = 0;
	COUNT++;

	for (long kk = 0; kk < obj->pbox->nb_physvert; kk++)
	{
		pv = &obj->pbox->vert[kk];
		oldpos[kk].x = pv->pos.x;
		oldpos[kk].y = pv->pos.y;
		oldpos[kk].z = pv->pos.z;
		pv->inertia.x = 0.f;
		pv->inertia.y = 0.f;
		pv->inertia.z = 0.f;

		if (pv->velocity.x > VELOCITY_THRESHOLD) pv->velocity.x = VELOCITY_THRESHOLD;
		else if (pv->velocity.x < -VELOCITY_THRESHOLD) pv->velocity.x = -VELOCITY_THRESHOLD;

		if (pv->velocity.y > VELOCITY_THRESHOLD) pv->velocity.y = VELOCITY_THRESHOLD;
		else if (pv->velocity.y < -VELOCITY_THRESHOLD) pv->velocity.y = -VELOCITY_THRESHOLD;

		if (pv->velocity.z > VELOCITY_THRESHOLD) pv->velocity.z = VELOCITY_THRESHOLD;
		else if (pv->velocity.z < -VELOCITY_THRESHOLD) pv->velocity.z = -VELOCITY_THRESHOLD;
	}

 
	CUR_COLLISION_MATERIAL = MATERIAL_STONE;

	RK4Integrate(obj, framediff);


	PHYS_COLLIDER = -1;
	EERIE_SPHERE sphere;
	pv = &obj->pbox->vert[0];
	sphere.origin.x = pv->pos.x;
	sphere.origin.y = pv->pos.y;
	sphere.origin.z = pv->pos.z;
	sphere.radius = obj->pbox->radius;
	long colidd = 0;

	for (int kk = 0; kk < obj->pbox->nb_physvert; kk += 2)
	{
		pv = &obj->pbox->vert[kk];

		if (!IsValidPos3(&pv->pos))
		{
			colidd = 1;
			break;
		}
	}

	if ((!_IsFULLObjectVertexInValidPosition(obj))
	    ||	ARX_INTERACTIVE_CheckFULLCollision(obj, source)
	    || colidd
	    || (IsObjectInField(obj))
	)
	{
		colidd = 1;
		float power = (EEfabs(obj->pbox->vert[0].velocity.x) + EEfabs(obj->pbox->vert[0].velocity.y) + EEfabs(obj->pbox->vert[0].velocity.z)) * ( 1.0f / 100 );


		if (ValidIONum(source) && (inter.iobj[source]->ioflags & IO_BODY_CHUNK))
		{
		}
		else
			ARX_TEMPORARY_TrySound(0.4f + power);


		if (!LAST_COLLISION_POLY)
		{
			for (long k = 0; k < obj->pbox->nb_physvert; k++)
			{
				pv = &obj->pbox->vert[k];

				{
					pv->velocity.x *= -0.3f;
					pv->velocity.z *= -0.3f;
					pv->velocity.y *= -0.4f;
				}

				pv->pos = oldpos[k];
			}
		}
		else
		{
			for (long k = 0; k < obj->pbox->nb_physvert; k++)
			{
				pv = &obj->pbox->vert[k];


				float t =	(LAST_COLLISION_POLY->norm.x) * (pv->velocity.x) +
				            (LAST_COLLISION_POLY->norm.y) * (pv->velocity.y) +
				            (LAST_COLLISION_POLY->norm.z) * (pv->velocity.z);

				float x = t * LAST_COLLISION_POLY->norm.x;
				float y = t * LAST_COLLISION_POLY->norm.y;
				float z = t * LAST_COLLISION_POLY->norm.z;

				pv->velocity.x = pv->velocity.x - 2.f * x;
				pv->velocity.y = pv->velocity.y - 2.f * y;
				pv->velocity.z = pv->velocity.z - 2.f * z;

				pv->velocity.x *= 0.3f;
				pv->velocity.z *= 0.3f;
				pv->velocity.y *= 0.4f;

				pv->pos = oldpos[k];
			}
		}
	}

	if (colidd)
	{
		obj->pbox->stopcount += 1;
	}
	else
	{
		obj->pbox->stopcount -= 2;

		if (obj->pbox->stopcount < 0)
			obj->pbox->stopcount = 0;
	}

	return true;//ret;
}

long ARX_PHYSICS_BOX_ApplyModel(EERIE_3DOBJ * obj, float framediff, float rubber, long source) {
	
	VELOCITY_THRESHOLD = 400.f; 
	long ret = 0;

	if ((!obj) || (!obj->pbox)) return ret;

	if (obj->pbox->active == 2) return ret;

	if (framediff == 0.f) return ret;

	PHYSVERT * pv;

	// Memorizes initpos
	for (long k = 0; k < obj->pbox->nb_physvert; k++)
	{
		pv = &obj->pbox->vert[k];
		pv->temp.x = pv->pos.x;
		pv->temp.y = pv->pos.y;
		pv->temp.z = pv->pos.z;
	}

	float timing = obj->pbox->storedtiming + framediff * rubber * 0.0055f; 
	float t_threshold = 0.18f; 

	if (timing < t_threshold)
	{
		obj->pbox->storedtiming = timing;
		return 1;
	}
	else
	{

		while (timing >= t_threshold) 
		{

			ComputeForces(obj->pbox->vert, obj->pbox->nb_physvert);

			if (!ARX_EERIE_PHYSICS_BOX_Compute(obj, std::min(0.11f, timing * 10), source))
				ret = 1;

			timing -= t_threshold; 
		}

		obj->pbox->storedtiming = timing;
	}


	if (obj->pbox->stopcount < 16) return ret; 

	obj->pbox->active = 2;
	obj->pbox->stopcount = 0;

	if (ValidIONum(source))
	{
		inter.iobj[source]->soundcount = 0;
		inter.iobj[source]->soundtime = ARXTimeUL() + 2000; 
	}

	return ret;
}

void ARX_PrepareBackgroundNRMLs()
{
	long i, j, k, mai, maj, mii, mij;
	long i2, j2, k2;
	EERIE_BKG_INFO * eg;
	EERIE_BKG_INFO * eg2;
	EERIEPOLY * ep;
	EERIEPOLY * ep2;
	Vec3f nrml;
	Vec3f cur_nrml;
	float count;
	long nbvert;
	long nbvert2;

	for (j = 0; j < ACTIVEBKG->Zsize; j++)
		for (i = 0; i < ACTIVEBKG->Xsize; i++)
		{
			eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];

			for (long l = 0; l < eg->nbpoly; l++)
			{
				ep = &eg->polydata[l];

				if (ep->type & POLY_QUAD) nbvert = 4;
				else nbvert = 3;

				for (k = 0; k < nbvert; k++)
				{
					float ttt = 1.f;

					if (k == 3)
					{
						nrml.x = ep->norm2.x;
						nrml.y = ep->norm2.y;
						nrml.z = ep->norm2.z;
						count = 1.f;
					}
					else if ((k > 0) && (nbvert > 3))
					{
						nrml.x = (ep->norm.x + ep->norm2.x);
						nrml.y = (ep->norm.y + ep->norm2.y);
						nrml.z = (ep->norm.z + ep->norm2.z);
						count = 2.f;
						ttt = ( 1.0f / 2 );
					}
					else
					{
						nrml.x = ep->norm.x;
						nrml.y = ep->norm.y;
						nrml.z = ep->norm.z;
						count = 1.f;
					}

					cur_nrml.x = nrml.x * ttt;
					cur_nrml.y = nrml.y * ttt;
					cur_nrml.z = nrml.z * ttt;

					mai = i + 4;
					maj = j + 4;
					mii = i - 4;
					mij = j - 4;

					if (mij < 0) mij = 0;

					if (mii < 0) mii = 0;

					if (maj >= ACTIVEBKG->Zsize) maj = ACTIVEBKG->Zsize - 1;

					if (mai >= ACTIVEBKG->Xsize) mai = ACTIVEBKG->Xsize - 1;

					for (j2 = mij; j2 < maj; j2++)
						for (i2 = mii; i2 < mai; i2++)
						{
							eg2 = &ACTIVEBKG->Backg[i2+j2*ACTIVEBKG->Xsize];

							for (long kr = 0; kr < eg2->nbpoly; kr++)
							{
								//	continue;
								ep2 = &eg2->polydata[kr];

								if (ep2->type & POLY_QUAD) nbvert2 = 4;
								else nbvert2 = 3;

								if (ep != ep2)

									for (k2 = 0; k2 < nbvert2; k2++)
									{
										if ((EEfabs(ep2->v[k2].sx - ep->v[k].sx) < 2.f)
										        &&	(EEfabs(ep2->v[k2].sy - ep->v[k].sy) < 2.f)
										        &&	(EEfabs(ep2->v[k2].sz - ep->v[k].sz) < 2.f))
										{
											if (k2 == 3)
											{
												if (LittleAngularDiff(&cur_nrml, &ep2->norm2))
												{
													nrml.x += ep2->norm2.x;
													nrml.y += ep2->norm2.y;
													nrml.z += ep2->norm2.z;
													count += 1.f;
													nrml.x += cur_nrml.x;
													nrml.y += cur_nrml.y;
													nrml.z += cur_nrml.z;
													count += 1.f;
												}
											}
											else if ((k2 > 0) && (nbvert2 > 3))
											{
												Vec3f tnrml;
												tnrml.x = (ep2->norm.x + ep2->norm2.x) * ( 1.0f / 2 );
												tnrml.y = (ep2->norm.y + ep2->norm2.y) * ( 1.0f / 2 );
												tnrml.z = (ep2->norm.z + ep2->norm2.z) * ( 1.0f / 2 );

												if (LittleAngularDiff(&cur_nrml, &tnrml))
												{
													nrml.x += tnrml.x * 2.f;
													nrml.y += tnrml.y * 2.f;
													nrml.z += tnrml.z * 2.f;
													count += 2.f;
												}
											}
											else
											{
												if (LittleAngularDiff(&cur_nrml, &ep2->norm))
												{
													nrml.x += ep2->norm.x;
													nrml.y += ep2->norm.y;
													nrml.z += ep2->norm.z;
													count += 1.f;

												}
											}
										}
									}
							}
						}

					count = 1.f / count;
					ep->tv[k].sx = nrml.x * count;

					ep->tv[k].sy = nrml.y * count;

					ep->tv[k].sz = nrml.z * count;

				}
			}
		}

	for (j = 0; j < ACTIVEBKG->Zsize; j++)
		for (i = 0; i < ACTIVEBKG->Xsize; i++)
		{
			eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];

			for (long l = 0; l < eg->nbpoly; l++)
			{
				ep = &eg->polydata[l];

				if (ep->type & POLY_QUAD) nbvert = 4;
				else nbvert = 3;

				for (k = 0; k < nbvert; k++)
				{
					ep->nrml[k].x = ep->tv[k].sx;
					ep->nrml[k].y = ep->tv[k].sy;
					ep->nrml[k].z = ep->tv[k].sz;
				}

				float d = 0.f;

				for(long ii = 0; ii < nbvert; ii++) {
					d = max(d, dist(ep->center, ep->v[ii]));
				}

				ep->v[0].rhw = d;
			}
		}

}

void EERIE_PHYSICS_BOX_Launch_NOCOL(INTERACTIVE_OBJ * io, EERIE_3DOBJ * obj, Vec3f * pos, Vec3f * vect, long flags, Anglef * angle)
{
	io->GameFlags |= GFLAG_NO_PHYS_IO_COL;
	EERIE_PHYSICS_BOX_Launch(obj, pos, vect, flags, angle);
}
