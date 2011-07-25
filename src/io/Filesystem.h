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

#include "platform/Platform.h"

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

namespace fs_boost = boost::filesystem;

// WIP!!
namespace fs {

	class path {
	public:
		inline path(const fs_boost::path& p) : boost_path(p) {}
		inline operator fs_boost::path () const { return boost_path; }

	public:
	
		inline path() {}
		inline path(const path & name) : boost_path(name.filename()) {}
				
		// TODO allow trailing slashes? boost removes it!
		inline path(const std::string & name) : boost_path(name) {}
			
		// maybe? - to load from char constants without strlen?
		inline path(const char * name) : boost_path(name) {}
	
		// same for operator=
	
		// "foo" / "bar" -> "foo/bar"  |  "" / "foo" -> "foo"  |  "foo / "" -> "foo"
		// +overloads for std::string
		inline path operator/(const path & other) const { return boost_path / other.boost_path; }
			
		// *this = path(*this) / other;
		// returns *this
		// +overloads for std::string
		inline path & operator/=(const path & other) { boost_path /= other.boost_path; return *this; }
	
		// retrun pathname;
		inline const std::string & string() const { pathname = boost_path.string(); return pathname; }
	
		// "foo/bar" => "foo" | foo => ""
		inline const path parent() const { return boost_path.parent_path(); }
	
		// "foo/bar.ext" => "bar.ext"  |  "foo/bar" -> "bar"  |  "foo" -> "foo"  |  "foo.ext" -> "foo.ext"
		inline const std::string filename() const { return boost_path.filename().string(); }
	
		// "foo/bar.ext" => "bar"  |  "foo/bar" -> "bar"  |  "foo" -> "foo"  |  "foo.ext" -> "foo"
		const std::string basename() const;
	
		// "foo/bar.ext" => ".ext"  | "foo/bar" -> ""
		inline const std::string ext() const { return boost_path.extension().string(); }
	
		// "" -> true  |  "foo" -> false
		inline bool empty() const { return boost_path.empty(); }
	
		 // TODO +overlad for std::string?
		inline bool operator==(const path & other) const { return boost_path == other.boost_path; }
	
		// so it can be used in ordered maps, etc
		inline bool operator<(const path & other) const { return boost_path < other.boost_path; }
	
		/*
		 * "foo/bar".set_ext(".ext") -> "foo/bar.ext"
		 * "foo/bar".set_ext("ext") -> "foo/bar.ext"
		 * "foo/bar".set_ext("") -> "foo/bar"
		 * "foo/bar.abc".set_ext(".ext") -> "foo/bar.ext"
		 * "foo/bar.abc".set_ext("ext") -> "foo/bar.ext"
		 * "foo/bar.abc".set_ext("") -> "foo/bar"
		 * returns *this
		 */
		inline path & set_ext(const std::string & ext = std::string()) { boost_path.replace_extension(ext); return *this; }
	
		// *this = parent() / filename;
		inline path & set_filename(const std::string & filename) { boost_path = boost_path.parent_path() / filename; return *this; }
	
		// TODO some form of append?
	
	private:
	
		// TMP - mutable !
		mutable std::string pathname;

		// TMP
		fs_boost::path boost_path;	
	};

	inline path operator/(const std::string & base, const path & p) {
		return path(base) / p;
	}

	inline std::ostream & operator<<(std::ostream & strm, const path & p) {
		return strm << '"' << p.string() << '"';
	}
			
	inline bool exists(const path& p) { return fs_boost::exists(p); }
	inline bool is_directory(const path& p) { return fs_boost::is_directory(p); }
	inline bool is_regular_file(const path& p) { return fs_boost::is_regular_file(p); }

	inline std::time_t last_write_time(const path& p) { return fs_boost::last_write_time(p); }
	inline u64 file_size(const path& p) { return fs_boost::file_size(p); }

	inline bool remove(const path& p) { return fs_boost::remove(p); }

	inline bool remove_all(const fs::path& p)
	{
		boost::system::error_code ec;
		fs_boost::remove_all(p, ec);
		return !ec;
	}

	inline bool create_directory(const fs::path& p)
	{
		boost::system::error_code ec;
		fs_boost::create_directory(p, ec);
		return !ec;
	}

	inline bool create_directories(const fs::path& p)
	{
		boost::system::error_code ec;
		fs_boost::create_directories(p, ec);
		return !ec;
	}

	inline bool copy_file(const fs::path& from_p, const fs::path& to_p)
	{
		boost::system::error_code ec;
		fs_boost::copy_file(from_p, to_p, ec);
		return !ec;
	}

	inline bool rename(const fs::path& old_p, const fs::path& new_p)
	{
		boost::system::error_code ec;
		fs_boost::rename(old_p, new_p, ec);
		return !ec;
	}
}



char * read_file(const fs::path & path, size_t & size);

#endif // ARX_IO_FILESYSTEM_H
