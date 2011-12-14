
#ifndef ARX_CORE_SAVEGAME_H
#define ARX_CORE_SAVEGAME_H

#include <stddef.h>
#include <vector>
#include <string>
#include <ctime>

#include "io/fs/FilePath.h"
#include "io/resource/ResourcePath.h"

struct SaveGame {
	
	bool quicksave;
	
	std::string name;
	
	fs::path savefile;
	res::path thumbnail;
	
	long level;
	std::time_t stime;
	
	std::string time;
	
	SaveGame() : level(0) { }
};

//! Central management of the list of savegames.
class SaveGameList {
	
public:
	
	typedef std::vector<SaveGame>::const_iterator iterator;
	
	//! Update the savegame list. This is automatically called by save() and remove()
	void update();
	
	/*! Save the current game state
	 * @param name The name of the new savegame.
	 * @param overwrite A savegame to overwrite with this save or end()
	 * @return true if the game was successfully saved.
	 */
	bool save(const std::string & name, iterator overwrite);
	
	/*! Save the current game state
	 * @param name The name of the new savegame.
	 * @param overwrite A savegame to overwrite with this save or end()
	 * @return true if the game was successfully saved.
	 */
	bool save(const std::string & name, size_t overwrite = size_t(-1)) {
		return save(name, (overwrite == size_t(-1)) ? end() : begin() + overwrite);
	}
	
	//! Perform a quicksave: Maintain a number of quicksave slots and always overwrite the oldest one.
	bool quicksave();
	
	//! Return the newest savegame or end() if there is no savegame.
	iterator quickload();
	
	//! Delete the given savegame. This removes the actual on-disk files.
	void remove(iterator idx);
	
	//! Delete the given savegame. This removes the actual on-disk files.
	void remove(size_t idx) { remove(begin() + idx); }
	
	iterator begin() const { return savelist.begin(); }
	iterator end() const { return savelist.end(); }
	
	size_t size() const { return savelist.size(); }
	const SaveGame & operator[](size_t index) const { return savelist[index]; }
	
private:
	
	std::vector<SaveGame> savelist;
	
};

extern SaveGameList savegames;

#endif // ARX_CORE_SAVEGAME_H
