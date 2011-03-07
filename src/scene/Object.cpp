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
// EERIEObject
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include "scene/Object.h"

#include <cstdio>
#include <algorithm>
#include <sstream>
#include <vector>
#include <climits>

#include "animation/AnimationRender.h"

#include "core/Application.h"
#include "core/TheoData.h"

#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"
#include "graphics/data/Progressive.h"

#include "io/IO.h"
#include "io/FilePath.h"
#include "io/PakManager.h"
#include "io/PakEntry.h"
#include "io/Filesystem.h"
#include "io/Logger.h"

#include "physics/Clothes.h"
#include "physics/Box.h"
#include "physics/CollisionShapes.h"

#include "scene/LinkedObject.h"
#include "scene/GameSound.h"

using std::sprintf;
using std::min;
using std::max;

extern char LastLoadedScene[256];
extern PakManager * pPakManager;

void EERIE_RemoveCedricData(EERIE_3DOBJ * eobj);
void EERIEOBJECT_CreatePFaces(EERIE_3DOBJ * eobj);
void EERIEOBJECT_DeletePFaces(EERIE_3DOBJ * eobj);

//-----------------------------------------------------------------------------------------------------
long GetGroupOriginByName(EERIE_3DOBJ * eobj, const char * text)
{
	if (!eobj) return -1;

	for (long i = 0; i < eobj->nbgroups; i++)
	{
		if ( !strcasecmp(text, eobj->grouplist[i].name.c_str() )) return eobj->grouplist[i].origin;
	}

	return -1;
}
//-----------------------------------------------------------------------------------------------------
long GetActionPointIdx(EERIE_3DOBJ * eobj, const char * text)
{
	if (!eobj) return -1;

	for(vector<EERIE_ACTIONLIST>::iterator i = eobj->actionlist.begin();
	    i != eobj->actionlist.end(); ++i) {
		// TODO use string compare
		if (!strcasecmp(text, i->name.c_str() )) return i->idx;
	}

	return -1;
}
//-----------------------------------------------------------------------------------------------------
long GetActionPointGroup(EERIE_3DOBJ * eobj, long idx)
{
	if (!eobj) return -1;

	for (long i = eobj->nbgroups - 1; i >= 0; i--)
	{
		for (size_t j = 0; j < eobj->grouplist[i].indexes.size(); j++)
		{
			if (((long)eobj->grouplist[i].indexes[j]) == idx) return i;
		}
	}

	return -1;
}
//-----------------------------------------------------------------------------------------------------
void EERIE_Object_Precompute_Fast_Access(EERIE_3DOBJ * eerie)
{
	if (!eerie) return;

	long lVRight		=	GetActionPointIdx(eerie, "V_RIGHT");
	long lURight		=	GetActionPointIdx(eerie, "U_RIGHT");
	long lViewAttach	=	GetActionPointIdx(eerie, "View_attach") ;
	long lPrimAttach	=	GetActionPointIdx(eerie, "PRIMARY_ATTACH");
	long lLeftAttach	=	GetActionPointIdx(eerie, "LEFT_ATTACH");

	ARX_CHECK_SHORT(lVRight);
	ARX_CHECK_SHORT(lURight);
	ARX_CHECK_SHORT(lViewAttach);
	ARX_CHECK_SHORT(lPrimAttach);
	ARX_CHECK_SHORT(lLeftAttach);

	eerie->fastaccess.V_right		=	ARX_CLEAN_WARN_CAST_SHORT(lVRight);
	eerie->fastaccess.U_right		=	ARX_CLEAN_WARN_CAST_SHORT(lURight);
	eerie->fastaccess.view_attach	=	ARX_CLEAN_WARN_CAST_SHORT(lViewAttach);
	eerie->fastaccess.primary_attach =	ARX_CLEAN_WARN_CAST_SHORT(lPrimAttach);
	eerie->fastaccess.left_attach	=	ARX_CLEAN_WARN_CAST_SHORT(lLeftAttach);


	long lWeapAttach				=	GetActionPointIdx(eerie, "WEAPON_ATTACH");
	long lSecAttach					=	GetActionPointIdx(eerie, "SECONDARY_ATTACH");
	long lJaw						=	EERIE_OBJECT_GetGroup(eerie, "jaw");
	long lMouthAll					=	EERIE_OBJECT_GetGroup(eerie, "mouth all");

	ARX_CHECK_SHORT(lWeapAttach);
	ARX_CHECK_SHORT(lSecAttach);
	ARX_CHECK_SHORT(lJaw);
	ARX_CHECK_SHORT(lMouthAll);

	eerie->fastaccess.weapon_attach		=	ARX_CLEAN_WARN_CAST_SHORT(lWeapAttach);
	eerie->fastaccess.secondary_attach	=	ARX_CLEAN_WARN_CAST_SHORT(lSecAttach);
	eerie->fastaccess.jaw_group			=	ARX_CLEAN_WARN_CAST_SHORT(lJaw);
	eerie->fastaccess.mouth_group		=	ARX_CLEAN_WARN_CAST_SHORT(lMouthAll);


	if (eerie->fastaccess.mouth_group == -1)
		eerie->fastaccess.mouth_group_origin = -1;
	else
	{
		long lMouthOrigin = eerie->grouplist[eerie->fastaccess.mouth_group].origin;
		ARX_CHECK_SHORT(lMouthOrigin);
		eerie->fastaccess.mouth_group_origin = ARX_CLEAN_WARN_CAST_SHORT(lMouthOrigin);
	}

	long lHeadGroup					=	EERIE_OBJECT_GetGroup(eerie, "head");
	ARX_CHECK_SHORT(lHeadGroup);
	eerie->fastaccess.head_group	=	ARX_CLEAN_WARN_CAST_SHORT(lHeadGroup);

	if (eerie->fastaccess.head_group == -1)
		eerie->fastaccess.head_group_origin = -1;
	else
	{
		long lHeadOrigin  = eerie->grouplist[eerie->fastaccess.head_group].origin;
		ARX_CHECK_SHORT(lHeadOrigin);
		eerie->fastaccess.head_group_origin = ARX_CLEAN_WARN_CAST_SHORT(lHeadOrigin);
	}


	long lFire = GetActionPointIdx(eerie, "FIRE");
	long lCarryAttach = GetActionPointIdx(eerie, "CARRY_ATTACH");
	long lHead = EERIE_OBJECT_GetSelection(eerie, "head");
	long lChest = EERIE_OBJECT_GetSelection(eerie, "chest");
	long lLeggings = EERIE_OBJECT_GetSelection(eerie, "leggings") ;

	ARX_CHECK_SHORT(lFire);
	ARX_CHECK_SHORT(lCarryAttach);
	ARX_CHECK_SHORT(lHead);
	ARX_CHECK_SHORT(lChest);
	ARX_CHECK_SHORT(lLeggings);

	eerie->fastaccess.fire = ARX_CLEAN_WARN_CAST_SHORT(lFire);
	eerie->fastaccess.carry_attach = ARX_CLEAN_WARN_CAST_SHORT(lCarryAttach);
	eerie->fastaccess.sel_head = ARX_CLEAN_WARN_CAST_SHORT(lHead);
	eerie->fastaccess.sel_chest = ARX_CLEAN_WARN_CAST_SHORT(lChest);
	eerie->fastaccess.sel_leggings = ARX_CLEAN_WARN_CAST_SHORT(lLeggings);
}

//-----------------------------------------------------------------------------------------------------
void ReleaseAnim(EERIE_ANIM * ea)
{
	if (!ea) return;

	if (ea->frames)
	{
		for (long i = 0; i < ea->nb_key_frames; i++)
		{
			ARX_SOUND_Free(ea->frames[i].sample);
		}

		free(ea->frames);
	}

	if (ea->groups)
		free(ea->groups);

	if (ea->voidgroups)
		free(ea->voidgroups);

	free(ea);
}
//-----------------------------------------------------------------------------------------------------
float GetTimeBetweenKeyFrames(EERIE_ANIM * ea, long f1, long f2)
{
	if (!ea) return 0;

	if (f1 < 0) return 0;

	if (f1 > ea->nb_key_frames - 1) return 0;

	if (f2 < 0) return 0;

	if (f2 > ea->nb_key_frames - 1) return 0;

	float time = 0;

	for (long kk = f1 + 1; kk <= f2; kk++)
	{
		time += ea->frames[kk].time;
	}

	return time;
}

static void * safeAlloc(size_t size, const char * desc) {
	void * result;
	do {
		result = malloc(size); 
		if(!result) {
			HERMES_Memory_Emergency_Out(size, desc);
		}
	} while(!result);
	return result;
}

static void * safeAllocZero(size_t size, const char * desc) {
	void * result = safeAlloc(size, desc);
	memset(result, 0, size);
	return result;
}

template <class T>
static T * allocStruct(const char * desc, size_t n = 1) {
	return (T*)safeAlloc(n * sizeof(T), desc);
}

template <class T>
static T * allocStructZero(const char * desc, size_t n = 1) {
	return (T*)safeAllocZero(n * sizeof(T), desc);
}

template <class T>
static T * copyStruct(const T * src, const char * desc, size_t n = 1) {
	T * result = allocStruct<T>(desc, n);
	memcpy(result, src, sizeof(T) * n);
	return result;
}



EERIE_ANIM * TheaToEerie(unsigned char * adr, size_t size, const char * file) {
	
	(void)size; // TODO use size
	
	LogInfo << "Loading animation file " << file;
	
	size_t pos = 0;
	
	EERIE_ANIM * eerie = allocStructZero<EERIE_ANIM>("EEAnim");
	
	THEA_HEADER th;
	memcpy(&th, adr + pos, sizeof(THEA_HEADER));
	if(th.version < 2014) {
		LogError << "Invalid TEA Version " << th.version << " in " << file;
		free(eerie);
		return NULL;
	}
	pos += sizeof(THEA_HEADER);
	
	LogDebug << "TEA header size: " << sizeof(THEA_HEADER);
	LogDebug << "Identity " << th.identity;
	LogDebug << "Version - " << th.version << "  Frames " << th.nb_frames
	         << "  Groups " << th.nb_groups << "  KeyFrames " << th.nb_key_frames;
	
	eerie->nb_groups = th.nb_groups;
	eerie->nb_key_frames = th.nb_key_frames;
	
	eerie->frames = allocStructZero<EERIE_FRAME>("EEAnimFrames", th.nb_key_frames);
	eerie->groups = allocStructZero<EERIE_GROUP>("EEAnimGroups", th.nb_key_frames * th.nb_groups);
	eerie->voidgroups = allocStructZero<unsigned char>("EEAnimVoidGroups", th.nb_groups);
	
	eerie->anim_time = 0.f;
	
	// Go For Keyframes read
	for(long i = 0; i < th.nb_key_frames; i++) {
		LogDebug << "Loading keyframe " << i;
		
		THEA_KEYFRAME_2015 tkf2015;
		if(th.version >= 2015) {
			LogDebug << " New keyframe version THEA_KEYFRAME_2015:" << sizeof(THEA_KEYFRAME_2015);
			memcpy(&tkf2015, adr + pos, sizeof(THEA_KEYFRAME_2015));
			pos += sizeof(THEA_KEYFRAME_2015);
		} else {
			LogDebug << " Old keyframe version THEA_KEYFRAME:" << sizeof(THEA_KEYFRAME);
			THEA_KEYFRAME tkf;
			memcpy(&tkf, adr + pos, sizeof(THEA_KEYFRAME));
			pos += sizeof(THEA_KEYFRAME);
			memset(&tkf2015, 0, sizeof(THEA_KEYFRAME_2015));
			tkf2015.num_frame = tkf.num_frame;
			tkf2015.flag_frame = tkf.flag_frame;
			tkf2015.master_key_frame = tkf.master_key_frame;
			tkf2015.key_frame = tkf.key_frame;
			tkf2015.key_move = tkf.key_move;
			tkf2015.key_orient = tkf.key_orient;
			tkf2015.key_morph = tkf.key_morph;
			tkf2015.time_frame = tkf.time_frame;
		}
		
		eerie->frames[i].master_key_frame = tkf2015.master_key_frame;
		eerie->frames[i].num_frame = tkf2015.num_frame;
		
		long lKeyOrient = tkf2015.key_orient ;
		long lKeyMove = tkf2015.key_move ;
		ARX_CHECK_SHORT(tkf2015.key_orient);
		ARX_CHECK_SHORT(tkf2015.key_move);
		eerie->frames[i].f_rotate = ARX_CLEAN_WARN_CAST_SHORT(lKeyOrient);
		eerie->frames[i].f_translate = ARX_CLEAN_WARN_CAST_SHORT(lKeyMove);
		
		tkf2015.time_frame = (tkf2015.num_frame) * 1000; // TODO this looks like a hack
		eerie->frames[i].time = tkf2015.time_frame * ( 1.0f / 24 );
		eerie->anim_time += tkf2015.time_frame;
		eerie->frames[i].flag = tkf2015.flag_frame;
		
		LogDebug << " pos " << pos << " - NumFr " << eerie->frames[i].num_frame
		         << " MKF " << tkf2015.master_key_frame << " THEA_KEYFRAME " << sizeof(THEA_KEYFRAME)
		         << " TIME " << (float)(eerie->frames[i].time / 1000.f) << "s -Move " << tkf2015.key_move
		         << " Orient " << tkf2015.key_orient << " Morph " << tkf2015.key_morph;
		
		// Is There a Global translation ?
		if(tkf2015.key_move != 0) {
			THEA_KEYMOVE * tkm = (THEA_KEYMOVE *)(adr + pos);
			pos += sizeof(THEA_KEYMOVE);
			
			LogDebug << " -> move x " << tkm->x << " y " << tkm->y << " z " << tkm->z
			         << " THEA_KEYMOVE:" << sizeof(THEA_KEYMOVE);
			
			eerie->frames[i].translate.x = tkm->x;
			eerie->frames[i].translate.y = tkm->y;
			eerie->frames[i].translate.z = tkm->z;
		}
		
		// Is There a Global Rotation ?
		if(tkf2015.key_orient != 0) {
			pos += 8; // THEO_ANGLE
			ArxQuat * quat = (ArxQuat *)(adr + pos);
			pos += sizeof(ArxQuat);
			
			LogDebug << " -> rotate x " << quat->x << " y " << quat->y << " z " << quat->z
			         << " w " << quat->w << " ArxQuat:" << sizeof(ArxQuat);
			
			eerie->frames[i].quat.x = quat->x;
			eerie->frames[i].quat.y = quat->y;
			eerie->frames[i].quat.z = quat->z;
			eerie->frames[i].quat.w = quat->w;
		}
		
		// Is There a Global Morph ? (IGNORED!)
		if(tkf2015.key_morph != 0) {
			pos += 16; // THEA_MORPH
		}
		
		// Now go for Group Rotations/Translations/scaling for each GROUP
		for(long j = 0; j < th.nb_groups; j++) {
			THEO_GROUPANIM * tga = (THEO_GROUPANIM *)(adr + pos);
			pos += sizeof(THEO_GROUPANIM);
			
			// LogDebug << " -> group anim " << j << " THEO_GROUPANIM:" << sizeof(THEO_GROUPANIM);
			
			EERIE_GROUP * eg = &eerie->groups[j + i * th.nb_groups];
			eg->key = tga->key_group;
			
			eg->quat.x = tga->Quaternion.x;
			eg->quat.y = tga->Quaternion.y;
			eg->quat.z = tga->Quaternion.z;
			eg->quat.w = tga->Quaternion.w;

			eg->translate.x = tga->translate.x;
			eg->translate.y = tga->translate.y;
			eg->translate.z = tga->translate.z;
			eg->zoom.x = tga->zoom.x;
			eg->zoom.y = tga->zoom.y;
			eg->zoom.z = tga->zoom.z;
		}
		
		// Now Read Sound Data included in this frame
		s32 num_sample;
		memcpy(&num_sample, adr + pos, sizeof(s32));
		pos += sizeof(s32);
		LogDebug << " -> num_sample " << num_sample << " s32:" << sizeof(s32);
		
		eerie->frames[i].sample = -1;
		if(num_sample != -1) {
			THEA_SAMPLE * ts = (THEA_SAMPLE *)(adr + pos);
			pos += sizeof(THEA_SAMPLE);
			pos += ts->sample_size;
			
			LogDebug << " -> sample " << ts->sample_name << " size " << ts->sample_size
			         << " THEA_SAMPLE:" << sizeof(THEA_SAMPLE);
			
			eerie->frames[i].sample = ARX_SOUND_Load(ts->sample_name);
		}
		
		//long num_sfx;
		//memcpy(&num_sfx, adr + pos, sizeof(long));
		pos += sizeof(s32);
	}
	
	for(long i = 0; i < th.nb_key_frames; i++) {
		
		if(!eerie->frames[i].f_translate) {
			
			long k = i;
			while((k >= 0) && (!eerie->frames[k].f_translate)) {
				k--;
			}
			
			long j = i;
			while((j < th.nb_key_frames) && (!eerie->frames[j].f_translate)) {
				j++;
			}
			
			if((j < th.nb_key_frames) && (k >= 0)) {
				float r1 = GetTimeBetweenKeyFrames(eerie, k, i);
				float r2 = GetTimeBetweenKeyFrames(eerie, i, j);
				float tot = 1.f / (r1 + r2);
				r1 *= tot;
				r2 *= tot;
				eerie->frames[i].translate.x = eerie->frames[j].translate.x * r1 + eerie->frames[k].translate.x * r2;
				eerie->frames[i].translate.y = eerie->frames[j].translate.y * r1 + eerie->frames[k].translate.y * r2;
				eerie->frames[i].translate.z = eerie->frames[j].translate.z * r1 + eerie->frames[k].translate.z * r2;
			}
		}
		
		if(!eerie->frames[i].f_rotate) {
			
			long k = i;
			while((k >= 0) && (!eerie->frames[k].f_rotate)) {
				k--;
			}
			
			long j = i;
			while ((j < th.nb_key_frames) && (!eerie->frames[j].f_rotate)) {
				j++;
			}
			
			if ((j < th.nb_key_frames) && (k >= 0)) {
				float r1 = GetTimeBetweenKeyFrames(eerie, k, i);
				float r2 = GetTimeBetweenKeyFrames(eerie, i, j);
				float tot = 1.f / (r1 + r2);
				r1 *= tot;
				r2 *= tot;
				eerie->frames[i].quat.w = eerie->frames[j].quat.w * r1 + eerie->frames[k].quat.w * r2;
				eerie->frames[i].quat.x = eerie->frames[j].quat.x * r1 + eerie->frames[k].quat.x * r2;
				eerie->frames[i].quat.y = eerie->frames[j].quat.y * r1 + eerie->frames[k].quat.y * r2;
				eerie->frames[i].quat.z = eerie->frames[j].quat.z * r1 + eerie->frames[k].quat.z * r2;
			}
		}
	}
	
	for(long i = 0; i < th.nb_key_frames; i++) {
		eerie->frames[i].f_translate = true;
		eerie->frames[i].f_rotate = true;
	}
	
	// Sets Flag for voidgroups (unmodified groups for whole animation)
	for(long i = 0; i < eerie->nb_groups; i++) {
		
		bool voidd = true;
		for(long j = 0; j < eerie->nb_key_frames; j++) {
			long pos = i + (j * eerie->nb_groups);
			
			if((eerie->groups[pos].quat.x != 0.f)
			   || (eerie->groups[pos].quat.y != 0.f)
			   || (eerie->groups[pos].quat.z != 0.f)
			   || (eerie->groups[pos].quat.w != 1.f)
			   || (eerie->groups[pos].translate.x != 0.f)
			   || (eerie->groups[pos].translate.y != 0.f)
			   || (eerie->groups[pos].translate.z != 0.f)
			   || (eerie->groups[pos].zoom.x != 0.f)
			   || (eerie->groups[pos].zoom.y != 0.f)
			   || (eerie->groups[pos].zoom.z != 0.f)
			   ) {
				voidd = false;
				break;
			}
		}
		
		if(voidd) {
			eerie->voidgroups[i] = 1;
		}
	}
	
	eerie->anim_time = (float)th.nb_frames * 1000.f * ( 1.0f / 24 );
	if(eerie->anim_time < 1) {
		eerie->anim_time = 1;
	}
	
	LogDebug << "Finished Conversion TEA -> EERIE - " << (eerie->anim_time / 1000) << " seconds";
	
	return eerie;
}

// Magic conversion constant used for loading rotations in TEO files.
#define THEO_ROTCONVERT 0.087890625f

//-----------------------------------------------------------------------------------------------------
void _THEObjLoad(EERIE_3DOBJ * eerie, unsigned char * adr, long * poss, long version, long flag, long flag2)
{
	
	(void)eerie;
	(void)adr;
	(void)poss;
	(void)version;
	(void)flag;
	(void)flag2;
	
	LogError << "Broken _THEObjLoad";
	/*
	THEO_OFFSETS		*	to;
	THEO_NB					tn;
	THEO_VERTEX 	*		ptv;
	THEO_ACTION_POINT	*	ptap;
	THEO_FACES 	*		ptf;
	THEO_FACES_3006 	*	ptf3006;

	THEO_GROUPS_3011	*	ptg3011;
	THEO_EXTRA_DATA		*	pted;
	THEO_EXTRA_DATA_3005 *	pted3005;

	char groupname[256];
	long pos;
	long i;

	pos = *poss;

	to = (THEO_OFFSETS *)(adr + pos);
	pos += sizeof(THEO_OFFSETS);
	memcpy(&tn, adr + pos, sizeof(THEO_NB));
	pos += sizeof(THEO_NB);

	LogDebug << "Nb Vertex " << tn.nb_vertex << " Nb Action Points " << tn.nb_action_point
	         << " Nb Lines " << tn.nb_lines;
	LogDebug << "Nb Faces " << tn.nb_faces << " Nb Groups " << tn.nb_groups;

	eerie->nbvertex = tn.nb_vertex;
	eerie->nbfaces = tn.nb_faces;
	eerie->nbgroups = tn.nb_groups;
	// TODO eerie->nbaction = tn.nb_action_point;

	eerie->vertexlist = allocStructZero<EERIE_VERTEX>("EEvertexlist", tn.nb_vertex);

	eerie->ndata = NULL;
	eerie->pdata = NULL;
	eerie->cdata = NULL;
	eerie->sdata = NULL;

	eerie->vertexlist3 = allocStructZero<EERIE_VERTEX>("EEvertexlist3", tn.nb_vertex);

	eerie->facelist = allocStructZero<EERIE_FACE>("EEfacelist", tn.nb_faces);

	if (tn.nb_groups == 0) eerie->grouplist = NULL;
	else {
		eerie->grouplist = new EERIE_GROUPLIST[tn.nb_groups]; 
	}

	if (tn.nb_action_point == 0) eerie->actionlist.clear();
	else {
		eerie->actionlist.resize(tn.nb_action_point); 
	}

	// Lecture des VERTEX THEO
	pos = to->vertex_seek;

	if (tn.nb_vertex > 65535) LogError << ("Warning Vertex Number Too High...");

	for (i = 0; i < tn.nb_vertex; i++)
	{
		ptv = (THEO_VERTEX *)(adr + pos);
		pos += sizeof(THEO_VERTEX);
		eerie->vertexlist[i].v.x = ptv->x;
		eerie->vertexlist[i].v.y = ptv->y;
		eerie->vertexlist[i].v.z = ptv->z;
		eerie->cub.xmin = min(eerie->cub.xmin, ptv->x);
		eerie->cub.xmax = max(eerie->cub.xmax, ptv->x);
		eerie->cub.ymin = min(eerie->cub.ymin, ptv->y);
		eerie->cub.ymax = max(eerie->cub.ymax, ptv->y);
		eerie->cub.zmin = min(eerie->cub.zmin, ptv->z);
		eerie->cub.zmax = max(eerie->cub.zmax, ptv->z);
	}

	// Lecture des FACES THEO
	pos = to->faces_seek;
	THEO_FACES_3006				ltf;

	for (i = 0; i < tn.nb_faces; i++)
	{
		if (version >= 3006)
		{
			ptf3006 = (THEO_FACES_3006 *)(adr + pos);
			pos += sizeof(THEO_FACES_3006);
		}
		else
		{
			memset(&ltf, 0, sizeof(THEO_FACES_3006));
			ptf = (THEO_FACES *)(adr + pos);
			ltf.color =		ptf->color;
			ltf.index1 =		ptf->index1;
			ltf.index2 =		ptf->index2;
			ltf.index3 =		ptf->index3;
			ltf.ismap =		ptf->ismap;
			memcpy(&ltf.liste_uv, &ptf->liste_uv, sizeof(THEO_FACE_UV));
			ltf.element_uv =	ptf->element_uv;
			ltf.num_map =	ptf->num_map;
			ltf.tile_x =		ptf->tile_x;
			ltf.tile_y =		ptf->tile_y;
			ltf.user_tile_x = ptf->user_tile_x;
			ltf.user_tile_y = ptf->user_tile_y;
			ltf.flag =		ptf->flag;
			ltf.collision_type = ptf->collision_type;
			ltf.rgb =		ptf->rgb;
			memcpy(&ltf.rgb1, &ptf->rgb1, sizeof(THEO_FACE_RGB));
			memcpy(&ltf.rgb2, &ptf->rgb2, sizeof(THEO_FACE_RGB));
			memcpy(&ltf.rgb3, &ptf->rgb3, sizeof(THEO_FACE_RGB));
			ltf.double_side = ptf->double_side;
			ltf.transparency = ptf->transparency;
			ltf.trans =		ptf->trans;
			pos += sizeof(THEO_FACES);
			ptf3006 = &ltf;
		}

		eerie->facelist[i].vid[0] = (unsigned short)ptf3006->index1;
		eerie->facelist[i].vid[1] = (unsigned short)ptf3006->index2;
		eerie->facelist[i].vid[2] = (unsigned short)ptf3006->index3;

		if (ptf3006->num_map >= eerie->nbmaps)
			ptf3006->num_map = -1;

		if (ptf3006->ismap)
		{
			eerie->facelist[i].texid = (short)ptf3006->num_map;
			eerie->facelist[i].facetype = 1;

			if ((ptf3006->num_map >= 0) && (eerie->texturecontainer[ptf3006->num_map]) && (eerie->texturecontainer[ptf3006->num_map]->userflags & POLY_NOCOL))
				eerie->facelist[i].facetype |= POLY_NOCOL;
		}
		else if (ptf3006->rgb)
		{
			eerie->facelist[i].texid = -1;
			// To Keep
			//		eerie->facelist[i].rgb[0]=EERIELRGB255(ptf3006->rgb1.r,ptf3006->rgb1.g,ptf3006->rgb1.b);
			//		eerie->facelist[i].rgb[1]=EERIELRGB255(ptf3006->rgb2.r,ptf3006->rgb2.g,ptf3006->rgb2.b);
			//		eerie->facelist[i].rgb[2]=EERIELRGB255(ptf3006->rgb3.r,ptf3006->rgb3.g,ptf3006->rgb3.b);
		}
		else
		{
			eerie->facelist[i].texid = -1;
			// To Keep
			//	memcpy(trgb,&ptf3006->color,4);
			//	r=(long)trgb[2];
			//	g=(long)trgb[1];
			//	b=(long)trgb[0];
			//	dc=EERIELRGB255(r,g,b);
			//	eerie->facelist[i].rgb[0]=eerie->facelist[i].rgb[1]=eerie->facelist[i].rgb[2]=dc;
		}

		if (ptf3006->flag != -1)
		{
			if (ptf3006->flag == 0)
				eerie->facelist[i].facetype |= POLY_GLOW;

			if (ptf3006->flag == 1)
				eerie->facelist[i].facetype |= POLY_NO_SHADOW;

			if (ptf3006->flag == 4)
				eerie->facelist[i].facetype |= POLY_METAL;

			if (ptf3006->flag == 10)
				eerie->facelist[i].facetype |= POLY_NOPATH;

			if (ptf3006->flag == 11)
				eerie->facelist[i].facetype |= POLY_CLIMB;

			if (ptf3006->flag == 12)
				eerie->facelist[i].facetype |= POLY_NOCOL;

			if (ptf3006->flag == 13)
				eerie->facelist[i].facetype |= POLY_NODRAW;

			if (ptf3006->flag == 14)
				eerie->facelist[i].facetype |= POLY_PRECISE_PATH;

			if (ptf3006->flag == 15)
				ptf3006->flag = 15;

			if (ptf3006->flag == 16)
				eerie->facelist[i].facetype |= POLY_NO_CLIMB;
		}

		eerie->facelist[i].ou[0] = (short)ptf3006->liste_uv.u1;
		eerie->facelist[i].ov[0] = (short)ptf3006->liste_uv.v1;
		eerie->facelist[i].ou[1] = (short)ptf3006->liste_uv.u2;
		eerie->facelist[i].ov[1] = (short)ptf3006->liste_uv.v2;
		eerie->facelist[i].ou[2] = (short)ptf3006->liste_uv.u3;
		eerie->facelist[i].ov[2] = (short)ptf3006->liste_uv.v3;

		if (ptf3006->double_side) eerie->facelist[i].facetype |= POLY_DOUBLESIDED;

		if (ptf3006->transparency > 0) 
		{
			if (ptf3006->transparency == 2)
			{
				//NORMAL TRANS 0.00001 to 0.999999
				if (ptf3006->trans < 1.f)
				{
					eerie->facelist[i].facetype |= POLY_TRANS;
					eerie->facelist[i].transval = ptf3006->trans;
				}
			}
			else if (ptf3006->transparency == 1)
			{
				if (ptf3006->trans < 0.f)
				{
					//SUBTRACTIVE -0.000001 to -0.999999
					eerie->facelist[i].facetype |= POLY_TRANS;
					eerie->facelist[i].transval = ptf3006->trans;
				}
				else //ADDITIVE 1.000001 to 1.9999999
				{
					eerie->facelist[i].facetype |= POLY_TRANS;
					eerie->facelist[i].transval = ptf3006->trans + 1.f;
				}
			}
			else //MULTIPLICATIVE 2.000001 to 2.9999999
			{
				eerie->facelist[i].facetype |= POLY_TRANS;
				eerie->facelist[i].transval = ptf3006->trans + 2.f;
			}
		}

		if ((eerie->facelist[i].texid != -1) && (eerie->nbmaps > 0) && (eerie->texturecontainer[eerie->facelist[i].texid] != NULL))
		{
			if (eerie->texturecontainer[eerie->facelist[i].texid]->userflags & POLY_TRANS)
			{
				if (!(eerie->facelist[i].facetype & POLY_TRANS))
				{
					eerie->facelist[i].facetype |= POLY_TRANS;
					eerie->facelist[i].transval = ptf3006->trans;
				}
			}

			if (eerie->texturecontainer[eerie->facelist[i].texid]->userflags & POLY_WATER)
				eerie->facelist[i].facetype |= POLY_WATER;

			if (eerie->texturecontainer[eerie->facelist[i].texid]->userflags & POLY_LAVA)
				eerie->facelist[i].facetype |= POLY_LAVA;

			if (eerie->texturecontainer[eerie->facelist[i].texid]->userflags & POLY_FALL)
				eerie->facelist[i].facetype |= POLY_FALL;

			if (eerie->texturecontainer[eerie->facelist[i].texid]->userflags & POLY_CLIMB)
				eerie->facelist[i].facetype |= POLY_CLIMB;
		}
	}

	// Groups Data
	pos = to->groups_seek;
	THEO_GROUPS ltg;
	THEO_GROUPS_3011 ltg3011;

	for (i = 0; i < tn.nb_groups; i++)
	{
		if (version >= 3011)
		{
			ptg3011 = (THEO_GROUPS_3011 *)(adr + pos);
			pos += sizeof(THEO_GROUPS_3011);
		}
		else
		{
			memset(&ltg3011, 0, sizeof(THEO_GROUPS_3011));
			memcpy(&ltg, adr + pos, sizeof(THEO_GROUPS));
			pos += sizeof(THEO_GROUPS);
			ltg3011.origin = ltg.origin;
			ltg3011.nb_index = ltg.nb_index;
			ptg3011 = &ltg3011;
		}

		eerie->grouplist[i].origin = ptg3011->origin;
		//eerie->grouplist[i].nb_index = ptg3011->nb_index;
		
		//eerie->grouplist[i].indexes = allocStructZero<long>("EEGlistIdx", ptg3011->nb_index + 1);

		//memcpy(eerie->grouplist[i].indexes, adr + pos, ptg3011->nb_index * sizeof(long));
		pos += ptg3011->nb_index * sizeof(long);
		memcpy(groupname, adr + pos, 256);
		eerie->grouplist[i].name = groupname;
		eerie->grouplist[i].siz = 0.f;

		for (long o = 0; o < ptg3011->nb_index; o++)
		{
			eerie->grouplist[i].siz = max(eerie->grouplist[i].siz,
			                                EEDistance3D(&eerie->vertexlist[eerie->grouplist[i].origin].v,
			                                        &eerie->vertexlist[eerie->grouplist[i].indexes[o]].v));
		}

		eerie->grouplist[i].siz = EEsqrt(eerie->grouplist[i].siz) * ( 1.0f / 16 );
		pos += 256;
	}

	// SELECTIONS
	long	THEO_nb_selected;
	THEO_SELECTED * pts;
	memcpy(&THEO_nb_selected, adr + pos, sizeof(long));
	pos += sizeof(long);
	eerie->selections.resize(THEO_nb_selected);

	for (i = 0; i < THEO_nb_selected; i++)
	{
		pts = (THEO_SELECTED *)(adr + pos);
		pos += sizeof(THEO_SELECTED);

		if (strlen(pts->name) > 63) pts->name[63] = 0;

		eerie->selections[i].name = pts->name;
		eerie->selections[i].selected.resize(pts->nb_index);

		if(pts->nb_index > 0) {
			long * indices = (long*)(adr + pos);
			std::copy(indices, indices + pts->nb_index, eerie->selections[i].selected.begin());
			pos += sizeof(long) * pts->nb_index;
		}
	}

	// Theo Action Points Read
	pos = to->action_point_seek;

	for (i = 0; i < tn.nb_action_point; i++)
	{
		ptap = (THEO_ACTION_POINT *)(adr + pos);
		pos += sizeof(THEO_ACTION_POINT);
		eerie->actionlist[i].act = ptap->action;
		eerie->actionlist[i].sfx = ptap->num_sfx;
		eerie->actionlist[i].idx = ptap->vert_index;
		eerie->actionlist[i].name = ptap->name;
		MakeUpcase(eerie->actionlist[i].name);
	}

	eerie->angle.g = eerie->angle.b = eerie->angle.a = 0.f;
	eerie->pos.z = eerie->pos.y = eerie->pos.x = 0.f;

	// Now Interpret Extra Data chunk
	pos = to->extras_seek + 4;

	if (version >= 3005)
	{
		pted3005 = (THEO_EXTRA_DATA_3005 *)(adr + pos);
		pos += sizeof(THEO_EXTRA_DATA_3005);
		eerie->pos.x = pted3005->posx;
		eerie->pos.y = pted3005->posy;
		eerie->pos.z = pted3005->posz;

		eerie->angle.a = (float)(pted3005->alpha & 0xfff) * THEO_ROTCONVERT;
		eerie->angle.b = (float)(pted3005->beta & 0xfff) * THEO_ROTCONVERT;
		eerie->angle.g = (float)(pted3005->gamma & 0xfff) * THEO_ROTCONVERT;

		eerie->point0.x = eerie->vertexlist[pted3005->origin_index].v.x;
		eerie->point0.y = eerie->vertexlist[pted3005->origin_index].v.y;
		eerie->point0.z = eerie->vertexlist[pted3005->origin_index].v.z;
		eerie->origin = pted3005->origin_index;

		eerie->quat.x = pted3005->quat.x;
		eerie->quat.y = pted3005->quat.y;
		eerie->quat.z = pted3005->quat.z;
		eerie->quat.w = pted3005->quat.w;

	}
	else
	{
		pted = (THEO_EXTRA_DATA *)adr + pos;
		pos += sizeof(THEO_EXTRA_DATA);
		eerie->pos.x = pted->posx;
		eerie->pos.y = pted->posy;
		eerie->pos.z = pted->posz;

		eerie->angle.a = (float)(pted->alpha & 0xfff) * THEO_ROTCONVERT;
		eerie->angle.b = (float)(pted->beta & 0xfff) * THEO_ROTCONVERT;
		eerie->angle.g = (float)(pted->gamma & 0xfff) * THEO_ROTCONVERT;

		eerie->point0.x = eerie->vertexlist[pted->origin_index].v.x;
		eerie->point0.y = eerie->vertexlist[pted->origin_index].v.y;
		eerie->point0.z = eerie->vertexlist[pted->origin_index].v.z;
		eerie->origin = pted->origin_index;
	}

	memcpy(poss, &pos, sizeof(long));
	memcpy(eerie->vertexlist3, eerie->vertexlist, eerie->nbvertex * sizeof(EERIE_VERTEX));
	ReCreateUVs(eerie);
	EERIE_Object_Precompute_Fast_Access(eerie);*/
}
//-----------------------------------------------------------------------------------------------------
void ReleaseScene(EERIE_3DSCENE * scene)
{
	long i;

	if (scene->texturecontainer != NULL)
	{
		free(scene->texturecontainer);
		scene->texturecontainer = NULL;
	}

	for (i = 0; i < scene->nbobj; i++)
		ReleaseEERIE3DObjFromScene(scene->objs[i]);

	if (scene->objs != NULL)
	{
		free(scene->objs);
		scene->objs = NULL;
	}

	if (scene->texturecontainer != NULL)
	{
		free(scene->texturecontainer);
		scene->texturecontainer = NULL;
	}

	if (scene->light)
	{
		for (i = 0; i < scene->nblight; i++)
		{
			if (scene->light[i] != NULL)
			{
				free(scene->light[i]);
				scene->light[i] = NULL;
			}
		}

		free(scene->light);
		scene->light = NULL;
	}

	free(scene);
	scene = NULL;
}
//-----------------------------------------------------------------------------------------------------
void MakeUserFlag(TextureContainer * tc)
{
	if (tc == NULL) return;

	if (NC_IsIn(tc->m_texName, "NPC_"))
	{
		tc->userflags |= POLY_LATE_MIP;
	}

	if (NC_IsIn(tc->m_texName, "nocol"))
	{
		tc->userflags |= POLY_NOCOL;
	}

	if (NC_IsIn(tc->m_texName, "climb")) // change string depending on GFX guys
	{
		tc->userflags |= POLY_CLIMB;
	}

	if (NC_IsIn(tc->m_texName, "fall"))
	{
		tc->userflags |= POLY_FALL;
	}

	if (NC_IsIn(tc->m_texName, "lava"))
	{
		tc->userflags |= POLY_LAVA;
	}

	if (NC_IsIn(tc->m_texName, "water"))
	{
		tc->userflags |= POLY_WATER;
		tc->userflags |= POLY_TRANS;
	}
	else if (NC_IsIn(tc->m_texName, "spider_web"))
	{
		tc->userflags |= POLY_WATER;
		tc->userflags |= POLY_TRANS;
	}
	else if (NC_IsIn(tc->m_texName, "[metal]"))
	{
		tc->userflags |= POLY_METAL;
	}
}
//-----------------------------------------------------------------------------------------------------
void ReleaseMultiScene(EERIE_MULTI3DSCENE * ms)
{
	if (ms)
	{
		for (long i = 0; i < ms->nb_scenes; i++)
		{
			ReleaseScene(ms->scenes[i]);
			ms->scenes[i] = NULL;
		}
	}

	free(ms);
}
//-----------------------------------------------------------------------------------------------------
EERIE_MULTI3DSCENE * MultiSceneToEerie(const char * dirr)
{
	EERIE_MULTI3DSCENE * es;
	char pathh[512];

	es = allocStructZero<EERIE_MULTI3DSCENE>("EEMultiScn");

	strcpy(LastLoadedScene, dirr);
	sprintf(pathh, "%s*.scn", dirr);

	LogWarning << "partially unimplemented MultiSceneToEerie";
//	todo: finddata
//	long idx;
//	struct _finddata_t fd;
//	if ((idx = _findfirst(pathh, &fd)) != -1)
//	{
//		do
//		{
//			if (!(fd.attrib & _A_SUBDIR))
//			{
//				char * tex = GetExt(fd.name);
//
//				if (!strcasecmp(tex, ".SCN"))
//				{
//					char path[512];
//					sprintf(path, "%s%s", dirr, fd.name);
//					size_t SizeAlloc = 0;
//
//					unsigned char * adr;
//					if (adr = (unsigned char *)PAK_FileLoadMalloc(path, &SizeAlloc))
//					{
//						es->scenes[es->nb_scenes] = (EERIE_3DSCENE *)ScnToEerie(adr, SizeAlloc, path);
//						es->nb_scenes++;
//						free(adr);
//					}
//				}
//			}
//		}
//		while (!(_findnext(idx, &fd)));
//
//		_findclose(idx);
//	}

	es->cub.xmax = -9999999999.f;
	es->cub.xmin = 9999999999.f;
	es->cub.ymax = -9999999999.f;
	es->cub.ymin = 9999999999.f;
	es->cub.zmax = -9999999999.f;
	es->cub.zmin = 9999999999.f;

	for (long i = 0; i < es->nb_scenes; i++)
	{
		es->cub.xmax = max(es->cub.xmax, es->scenes[i]->cub.xmax);
		es->cub.xmin = min(es->cub.xmin, es->scenes[i]->cub.xmin);
		es->cub.ymax = max(es->cub.ymax, es->scenes[i]->cub.ymax);
		es->cub.ymin = min(es->cub.ymin, es->scenes[i]->cub.ymin);
		es->cub.zmax = max(es->cub.zmax, es->scenes[i]->cub.zmax);
		es->cub.zmin = min(es->cub.zmin, es->scenes[i]->cub.zmin);
		es->pos.x = es->scenes[i]->pos.x;
		es->pos.y = es->scenes[i]->pos.y;
		es->pos.z = es->scenes[i]->pos.z;

		if ((es->scenes[i]->point0.x != -999999999999.f) &&
		        (es->scenes[i]->point0.y != -999999999999.f) &&
		        (es->scenes[i]->point0.z != -999999999999.f))
		{
			es->point0.x = es->scenes[i]->point0.x;
			es->point0.y = es->scenes[i]->point0.y;
			es->point0.z = es->scenes[i]->point0.z;
		}
	}

	if (es->nb_scenes == 0)
	{
		free(es);
		return NULL;
	}

	return es;
}
//-----------------------------------------------------------------------------------------------------
EERIE_MULTI3DSCENE * _PAK_MultiSceneToEerie(const char * dirr)
{
	EERIE_MULTI3DSCENE	* es;
	unsigned char 	*	adr;

	es = allocStructZero<EERIE_MULTI3DSCENE>("EEMulti3DScn");

	strcpy(LastLoadedScene, dirr);

	std::string path = dirr;
	RemoveName(path);

	vector<PakDirectory *> *pvRepertoire;
	pvRepertoire = pPakManager->ExistDirectory(path);

	if (!pvRepertoire->size())
	{
		pvRepertoire->clear();
		delete pvRepertoire;
	}

	vector<PakDirectory *>::iterator iT;

	for (iT = pvRepertoire->begin(); iT < pvRepertoire->end(); iT++)
	{
		int nb = (*iT)->nbfiles;
		PakFile * et;
		et = (*iT)->files;

		while (nb--)
		{
			if ( !strcasecmp( GetExt( et->name.c_str() ), ".scn") )
			{
				std::string path2;
				path2 = dirr + et->name;
				size_t SizeAlloc = 0;

				adr = (unsigned char *)PAK_FileLoadMalloc(path2, SizeAlloc);
				if (adr)
				{
					es->scenes[es->nb_scenes] = (EERIE_3DSCENE *)ScnToEerie(adr, SizeAlloc, path.c_str());
					es->nb_scenes++;
					free(adr);
				}
			}

			et = et->next;
		}
	}

	pvRepertoire->clear();
	delete pvRepertoire;


	es->cub.xmax = -9999999999.f;
	es->cub.xmin = 9999999999.f;
	es->cub.ymax = -9999999999.f;
	es->cub.ymin = 9999999999.f;
	es->cub.zmax = -9999999999.f;
	es->cub.zmin = 9999999999.f;

	for (long i = 0; i < es->nb_scenes; i++)
	{
		es->cub.xmax = max(es->cub.xmax, es->scenes[i]->cub.xmax);
		es->cub.xmin = min(es->cub.xmin, es->scenes[i]->cub.xmin);
		es->cub.ymax = max(es->cub.ymax, es->scenes[i]->cub.ymax);
		es->cub.ymin = min(es->cub.ymin, es->scenes[i]->cub.ymin);
		es->cub.zmax = max(es->cub.zmax, es->scenes[i]->cub.zmax);
		es->cub.zmin = min(es->cub.zmin, es->scenes[i]->cub.zmin);
		es->pos.x = es->scenes[i]->pos.x;
		es->pos.y = es->scenes[i]->pos.y;
		es->pos.z = es->scenes[i]->pos.z;

		if ((es->scenes[i]->point0.x != -999999999999.f) &&
		        (es->scenes[i]->point0.y != -999999999999.f) &&
		        (es->scenes[i]->point0.z != -999999999999.f))
		{
			es->point0.x = es->scenes[i]->point0.x;
			es->point0.y = es->scenes[i]->point0.y;
			es->point0.z = es->scenes[i]->point0.z;
		}
	}

	if (es->nb_scenes == 0)
	{
		free(es);
		return NULL;
	}

	return es;
}
//-----------------------------------------------------------------------------------------------------
EERIE_MULTI3DSCENE * PAK_MultiSceneToEerie(const char * dirr)
{
	LogInfo << "Loading Multiscene " << dirr;
	
	EERIE_MULTI3DSCENE * em = NULL;

// TODO create unified implementation for both pak and non-pak
// TODO is this even used?
	
	em = _PAK_MultiSceneToEerie(dirr);

	if(!em)
		em = MultiSceneToEerie(dirr);


	EERIEPOLY_Compute_PolyIn();
	return em;
}
//-----------------------------------------------------------------------------------------------------
EERIE_3DSCENE * ScnToEerie(unsigned char * adr, size_t size, const std::string& fic) {
	
	(void)size; // TODO use size
	
	LogInfo << "Loading Scene " << fic;
	
	if (adr == NULL) return NULL;

	EERIE_3DSCENE 	*	seerie;
	TSCN_OBJHEADER	*	ptoh;
	TSCN_HEADER		*	psth;
	TSCN_LIGHT			tsl;
	TSCN_LIGHT_3019		tsl3019;
	TSCN_LIGHT_3024		tsl3024;
	THEO_TEXTURE		tt;
	THEO_SAVE_MAPS_IN	tsmi;
	THEO_SAVE_MAPS_IN_3019	tsmi3019;
	THEO_SAVE_MAPS_IN_3019 *	ptsmi3019;
	char mapsname[512];
	const char * temp = "Graph\\Obj3D\\Textures\\";
	long				nbo, nbl;
	char texx[512];
	long i;
	long sizmap;

	long pos = 0;

	seerie = allocStructZero<EERIE_3DSCENE>("Scn2EE");

	Clear3DScene(seerie);

	psth = (TSCN_HEADER *)(adr + pos);
	pos += sizeof(TSCN_HEADER);

	if (DEBUGG)
	{
		sprintf(texx, "SCNtoEERIE %s", fic.c_str());
		SendConsole(texx, 2, 0, (HWND)MSGhwnd);
		sprintf(texx, "---------THEO SCN FILE---------");
		SendConsole(texx, 3, 0, (HWND)MSGhwnd);
		sprintf(texx, "Version %u Nb Textures %d", psth->version, psth->nb_maps);
		SendConsole(texx, 3, 0, (HWND)MSGhwnd);
	}

	if ((psth->version < 3008) || (psth->version > 3024))
	{
		sprintf(texx, "\nINVALID Theo Version !!!\nVersion Found    - %u\nVersion Required - %d to %d\n\nPlease Update File\n%s", psth->version, 3008, 3024, fic.c_str());
		LogError << "ScnToEerie " << texx;
		free(seerie);
		seerie = NULL;
		return NULL;
	}

	seerie->nbtex = psth->nb_maps;

	if (psth->type_write == 0)
	{
		// LECTURE DE TEXTURE THEO IN_OBJECT... NOT QUITE IMPLEMENTED !!!
		if (DEBUGG) SendConsole("SAVE_MAP_IN_OBJECT = true", 3, 0, (HWND)MSGhwnd);

		seerie->texturecontainer = allocStructZero<TextureContainer *>("SEETc", psth->nb_maps); 

		for (i = 0; i < psth->nb_maps; i++)
		{
			memcpy(&tt, adr + pos, sizeof(THEO_TEXTURE));
			pos += sizeof(THEO_TEXTURE);
			sprintf(mapsname, "%s%s.bmp", temp, tt.texture_name);
			seerie->texturecontainer[i] = D3DTextr_CreateTextureFromFile(mapsname, 0, 0, EERIETEXTUREFLAG_LOADSCENE_RELEASE);
			MakeUserFlag(seerie->texturecontainer[i]);
			sizmap = tt.dx * tt.dy * (tt.bpp / 8);
		}
	}
	else
	{
		if (DEBUGG) SendConsole("SAVE_MAP_IN_OBJECT = false", 3, 0, (HWND)MSGhwnd);

		if ((psth->type_write & SAVE_MAP_BMP) || (psth->type_write & SAVE_MAP_TGA))
		{
			if (DEBUGG) SendConsole("SAVE_MAP_BMP or TGA = true", 3, 0, (HWND)MSGhwnd);

			seerie->texturecontainer = allocStructZero<TextureContainer *>("SEETc", psth->nb_maps);

			for (i = 0; i < psth->nb_maps; i++)
			{
				if (psth->version >= 3019)
				{
					ptsmi3019 = (THEO_SAVE_MAPS_IN_3019 *)(adr + pos);
					pos += sizeof(THEO_SAVE_MAPS_IN_3019);
				}
				else
				{
					memcpy(&tsmi, adr + pos, sizeof(THEO_SAVE_MAPS_IN));
					pos += sizeof(THEO_SAVE_MAPS_IN);
					tsmi3019.animated_map = tsmi.animated_map;
					tsmi3019.color_mask = 0;
					tsmi3019.map_type = tsmi.map_type;
					tsmi3019.mipmap_level = tsmi.mipmap_level;
					tsmi3019.reflect_map = tsmi.reflect_map;
					strcpy(tsmi3019.texture_name, tsmi.texture_name);
					tsmi3019.water_intensity = tsmi.water_intensity;
					ptsmi3019 = &tsmi3019;
				}

				if (ptsmi3019->texture_name[0] != 0)
				{

					if (psth->type_write & SAVE_MAP_BMP) sprintf(mapsname, "%s%s.bmp", temp, ptsmi3019->texture_name);
					else sprintf(mapsname, "%s%s.tga", temp, ptsmi3019->texture_name);

					seerie->texturecontainer[i] = D3DTextr_CreateTextureFromFile(mapsname, 0, 0, EERIETEXTUREFLAG_LOADSCENE_RELEASE);
					MakeUserFlag(seerie->texturecontainer[i]);
				}
			}
		}
	}

	// Objects read /////////////////////////////////////////////////
	pos = psth->object_seek;
	memcpy(&nbo, adr + pos, sizeof(s32));
	pos += sizeof(s32);

	if (DEBUGG)
	{
		sprintf(texx, "Nb objects %ld", nbo);
		SendConsole(texx, 3, 0, (HWND)MSGhwnd);
	}

	seerie->nbobj = nbo;
	seerie->objs = allocStructZero<EERIE_3DOBJ *>("SceneObjectList", nbo); 

	seerie->point0.x = -999999999999.f;
	seerie->point0.y = -999999999999.f;
	seerie->point0.z = -999999999999.f;

	long id = 0;

	for (i = 0; i < nbo; i++)
	{
		ptoh = (TSCN_OBJHEADER *)(adr + pos);
		pos += sizeof(TSCN_OBJHEADER);

		seerie->objs[id] = new EERIE_3DOBJ(); 

		// TODO most is done in the constructor already
		if(seerie->objs[id]) seerie->objs[id]->clear();

		seerie->objs[id]->texturecontainer.resize(seerie->nbtex);
		std::copy(seerie->texturecontainer, seerie->texturecontainer + seerie->nbtex, seerie->objs[id]->texturecontainer.begin());

		if	(psth->version < 3013)	_THEObjLoad(seerie->objs[id], adr, &pos, 3004, TTE_NO_NDATA | TTE_NO_PDATA);
		else if (psth->version < 3015)	_THEObjLoad(seerie->objs[id], adr, &pos, 3005, TTE_NO_NDATA | TTE_NO_PDATA);
		else if (psth->version < 3019)	_THEObjLoad(seerie->objs[id], adr, &pos, 3006, TTE_NO_NDATA | TTE_NO_PDATA);
		else if (psth->version < 3023)	_THEObjLoad(seerie->objs[id], adr, &pos, 3008, TTE_NO_NDATA | TTE_NO_PDATA);
		else							_THEObjLoad(seerie->objs[id], adr, &pos, 3011, TTE_NO_NDATA | TTE_NO_PDATA);

		seerie->cub.xmin = min(seerie->cub.xmin, seerie->objs[id]->cub.xmin + seerie->objs[id]->pos.x);
		seerie->cub.xmax = max(seerie->cub.xmax, seerie->objs[id]->cub.xmax + seerie->objs[id]->pos.x);
		seerie->cub.ymin = min(seerie->cub.ymin, seerie->objs[id]->cub.ymin + seerie->objs[id]->pos.y);
		seerie->cub.ymax = max(seerie->cub.ymax, seerie->objs[id]->cub.ymax + seerie->objs[id]->pos.y);
		seerie->cub.zmin = min(seerie->cub.zmin, seerie->objs[id]->cub.zmin + seerie->objs[id]->pos.z);
		seerie->cub.zmax = max(seerie->cub.zmax, seerie->objs[id]->cub.zmax + seerie->objs[id]->pos.z);

		if (!strcmp(ptoh->object_name, "map_origin"))
		{
			seerie->point0.x = seerie->objs[id]->point0.x + seerie->objs[id]->pos.x;
			seerie->point0.y = seerie->objs[id]->point0.y + seerie->objs[id]->pos.y;
			seerie->point0.z = seerie->objs[id]->point0.z + seerie->objs[id]->pos.z;
			ReleaseEERIE3DObjFromScene(seerie->objs[id]);
			seerie->nbobj--;
			id--;
		}
		else seerie->objs[id]->name = ptoh->object_name;

		id++;

		if (i < nbo - 1) pos = ptoh->next_obj;
	}

	pos = psth->light_seek; // ambient
	memcpy(&seerie->ambient_r, adr + pos, sizeof(float));
	pos += sizeof(float);
	memcpy(&seerie->ambient_g, adr + pos, sizeof(float));
	pos += sizeof(float);
	memcpy(&seerie->ambient_b, adr + pos, sizeof(float));
	pos += sizeof(float);

	memcpy(&nbl, adr + pos, sizeof(long));
	pos += sizeof(long);

	if (DEBUGG)
	{
		sprintf(texx, "Nb Lights %ld", nbl);
		SendConsole(texx, 3, 0, (HWND)MSGhwnd);
	}

	seerie->light = allocStructZero<EERIE_LIGHT *>("SceneLights", nbl); 

	seerie->nblight = nbl;

	for (i = 0; i < nbl; i++) //lights
	{
		if (psth->version >= 3024)
		{
			memcpy(&tsl3024, adr + pos, sizeof(TSCN_LIGHT_3024));
			pos += sizeof(TSCN_LIGHT_3024);
		}
		else if (psth->version >= 3019)
		{
			memcpy(&tsl3019, adr + pos, sizeof(TSCN_LIGHT_3019));
			pos += sizeof(TSCN_LIGHT_3019);
			memset(&tsl3024, 0, sizeof(TSCN_LIGHT_3024));
			tsl3024.red = tsl3019.red;
			tsl3024.green = tsl3019.green;
			tsl3024.blue = tsl3019.blue;
			tsl3024.pos.x = tsl3019.pos.x;
			tsl3024.pos.y = tsl3019.pos.y;
			tsl3024.pos.z = tsl3019.pos.z;
			tsl3024.hotspot = tsl3019.hotspot;
			tsl3024.falloff = tsl3019.falloff;
			tsl3024.intensity = tsl3019.intensity;
		}
		else
		{
			memcpy(&tsl, adr + pos, sizeof(TSCN_LIGHT));
			pos += sizeof(TSCN_LIGHT);
			memset(&tsl3024, 0, sizeof(TSCN_LIGHT_3024));
			tsl3024.red = tsl.red;
			tsl3024.green = tsl.green;
			tsl3024.blue = tsl.blue;
			tsl3024.pos.x = tsl.pos.x;
			tsl3024.pos.y = tsl.pos.y;
			tsl3024.pos.z = tsl.pos.z;
			tsl3024.hotspot = tsl.hotspot;
			tsl3024.falloff = tsl.falloff;
			tsl3024.intensity = tsl.intensity;
		}

		seerie->light[i] = allocStructZero<EERIE_LIGHT>("SceneLightInfo"); 

		seerie->light[i]->rgb.r = (float)tsl3024.red * ( 1.0f / 255 );
		seerie->light[i]->rgb.g = (float)tsl3024.green * ( 1.0f / 255 );
		seerie->light[i]->rgb.b = (float)tsl3024.blue * ( 1.0f / 255 );
		seerie->light[i]->pos.x = (float)tsl3024.pos.x;
		seerie->light[i]->pos.y = (float)tsl3024.pos.y;
		seerie->light[i]->pos.z = (float)tsl3024.pos.z;
		seerie->light[i]->fallstart = (float)tsl3024.hotspot;
		seerie->light[i]->fallend = (float)tsl3024.falloff;

		float t = seerie->light[i]->fallend - seerie->light[i]->fallstart;

		if (t < 150.f)
		{
			seerie->light[i]->fallend += 150.f - t;
		}

		seerie->light[i]->intensity = (float)tsl3024.intensity;
		seerie->light[i]->exist = 1;
		seerie->light[i]->treat = 1;
		seerie->light[i]->selected = 0;
		seerie->light[i]->type = 0;
		EERIE_LIGHT_GlobalAdd(seerie->light[i]);
		free(seerie->light[i]);
		seerie->light[i] = NULL;
	}

	if (seerie->light) free(seerie->light);

	seerie->light = NULL;

	if (DEBUGG)
	{
		sprintf(texx, "Version %d Nb Textures %d", psth->version, psth->nb_maps);
		SendConsole(texx, 3, 0, (HWND)MSGhwnd);
	}

	return seerie;
}
//-----------------------------------------------------------------------------------------------------
// Warning Clear3DObj/Clear3DScene don't release Any pointer Just Clears Structures
void EERIE_3DOBJ::clear() {
		// TODO Make it possible to use a default
		// conststructor for these
		pos.x = pos.y = pos.z = 0;
		point0 = pos;
		angle = pos;

		origin = 0;
		ident = 0;
		nbpfaces = 0;
		nbgroups = 0;
		drawflags = 0;

		vertexlocal = NULL;
		vertexlist.clear();
		vertexlist3.clear();

		facelist.clear();
		pfacelist = NULL;
		grouplist = NULL;
		texturecontainer.clear();

		originaltextures = NULL;
		linked = NULL;

		// TODO Default constructor
		quat.x = quat.y = quat.z = quat.w = 0;
		nblinked = 0;

		pbox = 0;
		pdata = 0;
		ndata = 0;
		cdata = 0;
		sdata = 0;

		fastaccess.view_attach = 0;
		fastaccess.primary_attach = 0;
		fastaccess.left_attach = 0;
		fastaccess.weapon_attach = 0;
		fastaccess.secondary_attach = 0;
		fastaccess.mouth_group = 0;
		fastaccess.jaw_group = 0;
		fastaccess.head_group_origin = 0;
		fastaccess.head_group = 0;
		fastaccess.mouth_group_origin = 0;
		fastaccess.V_right = 0;
		fastaccess.U_right = 0;
		fastaccess.fire = 0;
		fastaccess.sel_head = 0;
		fastaccess.sel_chest = 0;
		fastaccess.sel_leggings = 0;
		fastaccess.carry_attach = 0;
		fastaccess.__padd = 0;

		c_data = 0;
		
	cub.xmin = cub.ymin = cub.zmin = EEdef_MAXfloat;
	cub.xmax = cub.ymax = cub.zmax = EEdef_MINfloat;
}
//-----------------------------------------------------------------------------------------------------
void Clear3DScene(EERIE_3DSCENE * eerie)
{
	if (eerie == NULL) return;

	memset(eerie, 0, sizeof(EERIE_3DSCENE));
	eerie->cub.xmin = eerie->cub.ymin = eerie->cub.zmin = EEdef_MAXfloat;
	eerie->cub.xmax = eerie->cub.ymax = eerie->cub.zmax = EEdef_MINfloat;
}
//-----------------------------------------------------------------------------------------------------
void ReleaseEERIE3DObjFromScene(EERIE_3DOBJ * eerie)
{
	if (!eerie) return;

	if (eerie->ndata != NULL)
	{
		KillNeighbours(eerie);
	}

	if (eerie->originaltextures != NULL) free(eerie->originaltextures);

	eerie->originaltextures = NULL;

	if (eerie->pdata != NULL)
	{
		KillProgressiveData(eerie);
	}

	if (eerie->cdata != NULL)
	{
		KillClothesData(eerie);
	}

	EERIEOBJECT_DeletePFaces(eerie);
	EERIE_RemoveCedricData(eerie);
	EERIE_PHYSICS_BOX_Release(eerie);
	EERIE_COLLISION_SPHERES_Release(eerie);

	if (eerie->grouplist != NULL)
	{
		delete[] eerie->grouplist;
	}

	eerie->grouplist = NULL;

	if ((eerie->nblinked) &&
	        (eerie->linked))
	{
		free((void *)eerie->linked);
	}

	delete eerie;
}
//-----------------------------------------------------------------------------------------------------
void ReleaseEERIE3DObj(EERIE_3DOBJ * eerie)
{
	if (!eerie) return;

	if (eerie->originaltextures != NULL)
	{
		free(eerie->originaltextures);
		eerie->originaltextures = NULL;
	}

	if (eerie->ndata != NULL)
	{
		KillNeighbours(eerie);
	}

	if (eerie->pdata != NULL)
	{
		KillProgressiveData(eerie);
	}

	if (eerie->cdata != NULL)
	{
		KillClothesData(eerie);
	}

	EERIEOBJECT_DeletePFaces(eerie);
	EERIE_RemoveCedricData(eerie);
	EERIE_PHYSICS_BOX_Release(eerie);
	EERIE_COLLISION_SPHERES_Release(eerie);

	if (eerie->grouplist != NULL)
	{
		delete[] eerie->grouplist;
	}

	eerie->grouplist = NULL;

	if ((eerie->nblinked) &&
	        (eerie->linked))
	{
		free((void *)eerie->linked);
	}

	delete eerie;
}

void ReCreateUVs(EERIE_3DOBJ * eerie) {
	
	if(eerie->texturecontainer.empty()) return;

	float sxx, syy;
	float sxmod, symod;

	for (size_t i = 0; i < eerie->facelist.size(); i++)
	{
		if (eerie->facelist[i].texid == -1) continue;

		if (eerie->texturecontainer[eerie->facelist[i].texid])
		{
			sxx = eerie->texturecontainer[eerie->facelist[i].texid]->m_odx;
			syy = eerie->texturecontainer[eerie->facelist[i].texid]->m_ody;
			sxmod = eerie->texturecontainer[eerie->facelist[i].texid]->m_hdx;
			symod = eerie->texturecontainer[eerie->facelist[i].texid]->m_hdy;
		}
		else
		{
			sxx = ( 1.0f / 256 );
			syy = ( 1.0f / 256 );
			sxmod = 0.5f * ( 1.0f / 256 );
			symod = 0.5f * ( 1.0f / 256 );
		}

		eerie->facelist[i].u[0] = (float)eerie->facelist[i].ou[0] * sxx; 
		eerie->facelist[i].u[1] = (float)eerie->facelist[i].ou[1] * sxx; 
		eerie->facelist[i].u[2] = (float)eerie->facelist[i].ou[2] * sxx; 
		eerie->facelist[i].v[0] = (float)eerie->facelist[i].ov[0] * syy; 
		eerie->facelist[i].v[1] = (float)eerie->facelist[i].ov[1] * syy; 
		eerie->facelist[i].v[2] = (float)eerie->facelist[i].ov[2] * syy; 
	}
}

//-----------------------------------------------------------------------------------------------------
EERIE_3DOBJ * Eerie_Copy(EERIE_3DOBJ * obj)
{
	EERIE_3DOBJ * nouvo = new EERIE_3DOBJ(); 

	nouvo->vertexlist = obj->vertexlist;

	nouvo->vertexlist3 = obj->vertexlist3;

	nouvo->linked = NULL;
	nouvo->ndata = NULL;
	nouvo->pbox = NULL;
	nouvo->pdata = NULL;
	nouvo->cdata = NULL;
	nouvo->sdata = NULL;
	nouvo->c_data = NULL;
	nouvo->vertexlocal = NULL;

	Vector_Copy(&nouvo->angle, &obj->angle);
	Vector_Copy(&nouvo->pos, &obj->pos);
	nouvo->cub.xmax = obj->cub.xmax;
	nouvo->cub.xmin = obj->cub.xmin;
	nouvo->cub.ymax = obj->cub.ymax;
	nouvo->cub.ymin = obj->cub.ymin;
	nouvo->cub.zmax = obj->cub.zmax;
	nouvo->cub.zmin = obj->cub.zmin;
	nouvo->drawflags = obj->drawflags;

	if ( !obj->file.empty() )
		nouvo->file = obj->file;

	nouvo->ident = obj->ident;

	if ( !obj->name.empty() )
		nouvo->name = obj->name;

	nouvo->origin = obj->origin;
	Vector_Copy(&nouvo->point0, &obj->point0);
	Quat_Copy(&nouvo->quat, &obj->quat);


	if (obj->ndata)
	{
		nouvo->ndata = copyStruct(obj->ndata, "EECopyNdata", obj->vertexlist.size());
	}
	else nouvo->ndata = NULL;

	nouvo->facelist = obj->facelist;

	if (obj->nbgroups) {
		nouvo->nbgroups = obj->nbgroups;
		nouvo->grouplist = new EERIE_GROUPLIST[obj->nbgroups];
		std::copy(obj->grouplist, obj->grouplist + obj->nbgroups, nouvo->grouplist);
	}

	nouvo->actionlist = obj->actionlist;

	nouvo->selections = obj->selections;

	nouvo->texturecontainer = obj->texturecontainer;
	
	memcpy(&nouvo->fastaccess, &obj->fastaccess, sizeof(EERIE_FASTACCESS));
	EERIE_CreateCedricData(nouvo);
	EERIEOBJECT_CreatePFaces(nouvo);

	if (obj->pbox)
	{
		nouvo->pbox = allocStructZero<PHYSICS_BOX_DATA>("Physics Data");
		nouvo->pbox->nb_physvert = obj->pbox->nb_physvert;
		nouvo->pbox->stopcount = 0;
		nouvo->pbox->radius = obj->pbox->radius;
		
		nouvo->pbox->vert = copyStruct(obj->pbox->vert, "PhysVerts", obj->pbox->nb_physvert);
	}

	nouvo->linked = NULL;
	nouvo->nblinked = 0;
	nouvo->originaltextures = NULL;
	return nouvo;
}
//-----------------------------------------------------------------------------------------------------
long EERIE_OBJECT_GetSelection(const EERIE_3DOBJ * obj, const char * selname)
{
	if (!obj) return -1;

	for (size_t i = 0; i < obj->selections.size(); i++)
	{ // TODO iterator
		if (!strcasecmp(obj->selections[i].name.c_str(), selname)) return i;
	}

	return -1;
}
//-----------------------------------------------------------------------------------------------------
long EERIE_OBJECT_GetGroup(EERIE_3DOBJ * obj, const char * groupname)
{
	if (!obj) return -1;

	for (long i = 0; i < obj->nbgroups; i++)
	{
		if (!strcasecmp(obj->grouplist[i].name.c_str(), groupname)) return i;
	}

	return -1;
}
//-----------------------------------------------------------------------------------------------------
void AddIdxToBone(EERIE_BONE * bone, long idx)
{
	bone->idxvertices = (long *)realloc(bone->idxvertices, sizeof(long) * (bone->nb_idxvertices + 1));

	if (bone->idxvertices)
	{
		bone->idxvertices[bone->nb_idxvertices] = idx;
		bone->nb_idxvertices++;
	}
}
//-----------------------------------------------------------------------------------------------------
long GetFather(EERIE_3DOBJ * eobj, long origin, long startgroup)
{
	for (long i = startgroup; i >= 0; i--)
	{
		for (size_t j = 0; j < eobj->grouplist[i].indexes.size(); j++)
		{
			if (eobj->grouplist[i].indexes[j] == origin)
			{
				return i;
			}
		}
	}

	return -1;
}
//-----------------------------------------------------------------------------------------------------
void EERIE_RemoveCedricData(EERIE_3DOBJ * eobj)
{
	if (!eobj) return;

	if (!eobj->c_data) return;

	for (long i = 0; i < eobj->c_data->nb_bones; i++)
	{
		if (eobj->c_data->bones[i].idxvertices)
			free(eobj->c_data->bones[i].idxvertices);

		eobj->c_data->bones[i].idxvertices = NULL;
	}

	if (eobj->c_data->bones) delete[] eobj->c_data->bones;

	eobj->c_data->bones = NULL;
	delete eobj->c_data;
	eobj->c_data = NULL;

	if (eobj->vertexlocal) delete[] eobj->vertexlocal;

	eobj->vertexlocal = NULL;
}
//-----------------------------------------------------------------------------------------------------
void EERIE_CreateCedricData(EERIE_3DOBJ * eobj)
{
	eobj->c_data = new EERIE_C_DATA();
	memset(eobj->c_data, 0, sizeof(EERIE_C_DATA));

	if (eobj->nbgroups <= 0) // If no groups were specified
	{
		// Make one bone
		eobj->c_data->nb_bones = 1;
		eobj->c_data->bones = new EERIE_BONE[eobj->c_data->nb_bones];
		memset(eobj->c_data->bones, 0, sizeof(EERIE_BONE)*eobj->c_data->nb_bones);

		// Add all vertices to the bone
		for (size_t i = 0; i < eobj->vertexlist.size(); i++)
			AddIdxToBone(&eobj->c_data->bones[0], i);

		// Initialize the bone
		Quat_Init(&eobj->c_data->bones[0].quatinit);
		Quat_Init(&eobj->c_data->bones[0].quatanim);
		Vector_Init(&eobj->c_data->bones[0].scaleinit);
		Vector_Init(&eobj->c_data->bones[0].scaleanim);
		Vector_Init(&eobj->c_data->bones[0].transinit);
		Vector_Copy(&eobj->c_data->bones[0].transinit_global, &eobj->c_data->bones[0].transinit);
		eobj->c_data->bones[0].original_group = NULL;
		eobj->c_data->bones[0].father = -1;
	}
	else // Groups were specified
	{
		// Alloc the bones
		eobj->c_data->nb_bones = eobj->nbgroups;
		eobj->c_data->bones = new EERIE_BONE[eobj->c_data->nb_bones];
		// TODO memset -> use constructor instead
		memset(eobj->c_data->bones, 0, sizeof(EERIE_BONE)*eobj->c_data->nb_bones);

		bool * temp = new bool[eobj->vertexlist.size()];
		memset(temp, 0, eobj->vertexlist.size());

		for (long i = eobj->nbgroups - 1; i >= 0; i--)
		{
			EERIE_VERTEX * v_origin = &eobj->vertexlist[eobj->grouplist[i].origin];

			for (size_t j = 0; j < eobj->grouplist[i].indexes.size(); j++)
			{
				if (!temp[eobj->grouplist[i].indexes[j]])
				{
					temp[eobj->grouplist[i].indexes[j]] = true;
					AddIdxToBone(&eobj->c_data->bones[i], eobj->grouplist[i].indexes[j]);
				}
			}

			Quat_Init(&eobj->c_data->bones[i].quatinit);
			Quat_Init(&eobj->c_data->bones[i].quatanim);
			Vector_Init(&eobj->c_data->bones[i].scaleinit);
			Vector_Init(&eobj->c_data->bones[i].scaleanim);
			Vector_Init(&eobj->c_data->bones[i].transinit, v_origin->v.x, v_origin->v.y, v_origin->v.z);
			Vector_Copy(&eobj->c_data->bones[i].transinit_global, &eobj->c_data->bones[i].transinit);
			eobj->c_data->bones[i].original_group = &eobj->grouplist[i];
			eobj->c_data->bones[i].father = GetFather(eobj, eobj->grouplist[i].origin, i - 1);
		}

		delete[] temp;

		// Try to correct lonely vertex
		for (size_t i = 0; i < eobj->vertexlist.size(); i++)
		{
			long ok = 0;

			for (long j = 0; j < eobj->nbgroups; j++)
			{
				for (size_t k = 0; k < eobj->grouplist[j].indexes.size(); k++)
				{
					if ((size_t)eobj->grouplist[j].indexes[k] == i)
					{
						ok = 1;
						break;
					}
				}

				if (ok)
					break;
			}

			if (!ok)
			{
				AddIdxToBone(&eobj->c_data->bones[0], i);
			}
		}

		for (long i = eobj->nbgroups - 1; i >= 0; i--)
		{
			if (eobj->c_data->bones[i].father >= 0)
			{
				eobj->c_data->bones[i].transinit.x -= eobj->c_data->bones[eobj->c_data->bones[i].father].transinit.x;
				eobj->c_data->bones[i].transinit.y -= eobj->c_data->bones[eobj->c_data->bones[i].father].transinit.y;
				eobj->c_data->bones[i].transinit.z -= eobj->c_data->bones[eobj->c_data->bones[i].father].transinit.z;
			}

			Vector_Copy(&eobj->c_data->bones[i].transinit_global, &eobj->c_data->bones[i].transinit);
		}

	}

#if CEDRIC
	/* Build proper mesh */
	{
		EERIE_C_DATA* obj = eobj->c_data;


		for (long i = 0; i != obj->nb_bones; i++)
		{
			EERIE_QUAT	qt1;

			if (obj->bones[i].father >= 0)
			{
				/* Rotation*/
				Quat_Copy(&qt1, &obj->bones[i].quatinit);
				Quat_Multiply(&obj->bones[i].quatanim, &obj->bones[obj->bones[i].father].quatanim, &qt1);
				/* Translation */
				TransformVertexQuat(&obj->bones[obj->bones[i].father].quatanim, &obj->bones[i].transinit, &obj->bones[i].transanim);
				Vector_Add(&obj->bones[i].transanim, &obj->bones[obj->bones[i].father].transanim, &obj->bones[i].transanim);
				/* Scale */
				Vector_Init(&obj->bones[i].scaleanim, 1.0f, 1.0f, 1.0f);
			}
			else
			{
				/* Rotation*/
				Quat_Copy(&obj->bones[i].quatanim, &obj->bones[i].quatinit);
				/* Translation */
				Vector_Copy(&obj->bones[i].transanim, &obj->bones[i].transinit);
				/* Scale */
				Vector_Init(&obj->bones[i].scaleanim, 1.0f, 1.0f, 1.0f);
			}
		}

		eobj->vertexlocal = new EERIE_3DPAD[eobj->vertexlist.size()];
		// TODO constructor is better than memset
		memset(eobj->vertexlocal, 0, sizeof(EERIE_3DPAD)*eobj->vertexlist.size());

		for (long i = 0; i != obj->nb_bones; i++)
		{
			EERIE_3D	vector;
			EERIE_VERTEX * inVert;
			EERIE_3DPAD * outVert;

			Vector_Copy(&vector, &obj->bones[i].transanim);

			for (int v = 0; v != obj->bones[i].nb_idxvertices; v++)
			{
				inVert  = &eobj->vertexlist[obj->bones[i].idxvertices[v]];
				outVert = &eobj->vertexlocal[obj->bones[i].idxvertices[v]];

				Vector_Sub((EERIE_3D *)outVert, &inVert->v, &vector);
				TransformInverseVertexQuat(&obj->bones[i].quatanim, (EERIE_3D *)outVert, (EERIE_3D *)outVert);
			}
		}
	}
#endif
}

void EERIEOBJECT_DeletePFaces(EERIE_3DOBJ * eobj)
{
	// todo Why does this return? Nonfunctional?
	return;

	if (eobj->pfacelist) free(eobj->pfacelist);

	eobj->pfacelist = NULL;
	eobj->nbpfaces = 0;
}

bool Is_Svert(EERIE_PFACE * epf, long epi, EERIE_FACE * ef, long ei)
{
	if ((epf->vid[epi] == ef->vid[ei])
	        &&	(epf->u[epi] == ef->u[ei])
	        &&	(epf->v[epi] == ef->v[ei])) 
			return true;

	return false;
}

long Strippable(EERIE_PFACE * epf, EERIE_FACE * ef)
{
	if ((Is_Svert(epf, epf->nbvert - 1, ef, 0))
	        &&	(Is_Svert(epf, epf->nbvert - 2, ef, 1))) return 2;

	if ((Is_Svert(epf, epf->nbvert - 1, ef, 1))
	        &&	(Is_Svert(epf, epf->nbvert - 2, ef, 2))) return 0;

	if ((Is_Svert(epf, epf->nbvert - 1, ef, 2))
	        &&	(Is_Svert(epf, epf->nbvert - 2, ef, 0))) return 1;

	return -1;
}

static bool EERIEOBJECT_AddFaceToPFace(EERIE_3DOBJ * eobj, EERIE_FACE * face) {
	
	for (long i = 0; i < eobj->nbpfaces; i++)
	{
		// TODO pfacelist is never really used
		EERIE_PFACE * epf = &eobj->pfacelist[i];

		if (epf->nbvert >= MAX_PFACE) continue;

		if (epf->facetype != face->facetype) continue;

		if (face->facetype & POLY_TRANS) continue;

		long r;

		if ((r = Strippable(epf, face)) >= 0)
		{
			epf->color[epf->nbvert] = face->color[r];
			epf->u[epf->nbvert] = face->u[r];
			epf->v[epf->nbvert] = face->v[r];
			epf->vid[epf->nbvert] = face->vid[r];
			epf->nbvert++;
			return true;
		}
	}

	return false;
}

void EERIEOBJECT_AddFace(EERIE_3DOBJ * eobj, EERIE_FACE * face, long faceidx)
{
	if (EERIEOBJECT_AddFaceToPFace(eobj, face)) return;

	eobj->pfacelist = (EERIE_PFACE *)realloc(eobj->pfacelist, sizeof(EERIE_PFACE) * (eobj->nbpfaces + 1));
	EERIE_PFACE * epf = &eobj->pfacelist[eobj->nbpfaces];
	epf->facetype = face->facetype;
	epf->nbvert = 3;
	epf->texid = face->texid;
	epf->transval = face->transval;

	ARX_CHECK_SHORT(faceidx);
	short sfaceIdx = static_cast<short>(faceidx);

	for (long i = 0; i < 3; i++)
	{
		epf->faceidx[i] = sfaceIdx;
		epf->color[i] = face->color[i];
		epf->u[i] = face->u[i];
		epf->v[i] = face->v[i];
		epf->vid[i] = face->vid[i];
	}

	epf->faceidx[0] = 0;
	eobj->nbpfaces++;
}

void EERIEOBJECT_CreatePFaces(EERIE_3DOBJ * eobj)
{
	// todo Find out why this function doesn't work
	return;
	EERIEOBJECT_DeletePFaces(eobj);

	for (size_t i = 0; i < eobj->facelist.size(); i++)
		EERIEOBJECT_AddFace(eobj, &eobj->facelist[i], i);
}

// Converts a Theo Object to an EERIE object
EERIE_3DOBJ * TheoToEerie(unsigned char * adr, long size, const char * texpath, const char * fic, long flag, LPDIRECT3DDEVICE7 pd3dDevice, long flag2) // flag 1 progressive alloc 2 SLOW
{
	
	LogWarning << "TheoToEerie " << fic;
	
	if (adr == NULL) 	return NULL;

	THEO_HEADER		*		pth;
	THEO_TEXTURE			tt;
	THEO_SAVE_MAPS_IN		tsmi;
	THEO_SAVE_MAPS_IN_3019  tsmi3019;
	THEO_SAVE_MAPS_IN_3019  * ptsmi3019;
	EERIE_3DOBJ 	*		eerie;
	char mapsname[512];
	char texx[512];
	const char * txpath;
	long pos2;
	long pos = 0;

	if ((texpath == NULL) || (texpath[0] == 0))
	{
		txpath = "Graph\\Obj3D\\Textures\\";
	}
	else txpath = texpath;

	if (size < 10) return NULL;

	pth = (THEO_HEADER *)(adr + pos);
	pos += sizeof(THEO_HEADER);

	if (DEBUGG)
	{
		sprintf(texx, "THEOtoEERIE %s", fic);
		SendConsole(texx, 2, 0, (HWND)MSGhwnd);
		sprintf(texx, "-----------THEO FILE-----------");
		SendConsole(texx, 3, 0, (HWND)MSGhwnd);
		sprintf(texx, "----------THEO header---------- size %ld", sizeof(THEO_HEADER));
		SendConsole(texx, 3, 0, (HWND)MSGhwnd);
		sprintf(texx, "Identity----------------------- %s", pth->identity);
		SendConsole(texx, 3, 0, (HWND)MSGhwnd);
		sprintf(texx, "Version------------------------ %u", pth->version);
		SendConsole(texx, 3, 0, (HWND)MSGhwnd);
		sprintf(texx, "Maps Seek---------------------- %d", pth->maps_seek);
		SendConsole(texx, 3, 0, (HWND)MSGhwnd);
		sprintf(texx, "Objects Seek------------------- %d", pth->object_seek);
		SendConsole(texx, 3, 0, (HWND)MSGhwnd);
		sprintf(texx, "NB maps------------------------ %d", pth->nb_maps);
		SendConsole(texx, 3, 0, (HWND)MSGhwnd);
		sprintf(texx, "Type_Write--------------------- %u", pth->type_write);
		SendConsole(texx, 3, 0, (HWND)MSGhwnd);
	}

	if ((pth->version < 3003) || (pth->version > 3011))
	{
		if (!(flag & TTE_NOPOPUP))
		{
			sprintf(texx, "\nINVALID Theo Version !!!\nVersion Found    - %u\nVersion Required - %d to %d\n\nPlease Update File\n%s", pth->version, 3004, 3011, fic);
			LogError << "TheoToEerie " << texx;
		}

		eerie = NULL;
		return NULL;
	}

	eerie = new EERIE_3DOBJ;
	eerie->clear();
	
	eerie->file = fic;

	if (pth->type_write == 0)
	{
		// LECTURE DE TEXTURE THEO IN_OBJECT
		char text[512];
		sprintf(text, "WARNING object %s SAVE MAP IN OBJECT = INVALID... Using Dummy Textures...", fic);
		LogError << (text);


		if (DEBUGG) SendConsole("SAVE_MAP_IN_OBJECT = true", 3, 0, (HWND)MSGhwnd);

		eerie->texturecontainer.resize(pth->nb_maps);

		if (pth->nb_maps > 0)
		{
			pos2 = pth->maps_seek;
		}

		for (long i = 0; i < pth->nb_maps; i++)
		{
			memcpy(&tt, adr + pos, sizeof(THEO_TEXTURE));
			pos += sizeof(THEO_TEXTURE);
			eerie->texturecontainer[i] = GetAnyTexture();
		}

	}
	else
	{
		if (DEBUGG) SendConsole("SAVE_MAP_IN_OBJECT = false", 3, 0, (HWND)MSGhwnd);

		if ((pth->type_write & SAVE_MAP_BMP) || (pth->type_write & SAVE_MAP_TGA))
		{
			eerie->texturecontainer.resize(pth->nb_maps);

			if (DEBUGG)	SendConsole("SAVE_MAP_BMP or TGA = true", 3, 0, (HWND)MSGhwnd);

			for (long i = 0; i < pth->nb_maps; i++)
			{
				if (pth->version >= 3008)
				{
					ptsmi3019 = (THEO_SAVE_MAPS_IN_3019 *)(adr + pos);
					pos += sizeof(THEO_SAVE_MAPS_IN_3019);
				}
				else
				{
					memcpy(&tsmi, adr + pos, sizeof(THEO_SAVE_MAPS_IN));
					pos += sizeof(THEO_SAVE_MAPS_IN);
					tsmi3019.animated_map = tsmi.animated_map;
					tsmi3019.color_mask = 0;
					tsmi3019.map_type = tsmi.map_type;
					tsmi3019.mipmap_level = tsmi.mipmap_level;
					tsmi3019.reflect_map = tsmi.reflect_map;
					strcpy(tsmi3019.texture_name, tsmi.texture_name);
					tsmi3019.water_intensity = tsmi.water_intensity;
					ptsmi3019 = &tsmi3019;
				}

				if (pth->type_write & SAVE_MAP_BMP) sprintf(mapsname, "%s%s.bmp", txpath, ptsmi3019->texture_name);
				else sprintf(mapsname, "%s%s.tga", txpath, ptsmi3019->texture_name);

				eerie->texturecontainer[i] = D3DTextr_CreateTextureFromFile(mapsname, 0, 0, EERIETEXTUREFLAG_LOADSCENE_RELEASE);
				MakeUserFlag(eerie->texturecontainer[i]);

				if (eerie->texturecontainer[i])
				{
					if ((!(flag & TTE_NO_RESTORE)) && (pd3dDevice))
						eerie->texturecontainer[i]->Restore(pd3dDevice);
				}
			}
		}
	}

	pos = pth->object_seek;
	_THEObjLoad(eerie, adr, &pos, pth->version, flag2, flag);
	eerie->angle.a = eerie->angle.b = eerie->angle.g	= 0.f;
	eerie->pos.x = eerie->pos.y = eerie->pos.z			= 0.f;

	//***********************************************************
	// NORMALS CALCULATIONS START
	EERIE_3D nrml;
	EERIE_3D nrrr;
	float count;
	long j, j2;

	//Compute Faces Areas
	for (size_t i = 0; i < eerie->facelist.size(); i++)
	{
		D3DTLVERTEX * ev[3];
		ev[0] = (D3DTLVERTEX *)&eerie->vertexlist[eerie->facelist[i].vid[0]].v;
		ev[1] = (D3DTLVERTEX *)&eerie->vertexlist[eerie->facelist[i].vid[1]].v;
		ev[2] = (D3DTLVERTEX *)&eerie->vertexlist[eerie->facelist[i].vid[2]].v;
		eerie->facelist[i].temp = TRUEDistance3D((ev[0]->sx + ev[1]->sx) * ( 1.0f / 2 ),
		                          (ev[0]->sy + ev[1]->sy) * ( 1.0f / 2 ),
		                          (ev[0]->sz + ev[1]->sz) * ( 1.0f / 2 ),
		                          ev[2]->sx, ev[2]->sy, ev[2]->sz)
		                          * TRUEDistance3D(ev[0]->sx, ev[0]->sy, ev[0]->sz,
		                                  ev[1]->sx, ev[1]->sy, ev[1]->sz) * ( 1.0f / 2 );
	}

	for (size_t i = 0; i < eerie->facelist.size(); i++)
	{
		CalcObjFaceNormal(
		    &eerie->vertexlist[eerie->facelist[i].vid[0]].v,
		    &eerie->vertexlist[eerie->facelist[i].vid[1]].v,
		    &eerie->vertexlist[eerie->facelist[i].vid[2]].v,
		    &eerie->facelist[i]
		);
		D3DTLVERTEX * ev[3];
		ev[0] = (D3DTLVERTEX *)&eerie->vertexlist[eerie->facelist[i].vid[0]].v;
		ev[1] = (D3DTLVERTEX *)&eerie->vertexlist[eerie->facelist[i].vid[1]].v;
		ev[2] = (D3DTLVERTEX *)&eerie->vertexlist[eerie->facelist[i].vid[2]].v;
		float area = eerie->facelist[i].temp;

		for (j = 0; j < 3; j++)
		{
			float mod = area * area;
			nrrr.x = nrml.x = eerie->facelist[i].norm.x * mod;
			nrrr.y = nrml.y = eerie->facelist[i].norm.y * mod;
			nrrr.z = nrml.z = eerie->facelist[i].norm.z * mod;
			count = mod;

			for (size_t i2 = 0; i2 < eerie->facelist.size(); i2++)
			{
				if (i != i2)
				{
					float area2 = eerie->facelist[i].temp;

					for (j2 = 0; j2 < 3; j2++)
					{
						float seuil2 = 0.1f; 
						
						float dist = TRUEDistance3D(eerie->vertexlist[eerie->facelist[i2].vid[j2]].v.x, eerie->vertexlist[eerie->facelist[i2].vid[j2]].v.y, eerie->vertexlist[eerie->facelist[i2].vid[j2]].v.z,
						                            eerie->vertexlist[eerie->facelist[i].vid[j]].v.x, eerie->vertexlist[eerie->facelist[i].vid[j]].v.y, eerie->vertexlist[eerie->facelist[i].vid[j]].v.z); 
						if (dist < seuil2)
						{
							mod = (area2 * area2);
							nrml.x += eerie->facelist[i2].norm.x * mod; 
							nrml.y += eerie->facelist[i2].norm.y * mod; 
							nrml.z += eerie->facelist[i2].norm.z * mod; 
							count += mod; 
						}
					}
				}
			}

			count = 1.f / count;
			eerie->vertexlist[eerie->facelist[i].vid[j]].vert.sx = nrml.x * count;
			eerie->vertexlist[eerie->facelist[i].vid[j]].vert.sy = nrml.y * count;
			eerie->vertexlist[eerie->facelist[i].vid[j]].vert.sz = nrml.z * count;
		}
	}

	for (size_t i = 0; i < eerie->facelist.size(); i++)
	{
		for (j = 0; j < 3; j++)
		{
			eerie->vertexlist[eerie->facelist[i].vid[j]].norm.x = eerie->vertexlist[eerie->facelist[i].vid[j]].vert.sx;
			eerie->vertexlist[eerie->facelist[i].vid[j]].norm.y = eerie->vertexlist[eerie->facelist[i].vid[j]].vert.sy;
			eerie->vertexlist[eerie->facelist[i].vid[j]].norm.z = eerie->vertexlist[eerie->facelist[i].vid[j]].vert.sz;
		}
	}

	// Apply Normals Spherical correction for NPC head
	long neck_orgn = GetGroupOriginByName(eerie, "NECK");
	long head_idx = EERIE_OBJECT_GetGroup(eerie, "head");

	if ((head_idx >= 0) && (neck_orgn >= 0))
	{
		EERIE_3D center;
		EERIE_3D origin;
		Vector_Init(&center);
		Vector_Copy(&origin, &eerie->vertexlist[neck_orgn].v);
		float count = (float)eerie->grouplist[head_idx].indexes.size();

		if (count > 0.f)
		{
			for (size_t idx = 0 ; idx < eerie->grouplist[head_idx].indexes.size() ; idx++)
			{
				center.x += eerie->vertexlist[ eerie->grouplist[head_idx].indexes[idx] ].v.x;
				center.y += eerie->vertexlist[ eerie->grouplist[head_idx].indexes[idx] ].v.y;
				center.z += eerie->vertexlist[ eerie->grouplist[head_idx].indexes[idx] ].v.z;
			}

			float divc = 1.f / count;
			center.x *= divc;
			center.y *= divc;
			center.z *= divc;
			center.x = (center.x + origin.x + origin.x) * ( 1.0f / 3 );
			center.y = (center.y + origin.y + origin.y) * ( 1.0f / 3 );
			center.z = (center.z + origin.z + origin.z) * ( 1.0f / 3 );
			float max_threshold = TRUEEEDistance3D(&origin, &center);

			for (size_t i = 0; i < eerie->grouplist[head_idx].indexes.size(); i++)
			{
				EERIE_VERTEX * ev = &eerie->vertexlist[eerie->grouplist[head_idx].indexes[i]];
				float dist = TRUEEEDistance3D(&ev->v, &origin);
				float factor = 1.f;

				if (dist < max_threshold)
				{
					factor = dist / max_threshold;
				}

				float ifactor = 1.f - factor;
				EERIE_3D fakenorm;
				fakenorm.x = ev->v.x - center.x;
				fakenorm.y = ev->v.y - center.y;
				fakenorm.z = ev->v.z - center.z;
				TRUEVector_Normalize(&fakenorm);
				ev->norm.x = ev->norm.x * ifactor + fakenorm.x * factor;
				ev->norm.y = ev->norm.y * ifactor + fakenorm.y * factor;
				ev->norm.z = ev->norm.z * ifactor + fakenorm.z * factor;
				TRUEVector_Normalize(&ev->norm);
			}
		}
	}

	// NORMALS CALCULATIONS END
	//***********************************************************

	EERIE_LINKEDOBJ_InitData(eerie);
	eerie->c_data = NULL;
	EERIE_CreateCedricData(eerie);
	EERIEOBJECT_CreatePFaces(eerie);
	return(eerie);
}

// TODO why is this in EERIEobject
ACTIONSTRUCT actions[MAX_ACTIONS];
void RemoveAllBackgroundActions()
{
	memset(actions, 0, sizeof(ACTIONSTRUCT)*MAX_ACTIONS);

	for (long i = 0; i < MAX_ACTIONS; i++) actions[i].dl = -1;
}

void EERIE_3DOBJ_RestoreTextures(EERIE_3DOBJ * eobj)
{
	if ((eobj) && !eobj->texturecontainer.empty())
	{
		for (size_t i = 0; i < eobj->texturecontainer.size(); i++)
		{
			if (eobj->texturecontainer[i])
			{
				eobj->texturecontainer[i]->Restore(GDevice);
			}
		}
	}
}

void EERIE_OBJECT_CenterObjectCoordinates(EERIE_3DOBJ * ret)
{
	if (!ret) return;

	EERIE_3D offset;
	Vector_Copy(&offset, &ret->vertexlist[ret->origin].v);

	if ((offset.x == 0) && (offset.y == 0) && (offset.z == 0))
		return;

	LogWarning << "NOT CENTERED " << ret->file;


	for (size_t i = 0; i < ret->vertexlist.size(); i++)
	{
		ret->vertexlist[i].v.x -= offset.x;
		ret->vertexlist[i].v.y -= offset.y;
		ret->vertexlist[i].v.z -= offset.z;
		ret->vertexlist[i].vert.sx -= offset.x;
		ret->vertexlist[i].vert.sy -= offset.y;
		ret->vertexlist[i].vert.sz -= offset.z;

		ret->vertexlist3[i].v.x -= offset.x;
		ret->vertexlist3[i].v.y -= offset.y;
		ret->vertexlist3[i].v.z -= offset.z;
		ret->vertexlist3[i].vert.sx -= offset.x;
		ret->vertexlist3[i].vert.sy -= offset.y;
		ret->vertexlist3[i].vert.sz -= offset.z;

		ret->vertexlist3[i].v.x -= offset.x;
		ret->vertexlist3[i].v.y -= offset.y;
		ret->vertexlist3[i].v.z -= offset.z;
		ret->vertexlist3[i].vert.sx -= offset.x;
		ret->vertexlist3[i].vert.sy -= offset.y;
		ret->vertexlist3[i].vert.sz -= offset.z;
	}

	ret->point0.x -= offset.x;
	ret->point0.y -= offset.y;
	ret->point0.z -= offset.z;
}
