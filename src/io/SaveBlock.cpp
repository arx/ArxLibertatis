/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

static const u32 SAV_VERSION_OLD = (1<<16) | 0;
static const u32 SAV_VERSION_RELEASE = (1<<16) | 1;
static const u32 SAV_VERSION_DEFLATE = (2<<16) | 0;
static const u32 SAV_VERSION_NOEXT = (2<<16) | 1;

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
		case File::Unknown: _comp = (u32)-1; break;
	}
	fs::write(handle, _comp);
	
	for(File::ChunkList::const_iterator chunk = chunks.begin(); chunk != chunks.end(); ++chunk) {
		
		u32 chunkSize = chunk->size;
		fs::write(handle, chunkSize);
		
		u32 chunkOffset = chunk->offset;
		fs::write(handle, chunkOffset);
		
	}
	
}

char * SaveBlock::File::loadData(std::istream & handle, size_t & size, const std::string & name) const {
	
	LogDebug("Loading " << name << ' ' << storedSize << "b in " << chunks.size() << " chunks, "
	         << compressionName() << " -> " << (int)uncompressedSize << "b");
	
	if(storedSize == 0) {
		size = 0;
		return NULL;
	}
	
	char * buf = (char*)malloc(storedSize);
	char * p = buf;
	
	for(File::ChunkList::const_iterator chunk = chunks.begin();
	    chunk != chunks.end(); ++chunk) {
		handle.seekg(chunk->offset + 4);
		handle.read(p, chunk->size);
		p += chunk->size;
	}
	
	arx_assert(p == buf + storedSize);
	
	switch(comp) {
		
		case File::None: {
			arx_assert(uncompressedSize == storedSize);
			size = uncompressedSize;
			return buf;
		}
		
		case File::ImplodeCrypt: {
			unsigned char * crypt = (unsigned char *)buf;
			for(size_t i = 0; i < storedSize; i += 2) {
				crypt[i] = (unsigned char)~(unsigned int)crypt[i];
			}
			char * uncompressed = blastMemAlloc(buf, storedSize, size);
			free(buf);
			if(!uncompressed) {
				LogError << "Error decompressing imploded " << name;
				return NULL;
			}
			arx_assert(uncompressedSize == (size_t)-1 || size == uncompressedSize);
			return uncompressed;
		}
		
		case File::Deflate: {
			arx_assert(uncompressedSize != (size_t)-1);
			uLongf decompressedSize = uncompressedSize;
			char * uncompressed = (char*)malloc(uncompressedSize);
			int ret = uncompress((Bytef*)uncompressed, &decompressedSize, (const Bytef*)buf, storedSize);
			if(ret != Z_OK) {
				LogError << "Error decompressing deflated " << name << ": " << zError(ret) << " (" << ret << ')';
				free(buf);
				free(uncompressed);
				size = 0;
				return NULL;
			}
			if(decompressedSize != uncompressedSize) {
				LogError << "Unexpedect uncompressed size " << decompressedSize << " while loading "
				         << name << ", expected " << uncompressedSize;
			}
			size = decompressedSize;
			free(buf);
			return uncompressed;
		}
		
		default: {
			LogError << "Error decompressing " << name << ": unknown format";
			free(buf);
			size = 0;
			return NULL;
		}
		
	}
}

SaveBlock::SaveBlock(const fs::path & _savefile) : savefile(_savefile), totalSize(0), usedSize(0), chunkCount(0) { }

SaveBlock::~SaveBlock() { }

bool SaveBlock::loadFileTable() {
	
	handle.seekg(0);
	
	u32 fatOffset;
	if(fs::read(handle, fatOffset).fail()) {
		return false;
	}
	if(handle.seekg(fatOffset + 4).fail()) {
		LogError << "Cannot seek to FAT";
		return false;
	}
	totalSize = fatOffset;
	
	u32 version;
	if(fs::read(handle, version).fail()) {
		return false;
	}
	if(version != SAV_VERSION_DEFLATE && version != SAV_VERSION_RELEASE && version != SAV_VERSION_NOEXT) {
		LogWarning << "Unexpected savegame version: " << (version >> 16) << '.' << (version & 0xffff) << " for " << savefile;
	}
	
	u32 nFiles;
	if(fs::read(handle, nFiles).fail()) {
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
	files.rehash(hashMapSize);
	
	if(version == SAV_VERSION_OLD) {
		char c;
		do {
			c = static_cast<char>(handle.get());
		} while(c != '\0' && handle.good());
		File dummy;
		if(!dummy.loadOffsets(handle, version)) {
			return false;
		}
	}
	
	usedSize = 0;
	chunkCount = 0;
	
	for(u32 i = 0; i < nFiles; i++) {
		
		// Read the file name.
		std::string name;
		if(fs::read(handle, name).fail()) {
			return false;
		}
		if(version < SAV_VERSION_NOEXT) {
			boost::to_lower(name);
			if(name.size() > 4 && !name.compare(name.size() - 4, 4, ".sav", 4)) {
				name.resize(name.size() - 4);
			}
		}
		
		File & file = files[name];
		
		if(!file.loadOffsets(handle, version)) {
			return false;
		}
		
		usedSize += file.storedSize, chunkCount += file.chunks.size();
	}
	
	return true;
}

void SaveBlock::writeFileTable(const std::string & important) {
	
	LogDebug("writeFileTable " << savefile);
	
	u32 fatOffset = totalSize;
	handle.seekp(fatOffset + 4);
	
	fs::write(handle, SAV_VERSION_NOEXT);
	
	u32 nFiles = files.size();
	fs::write(handle, nFiles);
	
	Files::const_iterator ifile = files.find(important);
	if(ifile != files.end()) {
		ifile->second.writeEntry(handle, ifile->first);
	}
	
	for(Files::const_iterator file = files.begin(); file != files.end(); ++file) {
		if(file != ifile) {
			file->second.writeEntry(handle, file->first);
		}
	}
	
	handle.seekp(0);
	fs::write(handle, fatOffset);
	
}

bool SaveBlock::open(bool writable) {
	
	LogDebug("opening savefile " << savefile << " witable=" << writable);
	
	fs::fstream::openmode mode = fs::fstream::in | fs::fstream::binary | fs::fstream::ate;
	if(writable) {
		mode |= fs::fstream::out;
	}
	
	handle.clear();
	handle.open(savefile, mode);
	if(!handle.is_open()) {
		handle.clear();
		if(writable) {
			handle.open(savefile, mode | fs::fstream::trunc);
		}
		if(!handle.is_open()) {
			LogError << "Could not open " << savefile << " for "
			         << (writable ? "reading/writing" : "reading");
			return false;
		}
	}
	
	if(handle.tellg() > 0 && !loadFileTable()) {
		LogError << "Broken save file";
		return false;
	}
	
	return true;
}

bool SaveBlock::flush(const std::string & important) {
	
	arx_assert(important.find_first_of(BADSAVCHAR) == std::string::npos,
	           "bad save filename: \"%s\"", important.c_str());
	
	if((usedSize * 2 < totalSize || chunkCount > (files.size() * 4 / 3))) {
		defragment();
	}
	
	writeFileTable(important);
	
	handle.flush();
	
	return handle.good();
}

bool SaveBlock::defragment() {
	
	LogDebug("defragmenting " << savefile << " save: using " << usedSize << " / " << totalSize
	         << " b for " << files.size() << " files in " << chunkCount << " chunks");
	
	fs::path tempFileName = savefile;
	int i = 0;
	
	do {
		std::ostringstream oss;
		oss << "defrag" << i++;
		tempFileName.set_ext(oss.str());
	} while(fs::exists(tempFileName));
	
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
	BOOST_FOREACH(const File & file, files | boost::adaptors::map_values) {
		
		if(file.storedSize == 0) {
			continue;
		}
		
		buffer.resize(file.storedSize);
		char * p = &buffer.front();
		
		BOOST_FOREACH(const File::Chunk & chunk, file.chunks) {
			handle.seekg(chunk.offset + 4);
			handle.read(p, chunk.size);
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
	
	tempFile.flush(), tempFile.close(), handle.close();
	
	bool renamed = fs::rename(tempFileName, savefile, true);
	if(!renamed) {
		fs::remove(tempFileName);
		LogWarning << "Failed to move defragmented save file " << tempFileName << " to " << savefile;
	} else {
		
		size_t newTotalSize = 0;
		for(Files::iterator i = files.begin(); i != files.end(); ++i) {
			File & file = i->second;
			if(file.storedSize != 0) {
				file.chunks.resize(1);
				file.chunks.front().offset = newTotalSize;
				file.chunks.front().size = file.storedSize;
				newTotalSize += file.storedSize;
			}
		}
		arx_assert(checkTotalSize == newTotalSize);
		
		usedSize = totalSize = newTotalSize;
		chunkCount = files.size();
		
	}
	
	handle.open(savefile, fs::fstream::in | fs::fstream::out | fs::fstream::binary | fs::fstream::ate);
	if(!handle.is_open()) {
		files.clear();
		LogError << "Failed to open defragmented save file: " << savefile;
		return false;
	}
	
	if(handle.tellg() < fs::fstream::pos_type(totalSize + 4)) {
		files.clear();
		handle.close();
		LogError << "Save file corrupted after defragmenting: " << savefile;
		return false;
	}
	
	return renamed;
}

bool SaveBlock::save(const std::string & name, const char * data, size_t size) {
	
	if(!handle) {
		return false;
	}
	
	arx_assert(name.find_first_of(BADSAVCHAR) == std::string::npos,
	           "bad save filename: \"%s\"", name.c_str());
	
	File * file = &files[name];
	
	file->uncompressedSize = size;
	
	if(size == 0) {
		file->comp = File::None;
		file->storedSize = 0;
		return true;
	}
	
	uLongf compressedSize = size - 1;
	char * compressed = new char[compressedSize];
	const char * p;
	if(compress2((Bytef*)compressed, &compressedSize, (const Bytef*)data, size, 1) == Z_OK) {
		file->comp = File::Deflate;
		file->storedSize = compressedSize;
		p = compressed;
	} else {
		file->comp = File::None;
		file->storedSize = size;
		p = data;
	}
	
	LogDebug("saving " << name << " " << file->uncompressedSize << " " << file->storedSize);
	
	size_t remaining = file->storedSize;
	
	for(File::ChunkList::iterator chunk = file->chunks.begin();
	    chunk != file->chunks.end(); ++chunk) {
		
		handle.seekp(chunk->offset + 4);
		
		if(chunk->size > remaining) {
			usedSize -= chunk->size - remaining;
			chunk->size = remaining;
		}
		
		handle.write(p, chunk->size);
		p += chunk->size;
		remaining -= chunk->size;
		
		if(remaining == 0) {
			file->chunks.erase(++chunk, file->chunks.end());
			delete[] compressed;
			return true;
		}
	}
	
	file->chunks.push_back(File::Chunk(remaining, totalSize));
	handle.seekp(totalSize + 4);
	handle.write(p, remaining);
	totalSize += remaining, usedSize += remaining, chunkCount++;
	
	delete[] compressed;
	
	return !handle.fail();
}

void SaveBlock::remove(const std::string & name) {
	files.erase(name);
}

char * SaveBlock::load(const std::string & name, size_t & size) {
	
	arx_assert(name.find_first_of(BADSAVCHAR) == std::string::npos,
	           "bad save filename: \"%s\"", name.c_str());
	
	Files::const_iterator file = files.find(name);
	
	return (file == files.end()) ? NULL : file->second.loadData(handle, size, name);
}

bool SaveBlock::hasFile(const std::string & name) const {
	arx_assert(name.find_first_of(BADSAVCHAR) == std::string::npos,
	           "bad save filename: \"%s\"", name.c_str());
	return (files.find(name) != files.end());
}

std::vector<std::string> SaveBlock::getFiles() const {
	
	std::vector<std::string> result;
	
	for(Files::const_iterator file = files.begin(); file != files.end(); ++file) {
		result.push_back(file->first);
	}
	
	return result;
}

char * SaveBlock::load(const fs::path & savefile, const std::string & filename, size_t & size) {
	
	arx_assert(filename.find_first_of(BADSAVCHAR) == std::string::npos,
	           "bad save filename: \"%s\"", filename.c_str());
	
	LogDebug("reading savefile " << savefile);
	
	size = 0;
	
	fs::ifstream handle(savefile, fs::fstream::in | fs::fstream::binary);
	if(!handle.is_open()) {
		LogWarning << "Cannot open save file " << savefile;
		return NULL;
	}
	
	u32 fatOffset;
	if(fs::read(handle, fatOffset).fail()) {
		return NULL;
	}
	if(handle.seekg(fatOffset + 4).fail()) {
		LogError << "Cannot seek to FAT";
		return NULL;
	}
	
	u32 version;
	if(fs::read(handle, version).fail()) {
		return NULL;
	}
	if(version != SAV_VERSION_DEFLATE && version != SAV_VERSION_RELEASE && version != SAV_VERSION_NOEXT) {
		LogWarning << "Unexpected savegame version: " << version << " for " << savefile;
	}
	
	u32 nFiles;
	if(fs::read(handle, nFiles).fail()) {
		return NULL;
	}
	
	File file;
	
	for(u32 i = 0; i < nFiles; i++) {
		
		// Read the file name.
		std::string name;
		if(fs::read(handle, name).fail()) {
			return NULL;
		}
		if(version < SAV_VERSION_NOEXT) {
			boost::to_lower(name);
			if(name.size() > 4 && !name.compare(name.size() - 4, 4, ".sav", 4)) {
				name.resize(name.size() - 4);
			}
		}
		
		if(!file.loadOffsets(handle, version)) {
			return NULL;
		}
		
		if(!i && version == SAV_VERSION_OLD) {
			continue;
		}
		
		if(name != filename) {
			file.chunks.clear();
			continue;
		}
		
		return file.loadData(handle, size, name);
	}
	
	return NULL;
}
