/*
 * Copyright 2015 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GUI_BOOK_BOOK_H
#define ARX_GUI_BOOK_BOOK_H

#include "graphics/Color.h"
#include "gui/Note.h"

enum ARX_INTERFACE_BOOK_MODE
{
	BOOKMODE_STATS = 0,
	BOOKMODE_SPELLS,
	BOOKMODE_MINIMAP,
	BOOKMODE_QUESTS
};

class PlayerBookPage {
public:
	void playReleaseSound();
	void playErrorSound();
	void manageLeftTabsCommon(bool tabVisibility[10], long & activeTab);
	static void manageLeftTabOneCommon(bool tabVisibility[10], long & activeTab, int t, Vec2f pos, Vec2f activePos);
};

class StatsPage : public PlayerBookPage {
public:
	void manage();
	void manageNewQuest();
private:
	void manageStats();
	void RenderBookPlayerCharacter();
	bool CheckAttributeClick(Vec2f pos, float * val, TextureContainer * tc);
	bool CheckSkillClick(Vec2f pos, float * val, TextureContainer * tc, float * oldval);
	Color attributeModToColor(float modValue, float baseValue = 0.f);
};

class SpellsPage : public PlayerBookPage {
public:
	SpellsPage();
	void manage();
private:
	long m_currentTab;

	void drawLeftTabs();
	void drawSpells();
};

class MapPage : public PlayerBookPage {
public:
	MapPage();
	void manage();
	void setMapLevel(long level);
private:
	long m_currentLevel;

	void drawLeftTabs();
	void drawMaps();
};

class QuestBookPage : public PlayerBookPage {
public:
	void manage();
	void update();
private:
	Note m_questBook;
};

class PlayerBook {
public:
	StatsPage stats;
	SpellsPage spells;
	MapPage map;
	QuestBookPage questBook;

	ARX_INTERFACE_BOOK_MODE m_currentPage;

	PlayerBook();
	void manage();
	void openPage(ARX_INTERFACE_BOOK_MODE newPage, bool toggle = false);
	void openNextPage();
	ARX_INTERFACE_BOOK_MODE prevPage();
	ARX_INTERFACE_BOOK_MODE currentPage() { return m_currentPage; }
	void forcePage(ARX_INTERFACE_BOOK_MODE page);
	void open();
	void close();
	void toggle();
private:
	bool canOpenPage(ARX_INTERFACE_BOOK_MODE page);
	ARX_INTERFACE_BOOK_MODE nextPage();
	void onClosePage();
	void drawTopTabs();
};

extern PlayerBook g_playerBook;

extern long BOOKZOOM;

ARX_INTERFACE_BOOK_MODE prevBookPage();

void updateQuestBook();

#endif // ARX_GUI_BOOK_BOOK_H
