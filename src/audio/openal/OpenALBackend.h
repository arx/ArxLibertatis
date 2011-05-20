
#ifndef ARX_AUDIO_OPENAL_OPENALBACKEND_H
#define ARX_AUDIO_OPENAL_OPENALBACKEND_H

#include "Configure.h"

#include <al.h>
#include <alc.h>
#ifdef HAVE_OPENAL_EFX
	#include <efx.h>
#endif

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
	
	aalError setListenerPosition(const Vec3f & position);
	aalError setListenerOrientation(const Vec3f & front, const Vec3f & up);
	
	aalError setListenerEnvironment(const Environment & env);
	aalError setRoomRolloffFactor(float factor);
	
	source_iterator sourcesBegin();
	source_iterator sourcesEnd();
	source_iterator deleteSource(source_iterator it);
	
private:
	
	ALCdevice * device;
	ALCcontext * context;
	
#ifdef HAVE_OPENAL_EFX
	
	aalError setEffect(ALenum type, float val);
	
	bool hasEFX;
	LPALGENEFFECTS alGenEffects;
	LPALDELETEEFFECTS alDeleteEffects;
	LPALEFFECTF alEffectf;
	
	bool effectEnabled;
	ALuint effect;
	
#endif
	
	ResourceList<OpenALSource> sources;
	
	float rolloffFactor;
	
	friend class OpenALSource;
};

} // namespace audio

#endif // ARX_AUDIO_OPENAL_OPENALBACKEND_H
