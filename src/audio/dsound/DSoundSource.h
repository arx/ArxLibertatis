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

#ifndef ARX_AUDIO_AUDIOINSTANCE_H
#define ARX_AUDIO_AUDIOINSTANCE_H

#include "audio/AudioTypes.h"
#include "audio/dsound/dsoundfwd.h"

namespace audio {
	
	class Stream;
	class Sample;
	
	class Source {
		
	public:
		
		Source();
		~Source();
		
		// Setup
		aalError Init(Sample * sample, const aalChannel & channel);
		aalError Init(Source * instance, const aalChannel & channel);
		aalError Clean();
		aalError SetVolume(const aalFloat & volume);
		aalError SetPitch(const aalFloat & pitch);
		aalError SetPan(const aalFloat & pan);
		aalError SetPosition(const aalVector & position);
		aalError SetVelocity(const aalVector & velocity);
		aalError SetDirection(const aalVector & direction);
		aalError SetCone(const aalCone & cone);
		aalError SetFalloff(const aalFalloff & falloff);
		aalError SetMixer(const aalSLong & mixer_handle);
		aalError SetEnvironment(const aalSLong & environment_handle);
		
		//Status
		aalError GetPosition(aalVector & position) const;
		aalError GetFalloff(aalFalloff & falloff) const;
		aalError GetMixer(aalSLong & mixer_handle);
		aalError GetEnvironment(aalSLong & environment_handle);
		aalError GetStatistics(aalFloat & average_volume, aalFloat & deviance) const;
		aalUBool IsIdled();
		aalUBool IsPlaying();
		aalULong Time(const aalUnit & unit = AAL_UNIT_MS);
		
		// Control
		aalError Play(const aalULong & play_count = 1);
		aalError Stop();
		aalError Pause();
		aalError Resume();
		aalError Update();
		void UpdateStreaming();
		aalUBool IsTooFar();
		
		// Data
		aalSLong id;
		Sample * sample;
		aalChannel channel;
		aalULong status;
		aalULong callb_i; // Next callback index
		aalULong loop; // Remaining loop count
		aalULong time; // Elapsed 'time'
		Stream * stream;
		aalULong read, write; // Streaming status
		aalULong size; // Buffer size
		LPDIRECTSOUNDBUFFER lpdsb;
		LPDIRECTSOUND3DBUFFER lpds3db;
		LPKSPROPERTYSET lpeax;
		
	};
	
} // namespace audio

#endif // ARX_AUDIO_AUDIOINSTANCE_H
