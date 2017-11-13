/*
 * Copyright 2014-2017 Arx Libertatis Team (see the AUTHORS file)
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

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include "audio/Audio.h"
#include "core/Application.h"
#include "core/Benchmark.h"
#include "core/Config.h"
#include "core/Core.h"
#include "core/Localisation.h"
#include "core/SaveGame.h"
#include "core/Version.h"

#include "gui/Menu.h"
#include "gui/MenuWidgets.h"
#include "gui/MenuPublic.h"
#include "gui/Hud.h"
#include "gui/Text.h"
#include "gui/TextManager.h"
#include "gui/menu/MenuCursor.h"
#include "gui/menu/MenuFader.h"
#include "gui/widget/CheckboxWidget.h"
#include "gui/widget/CycleTextWidget.h"
#include "gui/widget/PanelWidget.h"
#include "gui/widget/SliderWidget.h"
#include "graphics/Draw.h"
#include "graphics/Math.h"
#include "graphics/Renderer.h"
#include "graphics/font/Font.h"
#include "graphics/data/TextureContainer.h"
#include "input/Input.h"
#include "input/Keyboard.h"
#include "scene/GameSound.h"
#include "window/RenderWindow.h"

TextWidget * pMenuElementApply = NULL;

class NewQuestMenuPage : public MenuPage {
	
public:
	
	NewQuestMenuPage()
		: MenuPage(Page_NewQuestConfirm)
	{}
	
	~NewQuestMenuPage() { }
	
	void init() {
		
		{
			std::string szMenuText = getLocalised("system_menus_main_editquest_confirm");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText);
			txt->SetCheckOff();
			addCenter(txt, true);
		}
		
		{
			std::string szMenuText = getLocalised("system_menus_main_newquest_confirm");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText);
			txt->SetCheckOff();
			addCenter(txt, true);
		}
		
		{
			std::string szMenuText = getLocalised("system_yes");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(m_size.x - 40, 380));
			txt->clicked = boost::bind(ARXMenu_NewQuest);
			add(txt);
		}
		
		{
			std::string szMenuText = getLocalised("system_no");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(10, 380));
			txt->m_targetMenu = Page_None;
			txt->SetShortCut(Keyboard::Key_Escape);
			add(txt);
		}
	}
	
};

extern bool bNoMenu;

class SaveConfirmMenuPage : public MenuPage {
	
public:
	
	SaveConfirmMenuPage()
		: MenuPage(Page_SaveConfirm)
		, m_savegame(SavegameHandle())
		, m_textbox(NULL)
		, pDeleteButton(NULL)
	{}
	
	~SaveConfirmMenuPage() { }
	
	void init() {
		
		{
			ButtonWidget * cb = new ButtonWidget(Vec2f_ZERO, Vec2f(48, 48), "graph/interface/icons/menu_main_save");
			cb->SetCheckOff();
			addCenter(cb, true);
		}
		
		{
			std::string szMenuText = getLocalised("system_menu_editquest_newsavegame", "---");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->m_savegame = SavegameHandle();
			txt->eState=EDIT;
			txt->ePlace=CENTER;
			addCenter(txt, true);
			m_textbox = txt;
		}
		
		// Delete button
		{
			std::string szMenuText = getLocalised("system_menus_main_editquest_delete");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f_ZERO);
			txt->clicked = boost::bind(&SaveConfirmMenuPage::onClickedSaveDelete, this, _1);
			txt->m_targetMenu = Page_Save;
			txt->SetPos(Vec2f(RATIO_X(m_size.x-10)-txt->m_rect.width(), RATIO_Y(5)));
			txt->lOldColor = txt->lColor;
			add(txt);
			pDeleteButton = txt;
		}
		
		// Save button
		{
			std::string szMenuText = getLocalised("system_menus_main_editquest_save");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f_ZERO);
			txt->clicked = boost::bind(&SaveConfirmMenuPage::onClickedSaveConfirm, this, _1);
			txt->m_targetMenu = Page_None;
			txt->SetPos(Vec2f(RATIO_X(m_size.x-10)-txt->m_rect.width(), RATIO_Y(380)));
			add(txt);
		}
		
		// Back button
		{
			ButtonWidget * cb = new ButtonWidget(Vec2f(20, 380), Vec2f(16, 16), "graph/interface/menus/back");
			cb->m_targetMenu = Page_Save;
			cb->SetShortCut(Keyboard::Key_Escape);
			add(cb);
		}
	}
	
	void setSaveHandle(SavegameHandle savegame) {
		m_savegame = savegame;
		m_textbox->m_savegame = savegame;
		
		if(savegame != SavegameHandle()) {
			m_textbox->SetText(savegames[savegame.handleData()].name);
			pDeleteButton->lColor = pDeleteButton->lOldColor;
			pDeleteButton->SetCheckOn();
		} else {
			pDeleteButton->lColor = Color::grayb(127);
			pDeleteButton->SetCheckOff();
			m_textbox->SetText(getLocalised("system_menu_editquest_newsavegame"));
		}
		
		AlignElementCenter(m_textbox);
	}
	
private:
	SavegameHandle m_savegame;
	TextWidget * m_textbox;
	TextWidget * pDeleteButton;
	
	void onClickedSaveConfirm(TextWidget * txt) {
		m_savegame = txt->m_savegame;
		
		ARXMenu_SaveQuest(m_textbox->m_text, m_textbox->m_savegame);
	}
	
	void onClickedSaveDelete(TextWidget * txt) {
		m_savegame = txt->m_savegame;
		
		g_mainMenu->bReInitAll = true;
		savegames.remove(m_textbox->m_savegame);
		return;
	}
};

class SaveSlotWidget : public TextWidget {
	
public:
	SaveSlotWidget(Font * font, const std::string & text, Vec2f pos = Vec2f_ZERO)
		: TextWidget(font, text, pos)
	{ }
	
	virtual ~SaveSlotWidget() { }
	
	void RenderMouseOver() {
		TextWidget::RenderMouseOver();
		
		arx_assert(m_id == BUTTON_MENUEDITQUEST_LOAD || m_id == BUTTON_MENUEDITQUEST_SAVEINFO);
		
		if(m_savegame == SavegameHandle()) {
			g_thumbnailCursor.clear();
			return;
		}
		
		const res::path & image = savegames[m_savegame.handleData()].thumbnail;
		if(!image.empty()) {
			TextureContainer * t = TextureContainer::LoadUI(image, TextureContainer::NoColorKey);
			if(t != g_thumbnailCursor.m_loadTexture) {
				delete g_thumbnailCursor.m_loadTexture;
				g_thumbnailCursor.m_loadTexture = t;
			}
			g_thumbnailCursor.m_renderTexture = g_thumbnailCursor.m_loadTexture;
		}
	}
	
};

class LoadMenuPage : public MenuPage {
	
public:
	
	LoadMenuPage()
		: MenuPage(Page_Load)
		, m_selectedSave(SavegameHandle())
		, pLoadConfirm(NULL)
		, pDeleteConfirm(NULL)
	{}
	
	~LoadMenuPage() { }
	
	void init() {
		
		{
			ButtonWidget * cb = new ButtonWidget(Vec2f_ZERO, Vec2f(48, 48), "graph/interface/icons/menu_main_load");
			cb->SetCheckOff();
			addCenter(cb, true);
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
			
			SaveSlotWidget * txt = new SaveSlotWidget(hFontControls, text.str(), Vec2f(20, 0));
			txt->m_id = BUTTON_MENUEDITQUEST_LOAD;
			txt->clicked = boost::bind(&LoadMenuPage::onClickQuestLoad, this, _1);
			txt->doubleClicked = boost::bind(&LoadMenuPage::onDoubleClickQuestLoad, this, _1);
			txt->m_savegame = SavegameHandle(i);
			addCenter(txt);
			m_saveSlotWidgets.push_back(txt);
		}
		
		// Show regular saves.
		for(size_t i = 0; i < savegames.size(); i++) {
			const SaveGame & save = savegames[i];
			
			if(save.quicksave) {
				continue;
			}
			
			std::string text = save.name +  "   " + save.time;
			
			SaveSlotWidget * txt = new SaveSlotWidget(hFontControls, text, Vec2f(20, 0));
			txt->m_id = BUTTON_MENUEDITQUEST_LOAD;
			txt->clicked = boost::bind(&LoadMenuPage::onClickQuestLoad, this, _1);
			txt->doubleClicked = boost::bind(&LoadMenuPage::onDoubleClickQuestLoad, this, _1);
			txt->m_savegame = SavegameHandle(i);
			addCenter(txt);
			m_saveSlotWidgets.push_back(txt);
		}
		
		{
			TextWidget * txt = new TextWidget(hFontControls, std::string(), Vec2f(20, 0));
			txt->m_targetMenu = Page_SaveConfirm;
			txt->SetCheckOff();
			txt->m_savegame = SavegameHandle();
			addCenter(txt);
		}
		
		// Delete button
		{
			std::string szMenuText = getLocalised("system_menus_main_editquest_delete");
			szMenuText += "   ";
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f_ZERO);
			txt->clicked = boost::bind(&LoadMenuPage::onClickQuestDelete, this);
			txt->m_targetMenu = Page_Load;
			txt->SetPos(Vec2f(RATIO_X(m_size.x-10)-txt->m_rect.width(), RATIO_Y(14)));
			txt->SetCheckOff();
			txt->lOldColor = txt->lColor;
			txt->lColor = Color::grayb(127);
			add(txt);
			pDeleteConfirm = txt;
		}
		
		// Load button
		{
			std::string szMenuText = getLocalised("system_menus_main_editquest_load");
			szMenuText += "   ";
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f_ZERO);
			txt->clicked = boost::bind(&LoadMenuPage::onClickQuestLoadConfirm, this);
			txt->m_targetMenu = Page_None;
			txt->SetPos(Vec2f(RATIO_X(m_size.x-10)-txt->m_rect.width(), RATIO_Y(380)));
			txt->SetCheckOff();
			txt->lOldColor = txt->lColor;
			txt->lColor = Color::grayb(127);
			add(txt);
			pLoadConfirm = txt;
		}
		
		// Back button
		{
			ButtonWidget * cb = new ButtonWidget(Vec2f(20, 380), Vec2f(16, 16), "graph/interface/menus/back");
			cb->clicked = boost::bind(&LoadMenuPage::onClickBack, this);
			cb->m_targetMenu = Page_LoadOrSave;
			cb->SetShortCut(Keyboard::Key_Escape);
			add(cb);
		}
		}
	}
	
	void resetSelection() {
		
		m_selectedSave = SavegameHandle();
		
		for(size_t j = 0; j < m_saveSlotWidgets.size(); j++) {
			SaveSlotWidget * widget = m_saveSlotWidgets[j];
			widget->bSelected = false;
		}
	}
	
private:
	std::vector<SaveSlotWidget *> m_saveSlotWidgets;
	
	SavegameHandle m_selectedSave;
	TextWidget * pLoadConfirm;
	TextWidget * pDeleteConfirm;
	
	void enableLoadDeleteButtons() {
		pLoadConfirm->SetCheckOn();
		pLoadConfirm->lColor = pLoadConfirm->lOldColor;
		pDeleteConfirm->SetCheckOn();
		pDeleteConfirm->lColor = pDeleteConfirm->lOldColor;
	}
	
	void disableLoadDeleteButtons() {
		pLoadConfirm->SetCheckOff();
		pLoadConfirm->lColor = Color::grayb(127);
		pDeleteConfirm->SetCheckOff();
		pDeleteConfirm->lColor = Color::grayb(127);
	}
	
	void onClickQuestLoad(TextWidget * txt) {
		resetSelection();
		enableLoadDeleteButtons();
		
		m_selectedSave = txt->m_savegame;
		txt->bSelected = true;
	}
	
	void onDoubleClickQuestLoad(TextWidget * txt) {
		txt->OnMouseClick();
		pLoadConfirm->OnMouseClick();
	}
	
	void onClickQuestLoadConfirm() {
		
		if(m_selectedSave != SavegameHandle()) {
			ARXMenu_LoadQuest(m_selectedSave);
			
			bNoMenu = true;
			
			if(pTextManage) {
				pTextManage->Clear();
			}
		}
		
		disableLoadDeleteButtons();
	}
	
	void onClickQuestDelete() {
		if(m_selectedSave != SavegameHandle()) {
			g_mainMenu->bReInitAll = true;
			savegames.remove(m_selectedSave);
			return;
		}
		
		disableLoadDeleteButtons();
	}
	
	void onClickBack() {
		disableLoadDeleteButtons();
	}
	
};

class SaveMenuPage : public MenuPage {
	
public:
	
	SaveMenuPage()
		: MenuPage(Page_Save)
	{}
	
	~SaveMenuPage() { }
	
	void init() {
		
		{
			ButtonWidget * cb = new ButtonWidget(Vec2f(10, 0), Vec2f(48, 48), "graph/interface/icons/menu_main_save");
			cb->SetCheckOff();
			addCenter(cb, true);
		}
		
		std::string quicksaveName = getLocalised("system_menus_main_quickloadsave", "Quicksave");
		size_t quicksaveNum = 0;
		
		// Show quicksaves.
		for(size_t i = 0; i < savegames.size(); i++) {
			const SaveGame & save = savegames[i];
			
			if(!save.quicksave) {
				continue;
			}
			
			std::ostringstream text;
			text << quicksaveName << ' ' << ++quicksaveNum << "   " << save.time;
			
			SaveSlotWidget * txt = new SaveSlotWidget(hFontControls, text.str(), Vec2f(20, 0));
			txt->m_id = BUTTON_MENUEDITQUEST_SAVEINFO;
			txt->clicked = boost::bind(&SaveMenuPage::onClickQuestSaveConfirm, this, _1);
			txt->m_targetMenu = Page_SaveConfirm;
			txt->setColor(Color::grayb(127));
			txt->SetCheckOff();
			txt->m_savegame = SavegameHandle(i);
			addCenter(txt);
		}
		
		// Show regular saves.
		for(size_t i = 0; i < savegames.size(); i++) {
			const SaveGame & save = savegames[i];
			
			if(save.quicksave) {
				continue;
			}
			
			std::string text = save.name +  "   " + save.time;
			
			SaveSlotWidget * txt = new SaveSlotWidget(hFontControls, text, Vec2f(20, 0));
			txt->m_id = BUTTON_MENUEDITQUEST_SAVEINFO;
			txt->clicked = boost::bind(&SaveMenuPage::onClickQuestSaveConfirm, this, _1);
			txt->m_targetMenu = Page_SaveConfirm;
			txt->m_savegame = SavegameHandle(i);
			addCenter(txt);
		}
		
		for(size_t i = savegames.size(); i <= 15; i++) {
			
			std::ostringstream text;
			text << '-' << std::setfill('0') << std::setw(4) << i << '-';
			
			SaveSlotWidget * txt = new SaveSlotWidget(hFontControls, text.str(), Vec2f(20, 0));
			txt->m_id = BUTTON_MENUEDITQUEST_SAVEINFO;
			txt->clicked = boost::bind(&SaveMenuPage::onClickQuestSaveConfirm, this, _1);
			txt->m_targetMenu = Page_SaveConfirm;
			txt->m_savegame = SavegameHandle();
			addCenter(txt);
		}
	
		{
			TextWidget * txt = new TextWidget(hFontControls, std::string(), Vec2f(20, 0));
			txt->clicked = boost::bind(&SaveMenuPage::onClickQuestSaveConfirm, this, _1);
			txt->m_targetMenu = Page_SaveConfirm;
			txt->m_savegame = SavegameHandle();
			txt->SetCheckOff();
			addCenter(txt);
		}
		
		{
			ButtonWidget * cb = new ButtonWidget(Vec2f(20, 380), Vec2f(16, 16), "graph/interface/menus/back");
			cb->m_targetMenu = Page_LoadOrSave;
			cb->SetShortCut(Keyboard::Key_Escape);
			add(cb);
		}
	}
	
private:
	void onClickQuestSaveConfirm(TextWidget * txt) {
		g_mainMenu->m_window->m_pageSaveConfirm->setSaveHandle(txt->m_savegame);
	}
};

class ChooseLoadOrSaveMenuPage : public MenuPage {
	
public:
	
	ChooseLoadOrSaveMenuPage()
		: MenuPage(Page_LoadOrSave)
	{}
	
	~ChooseLoadOrSaveMenuPage() { }
	
	void init() {
		
		{
			std::string szMenuText = getLocalised("system_menus_main_editquest_load");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f_ZERO);
			txt->clicked = boost::bind(&ChooseLoadOrSaveMenuPage::onClickLoad, this);
			txt->m_targetMenu = Page_Load;
			txt->m_savegame = SavegameHandle();
			addCenter(txt, true);
		}
		
		{
			std::string szMenuText = getLocalised( "system_menus_main_editquest_save");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f_ZERO);
			txt->m_targetMenu = Page_Save;
			
			if(!g_canResumeGame) {
				txt->SetCheckOff();
				txt->lColor = Color(127, 127, 127);
			}
			addCenter(txt, true);
		}
		
		{
			ButtonWidget * cb = new ButtonWidget(Vec2f(20, 380), Vec2f(16, 16), "graph/interface/menus/back");
			cb->m_targetMenu = Page_None;
			cb->SetShortCut(Keyboard::Key_Escape);
			add(cb);
		}
	}
	
private:
	
	void onClickLoad() {
		g_mainMenu->m_window->m_pageLoad->resetSelection();
	}
	
};



int newWidth;
int newHeight;
bool newFullscreen;

class OptionsMenuPage : public MenuPage {
	
public:
	
	OptionsMenuPage()
		: MenuPage(Page_Options)
	{}
	
	~OptionsMenuPage() { }
	
	void init() {
		
		{
			std::string szMenuText = getLocalised("system_menus_options_video");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f_ZERO);
			txt->clicked = boost::bind(&OptionsMenuPage::onClickedVideo, this);
			txt->m_targetMenu = Page_OptionsVideo;
			addCenter(txt, true);
		}
		
		{
			std::string szMenuText = getLocalised("system_menus_options_render", "Render settings");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f_ZERO);
			txt->m_targetMenu = Page_OptionsRender;
			addCenter(txt, true);
		}
		
		{
			std::string szMenuText = getLocalised("system_menus_options_interface", "Interface settings");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f_ZERO);
			txt->m_targetMenu = Page_OptionsInterface;
			addCenter(txt, true);
		}
		
		{
			std::string szMenuText = getLocalised("system_menus_options_audio");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f_ZERO);
			txt->m_targetMenu = Page_OptionsAudio;
			addCenter(txt, true);
		}
		
		{
			std::string szMenuText = getLocalised("system_menus_options_input");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f_ZERO);
			txt->m_targetMenu = Page_OptionsInput;
			addCenter(txt, true);
		}
		
		{
			ButtonWidget * cb = new ButtonWidget(Vec2f(20, 380), Vec2f(16, 16), "graph/interface/menus/back");
			cb->m_targetMenu = Page_None;
			cb->SetShortCut(Keyboard::Key_Escape);
			add(cb);
		}
	}
	
private:
	
	void onClickedVideo() {
		newWidth = config.video.resolution.x;
		newHeight = config.video.resolution.y;
		newFullscreen = config.video.fullscreen;
	}
	
};

// TODO remove this
const std::string AUTO_RESOLUTION_STRING = "Desktop";

class VideoOptionsMenuPage : public MenuPage {
	
public:
	
	VideoOptionsMenuPage()
		: MenuPage(Page_OptionsVideo)
		, m_gammaSlider(NULL)
		, m_minimizeOnFocusLostCheckbox(NULL)
	{
		fullscreenCheckbox = NULL;
		pMenuSliderResol = NULL;
	}
	
	~VideoOptionsMenuPage() { }
	
	CheckboxWidget * fullscreenCheckbox;
	CycleTextWidget * pMenuSliderResol;
	SliderWidget * m_gammaSlider;
	CheckboxWidget * m_minimizeOnFocusLostCheckbox;
	
	void init() {
		
		{
			std::string szMenuText = getLocalised("system_menus_options_videos_full_screen");
			if(szMenuText.empty()) {
				// TODO once we ship our own amendmends to the loc files a cleaner
				// fix would be to just define system_menus_options_videos_full_screen
				// for the german version there
				szMenuText = getLocalised("system_menus_options_video_full_screen");
			}
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			CheckboxWidget * cb = new CheckboxWidget(txt);
			cb->stateChanged = boost::bind(&VideoOptionsMenuPage::onChangedFullscreen, this, _1);
			cb->iState = config.video.fullscreen ? 1 : 0;
			addCenter(cb);
			fullscreenCheckbox = cb;
		}
		
		{
			PanelWidget * panel = new PanelWidget;
			std::string szMenuText = getLocalised("system_menus_options_video_resolution");
			szMenuText += "  ";
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			panel->AddElement(txt);
			pMenuSliderResol = new CycleTextWidget;
			pMenuSliderResol->valueChanged = boost::bind(&VideoOptionsMenuPage::onChangedResolution, this, _1, _2);
			
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
				
				pMenuSliderResol->AddText(new TextWidget(hFontMenu, ss.str()));
				
				if(mode.resolution == config.video.resolution) {
					pMenuSliderResol->selectLast();
				}
			}
			
			pMenuSliderResol->AddText(new TextWidget(hFontMenu, AUTO_RESOLUTION_STRING));
			
			if(config.video.resolution == Vec2i_ZERO) {
				pMenuSliderResol->selectLast();
			}
		
			float fRatio = RATIO_X(m_size.x - 9) - pMenuSliderResol->m_rect.width();
			pMenuSliderResol->Move(Vec2f(fRatio, 0));
			
			panel->AddElement(pMenuSliderResol);
			addCenter(panel);
		}
		
		{
			PanelWidget * panel = new PanelWidget;
			std::string szMenuText = getLocalised("system_menus_options_video_gamma");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			panel->AddElement(txt);
			SliderWidget * sld = new SliderWidget(Vec2f(200, 0));
			sld->valueChanged = boost::bind(&VideoOptionsMenuPage::onChangedGamma, this, _1);
			setGammaState(sld, config.video.fullscreen);
			panel->AddElement(sld);
			addCenter(panel);
			m_gammaSlider = sld;
		}
		
		{
			std::string szMenuText = getLocalised("system_menus_options_videos_minimize_on_focus_lost",
			                                      "Minimize on focus loss");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			CheckboxWidget * cb = new CheckboxWidget(txt);
			cb->stateChanged = boost::bind(&VideoOptionsMenuPage::onChangedMinimizeOnFocusLost, this, _1);
			setMinimizeOnFocusLostState(cb, config.video.fullscreen);
			addCenter(cb);
			m_minimizeOnFocusLostCheckbox = cb;
		}
		
		{
			// Add spacing
			addCenter(new TextWidget(hFontMenu, std::string(), Vec2f(20, 0)));
		}
		
		{
			PanelWidget * panel = new PanelWidget;
			std::string szMenuText = getLocalised("system_menus_options_video_vsync", "VSync");
			szMenuText += " ";
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			panel->AddElement(txt);
			
			CycleTextWidget * cb = new CycleTextWidget;
			cb->valueChanged = boost::bind(&VideoOptionsMenuPage::onChangedVSync, this, _1, _2);
			szMenuText = getLocalised("system_menus_options_video_vsync_off", "Disabled");
			cb->AddText(new TextWidget(hFontMenu, szMenuText));
			if(benchmark::isEnabled()) {
				cb->setValue(0);
				cb->setEnabled(false);
			} else {
				szMenuText = getLocalised("system_menus_options_video_vsync_on", "Enabled");
				cb->AddText(new TextWidget(hFontMenu, szMenuText));
				szMenuText = getLocalised("system_menus_options_video_vsync_auto", "Adaptive");
				cb->AddText(new TextWidget(hFontMenu, szMenuText));
				cb->setValue(config.video.vsync < 0 ? 2 : config.video.vsync);
			}
			
			cb->Move(Vec2f(RATIO_X(m_size.x-9) - cb->m_rect.width(), 0));
			panel->AddElement(cb);
			
			addCenter(panel);
		}
		
		{
			PanelWidget * panel = new PanelWidget;
			std::string szMenuText = getLocalised("system_menus_options_video_fps_limit", "FPS Limit ");
			szMenuText += " ";
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			panel->AddElement(txt);
			
			CycleTextWidget * cb = new CycleTextWidget;
			cb->valueChanged = boost::bind(&VideoOptionsMenuPage::onChangedFPSLimit, this, _1, _2);
			szMenuText = getLocalised("system_menus_options_video_vsync_off", "Disabled");
			cb->AddText(new TextWidget(hFontMenu, szMenuText));
			if(benchmark::isEnabled()) {
				cb->setValue(0);
				cb->setEnabled(false);
			} else {
				cb->AddText(new TextWidget(hFontMenu, "60"));
				cb->AddText(new TextWidget(hFontMenu, "120"));
				cb->AddText(new TextWidget(hFontMenu, "240"));
				cb->AddText(new TextWidget(hFontMenu, "480"));
				if(config.video.fpsLimit == 0) {
					cb->setValue(0);
				} else if(config.video.fpsLimit == 60) {
					cb->setValue(1);
				} else if(config.video.fpsLimit == 120) {
					cb->setValue(2);
				} else if(config.video.fpsLimit == 240) {
					cb->setValue(3);
				} else if(config.video.fpsLimit == 480) {
					cb->setValue(4);
				} else {
					std::string text = boost::lexical_cast<std::string>(config.video.fpsLimit);
					cb->AddText(new TextWidget(hFontMenu, text));
					cb->setValue(5);
				}
			}
			
			cb->Move(Vec2f(RATIO_X(m_size.x-9) - cb->m_rect.width(), 0));
			panel->AddElement(cb);
			
			addCenter(panel);
		}
		
		{
			std::string szMenuText = getLocalised("system_menus_video_apply");
			szMenuText += "   ";
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(240, 0));
			txt->clicked = boost::bind(&VideoOptionsMenuPage::onClickedApply, this);
			txt->SetPos(Vec2f(RATIO_X(m_size.x-10)-txt->m_rect.width(), RATIO_Y(380) + RATIO_Y(40)));
			txt->SetCheckOff();
			add(txt);
			pMenuElementApply = txt;
		}
		
		{
			ButtonWidget * cb = new ButtonWidget(Vec2f(20, 380), Vec2f(16, 16), "graph/interface/menus/back");
			cb->clicked = boost::bind(&VideoOptionsMenuPage::onClickedBack, this);
			cb->m_targetMenu = Page_Options;
			cb->SetShortCut(Keyboard::Key_Escape);
			add(cb);
		}
	}
	
private:
	
	void setGammaState(SliderWidget * sld, bool fullscreen) {
		
		if(!fullscreen) {
			sld->setValue(5);
			sld->SetCheckOff();
			return;
		}
		
		sld->setValue(int(config.video.gamma));
		sld->SetCheckOn();
		
	}
	
	void setMinimizeOnFocusLostState(CheckboxWidget * cb, bool fullscreen) {
		
		if(!fullscreen) {
			cb->iState = 0;
			cb->SetCheckOff();
			return;
		}
		
		Window::MinimizeSetting minimize = mainApp->getWindow()->willMinimizeOnFocusLost();
		cb->iState = (minimize == Window::Enabled || minimize == Window::AlwaysEnabled) ? 1 : 0;
		if(minimize != Window::AlwaysDisabled && minimize != Window::AlwaysEnabled) {
			cb->SetCheckOn();
		} else {
			cb->SetCheckOff();
		}
		
	}
	
	void onChangedFullscreen(int state) {
		newFullscreen = ((state)?true:false);
		
		if(pMenuSliderResol) {
			pMenuSliderResol->setEnabled(newFullscreen);
			setGammaState(m_gammaSlider, newFullscreen);
			setMinimizeOnFocusLostState(m_minimizeOnFocusLostCheckbox, newFullscreen);
		}
	}
	
	void onChangedResolution(int pos, const std::string & str) {
		ARX_UNUSED(pos);
		
		if(str == AUTO_RESOLUTION_STRING) {
			newWidth = newHeight = 0;
		} else {
			std::stringstream ss(str);
			int iX = config.video.resolution.x;
			int iY = config.video.resolution.y;
			char tmp;
			ss >> iX >> tmp >> iY;
			newWidth = iX;
			newHeight = iY;
		}
	}
	
	void onChangedGamma(int state) {
		ARXMenu_Options_Video_SetGamma(float(state));
	}
	
	void onChangedMinimizeOnFocusLost(int state) {
		config.window.minimizeOnFocusLost = state ? true : false;
		mainApp->getWindow()->setMinimizeOnFocusLost(config.window.minimizeOnFocusLost);
	}
	
	void onChangedVSync(int pos, const std::string & str) {
		ARX_UNUSED(str);
		config.video.vsync = pos > 1 ? -1 : pos;
		mainApp->getWindow()->setVSync(config.video.vsync);
	}
	
	void onChangedFPSLimit(int pos, const std::string & str) {
		ARX_UNUSED(str);
		if(pos == 0) {
			config.video.fpsLimit = 0;
		} else {
			config.video.fpsLimit = boost::lexical_cast<int>(str);
		}
	}
	
	void onClickedBack() {
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
	}
	
	void onClickedApply() {
		if(newWidth != config.video.resolution.x
		   || newHeight!=config.video.resolution.y
		   || newFullscreen != config.video.fullscreen
		) {
			ARXMenu_Private_Options_Video_SetResolution(newFullscreen, newWidth, newHeight);
			pMenuSliderResol->setOldValue(-1);
			fullscreenCheckbox->iOldState = -1;
		}
		g_mainMenu->bReInitAll=true;
	}
	
};

class RenderOptionsMenuPage : public MenuPage {
	
public:
	
	RenderOptionsMenuPage()
		: MenuPage(Page_OptionsRender)
		, m_alphaCutoutAntialiasingCycleText(NULL)
	{ }
	
	~RenderOptionsMenuPage() { }
	
	CycleTextWidget * m_alphaCutoutAntialiasingCycleText;
	
	void init() {
		
		// Renderer selection
		{
			PanelWidget * panel = new PanelWidget;
			std::string szMenuText = getLocalised("system_menus_options_video_renderer", "Renderer");
			szMenuText += "  ";
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			panel->AddElement(txt);
			CycleTextWidget * slider = new CycleTextWidget;
			slider->valueChanged = boost::bind(&RenderOptionsMenuPage::onChangedRenderer, this, _1, _2);
			
			{
				TextWidget * txt = new TextWidget(hFontMenu, "Auto-Select", Vec2f_ZERO);
				slider->AddText(txt);
				slider->selectLast();
			}
			
			{
				TextWidget * txt = new TextWidget(hFontMenu, "OpenGL", Vec2f_ZERO);
				slider->AddText(txt);
				if(config.video.renderer == "OpenGL") {
					slider->selectLast();
				}
			}
			
			float fRatio    = (RATIO_X(m_size.x-9) - slider->m_rect.width());
			slider->Move(Vec2f(fRatio, 0));
			panel->AddElement(slider);
			addCenter(panel);
		}
		
		{
			// Add spacing
			addCenter(new TextWidget(hFontMenu, std::string(), Vec2f(20, 0)));
		}
		
		{
			PanelWidget * panel = new PanelWidget;
			std::string szMenuText = getLocalised("system_menus_options_detail");
			szMenuText += " ";
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			panel->AddElement(txt);
			
			CycleTextWidget * cb = new CycleTextWidget;
			cb->valueChanged = boost::bind(&RenderOptionsMenuPage::onChangedQuality, this, _1, _2);
			szMenuText = getLocalised("system_menus_options_video_texture_low");
			cb->AddText(new TextWidget(hFontMenu, szMenuText));
			szMenuText = getLocalised("system_menus_options_video_texture_med");
			cb->AddText(new TextWidget(hFontMenu, szMenuText));
			szMenuText = getLocalised("system_menus_options_video_texture_high");
			cb->AddText(new TextWidget(hFontMenu, szMenuText));
			cb->setValue(config.video.levelOfDetail);
			
			cb->Move(Vec2f(RATIO_X(m_size.x-9) - cb->m_rect.width(), 0));
			panel->AddElement(cb);
			
			addCenter(panel);
		}
		
		{
			PanelWidget * panel = new PanelWidget;
			std::string szMenuText = getLocalised("system_menus_options_video_brouillard");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			panel->AddElement(txt);
			SliderWidget * sld = new SliderWidget(Vec2f(200, 0));
			sld->valueChanged = boost::bind(&RenderOptionsMenuPage::onChangedFogDistance, this, _1);
			sld->setValue(int(config.video.fogDistance));
			panel->AddElement(sld);
			addCenter(panel);
		}
		
		{
			std::string szMenuText = getLocalised("system_menus_options_video_antialiasing", "antialiasing");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			CheckboxWidget * cb = new CheckboxWidget(txt);
			cb->stateChanged = boost::bind(&RenderOptionsMenuPage::onChangedAntialiasing, this, _1);
			cb->iState = config.video.antialiasing ? 1 : 0;
			addCenter(cb);
		}
		
		{
			std::string szMenuText = getLocalised("system_menus_options_video_colorkey_antialiasing", "Color Key AA");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			CheckboxWidget * cb = new CheckboxWidget(txt);
			cb->stateChanged = boost::bind(&RenderOptionsMenuPage::onChangedColorkeyAntialiasing, this, _1);
			cb->iState = config.video.colorkeyAntialiasing ? 1 : 0;
			addCenter(cb);
		}
		
		{
			PanelWidget * panel = new PanelWidget;
			std::string szMenuText = getLocalised("system_menus_options_video_alpha_cutout_antialising", "Alpha Cutout AA");
			szMenuText += " ";
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			panel->AddElement(txt);
			
			CycleTextWidget * cb = new CycleTextWidget;
			cb->valueChanged = boost::bind(&RenderOptionsMenuPage::onChangedAlphaCutoutAntialiasing, this, _1, _2);
			szMenuText = getLocalised("system_menus_options_video_alpha_cutout_antialising_off", "Disabled");
			cb->AddText(new TextWidget(hFontMenu, szMenuText));
			Renderer::AlphaCutoutAntialising maxAA = GRenderer->getMaxSupportedAlphaCutoutAntialiasing();
			if(maxAA >= Renderer::FuzzyAlphaCutoutAA) {
				szMenuText = getLocalised("system_menus_options_video_alpha_cutout_antialising_fuzzy", "Fuzzy");
				cb->AddText(new TextWidget(hFontMenu, szMenuText));
			}
			if(maxAA >= Renderer::CrispAlphaCutoutAA) {
				szMenuText = getLocalised("system_menus_options_video_alpha_cutout_antialising_crisp", "Crisp");
				cb->AddText(new TextWidget(hFontMenu, szMenuText));
			}
			m_alphaCutoutAntialiasingCycleText = cb;
			setAlphaCutoutAntialisingState();
			
			cb->Move(Vec2f(RATIO_X(m_size.x-9) - cb->m_rect.width(), 0));
			panel->AddElement(cb);
			
			addCenter(panel);
		}
		
		{
			PanelWidget * panel = new PanelWidget;
			std::string szMenuText = getLocalised("system_menus_options_video_texture_filter_anisotropic",
			                                      "Anisotropic filtering");
			szMenuText += " ";
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			panel->AddElement(txt);
			
			CycleTextWidget * cb = new CycleTextWidget;
			cb->valueChanged = boost::bind(&RenderOptionsMenuPage::onChangedMaxAnisotropy, this, _1, _2);
			szMenuText = getLocalised("system_menus_options_video_filter_anisotropic_off", "Disabled");
			cb->AddText(new TextWidget(hFontMenu, szMenuText));
			int maxAnisotropy = int(GRenderer->getMaxSupportedAnisotropy());
			int selected = 0;
			if(maxAnisotropy > 1) {
				int i = 1;
				std::ostringstream oss;
				for(int anisotropy = 2; ; anisotropy *= 2, i++) {
					if(anisotropy > maxAnisotropy) {
						anisotropy = maxAnisotropy;
					}
					oss.str(std::string());
					oss << 'x' << anisotropy;
					cb->AddText(new TextWidget(hFontMenu, oss.str()));
					if(config.video.maxAnisotropicFiltering == anisotropy) {
						selected = i;
					}
					if(config.video.maxAnisotropicFiltering > anisotropy
					   && config.video.maxAnisotropicFiltering < anisotropy * 2
					   && config.video.maxAnisotropicFiltering <= maxAnisotropy) {
						oss.str(std::string());
						oss << 'x' << config.video.maxAnisotropicFiltering;
						cb->AddText(new TextWidget(hFontMenu, oss.str()));
						selected = ++i;
					}
					if(anisotropy == maxAnisotropy) {
						i++;
						break;
					}
				}
				szMenuText = getLocalised("system_menus_options_video_filter_anisotropic_max", "Unlimited");
				cb->AddText(new TextWidget(hFontMenu, szMenuText));
				if(config.video.maxAnisotropicFiltering > maxAnisotropy) {
					selected = i;
				}
			}
			cb->setValue(selected);
			cb->Move(Vec2f(RATIO_X(m_size.x-9) - cb->m_rect.width(), 0));
			if(maxAnisotropy <= 1) {
				cb->setEnabled(false);
			}
			panel->AddElement(cb);
			
			addCenter(panel);
		}
		
		{
			ButtonWidget * cb = new ButtonWidget(Vec2f(20, 380), Vec2f(16, 16), "graph/interface/menus/back");
			cb->m_targetMenu = Page_Options;
			cb->SetShortCut(Keyboard::Key_Escape);
			add(cb);
		}
	}
	
private:
	
	void setAlphaCutoutAntialisingState() {
		
		CycleTextWidget * cb = m_alphaCutoutAntialiasingCycleText;
		
		int maxAA = int(GRenderer->getMaxSupportedAlphaCutoutAntialiasing());
		if(config.video.antialiasing || maxAA == int(Renderer::NoAlphaCutoutAA)) {
			int value = config.video.alphaCutoutAntialiasing;
			if(value > maxAA) {
				value = int(Renderer::NoAlphaCutoutAA);
			}
			cb->setValue(value);
			cb->setEnabled(true);
		} else {
			cb->setValue(0);
			cb->setEnabled(false);
		}
		
	}
	
	void onChangedRenderer(int pos, const std::string & str) {
		ARX_UNUSED(str);
		
		switch(pos) {
			case 0:  config.video.renderer = "auto"; break;
			case 1:  config.video.renderer = "OpenGL";  break;
			default: config.video.renderer = "auto"; break;
		}
	}
	
	void onChangedQuality(int pos, const std::string & str) {
		ARX_UNUSED(str);
		
		ARXMenu_Options_Video_SetDetailsQuality(pos);
	}
	
	void onChangedFogDistance(int value) {
		ARXMenu_Options_Video_SetFogDistance(value);
	}
	
	void onChangedAntialiasing(int state) {
		config.video.antialiasing = state ? true : false;
		setAlphaCutoutAntialisingState();
	}
	
	void onChangedColorkeyAntialiasing(int state) {
		config.video.colorkeyAntialiasing = state ? true : false;
		GRenderer->reloadColorKeyTextures();
	}
	
	void onChangedAlphaCutoutAntialiasing(int pos, const std::string & str) {
		ARX_UNUSED(str);
		config.video.alphaCutoutAntialiasing = pos;
	}
	
	void onChangedMaxAnisotropy(int pos, const std::string & str) {
		ARX_UNUSED(str);
		
		int anisotropy = 1;
		if(pos > 0) {
			if(!str.empty() && str[0] == 'x') {
				std::stringstream ss(str.substr(1));
				ss >> anisotropy;
			} else {
				anisotropy = 9001;
			}
		}
		
		config.video.maxAnisotropicFiltering = anisotropy;
		GRenderer->setMaxAnisotropy(anisotropy);
	}
	
};

class InterfaceOptionsMenuPage : public MenuPage {
	
public:
	
	InterfaceOptionsMenuPage()
		: MenuPage(Page_OptionsInterface)
	{ }
	
	~InterfaceOptionsMenuPage() { }
	
	void init() {
		
		{
			std::string szMenuText = getLocalised("system_menus_options_video_crosshair",
			                         "Cross hair cursor");
			szMenuText = getLocalised("system_menus_options_interface_crosshair", szMenuText);
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			CheckboxWidget * cb = new CheckboxWidget(txt);
			cb->stateChanged = boost::bind(&InterfaceOptionsMenuPage::onChangedCrosshair, this, _1);
			cb->iState = config.interface.showCrosshair ? 1 : 0;
			addCenter(cb);
		}
		
		{
			std::string szMenuText = getLocalised("system_menus_options_interface_limit_speech_width",
			                         "Limit speech text width");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			CheckboxWidget * cb = new CheckboxWidget(txt);
			cb->stateChanged = boost::bind(&InterfaceOptionsMenuPage::onChangedSpeechWidth, this, _1);
			cb->iState = config.interface.limitSpeechWidth ? 1 : 0;
			addCenter(cb);
		}
		
		{
			PanelWidget * panel = new PanelWidget;
			std::string szMenuText = getLocalised("system_menus_options_interface_cinematic_widescreen_mode",
			                                      "Cinematics mode");
			szMenuText += " ";
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			panel->AddElement(txt);
			
			CycleTextWidget * cb = new CycleTextWidget;
			cb->valueChanged = boost::bind(&InterfaceOptionsMenuPage::onChangedCinematicMode, this, _1, _2);
			szMenuText = getLocalised("system_menus_options_interface_letterbox", "Letterbox");
			cb->AddText(new TextWidget(hFontMenu, szMenuText));
			szMenuText = getLocalised("system_menus_options_interface_hard_edges", "Hard Edges");
			cb->AddText(new TextWidget(hFontMenu, szMenuText));
			szMenuText = getLocalised("system_menus_options_interface_fade_edges", "Fade Edges");
			cb->AddText(new TextWidget(hFontMenu, szMenuText));
			cb->setValue(config.interface.cinematicWidescreenMode);
			
			cb->Move(Vec2f(RATIO_X(m_size.x-9) - cb->m_rect.width(), 0));
			panel->AddElement(cb);
			
			addCenter(panel);
		}
		
		{
			PanelWidget * panel = new PanelWidget;
			std::string szMenuText = getLocalised("system_menus_options_interface_hud_scale",
			                                      "HUD size");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			panel->AddElement(txt);
			SliderWidget * sld = new SliderWidget(Vec2f(200, 0));
			sld->valueChanged = boost::bind(&InterfaceOptionsMenuPage::onChangedHudScale, this, _1);
			sld->setValue(int(config.interface.hudScale * 10.f));
			panel->AddElement(sld);
			addCenter(panel);
		}
		
		{
			std::string szMenuText = getLocalised("system_menus_options_interface_hud_scale_integer",
			                                      "Round HUD scale factor");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			CheckboxWidget * cb = new CheckboxWidget(txt);
			cb->stateChanged = boost::bind(&InterfaceOptionsMenuPage::onChangedHudScaleInteger, this, _1);
			cb->iState = config.interface.hudScaleInteger ? 1 : 0;
			addCenter(cb);
		}

		{
			std::string szMenuText = getLocalised("system_menus_options_interface_scale_cursor_with_hud",
												  "Scale cursor with HUD");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			CheckboxWidget * cb = new CheckboxWidget(txt);
			cb->stateChanged = boost::bind(&InterfaceOptionsMenuPage::onChangedScaleCursorWithHud, this, _1);
			cb->iState = config.interface.scaleCursorWithHud ? 1 : 0;
			addCenter(cb);
		}
		
		{
			PanelWidget * panel = new PanelWidget;
			std::string szMenuText = getLocalised("system_menus_options_interface_hud_scale_filter",
			                                      "HUD scale filter");
			szMenuText += " ";
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			panel->AddElement(txt);
			
			CycleTextWidget * cb = new CycleTextWidget;
			cb->valueChanged = boost::bind(&InterfaceOptionsMenuPage::onChangedHudScaleFilter, this, _1, _2);
			szMenuText = getLocalised("system_menus_options_video_filter_nearest", "Nearest");
			cb->AddText(new TextWidget(hFontMenu, szMenuText));
			szMenuText = getLocalised("system_menus_options_video_filter_bilinear", "Bilinear");
			cb->AddText(new TextWidget(hFontMenu, szMenuText));
			cb->setValue(config.interface.hudScaleFilter);
			
			cb->Move(Vec2f(RATIO_X(m_size.x-9) - cb->m_rect.width(), 0));
			panel->AddElement(cb);
			
			addCenter(panel);
		}
		
		{
			ButtonWidget * cb = new ButtonWidget(Vec2f(20, 380), Vec2f(16, 16), "graph/interface/menus/back");
			cb->m_targetMenu = Page_Options;
			cb->SetShortCut(Keyboard::Key_Escape);
			add(cb);
		}
	}
	
private:
	
	void onChangedCrosshair(int state) {
		config.interface.showCrosshair = state ? true : false;
	}
	
	void onChangedSpeechWidth(int state) {
		config.interface.limitSpeechWidth = state ? true : false;
	}
	
	void onChangedCinematicMode(int pos, const std::string & str) {
		ARX_UNUSED(str);
		
		config.interface.cinematicWidescreenMode = CinematicWidescreenMode(pos);
	}
	
	void onChangedHudScale(int state) {
		config.interface.hudScale = float(state) * 0.1f;
		g_hudRoot.recalcScale();
	}
	
	void onChangedHudScaleInteger(int state) {
		config.interface.hudScaleInteger = state ? true : false;
		g_hudRoot.recalcScale();
	}

	void onChangedScaleCursorWithHud(int state) {
		config.interface.scaleCursorWithHud = state ? true : false;
	}
	
	void onChangedHudScaleFilter(int pos, const std::string & str) {
		ARX_UNUSED(str);
		
		config.interface.hudScaleFilter = UIScaleFilter(pos);
	}
	
};


class AudioOptionsMenuPage : public MenuPage {
	
public:
	
	AudioOptionsMenuPage()
		: MenuPage(Page_OptionsAudio)
	{}
	
	~AudioOptionsMenuPage() { }
	
	void init() {
		
		// Audio backend selection
		{
			
			PanelWidget * panel = new PanelWidget;
			std::string szMenuText = getLocalised("system_menus_options_audio_device", "Device");
			szMenuText += "  ";
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			panel->AddElement(txt);
			CycleTextWidget * slider = new CycleTextWidget;
			slider->valueChanged = boost::bind(&AudioOptionsMenuPage::onChangedDevice, this, _1, _2);
			
			float maxwidth = RATIO_X(m_size.x - 28) - txt->m_rect.width() - slider->m_rect.width();
			
			slider->AddText(new TextWidget(hFontControls, "Default"));
			slider->selectLast();
			
			BOOST_FOREACH(const std::string & device, audio::getDevices()) {
				TextWidget * txt = new TextWidget(hFontControls, device);
				if(txt->m_rect.width() > maxwidth) {
					txt->m_rect.right = txt->m_rect.left + maxwidth;
				}
				slider->AddText(txt);
				if(config.audio.device == device) {
					slider->selectLast();
				}
			}
			
			float fRatio    = (RATIO_X(m_size.x-9) - slider->m_rect.width());
			slider->Move(Vec2f(fRatio, 0));
			panel->AddElement(slider);
			addCenter(panel);
			
		}
		
		{
			PanelWidget * panel = new PanelWidget;
			std::string szMenuText = getLocalised("system_menus_options_audio_master_volume");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			panel->AddElement(txt);
			SliderWidget * sld = new SliderWidget(Vec2f(200, 0));
			sld->valueChanged = boost::bind(&AudioOptionsMenuPage::onChangedMasterVolume, this, _1);
			sld->setValue((int)config.audio.volume); // TODO use float sliders
			panel->AddElement(sld);
			addCenter(panel);
		}
		
		{
			PanelWidget * panel = new PanelWidget;
			std::string szMenuText = getLocalised("system_menus_options_audio_effects_volume");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			panel->AddElement(txt);
			SliderWidget * sld = new SliderWidget(Vec2f(200, 0));
			sld->valueChanged = boost::bind(&AudioOptionsMenuPage::onChangedEffectsVolume, this, _1);
			sld->setValue((int)config.audio.sfxVolume);
			panel->AddElement(sld);
			addCenter(panel);
		}
		
		{
			PanelWidget * panel = new PanelWidget;
			std::string szMenuText = getLocalised("system_menus_options_audio_speech_volume");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			panel->AddElement(txt);
			SliderWidget * sld = new SliderWidget(Vec2f(200, 0));
			sld->valueChanged = boost::bind(&AudioOptionsMenuPage::onChangedSpeechVolume, this, _1);
			sld->setValue((int)config.audio.speechVolume);
			panel->AddElement(sld);
			addCenter(panel);
		}
		
		{
			PanelWidget * panel = new PanelWidget;
			std::string szMenuText = getLocalised("system_menus_options_audio_ambiance_volume");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			panel->AddElement(txt);
			SliderWidget * sld = new SliderWidget(Vec2f(200, 0));
			sld->valueChanged = boost::bind(&AudioOptionsMenuPage::onChangedAmbianceVolume, this, _1);
			sld->setValue((int)config.audio.ambianceVolume);
			panel->AddElement(sld);
			addCenter(panel);
		}
		
		{
			std::string szMenuText = getLocalised("system_menus_options_audio_mute_on_focus_lost", "Mute when not focused");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			CheckboxWidget * cb = new CheckboxWidget(txt);
			cb->stateChanged = boost::bind(&AudioOptionsMenuPage::onChangedMuteOnFocusLost, this, _1);
			cb->iState = config.audio.muteOnFocusLost ? 1 : 0;
			addCenter(cb);
		}
		
		{
			std::string szMenuText = getLocalised("system_menus_options_audio_eax", "EAX");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			CheckboxWidget * cb = new CheckboxWidget(txt);
			cb->stateChanged = boost::bind(&AudioOptionsMenuPage::onChangedEax, this, _1);
			if(audio::isReverbSupported()) {
				cb->iState = config.audio.eax ? 1 : 0;
			} else {
				cb->SetCheckOff();
			}
			addCenter(cb);
		}
		
		audio::HRTFStatus hrtf = audio::getHRTFStatus();
		if(hrtf != audio::HRTFUnavailable) {
			
			PanelWidget * panel = new PanelWidget;
			std::string szMenuText = getLocalised("system_menus_options_audio_hrtf", "Virtual surround");
			szMenuText += "  ";
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			panel->AddElement(txt);
			
			CycleTextWidget * slider = new CycleTextWidget;
			slider->valueChanged = boost::bind(&AudioOptionsMenuPage::onChangedHRTF, this, _1, _2);
			slider->AddText(new TextWidget(hFontMenu, "Disabled"));
			if(config.audio.hrtf == audio::HRTFDisable || hrtf == audio::HRTFForbidden) {
				slider->selectLast();
			}
			slider->AddText(new TextWidget(hFontMenu, "Automatic"));
			if(config.audio.hrtf == audio::HRTFDefault) {
				slider->selectLast();
			}
			slider->AddText(new TextWidget(hFontMenu, "Enabled"));
			if(config.audio.hrtf == audio::HRTFEnable || hrtf == audio::HRTFRequired) {
				slider->selectLast();
			}
			if(hrtf == audio::HRTFRequired || hrtf == audio::HRTFForbidden) {
				slider->setEnabled(false);
			}
			float fRatio    = (RATIO_X(m_size.x-9) - slider->m_rect.width());
			slider->Move(Vec2f(fRatio, 0));
			panel->AddElement(slider);
			
			addCenter(panel);
			
		}
		
		{
			ButtonWidget * cb = new ButtonWidget(Vec2f(20, 380), Vec2f(16, 16), "graph/interface/menus/back");
			cb->m_targetMenu = Page_Options;
			cb->SetShortCut(Keyboard::Key_Escape);
			add(cb);
		}
	}
	
private:
	
	void onChangedDevice(int pos, const std::string & str) {
		if(pos == 0) {
			ARXMenu_Options_Audio_SetDevice("auto");
		} else {
			ARXMenu_Options_Audio_SetDevice(str);
		}
	}
	
	void onChangedMasterVolume(int value) {
		ARXMenu_Options_Audio_SetMasterVolume(value);
	}
	
	void onChangedEffectsVolume(int value) {
		ARXMenu_Options_Audio_SetSfxVolume(value);
	}
	
	void onChangedSpeechVolume(int value) {
		ARXMenu_Options_Audio_SetSpeechVolume(value);
	}
	
	void onChangedAmbianceVolume(int value) {
		ARXMenu_Options_Audio_SetAmbianceVolume(value);
	}
	
	void onChangedEax(int state) {
		config.audio.eax = (state != 0);
		ARX_SOUND_SetReverb(config.audio.eax);
	}
	
	void onChangedHRTF(int pos, const std::string & str) {
		ARX_UNUSED(str);
		switch(pos) {
			case 0: config.audio.hrtf = audio::HRTFDisable; break;
			case 1: config.audio.hrtf = audio::HRTFDefault; break;
			case 2: config.audio.hrtf = audio::HRTFEnable; break;
			default: ARX_DEAD_CODE();
		}
		audio::setHRTFEnabled(config.audio.hrtf);
	}
	
	void onChangedMuteOnFocusLost(int state) {
		config.audio.muteOnFocusLost = (state != 0);
		if(!mainApp->getWindow()->hasFocus()) {
			ARXMenu_Options_Audio_SetMuted(config.audio.muteOnFocusLost);
		}
	}
	
};

class InputOptionsMenuPage : public MenuPage {
	
public:
	
	InputOptionsMenuPage()
		: MenuPage(Page_OptionsInput)
	{}
	
	~InputOptionsMenuPage() { }
	
	void init() {
		
		{
			std::string szMenuText = getLocalised("system_menus_options_input_customize_controls");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->m_targetMenu = Page_OptionsInputCustomizeKeys1;
			addCenter(txt);
		}
		
		{
			std::string szMenuText = getLocalised("system_menus_options_input_invert_mouse");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			CheckboxWidget * cb = new CheckboxWidget(txt);
			cb->stateChanged = boost::bind(&InputOptionsMenuPage::onChangedInvertMouse, this, _1);
			cb->iState = config.input.invertMouse ? 1 : 0;
			addCenter(cb);
		}
		
		{
			PanelWidget * panel = new PanelWidget;
			std::string szMenuText = getLocalised("system_menus_options_auto_ready_weapon");
			szMenuText += " ";
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			panel->AddElement(txt);
			
			CycleTextWidget * cb = new CycleTextWidget;
			cb->valueChanged = boost::bind(&InputOptionsMenuPage::onChangedAutoReadyWeapon, this, _1, _2);
			szMenuText = getLocalised("system_menus_options_auto_ready_weapon_off", "Disabled");
			cb->AddText(new TextWidget(hFontMenu, szMenuText));
			szMenuText = getLocalised("system_menus_options_auto_ready_weapon_enemies", "Enemies");
			cb->AddText(new TextWidget(hFontMenu, szMenuText));
			szMenuText = getLocalised("system_menus_options_auto_ready_weapon_always", "Always");
			cb->AddText(new TextWidget(hFontMenu, szMenuText));
			cb->setValue(int(config.input.autoReadyWeapon));
			
			cb->Move(Vec2f(RATIO_X(m_size.x-9) - cb->m_rect.width(), 0));
			panel->AddElement(cb);
			
			addCenter(panel);
		}
		
		{
			std::string szMenuText = getLocalised("system_menus_options_input_mouse_look_toggle");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			CheckboxWidget * cb = new CheckboxWidget(txt);
			cb->stateChanged = boost::bind(&InputOptionsMenuPage::onChangedToggleMouselook, this, _1);
			cb->iState = config.input.mouseLookToggle ? 1 : 0;
			addCenter(cb);
		}
		
		{
			PanelWidget * panel = new PanelWidget;
			std::string szMenuText = getLocalised("system_menus_options_input_mouse_sensitivity");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			panel->AddElement(txt);
			SliderWidget * sld = new SliderWidget(Vec2f(200, 0));
			sld->valueChanged = boost::bind(&InputOptionsMenuPage::onChangedMouseSensitivity, this, _1);
			sld->setValue(config.input.mouseSensitivity);
			panel->AddElement(sld);
			addCenter(panel);
		}
		
		{
			PanelWidget * panel = new PanelWidget;
			std::string szMenuText = getLocalised("system_menus_options_input_mouse_acceleration", "Mouse acceleration");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			panel->AddElement(txt);
			SliderWidget * sld = new SliderWidget(Vec2f(200, 0));
			sld->valueChanged = boost::bind(&InputOptionsMenuPage::onChangedMouseAcceleration, this, _1);
			sld->setValue(config.input.mouseAcceleration);
			panel->AddElement(sld);
			addCenter(panel);
		}
		
		{
			std::string szMenuText = getLocalised("system_menus_options_raw_mouse_input", "Raw mouse input");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			CheckboxWidget * cb = new CheckboxWidget(txt);
			cb->stateChanged = boost::bind(&InputOptionsMenuPage::onChangedRawMouseInput, this, _1);
			cb->iState = config.input.rawMouseInput ? 1 : 0;
			addCenter(cb);
		}
		
		{
			std::string szMenuText = getLocalised("system_menus_autodescription", "auto_description");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			CheckboxWidget * cb = new CheckboxWidget(txt);
			cb->stateChanged = boost::bind(&InputOptionsMenuPage::onChangedAutoDescription, this, _1);
			cb->iState = config.input.autoDescription ? 1 : 0;
			addCenter(cb);
		}
		
		{
			PanelWidget * panel = new PanelWidget;
			std::string szMenuText = getLocalised("system_menus_options_misc_quicksave_slots", "Quicksave slots");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			panel->AddElement(txt);
			SliderWidget * sld = new SliderWidget(Vec2f(200, 0));
			sld->setMinimum(1);
			sld->valueChanged = boost::bind(&InputOptionsMenuPage::onChangedQuicksaveSlots, this, _1);
			sld->setValue(config.misc.quicksaveSlots);
			panel->AddElement(sld);
			addCenter(panel);
		}
		
		{
			std::string szMenuText = getLocalised("system_menus_border_turning", "Border turning");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			CheckboxWidget * cb = new CheckboxWidget(txt);
			cb->stateChanged = boost::bind(&InputOptionsMenuPage::onChangedBorderTurning, this, _1);
			cb->iState = config.input.borderTurning ? 1 : 0;
			addCenter(cb);
		}
		
		{
			std::string szMenuText = getLocalised("system_menus_alt_rune_recognition", "Improved rune recognition");
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			CheckboxWidget * cb = new CheckboxWidget(txt);
			cb->stateChanged = boost::bind(&InputOptionsMenuPage::onChangedAltRuneRecognition, this, _1);
			cb->iState = config.input.useAltRuneRecognition ? 1 : 0;
			addCenter(cb);
		}
		
		{
			PanelWidget * panel = new PanelWidget;
			std::string szMenuText = getLocalised("system_menus_quick_level_transition", "Quick level transition");
			szMenuText += " ";
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText, Vec2f(20, 0));
			txt->SetCheckOff();
			panel->AddElement(txt);
			
			CycleTextWidget * cb = new CycleTextWidget;
			cb->valueChanged = boost::bind(&InputOptionsMenuPage::onChangedQuickLevelTransition, this, _1, _2);
			szMenuText = getLocalised("system_menus_quick_level_transition_off", "Disabled");
			cb->AddText(new TextWidget(hFontMenu, szMenuText));
			szMenuText = getLocalised("system_menus_quick_level_transition_jump", "Jump");
			cb->AddText(new TextWidget(hFontMenu, szMenuText));
			szMenuText = getLocalised("system_menus_quick_level_transition_immediate", "Immediate");
			cb->AddText(new TextWidget(hFontMenu, szMenuText));
			cb->setValue(int(config.input.quickLevelTransition));
			
			cb->Move(Vec2f(RATIO_X(m_size.x-9) - cb->m_rect.width(), 0));
			panel->AddElement(cb);
			
			addCenter(panel);
		}
		
		{
			ButtonWidget * cb = new ButtonWidget(Vec2f(20, 380), Vec2f(16, 16), "graph/interface/menus/back");
			cb->m_targetMenu = Page_Options;
			cb->SetShortCut(Keyboard::Key_Escape);
			add(cb);
		}
	}
	
private:
	
	void onChangedInvertMouse(int state) {
		config.input.invertMouse = (state) ? true : false;
		GInput->setInvertMouseY(config.input.invertMouse);
	}
	
	void onChangedAutoReadyWeapon(int pos, const std::string & str) {
		ARX_UNUSED(str);
		config.input.autoReadyWeapon = AutoReadyWeapon(pos);
	}
	
	void onChangedToggleMouselook(int state) {
		config.input.mouseLookToggle = (state) ? true : false;
	}
	
	void onChangedMouseSensitivity(int value) {
		config.input.mouseSensitivity = glm::clamp(value, 0, 10);
		GInput->setMouseSensitivity(config.input.mouseSensitivity);
	}
	
	void onChangedMouseAcceleration(int value) {
		config.input.mouseAcceleration = glm::clamp(value, 0, 10);
		GInput->setMouseAcceleration(config.input.mouseAcceleration);
	}
	
	void onChangedRawMouseInput(int state) {
		config.input.rawMouseInput = (state) ? true : false;
		GInput->setRawMouseInput(config.input.rawMouseInput);
	}
	
	void onChangedAutoDescription(int state) {
		config.input.autoDescription = (state) ? true : false;
	}
	
	void onChangedQuicksaveSlots(int value) {
		config.misc.quicksaveSlots = value;
	}
	
	void onChangedBorderTurning(int value) {
		config.input.borderTurning = (value) ? true : false;
	}
	
	void onChangedAltRuneRecognition(int value) {
		config.input.useAltRuneRecognition = (value) ? true : false;
	}
	
	void onChangedQuickLevelTransition(int pos, const std::string & str) {
		ARX_UNUSED(str);
		config.input.quickLevelTransition = QuickLevelTransition(pos);
	}
	
};


class ControlOptionsPage : public MenuPage {
	
public:
	
	ControlOptionsPage(MENUSTATE state)
		: MenuPage(state)
	{}
	
protected:
	
	void addControlRow(long & y, ControlAction controlAction,
	                             const std::string & a,
	                             const char * defaultText = "?",
	                             const char * specialSuffix = "") {
		
		PanelWidget * panel = new PanelWidget;
		
		{
		std::string szMenuText = getLocalised(a, defaultText);
		szMenuText += specialSuffix;
		TextWidget * txt = new TextWidget(hFontControls, szMenuText, Vec2f(20, 0));
		txt->SetCheckOff();
		panel->AddElement(txt);
		}
		
		{
		TextWidget * txt = new TextWidget(hFontControls, "---", Vec2f(150, 0));
		
		txt->m_isKeybind = true;
		txt->m_keybindAction = controlAction;
		txt->m_keybindIndex = 0;
		
		txt->eState=GETTOUCH;
		panel->AddElement(txt);
		}
		
		{
		TextWidget * txt = new TextWidget(hFontControls, "---", Vec2f(245, 0));
		
		txt->m_isKeybind = true;
		txt->m_keybindAction = controlAction;
		txt->m_keybindIndex = 1;
		
		txt->eState=GETTOUCH;
		panel->AddElement(txt);
		}
		
		panel->Move(Vec2f(0, y));
		add(panel);
		y = long(y + panel->m_rect.height() + RATIO_Y(3.f));
	}
	
protected:
	void resetActionKeys() {
		config.setDefaultActionKeys();
		ReInitActionKey();
		bMouseAttack=false;
	}
};


class ControlOptionsMenuPage1 : public ControlOptionsPage {
	
public:
	
	ControlOptionsMenuPage1()
		: ControlOptionsPage(Page_OptionsInputCustomizeKeys1)
	{}
	
	~ControlOptionsMenuPage1() { }
	
	void init() {
		
		long y = static_cast<long>(RATIO_Y(8.f));
		
		addControlRow(y, CONTROLS_CUST_USE,          "system_menus_options_input_customize_controls_mouselook");
		
		addControlRow(y, CONTROLS_CUST_ACTION,       "system_menus_options_input_customize_controls_action_combine");
		addControlRow(y, CONTROLS_CUST_JUMP,         "system_menus_options_input_customize_controls_jump");
		addControlRow(y, CONTROLS_CUST_MAGICMODE,    "system_menus_options_input_customize_controls_magic_mode");
		addControlRow(y, CONTROLS_CUST_STEALTHMODE,  "system_menus_options_input_customize_controls_stealth_mode");
		addControlRow(y, CONTROLS_CUST_WALKFORWARD,  "system_menus_options_input_customize_controls_walk_forward");
		addControlRow(y, CONTROLS_CUST_WALKBACKWARD, "system_menus_options_input_customize_controls_walk_backward");
		addControlRow(y, CONTROLS_CUST_STRAFELEFT,   "system_menus_options_input_customize_controls_strafe_left");
		addControlRow(y, CONTROLS_CUST_STRAFERIGHT,  "system_menus_options_input_customize_controls_strafe_right");
		addControlRow(y, CONTROLS_CUST_LEANLEFT,     "system_menus_options_input_customize_controls_lean_left");
		addControlRow(y, CONTROLS_CUST_LEANRIGHT,    "system_menus_options_input_customize_controls_lean_right");
		addControlRow(y, CONTROLS_CUST_CROUCH,       "system_menus_options_input_customize_controls_crouch");
		addControlRow(y, CONTROLS_CUST_CROUCHTOGGLE, "system_menus_options_input_customize_controls_crouch_toggle");
		
		addControlRow(y, CONTROLS_CUST_STRAFE,       "system_menus_options_input_customize_controls_strafe");
		addControlRow(y, CONTROLS_CUST_CENTERVIEW,   "system_menus_options_input_customize_controls_center_view");
		addControlRow(y, CONTROLS_CUST_FREELOOK,     "system_menus_options_input_customize_controls_freelook");
		
		addControlRow(y, CONTROLS_CUST_TURNLEFT,     "system_menus_options_input_customize_controls_turn_left");
		addControlRow(y, CONTROLS_CUST_TURNRIGHT,    "system_menus_options_input_customize_controls_turn_right");
		addControlRow(y, CONTROLS_CUST_LOOKUP,       "system_menus_options_input_customize_controls_look_up");
		addControlRow(y, CONTROLS_CUST_LOOKDOWN,     "system_menus_options_input_customize_controls_look_down");
		
		addControlRow(y, CONTROLS_CUST_MINIMAP,      "system_menus_options_input_customize_controls_bookmap", "?", "2");
		
		{
			ButtonWidget * cb = new ButtonWidget(Vec2f(20, 380), Vec2f(16, 16), "graph/interface/menus/back");
			cb->m_targetMenu = Page_OptionsInput;
			cb->SetShortCut(Keyboard::Key_Escape);
			cb->clicked = boost::bind(&ControlOptionsMenuPage1::onClickedBack, this);
			add(cb);
		}
		
		{
			std::string szMenuText = getLocalised( "system_menus_options_input_customize_default" );
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText);
			txt->clicked = boost::bind(&ControlOptionsMenuPage1::onClickedDefault, this);
			txt->SetPos(Vec2f((RATIO_X(m_size.x) - txt->m_rect.width())*0.5f, RATIO_Y(380)));
			add(txt);
		}
		
		{
			ButtonWidget * cb = new ButtonWidget(Vec2f(280, 380), Vec2f(16, 16), "graph/interface/menus/next");
			cb->m_targetMenu = Page_OptionsInputCustomizeKeys2;
			add(cb);
		}
	
		ReInitActionKey();
	}
	
private:
	
	void onClickedBack(){
		config.save();
	}
	
	void onClickedDefault() {
		resetActionKeys();
	}
	
};

class ControlOptionsMenuPage2 : public ControlOptionsPage {
	
public:
	
	ControlOptionsMenuPage2()
		: ControlOptionsPage(Page_OptionsInputCustomizeKeys2)
	{}
	
	~ControlOptionsMenuPage2() { }
	
	void init() {
		
		long y = static_cast<long>(RATIO_Y(8.f));
		
		addControlRow(y, CONTROLS_CUST_INVENTORY,         "system_menus_options_input_customize_controls_inventory");
		addControlRow(y, CONTROLS_CUST_BOOK,              "system_menus_options_input_customize_controls_book");
		addControlRow(y, CONTROLS_CUST_BOOKCHARSHEET,     "system_menus_options_input_customize_controls_bookcharsheet");
		addControlRow(y, CONTROLS_CUST_BOOKMAP,           "system_menus_options_input_customize_controls_bookmap");
		addControlRow(y, CONTROLS_CUST_BOOKSPELL,         "system_menus_options_input_customize_controls_bookspell");
		addControlRow(y, CONTROLS_CUST_BOOKQUEST,         "system_menus_options_input_customize_controls_bookquest");
		addControlRow(y, CONTROLS_CUST_DRINKPOTIONLIFE,   "system_menus_options_input_customize_controls_drink_potion_life");
		addControlRow(y, CONTROLS_CUST_DRINKPOTIONMANA,   "system_menus_options_input_customize_controls_drink_potion_mana");
		addControlRow(y, CONTROLS_CUST_DRINKPOTIONCURE,   "system_menus_options_input_customize_controls_drink_potion_cure", "Antidote potion");
		addControlRow(y, CONTROLS_CUST_TORCH,             "system_menus_options_input_customize_controls_torch");
		
		addControlRow(y, CONTROLS_CUST_CANCELCURSPELL,    "system_menus_options_input_customize_controls_cancelcurrentspell");
		addControlRow(y, CONTROLS_CUST_PRECAST1,          "system_menus_options_input_customize_controls_precast1");
		addControlRow(y, CONTROLS_CUST_PRECAST2,          "system_menus_options_input_customize_controls_precast2");
		addControlRow(y, CONTROLS_CUST_PRECAST3,          "system_menus_options_input_customize_controls_precast3");
		
		addControlRow(y, CONTROLS_CUST_WEAPON,            "system_menus_options_input_customize_controls_weapon");
		addControlRow(y, CONTROLS_CUST_UNEQUIPWEAPON,     "system_menus_options_input_customize_controls_unequipweapon");
		
		addControlRow(y, CONTROLS_CUST_PREVIOUS,          "system_menus_options_input_customize_controls_previous");
		addControlRow(y, CONTROLS_CUST_NEXT,              "system_menus_options_input_customize_controls_next");
		
		addControlRow(y, CONTROLS_CUST_QUICKLOAD,         "system_menus_options_input_customize_controls_quickload");
		addControlRow(y, CONTROLS_CUST_QUICKSAVE,         "system_menus_options_input_customize_controls_quicksave");
		
		addControlRow(y, CONTROLS_CUST_TOGGLE_FULLSCREEN, "system_menus_options_input_customize_controls_toggle_fullscreen", "Toggle fullscreen");
		
		if(config.input.allowConsole) {
			addControlRow(y, CONTROLS_CUST_CONSOLE, "system_menus_options_input_customize_controls_console", "Script console");
		}
		
		{
			ButtonWidget * cb = new ButtonWidget(Vec2f(20, 380), Vec2f(16, 16), "graph/interface/menus/back");
			cb->m_targetMenu = Page_OptionsInputCustomizeKeys1;
			cb->SetShortCut(Keyboard::Key_Escape);
			add(cb);
		}
		
		{
			std::string szMenuText = getLocalised( "system_menus_options_input_customize_default" );
			TextWidget * txt = new TextWidget(hFontMenu, szMenuText);
			txt->clicked = boost::bind(&ControlOptionsMenuPage2::onClickedDefault, this);
			txt->SetPos(Vec2f((RATIO_X(m_size.x) - txt->m_rect.width())*0.5f, RATIO_Y(380)));
			add(txt);
		}
		
		ReInitActionKey();
	}
	
private:
	void onClickedDefault() {
		resetActionKeys();
	}
};

class QuitConfirmMenuPage : public MenuPage {
	
public:
	
	QuitConfirmMenuPage()
		: MenuPage(Page_QuitConfirm)
	{}
	
	~QuitConfirmMenuPage() { }
	
	void init() {
		
		{
			TextWidget * txt = new TextWidget(hFontMenu, getLocalised("system_menus_main_quit"));
			txt->SetCheckOff();
			addCenter(txt, true);
		}
		
		{
			TextWidget * txt = new TextWidget(hFontMenu, getLocalised("system_menus_main_editquest_confirm"));
			txt->SetCheckOff();
			addCenter(txt, true);
		}
		
		{
			TextWidget * yes = new TextWidget(hFontMenu, getLocalised("system_yes"), Vec2f(m_size.x - 40, 380));
			yes->clicked = boost::bind(&QuitConfirmMenuPage::onClickedYes, this);
			add(yes);
		}
		
		{
			TextWidget * no = new TextWidget(hFontMenu, getLocalised("system_no"), Vec2f(10, 380));
			no->m_targetMenu = Page_None;
			no->SetShortCut(Keyboard::Key_Escape);
			add(no);
		}
	}
	
private:
	
	void onClickedYes() {
		MenuFader_start(Fade_In, Mode_InGame);
	}
};





void MainMenu::initWindowPages()
{
	
	delete m_window, m_window = NULL;
	
	m_window = new MenuWindow();
	
	{
	NewQuestMenuPage * page = new NewQuestMenuPage();
	page->init();
	m_window->add(page);
	}

	{
	ChooseLoadOrSaveMenuPage * page = new ChooseLoadOrSaveMenuPage();
	page->init();
	m_window->add(page);
	}
	
	{
	LoadMenuPage * page = new LoadMenuPage();
	page->m_rowSpacing = 5;
	page->init();
	m_window->add(page);
	m_window->m_pageLoad = page;
	}
	
	{
	SaveMenuPage * page = new SaveMenuPage();
	page->m_rowSpacing = 5;
	page->init();
	m_window->add(page);
	}
	
	{
	SaveConfirmMenuPage * page = new SaveConfirmMenuPage();
	page->init();
	m_window->add(page);
	m_window->m_pageSaveConfirm = page;
	}
	
	{
	OptionsMenuPage * page = new OptionsMenuPage();
	page->init();
	m_window->add(page);
	}
	
	{
	VideoOptionsMenuPage * page = new VideoOptionsMenuPage();
	page->init();
	m_window->add(page);
	}
	
	{
	RenderOptionsMenuPage * page = new RenderOptionsMenuPage();
	page->init();
	m_window->add(page);
	}
	
	{
	InterfaceOptionsMenuPage * page = new InterfaceOptionsMenuPage();
	page->init();
	m_window->add(page);
	}
	
	{
	AudioOptionsMenuPage * page = new AudioOptionsMenuPage();
	page->init();
	m_window->add(page);
	}
	
	{
	InputOptionsMenuPage * page = new InputOptionsMenuPage();
	page->init();
	m_window->add(page);
	}
	
	{
	ControlOptionsMenuPage1 * page = new ControlOptionsMenuPage1();
	page->init();
	m_window->add(page);
	}
	
	{
	ControlOptionsMenuPage2 * page = new ControlOptionsMenuPage2();
	page->init();
	m_window->add(page);
	}
	
	{
	QuitConfirmMenuPage * page = new QuitConfirmMenuPage();
	page->init();
	m_window->add(page);
	}
}



MainMenu::MainMenu()
	: bReInitAll(false)
	, eOldMenuState(NOP)
	, eOldMenuWindowState(NOP)
	, m_window(NULL)
	, m_background(NULL)
	, m_widgets(new WidgetContainer())
	, m_resumeGame(NULL)
	, m_selected(NULL)
{}

MainMenu::~MainMenu() {
	delete m_window;
	delete m_widgets;
	delete m_background;
}

void MainMenu::init()
{
	m_background = TextureContainer::LoadUI("graph/interface/menus/menu_main_background", TextureContainer::NoColorKey);

	Vec2f pos = Vec2f(370, 100);
	int yOffset = 50;
	
	{
	std::string szMenuText = getLocalised("system_menus_main_resumegame");
	TextWidget * txt = new TextWidget(hFontMainMenu, szMenuText, pos);
	txt->clicked = boost::bind(&MainMenu::onClickedResumeGame, this);
	txt->m_targetMenu = RESUME_GAME;
	m_widgets->add(txt);
	m_resumeGame = txt;
	}
	pos.y += yOffset;
	{
	std::string szMenuText = getLocalised("system_menus_main_newquest");
	TextWidget * txt = new TextWidget(hFontMainMenu, szMenuText, pos);
	txt->clicked = boost::bind(&MainMenu::onClickedNewQuest, this);
	txt->m_targetMenu = Page_NewQuestConfirm;
	m_widgets->add(txt);
	}
	pos.y += yOffset;
	{
	std::string szMenuText = getLocalised("system_menus_main_editquest");
	TextWidget * txt = new TextWidget(hFontMainMenu, szMenuText, pos);
	txt->m_targetMenu = Page_LoadOrSave;
	m_widgets->add(txt);
	}
	pos.y += yOffset;
	{
	std::string szMenuText = getLocalised("system_menus_main_options");
	TextWidget * txt = new TextWidget(hFontMainMenu, szMenuText, pos);
	txt->m_targetMenu = Page_Options;
	m_widgets->add(txt);
	}
	pos.y += yOffset;
	{
	std::string szMenuText = getLocalised("system_menus_main_credits");
	TextWidget * txt = new TextWidget(hFontMainMenu, szMenuText, pos);
	txt->clicked = boost::bind(&MainMenu::onClickedCredits, this);
	txt->m_targetMenu = CREDITS;
	m_widgets->add(txt);
	}
	pos.y += yOffset;
	{
	std::string szMenuText = getLocalised("system_menus_main_quit");
	TextWidget * txt = new TextWidget(hFontMainMenu, szMenuText, pos);
	txt->m_targetMenu = Page_QuitConfirm;
	m_widgets->add(txt);
	}
	pos.y += yOffset;
	
	std::string version = arx_name + " " + arx_version;
	if(!arx_release_codename.empty()) {
		version += " \"";
		version += arx_release_codename;
		version += "\"";
	}

	float verPosX = g_size.right - 20 * g_sizeRatio.x - hFontControls->getTextSize(version).width();
	TextWidget * txt = new TextWidget(hFontControls, version, Vec2f(verPosX / g_sizeRatio.x, 80));
	
	txt->SetCheckOff();
	txt->lColor = Color(127, 127, 127);
	m_widgets->add(txt);
}

void MainMenu::onClickedResumeGame(){
	pTextManage->Clear();
	if(!g_canResumeGame) {
		ARX_QuickLoad();
	} else {
		ARXMenu_ResumeGame();
	}
}

void MainMenu::onClickedNewQuest() {
	if(!g_canResumeGame) {
		ARXMenu_NewQuest();
	}
}

void MainMenu::onClickedCredits() {
	MenuFader_start(Fade_In, Mode_Credits);
}


void MainMenu::Update() {
	
	if(m_resumeGame) {
		if(g_canResumeGame) {
			m_resumeGame->SetCheckOn();
			m_resumeGame->lColor = Color(232, 204, 142);
		} else if(savegames.size() == 0) {
			m_resumeGame->SetCheckOff();
			m_resumeGame->lColor = Color(127, 127, 127);
		}
	}
	
	m_selected = m_widgets->getAtPos(Vec2f(GInput->getMousePosition()));
}

// TODO remove this
extern bool bNoMenu;

void MainMenu::Render() {

	if(bNoMenu)
		return;

	if(m_background) {
		UseRenderState state(render2D().noBlend());
		EERIEDrawBitmap(Rectf(Vec2f(0, 0), g_size.width(), g_size.height()), 0.999f, m_background, Color::white);
	}
	
	BOOST_FOREACH(Widget * w, m_widgets->m_widgets) {
		w->Update();
		w->Render();
	}

	//HIGHLIGHT
	if(m_selected) {
		m_selected->RenderMouseOver();
	}

	//DEBUG ZONE
	GRenderer->ResetTexture(0);
	m_widgets->drawDebug();
}

MainMenu * g_mainMenu;
