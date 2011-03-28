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

#include "io/CinematicLoad.h"

#include <stddef.h>
#include <algorithm>
#include <climits>

#include "animation/Cinematic.h"
#include "animation/CinematicKeyframer.h"

#include "core/Common.h"
#include "core/Application.h"

#include "graphics/data/CinematicTexture.h"

#include "io/PakManager.h"
#include "io/Logger.h"
#include "io/FilePath.h"

#include "scene/CinematicSound.h"

using std::search;
using std::string;

static const s32 CINEMATIC_FILE_VERSION = (1<<16) | 76;
static const s16 INTERP_NO_FADE = 2;

extern CinematicTrack * CKTrack;
extern C_KEY KeyTemp;
extern int LSoundChoose;

#pragma pack(push,1)

// Version 1.75 structure
struct C_KEY_1_75 {
	int frame;
	int numbitmap;
	int fx; // associated fx
	short typeinterp, force;
	EERIE_3D pos;
	float angz;
	int color;
	int colord;
	int colorf;
	int idsound;
	float speed;
	CinematicLight light;
	EERIE_3D posgrille;
	float angzgrille;
	float speedtrack;
};

#pragma pack(pop)

bool charCaseEqual(char ch1, char ch2) {
	return toupper(ch1) == toupper(ch2);
}

// TODO useful elsewhere too -> move to shared file
size_t strcasefind(const string & haystack, const string & needle) {
	string::const_iterator pos = search(haystack.begin(), haystack.end(), needle.begin(),
	                                    needle.end(), charCaseEqual);
	if(pos == haystack.end()) {
		return string::npos;
	} else {
		return pos - haystack.begin();
	}
}

void clearAbsDirectory(string & path, const string & delimiter) {
	
	size_t abs_dir = strcasefind(path, delimiter);
	
	if(abs_dir != std::string::npos) {
		path = path.substr(abs_dir + delimiter.length());
	}
}

void fixSoundPath(string & path) {
	
	size_t sfx_pos = strcasefind(path, "sfx");
	
	if(sfx_pos != string::npos) {
		// Sfx
		path.erase(0, sfx_pos);
		
		size_t uk_pos = strcasefind(path, "uk");
		if(uk_pos != string::npos) {
			path.replace(uk_pos, 3, "english\\");
		}
		
		size_t fr_pos = strcasefind(path, "fr");
		if(fr_pos != string::npos) {
			path.replace(fr_pos, 3, "francais\\");
		}
		
		size_t sfxspeech_pos = strcasefind(path, "sfx\\speech\\");
		if(sfxspeech_pos != string::npos) {
			path.erase(0, sfxspeech_pos + 4);
		}
		
	} else {
		// Speech
		path = "speech\\" + Project.localisationpath + "\\" + GetName(path);
	}
	
}

bool parseCinematic(Cinematic * c, const char * data, size_t size);

bool LoadProject(Cinematic * c, const char * dir, const char * name) {
	
	LogInfo << "loading cinematic " << dir << name;
	
	InitSound();
	
	string projectfile = dir;
	projectfile += name;
	
	size_t size;
	char * data = (char*)PAK_FileLoadMalloc(projectfile, size);
	if(!data) {
		LogError << "cinematic " << dir << name << " not found";
		return false;
	}
	
	bool ret = parseCinematic(c, data, size);
	free(data);
	if(!ret) {
		LogError << "loading cinematic " << dir << name;
		c->New();
	}
	
	return ret;
}

bool parseCinematic(Cinematic * c, const char * data, size_t size) {
	
	const char * cinematicId = safeGetString(data, size);
	if(!cinematicId) {
		LogDebug << "error parsing file magic number";
		return false;
	}
	
	if(strcmp(cinematicId, "KFA")) {
		LogDebug << "wrong magic number";
		return false;
	}
	
	s32 version;
	if(!safeGet(version, data, size)) {
		LogDebug << "error reading file version";
		return false;
	}
	LogDebug << "version " << version;
	
	if(version > CINEMATIC_FILE_VERSION) {
		LogDebug << "wrong version " << version << " expected max " << CINEMATIC_FILE_VERSION;
		return false;
	}
	
	// Ignore a string.
	safeGetString(data, size);
	
	// Load bitmaps.
	s32 nbitmaps;
	if(!safeGet(nbitmaps, data, size)) {
		LogDebug << "error reading bitmap count";
		return false;
	}
	LogDebug << "nbitmaps " << nbitmaps;

	c->m_bitmaps.reserve(nbitmaps);

	for(int i = 0; i < nbitmaps; i++) {

		s32 scale = 0;
		if(!safeGet(scale, data, size)) {
			LogDebug << "error reading bitmap scale";
			return false;
		}
		
		const char * str = safeGetString(data, size);
		if(!str) {
			LogDebug << "error bitmap path";
			return false;
		}
		string path = str;
		
		clearAbsDirectory(path, "ARX\\");
		
		LogDebug << "adding bitmap " << i << ": " << path;
		
		CinematicBitmap* newBitmap = CreateCinematicBitmap(path, c, scale);
		if(newBitmap)
			c->m_bitmaps.push_back(newBitmap);
	}
	
	// Load sounds.
	LSoundChoose = C_KEY::French;
			
	s32 nsounds;
	if(!safeGet(nsounds, data, size)) {
		LogDebug << "error reading sound count";
		return false;
	}
	
	LogDebug << "nsounds " << nsounds;
	for(int i = 0; i < nsounds; i++) {
		
		if(version >= CINEMATIC_FILE_VERSION) {
			s16 il;
			if(!safeGet(il, data, size)) {
				LogDebug << "error reading sound id";
				return false;
			}
			LSoundChoose = il;
		}
		
		const char * str = safeGetString(data, size);
		if(!str) {
			LogDebug << "error reading sound path";
			return false;
		}
		string path = str;
		
		LogDebug << "raw sound path " << i << ": " << path;
		
		fixSoundPath(path);
		
		LogDebug << "adding sound " << i << ": " << path;
		
		if(AddSoundToList(path) < 0) {
			LogError << "AddSoundToList failed for " << path;
		}		
	}
	
	// Load track and keys.
	
	// TODO struct loading
	CinematicTrack t;
	if(!safeGet(t, data, size)) {
		LogDebug << "error reading track";
		return false;
	}
	AllocTrack(t.startframe, t.endframe, t.fps);
	
	// Hack: ignore the pointer at the end of the struct.
	data -= 4;
	size += 4;
	
	LogDebug << "nkey " << t.nbkey << " " << size << " " << sizeof(C_KEY);
	for(int i = 0; i < t.nbkey; i++) {
		
		C_KEY k;
		
		LogDebug << "loading key " << i << " " << size;
				
		if(version <= ((1 << 16) | 75)) {
			
			C_KEY_1_75 k175;
			if(!safeGet(k175, data, size)) {
				LogDebug << "error reading key v1.75";
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
			k.idsound[C_KEY::French] = k175.idsound;
			k.light.pos = k175.light.pos;
			k.light.fallin = k175.light.fallin;
			k.light.fallout = k175.light.fallout;
			k.light.r = k175.light.r;
			k.light.g = k175.light.g;
			k.light.b = k175.light.b;
			k.light.intensity = k175.light.intensity;
			k.light.intensiternd = k175.light.intensiternd;
			k.posgrille.x = k175.posgrille.x;
			k.posgrille.y = k175.posgrille.y;
			k.posgrille.z = k175.posgrille.z;
			k.angzgrille = k175.angzgrille;
			k.speedtrack = k175.speedtrack;
			
		} else {
			if(!safeGet(k, data, size)) {
				LogDebug << "error reading key v1.76";
				return false;
			}
		}

		if(version <= ((1 << 16) | 75)) {
			for (int i = 1; i < 16; i++) k.idsound[i] = -1;
		}
		
		if(k.force < 0) {
			k.force = 1;
		}
		
		FillKeyTemp(&k.pos, k.angz, k.frame, k.numbitmap, k.fx, k.typeinterp, k.color, k.colord, k.colorf, k.speed, -1, k.force, &k.light, &k.posgrille, k.angzgrille, k.speedtrack);
		memcpy(&KeyTemp.idsound, &k.idsound, 16 * 4);
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
	
	LogDebug << "loaded cinematic";
	
	return true;
}
