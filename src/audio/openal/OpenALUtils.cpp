
#include "audio/openal/OpenALUtils.h"

#include <alc.h>

const char * getAlcErrorString(ALenum error) {
	
	switch(error) {
		case ALC_NO_ERROR: return "No error";
		case ALC_INVALID_DEVICE: return "Invalid device";
		case ALC_INVALID_CONTEXT: return "Invalid context";
		case ALC_INVALID_ENUM: return "Invalid enum";
		case ALC_INVALID_VALUE: return "Invalid value";
		case ALC_OUT_OF_MEMORY: return "Out of memory";
		default: return "(unknown error)";
	}
}

const char * getAlErrorString(ALenum error) {
	
	switch(error) {
		case AL_NO_ERROR: return "No error";
		case AL_INVALID_NAME: return "Invalid name";
		case AL_INVALID_ENUM: return "Invalid enum";
		case AL_INVALID_VALUE: return "Invalid value";
		case AL_INVALID_OPERATION: return "Invalid operation";
		case AL_OUT_OF_MEMORY: return "Out of memory";
		default: return "(unknown error)";
	}
}
