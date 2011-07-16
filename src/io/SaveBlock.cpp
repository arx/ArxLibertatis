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

#include "io/SaveBlock.h"

#include <cstdlib>
#include <cassert>

#include <zlib.h>

#include "io/Filesystem.h"
#include "io/HashMap.h"
#include "io/Logger.h"
#include "io/Blast.h"

#include "platform/String.h"
#include "platform/Platform.h"

using std::string;
using std::vector;
using std::min;

static const u32 SAV_VERSION_OLD = (1<<16) | 0;
static const u32 SAV_VERSION_RELEASE = (1<<16) | 1;
static const u32 SAV_VERSION_CURRENT = (2<<16) | 0;

static const u32 SAV_COMP_NONE = 0;
static const u32 SAV_COMP_IMPLODE = 1;
static const u32 SAV_COMP_DEFLATE = 2;

struct FileChunk {
	
	size_t size;
	size_t offset;
	
	FileChunk() : size(0), offset(0) { };
	FileChunk(size_t _size, size_t _offset) : size(_size), offset(_offset) { };
	
};

struct SaveBlock::File {
	
	typedef vector<FileChunk> ChunkList;
	
	enum Compression {
		Unknown,
		None,
		ImplodeCrypt,
		Deflate
	};
	
	string name;
	size_t storedSize;
	size_t uncompressedSize;
	ChunkList chunks;
	Compression comp;
	
	File(const string & _name = string()) : name(_name), chunks() { };
	
	const char * compressionName() const {
		switch(comp) {
			case SaveBlock::File::None: return "none";
			case SaveBlock::File::ImplodeCrypt: return "implode+crypt";
			case SaveBlock::File::Deflate: return "deflate";
			default: return "(unknown)";
		}
	}
	
	bool loadOffsets(FileHandle handle, u32 version);
	
	char * loadData(FileHandle handle, size_t & size) const;
	
};

bool SaveBlock::File::loadOffsets(FileHandle handle, u32 version) {
	
	if(version < SAV_VERSION_CURRENT) {
		// ignore the size, calculate from chunks
		FileSeek(handle, 4, SEEK_CUR);
		uncompressedSize = (size_t)-1;
	} else {
		u32 uncompressed;
		if(FileRead(handle, &uncompressed, 4) != 4) {
			return false;
		}
		uncompressedSize = uncompressed;
	}
	
	u32 nChunks;
	if(FileRead(handle, &nChunks, 4) != 4) {
		return false;
	}
	if(version < SAV_VERSION_CURRENT && nChunks == 0) {
		nChunks = 1;
	}
	chunks.resize(nChunks);
	
	if(version < SAV_VERSION_CURRENT) {
		// ignored
		FileSeek(handle, 4, SEEK_CUR);
		comp = File::ImplodeCrypt;
	} else {
		u32 compid;
		if(FileRead(handle, &compid, 4) != 4) {
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
		if(FileRead(handle, &chunkSize, 4) != 4) {
			return false;
		}
		size += chunkSize;
		
		u32 chunkOffset;
		if(FileRead(handle, &chunkOffset, 4) != 4) {
		}
		
		chunks[i].size = chunkSize;
		chunks[i].offset = chunkOffset;
	}
	
	storedSize = size;
	
	return true;
}

char * SaveBlock::File::loadData(FileHandle handle, size_t & size) const {
	
	LogDebug << "Loading " << name << " " << storedSize << "b in " << chunks.size() << " chunks, "
	         << compressionName() << " -> " << (int)uncompressedSize << "b";
	
	char * buf = (char*)malloc(storedSize);
	char * p = buf;
	
	for(File::ChunkList::const_iterator chunk = chunks.begin();
	    chunk != chunks.end(); ++chunk) {
		FileSeek(handle, chunk->offset + 4, SEEK_SET);
		FileRead(handle, p, chunk->size);
		p += chunk->size;
	}
	
	assert(p == buf + storedSize);
	
	switch(comp) {
		
		case File::None: {
			assert(uncompressedSize == storedSize);
			size = uncompressedSize;
			return buf;
		}
		
		case File::ImplodeCrypt: {
			unsigned char * crypt = (unsigned char *)buf;
			for(size_t i = 0; i < storedSize; i += 2) {
				crypt[i] = ~crypt[i];
			}
			char * uncompressed = blastMemAlloc(buf, storedSize, size);
			free(buf);
			if(!uncompressed) {
				LogError << "error decompressing imploded " << name;
				return NULL;
			}
			assert(uncompressedSize == (size_t)-1 || size == uncompressedSize);
			return uncompressed;
		}
		
		case File::Deflate: {
			assert(uncompressedSize != (size_t)-1);
			uLongf decompressedSize = uncompressedSize;
			char * uncompressed = (char*)malloc(uncompressedSize);
			int ret = uncompress((Bytef*)uncompressed, &decompressedSize, (const Bytef*)buf, storedSize);
			if(ret != Z_OK) {
				LogError << "error decompressing deflated " << name << ": " << ret;
				free(buf);
				free(uncompressed);
				size = 0;
				return NULL;
			}
			if(decompressedSize != uncompressedSize) {
				LogError << "unexpedect uncompressed size " << decompressedSize << " while loading "
				         << name << ", expected " << uncompressedSize;
			}
			size = decompressedSize;
			free(buf);
			return uncompressed;
		}
		
		default: {
			LogError << "error decompressing " << name << ": unknown format";
			free(buf);
			size = 0;
			return NULL;
		}
		
	}
}


SaveBlock::SaveBlock(const string & _savefile) {
	
	savefile = _savefile;
	
	handle = NULL;
	totalSize = 0;
	
	firstSave = false;
	
	hashMap = NULL;
}

SaveBlock::~SaveBlock() {
	
	if(handle) {
		FileClose(handle);
		handle = NULL;
	}
	
	if(hashMap) {
		delete hashMap;
		hashMap = NULL;
	}
}

bool SaveBlock::BeginRead() {
	
	LogDebug << "reading savefile " << savefile;
	
	handle = FileOpenRead(savefile);
	if(!handle) {
		LogWarning << "cannot open save file " << savefile;
		return false;
	}
	
	if(!loadFileTable()) {
		LogError << "broken save file";
		return false;
	}
	
	size_t hashMapSize = 1;
	while(hashMapSize < files.size()) {
		hashMapSize <<= 1;
	}
	size_t iNbHacheTroisQuart = (hashMapSize * 3) / 4;
	if(files.size() > iNbHacheTroisQuart) {
		hashMapSize <<= 1;
	}
	hashMap = new HashMap(hashMapSize);
	
	for(FileList::iterator i = files.begin(); i != files.end(); ++i) {
		hashMap->add(i->name, &*i);
	}
	
	return true;
}

bool SaveBlock::loadFileTable() {
	
	u32 fatOffset;
	if(FileRead(handle, &fatOffset, 4) != 4) {
		return false;
	}
	if((size_t)FileSeek(handle, fatOffset + 4, SEEK_SET) != fatOffset + 4) {
		LogError << "cannot seek to FAT";
		return false;
	}
	totalSize = fatOffset;
	
	u32 version;
	if(FileRead(handle, &version, 4) != 4) {
		return false;
	}
	if(version != SAV_VERSION_CURRENT && version != SAV_VERSION_RELEASE) {
		LogWarning << "unexpected savegame version: " << version << " for " << savefile;
	}
	
	u32 nFiles;
	if(FileRead(handle, &nFiles, 4) != 4) {
		return false;
	}
	files.resize(version == SAV_VERSION_OLD ? nFiles - 1 : nFiles);
	
	for(u32 i = 0; i < nFiles; i++) {
		
		// Read the file name.
		string name;
		while(true) {
			char c;
			if(FileRead(handle, &c, 1) != 1) {
				return false;
			}
			if(c == '\0') {
				break;
			}
			name.push_back(c);
		}
		
		if(!i && version == SAV_VERSION_OLD) {
			File dummy;
			if(!dummy.loadOffsets(handle, version)) {
				return false;
			}
			continue;
		}
		
		File & file = (version == SAV_VERSION_OLD) ? files[i - 1] : files[i];
		
		file.name = name;
		
		if(!file.loadOffsets(handle, version)) {
			return false;
		}
	}
	
	return true;
}

void SaveBlock::writeFileTable() {
	
	LogDebug << "writeFileTable " << savefile;
	
	size_t fatOffset = totalSize;
	FileSeek(handle, fatOffset + 4, SEEK_SET);
	
	FileWrite(handle, &SAV_VERSION_CURRENT, 4);
	
	u32 nFiles = files.size();
	FileWrite(handle, &nFiles, 4);
	
	for(FileList::iterator file = files.begin(); file != files.end(); ++file) {
		
		FileWrite(handle, file->name.c_str(), file->name.length() + 1);
		
		u32 uncompressedSize = file->uncompressedSize;
		FileWrite(handle, &uncompressedSize, 4);
		
		u32 nChunks = file->chunks.size();
		FileWrite(handle, &nChunks, 4);
		
		u32 comp;
		switch(file->comp) {
			case File::None: comp = SAV_COMP_NONE; break;
			case File::ImplodeCrypt: comp = SAV_COMP_IMPLODE; break;
			case File::Deflate: comp = SAV_COMP_DEFLATE; break;
			case File::Unknown: comp = (u32)-1; break;
		}
		FileWrite(handle, &comp, 4);
		
		for(File::ChunkList::iterator chunk = file->chunks.begin();
		    chunk != file->chunks.end(); ++chunk) {
			
			u32 chunkSize = chunk->size;
			FileWrite(handle, &chunkSize, 4);
			
			u32 chunkOffset = chunk->offset;
			FileWrite(handle, &chunkOffset, 4);
			
		}
		
	}
	
	FileSeek(handle, 0, SEEK_SET);
	FileWrite(handle, &fatOffset, 4);
	
}

bool SaveBlock::BeginSave() {
	
	handle = FileOpenRead(savefile);
	
	if(!handle) {
		
		handle = FileOpenWrite(savefile);
		if(!handle) {
			LogError << "could not open " << savefile << " for writing";
			return false;
		}
		u32 fakeFATOffset = 0;
		FileWrite(handle, &fakeFATOffset, 4);
		firstSave = true;
		
	} else {
		
		if(!loadFileTable()) {
			LogError << "broken save file";
			return false;
		}
		
		FileClose(handle);
		handle = FileOpenReadWrite(savefile);
		if (!handle) {
			LogError << "could not open " << savefile << " read/write";
			return false;
		}
		
	}
	
	return true;
}

bool SaveBlock::flush() {
	
	// TODO why defragment every time?
	if(!firstSave) {
		return defragment();
	}
	
	writeFileTable();
	
	FileClose(handle);
	handle = NULL;
	files.clear();
	
	return true;
}

bool SaveBlock::defragment() {
	
	LogDebug << "defrag " << savefile;
	
	string tempFileName = savefile + "dfg";
	FileHandle tempFile = FileOpenWrite(tempFileName);
	
	u32 fakeFATOffset = 0;
	FileWrite(tempFile, &fakeFATOffset, 4);
	
	totalSize = 0;
	
	for(FileList::iterator file = files.begin(); file != files.end(); ++file) {
		
		if(file->storedSize == 0) {
			continue;
		}
		
		char * buf = new char[file->storedSize];
		char * p = buf;
		
		for(File::ChunkList::iterator chunk = file->chunks.begin();
		    chunk != file->chunks.end(); ++chunk) {
			FileSeek(handle, chunk->offset + 4, SEEK_SET);
			FileRead(handle, p, chunk->size);
			p += chunk->size;
		}
		
		assert(p == buf + file->storedSize);
		
		FileWrite(tempFile, buf, file->storedSize);
		
		file->chunks.resize(1);
		file->chunks.front().offset = totalSize;
		file->chunks.front().size = file->storedSize;
		
		delete[] buf;
		
		totalSize += file->storedSize;
	}
	
	FileClose(handle);
	FileDelete(savefile);
	
	handle = tempFile;
	writeFileTable();
	
	FileClose(handle);
	handle = NULL;
	files.clear();
	
	FileMove(tempFileName, savefile);
	
	return true;
}

SaveBlock::File * SaveBlock::getFile(const std::string & name) {
	
	for(FileList::iterator file = files.begin(); file != files.end(); ++file) {
		if(!strcasecmp(file->name, name)) {
			return &*file;
			break;
		}
	}
	
	return NULL;
}

bool SaveBlock::save(const string & name, const char * data, size_t size) {
	
	if(!handle) {
		return false;
	}
	
	File * file = getFile(name);
	if(!file) {
		files.push_back(File(name));
		file = &files.back();
	}
	
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
		LogWarning << "";
		file->comp = File::None;
		file->storedSize = size;
		p = data;
	}
	
	LogDebug << "saving " << name << " " << file->uncompressedSize << " " << file->storedSize;
	
	size_t remaining = file->storedSize;
	
	for(File::ChunkList::iterator chunk = file->chunks.begin();
	    chunk != file->chunks.end(); ++chunk) {
		
		FileSeek(handle, chunk->offset + 4, SEEK_SET);
		
		if(chunk->size > remaining) {
			chunk->size = remaining;
		}
		
		FileWrite(handle, p, chunk->size);
		p += chunk->size;
		remaining -= chunk->size;
		
		if(remaining == 0) {
			file->chunks.erase(++chunk, file->chunks.end());
			delete[] compressed;
			return true;
		}
	}
	
	file->chunks.push_back(FileChunk(remaining, totalSize));
	FileSeek(handle, totalSize + 4, SEEK_SET);
	FileWrite(handle, p, remaining);
	totalSize += remaining;
	
	delete[] compressed;
	
	return true;
}

char * SaveBlock::load(const string & name, size_t & size) const {
	
	const File * file;
	if(hashMap) {
		file = (File *)hashMap->get(name);
	} else {
		// TODO always create the hashMap
		file = NULL;
		for(FileList::const_iterator f = files.begin(); f != files.end(); ++f) {
			if(!strcasecmp(f->name, name)) {
				file = &*f;
			}
		}
	}
	if(!file) {
		LogWarning << "Could not load " << name << " from " << savefile;
		size = 0;
		return NULL;
	}
	
	return file->loadData(handle, size);
}

bool SaveBlock::hasFile(const string & name) const {
	
	if(hashMap) {
		File * file = (File *)hashMap->get(name);
		return file ? true : false;
	} else {
		// TODO always create the hashMap
		for(FileList::const_iterator file = files.begin(); file != files.end(); ++file) {
			if(!strcasecmp(file->name, name)) {
				return true;
			}
		}
		return false;
	}
	
}

vector<string> SaveBlock::getFiles() const {
	
	vector<string> result;
	
	for(FileList::const_iterator file = files.begin(); file != files.end(); ++file) {
		result.push_back(file->name);
	}
	
	return result;
}

class Autoclose {
	
public:
	
	Autoclose(FileHandle _handle) : handle(_handle) { }
	
	~Autoclose() {
		FileClose(handle);
	}
	
private:
	
	FileHandle handle;
	
};

char * SaveBlock::load(const std::string & savefile, const std::string & filename, size_t & size) {
	
	LogDebug << "reading savefile " << savefile;
	
	size = 0;
	
	FileHandle handle = FileOpenRead(savefile);
	if(!handle) {
		LogWarning << "cannot open save file " << savefile;
		return NULL;
	}
	
	Autoclose close(handle);
	
	u32 fatOffset;
	if(FileRead(handle, &fatOffset, 4) != 4) {
		return NULL;
	}
	if((size_t)FileSeek(handle, fatOffset + 4, SEEK_SET) != fatOffset + 4) {
		LogError << "cannot seek to FAT";
		return NULL;
	}
	
	u32 version;
	if(FileRead(handle, &version, 4) != 4) {
		return NULL;
	}
	if(version != SAV_VERSION_CURRENT && version != SAV_VERSION_RELEASE) {
		LogWarning << "unexpected savegame version: " << version << " for " << savefile;
	}
	
	u32 nFiles;
	if(FileRead(handle, &nFiles, 4) != 4) {
		return NULL;
	}
	
	
	File file;
	
	for(u32 i = 0; i < nFiles; i++) {
		
		// Read the file name.
		string name;
		while(true) {
			char c;
			if(FileRead(handle, &c, 1) != 1) {
				return NULL;
			}
			if(c == '\0') {
				break;
			}
			name.push_back(c);
		}
		
		if(!file.loadOffsets(handle, version)) {
			return NULL;
		}
		
		if(!i && version == SAV_VERSION_OLD) {
			continue;
		}
		
		if(strcasecmp(name, filename)) {
			file.chunks.clear();
			continue;
		}
		
		return file.loadData(handle, size);
	}
	
	return NULL;
}
