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

#include "core/SaveGame.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

#include "core/Config.h"
#include "io/fs/Filesystem.h"
#include "io/fs/SystemPaths.h"
#include "io/log/Logger.h"
#include "io/resource/PakReader.h"
#include "scene/ChangeLevel.h"

const fs::path SAVEGAME_NAME = "gsave.sav";

namespace {

const fs::path SAVEGAME_DIR = "save";
const fs::path SAVEGAME_THUMBNAIL = "gsave.bmp";
const std::string QUICKSAVE_ID = "ARX_QUICK_ARX";

enum SaveGameChange {
	SaveGameRemoved,
	SaveGameUnchanged,
	SaveGameChanged
};

int saveTimeCompare(const SaveGame & a, const SaveGame & b) {
	return (a.stime > b.stime);
}

} // anonymous namespace

SaveGameList savegames;

void SaveGameList::update(bool verbose) {
	
	LogDebug("SaveGameList::update()");
	
	size_t old_count = savelist.size();
	std::vector<SaveGameChange> found(old_count, SaveGameRemoved);
	
	bool new_saves = false;
	
	size_t max_name_length = 0;
	
	fs::path savedir = fs::getUserDir() / SAVEGAME_DIR;
	
	if(verbose) {
		LogInfo << "Using save game dir " << savedir;
	}
	
	for(fs::directory_iterator it(savedir); !it.end(); ++it) {
		
		fs::path dirname = it.name();
		fs::path path = savedir / dirname / SAVEGAME_NAME;
		
		std::time_t stime = fs::last_write_time(path);
		if(stime == 0) {
			LogDebug("Ignoring directory without " << SAVEGAME_NAME << ": " << path);
			continue;
		}
		
		size_t index = size_t(-1);
		for(size_t i = 0; i < old_count; i++) {
			if(savelist[i].savefile == path) {
				index = i;
			}
		}
		if(index != size_t(-1) && savelist[index].stime == stime) {
			found[index] = SaveGameUnchanged;
			continue;
		}
		
		std::string name;
		float version;
		long level;
		if(ARX_CHANGELEVEL_GetInfo(path, name, version, level) == -1) {
			LogWarning << "Unable to get save file info for " << path;
			continue;
		}
		
		new_saves = true;
		
		if(index == size_t(-1)) {
			// Make another save game slot at the end
			index = savelist.size();
			savelist.resize(savelist.size() + 1);
		} else {
			found[index] = SaveGameChanged;
		}
		
		SaveGame * save = &savelist[index];
		
		save->name = name;
		save->level = level;
		save->stime = stime;
		save->savefile = path;
		
		save->quicksave = (name == QUICKSAVE_ID || name == "ARX_QUICK_ARX1");
		
		fs::path thumbnail = path.parent() / SAVEGAME_THUMBNAIL;
		if(fs::exists(thumbnail)) {
			// Resource paths must be lowercase (for now), but filesystem paths can be
			// mixed case and case sensitive, so we can't just convert the save dirname
			// to lowercase and expect to not get collisions.
			// Instead, choose a unique number.
			res::path thumbnail_res;
			size_t i = 0;
			do {
				std::ostringstream oss;
				oss << "thumbnail" << i << SAVEGAME_THUMBNAIL.ext();
				thumbnail_res = res::path("save") / oss.str();
				i++;
			} while(g_resources->getFile(thumbnail_res));
			g_resources->addFiles(thumbnail, thumbnail_res);
			save->thumbnail = thumbnail_res.remove_ext();
		} else {
			save->thumbnail.clear();
		}
		
		size_t length = save->quicksave ? 9 : std::min(name.length(), size_t(50));
		max_name_length = std::max(length, max_name_length);
		
	}
	
	size_t o = 0;
	for(size_t i = 0; i < savelist.size(); i++) {
		
		// print new savegames
		if(verbose || i >= old_count || found[i] == SaveGameChanged) {
			
			std::ostringstream oss;
			if(savelist[i].quicksave) {
				oss << "(quicksave)" << std::setw(max_name_length - 8) << ' ';
			} else if(savelist[i].name.length() <= 50)  {
				oss << "\"" << savelist[i].name << "\""
				    << std::setw(max_name_length - savelist[i].name.length() + 1) << ' ';
			} else {
				oss << "\"" << savelist[i].name.substr(0, 49) << "\"…"
				    << std::setw(max_name_length - 50 + 1) << ' ';
			}
			
			const char * lead = """Found save ";
			if(verbose) {
				if(i + 1 == savelist.size()) {
					lead = " └─ ";
				} else {
					lead = " ├─ ";
				}
			}
			
			const std::tm & t = *localtime(&savelist[i].stime);
			
			LogInfo << lead << oss.str() << "  "
			        << std::setfill('0') << (t.tm_year + 1900) << "-"
			        << std::setw(2) << (t.tm_mon + 1) << "-"
			        << std::setw(2) << t.tm_mday << "  "
			        << std::setfill(' ') << std::setw(2) << t.tm_hour << ":"
			        << std::setfill('0') << std::setw(2) << t.tm_min;
			
		}
		
		if(i >= old_count || found[i] != SaveGameRemoved) {
			if(o != i) {
				savelist[o] = savelist[i];
			}
			o++;
		} else {
			// Clean obsolete mounts
			g_resources->removeFile(savelist[i].thumbnail);
			g_resources->removeDirectory(savelist[i].thumbnail.parent());
		}
	}
	savelist.resize(o);
	
	if(new_saves) {
		std::sort(savelist.begin(), savelist.end(), saveTimeCompare);
	}
	
	LogDebug("Found " << savelist.size() << " savegames");
}

void SaveGameList::remove(SavegameHandle handle) {
	
	const SaveGame & save = savelist[handle.handleData()];
	
	fs::remove(save.savefile);
	fs::path savedir = save.savefile.parent();
	fs::remove(savedir / SAVEGAME_THUMBNAIL);
	if(fs::directory_iterator(savedir).end()) {
		fs::remove(savedir);
	}
	
	update();
}

bool SaveGameList::save(const std::string & name, SavegameHandle overwrite, const Image & thumbnail) {
	
	fs::path savefile;
	if(overwrite != SavegameHandle()) {
		savefile = savelist[size_t(overwrite.handleData())].savefile;
	} else {
		size_t index = 0;
		do {
			std::ostringstream oss;
			oss << "save" << std::setfill('0') << std::setw(4) << index++;
			savefile = fs::getUserDir() / SAVEGAME_DIR / oss.str();
		} while(fs::exists(savefile));
		
		if(!fs::create_directories(savefile)) {
			LogWarning << "Failed to create save directory";
		}
		
		savefile /= SAVEGAME_NAME;
	}
	
	if(!ARX_CHANGELEVEL_Save(name, savefile)) {
		return false;
	}
	
	if(thumbnail.isValid() && !thumbnail.save(savefile.parent() / SAVEGAME_THUMBNAIL)) {
		LogWarning << "Failed to save screenshot to " << (savefile.parent() / SAVEGAME_THUMBNAIL);
	}
	
	update();
	
	return true;
}

bool SaveGameList::quicksave(const Image & thumbnail) {
	
	SavegameHandle overwrite = SavegameHandle();
	std::time_t time = std::numeric_limits<std::time_t>::max();
	
	size_t nfound = 0;
	
	// Find the oldest quicksave.
	for(size_t i = 0; i != size(); ++i) {
		if(savelist[i].quicksave) {
			nfound++;
			if(savelist[i].stime < time) {
				overwrite = SavegameHandle(long(i));
				time = savelist[i].stime;
			}
		}
	}
	
	// Create a new quicksave slot if there aren't enough already.
	if(nfound < size_t(config.misc.quicksaveSlots)) {
		overwrite = SavegameHandle();
	}
	
	return save(QUICKSAVE_ID, overwrite, thumbnail);
}

SavegameHandle SaveGameList::quickload() {
	
	if(savelist.empty()) {
		return SavegameHandle();
	}
	
	// update() already sorts the savegame list so we can just return the first one.
	
	return SavegameHandle(0);
}
