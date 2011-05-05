
#include "audio/AudioSource.h"

#include "audio/Sample.h"

namespace audio {

Source::Source(Sample * _sample) : id(INVALID_ID), sample(_sample), status(Idle) {
	sample->reference();
}

Source::~Source() {
	sample->dereference();
}

} // namespace audio
