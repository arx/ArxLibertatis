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

const u32 SAV_VERSION_OLD = (1<<16) | 0;
const u32 SAV_VERSION_RELEASE = (1<<16) | 1;
const u32 SAV_VERSION_CURRENT = (2<<16) | 0;

const u32 SAV_COMP_NONE = 0;
const u32 SAV_COMP_IMPLODE = 1;
const u32 SAV_COMP_DEFLATE = 2;

using std::string;
using std::vector;
using std::min;

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
	
	File(const string & _name) : name(_name), chunks() { };
	
	const char * compressionName() const {
		switch(comp) {
			case SaveBlock::File::None: return "none";
			case SaveBlock::File::ImplodeCrypt: return "implode+crypt";
			case SaveBlock::File::Deflate: return "deflate";
			default: return "(unknown)";
		}
	}
	
};

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
	
	handle = FileOpenRead(savefile.c_str());
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
	files.reserve(nFiles);
	
	while(nFiles--) {
		
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
		
		//LogDebug << "name: " << name;
		
		files.push_back(File(name));
		File & file = files.back();
		
		if(version < SAV_VERSION_CURRENT) {
			// ignore the size, calculate from chunks
			FileSeek(handle, 4, SEEK_CUR);
			file.uncompressedSize = (size_t)-1;
		} else {
			u32 uncompressedSize;
			if(FileRead(handle, &uncompressedSize, 4) != 4) {
				return false;
			}
			file.uncompressedSize = uncompressedSize;
		}
		
		u32 nChunks;
		if(FileRead(handle, &nChunks, 4) != 4) {
			return false;
		}
		if(version < SAV_VERSION_CURRENT && nChunks == 0) {
			nChunks = 1;
		}
		file.chunks.reserve(nChunks);
		
		if(version < SAV_VERSION_CURRENT) {
			// ignored
			FileSeek(handle, 4, SEEK_CUR);
			file.comp = File::ImplodeCrypt;
		} else {
			u32 comp;
			if(FileRead(handle, &comp, 4) != 4) {
				return false;
			}
			switch(comp) {
				case SAV_COMP_NONE: file.comp = File::None; break;
				case SAV_COMP_IMPLODE: file.comp = File::ImplodeCrypt; break;
				case SAV_COMP_DEFLATE: file.comp = File::Deflate; break;
				default: file.comp = File::Unknown;
			}
		}
		
		size_t size = 0;
		while(nChunks--) {
			
			u32 chunkSize;
			if(FileRead(handle, &chunkSize, 4) != 4) {
				return false;
			}
			size += chunkSize;
			
			u32 chunkOffset;
			if(FileRead(handle, &chunkOffset, 4) != 4) {
			}
			
			file.chunks.push_back(FileChunk(chunkSize, chunkOffset));
			
		}
		
		file.storedSize = size;
	}
	
	if(version == SAV_VERSION_OLD && !files.empty()) {
		files.erase(files.begin());
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
	
	handle = FileOpenRead(savefile.c_str());
	
	if(!handle) {
		
		handle = FileOpenWrite(savefile.c_str());
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
		handle = FileOpenReadWrite(savefile.c_str());
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
	
	string tempFileName = savefile + "DFG";
	FileHandle tempFile = FileOpenWrite(tempFileName.c_str());
	
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
		if(!strcasecmp(file->name.c_str(), name.c_str())) {
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
	
	const File * file = (const File *)hashMap->get(name);
	if(!file) {
		LogWarning << "Could not load " << name << " from " << savefile;
		size = 0;
		return NULL;
	}
	
	LogDebug << "Loading " << name << " " << file->storedSize << "b in "
	         << file->chunks.size() << " chunks, " << file->compressionName()
	         << " -> " << (int)file->uncompressedSize << "b";
	
	char * buf = (char*)malloc(file->storedSize);
	char * p = buf;
	
	for(File::ChunkList::const_iterator chunk = file->chunks.begin();
	    chunk != file->chunks.end(); ++chunk) {
		FileSeek(handle, chunk->offset + 4, SEEK_SET);
		FileRead(handle, p, chunk->size);
		p += chunk->size;
	}
	
	assert(p == buf + file->storedSize);
	
	switch(file->comp) {
		
		case File::None: {
			assert(file->uncompressedSize == file->storedSize);
			size = file->uncompressedSize;
			return buf;
		}
		
		case File::ImplodeCrypt: {
			unsigned char * crypt = (unsigned char *)buf;
			for(size_t i = 0; i < file->storedSize; i += 2) {
				crypt[i] = ~crypt[i];
			}
			char * uncompressed = blastMemAlloc(buf, file->storedSize, size);
			free(buf);
			if(!uncompressed) {
				LogError << "error decompressing imploded " << name << " in " << savefile;
				return NULL;
			}
			assert(file->uncompressedSize == (size_t)-1 || size == file->uncompressedSize);
			return uncompressed;
		}
		
		case File::Deflate: {
			assert(file->uncompressedSize != (size_t)-1);
			uLongf uncompressedSize = file->uncompressedSize;
			char * uncompressed = (char*)malloc(uncompressedSize);
			int ret = uncompress((Bytef*)uncompressed, &uncompressedSize, (const Bytef*)buf,
			          file->storedSize);
			if(ret != Z_OK) {
				LogError << "error decompressing deflated " << name << " in " << savefile << ": " << ret;
				free(buf);
				free(uncompressed);
				size = 0;
				return NULL;
			}
			if(uncompressedSize != file->uncompressedSize) {
				LogError << "unexpedect uncompressed size " << uncompressedSize << " while loading "
				         << name << " in " << savefile << ", expected " << file->uncompressedSize;
			}
			size = file->uncompressedSize;
			free(buf);
			return uncompressed;
		}
		
		default: {
			LogError << "error decompressing " << name << " in " << savefile << ": unknown format";
			free(buf);
			size = 0;
			return NULL;
		}
		
	}
}

bool SaveBlock::hasFile(const string & name) const {
	
	if(hashMap) {
		File * file = (File *)hashMap->get(name);
		return file ? true : false;
	} else {
		// TODO always create the hashMap
		for(FileList::const_iterator file = files.begin(); file != files.end(); ++file) {
			if(!strcasecmp(file->name.c_str(), name.c_str())) {
				return true;
			}
		}
		return false;
	}
	
}
