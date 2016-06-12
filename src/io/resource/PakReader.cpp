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

#include "io/resource/PakReader.h"

#include <cstring>
#include <algorithm>
#include <iomanip>
#include <ios>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/foreach.hpp>

#include "io/log/Logger.h"
#include "io/Blast.h"
#include "io/resource/PakEntry.h"
#include "io/fs/FilePath.h"
#include "io/fs/Filesystem.h"
#include "io/fs/FileStream.h"

#include "util/String.h"

namespace {

const size_t PAK_READ_BUF_SIZE = 1024;

static PakReader::ReleaseType guessReleaseType(u32 first_bytes) {
	switch(first_bytes) {
		case 0x46515641:
			return PakReader::FullGame;
		case 0x4149534E:
			return PakReader::Demo;
		default:
			return PakReader::Unknown;
	}
}

static void pakDecrypt(char * fat, size_t fat_size, PakReader::ReleaseType keyId) {
	
	static const char PAK_KEY_DEMO[] = "NSIARKPRQPHBTE50GRIH3AYXJP2AMF3FCEYAVQO5Q"
		"GA0JGIIH2AYXKVOA1VOGGU5GSQKKYEOIAQG1XRX0J4F5OEAEFI4DD3LL45VJTVOA1VOGGUKE50GRI";
	static const char PAK_KEY_FULL[] = "AVQF3FCKE50GRIAYXJP2AMEYO5QGA0JGIIH2NHBTV"
		"OA1VOGGU5H3GSSIARKPRQPQKKYEOIAQG1XRX0J4F5OEAEFI4DD3LL45VJTVOA1VOGGUKE50GRIAYX";
	
	const char * key;
	size_t keysize;
	if(keyId == PakReader::FullGame) {
		key = PAK_KEY_FULL, keysize = ARRAY_SIZE(PAK_KEY_FULL) - 1;
	} else {
		key = PAK_KEY_DEMO, keysize = ARRAY_SIZE(PAK_KEY_DEMO) - 1;
	}
	
	for(size_t i = 0, ki = 0; i < fat_size; i++, ki = (ki + 1) % keysize) {
		fat[i] ^= key[ki];
	}
	
}

/*! Uncompressed file in a .pak file archive. */
class UncompressedFile : public PakFile {
	
	std::istream & archive;
	size_t offset;
	
public:
	
	explicit UncompressedFile(std::istream * _archive, size_t _offset, size_t size)
		: PakFile(size), archive(*_archive), offset(_offset) { }
	
	void read(void * buf) const;
	
	PakFileHandle * open() const;
	
	friend class UncompressedFileHandle;
	
};

class UncompressedFileHandle : public PakFileHandle {
	
	const UncompressedFile & file;
	size_t offset;
	
public:
	
	explicit UncompressedFileHandle(const UncompressedFile * _file)
		: file(*_file), offset(0) { }
	
	size_t read(void * buf, size_t size);
	
	int seek(Whence whence, int offset);
	
	size_t tell();
	
	~UncompressedFileHandle() { }
	
};

void UncompressedFile::read(void * buf) const {
	
	archive.seekg(offset);
	
	fs::read(archive, buf, size());
	
	arx_assert(!archive.fail());
	arx_assert(size_t(archive.gcount()) == size());
	
	archive.clear();
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
	
	fs::read(file.archive, buf, size);
	
	size_t nread = file.archive.gcount();
	offset += nread;
	
	file.archive.clear();
	
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
	
	fs::ifstream & archive;
	size_t offset;
	size_t storedSize;
	
public:
	
	explicit CompressedFile(fs::ifstream * _archive, size_t _offset, size_t size,
	                        size_t _storedSize)
		: PakFile(size), archive(*_archive), offset(_offset), storedSize(_storedSize) { }
	
	void read(void * buf) const;
	
	PakFileHandle * open() const;
	
	friend class CompressedFileHandle;
	
};

class CompressedFileHandle : public PakFileHandle {
	
	const CompressedFile & file;
	size_t offset;
	
public:
	
	explicit CompressedFileHandle(const CompressedFile * _file)
		: file(*_file), offset(0) { }
	
	size_t read(void * buf, size_t size);
	
	int seek(Whence whence, int offset);
	
	size_t tell();
	
	~CompressedFileHandle() { }
	
};

struct BlastFileInBuffer : private boost::noncopyable {
	
	fs::ifstream & file;
	size_t remaining;
	
	unsigned char readbuf[PAK_READ_BUF_SIZE];
	
	explicit BlastFileInBuffer(fs::ifstream * f, size_t count)
		: file(*f), remaining(count) { }
	
};

size_t blastInFile(void * Param, const unsigned char ** buf) {
	
	BlastFileInBuffer * p = static_cast<BlastFileInBuffer *>(Param);
	
	*buf = p->readbuf;
	
	size_t count = std::min(p->remaining, ARRAY_SIZE(p->readbuf));
	p->remaining -= count;
	
	return fs::read(p->file, p->readbuf, count).gcount();
}

void CompressedFile::read(void * buf) const {
	
	archive.seekg(offset);
	
	BlastFileInBuffer in(&archive, storedSize);
	BlastMemOutBuffer out(reinterpret_cast<char *>(buf), size());
	
	int r = blast(blastInFile, &in, blastOutMem, &out);
	if(r) {
		LogError << "Blast error " << r << " outSize=" << size();
	}
	
	arx_assert(!archive.fail());
	arx_assert(in.remaining == 0);
	arx_assert(out.size == 0);
	
	archive.clear();
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
	
	BlastMemOutBufferOffset * p = static_cast<BlastMemOutBufferOffset *>(Param);
	
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
	
	size_t toCopy = std::min(len, p->endOffset - p->currentOffset);
	
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
		LogWarning << "Partially reading a compressed file - inefficent: size=" << size
		           << " offset=" << offset << " total=" << file.size();
	}
	
	file.archive.seekg(file.offset);
	
	BlastFileInBuffer in(&file.archive, file.storedSize);
	BlastMemOutBufferOffset out;
	
	out.buf = reinterpret_cast<char *>(buf);
	out.currentOffset = 0;
	out.startOffset = offset;
	out.endOffset = std::min(offset + size, file.size());
	
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
	
	file.archive.clear();
	
	return size;
}

int CompressedFileHandle::seek(Whence whence, int _offset) {
	
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

size_t CompressedFileHandle::tell() {
	return offset;
}

/*! Plain file not in a .pak file archive. */
class PlainFile : public PakFile {
	
	fs::path path;
	
public:
	
	PlainFile(const fs::path & _path, size_t size) : PakFile(size), path(_path) { }
	
	void read(void * buf) const;
	
	PakFileHandle * open() const;
	
};

class PlainFileHandle : public PakFileHandle {
	
	fs::ifstream ifs;
	
public:
	
	explicit  PlainFileHandle(const fs::path & path)
		: ifs(path, fs::fstream::in | fs::fstream::binary) {
		arx_assert(ifs.is_open());
	};
	
	size_t read(void * buf, size_t size);
	
	int seek(Whence whence, int offset);
	
	size_t tell();
	
	~PlainFileHandle() { }
	
};

void PlainFile::read(void * buf) const {
	
	fs::ifstream ifs(path, fs::fstream::in | fs::fstream::binary);
	arx_assert(ifs.is_open());
	
	fs::read(ifs, buf, size());
	
	arx_assert(!ifs.fail());
	arx_assert(size_t(ifs.gcount()) == size());
}

PakFileHandle * PlainFile::open() const {
	return new PlainFileHandle(path);
}

size_t PlainFileHandle::read(void * buf, size_t size) {
	return fs::read(ifs, buf, size).gcount();
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

} // anonymous namespace

PakReader::~PakReader() {
	clear();
}

bool PakReader::addArchive(const fs::path & pakfile) {
	
	fs::ifstream * ifs = new fs::ifstream(pakfile, fs::fstream::in | fs::fstream::binary);
	
	if(!ifs->is_open()) {
		delete ifs;
		return false;
	}
	
	// Read fat location and size.
	u32 fat_offset;
	u32 fat_size;
	
	if(fs::read(*ifs, fat_offset).fail()) {
		LogError << pakfile << ": error reading FAT offset";
		delete ifs;
		return false;
	}
	if(ifs->seekg(fat_offset).fail()) {
		LogError << pakfile << ": error seeking to FAT offset " << fat_offset;
		delete ifs;
		return false;
	}
	if(fs::read(*ifs, fat_size).fail()) {
		LogError << pakfile << ": error reading FAT size at offset " << fat_offset;
		delete ifs;
		return false;
	}
	
	// Read the whole FAT.
	char * fat = new char[fat_size];
	if(ifs->read(fat, fat_size).fail()) {
		LogError << pakfile << ": error reading FAT at " << fat_offset
		         << " with size " << fat_size;
		delete[] fat;
		delete ifs;
		return false;
	}
	
	// Decrypt the FAT.
	ReleaseType key = guessReleaseType(*reinterpret_cast<const u32 *>(fat));
	if(key != Unknown) {
		pakDecrypt(fat, fat_size, key);
	} else {
		LogWarning << pakfile << ": unknown PAK key ID 0x" << std::hex << std::setfill('0')
		           << std::setw(8) << *(u32*)fat << ", assuming no key";
	}
	release |= key;
	
	char * pos = fat;
	
	paks.push_back(ifs);
	
	while(fat_size) {
		
		char * dirname = util::safeGetString(pos, fat_size);
		if(!dirname) {
			LogError << pakfile << ": error reading directory name from FAT, wrong key?";
			goto error;
		}
		
		PakDirectory * dir = addDirectory(res::path::load(dirname));
		
		u32 nfiles;
		if(!util::safeGet(nfiles, pos, fat_size)) {
			LogError << pakfile << ": error reading file count from FAT, wrong key?";
			goto error;
		}
		
		while(nfiles--) {
			
			char * filename =  util::safeGetString(pos, fat_size);
			if(!filename) {
				LogError << pakfile << ": error reading file name from FAT, wrong key?";
				goto error;
			}
			
			size_t len = std::strlen(filename);
			std::transform(filename, filename + len, filename, ::tolower);
			
			u32 offset;
			u32 flags;
			u32 uncompressedSize;
			u32 size;
			if(!util::safeGet(offset, pos, fat_size) || !util::safeGet(flags, pos, fat_size)
			   || !util::safeGet(uncompressedSize, pos, fat_size)
				 || !util::safeGet(size, pos, fat_size)) {
				LogError << pakfile << ": error reading file attributes from FAT, wrong key?";
				goto error;
			}
			
			const u32 PAK_FILE_COMPRESSED = 1;
			PakFile * file;
			if((flags & PAK_FILE_COMPRESSED) && size != 0) {
				file = new CompressedFile(ifs, offset, uncompressedSize, size);
			} else {
				file = new UncompressedFile(ifs, offset, size);
			}
			
			dir->addFile(std::string(filename, len), file);
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
	
	release = 0;
	
	files.clear();
	dirs.clear();
	
	BOOST_FOREACH(std::istream * is, paks) {
		delete is;
	}
}

bool PakReader::read(const res::path & name, void * buf) {
	
	PakFile * f = getFile(name);
	if(!f) {
		return false;
	}
	
	f->read(buf);
	
	return true;
}

char * PakReader::readAlloc(const res::path & name, size_t & sizeRead) {
	
	PakFile * f = getFile(name);
	if(!f) {
		return NULL;
	}
	
	sizeRead = f->size();
	
	return f->readAlloc();
}

PakFileHandle * PakReader::open(const res::path & name) {
	
	PakFile * f = getFile(name);
	if(!f) {
		return NULL;
	}
	
	return f->open();
}

bool PakReader::addFiles(const fs::path & path, const res::path & mount) {
	
	if(fs::is_directory(path)) {
			
		bool ret = addFiles(addDirectory(mount), path);
	
		if(ret) {
			LogInfo << "Added dir " << path;
		}
		
		return ret;
		
	} else if(fs::is_regular_file(path) && !mount.empty()) {
		
		PakDirectory * dir = addDirectory(mount.parent());
		
		return addFile(dir, path, mount.filename());
		
	}
	
	return false;
}

void PakReader::removeFile(const res::path & file) {
	
	PakDirectory * dir = getDirectory(file.parent());
	if(dir) {
		dir->removeFile(file.filename());
	}
}

bool PakReader::removeDirectory(const res::path & name) {
	
	PakDirectory * pdir = getDirectory(name.parent());
	if(pdir) {
		return pdir->removeDirectory(name.filename());
	} else {
		return true;
	}
}

bool PakReader::addFile(PakDirectory * dir, const fs::path & path,
                        const std::string & name) {
	
	if(name.empty()) {
		return false;
	}
	
	u64 size = fs::file_size(path);
	if(size == (u64)-1) {
		return false;
	}
	
	dir->addFile(name, new PlainFile(path, size));
	return true;
}

bool PakReader::addFiles(PakDirectory * dir, const fs::path & path) {
	
	bool ret = true;
	
	for(fs::directory_iterator it(path); !it.end(); ++it) {
		
		std::string name = it.name();
		
		if(name.empty() || name[0] == '.') {
			// Ignore
			continue;
		}
		
		fs::path entry = path / name;
		
		boost::to_lower(name);
		
		if(it.is_directory()) {
			ret &= addFiles(dir->addDirectory(name), entry);
		} else if(it.is_regular_file()) {
			ret &= addFile(dir, entry, name);
		}
		
	}
	
	return ret;
}

PakReader * resources;
