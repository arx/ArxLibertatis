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

#include "io/Filesystem.h"
#include "io/HashMap.h"
#include "io/Logger.h"

const u32 SAV_VERSION_OLD = (1<<16) | 0;
const u32 SAV_VERSION = SAV_VERSION_OLD + 1;

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
	
	string name;
	size_t size;
	ChunkList chunks;
	
	File(const string & _name, size_t _size = 0) : name(_name), size(_size), chunks() { };
	
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
	LogDebug << "FAT offset is " << fatOffset;
	if(FileSeek(handle, fatOffset + 4, SEEK_SET) != fatOffset + 4) {
		LogError << "cannot seek to FAT";
		return false;
	}
	
	if(FileRead(handle, &version, 4) != 4) {
		return false;
	}
	if(version !=  SAV_VERSION) {
		LogDebug << "unexpected savegame version: " << version << " for " << savefile;
	}
	
	u32 nFiles;
	if(FileRead(handle, &nFiles, 4) != 4) {
		return false;
	}
	LogDebug << "number of files is " << nFiles;
	
	size_t hashMapSize = 1;
	while(hashMapSize < nFiles) {
		hashMapSize <<= 1;
	}
	int iNbHacheTroisQuart = (hashMapSize * 3) / 4;
	if(nFiles > iNbHacheTroisQuart) {
		hashMapSize <<= 1;
	}
	hashMap = new HashMap(hashMapSize);
	
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
		
		files.push_back(File(name));
		File & file = files.back();
		
		// ignore the size, calculate from chunks
		FileSeek(handle, 4, SEEK_CUR);
		
		u32 nChunks;
		if(FileRead(handle, &nChunks, 4) != 4) {
			return false;
		}
		file.chunks.reserve(nChunks);
		
		// ignored
		FileSeek(handle, 4, SEEK_CUR);
		
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
		
		file.size = size;
	}
	
	// TODO shouldn't this be done when loading the FAT?
	if(version == SAV_VERSION_OLD && !files.empty()) {
		files.erase(files.begin());
	}
	
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

void SaveBlock::writeFileTable() {
	
	size_t fatOffset = totalSize;
	FileSeek(handle, fatOffset + 4, SEEK_SET);
	
	FileWrite(handle, &SAV_VERSION, 4);
	
	u32 nFiles = files.size();
	FileWrite(handle, &nFiles, 4);
	
	for(FileList::iterator i = files.begin(); i != files.end(); ++i) {
		
		FileWrite(handle, i->name.c_str(), i->name.length() + 1);
		
		u32 size = i->size;
		FileWrite(handle, &size, 4);
		
		u32 nChunks = i->chunks.size();
		FileWrite(handle, &nChunks, 4);
		
		// TODO ok....
		u32 entrySize = nChunks * (sizeof(u32) + sizeof(u32) + i->name.length() + 1 + 20);
		FileWrite(handle, &entrySize, 4);
		
		for(File::ChunkList::iterator chunk = i->chunks.begin();
		    chunk != i->chunks.end(); ++chunk) {
			
			u32 chunkSize = chunk->size;
			FileWrite(handle, &chunk, 4);
			
			u32 chunkOffset = chunk->offset;
			FileWrite(handle, &chunkOffset, 4);
			
		}
		
	}
	
	FileSeek(handle, 0, SEEK_SET);
	FileWrite(handle, &fatOffset, 4);
	
}

bool SaveBlock::EndSave(void) {
	
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
	
	string tempFileName = savefile + "DFG";
	FileHandle tempFile = FileOpenWrite(tempFileName.c_str());
	
	u32 fakeFATOffset = 0;
	FileWrite(tempFile, &fakeFATOffset, 4);
	
	totalSize = 0;
	
	for(FileList::iterator i = files.begin(); i != files.end(); ++i) {
		
		char * buf = new char[i->size];
		char * p = buf;
		
		for(File::ChunkList::iterator chunk = i->chunks.begin();
		    chunk != i->chunks.end(); ++chunk) {
			FileSeek(handle, chunk->offset + 4, SEEK_SET);
			FileRead(handle, p, chunk->size);
			p += chunk->size;
		}
		
		FileWrite(tempFile, buf, i->size);
		
		i->chunks.resize(1);
		i->chunks.front().offset = totalSize;
		i->chunks.front().size = i->size;
		
		delete[] buf;
		
		totalSize += i->size;
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
	
	for(FileList::iterator i = files.begin(); i != files.end(); ++i) {
		if(!strcasecmp(i->name.c_str(), name.c_str())) {
			return &*i;
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
		files.push_back(File(name, size));
		file = &files.back();
	} else {
		file->size = size;
	}
	
	// TODO implode
	//implodeAlloc((char *)dat, pos, cpr_pos);
	//for (long i = 0; i < cpr_pos; i += 2)
	//	compressed[i] = ~compressed[i];
	
	size_t remaining = size;
	
	for(File::ChunkList::iterator i = file->chunks.begin();
	    i != file->chunks.end() && remaining; ++i) {
		
		FileSeek(handle, i->size + 4, SEEK_SET);
		
		if(i->size > remaining) {
			i->size = remaining;
		}
		
		FileWrite(handle, data, i->size);
		data += i->size;
		remaining -= i->size;
	}
	
	FileSeek(handle, totalSize + 4, SEEK_SET);
	
	if(remaining) {
		file->chunks.push_back(FileChunk(remaining, totalSize));
		totalSize += remaining;
		FileWrite(handle, data, remaining);
	}
	
	return true;
}

char * SaveBlock::load(const string & name, size_t & size) const {
	
	const File * file = (const File *)hashMap->get(name);
	if(!file) {
		size = 0;
		return NULL;
	}
	
	char * buf = (char*)malloc(file->size);
	char * p = buf;
	
	for(File::ChunkList::const_iterator i = file->chunks.begin();
	    i != file->chunks.end(); ++i) {
		FileSeek(handle, i->offset + 4, SEEK_SET);
		FileRead(handle, p, i->size);
		p += i->size;
	}
	
	size = file->size;
	
	// TODO implode
	//for (size_t i = 0; i < size; i += 2)
	//	compressed[i] = ~compressed[i];
	//dat = (unsigned char *)blastMemAlloc(compressed, ssize, size); 
	
	return buf;
}

bool SaveBlock::hasFile(const string & name) const {
	
	if(hashMap) {
		File * file = (File *)hashMap->get(name);
		return file ? true : false;
	} else {
		// TODO always create the hashMap
		for(FileList::const_iterator i = files.begin(); i != files.end(); ++i) {
			if(!strcasecmp(i->name.c_str(), name.c_str())) {
				return true;
			}
		}
		return false;
	}
	
}
