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
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
// ARX_NPC
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX NPC Management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////
#ifndef ARX_NPC_H
#define ARX_NPC_H

#include "EERIETypes.h"
#include "EERIEPoly.h"

//-----------------------------------------------------------------------------
const float ARX_NPC_AUDIBLE_VOLUME_MIN(0.94F);
const float ARX_NPC_AUDIBLE_VOLUME_MAX(1.0F);
const float ARX_NPC_AUDIBLE_VOLUME_DEFAULT(ARX_NPC_AUDIBLE_VOLUME_MAX);
const float ARX_NPC_AUDIBLE_VOLUME_RANGE(ARX_NPC_AUDIBLE_VOLUME_MAX - ARX_NPC_AUDIBLE_VOLUME_MIN);
const float ARX_NPC_AUDIBLE_FACTOR_MIN(1.0F);
const float ARX_NPC_AUDIBLE_FACTOR_MAX(4.5F);
const float ARX_NPC_AUDIBLE_FACTOR_DEFAULT(ARX_NPC_AUDIBLE_FACTOR_MIN);
const float ARX_NPC_AUDIBLE_FACTOR_RANGE(ARX_NPC_AUDIBLE_FACTOR_MAX - ARX_NPC_AUDIBLE_FACTOR_MIN);
const float ARX_NPC_AUDIBLE_PRESENCE_DEFAULT(1.0F);

void ARX_NPC_Revive(INTERACTIVE_OBJ * io, long flag);
void ARX_NPC_SetStat(INTERACTIVE_OBJ * io, char * statname, float value);
void ARX_NPC_TryToCutSomething(INTERACTIVE_OBJ * target, EERIE_3D * pos);
BOOL ARX_NPC_LaunchPathfind(INTERACTIVE_OBJ * io, long target);
BOOL IsDeadNPC(INTERACTIVE_OBJ * io);

void FaceTarget2(INTERACTIVE_OBJ * io);
void ARX_TEMPORARY_TrySound(float power);
void ARX_NPC_Behaviour_Stack(INTERACTIVE_OBJ * io);
void ARX_NPC_Behaviour_UnStack(INTERACTIVE_OBJ * io);
void ARX_NPC_Behaviour_Reset(INTERACTIVE_OBJ * io);
void ARX_NPC_Behaviour_ResetAll();
void ARX_NPC_Behaviour_Change(INTERACTIVE_OBJ * io, long behavior, long behavior_param);
void ARX_NPC_ChangeMoveMode(INTERACTIVE_OBJ * io, long MOVEMODE);
void ARX_NPC_SpawnAudibleSound(EERIE_3D * pos, INTERACTIVE_OBJ * source,
                               const float factor = ARX_NPC_AUDIBLE_FACTOR_DEFAULT,
                               const float presence = ARX_NPC_AUDIBLE_PRESENCE_DEFAULT);
void ARX_NPC_NeedStepSound(INTERACTIVE_OBJ * io, EERIE_3D * pos,
                           const float volume = ARX_NPC_AUDIBLE_VOLUME_DEFAULT,
                           const float factor = ARX_NPC_AUDIBLE_FACTOR_DEFAULT);

INTERACTIVE_OBJ * ARX_NPC_GetFirstNPCInSight(INTERACTIVE_OBJ * ioo);
void CheckNPC(INTERACTIVE_OBJ * io);
void ManageIgnition(INTERACTIVE_OBJ * io);
void ManageIgnition_2(INTERACTIVE_OBJ * io);
void CheckUnderWaterIO(INTERACTIVE_OBJ * io);
BOOL IsDeadNPC(INTERACTIVE_OBJ * io);
void ARX_NPC_Kill_Spell_Launch(INTERACTIVE_OBJ * io);
#endif