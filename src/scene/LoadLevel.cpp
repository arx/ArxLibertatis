/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
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

#include <boost/foreach.hpp>
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

#include "gui/LoadLevelScreen.h"
#include "gui/MiniMap.h"
#include "gui/Interface.h"

#include "graphics/Math.h"
#include "graphics/data/FTL.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/Fade.h"
#include "graphics/effects/Fog.h"
#include "graphics/particle/ParticleEffects.h"

#include "io/resource/ResourcePath.h"
#include "io/resource/PakReader.h"
#include "io/Blast.h"
#include "io/log/Logger.h"

#include "physics/CollisionShapes.h"

#include "scene/Object.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"
#include "scene/LevelFormat.h"
#include "scene/Light.h"

#include "util/String.h"


extern bool bGCroucheToggle;

Entity * LoadInter_Ex(const res::path & classPath, EntityInstance instance,
                      const Vec3f & pos, const Anglef & angle) {
	
	EntityHandle t = entities.getById(EntityId(classPath, instance));
	if(t != EntityHandle()) {
		return entities[t];
	}
	
	arx_assert(instance != 0);
	
	Entity * io = AddInteractive(classPath, instance, NO_MESH | NO_ON_LOAD);
	if(!io) {
		return NULL;
	}
	
	RestoreInitialIOStatusOfIO(io);
	ARX_INTERACTIVE_HideGore(io);
	
	io->lastpos = io->initpos = io->pos = pos;
	io->move = Vec3f(0.f);
	io->initangle = io->angle = angle;
	
	if(PakDirectory * dir = g_resources->getDirectory(io->instancePath())) {
		loadScript(io->over_script, dir->getFile(io->className() + ".asl"));
	}
	
	if(SendIOScriptEvent(NULL, io, SM_LOAD) == ACCEPT && io->obj == NULL) {
		bool pbox = (io->ioflags & IO_ITEM) == IO_ITEM;
		io->obj = loadObject(io->classPath() + ".teo", pbox);
		if(io->ioflags & IO_NPC) {
			EERIE_COLLISION_Cylinder_Create(io);
		}
	}
	
	return io;
}

struct ToColorBGRA {
	template <typename T>
	ColorBGRA operator()(const T & bgra) const {
		return ColorBGRA(bgra);
	}
};

static std::vector<ColorBGRA> g_levelLighting;

static void loadLights(const char * dat, size_t & pos, size_t count, const Vec3f & trans = Vec3f(0.f)) {
	
	if(count != 0) {
		EERIE_LIGHT_GlobalInit();
	}
	
	for(size_t i = 0; i < count; i++) {
		
		const DANAE_LS_LIGHT * dlight = reinterpret_cast<const DANAE_LS_LIGHT *>(dat + pos);
		pos += sizeof(DANAE_LS_LIGHT);
		
		long j = EERIE_LIGHT_Create();
		if(j >= 0) {
			EERIE_LIGHT * el = g_staticLights[j];
			
			el->m_exists = true;
			el->m_isVisible = true;
			el->fallend = dlight->fallend;
			el->fallstart = dlight->fallstart;
			el->falldiffmul = 1.f / (el->fallend - el->fallstart);
			el->intensity = dlight->intensity;
			
			el->pos = dlight->pos.toVec3() + trans;
			
			el->rgb = dlight->rgb;
			
			el->extras = ExtrasType::load(dlight->extras);
			
			el->ex_flicker = dlight->ex_flicker;
			el->ex_radius = dlight->ex_radius;
			el->ex_frequency = dlight->ex_frequency;
			el->ex_size = dlight->ex_size;
			el->ex_speed = dlight->ex_speed;
			el->ex_flaresize = dlight->ex_flaresize;
			
			el->m_ignitionStatus = !(el->extras & EXTRAS_STARTEXTINGUISHED);
			
			if((el->extras & EXTRAS_SPAWNFIRE) && !(el->extras & EXTRAS_FLARE)) {
				el->extras |= EXTRAS_FLARE;
				if(el->extras & EXTRAS_FIREPLACE) {
					el->ex_flaresize = 95.f;
				} else {
					el->ex_flaresize = 80.f;
				}
			}
			
			el->m_ignitionLightHandle = LightHandle();
			el->sample = audio::SourcedSample();
			
		}
	}
	
}

static void loadLighting(const char * dat, size_t & pos, bool compact, bool skip = false) {
	
	const DANAE_LS_LIGHTINGHEADER * dll = reinterpret_cast<const DANAE_LS_LIGHTINGHEADER *>(dat + pos);
	pos += sizeof(DANAE_LS_LIGHTINGHEADER);
	
	size_t count = dll->nb_values;
	
	if(!skip) {
		g_levelLighting.resize(count);
	}
	
	if(compact) {
		if(!skip) {
			const u32 * begin = reinterpret_cast<const u32 *>(dat + pos);
			std::transform(begin, begin + count, g_levelLighting.begin(), ToColorBGRA());
		}
		pos += sizeof(u32) * count;
	} else {
		if(!skip) {
			const DANAE_LS_VLIGHTING * begin = reinterpret_cast<const DANAE_LS_VLIGHTING *>(dat + pos);
			std::transform(begin, begin + count, g_levelLighting.begin(), ToColorBGRA());
		}
		pos += sizeof(DANAE_LS_VLIGHTING) * count;
	}
	
}

bool DanaeLoadLevel(const res::path & file, bool loadEntities) {
	
	LogInfo << "Loading level " << file;
	
	CURRENTLEVEL = GetLevelNumByName(file.string());
	
	res::path lightingFileName = res::path(file).set_ext("llf");

	LogDebug("fic2 " << lightingFileName);
	LogDebug("fileDlf " << file);
	
	std::string buffer = g_resources->read(file);
	if(buffer.empty()) {
		LogError << "Unable to find " << file;
		return false;
	}
	
	g_requestLevelInit = true;
	
	PakFile * lightingFile = g_resources->getFile(lightingFileName);
	
	progressBarAdvance();
	LoadLevelScreen();
	
	DANAE_LS_HEADER dlh;
	memcpy(&dlh, buffer.data(), sizeof(DANAE_LS_HEADER));
	size_t pos = sizeof(DANAE_LS_HEADER);
	
	LogDebug("dlh.version " << dlh.version << " header size " << sizeof(DANAE_LS_HEADER));
	
	if(dlh.version > DLH_CURRENT_VERSION) {
		LogError << "Unexpected level file version: " << dlh.version << " for " << file;
		return false;
	}
	
	// using compression
	if(dlh.version >= 1.44f) {
		buffer = blast(buffer.data() + pos, buffer.size() - pos);
		if(buffer.empty()) {
			LogError << "Could not decompress level file " << file;
			return false;
		}
		pos = 0;
	}
	
	const char * dat = buffer.data();
	
	player.desiredangle = player.angle = dlh.angle_edit;
	
	if(strcmp(dlh.ident, "DANAE_FILE") != 0) {
		LogError << "Not a valid file " << file << ": \"" << util::loadString(dlh.ident) << '"';
		return false;
	}
	
	LogDebug("Loading Scene");
	
	Vec3f trans(0.f);
	
	// Loading Scene
	if(dlh.nb_scn > 0) {
		
		const DANAE_LS_SCENE * dls = reinterpret_cast<const DANAE_LS_SCENE *>(dat + pos);
		pos += sizeof(DANAE_LS_SCENE);
		
		res::path scene = res::path::load(util::loadString(dls->name));
		
		if(FastSceneLoad(scene, trans)) {
			LogDebug("done loading scene");
		} else {
			LogError << "Fast loading scene failed";
		}
		
		EERIEPOLY_Compute_PolyIn();
		LastLoadedScene = scene;
	}
	
	player.pos = dlh.pos_edit.toVec3() + trans;
	
	float increment = 0;
	if(dlh.nb_inter > 0) {
		increment = 60.f / float(dlh.nb_inter);
	} else {
		progressBarAdvance(60);
		LoadLevelScreen();
	}
	
	for(long i = 0 ; i < dlh.nb_inter ; i++) {
		
		progressBarAdvance(increment);
		LoadLevelScreen();
		
		const DANAE_LS_INTER * dli = reinterpret_cast<const DANAE_LS_INTER *>(dat + pos);
		pos += sizeof(DANAE_LS_INTER);
		
		if(loadEntities) {
			
			std::string pathstr = boost::to_lower_copy(util::loadString(dli->name));
			
			size_t graphPos = pathstr.find("graph");
			if(graphPos != std::string::npos) {
				pathstr = pathstr.substr(graphPos);
			}
			
			res::path classPath = res::path::load(pathstr).remove_ext();
			LoadInter_Ex(classPath, dli->ident, dli->pos.toVec3() + trans, dli->angle);
		}
	}
	
	if(dlh.lighting) {
		loadLighting(dat, pos, dlh.version > 1.001f, lightingFile != NULL);
	}
	
	progressBarAdvance();
	LoadLevelScreen();
	
	size_t nb_lights = (dlh.version < 1.003f) ? 0 : size_t(dlh.nb_lights);
	
	if(!lightingFile) {
		loadLights(dat, pos, nb_lights);
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
			fd->exist = true;
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
			Vec3f out = VRotateY(fd->move, MAKEANGLE(fd->angle.getYaw()));
			fd->move = VRotateX(out, MAKEANGLE(fd->angle.getPitch()));
		}
	}
	
	progressBarAdvance(2.f);
	LoadLevelScreen();
	
	// Skip nodes
	pos += (dlh.version < 1.001f) ? 0 : dlh.nb_nodes * (204 + dlh.nb_nodeslinks * 64);
	
	LogDebug("Loading Paths");
	ARX_PATH_ReleaseAllPath();
	
	g_paths.resize(dlh.nb_paths);
	
	for(long i = 0; i < dlh.nb_paths; i++) {
		
		const DANAE_LS_PATH * dlp = reinterpret_cast<const DANAE_LS_PATH *>(dat + pos);
		pos += sizeof(DANAE_LS_PATH);
		
		Vec3f ppos = dlp->initpos.toVec3() + trans;
		ARX_PATH * ap = new ARX_PATH(boost::to_lower_copy(util::loadString(dlp->name)), ppos);
		
		ap->flags = PathFlags::load(dlp->flags); // TODO save/load flags
		ap->pos = dlp->pos.toVec3() + trans;
		ap->height = dlp->height;
		ap->ambiance = res::path::load(util::loadString(dlp->ambiance));
		
		ap->amb_max_vol = dlp->amb_max_vol;
		if(ap->amb_max_vol <= 1.f) {
			ap->amb_max_vol = 100.f;
		}
		
		ap->farclip = dlp->farclip;
		ap->reverb = dlp->reverb;
		ap->rgb = dlp->rgb;
		
		ap->pathways.resize(dlp->nb_pathways);
		for(long j = 0; j < dlp->nb_pathways; j++) {
			
			const DANAE_LS_PATHWAYS * dlpw = reinterpret_cast<const DANAE_LS_PATHWAYS *>(dat + pos);
			pos += sizeof(DANAE_LS_PATHWAYS);
			
			ap->pathways[j].flag = PathwayType(dlpw->flag); // TODO save/load enum
			ap->pathways[j].rpos = dlpw->rpos.toVec3();
			ap->pathways[j]._time = GameDurationMs(dlpw->time); // TODO save/load time
		}
		
		g_paths[i] = ap;
		
	}
	
	ARX_PATH_ComputeAllBoundingBoxes();
	progressBarAdvance(5.f);
	LoadLevelScreen();
	
	ARX_UNUSED(pos);
	arx_assert(pos <= buffer.size());
	
	
	// Now load a separate LLF lighting file
	
	pos = 0;
	buffer.clear();
	
	if(lightingFile) {
		
		LogDebug("Loading LLF Info");
		
		buffer = lightingFile->read();
		
		// using compression
		if(dlh.version >= 1.44f) {
			buffer = blast(buffer);
		}
		
	}
	
	if(buffer.empty()) {
		USE_PLAYERCOLLISIONS = true;
		LogInfo << "Done loading level";
		return true;
	}
	
	dat = buffer.data();
	
	const DANAE_LLF_HEADER * llh = reinterpret_cast<const DANAE_LLF_HEADER *>(dat + pos);
	pos += sizeof(DANAE_LLF_HEADER);
	
	progressBarAdvance(4.f);
	LoadLevelScreen();
	
	loadLights(dat, pos, size_t(llh->nb_lights), trans);
	
	progressBarAdvance(2.f);
	LoadLevelScreen();
	
	loadLighting(dat, pos, dlh.version > 1.001f);
	
	ARX_UNUSED(pos);
	arx_assert(pos <= buffer.size());
	
	progressBarAdvance();
	LoadLevelScreen();
	
	USE_PLAYERCOLLISIONS = true;
	
	LogInfo << "Done loading level";
	
	return true;
	
}

long FAST_RELEASE = 0;

void DanaeClearLevel() {
	
	g_miniMap.reset();
	
	fadeReset();
	LAST_JUMP_ENDTIME = 0;
	FAST_RELEASE = 1;
	ARX_GAME_Reset();
	FlyingOverIO = NULL;

	EERIE_PATHFINDER_Release();

	InitBkg(ACTIVEBKG);
	
	EERIE_LIGHT_GlobalInit();
	ARX_FOGS_Clear();
	
	culledStaticLightsReset();
	
	UnlinkAllLinkedObjects();
	
	entities.clear();
	
	TextureContainer::DeleteAll(TextureContainer::Level);
	g_miniMap.clearMarkerTexCont();
	
	bGCroucheToggle = false;
	
	resetDynLights();
	
	TREATZONE_Release();
	TREATZONE_Clear();
	
	FAST_RELEASE = 0;
	
	CURRENTLEVEL = -1;
}

void RestoreLastLoadedLightning(BackgroundData & eb) {
	
	if(g_levelLighting.empty()) {
		return;
	}
	
	if(g_levelLighting.size() != size_t(CountBkgVertex())) {
		g_levelLighting.clear();
		return;
	}
	
	size_t i = 0;
	
	// TODO copy-paste poly iteration
	for(short z = 0; z < eb.m_size.y; z++)
	for(short x = 0; x < eb.m_size.x; x++) {
		BackgroundTileData & eg = eb.m_tileData[x][z];
		BOOST_FOREACH(EERIEPOLY & ep, eg.polydata) {
			long nbvert = (ep.type & POLY_QUAD) ? 4 : 3;
			for(long k = 0; k < nbvert; k++) {
				if(i >= g_levelLighting.size()) {
					g_levelLighting.clear();
					return;
				}
				ep.color[k] = ep.v[k].color = Color::fromBGRA(g_levelLighting[i++]).toRGB();
			}
		}
	}
	
}
