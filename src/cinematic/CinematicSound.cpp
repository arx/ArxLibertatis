/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "cinematic/CinematicSound.h"

#include <iomanip>

#include <boost/array.hpp>

#include "audio/AudioTypes.h"
#include "graphics/Math.h"
#include "io/log/Logger.h"
#include "io/resource/ResourcePath.h"
#include "scene/GameSound.h"

namespace {

// TODO move this into the Cinematic class

struct CinematicSound {
	
	CinematicSound()
		: exists(false)
		, isSpeech(false)
	{ }
	
	bool exists;
	bool isSpeech;
	res::path file;
	audio::SourcedSample handle;
	
};

boost::array<CinematicSound, 256> TabSound;

} // anonymous namespace

static CinematicSound * GetFreeSound() {
	
	for(size_t i = 0; i < TabSound.size(); i++) {
		if(!TabSound[i].exists) {
			return &TabSound[i];
		}
	}
	
	return NULL;
}

static bool DeleteFreeSound(size_t num) {
	
	if(!TabSound[num].exists) {
		return false;
	}
	
	TabSound[num].file.clear();
	
	TabSound[num].exists = false;
	
	return true;
}

void DeleteAllSound() {
	for(size_t i = 0; i < TabSound.size(); i++) {
		DeleteFreeSound(i);
	}
}

void AddSoundToList(const res::path & path, bool isSpeech) {
	
	CinematicSound * cs = GetFreeSound();
	if(!cs) {
		LogError << "AddSoundToList failed for " << path;
		return;
	}
	
	cs->file = path;
	cs->isSpeech = isSpeech;
	cs->exists = true;
}

bool PlaySoundKeyFramer(size_t index) {
	
	if(index >= TabSound.size() || !TabSound[index].exists) {
		return false;
	}
	
	CinematicSound & cs = TabSound[index];
	
	LogDebug("playing " << (cs.isSpeech ? "speech" : "sound")
	         << ' ' << index << " = " << cs.file);
	cs.handle = ARX_SOUND_PlayCinematic(cs.file, cs.isSpeech);
	
	return true;
}

void StopSoundKeyFramer() {
	
	for(size_t i = 0; i < TabSound.size(); i++) {
		if(TabSound[i].exists) {
			ARX_SOUND_Stop(TabSound[i].handle);
			TabSound[i].handle = audio::SourcedSample();
		}
	}
}
