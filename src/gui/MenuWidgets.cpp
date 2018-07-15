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

extern int newWidth;
extern int newHeight;
extern bool newFullscreen;

extern TextWidget * pMenuElementApply;

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

static void Check_Apply() {
	if(pMenuElementApply) {
		bool enable = config.video.resolution.x != newWidth
		              || config.video.resolution.y != newHeight
		              || config.video.fullscreen != newFullscreen;
		pMenuElementApply->setEnabled(enable);
	}
}

void MainMenuDoFrame() {
	
	UseRenderState state(render2D());
	UseTextureState textureState(TextureStage::FilterLinear, TextureStage::WrapClamp);
	
	if(!g_mainMenu || g_mainMenu->bReInitAll) {
		
		MENUSTATE page = Page_None;
		float scroll = 0.f;
		
		if(g_mainMenu) {
			if(g_mainMenu->m_window) {
				page = g_mainMenu->m_window->currentPageId();
				scroll = g_mainMenu->m_window->scroll();
			}
			delete g_mainMenu, g_mainMenu = NULL;
		}
		
		g_mainMenu = new MainMenu();
		g_mainMenu->init();
		
		if(page != Page_None) {
			g_mainMenu->initWindowPages();
			g_mainMenu->m_window->setCurrentPageId(page);
			g_mainMenu->m_window->setScroll(scroll);
		}
		
	}
	
	if(pMenuCursor == NULL) {
		pMenuCursor = new MenuCursor();
	}
	pMenuCursor->update();
	
	g_mainMenu->Update();
	
	g_mainMenu->Render();
	
	if(g_mainMenu->m_window && g_mainMenu->m_window->currentPageId() != Page_None) {
		Check_Apply();
	}
	
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


MenuWindow::MenuWindow() {
	
	Vec2f windowMenuPos = Vec2f(20, 25);
	Vec2f size = Vec2f(321, 430);
	
	m_pos = RATIO_2(windowMenuPos);
	m_size = RATIO_2(size);

	m_pages.clear();

	m_initalOffsetX = -m_size.x;
	m_fadeDistance = m_size.x + m_pos.x;
	
	m_pageSaveConfirm = NULL;
	fAngle = 0.f;
	
	m_currentPageId = Page_None;
	m_currentPage = NULL;
	
	m_background = TextureContainer::LoadUI("graph/interface/menus/menu_console_background");
	m_border = TextureContainer::LoadUI("graph/interface/menus/menu_console_background_border");
	
}

MenuWindow::~MenuWindow() {
	for(std::vector<MenuPage *>::iterator it = m_pages.begin(), it_end = m_pages.end(); it < it_end; ++it) {
		delete *it;
	}
}

void MenuWindow::add(MenuPage * page) {
	m_pages.push_back(page);
}

void MenuWindow::Update() {
	
	m_pos.x = m_initalOffsetX + (m_fadeDistance * glm::sin(glm::radians(fAngle)));
	
	fAngle = std::min(fAngle + g_platformTime.lastFrameDuration() / PlatformDurationMsf(12.5f), 90.f);
	
	if(m_currentPage) {
		m_currentPage->Update(m_pos);
	}
	
}

void MenuWindow::Render() {
	
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
	
	m_currentPage->Render();
	
	if(g_debugInfo == InfoPanelGuiDebug) {
		m_currentPage->drawDebug();
	}
	
}

void MenuWindow::setCurrentPageId(MENUSTATE id) {
	
	if(m_currentPageId == id) {
		return;
	}
	
	m_currentPageId = id;
	
	if(m_currentPage) {
		m_currentPage->unfocus();
	}
	
	m_currentPage = NULL;
	BOOST_FOREACH(MenuPage * page, m_pages) {
		if(id == page->eMenuState) {
			m_currentPage = page;
			m_currentPage->focus();
		}
	}
	
}

MenuPage::MenuPage(MENUSTATE _eMenuState)
	: m_rowSpacing(10)
	, m_initialized(false)
	, m_selected(NULL)
	, m_focused(NULL)
	, m_disableShortcuts(false)
{
	m_size = Vec2f(321, 430);
	
	Vec2f scaledSize = RATIO_2(m_size);
	m_rect = Rectf(Vec2f_ZERO, scaledSize.x, scaledSize.y);
	
	eMenuState = _eMenuState;
}

MenuPage::~MenuPage() { }

void MenuPage::add(Widget * widget) {
	widget->Move(m_rect.topLeft());
	m_children.add(widget);
}

void MenuPage::addCenter(Widget * widget, bool centerX) {
	
	int dx;
	if(centerX) {
		float iDx = widget->m_rect.width();
		dx  = int(((m_rect.width() - iDx) / 2) - widget->m_rect.left);
		if(dx < 0) {
			dx = 0;
		}
	} else {
		dx = 0;
	}
	
	float iDy = widget->m_rect.height();
	
	BOOST_FOREACH(Widget * w, m_children.m_widgets) {
		iDy += m_rowSpacing;
		iDy += w->m_rect.height();
	}

	int iDepY = int(m_rect.left);

	if(iDy < m_rect.height()) {
		iDepY += int((m_rect.height() - iDy) / 2);
	}

	int dy = 0;

	if(!m_children.m_widgets.empty()) {
		dy = int(iDepY - m_children.m_widgets[0]->m_rect.top);
	}
	
	BOOST_FOREACH(Widget * w, m_children.m_widgets) {
		iDepY += int(w->m_rect.height()) + m_rowSpacing;
		w->Move(Vec2f(0, dy));
	}
	
	widget->Move(Vec2f(dx, iDepY));

	m_children.add(widget);
}

void MenuPage::Update(Vec2f pos) {
	
	m_children.Move(pos - m_pos);
	
	m_pos = pos;
	
	if(m_focused && !m_focused->wantFocus()) {
		m_focused->unfocus();
		m_focused = NULL;
		m_disableShortcuts = true;
	}
	
	if(m_disableShortcuts) {
		
		bool isShortcutPressed = false;
		
		BOOST_FOREACH(Widget * w, m_children.m_widgets) {
			arx_assert(w);
			if(w->m_shortcut != ActionKey::UNUSED && GInput->isKeyPressed(w->m_shortcut)) {
				isShortcutPressed = true;
			}
		}
		
		if(!isShortcutPressed) {
			m_disableShortcuts = false;
		}
		
	} else {
		
		BOOST_FOREACH(Widget * w, m_children.m_widgets) {
			arx_assert(w);
			
			if(m_focused && w != m_focused) {
				continue;
			}
			
			if(w->m_shortcut != ActionKey::UNUSED && GInput->isKeyPressedNowUnPressed(w->m_shortcut)) {
				
				m_selected = w;
				
				if(w->click() && w != m_focused) {
					m_focused = w;
				}
				
				return;
			}
			
		}
		
	}
	
	m_selected = m_children.getAtPos(Vec2f(GInput->getMousePosition()));
	
	if(m_focused && m_selected != m_focused && GInput->getMouseButton(Mouse::Button_0)) {
		m_focused->unfocus();
		m_focused = NULL;
	}
	
	if(m_selected && (!m_focused || m_focused == m_selected)) {
		
		if(GInput->getMouseButtonDoubleClick(Mouse::Button_0)) {
			if(m_selected->doubleClick()) {
				m_focused = m_selected;
			}
			return;
		}
		
		if(GInput->getMouseButton(Mouse::Button_0)) {
			if(m_selected->click()) {
				m_focused = m_selected;
			}
			return;
		}
		
		m_selected->hover();
		
	}
	
}

void MenuPage::Render() {
	
	BOOST_FOREACH(Widget * w, m_children.m_widgets) {
		w->update();
		w->render(w == m_selected);
	}
	
	if(m_selected) {
		pMenuCursor->SetMouseOver();
	}
	
}

void MenuPage::drawDebug() {
	Rectf rect = Rectf(Vec2f(m_pos.x, m_pos.y), m_rect.width(), m_rect.height());
	drawLineRectangle(rect, 0.f, Color::green);
	
	m_children.drawDebug();
}

void MenuPage::focus() {
	
	if(!m_initialized) {
		m_initialized = true;
		init();
	}
	
}

void MenuPage::unfocus() {
	
	if(m_focused) {
		m_focused->unfocus();
		m_focused = NULL;
	}
	
}

void MenuPage::activate(Widget * widget) {
	
	if(m_focused != widget) {
		unfocus();
		if(widget->click()) {
			m_focused = widget;
		}
	}
	
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
