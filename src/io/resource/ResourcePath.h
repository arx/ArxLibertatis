/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include <string>
#include <ostream>

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
	
	static const char dir_or_ext_sep[];
	static const char dir_sep = '/';
	static const char ext_sep = '.';
	
	path() { }
	/* implicit */ path(const std::string & str) : pathstr(str) { check(); }
	/* implicit */ path(const char * str) : pathstr(str) { check(); }
	
	path & operator=(const std::string & str) {
		pathstr = str;
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
	
	const std::string & string() const {
		return pathstr;
	}
	
	/*!
	 * If pathstr contains a slash, return everything preceding it.
	 * Otherwise, return path().
	 */
	path parent() const {
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
	std::string filename() const {
		size_t dirpos = pathstr.find_last_of(dir_sep);
		return (dirpos == std::string::npos) ? pathstr : pathstr.substr(dirpos + 1);
	}
	
	/*!
	 * If filename() constains a dot, return everything in filename() preceeding the dot.
	 * Otherwise, return filename().
	 */
	std::string basename() const;
	
	/*!
	 * If filename() constains a dot, return dot and everything folowing it.
	 * Otherwise, return std::string().
	 */
	std::string ext() const;
	
	bool empty() const {
		return pathstr.empty();
	}
	
	//! \return pathstr == other.pathstr
	bool operator==(const path & other) const {
		return (pathstr == other.pathstr);
	}
	
	//! \return pathstr == str
	bool operator==(const std::string & str) const {
		return (pathstr == str);
	}
	
	/*!
	 * \return pathstr == str
	 * This overload is neccessary so comparing with string constants isn't ambigous
	 */
	bool operator==(const char * str) const {
		return !pathstr.compare(0, pathstr.length(), str);
	}
	
	//! \return pathstr != other.pathstr
	bool operator!=(const path & other) const {
		return (pathstr != other.pathstr);
	}
	
	//! \return pathstr != str
	bool operator!=(const std::string & str) const {
		return (pathstr != str);
	}
	
	/*!
	 * \return pathstr != str
	 * This overload is neccessary so comparing with string constants isn't ambigous
	 */
	bool operator!=(const char * str) const {
		return pathstr.compare(0, pathstr.length(), str) != 0;
	}
	
	/*!
	 * To allow path being used in std::map, etc
	 * \return pathstr < other.pathstr
	 */
	bool operator<(const path & other) const {
		return (pathstr < other.pathstr);
	}
	
	/*!
	 * If ext starts with a dot, return *this = remove_ext().append(ext).
	 * Otherwise, return *this = remove_ext().append('.').append(ext).
	 */
	path & set_ext(const std::string & ext);
	
	/*!
	 * If pathstr constains a dot after the last slash, return everything preceeding the last dot
	 * \return *this
	 */
	path & remove_ext();
	
	//! *this = parent() / filename;
	path & set_filename(const std::string & filename);
	
	path & set_basename(const std::string & basename);
	
	//! \return set_basename(get_basename() + basename_part)
	path & append_basename(const std::string & basename_part);
	
	void swap(path & other) {
		pathstr.swap(other.pathstr);
	}
	
	//! return str.empty() ? !ext().empty() : ext() == str || ext.substr(1) == str();
	bool has_ext(const std::string & str = std::string()) const;
	
	bool is_up() const {
		return (pathstr.length() == 2 && pathstr[0] == '.' && pathstr[1] == '.')
		       || (pathstr.length() >= 3 && pathstr[0] == '.' && pathstr[1] == '.' && pathstr[2] == dir_sep);
	}
	
	bool has_info() const {
		return !pathstr.empty() && !(pathstr.length() == 2 && pathstr[0] == '.' && pathstr[1] == '.')
		       && !(pathstr.length() >= 3 && pathstr[pathstr.length() - 1] == '.'
		           && pathstr[pathstr.length() - 2] == '.' && pathstr[pathstr.length() - 3] == dir_sep);
	}
	
	static path load(const std::string & str);
	
	/*!
	 * Append a string to the paths filename component
	 *
	 * The appended string must not contain a path seperator or be "." or "..".
	 */
	path & append(const std::string & str);
	
	//! \return append(str)
	path & operator+=(const std::string & str) {
		append(str);
		return *this;
	}
	
	//! \return path(*this).append(str)
	path operator+(const std::string & str) const {
		return path(*this) += str;
	}
	
	void clear() { pathstr.clear(); }
	
};

inline path operator/(const char * a, const path & b) {
	return path(a) / b;
}

inline path operator/(const std::string & a, const path & b) {
	return path(a) / b;
}

inline bool operator==(const std::string & a, const path & b) {
	return (b == a);
}

inline bool operator==(const char * a, const path & b) {
	return (b == a);
}

inline bool operator!=(const std::string & a, const path & b) {
	return (b != a);
}

inline bool operator!=(const char * a, const path & b) {
	return (b != a);
}

inline std::ostream & operator<<(std::ostream & strm, const path & path) {
	return strm << '"' << path.string() << '"';
}

} // namespace res

#endif // ARX_IO_RESOURCE_RESOURCEPATH_H
