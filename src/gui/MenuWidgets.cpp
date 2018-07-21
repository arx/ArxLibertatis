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
	
	g_mainMenu->Update();
	
	g_mainMenu->Render();
	
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
	: m_currentPage(NULL)
{
	
	Vec2f windowMenuPos = Vec2f(20, 25);
	Vec2f size = Vec2f(321, 430);
	
	m_pos = RATIO_2(windowMenuPos);
	m_size = RATIO_2(size);

	m_pages.clear();

	m_initalOffsetX = -m_size.x;
	m_fadeDistance = m_size.x + m_pos.x;
	
	fAngle = 0.f;
	
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
		m_currentPage->Update(m_pos + Vec2f(RATIO_X(14.5f), RATIO_Y(12.f)));
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

MenuPage::MenuPage(MENUSTATE id)
	: m_rowSpacing(10)
	, m_id(id)
	, m_initialized(false)
	, m_selected(NULL)
	, m_focused(NULL)
	, m_disableShortcuts(false)
{
	Vec2f scaledSize = RATIO_2(Vec2f(292, 395));
	m_content = m_rect = Rectf(Vec2f_ZERO, scaledSize.x, scaledSize.y);
}

MenuPage::~MenuPage() { }

void MenuPage::addCorner(Widget * widget, Anchor anchor) {
	
	Vec2f pos = Vec2f_ZERO;
	
	switch(anchor) {
		case TopLeft:
		case BottomLeft: {
			pos.x = m_rect.left;
			break;
		}
		case TopCenter:
		case BottomCenter: {
			pos.x = m_rect.center().x - widget->m_rect.width() / 2;
			break;
		}
		case TopRight:
		case BottomRight: {
			pos.x = m_rect.right - widget->m_rect.width();
			break;
		}
	}
	
	switch(anchor) {
		case TopLeft:
		case TopCenter:
		case TopRight: {
			pos.y = m_rect.top;
			break;
		}
		case BottomLeft:
		case BottomCenter:
		case BottomRight: {
			pos.y = m_rect.bottom - widget->m_rect.height();
			break;
		}
	}
	
	widget->SetPos(pos);
	
	m_children.add(widget);
	
}

void MenuPage::addCenter(Widget * widget, bool centerX) {
	
	float x = m_content.left;
	if(centerX) {
		x = std::floor(m_content.center().x - widget->m_rect.width() / 2.f);
	}
	
	float height = widget->m_rect.height();
	float whitespace = 0.f;
	BOOST_FOREACH(Widget * w, m_children.m_widgets) {
		height += RATIO_Y(m_rowSpacing);
		height += w->m_rect.height();
		whitespace += RATIO_Y(m_rowSpacing);
		if(w->type() == WidgetType_Spacer) {
			whitespace += w->m_rect.height();
		}
	}
	
	float squish = 1.f;
	if(height > m_content.height() && whitespace > 0.f) {
		height -= whitespace;
		squish = std::max(0.f, m_content.height() - height) / whitespace;
		height += whitespace * squish;
	}
	
	float y = std::floor(m_content.center().y - height / 2.f);
	
	BOOST_FOREACH(Widget * w, m_children.m_widgets) {
		w->SetPos(Vec2f(w->m_rect.left, y));
		y += w->m_rect.height() * ((w->type() == WidgetType_Spacer) ? squish : 1.f);
		y += RATIO_Y(m_rowSpacing) * squish;
	}
	
	widget->SetPos(Vec2f(x, y));
	
	m_children.add(widget);
	
}

void MenuPage::Update(Vec2f pos) {
	
	m_children.Move(pos - m_rect.topLeft());
	
	m_content.move(pos - m_rect.topLeft());
	
	m_rect.moveTo(pos);
	
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
	drawLineRectangle(m_rect, 0.f, Color::green);
	drawLineRectangle(m_content, 0.f, Color::blue);
	m_children.drawDebug();
}


void MenuPage::reserveTop() {
	m_content.top = m_rect.top + hFontMenu->getLineHeight() + RATIO_Y(5);
}

void MenuPage::reserveBottom() {
	m_content.bottom = m_rect.bottom - hFontMenu->getLineHeight() - RATIO_Y(5);
}

Vec2f MenuPage::buttonSize(float x, float y) const {
	return RATIO_2(Vec2f(x, y));
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
