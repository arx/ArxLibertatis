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
#include "Athena_Track.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

namespace ATHENA
{

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Key setup                                                                 //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	ATHENAError Track::SetKeyStart(const SLong & k_id, const ULong & start)
	{
		if (ULong(k_id) >= key_c) return ATHENA_ERROR_HANDLE;

		key_l[k_id].start = start;

		return ATHENA_OK;
	}

	ATHENAError Track::SetKeyLoop(const SLong & k_id, const ULong & loop)
	{
		if (ULong(k_id) >= key_c) return ATHENA_ERROR_HANDLE;

		key_l[k_id].loop = loop;

		return ATHENA_OK;
	}

	ATHENAError Track::SetKeyDelay(const SLong & k_id, const ULong & min, const ULong & max)
	{
		if (ULong(k_id) >= key_c) return ATHENA_ERROR_HANDLE;

		key_l[k_id].delay_min = min;
		key_l[k_id].delay_max = max;

		return ATHENA_OK;
	}

	ATHENAError Track::SetKeyVolume(const SLong & k_id, const ATHENAKeySetting & setting)
	{
		if (ULong(k_id) >= key_c) return ATHENA_ERROR_HANDLE;

		TrackKey * key = &key_l[k_id];

		key->volume.min = setting.min;
		key->volume.max = setting.max;
		key->volume.interval = setting.interval;
		key->volume.flags = setting.flags;

		if (flags & (IS_PLAYING | IS_PAUSED) && key_i == ULong(k_id))
		{
			SLong i_id(GetInstanceID(s_id));
			key->volume.cur = (setting.min + setting.max) / 2.0F;

			if (_inst.IsValid(i_id)) _inst[i_id]->SetVolume(key->volume.cur);
		}

		return ATHENA_OK;
	}

	ATHENAError Track::SetKeyPitch(const SLong & k_id, const ATHENAKeySetting & setting)
	{
		if (ULong(k_id) >= key_c) return ATHENA_ERROR_HANDLE;

		TrackKey * key = &key_l[k_id];

		key->pitch.min = setting.min;
		key->pitch.max = setting.max;
		key->pitch.interval = setting.interval;
		key->pitch.flags = setting.flags;

		if (flags & (IS_PLAYING | IS_PAUSED) && key_i == ULong(k_id))
		{
			SLong i_id(GetInstanceID(s_id));
			key->pitch.cur = (setting.min + setting.max) / 2.0F;

			if (_inst.IsValid(i_id)) _inst[i_id]->SetPitch(key->pitch.cur);
		}

		return ATHENA_OK;
	}

	ATHENAError Track::SetKeyPan(const SLong & k_id, const ATHENAKeySetting & setting)
	{
		if (ULong(k_id) >= key_c) return ATHENA_ERROR_HANDLE;

		Track * track = &track_l[t_id];
		TrackKey * key = &key_l[k_id];

		key->pan.min = setting.min;
		key->pan.max = setting.max;
		key->pan.interval = setting.interval;
		key->pan.flags = setting.flags;

		if (flags & (IS_PLAYING | IS_PAUSED) && key_i == ULong(k_id))
		{
			SLong i_id(GetInstanceID(s_id));
			key->pan.cur = (setting.min + setting.max) / 2.0F;

			if (_inst.IsValid(i_id)) _inst[i_id]->SetPan(key->pan.cur);
		}

		return ATHENA_OK;
	}

	ATHENAError Track::SetKeyPositionX(const SLong & k_id, const ATHENAKeySetting & setting)
	{
		if (ULong(k_id) >= key_c) return ATHENA_ERROR_HANDLE;

		TrackKey * key = &key_l[k_id];

		key->x.min = setting.min;
		key->x.max = setting.max;
		key->x.interval = setting.interval;
		key->x.flags = setting.flags;

		if (flags & (IS_PLAYING | IS_PAUSED) && key_i == ULong(k_id))
		{
			SLong i_id(GetInstanceID(s_id));

			if (_inst.IsValid(i_id))
			{
				ATHENAVector position;

				_inst[i_id]->GetPosition(position);
				position.x = key->x.cur = (setting.min + setting.max) / 2.0F;
				_inst[i_id]->SetPosition(position);
			}
		}

		return ATHENA_OK;
	}

	ATHENAError Track::SetKeyPositionY(const SLong & k_id, const ATHENAKeySetting & setting)
	{
		if (ULong(k_id) >= key_c) return ATHENA_ERROR_HANDLE;

		Track * track = &track_l[t_id];
		TrackKey * key = &key_l[k_id];

		key->y.min = setting.min;
		key->y.max = setting.max;
		key->y.interval = setting.interval;
		key->y.flags = setting.flags;

		if (flags & (IS_PLAYING | IS_PAUSED) && key_i == ULong(k_id))
		{
			SLong i_id(GetInstanceID(s_id));

			if (_inst.IsValid(i_id))
			{
				ATHENAVector position;

				_inst[i_id]->GetPosition(position);
				position.y = key->y.cur = (setting.min + setting.max) / 2.0F;
				_inst[i_id]->SetPosition(position);
			}
		}

		return ATHENA_OK;
	}

	ATHENAError Track::SetKeyPositionZ(const SLong & k_id, const ATHENAKeySetting & setting)
	{
		if (ULong(k_id) >= key_c) return ATHENA_ERROR_HANDLE;

		TrackKey * key = &key_l[k_id];

		key->z.min = setting.min;
		key->z.max = setting.max;
		key->z.interval = setting.interval;
		key->z.flags = setting.flags;

		if (flags & (IS_PLAYING | IS_PAUSED) && key_i == ULong(k_id))
		{
			SLong i_id(GetInstanceID(s_id));

			if (_inst.IsValid(i_id))
			{
				ATHENAVector position;

				_inst[i_id]->GetPosition(position);
				position.z = key->z.cur = (setting.min + setting.max) / 2.0F;
				_inst[i_id]->SetPosition(position);
			}
		}

		return ATHENA_OK;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Key status                                                                //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	ATHENAError Track::GetKeyStart(const SLong & k_id, ULong & start)
	{
		if (ULong(k_id) >= key_c) return ATHENA_ERROR_HANDLE;

		start = key_l[k_id].start;

		return ATHENA_OK;
	}

	ATHENAError Track::GetKeyLoop(const SLong & k_id, ULong & loop)
	{
		if (ULong(k_id) >= key_c) return ATHENA_ERROR_HANDLE;

		loop = key_l[k_id].loop;

		return ATHENA_OK;
	}

	ATHENAError Track::GetKeyDelay(const SLong & k_id, ULong & min, ULong & max)
	{
		if (ULong(k_id) >= key_c) return ATHENA_ERROR_HANDLE;

		TrackKey * key = &key_l[k_id];

		min = key->delay_min;
		max = key->delay_max;

		return ATHENA_OK;
	}

	ATHENAError Track::GetKeyVolume(const SLong & k_id, ATHENAKeySetting & setting)
	{
		if (ULong(k_id) >= key_c) return ATHENA_ERROR_HANDLE;

		TrackKey * key = &key_l[k_id];

		setting.min = key->volume.min;
		setting.max = key->volume.max;
		setting.interval = key->volume.interval;
		setting.flags = key->volume.flags;

		return ATHENA_OK;
	}

	ATHENAError Track::GetKeyPitch(const SLong & k_id, ATHENAKeySetting & setting)
	{
		if (ULong(k_id) >= key_c) return ATHENA_ERROR_HANDLE;

		TrackKey * key = &key_l[k_id];

		setting.min = key->pitch.min;
		setting.max = key->pitch.max;
		setting.interval = key->pitch.interval;
		setting.flags = key->pitch.flags;

		return ATHENA_OK;
	}

	ATHENAError Track::GetKeyPan(const SLong & k_id, ATHENAKeySetting & setting)
	{
		if (ULong(k_id) >= key_c) return ATHENA_ERROR_HANDLE;

		TrackKey * key = &key_l[k_id];

		setting.min = key->pan.min;
		setting.max = key->pan.max;
		setting.interval = key->pan.interval;
		setting.flags = key->pan.flags;

		return ATHENA_OK;
	}

	ATHENAError Track::GetKeyPositionX(const SLong & k_id, ATHENAKeySetting & setting)
	{
		if (ULong(k_id) >= key_c) return ATHENA_ERROR_HANDLE;

		TrackKey * key = &key_l[k_id];

		setting.min = key->x.min;
		setting.max = key->x.max;
		setting.interval = key->x.interval;
		setting.flags = key->x.flags;

		return ATHENA_OK;
	}

	ATHENAError Track::GetKeyPositionY(const SLong & k_id, ATHENAKeySetting & setting)
	{
		if (ULong(k_id) >= key_c) return ATHENA_ERROR_HANDLE;

		TrackKey * key = &key_l[k_id];

		setting.min = key->y.min;
		setting.max = key->y.max;
		setting.interval = key->y.interval;
		setting.flags = key->y.flags;

		return ATHENA_OK;
	}

	ATHENAError Track::GetKeyPositionZ(const SLong & k_id, ATHENAKeySetting & setting)
	{
		if (ULong(k_id) >= key_c) return ATHENA_ERROR_HANDLE;

		TrackKey * key = &key_l[k_id];

		setting.min = key->z.min;
		setting.max = key->z.max;
		setting.interval = key->z.interval;
		setting.flags = key->z.flags;

		return ATHENA_OK;
	}

	ATHENAError Track::GetKeyLength(const SLong & k_id, ULong & length)
	{
		Float f_lgt(0.0F);
		SLong s_id;
		Sample * sample;

		if (ULong(k_id) >= key_c) return ATHENA_ERROR_HANDLE;

		s_id = GetSampleID(s_id);

		if (_sample.IsNotValid(s_id))
		{
			length = 0;
			return ATHENA_OK;
		}

		sample = _sample[s_id];

		TrackKey * key = &key_l[k_id];

		if (key->pitch.interval)
		{
			Float min, max, cur;
			Float size;

			length = sample->length;
			size = Float(length) * (key->loop + 1);

			min = key->pitch.min * sample->format.frequency * key->pitch.interval * 0.001F;
			max = key->pitch.max * sample->format.frequency * key->pitch.interval * 0.001F;

			cur = max;

			while (size > 0.0F)
			{
				f_lgt += key->pitch.interval;
				size -= cur = cur == min ? max : min;
			}

			if (size < 0.0F) f_lgt += key->pitch.interval * (size / cur);

			f_lgt *= 0.5F;
		}
		else
		{
			ULong size, loop;

			sample->GetLength(size);

			loop = (key->loop + 1) / 2;
			f_lgt = (size * loop) * (1.0F / key->pitch.max);
			loop = (key->loop + 1) - loop;
			f_lgt += (size * loop) * (1.0F / key->pitch.min);
		}

		length = key->start + (key->loop + 1) * key->delay_max;
		length += ULong(f_lgt);

		return ATHENA_OK;
	}

	ATHENAError Track::GetKeyLoopLength(const SLong & k_id, const ULong & loop_i, ULong & length)
	{
		Float f_lgt(0.0F);

		if (ULong(k_id) >= key_c || loop_i > key_l[k_id].loop) return ATHENA_ERROR_HANDLE;

		SLong s_id(GetSampleID(s_id));

		if (_sample.IsNotValid(s_id))
		{
			length = 0;
			return ATHENA_OK;
		}

		Sample * sample = _sample[s_id];
		TrackKey * key = &key_l[k_id];

		if (key->pitch.interval)
		{
			Float min, max, cur;
			Float size;

			length = sample->length; //length in bytes
			size = Float(length) * (key->loop + 1);

			min = key->pitch.min * sample->format.frequency * key->pitch.interval * 0.001F;
			max = key->pitch.max * sample->format.frequency * key->pitch.interval * 0.001F;

			cur = max;

			while (size > 0.0F)
			{
				f_lgt += key->pitch.interval;
				size -= cur = cur == min ? max : min;
			}

			if (size < 0.0F) f_lgt += key->pitch.interval * (size / cur);
		}
		else
		{
			sample->GetLength(length);

			length = ULong((loop_i & 0x00000001 ? key->pitch.max : key->pitch.min) * length);
		}

		return ATHENA_OK;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Track setup                                                               //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	ATHENAError Track::SetName(const char * _name)
	{
		Void * ptr;

		if (!strlen(_name)) free(name), name = NULL;
		else
		{
			ptr = realloc(name, strlen(_name) + 1);

			if (!ptr) return ATHENA_ERROR_MEMORY;

			name = (char *)ptr;
			strcpy(name, _name);
		}

		return ATHENA_OK;
	}

	ATHENAError Track::SetMode(const ULong & _flags)
	{
		if (_flags & ATHENA_FLAG_POSITION)
			flags |= TRACK_3D;
		else
			flags &= ~TRACK_3D;

		if (_flags & ATHENA_FLAG_REVERBERATION)
			flags |= TRACK_REVERB;
		else
			flags &= ~TRACK_REVERB;

		return ATHENA_OK;
	}

	ATHENAError Track::SetSample(const SLong & s_id)
	{
		if (_sample.IsNotValid(GetSampleID(s_id))) s_id = SFALSE;
		else s_id = s_id | 0xffff0000;

		return ATHENA_OK;
	}

	ATHENAError Track::SetKeyCount(const ULong & count)
	{
		Void * ptr;

		if (count == key_c) return ATHENA_OK;

		ptr = realloc(key_l, count * sizeof(TrackKey));

		if (count && !ptr) return ATHENA_ERROR_MEMORY;

		key_l = (TrackKey *)ptr;

		if (count > key_c)
		{
			memset(&key_l[key_c], 0, (count - key_c) * sizeof(TrackKey));

			TrackKey * key = &key_l[count];
			TrackKey * key_d = &key_l[key_c];

			while (key > key_d)
			{
				--key;
				key->volume.min = key->volume.max = key->pitch.min = key->pitch.max = ATHENA_DEFAULT_VOLUME;
			}
		}

		key_c = count;

		return ATHENA_OK;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Track status                                                              //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	ATHENAError Track::GetName(char * _name, const ULong & max_char)
	{
		if (name) strncpy(_name, name, max_char);
		else
		{
			SLong s_id(GetSampleID(s_id));

			if (_sample.IsValid(s_id)) strncpy(_name, _sample[s_id]->name, max_char);
			else *_name = 0;
		}

		return ATHENA_OK;
	}

	ATHENAError Track::GetMode(ULong & _flags)
	{
		_flags = 0;

		if (flags & TRACK_3D) _flags |= ATHENA_FLAG_POSITION;

		if (flags & TRACK_REVERB) _flags |= ATHENA_FLAG_REVERBERATION;

		return ATHENA_OK;
	}

	ATHENAError Track::GetSample(SLong & sample_id)
	{
		sample_id = s_id | 0xffff0000;

		return ATHENA_OK;
	}

	ATHENAError Track::GetLength(ULong & length)
	{
		length = 0;

		for (ULong i(0); i < key_c; i++)
		{
			ULong k_lgt;
			GetKeyLength(t_id, i, k_lgt);
			length += k_lgt;
		}

		return ATHENA_OK;
	}

	ATHENAError Track::GetKeyCount(ULong & count)
	{
		count = key_c;

		return ATHENA_OK;
	}

}//ATHENA::