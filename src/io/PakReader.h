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

#ifndef ARX_IO_PAKREADER_H
#define ARX_IO_PAKREADER_H

#include <vector>
#include <iostream>

#include "io/PakEntry.h"
#include "io/FilePath.h"
#include "platform/Flags.h"

enum Whence {
	SeekSet,
	SeekCur,
	SeekEnd
};

class PakFileHandle {
	
public:
	
	virtual size_t read(void * buf, size_t size) = 0;
	
	virtual int seek(Whence whence, int offset) = 0;
	
	virtual size_t tell() = 0;
	
	virtual ~PakFileHandle() { };
	
};

class PakReader : public PakDirectory {
	
public:
	
	enum ReleaseType {
		Demo     = (1<<0),
		FullGame = (1<<1),
		Unknown  = (1<<2)
	};
	DECLARE_FLAGS(ReleaseType, ReleaseFlags)
	
	inline PakReader() : release(0) { }
	~PakReader();
	
	void removeFile(const fs::path & name);
	
	bool addFiles(const fs::path & path, const fs::path & mount = fs::path());
	
	bool addArchive(const fs::path & pakfile);
	void clear();
	
	bool read(const fs::path & name, void * buf);
	char * readAlloc(const fs::path & name , size_t & size);
	
	PakFileHandle * open(const fs::path & name);
	
	inline ReleaseFlags getReleaseType() { return release; }
	
private:
	
	ReleaseFlags release;
	std::vector<std::istream *> paks;
	
	bool addFiles(PakDirectory * dir, const fs::path & path);
	bool addFile(PakDirectory * dir, const fs::path & path, const std::string & name);
	
};

DECLARE_FLAGS_OPERATORS(PakReader::ReleaseFlags)

extern PakReader * resources;

#endif // ARX_IO_PAKREADER_H
