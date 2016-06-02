/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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
#include "gui/widget/CycleTextWidget.h"

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

// Imported global variables and functions
extern ARX_MENU_DATA ARXmenu;

extern bool REFUSE_GAME_RETURN;

bool bNoMenu=false;

MenuCursor * pMenuCursor = NULL;

extern CWindowMenu * pWindowMenu;
MainMenu *mainMenu;

extern TextWidget * pMenuElementApply;

float ARXTimeMenu;
float ARXDiffTimeMenu;

bool bFade=false;
bool bFadeInOut=false;
int iFadeAction=-1;
float fFadeInOut=0.f;

void ARX_QuickSave() {
	
	if(REFUSE_GAME_RETURN) {
		return;
	}
	
	ARX_SOUND_MixerPause(ARX_SOUND_MixerGame);
	
	savegames.quicksave(savegame_thumbnail);
	
	ARX_SOUND_MixerResume(ARX_SOUND_MixerGame);
}

static bool ARX_LoadGame(const SaveGame & save) {
	
	benchmark::begin(benchmark::LoadLevel);
	
	LoadLevelScreen();
	progressBarSetTotal(238);
	progressBarReset();
	progressBarAdvance();
	LoadLevelScreen(save.level);
	
	DanaeClearLevel();
	
	long ret = ARX_CHANGELEVEL_Load(save.savefile);
	
	REFUSE_GAME_RETURN = false;

	return ret != -1;
}

bool ARX_QuickLoad() {
	bool loaded;

	SaveGameList::iterator save = savegames.quickload();
	if(save == savegames.end()) {
		// No saves found!
		return false;
	}

	ARX_SOUND_MixerPause(ARX_SOUND_MixerGame);
	loaded = ARX_LoadGame(*save);
	ARX_SOUND_MixerResume(ARX_SOUND_MixerGame);
	
	return loaded;
}

bool ARX_SlotLoad(SavegameHandle slotIndex) {
	if(slotIndex.handleData() >= (int)savegames.size()) {
		// Invalid slot!
		return false;
	}

	ARX_SOUND_MixerPause(ARX_SOUND_MixerMenu);
	return ARX_LoadGame(savegames[slotIndex.handleData()]);
}

bool MENU_NoActiveWindow() {
	
	if(!pWindowMenu || pWindowMenu->m_currentPageId == MAIN) {
		return true;
	}
	
	return false;
}

static void Check_Apply() {
	if(pMenuElementApply) {
		if(config.video.resolution.x != newWidth
		   || config.video.resolution.y != newHeight
		   || config.video.fullscreen != newFullscreen
		) {
			pMenuElementApply->SetCheckOn();
			pMenuElementApply->lColor = pMenuElementApply->lOldColor;
		} else {
			if(pMenuElementApply->lColor != Color(127,127,127)) {
				pMenuElementApply->SetCheckOff();
				pMenuElementApply->lOldColor = pMenuElementApply->lColor;
				pMenuElementApply->lColor = Color(127,127,127);
			}
		}
	}
}

static void FadeInOut(float _fVal) {

	TexturedVertex d3dvertex[4];

	ColorRGBA iColor = Color::gray(_fVal).toRGB();
	d3dvertex[0].p.x=0;
	d3dvertex[0].p.y=0;
	d3dvertex[0].p.z=0.f;
	d3dvertex[0].rhw=1.f;
	d3dvertex[0].color=iColor;

	d3dvertex[1].p.x=static_cast<float>(g_size.width());
	d3dvertex[1].p.y=0;
	d3dvertex[1].p.z=0.f;
	d3dvertex[1].rhw=1.f;
	d3dvertex[1].color=iColor;

	d3dvertex[2].p.x=0;
	d3dvertex[2].p.y=static_cast<float>(g_size.height());
	d3dvertex[2].p.z=0.f;
	d3dvertex[2].rhw=1.f;
	d3dvertex[2].color=iColor;

	d3dvertex[3].p.x=static_cast<float>(g_size.width());
	d3dvertex[3].p.y=static_cast<float>(g_size.height());
	d3dvertex[3].p.z=0.f;
	d3dvertex[3].rhw=1.f;
	d3dvertex[3].color=iColor;

	GRenderer->ResetTexture(0);
	GRenderer->SetBlendFunc(BlendZero, BlendInvSrcColor);

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::DepthTest, false);
	GRenderer->SetCulling(CullNone);

	EERIEDRAWPRIM(Renderer::TriangleStrip, d3dvertex, 4, true);

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetRenderState(Renderer::DepthTest, true);
	GRenderer->SetCulling(CullCCW);
}

bool ProcessFadeInOut(bool _bFadeIn, float _fspeed) {

	FadeInOut(fFadeInOut);

	if(!bFade)
		return true;

	if(_bFadeIn) {
		fFadeInOut += _fspeed * ARXDiffTimeMenu * (1.f/100);

		if(fFadeInOut > 1.f) {
			fFadeInOut = 1.f;
			bFade = false;
		}
	} else {
		fFadeInOut -= _fspeed * ARXDiffTimeMenu * (1.f/100);

		if(fFadeInOut < 0.f) {
			fFadeInOut = 0.f;
			bFade = false;
		}
	}

	return false;
}

bool Menu2_Render() {
	
	arxtime.update(false);
	float time = arxtime.now_f();
	ARXDiffTimeMenu = time - ARXTimeMenu;
	ARXTimeMenu = time;
	
	// this means ArxTimeMenu is reset
	if(ARXDiffTimeMenu < 0) {
		ARXDiffTimeMenu = 0;
	}
	
	if(pMenuCursor == NULL) {
		pMenuCursor = new MenuCursor();
	}
	pMenuCursor->update(ARXDiffTimeMenu);
	
	GRenderer->GetTextureStage(0)->setMinFilter(TextureStage::FilterLinear);
	GRenderer->GetTextureStage(0)->setMagFilter(TextureStage::FilterLinear);
	
	if(AMCM_NEWQUEST == ARXmenu.currentmode || AMCM_CREDITS == ARXmenu.currentmode) {
		
		delete pWindowMenu, pWindowMenu = NULL;
		delete mainMenu, mainMenu = NULL;
		
		if(ARXmenu.currentmode == AMCM_CREDITS){
			credits::render();
			return true;
		}
		
		return false;
	}
	
	if(pTextManage) {
		pTextManage->Clear();
	}

	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapClamp);

	GRenderer->SetRenderState(Renderer::Fog, false);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::DepthTest, false);
	GRenderer->SetCulling(CullNone);

	MENUSTATE eOldMenuState=NOP;
	MENUSTATE eM;

	if(!mainMenu) {
		eM = NOP;
	} else {
		eM = mainMenu->eOldMenuWindowState;
	}
	
	if(!mainMenu || mainMenu->bReInitAll) {
		
		if(mainMenu && mainMenu->bReInitAll) {
			eOldMenuState = mainMenu->eOldMenuState;
			delete pWindowMenu, pWindowMenu = NULL;
			delete mainMenu, mainMenu = NULL;
		}
		
		mainMenu = new MainMenu();
		mainMenu->eOldMenuWindowState=eM;
		
		mainMenu->init();
	}

	bool bScroll=true;
	
	MENUSTATE eMenuState = mainMenu->Update();
	
	if(eOldMenuState != NOP) {
		eMenuState=eOldMenuState;
		bScroll=false;
	}
	
	if(eMenuState == RESUME_GAME) {
		pTextManage->Clear();
		ARXmenu.currentmode = AMCM_OFF;
		mainMenu->m_selected = NULL;
		
		delete pWindowMenu, pWindowMenu = NULL;
		delete mainMenu, mainMenu = NULL;
		
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);
		GRenderer->SetRenderState(Renderer::DepthWrite, true);
		GRenderer->SetRenderState(Renderer::DepthTest, true);
		
		return true;
	} else if(eMenuState != NOP) {
		MainMenuLeftCreate(eMenuState);
	
	}
	
	mainMenu->Render();
	
	if(pWindowMenu)
	if(   (pWindowMenu->m_currentPageId != MAIN && pWindowMenu->m_currentPageId != NEW_QUEST && pWindowMenu->m_currentPageId != CREDITS)
	   || (pWindowMenu->m_currentPageId == NEW_QUEST && ARXMenu_CanResumeGame())
	) {
		if(!bScroll) {
			pWindowMenu->fAngle=90.f;
			pWindowMenu->m_currentPageId=mainMenu->eOldMenuWindowState;
		}

		pWindowMenu->Update(ARXDiffTimeMenu);
		MENUSTATE eMS = pWindowMenu->Render();
		if(eMS != NOP) {
			mainMenu->eOldMenuWindowState=eMS;
		}
		Check_Apply();
	}

	bNoMenu=false;

	// If the menu needs to be reinitialized, then the text in the TextManager is probably using bad fonts that were deleted already
	// Skip one update in this case
	if(pTextManage && !mainMenu->bReInitAll) {
		pTextManage->Update(ARXDiffTimeMenu);
		pTextManage->Render();
	}

	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapClamp);

	GRenderer->SetRenderState(Renderer::Fog, false);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetCulling(CullNone);
	pMenuCursor->DrawCursor();
	
	g_thumbnailCursor.render();
	
	if(ProcessFadeInOut(bFadeInOut,0.1f)) {
		switch(iFadeAction) {
			case AMCM_CREDITS:
				ARX_MENU_Clicked_CREDITS();
				iFadeAction=-1;
				bFadeInOut=false;
				bFade=true;
				break;
			case AMCM_NEWQUEST:
				ARX_MENU_Clicked_NEWQUEST();
				iFadeAction=-1;
				bFadeInOut=false;
				bFade=true;
				cinematicBorder.reset();
				break;
			case AMCM_OFF:
				ARX_MENU_Clicked_QUIT_GAME();
				iFadeAction=-1;
				bFadeInOut=false;
				bFade=true;
				break;
		}
	}

	GRenderer->GetTextureStage(0)->setMinFilter(TextureStage::FilterLinear);
	GRenderer->GetTextureStage(0)->setMagFilter(TextureStage::FilterLinear);
	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetRenderState(Renderer::DepthTest, true);
	GRenderer->SetCulling(CullCCW);

	return true;
}








CWindowMenu::CWindowMenu(const Vec2f & pos, const Vec2f & size)
{
	m_pos = RATIO_2(pos);
	m_size = RATIO_2(size);

	m_pages.clear();

	fPosXCalc = -m_size.x;
	fDist = m_size.x + m_pos.x;
	fAngle=0.f;

	m_currentPageId=NOP;


	float fCalc	= fPosXCalc + (fDist * glm::sin(glm::radians(fAngle)));

	m_pos.x = checked_range_cast<int>(fCalc);
	
	m_background = TextureContainer::LoadUI("graph/interface/menus/menu_console_background");
	m_border = TextureContainer::LoadUI("graph/interface/menus/menu_console_background_border");
}

CWindowMenu::~CWindowMenu() {

	for(std::vector<MenuPage*>::iterator it = m_pages.begin(), it_end = m_pages.end(); it < it_end; ++it)
		delete *it;
}

void CWindowMenu::add(MenuPage *page) {

	m_pages.push_back(page);
	page->m_oldPos.x = 0;
	page->m_oldPos.y = 0;
	page->m_pos = m_pos;
}

void CWindowMenu::Update(float _fDTime) {

	float fCalc	= fPosXCalc + (fDist * glm::sin(glm::radians(fAngle)));

	m_pos.x = checked_range_cast<int>(fCalc);
	fAngle += _fDTime * 0.08f;

	if(fAngle > 90.f)
		fAngle = 90.f;
}

MENUSTATE CWindowMenu::Render() {
	
	if(bNoMenu)
		return NOP;
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetBlendFunc(BlendOne, BlendOne);
	
	MENUSTATE eMS=NOP;
	
	BOOST_FOREACH(MenuPage * page, m_pages) {
		if(m_currentPageId == page->eMenuState) {
			eMS = page->Update(m_pos);
			
			if(eMS != NOP)
				break;
		}
	}
	
	// Draw backgound and border
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(BlendZero, BlendInvSrcColor);

	EERIEDrawBitmap2(Rectf(Vec2f(m_pos.x, m_pos.y),
	                 RATIO_X(m_background->m_size.x), RATIO_Y(m_background->m_size.y)),
	                 0, m_background, Color::white);

	GRenderer->SetBlendFunc(BlendOne, BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	
	EERIEDrawBitmap2(Rectf(Vec2f(m_pos.x, m_pos.y),
	                 RATIO_X(m_border->m_size.x), RATIO_Y(m_border->m_size.y)),
	                 0, m_border, Color::white);
	
	BOOST_FOREACH(MenuPage * page, m_pages) {
		if(m_currentPageId == page->eMenuState) {
			page->Render();
			
			if(g_debugInfo == InfoPanelGuiDebug)
				page->drawDebug();
			
			break;
		}
	}
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	
	if(eMS != NOP) {
		m_currentPageId=eMS;
	}
	
	return eMS;
}

MenuPage::MenuPage(const Vec2f & pos, const Vec2f & size, MENUSTATE _eMenuState)
	: m_rowSpacing(10)
	, m_savegame(0)
	, m_selected(NULL)
	, bEdit(false)
	, bMouseAttack(false)
	, m_blinkTime(0.f)
	, m_blink(true)
{
	m_size = size;
	
	Vec2f scaledSize = RATIO_2(size);
	m_rect = Rectf(RATIO_2(pos), scaledSize.x, scaledSize.y);
	
	eMenuState=_eMenuState;
}

void MenuPage::add(Widget * widget) {
	widget->ePlace = NOCENTER;
	widget->Move(m_rect.topLeft());
	m_children.add(widget);
}

void MenuPage::addCenter(Widget * widget, bool centerX) {

	int dx;
	
	if(centerX) {
		widget->ePlace = CENTER;
		
		int iDx = widget->m_rect.width();
		dx  = ((m_rect.width() - iDx) / 2) - widget->m_rect.left;
	
		if(dx < 0) {
			dx = 0;
		}
	} else {
		dx = 0;
	}
	
	int iDy = widget->m_rect.height();

	BOOST_FOREACH(Widget * widget, m_children.m_widgets) {
		iDy += m_rowSpacing;
		iDy += widget->m_rect.height();
	}

	int iDepY = m_rect.left;

	if(iDy < m_rect.height()) {
		iDepY += ((m_rect.height() - iDy) / 2);
	}

	int dy = 0;

	if(!m_children.m_widgets.empty()) {
		dy = iDepY - m_children.m_widgets[0]->m_rect.top;
	}
	
	BOOST_FOREACH(Widget * widget, m_children.m_widgets) {
		iDepY += (widget->m_rect.height()) + m_rowSpacing;
		
		widget->Move(Vec2f(0, dy));
	}

	widget->Move(Vec2f(dx, iDepY));

	m_children.add(widget);
}

void MenuPage::AlignElementCenter(Widget * widget) {
	
	widget->Move(Vec2f(-widget->m_rect.left, 0));
	widget->ePlace = CENTER;
	
	float iDx = widget->m_rect.width();
	float dx = (m_rect.width() - iDx) / 2 - widget->m_rect.left;
	
	widget->Move(Vec2f(std::max(dx, 0.f), 0.f));
}

void MenuPage::updateTextRect(TextWidget * widget) {
	float iDx = widget->m_rect.width();
	
	if(widget->ePlace) {
		widget->m_rect.left = m_pos.x + ((m_rect.width() - iDx) / 2.f);
		
		if(widget->m_rect.left < 0) {
			widget->m_rect.left = 0;
		}
	}
	
	widget->m_rect.right = widget->m_rect.left+iDx;
}

void MenuPage::UpdateText() {
	
	TextWidget * textWidget = (TextWidget*)m_selected;
	
	if(GInput->isAnyKeyPressed()) {
		
		if(GInput->isKeyPressed(Keyboard::Key_Enter)
		   || GInput->isKeyPressed(Keyboard::Key_NumPadEnter)
		   || GInput->isKeyPressed(Keyboard::Key_Escape)
		) {
			ARX_SOUND_PlayMenu(SND_MENU_CLICK);
			textWidget->eState = EDIT;
			
			if(textWidget->m_text.empty()) {
				std::string szMenuText;
				szMenuText = getLocalised("system_menu_editquest_newsavegame");
				
				textWidget->SetText(szMenuText);
				
				updateTextRect(textWidget);
			}
			
			m_selected = NULL;
			bEdit = false;
			return;
		}
		
		bool bKey = false;
		std::string tText;
		
		if(GInput->isKeyPressedNowPressed(Keyboard::Key_Backspace)) {
			tText = textWidget->m_text;
			
			if(!tText.empty()) {
				tText.resize(tText.size() - 1);
				bKey = true;
			}
		} else {
			int iKey = GInput->getKeyPressed();
			iKey &= 0xFFFF;
			
			if(GInput->isKeyPressedNowPressed(iKey)) {
				tText = textWidget->m_text;
				
				char tCat;
				
				bKey = GInput->getKeyAsText(iKey, tCat);
				
				if(bKey) {
					int iChar = tCat & 0x000000FF; // To prevent ascii chars between [128, 255] from causing an assertion in the functions below...
					if((isalnum(iChar) || isspace(iChar) || ispunct(iChar)) && (tCat != '\t') && (tCat != '*'))
						tText += tCat;
				}
			}
		}
		
		if(bKey) {
			textWidget->SetText(tText);
			
			if(textWidget->m_rect.width() > m_rect.width() - RATIO_X(64)) {
				if(!tText.empty()) {
					tText.resize(tText.size() - 1);
					textWidget->SetText(tText);
				}
			}
			
			updateTextRect(textWidget);
		}
	}
	
	if(m_selected->m_rect.top == m_selected->m_rect.bottom) {
		Vec2i textSize = ((TextWidget*)m_selected)->m_font->getTextSize("|");
		m_selected->m_rect.bottom += textSize.y;
	}
	
	if(m_blink) {
		//DRAW CURSOR
		TexturedVertex v[4];
		GRenderer->ResetTexture(0);
		v[0].color = v[1].color = v[2].color = v[3].color = Color::white.toRGB();
		v[0].p.z=v[1].p.z=v[2].p.z=v[3].p.z=0.f;
		v[0].rhw=v[1].rhw=v[2].rhw=v[3].rhw=1.f;
		
		v[0].p.x = m_selected->m_rect.right;
		v[0].p.y = m_selected->m_rect.top;
		v[1].p.x = v[0].p.x+2.f;
		v[1].p.y = v[0].p.y;
		v[2].p.x = v[0].p.x;
		v[2].p.y = m_selected->m_rect.bottom;
		v[3].p.x = v[1].p.x;
		v[3].p.y = v[2].p.y;
		
		EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);
	}
}

Widget * MenuPage::GetTouch(bool keyTouched, int keyId, InputKeyId* pInputKeyId, bool _bValidateTest)
{
	int iMouseButton = keyTouched ? 0 : GInput->getMouseButtonClicked();
	
	if(pInputKeyId)
		*pInputKeyId = keyId;

	if(keyTouched || (iMouseButton & (Mouse::ButtonBase | Mouse::WheelBase))) {
		if(!keyTouched && !bMouseAttack) {
			bMouseAttack=!bMouseAttack;
			return NULL;
		}

		TextWidget * textWidget = static_cast<TextWidget *>(m_selected);

		if(_bValidateTest) {
			if(m_selected->m_id == BUTTON_MENUOPTIONS_CONTROLS_CUST_ACTIONCOMBINE1 ||
			   m_selected->m_id == BUTTON_MENUOPTIONS_CONTROLS_CUST_ACTIONCOMBINE2)
			{
				bool bOk=true;

				if((iMouseButton & Mouse::ButtonBase) && !(iMouseButton & Mouse::WheelBase)) {
					bOk=false;
				} else {
					for(int buttonId = Mouse::ButtonBase; buttonId < Mouse::ButtonMax; buttonId++) {
						if(keyId == buttonId) {
							bOk=false;
							break;
						}
					}
				}

				if(bOk)
					return NULL;
			}
		}

		std::string pText;
		if(iMouseButton & (Mouse::ButtonBase | Mouse::WheelBase))
			pText = GInput->getKeyName(iMouseButton, true); 
		else
			pText = GInput->getKeyName(keyId, true);

		if(!pText.empty()) {
			textWidget->lColorHighlight = textWidget->lOldColor;

			textWidget->eState = GETTOUCH;
			textWidget->SetText(pText);
			
			float iDx = m_selected->m_rect.width();

			if(m_selected->ePlace) {
				m_selected->m_rect.left = (m_rect.width() - iDx) / 2.f;

				if(m_selected->m_rect.left < 0) {
					m_selected->m_rect.left=0;
				}
			}

			m_selected->m_rect.right=m_selected->m_rect.left+iDx;

			m_selected=NULL;
			bEdit=false;

			if(iMouseButton & (Mouse::ButtonBase | Mouse::WheelBase)) {
				if(pInputKeyId)
					*pInputKeyId = iMouseButton;
			}

			bMouseAttack=false;

			return textWidget;
		}
	}

	return NULL;
}

MENUSTATE MenuPage::Update(Vec2f pos) {

	m_children.Move(m_pos - m_oldPos);
	
	m_oldPos.x=m_pos.x;
	m_oldPos.y=m_pos.y;
	m_pos = pos;
	
	// Check if mouse over
	if(!bEdit) {
		m_selected=NULL;
		Widget * widget = m_children.getAtPos(Vec2f(GInput->getMousePosAbs()));
		
		if(widget) {
			m_selected = widget;
			
			if(GInput->getMouseButtonDoubleClick(Mouse::Button_0, 300)) {
				MENUSTATE e = m_selected->m_targetMenu;
				bEdit = m_selected->OnMouseDoubleClick();
				
				if(m_selected->m_id == BUTTON_MENUEDITQUEST_LOAD)
					return MAIN;
				
				if(bEdit)
					return m_selected->m_targetMenu;
				
				return e;
			}
			
			if(GInput->getMouseButton(Mouse::Button_0)) {
				MENUSTATE e = m_selected->m_targetMenu;
				bEdit = m_selected->OnMouseClick();
				return e;
			} else {
				m_selected->EmptyFunction();
			}
		}
	} else {
		if(!m_selected) {
			Widget * widget = m_children.getAtPos(Vec2f(GInput->getMousePosAbs()));
			
			if(widget) {
				m_selected = widget;
				
				if(GInput->getMouseButtonDoubleClick(Mouse::Button_0, 300)) {
					bEdit = m_selected->OnMouseDoubleClick();
					
					if(bEdit)
						return m_selected->m_targetMenu;
				}
			}
		}
	}
	
	//check les shortcuts
	if(!bEdit) {
		BOOST_FOREACH(Widget * widget, m_children.m_widgets) {
			arx_assert(widget);
			
			if(widget->m_shortcut != -1) {
				if(GInput->isKeyPressedNowUnPressed(widget->m_shortcut)) {
					bEdit = widget->OnMouseClick();
					m_selected = widget;
					return widget->m_targetMenu;
				}
			}
		}
	}
	
	return NOP;
}

static void UpdateGameKey(bool bEdit, Widget * widget, InputKeyId inputKeyId) {

	if(!bEdit && widget) {
		switch(widget->m_id) {
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP2:
			config.setActionKey(CONTROLS_CUST_JUMP,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_MAGICMODE1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_MAGICMODE2:
			config.setActionKey(CONTROLS_CUST_MAGICMODE,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_MAGICMODE1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STEALTHMODE1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STEALTHMODE2:
			config.setActionKey(CONTROLS_CUST_STEALTHMODE,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_STEALTHMODE1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKFORWARD1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKFORWARD2:
			config.setActionKey(CONTROLS_CUST_WALKFORWARD,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKFORWARD1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKBACKWARD1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKBACKWARD2:
			config.setActionKey(CONTROLS_CUST_WALKBACKWARD,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKBACKWARD1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFELEFT1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFELEFT2:
			config.setActionKey(CONTROLS_CUST_STRAFELEFT,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFELEFT1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFERIGHT1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFERIGHT2:
			config.setActionKey(CONTROLS_CUST_STRAFERIGHT,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFERIGHT1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANLEFT1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANLEFT2:
			config.setActionKey(CONTROLS_CUST_LEANLEFT,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANLEFT1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANRIGHT1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANRIGHT2:
			config.setActionKey(CONTROLS_CUST_LEANRIGHT,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANRIGHT1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCH1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCH2:
			config.setActionKey(CONTROLS_CUST_CROUCH,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCH1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_USE1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_USE2:
			config.setActionKey(CONTROLS_CUST_USE,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_USE1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_ACTIONCOMBINE1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_ACTIONCOMBINE2:
			config.setActionKey(CONTROLS_CUST_ACTION,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_ACTIONCOMBINE1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_INVENTORY1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_INVENTORY2:
			config.setActionKey(CONTROLS_CUST_INVENTORY,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_INVENTORY1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOK1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOK2:
			config.setActionKey(CONTROLS_CUST_BOOK,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOK1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKCHARSHEET1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKCHARSHEET2:
			config.setActionKey(CONTROLS_CUST_BOOKCHARSHEET,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKCHARSHEET1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKSPELL1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKSPELL2:
			config.setActionKey(CONTROLS_CUST_BOOKSPELL,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKSPELL1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKMAP1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKMAP2:
			config.setActionKey(CONTROLS_CUST_BOOKMAP,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKMAP1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKQUEST1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKQUEST2:
			config.setActionKey(CONTROLS_CUST_BOOKQUEST,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKQUEST1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONLIFE1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONLIFE2:
			config.setActionKey(CONTROLS_CUST_DRINKPOTIONLIFE,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONLIFE1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONMANA1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONMANA2:
			config.setActionKey(CONTROLS_CUST_DRINKPOTIONMANA,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONMANA1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_TORCH1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_TORCH2:
			config.setActionKey(CONTROLS_CUST_TORCH,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_TORCH1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CANCELCURSPELL1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CANCELCURSPELL2:    
			config.setActionKey(CONTROLS_CUST_CANCELCURSPELL,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_CANCELCURSPELL1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST1_2:
			config.setActionKey(CONTROLS_CUST_PRECAST1,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST2:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST2_2:
			config.setActionKey(CONTROLS_CUST_PRECAST2,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST2,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST3:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST3_2:
			config.setActionKey(CONTROLS_CUST_PRECAST3,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST3,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_WEAPON1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_WEAPON2:
			config.setActionKey(CONTROLS_CUST_WEAPON,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_WEAPON1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKLOAD:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKLOAD2:
			config.setActionKey(CONTROLS_CUST_QUICKLOAD,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKLOAD,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKSAVE:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKSAVE2:
			config.setActionKey(CONTROLS_CUST_QUICKSAVE,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKSAVE,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNLEFT1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNLEFT2:
			config.setActionKey(CONTROLS_CUST_TURNLEFT,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNLEFT1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNRIGHT1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNRIGHT2:
			config.setActionKey(CONTROLS_CUST_TURNRIGHT,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNRIGHT1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKUP1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKUP2:
			config.setActionKey(CONTROLS_CUST_LOOKUP,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKUP1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKDOWN1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKDOWN2:
			config.setActionKey(CONTROLS_CUST_LOOKDOWN,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKDOWN1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFE1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFE2:
			config.setActionKey(CONTROLS_CUST_STRAFE,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFE1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CENTERVIEW1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CENTERVIEW2:
			config.setActionKey(CONTROLS_CUST_CENTERVIEW,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_CENTERVIEW1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_FREELOOK1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_FREELOOK2:
			config.setActionKey(CONTROLS_CUST_FREELOOK,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_FREELOOK1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PREVIOUS1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PREVIOUS2:
			config.setActionKey(CONTROLS_CUST_PREVIOUS,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_PREVIOUS1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_NEXT1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_NEXT2:    
			config.setActionKey(CONTROLS_CUST_NEXT,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_NEXT1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCHTOGGLE1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCHTOGGLE2:    
			config.setActionKey(CONTROLS_CUST_CROUCHTOGGLE,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCHTOGGLE1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_UNEQUIPWEAPON1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_UNEQUIPWEAPON2:    
			config.setActionKey(CONTROLS_CUST_UNEQUIPWEAPON,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_UNEQUIPWEAPON1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_MINIMAP1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_MINIMAP2:
			config.setActionKey(CONTROLS_CUST_MINIMAP,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_MINIMAP1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_TOGGLE_FULLSCREEN1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_TOGGLE_FULLSCREEN2:
			config.setActionKey(CONTROLS_CUST_TOGGLE_FULLSCREEN,widget->m_id-BUTTON_MENUOPTIONS_CONTROLS_CUST_TOGGLE_FULLSCREEN1,inputKeyId);
			break;
		default:
			break;
		}
	}
}

void MenuPage::Render() {

	if(bNoMenu)
		return;
	
	BOOST_FOREACH(Widget * widget, m_children.m_widgets) {
		widget->Update();
		widget->Render();
	}
	
	//HIGHLIGHT
	if(m_selected) {
		bool bReInit=false;

		m_selected->RenderMouseOver();
		
		{
			m_blinkTime += ARXDiffTimeMenu;
			if(m_blinkTime > m_blinkDuration * 2)
				m_blinkTime = 0;
			
			m_blink = m_blinkTime > m_blinkDuration;
		}
		
		switch(m_selected->eState) {
			case EDIT_TIME:
				UpdateText();
				break;
			case GETTOUCH_TIME: {
				if(m_blink)
					((TextWidget*)m_selected)->lColorHighlight = Color(255, 0, 0);
				else
					((TextWidget*)m_selected)->lColorHighlight = Color(50, 0, 0);
				
				bool keyTouched = GInput->isAnyKeyPressed();
				int keyId = GInput->getKeyPressed();
				
				if( GInput->isKeyPressed(Keyboard::Key_LeftShift)||
					GInput->isKeyPressed(Keyboard::Key_RightShift)||
					GInput->isKeyPressed(Keyboard::Key_LeftCtrl)||
					GInput->isKeyPressed(Keyboard::Key_RightCtrl)||
					GInput->isKeyPressed(Keyboard::Key_LeftAlt)||
					GInput->isKeyPressed(Keyboard::Key_RightAlt)
				) {
					if(!((keyId & INPUT_COMBINATION_MASK )>>16))
						keyTouched = false;
				} else {
					if(GInput->isKeyPressedNowUnPressed(Keyboard::Key_LeftShift)) {
						keyTouched = true;
						keyId = Keyboard::Key_LeftShift;
					}
					
					if(GInput->isKeyPressedNowUnPressed(Keyboard::Key_RightShift)) {
						keyTouched = true;
						keyId = Keyboard::Key_RightShift;
					}
					
					if(GInput->isKeyPressedNowUnPressed(Keyboard::Key_LeftCtrl)) {
						keyTouched = true;
						keyId = Keyboard::Key_LeftCtrl;
					}
					
					if(GInput->isKeyPressedNowUnPressed(Keyboard::Key_RightCtrl)) {
						keyTouched = true;
						keyId = Keyboard::Key_RightCtrl;
					}
					
					if(GInput->isKeyPressedNowUnPressed(Keyboard::Key_LeftAlt)) {
						keyTouched = true;
						keyId = Keyboard::Key_LeftAlt;
					}
					
					if(GInput->isKeyPressedNowUnPressed(Keyboard::Key_RightAlt)) {
						keyTouched = true;
						keyId = Keyboard::Key_RightAlt;
					}
				}
				
				InputKeyId inputKeyId;
				Widget * widget = GetTouch(keyTouched, keyId, &inputKeyId, true);
				
				if(widget) {
					UpdateGameKey(bEdit,widget, inputKeyId);
					bReInit = true;
				}
			}
			break;
			default:
			{
				if(GInput->getMouseButtonNowPressed(Mouse::Button_0)) {
					Widget * widget = m_children.GetZoneWithID(BUTTON_MENUOPTIONS_CONTROLS_CUST_DEFAULT);
					
					if(widget == m_selected) {
						config.setDefaultActionKeys();
						bReInit=true;
					}
				}
			}
			break;
		}
		
		if(bReInit) {
			ReInitActionKey();
			bMouseAttack=false;
		}
	}
}

void MenuPage::drawDebug() {
	Rectf rect = Rectf(Vec2f(m_pos.x, m_pos.y), m_rect.width(), m_rect.height());
	drawLineRectangle(rect, 0.f, Color::green);
	
	m_children.drawDebug();
}

void MenuPage::ReInitActionKey()
{
	int iID=BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP1;
	int iI=NUM_ACTION_KEY;
	
	while(iI--) {
		int iTab=(iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP1)>>1;

		Widget * widget = m_children.GetZoneWithID(MenuButton(iID));

		if(widget) {
			if(widget) {
				m_selected = widget;
				GetTouch(true, config.actions[iTab].key[0], NULL, false);
			}

			widget = m_children.GetZoneWithID(MenuButton(iID + 1));

			if(widget) {
				m_selected = widget;
				GetTouch(true, config.actions[iTab].key[1], NULL, false);
			}
		}

		iID+=2;
	}
}


void Menu2_Open() {
	if(pMenuCursor) {
		pMenuCursor->reset();
	}
}

void Menu2_Close() {
	
	ARXmenu.currentmode = AMCM_OFF;
	
	delete pWindowMenu, pWindowMenu = NULL;
	delete mainMenu, mainMenu = NULL;
	delete pMenuCursor, pMenuCursor = NULL;
}

void MenuReInitAll() {
	
	if(!mainMenu)
		return;
	
	mainMenu->bReInitAll=true;
}
