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

#include "scene/LoadLevel.h"

#include <cstdio>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <vector>
#include <cassert>

#include "ai/PathFinderManager.h"
#include "ai/Paths.h"

#include "core/Time.h"
#include "core/Dialog.h"
#include "core/Localization.h"
#include "core/Core.h"

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

#include "platform/String.h"

#include "scene/Object.h"
#include "scene/Scene.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"
#include "scene/LevelFormat.h"
#include "scene/Light.h"

using std::max;
using std::string;

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

		for(size_t i = 0; i < MAX_LIGHTS; i++) {
			if(GLight[i]) {
				if(CanPurge(&GLight[i]->pos)) {
					// purge light
					EERIE_LIGHT_ClearByIndex(i);
					LIGHT_count++;
				}
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


long DanaeSaveLevel(const string & _fic) {
	
	long nb_inter = GetNumberInterWithOutScriptLoadForLevel(CURRENTLEVEL); // Without Player
	EERIE_BACKGROUND * eb = ACTIVEBKG;
	
	// Initializing datas
	HERMES_DATE_TIME hdt;
	GetDate(&hdt);
	char tx[128];
	sprintf(tx, "_%02ld_%02ld_%ld__%ldh%ldmn", hdt.months, hdt.days, hdt.years, hdt.hours, hdt.mins);
	
	string fic = _fic;
	SetExt(fic, ".DLF");
	if(FileExist(fic)) {
		std::string fic2 = fic;
		char newtext[128];
		sprintf(newtext, "Backup_DLF_%s", tx);
		SetExt(fic2, newtext);
		FileMove(fic, fic2.c_str());
	}

	std::string fic2 = fic;
	SetExt(fic2, ".LLF");
	if(FileExist(fic2)) {
		std::string fic3 = fic;
		char newtext[128];
		sprintf(newtext, "Backup_LLF_%s", tx);
		SetExt(fic3, newtext);
		FileMove(fic2, fic3);
	}
	
	DANAE_LS_HEADER dlh;
	memset(&dlh, 0, sizeof(DANAE_LS_HEADER));
	dlh.nb_nodes = CountNodes();
	assert(SAVED_MAX_LINKS == MAX_LINKS);
	dlh.nb_nodeslinks = SAVED_MAX_LINKS;
	dlh.nb_lights = 0; // MUST BE 0 !!!!
	dlh.nb_fogs = ARX_FOGS_Count();
	dlh.nb_bkgpolys = BKG_CountPolys(ACTIVEBKG);
	dlh.nb_childpolys = 0;
	dlh.nb_ignoredpolys = BKG_CountIgnoredPolys(ACTIVEBKG);
	dlh.nb_paths = nbARXpaths;
	
	long bcount = CountBkgVertex();
	size_t allocsize = sizeof(DANAE_LS_HEADER) + sizeof(DANAE_LS_HEADER) * 1 + sizeof(DANAE_LS_INTER) * nb_inter + 512
					 + sizeof(DANAE_LS_LIGHTINGHEADER) + (bcount + 1) * sizeof(u32)
					 + dlh.nb_nodes * (sizeof(DANAE_LS_NODE) + 64 * dlh.nb_nodeslinks)
					 + dlh.nb_lights * sizeof(DANAE_LS_LIGHT)

					 + 1000000
					 + nbARXpaths * sizeof(DANAE_LS_PATH) + nbARXpaths * sizeof(DANAE_LS_PATHWAYS) * 30;
	allocsize = max((size_t)dlh.nb_bkgpolys * (sizeof(u32) + 2) + 1000000, allocsize);
	
	char * dat = new char[allocsize];
	if(!dat) {
		LogError << "Unable to allocate Buffer for save...";
		return -1;
	}
	
	memset(dat, 0, allocsize);
	
	// Preparing HEADER
	dlh.version = DLH_CURRENT_VERSION;
	if(NODIRCREATION) {
		dlh.version = 1.004f;
	}
	
	strcpy(dlh.ident, "DANAE_FILE");
	dlh.nb_scn = 1;
	
	if(LastLoadedScene[0] == 0) {
		dlh.nb_scn = 0;
	}
	
	dlh.nb_inter = nb_inter;
	dlh.nb_zones = 0;
	
	if(dlh.nb_scn != 0) {
		// TODO operator overloading
		dlh.pos_edit.x = subj.pos.x - Mscenepos.x;
		dlh.pos_edit.y = subj.pos.y - Mscenepos.y;
		dlh.pos_edit.z = subj.pos.z - Mscenepos.z;
	} else {
		dlh.pos_edit = subj.pos;
	}

	dlh.angle_edit = player.angle;
	dlh.lighting = false; // MUST BE false !!!!
	
	// TODO w32 api
	// _time32(&dlh.time);
	dlh.time = 0;
	
	u32 siz = 255;
	GetUserName(dlh.lastuser, &siz);
	
	size_t pos = 0;
	
	memcpy(dat + pos, &dlh, sizeof(DANAE_LS_HEADER));
	pos += sizeof(DANAE_LS_HEADER);

	// Preparing SCENE DATA
	if(dlh.nb_scn > 0) {
		DANAE_LS_SCENE dls;
		memset(&dls, 0, sizeof(DANAE_LS_SCENE));
		strcpy(dls.name, LastLoadedScene);
		memcpy(dat + pos, &dls, sizeof(DANAE_LS_SCENE));
		pos += sizeof(DANAE_LS_SCENE);
	}
	
	// preparing INTER DATA, Ignoring Player Data
	for(long i = 1; i < inter.nbmax; i++) {
		if((inter.iobj[i] != NULL)  && (!inter.iobj[i]->scriptload)
			&& (inter.iobj[i]->truelevel == CURRENTLEVEL)) {
			
			INTERACTIVE_OBJ * io = inter.iobj[i];
			
			DANAE_LS_INTER dli;
			memset(&dli, 0, sizeof(DANAE_LS_INTER));
			
			if(dlh.nb_scn != 0) {
				dli.pos.x = io->initpos.x - Mscenepos.x;
				dli.pos.y = io->initpos.y - Mscenepos.y;
				dli.pos.z = io->initpos.z - Mscenepos.z;
			} else {
				dli.pos = io->initpos;
			}
			
			dli.angle = io->initangle;
			strcpy( dli.name, io->filename);
			
			if(io->ident == 0) {
				MakeIOIdent(io);
			}
			dli.ident = io->ident;
			
			if(io->ioflags & IO_FREEZESCRIPT) {
				dli.flags = IO_FREEZESCRIPT;
			}
			
			inter.iobj[i]->EditorFlags &= ~EFLAG_NOTSAVED;
			memcpy(dat + pos, &dli, sizeof(DANAE_LS_INTER));
			pos += sizeof(DANAE_LS_INTER);
		}
	}
	
	for(size_t i = 0; i < MAX_FOG; i++) {
		if(fogs[i].exist) {
			DANAE_LS_FOG dlf;
			memset(&dlf, 0, sizeof(DANAE_LS_FOG));
			dlf.rgb = fogs[i].rgb;
			dlf.angle = fogs[i].angle;
			dlf.pos.x = fogs[i].pos.x - Mscenepos.x;
			dlf.pos.y = fogs[i].pos.y - Mscenepos.y;
			dlf.pos.z = fogs[i].pos.z - Mscenepos.z;
			dlf.blend = fogs[i].blend;
			dlf.frequency = fogs[i].frequency;
			dlf.move = fogs[i].move;
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
	
	for(long i = 0; i < nodes.nbmax; i++) {
		if(nodes.nodes[i].exist) {
			
			DANAE_LS_NODE dln;
			memset(&dln, 0, sizeof(DANAE_LS_NODE));
			strcpy(dln.name, nodes.nodes[i].name);
			dln.pos.x = nodes.nodes[i].pos.x - Mscenepos.x;
			dln.pos.y = nodes.nodes[i].pos.y - Mscenepos.y;
			dln.pos.z = nodes.nodes[i].pos.z - Mscenepos.z;
			memcpy(dat + pos, &dln, sizeof(DANAE_LS_NODE));
			pos += sizeof(DANAE_LS_NODE);
			
			for(size_t j = 0; j < SAVED_MAX_LINKS; j++) {
				
				char name[64];
				memset(name, 0, 64);
				
				if(nodes.nodes[i].link[j] != -1 && nodes.nodes[nodes.nodes[i].link[j]].exist) {
						strcpy(name, nodes.nodes[nodes.nodes[i].link[j]].name);
				}
				
				memcpy(dat + pos, name, 64);
				pos += 64;
			}
		}
	}
	
	for(long i = 0; i < nbARXpaths; i++) {
		
		DANAE_LS_PATH dlp;
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
		dlp.rgb = ARXpaths[i]->rgb;
		
		memcpy(dat + pos, &dlp, sizeof(DANAE_LS_PATH));
		pos += sizeof(DANAE_LS_PATH);
		
		for(long j = 0; j < dlp.nb_pathways; j++) {
			
			DANAE_LS_PATHWAYS dlpw;
			memset(&dlpw, 0, sizeof(DANAE_LS_PATHWAYS));
			dlpw.flag = ARXpaths[i]->pathways[j].flag;
			dlpw.rpos = ARXpaths[i]->pathways[j].rpos;
			
			float fValue = ARXpaths[i]->pathways[j]._time ;
			ARX_CHECK_ULONG(fValue);
			dlpw.time = ARX_CLEAN_WARN_CAST_ULONG(fValue);
			
			memcpy(dat + pos, &dlpw, sizeof(DANAE_LS_PATHWAYS));
			pos += sizeof(DANAE_LS_PATHWAYS);
		}
	}
	
	//Saving Special Polys
	if(pos > allocsize) {
		LogError << "Badly Allocated SaveBuffer..." << fic;
		delete[] dat;
		return -1;
	}
	
	// Now Saving Whole Buffer
	FileHandle handle;
	if(!(handle = FileOpenWrite(fic))) {
		LogError << "Unable to Open " << fic << " for Write...";
		delete[] dat;
		return -1;
	}
	
	if(FileWrite(handle, dat, sizeof(DANAE_LS_HEADER)) != sizeof(DANAE_LS_HEADER)) {
		LogError << "Unable to Write to " << fic;
		delete[] dat;
		return -1;
	}

	size_t cpr_pos = 0;
	char * compressed  = implodeAlloc(dat + sizeof(DANAE_LS_HEADER), pos - sizeof(DANAE_LS_HEADER), cpr_pos);
	
	size_t sizeWritten;
	sizeWritten = FileWrite(handle, compressed, cpr_pos);
	
	delete[] compressed;
	FileClose(handle);
	
	if(sizeWritten != cpr_pos) {
		delete[] dat;
		return false;
	}
	
	
	//Now Save Separate LLF Lighting File
	
	pos = 0;
	
	DANAE_LLF_HEADER llh;
	memset(&llh, 0, sizeof(DANAE_LLF_HEADER));
	
	// Preparing HEADER
	llh.version = DLH_CURRENT_VERSION;
	llh.nb_lights = EERIE_LIGHT_Count();
	llh.nb_bkgpolys = BKG_CountPolys(ACTIVEBKG);
	strcpy(llh.ident, "DANAE_LLH_FILE");
	
	// todo w32api
	// _time32(&llh.time);
	
	u32 siz2 = 255;
	GetUserName(llh.lastuser, &siz2);
	
	memcpy(dat, &llh, sizeof(DANAE_LLF_HEADER));
	pos += sizeof(DANAE_LLF_HEADER);
	
	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		
		EERIE_LIGHT * el = GLight[i];
		
		if(!el || (el->type & TYP_SPECIAL1)) {
			continue;
		}
		
		DANAE_LS_LIGHT dlight;
		memset(&dlight, 0, sizeof(DANAE_LS_LIGHT));
		dlight.fallend = el->fallend;
		dlight.fallstart = el->fallstart;
		dlight.intensity = el->intensity;
		dlight.pos.x = el->pos.x - Mscenepos.x;
		dlight.pos.y = el->pos.y - Mscenepos.y;
		dlight.pos.z = el->pos.z - Mscenepos.z;
		dlight.rgb = el->rgb;
		
		dlight.extras = el->extras;
		dlight.ex_flicker = el->ex_flicker;
		dlight.ex_radius = el->ex_radius;
		dlight.ex_frequency = el->ex_frequency;
		dlight.ex_size = el->ex_size;
		dlight.ex_speed = el->ex_speed;
		dlight.ex_flaresize = el->ex_flaresize;
		
		memcpy(dat + pos, &dlight, sizeof(DANAE_LS_LIGHT));
		pos += sizeof(DANAE_LS_LIGHT);
	}
	
	//Saving Special Polys
	DANAE_LS_LIGHTINGHEADER dll;
	memset(&dll, 0, sizeof(DANAE_LS_LIGHTINGHEADER));
	dll.nb_values = bcount;
	dll.ModeLight = ModeLight;
	dll.ViewMode = ViewMode;
	memcpy(dat + pos, &dll, sizeof(DANAE_LS_LIGHTINGHEADER));
	pos += sizeof(DANAE_LS_LIGHTINGHEADER);
	
	for(long j = 0; j < eb->Zsize; j++) {
		for(long i = 0; i < eb->Xsize; i++) {
			
			EERIE_BKG_INFO * eg = (EERIE_BKG_INFO *)&eb->Backg[i + j*eb->Xsize];
			for(long l = 0; l < eg->nbpoly; l++) {
				
				EERIEPOLY * ep = &eg->polydata[l];
				if(ep) {
					long nbvert = (ep->type & POLY_QUAD) ? 4 : 3;
					for(long k = 0; k < nbvert; k++) {
						u32 tmp = (u32)ep->v[k].color;
						memcpy(dat + pos, &tmp, sizeof(u32));
						pos += sizeof(u32);
					}
				}
			}
		}
	}
	
	if(pos > allocsize) {
		LogError << "Badly Allocated SaveBuffer..." << fic2;
		delete[] dat;
		return -1;
	}
	
	// Now Saving Whole Buffer
	if(!(handle = FileOpenWrite(fic2))) {
		LogError << "Unable to Open " << fic2 << " for Write...";
		delete[] dat;
		return -1;
	}
	
	compressed = NULL;
	cpr_pos = 0;
	compressed = implodeAlloc(dat, pos, cpr_pos);
	delete[] dat;

	size_t sizeWritten2;
	sizeWritten2 = FileWrite(handle, compressed, cpr_pos);
	
	delete[] compressed;
	FileClose(handle);
	
	if(sizeWritten2 != cpr_pos) {
		LogError << "Unable to Write to " << fic2;
		return -1;
	}
	
	ADDED_IO_NOT_SAVED = 0;
	
	return 1;
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

	if (DirectoryExist(dir))
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


void LogDirCreation(const string & dir) {
	if(DirectoryExist(dir)) {
		LogDebug << "LogDirCreation: " << dir;
	}
}


static void LogDirDestruction(const string & dir ) {
	if(DirectoryExist(dir)) {
		LogDebug << "LogDirDestruction: " << dir;
	}
}

//*************************************************************************************
// Checks for IO created during this session but not saved...
//*************************************************************************************
void CheckIO_NOT_SAVED()
{
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
							std::string temp = inter.iobj[i]->full_name();

							if (DirectoryExist(temp))
							{
								std::string temp3 = "Really remove Directory & Directory Contents ?\n\n" + temp;

								if (OKBox(temp3, "WARNING"))
								{
									temp += "\\";
									LogDirDestruction(temp);
									KillAllDirectory(temp);
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
				temp = io->full_name() + "\\" + io->short_name() + ".asl"; // Looks odd, results in /path/human_0001/human.asl and so on

				if (DirectoryExist( io->full_name() ))
				{
//					TODO win32api
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

INTERACTIVE_OBJ * LoadInter_Ex(const string & name, long ident, const EERIE_3D & pos, const EERIE_3D & angle, const EERIE_3D & trans) {
	char nameident[256];
	string tmp;
	size_t FileSize;
	INTERACTIVE_OBJ * io;
	
	sprintf(nameident, "%s_%04ld", GetName(name).c_str(), ident);
	
	long t = GetTargetByNameTarget(nameident);
	if(t >= 0) {
		return inter.iobj[t];
	}
	
	char * nname = strdup(name.c_str()); // TODO use string
	ReplaceSpecifics(nname);
	
	io = AddInteractive(nname, ident, NO_MESH | NO_ON_LOAD);
	
	if (io)
	{
		RestoreInitialIOStatusOfIO(io);
		ARX_INTERACTIVE_HideGore(io);

		io->lastpos = io->initpos = io->pos = pos + trans; // RELATIVE !!!!!!!!!
		io->move.x = io->move.y = io->move.z = 0.f;
		io->initangle = io->angle = angle;

		if (!NODIRCREATION)
		{
			io->ident = ident;
			tmp = tmp = io->full_name(); // Get the directory name to check for

			if (PAK_DirectoryExist(tmp))
			{
				tmp += '\\' + io->short_name() + ".asl"; // Create the filename to be loaded

				if (PAK_FileExist(tmp))
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
			if(io->obj == NULL) {
				bool pbox = (io->ioflags & IO_ITEM) == IO_ITEM;
				io->obj = loadObject(io->filename, pbox);

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

long DanaeLoadLevel(const string & fic) {
	
	LogInfo << "Loading Level " << fic;
	
	ClearCurLoadInfo();
	CURRENTLEVEL = GetLevelNumByName(fic);
	
	HERMES_DATE_TIME hdt;
	GetDate(&hdt);
	char tstr[128];
	sprintf(tstr, "%2ldh%02ldm%02ld LOADLEVEL start", hdt.hours, hdt.mins, hdt.secs);
	ForceSendConsole(tstr, 1, 0, (HWND)1);
	
	string fileDlf = fic;
	// SetExt(fileDlf, ".DLF");
	string fic2 = fic;
	SetExt(fic2, ".LLF");

	LogDebug << "fic2 " << fic2;
	LogDebug << "fileDlf " << fileDlf;

	if(!PAK_FileExist(fileDlf)) {
		LogError << "Unable to find " << fileDlf;
		return -1;
	}
	
	strcpy(LastLoadedDLF, fic.c_str());
	
	size_t FileSize = 0;
	char * dat = (char*)PAK_FileLoadMalloc(fic, FileSize);
	PROGRESS_BAR_COUNT += 1.f;
	LoadLevelScreen();
	
	size_t pos = 0;
	
	DANAE_LS_HEADER dlh;
	memcpy(&dlh, dat + pos, sizeof(DANAE_LS_HEADER));
	pos += sizeof(DANAE_LS_HEADER);
	
	LogDebug << "dlh.version " << dlh.version << " header size " << sizeof(DANAE_LS_HEADER);
	
	if(dlh.version > DLH_CURRENT_VERSION) {
		LogError << "DANAE Version too OLD to load this File... Please upgrade to a new DANAE Version...";
		free(dat);
		dat = NULL;
		return -1;
	}
	
	// using compression
	if(dlh.version >= 1.44f) {
		char * torelease = dat;
		dat = blastMemAlloc(dat + pos, FileSize - pos, FileSize);
		free(torelease);
		pos = 0;
		if(!dat) {
			LogError << "STD_Explode did not return anything " << fileDlf;
			return -1;
		}
	}
	
	loddpos = subj.pos = dlh.pos_edit;
	player.desiredangle = player.angle = subj.angle = dlh.angle_edit;
	
	if(strcmp(dlh.ident, "DANAE_FILE")) {
		LogError << "Not a valid file "<< fileDlf;
		return -1;
	}
	
	if(dlh.version < 1.001f) {
		dlh.nb_nodes = 0;
	}
	
	LogDebug << "Loading Scene";
	
	// Loading Scene
	if(dlh.nb_scn >= 1) {
		
		DANAE_LS_SCENE * dls = (DANAE_LS_SCENE*)(dat + pos);
		pos += sizeof(DANAE_LS_SCENE);
		
		string ftemp;
		if(FAKE_DIR) {
			ftemp = fic;
			RemoveName(ftemp);
			FAKE_DIR = 0;
		} else {
			ftemp = dls->name;
			RemoveName(ftemp);
		}
		
		if(FastSceneLoad(ftemp)) {
			LogDebug << "done loading scene";
			FASTmse = 1;
		} else {
			LogDebug << "fast loading scene failed";
			ARX_SOUND_PlayCinematic("Editor_Humiliation.wav");
			mse = PAK_MultiSceneToEerie(ftemp);
			PROGRESS_BAR_COUNT += 20.f;
			LoadLevelScreen();
		}
		
		EERIEPOLY_Compute_PolyIn();
		strcpy(LastLoadedScene, ftemp.c_str());
	}
	
	EERIE_3D trans;
	if(FASTmse) {
		trans = Mscenepos;
		player.pos = loddpos + trans;
	} else if(mse != NULL) {
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
		mse->pos.y = Mscenepos.y = -mse->cub.ymin - 100.f - mse->point0.y;
		lastteleport.x = mapcam.pos.x = player.pos.x = subj.pos.x = moveto.x = mse->pos.x + mse->point0.x;
		lastteleport.z = mapcam.pos.z = player.pos.z = subj.pos.z = moveto.z = mse->pos.z + mse->point0.z;
		lastteleport.y                = player.pos.y = subj.pos.y = moveto.y = mse->pos.y + mse->point0.y;
		lastteleport.y -= 180.f;
		player.pos.y = subj.pos.y -= 180.f;
		trans = mse->pos;
	} else {
		lastteleport.x = 0.f;
		lastteleport.y = PLAYER_BASE_HEIGHT;
		lastteleport.z = 0.f;
		trans.x = 0;
		trans.y = 0;
		trans.z = 0;
		Mscenepos = trans;
	}
	
	MSP = trans;
	
	ClearCurLoadInfo();
	
	float increment = 0;
	if(dlh.nb_inter > 0) {
		increment = (60.f / (float)dlh.nb_inter);
	} else {
		PROGRESS_BAR_COUNT += 60;
		LoadLevelScreen();
	}
	
	for(long i = 0 ; i < dlh.nb_inter ; i++) {
		
		PROGRESS_BAR_COUNT += increment;
		LoadLevelScreen();
		
		DANAE_LS_INTER * dli = (DANAE_LS_INTER *)(dat + pos);
		pos += sizeof(DANAE_LS_INTER);
		if(!DONT_LOAD_INTERS) {
			LoadInter_Ex(dli->name, dli->ident, dli->pos, dli->angle, trans);
		}
	}
	
	if(dlh.lighting) {
		
		DANAE_LS_LIGHTINGHEADER * dll = (DANAE_LS_LIGHTINGHEADER *)(dat + pos);
		pos += sizeof(DANAE_LS_LIGHTINGHEADER);
		long bcount = dll->nb_values;
		
		if(!PAK_FileExist(fic2)) {
			
			LastLoadedLightningNb = bcount;
			if(LastLoadedLightning != NULL) {
				free(LastLoadedLightning);
				LastLoadedLightning = NULL;
			}
			
			//DANAE_LS_VLIGHTING
			D3DCOLOR * ll = LastLoadedLightning = (D3DCOLOR *)malloc(sizeof(D3DCOLOR) * bcount);
			
			if(dlh.version > 1.001f) {
				std::copy((u32*)(dat + pos), (u32*)(dat + pos) + bcount, LastLoadedLightning);
				pos += sizeof(u32) * bcount;
			} else {
				while(bcount) {
					DANAE_LS_VLIGHTING dlv;
					memcpy(&dlv, dat + pos, sizeof(DANAE_LS_VLIGHTING));
					pos += sizeof(DANAE_LS_VLIGHTING);
					*ll = 0xff000000L | ((dlv.r & 255) << 16) | ((dlv.g & 255) << 8) | (dlv.b & 255);
					ll++;
					bcount--;
				}
			}
			
		} else {
			pos += sizeof(u32) * bcount;
		}
		
		ModeLight = Flag(dll->ModeLight); // TODO save/load flags
		ViewMode = Flag(dll->ViewMode); // TODO save/load flags
		ViewMode &= ~VIEWMODE_WIRE;
	}
	
	PROGRESS_BAR_COUNT += 1;
	LoadLevelScreen();
	
	if(dlh.version < 1.003f) {
		dlh.nb_lights = 0;
	}
	
	if(!PAK_FileExist(fic2)) {
		
		if(dlh.nb_lights != 0) {
			EERIE_LIGHT_GlobalInit();
		}
		
		for(long i = 0; i < dlh.nb_lights; i++) {
			
			DANAE_LS_LIGHT * dlight = (DANAE_LS_LIGHT*)(dat + pos);
			pos += sizeof(DANAE_LS_LIGHT);
			
			long j = EERIE_LIGHT_Create();
			if(j >= 0) {
				EERIE_LIGHT * el = GLight[j];
				
				el->exist = 1;
				el->treat = 1;
				el->fallend = dlight->fallend;
				el->fallstart = dlight->fallstart;
				el->falldiff = el->fallend - el->fallstart;
				el->falldiffmul = 1.f / el->falldiff;
				el->intensity = dlight->intensity;
				el->pos = dlight->pos;
				el->rgb = dlight->rgb;
				
				ARX_CHECK_SHORT(dlight->extras);
				el->extras = static_cast<short>(dlight->extras);
				
				el->ex_flicker = dlight->ex_flicker;
				el->ex_radius = dlight->ex_radius;
				el->ex_frequency = dlight->ex_frequency;
				el->ex_size = dlight->ex_size;
				el->ex_speed = dlight->ex_speed;
				el->tl = -1;
				el->sample = ARX_SOUND_INVALID_RESOURCE;
				
				if((el->extras & EXTRAS_SPAWNFIRE)) {
					el->extras |= EXTRAS_FLARE;
					if(el->extras & EXTRAS_FIREPLACE) {
						el->ex_flaresize = 95.f;
					} else {
						el->ex_flaresize = 40.f;
					}
				}
			}
		}
		
	} else {
		pos += sizeof(DANAE_LS_LIGHT) * dlh.nb_lights;
	}
	
	ClearCurLoadInfo();
	LogDebug << "Loading FOGS";
	ARX_FOGS_Clear();
	
	for(long i = 0; i < dlh.nb_fogs; i++) {
		
		DANAE_LS_FOG * dlf = (DANAE_LS_FOG *)(dat + pos);
		pos += sizeof(DANAE_LS_FOG);
		
		long n = ARX_FOGS_GetFree();
		if(n > -1) {
			
			FOG_DEF * fd = &fogs[n];
			fd->exist = 1;
			fd->rgb = dlf->rgb;
			fd->angle = dlf->angle;
			fd->pos.x = dlf->pos.x + trans.x;
			fd->pos.y = dlf->pos.y + trans.y;
			fd->pos.z = dlf->pos.z + trans.z;
			fd->blend = dlf->blend;
			fd->frequency = dlf->frequency;
			fd->rotatespeed = dlf->rotatespeed;
			fd->scale = dlf->scale;
			fd->size = dlf->size;
			fd->special = dlf->special;
			fd->speed = dlf->speed;
			fd->tolive = dlf->tolive;
			fd->move.x = 1.f;
			fd->move.y = 0.f;
			fd->move.z = 0.f;
			EERIE_3D out;
			float ta = radians(MAKEANGLE(fd->angle.b));
			_YRotatePoint(&fd->move, &out, EEcos(ta), EEsin(ta));
			float tb = radians(MAKEANGLE(fd->angle.a));
			_XRotatePoint(&out, &fd->move, EEcos(tb), EEsin(tb));
		}
	}
	
	PROGRESS_BAR_COUNT += 2.f;
	LoadLevelScreen();
	
	ClearCurLoadInfo();
	LogDebug << "Loading Nodes";
	ClearNodes();
	
	for(long i = 0; i < dlh.nb_nodes; i++) {
		
		nodes.nodes[i].exist = 1;
		nodes.nodes[i].selected = 0;
		DANAE_LS_NODE * dln = (DANAE_LS_NODE*)(dat + pos);
		pos += sizeof(DANAE_LS_NODE);
		
		strcpy(nodes.nodes[i].name, dln->name);
		nodes.nodes[i].pos.x = dln->pos.x + trans.x;
		nodes.nodes[i].pos.y = dln->pos.y + trans.y;
		nodes.nodes[i].pos.z = dln->pos.z + trans.z;
		
		for(long j = 0; j < dlh.nb_nodeslinks; j++) {
			char name[64];
			memcpy(name, dat + pos, 64);
			pos += 64;
			if(name[0] != 0) {
				strcpy(nodes.nodes[i].lnames[j], name);
			}
		}
	}
	
	RestoreNodeNumbers();
	
	ClearCurLoadInfo();
	LogDebug << "Loading Paths";
	ARX_PATH_ReleaseAllPath();
	
	if(dlh.nb_paths) {
		ARXpaths = (ARX_PATH **)malloc(sizeof(ARX_PATH *) * dlh.nb_paths);
		nbARXpaths = dlh.nb_paths;
	}
	
	for(long i = 0; i < dlh.nb_paths; i++) {
		
		ARX_PATH * ap = ARXpaths[i] = (ARX_PATH *)malloc(sizeof(ARX_PATH));
		memset(ap, 0, sizeof(ARX_PATH));
		
		DANAE_LS_PATH * dlp = (DANAE_LS_PATH *)(dat + pos);
		pos += sizeof(DANAE_LS_PATH);
		
		ap->flags = Flag(dlp->flags); // TODO save/load flags
		ap->idx = dlp->idx;
		ap->initpos.x = dlp->initpos.x + trans.x;
		ap->initpos.y = dlp->initpos.y + trans.y;
		ap->initpos.z = dlp->initpos.z + trans.z;
		ap->pos.x = dlp->pos.x + trans.x;
		ap->pos.y = dlp->pos.y + trans.y;
		ap->pos.z = dlp->pos.z + trans.z;
		strcpy(ap->name, dlp->name);
		ap->nb_pathways = dlp->nb_pathways;
		ap->height = dlp->height;
		strcpy(ap->ambiance, dlp->ambiance);
		ap->amb_max_vol = dlp->amb_max_vol;
		
		if(ap->amb_max_vol <= 1.f) {
			ap->amb_max_vol = 100.f;
		}
		
		ap->farclip = dlp->farclip;
		ap->reverb = dlp->reverb;
		ap->rgb = dlp->rgb;
		
		ARX_PATHWAY * app = ap->pathways = (ARX_PATHWAY *)malloc(sizeof(ARX_PATHWAY) * dlp->nb_pathways);
		memset(app, 0, sizeof(ARX_PATHWAY)*dlp->nb_pathways);
		
		for(long j = 0; j < dlp->nb_pathways; j++) {
			
			DANAE_LS_PATHWAYS  * dlpw = (DANAE_LS_PATHWAYS *)(dat + pos);
			pos += sizeof(DANAE_LS_PATHWAYS);
			
			app[j].flag = (PathwayType)dlpw->flag; // save/load enum
			app[j].rpos = dlpw->rpos;
			app[j]._time = static_cast<float>(dlpw->time);
		}
	}
	
	ARX_PATH_ComputeAllBoundingBoxes();
	PROGRESS_BAR_COUNT += 5.f;
	LoadLevelScreen();
	
	
	//Now LOAD Separate LLF Lighting File
	
	free(dat);
	pos = 0;
	dat = NULL;
	
	if(PAK_FileExist(fic2)) {
		
		ClearCurLoadInfo();
		LogDebug << "Loading LLF Info";
		
		// using compression
		if(dlh.version >= 1.44f) {
			size_t size;
			char * compressed = (char *)PAK_FileLoadMalloc(fic2, size);
			dat = (char*)blastMemAlloc(compressed, size, FileSize);
			free(compressed);
		} else {
			dat = (char*)PAK_FileLoadMalloc(fic2, FileSize);
		}
	}
	// TODO size ignored
	
	if(!dat) {
		LOADEDD = 1;
		FASTmse = 0;
		USE_PLAYERCOLLISIONS = 1;
		LogInfo << "Done loading level";
		return 1;
	}
	
	DANAE_LLF_HEADER * llh = (DANAE_LLF_HEADER *)(dat);
	pos += sizeof(DANAE_LLF_HEADER);
	
	PROGRESS_BAR_COUNT += 4.f;
	LoadLevelScreen();
	
	if(llh->nb_lights != 0) {
		EERIE_LIGHT_GlobalInit();
	}
	
	for(int i = 0; i < llh->nb_lights; i++) {
		
		DANAE_LS_LIGHT * dlight = (DANAE_LS_LIGHT *)(dat + pos);
		pos += sizeof(DANAE_LS_LIGHT);
		
		long j = EERIE_LIGHT_Create();
		if(j >= 0) {
			EERIE_LIGHT * el = GLight[j];
			
			el->exist = 1;
			el->treat = 1;
			el->fallend = dlight->fallend;
			el->fallstart = dlight->fallstart;
			el->falldiff = el->fallend - el->fallstart;
			el->falldiffmul = 1.f / el->falldiff;
			el->intensity = dlight->intensity;
			
			if(FASTmse) {
				el->pos.x = dlight->pos.x + trans.x;
				el->pos.y = dlight->pos.y + trans.y;
				el->pos.z = dlight->pos.z + trans.z;
			} else {
				el->pos = dlight->pos;
			}
			
			el->rgb = dlight->rgb;
			
			ARX_CHECK_SHORT(dlight->extras);
			el->extras = static_cast<short>(dlight->extras);
			
			el->ex_flicker = dlight->ex_flicker;
			el->ex_radius = dlight->ex_radius;
			el->ex_frequency = dlight->ex_frequency;
			el->ex_size = dlight->ex_size;
			el->ex_speed = dlight->ex_speed;
			el->ex_flaresize = dlight->ex_flaresize;
			
			el->status = (el->extras & EXTRAS_STARTEXTINGUISHED) ? 0 : 1;
			
			if((el->extras & EXTRAS_SPAWNFIRE) && (!(el->extras & EXTRAS_FLARE))) {
				el->extras |= EXTRAS_FLARE;
				if(el->extras & EXTRAS_FIREPLACE) {
					el->ex_flaresize = 95.f;
				} else {
					el->ex_flaresize = 80.f;
				}
			}
			
			el->tl = -1;
			el->sample = ARX_SOUND_INVALID_RESOURCE;
		}
	}
	
	PROGRESS_BAR_COUNT += 2.f;
	LoadLevelScreen();
	
	DANAE_LS_LIGHTINGHEADER * dll = (DANAE_LS_LIGHTINGHEADER *)(dat + pos);
	pos += sizeof(DANAE_LS_LIGHTINGHEADER);
	
	long bcount = dll->nb_values;
	LastLoadedLightningNb = bcount;
	if(LastLoadedLightning != NULL) {
		free(LastLoadedLightning);
		LastLoadedLightning = NULL;
	}
	
	//DANAE_LS_VLIGHTING
	D3DCOLOR * ll;
	ll = LastLoadedLightning = (D3DCOLOR *)malloc(sizeof(D3DCOLOR) * bcount);
	if(dlh.version > 1.001f) {
		std::copy((u32*)(dat + pos), (u32*)(dat + pos) + bcount, LastLoadedLightning);
		pos += sizeof(D3DCOLOR) * bcount;
	} else {
		while(bcount) {
			DANAE_LS_VLIGHTING dlv;
			memcpy(&dlv, dat + pos, sizeof(DANAE_LS_VLIGHTING));
			pos += sizeof(DANAE_LS_VLIGHTING);
			*ll = 0xff000000L | ((dlv.r & 255) << 16) | ((dlv.g & 255) << 8) | (dlv.b & 255);
			ll++;
			bcount--;
		}
	}
	
	ModeLight = Flag(dll->ModeLight); // TODO save/load flags
	ViewMode = Flag(dll->ViewMode); // TODO save/load flags
	ViewMode &= ~VIEWMODE_WIRE;
	
	free(dat);
	
	PROGRESS_BAR_COUNT += 1.f;
	LoadLevelScreen();
	
	LOADEDD = 1;
	FASTmse = 0;
	USE_PLAYERCOLLISIONS = 1;
	
	LogInfo << "Done loading level";
	
	return 1;
	
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

void ReleaseAllSpellResources() {
	
	smissile_count = 0;
	if(smissile) {
		delete smissile;
		smissile = NULL;
	}
	
	stite_count = 0;
	if(stite) {
		delete stite;
		stite = NULL;
	}
	
	smotte_count = 0;
	if(smotte) {
		delete smotte;
		smotte = NULL;
	}
	
	ssol_count = 0;
	if(ssol) {
		delete ssol;
		ssol = NULL;
	}
	
	slight_count = 0;
	if(slight) {
		delete slight;
		slight = NULL;
	}
	
	srune_count = 0;
	if(srune) {
		delete srune;
		srune = NULL;
	}
	
	svoodoo_count--;
	if(svoodoo) {
		delete svoodoo;
		svoodoo = NULL;
	}
	
	stone0_count = 0;
	if(stone0) {
		delete stone0;
		stone0 = NULL;
	}
	
	stone1_count = 0;
	if(stone1) {
		delete stone1;
		stone1 = NULL;
	}
	
	spapi_count = 0;
	if(spapi) {
		delete spapi;
		spapi = NULL;
	}
	
	sfirewave_count = 0;
	if(sfirewave) {
		delete sfirewave;
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

	for(size_t i = 0; i < MAX_DYNLIGHTS; i++) {
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

	bcount = LastLoadedLightningNb;

	EERIEPOLY * ep;
	long nbvert;

	for (long j = 0; j < eb->Zsize; j++)
		for (long i = 0; i < eb->Xsize; i++)
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

	for (size_t i = 0; i < MAX_ACTIONS; i++)
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
				case ACT_FIREOFF:
				case ACT_FIRE2OFF:
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
		if (!strcasecmp(dlfcheck[n].ident, ident))
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

static void ARX_SAVELOAD_DLFCheckAdd(char * path, long num) {
	
	std::string fic("Graph\\Levels\\Level");
	fic += path;

	char _error[512];
	DANAE_LS_HEADER				dlh;
	DANAE_LS_INTER		*		dli;
	unsigned char * dat = NULL;

	long pos = 0;
	long i;
	size_t FileSize = 0;

	SetExt(fic, ".DLF");

	if (!PAK_FileExist(fic))
	{
		return;
	}

	dat = (unsigned char *)PAK_FileLoadMalloc(fic, FileSize);
	memcpy(&dlh, dat, sizeof(DANAE_LS_HEADER));
	pos += sizeof(DANAE_LS_HEADER);

	if (dlh.version > DLH_CURRENT_VERSION) // using compression
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
