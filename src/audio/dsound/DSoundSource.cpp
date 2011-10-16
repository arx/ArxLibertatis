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

#include "audio/dsound/DSoundSource.h"

#include <cstdio>

#include "audio/dsound/eax.h"
#include "audio/dsound/DSoundBackend.h"
#include "audio/AudioGlobal.h"
#include "audio/Stream.h"
#include "audio/Sample.h"
#include "audio/Mixer.h"
#include "io/log/Logger.h"

const GUID DSPROPSETID_EAX20_BufferProperties = { 0x306a6a7, 0xb224, 0x11d2, { 0x99, 0xe5, 0x0, 0x0, 0xe8, 0xd8, 0xc7, 0x22 } };
const GUID DSPROPSETID_EAX20_ListenerProperties = { 0x306a6a8, 0xb224, 0x11d2, { 0x99, 0xe5, 0x0, 0x0, 0xe8, 0xd8, 0xc7, 0x22 } };
const GUID CLSID_EAXDirectSound = { 0x4ff53b81, 0x1ce0, 0x11d3, { 0xaa, 0xb8, 0x0, 0xa0, 0xc9, 0x59, 0x49, 0xd5 } };

namespace audio {

DSoundSource::DSoundSource(Sample * _sample, DSoundBackend * _backend) :
	Source(_sample), tooFar(false), loop(0),
	stream(NULL), read(0), write(0), size(0),
	lpdsb(NULL), lpds3db(NULL), lpeax(NULL), backend(_backend) {
}

DSoundSource::~DSoundSource() {
	clean();
}

aalError DSoundSource::init(SourceId _id, const Channel & _channel) {
	
	id = _id;
	
	DSBUFFERDESC _desc;
	WAVEFORMATEX _format;
	bool streaming(false);
	
	clean();
	
	channel = _channel;
	
	memset(&_desc, 0, sizeof(DSBUFFERDESC));
	_desc.dwSize = sizeof(DSBUFFERDESC);
	_desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
	
	if(channel.flags & FLAG_VOLUME) {
		_desc.dwFlags |= DSBCAPS_CTRLVOLUME;
	}
	if(channel.flags & FLAG_PITCH) {
		_desc.dwFlags |= DSBCAPS_CTRLFREQUENCY;
	}
	if(channel.flags & FLAG_PAN) {
		_desc.dwFlags |= DSBCAPS_CTRLPAN;
	}
	
	if(channel.flags & FLAG_ANY_3D_FX) {
		_desc.dwFlags |= DSBCAPS_CTRL3D;
		_desc.dwFlags &= ~DSBCAPS_CTRLPAN;
		channel.flags &= ~FLAG_PAN;
	}
	
	_desc.lpwfxFormat = &_format;
	
	_format.nSamplesPerSec = sample->getFormat().frequency;
	_format.wBitsPerSample = (WORD)sample->getFormat().quality;
	_format.nChannels = (WORD)sample->getFormat().channels;
	_format.wFormatTag = WAVE_FORMAT_PCM;
	_format.nBlockAlign = (WORD)(sample->getFormat().channels * (sample->getFormat().quality >> 3));
	_format.nAvgBytesPerSec = _format.nBlockAlign * sample->getFormat().frequency;
	_format.cbSize = 0;
	
	// Get buffer size and determine if streaming must be enable
	if(sample->getLength() > stream_limit_bytes) {
		size = stream_limit_bytes, streaming = true;
	} else {
		size = sample->getLength();
	}
	_desc.dwBufferBytes = size;
	
	if(backend->device->CreateSoundBuffer(&_desc, &lpdsb, NULL)) {
		return AAL_ERROR_SYSTEM;
	}
	
	if(aalError error = init()) {
		return error;
	}
	
	stream = createStream(sample->getName());
	
	if(!stream) {
		return AAL_ERROR_FILEIO;
	}
	
	// Load sample data if not streamed
	if(!streaming) {
		
		if(stream->setPosition(0)) {
			return AAL_ERROR_SYSTEM;
		}
		
		DWORD cur0, cur1;
		void * ptr0, *ptr1;
		if(lpdsb->Lock(0, 0, &ptr0, &cur0, &ptr1, &cur1, DSBLOCK_ENTIREBUFFER)) {
			return AAL_ERROR_SYSTEM;
		}
		
		stream->read(ptr0, size, write);
		
		if(lpdsb->Unlock(ptr0, cur0, ptr1, cur1)) {
			return AAL_ERROR_SYSTEM;
		}
		
		if(write != size) {
			return AAL_ERROR_SYSTEM;
		}
		
		deleteStream(stream);
	}
	
	return AAL_OK;
}

aalError DSoundSource::init(SourceId _id, DSoundSource * instance, const Channel & _channel) {
	
	arx_assert(instance->sample == sample);
	
	if(instance->stream || _channel.flags != instance->channel.flags) {
		return init(id, _channel);
	}
	
	id = _id;
	
	clean();
	
	channel = _channel;
	size = instance->size;
	
	if(backend->device->DuplicateSoundBuffer(instance->lpdsb, &lpdsb)) {
		return AAL_ERROR_SYSTEM;
	}
	
	return init();
}

aalError DSoundSource::init() {
	
	setVolume(channel.volume);
	setPitch(channel.pitch);
	
	// Create 3D interface if required
	if(channel.flags & FLAG_ANY_3D_FX) {
		
		if(sample->getFormat().channels != 1) {
			// TODO(broken-assets) newer DSound versions don't supprt this
		}
		
		if(lpdsb->QueryInterface(IID_IDirectSound3DBuffer, (void **)&lpds3db)) {
			return AAL_ERROR_SYSTEM;
		}
		
		if(channel.flags & FLAG_RELATIVE && lpds3db->SetMode(DS3DMODE_HEADRELATIVE, DS3D_DEFERRED)) {
			return AAL_ERROR_SYSTEM;
		}
		
		setPosition(channel.position);
		setVelocity(channel.velocity);
		setDirection(channel.direction);
		setCone(channel.cone);
		setFalloff(channel.falloff);
		
		if(backend->hasEAX) {
			
			lpds3db->QueryInterface(IID_IKsPropertySet, (void **)&lpeax);
			
			s32 value = 0;
			lpeax->Set(DSPROPSETID_EAX_BufferProperties,
			           DSPROPERTY_EAXBUFFER_FLAGS | DSPROPERTY_EAXBUFFER_DEFERRED,
			           NULL, 0, &value, sizeof(s32));
			
			if(!backend->environment || !(channel.flags & FLAG_REVERBERATION)) {
				value = -10000;
				lpeax->Set(DSPROPSETID_EAX_BufferProperties,
				           DSPROPERTY_EAXBUFFER_ROOM | DSPROPERTY_EAXBUFFER_DEFERRED,
				           NULL, 0, &value, sizeof(s32));
				lpeax->Set(DSPROPSETID_EAX_BufferProperties,
				           DSPROPERTY_EAXBUFFER_ROOMHF | DSPROPERTY_EAXBUFFER_DEFERRED,
				           NULL, 0, &value, sizeof(s32));
			}
		}
	} else {
		setPan(channel.pan);
	}
	
	return AAL_OK;
}

aalError DSoundSource::clean() {
	
	if(lpeax) {
		lpeax->Release(), lpeax = NULL;
	}
	
	if(lpds3db) {
		lpds3db->Release(), lpds3db = NULL;
	}
	
	if(lpdsb) {
		if(checkPlaying()) {
			lpdsb->Stop();
		}
		lpdsb->Release(), lpdsb = NULL;
	}
	
	if(stream) {
		deleteStream(stream), stream = NULL;
	}
	
	status = Idle;
	
	return AAL_OK;
}

aalError DSoundSource::updateVolume() {
	
	if(!(channel.flags & FLAG_VOLUME)) {
		return AAL_ERROR_INIT;
	}
	
	const Mixer * mixer = _mixer[channel.mixer];
	float volume = mixer ? mixer->getFinalVolume() : 1.f;
	
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
	
	if(!(channel.flags & FLAG_PITCH)) {
		return AAL_ERROR_INIT;
	}
	
	channel.pitch = clamp(p, 0.1f, 2.f);
	
	if(lpdsb->SetFrequency((DWORD)(channel.pitch * sample->getFormat().frequency))) {
		return AAL_ERROR_SYSTEM;
	}
	
	return AAL_OK;
}

aalError DSoundSource::setPan(float p) {
	
	if(!(channel.flags & FLAG_PAN)) {
		return AAL_ERROR_INIT;
	}
	
	channel.pan = clamp(p, -1.f, 1.f);
	
	if(lpdsb->SetPan((LONG)(channel.pan * 10000.0F))) {
		return AAL_ERROR_SYSTEM;
	}
	
	return AAL_OK;
}

aalError DSoundSource::setPosition(const Vec3f & position) {
	
	if(!lpds3db || !(channel.flags & FLAG_POSITION)) {
		return AAL_ERROR_INIT;
	}
	
	channel.position = position;
	
	if(lpds3db->SetPosition(position.x, position.y, position.z, DS3D_DEFERRED)) {
		return AAL_ERROR_SYSTEM;
	}
	
	return AAL_OK;
}

aalError DSoundSource::setVelocity(const Vec3f & velocity) {
	
	if(!lpds3db || !(channel.flags & FLAG_VELOCITY)) {
		return AAL_ERROR_INIT;
	}
	
	channel.velocity = velocity;
	
	if(lpds3db->SetVelocity(velocity.x, velocity.y, velocity.z, DS3D_DEFERRED)) {
		return AAL_ERROR_SYSTEM;
	}
	
	return AAL_OK;
}

aalError DSoundSource::setDirection(const Vec3f & direction) {
	
	if(!lpds3db || !(channel.flags & FLAG_DIRECTION)) {
		return AAL_ERROR_INIT;
	}
	
	channel.direction = direction;
	
	if(lpds3db->SetConeOrientation(direction.x, direction.y, direction.z, DS3D_DEFERRED)) {
		return AAL_ERROR_INIT;
	}
	
	return AAL_OK;
}

aalError DSoundSource::setCone(const SourceCone & cone) {
	
	if(!lpds3db || !(channel.flags & FLAG_CONE)) {
		return AAL_ERROR_INIT;
	}
	
	channel.cone.inner_angle = cone.inner_angle;
	channel.cone.outer_angle = cone.outer_angle;
	channel.cone.outer_volume = cone.outer_volume > 1.0F ? 1.0F : cone.outer_volume < 0.0F ? 0.0F : cone.outer_volume;
	
	if(lpds3db->SetConeAngles((DWORD)channel.cone.inner_angle, (DWORD)channel.cone.outer_angle, DS3D_DEFERRED)) {
		return AAL_ERROR_SYSTEM;
	}
	
	if(lpds3db->SetConeOutsideVolume((LONG)((channel.cone.outer_volume - 1.0F) * 10000.0F), DS3D_DEFERRED)) {
		return AAL_ERROR_SYSTEM;
	}
	
	return AAL_OK;
}

aalError DSoundSource::setFalloff(const SourceFalloff & falloff) {
	
	if(!lpds3db || !(channel.flags & FLAG_FALLOFF)) {
		return AAL_ERROR_INIT;
	}
	
	channel.falloff = falloff;
	
	if(lpds3db->SetMinDistance(falloff.start, DS3D_DEFERRED)
	   || lpds3db->SetMaxDistance(falloff.end, DS3D_DEFERRED)) {
		return AAL_ERROR_SYSTEM;
	}
	
	return AAL_OK;
}

bool DSoundSource::checkPlaying() {
	
	DWORD value;
	if(lpdsb->GetStatus(&value)) {
		value = 0;
	}
	
	return value & DSBSTATUS_PLAYING ? true : false;
}

aalError DSoundSource::play(unsigned play_count) {
	
	// Enqueue _loop count if instance is already playing
	if(isPlaying()) {
		if(play_count) {
			loop += play_count;
		} else {
			loop = 0xffffffff;
		}
		lpdsb->Play(0, 0, loop || stream ? DSBPLAY_LOOPING : 0);
		return AAL_OK;
	}
	
	// Streaming: preload first segment
	if(stream) {
		
		DWORD cur0, cur1;
		void * ptr0, *ptr1;
		
		if(stream->setPosition(0)) {
			return AAL_ERROR;
		}
		
		if(lpdsb->Lock(0, size, &ptr0, &cur0, &ptr1, &cur1, DSBLOCK_ENTIREBUFFER)) {
			return AAL_ERROR_SYSTEM;
		}
		
		stream->read(ptr0, size, write);
		
		if(lpdsb->Unlock(ptr0, cur0, ptr1, cur1)) {
			return AAL_ERROR_SYSTEM;
		}
		
		if(write != size) {
			return AAL_ERROR;
		}
	}
	
	status = Playing;
	read = write = 0;
	loop = play_count - 1;
	reset();
	
	if(lpdsb->SetCurrentPosition(0)) {
		return AAL_ERROR_SYSTEM;
	}
	
	if(lpdsb->Play(0, 0, loop || stream ? DSBPLAY_LOOPING : 0)) {
		return AAL_ERROR_SYSTEM;
	}
	
	return AAL_OK;
}

aalError DSoundSource::stop() {
	
	if(status == Idle) {
		return AAL_OK;
	}
	
	if(lpdsb->Stop() || lpdsb->SetCurrentPosition(0)) {
		return AAL_ERROR_SYSTEM;
	}
	
	status = Idle;
	
	return AAL_OK;
}

aalError DSoundSource::pause() {
	
	if(status == Idle || status == Paused) {
		return AAL_OK;
	}
	
	lpdsb->Stop();
	status = Paused;
	
	return AAL_OK;
}

aalError DSoundSource::resume() {
	
	if(status != Paused) {
		return AAL_OK;
	}
	
	status = Playing;
	
	if(updateCulling()) {
		return AAL_OK;
	}
	
	if(lpdsb->Play(0, 0, loop || stream ? DSBPLAY_LOOPING : 0)) {
		return AAL_ERROR_SYSTEM;
	}
	
	return AAL_OK;
}

bool DSoundSource::updateCulling() {
	
	if(!(channel.flags & FLAG_POSITION) || !lpds3db) {
		return false;
	}
	
	float max;
	lpds3db->GetMaxDistance(&max);
	
	Vec3f listener_pos;
	if(channel.flags & FLAG_RELATIVE) {
		listener_pos.x = listener_pos.y = listener_pos.z = 0.0F;
	} else {
		D3DVECTOR pos;
		backend->listener->GetPosition(&pos);
		listener_pos.x = pos.x, listener_pos.y = pos.y, listener_pos.z = pos.z;
	}
	
	float d = dist(channel.position, listener_pos);
	
	if(tooFar) {
		if(d > max) {
			return true;
		}
		tooFar = false;
		lpdsb->Play(0, 0, loop || stream ? DSBPLAY_LOOPING : 0);
		return false;
	} else {
		if(d <= max) {
			return false;
		}
		tooFar = true;
		lpdsb->Stop();
		if(!loop) {
			stop();
			return false;
		}
		return true;
	}
}

void DSoundSource::updateStreaming() {
	
	void * ptr0, * ptr1;
	DWORD cur0, cur1;
	
	size_t to_fill = write >= read ? read + size - write : read - write;
	
	if(!lpdsb->Lock(write, to_fill, &ptr0, &cur0, &ptr1, &cur1, 0)) {
		
		if(ptr0) {
			size_t count;
			stream->read(ptr0, cur0, count);
			if(count < cur0) {
				if(loop) {
					stream->setPosition(0);
					stream->read((u8*)ptr0 + count, cur0 - count, count);
				} else {
					memset((u8*)ptr0 + count, 0, cur0 - count);
				}
			}
		}
		
		if(ptr1) {
			size_t count;
			stream->read(ptr1, cur1, count);
			if(count < cur1) {
				if(loop) {
					stream->setPosition(0);
					stream->read((u8*)ptr1 + count, cur1 - count, count);
				} else {
					memset((u8*)ptr1 + count, 0, cur1 - count);
					lpdsb->Play(0, 0, 0);
				}
			}
		}
		
		lpdsb->Unlock(ptr0, cur0, ptr1, cur1);
	}
	
	write += to_fill;
	
	if(write >= size) {
		write -= size;
	}
}

aalError DSoundSource::updateBuffers() {
	
	size_t last = read;
	DWORD pos;
	lpdsb->GetCurrentPosition(&pos, NULL);
	read = pos;
	
	if(read == last) {
		if(!checkPlaying()) {
			stop();
		}
		return AAL_OK;
	}
	
	time += read < last ? read + size - last : read - last;
	
	for(size_t curtime = time; curtime >= sample->getLength(); ) {
		curtime -= sample->getLength();
		if(!loop) {
			// make sure time doesn't advance further than the requested play count
			// otherwise the callbacks might be called too often
			time -= curtime;
			status = Idle;
			return AAL_OK;
		}
		if(!--loop && !stream) {
			lpdsb->Play(0, 0, 0);
		}
	}
	
	if(stream) {
		updateStreaming();
	}
	
	return AAL_OK;
}

} // namespace audio
