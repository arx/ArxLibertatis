
#ifndef ARX_AUDIO_AUDIOBACKEND_H
#define ARX_AUDIO_AUDIOBACKEND_H

#include "audio/AudioTypes.h"

namespace audio {

class Source;
class Environment;
class Mixer;
class Sample;

/*!
 * An audio source that can play one sample.
 */
class Backend {
	
public:
	
	virtual ~Backend() {};
	
	virtual aalError updateDeferred() = 0;
	
	virtual Source * createSource(SampleId sampleId, const Channel & channel) = 0;
	
	virtual Source * getSource(SourceId sourceId) = 0;
	
	virtual aalError setReverbEnabled(bool enable) = 0;
	
	virtual aalError setUnitFactor(float factor) = 0;
	virtual aalError setRolloffFactor(float factor) = 0;
	
	virtual aalError setListenerPosition(const Vector3f & position) = 0;
	virtual aalError setListenerOrientation(const Vector3f & front, const Vector3f & up) = 0;
	
	virtual aalError setListenerEnvironment(const Environment & env) = 0;
	virtual aalError setRoomRolloffFactor(float factor) = 0;
	
	typedef Source * const * source_iterator;
	virtual source_iterator sourcesBegin() = 0;
	virtual source_iterator sourcesEnd() = 0;
	virtual source_iterator deleteSource(source_iterator it) = 0;
	
	static inline SampleId getSampleId(SourceId sourceId) { return sourceId & 0x0000ffff; }
	static inline SourceId clearSource(SourceId sourceId) { return sourceId | 0xffff0000; }
	
};

} // namespace audio

#endif // ARX_AUDIO_AUDIOBACKEND_H
