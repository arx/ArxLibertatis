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
// DanaeSaveLoad.CPP
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		DANAE Save & Load Management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////
//TODO(lubosz): too many includes

#include "scene/LoadLevel.h"

#include <cstdio>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <vector>

#include "ai/PathFinderManager.h"
#include "ai/Paths.h"

#include "core/Time.h"
#include "core/Dialog.h"
#include "core/Localization.h"

#include "game/Damage.h"
#include "game/Levels.h"
#include "game/Missile.h"
#include "game/Spells.h"
#include "game/Player.h"

#include "gui/MiniMap.h"
#include "gui/Speech.h"

#include "graphics/Math.h"
#include "graphics/Draw.h"
#include "graphics/effects/Fog.h"
#include "graphics/particle/ParticleEffects.h"

#include "io/IO.h"
#include "io/FilePath.h"
#include "io/PakManager.h"
#include "io/Filesystem.h"
#include "io/Logger.h"
#include "io/Blast.h"
#include "io/Implode.h"

#include "physics/CollisionShapes.h"
#include "physics/Actors.h"

#include "scene/Object.h"
#include "scene/Scene.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"

using std::max;

extern float PROGRESS_BAR_COUNT;
extern float PROGRESS_BAR_TOTAL;
extern long DONT_ERASE_PLAYER;
extern EERIE_BACKGROUND bkrgnd;

extern QUAKE_FX_STRUCT QuakeFx;
extern bool bGToggleCombatModeWithKey;
extern bool bGCroucheToggle;

bool CanPurge(EERIE_3D * pos)
{
	long px, pz;
	px = pos->x * ACTIVEBKG->Xmul;

	if (px > ACTIVEBKG->Xsize - 3)
	{
		return true;
	}

	if (px < 2)
	{
		return true;
	}

	pz = pos->z * ACTIVEBKG->Zmul;

	if (pz > ACTIVEBKG->Zsize - 3)
	{
		return true;
	}

	if (pz < 2)
	{
		return true;
	}

	EERIE_BKG_INFO * eg;

	for (long j = pz - 1; j <= pz + 1; j++)
		for (long i = px - 1; i <= px + 1; i++)
		{
			eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];

			if (eg->nbpoly) return false;
		}

	return true;
}

void BIG_PURGE()
{
	long IO_count = 0;
	long LIGHT_count = 0;
	long PATH_count = 0;
	long FOG_count = 0;

	if (OKBox("Do you really want to PURGE this level ???", "Confirm Box"))
	{
		for (long i = 1; i < inter.nbmax; i++)
		{
			if (inter.iobj[i])
				if (CanPurge(&inter.iobj[i]->initpos))
				{
					// purge io
					ARX_INTERACTIVE_DeleteByIndex(i, FLAG_NOCONFIRM | FLAG_DONTKILLDIR);
					IO_count++;
				}
		}

		for (int i = 0; i < MAX_LIGHTS; i++)
		{
			if (GLight[i])
				if (CanPurge(&GLight[i]->pos))
				{
					// purge light
					EERIE_LIGHT_ClearByIndex(i);
					LIGHT_count++;
				}
		}

		for (int i = nbARXpaths - 1; i >= 0; i--)
		{
			EERIE_3D pos;
			pos.x = ARXpaths[i]->initpos.x;
			pos.y = ARXpaths[i]->initpos.y;
			pos.z = ARXpaths[i]->initpos.z;

			if (CanPurge(&pos))   // To check (pos)
			{
				// purge path
				ARX_PATHS_Delete(ARXpaths[i]);
				PATH_count++;
			}
		}

		for (int i = 0; i < MAX_FOG; i++)
		{
			if (fogs[i].exist)
			{
				if (CanPurge(&fogs[i].pos))
				{
					// purge fog
					ARX_FOGS_KillByIndex(i);
					FOG_count++;
				}
			}
		}

		char text[256];
		sprintf(text, "Killed: %ld IO; %ld Lights; %ld Paths; %ld Fogs.",
				IO_count, LIGHT_count, PATH_count, FOG_count);
		LogError << (text);
	}
}

//*************************************************************************************
//*************************************************************************************

EERIE_3DOBJ * _LoadTheObj(const char * text, const char * path)
{
	
	std::string tex1;
	EERIE_3DOBJ * wr;
	
	if (path == NULL)
	{
		tex1 = "Graph\\obj3D\\textures\\";
	}
	else
	{
		tex1 = text;
		RemoveName(tex1);
		tex1 += path;
	}
	
	wr = TheoToEerie_Fast(tex1.c_str(), text, 0);
	return wr;
}

//*************************************************************************************
//*************************************************************************************

void ReplaceSpecifics( char* text )
{
/*	std::string temp = text;
	MakeUpcase( temp );
	size_t graph_loc = temp.find_first_of( "GRAPH" );

	if ( graph_loc != string::npos )
	{
		text = text.substr( graph_loc );
	}
*/
	char			temp[512];
	UINT			size_text = strlen(text);

	for (unsigned long i = 0 ; i < size_text ; i++)
	{
		memcpy(temp, text + i, 5);
		temp[5] = 0;
		std::string temp2 = temp;
		MakeUpcase(temp2);
		strcpy( temp, temp2.c_str() );

		if (!strcmp(temp, "GRAPH"))
		{
			strcpy(temp, text + i);
			strcpy(text, temp);
			return;
		}
	}

	return;
}

extern long NODIRCREATION;
extern long ADDED_IO_NOT_SAVED;

#pragma pack(push,1)
typedef struct
{
	f32	version;
	char	ident[16];
	char	lastuser[256];
	s32	time;
	EERIE_3D pos_edit;
	EERIE_3D angle_edit;
	s32	nb_scn;
	s32	nb_inter;
	s32	nb_nodes;
	s32	nb_nodeslinks;
	s32	nb_zones;
	BOOL	lighting;
	BOOL    Bpad[256];
	s32	nb_lights;
	s32	nb_fogs;

	s32	nb_bkgpolys;
	s32	nb_ignoredpolys;
	s32	nb_childpolys;
	s32	nb_paths;
	s32	pad[250];
	EERIE_3D	offset;
	f32	fpad[253];
	char	cpad[4096];
	BOOL	bpad[256];
} DANAE_LS_HEADER; // Aligned 1 2 4
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
	char	name[512];
	s32	pad[16];
	f32	fpad[16];
} DANAE_LS_SCENE; // Aligned 1 2 4 8
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
	s32		nb_values;
	s32		ViewMode;
	s32		ModeLight;
	s32		pad;
} DANAE_LS_LIGHTINGHEADER; // Aligned 1 2 4 8
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
	s32		r;
	s32		g;
	s32		b;
} DANAE_LS_VLIGHTING; // Aligned 1 2 4
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
	EERIE_3D	pos;
	EERIE_RGB	rgb;
	f32		fallstart;
	f32		fallend;
	f32		intensity;
	f32		i;
	EERIE_RGB	ex_flicker;
	f32		ex_radius;
	f32		ex_frequency;
	f32		ex_size;
	f32		ex_speed;
	f32		ex_flaresize;
	f32		fpadd[24];
	s32		extras;
	s32		lpadd[31];
} DANAE_LS_LIGHT; //ver 1.003f // Aligned 1 2 4
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
	EERIE_3D	pos;
	EERIE_RGB	rgb;
	f32		size;
	s32		special;
	f32		scale;
	EERIE_3D	move;
	EERIE_3D	angle;
	f32		speed;
	f32		rotatespeed;
	s32		tolive;
	s32		blend;
	f32		frequency;
	f32		fpadd[32];
	s32		lpadd[32];
	char		cpadd[256];
} DANAE_LS_FOG; // Aligned 1 2 4
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
	char		name[64];
	EERIE_3D	pos;
	s32		pad[16];
	f32		fpad[16];
} DANAE_LS_NODE; // Aligned 1 2 4
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
	char		name[64];
	s16		idx;
	s16		flags;
	EERIE_3D	initpos;
	EERIE_3D	pos;
	s32		nb_pathways;
	EERIE_RGB	rgb; 
	f32		farclip;
	f32		reverb;
	f32		amb_max_vol;
	f32		fpadd[26];
	s32		height;
	s32		lpadd[31];
	char		ambiance[128]; 
	char		cpadd[128];
} DANAE_LS_PATH; // Aligned 1 2 4
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
	EERIE_3D		rpos;
	s32			flag;
	u32	time;
	f32		fpadd[2];
	s32		lpadd[2];
	char		cpadd[32];
} DANAE_LS_PATHWAYS; // Aligned 1 2 4
#pragma pack(pop)

//Lighting File Header
#pragma pack(push,1)
typedef struct
{
	f32	version;
	char	ident[16];
	char	lastuser[256];
	s32	time;

	s32	nb_lights;
	s32	nb_Shadow_Polys;
	s32	nb_IGNORED_Polys;
	s32	nb_bkgpolys;
	s32	pad[256];
	f32	fpad[256];
	char	cpad[4096];
	BOOL	bpad[256];
} DANAE_LLF_HEADER; //Aligned 1 2 4
#pragma pack(pop)

//*************************************************************************************
//*************************************************************************************

long DanaeSaveLevel( const std::string& _fic )
{
	std::string fic = _fic;
	std::string fic2;
	std::string fic3;
	char _error[512];
	DANAE_LS_HEADER				dlh;
	DANAE_LS_SCENE				dls;
	DANAE_LS_INTER				dli;
	DANAE_LS_LIGHTINGHEADER		dll;
	DANAE_LS_LIGHT				dlight;
	DANAE_LS_FOG				dlf;
	DANAE_LS_NODE				dln;
	char						name[64];
	long nb_inter		=		GetNumberInterWithOutScriptLoadForLevel(CURRENTLEVEL); // Without Player
	unsigned char * dat	=		NULL;
	u32 siz				=		255;
	long pos			=		0;
	FileHandle handle;
	long bcount;

	EERIE_BACKGROUND * eb = ACTIVEBKG;
	long i, j;
	EERIEPOLY * ep;
	EERIE_BKG_INFO * eg;

	memset(&dlh, 0, sizeof(DANAE_LS_HEADER));
	memset(&dls, 0, sizeof(DANAE_LS_SCENE));
	memset(&dli, 0, sizeof(DANAE_LS_INTER));
	// Initializing datas
	HERMES_DATE_TIME hdt;
	GetDate(&hdt);
	char tx[128];
	char newtext[128];
	sprintf(tx, "_%02ld_%02ld_%ld__%ldh%ldmn", hdt.months, hdt.days, hdt.years, hdt.hours, hdt.mins);
	SetExt(fic, ".DLF");

	if (FileExist(fic.c_str()))
	{
		fic2 = fic;
		sprintf(newtext, "Backup_DLF_%s", tx);
		SetExt(fic2, newtext);
		rename(fic.c_str(), fic2.c_str());
	}

	fic2 = fic;
	SetExt(fic2, ".LLF");

	if (FileExist(fic2.c_str()))
	{
		fic3 = fic;
		sprintf(newtext, "Backup_LLF_%s", tx);
		SetExt(fic3, newtext);
		rename(fic2.c_str(), fic3.c_str());
	}

	SetExt(fic, ".DLF");
	bcount = CountBkgVertex();
	dlh.nb_nodes = CountNodes();
	dlh.nb_nodeslinks = MAX_LINKS;
	dlh.nb_lights = 0; // MUST BE 0 !!!!
	dlh.nb_fogs = ARX_FOGS_Count();
	dlh.nb_bkgpolys = BKG_CountPolys(ACTIVEBKG);
	dlh.nb_childpolys = 0;
	dlh.nb_ignoredpolys = BKG_CountIgnoredPolys(ACTIVEBKG);
	dlh.nb_paths = nbARXpaths;
	long allocsize = sizeof(DANAE_LS_HEADER) + sizeof(DANAE_LS_HEADER) * 1 + sizeof(DANAE_LS_INTER) * nb_inter + 512
					 + sizeof(DANAE_LS_LIGHTINGHEADER) + (bcount + 1) * sizeof(D3DCOLOR)
					 + dlh.nb_nodes * (sizeof(DANAE_LS_NODE) + 64 * MAX_LINKS)
					 + dlh.nb_lights * sizeof(DANAE_LS_LIGHT)

					 + 1000000
					 + nbARXpaths * sizeof(DANAE_LS_PATH) + nbARXpaths * sizeof(DANAE_LS_PATHWAYS) * 30;
	long tmpp = dlh.nb_bkgpolys * (sizeof(D3DCOLOR) + 2) + 1000000;
	allocsize = max(tmpp, allocsize);
	dat = (unsigned char *)malloc(allocsize);

	if (dat == NULL)
	{
		strcpy(_error, "Unable to allocate Buffer for save...");
		goto error;
	}

	memset(dat, 0, allocsize);

	// Preparing HEADER
	dlh.version = CURRENT_VERSION;

	if (NODIRCREATION) dlh.version = 1.004f;

	strcpy(dlh.ident, "DANAE_FILE");
	dlh.nb_scn = 1;

	if (LastLoadedScene[0] == 0) dlh.nb_scn = 0;

	dlh.nb_inter = nb_inter;
	dlh.nb_zones = 0;

	if (dlh.nb_scn != 0)
	{
		dlh.pos_edit.x = subj.pos.x - Mscenepos.x;
		dlh.pos_edit.y = subj.pos.y - Mscenepos.y;
		dlh.pos_edit.z = subj.pos.z - Mscenepos.z;
	}
	else
	{
		dlh.pos_edit.x = subj.pos.x;
		dlh.pos_edit.y = subj.pos.y;
		dlh.pos_edit.z = subj.pos.z;
	}

	dlh.angle_edit.a = player.angle.a;
	dlh.angle_edit.b = player.angle.b;
	dlh.angle_edit.g = player.angle.g;
	dlh.lighting = false; // MUST BE false !!!!

//	todo w32 api
//	_time32(&dlh.time);

	GetUserName(dlh.lastuser, &siz);
	memcpy(dat, &dlh, sizeof(DANAE_LS_HEADER));
	pos += sizeof(DANAE_LS_HEADER);

	// Preparing SCENE DATA
	if (dlh.nb_scn > 0)
	{
		strcpy(dls.name, LastLoadedScene);
		memcpy(dat + pos, &dls, sizeof(DANAE_LS_SCENE));
		pos += sizeof(DANAE_LS_SCENE);
	}

	//preparing INTER DATA
	for (i = 1; i < inter.nbmax; i++) // Ignoring Player Data
	{
		if ((inter.iobj[i] != NULL)  && (!inter.iobj[i]->scriptload)
				&& (inter.iobj[i]->truelevel == CURRENTLEVEL))
		{
			INTERACTIVE_OBJ * io = inter.iobj[i];
			memset(&dli, 0, sizeof(DANAE_LS_INTER));

			if (dlh.nb_scn != 0)
			{
				dli.pos.x = io->initpos.x - Mscenepos.x;
				dli.pos.y = io->initpos.y - Mscenepos.y;
				dli.pos.z = io->initpos.z - Mscenepos.z;
			}
			else
			{
				dli.pos.x = io->initpos.x;
				dli.pos.y = io->initpos.y;
				dli.pos.z = io->initpos.z;
			}

			dli.angle.a = io->initangle.a;
			dli.angle.b = io->initangle.b;
			dli.angle.g = io->initangle.g;
			strcpy( dli.name, io->filename);

			if (io->ident == 0)
			{
				MakeIOIdent(io);
			}

			dli.ident = io->ident;

			if (io->ioflags & IO_FREEZESCRIPT)
				dli.flags = IO_FREEZESCRIPT;

			inter.iobj[i]->EditorFlags &= ~EFLAG_NOTSAVED;
			memcpy(dat + pos, &dli, sizeof(DANAE_LS_INTER));
			pos += sizeof(DANAE_LS_INTER);
		}
	}

	long nbvert;

	for (i = 0; i < MAX_FOG; i++)
	{
		if (fogs[i].exist)
		{
			memset(&dlf, 0, sizeof(DANAE_LS_LIGHT));
			dlf.rgb.r = fogs[i].rgb.r;
			dlf.rgb.g = fogs[i].rgb.g;
			dlf.rgb.b = fogs[i].rgb.b;
			dlf.angle.a = fogs[i].angle.a;
			dlf.angle.b = fogs[i].angle.b;
			dlf.angle.g = fogs[i].angle.g;
			dlf.pos.x = fogs[i].pos.x - Mscenepos.x;
			dlf.pos.y = fogs[i].pos.y - Mscenepos.y;
			dlf.pos.z = fogs[i].pos.z - Mscenepos.z;
			dlf.blend = fogs[i].blend;
			dlf.frequency = fogs[i].frequency;
			dlf.move.x = fogs[i].move.x;
			dlf.move.y = fogs[i].move.y;
			dlf.move.z = fogs[i].move.z;
			dlf.rotatespeed = fogs[i].rotatespeed;
			dlf.scale = fogs[i].scale;
			dlf.size = fogs[i].size;
			dlf.special = fogs[i].special;
			dlf.speed = fogs[i].speed;
			dlf.tolive = fogs[i].tolive;
			memcpy(dat + pos, &dlf, sizeof(DANAE_LS_FOG));
			pos += sizeof(DANAE_LS_FOG);
		}
	}

	for (i = 0; i < nodes.nbmax; i++)
	{
		if (nodes.nodes[i].exist)
		{
			memset(&dln, 0, sizeof(DANAE_LS_NODE));
			strcpy(dln.name, nodes.nodes[i].name);
			dln.pos.x = nodes.nodes[i].pos.x - Mscenepos.x;
			dln.pos.y = nodes.nodes[i].pos.y - Mscenepos.y;
			dln.pos.z = nodes.nodes[i].pos.z - Mscenepos.z;
			memcpy(dat + pos, &dln, sizeof(DANAE_LS_NODE));
			pos += sizeof(DANAE_LS_NODE);

			for (long j = 0; j < MAX_LINKS; j++)
			{
				memset(name, 0, 64);

				if (nodes.nodes[i].link[j] != -1)
				{
					if (nodes.nodes[nodes.nodes[i].link[j]].exist)
						strcpy(name, nodes.nodes[nodes.nodes[i].link[j]].name);
				}

				memcpy(dat + pos, name, 64);
				pos += 64;
			}
		}
	}

	DANAE_LS_PATH dlp;
	DANAE_LS_PATHWAYS dlpw;

	for (i = 0; i < nbARXpaths; i++)
	{
		memset(&dlp, 0, sizeof(DANAE_LS_PATH));
		dlp.flags = (short)ARXpaths[i]->flags;
		dlp.idx = ARXpaths[i]->idx;
		dlp.initpos.x = ARXpaths[i]->initpos.x - Mscenepos.x;
		dlp.initpos.y = ARXpaths[i]->initpos.y - Mscenepos.y;
		dlp.initpos.z = ARXpaths[i]->initpos.z - Mscenepos.z;
		dlp.pos.x = ARXpaths[i]->pos.x - Mscenepos.x;
		dlp.pos.y = ARXpaths[i]->pos.y - Mscenepos.y;
		dlp.pos.z = ARXpaths[i]->pos.z - Mscenepos.z;
		strcpy(dlp.name, ARXpaths[i]->name);
		dlp.nb_pathways = ARXpaths[i]->nb_pathways;
		dlp.height = ARXpaths[i]->height;
		strcpy(dlp.ambiance, ARXpaths[i]->ambiance);
		dlp.amb_max_vol = ARXpaths[i]->amb_max_vol;
		dlp.farclip = ARXpaths[i]->farclip;
		dlp.reverb = ARXpaths[i]->reverb;
		dlp.rgb.r = ARXpaths[i]->rgb.r;
		dlp.rgb.g = ARXpaths[i]->rgb.g;
		dlp.rgb.b = ARXpaths[i]->rgb.b;

		memcpy(dat + pos, &dlp, sizeof(DANAE_LS_PATH));
		pos += sizeof(DANAE_LS_PATH);

		for (long j = 0; j < dlp.nb_pathways; j++)
		{
			memset(&dlpw, 0, sizeof(DANAE_LS_PATHWAYS));
			dlpw.flag = ARXpaths[i]->pathways[j].flag;
			dlpw.rpos.x = ARXpaths[i]->pathways[j].rpos.x;
			dlpw.rpos.y = ARXpaths[i]->pathways[j].rpos.y;
			dlpw.rpos.z = ARXpaths[i]->pathways[j].rpos.z;


			float fValue = ARXpaths[i]->pathways[j]._time ;
			ARX_CHECK_ULONG(fValue);

			dlpw.time = ARX_CLEAN_WARN_CAST_ULONG(fValue);


			memcpy(dat + pos, &dlpw, sizeof(DANAE_LS_PATHWAYS));
			pos += sizeof(DANAE_LS_PATHWAYS);
		}
	}

	//Saving Special Polys

	if (pos > allocsize)
	{
		sprintf(_error, "Badly Allocated SaveBuffer...%s", fic.c_str());
		goto error;
	}

	// Now Saving Whole Buffer
	if (!(handle = FileOpenWrite(fic.c_str())))
	{
		sprintf(_error, "Unable to Open %s for Write...", fic.c_str());
		goto error;
	}

	if (FileWrite(handle, dat, sizeof(DANAE_LS_HEADER)) != sizeof(DANAE_LS_HEADER))
	{
		sprintf(_error, "Unable to Write to %s", fic.c_str());
		goto error;
	}

	char * compressed;
	compressed = NULL;
	size_t cpr_pos;
	cpr_pos = 0;
	compressed = implodeAlloc((char *)(dat + sizeof(DANAE_LS_HEADER)), pos - sizeof(DANAE_LS_HEADER), cpr_pos);

	size_t sizeWritten;
	sizeWritten = FileWrite(handle, compressed, cpr_pos);
	
	delete[] compressed;
	FileClose(handle);
	
	if(sizeWritten != cpr_pos) {
		free(dat);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////////
	//Now Save Separate IO Ref Sheet

	//////////////////////////////////////////////////////////////////////////////
	//Now Save Separate LLF Lighting File
	fic2 = fic;
	SetExt(fic2, ".LLF");
	pos = 0;
	DANAE_LLF_HEADER llh;
	memset(&llh, 0, sizeof(DANAE_LLF_HEADER));

	// Preparing HEADER
	llh.version = CURRENT_VERSION;
	llh.nb_lights = EERIE_LIGHT_Count();
	llh.nb_bkgpolys = BKG_CountPolys(ACTIVEBKG);
	strcpy(llh.ident, "DANAE_LLH_FILE");

//	todo w32api
//	_time32(&llh.time);

	GetUserName(llh.lastuser, &siz);

	memcpy(dat, &llh, sizeof(DANAE_LLF_HEADER));
	pos += sizeof(DANAE_LLF_HEADER);

	for (i = 0; i < MAX_LIGHTS; i++)
	{
		EERIE_LIGHT * el = GLight[i];

		if (el != NULL)
			if (!(el->type & TYP_SPECIAL1))
			{
				memset(&dlight, 0, sizeof(DANAE_LS_LIGHT));
				dlight.fallend = el->fallend;
				dlight.fallstart = el->fallstart;
				dlight.intensity = el->intensity;
				dlight.pos.x = el->pos.x - Mscenepos.x;
				dlight.pos.y = el->pos.y - Mscenepos.y;
				dlight.pos.z = el->pos.z - Mscenepos.z;
				dlight.rgb.r = el->rgb.r;
				dlight.rgb.g = el->rgb.g;
				dlight.rgb.b = el->rgb.b;

				dlight.extras = el->extras;
				dlight.ex_flicker.r = el->ex_flicker.r;
				dlight.ex_flicker.g = el->ex_flicker.g;
				dlight.ex_flicker.b = el->ex_flicker.b;
				dlight.ex_radius = el->ex_radius;
				dlight.ex_frequency = el->ex_frequency;
				dlight.ex_size = el->ex_size;
				dlight.ex_speed = el->ex_speed;
				dlight.ex_flaresize = el->ex_flaresize;

				memcpy(dat + pos, &dlight, sizeof(DANAE_LS_LIGHT));
				pos += sizeof(DANAE_LS_LIGHT);
			}
	}

	//Saving Special Polys
	memset(&dll, 0, sizeof(DANAE_LS_LIGHTINGHEADER));
	dll.nb_values = bcount;
	dll.ModeLight = ModeLight;
	dll.ViewMode = ViewMode;

	memcpy(dat + pos, &dll, sizeof(DANAE_LS_LIGHTINGHEADER));
	pos += sizeof(DANAE_LS_LIGHTINGHEADER);

	{
		for (j = 0; j < eb->Zsize; j++)
			for (i = 0; i < eb->Xsize; i++)
			{
				eg = (EERIE_BKG_INFO *)&eb->Backg[i+j*eb->Xsize];

				for (long l = 0; l < eg->nbpoly; l++)
				{
					ep = &eg->polydata[l];

					if (ep != NULL)
					{
						if (ep->type & POLY_QUAD) nbvert = 4;
						else nbvert = 3;

						for (long k = 0; k < nbvert; k++)
						{
							D3DCOLOR tmp = ep->v[k].color;
							memcpy(dat + pos, &tmp, sizeof(D3DCOLOR));
							pos += sizeof(D3DCOLOR);
						}
					}
				}
			}
	}

	if (pos > allocsize)
	{
		sprintf(_error, "Badly Allocated SaveBuffer...%s", fic2.c_str());
		goto error;
	}

	// Now Saving Whole Buffer
	if (!(handle = FileOpenWrite(fic2.c_str())))
	{
		sprintf(_error, "Unable to Open %s for Write...", fic2.c_str());
		goto error;
	}

	compressed = NULL;
	cpr_pos = 0;
	compressed = implodeAlloc((char *)dat, pos, cpr_pos);

	size_t sizeWritten2;
	sizeWritten2 = FileWrite(handle, compressed, cpr_pos);
	
	delete[] compressed;
	FileClose(handle);
	
	if (sizeWritten2 != cpr_pos)
	{
		sprintf(_error, "Unable to Write to %s", fic2.c_str());
		goto error;
	}


	// End of LLF Save
	//////////////////////////////////////////////////////////////////////////////

	// Finish
	free(dat);
	ADDED_IO_NOT_SAVED = 0;
	return 1;
error:
	;
	LogError << (_error);

	if (dat) free(dat);

	return -1;
}

extern char LastLoadedDLF[512];
extern long LOADEDD;

//*************************************************************************************
//*************************************************************************************

void WriteIOInfo(INTERACTIVE_OBJ * io, const std::string& dir)
{
	char dfile[256];
	char temp[256];
	FILE * fic;
	HERMES_DATE_TIME hdt;

	if (DirectoryExist(dir.c_str()))
	{
		strcpy(temp, GetName(io->filename).c_str());
		sprintf(dfile, "%s\\%s.log", dir.c_str(), temp);

		if ((fic = fopen(dfile, "w")) != NULL)
		{
			char name[256];
			u32 num = 255;
			fprintf(fic, "Object   : %s%04ld\n", temp, io->ident);
			fprintf(fic, "_______________________________\n\n");
			GetUserName(name, &num);
			fprintf(fic, "Creator  : %s\n", name);
			GetDate(&hdt);
			fprintf(fic, "Date     : %02ld/%02ld/%ld\n", hdt.days, hdt.months, hdt.years);
			fprintf(fic, "Time     : %ldh%ld\n", hdt.hours, hdt.mins);
			fprintf(fic, "Level    : %s\n", LastLoadedScene);

			if (LastLoadedDLF[0] != 0)
				fprintf(fic, "DLF File : %s\n", LastLoadedDLF);
			else
				fprintf(fic, "DLF File : None\n");

			fprintf(fic, "Position : x %8.f y %8.f z %8.f (relative to anchor)\n",
					io->initpos.x - Mscenepos.x, io->initpos.y - Mscenepos.y, io->initpos.z - Mscenepos.z);
			fclose(fic);
		}
	}
}

//*************************************************************************************
//*************************************************************************************

void LogDirCreation( const std::string& dir) {
	if(DirectoryExist(dir.c_str())) {
		LogDebug << "LogDirCreation: " << dir;
	}
}

//*************************************************************************************
//*************************************************************************************

void LogDirDestruction( const std::string& dir ) {
	if(DirectoryExist(dir.c_str())) {
		LogDebug << "LogDirDestruction: " << dir;
	}
}

//*************************************************************************************
// Checks for IO created during this session but not saved...
//*************************************************************************************
void CheckIO_NOT_SAVED()
{
	std::string temp;
	std::string temp3;

	if (ADDED_IO_NOT_SAVED)
	{
		if (OKBox("You have added objects, but not saved them...\nDELETE THEM ??????", "Danae WARNING"))
		{
			for (long i = 1; i < inter.nbmax; i++) // ignoring player
			{
				if ((inter.iobj[i] != NULL)  && (!inter.iobj[i]->scriptload))

					if (inter.iobj[i]->EditorFlags & EFLAG_NOTSAVED)
					{
						if (inter.iobj[i]->ident > 0)
						{
							temp = inter.iobj[i]->filename;
							string temp2 = GetName(temp);
							RemoveName(temp);
							std::stringstream ss;
							ss << temp << temp2 << '_' << std::setfill('0') << std::setw(4) << inter.iobj[i]->ident;
							ss << std::setw(0) << '.';
							temp = ss.str();
							//temp += "%s%s_%04d." temp2 + '_' + inter.iobj[i]->ident + '.';

							if (DirectoryExist(temp.c_str()))
							{
								temp3 = "Really remove Directory & Directory Contents ?\n\n" + temp;

								if (OKBox(temp3.c_str(), "WARNING"))
								{
									temp += "\\";
									LogDirDestruction(temp.c_str());
									KillAllDirectory(temp.c_str());
								}
							}

							ReleaseInter(inter.iobj[i]);
						}
					}
			}
		}
	}
}

//*************************************************************************************
//*************************************************************************************

void SaveIOScript(INTERACTIVE_OBJ * io, long fl)
{
	std::string temp;
	char temp2[256];
	char temp3[256];

	switch (fl)
	{
		case 1: //CLASS SCRIPT
			temp = io->filename;
			SetExt(temp, "ASL");

//			todo win32api
//			int fic;
//			if ((fic = _open(temp, _O_WRONLY | _O_TRUNC  | _O_CREAT | _O_BINARY, _S_IWRITE)) != -1)
//			{
//				_write(fic, io->script.data, strlen(io->script.data));
//				_close(fic);
//			}
//			else LogError << ("Unable To Save...");

			ARX_SCRIPT_ComputeShortcuts(io->script);
			break;
		case 2: //LOCAL SCRIPT

			if (io->ident != 0)
			{
				temp = io->filename;
				strcpy(temp2, GetName(temp).c_str());
				RemoveName(temp);
				sprintf(temp3, "%s%s_%04ld", temp.c_str(), temp2, io->ident);
				temp = temp3;
				temp += "\\";
				temp += temp2;
				temp += ".asl";

				if (DirectoryExist(temp3))
				{
//					todo win32api
//					int fic;
//					if ((fic = _open(temp, _O_WRONLY | _O_TRUNC  | _O_CREAT | _O_BINARY, _S_IWRITE)) != -1)
//					{
//						_write(fic, io->over_script.data, strlen(io->over_script.data));
//						_close(fic);
//					}
//					else LogError << ("Unable To Save...");
				}
				else LogError << ("Local DIR don't Exists...");
			}
			else LogError << ("NO IDENT...");

			ARX_SCRIPT_ComputeShortcuts(io->over_script);
			break;
	}
}
extern long FORCE_IO_INDEX;
INTERACTIVE_OBJ * LoadInter_Ex(DANAE_LS_INTER * dli, EERIE_3D * trans)
{
	char nameident[256];
	std::string tmp;
	char tmp2[512];
	size_t FileSize;
	INTERACTIVE_OBJ * io;
	sprintf(nameident, "%s_%04d", GetName(dli->name).c_str(), dli->ident);
	long t;
	t = GetTargetByNameTarget(nameident);

	if (FORCE_IO_INDEX != -1)
	{
		io = AddInteractive(GDevice, dli->name, dli->ident, NO_MESH | NO_ON_LOAD);
		goto suite;
	}

	sprintf(nameident, "%s_%04d", GetName(dli->name).c_str(), dli->ident);

	t = GetTargetByNameTarget(nameident);

	if (t >= 0)
	{
		return inter.iobj[t];
	}

	ReplaceSpecifics(dli->name);

	io = AddInteractive(GDevice, dli->name, dli->ident, NO_MESH | NO_ON_LOAD);
suite:
	;

	if (io)
	{
		RestoreInitialIOStatusOfIO(io);
		ARX_INTERACTIVE_HideGore(io);

		io->lastpos.x = io->initpos.x = io->pos.x = dli->pos.x + trans->x; // RELATIVE !!!!!!!!!
		io->lastpos.y = io->initpos.y = io->pos.y = dli->pos.y + trans->y;
		io->lastpos.z = io->initpos.z = io->pos.z = dli->pos.z + trans->z;
		io->move.x = io->move.y = io->move.z = 0.f;
		io->initangle.a = io->angle.a = dli->angle.a;
		io->initangle.b = io->angle.b = dli->angle.b;
		io->initangle.g = io->angle.g = dli->angle.g;

		if (!NODIRCREATION)
		{
			io->ident = dli->ident;
			tmp = io->filename;
			strcpy(tmp2, GetName(tmp).c_str());
			RemoveName(tmp);
			std::stringstream ss;
			ss << tmp << tmp2 << '_' << std::setfill('0') << std::setw(4) << io->ident;
			tmp = ss.str();
			//sprintf(tmp, "%s%s_%04d", tmp.c_str(), tmp2, io->ident);

			if (PAK_DirectoryExist(tmp.c_str()))
			{
				tmp = io->filename;
				strcpy(tmp2, GetName(tmp).c_str());
				RemoveName(tmp);
				std::stringstream ss;
				ss << tmp << tmp2 << '_' << std::setfill('0') << std::setw(4) << io->ident;
				ss << std::setw(0) << '\\' << tmp2 << ".asl";
				tmp = ss.str();
				//sprintf(tmp, "%s%s_%04d\\%s.asl", tmp, tmp2, io->ident, tmp2);

				if (PAK_FileExist(tmp.c_str()))
				{
					if (io->over_script.data)
					{
						free(io->over_script.data);
						io->over_script.data = NULL;
					}

					io->over_script.data = (char *)PAK_FileLoadMallocZero(tmp, FileSize);

					if (io->over_script.data != NULL)
					{
						LogDebug << "Loaded overscript " << tmp << " for IO " << io->filename;
						io->over_script.size = FileSize;
						InitScript(&io->over_script);
					}

					if (io->script.data != NULL)
						io->over_script.master = &io->script;
					else io->over_script.master = NULL;
				}
			} else {
				CreateFullPath(tmp);
				LogDirCreation(tmp);
				WriteIOInfo(io, tmp);
			}
		}

		if (SendIOScriptEvent(io, SM_LOAD) == ACCEPT)
		{
			if (io->obj == NULL)
			{
				const char dirpath[] = "Graph\\Obj3D\\Textures\\";

				if (io->ioflags & IO_ITEM)
					io->obj = TheoToEerie_Fast(dirpath, io->filename, 0, GDevice);
				else if (io->ioflags & IO_NPC)
					io->obj = TheoToEerie_Fast(dirpath, io->filename, TTE_NO_PHYSICS_BOX | TTE_NPC, GDevice);
				else
					io->obj = TheoToEerie_Fast(dirpath, io->filename, TTE_NO_PHYSICS_BOX, GDevice);

				if (io->ioflags & IO_NPC)
					EERIE_COLLISION_Cylinder_Create(io);
			}
		}

		InterTreeViewItemAdd(io);
	}

	return io;
}
long LastLoadedLightningNb = 0;
D3DCOLOR * LastLoadedLightning = NULL;
EERIE_3D loddpos;
EERIE_3D MSP;
extern long DEBUGCODE;

//*************************************************************************************
//*************************************************************************************

extern char  _CURRENTLOAD_[256];
void ClearCurLoadInfo()
{
	memset(_CURRENTLOAD_, 0, 256);
}

extern long FASTmse;
long DONT_LOAD_INTERS = 0;
long FAKE_DIR = 0;

long DanaeLoadLevel(const std::string& fic) {
	
	DANAE_LS_HEADER				dlh;
	DANAE_LS_SCENE		*		dls;
	DANAE_LS_INTER		*		dli;
	DANAE_LS_LIGHT		*		dlight;
	DANAE_LS_FOG		*		dlf;
	DANAE_LS_LIGHTINGHEADER	*	dll;
	DANAE_LS_NODE		*		dln;
	char name[64];
	std::string temp;
	EERIE_3D trans;
	unsigned char * dat = NULL;
	float increment = 0;

	long pos = 0;
	long i;
	size_t FileSize = 0;
	char tstr[128];
	HERMES_DATE_TIME hdt;
	LogInfo << "Loading Level " << fic;
	ClearCurLoadInfo();
	CURRENTLEVEL = GetLevelNumByName(fic.c_str());
	GetDate(&hdt);
	sprintf(tstr, "%2ldh%02ldm%02ld LOADLEVEL start", hdt.hours, hdt.mins, hdt.secs);
	ForceSendConsole(tstr, 1, 0, (HWND)1);
	std::string fileDlf;
	fileDlf = fic;
//	SetExt(fileDlf, ".DLF");
	std::string fic2;
	fic2 = fic;
	SetExt(fic2, ".LLF");

	LogDebug << "fic2 " << fic2;
	LogDebug << "fileDlf " << fileDlf;

	if (!PAK_FileExist(fileDlf.c_str())) {
		LogError <<"Unable to find "<< fileDlf;
		return -1;
	}

	strcpy(LastLoadedDLF, fic.c_str());

	dat = (unsigned char *)PAK_FileLoadMalloc(fic, FileSize);
	PROGRESS_BAR_COUNT += 1.f;
	LoadLevelScreen();
	memcpy(&dlh, dat, sizeof(DANAE_LS_HEADER));
	pos += sizeof(DANAE_LS_HEADER);
	// TODO copying directly into structs is not very portable

	LogDebug << "dlh.version " << dlh.version << " header size " << sizeof(DANAE_LS_HEADER);

	if (dlh.version > CURRENT_VERSION) // using compression
	{
		LogError << ("DANAE Version too OLD to load this File... Please upgrade to a new DANAE Version...");
		free(dat);
		dat = NULL;
		return -1;
	}

	if (dlh.version >= 1.44f) // using compression
	{
		char * torelease = (char *)dat;
		char * compressed = (char *)(dat + pos);
		long cpr_pos;
		cpr_pos = 0;
		dat = (unsigned char *)blastMemAlloc(compressed, FileSize - pos, FileSize);

		free(torelease);
		compressed = NULL;
		pos = 0;

		if (dat == NULL) {
			LogError << "STD_Explode did not return anything "<< fileDlf;
			return -1;
		}
	}

	loddpos.x = subj.pos.x = dlh.pos_edit.x;
	loddpos.y = subj.pos.y = dlh.pos_edit.y;
	loddpos.z = subj.pos.z = dlh.pos_edit.z;
	player.desiredangle.a = player.angle.a = subj.angle.a = dlh.angle_edit.a;
	player.desiredangle.b = player.angle.b = subj.angle.b = dlh.angle_edit.b;
	player.desiredangle.g = player.angle.g = subj.angle.g = dlh.angle_edit.g;

	if (strcmp(dlh.ident, "DANAE_FILE"))
	{
		LogError << "Not a valid file "<< fileDlf;
		return -1;
	}

	if (dlh.version < 1.001f)
	{
		dlh.nb_nodes = 0;
	}

	LogDebug << "Loading Scene";

	// Loading Scene
	if (dlh.nb_scn >= 1)
	{
		dls = (DANAE_LS_SCENE *)(dat + pos);
		pos += sizeof(DANAE_LS_SCENE);
		std::string ftemp;

		if (FAKE_DIR)
		{
			ftemp = fic;
			RemoveName(ftemp);
			temp = fic;
			RemoveName(temp);
			FAKE_DIR = 0;
		}
		else // normal load
		{
			ftemp = dls->name;
			RemoveName(ftemp);
			temp = dls->name;
			RemoveName(temp);
		}

		if (FastSceneLoad(ftemp.c_str()))
		{
			LogDebug << "done loading scene";
			FASTmse = 1;
			goto suite;
		}
		LogDebug << "loading scene failed";

		ARX_SOUND_PlayCinematic("Editor_Humiliation.wav");
		mse = PAK_MultiSceneToEerie(temp.c_str());
		PROGRESS_BAR_COUNT += 20.f;
		LoadLevelScreen();
	suite:
		;
		EERIEPOLY_Compute_PolyIn();
		strcpy(LastLoadedScene, ftemp.c_str());
		std::string str_LastLoadedScene = LastLoadedScene;
		RemoveName(str_LastLoadedScene);
	}

	if (FASTmse)
	{
		trans.x = Mscenepos.x;
		trans.y = Mscenepos.y;
		trans.z = Mscenepos.z;
		player.pos.x = loddpos.x + trans.x;
		player.pos.y = loddpos.y + trans.y;
		player.pos.z = loddpos.z + trans.z;
	}
	else if (mse != NULL)
	{
		Mscenepos.x = -mse->cub.xmin - (mse->cub.xmax - mse->cub.xmin) * ( 1.0f / 2 ) + ((float)ACTIVEBKG->Xsize * (float)ACTIVEBKG->Xdiv) * ( 1.0f / 2 );

		Mscenepos.z = -mse->cub.zmin - (mse->cub.zmax - mse->cub.zmin) * ( 1.0f / 2 ) + ((float)ACTIVEBKG->Zsize * (float)ACTIVEBKG->Zdiv) * ( 1.0f / 2 );
		float t1 = (float)(long)(mse->point0.x / BKG_SIZX);
		float t2 = (float)(long)(mse->point0.z / BKG_SIZZ);
		t1 = mse->point0.x - t1 * BKG_SIZX;
		t2 = mse->point0.z - t2 * BKG_SIZZ;
		Mscenepos.x = (float)((long)(Mscenepos.x / BKG_SIZX)) * BKG_SIZX + (float)BKG_SIZX * ( 1.0f / 2 );
		Mscenepos.z = (float)((long)(Mscenepos.z / BKG_SIZZ)) * BKG_SIZZ + (float)BKG_SIZZ * ( 1.0f / 2 );
		mse->pos.x = Mscenepos.x = Mscenepos.x + BKG_SIZX - t1;
		mse->pos.z = Mscenepos.z = Mscenepos.z + BKG_SIZZ - t2;
		Mscenepos.y = mse->pos.y = -mse->cub.ymin - 100.f - mse->point0.y;
		lastteleport.x = mapcam.pos.x = player.pos.x = subj.pos.x = moveto.x = mse->pos.x + mse->point0.x;
		lastteleport.z = mapcam.pos.z = player.pos.z = subj.pos.z = moveto.z = mse->pos.z + mse->point0.z;
		lastteleport.y         = player.pos.y = subj.pos.y = moveto.y = mse->pos.y + mse->point0.y;
		lastteleport.y -= 180.f;
		player.pos.y = subj.pos.y -= 180.f;
		trans.x = mse->pos.x;
		trans.y = mse->pos.y;
		trans.z = mse->pos.z;
	}
	else
	{
		lastteleport.x = 0.f;
		lastteleport.y = PLAYER_BASE_HEIGHT;
		lastteleport.z = 0.f;
		trans.x = 0;
		trans.y = 0;
		trans.z = 0;
		Mscenepos.x = 0.f;
		Mscenepos.z = 0.f;
		Mscenepos.y = 0.f;
	}

	MSP.x = trans.x;
	MSP.y = trans.y;
	MSP.z = trans.z;

	ClearCurLoadInfo();

	if (dlh.nb_inter > 0)
	{
		increment = (60.f / (float)dlh.nb_inter);
	}
	else
	{
		PROGRESS_BAR_COUNT += 60;
		LoadLevelScreen();
	}

	for (i = 0 ; i < dlh.nb_inter ; i++)
	{
		PROGRESS_BAR_COUNT += increment;
		LoadLevelScreen();
		dli = (DANAE_LS_INTER *)(dat + pos);
		pos += sizeof(DANAE_LS_INTER);

		if (!DONT_LOAD_INTERS)
		{
			LoadInter_Ex(dli, &trans);
		}
	}

	if (dlh.lighting)
	{
		if (!PAK_FileExist(fic2.c_str()))
		{
			dll = (DANAE_LS_LIGHTINGHEADER *)(dat + pos);
			pos += sizeof(DANAE_LS_LIGHTINGHEADER);
			long bcount = dll->nb_values;
			LastLoadedLightningNb = bcount;

			if (LastLoadedLightning != NULL)
			{
				free(LastLoadedLightning);
				LastLoadedLightning = NULL;
			}

			//DANAE_LS_VLIGHTING
			D3DCOLOR * ll = LastLoadedLightning = (D3DCOLOR *)malloc(sizeof(D3DCOLOR) * bcount);

			if (dlh.version > 1.001f)
			{
				memcpy(LastLoadedLightning, dat + pos, sizeof(D3DCOLOR)*bcount);
				pos += sizeof(D3DCOLOR) * bcount;
			}
			else
			{
				DANAE_LS_VLIGHTING dlv;

				while (bcount)
				{
					memcpy(&dlv, dat + pos, sizeof(DANAE_LS_VLIGHTING));
					pos += sizeof(DANAE_LS_VLIGHTING);
					*ll = 0xff000000L | ((dlv.r & 255) << 16) | ((dlv.g & 255) << 8) | (dlv.b & 255);
					ll++;
					bcount--;
				}
			}

			ModeLight = dll->ModeLight;
			ViewMode = dll->ViewMode;
			ViewMode &= ~VIEWMODE_WIRE;
		}
		else
		{
			dll = (DANAE_LS_LIGHTINGHEADER *)(dat + pos);

			pos += sizeof(DANAE_LS_LIGHTINGHEADER);
			long bcount = dll->nb_values;
			pos += sizeof(D3DCOLOR) * bcount;
			ModeLight = dll->ModeLight;
			ViewMode = dll->ViewMode;
			ViewMode &= ~VIEWMODE_WIRE;
		}

	}

	PROGRESS_BAR_COUNT += 1;
	LoadLevelScreen();

	if (dlh.version < 1.003f) dlh.nb_lights = 0;

	if (!PAK_FileExist(fic2.c_str()))
	{
		if (dlh.nb_lights != 0)
		{
			EERIE_LIGHT_GlobalInit();
		}

		long j;

		for (i = 0; i < dlh.nb_lights; i++)
		{
			dlight = (DANAE_LS_LIGHT *)(dat + pos);

			pos += sizeof(DANAE_LS_LIGHT);
			j = EERIE_LIGHT_Create();

			if (j >= 0)
			{
				EERIE_LIGHT * el = GLight[j];

				el->exist		= 1;
				el->treat		= 1;
				el->fallend		= dlight->fallend;
				el->fallstart	= dlight->fallstart;
				el->falldiff	= el->fallend - el->fallstart;
				el->falldiffmul	= 1.f / el->falldiff;
				el->intensity	= dlight->intensity;

				el->pos.x		= dlight->pos.x;
				el->pos.y		= dlight->pos.y;
				el->pos.z		= dlight->pos.z;
				el->rgb.r		= dlight->rgb.r;
				el->rgb.g		= dlight->rgb.g;
				el->rgb.b		= dlight->rgb.b;


				ARX_CHECK_SHORT(dlight->extras);
				el->extras		= ARX_CLEAN_WARN_CAST_SHORT(dlight->extras);

				el->ex_flicker.r = dlight->ex_flicker.r;
				el->ex_flicker.g = dlight->ex_flicker.g;
				el->ex_flicker.b = dlight->ex_flicker.b;
				el->ex_radius	= dlight->ex_radius;
				el->ex_frequency = dlight->ex_frequency;
				el->ex_size		= dlight->ex_size;
				el->ex_speed	= dlight->ex_speed;
				el->tl = -1;
				el->sample = ARX_SOUND_INVALID_RESOURCE;

				if ((el->extras & EXTRAS_SPAWNFIRE))
				{
					el->extras |= EXTRAS_FLARE;

					if (el->extras & EXTRAS_FIREPLACE)
						el->ex_flaresize = 95.f;
					else
						el->ex_flaresize = 40.f;
				}
			}
		}
	}
	else
	{
		pos += sizeof(DANAE_LS_LIGHT) * dlh.nb_lights;
	}

	ClearCurLoadInfo();
	LogDebug << "Loading FOGS";
	ARX_FOGS_Clear();

	for (i = 0; i < dlh.nb_fogs; i++)
	{
		dlf = (DANAE_LS_FOG *)(dat + pos);
		pos += sizeof(DANAE_LS_FOG);
		long n = ARX_FOGS_GetFree();

		if (n > -1)
		{
			FOG_DEF * fd = &fogs[n];
			fd->exist	= 1;
			fd->rgb.r	= dlf->rgb.r;
			fd->rgb.g	= dlf->rgb.g;
			fd->rgb.b	= dlf->rgb.b;
			fd->angle.a	= dlf->angle.a;
			fd->angle.b	= dlf->angle.b;
			fd->angle.g	= dlf->angle.g;
			fd->pos.x	= dlf->pos.x + trans.x;
			fd->pos.y	= dlf->pos.y + trans.y;
			fd->pos.z	= dlf->pos.z + trans.z;
			fd->blend	= dlf->blend;
			fd->frequency = dlf->frequency;
			fd->move.x	= dlf->move.x;
			fd->move.y	= dlf->move.y;
			fd->move.z	= dlf->move.z;
			fd->rotatespeed = dlf->rotatespeed;
			fd->scale	= dlf->scale;
			fd->size	= dlf->size;
			fd->special	= dlf->special;
			fd->speed	= dlf->speed;
			fd->tolive	= dlf->tolive;

			fd->move.x = 1.f;
			fd->move.y = 0.f;
			fd->move.z = 0.f;
			EERIE_3D out;
			float t;
			t = radians(MAKEANGLE(fd->angle.b));
			_YRotatePoint(&fd->move, &out, EEcos(t), EEsin(t));
			t = radians(MAKEANGLE(fd->angle.a));
			_XRotatePoint(&out, &fd->move, EEcos(t), EEsin(t));
		}
	}

	PROGRESS_BAR_COUNT += 2.f;
	LoadLevelScreen();

	ClearCurLoadInfo();
	LogDebug << "Loading Nodes";
	ClearNodes();

	for (i = 0; i < dlh.nb_nodes; i++)
	{
		nodes.nodes[i].exist = 1;
		nodes.nodes[i].selected = 0;
		dln = (DANAE_LS_NODE *)(dat + pos);
		pos += sizeof(DANAE_LS_NODE);

		strcpy(nodes.nodes[i].name, dln->name);
		nodes.nodes[i].pos.x = dln->pos.x + trans.x;
		nodes.nodes[i].pos.y = dln->pos.y + trans.y;
		nodes.nodes[i].pos.z = dln->pos.z + trans.z;

		for (long j = 0; j < dlh.nb_nodeslinks; j++)
		{
			memcpy(name, dat + pos, 64);
			pos += 64;

			if (name[0] != 0)
			{
				strcpy(nodes.nodes[i].lnames[j], name);
			}
		}
	}

	RestoreNodeNumbers();

	ClearCurLoadInfo();
	LogDebug << "Loading Paths";
	ARX_PATH_ReleaseAllPath();
	DANAE_LS_PATH  * dlp;
	DANAE_LS_PATHWAYS  * dlpw;

	if (dlh.nb_paths)
	{
		ARXpaths = (ARX_PATH **)malloc(sizeof(ARX_PATH *) * dlh.nb_paths);
		nbARXpaths = dlh.nb_paths;
	}

	for (i = 0; i < dlh.nb_paths; i++)
	{
		ARX_PATH * ap = ARXpaths[i] = (ARX_PATH *)malloc(sizeof(ARX_PATH));
		memset(ap, 0, sizeof(ARX_PATH));
		dlp = (DANAE_LS_PATH *)(dat + pos);
		pos += sizeof(DANAE_LS_PATH);
		ap->flags		= dlp->flags;
		ap->idx		= dlp->idx;
		ap->initpos.x	= dlp->initpos.x + trans.x;
		ap->initpos.y	= dlp->initpos.y + trans.y;
		ap->initpos.z	= dlp->initpos.z + trans.z;
		ap->pos.x		= dlp->pos.x + trans.x;
		ap->pos.y		= dlp->pos.y + trans.y;
		ap->pos.z		= dlp->pos.z + trans.z;
		strcpy(ap->name, dlp->name);
		ap->nb_pathways = dlp->nb_pathways;
		ap->height		= dlp->height;
		strcpy(ap->ambiance, dlp->ambiance);
		ap->amb_max_vol = dlp->amb_max_vol;

		if (ap->amb_max_vol <= 1.f) ap->amb_max_vol = 100.f;

		ap->farclip	= dlp->farclip;
		ap->reverb		= dlp->reverb;
		ap->rgb.r		= dlp->rgb.r;
		ap->rgb.g		= dlp->rgb.g;
		ap->rgb.b		= dlp->rgb.b;

		ARX_PATHWAY * app = ap->pathways = (ARX_PATHWAY *)malloc(sizeof(ARX_PATHWAY) * dlp->nb_pathways);
		memset(app, 0, sizeof(ARX_PATHWAY)*dlp->nb_pathways);
//
		for (long j = 0; j < dlp->nb_pathways; j++)
		{
			dlpw = (DANAE_LS_PATHWAYS *)(dat + pos);
			pos += sizeof(DANAE_LS_PATHWAYS);

			app[j].flag		=	dlpw->flag;
			app[j].rpos.x	=	dlpw->rpos.x;
			app[j].rpos.y	=	dlpw->rpos.y;
			app[j].rpos.z	=	dlpw->rpos.z;
			app[j]._time	=	ARX_CLEAN_WARN_CAST_FLOAT(dlpw->time);
		}
	}

	ARX_PATH_ComputeAllBoundingBoxes();
	PROGRESS_BAR_COUNT += 5.f;
	LoadLevelScreen();

	//////////////////////////////////////////////////////////////////////////////
	//Now LOAD Separate LLF Lighting File
	free(dat);
	pos = 0;
	DANAE_LLF_HEADER * llh;

	if (!PAK_FileExist(fic2.c_str()))
	{
		goto finish;
	}

	ClearCurLoadInfo();
	LogDebug << "Loading LLF Info";

	if (dlh.version >= 1.44f) // using compression
	{
		size_t size;
		char * compressed = (char *)PAK_FileLoadMalloc(fic2, size);
		dat = (unsigned char *)blastMemAlloc(compressed, size, FileSize);

		if (dat == NULL)
		{
			free(compressed);
			goto finish;
		}

		free(compressed);
		pos = 0;
	}
	else
	{
		dat = (unsigned char *)PAK_FileLoadMalloc(fic2, FileSize);
	}

	llh = (DANAE_LLF_HEADER *)(dat);
	pos += sizeof(DANAE_LLF_HEADER);
	PROGRESS_BAR_COUNT += 4.f;
	LoadLevelScreen();

	if (llh->nb_lights != 0)
	{
		EERIE_LIGHT_GlobalInit();
	}

	long j;

	for (int i = 0; i < llh->nb_lights; i++)
	{
		dlight = (DANAE_LS_LIGHT *)(dat + pos);
		pos += sizeof(DANAE_LS_LIGHT);

		j = EERIE_LIGHT_Create();

		if (j >= 0)
		{
			EERIE_LIGHT * el = GLight[j];

			el->exist = 1;
			el->treat = 1;
			el->fallend = dlight->fallend;
			el->fallstart = dlight->fallstart;
			el->falldiff = el->fallend - el->fallstart;
			el->falldiffmul = 1.f / el->falldiff;
			el->intensity = dlight->intensity;

			if (FASTmse)
			{
				el->pos.x = dlight->pos.x + trans.x;
				el->pos.y = dlight->pos.y + trans.y;
				el->pos.z = dlight->pos.z + trans.z;
			}
			else
			{
				el->pos.x = dlight->pos.x;
				el->pos.y = dlight->pos.y;
				el->pos.z = dlight->pos.z;
			}

			el->rgb.r = dlight->rgb.r;
			el->rgb.g = dlight->rgb.g;
			el->rgb.b = dlight->rgb.b;


			ARX_CHECK_SHORT(dlight->extras);
			el->extras		= ARX_CLEAN_WARN_CAST_SHORT(dlight->extras);

			el->ex_flicker.r = dlight->ex_flicker.r;
			el->ex_flicker.g = dlight->ex_flicker.g;
			el->ex_flicker.b = dlight->ex_flicker.b;
			el->ex_radius = dlight->ex_radius;
			el->ex_frequency = dlight->ex_frequency;
			el->ex_size = dlight->ex_size;
			el->ex_speed = dlight->ex_speed;
			el->ex_flaresize = dlight->ex_flaresize;

			if (el->extras & EXTRAS_STARTEXTINGUISHED)
				el->status = 0;
			else el->status = 1;

			if ((el->extras & EXTRAS_SPAWNFIRE) && (!(el->extras & EXTRAS_FLARE)))
			{
				el->extras |= EXTRAS_FLARE;

				if (el->extras & EXTRAS_FIREPLACE)
					el->ex_flaresize = 95.f;
				else
					el->ex_flaresize = 80.f;
			}

			el->tl = -1;
			el->sample = ARX_SOUND_INVALID_RESOURCE;
		}
	}

	PROGRESS_BAR_COUNT += 2.f;
	LoadLevelScreen();
	dll = (DANAE_LS_LIGHTINGHEADER *)(dat + pos);
	pos += sizeof(DANAE_LS_LIGHTINGHEADER);
	long bcount;
	bcount = dll->nb_values;
	LastLoadedLightningNb = bcount;

	if (LastLoadedLightning != NULL)
	{
		free(LastLoadedLightning);
		LastLoadedLightning = NULL;
	}

	//DANAE_LS_VLIGHTING
	D3DCOLOR * ll;
	ll = LastLoadedLightning = (D3DCOLOR *)malloc(sizeof(D3DCOLOR) * bcount);

	if (dlh.version > 1.001f)
	{
		memcpy(LastLoadedLightning, dat + pos, sizeof(D3DCOLOR)*bcount);
		pos += sizeof(D3DCOLOR) * bcount;
	}
	else
	{
		DANAE_LS_VLIGHTING dlv;

		while (bcount)
		{
			memcpy(&dlv, dat + pos, sizeof(DANAE_LS_VLIGHTING));
			pos += sizeof(DANAE_LS_VLIGHTING);
			*ll = 0xff000000L | ((dlv.r & 255) << 16) | ((dlv.g & 255) << 8) | (dlv.b & 255);
			ll++;
			bcount--;
		}
	}

	ModeLight = dll->ModeLight;
	ViewMode = dll->ViewMode;
	ViewMode &= ~VIEWMODE_WIRE;
	free(dat);

	PROGRESS_BAR_COUNT += 1.f;
	LoadLevelScreen();
finish:
	;

	LOADEDD = 1;
	FASTmse = 0;
	USE_PLAYERCOLLISIONS = 1;
	
	LogInfo << "Done loading level";
	
	return 1;

//loaderror:
//	;
//	FASTmse = 0;
//	LogError << (_error);
//
//	if (dat) free(dat);
//
//	LOADEDD = 1;
//	ARX_SCENE_Render(NULL,3);
//	return -1;
}
extern void MCache_ClearAll();
extern TextureContainer * MapMarkerTc;
extern HANDLE LIGHTTHREAD;
extern long DONT_CLEAR_SCENE;
long FAST_RELEASE = 0;
extern INTERACTIVE_OBJ * FlyingOverIO;
extern void ARX_SOUND_Reinit();
extern unsigned long LAST_JUMP_ENDTIME;

extern EERIE_3DOBJ * stone0;
extern long stone0_count;
extern EERIE_3DOBJ * stone1;
extern long stone1_count;
extern EERIE_3DOBJ * ssol;
extern long ssol_count;
extern EERIE_3DOBJ * slight;
extern long slight_count;
extern EERIE_3DOBJ * srune;
extern long srune_count;
extern EERIE_3DOBJ * smotte;
extern long smotte_count;
extern EERIE_3DOBJ * stite;
extern long stite_count;
extern EERIE_3DOBJ * smissile;
extern long smissile_count;
extern EERIE_3DOBJ * spapi;
extern long spapi_count;
extern EERIE_3DOBJ * sfirewave;
extern long sfirewave_count;
extern EERIE_3DOBJ * svoodoo;
extern long svoodoo_count;
void ReleaseAllSpellResources()
{
	smissile_count = 0;

	if (smissile)
	{
		ReleaseEERIE3DObj(smissile);
		smissile = NULL;
	}

	stite_count = 0;

	if (stite)
	{
		ReleaseEERIE3DObj(stite);
		stite = NULL;
	}

	smotte_count = 0;

	if (smotte)
	{
		ReleaseEERIE3DObj(smotte);
		smotte = NULL;
	}

	ssol_count = 0;

	if (ssol)
	{
		ReleaseEERIE3DObj(ssol);
		ssol = NULL;
	}

	slight_count = 0;

	if (slight)
	{
		ReleaseEERIE3DObj(slight);
		slight = NULL;
	}

	srune_count = 0;

	if (srune)
	{
		ReleaseEERIE3DObj(srune);
		srune = NULL;
	}

	svoodoo_count--;

	if (svoodoo)
	{
		ReleaseEERIE3DObj(svoodoo);
		svoodoo = NULL;
	}

	stone0_count = 0;

	if (stone0)
	{
		ReleaseEERIE3DObj(stone0);
		stone0 = NULL;
	}

	stone1_count = 0;

	if (stone1)
	{
		ReleaseEERIE3DObj(stone1);
		stone1 = NULL;
	}

	spapi_count = 0;

	if (spapi)
	{
		ReleaseEERIE3DObj(spapi);
		spapi = NULL;
	}

	sfirewave_count = 0;

	if (sfirewave)
	{
		ReleaseEERIE3DObj(sfirewave);
		sfirewave = NULL;
	}
}

extern long JUST_RELOADED;

void DanaeClearLevel(long flag)
{
	JUST_RELOADED = 0;
	ARX_MINIMAP_Reset();

	FADEDIR = 0;
	FADEDURATION = 0;
	LAST_JUMP_ENDTIME = 0;
	FAST_RELEASE = 1;
	MCache_ClearAll();
	ARX_MINIMAP_PurgeTC();
	ARX_GAME_Reset(flag);
	FlyingOverIO = NULL;

	EERIE_PATHFINDER_Release();

	if (LIGHTTHREAD != NULL)
	{
		TerminateThread(LIGHTTHREAD, 1);
		LIGHTTHREAD = NULL;
	}

	InitBkg(ACTIVEBKG, MAX_BKGX, MAX_BKGZ, BKG_SIZX, BKG_SIZZ);
	RemoveAllBackgroundActions();
	ClearNodes();

	if (mse != NULL)
	{
		ReleaseMultiScene(mse);
		mse = NULL;
	}

	EERIE_LIGHT_GlobalInit();
	ARX_FOGS_Clear();
	TOTIOPDL = 0;

	UnlinkAllLinkedObjects();

	FreeAllInter();

	ReleaseAllSpellResources();
	ReleaseAllTCWithFlag(EERIETEXTUREFLAG_LOADSCENE_RELEASE);
	danaeApp.EvictManagedTextures();
	MapMarkerTc = NULL;
	ARX_TIME_Init();

	bGToggleCombatModeWithKey = false;
	bGCroucheToggle = false;

	INTERTRANSPOLYSPOS = 0;

	for (long i = 0; i < MAX_DYNLIGHTS; i++) // DynLight 0 is reserved for torch & flares !
	{
		DynLight[i].exist = 0;
	}

	TREATZONE_Release();
	TREATZONE_Clear();

	FAST_RELEASE = 0;
}

extern void ARX_SOUND_PreloadAll();

void DanaeClearAll()
{
	DanaeClearLevel();
	ARX_SOUND_PreloadAll();
}

extern long MOULINEX;

//*************************************************************************************
//*************************************************************************************

void RestoreLastLoadedLightning()
{
	D3DCOLOR dc;
	long pos = 0;
	long bcount = CountBkgVertex();

	if (LastLoadedLightningNb <= 0) return;

	if (LastLoadedLightning == NULL) return;

	if (bcount != LastLoadedLightningNb)
	{
		free(LastLoadedLightning);
		LastLoadedLightning = NULL;
		LastLoadedLightningNb = 0;
		return;
	}

	EERIE_BKG_INFO * eg;

	EERIE_BACKGROUND * eb = ACTIVEBKG;
	long i, j;

	bcount = LastLoadedLightningNb;

	EERIEPOLY * ep;
	long nbvert;

	for (j = 0; j < eb->Zsize; j++)
		for (i = 0; i < eb->Xsize; i++)
		{
			eg = (EERIE_BKG_INFO *)&eb->Backg[i+j*eb->Xsize];

			for (long l = 0; l < eg->nbpoly; l++)
			{
				ep = &eg->polydata[l];

				if (ep->type & POLY_QUAD) nbvert = 4;
				else nbvert = 3;

				for (long k = 0; k < nbvert; k++)
				{
					memcpy(&dc, LastLoadedLightning + pos, sizeof(D3DCOLOR));
					pos++;
					dc = dc | 0xFF000000;
					ep->tv[k].color = ep->v[k].color = dc;
					ep->tv[k].specular = ep->v[k].specular = 0xFF000000;
					bcount--;

					if (bcount <= 0) goto plusloin;
				}
			}
		}

plusloin:
	;
	free(LastLoadedLightning);
	LastLoadedLightning = NULL;
	LastLoadedLightningNb = 0;

	for (i = 0; i < MAX_ACTIONS; i++)
	{
		if (actions[i].exist)
		{
			long modd = 0;

			switch (actions[i].type)
			{
				case ACT_FIRE:
					modd = 1;
					break;
				case ACT_FIRE2:
					modd = 1;
					break;
			}

			if (modd) _RecalcLightZone(actions[i].pos.x, actions[i].pos.z, (long)((float)(DynLight[actions[i].dl].fallend * ACTIVEBKG->Xmul) + 5.f));
		}
	}
}
struct DLFCHECK
{
	char ident[256];
	char nums[512];
	long occurence;
};
DLFCHECK * dlfcheck = NULL;
long dlfcount = 0;
void ARX_SAVELOAD_DLFCheckInit()
{
	if (dlfcheck)
		free(dlfcheck);

	dlfcheck = NULL;
	dlfcount = 0;
}

long GetIdent( const std::string& ident)
{
	for (long n = 0; n < dlfcount; n++)
	{
		if (!strcasecmp(dlfcheck[n].ident, ident.c_str()))
			return n;
	}

	return -1;
}

void AddIdent( std::string& ident, long num)
{
	MakeUpcase(ident);
	long n = GetIdent(ident);

	if (n != -1)
	{
		dlfcheck[n].occurence++;
		char temp[64];
		sprintf(temp, "%ld ", num);

		if (strlen(dlfcheck[n].nums) < 500)
			strcat(dlfcheck[n].nums, temp);
	}
	else
	{
		dlfcheck = (DLFCHECK *)realloc(dlfcheck, sizeof(DLFCHECK) * (dlfcount + 1));
		strcpy(dlfcheck[dlfcount].ident, ident.c_str());
		dlfcheck[dlfcount].occurence = 1;
		sprintf(dlfcheck[dlfcount].nums, "%ld ", num);
		dlfcount++;
	}
}
void ARX_SAVELOAD_DLFCheckAdd(char * path, long num)
{
	std::string fic("Graph\\Levels\\Level");
	fic += path;

	char _error[512];
	DANAE_LS_HEADER				dlh;
	DANAE_LS_SCENE		*		dls;
	DANAE_LS_INTER		*		dli;
	unsigned char * dat = NULL;

	long pos = 0;
	long i;
	size_t FileSize = 0;

	SetExt(fic, ".DLF");

	if (!PAK_FileExist(fic.c_str()))
	{
		return;
	}

	dat = (unsigned char *)PAK_FileLoadMalloc(fic, FileSize);
	memcpy(&dlh, dat, sizeof(DANAE_LS_HEADER));
	pos += sizeof(DANAE_LS_HEADER);

	if (dlh.version > CURRENT_VERSION) // using compression
	{
		LogError << ("DANAE Version too OLD to load this File... Please upgrade to a new DANAE Version...");
		free(dat);
		dat = NULL;
		return;
	}

	if (dlh.version >= 1.44f) // using compression
	{
		char * torelease = (char *)dat;
		char * compressed = (char *)(dat + pos);
		long cpr_pos;
		cpr_pos = 0;
		dat = (unsigned char *)blastMemAlloc(compressed, FileSize - pos, FileSize);

		if (dat == NULL)
		{
			free(torelease);
			return;
		}

		free(torelease);
		compressed = NULL;
		pos = 0;
	}

	if (strcmp(dlh.ident, "DANAE_FILE"))
	{
		free(dat);
		sprintf(_error, "File %s is not a valid file", fic.c_str());
		return;
	}

	// Loading Scene
	if (dlh.nb_scn >= 1)
	{
		dls = (DANAE_LS_SCENE *)(dat + pos);
		pos += sizeof(DANAE_LS_SCENE);
	}

	for (i = 0; i < dlh.nb_inter; i++)
	{
		dli = (DANAE_LS_INTER *)(dat + pos);
		pos += sizeof(DANAE_LS_INTER);
		std::stringstream ss;
		ss << GetName(dli->name) << '_' << std::setfill('0') << std::setw(4) << dli->ident;
		string id = ss.str();
		AddIdent(id, num);
	}

	free(dat);
}

void ARX_SAVELOAD_CheckDLFs()
{
	ARX_SAVELOAD_DLFCheckInit();

	for (long n = 0; n < 24; n++)
	{
		char temp[256];
		sprintf(temp, "%ld\\Level%ld.dlf", n, n);
		ARX_SAVELOAD_DLFCheckAdd(temp, n);
	}

	for (int n = 0; n < dlfcount; n++)
	{
		char text[256];

		if (dlfcheck[n].occurence > 1)
		{
			sprintf(text, "Found %ld times : %s in levels %s", dlfcheck[n].occurence, dlfcheck[n].ident, dlfcheck[n].nums);
			LogError << (text);
		}
	}

	ARX_SAVELOAD_DLFCheckInit();
}
