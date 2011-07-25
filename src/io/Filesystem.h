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

#ifndef ARX_IO_FILESYSTEM_H
#define ARX_IO_FILESYSTEM_H

#include <cstddef>

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>

#include "platform/Platform.h"

namespace fs_boost = boost::filesystem;

namespace fs {

class path;

bool exists(const path & p);
bool is_directory(const path & p);
bool is_regular_file(const path & p);

std::time_t last_write_time(const path & p);
u64 file_size(const path & p);

bool remove(const path & p);

bool remove_all(const path & p);

bool create_directory(const path & p);

bool create_directories(const path & p);

bool copy_file(const path & from_p, const path & to_p);

bool rename(const path & old_p, const path & new_p);

char * read_file(const path & p, size_t & size);

}


#endif // ARX_IO_FILESYSTEM_H
