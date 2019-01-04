/*
 * Copyright 2017-2018 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GUI_CHARACTERCREATION_H
#define ARX_GUI_CHARACTERCREATION_H

#include <string>

#include "platform/Platform.h"

class TextureContainer;

struct CharacterCreation {
	
	CharacterCreation();
	
	void loadData();
	void freeData();
	
	void resetCheat();
	void render();

private:
	TextureContainer * BookBackground;
	std::string str_button_quickgen;
	std::string str_button_skin;
	std::string str_button_done;
	
	std::string m_desc_quickgen;
	std::string m_desc_skin;
	std::string m_desc_done;
	
	s8 m_cheatSkinButtonClickCount;
	char m_cheatQuickGenButtonClickCount;
};

extern CharacterCreation g_characterCreation;

#endif // ARX_GUI_CHARACTERCREATION_H
