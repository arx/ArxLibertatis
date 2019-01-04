/*
 * Copyright 2015-2018 Arx Libertatis Team (see the AUTHORS file)
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
#include "gui/Menu.h"
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
	void manageLeftTabs(long tabNum, long & activeTab);
	
private:
	
	static const Vec2f m_activeTabOffsets[10];
	static const Vec2f m_tabOffsets[10];
	
	void drawTab(long tabNum);
	void drawActiveTab(long tabNum);
	void checkTabClick(long tabNum, long & activeTab);
	
};

static const size_t MAX_FLYOVER = 32;

class StatsPage : public PlayerBookPage {
public:
	void loadStrings();
	
	void manage();
	void manageNewQuest();
private:
	void manageStats();
	void RenderBookPlayerCharacter();
	bool CheckAttributeClick(Vec2f pos, float * val, TextureContainer * tc);
	bool CheckSkillClick(Vec2f pos, float * val, TextureContainer * tc, float oldval);
	Color attributeModToColor(float modValue, float baseValue = 0.f);
	
	std::string flyover[MAX_FLYOVER];
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
	void update();
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
	
	float getScale();
	const Rectf & getArea();
	
private:
	
	Vec2f lastRatio;
	float lastHudScale;
	float lastScaleSetting;
	MenuMode lastMenuMode;
	
	bool canOpenPage(ARX_INTERFACE_BOOK_MODE page);
	ARX_INTERFACE_BOOK_MODE nextPage();
	ARX_INTERFACE_BOOK_MODE prevPage();
	void onClosePage();
	void manageTopTabs();
	
	bool needsUpdate();
	void updateRect();
	void updateScale();
	
};

extern PlayerBook g_playerBook;

#endif // ARX_GUI_BOOK_BOOK_H
