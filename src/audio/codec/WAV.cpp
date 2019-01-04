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

#include "audio/codec/WAV.h"

#include <cstring>
#include <cstdlib>

#include "audio/codec/ADPCM.h"
#include "audio/codec/Codec.h"
#include "audio/codec/RAW.h"
#include "audio/codec/WAVFormat.h"
#include "io/resource/PakReader.h"
#include "platform/Platform.h"

namespace {

class ChunkFile {
	
public:
	
	explicit ChunkFile(PakFileHandle * file);
	
	bool read(void * buffer, size_t size);
	bool skip(size_t size);
	bool find(const char * id);
	//! \return true if next four bytes = chunk id;
	bool check(const char * id);
	size_t size() { return offset; }
	bool restart();
	
private:
	
	PakFileHandle * m_file;
	size_t offset;
	
};

ChunkFile::ChunkFile(PakFileHandle * file) : m_file(file), offset(0) {
}

bool ChunkFile::read(void * buffer, size_t size) {
	
	if(!m_file->read(buffer, size)) {
		return false;
	}
	
	if(offset) {
		offset -= size;
	}
	
	return true;
}

bool ChunkFile::skip(size_t size) {
	
	if(m_file->seek(SeekCur, size) == -1) {
		return false;
	}
	
	if(offset) {
		offset -= size;
	}
	
	return true;
}

bool ChunkFile::find(const char * id) {
	
	m_file->seek(SeekCur, offset);
	
	u8 cc[4];
	while(m_file->read(cc, 4)) {
		u32 _offset;
		if(!m_file->read(&_offset, 4)) {
			return false;
		}
		offset = _offset;
		if(!memcmp(cc, id, 4)) {
			return true;
		}
		if(m_file->seek(SeekCur, offset) == -1) {
			return false;
		}
	}
	
	return false;
}

bool ChunkFile::check(const char * id) {
	
	u8 cc[4];
	if(!m_file->read(cc, 4)) {
		return false;
	}
	
	if(memcmp(cc, id, 4) != 0) {
		return false;
	}
	
	if(offset) {
		offset -= 4;
	}
	
	return true;
}

bool ChunkFile::restart() {
	
	offset = 0;
	
	return (m_file->seek(SeekSet, 0) != -1);
}

} // anonymous namespace

namespace audio {

StreamWAV::StreamWAV() :
	m_stream(NULL), codec(NULL),
	size(0), outsize(0), offset(0), cursor(0) {
}

StreamWAV::~StreamWAV() {
	delete codec;
}

aalError StreamWAV::setStream(PakFileHandle * stream) {
	
	if(!stream) {
		return AAL_ERROR_FILEIO;
	}
	
	m_stream = stream;
	
	m_header.resize(sizeof(WaveHeader));
	WaveHeader * header = reinterpret_cast<WaveHeader *>(&m_header[0]);
	
	ChunkFile wave(m_stream);
	
	// Check for 'RIFF' chunk id and skip file size
	if(!wave.check("RIFF") || !wave.skip(4) || !wave.check("WAVE") || !wave.find("fmt ")
	   || !wave.read(&header->formatTag, 2)
	   || !wave.read(&header->channels, 2)
	   || !wave.read(&header->samplesPerSec, 4)
	   || !wave.read(&header->avgBytesPerSec, 4)
	   || !wave.read(&header->blockAlign, 2)
	   || !wave.read(&header->bitsPerSample, 2)) {
		return AAL_ERROR_FORMAT;
	}
	
	// Get codec specific infos from header for non-PCM format
	if(header->formatTag != WAV_FORMAT_PCM) {
		
		// Load extra bytes from header
		if(!wave.read(&header->size, 2)) {
			return AAL_ERROR_FORMAT;
		}
		
		m_header.resize(sizeof(WaveHeader) + header->size);
		header = reinterpret_cast<WaveHeader *>(&m_header[0]);
		
		wave.read(&m_header[0] + sizeof(WaveHeader), header->size);
		
		// Get sample count from the 'fact' chunk
		wave.find("fact");
		wave.read(&outsize, 4);
	}
	
	// Create codec
	switch(header->formatTag) {
		case WAV_FORMAT_PCM   :
			codec = new CodecRAW;
			break;
		case WAV_FORMAT_ADPCM :
			outsize <<= 1;
			codec = new CodecADPCM;
			break;
		default                :
			return AAL_ERROR_FORMAT;
	}
	
	// Check for 'data' chunk id, get data size and offset
	wave.restart();
	wave.skip(12);
	
	if(!wave.find("data")) {
		return AAL_ERROR_FORMAT;
	}
	
	size = wave.size();
	
	if(header->formatTag == WAV_FORMAT_PCM) {
		outsize = size;
	} else {
		outsize *= header->channels;
	}
	
	offset = m_stream->tell();
	
	codec->setStream(m_stream);
	
	if(aalError error = codec->setHeader(header)) {
		return error;
	}
	
	return AAL_OK;
}

aalError StreamWAV::setPosition(size_t position) {
	
	if(position >= outsize) {
		return AAL_ERROR_FILEIO;
	}
	
	cursor = position;
	
	// Reset stream position at the begining of data chunk
	if(m_stream->seek(SeekSet, offset) == -1) {
		return AAL_ERROR_FILEIO;
	}
	
	return codec->setPosition(cursor);
}

PakFileHandle * StreamWAV::getStream() {
	return m_stream;
}

aalError StreamWAV::getFormat(PCMFormat & format) {
	
	const WaveHeader * header = reinterpret_cast<const WaveHeader *>(&m_header[0]);
	
	format.frequency = header->samplesPerSec;
	format.channels = header->channels;
	
	switch(header->formatTag) {
		case WAV_FORMAT_PCM   :
			format.quality = header->bitsPerSample;
			break;
		case WAV_FORMAT_ADPCM :
			format.quality = 16;
			break;
	}
	
	return AAL_OK;
}

size_t StreamWAV::getLength() {
	return outsize;
}

aalError StreamWAV::read(void * buffer, size_t bufferSize, size_t & read) {
	
	read = 0;
	
	if(cursor >= outsize) {
		return AAL_OK;
	}
	
	size_t count = cursor + bufferSize > outsize ? outsize - cursor : bufferSize;
	
	if(aalError error = codec->read(buffer, count, read)) {
		return error;
	}
	
	cursor += read;
	
	return AAL_OK;
}

} // namespace audio
