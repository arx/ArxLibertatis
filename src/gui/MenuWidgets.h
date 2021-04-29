/*
 * Copyright 2011-2016 Arx Libertatis Team (see the AUTHORS file)
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
/* Based on:
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/

#ifndef ARX_GUI_MENUWIDGETS_H
#define ARX_GUI_MENUWIDGETS_H

#include <vector>
#include <string>

#include <boost/noncopyable.hpp>

#include "core/TimeTypes.h"
#include "graphics/Color.h"
#include "gui/widget/ButtonWidget.h"
#include "gui/widget/TextWidget.h"
#include "gui/widget/Widget.h"
#include "gui/widget/WidgetContainer.h"
#include "input/InputKey.h"
#include "math/Vector.h"
#include "math/Rectangle.h"
#include "util/HandleType.h"

class TextureContainer;
class Font;
struct SaveGame;

class MenuPage : private boost::noncopyable {
	
public:
	
	explicit MenuPage(MENUSTATE state);
	virtual ~MenuPage();
	
	void add(Widget * widget);
	void addCenter(Widget * widget, bool centerX = false);
	void AlignElementCenter(Widget * widget);
	void Update(Vec2f pos);
	void Render();
	void drawDebug();
	
	TextWidget * GetTouch(bool keyTouched, int keyId, InputKeyId * pInputKeyId, bool _bValidateTest);
	void ReInitActionKey();
	
	Vec2f m_pos;
	Vec2f m_oldPos;
	int m_rowSpacing;
	MENUSTATE eMenuState;
	WidgetContainer m_children;
	
protected:
	Rectf m_rect;
	Vec2f m_size;
	Widget * m_selected;
	bool bMouseAttack;
	
private:
	void updateTextRect(TextWidget * widget);
	void UpdateText();
	
	bool bEdit;
	bool m_disableShortcuts;
	PlatformDuration m_blinkTime;
	bool m_blink;
};

class LoadMenuPage;
class SaveConfirmMenuPage;

class MenuWindow : private boost::noncopyable {
	
private:
	
	Vec2f m_pos;
	Vec2f m_size;
	float m_initalOffsetX;
	float m_fadeDistance;
	
public:
	
	MenuWindow();
	virtual ~MenuWindow();
	
	void add(MenuPage * page);
	void Update();
	void Render();
	
	std::vector<MenuPage *> m_pages;
	LoadMenuPage * m_pageLoad;
	SaveConfirmMenuPage * m_pageSaveConfirm;
	
	float fAngle;
	
	MENUSTATE currentPageId() {
		return m_currentPageId;
	}
	
	void setCurrentPageId(MENUSTATE id);
	
	void requestPage(MENUSTATE page) {
		m_requestedPage = page;
	}
	
private:
	
	MENUSTATE m_requestedPage;
	MENUSTATE m_currentPageId;
	MenuPage * m_currentPage;
	
	TextureContainer * m_background;
	TextureContainer * m_border;
	
};

void MenuReInitAll();

void MainMenuDoFrame();
void Menu2_Close();



void ARX_MENU_Clicked_QUIT();

void ARX_LoadGame(const SaveGame & save);
void ARX_QuickLoad();
void ARX_QuickSave();

bool MENU_NoActiveWindow();

extern bool bNoMenu;

#endif // ARX_GUI_MENUWIDGETS_H
