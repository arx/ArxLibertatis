/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#include "scene/LoadLevel.h"

#include <cstdio>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "ai/PathFinderManager.h"
#include "ai/Paths.h"

#include "core/Application.h"
#include "core/GameTime.h"
#include "core/Core.h"

#include "game/Levels.h"
#include "game/Player.h"

#include "gui/MiniMap.h"
#include "gui/Interface.h"

#include "graphics/Math.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/Fog.h"
#include "graphics/particle/ParticleEffects.h"

#include "io/FilePath.h"
#include "io/FileStream.h"
#include "io/PakReader.h"
#include "io/Filesystem.h"
#include "io/Blast.h"
#include "io/Implode.h"
#include "io/log/Logger.h"

#include "physics/CollisionShapes.h"

#include "platform/String.h"

#include "scene/Object.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"
#include "scene/LevelFormat.h"
#include "scene/Light.h"

using std::max;
using std::string;

extern float PROGRESS_BAR_COUNT;
extern float PROGRESS_BAR_TOTAL;
extern long DONT_ERASE_PLAYER;

extern QUAKE_FX_STRUCT QuakeFx;
extern bool bGToggleCombatModeWithKey;
extern bool bGCroucheToggle;

bool CanPurge(Vec3f * pos)
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

#ifdef BUILD_EDIT_LOADSAVE

void LogDirCreation(const fs::path & dir) {
	if(fs::is_directory(dir)) {
		LogDebug("LogDirCreation: " << dir);
	}
}

long DanaeSaveLevel(const fs::path & _fic) {
	
	long nb_inter = GetNumberInterWithOutScriptLoadForLevel(CURRENTLEVEL); // Without Player
	EERIE_BACKGROUND * eb = ACTIVEBKG;
	
	fs::path fic = _fic;
	fic.set_ext("dlf");
	if(fs::exists(fic)) {
		fs::path backupfile = fic;
		int i = 0;
		do {
			std::stringstream s;
			s << "dlf_bak_" << i;
			backupfile.set_ext(s.str());
		} while(fs::exists(backupfile) && (i++, true));
		
		if(!fs::rename(fic, backupfile)) {
			LogError << "Unable to rename file " << fic << " to " << backupfile;
			return -1;
		}
	}
	
	fs::path fic2 = fic;
	fic2.set_ext("llf");
	if(fs::exists(fic2)) {
		fs::path backupfile = fic;
		int i = 0;
		do {
			std::stringstream s;
			s << "llf_bak_" << i;
			backupfile.set_ext(s.str());
		} while(fs::exists(backupfile) && (i++, true));
		
		if(!fs::rename(fic2, backupfile)) {
			LogError << "Unable to rename file " << fic2 << " to " << backupfile;
			return -1;
		}
	}
	
	DANAE_LS_HEADER dlh;
	memset(&dlh, 0, sizeof(DANAE_LS_HEADER));
	dlh.nb_nodes = CountNodes();
	BOOST_STATIC_ASSERT(SAVED_MAX_LINKS == MAX_LINKS);
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
#ifdef BUILD_EDITOR
	if(NODIRCREATION) {
		dlh.version = 1.004f;
	}
#endif
	
	strcpy(dlh.ident, "DANAE_FILE");
	
	dlh.nb_scn = LastLoadedScene.empty() ? 0 : 1;
	
	dlh.nb_inter = nb_inter;
	dlh.nb_zones = 0;
	
	dlh.pos_edit = (dlh.nb_scn != 0) ? subj.pos - Mscenepos : subj.pos;

	dlh.angle_edit = player.angle;
	dlh.lighting = false; // must be false
	
	dlh.time = std::time(NULL);
	
	memset(dlh.lastuser, 0, sizeof(dlh.lastuser));
	
	size_t pos = 0;
	
	memcpy(dat + pos, &dlh, sizeof(DANAE_LS_HEADER));
	pos += sizeof(DANAE_LS_HEADER);

	// Preparing SCENE DATA
	if(dlh.nb_scn > 0) {
		DANAE_LS_SCENE dls;
		memset(&dls, 0, sizeof(DANAE_LS_SCENE));
		strncpy(dls.name, LastLoadedScene.string().c_str(), sizeof(dls.name));
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
			strncpy(dli.name, io->filename.string().c_str(), sizeof(dli.name));
			
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
		dlp.idx = 0;
		dlp.initpos.x = ARXpaths[i]->initpos.x - Mscenepos.x;
		dlp.initpos.y = ARXpaths[i]->initpos.y - Mscenepos.y;
		dlp.initpos.z = ARXpaths[i]->initpos.z - Mscenepos.z;
		dlp.pos.x = ARXpaths[i]->pos.x - Mscenepos.x;
		dlp.pos.y = ARXpaths[i]->pos.y - Mscenepos.y;
		dlp.pos.z = ARXpaths[i]->pos.z - Mscenepos.z;
		strncpy(dlp.name, ARXpaths[i]->name.c_str(), sizeof(dlp.name));
		dlp.nb_pathways = ARXpaths[i]->nb_pathways;
		dlp.height = ARXpaths[i]->height;
		strncpy(dlp.ambiance, ARXpaths[i]->ambiance.string().c_str(), sizeof(dlp.ambiance));
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
			dlpw.time = checked_range_cast<u32>(fValue);
			
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
	fs::ofstream ofs(fic, fs::fstream::out | fs::fstream::binary | fs::fstream::trunc);
	if(!ofs.is_open()) {
		LogError << "Unable to open " << fic << " for write...";
		delete[] dat;
		return -1;
	}
	
	if(ofs.write(dat, sizeof(DANAE_LS_HEADER)).fail()) {
		LogError << "Unable to write to " << fic;
		delete[] dat;
		return -1;
	}

	size_t cpr_pos = 0;
	char * compressed  = implodeAlloc(dat + sizeof(DANAE_LS_HEADER), pos - sizeof(DANAE_LS_HEADER), cpr_pos);
	
	ofs.write(compressed, cpr_pos);
	
	delete[] compressed;
	
	if(ofs.fail()) {
		delete[] dat;
		return false;
	}
	
	ofs.close();
	
	//Now Save Separate LLF Lighting File
	
	pos = 0;
	
	DANAE_LLF_HEADER llh;
	memset(&llh, 0, sizeof(DANAE_LLF_HEADER));
	
	// Preparing HEADER
	llh.version = DLH_CURRENT_VERSION;
	llh.nb_lights = EERIE_LIGHT_Count();
	llh.nb_bkgpolys = BKG_CountPolys(ACTIVEBKG);
	strcpy(llh.ident, "DANAE_LLH_FILE");
	
	llh.time = std::time(NULL);
	
	memset(llh.lastuser, 0, sizeof(llh.lastuser));
	
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
		LogError << "Badly allocated save buffer..." << fic2;
		delete[] dat;
		return -1;
	}
	
	// Now Saving Whole Buffer
	ofs.open(fic2, fs::fstream::out | fs::fstream::binary | fs::fstream::trunc);
	if(!ofs.is_open()) {
		LogError << "Unable to open " << fic2 << " for write...";
		delete[] dat;
		return -1;
	}
	
	compressed = NULL;
	cpr_pos = 0;
	compressed = implodeAlloc(dat, pos, cpr_pos);
	delete[] dat;

	ofs.write(compressed, cpr_pos);
	
	delete[] compressed;
	
	if(ofs.fail()) {
		LogError << "Unable to Write to " << fic2;
		return -1;
	}
	
	ADDED_IO_NOT_SAVED = 0;
	
	return 1;
}

void WriteIOInfo(INTERACTIVE_OBJ * io, const fs::path & dir) {
	
	if(!fs::is_directory(dir)) {
		return;
	}
	
	fs::path file = (dir / io->short_name()).set_ext("log");
	
	fs::ofstream ofs(file, fs::fstream::out | fs::fstream::trunc);
	if(!ofs.is_open()) {
		return;
	}
	
	ofs << "Object   : " << io->long_name() << std::endl;
	ofs << "_______________________________" << std::endl << std::endl;
	ofs << "Level    : " << LastLoadedScene << std::endl;
	ofs << "Position : x " << (io->initpos.x - Mscenepos.x)
	              << " y " << (io->initpos.y - Mscenepos.y)
	              << " z " << (io->initpos.z - Mscenepos.z) << " (relative to anchor)" << std::endl;
	
}

void SaveIOScript(INTERACTIVE_OBJ * io, long fl) {
	
	fs::path file;
	EERIE_SCRIPT * script;
	
	switch(fl) {
		
		case 1: { // class script
			file = io->filename;
			script = &io->script;
			break;
		}
		
		case 2: { // local script
			if(io->ident == 0) {
				LogError << ("NO IDENT...");
				return;
			}
			file = io->full_name();
			if(!fs::is_directory(file)) {
				LogError << "Local DIR don't Exists...";
				return;
			}
			file /= io->short_name();
			script = &io->over_script;
			break;
		}
		
		default: return;
	}
	
	fs::ofstream ofs(file, fs::fstream::out | fs::fstream::trunc | fs::fstream::binary);
	if(!ofs.is_open()) {
		LogError << ("Unable To Save...");
		return;
	}
	
	ofs.write(script->data, script->size);
	
	ARX_SCRIPT_ComputeShortcuts(*script);
}

#endif // BUILD_EDIT_LOADSAVE


INTERACTIVE_OBJ * LoadInter_Ex(const fs::path & name, long ident, const Vec3f & pos, const Anglef & angle, const Vec3f & trans) {
	
	std::ostringstream nameident;
	nameident << name.basename() << std::setfill('0') << std::setw(4) << ident;
	
	long t = inter.getById(nameident.str());
	if(t >= 0) {
		return inter.iobj[t];
	}
	
	INTERACTIVE_OBJ * io = AddInteractive(name, ident, NO_MESH | NO_ON_LOAD);
	if(!io) {
		return NULL;
	}
	
	RestoreInitialIOStatusOfIO(io);
	ARX_INTERACTIVE_HideGore(io);
	
	io->lastpos = io->initpos = io->pos = pos + trans;
	io->move.x = io->move.y = io->move.z = 0.f;
	io->initangle = io->angle = angle;
	
	fs::path tmp = io->full_name(); // Get the directory name to check for
	string id = io->short_name();
	if(PakDirectory * dir = resources->getDirectory(tmp)) {
		if(PakFile * file = dir->getFile(id + ".asl")) {
			loadScript(io->over_script, file);
			io->over_script.master = (io->script.data != NULL) ? &io->script : NULL;
		}
	}
	
	if(SendIOScriptEvent(io, SM_LOAD) == ACCEPT && io->obj == NULL) {
		bool pbox = (io->ioflags & IO_ITEM) == IO_ITEM;
		io->obj = loadObject(io->filename, pbox);
		if(io->ioflags & IO_NPC) {
			EERIE_COLLISION_Cylinder_Create(io);
		}
	}
	
	return io;
}

long LastLoadedLightningNb = 0;
u32 * LastLoadedLightning = NULL;
Vec3f loddpos;
Vec3f MSP;

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

long DanaeLoadLevel(const fs::path & file) {
	
	LogInfo << "Loading Level " << file;
	
	ClearCurLoadInfo();
	CURRENTLEVEL = GetLevelNumByName(file.string());
	
	fs::path lightingFileName = fs::path(file).set_ext("llf");

	LogDebug("fic2 " << lightingFileName);
	LogDebug("fileDlf " << file);

	size_t FileSize = 0;
	char * dat = resources->readAlloc(file, FileSize);
	if(!dat) {
		LogError << "Unable to find " << file;
		return -1;
	}
	
	PakFile * lightingFile = resources->getFile(lightingFileName);
	
	PROGRESS_BAR_COUNT += 1.f;
	LoadLevelScreen();
	
	size_t pos = 0;
	
	DANAE_LS_HEADER dlh;
	memcpy(&dlh, dat + pos, sizeof(DANAE_LS_HEADER));
	pos += sizeof(DANAE_LS_HEADER);
	
	LogDebug("dlh.version " << dlh.version << " header size " << sizeof(DANAE_LS_HEADER));
	
	if(dlh.version > DLH_CURRENT_VERSION) {
		LogError << "Unexpected level file version: " << dlh.version << " for " << file;
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
			LogError << "STD_Explode did not return anything " << file;
			return -1;
		}
	}
	
	loddpos = subj.pos = dlh.pos_edit;
	player.desiredangle = player.angle = subj.angle = dlh.angle_edit;
	
	if(strcmp(dlh.ident, "DANAE_FILE")) {
		LogError << "Not a valid file " << file << ": \"" << safestring(dlh.ident) << '"';
		return -1;
	}
	
	LogDebug("Loading Scene");
	
	// Loading Scene
	if(dlh.nb_scn > 0) {
		
		const DANAE_LS_SCENE * dls = reinterpret_cast<const DANAE_LS_SCENE *>(dat + pos);
		pos += sizeof(DANAE_LS_SCENE);
		
		fs::path scene = (FAKE_DIR) ? file.parent() : fs::path::load(safestring(dls->name));
		FAKE_DIR = 0;
		
		if(FastSceneLoad(scene)) {
			LogDebug("done loading scene");
			FASTmse = 1;
		} else {
#ifdef BUILD_EDIT_LOADSAVE
			LogDebug("fast loading scene failed");
			ARX_SOUND_PlayCinematic("editor_humiliation.wav");
			mse = PAK_MultiSceneToEerie(scene);
			PROGRESS_BAR_COUNT += 20.f;
			LoadLevelScreen();
#else
			LogError << "fast loading scene failed";
#endif
		}
		
		EERIEPOLY_Compute_PolyIn();
		LastLoadedScene = scene;
	}
	
	Vec3f trans;
	if(FASTmse) {
		trans = Mscenepos;
		player.pos = loddpos + trans;
	}
#ifdef BUILD_EDIT_LOADSAVE
	else if(mse != NULL) {
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
	}
#endif // BUILD_EDIT_LOADSAVE
	else
	{
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
		
		const DANAE_LS_INTER * dli = reinterpret_cast<const DANAE_LS_INTER *>(dat + pos);
		pos += sizeof(DANAE_LS_INTER);
		if(!DONT_LOAD_INTERS) {
			
			string pathstr = toLowercase(safestring(dli->name));
			
			size_t pos = pathstr.find("graph");
			if(pos != std::string::npos) {
				pathstr = pathstr.substr(pos);
			}
			
			LoadInter_Ex(fs::path::load(pathstr), dli->ident, dli->pos, dli->angle, trans);
		}
	}
	
	if(dlh.lighting) {
		
		const DANAE_LS_LIGHTINGHEADER * dll = reinterpret_cast<const DANAE_LS_LIGHTINGHEADER *>(dat + pos);
		pos += sizeof(DANAE_LS_LIGHTINGHEADER);
		long bcount = dll->nb_values;
		
		if(!lightingFile) {
			
			LastLoadedLightningNb = bcount;
			if(LastLoadedLightning != NULL) {
				free(LastLoadedLightning);
				LastLoadedLightning = NULL;
			}
			
			//DANAE_LS_VLIGHTING
			u32 * ll = LastLoadedLightning = (u32 *)malloc(sizeof(u32) * bcount);
			
			if(dlh.version > 1.001f) {
				std::copy((u32*)(dat + pos), (u32*)(dat + pos) + bcount, LastLoadedLightning);
				pos += sizeof(u32) * bcount;
			} else {
				while(bcount) {
					const DANAE_LS_VLIGHTING * dlv = reinterpret_cast<const DANAE_LS_VLIGHTING *>(dat + pos);
					pos += sizeof(DANAE_LS_VLIGHTING);
					*ll = 0xff000000L | ((dlv->r & 255) << 16) | ((dlv->g & 255) << 8) | (dlv->b & 255);
					ll++;
					bcount--;
				}
			}
			
		} else {
			pos += sizeof(u32) * bcount;
		}
		
		ModeLight = LightMode::load(dll->ModeLight); // TODO save/load flags
		ViewMode = ViewModeFlags::load(dll->ViewMode); // TODO save/load flags
		ViewMode &= ~VIEWMODE_WIRE;
	}
	
	PROGRESS_BAR_COUNT += 1;
	LoadLevelScreen();
	
	long nb_lights = (dlh.version < 1.003f) ? 0 : dlh.nb_lights;
	
	if(!lightingFile) {
		
		if(nb_lights != 0) {
			EERIE_LIGHT_GlobalInit();
		}
		
		for(long i = 0; i < nb_lights; i++) {
			
			const DANAE_LS_LIGHT * dlight = reinterpret_cast<const DANAE_LS_LIGHT *>(dat + pos);
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
				
				el->extras = checked_range_cast<short>(dlight->extras);
				
				el->ex_flicker = dlight->ex_flicker;
				el->ex_radius = dlight->ex_radius;
				el->ex_frequency = dlight->ex_frequency;
				el->ex_size = dlight->ex_size;
				el->ex_speed = dlight->ex_speed;
				el->tl = -1;
				el->sample = audio::INVALID_ID;
				
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
		pos += sizeof(DANAE_LS_LIGHT) * nb_lights;
	}
	
	ClearCurLoadInfo();
	LogDebug("Loading FOGS");
	ARX_FOGS_Clear();
	
	for(long i = 0; i < dlh.nb_fogs; i++) {
		
		const DANAE_LS_FOG * dlf = reinterpret_cast<const DANAE_LS_FOG *>(dat + pos);
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
			Vec3f out;
			float ta = radians(MAKEANGLE(fd->angle.b));
			_YRotatePoint(&fd->move, &out, EEcos(ta), EEsin(ta));
			float tb = radians(MAKEANGLE(fd->angle.a));
			_XRotatePoint(&out, &fd->move, EEcos(tb), EEsin(tb));
		}
	}
	
	PROGRESS_BAR_COUNT += 2.f;
	LoadLevelScreen();
	
	ClearCurLoadInfo();
	LogDebug("Loading Nodes");
	ClearNodes();
	
	long nb_nodes = (dlh.version < 1.001f) ? 0 : dlh.nb_nodes;
	for(long i = 0; i < nb_nodes; i++) {
		
		nodes.nodes[i].exist = 1;
		nodes.nodes[i].selected = 0;
		const DANAE_LS_NODE * dln = reinterpret_cast<const DANAE_LS_NODE *>(dat + pos);
		pos += sizeof(DANAE_LS_NODE);
		
		strcpy(nodes.nodes[i].name, toLowercase(safestring(dln->name)).c_str());
		nodes.nodes[i].pos = (Vec3f)dln->pos + trans;
		
		for(long j = 0; j < dlh.nb_nodeslinks; j++) {
			if(dat[pos] != '\0') {
				strcpy(nodes.nodes[i].lnames[j], toLowercase(safestring(dat + pos, 64)).c_str());
			}
			pos += 64;
		}
	}
	
	RestoreNodeNumbers();
	
	ClearCurLoadInfo();
	LogDebug("Loading Paths");
	ARX_PATH_ReleaseAllPath();
	
	if(dlh.nb_paths) {
		ARXpaths = (ARX_PATH **)malloc(sizeof(ARX_PATH *) * dlh.nb_paths);
		nbARXpaths = dlh.nb_paths;
	}
	
	for(long i = 0; i < dlh.nb_paths; i++) {
		
		const DANAE_LS_PATH * dlp = reinterpret_cast<const DANAE_LS_PATH *>(dat + pos);
		pos += sizeof(DANAE_LS_PATH);
		
		Vec3f ppos = Vec3f(dlp->initpos) + trans;
		ARX_PATH * ap = ARXpaths[i] = new ARX_PATH(toLowercase(safestring(dlp->name)), ppos);
		
		ap->flags = PathFlags::load(dlp->flags); // TODO save/load flags
		ap->pos = Vec3f(dlp->pos) + trans;
		ap->nb_pathways = dlp->nb_pathways;
		ap->height = dlp->height;
		ap->ambiance = fs::path::load(safestring(dlp->ambiance));
		
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
			
			const DANAE_LS_PATHWAYS * dlpw = reinterpret_cast<const DANAE_LS_PATHWAYS *>(dat + pos);
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
	
	if(lightingFile) {
		
		ClearCurLoadInfo();
		LogDebug("Loading LLF Info");
		
		// using compression
		if(dlh.version >= 1.44f) {
			char * compressed = lightingFile->readAlloc();
			dat = (char*)blastMemAlloc(compressed, lightingFile->size(), FileSize);
			free(compressed);
		} else {
			dat = lightingFile->readAlloc();
			FileSize = lightingFile->size();
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
	
	const DANAE_LLF_HEADER * llh = reinterpret_cast<DANAE_LLF_HEADER *>(dat + pos);
	pos += sizeof(DANAE_LLF_HEADER);
	
	PROGRESS_BAR_COUNT += 4.f;
	LoadLevelScreen();
	
	if(llh->nb_lights != 0) {
		EERIE_LIGHT_GlobalInit();
	}
	
	for(int i = 0; i < llh->nb_lights; i++) {
		
		const DANAE_LS_LIGHT * dlight = reinterpret_cast<const DANAE_LS_LIGHT *>(dat + pos);
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
			
			el->extras = checked_range_cast<short>(dlight->extras);
			
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
			el->sample = audio::INVALID_ID;
		}
	}
	
	PROGRESS_BAR_COUNT += 2.f;
	LoadLevelScreen();
	
	const DANAE_LS_LIGHTINGHEADER * dll = reinterpret_cast<const DANAE_LS_LIGHTINGHEADER *>(dat + pos);
	pos += sizeof(DANAE_LS_LIGHTINGHEADER);
	
	long bcount = dll->nb_values;
	LastLoadedLightningNb = bcount;
	if(LastLoadedLightning != NULL) {
		free(LastLoadedLightning);
		LastLoadedLightning = NULL;
	}
	
	//DANAE_LS_VLIGHTING
	u32 * ll;
	ll = LastLoadedLightning = (u32 *)malloc(sizeof(u32) * bcount);
	if(dlh.version > 1.001f) {
		std::copy((u32*)(dat + pos), (u32*)(dat + pos) + bcount, LastLoadedLightning);
		pos += sizeof(u32) * bcount;
	} else {
		while(bcount) {
			const DANAE_LS_VLIGHTING * dlv = reinterpret_cast<const DANAE_LS_VLIGHTING *>(dat + pos);
			pos += sizeof(DANAE_LS_VLIGHTING);
			*ll = 0xff000000L | ((dlv->r & 255) << 16) | ((dlv->g & 255) << 8) | (dlv->b & 255);
			ll++;
			bcount--;
		}
	}
	
	arx_assert(pos <= FileSize);
	
	ModeLight = LightMode::load(dll->ModeLight); // TODO save/load flags
	ViewMode = ViewModeFlags::load(dll->ViewMode); // TODO save/load flags
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

	InitBkg(ACTIVEBKG, MAX_BKGX, MAX_BKGZ, BKG_SIZX, BKG_SIZZ);
	RemoveAllBackgroundActions();
	ClearNodes();

#ifdef BUILD_EDIT_LOADSAVE
	if(mse != NULL) {
		ReleaseMultiScene(mse);
		mse = NULL;
	}
#endif

	EERIE_LIGHT_GlobalInit();
	ARX_FOGS_Clear();
	TOTIOPDL = 0;

	UnlinkAllLinkedObjects();

	FreeAllInter();

	ReleaseAllSpellResources();
	TextureContainer::DeleteAll(TextureContainer::Level);
	mainApp->EvictManagedTextures();
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

void DanaeClearAll()
{
	DanaeClearLevel();
}

//*************************************************************************************
//*************************************************************************************

void RestoreLastLoadedLightning()
{
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
					u32 dc = LastLoadedLightning[pos];
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
		if (dlfcheck[n].ident == ident)
			return n;
	}

	return -1;
}

void AddIdent(std::string & ident, long num)
{
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

#ifdef BUILD_EDITOR

static void LogDirDestruction(const fs::path & dir ) {
	if(fs::is_directory(dir)) {
		LogDebug("LogDirDestruction: " << dir);
	}
}

//*************************************************************************************
// Checks for IO created during this session but not saved...
//*************************************************************************************
void CheckIO_NOT_SAVED() {
	
	if(!ADDED_IO_NOT_SAVED) {
		return;
	}
	
	for(long i = 1; i < inter.nbmax; i++) { // ignoring player

		if(!inter.iobj[i] || !inter.iobj[i]->scriptload) {
			continue;
		}
		
		if(!(inter.iobj[i]->EditorFlags & EFLAG_NOTSAVED) || inter.iobj[i]->ident <= 0) {
			continue;
		}
		
		fs::path temp = inter.iobj[i]->full_name();
		
		if(fs::is_directory(temp)) {
			LogDirDestruction(temp);
			if(!fs::remove_all(temp)) {
				LogError << "Could not remove directory " << temp;
			}
		}
		
		ReleaseInter(inter.iobj[i]);
	}
	
}

static void ARX_SAVELOAD_DLFCheckAdd(const fs::path & path, long num) {

	char _error[512];
	DANAE_LS_HEADER				dlh;
	DANAE_LS_INTER		*		dli;
	unsigned char * dat = NULL;

	long pos = 0;
	long i;
	size_t FileSize = 0;
	
	fs::path fic = fs::path(path).set_ext("dlf");
	
	dat = (unsigned char *)resources->readAlloc(fic, FileSize);
	if(!dat) {
		return;
	}
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
		sprintf(_error, "File %s is not a valid file", fic.string().c_str());
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
		ss << fs::path::load(dli->name).basename() << '_' << std::setfill('0') << std::setw(4) << dli->ident;
		string id = ss.str();
		AddIdent(id, num);
	}

	free(dat);
}

void ARX_SAVELOAD_CheckDLFs() {
	
	ARX_SAVELOAD_DLFCheckInit();

	for (long n = 0; n < 24; n++)
	{
		char temp[256];
		sprintf(temp, "graph/levels/level%ld/level%ld.dlf", n, n);
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

#endif // BUILD_EDITOR
