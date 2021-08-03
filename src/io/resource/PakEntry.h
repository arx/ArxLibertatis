/*
 * Copyright 2011-2020 Arx Libertatis Team (see the AUTHORS file)
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

#include <map>
#include <memory>
#include <string>
#include <string_view>

namespace res { class path; }

class PakFileHandle;

class PakFile {
	
private:
	
	PakFile * _alternative;
	
protected:
	
	explicit PakFile() : _alternative(nullptr) { }
	
	virtual ~PakFile();
	
	friend class PakReader;
	friend class PakDirectory;
	
public:
	
	PakFile(const PakFile &) = delete;
	PakFile & operator=(const PakFile &) = delete;
	
	PakFile * alternative() const { return _alternative; }
	
	[[nodiscard]] virtual std::string read() const = 0;
	
	[[nodiscard]] virtual std::unique_ptr<PakFileHandle> open() const = 0;
	
};

class PakDirectory {
	
private:
	
	// TODO hash maps might be a better fit
	std::map<std::string, PakFile *, std::less<>> files;
	std::map<std::string, PakDirectory, std::less<>> dirs;
	
	PakDirectory * addDirectory(const res::path & path);
	
	void addFile(std::string && name, PakFile * file);
	void removeFile(std::string_view name);
	bool removeDirectory(std::string_view name);
	
	friend class PakReader;
	friend class std::map<std::string, PakDirectory, std::less<>>;
	friend struct std::pair<const std::string, PakDirectory>;
	
public:
	
	~PakDirectory();
	
	typedef std::map<std::string, PakDirectory, std::less<>>::iterator dirs_iterator;
	typedef std::map<std::string, PakFile *, std::less<>>::const_iterator files_iterator;
	
	PakDirectory * getDirectory(const res::path & path);
	
	PakFile * getFile(const res::path & path);
	
	bool hasFile(const res::path & path) {
		return getFile(path) != nullptr;
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
