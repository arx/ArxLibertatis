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
/*
		   _.               _.    .    J)
		HNMMMM) .      L HNMMMM)  #H   H)     ()
	   (MMMMMM) M)    (M(MMMMMM)  `H)._UL_   JM)
	   `MF" `4) M)    NN`MF" `4)    JMMMMMM#.4F
		M       #)    M) M         (MF (0DANN.
		M       (M   .M  M  .NHH#L (N JMMMN.H#
		M       (M   (M  M   4##HF QQ(MF`"H#(M.
		M       `M   (N  M         #U(H(N (MJM)
	   .M  __L   M)  M) .M  ___    4)U#NF (M0M)
	   (MMMMMM   M)  M) (MMMMMM)     U#NHLJM(M`
	   (MMMMN#   4) (M  (MMMMM#  _.  (H`HMM)JN
	   (M""      (M (M  (M""   (MM)  (ML____M)
	   (M        (M H)  (M     `"`  ..4MMMMM# D
	   (M        `M M)  (M         JM)  """`  N#
	   (M         MLM`  (M        #MF   (L    `F
	   (M         MMM   (M        H`    (#
	   (M         (MN   (M              (Q
	   `M####H    (M)   `M####H         ``
		MMMMMM    `M`    NMMMMM
		`Q###F     "     `4###F   sebastien scieux @2001

*/

#include "io/PakReader.h"

#include <dirent.h>

#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <cassert>

#include "io/Blast.h"
#include "io/PakEntry.h"
#include "io/HashMap.h"
#include "io/Logger.h"
#include "platform/String.h"

#include "platform/Platform.h"
#include <iomanip>

using std::min;
using std::max;
using std::strlen;
using std::string;
using std::vector;

#define PAK_READ_BUF_SIZE 1024

static const char PAK_KEY_DEMO[] = "NSIARKPRQPHBTE50GRIH3AYXJP2AMF3FCEYAVQO5QGA0JGIIH2AYXKVOA1VOGGU5GSQKKYEOIAQG1XRX0J4F5OEAEFI4DD3LL45VJTVOA1VOGGUKE50GRI";
static const char PAK_KEY_FULL[] = "AVQF3FCKE50GRIAYXJP2AMEYO5QGA0JGIIH2NHBTVOA1VOGGU5H3GSSIARKPRQPQKKYEOIAQG1XRX0J4F5OEAEFI4DD3LL45VJTVOA1VOGGUKE50GRIAYX";

static const char * selectKey(u32 first_bytes) {
	switch(first_bytes) {
		case 0x46515641:
			return PAK_KEY_FULL;
		case 0x4149534E:
			return PAK_KEY_DEMO;
		default:
			return NULL;
	}
}

/*! Uncompressed file in a .pak file archive. */
class UncompressedFile : public PakFile {
	
	FILE * archive;
	size_t offset;
	
public:
	
	UncompressedFile(FILE * _archive, size_t _offset, size_t size) : PakFile(size), archive(_archive), offset(_offset) { };
	
	void read(void * buf) const;
	
	PakFileHandle * open() const;
	
	friend class UncompressedFileHandle;
	
};

class UncompressedFileHandle : public PakFileHandle {
	
	const UncompressedFile * file;
	size_t offset;
	
public:
	
	UncompressedFileHandle(const UncompressedFile * _file) : file(_file), offset(0) { };
	
	size_t read(void * buf, size_t size);
	
	int seek(Whence whence, int offset);
	
	size_t tell() const;
	
	~UncompressedFileHandle() { };
	
};

void UncompressedFile::read(void * buf) const {
	
	fseek(archive, offset, SEEK_SET);
	
	size_t nread = fread(buf, size(), 1, archive);
	
	arx_assert(nread == 1);
}

PakFileHandle * UncompressedFile::open() const {
	return new UncompressedFileHandle(this);
}

size_t UncompressedFileHandle::read(void * buf, size_t size) {
	
	if(offset >= file->size()) {
		return 0;
	}
	
	fseek(file->archive, file->offset + offset, SEEK_SET);
	
	if(file->size() < offset + size) {
		size = (offset > file->size()) ? 0 : (file->size() - offset);
	}
	
	size_t nread = fread(buf, 1, size, file->archive);
	
	offset += nread;
	
	return nread;
}

int UncompressedFileHandle::seek(Whence whence, int _offset) {
	
	size_t base;
	switch(whence) {
		case SeekSet: base = 0; break;
		case SeekEnd: base = file->size(); break;
		case SeekCur: base = offset; break;
		default: return -1;
	}
	
	if((int)base + _offset < 0) {
		return -1;
	}
	
	offset = (int)base + _offset;
	
	return offset;
}

size_t UncompressedFileHandle::tell() const {
	return offset;
}

/*! Compressed file in a .pak file archive. */
class CompressedFile : public PakFile {
	
	FILE * archive;
	size_t offset;
	size_t storedSize;
	
public:
	
	CompressedFile(FILE * _archive, size_t _offset, size_t size, size_t _storedSize) : PakFile(size), archive(_archive), offset(_offset), storedSize(_storedSize) { };
	
	void read(void * buf) const;
	
	PakFileHandle * open() const;
	
	friend class CompressedFileHandle;
	
};

class CompressedFileHandle : public PakFileHandle {
	
	const CompressedFile * file;
	size_t offset;
	
public:
	
	CompressedFileHandle(const CompressedFile * _file) : file(_file), offset(0) { };
	
	size_t read(void * buf, size_t size);
	
	int seek(Whence whence, int offset);
	
	size_t tell() const;
	
	~CompressedFileHandle() { };
	
};

struct BlastFileInBuffer {
	
	FILE * file;
	
	unsigned char readbuf[PAK_READ_BUF_SIZE];
	
	BlastFileInBuffer(FILE * f) : file(f) {};
	
};

size_t blastInFile(void * Param, const unsigned char ** buf) {
	
	BlastFileInBuffer * p = (BlastFileInBuffer *)Param;
	
	*buf = p->readbuf;
	
	return fread(p->readbuf, 1, PAK_READ_BUF_SIZE, p->file);
}

static int blast(FILE * file, char * buf, size_t size) {
	
	BlastFileInBuffer in(file);
	BlastMemOutBuffer out(buf, size);
	
	return blast(blastInFile, &in, blastOutMem, &out);
}

void CompressedFile::read(void * buf) const {
	
	fseek(archive, offset, SEEK_SET);
	
	int r = blast(archive, (char *)buf, size());
	if(r) {
		LogError << "PakReader::Read: blast error " << r << " outSize=" << size();
	}
	
}

PakFileHandle * CompressedFile::open() const {
	return new CompressedFileHandle(this);
}

struct BlastMemOutBufferOffset {
	char * buf;
	size_t currentOffset;
	size_t startOffset;
	size_t endOffset;
};

int blastOutMemOffset(void * Param, unsigned char * buf, size_t len) {
	
	BlastMemOutBufferOffset * p = (BlastMemOutBufferOffset *)Param;
	
	assert(p->currentOffset <= p->endOffset);
	
	if(p->currentOffset == p->endOffset) {
		return 1;
	}
	
	if(p->currentOffset < p->startOffset) {
		size_t toStart = p->startOffset - p->currentOffset;
		if(len <= toStart) {
			p->currentOffset += len;
			return 0;
		} else {
			p->currentOffset = p->startOffset;
			buf += toStart;
			len -= toStart;
		}
	}
	
	size_t toCopy = min(len, p->endOffset - p->currentOffset);
	
	assert(toCopy != 0);
	
	memcpy(p->buf, buf, toCopy);
	
	p->currentOffset += toCopy;
	p->buf += toCopy;
	
	return 0;
}

size_t CompressedFileHandle::read(void * buf, size_t size) {
	
	if(offset >= file->size()) {
		return 0;
	}
	
	if(size < file->size() || offset != 0) {
		LogWarning << "partially reading a compressed file - inefficent: size=" << size
		           << " offset=" << offset << " total=" << file->size();
	}
	
	fseek(file->archive, file->offset, SEEK_SET);
	
	BlastFileInBuffer in(file->archive);
	BlastMemOutBufferOffset out;
	
	out.buf = (char *)buf;
	out.currentOffset = 0;
	out.startOffset = offset;
	out.endOffset = min(offset + size, file->size());
	
	if(out.endOffset <= out.startOffset) {
		return 0;
	}
	
	// TODO this is really inefficient
	int r = blast(blastInFile, &in, blastOutMemOffset, &out);
	if(r && (r != 1 || (size == file->size() && offset == 0))) {
		LogError << "PakReader::fRead: blast error " << r << " outSize=" << file->size();
		return 0;
	}
	
	size = out.currentOffset - out.startOffset;
	
	offset += size;
	
	return size;
}

int CompressedFileHandle::seek(Whence whence, int _offset) {
	
	size_t base;
	switch(whence) {
		case SeekSet: base = 0;
		case SeekEnd: base = file->size();
		case SeekCur: base = offset;
		default: return -1;
	}
	
	if((int)base + _offset < 0) {
		return -1;
	}
	
	offset = (int)base + _offset;
	
	return offset;
}

size_t CompressedFileHandle::tell() const {
	return offset;
}

/*! Plain file not in a .pak file archive. */
class PlainFile : public PakFile {
	
	string path;
	
public:
	
	PlainFile(const std::string & _path, size_t size) : PakFile(size), path(_path) { };
	
	void read(void * buf) const;
	
	PakFileHandle * open() const;
	
};

class PlainFileHandle : public PakFileHandle {
	
	FILE * handle;
	
public:
	
	PlainFileHandle(FILE * _handle) : handle(_handle) { };
	
	size_t read(void * buf, size_t size);
	
	int seek(Whence whence, int offset);
	
	size_t tell() const;
	
	~PlainFileHandle() { };
	
};

void PlainFile::read(void * buf) const {
	
	FILE * handle = fopen(path.c_str(), "rb");
	arx_assert(handle);
	
	size_t nread = fread(buf, size(), 1, handle);
	arx_assert(nread == 1);
	
	fclose(handle);
}

PakFileHandle * PlainFile::open() const {
	
	FILE * handle = fopen(path.c_str(), "rb");
	arx_assert(handle);
	
	return new PlainFileHandle(handle);
}

size_t PlainFileHandle::read(void * buf, size_t size) {
	return fread(buf, 1, size, handle);
}

int arxToCSeekOrigin[] = {
	SEEK_SET,
	SEEK_CUR,
	SEEK_END
};

int PlainFileHandle::seek(Whence whence, int offset) {
	return fseek(handle, offset, arxToCSeekOrigin[whence]);
}

size_t PlainFileHandle::tell() const {
	return ftell(handle);
}

PakReader::~PakReader() {
	clear();
}

static void pakDecrypt(char * fat, size_t fat_size, const char * key) {
	
	size_t keysize = strlen((const char *)key);
	
	for(size_t i = 0, ki = 0; i < fat_size; i++, ki = (ki + 1) % keysize) {
		fat[i] ^= key[ki];
	}
	
}

bool PakReader::addArchive(const string & name) {
	
	FILE * newfile = fopen(name.c_str(), "rb");
	
	if(!newfile) {
		return false;
	}
	
	// Read fat location and size.
	u32 fat_offset;
	u32 fat_size;
	if(fread(&fat_offset, sizeof(fat_offset), 1, newfile) != 1) {
		LogError << "error reading FAT offset";
		fclose(newfile);
		return false;
	}
	if(fseek(newfile, fat_offset, SEEK_SET)) {
		LogError << "error seeking to FAT offset " << fat_offset;
		fclose(newfile);
		return false;
	}
	if(fread(&fat_size, sizeof(fat_size), 1, newfile) != 1) {
		LogError << "error reading FAT size at offset " << fat_offset;
		fclose(newfile);
		return false;
	}
	
	// Read the whole FAT.
	char * fat = new char[fat_size];
	if(fread(fat, fat_size, 1, newfile) != 1) {
		LogError << "error reading FAT";
		delete[] fat;
		fclose(newfile);
		return false;
	}
	
	// Decrypt the FAT.
	const char * key = selectKey(*(u32*)fat);
	if(key) {
		pakDecrypt(fat, fat_size, key);
	} else {
		LogWarning << "WARNING: unknown PAK key ID 0x" << std::hex << std::setfill('0') << std::setw(8) << *(u32*)fat << ", assuming no key";
	}
	
	char * pos = fat;
	
	paks.push_back(newfile);
	
	while(fat_size) {
		
		char * dirname = safeGetString(pos, fat_size);
		if(!dirname) {
			LogError << "error reading directory name from FAT, wrong key?";
			goto error;
		}
		
		size_t len = strlen(dirname);
		std::transform(dirname, dirname + len, dirname, ::tolower);
		
		PakDirectory * dir = (*dirname != '\0') ? addDirectory(strref(dirname, len)) : this;
		
		u32 nfiles;
		if(!safeGet(nfiles, pos, fat_size)) {
			LogError << "error reading file count from FAT, wrong key?";
			goto error;
		}
		
		while(nfiles--) {
			
			char * filename =  safeGetString(pos, fat_size);
			if(!filename) {
				LogError << "error reading file name from FAT, wrong key?";
				goto error;
			}
			
			size_t len = strlen(filename);
			std::transform(filename, filename + len, filename, ::tolower);
			
			u32 offset;
			u32 flags;
			u32 uncompressedSize;
			u32 size;
			if(!safeGet(offset, pos, fat_size) || !safeGet(flags, pos, fat_size)
			   || !safeGet(uncompressedSize, pos, fat_size) || !safeGet(size, pos, fat_size)) {
				LogError << "error reading file attributes from FAT, wrong key?";
				goto error;
			}
			
			const u32 PAK_FILE_COMPRESSED = 1;
			PakFile * file;
			if((flags & PAK_FILE_COMPRESSED) && size != 0) {
				file = new CompressedFile(newfile, offset, uncompressedSize, size);
			} else {
				file = new UncompressedFile(newfile, offset, size);
			}
			
			dir->addFile(string(filename, len), file);
		}
		
	}
	
	delete[] fat;
	
	LogInfo << "Loaded PAK " << name;
	return true;
	
	
error:
	
	delete[] fat;
	
	return false;
}

void PakReader::clear() {
	
	files.clear();
	dirs.clear();
	
	for(vector<FILE*>::const_iterator i = paks.begin(); i != paks.end(); i++) {
		fclose(*i);
	}
}

// TODO remove
PakFile * PakReader::getFile(const std::string & path) {
	arx_assert(path[0] != '\\');
	return PakDirectory::getFile(toLowercase(path));
}

bool PakReader::read(const string & name, void * buf) {
	
	PakFile * f = getFile(name);
	if(!f) {
		return false;
	}
	
	f->read(buf);
	
	return true;
}

char * PakReader::readAlloc(const string & name, size_t & sizeRead) {
	
	PakFile * f = getFile(name);
	if(!f) {
		return NULL;
	}
	
	sizeRead = f->size();
	
	return f->readAlloc();
}

PakFileHandle * PakReader::open(const string & name) {
	
	PakFile * f = getFile(name);
	if(!f) {
		return NULL;
	}
	
	return f->open();
}

bool PakReader::addFiles(const string & path) {
	return addFiles(this, path);
}

bool PakReader::addFiles(PakDirectory * dir, const string & path) {
	
	DIR * d = opendir(path.c_str());
	if(!d) {
		return false;
	}
	
	struct dirent * ent;
	
	while((ent = readdir(d)) != NULL) {
		
		if(ent->d_name[0] == '.') {
			continue;
		}
		
		string name = path + '/' + ent->d_name;
		// TODO remove
		string entry = toLowercase(ent->d_name);
		
		if(addFiles(dir->addDirectory(entry), name)) {
			continue;
		}
		
		FILE * f = fopen(name.c_str(), "rb");
		if(!f) {
			continue;
		}
		
		fseek(f, 0, SEEK_END);
		
		int size = ftell(f);
		if(size < 0) {
			continue;
		}
		
		fclose(f);
		
		dir->addFile(entry, new PlainFile(name, size));
		
	}
	
	closedir(d);
	
	return true;
}

PakReader * resources;
