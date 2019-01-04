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

#ifndef ARX_GUI_MENUPUBLIC_H
#define ARX_GUI_MENUPUBLIC_H

#include <stddef.h>
#include <string>

#include "core/SaveGame.h"

// Video options

void ARXMenu_Options_Video_SetFogDistance(float distance);
void ARXMenu_Options_Video_SetDetailsQuality(int lod);
void ARXMenu_Options_Video_SetGamma(float gamma);

// Audio options

void ARXMenu_Options_Audio_SetDevice(const std::string & device);
void ARXMenu_Options_Audio_SetMasterVolume(float volume);
void ARXMenu_Options_Audio_SetSfxVolume(float volume);
void ARXMenu_Options_Audio_SetSpeechVolume(float volume);
void ARXMenu_Options_Audio_SetAmbianceVolume(float volume);
void ARXMenu_Options_Audio_ApplyGameVolumes();
void ARXMenu_Options_Audio_SetMuted(bool mute);

// Resume game

void ARXMenu_ResumeGame();

// New quest

void ARXMenu_NewQuest();

void ARXMenu_Private_Options_Video_SetResolution(bool fullscreen, int _iWidth, int _iHeight);

#endif // ARX_GUI_MENUPUBLIC_H
