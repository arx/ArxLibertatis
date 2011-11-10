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

#ifndef ARX_SCENE_CINEMATICSOUND_H
#define ARX_SCENE_CINEMATICSOUND_H

#include "audio/AudioTypes.h"
#include "io/FilePath.h"

#define MAX_SOUND 256

struct CinematicSound {
	
	CinematicSound();
	
	short active;
	fs::path file;
	audio::SourceId handle;
	
};

void DeleteAllSound();

CinematicSound * GetFreeSound(int * num);
bool DeleteFreeSound(int num);
int AddSoundToList(const fs::path & path);
bool PlaySoundKeyFramer(int id);
void StopSoundKeyFramer(void);

#endif // ARX_SCENE_CINEMATICSOUND_H
