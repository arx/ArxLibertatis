/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
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

PakReader::ReleaseType guessReleaseType(u32 first_bytes) {
	switch(first_bytes) {
		case 0x46515641:
			return PakReader::FullGame;
		case 0x4149534E:
			return PakReader::Demo;
		default:
			return PakReader::Unknown;
	}
}

void pakDecrypt(char * fat, size_t fat_size, PakReader::ReleaseType keyId) {
	
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
	
	std::istream & m_archive;
	size_t m_offset;
	
public:
	
	explicit UncompressedFile(std::istream * archive, size_t offset, size_t size)
		: PakFile(size), m_archive(*archive), m_offset(offset) { }
	
	void read(void * buf) const;
	
	PakFileHandle * open() const;
	
	friend class UncompressedFileHandle;
	
};

class UncompressedFileHandle : public PakFileHandle {
	
	const UncompressedFile & m_file;
	size_t m_offset;
	
public:
	
	explicit UncompressedFileHandle(const UncompressedFile * file)
		: m_file(*file), m_offset(0) { }
	
	size_t read(void * buf, size_t size);
	
	int seek(Whence whence, int offset);
	
	size_t tell();
	
	~UncompressedFileHandle() { }
	
};

void UncompressedFile::read(void * buf) const {
	
	m_archive.seekg(m_offset);
	
	fs::read(m_archive, buf, size());
	
	arx_assert(!m_archive.fail());
	arx_assert(size_t(m_archive.gcount()) == size());
	
	m_archive.clear();
}

PakFileHandle * UncompressedFile::open() const {
	return new UncompressedFileHandle(this);
}

size_t UncompressedFileHandle::read(void * buf, size_t size) {
	
	if(m_offset >= m_file.size()) {
		return 0;
	}
	
	m_file.m_archive.seekg(m_file.m_offset + m_offset);
	
	if(m_file.size() < m_offset + size) {
		size = (m_offset > m_file.size()) ? 0 : (m_file.size() - m_offset);
	}
	
	fs::read(m_file.m_archive, buf, size);
	
	size_t nread = m_file.m_archive.gcount();
	m_offset += nread;
	
	m_file.m_archive.clear();
	
	return nread;
}

int UncompressedFileHandle::seek(Whence whence, int offset) {
	
	size_t base;
	switch(whence) {
		case SeekSet: base = 0; break;
		case SeekEnd: base = m_file.size(); break;
		case SeekCur: base = m_offset; break;
		default: return -1;
	}
	
	if(int(base) + offset < 0) {
		return -1;
	}
	
	m_offset = int(base) + offset;
	
	return m_offset;
}

size_t UncompressedFileHandle::tell() {
	return m_offset;
}

/*! Compressed file in a .pak file archive. */
class CompressedFile : public PakFile {
	
	fs::ifstream & m_archive;
	size_t m_offset;
	size_t m_storedSize;
	
public:
	
	explicit CompressedFile(fs::ifstream * archive, size_t offset, size_t size, size_t storedSize)
		: PakFile(size), m_archive(*archive), m_offset(offset), m_storedSize(storedSize) { }
	
	void read(void * buf) const;
	
	PakFileHandle * open() const;
	
	friend class CompressedFileHandle;
	
};

class CompressedFileHandle : public PakFileHandle {
	
	const CompressedFile & m_file;
	size_t m_offset;
	
public:
	
	explicit CompressedFileHandle(const CompressedFile * file)
		: m_file(*file), m_offset(0) { }
	
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
	
	m_archive.seekg(m_offset);
	
	std::string buffer;
	buffer.resize(m_storedSize);
	fs::read(m_archive, &buffer[0], m_storedSize);
	
	BlastMemInBuffer in(buffer.data(), buffer.size());
	BlastMemOutBuffer out(reinterpret_cast<char *>(buf), size());
	
	int r = blast(blastInMem, &in, blastOutMem, &out);
	if(r) {
		LogError << "Blast error " << r << " outSize=" << size();
	}
	
	arx_assert(!m_archive.fail());
	arx_assert(in.size == 0);
	arx_assert(out.size == 0);
	
	m_archive.clear();
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
	
	if(m_offset >= m_file.size()) {
		return 0;
	}
	
	if(size < m_file.size() || m_offset != 0) {
		LogWarning << "Partially reading a compressed file - inefficent: size=" << size
		           << " offset=" << m_offset << " total=" << m_file.size();
	}
	
	m_file.m_archive.seekg(m_file.m_offset);
	
	BlastFileInBuffer in(&m_file.m_archive, m_file.m_storedSize);
	BlastMemOutBufferOffset out;
	
	out.buf = reinterpret_cast<char *>(buf);
	out.currentOffset = 0;
	out.startOffset = m_offset;
	out.endOffset = std::min(m_offset + size, m_file.size());
	
	if(out.endOffset <= out.startOffset) {
		return 0;
	}
	
	// TODO this is really inefficient
	int r = ::blast(blastInFile, &in, blastOutMemOffset, &out);
	if(r && (r != 1 || (size == m_file.size() && m_offset == 0))) {
		LogError << "PakReader::fRead: blast error " << r << " outSize=" << m_file.size();
		return 0;
	}
	
	size = out.currentOffset - out.startOffset;
	
	m_offset += size;
	
	m_file.m_archive.clear();
	
	return size;
}

int CompressedFileHandle::seek(Whence whence, int offset) {
	
	size_t base;
	switch(whence) {
		case SeekSet: base = 0; break;
		case SeekEnd: base = m_file.size(); break;
		case SeekCur: base = m_offset; break;
		default: return -1;
	}
	
	if(int(base) + offset < 0) {
		return -1;
	}
	
	m_offset = int(base) + offset;
	
	return m_offset;
}

size_t CompressedFileHandle::tell() {
	return m_offset;
}

/*! Plain file not in a .pak file archive. */
class PlainFile : public PakFile {
	
	fs::path m_path;
	
public:
	
	PlainFile(const fs::path & path, size_t size) : PakFile(size), m_path(path) { }
	
	void read(void * buf) const;
	
	PakFileHandle * open() const;
	
};

class PlainFileHandle : public PakFileHandle {
	
	fs::ifstream ifs;
	
public:
	
	explicit  PlainFileHandle(const fs::path & path)
		: ifs(path, fs::fstream::in | fs::fstream::binary) {
		arx_assert(ifs.is_open());
	}
	
	size_t read(void * buf, size_t size);
	
	int seek(Whence whence, int offset);
	
	size_t tell();
	
	~PlainFileHandle() { }
	
};

void PlainFile::read(void * buf) const {
	
	fs::ifstream ifs(m_path, fs::fstream::in | fs::fstream::binary);
	arx_assert(ifs.is_open());
	
	fs::read(ifs, buf, size());
	
	arx_assert(!ifs.fail());
	arx_assert(size_t(ifs.gcount()) == size());
}

PakFileHandle * PlainFile::open() const {
	return new PlainFileHandle(m_path);
}

size_t PlainFileHandle::read(void * buf, size_t size) {
	return fs::read(ifs, buf, size).gcount();
}

std::ios_base::seekdir arxToStlSeekOrigin(Whence whence) {
	switch(whence) {
		case SeekSet: return std::ios_base::beg;
		case SeekCur: return std::ios_base::cur;
		case SeekEnd: return std::ios_base::end;
	}
	arx_unreachable();
}

int PlainFileHandle::seek(Whence whence, int offset) {
	return ifs.seekg(offset, arxToStlSeekOrigin(whence)).tellg();
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
	std::vector<char> fat(fat_size);
	if(ifs->read(&fat[0], fat_size).fail()) {
		LogError << pakfile << ": error reading FAT at " << fat_offset
		         << " with size " << fat_size;
		delete ifs;
		return false;
	}
	
	// Decrypt the FAT.
	ReleaseType key = guessReleaseType(*reinterpret_cast<const u32 *>(&fat[0]));
	if(key != Unknown) {
		pakDecrypt(&fat[0], fat_size, key);
	} else {
		LogWarning << pakfile << ": unknown PAK key ID 0x" << std::hex << std::setfill('0')
		           << std::setw(8) << *reinterpret_cast<u32 *>(&fat[0]) << ", assuming no key";
	}
	release |= key;
	
	char * pos = &fat[0];
	
	paks.push_back(ifs);
	
	while(fat_size) {
		
		char * dirname = util::safeGetString(pos, fat_size);
		if(!dirname) {
			LogError << pakfile << ": error reading directory name from FAT, wrong key?";
			return false;
		}
		
		PakDirectory * dir = addDirectory(res::path::load(dirname));
		
		u32 nfiles;
		if(!util::safeGet(nfiles, pos, fat_size)) {
			LogError << pakfile << ": error reading file count from FAT, wrong key?";
			return false;
		}
		
		while(nfiles--) {
			
			char * filename =  util::safeGetString(pos, fat_size);
			if(!filename) {
				LogError << pakfile << ": error reading file name from FAT, wrong key?";
				return false;
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
				return false;
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
	
	LogInfo << "Loaded PAK " << pakfile;
	return true;
	
}

void PakReader::clear() {
	
	release = 0;
	
	files.clear();
	dirs.clear();
	
	BOOST_FOREACH(std::istream * is, paks) {
		delete is;
	}
}

std::string PakReader::read(const res::path & name) {
	
	PakFile * f = getFile(name);
	if(!f) {
		return std::string();
	}
	
	return f->read();
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
			release |= External;
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
	if(size == u64(-1)) {
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

PakReader * g_resources;
