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

#ifndef ARX_IO_RESOURCE_PAKENTRY_H
#define ARX_IO_RESOURCE_PAKENTRY_H

#include <string>
#include <map>

#include <boost/noncopyable.hpp>

namespace res { class path; }

class PakFileHandle;

class PakFile : private boost::noncopyable {
	
private:
	
	size_t _size;
	
	PakFile * _alternative;
	
protected:
	
	explicit PakFile(size_t size) :  _size(size), _alternative(NULL) { }
	
	virtual ~PakFile();
	
	friend class PakReader;
	friend class PakDirectory;
	
public:
	
	size_t size() const { return _size; }
	PakFile * alternative() const { return _alternative; }
	
	virtual void read(void * buf) const = 0;
	std::string read() const;
	
	virtual PakFileHandle * open() const = 0;
	
};

class PakDirectory {
	
private:
	
	// TODO hash maps might be a better fit
	std::map<std::string, PakFile *> files;
	std::map<std::string, PakDirectory> dirs;
	
	PakDirectory * addDirectory(const res::path & path);
	
	void addFile(const std::string & name, PakFile * file);
	void removeFile(const std::string & name);
	bool removeDirectory(const std::string & name);
	
	friend class PakReader;
	friend class std::map<std::string, PakDirectory>;
	friend struct std::pair<const std::string, PakDirectory>;
	
public:
	
	PakDirectory();
	~PakDirectory();
	
	typedef std::map<std::string, PakDirectory>::iterator dirs_iterator;
	typedef std::map<std::string, PakFile *>::const_iterator files_iterator;
	
	PakDirectory * getDirectory(const res::path & path);
	
	PakFile * getFile(const res::path & path);
	
	bool hasFile(const res::path & path) {
		return getFile(path) != NULL;
	}
	
	dirs_iterator dirs_begin() { return dirs.begin(); }
	dirs_iterator dirs_end() { return dirs.end(); }
	
	files_iterator files_begin() { return files.begin(); }
	files_iterator files_end() { return files.end(); }
	
	bool has_files() { return files_begin() != files_end(); }
	bool has_dirs() { return dirs_begin() != dirs_end(); }
	
	bool empty() { return !has_dirs() && !has_files(); }
	
};

#endif // ARX_IO_RESOURCE_PAKENTRY_H
