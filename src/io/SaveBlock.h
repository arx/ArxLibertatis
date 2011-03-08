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
#ifndef _CLUSTER_SAVE_H_
#define _CLUSTER_SAVE_H_

#include <cstddef>
#include <string>
#include <vector>

#include "core/Common.h"


class HashMap;
typedef void * FileHandle;


class SaveBlock {
	
private:
	
	struct File;
	typedef std::vector<File> FileList;
	
	FileHandle handle;
	size_t totalSize;
	FileList files;
	bool firstSave;
	HashMap * hashMap;
	std::string savefile;
	
	File * getFile(const std::string & name);
	bool defragment();
	bool loadFileTable();
	void writeFileTable();
	
public:
	
	SaveBlock(const std::string & savefile);
	~SaveBlock();
	
	bool BeginSave();
	bool flush();
	bool save(const std::string & name, const char * data, size_t size);
	
	bool BeginRead();
	char * load(const std::string & name, size_t & size) const;
	bool hasFile(const std::string & name) const;
	
};

#endif // _CLUSTER_SAVE_H_
