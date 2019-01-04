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

#include "gui/MenuWidgets.h"

#include <cctype>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iosfwd>
#include <limits>
#include <algorithm>
#include <sstream>

#include <boost/foreach.hpp>

#include "core/Application.h"
#include "core/ArxGame.h"
#include "core/Benchmark.h"
#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"
#include "core/Localisation.h"
#include "core/SaveGame.h"
#include "core/Version.h"

#include "gui/Hud.h"
#include "gui/LoadLevelScreen.h"
#include "gui/MainMenu.h"
#include "gui/Menu.h"
#include "gui/MenuPublic.h"
#include "gui/Text.h"
#include "gui/Interface.h"
#include "gui/Credits.h"
#include "gui/TextManager.h"
#include "gui/menu/MenuCursor.h"
#include "gui/menu/MenuFader.h"
#include "gui/widget/CycleTextWidget.h"
#include "gui/widget/KeybindWidget.h"
#include "gui/widget/PanelWidget.h"
#include "gui/widget/TextInputWidget.h"
#include "gui/widget/TextWidget.h"

#include "graphics/Draw.h"
#include "graphics/DrawLine.h"
#include "graphics/Math.h"
#include "graphics/Renderer.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/font/Font.h"
#include "graphics/texture/TextureStage.h"

#include "input/Input.h"

#include "scene/ChangeLevel.h"
#include "scene/GameSound.h"
#include "scene/LoadLevel.h"

#include "window/RenderWindow.h"

void ARX_QuickSave() {
	
	if(!g_canResumeGame) {
		return;
	}
	
	ARX_SOUND_MixerPause(ARX_SOUND_MixerGame);
	
	savegames.quicksave(savegame_thumbnail);
	
	ARX_SOUND_MixerResume(ARX_SOUND_MixerGame);
}

void ARX_LoadGame(const SaveGame & save) {
	
	ARXmenu.requestMode(Mode_InGame);
	
	ARX_SOUND_MixerPause(ARX_SOUND_MixerMenu);
	ARX_SOUND_MixerPause(ARX_SOUND_MixerGame);
	
	benchmark::begin(benchmark::LoadLevel);
	
	LoadLevelScreen();
	progressBarSetTotal(238);
	progressBarReset();
	progressBarAdvance();
	LoadLevelScreen(save.level);
	
	ARX_CHANGELEVEL_Load(save.savefile);
	
	g_canResumeGame = true;
}

void ARX_QuickLoad() {
	
	SavegameHandle save = savegames.quickload();
	if(save == SavegameHandle()) {
		// No saves found!
		return;
	}
	
	ARX_LoadGame(savegames[save]);
}

bool MENU_NoActiveWindow() {
	return (!g_mainMenu->m_window || g_mainMenu->m_window->currentPageId() == Page_None);
}

void MainMenuDoFrame() {
	
	UseRenderState state(render2D());
	UseTextureState textureState(TextureStage::FilterLinear, TextureStage::WrapClamp);
	
	if(!g_mainMenu || g_mainMenu->bReInitAll) {
		
		MENUSTATE page = Page_None;
		float scroll = 0.f;
		
		if(g_mainMenu) {
			page = g_mainMenu->requestedPage();
			if(g_mainMenu->m_window) {
				scroll = g_mainMenu->m_window->scroll();
			}
			delete g_mainMenu, g_mainMenu = NULL;
		}
		
		g_mainMenu = new MainMenu();
		g_mainMenu->init();
		
		g_mainMenu->requestPage(page);
		if(page != Page_None) {
			g_mainMenu->initWindowPages();
			g_mainMenu->m_window->setScroll(scroll);
		}
		
	}
	
	if(pMenuCursor == NULL) {
		pMenuCursor = new MenuCursor();
	}
	pMenuCursor->update();
	
	g_mainMenu->update();
	
	g_mainMenu->render();
	
	pMenuCursor->DrawCursor();
	
	g_thumbnailCursor.render();
	
	if(MenuFader_process()) {
		switch(iFadeAction) {
			case Mode_Credits:
				ARX_MENU_Clicked_CREDITS();
				MenuFader_start(Fade_Out, -1);
				break;
			case Mode_CharacterCreation:
				ARX_MENU_Clicked_NEWQUEST();
				MenuFader_start(Fade_Out, -1);
				cinematicBorder.reset();
				break;
			case Mode_InGame:
				mainApp->quit();
				MenuFader_start(Fade_Out, -1);
				break;
			default: break;
		}
	}
	
}

MenuWindow::MenuWindow()
	: m_pos(RATIO_2(Vec2f(20, 25)))
	, m_size(RATIO_2(Vec2f(321, 430)))
	, m_initalOffsetX(-m_size.x)
	, m_fadeDistance(m_size.x + m_pos.x)
	, fAngle(0.f)
	, m_currentPage(NULL)
	, m_background(TextureContainer::LoadUI("graph/interface/menus/menu_console_background"))
	, m_border(TextureContainer::LoadUI("graph/interface/menus/menu_console_background_border"))
{ }

MenuWindow::~MenuWindow() {
	for(std::vector<MenuPage *>::iterator it = m_pages.begin(), it_end = m_pages.end(); it < it_end; ++it) {
		delete *it;
	}
}

void MenuWindow::add(MenuPage * page) {
	page->setSize(RATIO_2(Vec2f(292, 395)));
	m_pages.push_back(page);
}

void MenuWindow::update() {
	
	m_pos.x = m_initalOffsetX + (m_fadeDistance * glm::sin(glm::radians(fAngle)));
	
	fAngle = std::min(fAngle + g_platformTime.lastFrameDuration() / PlatformDurationMsf(12.5f), 90.f);
	
	if(m_currentPage) {
		m_currentPage->update(m_pos + Vec2f(RATIO_X(14.5f), RATIO_Y(12.f)));
	}
	
}

void MenuWindow::render() {
	
	if(!m_currentPage) {
		return;
	}
	
	// Draw backgound and border
	{
		UseRenderState state(render2D().blend(BlendZero, BlendInvSrcColor));
		EERIEDrawBitmap(Rectf(m_pos, RATIO_X(m_background->m_size.x), RATIO_Y(m_background->m_size.y)),
		                0, m_background, Color::white);
	}
	
	EERIEDrawBitmap(Rectf(m_pos, RATIO_X(m_border->m_size.x), RATIO_Y(m_border->m_size.y)),
	                0, m_border, Color::white);
	
	m_currentPage->render();
	
	if(g_debugInfo == InfoPanelGuiDebug) {
		m_currentPage->drawDebug();
	}
	
}

void MenuWindow::setCurrentPage(MENUSTATE id) {
	
	if(currentPageId() == id) {
		return;
	}
	
	if(m_currentPage) {
		m_currentPage->unfocus();
	}
	
	m_currentPage = getPage(id);
	
	if(m_currentPage) {
		m_currentPage->focus();
	}
	
}

MenuPage * MenuWindow::getPage(MENUSTATE id) const {
	
	BOOST_FOREACH(MenuPage * page, m_pages) {
		if(page->id() == id) {
			return page;
		}
	}
	
	return NULL;
}

void Menu2_Close() {
	
	delete g_mainMenu, g_mainMenu = NULL;
	delete pMenuCursor, pMenuCursor = NULL;
}

void MenuReInitAll() {
	
	if(!g_mainMenu) {
		return;
	}
	
	g_mainMenu->bReInitAll = true;
}
