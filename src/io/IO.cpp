/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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
/* Based on:
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
// Code: Cyril Meynier
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved

#include "io/IO.h"

#include <cstring>
#include <algorithm>

#include "io/fs/FilePath.h"
#include "io/fs/Filesystem.h"
#include "io/fs/FileStream.h"

using std::string;

#if BUILD_EDIT_LOADSAVE

void HERMES_CreateFileCheck(const fs::path & name, char * scheck, size_t size, float id) {
	
	// TODO this will not produce the exact same hashes as the original, so me may as well uses a better hash function
	
	memset(scheck, 0, size);
	
	s32 * dst = reinterpret_cast<s32 *>(scheck);
	size_t length = size / 4;
	arx_assert(length > 6);
	
	fs::ifstream ifs(name, fs::fstream::ate | fs::fstream::in | fs::fstream::binary);
	if(!ifs.is_open()) {
		return;
	}
	
	std::time_t write_time = fs::last_write_time(name);
	if(write_time == 0) {
		return;
	}
	
	memcpy(&dst[0], &id, 4);
	dst[1] = ifs.tellg();

	dst[2] = dst[4] = u32(write_time);
	dst[3] = dst[5] = u32(u64(write_time) >> 32);
	
	size_t i = 6;
	string namestr = name.string();
	size_t l = std::min(size - i*4, namestr.length());
	memcpy(&dst[i], namestr.c_str(), l);
	i += (l + 3) / 4;
	
	ifs.seekg(0);
	
	while(i < length) {
		
		long crc = 0;
		
		for(long j = 0; j < 256 && ifs.good(); j++) {
			crc += ifs.get();
		}
		
		dst[i++] = crc;
	}
}

#endif // BUILD_EDIT_LOADSAVE
