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

#include "io/SaveBlock.h"

#include <cstdlib>

#include <boost/foreach.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/range/adaptor/map.hpp>

#include <zlib.h>

#include "io/log/Logger.h"
#include "io/fs/Filesystem.h"
#include "io/Blast.h"

#include "platform/Platform.h"

static const u32 SAV_VERSION_OLD = (1 << 16) | 0;
static const u32 SAV_VERSION_RELEASE = (1 << 16) | 1;
static const u32 SAV_VERSION_DEFLATE = (2 << 16) | 0;
static const u32 SAV_VERSION_NOEXT = (2 << 16) | 1;

static const u32 SAV_COMP_NONE = 0;
static const u32 SAV_COMP_IMPLODE = 1;
static const u32 SAV_COMP_DEFLATE = 2;

static const u32 SAV_SIZE_UNKNOWN = 0xffffffff;

#ifdef ARX_DEBUG
static const char BADSAVCHAR[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ\\/.";
#endif

const char * SaveBlock::File::compressionName() const {
	switch(comp) {
		case None: return "none";
		case ImplodeCrypt: return "implode+crypt";
		case Deflate: return "deflate";
		default: return "(unknown)";
	}
}

bool SaveBlock::File::loadOffsets(std::istream & handle, u32 version) {
	
	if(version < SAV_VERSION_DEFLATE) {
		// ignore the size, calculate from chunks
		handle.seekg(4, std::istream::cur);
		uncompressedSize = size_t(-1);
	} else {
		u32 uncompressed;
		if(fs::read(handle, uncompressed).fail()) {
			return false;
		}
		uncompressedSize = (uncompressed == SAV_SIZE_UNKNOWN) ? size_t(-1) : uncompressed;
	}
	
	u32 nChunks;
	if(!fs::read(handle, nChunks)) {
		return false;
	}
	if(version < SAV_VERSION_DEFLATE && nChunks == 0) {
		nChunks = 1;
	}
	chunks.resize(nChunks);
	
	if(version < SAV_VERSION_DEFLATE) {
		// ignored
		handle.seekg(4, std::istream::cur);
		comp = File::ImplodeCrypt;
	} else {
		u32 compid;
		if(!fs::read(handle, compid)) {
			return false;
		}
		switch(compid) {
			case SAV_COMP_NONE: comp = File::None; break;
			case SAV_COMP_IMPLODE: comp = File::ImplodeCrypt; break;
			case SAV_COMP_DEFLATE: comp = File::Deflate; break;
			default: comp = File::Unknown;
		}
	}
	
	size_t size = 0;
	for(size_t i = 0; i < nChunks; i++) {
		
		u32 chunkSize;
		if(!fs::read(handle, chunkSize)) {
			return false;
		}
		size += chunkSize;
		
		u32 chunkOffset;
		if(!fs::read(handle, chunkOffset)) {
			return false;
		}
		
		chunks[i].size = chunkSize;
		chunks[i].offset = chunkOffset;
	}
	
	storedSize = size;
	
	return true;
}

void SaveBlock::File::writeEntry(std::ostream & handle, const std::string & name) const {
	
	handle.write(name.c_str(), name.length() + 1);
	
	u32 _uncompressedSize = (uncompressedSize == size_t(-1)) ? SAV_SIZE_UNKNOWN : uncompressedSize;
	fs::write(handle, _uncompressedSize);
	
	u32 nChunks = chunks.size();
	fs::write(handle, nChunks);
	
	u32 _comp;
	switch(comp) {
		case File::None: _comp = SAV_COMP_NONE; break;
		case File::ImplodeCrypt: _comp = SAV_COMP_IMPLODE; break;
		case File::Deflate: _comp = SAV_COMP_DEFLATE; break;
		case File::Unknown: _comp = u32(-1); break;
	}
	fs::write(handle, _comp);
	
	for(File::ChunkList::const_iterator chunk = chunks.begin(); chunk != chunks.end(); ++chunk) {
		
		u32 chunkSize = chunk->size;
		fs::write(handle, chunkSize);
		
		u32 chunkOffset = chunk->offset;
		fs::write(handle, chunkOffset);
		
	}
	
}

std::string SaveBlock::File::loadData(std::istream & handle, const std::string & name) const {
	
	LogDebug("Loading " << name << ' ' << storedSize << "b in " << chunks.size() << " chunks, "
	         << compressionName() << " -> " << (int)uncompressedSize << "b");
	
	std::string buffer;
	buffer.resize(storedSize);
	char * p = &buffer[0];
	
	for(File::ChunkList::const_iterator chunk = chunks.begin();
	    chunk != chunks.end(); ++chunk) {
		handle.seekg(chunk->offset + 4);
		handle.read(p, chunk->size);
		p += chunk->size;
	}
	
	arx_assert(p == &buffer[0] + storedSize);
	
	switch(comp) {
		
		case File::None: {
			arx_assert(uncompressedSize == storedSize);
			return buffer;
		}
		
		case File::ImplodeCrypt: {
			for(size_t i = 0; i < storedSize; i += 2) {
				buffer[i] = char(~u8(buffer[i]));
			}
			std::string uncompressed = blast(buffer, uncompressedSize);
			if(uncompressedSize != size_t(-1)
			   && uncompressed.size() != uncompressedSize) {
				LogError << "Error decompressing imploded " << name;
			}
			return uncompressed;
		}
		
		case File::Deflate: {
			arx_assert(uncompressedSize != size_t(-1));
			std::string uncompressed;
			uncompressed.resize(uncompressedSize);
			uLongf outSize = uLongf(uncompressedSize);
			Bytef * out = reinterpret_cast<Bytef *>(&uncompressed[0]);
			const Bytef * in = reinterpret_cast<const Bytef *>(buffer.data());
			int ret = uncompress(out, &outSize, in, uLongf(storedSize));
			if(ret != Z_OK) {
				LogError << "Error decompressing deflated " << name << ": " << zError(ret) << " (" << ret << ')';
				return std::string();
			}
			if(outSize != uncompressedSize) {
				LogError << "Unexpedect uncompressed size " << outSize << " while loading "
				         << name << ", expected " << uncompressedSize;
			}
			buffer.resize(outSize);
			return uncompressed;
		}
		
		default: {
			LogError << "Error decompressing " << name << ": unknown format";
			return std::string();
		}
		
	}
}

SaveBlock::SaveBlock(const fs::path & savefile)
	: m_savefile(savefile)
	, m_totalSize(0)
	, m_usedSize(0)
	, m_chunkCount(0)
{ }

SaveBlock::~SaveBlock() { }

bool SaveBlock::loadFileTable() {
	
	m_handle.seekg(0);
	
	u32 fatOffset;
	if(fs::read(m_handle, fatOffset).fail()) {
		return false;
	}
	if(m_handle.seekg(fatOffset + 4).fail()) {
		LogError << "Cannot seek to FAT";
		return false;
	}
	m_totalSize = fatOffset;
	
	u32 version;
	if(fs::read(m_handle, version).fail()) {
		return false;
	}
	if(version != SAV_VERSION_DEFLATE && version != SAV_VERSION_RELEASE && version != SAV_VERSION_NOEXT) {
		LogWarning << "Unexpected savegame version: " << (version >> 16) << '.' << (version & 0xffff) << " for " << m_savefile;
	}
	
	u32 nFiles;
	if(fs::read(m_handle, nFiles).fail()) {
		return false;
	}
	nFiles = (version == SAV_VERSION_OLD) ? nFiles - 1 : nFiles;
	size_t hashMapSize = 1;
	while(hashMapSize < nFiles) {
		hashMapSize <<= 1;
	}
	if(nFiles > (hashMapSize * 3) / 4) {
		hashMapSize <<= 1;
	}
	m_files.rehash(hashMapSize);
	
	if(version == SAV_VERSION_OLD) {
		char c;
		do {
			c = static_cast<char>(m_handle.get());
		} while(c != '\0' && m_handle.good());
		File dummy;
		if(!dummy.loadOffsets(m_handle, version)) {
			return false;
		}
	}
	
	m_usedSize = 0;
	m_chunkCount = 0;
	
	for(u32 i = 0; i < nFiles; i++) {
		
		// Read the file name.
		std::string name;
		if(fs::read(m_handle, name).fail()) {
			return false;
		}
		if(version < SAV_VERSION_NOEXT) {
			boost::to_lower(name);
			if(name.size() > 4 && !name.compare(name.size() - 4, 4, ".sav", 4)) {
				name.resize(name.size() - 4);
			}
		}
		
		File & file = m_files[name];
		
		if(!file.loadOffsets(m_handle, version)) {
			return false;
		}
		
		m_usedSize += file.storedSize, m_chunkCount += file.chunks.size();
	}
	
	return true;
}

void SaveBlock::writeFileTable(const std::string & important) {
	
	LogDebug("writeFileTable " << m_savefile);
	
	u32 fatOffset = m_totalSize;
	m_handle.seekp(fatOffset + 4);
	
	fs::write(m_handle, SAV_VERSION_NOEXT);
	
	u32 nFiles = m_files.size();
	fs::write(m_handle, nFiles);
	
	Files::const_iterator ifile = m_files.find(important);
	if(ifile != m_files.end()) {
		ifile->second.writeEntry(m_handle, ifile->first);
	}
	
	for(Files::const_iterator file = m_files.begin(); file != m_files.end(); ++file) {
		if(file != ifile) {
			file->second.writeEntry(m_handle, file->first);
		}
	}
	
	m_handle.seekp(0);
	fs::write(m_handle, fatOffset);
	
}

bool SaveBlock::open(bool writable) {
	
	LogDebug("opening savefile " << m_savefile << " witable=" << writable);
	
	fs::fstream::openmode mode = fs::fstream::in | fs::fstream::binary | fs::fstream::ate;
	if(writable) {
		mode |= fs::fstream::out;
	}
	
	m_handle.clear();
	m_handle.open(m_savefile, mode);
	if(!m_handle.is_open()) {
		m_handle.clear();
		if(writable) {
			m_handle.open(m_savefile, mode | fs::fstream::trunc);
		}
		if(!m_handle.is_open()) {
			LogError << "Could not open " << m_savefile << " for "
			         << (writable ? "reading/writing" : "reading");
			return false;
		}
	}
	
	if(m_handle.tellg() > 0 && !loadFileTable()) {
		LogError << "Broken save file";
		return false;
	}
	
	return true;
}

bool SaveBlock::flush(const std::string & important) {
	
	arx_assert_msg(important.find_first_of(BADSAVCHAR) == std::string::npos,
	               "bad save filename: \"%s\"", important.c_str());
	
	if((m_usedSize * 2 < m_totalSize || m_chunkCount > (m_files.size() * 4 / 3))) {
		defragment();
	}
	
	writeFileTable(important);
	
	m_handle.flush();
	
	return m_handle.good();
}

bool SaveBlock::defragment() {
	
	LogDebug("defragmenting " << m_savefile << " save: using " << m_usedSize << " / " << m_totalSize
	         << " b for " << m_files.size() << " files in " << m_chunkCount << " chunks");
	
	fs::path tempFileName = m_savefile;
	{
		int i = 0;
		do {
			std::ostringstream oss;
			oss << "defrag" << i++;
			tempFileName.set_ext(oss.str());
		} while(fs::exists(tempFileName));
	}
	
	fs::ofstream tempFile(tempFileName, fs::fstream::out | fs::fstream::binary | fs::fstream::trunc);
	if(!tempFile.is_open()) {
		LogWarning << "Could not open temporary save file for defragmenting: " << tempFileName;
		return false;
	}
	
	tempFile.seekp(4);
	
	#ifdef ARX_DEBUG
	size_t checkTotalSize = 0;
	#endif
	std::vector<char> buffer;
	BOOST_FOREACH(const File & file, m_files | boost::adaptors::map_values) {
		
		if(file.storedSize == 0) {
			continue;
		}
		
		buffer.resize(file.storedSize);
		char * p = &buffer.front();
		
		BOOST_FOREACH(const File::Chunk & chunk, file.chunks) {
			m_handle.seekg(chunk.offset + 4);
			m_handle.read(p, chunk.size);
			p += chunk.size;
		}
		
		arx_assert(p == &buffer.front() + file.storedSize);
		
		if(!tempFile.write(&buffer.front(), file.storedSize)) {
			break;
		}
		
		#ifdef ARX_DEBUG
		checkTotalSize += file.storedSize;
		#endif
	}
	
	if(!tempFile) {
		tempFile.close();
		fs::remove(tempFileName);
		LogWarning << "Failed to write defragmented save file: " << tempFileName;
		return false;
	}
	
	tempFile.flush(), tempFile.close(), m_handle.close();
	
	bool renamed = fs::rename(tempFileName, m_savefile, true);
	if(!renamed) {
		fs::remove(tempFileName);
		LogWarning << "Failed to move defragmented save file " << tempFileName << " to " << m_savefile;
	} else {
		
		size_t newTotalSize = 0;
		for(Files::iterator i = m_files.begin(); i != m_files.end(); ++i) {
			File & file = i->second;
			if(file.storedSize != 0) {
				file.chunks.resize(1);
				file.chunks.front().offset = newTotalSize;
				file.chunks.front().size = file.storedSize;
				newTotalSize += file.storedSize;
			}
		}
		arx_assert(checkTotalSize == newTotalSize);
		
		m_usedSize = m_totalSize = newTotalSize;
		m_chunkCount = m_files.size();
		
	}
	
	m_handle.open(m_savefile, fs::fstream::in | fs::fstream::out | fs::fstream::binary | fs::fstream::ate);
	if(!m_handle.is_open()) {
		m_files.clear();
		LogError << "Failed to open defragmented save file: " << m_savefile;
		return false;
	}
	
	if(m_handle.tellg() < fs::fstream::pos_type(m_totalSize + 4)) {
		m_files.clear();
		m_handle.close();
		LogError << "Save file corrupted after defragmenting: " << m_savefile;
		return false;
	}
	
	return renamed;
}

bool SaveBlock::save(const std::string & name, const char * data, size_t size) {
	
	if(!m_handle) {
		return false;
	}
	
	arx_assert_msg(name.find_first_of(BADSAVCHAR) == std::string::npos,
	               "bad save filename: \"%s\"", name.c_str());
	
	File * file = &m_files[name];
	
	file->uncompressedSize = size;
	
	if(size == 0) {
		file->comp = File::None;
		file->storedSize = 0;
		return true;
	}
	
	uLongf compressedSize = size - 1;
	std::vector<char> compressed(compressedSize);
	const char * p;
	if(compress2(reinterpret_cast<Bytef *>(&compressed[0]), &compressedSize,
	             reinterpret_cast<const Bytef *>(data), size, 1) == Z_OK) {
		file->comp = File::Deflate;
		file->storedSize = compressedSize;
		p = &compressed[0];
	} else {
		file->comp = File::None;
		file->storedSize = size;
		p = data;
	}
	
	LogDebug("saving " << name << " " << file->uncompressedSize << " " << file->storedSize);
	
	size_t remaining = file->storedSize;
	
	for(File::ChunkList::iterator chunk = file->chunks.begin();
	    chunk != file->chunks.end(); ++chunk) {
		
		m_handle.seekp(chunk->offset + 4);
		
		if(chunk->size > remaining) {
			m_usedSize -= chunk->size - remaining;
			chunk->size = remaining;
		}
		
		m_handle.write(p, chunk->size);
		p += chunk->size;
		remaining -= chunk->size;
		
		if(remaining == 0) {
			file->chunks.erase(++chunk, file->chunks.end());
			return true;
		}
	}
	
	file->chunks.push_back(File::Chunk(remaining, m_totalSize));
	m_handle.seekp(m_totalSize + 4);
	m_handle.write(p, remaining);
	m_totalSize += remaining, m_usedSize += remaining, m_chunkCount++;
	
	return !m_handle.fail();
}

void SaveBlock::remove(const std::string & name) {
	m_files.erase(name);
}

std::string SaveBlock::load(const std::string & name) {
	
	arx_assert_msg(name.find_first_of(BADSAVCHAR) == std::string::npos,
	               "bad save filename: \"%s\"", name.c_str());
	
	Files::const_iterator file = m_files.find(name);
	if(file == m_files.end()) {
		return std::string();
	}
	
	return file->second.loadData(m_handle, name);
}

bool SaveBlock::hasFile(const std::string & name) const {
	arx_assert_msg(name.find_first_of(BADSAVCHAR) == std::string::npos,
	               "bad save filename: \"%s\"", name.c_str());
	return (m_files.find(name) != m_files.end());
}

std::vector<std::string> SaveBlock::getFiles() const {
	
	std::vector<std::string> result;
	
	for(Files::const_iterator file = m_files.begin(); file != m_files.end(); ++file) {
		result.push_back(file->first);
	}
	
	return result;
}

std::string SaveBlock::load(const fs::path & savefile, const std::string & name) {
	
	arx_assert_msg(name.find_first_of(BADSAVCHAR) == std::string::npos,
	               "bad save filename: \"%s\"", name.c_str());
	
	LogDebug("reading savefile " << savefile);
	
	fs::ifstream handle(savefile, fs::fstream::in | fs::fstream::binary);
	if(!handle.is_open()) {
		LogWarning << "Cannot open save file " << savefile;
		return std::string();
	}
	
	u32 fatOffset;
	if(fs::read(handle, fatOffset).fail()) {
		return std::string();
	}
	if(handle.seekg(fatOffset + 4).fail()) {
		LogError << "Cannot seek to FAT";
		return std::string();
	}
	
	u32 version;
	if(fs::read(handle, version).fail()) {
		return std::string();
	}
	if(version != SAV_VERSION_DEFLATE && version != SAV_VERSION_RELEASE && version != SAV_VERSION_NOEXT) {
		LogWarning << "Unexpected savegame version: " << version << " for " << savefile;
	}
	
	u32 nFiles;
	if(fs::read(handle, nFiles).fail()) {
		return std::string();
	}
	
	File file;
	
	for(u32 i = 0; i < nFiles; i++) {
		
		// Read the file name.
		std::string filename;
		if(fs::read(handle, filename).fail()) {
			return std::string();
		}
		if(version < SAV_VERSION_NOEXT) {
			boost::to_lower(filename);
			if(filename.size() > 4 && !filename.compare(filename.size() - 4, 4, ".sav", 4)) {
				filename.resize(filename.size() - 4);
			}
		}
		
		if(!file.loadOffsets(handle, version)) {
			return std::string();
		}
		
		if(!i && version == SAV_VERSION_OLD) {
			continue;
		}
		
		if(filename != name) {
			file.chunks.clear();
			continue;
		}
		
		return file.loadData(handle, name);
	}
	
	return std::string();
}
