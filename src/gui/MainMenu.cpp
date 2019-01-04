/*
 * Copyright 2014-2018 Arx Libertatis Team (see the AUTHORS file)
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
#include "core/ArxGame.h"
#include "core/Core.h"
#include "core/Localisation.h"
#include "core/SaveGame.h"
#include "core/Version.h"

#include "gui/Hud.h"
#include "gui/Menu.h"
#include "gui/MenuPublic.h"
#include "gui/MenuWidgets.h"
#include "gui/Text.h"
#include "gui/TextManager.h"
#include "gui/menu/MenuCursor.h"
#include "gui/menu/MenuFader.h"
#include "gui/menu/MenuPage.h"
#include "gui/widget/CheckboxWidget.h"
#include "gui/widget/CycleTextWidget.h"
#include "gui/widget/KeybindWidget.h"
#include "gui/widget/PanelWidget.h"
#include "gui/widget/SaveSlotWidget.h"
#include "gui/widget/SliderWidget.h"
#include "gui/widget/TextInputWidget.h"
#include "gui/widget/TextWidget.h"
#include "gui/widget/Spacer.h"

#include "graphics/Draw.h"
#include "graphics/Math.h"
#include "graphics/Renderer.h"
#include "graphics/font/Font.h"
#include "graphics/data/TextureContainer.h"

#include "input/Input.h"
#include "input/Keyboard.h"

#include "platform/Platform.h"

#include "scene/GameSound.h"

#include "util/Unicode.h"

#include "window/RenderWindow.h"

class NewQuestMenuPage arx_final : public MenuPage {
	
public:
	
	NewQuestMenuPage()
		: MenuPage(Page_NewQuestConfirm)
	{}
	
	~NewQuestMenuPage() { }
	
	void init() {
		
		reserveBottom();
		
		{
			TextWidget * txt = new TextWidget(hFontMenu, getLocalised("system_menus_main_editquest_confirm"));
			txt->setEnabled(false);
			addCenter(txt);
		}
		
		{
			TextWidget * txt = new TextWidget(hFontMenu, getLocalised("system_menus_main_newquest_confirm"));
			txt->setEnabled(false);
			addCenter(txt);
		}
		
		{
			TextWidget * txt = new TextWidget(hFontMenu, getLocalised("system_yes"));
			txt->clicked = boost::bind(ARXMenu_NewQuest);
			addCorner(txt, BottomRight);
		}
		
		{
			TextWidget * txt = new TextWidget(hFontMenu, getLocalised("system_no"));
			txt->setTargetPage(Page_None);
			txt->setShortcut(Keyboard::Key_Escape);
			addCorner(txt, BottomLeft);
		}
		
	}
	
};

class SaveConfirmMenuPage arx_final : public MenuPage {
	
public:
	
	SaveConfirmMenuPage()
		: MenuPage(Page_SaveConfirm)
		, m_textbox(NULL)
		, pDeleteButton(NULL)
	{ }
	
	~SaveConfirmMenuPage() { }
	
	void init() {
		
		reserveTop();
		reserveBottom();
		
		{
			ButtonWidget * cb = new ButtonWidget(buttonSize(48, 48), "graph/interface/icons/menu_main_save");
			cb->setEnabled(false);
			addCenter(cb);
		}
		
		{
			std::string text = getLocalised("system_menu_editquest_newsavegame");
			TextInputWidget * txt = new TextInputWidget(hFontMenu, text, m_rect);
			txt->setMaxLength(255); // Don't allow the user to enter names that cannot be stored in save files
			txt->unfocused = boost::bind(&SaveConfirmMenuPage::onUnfocusedText, this, _1);
			addCenter(txt);
			m_textbox = txt;
		}
		
		// Delete button
		{
			TextWidget * txt = new TextWidget(hFontMenu, getLocalised("system_menus_main_editquest_delete"));
			txt->clicked = boost::bind(&SaveConfirmMenuPage::onClickedSaveDelete, this, _1);
			txt->setTargetPage(Page_Save);
			txt->setEnabled(m_savegame != SavegameHandle());
			addCorner(txt, TopRight);
		}
		
		// Save button
		{
			TextWidget * txt = new TextWidget(hFontMenu, getLocalised("system_menus_main_editquest_save"));
			txt->clicked = boost::bind(&SaveConfirmMenuPage::onClickedSaveConfirm, this, _1);
			txt->setTargetPage(Page_None);
			addCorner(txt, BottomRight);
		}
		
		addBackButton(Page_Save);
		
		setSaveHandle(m_savegame);
		
	}
	
	void focus() {
		MenuPage::focus();
		activate(m_textbox);
	}
	
	void setSaveHandle(SavegameHandle savegame) {
		
		m_savegame = savegame;
		
		if(pDeleteButton) {
			pDeleteButton->setEnabled(m_savegame != SavegameHandle());
		}
		
		if(m_textbox) {
			if(savegame != SavegameHandle()) {
				m_textbox->setText(savegames[savegame].name);
			} else {
				m_textbox->setText(getLocalised("system_menu_editquest_newsavegame"));
				m_textbox->selectAll();
			}
		}
		
	}
	
private:
	
	SavegameHandle m_savegame;
	TextInputWidget * m_textbox;
	TextWidget * pDeleteButton;
	
	void onUnfocusedText(TextInputWidget * /* widget */) {
		if(m_textbox->text().empty()) {
			setSaveHandle(m_savegame);
		}
	}
	
	void onClickedSaveConfirm(Widget * /* widget */) {
		
		m_textbox->unfocus();
		
		savegames.save(m_textbox->text(), m_savegame, savegame_thumbnail);
	}
	
	void onClickedSaveDelete(Widget * /* widget */) {
		g_mainMenu->bReInitAll = true;
		savegames.remove(m_savegame);
	}
	
};

class LoadMenuPage arx_final : public MenuPage {
	
public:
	
	LoadMenuPage()
		: MenuPage(Page_Load)
		, m_selected(NULL)
		, pLoadConfirm(NULL)
		, pDeleteConfirm(NULL)
	{
		m_rowSpacing = 5;
	}
	
	~LoadMenuPage() { }
	
	void init() {
		
		reserveTop();
		reserveBottom();
		
		{
			ButtonWidget * cb = new ButtonWidget(buttonSize(48, 48), "graph/interface/icons/menu_main_load");
			cb->setEnabled(false);
			addCenter(cb);
		}
		
		// TODO make this list scrollable
		
		// Show quicksaves.
		size_t quicksaveNum = 0;
		BOOST_FOREACH(SavegameHandle save, savegames) {
			if(savegames[save].quicksave) {
				SaveSlotWidget * txt = new SaveSlotWidget(save, ++quicksaveNum, hFontControls, m_rect);
				txt->clicked = boost::bind(&LoadMenuPage::onClickQuestLoad, this, _1);
				txt->doubleClicked = boost::bind(&LoadMenuPage::onDoubleClickQuestLoad, this, _1);
				addCenter(txt);
			}
		}
		
		// Show regular saves.
		BOOST_FOREACH(SavegameHandle save, savegames) {
			if(!savegames[save].quicksave) {
				SaveSlotWidget * txt = new SaveSlotWidget(save, 0, hFontControls, m_rect);
				txt->clicked = boost::bind(&LoadMenuPage::onClickQuestLoad, this, _1);
				txt->doubleClicked = boost::bind(&LoadMenuPage::onDoubleClickQuestLoad, this, _1);
				addCenter(txt);
			}
		}
		
		// Delete button
		{
			TextWidget * txt = new TextWidget(hFontMenu, getLocalised("system_menus_main_editquest_delete"));
			txt->clicked = boost::bind(&LoadMenuPage::onClickQuestDelete, this);
			txt->setTargetPage(Page_Load);
			addCorner(txt, TopRight);
			pDeleteConfirm = txt;
		}
		
		// Load button
		{
			TextWidget * txt = new TextWidget(hFontMenu, getLocalised("system_menus_main_editquest_load"));
			txt->clicked = boost::bind(&LoadMenuPage::onClickQuestLoadConfirm, this);
			txt->setTargetPage(Page_None);
			addCorner(txt, BottomRight);
			pLoadConfirm = txt;
		}
		
		addBackButton(Page_LoadOrSave);
		
	}
	
	void resetSelection() {
		if(m_selected) {
			m_selected->setSelected(false);
			m_selected = NULL;
		}
	}
	
	void focus() {
		MenuPage::focus();
		resetSelection();
		pLoadConfirm->setEnabled(false);
		pDeleteConfirm->setEnabled(false);
	}
	
private:
	
	SaveSlotWidget * m_selected;
	
	TextWidget * pLoadConfirm;
	TextWidget * pDeleteConfirm;
	
	void onClickQuestLoad(Widget * widget) {
		
		resetSelection();
		
		arx_assert(widget->type() == WidgetType_SaveSlot);
		m_selected = static_cast<SaveSlotWidget *>(widget);
		m_selected->setSelected(true);
		
		pLoadConfirm->setEnabled(true);
		pDeleteConfirm->setEnabled(true);
		
	}
	
	void onDoubleClickQuestLoad(Widget * widget) {
		widget->click();
		pLoadConfirm->click();
	}
	
	void onClickQuestLoadConfirm() {
		
		if(m_selected && m_selected->savegame() != SavegameHandle()) {
			
			ARX_SOUND_PlayMenu(g_snd.MENU_CLICK);
			
			LOADQUEST_SLOT = m_selected->savegame();
			
			if(pTextManage) {
				pTextManage->Clear();
			}
			
		}
		
	}
	
	void onClickQuestDelete() {
		
		if(m_selected && m_selected->savegame() != SavegameHandle()) {
			g_mainMenu->bReInitAll = true;
			savegames.remove(m_selected->savegame());
			return;
		}
		
	}
	
};

class SaveMenuPage arx_final : public MenuPage {
	
public:
	
	SaveMenuPage()
		: MenuPage(Page_Save)
	{
		m_rowSpacing = 5;
	}
	
	~SaveMenuPage() { }
	
	void init() {
		
		reserveBottom();
		
		{
			ButtonWidget * cb = new ButtonWidget(buttonSize(48, 48), "graph/interface/icons/menu_main_save");
			cb->setEnabled(false);
			addCenter(cb);
		}
		
		// Show quicksaves.
		size_t quicksaveNum = 0;
		BOOST_FOREACH(SavegameHandle save, savegames) {
			if(savegames[save].quicksave) {
				SaveSlotWidget * txt = new SaveSlotWidget(save, ++quicksaveNum, hFontControls, m_rect);
				txt->clicked = boost::bind(&SaveMenuPage::onClickQuestSaveConfirm, this, _1);
				txt->setTargetPage(Page_SaveConfirm);
				txt->setEnabled(false);
				addCenter(txt);
			}
		}
		
		// Show regular saves.
		BOOST_FOREACH(SavegameHandle save, savegames) {
			if(!savegames[save].quicksave) {
				SaveSlotWidget * txt = new SaveSlotWidget(save, 0, hFontControls, m_rect);
				txt->clicked = boost::bind(&SaveMenuPage::onClickQuestSaveConfirm, this, _1);
				txt->setTargetPage(Page_SaveConfirm);
				addCenter(txt);
			}
		}
		
		for(size_t i = savegames.size(); i <= 15; i++) {
			SaveSlotWidget * txt = new SaveSlotWidget(SavegameHandle(), i, hFontControls, m_rect);
			txt->clicked = boost::bind(&SaveMenuPage::onClickQuestSaveConfirm, this, _1);
			txt->setTargetPage(Page_SaveConfirm);
			addCenter(txt);
		}
		
		addBackButton(Page_LoadOrSave);
		
	}
	
private:
	
	void onClickQuestSaveConfirm(Widget * widget) {
		arx_assert(widget->type() == WidgetType_SaveSlot);
		SavegameHandle savegame = static_cast<SaveSlotWidget *>(widget)->savegame();
		MenuPage * page = g_mainMenu->m_window->getPage(Page_SaveConfirm);
		arx_assert(page->id() == Page_SaveConfirm);
		static_cast<SaveConfirmMenuPage *>(page)->setSaveHandle(savegame);
	}
	
};

class ChooseLoadOrSaveMenuPage arx_final : public MenuPage {
	
	TextWidget * m_loadButton;
	TextWidget * m_saveButton;
	
public:
	
	ChooseLoadOrSaveMenuPage()
		: MenuPage(Page_LoadOrSave)
		, m_loadButton(NULL)
		, m_saveButton(NULL)
	{ }
	
	~ChooseLoadOrSaveMenuPage() { }
	
	void init() {
		
		reserveTop();
		reserveBottom();
		
		{
			std::string label = getLocalised("system_menus_main_editquest_load");
			m_loadButton = new TextWidget(hFontMenu, label);
			m_loadButton->setTargetPage(Page_Load);
			addCenter(m_loadButton);
		}
		
		{
			std::string label = getLocalised("system_menus_main_editquest_save");
			m_saveButton = new TextWidget(hFontMenu, label);
			m_saveButton->setTargetPage(Page_Save);
			addCenter(m_saveButton);
		}
		
		addBackButton(Page_None);
		
	}
	
	void focus() {
		MenuPage::focus();
		m_loadButton->setEnabled(!savegames.empty());
		m_saveButton->setEnabled(g_canResumeGame);
	}
	
};

class OptionsMenuPage arx_final : public MenuPage {
	
public:
	
	OptionsMenuPage()
		: MenuPage(Page_Options)
	{}
	
	~OptionsMenuPage() { }
	
	void init() {
		
		reserveTop();
		reserveBottom();
		
		{
			std::string label = getLocalised("system_menus_options_video");
			TextWidget * txt = new TextWidget(hFontMenu, label);
			txt->setTargetPage(Page_OptionsVideo);
			addCenter(txt);
		}
		
		{
			std::string label = getLocalised("system_menus_options_render");
			TextWidget * txt = new TextWidget(hFontMenu, label);
			txt->setTargetPage(Page_OptionsRender);
			addCenter(txt);
		}
		
		{
			std::string label = getLocalised("system_menus_options_interface");
			TextWidget * txt = new TextWidget(hFontMenu, label);
			txt->setTargetPage(Page_OptionsInterface);
			addCenter(txt);
		}
		
		{
			std::string label = getLocalised("system_menus_options_audio");
			TextWidget * txt = new TextWidget(hFontMenu, label);
			txt->setTargetPage(Page_OptionsAudio);
			addCenter(txt);
		}
		
		{
			std::string label = getLocalised("system_menus_options_input");
			TextWidget * txt = new TextWidget(hFontMenu, label);
			txt->setTargetPage(Page_OptionsInput);
			addCenter(txt);
		}
		
		addBackButton(Page_None);
		
	}
	
};

class VideoOptionsMenuPage arx_final : public MenuPage {
	
	CheckboxWidget * m_fullscreenCheckbox;
	CycleTextWidget * m_resolutionSlider;
	SliderWidget * m_gammaSlider;
	CheckboxWidget * m_minimizeOnFocusLostCheckbox;
	TextWidget * m_applyButton;
	bool m_fullscreen;
	Vec2i m_resolution;
	
public:
	
	VideoOptionsMenuPage()
		: MenuPage(Page_OptionsVideo)
		, m_fullscreenCheckbox(NULL)
		, m_resolutionSlider(NULL)
		, m_gammaSlider(NULL)
		, m_minimizeOnFocusLostCheckbox(NULL)
		, m_applyButton(NULL)
		, m_fullscreen(false)
		, m_resolution(0)
	{ }
	
	~VideoOptionsMenuPage() { }
	
	
	void init() {
		
		reserveBottom();
		
		m_fullscreen = config.video.fullscreen;
		m_resolution = config.video.resolution;
		
		{
			std::string label = getLocalised("system_menus_options_videos_full_screen");
			CheckboxWidget * cb = new CheckboxWidget(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.video.fullscreen);
			cb->stateChanged = boost::bind(&VideoOptionsMenuPage::onChangedFullscreen, this, _1);
			addCenter(cb);
			m_fullscreenCheckbox = cb;
		}
		
		{
			std::string label = getLocalised("system_menus_options_video_resolution");
			m_resolutionSlider = new CycleTextWidget(sliderSize(), hFontMenu, label);
			m_resolutionSlider->valueChanged = boost::bind(&VideoOptionsMenuPage::onChangedResolution, this, _1, _2);
			
			m_resolutionSlider->setEnabled(config.video.fullscreen);
			
			BOOST_FOREACH(const DisplayMode & mode, mainApp->getWindow()->getDisplayModes()) {
				
				// Find the aspect ratio
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
				
				m_resolutionSlider->addEntry(ss.str());
				if(mode.resolution == config.video.resolution) {
					m_resolutionSlider->selectLast();
				}
				
			}
			
			m_resolutionSlider->addEntry(getLocalised("system_menus_options_video_resolution_desktop"));
			if(config.video.resolution == Vec2i(0)) {
				m_resolutionSlider->selectLast();
			}
			
			addCenter(m_resolutionSlider);
		}
		
		{
			std::string label = getLocalised("system_menus_options_video_gamma");
			SliderWidget * sld = new SliderWidget(sliderSize(), hFontMenu, label);
			sld->valueChanged = boost::bind(&VideoOptionsMenuPage::onChangedGamma, this, _1);
			addCenter(sld);
			m_gammaSlider = sld;
			updateGammaSlider();
		}
		
		{
			std::string label = getLocalised("system_menus_options_videos_minimize_on_focus_lost");
			CheckboxWidget * cb = new CheckboxWidget(checkboxSize(), hFontMenu, label);
			cb->stateChanged = boost::bind(&VideoOptionsMenuPage::onChangedMinimizeOnFocusLost, this, _1);
			addCenter(cb);
			m_minimizeOnFocusLostCheckbox = cb;
			updateMinimizeOnFocusLostStateCheckbox();
		}
		
		addCenter(new Spacer(hFontMenu->getLineHeight() / 2));
		
		{
			std::string label = getLocalised("system_menus_options_video_vsync");
			CycleTextWidget * cb = new CycleTextWidget(sliderSize(), hFontMenu, label);
			cb->valueChanged = boost::bind(&VideoOptionsMenuPage::onChangedVSync, this, _1, _2);
			cb->addEntry(getLocalised("system_menus_options_video_vsync_off"));
			if(benchmark::isEnabled()) {
				cb->setValue(0);
				cb->setEnabled(false);
			} else {
				cb->addEntry(getLocalised("system_menus_options_video_vsync_on"));
				cb->addEntry(getLocalised("system_menus_options_video_vsync_auto"));
				cb->setValue(config.video.vsync < 0 ? 2 : config.video.vsync);
			}
			addCenter(cb);
		}
		
		{
			std::string label = getLocalised("system_menus_options_video_fps_limit");
			CycleTextWidget * cb = new CycleTextWidget(sliderSize(), hFontMenu, label);
			cb->valueChanged = boost::bind(&VideoOptionsMenuPage::onChangedFPSLimit, this, _1, _2);
			cb->addEntry(getLocalised("system_menus_options_video_vsync_off"));
			if(benchmark::isEnabled()) {
				cb->setValue(0);
				cb->setEnabled(false);
			} else {
				cb->addEntry("60");
				cb->addEntry("120");
				cb->addEntry("240");
				cb->addEntry("480");
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
					cb->addEntry(boost::lexical_cast<std::string>(config.video.fpsLimit));
					cb->setValue(5);
				}
			}
			addCenter(cb);
		}
		
		addCenter(new Spacer(hFontMenu->getLineHeight() / 2));
		
		{
			std::string label = getLocalised("system_menus_options_video_fov");
			SliderWidget * sld = new SliderWidget(sliderSize(), hFontMenu, label);
			sld->valueChanged = boost::bind(&VideoOptionsMenuPage::onChangedFov, this, _1);
			sld->setValue(glm::clamp(int((config.video.fov - 75.f) / 5.f), 0, 10));
			addCenter(sld);
		}
		
		{
			TextWidget * txt = new TextWidget(hFontMenu, getLocalised("system_menus_video_apply"));
			txt->clicked = boost::bind(&VideoOptionsMenuPage::onClickedApply, this, _1);
			txt->setEnabled(false);
			addCorner(txt, BottomRight);
			m_applyButton = txt;
		}
		
		addBackButton(Page_Options);
		
	}
	
private:
	
	void updateGammaSlider() {
		
		if(m_fullscreen) {
			m_gammaSlider->setValue(int(config.video.gamma));
			m_gammaSlider->setEnabled(true);
		} else {
			m_gammaSlider->setEnabled(false);
			m_gammaSlider->setValue(5);
		}
		
	}
	
	void updateMinimizeOnFocusLostStateCheckbox() {
		
		if(m_fullscreen) {
			Window::MinimizeSetting minimize = mainApp->getWindow()->willMinimizeOnFocusLost();
			bool checked = (minimize == Window::Enabled || minimize == Window::AlwaysEnabled);
			bool enabled = (minimize != Window::AlwaysDisabled && minimize != Window::AlwaysEnabled);
			m_minimizeOnFocusLostCheckbox->setChecked(checked);
			m_minimizeOnFocusLostCheckbox->setEnabled(enabled);
		} else {
			m_minimizeOnFocusLostCheckbox->setEnabled(false);
			m_minimizeOnFocusLostCheckbox->setChecked(false);
		}
		
	}
	
	void updateApplyButton() {
		
		bool enable = m_resolution != config.video.resolution || m_fullscreen != config.video.fullscreen;
		m_applyButton->setEnabled(enable);
		
	}
	
	void onChangedFullscreen(bool checked) {
		
		m_fullscreen = checked;
		
		m_resolutionSlider->setEnabled(m_fullscreen);
		updateGammaSlider();
		updateMinimizeOnFocusLostStateCheckbox();
		updateApplyButton();
		
	}
	
	void onChangedResolution(int pos, const std::string & str) {
		
		ARX_UNUSED(str);
		
		const RenderWindow::DisplayModes & modes = mainApp->getWindow()->getDisplayModes();
		if(size_t(pos) < modes.size()) {
			m_resolution = modes[size_t(pos)].resolution;
		} else {
			m_resolution = Vec2i(0);
		}
		
		updateApplyButton();
		
	}
	
	void onChangedGamma(int state) {
		if(m_gammaSlider->isEnabled()) {
			ARXMenu_Options_Video_SetGamma(float(state));
		}
	}
	
	void onChangedMinimizeOnFocusLost(bool checked) {
		if(m_minimizeOnFocusLostCheckbox->isEnabled()) {
			config.window.minimizeOnFocusLost = checked;
			mainApp->getWindow()->setMinimizeOnFocusLost(config.window.minimizeOnFocusLost);
		}
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
	
	void onChangedFov(int state) {
		config.video.fov = 75.f + float(state) * 5.f;
	}
	
	void onClickedApply(Widget * /* widget */) {
		ARXMenu_Private_Options_Video_SetResolution(m_fullscreen, m_resolution.x, m_resolution.y);
		g_mainMenu->bReInitAll = true;
	}
	
};

class RenderOptionsMenuPage arx_final : public MenuPage {
	
public:
	
	RenderOptionsMenuPage()
		: MenuPage(Page_OptionsRender)
		, m_alphaCutoutAntialiasingCycleText(NULL)
	{ }
	
	~RenderOptionsMenuPage() { }
	
	CycleTextWidget * m_alphaCutoutAntialiasingCycleText;
	
	void init() {
		
		reserveBottom();
		
		// Renderer selection
		{
			std::string label = getLocalised("system_menus_options_video_renderer");
			CycleTextWidget * slider = new CycleTextWidget(sliderSize(), hFontMenu, label);
			slider->valueChanged = boost::bind(&RenderOptionsMenuPage::onChangedRenderer, this, _1, _2);
			slider->addEntry("Auto-Select");
			slider->selectLast();
			slider->addEntry("OpenGL");
			if(config.video.renderer == "OpenGL") {
				slider->selectLast();
			}
			addCenter(slider);
		}
		
		addCenter(new Spacer(hFontMenu->getLineHeight() / 2));
		
		{
			std::string label = getLocalised("system_menus_options_detail");
			CycleTextWidget * cb = new CycleTextWidget(sliderSize(), hFontMenu, label);
			cb->valueChanged = boost::bind(&RenderOptionsMenuPage::onChangedQuality, this, _1, _2);
			cb->addEntry(getLocalised("system_menus_options_video_texture_low"));
			cb->addEntry(getLocalised("system_menus_options_video_texture_med"));
			cb->addEntry(getLocalised("system_menus_options_video_texture_high"));
			cb->setValue(config.video.levelOfDetail);
			addCenter(cb);
		}
		
		{
			std::string label = getLocalised("system_menus_options_video_brouillard");
			SliderWidget * sld = new SliderWidget(sliderSize(), hFontMenu, label);
			sld->valueChanged = boost::bind(&RenderOptionsMenuPage::onChangedFogDistance, this, _1);
			sld->setValue(int(config.video.fogDistance));
			addCenter(sld);
		}
		
		{
			std::string label = getLocalised("system_menus_options_video_antialiasing");
			CheckboxWidget * cb = new CheckboxWidget(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.video.antialiasing);
			cb->stateChanged = boost::bind(&RenderOptionsMenuPage::onChangedAntialiasing, this, _1);
			addCenter(cb);
		}
		
		{
			std::string label = getLocalised("system_menus_options_video_colorkey_antialiasing");
			CheckboxWidget * cb = new CheckboxWidget(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.video.colorkeyAntialiasing);
			cb->stateChanged = boost::bind(&RenderOptionsMenuPage::onChangedColorkeyAntialiasing, this, _1);
			addCenter(cb);
		}
		
		{
			std::string label = getLocalised("system_menus_options_video_alpha_cutout_antialising");
			CycleTextWidget * cb = new CycleTextWidget(sliderSize(), hFontMenu, label);
			cb->valueChanged = boost::bind(&RenderOptionsMenuPage::onChangedAlphaCutoutAntialiasing, this, _1, _2);
			cb->addEntry(getLocalised("system_menus_options_video_alpha_cutout_antialising_off"));
			Renderer::AlphaCutoutAntialising maxAA = GRenderer->getMaxSupportedAlphaCutoutAntialiasing();
			if(maxAA >= Renderer::FuzzyAlphaCutoutAA) {
				cb->addEntry(getLocalised("system_menus_options_video_alpha_cutout_antialising_fuzzy"));
			}
			if(maxAA >= Renderer::CrispAlphaCutoutAA) {
				cb->addEntry(getLocalised("system_menus_options_video_alpha_cutout_antialising_crisp"));
			}
			m_alphaCutoutAntialiasingCycleText = cb;
			setAlphaCutoutAntialisingState();
			addCenter(cb);
		}
		
		{
			std::string label = getLocalised("system_menus_options_video_texture_filter_anisotropic");
			CycleTextWidget * cb = new CycleTextWidget(sliderSize(), hFontMenu, label);
			cb->valueChanged = boost::bind(&RenderOptionsMenuPage::onChangedMaxAnisotropy, this, _1, _2);
			cb->addEntry(getLocalised("system_menus_options_video_filter_anisotropic_off"));
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
					cb->addEntry(oss.str());
					if(config.video.maxAnisotropicFiltering == anisotropy) {
						selected = i;
					}
					if(config.video.maxAnisotropicFiltering > anisotropy
					   && config.video.maxAnisotropicFiltering < anisotropy * 2
					   && config.video.maxAnisotropicFiltering <= maxAnisotropy) {
						oss.str(std::string());
						oss << 'x' << config.video.maxAnisotropicFiltering;
						cb->addEntry(oss.str());
						selected = ++i;
					}
					if(anisotropy == maxAnisotropy) {
						i++;
						break;
					}
				}
				cb->addEntry(getLocalised("system_menus_options_video_filter_anisotropic_max"));
				if(config.video.maxAnisotropicFiltering > maxAnisotropy) {
					selected = i;
				}
			}
			cb->setValue(selected);
			cb->setEnabled(maxAnisotropy > 1);
			addCenter(cb);
		}
		
		addBackButton(Page_Options);
		
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
	
	void onChangedAntialiasing(bool checked) {
		config.video.antialiasing = checked;
		setAlphaCutoutAntialisingState();
	}
	
	void onChangedColorkeyAntialiasing(bool checked) {
		config.video.colorkeyAntialiasing = checked;
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

class InterfaceOptionsMenuPage arx_final : public MenuPage {
	
public:
	
	InterfaceOptionsMenuPage()
		: MenuPage(Page_OptionsInterface)
	{ }
	
	~InterfaceOptionsMenuPage() { }
	
	void init() {
		
		reserveBottom();
		
		{
			std::string label = getLocalised("system_menus_options_video_crosshair");
			label = getLocalised("system_menus_options_interface_crosshair", label);
			CheckboxWidget * cb = new CheckboxWidget(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.interface.showCrosshair);
			cb->stateChanged = boost::bind(&InterfaceOptionsMenuPage::onChangedCrosshair, this, _1);
			addCenter(cb);
		}
		
		{
			std::string label = getLocalised("system_menus_options_interface_limit_speech_width");
			CheckboxWidget * cb = new CheckboxWidget(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.interface.limitSpeechWidth);
			cb->stateChanged = boost::bind(&InterfaceOptionsMenuPage::onChangedSpeechWidth, this, _1);
			addCenter(cb);
		}
		
		{
			std::string label = getLocalised("system_menus_options_interface_cinematic_widescreen_mode");
			CycleTextWidget * cb = new CycleTextWidget(sliderSize(), hFontMenu, label, hFontControls);
			cb->valueChanged = boost::bind(&InterfaceOptionsMenuPage::onChangedCinematicMode, this, _1, _2);
			cb->addEntry(getLocalised("system_menus_options_interface_letterbox"));
			cb->addEntry(getLocalised("system_menus_options_interface_hard_edges"));
			cb->addEntry(getLocalised("system_menus_options_interface_fade_edges"));
			cb->setValue(config.interface.cinematicWidescreenMode);
			addCenter(cb);
		}
		
		addCenter(new Spacer(hFontMenu->getLineHeight() / 2));
		
		{
			std::string label = getLocalised("system_menus_options_interface_hud_scale");
			SliderWidget * sld = new SliderWidget(sliderSize(), hFontMenu, label);
			sld->valueChanged = boost::bind(&InterfaceOptionsMenuPage::onChangedHudScale, this, _1);
			sld->setValue(int(config.interface.hudScale * 10.f));
			addCenter(sld);
		}
		
		{
			std::string label = getLocalised("system_menus_options_interface_hud_scale_integer");
			CheckboxWidget * cb = new CheckboxWidget(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.interface.hudScaleInteger);
			cb->stateChanged = boost::bind(&InterfaceOptionsMenuPage::onChangedHudScaleInteger, this, _1);
			addCenter(cb);
		}
		
		{
			std::string label = getLocalised("system_menus_options_interface_book_scale");
			SliderWidget * sld = new SliderWidget(sliderSize(), hFontMenu, label);
			sld->valueChanged = boost::bind(&InterfaceOptionsMenuPage::onChangedBookScale, this, _1);
			sld->setValue(int(config.interface.bookScale * 10.f));
			addCenter(sld);
		}
		
		{
			std::string label = getLocalised("system_menus_options_interface_book_scale_integer");
			CheckboxWidget * cb = new CheckboxWidget(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.interface.bookScaleInteger);
			cb->stateChanged = boost::bind(&InterfaceOptionsMenuPage::onChangedBookScaleInteger, this, _1);
			addCenter(cb);
		}
		
		{
			std::string label = getLocalised("system_menus_options_interface_cursor_scale");
			SliderWidget * sld = new SliderWidget(sliderSize(), hFontMenu, label);
			sld->valueChanged = boost::bind(&InterfaceOptionsMenuPage::onChangedCursorScale, this, _1);
			sld->setValue(int(config.interface.cursorScale * 10.f));
			addCenter(sld);
		}
		
		{
			std::string label = getLocalised("system_menus_options_interface_cursor_scale_integer");
			CheckboxWidget * cb = new CheckboxWidget(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.interface.cursorScaleInteger);
			cb->stateChanged = boost::bind(&InterfaceOptionsMenuPage::onChangedCursorScaleInteger, this, _1);
			addCenter(cb);
		}
		
		{
			std::string label = getLocalised("system_menus_options_interface_scale_filter");
			CycleTextWidget * cb = new CycleTextWidget(sliderSize(), hFontMenu, label);
			cb->valueChanged = boost::bind(&InterfaceOptionsMenuPage::onChangedScaleFilter, this, _1, _2);
			cb->addEntry(getLocalised("system_menus_options_video_filter_nearest"));
			cb->addEntry(getLocalised("system_menus_options_video_filter_bilinear"));
			cb->setValue(config.interface.scaleFilter);
			addCenter(cb);
		}
		
		{
			std::string label = getLocalised("system_menus_options_interface_font_size");
			SliderWidget * sld = new SliderWidget(sliderSize(), hFontMenu, label);
			sld->valueChanged = boost::bind(&InterfaceOptionsMenuPage::onChangedFontSize, this, _1);
			sld->setValue(int(glm::clamp((config.interface.fontSize - 0.75f) * 20.f + 0.5f, 0.f, 10.f)));
			addCenter(sld);
		}
		
		{
			std::string label = getLocalised("system_menus_options_interface_font_weight");
			CycleTextWidget * cb = new CycleTextWidget(sliderSize(), hFontMenu, label);
			cb->valueChanged = boost::bind(&InterfaceOptionsMenuPage::onChangedFontWeight, this, _1, _2);
			cb->addEntry(getLocalised("system_menus_options_interface_font_weight_0"));
			cb->addEntry(getLocalised("system_menus_options_interface_font_weight_1"));
			cb->addEntry(getLocalised("system_menus_options_interface_font_weight_2"));
			cb->addEntry(getLocalised("system_menus_options_interface_font_weight_3"));
			cb->addEntry(getLocalised("system_menus_options_interface_font_weight_4"));
			cb->addEntry(getLocalised("system_menus_options_interface_font_weight_5"));
			cb->setValue(config.interface.fontWeight);
			addCenter(cb);
		}
		
		addBackButton(Page_Options);
		
	}
	
private:
	
	void onChangedCrosshair(bool checked) {
		config.interface.showCrosshair = checked;
	}
	
	void onChangedSpeechWidth(bool checked) {
		config.interface.limitSpeechWidth = checked;
	}
	
	void onChangedCinematicMode(int pos, const std::string & str) {
		ARX_UNUSED(str);
		
		config.interface.cinematicWidescreenMode = CinematicWidescreenMode(pos);
	}
	
	void onChangedHudScale(int state) {
		config.interface.hudScale = float(state) * 0.1f;
		g_hudRoot.recalcScale();
	}
	
	void onChangedHudScaleInteger(bool checked) {
		config.interface.hudScaleInteger = checked;
		g_hudRoot.recalcScale();
	}
	
	void onChangedBookScale(int state) {
		config.interface.bookScale = float(state) * 0.1f;
	}
	
	void onChangedBookScaleInteger(bool checked) {
		config.interface.bookScaleInteger = checked;
	}
	
	void onChangedCursorScale(int state) {
		config.interface.cursorScale = float(state) * 0.1f;
	}
	
	void onChangedCursorScaleInteger(bool checked) {
		config.interface.cursorScaleInteger = checked;
	}
	
	void onChangedScaleFilter(int pos, const std::string & str) {
		ARX_UNUSED(str);
		config.interface.scaleFilter = UIScaleFilter(pos);
	}
	
	void onChangedFontSize(int state) {
		config.interface.fontSize = 0.75f + float(state) / 20.f;
		ARX_Text_Init();
	}
	
	void onChangedFontWeight(int pos, const std::string & str) {
		ARX_UNUSED(str);
		config.interface.fontWeight = pos;
		ARX_Text_Init();
	}
	
};


class AudioOptionsMenuPage arx_final : public MenuPage {
	
public:
	
	AudioOptionsMenuPage()
		: MenuPage(Page_OptionsAudio)
	{}
	
	~AudioOptionsMenuPage() { }
	
	void init() {
		
		reserveBottom();
		
		// Audio backend selection
		{
			std::string label = getLocalised("system_menus_options_audio_device");
			CycleTextWidget * slider = new CycleTextWidget(sliderSize(), hFontMenu, label, hFontControls);
			slider->valueChanged = boost::bind(&AudioOptionsMenuPage::onChangedDevice, this, _1, _2);
			slider->addEntry("Default");
			slider->selectLast();
			BOOST_FOREACH(const std::string & device, audio::getDevices()) {
				slider->addEntry(device);
				if(config.audio.device == device) {
					slider->selectLast();
				}
			}
			addCenter(slider);
		}
		
		addCenter(new Spacer(hFontMenu->getLineHeight() / 2));
		
		{
			std::string label = getLocalised("system_menus_options_audio_master_volume");
			SliderWidget * sld = new SliderWidget(sliderSize(), hFontMenu, label);
			sld->valueChanged = boost::bind(&AudioOptionsMenuPage::onChangedMasterVolume, this, _1);
			sld->setValue(int(config.audio.volume)); // TODO use float sliders
			addCenter(sld);
		}
		
		{
			std::string label = getLocalised("system_menus_options_audio_effects_volume");
			SliderWidget * sld = new SliderWidget(sliderSize(), hFontMenu, label);
			sld->valueChanged = boost::bind(&AudioOptionsMenuPage::onChangedEffectsVolume, this, _1);
			sld->setValue(int(config.audio.sfxVolume));
			addCenter(sld);
		}
		
		{
			std::string label = getLocalised("system_menus_options_audio_speech_volume");
			SliderWidget * sld = new SliderWidget(sliderSize(), hFontMenu, label);
			sld->valueChanged = boost::bind(&AudioOptionsMenuPage::onChangedSpeechVolume, this, _1);
			sld->setValue(int(config.audio.speechVolume));
			addCenter(sld);
		}
		
		{
			std::string label = getLocalised("system_menus_options_audio_ambiance_volume");
			SliderWidget * sld = new SliderWidget(sliderSize(), hFontMenu, label);
			sld->valueChanged = boost::bind(&AudioOptionsMenuPage::onChangedAmbianceVolume, this, _1);
			sld->setValue(int(config.audio.ambianceVolume));
			addCenter(sld);
		}
		
		{
			std::string label = getLocalised("system_menus_options_audio_mute_on_focus_lost");
			CheckboxWidget * cb = new CheckboxWidget(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.audio.muteOnFocusLost);
			cb->stateChanged = boost::bind(&AudioOptionsMenuPage::onChangedMuteOnFocusLost, this, _1);
			addCenter(cb);
		}
		
		addCenter(new Spacer(hFontMenu->getLineHeight() / 2));
		
		{
			std::string label = getLocalised("system_menus_options_audio_eax");
			CheckboxWidget * cb = new CheckboxWidget(checkboxSize(), hFontMenu, label);
			if(audio::isReverbSupported()) {
				cb->setChecked(config.audio.eax);
			} else {
				cb->setEnabled(false);
			}
			cb->stateChanged = boost::bind(&AudioOptionsMenuPage::onChangedEax, this, _1);
			addCenter(cb);
		}
		
		audio::HRTFStatus hrtf = audio::getHRTFStatus();
		if(hrtf != audio::HRTFUnavailable) {
			std::string label = getLocalised("system_menus_options_audio_hrtf");
			CycleTextWidget * slider = new CycleTextWidget(sliderSize(), hFontMenu, label);
			slider->valueChanged = boost::bind(&AudioOptionsMenuPage::onChangedHRTF, this, _1, _2);
			slider->addEntry("Disabled");
			if(config.audio.hrtf == audio::HRTFDisable || hrtf == audio::HRTFForbidden) {
				slider->selectLast();
			}
			slider->addEntry("Automatic");
			if(config.audio.hrtf == audio::HRTFDefault) {
				slider->selectLast();
			}
			slider->addEntry("Enabled");
			if(config.audio.hrtf == audio::HRTFEnable || hrtf == audio::HRTFRequired) {
				slider->selectLast();
			}
			if(hrtf == audio::HRTFRequired || hrtf == audio::HRTFForbidden) {
				slider->setEnabled(false);
			}
			addCenter(slider);
		}
		
		addBackButton(Page_Options);
		
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
	
	void onChangedEax(bool checked) {
		config.audio.eax = checked;
		ARX_SOUND_SetReverb(config.audio.eax);
	}
	
	void onChangedHRTF(int pos, const std::string & str) {
		ARX_UNUSED(str);
		switch(pos) {
			case 0: config.audio.hrtf = audio::HRTFDisable; break;
			case 1: config.audio.hrtf = audio::HRTFDefault; break;
			case 2: config.audio.hrtf = audio::HRTFEnable; break;
			default: arx_unreachable();
		}
		audio::setHRTFEnabled(config.audio.hrtf);
	}
	
	void onChangedMuteOnFocusLost(bool checked) {
		config.audio.muteOnFocusLost = checked;
		if(!mainApp->getWindow()->hasFocus()) {
			ARXMenu_Options_Audio_SetMuted(config.audio.muteOnFocusLost);
		}
	}
	
};

class InputOptionsMenuPage arx_final : public MenuPage {
	
public:
	
	InputOptionsMenuPage()
		: MenuPage(Page_OptionsInput)
	{}
	
	~InputOptionsMenuPage() { }
	
	void init() {
		
		reserveBottom();
		
		{
			std::string label = getLocalised("system_menus_options_input_customize_controls");
			TextWidget * txt = new TextWidget(hFontMenu, label);
			txt->setTargetPage(Page_OptionsInputCustomizeKeys1);
			addCenter(txt);
		}
		
		addCenter(new Spacer(hFontMenu->getLineHeight() / 2));
		
		{
			std::string label = getLocalised("system_menus_options_input_invert_mouse");
			CheckboxWidget * cb = new CheckboxWidget(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.input.invertMouse);
			cb->stateChanged = boost::bind(&InputOptionsMenuPage::onChangedInvertMouse, this, _1);
			addCenter(cb);
		}
		
		{
			std::string label = getLocalised("system_menus_options_auto_ready_weapon");
			CycleTextWidget * cb = new CycleTextWidget(sliderSize(), hFontMenu, label);
			cb->valueChanged = boost::bind(&InputOptionsMenuPage::onChangedAutoReadyWeapon, this, _1, _2);
			cb->addEntry(getLocalised("system_menus_options_auto_ready_weapon_off"));
			cb->addEntry(getLocalised("system_menus_options_auto_ready_weapon_enemies"));
			cb->addEntry(getLocalised("system_menus_options_auto_ready_weapon_always"));
			cb->setValue(int(config.input.autoReadyWeapon));
			addCenter(cb);
		}
		
		{
			std::string label = getLocalised("system_menus_options_input_mouse_look_toggle");
			CheckboxWidget * cb = new CheckboxWidget(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.input.mouseLookToggle);
			cb->stateChanged = boost::bind(&InputOptionsMenuPage::onChangedToggleMouselook, this, _1);
			addCenter(cb);
		}
		
		{
			std::string label = getLocalised("system_menus_options_input_mouse_sensitivity");
			SliderWidget * sld = new SliderWidget(sliderSize(), hFontMenu, label);
			sld->valueChanged = boost::bind(&InputOptionsMenuPage::onChangedMouseSensitivity, this, _1);
			sld->setValue(config.input.mouseSensitivity);
			addCenter(sld);
		}
		
		{
			std::string label = getLocalised("system_menus_options_input_mouse_acceleration");
			SliderWidget * sld = new SliderWidget(sliderSize(), hFontMenu, label);
			sld->valueChanged = boost::bind(&InputOptionsMenuPage::onChangedMouseAcceleration, this, _1);
			sld->setValue(config.input.mouseAcceleration);
			addCenter(sld);
		}
		
		{
			std::string label = getLocalised("system_menus_options_raw_mouse_input");
			CheckboxWidget * cb = new CheckboxWidget(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.input.rawMouseInput);
			cb->stateChanged = boost::bind(&InputOptionsMenuPage::onChangedRawMouseInput, this, _1);
			addCenter(cb);
		}
		
		{
			std::string label = getLocalised("system_menus_autodescription");
			CheckboxWidget * cb = new CheckboxWidget(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.input.autoDescription);
			cb->stateChanged = boost::bind(&InputOptionsMenuPage::onChangedAutoDescription, this, _1);
			addCenter(cb);
		}
		
		{
			std::string label = getLocalised("system_menus_options_misc_quicksave_slots");
			SliderWidget * sld = new SliderWidget(sliderSize(), hFontMenu, label);
			sld->setMinimum(1);
			sld->valueChanged = boost::bind(&InputOptionsMenuPage::onChangedQuicksaveSlots, this, _1);
			sld->setValue(config.misc.quicksaveSlots);
			addCenter(sld);
		}
		
		{
			std::string label = getLocalised("system_menus_border_turning");
			CheckboxWidget * cb = new CheckboxWidget(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.input.borderTurning);
			cb->stateChanged = boost::bind(&InputOptionsMenuPage::onChangedBorderTurning, this, _1);
			addCenter(cb);
		}
		
		{
			std::string label = getLocalised("system_menus_alt_rune_recognition");
			CheckboxWidget * cb = new CheckboxWidget(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.input.useAltRuneRecognition);
			cb->stateChanged = boost::bind(&InputOptionsMenuPage::onChangedAltRuneRecognition, this, _1);
			addCenter(cb);
		}
		
		{
			std::string label = getLocalised("system_menus_quick_level_transition");
			CycleTextWidget * cb = new CycleTextWidget(sliderSize(), hFontMenu, label);
			cb->valueChanged = boost::bind(&InputOptionsMenuPage::onChangedQuickLevelTransition, this, _1, _2);
			cb->addEntry(getLocalised("system_menus_quick_level_transition_off"));
			cb->addEntry(getLocalised("system_menus_quick_level_transition_jump"));
			cb->addEntry(getLocalised("system_menus_quick_level_transition_immediate"));
			cb->setValue(int(config.input.quickLevelTransition));
			addCenter(cb);
		}
		
		addBackButton(Page_Options);
		
	}
	
private:
	
	void onChangedInvertMouse(bool checked) {
		config.input.invertMouse = checked;
		GInput->setInvertMouseY(config.input.invertMouse);
	}
	
	void onChangedAutoReadyWeapon(int pos, const std::string & str) {
		ARX_UNUSED(str);
		config.input.autoReadyWeapon = AutoReadyWeapon(pos);
	}
	
	void onChangedToggleMouselook(bool checked) {
		config.input.mouseLookToggle = checked;
	}
	
	void onChangedMouseSensitivity(int value) {
		config.input.mouseSensitivity = glm::clamp(value, 0, 10);
		GInput->setMouseSensitivity(config.input.mouseSensitivity);
	}
	
	void onChangedMouseAcceleration(int value) {
		config.input.mouseAcceleration = glm::clamp(value, 0, 10);
		GInput->setMouseAcceleration(config.input.mouseAcceleration);
	}
	
	void onChangedRawMouseInput(bool checked) {
		config.input.rawMouseInput = checked;
		GInput->setRawMouseInput(config.input.rawMouseInput);
	}
	
	void onChangedAutoDescription(bool checked) {
		config.input.autoDescription = checked;
	}
	
	void onChangedQuicksaveSlots(int value) {
		config.misc.quicksaveSlots = value;
	}
	
	void onChangedBorderTurning(bool checked) {
		config.input.borderTurning = checked;
	}
	
	void onChangedAltRuneRecognition(bool checked) {
		config.input.useAltRuneRecognition = checked;
	}
	
	void onChangedQuickLevelTransition(int pos, const std::string & str) {
		ARX_UNUSED(str);
		config.input.quickLevelTransition = QuickLevelTransition(pos);
	}
	
};

class ControlOptionsPage : public MenuPage {
	
public:
	
	explicit ControlOptionsPage(MENUSTATE state)
		: MenuPage(state)
	{
		m_rowSpacing = 3;
	}
	
	void focus() {
		MenuPage::focus();
		reinitActionKeys();
	}
	
protected:
	
	void addControlRow(ControlAction controlAction, const std::string & text) {
		
		PanelWidget * panel = new PanelWidget;
		
		{
			TextWidget * txt = new TextWidget(hFontControls, getLocalised(text));
			txt->setEnabled(false);
			panel->add(txt);
		}
		
		for(size_t i = 0; i < 2; i++) {
			KeybindWidget * keybind = new KeybindWidget(controlAction, i, hFontControls);
			keybind->keyChanged = boost::bind(&ControlOptionsPage::onKeyChanged, this, _1);
			keybind->setPosition(RATIO_2(Vec2f(150.f + 95.f * float(i), 0.f)));
			panel->add(keybind);
		}
		
		addCenter(panel, false);
		
	}
	
protected:
	
	void reinitActionKeys() {
		
		BOOST_FOREACH(Widget * widget, m_children.widgets()) {
			if(widget->type() == WidgetType_Panel) {
				PanelWidget * panel = static_cast<PanelWidget *>(widget);
				BOOST_FOREACH(Widget * child, panel->children()) {
					if(child->type() == WidgetType_Keybind) {
						KeybindWidget * keybind = static_cast<KeybindWidget *>(child);
						keybind->setKey(config.actions[keybind->action()].key[keybind->index()]);
					}
				}
			}
		}
		
	}
	
	void resetActionKeys() {
		config.setDefaultActionKeys();
		reinitActionKeys();
	}
	
	void onKeyChanged(KeybindWidget * keybind) {
		
		if(keybind->action() == CONTROLS_CUST_ACTION && !(keybind->key() & int(Mouse::ButtonBase))) {
			// Only allow mouse buttons for for the action binding
			keybind->setKey(config.actions[keybind->action()].key[keybind->index()]);
			return;
		}
		
		if(config.setActionKey(keybind->action(), keybind->index(), keybind->key())) {
			// Changing one key can unbind others so we have to update them all
			reinitActionKeys();
		}
		
	}
	
};


class ControlOptionsMenuPage1 arx_final : public ControlOptionsPage {
	
public:
	
	ControlOptionsMenuPage1()
		: ControlOptionsPage(Page_OptionsInputCustomizeKeys1)
	{ }
	
	~ControlOptionsMenuPage1() { }
	
	void init() {
		
		reserveBottom();
		
		addControlRow(CONTROLS_CUST_USE,          "system_menus_options_input_customize_controls_mouselook");
		
		addControlRow(CONTROLS_CUST_ACTION,       "system_menus_options_input_customize_controls_action_combine");
		addControlRow(CONTROLS_CUST_JUMP,         "system_menus_options_input_customize_controls_jump");
		addControlRow(CONTROLS_CUST_MAGICMODE,    "system_menus_options_input_customize_controls_magic_mode");
		addControlRow(CONTROLS_CUST_STEALTHMODE,  "system_menus_options_input_customize_controls_stealth_mode");
		addControlRow(CONTROLS_CUST_WALKFORWARD,  "system_menus_options_input_customize_controls_walk_forward");
		addControlRow(CONTROLS_CUST_WALKBACKWARD, "system_menus_options_input_customize_controls_walk_backward");
		addControlRow(CONTROLS_CUST_STRAFELEFT,   "system_menus_options_input_customize_controls_strafe_left");
		addControlRow(CONTROLS_CUST_STRAFERIGHT,  "system_menus_options_input_customize_controls_strafe_right");
		addControlRow(CONTROLS_CUST_LEANLEFT,     "system_menus_options_input_customize_controls_lean_left");
		addControlRow(CONTROLS_CUST_LEANRIGHT,    "system_menus_options_input_customize_controls_lean_right");
		addControlRow(CONTROLS_CUST_CROUCH,       "system_menus_options_input_customize_controls_crouch");
		addControlRow(CONTROLS_CUST_CROUCHTOGGLE, "system_menus_options_input_customize_controls_crouch_toggle");
		
		addControlRow(CONTROLS_CUST_STRAFE,       "system_menus_options_input_customize_controls_strafe");
		addControlRow(CONTROLS_CUST_CENTERVIEW,   "system_menus_options_input_customize_controls_center_view");
		addControlRow(CONTROLS_CUST_FREELOOK,     "system_menus_options_input_customize_controls_freelook");
		
		addControlRow(CONTROLS_CUST_TURNLEFT,     "system_menus_options_input_customize_controls_turn_left");
		addControlRow(CONTROLS_CUST_TURNRIGHT,    "system_menus_options_input_customize_controls_turn_right");
		addControlRow(CONTROLS_CUST_LOOKUP,       "system_menus_options_input_customize_controls_look_up");
		addControlRow(CONTROLS_CUST_LOOKDOWN,     "system_menus_options_input_customize_controls_look_down");
		
		addControlRow(CONTROLS_CUST_MINIMAP,      "system_menus_options_input_customize_controls_minimap");
		
		addBackButton(Page_OptionsInput);
		
		{
			std::string label = getLocalised("system_menus_options_input_customize_default");
			TextWidget * txt = new TextWidget(hFontMenu, label);
			txt->clicked = boost::bind(&ControlOptionsMenuPage1::onClickedDefault, this, _1);
			addCorner(txt, BottomCenter);
		}
		
		{
			ButtonWidget * cb = new ButtonWidget(buttonSize(16, 16), "graph/interface/menus/next");
			cb->setTargetPage(Page_OptionsInputCustomizeKeys2);
			addCorner(cb, BottomRight);
		}
		
		reinitActionKeys();
		
	}
	
private:
	
	void onClickedDefault(Widget * /* widget */) {
		resetActionKeys();
	}
	
};

class ControlOptionsMenuPage2 arx_final : public ControlOptionsPage {
	
public:
	
	ControlOptionsMenuPage2()
		: ControlOptionsPage(Page_OptionsInputCustomizeKeys2)
	{ }
	
	~ControlOptionsMenuPage2() { }
	
	void init() {
		
		reserveBottom();
		
		addControlRow(CONTROLS_CUST_INVENTORY,         "system_menus_options_input_customize_controls_inventory");
		addControlRow(CONTROLS_CUST_BOOK,              "system_menus_options_input_customize_controls_book");
		addControlRow(CONTROLS_CUST_BOOKCHARSHEET,     "system_menus_options_input_customize_controls_bookcharsheet");
		addControlRow(CONTROLS_CUST_BOOKMAP,           "system_menus_options_input_customize_controls_bookmap");
		addControlRow(CONTROLS_CUST_BOOKSPELL,         "system_menus_options_input_customize_controls_bookspell");
		addControlRow(CONTROLS_CUST_BOOKQUEST,         "system_menus_options_input_customize_controls_bookquest");
		addControlRow(CONTROLS_CUST_DRINKPOTIONLIFE,   "system_menus_options_input_customize_controls_drink_potion_life");
		addControlRow(CONTROLS_CUST_DRINKPOTIONMANA,   "system_menus_options_input_customize_controls_drink_potion_mana");
		addControlRow(CONTROLS_CUST_DRINKPOTIONCURE,   "system_menus_options_input_customize_controls_drink_potion_cure");
		addControlRow(CONTROLS_CUST_TORCH,             "system_menus_options_input_customize_controls_torch");
		
		addControlRow(CONTROLS_CUST_CANCELCURSPELL,    "system_menus_options_input_customize_controls_cancelcurrentspell");
		addControlRow(CONTROLS_CUST_PRECAST1,          "system_menus_options_input_customize_controls_precast1");
		addControlRow(CONTROLS_CUST_PRECAST2,          "system_menus_options_input_customize_controls_precast2");
		addControlRow(CONTROLS_CUST_PRECAST3,          "system_menus_options_input_customize_controls_precast3");
		
		addControlRow(CONTROLS_CUST_WEAPON,            "system_menus_options_input_customize_controls_weapon");
		addControlRow(CONTROLS_CUST_UNEQUIPWEAPON,     "system_menus_options_input_customize_controls_unequipweapon");
		
		addControlRow(CONTROLS_CUST_PREVIOUS,          "system_menus_options_input_customize_controls_previous");
		addControlRow(CONTROLS_CUST_NEXT,              "system_menus_options_input_customize_controls_next");
		
		addControlRow(CONTROLS_CUST_QUICKLOAD,         "system_menus_options_input_customize_controls_quickload");
		addControlRow(CONTROLS_CUST_QUICKSAVE,         "system_menus_options_input_customize_controls_quicksave");
		
		addControlRow(CONTROLS_CUST_TOGGLE_FULLSCREEN, "system_menus_options_input_customize_controls_toggle_fullscreen");
		
		if(config.input.allowConsole) {
			addControlRow(CONTROLS_CUST_CONSOLE, "system_menus_options_input_customize_controls_console");
		}
		
		addBackButton(Page_OptionsInputCustomizeKeys1);
		
		{
			std::string label = getLocalised("system_menus_options_input_customize_default");
			TextWidget * txt = new TextWidget(hFontMenu, label);
			txt->clicked = boost::bind(&ControlOptionsMenuPage2::onClickedDefault, this, _1);
			addCorner(txt, BottomCenter);
		}
		
		reinitActionKeys();
		
	}
	
private:
	
	void onClickedDefault(Widget * /* widget */) {
		resetActionKeys();
	}
	
};

class QuitConfirmMenuPage arx_final : public MenuPage {
	
public:
	
	QuitConfirmMenuPage()
		: MenuPage(Page_QuitConfirm)
	{}
	
	~QuitConfirmMenuPage() { }
	
	void init() {
		
		reserveBottom();
		
		{
			TextWidget * txt = new TextWidget(hFontMenu, getLocalised("system_menus_main_quit"));
			txt->setEnabled(false);
			addCenter(txt);
		}
		
		{
			TextWidget * txt = new TextWidget(hFontMenu, getLocalised("system_menus_main_editquest_confirm"));
			txt->setEnabled(false);
			addCenter(txt);
		}
		
		{
			TextWidget * yes = new TextWidget(hFontMenu, getLocalised("system_yes"));
			yes->clicked = boost::bind(&QuitConfirmMenuPage::onClickedYes, this, _1);
			addCorner(yes, BottomRight);
		}
		
		{
			TextWidget * no = new TextWidget(hFontMenu, getLocalised("system_no"));
			no->setTargetPage(Page_None);
			no->setShortcut(Keyboard::Key_Escape);
			addCorner(no, BottomLeft);
		}
		
	}
	
private:
	
	void onClickedYes(Widget * /* widget */) {
		MenuFader_start(Fade_In, Mode_InGame);
	}
	
};

void MainMenu::initWindowPages() {
	
	delete m_window, m_window = NULL;
	
	m_window = new MenuWindow();
	
	m_window->add(new NewQuestMenuPage());
	
	m_window->add(new ChooseLoadOrSaveMenuPage());
	m_window->add(new LoadMenuPage());
	m_window->add(new SaveMenuPage());
	m_window->add(new SaveConfirmMenuPage());
	
	m_window->add(new OptionsMenuPage());
	m_window->add(new VideoOptionsMenuPage());
	m_window->add(new RenderOptionsMenuPage());
	m_window->add(new InterfaceOptionsMenuPage());
	m_window->add(new AudioOptionsMenuPage());
	m_window->add(new InputOptionsMenuPage());
	m_window->add(new ControlOptionsMenuPage1());
	m_window->add(new ControlOptionsMenuPage2());
	
	m_window->add(new QuitConfirmMenuPage());
	
}



MainMenu::MainMenu()
	: bReInitAll(false)
	, m_window(NULL)
	, m_requestedPage(Page_None)
	, m_background(NULL)
	, m_widgets(new WidgetContainer())
	, m_resumeGame(NULL)
	, m_selected(NULL)
{ }

MainMenu::~MainMenu() {
	delete m_window;
	delete m_widgets;
	delete m_background;
}

void MainMenu::init()
{
	m_background = TextureContainer::LoadUI("graph/interface/menus/menu_main_background", TextureContainer::NoColorKey);

	Vec2f pos = RATIO_2(Vec2f(370, 100));
	float yOffset = RATIO_Y(50);
	
	{
	TextWidget * txt = new TextWidget(hFontMainMenu, getLocalised("system_menus_main_resumegame"));
	txt->clicked = boost::bind(&MainMenu::onClickedResumeGame, this, _1);
	txt->setPosition(pos);
	m_widgets->add(txt);
	m_resumeGame = txt;
	}
	pos.y += yOffset;
	{
	TextWidget * txt = new TextWidget(hFontMainMenu, getLocalised("system_menus_main_newquest"));
	txt->clicked = boost::bind(&MainMenu::onClickedNewQuest, this, _1);
	txt->setPosition(pos);
	m_widgets->add(txt);
	}
	pos.y += yOffset;
	{
	TextWidget * txt = new TextWidget(hFontMainMenu, getLocalised("system_menus_main_editquest"));
	txt->setTargetPage(Page_LoadOrSave);
	txt->setPosition(pos);
	m_widgets->add(txt);
	}
	pos.y += yOffset;
	{
	TextWidget * txt = new TextWidget(hFontMainMenu, getLocalised("system_menus_main_options"));
	txt->setTargetPage(Page_Options);
	txt->setPosition(pos);
	m_widgets->add(txt);
	}
	pos.y += yOffset;
	{
	TextWidget * txt = new TextWidget(hFontMainMenu, getLocalised("system_menus_main_credits"));
	txt->clicked = boost::bind(&MainMenu::onClickedCredits, this, _1);
	txt->setPosition(pos);
	m_widgets->add(txt);
	}
	pos.y += yOffset;
	{
	TextWidget * txt = new TextWidget(hFontMainMenu, getLocalised("system_menus_main_quit"));
	txt->setTargetPage(Page_QuitConfirm);
	txt->setPosition(pos);
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
	TextWidget * txt = new TextWidget(hFontControls, version);
	txt->setEnabled(false);
	txt->forceDisplay(TextWidget::Disabled);
	txt->setPosition(RATIO_2(Vec2f(verPosX / g_sizeRatio.x, 80)));
	m_widgets->add(txt);
}

void MainMenu::onClickedResumeGame(Widget * /* widget */) {
	if(g_canResumeGame) {
		ARXMenu_ResumeGame();
	} else {
		ARX_QuickLoad();
	}
}

void MainMenu::onClickedNewQuest(Widget * /* widget */) {
	if(g_canResumeGame) {
		requestPage(Page_NewQuestConfirm);
		if(m_window) {
			m_window->setScroll(0.f);
		}
	} else {
		ARXMenu_NewQuest();
	}
}

void MainMenu::onClickedCredits(Widget * /* widget */) {
	MenuFader_start(Fade_In, Mode_Credits);
}

void MainMenu::update() {
	
	if(m_resumeGame) {
		m_resumeGame->setEnabled(g_canResumeGame || !savegames.empty());
	}
	
	m_selected = m_widgets->getWidgetAt(Vec2f(GInput->getMousePosition()));
	
	if(m_selected && GInput->getMouseButton(Mouse::Button_0)) {
		m_selected->click();
		if(m_selected->targetPage() != NOP && m_window) {
			m_window->setScroll(0.f);
		}
	}
	
	if(m_requestedPage != (m_window ? m_window->currentPageId() : Page_None)) {
		if(!m_window) {
			initWindowPages();
		}
		m_window->setCurrentPage(m_requestedPage);
	}
	
	m_widgets->update();
	
	if(m_selected) {
		pMenuCursor->SetMouseOver();
	}
	
	if(m_window) {
		m_window->update();
	}
	
}

void MainMenu::render() {
	
	if(m_background) {
		UseRenderState state(render2D().noBlend());
		EERIEDrawBitmap(Rectf(Vec2f(0, 0), g_size.width(), g_size.height()), 0.999f, m_background, Color::white);
	}
	
	m_widgets->render(m_selected);
	
	m_widgets->drawDebug();
	
	if(m_window) {
		m_window->render();
	}
	
}

MainMenu * g_mainMenu;
