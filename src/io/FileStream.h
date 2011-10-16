/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_IO_FILESTREAM_H
#define ARX_IO_FILESTREAM_H

#include <fstream>

namespace fs {

class path;

class ifstream : public std::ifstream {
	
	ifstream(const ifstream &);
	ifstream & operator=(const ifstream &);
	
public:
	
	ifstream() { }
	
	ifstream(const path & p, openmode mode = in);
	
	void open(const path & p, openmode mode = in);
	
	virtual ~ifstream() { }
	
};

class ofstream : public std::ofstream {
	
	ofstream(const ofstream &);
	ofstream & operator=(const ofstream &);
	
public:
	
	ofstream() { }
	
	ofstream(const path & p, openmode mode = out);
	
	void open(const path & p, openmode mode = out);
	
	virtual ~ofstream() { }
	
};

class fstream : public std::fstream {
	
	fstream(const fstream &);
	fstream & operator=(const fstream &);
	
public:
	
	fstream() { }
	
	fstream(const path & p, openmode mode = out);
	
	void open(const path & p, openmode mode = out);
	
	virtual ~fstream() { }
	
};

template <class T>
inline std::istream & read(std::istream & ifs, T & data) {
	return ifs.read(reinterpret_cast<char *>(&data), sizeof(T));
}

inline std::istream & read(std::istream & ifs, void * buf, size_t n) {
	return ifs.read(reinterpret_cast<char *>(buf), n);
}

template <class T>
inline std::ostream & write(std::ostream & ifs, const T & data) {
	return ifs.write(reinterpret_cast<const char *>(&data), sizeof(T));
}

inline std::ostream & write(std::ostream & ifs, const void * buf, size_t n) {
	return ifs.write(reinterpret_cast<const char *>(buf), n);
}

std::istream & read(std::istream & ifs, std::string & buf);

} // namespace fs

#endif // ARX_IO_FILESTREAM_H
