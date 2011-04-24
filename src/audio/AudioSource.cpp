
#include "audio/AudioSource.h"

#include "audio/Sample.h"

namespace audio {

Source::Source(Sample * _sample) : id(INVALID_ID), sample(_sample), status(Idle) {
	sample->Catch();
}

Source::~Source() {
	sample->Release();
}

} // namespace audio
