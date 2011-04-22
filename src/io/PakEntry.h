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

#ifndef ARX_IO_PAKENTRY_H
#define ARX_IO_PAKENTRY_H

class HashMap;

#include <stddef.h>
#include <string>

#define PAK_FILE_COMPRESSED 1

class PakFile {
	
public:
	
	std::string name;
	size_t size;
	PakFile * prev;
	PakFile * next;
	size_t offset;
	unsigned int flags;
	size_t uncompressedSize;
	
public:
	
	PakFile( const std::string& n );
	~PakFile();
	
};

class PakDirectory {
	
public:
	
	const char* name;
	unsigned int nbsousreps;
	unsigned int nbfiles;
	PakDirectory * parent;
	PakDirectory * children;
	PakDirectory * prev;
	PakDirectory * next;
	
	PakFile * files;
	
	HashMap * filesMap;
	
public:
	
	PakDirectory( PakDirectory * p = 0, const std::string& n = "" );
	~PakDirectory();
	
	PakDirectory * addDirectory(const std::string& sname);
	
	bool removeDirectory(const std::string& dirname);
	
	PakFile * addFile(const std::string& filename);
	
	PakDirectory * getDirectory(const std::string& dirname);
	
	PakFile * getFile(const std::string& dirplusfilename);
	
	friend void kill(PakDirectory * r);
	
};

char * EVEF_GetDirName(const char * dirplusname);

#endif // ARX_IO_PAKENTRY_H
