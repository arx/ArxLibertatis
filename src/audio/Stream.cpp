/*
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/

#include "audio/Stream.h"

#include <cstdio>

#include "audio/AudioGlobal.h"
#include "audio/codec/WAV.h"

#include "io/PakReader.h"
#include "io/Logger.h"

using std::string;

namespace audio {

Stream * createStream(const string & name) {
	
	PakFileHandle * file = OpenResource(name.c_str(), sample_path);
	if(!file) {
		return NULL;
	}
	
	file->seek(SeekSet, 0);
	
	Stream * stream = new StreamWAV;
	
	if(stream->setStream(file)) {
		delete stream;
		delete file;
		return NULL;
	}
	
	return stream;
}

void deleteStream(Stream *& stream) {
	PakFileHandle * file = stream->getStream();
	delete file;
	delete stream;
	stream = NULL;
}

} // namespace audio
