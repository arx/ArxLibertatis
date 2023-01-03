/*
 * Copyright 2011-2022 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_IO_FS_FILEPATH_H
#define ARX_IO_FS_FILEPATH_H

#include <ostream>
#include <string>
#include <string_view>
#include <utility>

#include "platform/Platform.h"

#if ARX_PLATFORM == ARX_PLATFORM_WIN32
#include <boost/algorithm/string/predicate.hpp>
#endif

namespace fs {

class path {
	
private:
	
	std::string pathstr;
	
	static path resolve(const path & base, const path & branch);
	
	static path create(std::string src);
	
public:
	
	static inline constexpr const char * dir_or_ext_sep = "\\/.";
	static inline constexpr const char * any_dir_sep = "\\/";
	
	#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	static inline constexpr const char dir_sep = '\\';
	#else
	static inline constexpr const char dir_sep = '/';
	#endif
	
	static inline constexpr const char ext_sep = '.';
	
	path() noexcept = default;
	/* implicit */ path(std::string_view str);
	/* implicit */ path(const std::string & str) : path(std::string_view(str)) { }
	/* implicit */ path(const char * str) : path(std::string_view(str)) { }
	
	[[nodiscard]] path operator/(const path & other) const;
	
	path & operator/=(const path & other);
	
	[[nodiscard]] const std::string & string() const noexcept {
		return pathstr;
	}
	
	/*!
	 * If pathstr contains a slash, return everything preceding it.
	 * Otherwise, return path().
	 */
	[[nodiscard]] path parent() const {
		if(has_info()) {
			size_t dirpos = pathstr.find_last_of(dir_sep);
			if(dirpos == std::string::npos) {
				return create(".");
			} else {
				return create(pathstr.substr(0, dirpos ? dirpos : 1));
			}
		} else if(empty() || is_dot()) {
			return create("..");
		} else if(pathstr[pathstr.length() - 1] == dir_sep) {
			return create(pathstr + "..");
		} else {
			return create(pathstr + dir_sep + "..");
		}
	}
	
	/*!
	 * return *this = parent()
	 */
	path & up() {
		return (*this = parent());
	}
	
	/*!
	 * If pathstr contains a slash, return everything following it.
	 * Otherwise, return pathstr.
	 */
	[[nodiscard]] std::string_view filename() const noexcept {
		size_t dirpos = pathstr.find_last_of(dir_sep);
		return (dirpos == std::string::npos) ? pathstr : std::string_view(pathstr).substr(dirpos + 1);
	}
	
	/*!
	 * If filename() contains a dot, return everything in filename() preceding the dot.
	 * Otherwise, return filename().
	 */
	[[nodiscard]] std::string_view basename() const noexcept;
	
	/*!
	 * If filename() contains a dot, return dot and everything following it.
	 * Otherwise, return std::string_view().
	 */
	[[nodiscard]] std::string_view ext() const noexcept;
	
	[[nodiscard]] bool empty() const noexcept {
		return pathstr.empty();
	}
	
	//! \return pathstr == other.pathstr
	[[nodiscard]] bool operator==(const path & other) const noexcept {
		#if ARX_PLATFORM == ARX_PLATFORM_WIN32
		return boost::iequals(pathstr, other.pathstr);
		#else
		return (pathstr == other.pathstr);
		#endif
	}
	
	//! \return pathstr != other.pathstr
	[[nodiscard]] bool operator!=(const path & other) const noexcept {
		#if ARX_PLATFORM == ARX_PLATFORM_WIN32
		return !boost::iequals(pathstr, other.pathstr);
		#else
		return (pathstr != other.pathstr);
		#endif
	}
	
	/*!
	 * To allow path being used in std::map, etc
	 * \return pathstr < other.pathstr
	 */
	[[nodiscard]] bool operator<(const path & other) const noexcept {
		#if ARX_PLATFORM == ARX_PLATFORM_WIN32
		return boost::ilexicographical_compare(pathstr, other.pathstr);
		#else
		return (pathstr < other.pathstr);
		#endif
	}
	
	/*!
	 * If ext starts with a dot, return *this = remove_ext().append(ext).
	 * Otherwise, return *this = remove_ext().append('.').append(ext).
	 */
	path & set_ext(std::string_view ext);
	
	/*!
	 * If pathstr contains a dot after the last slash, return everything preceding the last dot.
	 * \return *this
	 */
	path & remove_ext();
	
	//! *this = parent() / filename;
	path & set_filename(std::string_view filename);
	
	path & set_basename(std::string_view basename);
	
	//! \return set_basename(get_basename() + basename_part)
	path & append_basename(std::string_view basename_part);
	
	void swap(path & other) noexcept {
		pathstr.swap(other.pathstr);
	}
	
	/*!
	 * Check if a path has a fixe extension.
	 *
	 * Comparison is is done case-insensitively.
	 *
	 * \return str.empty() ? !ext().empty() : ext() == str || ext.substr(1) == str();
	 */
	bool has_ext(std::string_view str = std::string_view()) const noexcept;
	
	//! ".." or starts with "../"
	bool is_up() const noexcept {
		return (pathstr.length() == 2 && pathstr[0] == '.' && pathstr[1] == '.')
		       || (pathstr.length() >= 3 && pathstr[0] == '.' && pathstr[1] == '.' && pathstr[2] == dir_sep);
	}
	
	//! \return true if not empty, not "/", not "." not ".." and doesn't end in "/.."
	bool has_info() const noexcept {
		size_t l = pathstr.length();
		return (!pathstr.empty() && !(l == 2 && pathstr[0] == '.' && pathstr[1] == '.')
		        && !(l >= 3 && pathstr[l - 1] == '.' && pathstr[l - 2] == '.'
		                    && pathstr[l - 3] == dir_sep)
		        && !(pathstr[l - 1] == dir_sep)
		        && !is_dot());
	}
	
	bool is_dot() const noexcept { return pathstr.length() == 1 && pathstr[0] == '.'; }
	
	bool is_root() const noexcept {
		return ((pathstr.length() == 1 && pathstr[0] == dir_sep)
		#if ARX_PLATFORM == ARX_PLATFORM_WIN32
			|| (pathstr.length() <= 3 && pathstr.length() >= 2 && pathstr[1] == ':')
		#endif
		);
	}
	
	//! Is this a relative path. An empty path is neither relative nor absolute.
	bool is_relative() const noexcept {
		return !empty() && !(pathstr[0] == dir_sep
			#if ARX_PLATFORM == ARX_PLATFORM_WIN32
			|| (pathstr.length() >= 2 && pathstr[1] == ':')
			#endif
		);
	}
	
	//! Is this an absolute path. An empty path is neither relative nor absolute.
	bool is_absolute() const noexcept {
		return !empty() && (pathstr[0] == dir_sep
			#if ARX_PLATFORM == ARX_PLATFORM_WIN32
			|| (pathstr.length() >= 2 && pathstr[1] == ':')
			#endif
		);
	}
	
	path & append(std::string_view str);
	
	void clear() noexcept { pathstr.clear(); }
	
};

[[nodiscard]] inline path operator/(std::string_view a, const path & b) {
	return path(a) / b;
}

[[nodiscard]] inline path operator/(const std::string & a, const path & b) {
	return path(a) / b;
}

[[nodiscard]] inline path operator/(const char * a, const path & b) {
	return path(a) / b;
}

inline std::ostream & operator<<(std::ostream & strm, const path & path) {
	return strm << '"' << path.string() << '"';
}

} // namespace fs

#endif // ARX_IO_FS_FILEPATH_H
