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
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#ifndef ARX_CORE_CORE_H
#define ARX_CORE_CORE_H

#include <stddef.h>
#include <string>

#include "graphics/Color.h"
#include "graphics/data/Mesh.h"
#include "io/FilePath.h"
#include "math/MathFwd.h"

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
extern EERIE_CAMERA subj, mapcam;
extern Vec3f moveto;
extern Vec2s STARTDRAG;
extern EERIE_3DOBJ * GoldCoinsObj[MAX_GOLD_COINS_VISUALS];
extern EERIE_3DOBJ * nodeobj;
extern Vec3f Mscenepos;
#ifdef BUILD_EDIT_LOADSAVE
extern EERIE_MULTI3DSCENE * mse;
extern long ADDED_IO_NOT_SAVED;
#endif
extern EERIE_CAMERA * Kam;
extern EERIE_BACKGROUND DefaultBkg;
extern INTERACTIVE_OBJ * COMBINE;
extern fs::path LastLoadedScene;
extern char TELEPORT_TO_LEVEL[64];
extern char TELEPORT_TO_POSITION[64];
extern float PULSATE;
extern float _framedelay;
extern float BASE_FOCAL;
extern float Xratio;
extern float Yratio;
extern long	FADEDURATION;
extern long	FADEDIR;
extern float FrameDiff;
extern long FirstFrame;
#ifdef BUILD_EDITOR
extern long EDITMODE;
extern long EDITION;
extern long MOULINEX;
extern long USE_COLLISIONS;
extern long WILLLOADLEVEL; // Is a LoadLevel command waiting ?
extern long WILLSAVELEVEL; // Is a SaveLevel command waiting ?
extern long NODIRCREATION; // No IO Directory Creation ?
extern const char * GTE_TITLE;
extern char * GTE_TEXT;
extern long GTE_SIZE;
extern long CHANGE_LEVEL_PROC_RESULT;
extern long DEBUGNPCMOVE;
extern long GAME_EDITOR;
extern long TRUEFIGHT;
#else
const long EDITMODE = 0;
#endif
extern long SHOW_TORCH;
extern long CURRENTLEVEL;
extern long TELEPORT_TO_ANGLE;
extern long DANAESIZX;
extern long DANAESIZY;
extern long DANAECENTERX;
extern long DANAECENTERY;
extern unsigned long FADESTART;
extern unsigned long AimTime;


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

void SetEditMode(long ed, const bool stop_sound = true);
void AddQuakeFX(float intensity, float duration, float period, long flags);

void SendGameReadyMsg();
void DanaeSwitchFullScreen();
void DANAE_KillCinematic();
void ARX_SetAntiAliasing();
void ReMappDanaeButton();
void AdjustMousePosition();
void DANAE_StartNewQuest();
bool DANAE_ManageSplashThings();
long DANAE_Manage_Cinematic();
void DanaeRestoreFullScreen();
void FirstFrameHandling();

void ShowTestText();
void ShowInfoText();
void ShowFPS();

void DrawImproveVisionInterface();
void DrawMagicSightInterface();
void RenderAllNodes();

void CheckMr();

void ManageFade();
void ManageQuakeFX();

void ManageCombatModeAnimations();
void ManageCombatModeAnimationsEND();
void ManageNONCombatModeAnimations();

INTERACTIVE_OBJ * FlyingOverObject(Vec2s * pos);

#endif // ARX_CORE_CORE_H
