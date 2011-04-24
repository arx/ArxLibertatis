
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
	
	Source * createSource(aalSLong sampleId, const aalChannel & channel);
	
	Source * getSource(aalSLong sourceId);
	
	aalError setReverbEnabled(bool enable);
	
	aalError setUnitFactor(float factor);
	aalError setRolloffFactor(float factor);
	
	aalError setListenerPosition(const aalVector & position);
	aalError setListenerOrientation(const aalVector & front, const aalVector & up);
	
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
