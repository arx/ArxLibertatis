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
// ARX_Time.H
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Time Management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////
#ifndef ARX_TIME_H
#define ARX_TIME_H

#include <windows.h>
#include <Mmsystem.h>
#include "Danae.h"

//-----------------------------------------------------------------------------
extern float ARXPausedTime;
extern float ARXTotalPausedTime;
extern float ARXTime;
extern bool ARXPausedTimer;

#define lARXTime ( ARX_CLEAN_WARN_CAST_LONG( ARXTime ) )
#define dwARX_TIME_Get() ( ARX_CLEAN_WARN_CAST_DWORD( ARX_TIME_Get() ) )

//-----------------------------------------------------------------------------
void ARX_TIME_Pause();
void ARX_TIME_UnPause();
void ARX_TIME_Init();
void ARX_TIME_Force_Time_Restore(float time);

float _ARX_TIME_GetTime();

//-----------------------------------------------------------------------------
__inline float ARX_TIME_Get(bool _bUsePause = true)
{
	float tim = _ARX_TIME_GetTime();

	if (ARXPausedTimer && _bUsePause)
	{
		ARXTime = tim - ARXTotalPausedTime - (tim - ARXPausedTime);
	}
	else ARXTime = tim - ARXTotalPausedTime;

	return ARXTime;
}



__inline unsigned long ARX_TIME_GetUL(bool _bUsePause = true)
{
	float time = ARX_TIME_Get(_bUsePause);
	ARX_CHECK_ULONG(time);
	return ARX_CLEAN_WARN_CAST_ULONG(time);;
}
__inline unsigned long ARXTimeUL()
{
	ARX_CHECK_ULONG(ARXTime);
	return ARX_CLEAN_WARN_CAST_ULONG(ARXTime);
}



#endif
