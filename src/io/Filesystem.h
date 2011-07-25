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
#include <string>

#include <iostream>

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>

template <class T>
inline std::istream & fread(std::istream & ifs, T & data) {
	return ifs.read(reinterpret_cast<char *>(&data), sizeof(T));
}

inline std::istream & fread(std::istream & ifs, void * buf, size_t n) {
	return ifs.read(reinterpret_cast<char *>(buf), n);
}

template <class T>
inline std::ostream & fwrite(std::ostream & ifs, const T & data) {
	return ifs.write(reinterpret_cast<const char *>(&data), sizeof(T));
}

inline std::ostream & fwrite(std::ostream & ifs, const void * buf, size_t n) {
	return ifs.write(reinterpret_cast<const char *>(buf), n);
}

std::istream & fread(std::istream & ifs, std::string & buf);

namespace fs = boost::filesystem;

namespace fs_tmp
{
	bool remove_all(const fs::path& p)
	{
		boost::system::error_code ec;
		fs::remove_all(p, ec);
		return !ec;
	}

	bool create_directory(const fs::path& p)
	{
		boost::system::error_code ec;
		fs::create_directory(p, ec);
		return !ec;
	}

	bool create_directories(const fs::path& p)
	{
		boost::system::error_code ec;
		fs::create_directories(p, ec);
		return !ec;
	}

	bool copy_file(const fs::path& from_p, const fs::path& to_p)
	{
		boost::system::error_code ec;
		fs::copy_file(from_p, to_p, ec);
		return !ec;
	}

	bool rename(const fs::path& old_p, const fs::path& new_p)
	{
		boost::system::error_code ec;
		fs::rename(old_p, new_p, ec);
		return !ec;
	}
}

char * read_file(const fs::path & path, size_t & size);

#endif // ARX_IO_FILESYSTEM_H
