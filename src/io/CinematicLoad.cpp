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

#include "core/Common.h"
#include "animation/Cinematic.h"
#include "animation/CinematicKeyframer.h"
#include "core/Application.h"
#include "graphics/data/CinematicTexture.h"
#include "io/PakManager.h"
#include "io/Logger.h"
#include "io/IO.h"
#include "scene/CinematicSound.h"

using std::search;
using std::string;

static const s32 CINEMATIC_FILE_VERSION = (1<<16) | 76;
static const s16 INTERP_NO_FADE = 2;

extern CinematicBitmap TabBitmap[];
extern CinematicTrack * CKTrack;
extern C_KEY KeyTemp;
extern int LSoundChoose;

#pragma pack(push,1)

// Version 1.59 structures

struct C_KEY_1_59 {
	int frame;
	int numbitmap;
	int fx; // associated fx
	int typeinterp;
	EERIE_3D pos;
	float angz;
	int color;
	int colord;
	int colorf;
	float speed;
};

// Version 1.65 structures

struct C_KEY_1_65 {
	int frame;
	int numbitmap;
	int fx; // associated fx
	int typeinterp;
	EERIE_3D pos;
	float angz;
	int color;
	int colord;
	int colorf;
	int idsound;
	float speed;
};

// Version 1.70 structures

struct C_KEY_1_70 {
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
};

// Version 1.71 structures

struct C_KEY_1_71 {
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
};

// Version 1.72 structures

struct CinematicLight_1_72 {
	EERIE_3D pos;
	float fallin;
	float fallout;
	float r, g, b;
	float intensite;
	float intensiternd;
};

struct C_KEY_1_72 {
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
	CinematicLight_1_72 light;
	EERIE_3D posgrille;
	float angzgrille;
};

// Version 1.74 structures

struct C_KEY_1_74 {
	int frame;
	int numbitmap;
	int fx; //associated fx
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
};

// Version 1.75 structures

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
	
	InitMapLoad(c);
	InitSound(c);
	
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
	
	if(version >= ((1 << 16) | 61)) {
		// Ignore a string.
		safeGetString(data, size);
	}
	
	// Load bitmaps.
	s32 nbitmaps;
	if(!safeGet(nbitmaps, data, size)) {
		LogDebug << "error reading bitmap count";
		return false;
	}
	LogDebug << "nbitmaps " << nbitmaps;
	for(int i = 0; i < nbitmaps; i++) {
		
		s32 scale = 0;
		if(version >= ((1 << 16) | 71)) {
			if(!safeGet(scale, data, size)) {
				LogDebug << "error reading bitmap scale";
				return false;
			}
		}
		
		const char * str = safeGetString(data, size);
		if(!str) {
			LogDebug << "error bitmap path";
			return false;
		}
		string path = str;
		
		clearAbsDirectory(path, "ARX\\");
		
		LogDebug << "adding bitmap " << i << ": " << path;
		
		int id = CreateAllMapsForBitmap(path, c);
		
		if(TabBitmap[id].load) {
			if(scale > 1) {
				TabBitmap[id].grid.echelle = scale;
				c->ReInitMapp(id);
			} else {
				TabBitmap[id].grid.echelle = 1;
			}
		} else {
			TabBitmap[id].grid.echelle = 1;
		}
		
	}
	
	// Load sounds.
	LSoundChoose = C_KEY::French;
	if(version >= ((1 << 16) | 60)) {
		
		s32 nsounds;
		if(!safeGet(nsounds, data, size)) {
			LogDebug << "error reading sound count";
			return false;
		}
		
		LogDebug << "nsounds " << nsounds;
		for(int i = 0; i < nsounds; i++) {
			
			if(version >= ((1 << 16) | 76)) {
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
		
		if(version <= ((1 << 16) | 59)) {
			
			C_KEY_1_59 k159;
			if(!safeGet(k159, data, size)) {
				LogDebug << "error reading key v1.59";
				return false;
			}
			
			k.angz = k159.angz;
			k.color = k159.color;
			k.colord = k159.colord;
			k.colorf = k159.colorf;
			k.frame = k159.frame;
			k.fx = k159.fx;
			k.numbitmap = k159.numbitmap;
			k.pos = k159.pos;
			k.speed = k159.speed;

			ARX_CHECK_SHORT(k159.typeinterp);
			k.typeinterp = ARX_CLEAN_WARN_CAST_SHORT(k159.typeinterp);
			k.force = 1;
			k.idsound[C_KEY::French] = -1;
			k.light.intensity = -1.f;
			k.posgrille.x = k.posgrille.y = k.posgrille.z = 0.f;
			k.angzgrille = 0.f;
			k.speedtrack = 1.f;
			
		} else if(version <= ((1 << 16) | 65)) {
			
			C_KEY_1_65 k165;
			if(!safeGet(k165, data, size)) {
				LogDebug << "error reading key v1.65";
				return false;
			}
			
			k.angz = k165.angz;
			k.color = k165.color;
			k.colord = k165.colord;
			k.colorf = k165.colorf;
			k.frame = k165.frame;
			k.fx = k165.fx;
			k.numbitmap = k165.numbitmap;
			k.pos = k165.pos;
			k.speed = k165.speed;
			
			ARX_CHECK_SHORT(k165.typeinterp);
			k.typeinterp = ARX_CLEAN_WARN_CAST_SHORT(k165.typeinterp);
			k.force = 1;
			k.idsound[C_KEY::French] = k165.idsound;
			k.light.intensity = -1.f;
			k.posgrille.x = k.posgrille.y = k.posgrille.z = 0.f;
			k.angzgrille = 0.f;
			k.speedtrack = 1.f;
			
		} else if(version <= ((1 << 16) | 70)) {
			
			C_KEY_1_70 k170;
			if(!safeGet(k170, data, size)) {
				LogDebug << "error reading key v1.70";
				return false;
			}
			
			k.angz = k170.angz;
			k.color = k170.color;
			k.colord = k170.colord;
			k.colorf = k170.colorf;
			k.frame = k170.frame;
			k.fx = k170.fx;
			k.numbitmap = k170.numbitmap;
			k.pos = k170.pos;
			k.speed = k170.speed;
			k.typeinterp = k170.typeinterp;
			k.force = k170.force;
			k.idsound[C_KEY::French] = k170.idsound;
			k.light.intensity = -1.f;
			k.posgrille.x = k.posgrille.y = k.posgrille.z = 0.f;
			k.angzgrille = 0.f;
			k.speedtrack = 1.f;
			
		} else if(version <= ((1 << 16) | 71)) {
			
			C_KEY_1_71 k171;
			if(!safeGet(k171, data, size)) {
				LogDebug << "error reading key v1.71";
				return false;
			}
			
			k.angz = k171.angz;
			k.color = k171.color;
			k.colord = k171.colord;
			k.colorf = k171.colorf;
			k.frame = k171.frame;
			k.fx = k171.fx;
			k.numbitmap = k171.numbitmap;
			k.pos = k171.pos;
			k.speed = k171.speed;
			k.typeinterp = k171.typeinterp;
			k.force = k171.force;
			k.idsound[C_KEY::French] = k171.idsound;
			k.light = k171.light;
			
			if((k.fx & 0xFF000000) != FX_LIGHT) {
				k.light.intensity = -1.f;
			}
			
			k.posgrille.x = k.posgrille.y = k.posgrille.z = 0.f;
			k.angzgrille = 0.f;
			k.speedtrack = 1.f;
			
		} else if(version <= ((1 << 16) | 72)) {
			
			C_KEY_1_72 k172;
			if(!safeGet(k172, data, size)) {
				LogDebug << "error reading key v1.72";
				return false;
			}
			
			k.angz = k172.angz;
			k.color = k172.color;
			k.colord = k172.colord;
			k.colorf = k172.colorf;
			k.frame = k172.frame;
			k.fx = k172.fx;
			k.numbitmap = k172.numbitmap;
			k.pos = k172.pos;
			k.speed = k172.speed;
			k.typeinterp = k172.typeinterp;
			k.force = k172.force;
			k.idsound[C_KEY::French] = k172.idsound;
			k.light.pos = k172.light.pos;
			k.light.fallin = k172.light.fallin;
			k.light.fallout = k172.light.fallout;
			k.light.r = k172.light.r;
			k.light.g = k172.light.g;
			k.light.b = k172.light.b;
			k.light.intensity = k172.light.intensite;
			k.light.intensiternd = k172.light.intensiternd;
			k.posgrille.x = k172.posgrille.x;
			k.posgrille.y = k172.posgrille.y;
			k.posgrille.z = k172.posgrille.z;
			k.angzgrille = k172.angzgrille;
			k.speedtrack = 1.f;
			
			if((k.fx & 0xFF000000) != FX_LIGHT) {
				k.light.intensity = -1.f;
			}
			
		} else if(version <= ((1 << 16) | 74)) {
			
			C_KEY_1_74 k174;
			if(!safeGet(k174, data, size)) {
				LogDebug << "error reading key v1.74";
				return false;
			}
			
			k.angz = k174.angz;
			k.color = k174.color;
			k.colord = k174.colord;
			k.colorf = k174.colorf;
			k.frame = k174.frame;
			k.fx = k174.fx;
			k.numbitmap = k174.numbitmap;
			k.pos = k174.pos;
			k.speed = k174.speed;
			k.typeinterp = k174.typeinterp;
			k.force = k174.force;
			k.idsound[C_KEY::French] = k174.idsound;
			k.light.pos = k174.light.pos;
			k.light.fallin = k174.light.fallin;
			k.light.fallout = k174.light.fallout;
			k.light.r = k174.light.r;
			k.light.g = k174.light.g;
			k.light.b = k174.light.b;
			k.light.intensity = k174.light.intensity;
			k.light.intensiternd = k174.light.intensiternd;
			k.posgrille.x = k174.posgrille.x;
			k.posgrille.y = k174.posgrille.y;
			k.posgrille.z = k174.posgrille.z;
			k.angzgrille = k174.angzgrille;
			k.speedtrack = 1.f;
			
		} else if(version <= ((1 << 16) | 75)) {
			
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
		
		if(version <= ((1 << 16) | 67)) {
			if(k.typeinterp == INTERP_NO_FADE) {
				k.typeinterp = INTERP_NO;
				k.force = 1;
			}
		}
		
		if(version <= ((1 << 16) | 73)) {
			k.light.pos.x = k.light.pos.y = k.light.pos.z = 0.f;
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
	
	ActiveAllTexture(c);
	
	SetCurrFrame(0);
	
	GereTrackNoPlay(c);
	c->projectload = true;
	
	InitUndo();
	
	// precalc
	if(version < ((1 << 16) | 71)) {
		C_KEY * kk = CKTrack->key;
		for(int i = 0; i < CKTrack->nbkey; i++, kk++) {
			if((kk->fx & 0x0000FF00) == FX_DREAM) {
				TabBitmap[kk->numbitmap].grid.echelle = 4;
				c->ReInitMapp(kk->numbitmap);
			}
		}
	}
	
	LSoundChoose = C_KEY::English << 8;
	
	LogDebug << "loaded cinematic";
	
	return true;
}
