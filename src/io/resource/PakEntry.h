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

#include "util/Range.h"

namespace res { class path; }

class PakFileHandle;

class PakFile {
	
	std::unique_ptr<PakFile> m_alternative;
	
	friend class PakReader;
	friend class PakDirectory;
	
public:
	
	PakFile() = default;
	virtual ~PakFile() = default;
	
	PakFile(const PakFile &) = delete;
	PakFile & operator=(const PakFile &) = delete;
	
	PakFile * alternative() const { return m_alternative.get(); }
	
	[[nodiscard]] virtual std::string read() const = 0;
	
	[[nodiscard]] virtual std::unique_ptr<PakFileHandle> open() const = 0;
	
};

class PakDirectory {
	
	template <typename T>
	struct Entry {
		
		const std::string & name;
		T & entry;
		
		operator const std::string &() {
			return name;
		}
		
		operator std::string_view() {
			return name;
		}
		
		operator T &() {
			return entry;
		}
		
	};
	
	// TODO hash maps might be a better fit
	std::map<std::string, std::unique_ptr<PakFile>, std::less<>> m_files;
	std::map<std::string, PakDirectory, std::less<>> m_dirs;
	
	PakDirectory * addDirectory(const res::path & path);
	
	void addFile(std::string && name, std::unique_ptr<PakFile> file);
	void removeFile(std::string_view name);
	bool removeDirectory(std::string_view name);
	
	friend class PakReader;
	friend class std::map<std::string, PakDirectory, std::less<>>;
	friend struct std::pair<const std::string, PakDirectory>;
	
public:
	
	PakDirectory() = default;
	~PakDirectory() = default;
	
	PakDirectory(const PakDirectory &) = delete;
	PakDirectory & operator=(const PakDirectory &) = delete;
	
	PakDirectory * getDirectory(const res::path & path);
	
	PakFile * getFile(const res::path & path);
	
	bool hasFile(const res::path & path) {
		return getFile(path) != nullptr;
	}
	
	auto dirs() {
		return util::transform(m_dirs, [](auto & base) {
			return Entry<PakDirectory>{ base.first, base.second };
		});
	}
	
	auto files() {
		return util::transform(m_files, [](auto & base) {
			arx_assert(base.second);
			return Entry<PakFile>{ base.first, *base.second.get() };
		});
	}
	
	bool empty() { return m_dirs.empty() && m_files.empty(); }
	
};

#endif // ARX_IO_RESOURCE_PAKENTRY_H
