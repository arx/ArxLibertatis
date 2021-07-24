/*
 * Copyright 2011-2021 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_IO_RESOURCE_RESOURCEPATH_H
#define ARX_IO_RESOURCE_RESOURCEPATH_H

#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace res {

class path {
	
private:
	
	std::string pathstr;
	
#ifdef ARX_DEBUG
	void check() const;
#else
	void check() const { }
#endif
	
	static path resolve(const path & base, const path & branch);
	
public:
	
	static constexpr const char * const dir_or_ext_sep = "/.";
	static constexpr const char dir_sep = '/';
	static constexpr const char ext_sep = '.';
	
	path() { }
	/* implicit */ path(std::string_view str) : pathstr(str) { check(); }
	/* implicit */ path(std::string str) : pathstr(std::move(str)) { check(); }
	/* implicit */ path(const char * str) : pathstr(str) { check(); }
	
	path & operator=(std::string_view str) {
		pathstr = str;
		check();
		return *this;
	}
	
	path & operator=(std::string str) {
		pathstr = std::move(str);
		check();
		return *this;
	}
	
	path & operator=(const char * str) {
		pathstr = str;
		check();
		return *this;
	}
	
	path operator/(const path & other) const;
	
	path & operator/=(const path & other);
	
	[[nodiscard]] const std::string & string() const {
		return pathstr;
	}
	
	/*!
	 * If pathstr contains a slash, return everything preceding it.
	 * Otherwise, return path().
	 */
	[[nodiscard]] path parent() const {
		if(has_info()) {
			size_t dirpos = pathstr.find_last_of(dir_sep);
			return (dirpos == std::string::npos) ? path() : path(pathstr.substr(0, dirpos));
		} else {
			return empty() ? path("..") : path(pathstr + dir_sep + "..");
		}
	}
	
	/*!
	 * return *this = parent()
	 */
	path & up() {
		if(has_info()) {
			size_t dirpos = pathstr.find_last_of(dir_sep);
			return (((dirpos != std::string::npos) ? pathstr.resize(dirpos) : pathstr.clear()), *this);
		} else {
			return ((empty() ? pathstr = ".." : (pathstr += dir_sep).append("..")), *this);
		}
	}
	
	/*!
	 * If pathstr contains a slash, return everything following it.
	 * Otherwise, return pathstr.
	 */
	[[nodiscard]] std::string_view filename() const {
		size_t dirpos = pathstr.find_last_of(dir_sep);
		return (dirpos == std::string::npos) ? pathstr : std::string_view(pathstr).substr(dirpos + 1);
	}
	
	/*!
	 * If filename() contains a dot, return everything in filename() preceding the dot.
	 * Otherwise, return filename().
	 */
	[[nodiscard]] std::string_view basename() const;
	
	/*!
	 * If filename() contains a dot, return dot and everything following it.
	 * Otherwise, return std::string().
	 */
	[[nodiscard]] std::string_view ext() const;
	
	[[nodiscard]] bool empty() const {
		return pathstr.empty();
	}
	
	//! \return pathstr == other.pathstr
	[[nodiscard]] bool operator==(const path & other) const {
		return (pathstr == other.pathstr);
	}
	
	//! \return pathstr == str
	[[nodiscard]] bool operator==(std::string_view str) const {
		return (pathstr == str);
	}
	
	//! \return pathstr == str
	[[nodiscard]] bool operator==(const std::string & str) const {
		return (pathstr == str);
	}
	
	/*!
	 * \return pathstr == str
	 * This overload is necessary so comparing with string constants isn't ambiguous.
	 */
	[[nodiscard]] bool operator==(const char * str) const {
		return !pathstr.compare(0, pathstr.length(), str);
	}
	
	//! \return pathstr != other.pathstr
	[[nodiscard]] bool operator!=(const path & other) const {
		return (pathstr != other.pathstr);
	}
	
	//! \return pathstr != str
	[[nodiscard]] bool operator!=(std::string_view str) const {
		return (pathstr != str);
	}
	
	//! \return pathstr != str
	[[nodiscard]] bool operator!=(const std::string & str) const {
		return (pathstr != str);
	}
	
	/*!
	 * \return pathstr != str
	 * This overload is necessary so comparing with string constants isn't ambiguous.
	 */
	[[nodiscard]] bool operator!=(const char * str) const {
		return pathstr.compare(0, pathstr.length(), str) != 0;
	}
	
	/*!
	 * To allow path being used in std::map, etc
	 * \return pathstr < other.pathstr
	 */
	[[nodiscard]] bool operator<(const path & other) const {
		return (pathstr < other.pathstr);
	}
	
	/*!
	 * If ext starts with a dot, return *this = remove_ext().append(ext).
	 * Otherwise, return *this = remove_ext().append('.').append(ext).
	 */
	path & set_ext(std::string_view ext);
	
	/*!
	 * If pathstr contains a dot after the last slash, return everything preceding the last dot
	 * \return *this
	 */
	path & remove_ext();
	
	//! *this = parent() / filename;
	path & set_filename(std::string_view filename);
	
	path & set_basename(std::string_view basename);
	
	//! \return set_basename(get_basename() + basename_part)
	path & append_basename(std::string_view basename_part);
	
	void swap(path & other) {
		pathstr.swap(other.pathstr);
	}
	
	//! return str.empty() ? !ext().empty() : ext() == str || ext.substr(1) == str();
	[[nodiscard]] bool has_ext(std::string_view str = std::string_view()) const;
	
	[[nodiscard]] bool is_up() const {
		return (pathstr.length() == 2 && pathstr[0] == '.' && pathstr[1] == '.')
		       || (pathstr.length() >= 3 && pathstr[0] == '.' && pathstr[1] == '.' && pathstr[2] == dir_sep);
	}
	
	[[nodiscard]] bool has_info() const {
		return !pathstr.empty() && !(pathstr.length() == 2 && pathstr[0] == '.' && pathstr[1] == '.')
		       && !(pathstr.length() >= 3 && pathstr[pathstr.length() - 1] == '.'
		           && pathstr[pathstr.length() - 2] == '.' && pathstr[pathstr.length() - 3] == dir_sep);
	}
	
	[[nodiscard]] static path load(std::string_view str);
	
	/*!
	 * Append a string to the paths filename component
	 *
	 * The appended string must not contain a path separator or be "." or "..".
	 */
	path & append(std::string_view str);
	
	//! \return append(str)
	path & operator+=(std::string_view str) {
		append(str);
		return *this;
	}
	
	//! \return path(*this).append(str)
	[[nodiscard]] path operator+(std::string_view str) const {
		return path(*this) += str;
	}
	
	void clear() { pathstr.clear(); }
	
};

[[nodiscard]] inline path operator/(std::string_view a, const path & b) {
	return path(a) / b;
}

[[nodiscard]] inline path operator/(std::string a, const path & b) {
	return path(std::move(a)) / b;
}

[[nodiscard]] inline path operator/(const char * a, const path & b) {
	return path(a) / b;
}

inline std::ostream & operator<<(std::ostream & strm, const path & path) {
	return strm << '"' << path.string() << '"';
}

} // namespace res

#endif // ARX_IO_RESOURCE_RESOURCEPATH_H
