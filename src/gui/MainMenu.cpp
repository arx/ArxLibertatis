/*
 * Copyright 2014 Arx Libertatis Team (see the AUTHORS file)
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

#include "gui/MainMenu.h"

#include <iomanip>

#include "core/Application.h"
#include "core/Config.h"
#include "core/Core.h"
#include "core/Localisation.h"
#include "core/SaveGame.h"
#include "gui/MenuWidgets.h"
#include "gui/MenuPublic.h"
#include "gui/Text.h"
#include "graphics/Math.h"
#include "graphics/data/TextureContainer.h"
#include "input/Keyboard.h"
#include "window/RenderWindow.h"

CWindowMenu * pWindowMenu = NULL;
CMenuElementText * pDeleteConfirm = NULL;
CMenuElementText * pLoadConfirm = NULL;
CMenuElementText * pDeleteButton = NULL;
CMenuCheckButton * fullscreenCheckbox = NULL;
CMenuSliderText * pMenuSliderResol = NULL;
CMenuElement * pMenuElementApply = NULL;

void Menu2_Render_NewQuest(Vec2i size, Vec2i offset)
{
	if(!ARXMenu_CanResumeGame())
		return;
	
	std::string szMenuText;
	CMenuElement *me = NULL;
	
	CWindowMenuConsole * console = new CWindowMenuConsole(offset, size, NEW_QUEST);
	
	szMenuText = getLocalised("system_menus_main_editquest_confirm");
	me = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(0, 0), NOP);
	me->bCheck = false;
	console->AddMenuCenter(me);
	
	szMenuText = getLocalised("system_menus_main_newquest_confirm");
	me = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(0, 0), NOP);
	me->bCheck = false;
	console->AddMenuCenter(me);
	
	CMenuPanel *pPanel = new CMenuPanel();
	szMenuText = getLocalised("system_yes");
	szMenuText += "   "; // TODO This space can probably go
	me = new CMenuElementText(BUTTON_MENUNEWQUEST_CONFIRM, hFontMenu, szMenuText, Vec2i(0, 0), NEW_QUEST_ENTER_GAME);
	me->SetPos(Vec2i(RATIO_X(size.x - (me->rZone.width() + 10)), 0));
	pPanel->AddElementNoCenterIn(me);
	
	szMenuText = getLocalised("system_no");
	me = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(RATIO_X(10), 0), MAIN);
	me->SetShortCut(Keyboard::Key_Escape);
	pPanel->AddElementNoCenterIn(me);
	
	pPanel->Move(Vec2i(0, RATIO_Y(380)));

	console->AddMenu(pPanel);
	pWindowMenu->AddConsole(console);
	pWindowMenu->eCurrentMenuState=NEW_QUEST;
}

void Menu2_Render_EditQuest(Vec2i size, Vec2i offset)
{
	CMenuElement *me;
	CMenuElement *me01;
	CMenuPanel *pPanel;
	TextureContainer *pTex;
	std::string szMenuText;
	CWindowMenuConsole *pWindowMenuConsole=new CWindowMenuConsole(offset, size, EDIT_QUEST);

	szMenuText = getLocalised( "system_menus_main_editquest_load");
	me = new CMenuElementText(BUTTON_MENUEDITQUEST_LOAD_INIT, hFontMenu, szMenuText, Vec2i(0, 0), EDIT_QUEST_LOAD);
	me->lData = -1;
	pWindowMenuConsole->AddMenuCenter(me);

	szMenuText = getLocalised( "system_menus_main_editquest_save");
	me = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(0, 0), EDIT_QUEST_SAVE);
	
	if(!ARXMenu_CanResumeGame()) {
		me->SetCheckOff();
		((CMenuElementText*)me)->lColor=Color(127,127,127);
	}

	pWindowMenuConsole->AddMenuCenter(me);
	
	{
	pTex = TextureContainer::Load("graph/interface/menus/back");
	CMenuButton * cb = new CMenuButton(RATIO_2(Vec2i(20, 380)), pTex);
	cb->eMenuState = MAIN;
	cb->SetShortCut(Keyboard::Key_Escape);
	pWindowMenuConsole->AddMenu(cb);
	}

	pWindowMenu->eCurrentMenuState = EDIT_QUEST;
	pWindowMenu->AddConsole(pWindowMenuConsole);

	// LOAD ---------------------------------------------------
	pWindowMenuConsole=new CWindowMenuConsole(offset + Vec2i(0, -40), size, EDIT_QUEST_LOAD);
	pWindowMenuConsole->lData = -1;
	pWindowMenuConsole->iInterligne = 5;
	
	{
	pTex = TextureContainer::Load("graph/interface/icons/menu_main_load");
	CMenuButton * cb = new CMenuButton(Vec2i(0, 0), pTex);
	cb->bCheck = false;
	pWindowMenuConsole->AddMenuCenter(cb);
	}
	
	std::string quicksaveName = getLocalised("system_menus_main_quickloadsave", "Quicksave");
	
	// TODO make this list scrollable
	// TODO align the date part to the right!
	
	{
		size_t quicksaveNum = 0;
		
		// Show quicksaves.
		for(size_t i = 0; i < savegames.size(); i++) {
			const SaveGame & save = savegames[i];
			
			if(!save.quicksave) {
				continue;
			}
			
			std::ostringstream text;
			text << quicksaveName << ' ' << ++quicksaveNum << "   " << save.time;
			
			CMenuElement * e = new CMenuElementText(BUTTON_MENUEDITQUEST_LOAD, hFontControls, text.str(), Vec2i(RATIO_X(20), 0), NOP);
			e->lData = i;
			pWindowMenuConsole->AddMenuCenterY(e);
		}
		
		// Show regular saves.
		for(size_t i = 0; i < savegames.size(); i++) {
			const SaveGame & save = savegames[i];
			
			if(save.quicksave) {
				continue;
			}
			
			std::string text = save.name +  "   " + save.time;
			
			CMenuElement * e = new CMenuElementText(BUTTON_MENUEDITQUEST_LOAD, hFontControls, text, Vec2i(RATIO_X(20), 0), NOP);
			e->lData = i;
			pWindowMenuConsole->AddMenuCenterY(e);
		}
		
		CMenuElement * confirm = new CMenuElementText(-1, hFontControls, " ", Vec2i(RATIO_X(20), 0), EDIT_QUEST_SAVE_CONFIRM);
		confirm->SetCheckOff();
		confirm->lData = -1;
		pWindowMenuConsole->AddMenuCenterY(confirm);
		
		// Delete button
		szMenuText = getLocalised("system_menus_main_editquest_delete");
		szMenuText += "   ";
		me = new CMenuElementText(BUTTON_MENUEDITQUEST_DELETE_CONFIRM, hFontMenu, szMenuText, Vec2i(0, 0), EDIT_QUEST_LOAD);
		me->SetPos(Vec2i(RATIO_X(size.x-10)-me->rZone.width(), RATIO_Y(42)));
		pDeleteConfirm=(CMenuElementText*)me;
		me->SetCheckOff();
		((CMenuElementText*)me)->lOldColor = ((CMenuElementText*)me)->lColor;
		((CMenuElementText*)me)->lColor = Color::grayb(127);
		pWindowMenuConsole->AddMenu(me);
		
		// Load button
		szMenuText = getLocalised("system_menus_main_editquest_load");
		szMenuText += "   ";
		me = new CMenuElementText(BUTTON_MENUEDITQUEST_LOAD_CONFIRM, hFontMenu, szMenuText, Vec2i(0, 0), MAIN);
		me->SetPos(Vec2i(RATIO_X(size.x-10)-me->rZone.width(), RATIO_Y(380) + RATIO_Y(40)));
		pLoadConfirm=(CMenuElementText*)me;
		me->SetCheckOff();
		((CMenuElementText*)me)->lOldColor = ((CMenuElementText*)me)->lColor;
		((CMenuElementText*)me)->lColor = Color::grayb(127);
		pWindowMenuConsole->AddMenu(me);
		
		// Back button
		{
		pTex = TextureContainer::Load("graph/interface/menus/back");
		CMenuButton * cb = new CMenuButton(RATIO_2(Vec2i(20, 420)), pTex);
		cb->eMenuState = EDIT_QUEST;
		cb->SetShortCut(Keyboard::Key_Escape);
		pWindowMenuConsole->AddMenu(cb);
		}
	}
	pWindowMenu->AddConsole(pWindowMenuConsole);

	// SAVE----------------------------------------------------
	pWindowMenuConsole=new CWindowMenuConsole(offset + Vec2i(0, -40), size, EDIT_QUEST_SAVE);
	pWindowMenuConsole->iInterligne = 5;

	{
	pTex = TextureContainer::Load("graph/interface/icons/menu_main_save");
	CMenuCheckButton * cb = new CMenuCheckButton(Vec2i(RATIO_X(10), 0), pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
	cb->bCheck = false;
	pWindowMenuConsole->AddMenuCenter(cb);
	}
	
	size_t quicksaveNum = 0;
	
	// Show quicksaves.
	for(size_t i = 0; i < savegames.size(); i++) {
		const SaveGame & save = savegames[i];
		
		if(!save.quicksave) {
			continue;
		}
		
		std::ostringstream text;
		text << quicksaveName << ' ' << ++quicksaveNum << "   " << save.time;
		
		CMenuElementText * e = new CMenuElementText(BUTTON_MENUEDITQUEST_SAVEINFO, hFontControls,
		                                        text.str(), Vec2i(RATIO_X(20), 0.f), EDIT_QUEST_SAVE_CONFIRM);
		e->setColor(Color::grayb(127));
		e->SetCheckOff();
		e->lData = i;
		pWindowMenuConsole->AddMenuCenterY(e);
	}
	
	// Show regular saves.
	for(size_t i = 0; i < savegames.size(); i++) {
		const SaveGame & save = savegames[i];
		
		if(save.quicksave) {
			continue;
		}
		
		std::string text = save.name +  "   " + save.time;
		
		CMenuElement * e = new CMenuElementText(BUTTON_MENUEDITQUEST_SAVEINFO, hFontControls,
		                                        text, Vec2i(RATIO_X(20), 0.f), EDIT_QUEST_SAVE_CONFIRM);
		e->lData = i;
		pWindowMenuConsole->AddMenuCenterY(e);
	}
	
	for(size_t i = savegames.size(); i <= 15; i++) {
		
		std::ostringstream text;
		text << '-' << std::setfill('0') << std::setw(4) << i << '-';
		
		CMenuElementText * e = new CMenuElementText(BUTTON_MENUEDITQUEST_SAVEINFO, hFontControls, text.str(), Vec2i(RATIO_X(20), 0), EDIT_QUEST_SAVE_CONFIRM);

		e->eMenuState = EDIT_QUEST_SAVE_CONFIRM;
		e->lData = -1;
		pWindowMenuConsole->AddMenuCenterY(e);
	}

	me01 = new CMenuElementText(-1, hFontControls, " ", Vec2i(RATIO_X(20), 0), EDIT_QUEST_SAVE_CONFIRM);
	me01->lData = -1;
	me01->SetCheckOff();
	pWindowMenuConsole->AddMenuCenterY((CMenuElementText*)me01);
	
	{
	pTex = TextureContainer::Load("graph/interface/menus/back");
	CMenuCheckButton * cb = new CMenuCheckButton(RATIO_2(Vec2i(10, 190)) + Vec2i(0, RATIO_Y(20)), pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
	cb->eMenuState = EDIT_QUEST;
	cb->SetShortCut(Keyboard::Key_Escape);
	pWindowMenuConsole->AddMenu(cb);
	}

	pWindowMenu->AddConsole(pWindowMenuConsole);

	// SAVE CONFIRM--------------------------------------------
	pWindowMenuConsole = new CWindowMenuConsole(offset, size, EDIT_QUEST_SAVE_CONFIRM);
	pWindowMenuConsole->lData = -1;
	
	{
	pTex = TextureContainer::Load("graph/interface/icons/menu_main_save");
	CMenuCheckButton * cb = new CMenuCheckButton(Vec2i(0, 0), pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
	cb->bCheck = false;
	pWindowMenuConsole->AddMenuCenter(cb);
	}
	
	szMenuText = getLocalised("system_menu_editquest_newsavegame", "---");

	me = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(RATIO_X(20), 0), NOP);
	me->lData = -1;

	pWindowMenuConsole->AddMenuCenter(me);
	me->eState=EDIT;
	me->ePlace=CENTER;

	pPanel = new CMenuPanel();
	
	// Delete button
	szMenuText = getLocalised("system_menus_main_editquest_delete");
	me = new CMenuElementText(BUTTON_MENUEDITQUEST_DELETE, hFontMenu, szMenuText, Vec2i(0, 0), EDIT_QUEST_SAVE);
	me->SetPos(Vec2i(RATIO_X(size.x-10)-me->rZone.width(), RATIO_Y(5)));
	pDeleteButton = (CMenuElementText*)me;
	((CMenuElementText*)me)->lOldColor = ((CMenuElementText*)me)->lColor;
	pPanel->AddElementNoCenterIn(me);
	
	// Save button
	szMenuText = getLocalised("system_menus_main_editquest_save");
	me = new CMenuElementText(BUTTON_MENUEDITQUEST_SAVE, hFontMenu, szMenuText, Vec2i(0, 0), MAIN);
	me->SetPos(Vec2i(RATIO_X(size.x-10)-me->rZone.width(), RATIO_Y(380)));
	pPanel->AddElementNoCenterIn(me);
	
	// Back button
	{
	pTex = TextureContainer::Load("graph/interface/menus/back");
	CMenuCheckButton * cb = new CMenuCheckButton(RATIO_2(Vec2i(10, 190)), pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
	cb->eMenuState = EDIT_QUEST_SAVE;
	cb->SetShortCut(Keyboard::Key_Escape);
	pPanel->AddElementNoCenterIn(cb);
	}

	pWindowMenuConsole->AddMenu(pPanel);

	pWindowMenu->AddConsole(pWindowMenuConsole);
}

void MainMenuOptionGroupsCreate(Vec2i size, Vec2i offset)
{
	std::string szMenuText;
	CMenuElement *me;
	TextureContainer *pTex;
	
	CWindowMenuConsole *pWindowMenuConsole=new CWindowMenuConsole(offset, size, OPTIONS);

	szMenuText = getLocalised("system_menus_options_video");
	me = new CMenuElementText(BUTTON_MENUOPTIONSVIDEO_INIT, hFontMenu, szMenuText, Vec2i(0, 0), OPTIONS_VIDEO);
	pWindowMenuConsole->AddMenuCenter(me);
	
	szMenuText = getLocalised("system_menus_options_audio");
	me = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(0, 0), OPTIONS_AUDIO);
	pWindowMenuConsole->AddMenuCenter(me);
	
	szMenuText = getLocalised("system_menus_options_input");
	me = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(0, 0), OPTIONS_INPUT);
	pWindowMenuConsole->AddMenuCenter(me);
	
	{
	pTex = TextureContainer::Load("graph/interface/menus/back");
	CMenuCheckButton * cb = new CMenuCheckButton(RATIO_2(Vec2i(10, 190)), pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
	cb->eMenuState = MAIN;
	cb->SetShortCut(Keyboard::Key_Escape);
	pWindowMenuConsole->AddMenu(cb);
	}

	pWindowMenu->AddConsole(pWindowMenuConsole);
}

void MainMenuOptionVideoCreate(Vec2i offset, Vec2i size)
{
	std::string szMenuText;
	CMenuElement *me;
	CMenuPanel *pc;
	TextureContainer *pTex;
	
	CWindowMenuConsole * pWindowMenuConsole=new CWindowMenuConsole(offset + Vec2i(0, -35), size, OPTIONS_VIDEO);
	
	// Renderer selection
	{
		
		pc = new CMenuPanel();
		szMenuText = getLocalised("system_menus_options_video_renderer", "Renderer");
		szMenuText += "  ";
		me = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(RATIO_X(20), 0), NOP);
		me->SetCheckOff();
		pc->AddElement(me);
		CMenuSliderText * slider = new CMenuSliderText(BUTTON_MENUOPTIONSVIDEO_RENDERER, Vec2i(0, 0));
		
		slider->AddText(new CMenuElementText(-1, hFontMenu, "Auto-Select", Vec2i(0, 0), OPTIONS_VIDEO_RENDERER_AUTOMATIC));
		slider->selectLast();
#if ARX_HAVE_SDL1 || ARX_HAVE_SDL2
		slider->AddText(new CMenuElementText(-1, hFontMenu, "OpenGL", Vec2i(0, 0), OPTIONS_VIDEO_RENDERER_OPENGL));
		if(config.window.framework == "SDL") {
			slider->selectLast();
		}
#endif
		
		float fRatio    = (RATIO_X(size.x-9) - slider->rZone.width()); 
		slider->Move(Vec2i(checked_range_cast<int>(fRatio), 0));
		pc->AddElement(slider);
		pWindowMenuConsole->AddMenuCenterY(pc);
		
	}
	
	{
	szMenuText = getLocalised("system_menus_options_videos_full_screen");
	if(szMenuText.empty()) {
		// TODO once we ship our own amendmends to the loc files a cleaner
		// fix would be to just define system_menus_options_videos_full_screen
		// for the german version there
		szMenuText = getLocalised("system_menus_options_video_full_screen");
	}
	szMenuText += "  ";
	CMenuElementText * text = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(RATIO_X(20), 0.f), NOP);
	text->SetCheckOff();
	TextureContainer *pTex1 = TextureContainer::Load("graph/interface/menus/menu_checkbox_off");
	TextureContainer *pTex2 = TextureContainer::Load("graph/interface/menus/menu_checkbox_on");
	CMenuCheckButton * cb = new CMenuCheckButton(Vec2i(0, 0), pTex1->m_dwWidth, pTex1, pTex2, text);
	cb->iID = BUTTON_MENUOPTIONSVIDEO_FULLSCREEN;
	cb->iState = config.video.fullscreen ? 1 : 0;
	pWindowMenuConsole->AddMenuCenterY(cb);
	fullscreenCheckbox = cb;
	}
	
	pc = new CMenuPanel();
	szMenuText = getLocalised("system_menus_options_video_resolution");
	szMenuText += "  ";
	me = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(RATIO_X(20), 0.f), NOP);
	me->SetCheckOff();
	pc->AddElement(me);
	pMenuSliderResol = new CMenuSliderText(BUTTON_MENUOPTIONSVIDEO_RESOLUTION, Vec2i(0, 0));
	
	pMenuSliderResol->setEnabled(config.video.fullscreen);
	
	const RenderWindow::DisplayModes & modes = mainApp->getWindow()->getDisplayModes();
	for(size_t i = 0; i != modes.size(); ++i) {
		
		const DisplayMode & mode = modes[i];
		
		// find the aspect ratio
		unsigned a = mode.resolution.x;
		unsigned b = mode.resolution.y;
		while(b != 0) {
			unsigned t = a % b;
			a = b, b = t;
		}
		Vec2i aspect = mode.resolution / Vec2i(a);
		
		std::stringstream ss;
		ss << mode;
		
		if(aspect.x < 100 && aspect.y < 100) {
			if(aspect == Vec2i(8, 5)) {
				aspect = Vec2i(16, 10);
			}
			ss << " (" << aspect.x << ':' << aspect.y << ')';
		}
		
		pMenuSliderResol->AddText(new CMenuElementText(-1, hFontMenu, ss.str(), Vec2i(0, 0), MENUSTATE(OPTIONS_VIDEO_RESOLUTION_0 + i)));
		
		if(mode.resolution == config.video.resolution) {
			pMenuSliderResol->selectLast();
		}
	}
	
	pMenuSliderResol->AddText(new CMenuElementText(-1, hFontMenu, AUTO_RESOLUTION_STRING, Vec2i(0, 0), MENUSTATE(OPTIONS_VIDEO_RESOLUTION_0 + modes.size())));
	
	if(config.video.resolution == Vec2i_ZERO) {
		pMenuSliderResol->selectLast();
	}

	float fRatio    = (RATIO_X(size.x-9) - pMenuSliderResol->rZone.width()); 

	pMenuSliderResol->Move(Vec2i(checked_range_cast<int>(fRatio), 0));


	pc->AddElement(pMenuSliderResol);

	pWindowMenuConsole->AddMenuCenterY(pc);

	pc = new CMenuPanel();
	szMenuText = getLocalised("system_menus_options_detail");
	szMenuText += " ";
	me = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(RATIO_X(20), 0), NOP);
	me->SetCheckOff();
	pc->AddElement(me);
	me = new CMenuSliderText(BUTTON_MENUOPTIONSVIDEO_OTHERSDETAILS, Vec2i(0, 0));
	szMenuText = getLocalised("system_menus_options_video_texture_low");
	((CMenuSliderText *)me)->AddText(new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(0, 0), OPTIONS_OTHERDETAILS));
	szMenuText = getLocalised("system_menus_options_video_texture_med");
	((CMenuSliderText *)me)->AddText(new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(0, 0), OPTIONS_OTHERDETAILS));
	szMenuText = getLocalised("system_menus_options_video_texture_high");
	((CMenuSliderText *)me)->AddText(new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(0, 0), OPTIONS_OTHERDETAILS));

	
	fRatio    = (RATIO_X(size.x-9) - me->rZone.width()); 
	me->Move(Vec2i(checked_range_cast<int>(fRatio), 0));


	pc->AddElement(me);
	((CMenuSliderText *)me)->setValue(config.video.levelOfDetail);

	pWindowMenuConsole->AddMenuCenterY(pc);
	
	pc = new CMenuPanel();
	szMenuText = getLocalised("system_menus_options_video_brouillard");
	me = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(RATIO_X(20), 0.f), NOP);
	me->SetCheckOff();
	pc->AddElement(me);
	me = new CMenuSlider(BUTTON_MENUOPTIONSVIDEO_FOG, Vec2i(RATIO_X(200), 0));
	((CMenuSlider *)me)->setValue(config.video.fogDistance);
	pc->AddElement(me);

	pWindowMenuConsole->AddMenuCenterY(pc);
	
	{
	szMenuText = getLocalised("system_menus_options_video_crosshair", "Show Crosshair");
	szMenuText += " ";
	CMenuElementText * text = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(RATIO_X(20), 0.f), NOP);
	text->SetCheckOff();
	TextureContainer *pTex1 = TextureContainer::Load("graph/interface/menus/menu_checkbox_off");
	TextureContainer *pTex2 = TextureContainer::Load("graph/interface/menus/menu_checkbox_on");
	CMenuCheckButton * cb = new CMenuCheckButton(Vec2i(0, 0), pTex1->m_dwWidth, pTex1, pTex2, text);
	cb->iID = BUTTON_MENUOPTIONSVIDEO_CROSSHAIR;
	cb->iState = config.video.showCrosshair ? 1 : 0;
	pWindowMenuConsole->AddMenuCenterY(cb);
	}
	
	{
	szMenuText = getLocalised("system_menus_options_video_antialiasing", "antialiasing");
	szMenuText += " ";
	CMenuElementText * text = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(RATIO_X(20), 0), NOP);
	text->SetCheckOff();
	TextureContainer *pTex1 = TextureContainer::Load("graph/interface/menus/menu_checkbox_off");
	TextureContainer *pTex2 = TextureContainer::Load("graph/interface/menus/menu_checkbox_on");
	CMenuCheckButton * cb = new CMenuCheckButton(Vec2i(0, 0), pTex1->m_dwWidth, pTex1, pTex2, text);
	cb->iID = BUTTON_MENUOPTIONSVIDEO_ANTIALIASING;
	cb->iState = config.video.antialiasing ? 1 : 0;
	pWindowMenuConsole->AddMenuCenterY(cb);
	}
	
	ARX_SetAntiAliasing();
	
	{
	szMenuText = getLocalised("system_menus_options_video_vsync", "VSync");
	szMenuText += " ";
	CMenuElementText * text = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(RATIO_X(20), 0), NOP);
	text->SetCheckOff();
	TextureContainer *pTex1 = TextureContainer::Load("graph/interface/menus/menu_checkbox_off");
	TextureContainer *pTex2 = TextureContainer::Load("graph/interface/menus/menu_checkbox_on");
	CMenuCheckButton * cb = new CMenuCheckButton(Vec2i(0, 0), pTex1->m_dwWidth, pTex1, pTex2, text);
	cb->iID = BUTTON_MENUOPTIONSVIDEO_VSYNC;
	cb->iState = config.video.vsync ? 1 : 0;
	pWindowMenuConsole->AddMenuCenterY(cb);
	}

	float fPosApply = RATIO_X(240);
	
	pc = new CMenuPanel();
	szMenuText = getLocalised("system_menus_video_apply");
	szMenuText += "   ";
	pMenuElementApply = me = new CMenuElementText(BUTTON_MENUOPTIONSVIDEO_APPLY, hFontMenu, szMenuText, Vec2i(fPosApply, 0), NOP);
	me->SetPos(Vec2i(RATIO_X(size.x-10)-me->rZone.width(), RATIO_Y(380) + RATIO_Y(40)));
	me->SetCheckOff();
	pc->AddElementNoCenterIn(me);
	
	{
	pTex = TextureContainer::Load("graph/interface/menus/back");
	CMenuCheckButton * cb = new CMenuCheckButton(RATIO_2(Vec2i(10, 190)) + Vec2i(0, RATIO_Y(20)), pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
	cb->iID = BUTTON_MENUOPTIONSVIDEO_BACK;
	cb->eMenuState = OPTIONS;
	cb->SetShortCut(Keyboard::Key_Escape);
	pc->AddElementNoCenterIn(cb);
	}

	pWindowMenuConsole->AddMenu(pc);

	pWindowMenu->AddConsole(pWindowMenuConsole);
}

void MainMenuOptionAudioCreate(Vec2i offset, Vec2i size)
{
	std::string szMenuText;
	CMenuElement *me;
	CMenuPanel *pc;
	TextureContainer *pTex;
	TextureContainer *pTex1;
	TextureContainer *pTex2;
	
	CWindowMenuConsole * pWindowMenuConsole = new CWindowMenuConsole(offset, size, OPTIONS_AUDIO);

	// Audio backend selection
	{
		
		pc = new CMenuPanel();
		szMenuText = getLocalised("system_menus_options_audio_backend", "Backend");
		szMenuText += "  ";
		me = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(RATIO_X(20), 0), NOP);
		me->SetCheckOff();
		pc->AddElement(me);
		CMenuSliderText * slider = new CMenuSliderText(BUTTON_MENUOPTIONSAUDIO_BACKEND, Vec2i(0, 0));
		
		slider->AddText(new CMenuElementText(-1, hFontMenu, "Auto-Select", Vec2i(0, 0), OPTIONS_AUDIO_BACKEND_AUTOMATIC));
		slider->selectLast();
#if ARX_HAVE_OPENAL
		slider->AddText(new CMenuElementText(-1, hFontMenu, "OpenAL", Vec2i(0, 0), OPTIONS_AUDIO_BACKEND_OPENAL));
		if(config.audio.backend == "OpenAL") {
			slider->selectLast();
		}
#endif
		
		float fRatio    = (RATIO_X(size.x-9) - slider->rZone.width()); 
		slider->Move(Vec2i(checked_range_cast<int>(fRatio), 0));
		pc->AddElement(slider);
		pWindowMenuConsole->AddMenuCenterY(pc);
		
	}
	
	pc = new CMenuPanel();
	szMenuText = getLocalised("system_menus_options_audio_master_volume");
	me = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(RATIO_X(20), 0.f), OPTIONS_AUDIO_VOLUME);
	me->SetCheckOff();
	pc->AddElement(me);
	me = new CMenuSlider(BUTTON_MENUOPTIONSAUDIO_MASTER, Vec2i(RATIO_X(200), 0));
	((CMenuSlider *)me)->setValue((int)config.audio.volume); // TODO use float sliders
	pc->AddElement(me);
	pWindowMenuConsole->AddMenuCenterY(pc);

	pc = new CMenuPanel();
	szMenuText = getLocalised("system_menus_options_audio_effects_volume");
	me = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(RATIO_X(20), 0.f), OPTIONS_AUDIO);
	me->SetCheckOff();
	pc->AddElement(me);
	me = new CMenuSlider(BUTTON_MENUOPTIONSAUDIO_SFX, Vec2i(RATIO_X(200), 0));
	((CMenuSlider *)me)->setValue((int)config.audio.sfxVolume);
	pc->AddElement(me);
	pWindowMenuConsole->AddMenuCenterY(pc);

	pc = new CMenuPanel();
	szMenuText = getLocalised("system_menus_options_audio_speech_volume");
	me = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(RATIO_X(20), 0.f), OPTIONS_AUDIO);
	me->SetCheckOff();
	pc->AddElement(me);
	me = new CMenuSlider(BUTTON_MENUOPTIONSAUDIO_SPEECH, Vec2i(RATIO_X(200), 0));
	((CMenuSlider *)me)->setValue((int)config.audio.speechVolume);
	pc->AddElement(me);
	pWindowMenuConsole->AddMenuCenterY(pc);

	pc = new CMenuPanel();
	szMenuText = getLocalised("system_menus_options_audio_ambiance_volume");
	me = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(RATIO_X(20), 0), OPTIONS_AUDIO);
	me->SetCheckOff();
	pc->AddElement(me);
	me = new CMenuSlider(BUTTON_MENUOPTIONSAUDIO_AMBIANCE, Vec2i(RATIO_X(200), 0));
	((CMenuSlider *)me)->setValue((int)config.audio.ambianceVolume);
	pc->AddElement(me);
	pWindowMenuConsole->AddMenuCenterY(pc);
	
	{
	szMenuText = getLocalised("system_menus_options_audio_eax", "EAX");
	szMenuText += " ";
	CMenuElementText * text = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(RATIO_X(20), 0), OPTIONS_INPUT);
	pTex1 = TextureContainer::Load("graph/interface/menus/menu_checkbox_off");
	pTex2 = TextureContainer::Load("graph/interface/menus/menu_checkbox_on");
	CMenuCheckButton * cb = new CMenuCheckButton(Vec2i(0, 0), pTex1->m_dwWidth, pTex1, pTex2, text);
	cb->iID = BUTTON_MENUOPTIONSAUDIO_EAX;
	cb->iState = config.audio.eax ? 1 : 0;
	pWindowMenuConsole->AddMenuCenterY(cb);
	}
	
	{
	pTex = TextureContainer::Load("graph/interface/menus/back");
	CMenuCheckButton * cb = new CMenuCheckButton(RATIO_2(Vec2i(10, 190)), pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
	cb->eMenuState = OPTIONS;
	cb->SetShortCut(Keyboard::Key_Escape);
	pWindowMenuConsole->AddMenu(cb);
	}

	pWindowMenu->AddConsole(pWindowMenuConsole);
}

void MainMenuOptionInputCreate(Vec2i size, Vec2i offset)
{
	std::string szMenuText;
	CMenuElement *me;
	CMenuPanel *pc;
	TextureContainer *pTex;
	TextureContainer *pTex1;
	TextureContainer *pTex2;
	
	CWindowMenuConsole * pWindowMenuConsole = new CWindowMenuConsole(offset, size, OPTIONS_INPUT);
	
	szMenuText = getLocalised("system_menus_options_input_customize_controls");
	me = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(RATIO_X(20), 0), OPTIONS_INPUT_CUSTOMIZE_KEYS_1);
	pWindowMenuConsole->AddMenuCenterY(me);
	
	{
	szMenuText = getLocalised("system_menus_options_input_invert_mouse");
	szMenuText += " ";
	CMenuElementText * text = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(RATIO_X(20), 0), OPTIONS_INPUT);
	pTex1 = TextureContainer::Load("graph/interface/menus/menu_checkbox_off");
	pTex2 = TextureContainer::Load("graph/interface/menus/menu_checkbox_on");
	CMenuCheckButton * cb = new CMenuCheckButton(Vec2i(0, 0), pTex1->m_dwWidth, pTex1, pTex2, text);
	cb->iID = BUTTON_MENUOPTIONS_CONTROLS_INVERTMOUSE;
	cb->iState = config.input.invertMouse ? 1 : 0;
	pWindowMenuConsole->AddMenuCenterY(cb);
	}
	
	{
	szMenuText = getLocalised("system_menus_options_auto_ready_weapon");
	szMenuText += " ";
	CMenuElementText * text = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(RATIO_X(20), 0), OPTIONS_INPUT);
	pTex1 = TextureContainer::Load("graph/interface/menus/menu_checkbox_off");
	pTex2 = TextureContainer::Load("graph/interface/menus/menu_checkbox_on");
	CMenuCheckButton * cb = new CMenuCheckButton(Vec2i(0, 0), pTex1->m_dwWidth, pTex1, pTex2, text);
	cb->iID = BUTTON_MENUOPTIONS_CONTROLS_AUTOREADYWEAPON;
	cb->iState = config.input.autoReadyWeapon ? 1 : 0;
	pWindowMenuConsole->AddMenuCenterY(cb);
	}

	{
	szMenuText = getLocalised("system_menus_options_input_mouse_look_toggle");
	szMenuText += " ";
	CMenuElementText * text = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(RATIO_X(20), 0.f), OPTIONS_INPUT);
	pTex1 = TextureContainer::Load("graph/interface/menus/menu_checkbox_off");
	pTex2 = TextureContainer::Load("graph/interface/menus/menu_checkbox_on");
	CMenuCheckButton * cb = new CMenuCheckButton(Vec2i(0, 0), pTex1->m_dwWidth, pTex1, pTex2, text);
	cb->iID = BUTTON_MENUOPTIONS_CONTROLS_MOUSELOOK;
	cb->iState = config.input.mouseLookToggle ? 1 : 0;
	pWindowMenuConsole->AddMenuCenterY(cb);
	}

	pc = new CMenuPanel();
	szMenuText = getLocalised("system_menus_options_input_mouse_sensitivity");
	me = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(RATIO_X(20), 0.f), NOP);
	me->SetCheckOff();
	pc->AddElement(me);
	me = new CMenuSlider(BUTTON_MENUOPTIONS_CONTROLS_MOUSESENSITIVITY, Vec2i(RATIO_X(200), 0));
	((CMenuSlider*)me)->setValue(config.input.mouseSensitivity);
	pc->AddElement(me);
	pWindowMenuConsole->AddMenuCenterY(pc);
	
	{
	szMenuText = getLocalised("system_menus_autodescription", "auto_description");
	szMenuText += " ";
	CMenuElementText * text = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(RATIO_X(20), 0), OPTIONS_INPUT);
	pTex1 = TextureContainer::Load("graph/interface/menus/menu_checkbox_off");
	pTex2 = TextureContainer::Load("graph/interface/menus/menu_checkbox_on");
	CMenuCheckButton * cb = new CMenuCheckButton(Vec2i(0, 0), pTex1->m_dwWidth, pTex1, pTex2, text);
	cb->iID = BUTTON_MENUOPTIONS_CONTROLS_AUTODESCRIPTION;
	cb->iState = config.input.autoDescription ? 1 : 0;
	pWindowMenuConsole->AddMenuCenterY(cb);
	}

	pc = new CMenuPanel();
	szMenuText = getLocalised("system_menus_options_misc_quicksave_slots", "Quicksave slots");
	me = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(RATIO_X(20), 0), NOP);
	me->SetCheckOff();
	pc->AddElement(me);
	me = new CMenuSlider(BUTTON_MENUOPTIONS_CONTROLS_QUICKSAVESLOTS, Vec2i(RATIO_X(200), 0));
	((CMenuSlider*)me)->setValue(config.misc.quicksaveSlots);
	pc->AddElement(me);
	pWindowMenuConsole->AddMenuCenterY(pc);
	
	{
	pTex = TextureContainer::Load("graph/interface/menus/back");
	CMenuCheckButton * cb = new CMenuCheckButton(RATIO_2(Vec2i(10, 190)), pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
	cb->eMenuState = OPTIONS;
	cb->SetShortCut(Keyboard::Key_Escape);
	pWindowMenuConsole->AddMenu(cb);
	}
	
	pWindowMenu->AddConsole(pWindowMenuConsole);
}

void MainMenuOptionControlsCreate(Vec2i offset, Vec2i size)
{
	std::string szMenuText;
	CMenuElement *me;
	CMenuPanel *pc;
	TextureContainer *pTex;
	
char pNoDef1[]="---";
char pNoDef2[]="---";

#define CUSTOM_CTRL_X0    RATIO_X(20)
#define CUSTOM_CTRL_X1    RATIO_X(150)
#define CUSTOM_CTRL_X2    RATIO_X(245)
	long fControlPosY    =    static_cast<long>(RATIO_Y(8.f));
#define CUSTOM_CTRL_FUNC(a,b,c,d){\
		pc=new CMenuPanel();\
		szMenuText = getLocalised(a, "?");\
		me = new CMenuElementText(-1, hFontControls, szMenuText, Vec2i(CUSTOM_CTRL_X0, 0), NOP);\
		me->SetCheckOff();\
		pc->AddElement(me);\
		me = new CMenuElementText(c, hFontControls, pNoDef1, Vec2i(CUSTOM_CTRL_X1, 0), NOP);\
		me->eState=GETTOUCH;\
		if((!b)||(c<0))\
		{\
			me->SetCheckOff();\
			((CMenuElementText*)me)->lColor=Color(127,127,127);\
		}\
		pc->AddElement(me);\
		me = new CMenuElementText(d, hFontControls, pNoDef2, Vec2i(CUSTOM_CTRL_X2, 0), NOP);\
		me->eState=GETTOUCH;\
		if(d<0)\
		{\
			me->SetCheckOff();\
			((CMenuElementText*)me)->lColor=Color(127,127,127);\
		}\
		pc->AddElement(me);\
		pc->Move(Vec2i(0, fControlPosY));\
		pWindowMenuConsole->AddMenu(pc);\
		fControlPosY += static_cast<long>( pc->rZone.height() + RATIO_Y(3.f) );\
	};


#define CUSTOM_CTRL_FUNC2(a,b,c,d){\
		pc=new CMenuPanel();\
		szMenuText = getLocalised(a, "?");\
		szMenuText += "2";\
		me = new CMenuElementText(-1, hFontControls, szMenuText, Vec2i(CUSTOM_CTRL_X0, 0), NOP);\
		me->SetCheckOff();\
		pc->AddElement(me);\
		me = new CMenuElementText(c, hFontControls, pNoDef1, Vec2i(CUSTOM_CTRL_X1, 0), NOP);\
		me->eState=GETTOUCH;\
		if((!b)||(c<0))\
		{\
			me->SetCheckOff();\
			((CMenuElementText*)me)->lColor=Color(127,127,127);\
		}\
		pc->AddElement(me);\
		me = new CMenuElementText(d, hFontControls, pNoDef2, Vec2i(CUSTOM_CTRL_X2, 0), NOP);\
		me->eState=GETTOUCH;\
		if(d<0)\
		{\
			me->SetCheckOff();\
			((CMenuElementText*)me)->lColor=Color(127,127,127);\
		}\
		pc->AddElement(me);\
		pc->Move(Vec2i(0, fControlPosY));\
		pWindowMenuConsole->AddMenu(pc);\
		fControlPosY += static_cast<long>( pc->rZone.height() + RATIO_Y(3.f) );\
	};
	
	
#define CUSTOM_CTRL_FUNC3(a,default,b,c,d){\
		pc=new CMenuPanel();\
		szMenuText = getLocalised(a, default);\
		me = new CMenuElementText(-1, hFontControls, szMenuText, Vec2i(CUSTOM_CTRL_X0, 0), NOP);\
		me->SetCheckOff();\
		pc->AddElement(me);\
		me = new CMenuElementText(c, hFontControls, pNoDef1, Vec2i(CUSTOM_CTRL_X1, 0), NOP);\
		me->eState=GETTOUCH;\
		if((!b)||(c<0))\
		{\
			me->SetCheckOff();\
			((CMenuElementText*)me)->lColor=Color(127,127,127);\
		}\
		pc->AddElement(me);\
		me = new CMenuElementText(d, hFontControls, pNoDef2, Vec2i(CUSTOM_CTRL_X2, 0), NOP);\
		me->eState=GETTOUCH;\
		if(d<0)\
		{\
			me->SetCheckOff();\
			((CMenuElementText*)me)->lColor=Color(127,127,127);\
		}\
		pc->AddElement(me);\
		pc->Move(Vec2i(0, fControlPosY));\
		pWindowMenuConsole->AddMenu(pc);\
		fControlPosY += static_cast<long>( pc->rZone.height() + RATIO_Y(3.f) );\
	};


	CWindowMenuConsole * pWindowMenuConsole=new CWindowMenuConsole(offset, size, OPTIONS_INPUT_CUSTOMIZE_KEYS_1);

	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_mouselook",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_USE1, BUTTON_MENUOPTIONS_CONTROLS_CUST_USE2);

	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_action_combine",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_ACTIONCOMBINE1, BUTTON_MENUOPTIONS_CONTROLS_CUST_ACTIONCOMBINE2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_jump",1,BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP1, BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_magic_mode",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_MAGICMODE1, BUTTON_MENUOPTIONS_CONTROLS_CUST_MAGICMODE2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_stealth_mode",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_STEALTHMODE1, BUTTON_MENUOPTIONS_CONTROLS_CUST_STEALTHMODE2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_walk_forward",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKFORWARD1, BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKFORWARD2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_walk_backward",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKBACKWARD1, BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKBACKWARD2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_strafe_left",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFELEFT1, BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFELEFT2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_strafe_right",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFERIGHT1, BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFERIGHT2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_lean_left",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANLEFT1, BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANLEFT2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_lean_right",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANRIGHT1, BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANRIGHT2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_crouch",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCH1, BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCH2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_crouch_toggle",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCHTOGGLE1, BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCHTOGGLE2);

	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_strafe",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFE1, BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFE2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_center_view",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_CENTERVIEW1, BUTTON_MENUOPTIONS_CONTROLS_CUST_CENTERVIEW2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_freelook",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_FREELOOK1, BUTTON_MENUOPTIONS_CONTROLS_CUST_FREELOOK2);

	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_turn_left",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNLEFT1, BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNLEFT2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_turn_right",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNRIGHT1, BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNRIGHT2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_look_up",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKUP1, BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKUP2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_look_down",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKDOWN1, BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKDOWN2);

	pc=new CMenuPanel();
	
	{
	pTex = TextureContainer::Load("graph/interface/menus/back");
	CMenuCheckButton * cb = new CMenuCheckButton(RATIO_2(Vec2i(10, 190)), pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
	cb->iID = BUTTON_MENUOPTIONS_CONTROLS_CUST_BACK;
	cb->eMenuState = OPTIONS_INPUT;
	cb->SetShortCut(Keyboard::Key_Escape);
	pc->AddElementNoCenterIn(cb);
	}
	
	szMenuText = getLocalised( "system_menus_options_input_customize_default" );
	me = new CMenuElementText(BUTTON_MENUOPTIONS_CONTROLS_CUST_DEFAULT, hFontMenu, szMenuText, Vec2i(0, 0), NOP);
	me->SetPos(Vec2i((RATIO_X(size.x) - me->rZone.width())*0.5f, RATIO_Y(380)));
	pc->AddElementNoCenterIn(me);
	
	{
	pTex = TextureContainer::Load("graph/interface/menus/next");
	CMenuCheckButton * cb = new CMenuCheckButton(RATIO_2(Vec2i(140, 190)), pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
	cb->iID = BUTTON_MENUOPTIONS_CONTROLS_CUST_BACK;
	cb->eMenuState = OPTIONS_INPUT_CUSTOMIZE_KEYS_2;
	cb->SetShortCut(Keyboard::Key_Escape);
	pc->AddElementNoCenterIn(cb);
	}

	pWindowMenuConsole->AddMenu(pc);

	pWindowMenu->AddConsole(pWindowMenuConsole);
	pWindowMenuConsole->ReInitActionKey();

	pWindowMenuConsole=new CWindowMenuConsole(offset, size, OPTIONS_INPUT_CUSTOMIZE_KEYS_2);

	fControlPosY = static_cast<long>(RATIO_Y(8.f));
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_inventory",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_INVENTORY1, BUTTON_MENUOPTIONS_CONTROLS_CUST_INVENTORY2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_book",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOK1, BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOK2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_bookcharsheet",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKCHARSHEET1, BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKCHARSHEET2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_bookmap",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKMAP1, BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKMAP2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_bookspell",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKSPELL1, BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKSPELL2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_bookquest",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKQUEST1, BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKQUEST2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_drink_potion_life",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONLIFE1, BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONLIFE2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_drink_potion_mana",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONMANA1, BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONMANA2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_torch",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_TORCH1, BUTTON_MENUOPTIONS_CONTROLS_CUST_TORCH2);

	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_cancelcurrentspell",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_CANCELCURSPELL1, BUTTON_MENUOPTIONS_CONTROLS_CUST_CANCELCURSPELL2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_precast1",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST1, BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST1_2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_precast2",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST2, BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST2_2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_precast3",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST3, BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST3_2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_weapon",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_WEAPON1, BUTTON_MENUOPTIONS_CONTROLS_CUST_WEAPON2);

	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_unequipweapon",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_UNEQUIPWEAPON1, BUTTON_MENUOPTIONS_CONTROLS_CUST_UNEQUIPWEAPON2);

	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_previous",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_PREVIOUS1, BUTTON_MENUOPTIONS_CONTROLS_CUST_PREVIOUS2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_next",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_NEXT1, BUTTON_MENUOPTIONS_CONTROLS_CUST_NEXT2);

	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_quickload",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKLOAD, BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKLOAD2);
	CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_quicksave",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKSAVE, BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKSAVE2);

	CUSTOM_CTRL_FUNC2("system_menus_options_input_customize_controls_bookmap",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_MINIMAP1, BUTTON_MENUOPTIONS_CONTROLS_CUST_MINIMAP2);

	CUSTOM_CTRL_FUNC3("system_menus_options_input_customize_controls_toggle_fullscreen", "Toggle fullscreen", 1,  BUTTON_MENUOPTIONS_CONTROLS_CUST_TOGGLE_FULLSCREEN1, BUTTON_MENUOPTIONS_CONTROLS_CUST_TOGGLE_FULLSCREEN2);

	pc=new CMenuPanel();

	{
	pTex = TextureContainer::Load("graph/interface/menus/back");
	CMenuCheckButton * cb = new CMenuCheckButton(RATIO_2(Vec2i(10, 190)), pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
	cb->iID = BUTTON_MENUOPTIONS_CONTROLS_CUST_BACK;
	cb->eMenuState = OPTIONS_INPUT_CUSTOMIZE_KEYS_1;
	cb->SetShortCut(Keyboard::Key_Escape);
	pc->AddElementNoCenterIn(cb);
	}
	
	szMenuText = getLocalised( "system_menus_options_input_customize_default" );
	me = new CMenuElementText(BUTTON_MENUOPTIONS_CONTROLS_CUST_DEFAULT, hFontMenu, szMenuText, Vec2i(0, 0), NOP);
	me->SetPos(Vec2i((RATIO_X(size.x) - me->rZone.width())*0.5f, RATIO_Y(380)));
	pc->AddElementNoCenterIn(me);

	pWindowMenuConsole->AddMenu(pc);

	pWindowMenu->AddConsole(pWindowMenuConsole);
	pWindowMenuConsole->ReInitActionKey();
	#undef CUSTOM_CTRL_X0
	#undef CUSTOM_CTRL_X1
	#undef CUSTOM_CTRL_X2
	#undef CUSTOM_CTRL_FUNC
	#undef CUSTOM_CTRL_FUNC2
	#undef CUSTOM_CTRL_FUNC3
}

void Menu2_Render_Options(Vec2i size, Vec2i offset)
{
	MainMenuOptionGroupsCreate(size, offset);
	MainMenuOptionVideoCreate(offset, size);
	MainMenuOptionAudioCreate(offset, size);
	MainMenuOptionInputCreate(size, offset);
	MainMenuOptionControlsCreate(offset, size);
	
	pWindowMenu->eCurrentMenuState=OPTIONS;	
}

void Menu2_Render_Quit(Vec2i size, Vec2i offset)
{
	std::string szMenuText;
	CMenuElement *me = NULL;
	CWindowMenuConsole *pWindowMenuConsole=new CWindowMenuConsole(offset, size, QUIT);
	
	szMenuText = getLocalised("system_menus_main_quit");
	me=new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(0, 0), NOP);
	me->bCheck = false;
	pWindowMenuConsole->AddMenuCenter(me);
	
	szMenuText = getLocalised("system_menus_main_editquest_confirm");
	me=new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(0, 0), NOP);
	me->bCheck = false;
	pWindowMenuConsole->AddMenuCenter(me);
	
	CMenuPanel *pPanel = new CMenuPanel();
	
	szMenuText = getLocalised("system_yes");
	me = new CMenuElementText(BUTTON_MENUMAIN_QUIT, hFontMenu, szMenuText, Vec2i(0, 0), NEW_QUEST_ENTER_GAME);
	me->SetPos(Vec2i(RATIO_X(size.x-10)-me->rZone.width(), 0));
	pPanel->AddElementNoCenterIn(me);
	
	szMenuText = getLocalised("system_no");
	me = new CMenuElementText(-1, hFontMenu, szMenuText, Vec2i(RATIO_X(10), 0), MAIN);
	me->SetShortCut(Keyboard::Key_Escape);
	pPanel->AddElementNoCenterIn(me);
	
	pPanel->Move(Vec2i(0, RATIO_Y(380)));
	pWindowMenuConsole->AddMenu(pPanel);
	pWindowMenu->AddConsole(pWindowMenuConsole);
	pWindowMenu->eCurrentMenuState=QUIT;
}

extern CMenuState *mainMenu;

void MainMenuLeftCreate(MENUSTATE eMenuState)
{
	mainMenu->eOldMenuState=eMenuState;
	
	delete pWindowMenu, pWindowMenu = NULL;
	
	Vec2i windowMenuPos = Vec2i(20, 25);
	Vec2i windowMenuSize = Vec2i(321, 430);
	
	Vec2i windowConsoleOffset = Vec2i(0, 14 - 10);
	Vec2i windowConsoleSize = windowMenuSize - windowConsoleOffset + Vec2i(0, 20);
	
	pWindowMenu = new CWindowMenu(windowMenuPos, windowMenuSize);
	
	switch(eMenuState) {
	case NEW_QUEST: {
			Menu2_Render_NewQuest(windowConsoleSize, windowConsoleOffset);
		
		break;
	}
	case EDIT_QUEST: {
			Menu2_Render_EditQuest(windowConsoleSize, windowConsoleOffset);
			}
		break;
	case OPTIONS: {
			Menu2_Render_Options(windowConsoleSize, windowConsoleOffset);
		}
		break;
	
	case QUIT: {
			Menu2_Render_Quit(windowConsoleSize, windowConsoleOffset);
		
		
		break;
	}
	default: break; // Unhandled menu state.
	}	
}
