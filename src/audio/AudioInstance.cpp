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

#include "audio/AudioInstance.h"

//#include "audio/eax.h"
#include "audio/AudioGlobal.h"
#include "audio/Stream.h"

#include "io/Logger.h"


// TODO find right library
// const GUID DSPROPSETID_EAX20_BufferProperties = { 0x306a6a7, 0xb224, 0x11d2, { 0x99, 0xe5, 0x0, 0x0, 0xe8, 0xd8, 0xc7, 0x22 } };
// const GUID DSPROPSETID_EAX20_ListenerProperties = { 0x306a6a8, 0xb224, 0x11d2, { 0x99, 0xe5, 0x0, 0x0, 0xe8, 0xd8, 0xc7, 0x22 } };
// const GUID CLSID_EAXDirectSound = { 0x4ff53b81, 0x1ce0, 0x11d3, { 0xaa, 0xb8, 0x0, 0xa0, 0xc9, 0x59, 0x49, 0xd5 } };

namespace ATHENA
{

	// Status flags                                                              //

	enum ATHENAInstance
	{
		IS_IDLED     = 0x00000001,
		IS_PAUSED    = 0x00000002,
		IS_TOOFAR    = 0x00000004
	};

	static aalVoid InstanceDebugLog(Instance * instance, const char * _text)
	{
		char text[256];
		aalULong _time(BytesToUnits(instance->time, instance->sample->format, AAL_UNIT_MS));

		sprintf(text, "[%03u - %03u][%02u\" %02u' %03u][%02u][%s]\n",
		        GetSampleID(instance->id), GetInstanceID(instance->id),
		        _time / 60000, _time % 60000 / 1000, _time % 1000,
		        instance->loop, _text);
		LogDebug << text;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Constructor and destructor                                                //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	Instance::Instance() :
		sample(NULL),
		status(0),
		loop(0), time(0),
		stream(NULL), size(0), read(0), write(0)
	{
		source[0] = 0;
	}

	extern  long NBREVERB;
	extern char szT[1024];
	extern bool bLog;

	Instance::~Instance()
	{
		Clean();
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Setup                                                                     //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	aalError Instance::Init(Sample * __sample, const aalChannel & _channel)
	{
		// DSBUFFERDESC _desc;
		// WAVEFORMATEX _format;
		aalUBool streaming(AAL_UFALSE);

		printf("Instance::Init no streaming\n");

		Clean();

		sample = __sample;
		sample->Catch();
		channel = _channel;

		// memset(&_desc, 0, sizeof(DSBUFFERDESC));
		// _desc.dwSize = sizeof(DSBUFFERDESC);
		// _desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2;

		// if (channel.flags & AAL_FLAG_VOLUME) _desc.dwFlags |= DSBCAPS_CTRLVOLUME;
		if (_mixer[channel.mixer]->flags & AAL_FLAG_VOLUME)
		{
			channel.flags |= AAL_FLAG_VOLUME;
			channel.volume = 1.0F;
		}

		// if (channel.flags & AAL_FLAG_PITCH) _desc.dwFlags |= DSBCAPS_CTRLFREQUENCY;
		if (_mixer[channel.mixer]->flags & AAL_FLAG_PITCH)
		{
			channel.flags |= AAL_FLAG_PITCH;
			channel.pitch = 1.0F;
		}

		// if (channel.flags & AAL_FLAG_PAN) _desc.dwFlags |= DSBCAPS_CTRLPAN;
		if (_mixer[channel.mixer]->flags & AAL_FLAG_PAN)
		{
			channel.flags |= AAL_FLAG_PAN;
			channel.pan = 0.0F;
		}

		if (channel.flags & FLAG_ANY_3D_FX)
		{
			// _desc.dwFlags |= DSBCAPS_CTRL3D;
			// _desc.dwFlags &= ~DSBCAPS_CTRLPAN;
			channel.flags &= ~AAL_FLAG_PAN;
		}

		// if (channel.flags & AAL_FLAG_BACKGROUND) _desc.dwFlags |= DSBCAPS_GLOBALFOCUS;

		// _desc.lpwfxFormat = &_format;

		// _format.nSamplesPerSec = sample->format.frequency;
		// _format.wBitsPerSample = (aalUWord)sample->format.quality;
		// _format.nChannels = (aalUWord)sample->format.channels;
		// _format.wFormatTag = WAVE_FORMAT_PCM;
		// _format.nBlockAlign = (aalUWord)(sample->format.channels * (sample->format.quality >> 3));
		// _format.nAvgBytesPerSec = _format.nBlockAlign * sample->format.frequency;
		// _format.cbSize = 0;

		// Get buffer size and determine if streaming must be enable
		if (sample->length > stream_limit_bytes)
			size = stream_limit_bytes, streaming = AAL_UTRUE;
		else
			size = sample->length;

		// _desc.dwBufferBytes = size;

		// if (device->CreateSoundBuffer(&_desc, &lpdsb, NULL)) return AAL_ERROR_SYSTEM;
		// FIXME: set the properties of the buffer based on the channel struct
		printf("before generating buffer and source\n");
		alGetError();
		alGenBuffers(1, buffer);
		int error;
		if ((error = alGetError()) != AL_NO_ERROR) {
			printf("0x%x %s\n", error, error == AL_OUT_OF_MEMORY ?  "AL_OUT_OF_MEMORY" : "AL_INVALID_VALUE");
			return AAL_ERROR_SYSTEM;
		}
		printf("before generating source\n");
		alGenSources(1, source);
		if (alGetError() != AL_NO_ERROR)
			return AAL_ERROR_SYSTEM;

		printf("generated buffer and source\n");

		

		SetVolume(channel.volume);
		SetPitch(channel.pitch);

		// Create 3D interface if required
		if (channel.flags & FLAG_ANY_3D_FX)
		{
			// if (lpdsb->QueryInterface(IID_IDirectSound3DBuffer, (aalVoid **)&lpds3db))
			// 	return AAL_ERROR_SYSTEM;

			if (channel.flags & AAL_FLAG_RELATIVE) {
				alSourcei(source[0], AL_SOURCE_RELATIVE, AL_TRUE);
				if (alGetError() != AL_NO_ERROR)
					return AAL_ERROR_SYSTEM;
			}

			SetPosition(channel.position);
			SetVelocity(channel.velocity);
			SetDirection(channel.direction);
			SetCone(channel.cone);
			SetFalloff(channel.falloff);

			if (is_reverb_present)
			{
				// lpds3db->QueryInterface(IID_IKsPropertySet, (aalVoid **)&lpeax);

				// aalSLong value(0);
				// lpeax->Set(DSPROPSETID_EAX_BufferProperties,
				//            DSPROPERTY_EAXBUFFER_FLAGS | DSPROPERTY_EAXBUFFER_DEFERRED,
				//            NULL, 0, &value, sizeof(aalSLong));

				// if (!environment || !(channel.flags & AAL_FLAG_REVERBERATION))
				// {
				// 	value = -10000;
				// 	lpeax->Set(DSPROPSETID_EAX_BufferProperties,
				// 	           DSPROPERTY_EAXBUFFER_ROOM | DSPROPERTY_EAXBUFFER_DEFERRED,
				// 	           NULL, 0, &value, sizeof(aalSLong));

				// 	lpeax->Set(DSPROPSETID_EAX_BufferProperties,
				// 	           DSPROPERTY_EAXBUFFER_ROOMHF | DSPROPERTY_EAXBUFFER_DEFERRED,
				// 	           NULL, 0, &value, sizeof(aalSLong));
				// }
			}
		}
		else SetPan(channel.pan);

		stream = CreateStream(sample->name);

		if (!stream) return AAL_ERROR_FILEIO;

		//Load sample data if not streamed
		if (!streaming)
		{
			aalULong cur0, cur1;
			aalVoid * ptr0, *ptr1;

			if (stream->SetPosition(0)) return AAL_ERROR_SYSTEM;

			//if (lpdsb->Lock(0, 0, &ptr0, &cur0, &ptr1, &cur1, DSBLOCK_ENTIREBUFFER)) return AAL_ERROR_SYSTEM;
			ptr0 = malloc(size);

			printf("pre-read\n");
			stream->Read(ptr0, size, write);

			//if (lpdsb->Unlock(ptr0, cur0, ptr1, cur1)) return AAL_ERROR_SYSTEM;
			alGetError();
			printf("channels: %d quality: %d frequency: %d\n", sample->format.channels, sample->format.quality, sample->format.frequency);
			
			alBufferData(buffer[0], AL_FORMAT_MONO16, ptr0, size, sample->format.frequency);
			// FIXME -- does the above cause a memleak?
			printf("post-read size: %d write: %d\n", size, write);
			printf("alBufferData error: 0x%x is buffer: %d\n", alGetError(), alIsBuffer(buffer[0]));

			printf("queueing buffers\n");
			alSourceQueueBuffers(source[0], 1, buffer);

			if (write != size)
				return AAL_ERROR_SYSTEM;

			DeleteStream(stream);
		}

		return AAL_OK;
	}

	aalError Instance::Init(Instance * instance, const aalChannel & _channel)
	{
		printf("Instance::Init\n");
		if (instance->stream || _channel.flags ^ instance->channel.flags)
			return Init(instance->sample, _channel);

		printf("no stream\n");

		Clean();

		sample = instance->sample;
		sample->Catch();
		channel = _channel;
		size = instance->size;

		// if (device->DuplicateSoundBuffer(instance->lpdsb, &lpdsb))
		// 	return AAL_ERROR_SYSTEM;

		buffer[0] = instance->buffer[1];

		SetVolume(channel.volume);
		SetPitch(channel.pitch);

		//Create 3D interface if required
		if (channel.flags & FLAG_ANY_3D_FX)
		{
			// if (lpdsb->QueryInterface(IID_IDirectSound3DBuffer, (aalVoid **)&lpds3db))
			// 	return AAL_ERROR_SYSTEM;

			// if (channel.flags & AAL_FLAG_RELATIVE &&
			//         lpds3db->SetMode(DS3DMODE_HEADRELATIVE, DS3D_DEFERRED))
			// 	return AAL_ERROR_SYSTEM;

			if (channel.flags & AAL_FLAG_RELATIVE) {
				alSourcei(source[0], AL_SOURCE_RELATIVE, 1);
				printf("relative\n");
				if (alGetError() != AL_NO_ERROR)
					return AAL_ERROR_SYSTEM;
				printf("no error\n");
			}

			SetPosition(channel.position);
			SetVelocity(channel.velocity);
			SetDirection(channel.direction);
			SetCone(channel.cone);
			SetFalloff(channel.falloff);

			if (is_reverb_present)
			{
				// FIXME
				// lpds3db->QueryInterface(IID_IKsPropertySet, (aalVoid **)&lpeax);

				// aalSLong value(0);
				// lpeax->Set(DSPROPSETID_EAX_BufferProperties,
				//            DSPROPERTY_EAXBUFFER_FLAGS | DSPROPERTY_EAXBUFFER_DEFERRED,
				//            NULL, 0, &value, sizeof(aalSLong));

				// if (!environment || !(channel.flags & AAL_FLAG_REVERBERATION))
				// {
				// 	value = -10000;
				// 	lpeax->Set(DSPROPSETID_EAX_BufferProperties,
				// 	           DSPROPERTY_EAXBUFFER_ROOM | DSPROPERTY_EAXBUFFER_DEFERRED,
				// 	           NULL, 0, &value, sizeof(aalSLong));

				// 	lpeax->Set(DSPROPSETID_EAX_BufferProperties,
				// 	           DSPROPERTY_EAXBUFFER_ROOMHF | DSPROPERTY_EAXBUFFER_DEFERRED,
				// 	           NULL, 0, &value, sizeof(aalSLong));
				// }
			}
		}
		else SetPan(channel.pan);

		printf("all ok\n");

		return AAL_OK;
	}

	aalError Instance::Clean()
	{
		// if (lpeax) lpeax->Release(), lpeax = NULL;

		// if (lpds3db) lpds3db->Release(), lpds3db = NULL;

		// if (lpdsb)
		// {
		// 	if (IsPlaying()) lpdsb->Stop();

		// 	lpdsb->Release(), lpdsb = NULL;
		// }

		alDeleteSources(1, source);
		alDeleteBuffers(1, buffer);

		if (stream) DeleteStream(stream);

		if (sample) sample->Release(), sample = NULL;

		status = 0;

		return AAL_OK;
	}

	aalError Instance::SetVolume(const aalFloat & v)
	{
		if (!(channel.flags & AAL_FLAG_VOLUME)) return AAL_ERROR_INIT;

		aalFloat volume(1.0F);
		const Mixer * mixer = _mixer[channel.mixer];

		channel.volume = v > 1.0F ? 1.0F : v < 0.0F ? 0.0F : v;

		while (mixer) volume *= mixer->volume, mixer = mixer->parent;

		if (volume) volume = LinearToLogVolume(volume) * channel.volume;

		aalSLong value(aalSLong((volume - 1.0F) * 10000.0F));

		// FIXME: I think OpenAL does this calculation for us

		alSourcef(source[0], AL_GAIN, volume);
		if (alGetError() != AL_NO_ERROR) return AAL_ERROR_SYSTEM;

		// if (lpdsb->SetVolume(value)) return AAL_ERROR_SYSTEM;

		// if (lpeax)
		// {
		// 	if (lpeax->Set(DSPROPSETID_EAX_SourceProperties, DSPROPERTY_EAXBUFFER_ROOM | DSPROPERTY_EAXBUFFER_DEFERRED,
		// 	               NULL, 0, &value, sizeof(aalSLong)))
		// 		return AAL_ERROR_SYSTEM;
		// }

		return AAL_OK;
	}

	aalError Instance::SetPitch(const aalFloat & p)
	{
		if (!(channel.flags & AAL_FLAG_PITCH)) return AAL_ERROR_INIT;

		float pitch = 1.f;
		const Mixer * mixer = _mixer[channel.mixer];

		channel.pitch = p;

		if (channel.pitch > 2.0F) channel.pitch = 2.0F;
		else if (channel.pitch < 0.1F) channel.pitch = 0.1F;

		while (mixer) pitch *= mixer->pitch, mixer = mixer->parent;

		if (pitch > 2.0F) pitch = 2.0F;
		else if (pitch < 0.1F) pitch = 0.1F;

		alSourcef(source[0], AL_PITCH, channel.pitch * pitch);
		if (alGetError() != AL_NO_ERROR) return AAL_ERROR_SYSTEM;

		// if (lpdsb->SetFrequency(aalULong(channel.pitch * pitch * sample->format.frequency)))
		// 	return AAL_ERROR_SYSTEM;

		return AAL_OK;
	}

	aalError Instance::SetPan(const aalFloat & p)
	{
		if (!(channel.flags & AAL_FLAG_PAN)) return AAL_ERROR_INIT;

		channel.pan = p > 1.0F ? 1.0F : p < -1.0F ? -1.0F : p;

		// FIXME -- OpenAL doesn't seem to have a pan feature
		// if (lpdsb->SetPan(aalSLong(channel.pan * 10000.0F))) return AAL_ERROR_SYSTEM;

		return AAL_OK;
	}

	aalError Instance::SetPosition(const aalVector & position)
	{
		//if (!lpds3db || !(channel.flags & AAL_FLAG_POSITION)) return AAL_ERROR_INIT;
		if (!alIsSource(source[0]) || !channel.flags & AAL_FLAG_POSITION)
			return AAL_ERROR_INIT;

		channel.position = position;

		const ALfloat pos[3] = {position.x, position.y, position.z};

		alSourcefv(source[0], AL_POSITION, pos);

		if (alGetError() != AL_NO_ERROR)
			return AAL_ERROR_SYSTEM;

		//if (lpds3db->SetPosition(position.x, position.y, position.z, DS3D_DEFERRED))
		//return AAL_ERROR_SYSTEM;

		return AAL_OK;
	}

	aalError Instance::SetVelocity(const aalVector & velocity)
	{
		if (!alIsSource(source[0]) || !(channel.flags & AAL_FLAG_VELOCITY)) return AAL_ERROR_INIT;

		channel.velocity = velocity;

		alSource3f(source[0], AL_VELOCITY, velocity.x, velocity.y, velocity.z);

		if (alGetError() != AL_NO_ERROR)
			return AAL_ERROR_SYSTEM;

		return AAL_OK;
	}

	aalError Instance::SetDirection(const aalVector & direction)
	{
		if (!alIsSource(source[0]) || !(channel.flags & AAL_FLAG_DIRECTION)) return AAL_ERROR_INIT;

		channel.direction = direction;

		alSource3f(source[0], AL_DIRECTION, direction.x, direction.y, direction.z);
		if (alGetError() != AL_NO_ERROR)
			return AAL_ERROR_SYSTEM;

		return AAL_OK;
	}

	aalError Instance::SetCone(const aalCone & cone)
	{
		if (!alIsSource(source[0]) || !(channel.flags & AAL_FLAG_CONE)) return AAL_ERROR_INIT;

		channel.cone.inner_angle = cone.inner_angle;
		channel.cone.outer_angle = cone.outer_angle;
		channel.cone.outer_volume = cone.outer_volume > 1.0F ? 1.0F : cone.outer_volume < 0.0F ? 0.0F : cone.outer_volume;

		// if (lpds3db->SetConeAngles(aalULong(channel.cone.inner_angle), aalULong(channel.cone.outer_angle), DS3D_DEFERRED))
		// 	return AAL_ERROR_SYSTEM;

		// if (lpds3db->SetConeOutsideVolume(aalSLong((channel.cone.outer_volume - 1.0F) * 10000.0F), DS3D_DEFERRED))
		// 	return AAL_ERROR_SYSTEM;

		
		alSourcef(source[0], AL_CONE_INNER_ANGLE, channel.cone.inner_angle);
		alSourcef(source[0], AL_CONE_OUTER_ANGLE, channel.cone.outer_angle);
		alSourcef(source[0], AL_CONE_OUTER_GAIN, channel.cone.outer_volume);

		if (alGetError() != AL_NO_ERROR)
			return AAL_ERROR_SYSTEM;

		return AAL_OK;
	}

	aalError Instance::SetFalloff(const aalFalloff & falloff)
	{
		if (!alIsSource(source[0]) || !(channel.flags & AAL_FLAG_FALLOFF)) return AAL_ERROR_INIT;

		channel.falloff = falloff;

		// if (lpds3db->SetMinDistance(falloff.start, DS3D_DEFERRED) ||
		//         lpds3db->SetMaxDistance(falloff.end, DS3D_DEFERRED))
		// 	return AAL_ERROR_SYSTEM;

		alSourcef(source[0], AL_MAX_DISTANCE, falloff.end);
		// FIXME -- OpenAL doesn't have a AL_MIN_DISTANCE
		if (alGetError() != AL_NO_ERROR)
			return AAL_ERROR_SYSTEM;

		return AAL_OK;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Status                                                                    //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////

	aalError Instance::GetStatistics(aalFloat & av_vol, aalFloat & av_dev) const
	{
		// aalULong pos, length(0);
		// aalULong cur0, cur1;
		// aalVoid * ptr0, *ptr1;

		// av_vol = av_dev = 0.0F;

		// if (lpdsb->GetCurrentPosition(&pos, NULL)) return AAL_ERROR_SYSTEM;

		// if (pos > 150) pos -= 150;

		// if (stream) length = write < pos ? write + size : write;
		// else length = sample->length;

		// length -= pos;

		// aalULong sec(256);

		// if (length > sec) length = sec;

		// if (lpdsb->Lock(pos, length, &ptr0, &cur0, &ptr1, &cur1, 0)) return AAL_ERROR_SYSTEM;

		// length >>= 1;

		// if (ptr0)
		// {
		// 	aalUWord * ptr = (aalUWord *)ptr0 + (cur0 >> 1);

		// 	while (ptr > ptr0) av_vol += *(--ptr);
		// }

		// if (ptr1)
		// {
		// 	aalUWord * ptr = (aalUWord *)ptr1 + (cur1 >> 1);

		// 	while (ptr > ptr1) av_vol += *(--ptr);
		// }

		// av_vol /= length;
		// av_vol /= 65535.0F;

		// if (ptr0)
		// {
		// 	aalFloat dev;
		// 	aalUWord * ptr = (aalUWord *)ptr0 + (cur0 >> 1);

		// 	while (ptr > ptr0)
		// 	{
		// 		dev = aalFloat(*(--ptr)) / 65535.0F - av_vol;
		// 		dev *= dev;
		// 		av_dev += dev;
		// 	}
		// }

		// if (ptr1)
		// {
		// 	aalFloat dev;
		// 	aalUWord * ptr = (aalUWord *)ptr1 + (cur1 >> 1);

		// 	while (ptr > ptr1)
		// 	{
		// 		dev = aalFloat(*(--ptr)) / 65535.0F - av_vol;
		// 		dev *= dev;
		// 		av_dev += dev;
		// 	}
		// }

		// av_dev /= length;
		// av_dev = (aalFloat)sqrt(av_dev);

		// lpdsb->Unlock(ptr0, cur0, ptr1, cur1);

		// LogDebug << "AVV[" << av_vol << "] - AVD[" << av_dev << "f]";

		return AAL_OK;
	}

	aalError Instance::GetPosition(aalVector & position) const
	{
		if (!alIsSource(source[0]) || !(channel.flags & AAL_FLAG_POSITION)) return AAL_ERROR_INIT;

		//if (lpds3db->GetPosition((D3DVECTOR *)&position)) return AAL_ERROR_SYSTEM;
		ALfloat pos[3];
		alGetSourcefv(source[0], AL_POSITION, pos);
		position.x = pos[0];
		position.y = pos[1];
		position.z = pos[2];

		return AAL_OK;
	}

	aalError Instance::GetFalloff(aalFalloff & falloff) const
	{
		falloff = channel.falloff;
		return AAL_OK;
	}

	aalUBool Instance::IsPlaying()
	{
		ALint value;

		//if (lpdsb->GetStatus(&value)) value = 0;
		alGetSourcei(source[0], AL_SOURCE_STATE, &value);

		return value == AL_PLAYING ? AAL_UTRUE : AAL_UFALSE;
	}

	aalUBool Instance::IsIdled()
	{
		return status & IS_IDLED ? AAL_UTRUE : AAL_UFALSE;
	}

	aalULong Instance::Time(const aalUnit & unit)
	{
		return BytesToUnits(time, sample->format, unit);
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Control                                                                   //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	aalError Instance::Play(const aalULong & play_count)
	{
		printf("Instance::Play\n");
		//Enqueue _loop count if instance is already playing
		if (IsPlaying())
		{
			if (play_count) loop += play_count;
			else loop = 0xffffffff;

			//lpdsb->Play(0, 0, loop || stream ? DSBPLAY_LOOPING : 0);
			// FIXME -- can only toggle looping, no way to set number of repeats
			alSourcePlay(source[0]);

			InstanceDebugLog(this, "QUEUED");

			return AAL_OK;
		}

		//Streaming : preload first segment
		if (stream)
		{
			aalULong cur0, cur1;
			aalVoid * ptr0, *ptr1;

			if (stream->SetPosition(0)) return AAL_ERROR;

			//if (lpdsb->Lock(0, size, &ptr0, &cur0, &ptr1, &cur1, DSBLOCK_ENTIREBUFFER)) return AAL_ERROR_SYSTEM;
			ptr0 = malloc(size);

			stream->Read(ptr0, size, write);

			//if (lpdsb->Unlock(ptr0, cur0, ptr1, cur1)) return AAL_ERROR_SYSTEM;
			alBufferData(buffer[0], AL_FORMAT_STEREO16, ptr0, size, sample->format.frequency);
			alSourceQueueBuffers(source[0], 1, buffer);
			int error;
			if ((error = alGetError()) != AL_NO_ERROR) {
				printf("error: 0x%x as buffer: %d\n", error, alIsBuffer(buffer[0]));
			}
			// FIXME -- does the above cause a memleak?

			//if (lpdsb->Unlock(ptr0, cur0, ptr1, cur1)) return AAL_ERROR_SYSTEM;

			if (write != size) {
				printf("write != size\n");
				return AAL_ERROR;
			}
		}

		status &= ~IS_PAUSED;
		time = read = write = 0;
		loop = play_count - 1;
		callb_i = channel.flags & AAL_FLAG_CALLBACK ? 0 : 0xffffffff;

		//if (lpdsb->SetCurrentPosition(0)) return AAL_ERROR_SYSTEM;
		alSourcei(source[0], AL_SEC_OFFSET, 0);

		// if (lpdsb->Play(0, 0, loop || stream ? DSBPLAY_LOOPING : 0))
		// 	return AAL_ERROR_SYSTEM;

		int error;
		alGetError(); // Clear error
		alSourcePlay(source[0]);
		if ((error = alGetError()) != AL_NO_ERROR) {
			printf("error = 0x%x\n", error);
			return AAL_ERROR_SYSTEM;
		}
		
		InstanceDebugLog(this, "STARTED");

		return AAL_OK;
	}

	aalError Instance::Stop()
	{
		if (status & IS_IDLED) return AAL_OK;
		int error;
		alGetError(); // Clear error
		int error;
		alGetError(); // Clear error

		InstanceDebugLog(this, "STOPPED");

		//if (lpdsb->Stop() || lpdsb->SetCurrentPosition(0)) return AAL_ERROR_SYSTEM;
		alSourceStop(source[0]);
		alSourcei(source[0], AL_SEC_OFFSET, 0);
		if (alGetError() != AL_NO_ERROR)
			return AAL_ERROR_SYSTEM;

		status &= ~IS_PAUSED;
		status |= IS_IDLED;

		return AAL_OK;
	}

	aalError Instance::Pause()
	{
		if (status & IS_IDLED) return AAL_OK;

		InstanceDebugLog(this, "PAUSED");

		//lpdsb->Stop();
		alSourcePause(source[0]);
		status |= IS_PAUSED;

		return AAL_OK;
	}

	aalError Instance::Resume()
	{
		if (status & IS_IDLED) return AAL_OK;

		InstanceDebugLog(this, "RESUMED");

		status &= ~IS_PAUSED;

		if (channel.flags & AAL_FLAG_POSITION && alIsSource(source[0]) && IsTooFar())
			return AAL_OK;

		// if (lpdsb->Play(0, 0, loop || stream ? DSBPLAY_LOOPING : 0))
		// 	return AAL_ERROR_SYSTEM;
		alSourcePlay(source[0]);
		if (alGetError() != AL_NO_ERROR)
			return AAL_ERROR_SYSTEM;

		return AAL_OK;
	}

	static inline aalFloat Distance(const aalVector & from, const aalVector & to)
	{
		aalFloat x, y, z;

		x = from.x - to.x;
		y = from.y - to.y;
		z = from.z - to.z;

		return aalFloat(sqrt(x * x + y * y + z * z));
	}

	aalUBool Instance::IsTooFar()
	{
		aalFloat dist, max;
		ALfloat _listener_pos[3];
		aalVector listener_pos;

		//lpds3db->GetMaxDistance(&max);
		alGetSourcef(source[0], AL_MAX_DISTANCE, &max);

		if (channel.flags & AAL_FLAG_RELATIVE)
			_listener_pos[0] = _listener_pos[1] = _listener_pos[2] = 0.0F;
		else
			alGetListenerfv(AL_POSITION, _listener_pos);

		listener_pos.x = _listener_pos[0];
		listener_pos.y = _listener_pos[1];
		listener_pos.z = _listener_pos[2];

		dist = Distance(listener_pos, channel.position);
		printf("distance is %f max is %f\n", dist, max);
		printf("status: 0x%x IS_TOOFAR: 0x%x\n", status, IS_TOOFAR);

		if (status & IS_TOOFAR)
		{
			printf("not too far\n");
			if (dist > max) return AAL_UTRUE;

			status &= ~IS_TOOFAR;
			//lpdsb->Play(0, 0, loop || stream ? DSBPLAY_LOOPING : 0);
			int error;
			alGetError();
			printf("playing 1\n");
			alSourcePlay(source[0]);
			if ((error = alGetError()) != AL_NO_ERROR) {
				printf("error: 0x%x\n", error);
				exit(0);
			}

			return AAL_UFALSE;
		}
		else
		{
			if (dist <= max) return AAL_UFALSE;

			status |= IS_TOOFAR;
			printf("status %d\n", status);
			// lpdsb->Stop();
			alSourceStop(source[0]);
		}

		return AAL_UTRUE;
	}

	aalVoid Instance::UpdateStreaming()
	{
		aalVoid * ptr0, *ptr1;
		aalULong cur0, cur1;
		aalULong to_fill, count;

		printf("Instance::UpdateStreaming\n");

		InstanceDebugLog(this, "STREAMED");

		to_fill = write >= read ? read + size - write : read - write;

		// FIXME TODO

		// if (!lpdsb->Lock(write, to_fill, &ptr0, &cur0, &ptr1, &cur1, 0))
		// {
		// 	if (ptr0)
		// 	{
		// 		stream->Read(ptr0, cur0, count);

		// 		if (count < cur0)
		// 		{
		// 			if (loop)
		// 			{
		// 				stream->SetPosition(0);
		// 				stream->Read((aalUByte *)ptr0 + count, cur0 - count, count);
		// 			}
		// 			else
		// 			{
		// 				memset((aalUByte *)ptr0 + count, 0, cur0 - count);
		// 			}
		// 		}
		// 	}

		// 	if (ptr1)
		// 	{
		// 		stream->Read(ptr1, cur1, count);

		// 		if (count < cur1)
		// 		{
		// 			if (loop)
		// 			{
		// 				stream->SetPosition(0);
		// 				stream->Read((aalUByte *)ptr1 + count, cur1 - count, count);
		// 			}
		// 			else
		// 			{
		// 				memset((aalUByte *)ptr1 + count, 0, cur1 - count);
		// 				lpdsb->Play(0, 0, 0);
		// 			}
		// 		}
		// 	}

		// 	lpdsb->Unlock(ptr0, cur0, ptr1, cur1);
		// }

		write += to_fill;

		if (write >= size) write -= size;
	}

	aalError Instance::Update()
	{
		aalULong last;

		printf("Instance::Update\n");

		if (status & (IS_IDLED | IS_PAUSED)) return AAL_OK;

		printf("not idled or paused\n");

		if (channel.flags & AAL_FLAG_POSITION && alIsSource(source[0]) && IsTooFar())
		{
			if (! this->loop)
			{
				this->Stop();
			}
			else
			{
				return AAL_OK;
			}
		}

		last = read;
		printf("last: %d read: %d\n", last, read);
		//lpdsb->GetCurrentPosition(&read, NULL);
		alGetSourcei(source[0], AL_BYTE_OFFSET, (ALint *)&read);

		if (read == last)
		{
			if (!IsPlaying())
			{
				printf("stopping\n");
				Stop();
			}

			return AAL_OK;
		}

		time += read < last ? read + size - last : read - last;
		printf("time: %d\n", time);

		//Check if's time to launch a callback
		if (callb_i < sample->callb_c && sample->callb[callb_i].time <= time)
		{
			sample->callb[callb_i].func(this, id, sample->callb[callb_i].data);
			callb_i++;
		}

		if (time >= sample->length)
		{
			if (loop)
			{
				InstanceDebugLog(this, "LOOPED");

				if (!--loop && !stream) {
					printf("playing 2\n");
					alSourcePlay(source[0]);
					int error;
					if ((error = alGetError()) != AL_NO_ERROR) {
						printf("error: 0x%x\n", error);
						exit(0);
					}
				}
			}
			else
			{
				InstanceDebugLog(this, "IDLED");

				status |= IS_IDLED;
				return AAL_OK;
			}

			if (channel.flags & AAL_FLAG_CALLBACK) callb_i = 0;

			time -= sample->length;
		}

		if (stream) UpdateStreaming();

		return AAL_OK;
	}

}//ATHENA::
