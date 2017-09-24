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
	void manageLeftTabsCommon(long tabNum, long & activeTab);
	static void manageLeftTabOneCommon(long tabNum, long & activeTab, Vec2f pos, Vec2f activePos);
private:
	Vec2f m_activeTabPositions[10] = { Vec2f(102.f, 82.f), Vec2f(98.f, 112.f), Vec2f(101.f, 141.f), Vec2f(100.f, 170.f), Vec2f(97.f, 199.f),
	                                 Vec2f(103.f, 226.f), Vec2f(101.f, 255.f), Vec2f(99.f, 283.f), Vec2f(99.f, 307.f), Vec2f(104.f, 331.f) };
	Vec2f m_tabPositions[10] = { Vec2f(100.f, 82.f), Vec2f(98.f, 112.f), Vec2f(97.f, 143.f), Vec2f(95.f, 170.f), Vec2f(95.f, 200.f),
	                           Vec2f(94.f, 229.f), Vec2f(94.f, 259.f), Vec2f(92.f, 282.f), Vec2f(90.f, 308.f), Vec2f(97.f, 331.f) };
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
	void clear();
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
	void openPrevPage();
	ARX_INTERFACE_BOOK_MODE currentPage() { return m_currentPage; }
	void forcePage(ARX_INTERFACE_BOOK_MODE page);
	void open();
	void close();
	void toggle();

	void clearJournal();
private:
	bool canOpenPage(ARX_INTERFACE_BOOK_MODE page);
	ARX_INTERFACE_BOOK_MODE nextPage();
	ARX_INTERFACE_BOOK_MODE prevPage();
	void onClosePage();
	void drawTopTabs();
};

extern PlayerBook g_playerBook;

extern long BOOKZOOM;

#endif // ARX_GUI_BOOK_BOOK_H
