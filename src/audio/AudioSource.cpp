
#include "audio/AudioSource.h"

#include "audio/Sample.h"

namespace audio {

Source::Source(Sample * _sample) : id((aalSLong)-1), sample(_sample), status(IDLE) {
	sample->Catch();
}

Source::~Source() {
	sample->Release();
}

} // namespace audio
