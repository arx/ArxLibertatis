/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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
/* Based on:
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

#ifndef ARX_AUDIO_CODEC_WAV_H
#define ARX_AUDIO_CODEC_WAV_H

#include <stddef.h>
#include <vector>

#include "audio/AudioTypes.h"
#include "audio/Stream.h"

class PakFileHandle;

namespace audio {

class Codec;

class StreamWAV : public Stream {
	
public:
	
	StreamWAV();
	~StreamWAV();
	
	aalError setStream(PakFileHandle * stream);
	aalError setPosition(size_t position);
	
	PakFileHandle * getStream();
	aalError getFormat(PCMFormat & format);
	size_t getLength();
	
	aalError read(void * buffer, size_t bufferSize, size_t & read);
	
private:
	
	PakFileHandle * m_stream;
	Codec * codec;
	std::vector<char> m_header; // File data format
	size_t size, outsize; // Compressed and uncompessed data size
	size_t offset; // Offset of data in file
	size_t cursor;
	
};

} // namespace audio

#endif // ARX_AUDIO_CODEC_WAV_H
