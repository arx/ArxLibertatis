/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

Arx Fatalis Source Code is free software: you can redistribute key and/or modify key under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that key will be useful, but WITHOUT ANY WARRANTY; without even the implied 
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

#include "audio/Ambiance.h"

#include <cctype>
#include <algorithm>
#include <sstream>

#include <boost/smart_ptr/scoped_ptr.hpp>

#include "audio/AudioBackend.h"
#include "audio/AudioGlobal.h"
#include "audio/AudioResource.h"
#include "audio/AudioSource.h"
#include "audio/Mixer.h"
#include "audio/Sample.h"

#include "io/resource/ResourcePath.h"
#include "io/resource/PakReader.h"
#include "io/log/Logger.h"

#include "math/Types.h"
#include "math/Random.h"
#include "math/Vector.h"

#include "platform/Flags.h"

#include "Configure.h"

using std::string;

#if UNITY_BUILD
// GCC complains if a type from an anonymous namespace
// is used in a file that isn't the main source file
#define ANONYMOUS_NAMESPACE extern "C++"
#else
#define ANONYMOUS_NAMESPACE namespace
#endif

ANONYMOUS_NAMESPACE {

static const u32 AMBIANCE_FILE_SIGNATURE = 0x424d4147; //'GAMB'
static const u32 AMBIANCE_FILE_VERSION_1002 = 0x01000002;
static const u32 AMBIANCE_FILE_VERSION_1003 = 0x01000003;
static const u32 AMBIANCE_FILE_VERSION = AMBIANCE_FILE_VERSION_1003;

struct KeySetting {
	
	// Key settings flags
	enum KeyFlag {
		FLAG_RANDOM      = 1,
		FLAG_INTERPOLATE = 2
	};
	DECLARE_FLAGS(KeyFlag, KeyFlags)
	
	KeyFlags flags; // A set of KeySettingFlag
	float min, max; // Min and max setting values
	float from, to, cur; // Current min and max values
	unsigned interval; // Interval between updates (On Start = 0)
	int tupdate; // Last update time
	
	KeySetting()
		: flags(0), min(0), max(0), from(0), to(0), cur(0), interval(0), tupdate(0)
		{ }
	
	bool load(PakFileHandle * file) {
		
		f32 _min, _max;
		u32 _interval, _flags;
		if(!file->read(&_min, 4) ||
		   !file->read(&_max, 4) ||
		   !file->read(&_interval, 4) ||
		   !file->read(&_flags, 4)) {
			return false;
		}
		min = _min, max = _max, interval = _interval;
		flags = KeyFlags::load(_flags); // TODO save/load flags
		
		return true;
	}
	
	void reset() {
		tupdate = 0;
		if(min != max && flags & FLAG_RANDOM) {
			cur = Random::getf(min, max);
		} else {
			cur = min;
		}
		from = min;
		to = max;
	}
	
	float update(signed timez = 0) {
		
		if(min == max) {
			return cur;
		}
		
		signed elapsed = timez - tupdate;
		if(elapsed >= (signed)interval) {
			elapsed = 0;
			tupdate += interval;
			if(flags & FLAG_RANDOM) {
				from = to;
				to = Random::getf(min, max);
			} else {
				if(from == min) {
					from = max, to = min;
				} else {
					from = min, to = max;
				}
			}
			cur = from;
		}
		
		if(flags & FLAG_INTERPOLATE) {
			cur = from + float(elapsed) / interval * (to - from);
		}
		
		return cur;
	};
	
};

struct TrackKey {
	
	size_t start; // Start time (after last key)
	size_t n_start; // Next time to play sample (when delayed)
	size_t loop; // Play count
	unsigned delay_min, delay_max; // Min and max delay before each sample loop
	unsigned delay; // Current delay
	KeySetting volume; // Volume settings
	KeySetting pitch; // Pitch settings
	KeySetting pan; // Pan settings
	KeySetting x, y, z; // Positon settings
	
	TrackKey() : start(0), n_start(0), loop(0),
	             delay_min(0), delay_max(0), delay(0) {
	}
	
	bool load(PakFileHandle * file) {
		
		u32 _start, _loop, _delay_min, _delay_max, _flags;
		
		if(!file->read(&_flags, 4) ||
		   !file->read(&_start, 4) ||
		   !file->read(&_loop, 4) ||
		   !file->read(&_delay_min, 4) ||
		   !file->read(&_delay_max, 4) ||
		   !volume.load(file) ||
		   !pitch.load(file) ||
		   !pan.load(file) ||
		   !x.load(file) ||
		   !y.load(file) ||
		   !z.load(file)) {
			return false;
		}
		start = _start, loop = _loop + 1;
		delay_min = _delay_min, delay_max = _delay_max;
		
		return true;
	}
	
	void updateSynch() {
		if(delay_min != delay_max) {
			delay = delay_max - delay;
			delay += Random::get(delay_min, delay_max);
		} else {
			delay = delay_min;
		}
	}
	
};

} // anonymous namespace

namespace audio {

enum aalAmbianceFlag {
	IS_PLAYING    = 0x00000001,
	IS_PAUSED     = 0x00000002,
	IS_LOOPED     = 0x00000004,
	IS_FADED_UP   = 0x00000008,
	IS_FADED_DOWN = 0x00000010
};
DECLARE_FLAGS(aalAmbianceFlag, aalAmbianceFlags)
DECLARE_FLAGS_OPERATORS(aalAmbianceFlags)

static const size_t KEY_CONTINUE = (size_t)-1;

struct Ambiance::Track : public Source::Callback {
	
	enum Flag {
		POSITION   = 0x00000001,
		REVERB     = 0x00000002,
		MASTER     = 0x00000004,
		MUTED      = 0x00000008,
		PAUSED     = 0x00000010,
		PREFETCHED = 0x00000020
	};
	DECLARE_FLAGS(Flag, TrackFlags)
	
	~Track() {
		if(s_id != INVALID_ID) {
			if(Source * source = backend->getSource(s_id)) {
				source->stop();
			}
			SampleId sid = Backend::getSampleId(s_id);
			arx_assert(_sample.isValid(sid));
			_sample[sid]->dereference();
		}
	}
	
	bool operator==(const std::string & str) const {
		return (name == str
		        || _sample[Backend::getSampleId(s_id)]->getName() == str);
	}
	
private:
	
	SourceId s_id; // Sample id
	
	Ambiance * ambiance; // Ambiance id
	
	string name; // Track name
	
	TrackFlags flags;

	typedef std::vector<TrackKey> KeyList;
	KeyList keys; // Key list
	KeyList::iterator key_i;
	

	size_t loopc; // How often the sample still needs to loop.
	size_t queued; // How many loop counts are already queued.
	
	explicit Track(Ambiance * _ambiance)
		: s_id(INVALID_ID), ambiance(_ambiance), flags(0), loopc(0), queued(0) { }
	
	void keyPlay();
	
	void onSampleStart(Source & source);
	
	void onSampleEnd(Source & source);
	
	void onSamplePosition(Source & source, size_t position) {
		if(position == 0) {
			onSampleStart(source);
		} else {
			onSampleEnd(source);
		}
	}
	
	void update(size_t time, size_t diff);
	
	aalError load(PakFileHandle * file, u32 version);
	
	friend class Ambiance;
};
DECLARE_FLAGS_OPERATORS(Ambiance::Track::TrackFlags)

void Ambiance::Track::keyPlay() {
	
	Source * source = backend->getSource(s_id);
	if(!source) {
		
		Channel channel;
		
		channel.mixer = ambiance->channel.mixer;
		channel.flags = FLAG_VOLUME | FLAG_PITCH | FLAG_RELATIVE;
		channel.flags |= ambiance->channel.flags;
		channel.volume = key_i->volume.cur;
		
		if(ambiance->channel.flags & FLAG_VOLUME) {
			channel.volume *= ambiance->channel.volume;
		}
		
		channel.pitch = key_i->pitch.cur;
		
		if(flags & POSITION) {
			channel.flags |= FLAG_POSITION;
			channel.position = Vec3f(key_i->x.cur, key_i->y.cur, key_i->z.cur);
			if(ambiance->channel.flags & FLAG_POSITION) {
				channel.position += ambiance->channel.position;
			}
			channel.flags |= ambiance->channel.flags & FLAG_REVERBERATION;
		} else {
			channel.flags |= FLAG_PAN;
			channel.pan = key_i->pan.cur;
		}
		
		source = backend->createSource(s_id, channel);
		if(!source) {
			s_id = Backend::clearSource(s_id);
			return;
		}
		
		source->addCallback(this, 0, UNIT_BYTES);
		source->addCallback(this, source->getSample()->getLength(), UNIT_BYTES);
		
		s_id = source->getId();
	}
	
	if(queued < loopc) {
		if(!key_i->delay_min && !key_i->delay_max) {
			size_t toqueue = loopc - queued;
			queued += toqueue;
			LogDebug("ambiance " << ambiance->getName() << ": playing "
			         << source->getSample()->getName() << " " << toqueue
			         << " -> " << queued << " / " << loopc);
			source->play(toqueue);
			arx_assert(loopc >= queued);
		} else {
			LogDebug("ambiance " << ambiance->getName() << ": playing "
			         << source->getSample()->getName());
			source->play();
			queued++;
		}
	}
	
	key_i->n_start = KEY_CONTINUE;
}

void Ambiance::Track::onSampleStart(Source & source) {
	
	LogDebug("ambiance " << ambiance->getName() << ": "
	         << source.getSample()->getName() << " started");
	
	if(flags & Ambiance::Track::PREFETCHED) {
		flags &= ~Ambiance::Track::PREFETCHED;
		if(key_i == keys.end()) {
			key_i = keys.begin();
		}
		key_i->n_start = KEY_CONTINUE;
		arx_assert(loopc >= queued);
	}
	
	arx_assert(key_i != keys.end());
	
	// Prefetch
	if(loopc == 1 && ambiance->isLooped()) {
		Ambiance::Track::KeyList::iterator keyPrefetch = key_i + 1;
		if(keyPrefetch == keys.end()) {
			keyPrefetch = keys.begin();
		}
		if(!keyPrefetch->start && !keyPrefetch->delay_min
		   && !keyPrefetch->delay_max) {
			LogDebug("ambiance " << ambiance->getName() << ": prefetching "
			         << source.getSample()->getName() << " " << keyPrefetch->loop);
			queued += keyPrefetch->loop;
			loopc += keyPrefetch->loop;
			source.play(keyPrefetch->loop);
			flags |= Ambiance::Track::PREFETCHED;
		}
	}
	
	float value = key_i->volume.update();
	if(ambiance->getChannel().flags & FLAG_VOLUME) {
		value *= ambiance->getChannel().volume;
	}
	source.setVolume(value);
	
	source.setPitch(key_i->pitch.update());
	
	if(flags & POSITION) {
		Vec3f position(key_i->x.update(), key_i->y.update(), key_i->z.update());
		if(ambiance->getChannel().flags & FLAG_POSITION) {
			position += ambiance->getChannel().position;
		}
		source.setPosition(position);
	} else {
		source.setPan(key_i->pan.update());
	}
	
}

void Ambiance::Track::onSampleEnd(Source & source) {
	
	ARX_UNUSED(source);
	
	LogDebug("ambiance " << ambiance->getName() << ": "
	         << source.getSample()->getName() << " ended");
	
	arx_assert(queued > 0);
	
	queued--;
	
	if(!--loopc) {
		
		arx_assert(queued == 0);
		
		//Key end
		key_i->delay = key_i->delay_max;
		key_i->updateSynch();
		key_i->n_start = key_i->start + key_i->delay;
		key_i->pitch.tupdate -= ambiance->time;
		
		if(++key_i == keys.end()) {
			//Track end
			
			LogDebug("ambiance " << ambiance->getName() << ": track ended");
			
			arx_assert(source.isIdle());
			
			if(flags & Track::MASTER) {
				//Ambiance end
				ambiance->time = 0;
				
				LogDebug("ambiance " << ambiance->getName() << ": master track ended");
				
				if(ambiance->isLooped()) {
					TrackList::iterator i = ambiance->tracks.begin();
					for(; i != ambiance->tracks.end(); ++i) {
						if(!(i->flags & Track::PREFETCHED)) {
							i->key_i = i->keys.begin();
						}
					}
					ambiance->start = session_time;
				} else {
					ambiance->stop();
				}
			}
			
		} else {
			loopc += key_i->loop;
		}
		
	} else if(key_i->delay_min || key_i->delay_max) {
		key_i->updateSynch();
		key_i->n_start = key_i->delay;
	}
	
}

void Ambiance::Track::update(size_t time, size_t diff) {
	
	if(!_sample.isValid(Backend::getSampleId(s_id)) || flags & Track::MUTED) {
		return;
	}
	
	if(key_i == keys.end()) {
		return;
	}
	
	//Run / update keys
	if(key_i->n_start <= diff) {
		keyPlay();
		return;
	} else if(key_i->n_start != KEY_CONTINUE) {
		key_i->n_start -= diff;
	}
	
	Source * source = backend->getSource(s_id);
	if(!source) {
		return;
	}
	
	if(key_i->volume.interval) {
		float value = key_i->volume.update(time);
		if(ambiance->channel.flags & FLAG_VOLUME) {
			value *= ambiance->channel.volume;
		}
		source->setVolume(value);
	} else {
		source->setVolume(key_i->volume.cur * ambiance->channel.volume);
	}
	if(key_i->pitch.interval) {
		source->setPitch(key_i->pitch.update(time));
	}
	if(flags & Track::POSITION) {
		Vec3f position;
		position.x = key_i->x.interval ? key_i->x.update(time) : key_i->x.cur;
		position.y = key_i->y.interval ? key_i->y.update(time) : key_i->y.cur;
		position.z = key_i->z.interval ? key_i->z.update(time) : key_i->z.cur;
		if(ambiance->channel.flags & FLAG_POSITION) {
			position += ambiance->channel.position;
		}
		source->setPosition(position);
	} else {
		source->setPan(key_i->pan.update(time));
	}
	
}

static aalError loadString(PakFileHandle * file, string & str) {
	
	std::ostringstream oss;
	
	aalError ret = AAL_OK;
	char c;
	while(file->read(&c, 1) ? c : (ret = AAL_ERROR_FILEIO, false)) {
		oss << (char)std::tolower(c);
	}
	
	str = oss.str();
	
	return ret;
}

aalError Ambiance::Track::load(PakFileHandle * file, u32 version) {
	
	// Get track sample name
	string sampleName;
	if(aalError error = loadString(file, sampleName)) {
		return error;
	}
	Sample * sample = new Sample(res::path::load(sampleName));
	if(sample->load() || (s_id = _sample.add(sample)) == INVALID_ID) {
		LogError << "Ambiance \"" << ambiance->name
		         << "\": missing sample \"" << sampleName << '"';
		delete sample;
		return AAL_ERROR_FILEIO;
	} else {
		sample->reference();
	}
	
	if(version >= AMBIANCE_FILE_VERSION_1002) {
		// Get track name (!= sample name)
		if(aalError error = loadString(file, name)) {
			return error;
		}
	}
	
	// Read flags and key count
	u32 iflags;
	u32 key_c;
	if(!file->read(&iflags, 4) || !file->read(&key_c, 4)) {
			return AAL_ERROR_FILEIO;
	}
	
	flags = Ambiance::Track::TrackFlags::load(iflags); // TODO save/load flags
	flags &= ~(Ambiance::Track::MUTED | Ambiance::Track::PAUSED
	           | Ambiance::Track::PREFETCHED);
	
	keys.resize(key_c);
	
	// Read settings for each key
	if(version < AMBIANCE_FILE_VERSION_1003) {
		Track::KeyList::iterator key;
		for(key = keys.begin(); key != keys.end(); ++key) {
			if(!key->load(file)) {
				return AAL_ERROR_FILEIO;
			}
		}
	} else {
		Ambiance::Track::KeyList::reverse_iterator key;
		for(key = keys.rbegin(); key != keys.rend(); ++key) {
			if(!key->load(file)) {
				return AAL_ERROR_FILEIO;
			}
		}
	}
	
	return AAL_OK;
}

Ambiance::Ambiance(const res::path & _name)
	: status(Idle), loop(false), fade(None), start(0),
	  time(0), name(_name), data(NULL) {
	channel.flags = 0;
}

Ambiance::~Ambiance() {
	LogDebug("deleting ambiance " << name);
}

aalError Ambiance::load() {
	
	if(!tracks.empty()) {
		return AAL_ERROR_INIT;
	}
	
	boost::scoped_ptr<PakFileHandle> file(OpenResource(name, ambiance_path));
	if(!file) {
		return AAL_ERROR_FILEIO;
	}
	
	// Read file signature and version
	u32 magic, version;
	if(!file->read(&magic, 4) || !file->read(&version, 4)) {
		return AAL_ERROR_FILEIO;
	}
	
	// Check file signature
	if(magic != AMBIANCE_FILE_SIGNATURE || version > AMBIANCE_FILE_VERSION) {
		return AAL_ERROR_FORMAT;
	}
	
	// Read track count and initialize track structures
	u32 nbtracks;
	file->read(&nbtracks, 4);
	tracks.resize(nbtracks, Track(this));
	
	Ambiance::TrackList::iterator track = tracks.begin();
	for(; track != tracks.end(); ++track) {
		if(aalError error = track->load(file.get(), version)) {
			return error;
		}
	}
	
	return AAL_OK;
}

aalError Ambiance::setVolume(float volume) {
	
	if(!(channel.flags & FLAG_VOLUME)) {
		return AAL_ERROR_INIT;
	}
	
	channel.volume = clamp(volume, 0.f, 1.f);
	
	if(!isPlaying()) {
		return AAL_OK;
	}
	
	TrackList::const_iterator track = tracks.begin();
	for(; track != tracks.end(); ++track) {
		if(Source * source = backend->getSource(track->s_id)) {
			if(track->key_i != track->keys.end()) {
				source->setVolume(track->key_i->volume.cur * channel.volume);
			}
		}
	}
	
	return AAL_OK;
}

aalError Ambiance::muteTrack(const string & name, bool mute) {
	
	if(tracks.empty()) {
		return AAL_OK;
	}
	
	TrackList::iterator track = std::find(tracks.begin(), tracks.end(), name);
	if(track == tracks.end()) {
		return AAL_OK;
	}
	
	if(mute) {
		track->flags |= Track::MUTED;
		if(isPlaying()) {
			if(Source * source = backend->getSource(track->s_id)) {
				source->stop();
			}
		}
	} else {
		track->flags &= ~Track::MUTED;
		if(isPlaying()) {
			track->key_i = track->keys.begin();
			track->keyPlay();
		}
	}
	
	return AAL_OK;
}

aalError Ambiance::play(const Channel & _channel, bool _loop,
                        size_t _fade_interval) {
	
	channel = _channel;
	
	if(isPlaying() || isPaused()) {
		stop();
	}
	
	loop = _loop;
	
	fade_interval = (float)_fade_interval;
	if(fade_interval) {
		fade = FadeUp;
		fade_max = channel.volume;
		channel.volume = 0.0F;
		fade_time = 0.0F;
	} else {
		fade = None;
	}
	
	TrackList::iterator track = tracks.begin();
	for(; track != tracks.end(); ++track) {
		
		//Init track keys
		Track::KeyList::iterator key = track->keys.begin();
		for(; key != track->keys.end(); ++key) {
			
			key->delay = key->delay_max;
			key->updateSynch();
			key->n_start = key->start + key->delay;
			
			key->volume.reset();
			key->pitch.reset();
			key->pan.reset();
			key->x.reset();
			key->y.reset();
			key->z.reset();
		}
		
		arx_assert(backend->getSource(track->s_id) == NULL
		           || backend->getSource(track->s_id)->isIdle());
		
		track->key_i = track->keys.begin();
		track->loopc = track->key_i->loop;
		track->queued = 0;
	}
	
	status = Playing;
	start = session_time;
	
	const Mixer * mixer = _mixer[channel.mixer];
	if(mixer && mixer->isPaused()) {
		status = Paused;
	}
	
	return AAL_OK;
}

aalError Ambiance::stop(size_t _fade_interval) {
	
	if(isIdle()) {
		return AAL_OK;
	}
	
	fade_interval = static_cast<float>(_fade_interval);
	if(fade_interval) {
		fade = FadeDown;
		fade_time = 0;
		return AAL_OK;
	}
	
	status = Idle;
	time = 0;
	
	TrackList::iterator track = tracks.begin();
	for(; track != tracks.end(); ++track) {
		if(Source * source = backend->getSource(track->s_id)) {
			source->stop();
		}
		track->s_id = Backend::clearSource(track->s_id);
	}
	
	return AAL_OK;
}

aalError Ambiance::pause() {
	
	if(!isPlaying()) {
		return AAL_ERROR;
	}
	
	status = Paused;
	time = session_time;
	
	TrackList::iterator track = tracks.begin();
	for(; track != tracks.end(); ++track) {
		if(Source * source = backend->getSource(track->s_id)) {
			source->pause();
			track->flags |= Track::PAUSED;
		}
	}
	
	return AAL_OK;
}

aalError Ambiance::resume() {
	
	if(!isPaused()) {
		return AAL_ERROR;
	}
	
	TrackList::iterator track = tracks.begin();
	for(; track != tracks.end(); ++track) {
		if(track->flags & Track::PAUSED) {
			if(Source * source = backend->getSource(track->s_id)) {
				source->resume();
			}
			track->flags &= ~Track::PAUSED;
		}
	}
	
	status = Playing;
	start += session_time - time;
	time = session_time - start;
	
	return AAL_OK;
}

aalError Ambiance::update() {
	
	if(!isPlaying()) {
		return AAL_OK;
	}
	
	size_t interval = session_time - start - time;
	time += interval;
	
	LogDebug("ambiance \"" << name << "\": update to time=" << time);
	
	// Fading
	if(fade_interval && fade != None) {
		fade_time += interval;
		if(fade == FadeUp) {
			channel.volume = fade_max * fade_time / fade_interval;
			if(channel.volume >= fade_max) {
				channel.volume = fade_max;
				fade_interval = 0.f;
			}
		} else {
			channel.volume = fade_max - fade_max * fade_time / fade_interval;
			if(channel.volume <= 0.f) {
				stop();
				return AAL_OK;
			}
		}
		channel.volume = LinearToLogVolume(channel.volume);
	}
	
	// Update tracks
	TrackList::iterator track = tracks.begin();
	for(; track != tracks.end(); ++track) {
		track->update(time, interval);
	}
	
	return AAL_OK;
}

} // namespace audio
