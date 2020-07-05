/*
 * Copyright 2020 Arx Libertatis Team (see the AUTHORS file)
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

#include "io/resource/ResourceSetup.h"

#include <cstring>
#include <limits>
#include <string>
#include <set>

#include <boost/foreach.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/range/size.hpp>

#include "io/fs/Filesystem.h"
#include "io/fs/SystemPaths.h"
#include "io/resource/PakReader.h"
#include "io/log/Logger.h"
#include "platform/Platform.h"

namespace {

struct ResourceEntry {
	
	int dir;
	
	int archive; // archive type
	int subtype; // archive suffix
	
	std::string name;
	fs::path path;
	
	ResourceEntry(int dir_, int archive_, int subtype_, std::string name_, fs::path path_)
		: dir(dir_)
		, archive(archive_)
		, subtype(subtype_)
		, name(name_)
		, path(path_)
	{ }
	
	bool operator<(const ResourceEntry & o) const {
		if(archive != o.archive) {
			return archive < o.archive;
		}
		if(dir != o.dir) {
			return dir < o.dir;
		}
		if(subtype != o.subtype) {
			return subtype < o.subtype;
		}
		int n = name.compare(o.name);
		if(n != 0) {
			return n < 0;
		}
		return path.string().compare(o.path.string()) < 0;
	}
	
};

} // anonymous namespace

bool addDefaultResources(PakReader * reader) {
	
	PakFilter speechFilter;
	// French version has French audio copied in the deutsch and english directories
	{
		std::vector<std::string> & filter = speechFilter[util::md5::checksum("6d2cea5370477464b43353a637a18c0c")];
		filter.push_back("speech/deutsch");
		filter.push_back("speech/english");
	}
	// German version has German audio partially copied in the english and francais directories
	{
		std::vector<std::string> & filter = speechFilter[util::md5::checksum("499b08d1a365ebea74b23a8efc9a8302")];
		filter.push_back("speech/english");
		filter.push_back("speech/francais");
	}
	// Italian version has German audio partially copied in the francais directory
	{
		std::vector<std::string> & filter = speechFilter[util::md5::checksum("ca067587556a908be255e9244209fc47")];
		filter.push_back("speech/francais");
	}
	// Spanish version has Spanish audio copied in the deustch (sic) and english directories
	{
		std::vector<std::string> & filter = speechFilter[util::md5::checksum("975be9f2c6a952b4d014dfaf0d6f1882")];
		filter.push_back("speech/deustch");
		filter.push_back("speech/english");
	}
	
	const char * const directories[] = { "editor", "game", "graph", "localisation", "misc", "sfx", "speech" };
	struct {
		const char * name;
		PakFilter * filter;
	} const archives[] = {
		{ "data", NULL }, { "loc", NULL }, { "data2", NULL }, { "sfx", NULL }, { "speech", &speechFilter }
	};
	
	std::set<ResourceEntry> resources;
	
	int dirindex = 0;
	BOOST_REVERSE_FOREACH(const fs::path & dir, fs::getDataDirs()) {
		for(fs::directory_iterator it(dir); !it.end(); ++it) {
			std::string file = it.name();
			std::string name = boost::to_lower_copy(file);
			if(it.is_directory()) {
				BOOST_FOREACH(const char * dirname, directories) {
					if(name == dirname) {
						resources.insert(ResourceEntry(dirindex, std::numeric_limits<int>::max(), 0, name, dir / file));
						break;
					}
				}
			}
			if(it.is_regular_file() && boost::ends_with(name, ".pak")) {
				name.resize(name.length() - 4);
				for(size_t archive = 0; archive < boost::size(archives); archive++) {
					size_t length = std::strlen(archives[archive].name);
					if(name.length() >= length && name.compare(0, length, archives[archive].name, length) == 0
					   && (name.length() == length || name[length] == '_')) {
						// Load name_default.pak → name_*.pak → name.pak
						int subtype = 0;
						if(name.length() != length) {
							if(name.compare(length + 1, name.length() - length - 1, "default")) {
								subtype = -2;
							} else {
								subtype = -1;
							}
						}
						resources.insert(ResourceEntry(dirindex, archive, subtype, name, dir / file));
					}
				}
			}
		}
		dirindex++;
	}
	
	bool ok[ARRAY_SIZE(archives)] = { false };
	BOOST_FOREACH(const ResourceEntry & entry, resources) {
		if(entry.archive == std::numeric_limits<int>::max()) {
			reader->addFiles(entry.path, entry.name); 
		} else {
			if(reader->addArchive(entry.path, archives[entry.archive].filter)) {
				ok[entry.archive] = true;
			}
		}
	}
	
	bool result = true;
	for(size_t archive = 0; archive < boost::size(archives); archive++) {
		if(!ok[archive]) {
			LogError << "Missing required data file: \"" << archives[archive].name << "*.pak\"";
			result = false;
		}
	}
	
	return result;
}
