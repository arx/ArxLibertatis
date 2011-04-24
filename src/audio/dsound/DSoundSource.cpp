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

#include "audio/dsound/DSoundSource.h"

#include <cstdio>

#include "audio/dsound/eax.h"
#include "audio/dsound/DSoundBackend.h"

#include "audio/AudioGlobal.h"
#include "audio/Stream.h"

#include "io/Logger.h"


// TODO find right library
const GUID DSPROPSETID_EAX20_BufferProperties = { 0x306a6a7, 0xb224, 0x11d2, { 0x99, 0xe5, 0x0, 0x0, 0xe8, 0xd8, 0xc7, 0x22 } };
const GUID DSPROPSETID_EAX20_ListenerProperties = { 0x306a6a8, 0xb224, 0x11d2, { 0x99, 0xe5, 0x0, 0x0, 0xe8, 0xd8, 0xc7, 0x22 } };
const GUID CLSID_EAXDirectSound = { 0x4ff53b81, 0x1ce0, 0x11d3, { 0xaa, 0xb8, 0x0, 0xa0, 0xc9, 0x59, 0x49, 0xd5 } };

namespace audio {

	DSoundSource::DSoundSource(Sample * _sample, DSoundBackend * _backend) :
		Source(_sample),
		tooFar(false),
		loop(0), time(0),
		stream(NULL), read(0), write(0), size(0),
		lpdsb(NULL), lpds3db(NULL), lpeax(NULL),
		backend(_backend)
	{
	}

	extern char szT[1024];
	extern bool bLog;

	DSoundSource::~DSoundSource() {
		clean();
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Setup                                                                     //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	aalError DSoundSource::init(aalSLong _id, const aalChannel & _channel)
	{
		
		id = _id;
		
		DSBUFFERDESC _desc;
		WAVEFORMATEX _format;
		aalUBool streaming(AAL_UFALSE);

		clean();

		channel = _channel;

		memset(&_desc, 0, sizeof(DSBUFFERDESC));
		_desc.dwSize = sizeof(DSBUFFERDESC);
		_desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2;

		if (channel.flags & AAL_FLAG_VOLUME) _desc.dwFlags |= DSBCAPS_CTRLVOLUME;
		else if (_mixer[channel.mixer]->flags & AAL_FLAG_VOLUME)
		{
			channel.flags |= AAL_FLAG_VOLUME;
			channel.volume = 1.0F;
		}

		if (channel.flags & AAL_FLAG_PITCH) _desc.dwFlags |= DSBCAPS_CTRLFREQUENCY;
		else if (_mixer[channel.mixer]->flags & AAL_FLAG_PITCH)
		{
			channel.flags |= AAL_FLAG_PITCH;
			channel.pitch = 1.0F;
		}

		if (channel.flags & AAL_FLAG_PAN) _desc.dwFlags |= DSBCAPS_CTRLPAN;
		else if (_mixer[channel.mixer]->flags & AAL_FLAG_PAN)
		{
			channel.flags |= AAL_FLAG_PAN;
			channel.pan = 0.0F;
		}

		if (channel.flags & FLAG_ANY_3D_FX)
		{
			_desc.dwFlags |= DSBCAPS_CTRL3D;
			_desc.dwFlags &= ~DSBCAPS_CTRLPAN;
			channel.flags &= ~AAL_FLAG_PAN;
		}

		if (channel.flags & AAL_FLAG_BACKGROUND) _desc.dwFlags |= DSBCAPS_GLOBALFOCUS;

		_desc.lpwfxFormat = &_format;

		_format.nSamplesPerSec = sample->format.frequency;
		_format.wBitsPerSample = (aalUWord)sample->format.quality;
		_format.nChannels = (aalUWord)sample->format.channels;
		_format.wFormatTag = WAVE_FORMAT_PCM;
		_format.nBlockAlign = (aalUWord)(sample->format.channels * (sample->format.quality >> 3));
		_format.nAvgBytesPerSec = _format.nBlockAlign * sample->format.frequency;
		_format.cbSize = 0;

		// Get buffer size and determine if streaming must be enable
		if (sample->length > stream_limit_bytes)
			size = stream_limit_bytes, streaming = AAL_UTRUE;
		else
			size = sample->length;

		_desc.dwBufferBytes = size;

		if(backend->device->CreateSoundBuffer(&_desc, &lpdsb, NULL)) return AAL_ERROR_SYSTEM;

		setVolume(channel.volume);
		setPitch(channel.pitch);

		// Create 3D interface if required
		if (channel.flags & FLAG_ANY_3D_FX)
		{
			if (lpdsb->QueryInterface(IID_IDirectSound3DBuffer, (void **)&lpds3db))
				return AAL_ERROR_SYSTEM;

			if (channel.flags & AAL_FLAG_RELATIVE &&
			        lpds3db->SetMode(DS3DMODE_HEADRELATIVE, DS3D_DEFERRED))
				return AAL_ERROR_SYSTEM;

			setPosition(channel.position);
			setVelocity(channel.velocity);
			setDirection(channel.direction);
			setCone(channel.cone);
			setFalloff(channel.falloff);

			if (backend->hasEAX)
			{
				lpds3db->QueryInterface(IID_IKsPropertySet, (void **)&lpeax);

				aalSLong value(0);
				lpeax->Set(DSPROPSETID_EAX_BufferProperties,
				           DSPROPERTY_EAXBUFFER_FLAGS | DSPROPERTY_EAXBUFFER_DEFERRED,
				           NULL, 0, &value, sizeof(aalSLong));

				if (!backend->environment || !(channel.flags & AAL_FLAG_REVERBERATION))
				{
					value = -10000;
					lpeax->Set(DSPROPSETID_EAX_BufferProperties,
					           DSPROPERTY_EAXBUFFER_ROOM | DSPROPERTY_EAXBUFFER_DEFERRED,
					           NULL, 0, &value, sizeof(aalSLong));

					lpeax->Set(DSPROPSETID_EAX_BufferProperties,
					           DSPROPERTY_EAXBUFFER_ROOMHF | DSPROPERTY_EAXBUFFER_DEFERRED,
					           NULL, 0, &value, sizeof(aalSLong));
				}
			}
		}
		else setPan(channel.pan);

		stream = CreateStream(sample->name);

		if (!stream) return AAL_ERROR_FILEIO;

		//Load sample data if not streamed
		if (!streaming)
		{
			aalULong cur0, cur1;
			void * ptr0, *ptr1;

			if (stream->SetPosition(0)) return AAL_ERROR_SYSTEM;

			if (lpdsb->Lock(0, 0, &ptr0, &cur0, &ptr1, &cur1, DSBLOCK_ENTIREBUFFER)) return AAL_ERROR_SYSTEM;

			stream->Read(ptr0, size, write);

			if (lpdsb->Unlock(ptr0, cur0, ptr1, cur1)) return AAL_ERROR_SYSTEM;

			if (write != size)
				return AAL_ERROR_SYSTEM;

			DeleteStream(stream);
		}

		return AAL_OK;
	}

	aalError DSoundSource::init(aalSLong _id, DSoundSource * instance, const aalChannel & _channel)
	{
		
		arx_assert(instance->sample == sample);
		
		if (instance->stream || _channel.flags ^ instance->channel.flags)
			return init(id, _channel);

		id = _id;
		
		clean();

		channel = _channel;
		size = instance->size;

		if (backend->device->DuplicateSoundBuffer(instance->lpdsb, &lpdsb))
			return AAL_ERROR_SYSTEM;

		setVolume(channel.volume);
		setPitch(channel.pitch);

		//Create 3D interface if required
		if (channel.flags & FLAG_ANY_3D_FX)
		{
			if (lpdsb->QueryInterface(IID_IDirectSound3DBuffer, (void **)&lpds3db))
				return AAL_ERROR_SYSTEM;

			if (channel.flags & AAL_FLAG_RELATIVE &&
			        lpds3db->SetMode(DS3DMODE_HEADRELATIVE, DS3D_DEFERRED))
				return AAL_ERROR_SYSTEM;

			setPosition(channel.position);
			setVelocity(channel.velocity);
			setDirection(channel.direction);
			setCone(channel.cone);
			setFalloff(channel.falloff);

			if (backend->hasEAX)
			{
				lpds3db->QueryInterface(IID_IKsPropertySet, (void **)&lpeax);

				aalSLong value(0);
				lpeax->Set(DSPROPSETID_EAX_BufferProperties,
				           DSPROPERTY_EAXBUFFER_FLAGS | DSPROPERTY_EAXBUFFER_DEFERRED,
				           NULL, 0, &value, sizeof(aalSLong));

				if (!backend->environment || !(channel.flags & AAL_FLAG_REVERBERATION))
				{
					value = -10000;
					lpeax->Set(DSPROPSETID_EAX_BufferProperties,
					           DSPROPERTY_EAXBUFFER_ROOM | DSPROPERTY_EAXBUFFER_DEFERRED,
					           NULL, 0, &value, sizeof(aalSLong));

					lpeax->Set(DSPROPSETID_EAX_BufferProperties,
					           DSPROPERTY_EAXBUFFER_ROOMHF | DSPROPERTY_EAXBUFFER_DEFERRED,
					           NULL, 0, &value, sizeof(aalSLong));
				}
			}
		}
		else setPan(channel.pan);

		return AAL_OK;
	}

	aalError DSoundSource::clean()
	{
		if (lpeax) lpeax->Release(), lpeax = NULL;

		if (lpds3db) lpds3db->Release(), lpds3db = NULL;

		if (lpdsb)
		{
			if (checkPlaying()) lpdsb->Stop();

			lpdsb->Release(), lpdsb = NULL;
		}

		if (stream) DeleteStream(stream);

		status = IDLE;

		return AAL_OK;
	}

aalError DSoundSource::setVolume(float v) {
	
	if(!(channel.flags & AAL_FLAG_VOLUME)) {
		return AAL_ERROR_INIT;
	}
	
	channel.volume = v > 1.0F ? 1.0F : v < 0.0F ? 0.0F : v;
	
	return updateVolume();
}

aalError DSoundSource::updateVolume() {
	
	if(!(channel.flags & AAL_FLAG_VOLUME)) {
		return AAL_ERROR_INIT;
	}
	
	aalFloat volume = 1.f;
	
	const Mixer * mixer = _mixer[channel.mixer];
	while(mixer) {
		volume *= mixer->volume, mixer = mixer->parent;
	}
	
	if(volume) {
		volume = LinearToLogVolume(volume) * channel.volume;
	}
	
	LONG value = (LONG)((volume - 1.0F) * 10000.0F);
	
	if(FAILED(lpdsb->SetVolume(value))) {
		return AAL_ERROR_SYSTEM;
	}
	
	if(lpeax) {
		if(FAILED(lpeax->Set(DSPROPSETID_EAX_SourceProperties, DSPROPERTY_EAXBUFFER_ROOM | DSPROPERTY_EAXBUFFER_DEFERRED, NULL, 0, &value, sizeof(LONG)))) {
			return AAL_ERROR_SYSTEM;
		}
	}
	
	return AAL_OK;
}

	aalError DSoundSource::setPitch(float p) {
		if (!(channel.flags & AAL_FLAG_PITCH)) return AAL_ERROR_INIT;

		float pitch = 1.f;
		const Mixer * mixer = _mixer[channel.mixer];

		channel.pitch = p;

		if (channel.pitch > 2.0F) channel.pitch = 2.0F;
		else if (channel.pitch < 0.1F) channel.pitch = 0.1F;

		while (mixer) pitch *= mixer->pitch, mixer = mixer->parent;

		if (pitch > 2.0F) pitch = 2.0F;
		else if (pitch < 0.1F) pitch = 0.1F;

		if (lpdsb->SetFrequency(aalULong(channel.pitch * pitch * sample->format.frequency)))
			return AAL_ERROR_SYSTEM;

		return AAL_OK;
	}

	aalError DSoundSource::setPan(float p) {
		
		if (!(channel.flags & AAL_FLAG_PAN)) return AAL_ERROR_INIT;

		channel.pan = p > 1.0F ? 1.0F : p < -1.0F ? -1.0F : p;

		if (lpdsb->SetPan(aalSLong(channel.pan * 10000.0F))) return AAL_ERROR_SYSTEM;

		return AAL_OK;
	}

	aalError DSoundSource::setPosition(const aalVector & position)
	{
		if (!lpds3db || !(channel.flags & AAL_FLAG_POSITION)) return AAL_ERROR_INIT;

		channel.position = position;

		if (lpds3db->SetPosition(position.x, position.y, position.z, DS3D_DEFERRED))
			return AAL_ERROR_SYSTEM;

		return AAL_OK;
	}

	aalError DSoundSource::setVelocity(const aalVector & velocity)
	{
		if (!lpds3db || !(channel.flags & AAL_FLAG_VELOCITY)) return AAL_ERROR_INIT;

		channel.velocity = velocity;

		if (lpds3db->SetVelocity(velocity.x, velocity.y, velocity.z, DS3D_DEFERRED))
			return AAL_ERROR_SYSTEM;

		return AAL_OK;
	}

	aalError DSoundSource::setDirection(const aalVector & direction)
	{
		if (!lpds3db || !(channel.flags & AAL_FLAG_DIRECTION)) return AAL_ERROR_INIT;

		channel.direction = direction;

		if (lpds3db->SetConeOrientation(direction.x, direction.y, direction.z, DS3D_DEFERRED))
			return AAL_ERROR_INIT;

		return AAL_OK;
	}

	aalError DSoundSource::setCone(const aalCone & cone)
	{
		if (!lpds3db || !(channel.flags & AAL_FLAG_CONE)) return AAL_ERROR_INIT;

		channel.cone.inner_angle = cone.inner_angle;
		channel.cone.outer_angle = cone.outer_angle;
		channel.cone.outer_volume = cone.outer_volume > 1.0F ? 1.0F : cone.outer_volume < 0.0F ? 0.0F : cone.outer_volume;

		if (lpds3db->SetConeAngles(aalULong(channel.cone.inner_angle), aalULong(channel.cone.outer_angle), DS3D_DEFERRED))
			return AAL_ERROR_SYSTEM;

		if (lpds3db->SetConeOutsideVolume(aalSLong((channel.cone.outer_volume - 1.0F) * 10000.0F), DS3D_DEFERRED))
			return AAL_ERROR_SYSTEM;

		return AAL_OK;
	}

	aalError DSoundSource::setFalloff(const aalFalloff & falloff)
	{
		if (!lpds3db || !(channel.flags & AAL_FLAG_FALLOFF)) return AAL_ERROR_INIT;

		channel.falloff = falloff;

		if (lpds3db->SetMinDistance(falloff.start, DS3D_DEFERRED) ||
		        lpds3db->SetMaxDistance(falloff.end, DS3D_DEFERRED))
			return AAL_ERROR_SYSTEM;

		return AAL_OK;
	}

aalError DSoundSource::setMixer(aalSLong mixer) {
	
	channel.mixer = mixer;
	
	return updateVolume();
}

aalError DSoundSource::setEnvironment(aalSLong environment) {
	channel.environment = environment;
	return AAL_OK;
}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Status                                                                    //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////

	aalError DSoundSource::getPosition(aalVector & position) const
	{
		if (!lpds3db || !(channel.flags & AAL_FLAG_POSITION)) return AAL_ERROR_INIT;

		D3DVECTOR pos;
		if (lpds3db->GetPosition(&pos)) return AAL_ERROR_SYSTEM;
		position.x = pos.x, position.y = pos.y, position.z = pos.z;

		return AAL_OK;
	}

	aalError DSoundSource::getFalloff(aalFalloff & falloff) const
	{
		falloff = channel.falloff;
		return AAL_OK;
	}

	aalUBool DSoundSource::checkPlaying()
	{
		aalULong value;

		if (lpdsb->GetStatus(&value)) value = 0;

		return value & DSBSTATUS_PLAYING ? AAL_UTRUE : AAL_UFALSE;
	}

	aalULong DSoundSource::getTime(aalUnit unit) const {
		return BytesToUnits(time, sample->format, unit);
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Control                                                                   //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	aalError DSoundSource::play(aalULong play_count)
	{
		//Enqueue _loop count if instance is already playing
		if (checkPlaying())
		{
			if (play_count) loop += play_count;
			else loop = 0xffffffff;

			lpdsb->Play(0, 0, loop || stream ? DSBPLAY_LOOPING : 0);

			return AAL_OK;
		}

		//Streaming : preload first segment
		if (stream)
		{
			aalULong cur0, cur1;
			void * ptr0, *ptr1;

			if (stream->SetPosition(0)) return AAL_ERROR;

			if (lpdsb->Lock(0, size, &ptr0, &cur0, &ptr1, &cur1, DSBLOCK_ENTIREBUFFER)) return AAL_ERROR_SYSTEM;

			stream->Read(ptr0, size, write);

			if (lpdsb->Unlock(ptr0, cur0, ptr1, cur1)) return AAL_ERROR_SYSTEM;

			if (write != size) return AAL_ERROR;
		}

		status = PLAYING;
		time = read = write = 0;
		loop = play_count - 1;
		callb_i = channel.flags & AAL_FLAG_CALLBACK ? 0 : 0xffffffff;

		if (lpdsb->SetCurrentPosition(0)) return AAL_ERROR_SYSTEM;

		if (lpdsb->Play(0, 0, loop || stream ? DSBPLAY_LOOPING : 0))
			return AAL_ERROR_SYSTEM;

		return AAL_OK;
	}

	aalError DSoundSource::stop() {
		
		if(status == IDLE) return AAL_OK;

		if (lpdsb->Stop() || lpdsb->SetCurrentPosition(0)) return AAL_ERROR_SYSTEM;

		status = IDLE;

		return AAL_OK;
	}

	aalError DSoundSource::pause()
	{
		if (status == IDLE || status == PAUSED) return AAL_OK;

		lpdsb->Stop();
		status = PAUSED;

		return AAL_OK;
	}

	aalError DSoundSource::resume()
	{
		if (status != PAUSED) return AAL_OK;

		status = PLAYING;

		if (channel.flags & AAL_FLAG_POSITION && lpds3db && isTooFar())
			return AAL_OK;

		if (lpdsb->Play(0, 0, loop || stream ? DSBPLAY_LOOPING : 0))
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

	aalUBool DSoundSource::isTooFar()
	{
		aalVector listener_pos;
		aalFloat dist, max;

		lpds3db->GetMaxDistance(&max);

		if (channel.flags & AAL_FLAG_RELATIVE)
			listener_pos.x = listener_pos.y = listener_pos.z = 0.0F;
		else
			backend->listener->GetPosition((D3DVECTOR *)&listener_pos);

		dist = Distance(listener_pos, channel.position);

		if (tooFar)
		{
			if (dist > max) return AAL_UTRUE;

			tooFar = false;
			lpdsb->Play(0, 0, loop || stream ? DSBPLAY_LOOPING : 0);

			return AAL_UFALSE;
		}
		else
		{
			if (dist <= max) return AAL_UFALSE;

			tooFar = true;
			lpdsb->Stop();
		}

		return AAL_UTRUE;
	}

	void DSoundSource::updateStreaming()
	{
		void * ptr0, *ptr1;
		aalULong cur0, cur1;
		aalULong to_fill, count;

		to_fill = write >= read ? read + size - write : read - write;

		if (!lpdsb->Lock(write, to_fill, &ptr0, &cur0, &ptr1, &cur1, 0))
		{
			if (ptr0)
			{
				stream->Read(ptr0, cur0, count);

				if (count < cur0)
				{
					if (loop)
					{
						stream->SetPosition(0);
						stream->Read((aalUByte *)ptr0 + count, cur0 - count, count);
					}
					else
					{
						memset((aalUByte *)ptr0 + count, 0, cur0 - count);
					}
				}
			}

			if (ptr1)
			{
				stream->Read(ptr1, cur1, count);

				if (count < cur1)
				{
					if (loop)
					{
						stream->SetPosition(0);
						stream->Read((aalUByte *)ptr1 + count, cur1 - count, count);
					}
					else
					{
						memset((aalUByte *)ptr1 + count, 0, cur1 - count);
						lpdsb->Play(0, 0, 0);
					}
				}
			}

			lpdsb->Unlock(ptr0, cur0, ptr1, cur1);
		}

		write += to_fill;

		if (write >= size) write -= size;
	}

	aalError DSoundSource::update()
	{
		aalULong last;

		if (status != PLAYING) return AAL_OK;

		if(channel.flags & AAL_FLAG_POSITION && lpds3db && isTooFar()) {
			if (! this->loop)
			{
				stop();
			}
			else
			{
				return AAL_OK;
			}
		}

		last = read;
		lpdsb->GetCurrentPosition(&read, NULL);

		if (read == last)
		{
			if (!checkPlaying())
			{
				stop();
			}

			return AAL_OK;
		}

		time += read < last ? read + size - last : read - last;

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
				if (!--loop && !stream) lpdsb->Play(0, 0, 0);
			}
			else
			{
				status = IDLE;
				return AAL_OK;
			}

			if (channel.flags & AAL_FLAG_CALLBACK) callb_i = 0;

			time -= sample->length;
		}

		if (stream) updateStreaming();

		return AAL_OK;
	}

} // namespace audio
