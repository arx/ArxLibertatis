/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

#include <boost/algorithm/string/case_conv.hpp>

#include "ai/PathFinderManager.h"
#include "ai/Paths.h"

#include "core/Application.h"
#include "core/GameTime.h"
#include "core/Config.h"
#include "core/Core.h"

#include "game/EntityManager.h"
#include "game/Levels.h"
#include "game/Player.h"

#include "gui/MiniMap.h"
#include "gui/Interface.h"

#include "graphics/Math.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/Fog.h"
#include "graphics/particle/ParticleEffects.h"

#include "io/fs/FilePath.h"
#include "io/fs/FileStream.h"
#include "io/fs/Filesystem.h"
#include "io/fs/SystemPaths.h"
#include "io/resource/ResourcePath.h"
#include "io/resource/PakReader.h"
#include "io/Blast.h"
#include "io/Implode.h"
#include "io/log/Logger.h"

#include "physics/CollisionShapes.h"

#include "scene/Object.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"
#include "scene/LevelFormat.h"
#include "scene/Light.h"

#include "util/String.h"

using std::max;
using std::string;

extern float PROGRESS_BAR_COUNT;
extern float PROGRESS_BAR_TOTAL;
extern long DONT_ERASE_PLAYER;

extern QUAKE_FX_STRUCT QuakeFx;
extern bool bGToggleCombatModeWithKey;
extern bool bGCroucheToggle;

#if BUILD_EDIT_LOADSAVE

void LogDirCreation(const fs::path & dir) {
	if(fs::is_directory(dir)) {
		LogDebug("LogDirCreation: " << dir);
	}
}

long DanaeSaveLevel(const fs::path & _fic) {
	
	long nb_inter = GetNumberInterWithOutScriptLoad(); // Without Player
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
	
	strcpy(dlh.ident, "DANAE_FILE");
	
	dlh.nb_scn = LastLoadedScene.empty() ? 0 : 1;
	
	dlh.nb_inter = nb_inter;
	dlh.nb_zones = 0;
	
	dlh.pos_edit = (dlh.nb_scn != 0) ? subj.orgTrans.pos - Mscenepos : subj.orgTrans.pos;

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
	for(size_t i = 1; i < entities.size(); i++) {
		if(entities[i] && !entities[i]->scriptload) {
			
			Entity * io = entities[i];
			
			DANAE_LS_INTER dli;
			memset(&dli, 0, sizeof(DANAE_LS_INTER));
			
			if(dlh.nb_scn != 0) {
				dli.pos = io->initpos - Mscenepos;
			} else {
				dli.pos = io->initpos;
			}
			
			dli.angle = io->initangle;
			strncpy(dli.name, (io->classPath() + ".teo").string().c_str(),
			        sizeof(dli.name));
			
			if(io->ident == 0) {
				MakeIOIdent(io);
			}
			dli.ident = io->ident;
			
			if(io->ioflags & IO_FREEZESCRIPT) {
				dli.flags = IO_FREEZESCRIPT;
			}
			
			memcpy(dat + pos, &dli, sizeof(DANAE_LS_INTER));
			pos += sizeof(DANAE_LS_INTER);
		}
	}
	
	for(size_t i = 0; i < MAX_FOG; i++) {
		FOG_DEF *fog = &fogs[i];

		if(!fog->exist)
			continue;

			DANAE_LS_FOG dlf;
			memset(&dlf, 0, sizeof(DANAE_LS_FOG));
			dlf.rgb = fog->rgb;
			dlf.angle = fog->angle;
			dlf.pos = fog->pos - Mscenepos;
			dlf.blend = fog->blend;
			dlf.frequency = fog->frequency;
			dlf.move = fog->move;
			dlf.rotatespeed = fog->rotatespeed;
			dlf.scale = fog->scale;
			dlf.size = fog->size;
			dlf.special = fog->special;
			dlf.speed = fog->speed;
			dlf.tolive = fog->tolive;
			memcpy(dat + pos, &dlf, sizeof(DANAE_LS_FOG));
			pos += sizeof(DANAE_LS_FOG);
	}
	
	for(long i = 0; i < nodes.nbmax; i++) {
		if(nodes.nodes[i].exist) {
			
			DANAE_LS_NODE dln;
			memset(&dln, 0, sizeof(DANAE_LS_NODE));
			strcpy(dln.name, nodes.nodes[i].name);
			dln.pos = nodes.nodes[i].pos - Mscenepos;
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
		dlp.initpos = ARXpaths[i]->initpos - Mscenepos;
		dlp.pos = ARXpaths[i]->pos - Mscenepos;
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
		dlight.pos = el->pos - Mscenepos;
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

void WriteIOInfo(Entity * io, const fs::path & dir) {
	
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

void SaveIOScript(Entity * io, long fl) {
	
	fs::path file;
	EERIE_SCRIPT * script;
	
	switch(fl) {
		
		case 1: { // class script
			file = fs::paths.user / io->classPath().string();
			script = &io->script;
			break;
		}
		
		case 2: { // local script
			if(io->ident == 0) {
				LogError << ("NO IDENT...");
				return;
			}
			file = fs::paths.user / io->full_name().string();
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
	
	file.append(".asl");
	
	fs::ofstream ofs(file, fs::fstream::out | fs::fstream::trunc | fs::fstream::binary);
	if(!ofs.is_open()) {
		LogError << ("Unable To Save...");
		return;
	}
	
	ofs.write(script->data, script->size);
	
	ARX_SCRIPT_ComputeShortcuts(*script);
}

#endif // BUILD_EDIT_LOADSAVE


Entity * LoadInter_Ex(const res::path & classPath, EntityInstance instance,
                      const Vec3f & pos, const Anglef & angle,
                      const Vec3f & trans) {
	
	std::ostringstream nameident;
	nameident << classPath.filename()
	          << std::setfill('0') << std::setw(4) << instance;
	
	long t = entities.getById(nameident.str());
	if(t >= 0) {
		return entities[t];
	}
	
	arx_assert(instance != 0);
	
	Entity * io = AddInteractive(classPath, instance, NO_MESH | NO_ON_LOAD);
	if(!io) {
		return NULL;
	}
	
	RestoreInitialIOStatusOfIO(io);
	ARX_INTERACTIVE_HideGore(io);
	
	io->lastpos = io->initpos = io->pos = pos + trans;
	io->move = Vec3f_ZERO;
	io->initangle = io->angle = angle;
	
	res::path tmp = io->full_name(); // Get the directory name to check for
	string id = io->short_name();
	if(PakDirectory * dir = resources->getDirectory(tmp)) {
		if(PakFile * file = dir->getFile(id + ".asl")) {
			loadScript(io->over_script, file);
			io->over_script.master = (io->script.data != NULL) ? &io->script : NULL;
		}
	}
	
	if(SendIOScriptEvent(io, SM_LOAD) == ACCEPT && io->obj == NULL) {
		bool pbox = (io->ioflags & IO_ITEM) == IO_ITEM;
		io->obj = loadObject(io->classPath() + ".teo", pbox);
		if(io->ioflags & IO_NPC) {
			EERIE_COLLISION_Cylinder_Create(io);
		}
	}
	
	return io;
}

static long LastLoadedLightningNb = 0;
static u32 * LastLoadedLightning = NULL;
Vec3f loddpos;
Vec3f MSP;

extern long FASTmse;

long DanaeLoadLevel(const res::path & file, bool loadEntities) {
	
	LogInfo << "Loading Level " << file;
	
	CURRENTLEVEL = GetLevelNumByName(file.string());
	
	res::path lightingFileName = res::path(file).set_ext("llf");

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
	
	loddpos = subj.orgTrans.pos = dlh.pos_edit.toVec3();
	player.desiredangle = player.angle = subj.angle = dlh.angle_edit;
	
	if(strcmp(dlh.ident, "DANAE_FILE")) {
		LogError << "Not a valid file " << file << ": \"" << util::loadString(dlh.ident) << '"';
		return -1;
	}
	
	LogDebug("Loading Scene");
	
	// Loading Scene
	if(dlh.nb_scn > 0) {
		
		const DANAE_LS_SCENE * dls = reinterpret_cast<const DANAE_LS_SCENE *>(dat + pos);
		pos += sizeof(DANAE_LS_SCENE);
		
		res::path scene = res::path::load(util::loadString(dls->name));
		
		if(FastSceneLoad(scene)) {
			LogDebug("done loading scene");
			FASTmse = 1;
		} else {
#if BUILD_EDIT_LOADSAVE
			LogDebug("fast loading scene failed");
			ARX_SOUND_PlayCinematic("editor_humiliation", false);
			mse = PAK_MultiSceneToEerie(scene);
			PROGRESS_BAR_COUNT += 20.f;
			LoadLevelScreen();
#else
			LogError << "Fast loading scene failed";
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
#if BUILD_EDIT_LOADSAVE
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
		lastteleport = player.pos = subj.orgTrans.pos = moveto = mse->pos + mse->point0;
		lastteleport.y -= 180.f;
		player.pos.y = subj.orgTrans.pos.y -= 180.f;
		trans = mse->pos;
	}
#endif // BUILD_EDIT_LOADSAVE
	else
	{
		lastteleport = player.baseOffset();
		Mscenepos = trans = Vec3f_ZERO;
	}
	
	MSP = trans;
	
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
		
		if(loadEntities) {
			
			string pathstr = boost::to_lower_copy(util::loadString(dli->name));
			
			size_t pos = pathstr.find("graph");
			if(pos != std::string::npos) {
				pathstr = pathstr.substr(pos);
			}
			
			res::path classPath = res::path::load(pathstr).remove_ext();
			LoadInter_Ex(classPath, dli->ident, dli->pos.toVec3(), dli->angle, trans);
		}
	}
	
	if(dlh.lighting) {
		
		const DANAE_LS_LIGHTINGHEADER * dll = reinterpret_cast<const DANAE_LS_LIGHTINGHEADER *>(dat + pos);
		pos += sizeof(DANAE_LS_LIGHTINGHEADER);
		long bcount = dll->nb_values;
		
		if(!lightingFile) {
			
			LastLoadedLightningNb = bcount;
			
			// DANAE_LS_VLIGHTING
			free(LastLoadedLightning);
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
				el->pos = dlight->pos.toVec3();
				el->rgb = dlight->rgb;
				
				el->extras = ExtrasType::load(dlight->extras);
				
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
			fd->pos = dlf->pos.toVec3() + trans;
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
			float ta = radians(MAKEANGLE(fd->angle.getPitch()));
			YRotatePoint(&fd->move, &out, EEcos(ta), EEsin(ta));
			float tb = radians(MAKEANGLE(fd->angle.getYaw()));
			XRotatePoint(&out, &fd->move, EEcos(tb), EEsin(tb));
		}
	}
	
	PROGRESS_BAR_COUNT += 2.f;
	LoadLevelScreen();
	
	LogDebug("Loading Nodes");
	ClearNodes();
	
	long nb_nodes = (dlh.version < 1.001f) ? 0 : dlh.nb_nodes;
	for(long i = 0; i < nb_nodes; i++) {
		
		nodes.nodes[i].exist = 1;
		nodes.nodes[i].selected = 0;
		const DANAE_LS_NODE * dln = reinterpret_cast<const DANAE_LS_NODE *>(dat + pos);
		pos += sizeof(DANAE_LS_NODE);
		
		strcpy(nodes.nodes[i].name, boost::to_lower_copy(util::loadString(dln->name)).c_str());
		nodes.nodes[i].pos = dln->pos.toVec3() + trans;
		
		for(long j = 0; j < dlh.nb_nodeslinks; j++) {
			if(dat[pos] != '\0') {
				strcpy(nodes.nodes[i].lnames[j], boost::to_lower_copy(util::loadString(dat + pos, 64)).c_str());
			}
			pos += 64;
		}
	}
	
	RestoreNodeNumbers();
	
	LogDebug("Loading Paths");
	ARX_PATH_ReleaseAllPath();
	
	if(dlh.nb_paths) {
		ARXpaths = (ARX_PATH **)malloc(sizeof(ARX_PATH *) * dlh.nb_paths);
		nbARXpaths = dlh.nb_paths;
	}
	
	for(long i = 0; i < dlh.nb_paths; i++) {
		
		const DANAE_LS_PATH * dlp = reinterpret_cast<const DANAE_LS_PATH *>(dat + pos);
		pos += sizeof(DANAE_LS_PATH);
		
		Vec3f ppos = dlp->initpos.toVec3() + trans;
		ARX_PATH * ap = ARXpaths[i] = new ARX_PATH(boost::to_lower_copy(util::loadString(dlp->name)), ppos);
		
		ap->flags = PathFlags::load(dlp->flags); // TODO save/load flags
		ap->pos = dlp->pos.toVec3() + trans;
		ap->nb_pathways = dlp->nb_pathways;
		ap->height = dlp->height;
		ap->ambiance = res::path::load(util::loadString(dlp->ambiance));
		
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
			app[j].rpos = dlpw->rpos.toVec3();
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
		USE_PLAYERCOLLISIONS = true;
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
			
			el->pos = dlight->pos.toVec3();
			if(FASTmse) {
				el->pos += trans;
			} 
			
			el->rgb = dlight->rgb;
			
			el->extras = ExtrasType::load(dlight->extras);
			
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
	
	//DANAE_LS_VLIGHTING
	free(LastLoadedLightning);
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
	
	ARX_UNUSED(pos), ARX_UNUSED(FileSize);
	arx_assert(pos <= FileSize);
	
	ModeLight = LightMode::load(dll->ModeLight); // TODO save/load flags
	ViewMode = ViewModeFlags::load(dll->ViewMode); // TODO save/load flags
	ViewMode &= ~VIEWMODE_WIRE;
	
	free(dat);
	
	PROGRESS_BAR_COUNT += 1.f;
	LoadLevelScreen();
	
	LOADEDD = 1;
	FASTmse = 0;
	USE_PLAYERCOLLISIONS = true;
	
	LogInfo << "Done loading level";
	
	return 1;
	
}

void MCache_ClearAll();
long FAST_RELEASE = 0;
extern Entity * FlyingOverIO;
extern unsigned long LAST_JUMP_ENDTIME;

extern EERIE_3DOBJ * stone0;
extern long stone0_count;
extern EERIE_3DOBJ * stone1;
extern long stone1_count;

extern long JUST_RELOADED;

void DanaeClearLevel(long flag)
{
	JUST_RELOADED = 0;
	g_miniMap.reset();

	FADEDIR = 0;
	FADEDURATION = 0;
	LAST_JUMP_ENDTIME = 0;
	FAST_RELEASE = 1;
	MCache_ClearAll();
	g_miniMap.purgeTexContainer();
	ARX_GAME_Reset(flag);
	FlyingOverIO = NULL;

	EERIE_PATHFINDER_Release();

	InitBkg(ACTIVEBKG, MAX_BKGX, MAX_BKGZ, BKG_SIZX, BKG_SIZZ);
	ClearNodes();
	
#if BUILD_EDIT_LOADSAVE
	if(mse != NULL) {
		ReleaseMultiScene(mse);
		mse = NULL;
	}
#endif
	
	EERIE_LIGHT_GlobalInit();
	ARX_FOGS_Clear();
	TOTIOPDL = 0;
	
	UnlinkAllLinkedObjects();
	
	entities.clear();
	
	DANAE_ReleaseAllDatasDynamic();
	delete stone0, stone0 = NULL, stone0_count = 0;
	delete stone1, stone1 = NULL, stone1_count = 0;
	
	TextureContainer::DeleteAll(TextureContainer::Level);
	g_miniMap.clearMarkerTexCont();
	
	arxtime.init();
	
	bGToggleCombatModeWithKey = false;
	bGCroucheToggle = false;
	
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

void RestoreLastLoadedLightning()
{
	long pos = 0;
	long bcount = CountBkgVertex();

	if(LastLoadedLightningNb <= 0)
		return;

	if(LastLoadedLightning == NULL)
		return;

	if(bcount != LastLoadedLightningNb) {
		free(LastLoadedLightning);
		LastLoadedLightning = NULL;
		LastLoadedLightningNb = 0;
		return;
	}

	EERIE_BACKGROUND * eb = ACTIVEBKG;

	bcount = LastLoadedLightningNb;

	for(long j = 0; j < eb->Zsize; j++)
		for(long i = 0; i < eb->Xsize; i++) {
			EERIE_BKG_INFO *eg = (EERIE_BKG_INFO *)&eb->Backg[i+j*eb->Xsize];

			for(long l = 0; l < eg->nbpoly; l++) {
				EERIEPOLY *ep = &eg->polydata[l];

				long nbvert = (ep->type & POLY_QUAD) ? 4 : 3;

				for(long k = 0; k < nbvert; k++) {
					u32 dc = LastLoadedLightning[pos];
					pos++;
					dc = dc | 0xFF000000;
					ep->tv[k].color = ep->v[k].color = dc;
					ep->tv[k].specular = ep->v[k].specular = 0xFF000000;
					bcount--;

					if(bcount <= 0)
						goto plusloin;
				}
			}
		}

plusloin:
	;
	free(LastLoadedLightning);
	LastLoadedLightning = NULL;
	LastLoadedLightningNb = 0;
}
