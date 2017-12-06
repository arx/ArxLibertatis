/*
 * Copyright 2013-2014 Arx Libertatis Team (see the AUTHORS file)
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

#include "savetool/SaveRename.h"

#include <iostream>

#include "io/SaveBlock.h"
#include "scene/SaveFormat.h"
#include "util/String.h"

class MappedPld {
	
public:
	
	explicit MappedPld(SaveBlock & save)
		: m_save(save)
		, m_fileName("pld")
		, m_data(NULL)
		, m_size(0)
		, m_pld(NULL)
	{}
	
	~MappedPld() {
		free(m_data);
	}
	
	bool load() {
		
		m_data = m_save.load(m_fileName, m_size);
		if(!m_data) {
			std::cerr << m_fileName << " not found" << std::endl;
			return false;
		}
		
		size_t pos = 0;
		m_pld = reinterpret_cast<ARX_CHANGELEVEL_PLAYER_LEVEL_DATA *>(m_data + pos);
		pos += sizeof(ARX_CHANGELEVEL_PLAYER_LEVEL_DATA);
		
		if(m_pld->version != ARX_GAMESAVE_VERSION) {
			std::cout << "bad version: " << m_pld->version << std::endl;
			return false;
		}
		
		arx_assert(m_size >= pos);
		ARX_UNUSED(pos);
		
		return true;
	}
	
	void save() {
		m_save.save(m_fileName, m_data, m_size);
		
		m_save.flush(m_fileName);
	}
	
	std::string getName() {
		return util::loadString(m_pld->name);
	}
	
	void setName(const std::string & name) {
		util::storeString(m_pld->name, name);
	}
	
private:
	SaveBlock & m_save;
	std::string m_fileName;
	
	char * m_data;
	size_t m_size;
	
	ARX_CHANGELEVEL_PLAYER_LEVEL_DATA * m_pld;
};

int main_rename(SaveBlock & save, const std::vector<std::string> & args) {
	
	if(args.size() != 1) {
		return -1;
	}
	
	if(!save.open(true)) {
		std::cerr << "failed to open savefile" << std::endl;
		return 2;
	}
	
	MappedPld pld(save);
	
	if(!pld.load()) {
		std::cerr << "failed to load pld data" << std::endl;
		return 3;
	}
	
	std::string oldName = pld.getName();
	const std::string & newName = args[0];
	
	std::cout << "Old Name: \"" << oldName << '"' << std::endl;
	std::cout << "New Name: \"" << newName << '"' << std::endl;
	
	pld.setName(newName);
	pld.save();
	
	return 0;
}
