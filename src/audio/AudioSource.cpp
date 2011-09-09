
#include "audio/AudioSource.h"

#include <algorithm>

#include "audio/AudioGlobal.h"
#include "audio/Sample.h"

namespace audio {

Source::Source(Sample * _sample) : id(INVALID_ID), sample(_sample), status(Idle), time(0), callback_i(0) {
	sample->reference();
}

Source::~Source() {
	sample->dereference();
}

void Source::addCallback(Callback * callback, size_t time, TimeUnit unit) {
	
	size_t pos = std::min(unitsToBytes(time, sample->getFormat(), unit), sample->getLength());
	
	size_t i = 0;
	while(i != callbacks.size() && callbacks[i].second <= pos) {
		i++;
	}
	
	if(i <= callback_i && !callbacks.empty()) {
		callback_i++;
	}
	
	callbacks.insert(callbacks.begin() + i, std::make_pair(callback, pos));
	
}

aalError Source::update() {
	
	if(status != Playing || updateCulling()) {
		return AAL_OK;
	}
	
	aalError ret = updateBuffers();
	
	updateCallbacks();
	
	return ret;
}

void Source::updateCallbacks() {
	
	while(true) {
		
		// Check if it's time to launch a callback
		for(; callback_i != callbacks.size() && callbacks[callback_i].second <= time; callback_i++) {
			callbacks[callback_i].first->onSamplePosition(*this, callbacks[callback_i].second);
		}
		
		if(time < sample->getLength()) {
			break;
		}
		
		time -= sample->getLength();
		callback_i = 0;
		
		if(!time && status != Playing) {
			// Prevent callback for time==0 being called again after playing.
			break;
		}
		
	}
	
}

} // namespace audio
