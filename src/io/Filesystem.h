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

namespace fs_boost = boost::filesystem;

// WIP!!
namespace fs {

	class path {
	public:
		path(const fs_boost::path& p) : boost_path(p) {}
		operator fs_boost::path () const { return boost_path; }

	public:
	
		path() {}
		path(const path & name) : boost_path(name.filename()) {}
				
		// TODO allow trailing slashes? boost removes it!
		path(const std::string & name) : boost_path(name) {}
			
		// maybe? - to load from char constants without strlen?
		template <size_t N>
		path(const char (&name)[N]) : boost_path(name) {}
	
		// same for operator=
	
		// "foo" / "bar" -> "foo/bar"  |  "" / "foo" -> "foo"  |  "foo / "" -> "foo"
		// +overloads for std::string
		path operator/(const path & other) const { return boost_path / other.boost_path; }
			
		// *this = path(*this) / other;
		// returns *this
		// +overloads for std::string
		path & operator/=(const path & other) { boost_path /= other.boost_path; return *this; }
	
		// retrun pathname;
		const std::string & string() const { pathname = boost_path.string(); return pathname; }
	
		// "foo/bar" => "foo" | foo => ""
		const path parent() const { return boost_path.parent_path(); }
	
		// "foo/bar.ext" => "bar.ext"  |  "foo/bar" -> "bar"  |  "foo" -> "foo"  |  "foo.ext" -> "foo.ext"
		const std::string filename() const { return boost_path.filename().string(); }
	
		// "foo/bar.ext" => "bar"  |  "foo/bar" -> "bar"  |  "foo" -> "foo"  |  "foo.ext" -> "foo"
		const std::string basename() const;
	
		// "foo/bar.ext" => ".ext"  | "foo/bar" -> ""
		const std::string ext() const { return boost_path.extension().string(); }
	
		// "" -> true  |  "foo" -> false
		bool empty() const { return boost_path.empty(); }
	
		 // TODO +overlad for std::string?
		bool operator==(const path & other) const { return boost_path == other.boost_path; }
	
		// so it can be used in ordered maps, etc
		bool operator<(const path & other) const { return boost_path < other.boost_path; }
	
		/*
		 * "foo/bar".set_ext(".ext") -> "foo/bar.ext"
		 * "foo/bar".set_ext("ext") -> "foo/bar.ext"
		 * "foo/bar".set_ext("") -> "foo/bar"
		 * "foo/bar.abc".set_ext(".ext") -> "foo/bar.ext"
		 * "foo/bar.abc".set_ext("ext") -> "foo/bar.ext"
		 * "foo/bar.abc".set_ext("") -> "foo/bar"
		 * returns *this
		 */
		path & set_ext(const std::string & ext = std::string()) { boost_path.replace_extension(ext); return *this; }
	
		// *this = parent() / filename;
		path & set_filename(const std::string & filename) { boost_path = boost_path.parent_path() / filename; return *this; }
	
		// TODO some form of append?
	
	private:
	
		// TMP - mutable !
		mutable std::string pathname;

		// TMP
		fs_boost::path boost_path;	
	};

	path operator/(const std::string & base, const path & p) {
		return path(base) / p;
	}

	std::ostream & operator<<(std::ostream & strm, const path & p) {
		return strm << '"' << p.string() << '"';
	}
			
	bool exists(const path& p) { return fs_boost::exists(p); }
	bool is_directory(const path& p) { return fs_boost::is_directory(p); }
	bool is_regular_file(const path& p) { return fs_boost::is_regular_file(p); }

	std::time_t last_write_time(const path& p) { return fs_boost::last_write_time(p); }
	u64 file_size(const path& p) { return fs_boost::file_size(p); }

	bool remove(const path& p) { return fs_boost::remove(p); }

	bool remove_all(const fs::path& p)
	{
		boost::system::error_code ec;
		fs_boost::remove_all(p, ec);
		return !ec;
	}

	bool create_directory(const fs::path& p)
	{
		boost::system::error_code ec;
		fs_boost::create_directory(p, ec);
		return !ec;
	}

	bool create_directories(const fs::path& p)
	{
		boost::system::error_code ec;
		fs_boost::create_directories(p, ec);
		return !ec;
	}

	bool copy_file(const fs::path& from_p, const fs::path& to_p)
	{
		boost::system::error_code ec;
		fs_boost::copy_file(from_p, to_p, ec);
		return !ec;
	}

	bool rename(const fs::path& old_p, const fs::path& new_p)
	{
		boost::system::error_code ec;
		fs_boost::rename(old_p, new_p, ec);
		return !ec;
	}
}



char * read_file(const fs::path & path, size_t & size);

#endif // ARX_IO_FILESYSTEM_H
