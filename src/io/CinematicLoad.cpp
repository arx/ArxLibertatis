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

#include "io/CinematicLoad.h"

#include <stddef.h>
#include <cstring>
#include <cstdlib>
#include <iomanip>
#include <utility>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "animation/Cinematic.h"
#include "animation/CinematicKeyframer.h"

#include "core/Config.h"

#include "graphics/data/CinematicTexture.h"

#include "io/resource/PakReader.h"
#include "io/log/Logger.h"
#include "io/resource/ResourcePath.h"
#include "io/CinematicFormat.h"

#include "platform/Platform.h"

#include "scene/CinematicSound.h"

extern C_KEY KeyTemp;

static res::path fixTexturePath(const std::string & path) {
	
	std::string copy = boost::to_lower_copy(path);
	
	size_t abs_dir = copy.find("arx\\");
	
	if(abs_dir != std::string::npos) {
		return res::path::load(copy.substr(abs_dir + 4));
	} else {
		return res::path::load(copy);
	}
}

static std::pair<res::path, bool> fixSoundPath(const std::string & str) {
	
	std::string path = boost::to_lower_copy(str);
	
	// Remove the .wav file extension
	if(boost::ends_with(path, ".wav")) {
		path.resize(path.size() - 4);
	}
	
	// Remove irrelevant absolute path components
	size_t sfx_pos = path.find("\\sfx\\");
	if(sfx_pos != std::string::npos) {
		path.erase(0, sfx_pos + 1);
	}
	
	// I guess they changed their minds about language names
	size_t uk_pos = path.find("\\uk\\");
	if(uk_pos != std::string::npos) {
		path.replace(uk_pos, 4, "\\english\\");
	}
	size_t fr_pos = path.find("\\fr\\");
	if(fr_pos != std::string::npos) {
		path.replace(fr_pos, 4, "\\francais\\");
	}
	
	// Change the speech directory
	if(boost::starts_with(path, "sfx\\speech\\")) {
		path.erase(0, 4);
	}
	
	// Remove hardcoded language for localised speech
	bool is_speech = false;
	if(boost::starts_with(path, "speech\\")) {
		size_t pos = path.find('\\', 7);
		if(pos != std::string::npos) {
			path.erase(0, pos + 1);
			is_speech = true;
		}
	}
	
	return std::make_pair(res::path::load(path), is_speech);
}

bool parseCinematic(Cinematic * c, const char * data, size_t size);

bool loadCinematic(Cinematic * c, const res::path & file) {
	
	LogInfo << "Loading cinematic " << file;
	
	size_t size;
	char * data = resources->readAlloc(file, size);
	if(!data) {
		LogError << "Cinematic " << file << " not found";
		return false;
	}
	
	bool ret = parseCinematic(c, data, size);
	std::free(data);
	if(!ret) {
		LogError << "Could not load cinematic " << file;
		c->New();
	}
	
	return ret;
}

bool parseCinematic(Cinematic * c, const char * data, size_t size) {
	
	const char * cinematicId = safeGetString(data, size);
	if(!cinematicId) {
		LogError << "Error parsing file magic number";
		return false;
	}
	
	if(std::strcmp(cinematicId, "KFA")) {
		LogError << "Wrong magic number";
		return false;
	}
	
	s32 version;
	if(!safeGet(version, data, size)) {
		LogError << "Error reading file version";
		return false;
	}
	LogDebug("version " << version);
	
	if(version < CINEMATIC_VERSION_1_75) {
		LogError << "Too old version " << version << " expected at least " << CINEMATIC_VERSION_1_75;
	}
	
	if(version > CINEMATIC_VERSION_1_76) {
		LogError << "Wrong version " << version << " expected max " << CINEMATIC_VERSION_1_76;
		return false;
	}
	
	// Ignore a string.
	safeGetString(data, size);
	
	// Load bitmaps.
	s32 nbitmaps;
	if(!safeGet(nbitmaps, data, size)) {
		LogError << "Error reading bitmap count";
		return false;
	}
	LogDebug(nbitmaps << " images:");
	
	c->m_bitmaps.reserve(nbitmaps);
	
	for(int i = 0; i < nbitmaps; i++) {
		
		s32 scale = 0;
		if(!safeGet(scale, data, size)) {
			LogError << "Error reading bitmap scale";
			return false;
		}
		
		const char * str = safeGetString(data, size);
		if(!str) {
			LogError << "Error reading bitmap path";
			return false;
		}
		LogDebug(" - " << i << ": \"" << str << '"');
		res::path path = fixTexturePath(str);
		LogDebug("   => " << path << " (scale x" << scale << ')');
		
		
		CinematicBitmap * newBitmap = CreateCinematicBitmap(path, scale);
		if(newBitmap) {
			c->m_bitmaps.push_back(newBitmap);
		}
	}
	
	// Load sounds.
	s32 nsounds;
	if(!safeGet(nsounds, data, size)) {
		LogError << "Error reading sound count";
		return false;
	}
	
	LogDebug(nsounds << " sounds:");
	for(int i = 0; i < nsounds; i++) {
		
		if(version >= CINEMATIC_VERSION_1_76) {
			s16 ignored;
			if(!safeGet(ignored, data, size)) {
				LogError << "Error reading sound id";
				return false;
			}
		}
		
		const char * str = safeGetString(data, size);
		if(!str) {
			LogError << "Error reading sound path";
			return false;
		}
		LogDebug(" - " << i << ": \"" << str << '"');
		std::pair<res::path, bool> path = fixSoundPath(str);
		LogDebug("   => " << path.first << (path.second ? " (speech)" : ""));
		
		AddSoundToList(path.first, path.second);
	}
	
	// Load track and keys.
	
	SavedCinematicTrack t;
	if(!safeGet(t, data, size)) {
		LogError << "Error reading track";
		return false;
	}
	AllocTrack(t.startframe, t.endframe, t.fps);
	
	LogDebug(t.nbkey << " keyframes:");
	for(int i = 0; i < t.nbkey; i++) {
		
		C_KEY k;
		int idsound;
		
		if(version <= CINEMATIC_VERSION_1_75) {
			
			C_KEY_1_75 k175;
			if(!safeGet(k175, data, size)) {
				LogError << "Error reading key v1.75";
				return false;
			}
			
			k.angz = k175.angz;
			k.color = k175.color;
			k.colord = k175.colord;
			k.colorf = k175.colorf;
			k.frame = k175.frame;
			k.fx = k175.fx;
			k.numbitmap = k175.numbitmap;
			k.pos = k175.pos.toVec3();
			k.speed = k175.speed;
			k.typeinterp = k175.typeinterp;
			k.force = k175.force;
			idsound = k175.idsound;
			k.idsound = -1;
			k.light = k175.light;
			k.posgrille = k175.posgrille.toVec3();
			k.angzgrille = k175.angzgrille;
			k.speedtrack = k175.speedtrack;
			
		} else {
			
			C_KEY_1_76 k176;
			if(!safeGet(k176, data, size)) {
				LogError << "Error reading key v1.76";
				return false;
			}
			
			k.angz = k176.angz;
			k.color = k176.color;
			k.colord = k176.colord;
			k.colorf = k176.colorf;
			k.frame = k176.frame;
			k.fx = k176.fx;
			k.numbitmap = k176.numbitmap;
			k.pos = k176.pos.toVec3();
			k.speed = k176.speed;
			k.typeinterp = k176.typeinterp;
			k.force = k176.force;
			k.light = k176.light;
			k.posgrille = k176.posgrille.toVec3();
			k.angzgrille = k176.angzgrille;
			k.speedtrack = k176.speedtrack;
			idsound = k176.idsound[0]; // 0 was the language code for 'French'
			k.idsound = k176.idsound[3]; // 3 was the language code for 'English'
			
		}
		
		if(k.force < 0) {
			k.force = 1;
		}
		
		FillKeyTemp(&k.pos, k.angz, k.frame, k.numbitmap, k.fx, k.typeinterp, k.color, k.colord, k.colorf, k.speed, k.idsound, k.force, &k.light, &k.posgrille, k.angzgrille, k.speedtrack);
		AddKeyLoad(&KeyTemp);
		
		LogDebug(" - " << i << ": frame " << k.frame << " image: " << k.numbitmap);
		if(k.idsound >= 0) {
			LogDebug("   + sound: " << k.idsound);
		}
		
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
			c->idsound = idsound;
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
	
	LogDebug("loaded cinematic");
	
	return true;
}
