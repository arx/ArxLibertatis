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

#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <iomanip>

#include "io/Blast.h"
#include "io/PakEntry.h"
#include "io/Logger.h"
#include "io/FilePath.h"
#include "io/Filesystem.h"

#include "platform/String.h"
#include "platform/Platform.h"

using std::min;
using std::max;
using std::strlen;
using std::string;
using std::vector;

namespace {

const size_t PAK_READ_BUF_SIZE = 1024;

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
	
	std::istream & archive;
	size_t offset;
	
public:
	
	UncompressedFile(std::istream * _archive, size_t _offset, size_t size) : PakFile(size), archive(*_archive), offset(_offset) { };
	
	void read(void * buf) const;
	
	PakFileHandle * open() const;
	
	friend class UncompressedFileHandle;
	
};

class UncompressedFileHandle : public PakFileHandle {
	
	const UncompressedFile & file;
	size_t offset;
	
public:
	
	UncompressedFileHandle(const UncompressedFile * _file) : file(*_file), offset(0) { };
	
	size_t read(void * buf, size_t size);
	
	int seek(Whence whence, int offset);
	
	size_t tell();
	
	~UncompressedFileHandle() { };
	
};

void UncompressedFile::read(void * buf) const {
	
	archive.seekg(offset);
	
	fread(archive, buf, size());
	
	arx_assert(!archive.fail());
}

PakFileHandle * UncompressedFile::open() const {
	return new UncompressedFileHandle(this);
}

size_t UncompressedFileHandle::read(void * buf, size_t size) {
	
	if(offset >= file.size()) {
		return 0;
	}
	
	file.archive.seekg(file.offset + offset);
	
	if(file.size() < offset + size) {
		size = (offset > file.size()) ? 0 : (file.size() - offset);
	}
	
	fread(file.archive, buf, size);
	
	size_t nread = file.archive.gcount();
	offset += nread;
	
	return nread;
}

int UncompressedFileHandle::seek(Whence whence, int _offset) {
	
	size_t base;
	switch(whence) {
		case SeekSet: base = 0; break;
		case SeekEnd: base = file.size(); break;
		case SeekCur: base = offset; break;
		default: return -1;
	}
	
	if((int)base + _offset < 0) {
		return -1;
	}
	
	offset = (int)base + _offset;
	
	return offset;
}

size_t UncompressedFileHandle::tell() {
	return offset;
}

/*! Compressed file in a .pak file archive. */
class CompressedFile : public PakFile {
	
	std::ifstream & archive;
	size_t offset;
	size_t storedSize;
	
public:
	
	CompressedFile(std::ifstream * _archive, size_t _offset, size_t size, size_t _storedSize) : PakFile(size), archive(*_archive), offset(_offset), storedSize(_storedSize) { };
	
	void read(void * buf) const;
	
	PakFileHandle * open() const;
	
	friend class CompressedFileHandle;
	
};

class CompressedFileHandle : public PakFileHandle {
	
	const CompressedFile & file;
	size_t offset;
	
public:
	
	CompressedFileHandle(const CompressedFile * _file) : file(*_file), offset(0) { };
	
	size_t read(void * buf, size_t size);
	
	int seek(Whence whence, int offset);
	
	size_t tell();
	
	~CompressedFileHandle() { };
	
};

struct BlastFileInBuffer {
	
	std::ifstream & file;
	
	unsigned char readbuf[PAK_READ_BUF_SIZE];
	
	BlastFileInBuffer(std::ifstream * f) : file(*f) {};
	
};

size_t blastInFile(void * Param, const unsigned char ** buf) {
	
	BlastFileInBuffer * p = (BlastFileInBuffer *)Param;
	
	*buf = p->readbuf;
	
	fread(p->file, p->readbuf, PAK_READ_BUF_SIZE);
	return p->file.gcount();
}

static int blast(std::ifstream & file, char * buf, size_t size) {
	
	BlastFileInBuffer in(&file);
	BlastMemOutBuffer out(buf, size);
	
	return blast(blastInFile, &in, blastOutMem, &out);
}

void CompressedFile::read(void * buf) const {
	
	archive.seekg(offset);
	
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
	
	arx_assert(p->currentOffset <= p->endOffset);
	
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
	
	arx_assert(toCopy != 0);
	
	memcpy(p->buf, buf, toCopy);
	
	p->currentOffset += toCopy;
	p->buf += toCopy;
	
	return 0;
}

size_t CompressedFileHandle::read(void * buf, size_t size) {
	
	if(offset >= file.size()) {
		return 0;
	}
	
	if(size < file.size() || offset != 0) {
		LogWarning << "partially reading a compressed file - inefficent: size=" << size
		           << " offset=" << offset << " total=" << file.size();
	}
	
	file.archive.seekg(file.offset);
	
	BlastFileInBuffer in(&file.archive);
	BlastMemOutBufferOffset out;
	
	out.buf = (char *)buf;
	out.currentOffset = 0;
	out.startOffset = offset;
	out.endOffset = min(offset + size, file.size());
	
	if(out.endOffset <= out.startOffset) {
		return 0;
	}
	
	// TODO this is really inefficient
	int r = ::blast(blastInFile, &in, blastOutMemOffset, &out);
	if(r && (r != 1 || (size == file.size() && offset == 0))) {
		LogError << "PakReader::fRead: blast error " << r << " outSize=" << file.size();
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
		case SeekEnd: base = file.size();
		case SeekCur: base = offset;
		default: return -1;
	}
	
	if((int)base + _offset < 0) {
		return -1;
	}
	
	offset = (int)base + _offset;
	
	return offset;
}

size_t CompressedFileHandle::tell() {
	return offset;
}

/*! Plain file not in a .pak file archive. */
class PlainFile : public PakFile {
	
	fs::path path;
	
public:
	
	PlainFile(const fs::path & _path, size_t size) : PakFile(size), path(_path) { };
	
	void read(void * buf) const;
	
	PakFileHandle * open() const;
	
};

class PlainFileHandle : public PakFileHandle {
	
	fs::ifstream ifs;
	
public:
	
	PlainFileHandle(const fs::path & path) : ifs(path, fs::ifstream::in | fs::ifstream::binary) {
		arx_assert(ifs.is_open());
	};
	
	size_t read(void * buf, size_t size);
	
	int seek(Whence whence, int offset);
	
	size_t tell();
	
	~PlainFileHandle() { };
	
};

void PlainFile::read(void * buf) const {
	
	fs::ifstream ifs(path, fs::ifstream::in | fs::ifstream::binary);
	arx_assert(ifs.is_open());
	
	fread(ifs, buf, size());
	
	arx_assert(!ifs.fail());
}

PakFileHandle * PlainFile::open() const {
	return new PlainFileHandle(path);
}

size_t PlainFileHandle::read(void * buf, size_t size) {
	return fread(ifs, buf, size).gcount();
}

std::ios_base::seekdir arxToStlSeekOrigin[] = {
	std::ios_base::beg,
	std::ios_base::cur,
	std::ios_base::end
};

int PlainFileHandle::seek(Whence whence, int offset) {
	return ifs.seekg(offset, arxToStlSeekOrigin[whence]).tellg();
}

size_t PlainFileHandle::tell() {
	return ifs.tellg();
}

static void pakDecrypt(char * fat, size_t fat_size, const char * key) {
	
	size_t keysize = strlen((const char *)key);
	
	for(size_t i = 0, ki = 0; i < fat_size; i++, ki = (ki + 1) % keysize) {
		fat[i] ^= key[ki];
	}
	
}

} // anonymous namespace

PakReader::~PakReader() {
	clear();
}

bool PakReader::addArchive(const fs::path & pakfile) {
	
	fs::ifstream * ifs = new fs::ifstream(pakfile, fs::ifstream::in | fs::ifstream::binary);
	
	if(!ifs->is_open()) {
		delete ifs;
		return false;
	}
	
	// Read fat location and size.
	u32 fat_offset;
	u32 fat_size;
	
	if(fread(*ifs, fat_offset).fail()) {
		LogError << "error reading FAT offset";
		delete ifs;
		return false;
	}
	if(ifs->seekg(fat_offset).fail()) {
		LogError << "error seeking to FAT offset " << fat_offset;
		delete ifs;
		return false;
	}
	if(fread(*ifs, fat_size).fail()) {
		LogError << "error reading FAT size at offset " << fat_offset;
		delete ifs;
		return false;
	}
	
	// Read the whole FAT.
	char * fat = new char[fat_size];
	if(ifs->read(fat, fat_size).fail()) {
		LogError << "error reading FAT";
		delete[] fat;
		delete ifs;
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
	
	paks.push_back(ifs);
	
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
				file = new CompressedFile(ifs, offset, uncompressedSize, size);
			} else {
				file = new UncompressedFile(ifs, offset, size);
			}
			
			dir->addFile(string(filename, len), file);
		}
		
	}
	
	delete[] fat;
	
	LogInfo << "Loaded PAK " << pakfile;
	return true;
	
	
error:
	
	delete[] fat;
	
	return false;
}

void PakReader::clear() {
	
	files.clear();
	dirs.clear();
	
	for(vector<std::istream *>::const_iterator i = paks.begin(); i != paks.end(); i++) {
		delete *i;
	}
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

bool PakReader::addFiles(const fs::path & path, const string & mount) {
	
	boost::system::error_code ec;
	
	if(!fs::exists(path, ec) || ec) {
		return false;
	}
		
	if(fs::is_directory(path, ec)) {
			
		return addFiles(addDirectory(mount), path);
			
	} else {
			
		size_t pos = mount.find_last_of(DIR_SEP);
			
		PakDirectory * dir = (pos == string::npos) ? this : addDirectory(strref(mount, 0, pos));
			
		return addFile(dir, path, (pos == string::npos) ? mount : mount.substr(pos + 1));
	}
}

void PakReader::removeFile(const string & file) {
	
	size_t pos = file.find_last_of(DIR_SEP);
	
	PakDirectory * dir = (pos == string::npos) ? this : getDirectory(strref(file, 0, pos));
	if(dir) {
		dir->removeFile((pos == string::npos) ? file : file.substr(pos + 1));
	}
}

bool PakReader::addFile(PakDirectory * dir, const fs::path & path, const std::string & name) {
	
	if(name.empty()) {
		return false;
	}
	
	boost::system::error_code ec;
	size_t size = fs::file_size(path, ec);
	if(ec) {
		return false;
	}
	
	dir->addFile(name, new PlainFile(path, size));
	return true;
}

bool PakReader::addFiles(PakDirectory * dir, const fs::path & path) {
	
	bool ret = true;
	
	boost::system::error_code ec;
	fs::directory_iterator end;
	for(fs::directory_iterator it(path, ec); it != end; it.increment(ec)) {
			
		const fs::path & entry = it->path();
			
		if(fs::is_directory(entry, ec)) {
			ret &= addFiles(dir->addDirectory(as_string(entry.filename())), entry);
		} else {
			ret &= addFile(dir, entry, as_string(entry.filename()));
		}
			
	}
	
	return ret;
}

PakReader * resources;
