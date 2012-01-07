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

#include "scene/CinematicSound.h"

#include "graphics/Math.h"
#include "io/resource/ResourcePath.h"
#include "scene/GameSound.h"

using std::string;

CinematicSound TabSound[MAX_SOUND];
int NbSound;

extern char DirectoryChoose[];
extern int	LSoundChoose;

CinematicSound::CinematicSound() : active(0), file(), handle(audio::INVALID_ID) { }

CinematicSound * GetFreeSound(int * num) {
	
	for(size_t i = 0; i < MAX_SOUND; i++) {
		if(!TabSound[i].active) {
			*num = i;
			return &TabSound[i];
		}
	}
	
	return NULL;
}

bool DeleteFreeSound(int num) {
	
	if(!TabSound[num].active) {
		return false;
	}
	
	TabSound[num].file.clear();
	
	TabSound[num].active = 0;
	NbSound--;
	
	return true;
}

void DeleteAllSound(void) {
	
	for(size_t i = 0; i < MAX_SOUND; i++) {
		DeleteFreeSound(i);
	}
}

static int findSound(const res::path & file) {
	
	for(size_t i = 0; i < MAX_SOUND; i++) {
		
		const CinematicSound & cs = TabSound[i];
		
		if(!cs.active || (cs.active & 0xFF00) != LSoundChoose) {
			continue;
		}
		
		if(cs.file == file) {
			return i;
		}
		
	}
	
	return -1;
}

int AddSoundToList(const res::path & path) {
	
	int num = findSound(path);
	if(num >= 0) {
		return num;
	}
	
	CinematicSound * cs = GetFreeSound(&num);
	if(!cs) {
		return -1;
	}
	
	cs->file = path;
	
	int iActive = 1 | LSoundChoose;
	
	cs->active = checked_range_cast<short>(iActive);
	
	NbSound++;
	return num;
}

bool PlaySoundKeyFramer(int id) {
	
	if(!TabSound[id].active) {
		return false;
	}
	
	TabSound[id].handle = ARX_SOUND_PlayCinematic(TabSound[id].file);
	
	return true;
}

void StopSoundKeyFramer() {
	
	for(size_t i = 0; i < MAX_SOUND; i++) {
		if(TabSound[i].active) {
			ARX_SOUND_Stop(TabSound[i].handle), TabSound[i].handle = audio::INVALID_ID;
		}
	}
}
