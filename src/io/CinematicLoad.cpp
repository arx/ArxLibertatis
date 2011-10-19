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

#include "io/CinematicLoad.h"

#include <stddef.h>
#include <cstring>
#include <cstdlib>

#include "animation/Cinematic.h"
#include "animation/CinematicKeyframer.h"

#include "core/Config.h"

#include "graphics/data/CinematicTexture.h"

#include "io/PakReader.h"
#include "io/log/Logger.h"
#include "io/FilePath.h"
#include "io/CinematicFormat.h"

#include "platform/Platform.h"
#include "platform/String.h"

#include "scene/CinematicSound.h"

using std::string;
using std::copy;
using std::strcmp;
using std::free;

extern C_KEY KeyTemp;
extern int LSoundChoose;

static fs::path fixTexturePath(const string & path) {
	
	string copy = toLowercase(path);
	
	size_t abs_dir = copy.find("arx\\");
	
	if(abs_dir != std::string::npos) {
		return fs::path::load(copy.substr(abs_dir + 4));
	} else {
		return fs::path::load(copy);
	}
}

static fs::path fixSoundPath(const string & str) {
	
	string path = toLowercase(str);
	
	size_t sfx_pos = path.find("sfx");
	
	// Sfx
	if(sfx_pos != string::npos) {
		path.erase(0, sfx_pos);
		
		size_t uk_pos = path.find("uk");
		if(uk_pos != string::npos) {
			path.replace(uk_pos, 3, "english\\");
		}
		
		size_t fr_pos = path.find("fr");
		if(fr_pos != string::npos) {
			path.replace(fr_pos, 3, "francais\\");
		}
		
		size_t sfxspeech_pos = path.find("sfx\\speech\\");
		if(sfxspeech_pos != string::npos) {
			path.erase(0, sfxspeech_pos + 4);
		}
		
		return fs::path::load(path);
	}
	
	// Speech
	
	size_t namepos = path.find_last_of('\\');
	namepos = (namepos == string::npos) ? 0 : namepos + 1;
	
	return fs::path("speech") / config.language / path.substr(namepos);
}

bool parseCinematic(Cinematic * c, const char * data, size_t size);

bool loadCinematic(Cinematic * c, const fs::path & file) {
	
	LogInfo << "loading cinematic " << file;
	
	size_t size;
	char * data = resources->readAlloc(file, size);
	if(!data) {
		LogError << "cinematic " << file << " not found";
		return false;
	}
	
	bool ret = parseCinematic(c, data, size);
	free(data);
	if(!ret) {
		LogError << "loading cinematic " << file;
		c->New();
	}
	
	return ret;
}

bool parseCinematic(Cinematic * c, const char * data, size_t size) {
	
	const char * cinematicId = safeGetString(data, size);
	if(!cinematicId) {
		LogError << "error parsing file magic number";
		return false;
	}
	
	if(strcmp(cinematicId, "KFA")) {
		LogError << "wrong magic number";
		return false;
	}
	
	s32 version;
	if(!safeGet(version, data, size)) {
		LogError << "error reading file version";
		return false;
	}
	LogDebug("version " << version);
	
	if(version < CINEMATIC_VERSION_1_75) {
		LogError << "too old version " << version << " expected at least " << CINEMATIC_VERSION_1_75;
	}
	
	if(version > CINEMATIC_VERSION_1_76) {
		LogError << "wrong version " << version << " expected max " << CINEMATIC_VERSION_1_76;
		return false;
	}
	
	// Ignore a string.
	safeGetString(data, size);
	
	// Load bitmaps.
	s32 nbitmaps;
	if(!safeGet(nbitmaps, data, size)) {
		LogError << "error reading bitmap count";
		return false;
	}
	LogDebug("nbitmaps " << nbitmaps);
	
	c->m_bitmaps.reserve(nbitmaps);
	
	for(int i = 0; i < nbitmaps; i++) {
		
		s32 scale = 0;
		if(!safeGet(scale, data, size)) {
			LogError << "error reading bitmap scale";
			return false;
		}
		
		const char * str = safeGetString(data, size);
		if(!str) {
			LogError << "error reading bitmap path";
			return false;
		}
		fs::path path = fixTexturePath(str);
		
		LogDebug("adding bitmap " << i << ": " << path);
		
		CinematicBitmap * newBitmap = CreateCinematicBitmap(path, scale);
		if(newBitmap) {
			c->m_bitmaps.push_back(newBitmap);
		}
	}
	
	// Load sounds.
	LSoundChoose = C_KEY::French;
			
	s32 nsounds;
	if(!safeGet(nsounds, data, size)) {
		LogError << "error reading sound count";
		return false;
	}
	
	LogDebug("nsounds " << nsounds);
	for(int i = 0; i < nsounds; i++) {
		
		if(version >= CINEMATIC_VERSION_1_76) {
			s16 il;
			if(!safeGet(il, data, size)) {
				LogError << "error reading sound id";
				return false;
			}
			LSoundChoose = il;
		}
		
		const char * str = safeGetString(data, size);
		if(!str) {
			LogError << "error reading sound path";
			return false;
		}
		fs::path path = fixSoundPath(str);
		
		LogDebug("adding sound " << i << ": " << path);
		
		if(AddSoundToList(path) < 0) {
			LogError << "AddSoundToList failed for " << path;
		}
	}
	
	// Load track and keys.
	
	SavedCinematicTrack t;
	if(!safeGet(t, data, size)) {
		LogError << "error reading track";
		return false;
	}
	AllocTrack(t.startframe, t.endframe, t.fps);
	
	LogDebug("nkey " << t.nbkey << " " << size << " " << sizeof(C_KEY_1_76));
	for(int i = 0; i < t.nbkey; i++) {
		
		C_KEY k;
		
		LogDebug("loading key " << i << " " << size);
				
		if(version <= CINEMATIC_VERSION_1_75) {
			
			C_KEY_1_75 k175;
			if(!safeGet(k175, data, size)) {
				LogError << "error reading key v1.75";
				return false;
			}
			
			k.angz = k175.angz;
			k.color = k175.color;
			k.colord = k175.colord;
			k.colorf = k175.colorf;
			k.frame = k175.frame;
			k.fx = k175.fx;
			k.numbitmap = k175.numbitmap;
			k.pos = k175.pos;
			k.speed = k175.speed;
			k.typeinterp = k175.typeinterp;
			k.force = k175.force;
			k.idsound[0] = k175.idsound;
			for(size_t i = 1; i < 16; i++) {
				k.idsound[i] = -1;
			}
			k.light = k175.light;
			k.posgrille = k175.posgrille;
			k.angzgrille = k175.angzgrille;
			k.speedtrack = k175.speedtrack;
			
		} else {
			
			C_KEY_1_76 k176;
			if(!safeGet(k176, data, size)) {
				LogError << "error reading key v1.76";
				return false;
			}
			
			k.angz = k176.angz;
			k.color = k176.color;
			k.colord = k176.colord;
			k.colorf = k176.colorf;
			k.frame = k176.frame;
			k.fx = k176.fx;
			k.numbitmap = k176.numbitmap;
			k.pos = k176.pos;
			k.speed = k176.speed;
			k.typeinterp = k176.typeinterp;
			k.force = k176.force;
			k.light = k176.light;
			k.posgrille = k176.posgrille;
			k.angzgrille = k176.angzgrille;
			k.speedtrack = k176.speedtrack;
			copy(k176.idsound, k176.idsound + 16, k.idsound);
			
		}
		
		if(k.force < 0) {
			k.force = 1;
		}
		
		FillKeyTemp(&k.pos, k.angz, k.frame, k.numbitmap, k.fx, k.typeinterp, k.color, k.colord, k.colorf, k.speed, -1, k.force, &k.light, &k.posgrille, k.angzgrille, k.speedtrack);
		copy(k.idsound, k.idsound + 16, KeyTemp.idsound);
		AddKeyLoad(&KeyTemp);
		
		if(i == 0) {
			c->pos = k.pos;
			c->angz = k.angz;
			c->numbitmap = k.numbitmap;
			c->fx = k.fx;
			c->ti = c->tichoose = k.typeinterp;
			c->color = c->colorchoose = k.color;
			c->colord = c->colorchoosed = k.colord;
			c->colorflash = c->colorflashchoose = k.colorf;
			c->speed = c->speedchoose = k.speed;
			c->idsound = k.idsound[C_KEY::French];
			c->force = k.force;
			c->light = c->lightchoose = k.light;
			c->posgrille = k.posgrille;
			c->angzgrille = k.angzgrille;
			c->speedtrack = k.speedtrack;
		}
		
	}
	
	UpDateAllKeyLight();
	
	SetCurrFrame(0);
	
	GereTrackNoPlay(c);
	c->projectload = true;
	
	InitUndo();
	
	LSoundChoose = C_KEY::English << 8;
	
	LogDebug("loaded cinematic");
	
	return true;
}
