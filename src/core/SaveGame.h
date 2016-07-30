/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_CORE_SAVEGAME_H
#define ARX_CORE_SAVEGAME_H

#include <stddef.h>
#include <vector>
#include <string>
#include <ctime>

#include "graphics/image/Image.h"
#include "io/fs/FilePath.h"
#include "io/resource/ResourcePath.h"
#include "util/HandleType.h"

typedef HandleType<struct SavegameHandleTag, long, -1> SavegameHandle;

struct SaveGame {
	
	bool quicksave;
	
	std::string name;
	
	fs::path savefile;
	res::path thumbnail;
	
	long level;
	std::time_t stime;
	
	std::string time;
	
	SaveGame()
		: quicksave(false)
		, level(0)
		, stime(0)
	{}
};

//! Central management of the list of savegames.
class SaveGameList {
	
public:
	
	typedef std::vector<SaveGame>::const_iterator iterator;
	
	//! Update the savegame list. This is automatically called by save() and remove()
	void update(bool verbose = false);
	
	/*! Save the current game state
	 * \param name The name of the new savegame.
	 * \param overwrite A savegame to overwrite with this save or end()
	 * \return true if the game was successfully saved.
	 */
	bool save(const std::string & name, iterator overwrite, const Image & thumbnail = Image());
	
	/*! Save the current game state
	 * \param name The name of the new savegame.
	 * \param overwrite A savegame to overwrite with this save or end()
	 * \return true if the game was successfully saved.
	 */
	bool save(const std::string & name, size_t overwrite = size_t(-1), const Image & th = Image()) {
		return save(name, (overwrite == size_t(-1)) ? end() : begin() + overwrite, th);
	}
	
	//! Perform a quicksave: Maintain a number of quicksave slots and always overwrite the oldest one.
	bool quicksave(const Image & thumbnail = Image());
	
	//! Return the newest savegame or end() if there is no savegame.
	iterator quickload();
	
	//! Delete the given savegame. This removes the actual on-disk files.
	void remove(SavegameHandle handle);
	
	iterator begin() const { return savelist.begin(); }
	iterator end() const { return savelist.end(); }
	
	size_t size() const { return savelist.size(); }
	const SaveGame & operator[](size_t index) const { return savelist[index]; }
	
private:
	
	std::vector<SaveGame> savelist;
	
};

extern SaveGameList savegames;

#endif // ARX_CORE_SAVEGAME_H
