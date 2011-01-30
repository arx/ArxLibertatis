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

#define PAK_READ_BUF_SIZE 1024

#include <stdint.h>
#include <cstring>
#include <algorithm>
#include <cassert>
using std::min;
using std::max;
using std::size_t;
using std::strlen;

#include "io/Blast.h"
#include "io/PakReader.h"
#include "io/PakEntry.h"
#include "io/HashMap.h"
#include "io/Logger.h"

const char PAK_KEY_DEMO[] = "NSIARKPRQPHBTE50GRIH3AYXJP2AMF3FCEYAVQO5QGA0JGIIH2AYXKVOA1VOGGU5GSQKKYEOIAQG1XRX0J4F5OEAEFI4DD3LL45VJTVOA1VOGGUKE50GRI";
const char PAK_KEY_FULL[] = "AVQF3FCKE50GRIAYXJP2AMEYO5QGA0JGIIH2NHBTVOA1VOGGU5H3GSSIARKPRQPQKKYEOIAQG1XRX0J4F5OEAEFI4DD3LL45VJTVOA1VOGGUKE50GRIAYX";

static const char * selectKey(uint32_t first_bytes) {
	switch(first_bytes) {
		case 0x46515641:
			return PAK_KEY_FULL;
		case 0x4149534E:
			return PAK_KEY_DEMO;
		default:
			return NULL;
	}
}

PakReader::PakReader() {
	
	pakname = NULL;
	file = 0;
	root = NULL;
	fat = NULL;
	
	int iI = PACK_MAX_FREAD;
	while(--iI) {
		tPackFile[iI].reader = this;
		tPackFile[iI].active = false;
		tPackFile[iI].iID = 0;
		tPackFile[iI].offset = 0;
	}
}

PakReader::~PakReader() {
	Close();
}

static void pakDecrypt(char * fat, size_t fat_size, const char * key) {
	
	size_t keysize = strlen((const char *)key);
	
	for(size_t i = 0, ki = 0; i < fat_size; i++, ki = (ki + 1) % keysize) {
		fat[i] ^= key[ki];
	}
	
}

static const char * safeGetString(const char * & pos, uint32_t & fat_size) {
	
	const char * begin = pos;
	
	for(size_t i = 0; i < fat_size; i++) {
		if(pos[i] == 0) {
			fat_size -= i + 1;
			pos += i + 1;
			return begin;
		}
	}
	
	return NULL;
}

template <class T>
inline bool safeGet(T & data, const char * & pos, uint32_t & fat_size) {
	if(fat_size < sizeof(T)) {
		return false;
	}
	data = *reinterpret_cast<const T *>(pos);
	pos += sizeof(T);
	fat_size -= sizeof(T);
	return true;
}

bool PakReader::Open(const char * name) {
	
	FILE * newfile = fopen(name, "rb");
	
	if(!newfile) {
		LogError <<"Cannot find PAK "<< name;
		return false;
	}
	
	// Read fat location and size.
	uint32_t fat_offset;
	uint32_t fat_size;
	if(fread(&fat_offset, sizeof(fat_offset), 1, newfile) != 1) {
		printf("error reading FAT offset\n");
		fclose(newfile);
		return false;
	}
	if(fseek(newfile, fat_offset, SEEK_SET)) {
		printf("error seeking to FAT offset\n");
		fclose(newfile);
		return false;
	}
	if(fread(&fat_size, sizeof(fat_size), 1, newfile) != 1) {
		printf("error reading FAT size\n");
		fclose(newfile);
		return false;
	}
	
	// Read the whole FAT.
	char * newfat = new char[fat_size];
	if(fread(newfat, fat_size, 1, newfile) != 1) {
		printf("error reading FAT\n");
		fclose(newfile);
		return false;
	}
	
	// Decrypt the FAT.
	const char * key = selectKey(*(uint32_t*)newfat);
	if(key) {
		pakDecrypt(newfat, fat_size, key);
	} else {
		printf("WARNING: unknown PAK key ID 0x%08x, assuming no key\n", *(uint32_t*)newfat);
	}
	
	PakDirectory * newroot = new PakDirectory(NULL, NULL);
	
	const char * pos = newfat;
	
	while(fat_size) {
		
		const char * dirname = safeGetString(pos, fat_size);
		if(!dirname) {
			printf("error reading directory name from FAT, wrong key?\n");
			goto error;
		}
		
		PakDirectory * dir;
		if(*dirname != '\0') {
			dir = newroot->addDirectory(dirname);
		} else {
			dir = newroot;
		}
		
		uint32_t nfiles;
		if(!safeGet(nfiles, pos, fat_size)) {
			printf("error reading file count from FAT, wrong key?\n");
			goto error;
		}
		
		if(nfiles && !dir->filesMap) {
			size_t hashsize = 1;
			while(hashsize < nfiles) {
				hashsize <<= 1;
			}
			size_t n = (hashsize * 3) / 4;
			if(nfiles > n) {
				hashsize <<= 1;
			}
			dir->filesMap = new HashMap(hashsize);
		}
		
		while(nfiles--) {
			
			const char * filename =  safeGetString(pos, fat_size);
			if(!filename) {
				printf("error reading file name from FAT, wrong key?\n");
				goto error;
			}
			
			PakFile * file = dir->addFile(filename);
			if(!file) {
				printf("could not add file while loading PAK\n");
				goto error;
			}
			
			uint32_t offset;
			uint32_t flags;
			uint32_t uncompressedSize;
			uint32_t size;
			
			if(!safeGet(offset, pos, fat_size) || !safeGet(flags, pos, fat_size)
			   || !safeGet(uncompressedSize, pos, fat_size) || !safeGet(size, pos, fat_size)) {
				printf("error reading file attributes from FAT, wrong key?\n");
				goto error;
			}
			
			file->offset = offset;
			file->flags = flags;
			file->uncompressedSize = uncompressedSize;
			file->size = size;
		}
		
	}
	
	Close();
	
	root = newroot;
	pakname = strdup((const char *)name);
	fat = newfat;
	file = newfile;
	
	LogInfo <<"Loaded PAK " << name;
	return true;
	
	
error:
	
	delete newroot;
	delete[] newfat;
	fclose(newfile);
	
	return false;
	
}

void PakReader::Close() {
	
	if(pakname) {
		free((void *)pakname);
		pakname = NULL;
	}
	
	if(file) {
		fclose(file);
		file = 0;
	}
	
	if(root) {
		delete root;
		root = NULL;
	}
	
	if(fat) {
		delete[] fat;
		fat = NULL;
	}
	
}

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

PakFile * PakReader::getFile(const char * name) {
	
	if(!name || !root) {
		// Not loaded.
		return NULL;
	}
	
	return root->getFile(name);
}


// TODO unchecked buffer size
bool PakReader::Read(const char * name, void * buf) {
	
	PakFile * f = getFile(name);
	if(!f) {
		return false;
	}
	
	fseek(file, f->offset, SEEK_SET);
	
	if(f->flags & PAK_FILE_COMPRESSED) {
		int r = blast(file, (char *)buf, f->uncompressedSize);
		if(r) {
			LogError << "PakReader::Read: blast error " << r << " outSize=" << f->uncompressedSize;
			return false;
		}
	} else {
		if(fread(buf, f->size, 1, file) != 1) {
			return false;
		}
	}
	
	return true;
}

void * PakReader::ReadAlloc(const char * name, size_t * sizeRead) {
	
	PakFile * f = getFile(name);
	if(!f) {
		return NULL;
	}
	
	fseek(file, f->offset, SEEK_SET);
	
	void * mem;
	if(f->flags & PAK_FILE_COMPRESSED) {
		mem = malloc(f->uncompressedSize);
		if(!mem) {
			if(sizeRead) {
				*sizeRead = 0;
			}
			return NULL;
		}
		int r = blast(file, (char *)mem, f->uncompressedSize);
		if(r) {
			LogError << "PakReader::ReadAlloc: blast error " << r << " outSize=" << f->uncompressedSize;
			free(mem);
			if(sizeRead) {
				*sizeRead = 0;
			}
			return NULL;
		}
		
		if(sizeRead) {
			*sizeRead = f->uncompressedSize;
		}
		
	} else {
		mem = malloc(f->size);
		if(!mem) {
			if(sizeRead) {
				*sizeRead = 0;
			}
			return NULL;
		}
		
		if(fread(mem, f->size, 1, file) != 1) {
			LogError << "PakReader::ReadAlloc: read error" << f->size;
			free(mem);
			if(sizeRead) {
				*sizeRead = 0;
			}
			return NULL;
		}
		
		if(sizeRead) {
			*sizeRead = f->size;
		}
	}
	
	return mem;
}

int PakReader::GetSize(const char * name) {
	
	PakFile * f = getFile(name);
	if(!f) {
		return -1;
	}
	
	if(f->flags & PAK_FILE_COMPRESSED) {
		return f->uncompressedSize;
	} else {
		return f->size;
	}
}

PakFileHandle * PakReader::fOpen(const char * name) {
	
	PakFile * f = getFile(name);
	if(!f) {
		return NULL;
	}
	
	int iNb = PACK_MAX_FREAD;
	
	while(--iNb) {
		if(!tPackFile[iNb].active) {
			assert(tPackFile[iNb].reader == this);
			tPackFile[iNb].iID = (void*)fat; // TODO why use the FAT address here?
			tPackFile[iNb].active = true;
			tPackFile[iNb].offset = 0;
			tPackFile[iNb].file = f;
			return &tPackFile[iNb];
		}
	}
	
	LogError << "cannot fopen file in PAK, ran out of handles";
	return NULL;
}

int PakReader::fClose(PakFileHandle * fh) {
	if((!fh) || (!fh->active) || (fh->iID != ((void*)fat))) {
		return EOF;
	}
	fh->active = false;
	return 0;
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

size_t PakReader::fRead(void * buf, size_t isize, size_t count, PakFileHandle * fh) {
	
	if((!fh) || (!fh->file) || (fh->iID != ((void*)fat))) {
		return 0;
	}
	
	size_t size = isize * count;
	
	if(!size) {
		return 0;
	}
	
	PakFile * f = fh->file;
	
	assert(fh->offset >= 0);
	
	if(f->flags & PAK_FILE_COMPRESSED) {
		
		if((size_t)fh->offset >= fh->file->uncompressedSize) {
			return 0;
		}
		
		if(size < fh->file->uncompressedSize) {
			LogWarning << "partially reading a compressed file - inefficent: " << f->name
			           << " size=" << size << " offset=" << fh->offset << " total=" << f->uncompressedSize;
		}
		
		fseek(file, f->offset, SEEK_SET);
		
		BlastFileInBuffer in(file);
		BlastMemOutBufferOffset out;
		
		out.buf = (char *)buf;
		out.currentOffset = 0;
		out.startOffset = fh->offset;
		out.endOffset = min(fh->offset + size, f->uncompressedSize);
		
		if(out.endOffset <= out.startOffset) {
			return 0;
		}
		
		// TODO this is really inefficient
		int r = blast(blastInFile, &in, blastOutMemOffset, &out);
		if(r && (r != 1 || (size == f->uncompressedSize && fh->offset == 0))) {
			LogError << "PakReader::fRead: blast error " << r << " outSize=" << f->uncompressedSize;
			return 0;
		}
		
		size = out.currentOffset - out.startOffset;
		
	} else {
		
		if((size_t)fh->offset >= fh->file->size) {
			return 0;
		}
		
		fseek(file, f->offset + fh->offset, SEEK_SET);
		
		if(f->size < (unsigned int)fh->offset + size) {
			size -= fh->offset + size - f->size;
		}
		
		size = fread(buf, 1, size, file);
	}
	
	fh->offset += size;
	
	return size; // TODO should this be count?
}

// TODO different return values from fseek in <cstdio>
int PakReader::fSeek(PakFileHandle * fh, int offset, long whence) {
	
	if((!fh) || (!fh->file) || (fh->iID != ((void*)fat))) {
		return 1;
	}
	
	size_t size;
	if(fh->file->flags & PAK_FILE_COMPRESSED) {
		size = fh->file->uncompressedSize;
	} else {
		size = fh->file->size;
	}
	
	switch(whence) {
		
		case SEEK_SET:
			
			if(offset < 0) {
				return 1;
			}
			
			if((size_t)offset > size) {
				return 1;
			}
			fh->offset = offset;
			break;
			
		case SEEK_END:
			
			if(offset < 0) {
				return 1;
			}
			
			if((size_t)offset > size) {
				return 1;
			}
			fh->offset = size - offset;
			break;
			
		case SEEK_CUR:
			
			int iOffset = fh->offset + offset;
			if((iOffset < 0) || ((unsigned int)iOffset > size)) {
				return 1;
			}
			fh->offset = iOffset;
		
	}
	
	return 0;
}

long PakReader::fTell(PakFileHandle * fh) {
	
	if((!fh) || (!fh->file) || (fh->iID != ((void*)fat))) {
		return -1;
	}
	
	return fh->offset;
}
