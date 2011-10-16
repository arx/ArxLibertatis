/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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
