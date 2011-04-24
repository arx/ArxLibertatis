
#ifndef ARX_AUDIO_OPENAL_OPENALBACKEND_H
#define ARX_AUDIO_OPENAL_OPENALBACKEND_H

#include "audio/openal/alwrapper.h"

#include "audio/AudioBackend.h"
#include "audio/AudioTypes.h"
#include "audio/AudioResource.h"

namespace audio {

class OpenALSource;

class OpenALBackend : public Backend {
	
public:
	
	OpenALBackend();
	~OpenALBackend();
	
	aalError init(bool enableEax);
	
	aalError updateDeferred();
	
	Source * createSource(SampleId sampleId, const Channel & channel);
	
	Source * getSource(SourceId sourceId);
	
	aalError setReverbEnabled(bool enable);
	
	aalError setUnitFactor(float factor);
	aalError setRolloffFactor(float factor);
	
	aalError setListenerPosition(const Vector3f & position);
	aalError setListenerOrientation(const Vector3f & front, const Vector3f & up);
	
	aalError setListenerEnvironment(const Environment & env);
	aalError setRoomRolloffFactor(float factor);
	
	source_iterator sourcesBegin();
	source_iterator sourcesEnd();
	source_iterator deleteSource(source_iterator it);
	
private:
	
	aalError setEffect(ALenum type, float val);
	
	ALCdevice * device;
	ALCcontext * context;
	
	bool hasEFX;
	LPALGENEFFECTS alGenEffects;
	LPALDELETEEFFECTS alDeleteEffects;
	LPALEFFECTF alEffectf;
	
	ResourceList<OpenALSource> sources;
	
	float rolloffFactor;
	
	bool effectEnabled;
	ALuint effect;
	
	friend class OpenALSource;
};

} // namespace audio

#endif // ARX_AUDIO_OPENAL_OPENALBACKEND_H
