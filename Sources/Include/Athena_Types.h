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
#ifndef __ATHENA_TYPES_H__
#define __ATHENA_TYPES_H__

#include <ARX_Common.h>

#ifndef NULL
#define NULL    0
#endif

namespace ATHENA
{

	typedef void aalVoid;
	typedef unsigned char aalUBool;
	typedef signed char aalSBool;
	typedef unsigned char aalUByte;
	typedef signed char aalSByte;
	typedef unsigned short aalUWord;
	typedef signed short aalSWord;
	typedef unsigned long aalULong;
	typedef signed long aalSLong;
 
	typedef signed aalSInt;
	typedef float aalFloat;

	const aalUBool AAL_UFALSE(0);
	const aalUBool AAL_UTRUE(1);
	const aalSBool AAL_SFALSE(-1);
	const aalSBool AAL_STRUE(0);

	// Default values                                                            //
	const aalULong AAL_DEFAULT_STREAMLIMIT(1000);                                //1 second

	const aalULong AAL_DEFAULT_STRINGSIZE(0xff);                                 //256 characters

	const aalFloat AAL_DEFAULT_LISTENER_UNIT_FACTOR(1.0F);                       //1 unit = 1 meter
	const aalFloat AAL_DEFAULT_LISTENER_DOPPLER_FACTOR(1.0F);                    //Air-like doppler effect
	const aalFloat AAL_DEFAULT_LISTENER_ROLLOFF_FACTOR(1.0F);                    //Air-like rolloff effect

	const aalFloat AAL_DEFAULT_ENVIRONMENT_SIZE(7.5F);                           //
	const aalFloat AAL_DEFAULT_ENVIRONMENT_DIFFUSION(1.0F);                      //High density echoes
	const aalFloat AAL_DEFAULT_ENVIRONMENT_ABSORPTION(0.05F);                    //Air-like absorbtion
	const aalFloat AAL_DEFAULT_ENVIRONMENT_REFLECTION_VOLUME(0.8F);              //
	const aalFloat AAL_DEFAULT_ENVIRONMENT_REFLECTION_DELAY(7);                  //
	const aalFloat AAL_DEFAULT_ENVIRONMENT_REVERBERATION_VOLUME(1.0F);           //
	const aalFloat AAL_DEFAULT_ENVIRONMENT_REVERBERATION_DELAY(10);              //
	const aalFloat AAL_DEFAULT_ENVIRONMENT_REVERBERATION_DECAY(1500);            //
	const aalFloat AAL_DEFAULT_ENVIRONMENT_REVERBERATION_HFDECAY(1200);          //

	const aalFloat AAL_DEFAULT_VOLUME(1.0F);                                     //Original gain
	const aalFloat AAL_DEFAULT_AVERAGEVOLUME(0.0F);                              //No gain
	const aalFloat AAL_DEFAULT_PITCH(1.0F);                                      //Original frequency
	const aalFloat AAL_DEFAULT_PAN(1.0F);                                        //Centered panning
	const aalFloat AAL_DEFAULT_FALLSTART(1.0F);                                  //Volume fallstart
	const aalFloat AAL_DEFAULT_FALLEND(1000000000.0F);                           //Volume fallend
	const aalFloat AAL_DEFAULT_CONE_INNERANGLE(360.0F);                          //All directions
	const aalFloat AAL_DEFAULT_CONE_OUTERANGLE(360.0F);                          //All directions
	const aalFloat AAL_DEFAULT_CONE_OUTERVOLUME(0.0F);                           //No gain

	// Flags                                                                     //
	const enum aalFlag
	{
		AAL_FLAG_DUPLICATE       = 0x00000000,                                     //Duplicate sample if already playing
		AAL_FLAG_RESTART         = 0x00000001,                                     //Force restart sample if already playing
		AAL_FLAG_ENQUEUE         = 0x00000002,                                     //Enqueue sample if already playing
		AAL_FLAG_VOLUME          = 0x00000004,                                     //Enable volume control
		AAL_FLAG_PITCH           = 0x00000008,                                     //Enable pitch control
		AAL_FLAG_PAN             = 0x00000010,                                     //Enable pan control
		AAL_FLAG_POSITION        = 0x00000020,                                     //Enable position control
		AAL_FLAG_VELOCITY        = 0x00000040,                                     //Enable velocity control
		AAL_FLAG_DIRECTION       = 0x00000080,                                     //Enable orientation control
		AAL_FLAG_CONE            = 0x00000100,                                     //Enable cone control
		AAL_FLAG_FALLOFF         = 0x00000200,                                     //Enable intensity control
		AAL_FLAG_REVERBERATION   = 0x00000400,                                     //Enable environment reverberation / reflection
		AAL_FLAG_OBSTRUCTION     = 0x00000800,                                     //Enable environment obstruction
		AAL_FLAG_RELATIVE        = 0x00001000,                                     //Compute position relative to the listener
		AAL_FLAG_BACKGROUND      = 0x00002000,                                     //Continue playing even if app is in background
		AAL_FLAG_PRELOAD         = 0x00004000,                                     //Preload sample if not streamed
		AAL_FLAG_AUTOFREE        = 0x00008000,                                     //Free resource when playing is finished
		AAL_FLAG_CALLBACK        = 0x00010000,                                     //Enable sample callback management
		AAL_FLAG_MULTITHREAD     = 0x00020000,                                     //Enable multithreaded processing safety
		AAL_FLAG_PACKEDRESOURCES = 0x00040000,                                     //Enable input (but not output) operations from packed file
		AAL_FLAG_DEBUG           = 0x00080000                                      //Enable debug logging
	};

	// Length units                                                              //
	const enum aalUnit
	{
		AAL_UNIT_MS,
		AAL_UNIT_SAMPLES,
		AAL_UNIT_BYTES
	};

	// Errors                                                                    //
	const enum aalError
	{
		AAL_OK,
		AAL_ERROR,                                                                 //General error
		AAL_ERROR_INIT,                                                            //Not initialized
		AAL_ERROR_TIMEOUT,                                                         //Wait timeout
		AAL_ERROR_MEMORY,                                                          //Not enough memory
		AAL_ERROR_FILEIO,                                                          //File input/output error
		AAL_ERROR_FORMAT,                                                          //Invalid or corrupted file format
		AAL_ERROR_SYSTEM,                                                          //Internal system error
		AAL_ERROR_HANDLE                                                           //Invalid resource handle
	};

	// Key settings flags                                                        //
	const enum aalKeySettingFlag
	{
		AAL_KEY_SETTING_FLAG_RANDOM      = 0x00000001,
		AAL_KEY_SETTING_FLAG_INTERPOLATE = 0x00000002
	};

	// Output format                                                             //
	typedef struct
	{
		aalULong frequency;                                                        //Samples per second
		aalULong quality;                                                          //Bits per sample
		aalULong channels;                                                         //Output channels count
	} aalFormat;

	// Vector                                                                    //
	typedef struct
	{
		aalFloat x, y, z;
	} aalVector;

	// Source cone                                                               //
	typedef struct
	{
		aalFloat inner_angle;
		aalFloat outer_angle;
		aalFloat outer_volume;
	} aalCone;

	// Source falloff                                                            //
	typedef struct
	{
		aalFloat start;
		aalFloat end;
	} aalFalloff;

	// Environment obstruction                                                   //
	typedef struct
	{
		aalFloat direct;                                                           //Direct attenuation per unit
		aalFloat direct_lf;                                                        //Direct low frequency attenuation per unit
		aalFloat reverb;                                                           //Reverberation attenuation per unit
	} aalObstruction;

	// Environment reflection                                                    //
	typedef struct
	{
		aalFloat volume;
		aalULong delay;
	} aalReflection;

	// Environment reverberation                                                 //
	typedef struct
	{
		aalFloat volume;
		aalULong delay;
		aalULong decay;
		aalULong hf_decay;
	} aalReverberation;

	// Play channel initialization parameters                                    //
	typedef struct
	{
		aalULong flags;                                                            //A set of aalFlag
		aalSLong mixer;                                                            //Mixer id
		aalSLong environment;                                                      //Environment id
		aalFloat volume;
		aalFloat pitch;
		aalFloat pan;
		aalVector position;
		aalVector velocity;
		aalVector direction;
		aalCone cone;
		aalFalloff falloff;
	} aalChannel;

	// Callbacks prototype                                                       //
	typedef aalVoid(* aalSampleCallback)(aalVoid * reserved, const aalSLong & sample_id, aalVoid * data);
	typedef aalVoid(* aalEnvironmentCallback)(const aalVector & sample_position, const aalVector & listener_position, const aalObstruction & obstruction);

}//ATHENA::

#endif//__ATHENA_TYPES_H__
