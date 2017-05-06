/*
 * Copyright 2011-2016 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_CORE_CORE_H
#define ARX_CORE_CORE_H

#include <stddef.h>
#include <string>
#include <ctime>

#include "graphics/Color.h"
#include "graphics/data/Mesh.h"
#include "io/resource/ResourcePath.h"
#include "math/Types.h"
#include "platform/Platform.h"

#include "Configure.h"

class TextureContainer;
struct EERIE_3DOBJ;
struct EERIE_MULTI3DSCENE;

const size_t MAX_GOLD_COINS_VISUALS = 7;
extern TextureContainer * GoldCoinsTC[MAX_GOLD_COINS_VISUALS];

extern TextureContainer * TC_smoke;

extern EERIE_3DOBJ * cameraobj;
extern EERIE_3DOBJ * markerobj;
extern Vec3f lastteleport;
extern EERIE_CAMERA bookcam;
extern Vec2s DANAEMouse;
extern EERIE_CAMERA subj;
extern Vec3f g_moveto;
extern Vec2s STARTDRAG;
extern EERIE_3DOBJ * GoldCoinsObj[MAX_GOLD_COINS_VISUALS];
extern Vec3f Mscenepos;
#if BUILD_EDIT_LOADSAVE
extern EERIE_MULTI3DSCENE * mse;
extern long ADDED_IO_NOT_SAVED;
#endif
extern Entity * COMBINE;
extern res::path LastLoadedScene;

extern std::string TELEPORT_TO_LEVEL;
extern std::string TELEPORT_TO_POSITION;

extern float PULSATE;
extern float g_framedelay;

extern bool g_requestLevelInit;

extern long CURRENTLEVEL;
extern long TELEPORT_TO_ANGLE;

extern float GLOBAL_SLOWDOWN;

inline float bowZoomFromDuration(float duration) {
	return duration / 710.f;
}

extern Rect g_size;

extern Vec2f g_sizeRatio;
inline float minSizeRatio() { return std::min(g_sizeRatio.x, g_sizeRatio.y); }

inline float RATIO_X(float a) { return a * g_sizeRatio.x; }
inline float RATIO_Y(float a) { return a * g_sizeRatio.y; }

inline Vec2f RATIO_2(const Vec2f & in) {
	return Vec2f(RATIO_X(in.x), RATIO_Y(in.y));
}

class Image;
extern Image savegame_thumbnail;

extern bool LOADEDD;

extern bool g_debugToggles[10];
extern bool g_debugTriggers[10];
extern u32 g_debugTriggersTime[10];
static const u32 g_debugTriggersDecayDuration = 200;
extern float g_debugValues[10];

extern long		CHANGE_LEVEL_ICON;

extern Vec3f LastValidPlayerPos;
extern Vec3f WILL_RESTORE_PLAYER_POSITION;
extern bool WILL_RESTORE_PLAYER_POSITION_FLAG;

void SetEditMode(long ed, const bool stop_sound = true);

void SendGameReadyMsg();
void ARX_SetAntiAliasing();
void DANAE_StartNewQuest();
void DanaeRestoreFullScreen();
bool AdjustUI();

void levelInit();

void DrawImproveVisionInterface();

void ManageCombatModeAnimations();
void ManageCombatModeAnimationsEND();
void ManageNONCombatModeAnimations();

Entity * FlyingOverObject(const Vec2s & pos);

void runGame();

struct Playthrough {
	
	std::time_t startTime;
	
	u64 uniqueId;
	
};

extern Playthrough g_currentPlathrough;

#endif // ARX_CORE_CORE_H
