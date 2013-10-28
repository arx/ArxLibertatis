/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/Color.h"
#include "graphics/data/Mesh.h"
#include "io/resource/ResourcePath.h"
#include "math/Types.h"

#include "Configure.h"

class TextureContainer;
struct EERIE_3DOBJ;
struct EERIE_MULTI3DSCENE;

const size_t MAX_GOLD_COINS_VISUALS = 7;

extern Color3f FADECOLOR;
extern TextureContainer * TC_fire2;
extern TextureContainer * TC_fire;
extern TextureContainer * TC_smoke;
extern TextureContainer * GoldCoinsTC[MAX_GOLD_COINS_VISUALS];
extern EERIE_3DOBJ * cabal;
extern EERIE_3DOBJ * cameraobj;
extern EERIE_3DOBJ * markerobj;
extern Vec3f lastteleport;
extern EERIE_CAMERA bookcam;
extern Vec2s DANAEMouse;
extern EERIE_CAMERA subj;
extern Vec3f moveto;
extern Vec2s STARTDRAG;
extern EERIE_3DOBJ * GoldCoinsObj[MAX_GOLD_COINS_VISUALS];
extern Vec3f Mscenepos;
#if BUILD_EDIT_LOADSAVE
extern EERIE_MULTI3DSCENE * mse;
extern long ADDED_IO_NOT_SAVED;
#endif
extern Entity * COMBINE;
extern res::path LastLoadedScene;
extern char TELEPORT_TO_LEVEL[64];
extern char TELEPORT_TO_POSITION[64];
extern float PULSATE;
extern float framedelay;
extern float BASE_FOCAL;

extern float Xratio;
extern float Yratio;
inline Vec2f sizeRatio() { return Vec2f(Xratio, Yratio); }
inline float minSizeRatio() { return std::min(Xratio, Yratio); }

extern long	FADEDURATION;
extern long	FADEDIR;
extern float framedelay;
extern bool FirstFrame;

#ifdef BUILD_EDITOR
extern long DEBUGNPCMOVE;
#endif
extern long CURRENTLEVEL;
extern long TELEPORT_TO_ANGLE;

extern Rect g_size;

extern unsigned long FADESTART;
extern unsigned long AimTime;

class Image;
extern Image savegame_thumbnail;

extern float Original_framedelay;
extern long LOADEDD;
extern std::string WILL_LAUNCH_CINE;
extern long PLAY_LOADED_CINEMATIC;
extern long CINE_PRELOAD;

struct QUAKE_FX_STRUCT {
	float intensity;
	float frequency;
	unsigned long start;
	unsigned long duration;
	long	flags;
};
extern QUAKE_FX_STRUCT QuakeFx;

extern bool g_debugToggles[10];

void SetEditMode(long ed, const bool stop_sound = true);
void AddQuakeFX(float intensity, float duration, float period, long flags);

void SendGameReadyMsg();
void DanaeSwitchFullScreen();
void DANAE_KillCinematic();
void ARX_SetAntiAliasing();
void ReMappDanaeButton();
void AdjustMousePosition();
void DANAE_StartNewQuest();
bool HandleGameFlowTransitions();
void DANAE_Manage_Cinematic();
void DanaeRestoreFullScreen();
void FirstFrameHandling();

void ShowTestText();
void ShowInfoText();
void ShowFPS();
void ShowDebugToggles();

void DrawImproveVisionInterface();

void ManageFade();
void ManageQuakeFX(EERIE_CAMERA *cam);

void ManageCombatModeAnimations();
void ManageCombatModeAnimationsEND();
void ManageNONCombatModeAnimations();

Entity * FlyingOverObject(Vec2s * pos);

void runGame();

void DANAE_ReleaseAllDatasDynamic();

#endif // ARX_CORE_CORE_H
