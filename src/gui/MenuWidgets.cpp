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

int newWidth;
int newHeight;
bool newFullscreen;

// Imported global variables and functions
extern ARX_MENU_DATA ARXmenu;

extern Rect g_size;

extern long REFUSE_GAME_RETURN;

bool bNoMenu=false;

MenuCursor * pMenuCursor = NULL;

extern CWindowMenu * pWindowMenu;
MainMenu *mainMenu;

extern TextWidget * pMenuElementApply;
extern CheckboxWidget * fullscreenCheckbox;
extern CycleTextWidget * pMenuSliderResol;

float ARXTimeMenu;
float ARXDiffTimeMenu;

bool bFade=false;
bool bFadeInOut=false;
int iFadeAction=-1;
float fFadeInOut=0.f;

TextureContainer *pTextureLoad=NULL;
TextureContainer *pTextureLoadRender=NULL;

void ARX_QuickSave() {
	
	if(REFUSE_GAME_RETURN) {
		return;
	}
	
	ARX_SOUND_MixerPause(ARX_SOUND_MixerGame);
	
	savegames.quicksave(savegame_thumbnail);
	
	ARX_SOUND_MixerResume(ARX_SOUND_MixerGame);
}

static bool ARX_LoadGame(const SaveGame & save) {
	
	LoadLevelScreen();
	progressBarSetTotal(238);
	progressBarReset();
	progressBarAdvance();
	LoadLevelScreen(save.level);
	
	DanaeClearLevel();
	
	long ret = ARX_CHANGELEVEL_Load(save.savefile);
	
	REFUSE_GAME_RETURN = 0;

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

bool ARX_SlotLoad(int slotIndex) {
	if(slotIndex >= (int)savegames.size()) {
		// Invalid slot!
		return false;
	}

	ARX_SOUND_MixerPause(ARX_SOUND_MixerMenu);
	return ARX_LoadGame(savegames[slotIndex]);
}

bool MENU_NoActiveWindow() {
	if(!pWindowMenu
	   || (pWindowMenu && pWindowMenu->eCurrentMenuState == MAIN)
	) {
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
	GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::DepthTest, false);
	GRenderer->SetCulling(Renderer::CullNone);

	EERIEDRAWPRIM(Renderer::TriangleStrip, d3dvertex, 4, true);

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetRenderState(Renderer::DepthTest, true);
	GRenderer->SetCulling(Renderer::CullCCW);
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
	
	float time = arxtime.get_updated(false);
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
			Credits::render();
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
	GRenderer->SetCulling(Renderer::CullNone);

	MENUSTATE eOldMenuState=NOP;
	MENUSTATE eM;

	if(!mainMenu) {
		eM = NOP;
	} else {
		eM = mainMenu->eOldMenuWindowState;
	}
	
	if(!mainMenu || (mainMenu && mainMenu->bReInitAll)) {
		
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
	{
	
	MENUSTATE eMenuState = mainMenu->Update();
	
	if(eOldMenuState != NOP) {
		eMenuState=eOldMenuState;
		bScroll=false;
	}
	
	if(eMenuState == RESUME_GAME) {
		pTextManage->Clear();
		ARXmenu.currentmode = AMCM_OFF;
		mainMenu->pZoneClick = NULL;
		
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
	}
	mainMenu->Render();

	if(pWindowMenu) {
		if(!bScroll) {
			pWindowMenu->fAngle=90.f;
			pWindowMenu->eCurrentMenuState=mainMenu->eOldMenuWindowState;
		}

		pWindowMenu->Update(ARXDiffTimeMenu);

		if(pWindowMenu) {
			MENUSTATE eMS=pWindowMenu->Render();

			if(eMS != NOP) {
				mainMenu->eOldMenuWindowState=eMS;
			}
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
	GRenderer->SetCulling(Renderer::CullNone);
	pMenuCursor->DrawCursor();

	if(pTextureLoadRender) {
		Vec2f size = Vec2f(pTextureLoad->size());
		
		Vec2f offset = Vec2f(0, 0);
		
		if(DANAEMouse.y + size.y > g_size.height()) {
			offset.y -= size.y;
		}
		
		Vec2f pos = Vec2f(DANAEMouse) + offset;
		
		Rectf rect = Rectf(pos, size.x, size.y);
		
		EERIEDrawBitmap(rect, 0.001f, pTextureLoad, Color::white);
		drawLineRectangle(rect, 0.01f, Color::white);

		pTextureLoadRender=NULL;
	}

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
	GRenderer->SetCulling(Renderer::CullCCW);

	return true;
}






CheckboxWidget::CheckboxWidget(TextWidget *_pText)
	:Widget(NOP)
{
	pRef = this; // TODO remove this
	
	arx_assert(_pText);
	
	m_textureOff = TextureContainer::Load("graph/interface/menus/menu_checkbox_off");
	m_textureOn = TextureContainer::Load("graph/interface/menus/menu_checkbox_on");
	arx_assert(m_textureOff);
	arx_assert(m_textureOn);
	arx_assert(m_textureOff->size() == m_textureOn->size());
	
	iID = BUTTON_INVALID;
	iState    = 0;
	iOldState = -1;
	pText    = _pText;
	rZone = _pText->rZone;
	
	rZone.right = rZone.left + RATIO_X(245.f);
}

CheckboxWidget::~CheckboxWidget() {
	delete pText;
}

void CheckboxWidget::Move(const Vec2i & offset) {
	
	Widget::Move(offset);
	pText->Move(offset);
}

bool CheckboxWidget::OnMouseClick() {
	
	if(iOldState<0)
		iOldState=iState;

	iState ++;

	//NB : It seems that iState cannot be negative (used as tabular index / used as bool) but need further approval
	arx_assert(iState >= 0);

	if((size_t)iState >= 2) {
		iState = 0;
	}

	ARX_SOUND_PlayMenu(SND_MENU_CLICK);

	switch (iID) {
		case BUTTON_MENUOPTIONSVIDEO_FULLSCREEN: {
			newFullscreen = ((iState)?true:false);
			
			if(pMenuSliderResol) {
				pMenuSliderResol->setEnabled(newFullscreen);
			}
			
		}
		break;
		case BUTTON_MENUOPTIONSVIDEO_CROSSHAIR: {
			config.video.showCrosshair=(iState)?true:false;
		}
		break;
		case BUTTON_MENUOPTIONSVIDEO_ANTIALIASING: {
			config.video.antialiasing = iState ? true : false;
			ARX_SetAntiAliasing();
			break;
		}
		case BUTTON_MENUOPTIONSVIDEO_VSYNC: {
			config.video.vsync = iState ? true : false;
			break;
		}
		case BUTTON_MENUOPTIONSVIDEO_HUDSCALE: {
			config.video.hudScale = iState ? true : false;
			g_hudRoot.recalcScale();
			break;
		}
		case BUTTON_MENUOPTIONSAUDIO_EAX: {
			ARXMenu_Options_Audio_SetEAX(iState != 0);
			break;
		}
		case BUTTON_MENUOPTIONS_CONTROLS_INVERTMOUSE: {
				ARXMenu_Options_Control_SetInvertMouse((iState)?true:false);
			}
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_AUTOREADYWEAPON: {
			config.input.autoReadyWeapon = (iState) ? true : false;
			break;
		}
		case BUTTON_MENUOPTIONS_CONTROLS_MOUSELOOK: {
			config.input.mouseLookToggle = (iState) ? true : false;
			break;
		}
		case BUTTON_MENUOPTIONS_CONTROLS_AUTODESCRIPTION: {
			config.input.autoDescription = (iState) ? true : false;
			break;
		}
		case BUTTON_MENUOPTIONSVIDEO_BACK: {
			if(pMenuSliderResol && pMenuSliderResol->getOldValue() >= 0) {
				pMenuSliderResol->setValue(pMenuSliderResol->getOldValue());
				pMenuSliderResol->setOldValue(-1);
				newWidth=config.video.resolution.x;
				newHeight=config.video.resolution.y;
			}
			
			if(fullscreenCheckbox && fullscreenCheckbox->iOldState >= 0) {
				fullscreenCheckbox->iState = fullscreenCheckbox->iOldState;
				fullscreenCheckbox->iOldState = -1;
				newFullscreen = config.video.fullscreen;
			}
			break;
		}
		default:
			break;
	}

	return false;
}

void CheckboxWidget::Update(int /*_iDTime*/)
{
}

void CheckboxWidget::renderCommon() {
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	
	Rectf checkboxRect;
	checkboxRect.top = rZone.top;
	checkboxRect.left = rZone.right - rZone.height();
	checkboxRect.bottom = rZone.bottom;
	checkboxRect.right = rZone.right;
	
	TextureContainer *pTex = (iState == 0) ? m_textureOff : m_textureOn;
	Color color = (bCheck) ? Color::white : Color(63, 63, 63, 255);
	
	EERIEDrawBitmap2(checkboxRect, 0.f, pTex, color);
}

void CheckboxWidget::Render() {

	if(bNoMenu)
		return;
	
	renderCommon();
	
	pText->Render();
}

void CheckboxWidget::RenderMouseOver() {

	if(bNoMenu)
		return;

	pMenuCursor->SetMouseOver();

	renderCommon();
	
	pText->RenderMouseOver();
}


CWindowMenu::CWindowMenu(Vec2i pos, Vec2i size)
{
	m_pos = RATIO_2(pos);
	m_size = RATIO_2(size);

	vWindowConsoleElement.clear();

	fPosXCalc=((float)-m_size.x);
	fDist=((float)(m_size.x+m_pos.x));
	fAngle=0.f;

	eCurrentMenuState=NOP;


	float fCalc	= fPosXCalc + (fDist * glm::sin(glm::radians(fAngle)));

	m_pos.x = checked_range_cast<int>(fCalc);
}

CWindowMenu::~CWindowMenu() {

	for(std::vector<CWindowMenuConsole*>::iterator it = vWindowConsoleElement.begin(), it_end = vWindowConsoleElement.end(); it < it_end; ++it)
		delete *it;
}

void CWindowMenu::AddConsole(CWindowMenuConsole *_pMenuConsoleElement) {

	vWindowConsoleElement.push_back(_pMenuConsoleElement);
	_pMenuConsoleElement->m_oldPos.x = 0;
	_pMenuConsoleElement->m_oldPos.y = 0;
	_pMenuConsoleElement->m_pos = m_pos;
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
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	
	MENUSTATE eMS=NOP;
	
	BOOST_FOREACH(CWindowMenuConsole * c, vWindowConsoleElement) {
		if(eCurrentMenuState == c->eMenuState) {
			eMS = c->Update(m_pos);
			
			if(eMS != NOP)
				break;
		}
	}
	
	BOOST_FOREACH(CWindowMenuConsole * c, vWindowConsoleElement) {
		if(eCurrentMenuState == c->eMenuState) {
			c->Render();
			break;
		}
	}
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	
	if(eMS != NOP) {
		eCurrentMenuState=eMS;
	}
	
	return eMS;
}

CWindowMenuConsole::CWindowMenuConsole(Vec2i pos, Vec2i size, MENUSTATE _eMenuState)
	: m_rowSpacing(10)
	, m_savegame(0)
	, pZoneClick(NULL)
	, bEdit(false)
	, bMouseAttack(false)
	, m_textCursorCurrentTime(0.f)
{
	m_offset = RATIO_2(pos);
	m_size = RATIO_2(size);
	
	eMenuState=_eMenuState;

	pTexBackground = TextureContainer::LoadUI("graph/interface/menus/menu_console_background");
	pTexBackgroundBorder = TextureContainer::LoadUI("graph/interface/menus/menu_console_background_border");

	bFrameOdd=false;
}

void CWindowMenuConsole::AddMenu(Widget * element) {
	element->ePlace = NOCENTER;
	element->Move(m_offset);
	MenuAllZone.AddZone(element);
}

void CWindowMenuConsole::AddMenuCenter(Widget * element, bool centerX) {

	int dx;
	
	if(centerX) {
		element->ePlace = CENTER;
		
		int iDx = element->rZone.right - element->rZone.left;
		dx  = ((m_size.x - iDx) >> 1) - element->rZone.left;
	
		if(dx < 0) {
			dx = 0;
		}
	} else {
		dx = 0;
	}
	
	int iDy = element->rZone.bottom - element->rZone.top;

	for(size_t iJ = 0; iJ < MenuAllZone.GetNbZone(); iJ++) {
		Widget * pZone = MenuAllZone.GetZoneNum(iJ);

		iDy += m_rowSpacing;
		iDy += pZone->rZone.bottom - pZone->rZone.top;
	}

	int iDepY = m_offset.y;

	if(iDy < m_size.y) {
		iDepY += ((m_size.y - iDy) >> 1);
	}

	int dy = 0;

	if(MenuAllZone.GetNbZone()) {
		dy = iDepY - MenuAllZone.GetZoneNum(0)->rZone.top;
	}
	
	for(size_t iJ = 0; iJ < MenuAllZone.GetNbZone(); iJ++) {
		Widget *pZone = MenuAllZone.GetZoneNum(iJ);
		iDepY += (pZone->rZone.bottom - pZone->rZone.top) + m_rowSpacing;
		
		pZone->Move(Vec2i(0, dy));
	}

	element->Move(Vec2i(dx, iDepY));

	MenuAllZone.AddZone(element);
}

void CWindowMenuConsole::AlignElementCenter(Widget *_pMenuElement) {
	
	_pMenuElement->Move(Vec2i(-_pMenuElement->rZone.left, 0));
	_pMenuElement->ePlace = CENTER;
	
	int iDx = _pMenuElement->rZone.right - _pMenuElement->rZone.left;
	int dx = (m_size.x - iDx) / 2 - _pMenuElement->rZone.left;
	
	_pMenuElement->Move(Vec2i(std::max(dx, 0), 0));
}

void CWindowMenuConsole::UpdateText() {

	if(GInput->isAnyKeyPressed()) {

		if(GInput->isKeyPressed(Keyboard::Key_Enter)
		   || GInput->isKeyPressed(Keyboard::Key_NumPadEnter)
		   || GInput->isKeyPressed(Keyboard::Key_Escape)
		) {
			ARX_SOUND_PlayMenu(SND_MENU_CLICK);
			((TextWidget*)pZoneClick)->eState=EDIT;

			if(((TextWidget*)pZoneClick)->lpszText.empty()) {
				std::string szMenuText;
				szMenuText = getLocalised("system_menu_editquest_newsavegame");

				((TextWidget*)pZoneClick)->SetText(szMenuText);

				int iDx=pZoneClick->rZone.right-pZoneClick->rZone.left;

				if(pZoneClick->ePlace) {
					pZoneClick->rZone.left=m_pos.x+((m_size.x-iDx)>>1);

					if(pZoneClick->rZone.left < 0) {
						pZoneClick->rZone.left=0;
					}
				}

				pZoneClick->rZone.right=pZoneClick->rZone.left+iDx;
			}

			pZoneClick=NULL;
			bEdit=false;
			return;
		}

		bool bKey=false;
		std::string tText;
		
		TextWidget *pZoneText=(TextWidget*)pZoneClick;

		if(GInput->isKeyPressedNowPressed(Keyboard::Key_Backspace)) {
			tText = pZoneText->lpszText;

			if(!tText.empty()) {
				tText.resize(tText.size() - 1);
				bKey=true;
			}
		} else {
			int iKey = GInput->getKeyPressed();
			iKey&=0xFFFF;

			if(GInput->isKeyPressedNowPressed(iKey)) {
				tText = pZoneText->lpszText;

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
			pZoneText->SetText(tText);

			if(pZoneText->rZone.right - pZoneText->rZone.left > m_size.x - RATIO_X(64)) {
				if(!tText.empty()) {
					tText.resize(tText.size() - 1);
					pZoneText->SetText(tText);
				}
			}

			int iDx=pZoneClick->rZone.right-pZoneClick->rZone.left;

			if(pZoneClick->ePlace) {
				pZoneClick->rZone.left=m_pos.x+((m_size.x-iDx)>>1);

				if(pZoneClick->rZone.left < 0) {
					pZoneClick->rZone.left=0;
				}
			}

			pZoneClick->rZone.right=pZoneClick->rZone.left+iDx;
		}
	}

	if(pZoneClick->rZone.top == pZoneClick->rZone.bottom) {
		Vec2i textSize = ((TextWidget*)pZoneClick)->pFont->getTextSize("|");
		pZoneClick->rZone.bottom += textSize.y;
	}
	
	m_textCursorCurrentTime += ARXDiffTimeMenu;
	if(m_textCursorCurrentTime > m_textCursorFlashDuration * 2)
		m_textCursorCurrentTime = 0;
	
	bool showTextCursor = m_textCursorCurrentTime > m_textCursorFlashDuration;
	
	if(showTextCursor) {
	//DRAW CURSOR
	TexturedVertex v[4];
	GRenderer->ResetTexture(0);
	v[0].color = v[1].color = v[2].color = v[3].color = Color::white.toRGB();
	v[0].p.z=v[1].p.z=v[2].p.z=v[3].p.z=0.f;    
	v[0].rhw=v[1].rhw=v[2].rhw=v[3].rhw=1.f;

	v[0].p.x = (float)pZoneClick->rZone.right;
	v[0].p.y = (float)pZoneClick->rZone.top;
	v[1].p.x = v[0].p.x+2.f;
	v[1].p.y = v[0].p.y;
	v[2].p.x = v[0].p.x;
	v[2].p.y = (float)pZoneClick->rZone.bottom;
	v[3].p.x = v[1].p.x;
	v[3].p.y = v[2].p.y;

	EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);
	}
}

Widget * CWindowMenuConsole::GetTouch(bool keyTouched, int keyId, InputKeyId* pInputKeyId, bool _bValidateTest)
{
	int iMouseButton = keyTouched ? 0 : GInput->getMouseButtonClicked();
	
	if(pInputKeyId)
		*pInputKeyId = keyId;

	if(keyTouched || (iMouseButton & (Mouse::ButtonBase | Mouse::WheelBase))) {
		if(!keyTouched && !bMouseAttack) {
			bMouseAttack=!bMouseAttack;
			return NULL;
		}

		TextWidget *pZoneText=(TextWidget*)pZoneClick;

		if(_bValidateTest) {
			if(pZoneClick->iID == BUTTON_MENUOPTIONS_CONTROLS_CUST_ACTIONCOMBINE1 ||
			   pZoneClick->iID == BUTTON_MENUOPTIONS_CONTROLS_CUST_ACTIONCOMBINE2)
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
			pZoneText->lColorHighlight=pZoneText->lOldColor;

			pZoneText->eState=GETTOUCH;
			pZoneText->SetText(pText);
			
			int iDx=pZoneClick->rZone.right-pZoneClick->rZone.left;

			if(pZoneClick->ePlace) {
				pZoneClick->rZone.left=(m_size.x-iDx)>>1;

				if(pZoneClick->rZone.left < 0) {
					pZoneClick->rZone.left=0;
				}
			}

			pZoneClick->rZone.right=pZoneClick->rZone.left+iDx;

			pZoneClick=NULL;
			bEdit=false;

			if(iMouseButton & (Mouse::ButtonBase | Mouse::WheelBase)) {
				if(pInputKeyId)
					*pInputKeyId = iMouseButton;
			}

			bMouseAttack=false;

			return pZoneText;
		}
	}

	return NULL;
}

MENUSTATE CWindowMenuConsole::Update(Vec2i pos) {

	bFrameOdd=!bFrameOdd;
	
	MenuAllZone.Move(m_pos - m_oldPos);
	
	m_oldPos.x=m_pos.x;
	m_oldPos.y=m_pos.y;
	m_pos = pos;
	
	// Check if mouse over
	if(!bEdit) {
		pZoneClick=NULL;
		Widget * iR = MenuAllZone.CheckZone(GInput->getMousePosAbs());
		
		if(iR) {
			pZoneClick = iR;
			
			if(GInput->getMouseButtonDoubleClick(Mouse::Button_0, 300)) {
				MENUSTATE e = pZoneClick->eMenuState;
				bEdit = pZoneClick->OnMouseDoubleClick();
				
				if(pZoneClick->iID == BUTTON_MENUEDITQUEST_LOAD)
					return MAIN;
				
				if(bEdit)
					return pZoneClick->eMenuState;
				
				return e;
			}
			
			if(GInput->getMouseButton(Mouse::Button_0)) {
				MENUSTATE e = pZoneClick->eMenuState;
				bEdit = pZoneClick->OnMouseClick();
				return e;
			} else {
				pZoneClick->EmptyFunction();
			}
		}
	} else {
		if(!pZoneClick) {
			Widget * iR = MenuAllZone.CheckZone(GInput->getMousePosAbs());
			
			if(iR) {
				pZoneClick = iR;
				
				if(GInput->getMouseButtonDoubleClick(Mouse::Button_0, 300)) {
					bEdit = pZoneClick->OnMouseDoubleClick();
					
					if(bEdit)
						return pZoneClick->eMenuState;
				}
			}
		}
	}
	
	//check les shortcuts
	if(!bEdit) {
		for(size_t iJ = 0; iJ < MenuAllZone.GetNbZone(); ++iJ) {
			Widget * pMenuElement = MenuAllZone.GetZoneNum(iJ);
			Widget *CMenuElementShortCut = pMenuElement->OnShortCut();
			
			if(CMenuElementShortCut) {
				pZoneClick=CMenuElementShortCut;
				MENUSTATE e = pZoneClick->eMenuState;
				bEdit = pZoneClick->OnMouseClick();
				pZoneClick=CMenuElementShortCut;
				return e;
			}
		}
	}
	
	return NOP;
}

static bool UpdateGameKey(bool bEdit, Widget *pmeElement, InputKeyId inputKeyId) {
	bool bChange=false;

	if(!bEdit && pmeElement) {
		switch(pmeElement->iID) {
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP2:
			bChange=config.setActionKey(CONTROLS_CUST_JUMP,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_MAGICMODE1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_MAGICMODE2:
			bChange=config.setActionKey(CONTROLS_CUST_MAGICMODE,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_MAGICMODE1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STEALTHMODE1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STEALTHMODE2:
			bChange=config.setActionKey(CONTROLS_CUST_STEALTHMODE,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_STEALTHMODE1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKFORWARD1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKFORWARD2:
			bChange=config.setActionKey(CONTROLS_CUST_WALKFORWARD,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKFORWARD1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKBACKWARD1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKBACKWARD2:
			bChange=config.setActionKey(CONTROLS_CUST_WALKBACKWARD,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKBACKWARD1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFELEFT1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFELEFT2:
			bChange=config.setActionKey(CONTROLS_CUST_STRAFELEFT,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFELEFT1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFERIGHT1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFERIGHT2:
			bChange=config.setActionKey(CONTROLS_CUST_STRAFERIGHT,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFERIGHT1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANLEFT1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANLEFT2:
			bChange=config.setActionKey(CONTROLS_CUST_LEANLEFT,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANLEFT1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANRIGHT1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANRIGHT2:
			bChange=config.setActionKey(CONTROLS_CUST_LEANRIGHT,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANRIGHT1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCH1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCH2:
			bChange=config.setActionKey(CONTROLS_CUST_CROUCH,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCH1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_USE1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_USE2:
			bChange=config.setActionKey(CONTROLS_CUST_USE,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_USE1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_ACTIONCOMBINE1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_ACTIONCOMBINE2:
			bChange=config.setActionKey(CONTROLS_CUST_ACTION,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_ACTIONCOMBINE1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_INVENTORY1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_INVENTORY2:
			bChange=config.setActionKey(CONTROLS_CUST_INVENTORY,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_INVENTORY1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOK1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOK2:
			bChange=config.setActionKey(CONTROLS_CUST_BOOK,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOK1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKCHARSHEET1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKCHARSHEET2:
			bChange=config.setActionKey(CONTROLS_CUST_BOOKCHARSHEET,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKCHARSHEET1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKSPELL1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKSPELL2:
			bChange=config.setActionKey(CONTROLS_CUST_BOOKSPELL,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKSPELL1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKMAP1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKMAP2:
			bChange=config.setActionKey(CONTROLS_CUST_BOOKMAP,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKMAP1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKQUEST1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKQUEST2:
			bChange=config.setActionKey(CONTROLS_CUST_BOOKQUEST,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKQUEST1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONLIFE1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONLIFE2:
			bChange=config.setActionKey(CONTROLS_CUST_DRINKPOTIONLIFE,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONLIFE1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONMANA1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONMANA2:
			bChange=config.setActionKey(CONTROLS_CUST_DRINKPOTIONMANA,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONMANA1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_TORCH1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_TORCH2:
			bChange=config.setActionKey(CONTROLS_CUST_TORCH,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_TORCH1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CANCELCURSPELL1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CANCELCURSPELL2:    
			bChange=config.setActionKey(CONTROLS_CUST_CANCELCURSPELL,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_CANCELCURSPELL1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST1_2:
			bChange=config.setActionKey(CONTROLS_CUST_PRECAST1,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST2:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST2_2:
			bChange=config.setActionKey(CONTROLS_CUST_PRECAST2,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST2,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST3:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST3_2:
			bChange=config.setActionKey(CONTROLS_CUST_PRECAST3,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST3,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_WEAPON1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_WEAPON2:
			bChange=config.setActionKey(CONTROLS_CUST_WEAPON,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_WEAPON1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKLOAD:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKLOAD2:
			bChange=config.setActionKey(CONTROLS_CUST_QUICKLOAD,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKLOAD,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKSAVE:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKSAVE2:
			bChange=config.setActionKey(CONTROLS_CUST_QUICKSAVE,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKSAVE,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNLEFT1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNLEFT2:
			bChange=config.setActionKey(CONTROLS_CUST_TURNLEFT,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNLEFT1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNRIGHT1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNRIGHT2:
			bChange=config.setActionKey(CONTROLS_CUST_TURNRIGHT,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNRIGHT1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKUP1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKUP2:
			bChange=config.setActionKey(CONTROLS_CUST_LOOKUP,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKUP1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKDOWN1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKDOWN2:
			bChange=config.setActionKey(CONTROLS_CUST_LOOKDOWN,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKDOWN1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFE1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFE2:
			bChange=config.setActionKey(CONTROLS_CUST_STRAFE,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFE1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CENTERVIEW1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CENTERVIEW2:
			bChange=config.setActionKey(CONTROLS_CUST_CENTERVIEW,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_CENTERVIEW1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_FREELOOK1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_FREELOOK2:
			bChange=config.setActionKey(CONTROLS_CUST_FREELOOK,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_FREELOOK1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PREVIOUS1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PREVIOUS2:
			bChange=config.setActionKey(CONTROLS_CUST_PREVIOUS,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_PREVIOUS1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_NEXT1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_NEXT2:    
			bChange=config.setActionKey(CONTROLS_CUST_NEXT,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_NEXT1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCHTOGGLE1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCHTOGGLE2:    
			bChange=config.setActionKey(CONTROLS_CUST_CROUCHTOGGLE,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCHTOGGLE1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_UNEQUIPWEAPON1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_UNEQUIPWEAPON2:    
			bChange=config.setActionKey(CONTROLS_CUST_UNEQUIPWEAPON,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_UNEQUIPWEAPON1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_MINIMAP1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_MINIMAP2:
			bChange=config.setActionKey(CONTROLS_CUST_MINIMAP,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_MINIMAP1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_TOGGLE_FULLSCREEN1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_TOGGLE_FULLSCREEN2:
			bChange=config.setActionKey(CONTROLS_CUST_TOGGLE_FULLSCREEN,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_TOGGLE_FULLSCREEN1,inputKeyId);
			break;
		default:
			break;
		}
	}

	return bChange;
}

void CWindowMenuConsole::Render() {

	if(bNoMenu)
		return;
	
	//------------------------------------------------------------------------
	// Console display
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);

	EERIEDrawBitmap2(Rectf(Vec2f(m_pos.x, m_pos.y),
	                 RATIO_X(pTexBackground->m_size.x), RATIO_Y(pTexBackground->m_size.y)),
	                 0, pTexBackground, Color::white);

	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	
	EERIEDrawBitmap2(Rectf(Vec2f(m_pos.x, m_pos.y),
	                 RATIO_X(pTexBackgroundBorder->m_size.x), RATIO_Y(pTexBackgroundBorder->m_size.y)),
	                 0, pTexBackgroundBorder, Color::white);

	//------------------------------------------------------------------------

	int iARXDiffTimeMenu  = checked_range_cast<int>(ARXDiffTimeMenu);

	for(size_t i = 0; i < MenuAllZone.GetNbZone(); ++i) {
		Widget * pMe = MenuAllZone.GetZoneNum(i);
		
		pMe->Update(iARXDiffTimeMenu);
		pMe->Render();
	}

	//HIGHLIGHT
	if(pZoneClick) {
		bool bReInit=false;

		pZoneClick->RenderMouseOver();

		switch(pZoneClick->eState) {
			case EDIT_TIME:
				UpdateText();
				break;
			case GETTOUCH_TIME: {
				if(bFrameOdd)
					((TextWidget*)pZoneClick)->lColorHighlight = Color(255, 0, 0);
				else
					((TextWidget*)pZoneClick)->lColorHighlight = Color(50, 0, 0);
				
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
				Widget *pmeElement = GetTouch(keyTouched, keyId, &inputKeyId, true);
				
				if(pmeElement) {
					if(UpdateGameKey(bEdit,pmeElement, inputKeyId)) {
						bReInit=true;
					}
				}
			}
			break;
			default:
			{
				if(GInput->getMouseButtonNowPressed(Mouse::Button_0)) {
					Widget *pmzMenuZone = MenuAllZone.GetZoneWithID(BUTTON_MENUOPTIONS_CONTROLS_CUST_DEFAULT);
					
					if(pmzMenuZone==pZoneClick) {
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
	
	//DEBUG ZONE
	MenuAllZone.DrawZone();
}

void CWindowMenuConsole::ReInitActionKey()
{
	int iID=BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP1;
	int iI=NUM_ACTION_KEY;
	
	while(iI--) {
		int iTab=(iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP1)>>1;

		Widget *pmzMenuZone = MenuAllZone.GetZoneWithID(iID);

		if(pmzMenuZone) {
			if(pmzMenuZone) {
				pZoneClick = pmzMenuZone;
				GetTouch(true, config.actions[iTab].key[0]);
			}

			pmzMenuZone = MenuAllZone.GetZoneWithID(iID+1);

			if(pmzMenuZone) {
				pZoneClick = pmzMenuZone;
				GetTouch(true, config.actions[iTab].key[1]);
			}
		}

		iID+=2;
	}
}





CycleTextWidget::CycleTextWidget(MenuButton _iID)
	: Widget(NOP)
{
	iID = _iID;
	
	pLeftButton = new ButtonWidget(Vec2i_ZERO, "graph/interface/menus/menu_slider_button_left");
	pRightButton = new ButtonWidget(Vec2i_ZERO, "graph/interface/menus/menu_slider_button_right");

	vText.clear();

	iPos = 0;
	iOldPos = -1;

	rZone.left   = 0;
	rZone.top    = 0;
	rZone.right  = pLeftButton->rZone.width() + pRightButton->rZone.width();
	rZone.bottom = std::max(pLeftButton->rZone.height(), pRightButton->rZone.height());

	pRef = this;
}

CycleTextWidget::~CycleTextWidget() {
	delete pLeftButton;
	delete pRightButton;
	BOOST_FOREACH(TextWidget * e, vText) {
		delete e;
	}
}

void CycleTextWidget::selectLast() {
	iPos = vText.size() - 1;
}

void CycleTextWidget::AddText(TextWidget *_pText) {
	
	_pText->setEnabled(enabled);
	
	_pText->Move(Vec2i(rZone.left + pLeftButton->rZone.width(), rZone.top + 0));
	vText.push_back(_pText);

	Vec2i textSize = _pText->rZone.size();

	rZone.right  = std::max(rZone.right, rZone.left + pLeftButton->rZone.width() + pRightButton->rZone.width() + textSize.x);
	rZone.bottom = std::max(rZone.bottom, rZone.top + textSize.y);

	pLeftButton->SetPos(Vec2i(rZone.left,
	                          rZone.top + rZone.height() / 2 - pLeftButton->rZone.height() / 2));
	pRightButton->SetPos(Vec2i(rZone.right - pRightButton->rZone.width(),
	                           rZone.top + rZone.height() / 2 - pRightButton->rZone.height() / 2));

	int dx=rZone.width()-pLeftButton->rZone.width()-pRightButton->rZone.width();
	//on recentre tout
	std::vector<TextWidget*>::iterator it;

	for(it = vText.begin(); it < vText.end(); ++it) {
		TextWidget *pMenuElementText=*it;
		
		textSize = pMenuElementText->rZone.size();

		int dxx=(dx-textSize.x)>>1;
		pMenuElementText->SetPos(Vec2i(pLeftButton->rZone.right + dxx, rZone.top + rZone.height() / 2 - textSize.y/2));
	}
}

void CycleTextWidget::Move(const Vec2i & offset) {

	Widget::Move(offset);

	pLeftButton->Move(offset);
	pRightButton->Move(offset);

	for(std::vector<TextWidget*>::const_iterator i = vText.begin(), i_end = vText.end(); i != i_end; ++i)
		(*i)->Move(offset);
}

void CycleTextWidget::EmptyFunction() {

	//Touche pour la selection
	if(GInput->isKeyPressedNowPressed(Keyboard::Key_LeftArrow)) {
		iPos--;

		if(iPos <= 0)
			iPos = 0;
	} else {
		if(GInput->isKeyPressedNowPressed(Keyboard::Key_RightArrow)) {
			iPos++;

			arx_assert(iPos >= 0);

			if((size_t)iPos >= vText.size() - 1)
				iPos = vText.size() - 1;
		}
	}
}

bool CycleTextWidget::OnMouseClick() {
	
	if(!enabled) {
		return false;
	}
	
	ARX_SOUND_PlayMenu(SND_MENU_CLICK);

	if(iOldPos<0)
		iOldPos=iPos;
	
	const Vec2i cursor = Vec2i(GInput->getMousePosAbs());

	if(rZone.contains(cursor)) {
		if(pLeftButton->rZone.contains(cursor)) {
			iPos--;

			if(iPos < 0) {
				iPos = vText.size() - 1;
			}
		}
		
		if(pRightButton->rZone.contains(cursor)) {
			iPos++;

			arx_assert(iPos >= 0);

			if(size_t(iPos) >= vText.size()) {
				iPos = 0;
			}
		}
	}

	switch(iID) {
		
		case BUTTON_MENUOPTIONSAUDIO_DEVICE: {
			if(iPos == 0) {
				ARXMenu_Options_Audio_SetDevice("auto");
			} else {
				ARXMenu_Options_Audio_SetDevice(vText.at(iPos)->lpszText);
			}
			break;
		}
		
		case BUTTON_MENUOPTIONSVIDEO_RESOLUTION: {
			std::string pcText = (vText.at(iPos))->lpszText;
			
			if(pcText == AUTO_RESOLUTION_STRING) {
				newWidth = newHeight = 0;
			} else {
				std::stringstream ss( pcText );
				int iX = config.video.resolution.x;
				int iY = config.video.resolution.y;
				char tmp;
				ss >> iX >> tmp >> iY;
				newWidth = iX;
				newHeight = iY;
			}
			break;
		}
		case BUTTON_MENUOPTIONSVIDEO_RENDERER: {
			switch((vText.at(iPos))->eMenuState) {
				case OPTIONS_VIDEO_RENDERER_OPENGL:    config.window.framework = "SDL"; break;
				case OPTIONS_VIDEO_RENDERER_AUTOMATIC: config.window.framework = "auto"; break;
				default: break;
			}
			break;
		}
		// MENUOPTIONS_VIDEO
		case BUTTON_MENUOPTIONSVIDEO_OTHERSDETAILS: {
			ARXMenu_Options_Video_SetDetailsQuality(iPos);
			break;
		}
		default:
			break;
	}
	
	return false;
}

void CycleTextWidget::Update(int _iTime) {

	pLeftButton->Update(_iTime);
	pRightButton->Update(_iTime);
}

void CycleTextWidget::Render() {
	
	if(bNoMenu)
		return;
	
	if(enabled) {
		pLeftButton->Render();
		pRightButton->Render();
	}
	
	if(iPos >= 0 && size_t(iPos) < vText.size() && vText[iPos]) {
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		vText[iPos]->Render();
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	}
}

void CycleTextWidget::setEnabled(bool enable) {
	Widget::setEnabled(enable);
	pLeftButton->setEnabled(enable);
	pRightButton->setEnabled(enable);
	for(size_t i = 0; i < vText.size(); i++) {
		vText[i]->setEnabled(enable);
	}
}

void CycleTextWidget::RenderMouseOver() {

	if(bNoMenu)
		return;

	pMenuCursor->SetMouseOver();

	Vec2i cursor = Vec2i(GInput->getMousePosAbs());
	
	if(!enabled) {
		return;
	}

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);

	if(rZone.contains(cursor)) {
		if(pLeftButton->rZone.contains(cursor)) {
			pLeftButton->Render();
		}
		
		if(pRightButton->rZone.contains(cursor)) {
			pRightButton->Render();
		}
	}
}


//-----------------------------------------------------------------------------
// CMenuSlider
//-----------------------------------------------------------------------------

SliderWidget::SliderWidget(MenuButton _iID, Vec2i pos)
	: Widget(NOP)
{
	iID = _iID;
	
	pLeftButton = new ButtonWidget(pos, "graph/interface/menus/menu_slider_button_left");
	pRightButton = new ButtonWidget(pos, "graph/interface/menus/menu_slider_button_right");
	pTex1 = TextureContainer::Load("graph/interface/menus/menu_slider_on");
	pTex2 = TextureContainer::Load("graph/interface/menus/menu_slider_off");
	arx_assert(pTex1);
	arx_assert(pTex2);
	
	m_value = 0;

	rZone.left   = pos.x;
	rZone.top    = pos.y;
	rZone.right  = pos.x + pLeftButton->rZone.width() + pRightButton->rZone.width() + 10 * std::max(pTex1->m_size.x, pTex2->m_size.x);
	rZone.bottom = pos.y + std::max(pLeftButton->rZone.height(), pRightButton->rZone.height());
	
	pRightButton->Move(Vec2i(pLeftButton->rZone.width() + 10 * std::max(pTex1->m_size.x, pTex2->m_size.x), 0));

	pRef = this;
}

SliderWidget::~SliderWidget() {
	delete pLeftButton;
	delete pRightButton;
}

void SliderWidget::Move(const Vec2i & offset) {
	Widget::Move(offset);
	pLeftButton->Move(offset);
	pRightButton->Move(offset);
}

void SliderWidget::EmptyFunction() {

	//Touche pour la selection
	if(GInput->isKeyPressedNowPressed(Keyboard::Key_LeftArrow)) {
		m_value--;

		if(m_value <= 0)
			m_value = 0;
	} else {
		if(GInput->isKeyPressedNowPressed(Keyboard::Key_RightArrow)) {
			m_value++;

			if(m_value >= 10)
				m_value = 10;
		}
	}
}

bool SliderWidget::OnMouseClick() {
	
	ARX_SOUND_PlayMenu(SND_MENU_CLICK);

	const Vec2i cursor = Vec2i(GInput->getMousePosAbs());
	
	if(rZone.contains(cursor)) {
		if(pLeftButton->rZone.contains(cursor)) {
			m_value--;
			if(m_value <= 0)
				m_value = 0;
		}
		
		if(pRightButton->rZone.contains(cursor)) {
			m_value++;
			if(m_value >= 10)
				m_value = 10;
		}
	}
	
	switch (iID) {
		// MENUOPTIONS_VIDEO
		case BUTTON_MENUOPTIONSVIDEO_FOG:
			ARXMenu_Options_Video_SetFogDistance(m_value);
			break;
		// MENUOPTIONS_AUDIO
		case BUTTON_MENUOPTIONSAUDIO_MASTER:
			ARXMenu_Options_Audio_SetMasterVolume(m_value);
			break;
		case BUTTON_MENUOPTIONSAUDIO_SFX:
			ARXMenu_Options_Audio_SetSfxVolume(m_value);
			break;
		case BUTTON_MENUOPTIONSAUDIO_SPEECH:
			ARXMenu_Options_Audio_SetSpeechVolume(m_value);
			break;
		case BUTTON_MENUOPTIONSAUDIO_AMBIANCE:
			ARXMenu_Options_Audio_SetAmbianceVolume(m_value);
			break;
		// MENUOPTIONS_CONTROLS
		case BUTTON_MENUOPTIONS_CONTROLS_MOUSESENSITIVITY:
			ARXMenu_Options_Control_SetMouseSensitivity(m_value);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_QUICKSAVESLOTS: {
			m_value = std::max(m_value, 1);
			config.misc.quicksaveSlots = m_value;
			break;
		}
		default:
			break;
	}

	return false;
}

void SliderWidget::Update(int _iTime) {
	
	pLeftButton->Update(_iTime);
	pRightButton->Update(_iTime);
	pRightButton->SetPos(rZone.topLeft());


	float fWidth = pLeftButton->rZone.width() + RATIO_X(10 * std::max(pTex1->m_size.x, pTex2->m_size.x)) ;
	pRightButton->Move(Vec2i(fWidth, 0));

	rZone.right = rZone.left + pLeftButton->rZone.width() + pRightButton->rZone.width() + RATIO_X(10*std::max(pTex1->m_size.x, pTex2->m_size.x));

	rZone.bottom = rZone.top + std::max(pLeftButton->rZone.height(), pRightButton->rZone.height());
}

void SliderWidget::Render() {

	if(bNoMenu)
		return;

	pLeftButton->Render();
	pRightButton->Render();

	Vec2f pos(rZone.left + pLeftButton->rZone.width(), rZone.top);
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	
	for(int i = 0; i < 10; i++) {
		TextureContainer * pTex = (i < m_value) ? pTex1 : pTex2;
		Rectf rect = Rectf(pos, RATIO_X(pTex->m_size.x), RATIO_Y(pTex->m_size.y));
		
		EERIEDrawBitmap2(rect, 0, pTex, Color::white);
		
		pos.x += rect.width();
	}

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}

void SliderWidget::RenderMouseOver() {

	if(bNoMenu)
		return;

	pMenuCursor->SetMouseOver();

	const Vec2i cursor = Vec2i(GInput->getMousePosAbs());

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	
	if(rZone.contains(cursor)) {
		if(pLeftButton->rZone.contains(cursor)) {
			pLeftButton->Render();
		}
		
		if(pRightButton->rZone.contains(cursor)) {
			pRightButton->Render();
		}
	}
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
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
