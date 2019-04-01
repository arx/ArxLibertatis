/*
 * Copyright 2019 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_IO_ZIP_ZIPREADER_H
#define ARX_IO_ZIP_ZIPREADER_H

#include "io/fs/FileStream.h"

namespace zip {

struct ZipCallback {
	virtual void zipEntry(const std::string & filePath, size_t offset, size_t size) = 0;
};

class ZipFileException : public std::exception {
	
	const char * m_message;
	
public:
	explicit ZipFileException(const char * message)
		:m_message(message)
	{ }
	
	virtual const char * what() const throw() {
		return m_message;
	}
};


void readFile(fs::ifstream & ifs, ZipCallback & callback);

} // namespace zip

#endif // ARX_IO_ZIP_ZIPREADER_H
