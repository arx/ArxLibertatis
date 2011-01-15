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
#ifndef __ATHENA_AMBIANCE_H__
#define __ATHENA_AMBIANCE_H__

#include <Athena_Types.h>
#include "Athena_Resource.h"

namespace ATHENA
{

	typedef struct
	{
		aalULong flags;                   //A set of KeySettingFlag
		aalFloat min, max;                //Min and max setting values
		aalFloat from, to, cur;           //Current min and max values
		aalULong interval;                //Interval between updates (On Start = 0)
		aalSLong update;                  //Last update time
	} KeySetting;

	typedef struct
	{
		aalULong flags;                   //Nothing (padding! ;)
		aalULong start;                   //Start time (after last key)
		aalULong n_start;                 //Next time to play sample (when delayed)
		aalULong loop, loopc;             //Loop count
		aalULong delay_min, delay_max;    //Min and max delay before each sample loop
		aalULong delay;                   //Current delay
		KeySetting volume;             //Volume settings
		KeySetting pitch;              //Pitch settings
		KeySetting pan;                //Pan settings
		KeySetting x, y, z;            //Positon settings
	} TrackKey;

	typedef struct
	{
		aalSLong s_id;                    //Sample id
		aalSLong a_id;                    //Ambiance id
		char * name;                   //Track name (if NULL use sample name instead)
		aalULong flags;                   //A set of ATHENAAmbianceFlag
		aalULong key_c, key_i;            //Key count and current index
		TrackKey * key_l;              //Key list
	} Track;

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Class ATHENA::Ambiance                                                    //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	class Ambiance
	{
		public:
			// Constructor and destructor                                                //
			Ambiance();
			~Ambiance();

			// File input/output                                                         //
			aalError Load(const char * name);

			// Setup                                                                     //
			aalError SetUserData(aalVoid * data);
			aalError SetVolume(const aalFloat & volume);

			// Status                                                                    //
			aalError GetName(char * name, const aalULong & max_char = AAL_DEFAULT_STRINGSIZE);
			aalError GetUserData(aalVoid ** data);
			aalError GetTrackID(const char * name, aalSLong & track_id);
			aalError GetVolume(aalFloat & volume);
			aalUBool IsPaused();
			aalUBool IsPlaying();
			aalUBool IsLooped();

			// Control                                                                   //
			aalError Play(const aalChannel & channel, const aalULong & loop = 0, const aalULong & fade_interval = 0.0F);
			aalError Stop(const aalULong & volume_interval = 0.0F);
			aalError Pause();
			aalError Resume();
			aalError Update();

			// Key status                                                                //
			aalError GetTrackKeyLength(const aalSLong & track_id, const aalSLong & key_id, aalULong & length);

			// Track setup                                                               //
			aalError MuteTrack(const aalSLong & track_id, const aalUBool & mute);

			// Track status                                                              //
			aalError GetTrackLength(const aalSLong & track_id, aalULong & length);

			// Macros
			inline aalUBool IsNotTrack(const aalSLong & track_id);
			inline aalUBool IsNotTrackKey(const aalSLong & t_id, const aalSLong & k_id);

			// Data                                                                      //
			aalChannel channel;
			aalFloat fade_time, fade_interval, fade_max;
			aalULong flags;
			aalULong start, time;
			aalULong track_c;
			Track * track_l;
			char name[256];
			aalVoid * data;                               //User data
	};

}//ATHENA::

#endif//__ATHENA_AMBIANCE_H__
