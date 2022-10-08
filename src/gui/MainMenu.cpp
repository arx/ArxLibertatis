/*
 * Copyright 2014-2022 Arx Libertatis Team (see the AUTHORS file)
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

#include <algorithm>
#include <functional>
#include <iomanip>
#include <memory>
#include <numeric>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

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
#include "gui/book/Book.h"
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

#include "io/log/Logger.h"

#include "platform/Platform.h"

#include "scene/GameSound.h"

#include "util/Number.h"
#include "util/Unicode.h"

#include "window/RenderWindow.h"


class NewQuestMenuPage final : public MenuPage {
	
public:
	
	NewQuestMenuPage()
		: MenuPage(Page_NewQuestConfirm)
	{ }
	
	void init() override {
		
		reserveBottom();
		
		{
			auto txt = std::make_unique<TextWidget>(hFontMenu, getLocalised("system_menus_main_editquest_confirm"));
			txt->setEnabled(false);
			addCenter(std::move(txt));
		}
		
		{
			auto txt = std::make_unique<TextWidget>(hFontMenu, getLocalised("system_menus_main_newquest_confirm"));
			txt->setEnabled(false);
			addCenter(std::move(txt));
		}
		
		{
			auto txt = std::make_unique<TextWidget>(hFontMenu, getLocalised("system_yes"));
			txt->clicked = [](Widget * /* widget */) {
				ARXMenu_NewQuest();
			};
			addCorner(std::move(txt), BottomRight);
		}
		
		{
			auto txt = std::make_unique<TextWidget>(hFontMenu, getLocalised("system_no"));
			txt->setTargetPage(Page_None);
			txt->setShortcut(Keyboard::Key_Escape);
			addCorner(std::move(txt), BottomLeft);
		}
		
	}
	
};

class SaveConfirmMenuPage final : public MenuPage {
	
public:
	
	SaveConfirmMenuPage()
		: MenuPage(Page_SaveConfirm)
		, m_textbox(nullptr)
		, pDeleteButton(nullptr)
	{ }
	
	void init() override {
		
		reserveTop();
		reserveBottom();
		
		{
			auto cb = std::make_unique<ButtonWidget>(buttonSize(48, 48), "graph/interface/icons/menu_main_save");
			cb->setEnabled(false);
			addCenter(std::move(cb));
		}
		
		{
			std::string_view text = getLocalised("system_menu_editquest_newsavegame");
			auto txt = std::make_unique<TextInputWidget>(hFontMenu, text, m_rect);
			txt->setMaxLength(255); // Don't allow the user to enter names that cannot be stored in save files
			txt->unfocused = [this](TextInputWidget * /* widget */) {
				if(m_textbox->text().empty()) {
					setSaveHandle(m_savegame);
				}
			};
			m_textbox = txt.get();
			addCenter(std::move(txt));
		}
		
		// Delete button
		{
			auto txt = std::make_unique<TextWidget>(hFontMenu, getLocalised("system_menus_main_editquest_delete"));
			txt->clicked = [this](Widget * /* widget */) {
				g_mainMenu->bReInitAll = true;
				savegames.remove(m_savegame);
			};
			txt->setTargetPage(Page_Save);
			txt->setEnabled(m_savegame != SavegameHandle());
			addCorner(std::move(txt), TopRight);
		}
		
		// Save button
		{
			auto txt = std::make_unique<TextWidget>(hFontMenu, getLocalised("system_menus_main_editquest_save"));
			txt->clicked = [this](Widget * /* widget */) {
				m_textbox->unfocus();
				savegames.save(m_textbox->text(), m_savegame, savegame_thumbnail);
			};
			txt->setTargetPage(Page_None);
			addCorner(std::move(txt), BottomRight);
		}
		
		addBackButton(Page_Save);
		
		setSaveHandle(m_savegame);
		
	}
	
	void focus() override {
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
	
};

class LoadMenuPage final : public MenuPage {
	
public:
	
	LoadMenuPage()
		: MenuPage(Page_Load)
		, m_selected(nullptr)
		, pLoadConfirm(nullptr)
		, pDeleteConfirm(nullptr)
	{
		m_rowSpacing = 5;
	}
	
	void init() override {
		
		reserveTop();
		reserveBottom();
		
		{
			auto cb = std::make_unique<ButtonWidget>(buttonSize(48, 48), "graph/interface/icons/menu_main_load");
			cb->setEnabled(false);
			addCenter(std::move(cb));
		}
		
		// TODO make this list scrollable
		
		std::function<void(Widget * widget)> saveClicked = [this](Widget * widget) {
			resetSelection();
			
			arx_assert(widget->type() == WidgetType_SaveSlot);
			m_selected = static_cast<SaveSlotWidget *>(widget);
			m_selected->setSelected(true);
			
			pLoadConfirm->setEnabled(true);
			pDeleteConfirm->setEnabled(true);
		};
		
		std::function<void(Widget * widget)> saveDoubleClicked = [this](Widget * widget) {
			widget->click();
			pLoadConfirm->click();
		};
		
		// Show quicksaves.
		size_t quicksaveNum = 0;
		for(SavegameHandle save : savegames) {
			if(savegames[save].quicksave) {
				auto txt = std::make_unique<SaveSlotWidget>(save, ++quicksaveNum, hFontControls, m_rect);
				txt->clicked = saveClicked;
				txt->doubleClicked = saveDoubleClicked;
				addCenter(std::move(txt));
			}
		}
		
		// Show regular saves.
		for(SavegameHandle save : savegames) {
			if(!savegames[save].quicksave) {
				auto txt = std::make_unique<SaveSlotWidget>(save, 0, hFontControls, m_rect);
				txt->clicked = saveClicked;
				txt->doubleClicked = saveDoubleClicked;
				addCenter(std::move(txt));
			}
		}
		
		// Delete button
		{
			auto txt = std::make_unique<TextWidget>(hFontMenu, getLocalised("system_menus_main_editquest_delete"));
			txt->clicked = [this](Widget * /* widget */) {
				if(m_selected && m_selected->savegame() != SavegameHandle()) {
					g_mainMenu->bReInitAll = true;
					savegames.remove(m_selected->savegame());
					return;
				}
			};
			txt->setTargetPage(Page_Load);
			pDeleteConfirm = txt.get();
			addCorner(std::move(txt), TopRight);
		}
		
		// Load button
		{
			auto txt = std::make_unique<TextWidget>(hFontMenu, getLocalised("system_menus_main_editquest_load"));
			txt->clicked = [this](Widget * /* widget */) {
				if(m_selected && m_selected->savegame() != SavegameHandle()) {
					ARX_SOUND_PlayMenu(g_snd.MENU_CLICK);
					LOADQUEST_SLOT = m_selected->savegame();
					if(pTextManage) {
						pTextManage->Clear();
					}
				}
			};
			txt->setTargetPage(Page_None);
			pLoadConfirm = txt.get();
			addCorner(std::move(txt), BottomRight);
		}
		
		addBackButton(Page_LoadOrSave);
		
	}
	
	void resetSelection() {
		if(m_selected) {
			m_selected->setSelected(false);
			m_selected = nullptr;
		}
	}
	
	void focus() override {
		MenuPage::focus();
		resetSelection();
		pLoadConfirm->setEnabled(false);
		pDeleteConfirm->setEnabled(false);
	}
	
private:
	
	SaveSlotWidget * m_selected;
	
	TextWidget * pLoadConfirm;
	TextWidget * pDeleteConfirm;
	
};

class SaveMenuPage final : public MenuPage {
	
public:
	
	SaveMenuPage()
		: MenuPage(Page_Save)
	{
		m_rowSpacing = 5;
	}
	
	void init() override {
		
		reserveBottom();
		
		{
			auto cb = std::make_unique<ButtonWidget>(buttonSize(48, 48), "graph/interface/icons/menu_main_save");
			cb->setEnabled(false);
			addCenter(std::move(cb));
		}
		
		std::function<void(Widget * widget)> saveClicked = [](Widget * widget) {
			arx_assert(widget->type() == WidgetType_SaveSlot);
			SavegameHandle savegame = static_cast<SaveSlotWidget *>(widget)->savegame();
			MenuPage * page = g_mainMenu->m_window->getPage(Page_SaveConfirm);
			arx_assert(page->id() == Page_SaveConfirm);
			static_cast<SaveConfirmMenuPage *>(page)->setSaveHandle(savegame);
		};
		
		// Show quicksaves.
		size_t quicksaveNum = 0;
		for(SavegameHandle save : savegames) {
			if(savegames[save].quicksave) {
				auto txt = std::make_unique<SaveSlotWidget>(save, ++quicksaveNum, hFontControls, m_rect);
				txt->clicked = saveClicked;
				txt->setTargetPage(Page_SaveConfirm);
				txt->setEnabled(false);
				addCenter(std::move(txt));
			}
		}
		
		// Show regular saves.
		for(SavegameHandle save : savegames) {
			if(!savegames[save].quicksave) {
				auto txt = std::make_unique<SaveSlotWidget>(save, 0, hFontControls, m_rect);
				txt->clicked = saveClicked;
				txt->setTargetPage(Page_SaveConfirm);
				addCenter(std::move(txt));
			}
		}
		
		for(size_t i = savegames.size(); i <= 15; i++) {
			auto txt = std::make_unique<SaveSlotWidget>(SavegameHandle(), i, hFontControls, m_rect);
			txt->clicked = saveClicked;
			txt->setTargetPage(Page_SaveConfirm);
			addCenter(std::move(txt));
		}
		
		addBackButton(Page_LoadOrSave);
		
	}
	
};

class ChooseLoadOrSaveMenuPage final : public MenuPage {
	
	TextWidget * m_loadButton;
	TextWidget * m_saveButton;
	
public:
	
	ChooseLoadOrSaveMenuPage()
		: MenuPage(Page_LoadOrSave)
		, m_loadButton(nullptr)
		, m_saveButton(nullptr)
	{ }
	
	void init() override {
		
		reserveTop();
		reserveBottom();
		
		{
			auto load = std::make_unique<TextWidget>(hFontMenu, getLocalised("system_menus_main_editquest_load"));
			load->setTargetPage(Page_Load);
			m_loadButton = load.get();
			addCenter(std::move(load));
		}
		
		{
			auto save = std::make_unique<TextWidget>(hFontMenu, getLocalised("system_menus_main_editquest_save"));
			save->setTargetPage(Page_Save);
			m_saveButton = save.get();
			addCenter(std::move(save));
		}
		
		addBackButton(Page_None);
		
	}
	
	void focus() override {
		MenuPage::focus();
		m_loadButton->setEnabled(!savegames.empty());
		m_saveButton->setEnabled(g_canResumeGame);
	}
	
};

class OptionsMenuPage final : public MenuPage {
	
public:
	
	OptionsMenuPage()
		: MenuPage(Page_Options)
	{ }
	
	void init() override {
		
		reserveTop();
		reserveBottom();
		
		{
			auto txt = std::make_unique<TextWidget>(hFontMenu, getLocalised("system_menus_options_localization"));
			txt->setTargetPage(Page_Localization);
			addCenter(std::move(txt));
		}
		
		{
			auto txt = std::make_unique<TextWidget>(hFontMenu, getLocalised("system_menus_options_video"));
			txt->setTargetPage(Page_OptionsVideo);
			addCenter(std::move(txt));
		}
		
		{
			auto txt = std::make_unique<TextWidget>(hFontMenu, getLocalised("system_menus_options_render"));
			txt->setTargetPage(Page_OptionsRender);
			addCenter(std::move(txt));
		}
		
		{
			auto txt = std::make_unique<TextWidget>(hFontMenu, getLocalised("system_menus_options_interface"));
			txt->setTargetPage(Page_OptionsInterface);
			addCenter(std::move(txt));
		}
		
		{
			auto txt = std::make_unique<TextWidget>(hFontMenu, getLocalised("system_menus_options_audio"));
			txt->setTargetPage(Page_OptionsAudio);
			addCenter(std::move(txt));
		}
		
		{
			auto txt = std::make_unique<TextWidget>(hFontMenu, getLocalised("system_menus_options_input"));
			txt->setTargetPage(Page_OptionsInput);
			addCenter(std::move(txt));
		}
		
		addBackButton(Page_None);
		
	}
	
};

class LocalizationMenuPage final : public MenuPage {
	
	std::vector<std::string> m_textLanguages;
	std::vector<std::string> m_audioLanguages;
	
	CycleTextWidget * m_textLanguageSlider;
	CycleTextWidget * m_audioLanguageSlider;
	
public:
	
	LocalizationMenuPage()
		: MenuPage(Page_Localization)
	{ }
	
	void init() override {
		
		reserveBottom();
		
		addCenter(std::make_unique<Spacer>(hFontMainMenu->getLineHeight()));
		
		{
			std::unique_ptr<TextWidget> txt;
			if(g_iconFont) {
				txt = std::make_unique<TextWidget>(g_iconFont, getLocalised("system_localization_text"));
			} else {
				txt = std::make_unique<TextWidget>(hFontMenu, "Text");
			}
			txt->setEnabled(false);
			addCenter(std::move(txt));
		}
		
		{
			m_textLanguages.clear();
			auto slider = std::make_unique<CycleTextWidget>(sliderSize(), hFontMainMenu, "");
			slider->valueChanged = [this](int pos, std::string_view /* string */) {
				if(size_t(pos) >= m_textLanguages.size() || config.interface.language == m_textLanguages[size_t(pos)]) {
					return;
				}
				config.interface.language = m_textLanguages[size_t(pos)];
				g_playerBook.questBook.clear();
				initLocalisation();
				ARX_Text_Init(true);
				g_mainMenu->bReInitAll = true;
				config.save();
			};
			Languages languages = getAvailableTextLanguages();
			for(const Languages::value_type & language : languages) {
				slider->addEntry(language.second.name);
				if(m_textLanguages.empty() || config.interface.language == language.first) {
					slider->selectLast();
				}
				m_textLanguages.push_back(language.first);
			}
			slider->setEnabled(m_textLanguages.size() > 1);
			m_textLanguageSlider = slider.get();
			addCenter(std::move(slider));
		}
		
		addCenter(std::make_unique<Spacer>(hFontMainMenu->getLineHeight()));
		
		{
			std::unique_ptr<TextWidget> txt;
			if(g_iconFont) {
				txt = std::make_unique<TextWidget>(g_iconFont, getLocalised("system_localization_audio"));
			} else {
				txt = std::make_unique<TextWidget>(hFontMenu, "Audio");
			}
			txt->setEnabled(false);
			addCenter(std::move(txt));
		}
		
		{
			m_audioLanguages.clear();
			auto slider = std::make_unique<CycleTextWidget>(sliderSize(), hFontMainMenu, "");
			slider->valueChanged = [this](int pos, std::string_view /* string */) {
				if(size_t(pos) >= m_audioLanguages.size() || config.audio.language == m_audioLanguages[size_t(pos)]) {
					return;
				}
				config.audio.language = m_audioLanguages[size_t(pos)];
				config.save();
			};
			Languages languages = getAvailableAudioLanguages();
			for(const Languages::value_type & language : languages) {
				slider->addEntry(language.second.name);
				if(m_audioLanguages.empty() || config.audio.language == language.first) {
					slider->selectLast();
				}
				m_audioLanguages.push_back(language.first);
			}
			slider->setEnabled(m_audioLanguages.size() > 1);
			m_audioLanguageSlider = slider.get();
			addCenter(std::move(slider));
		}
		
		addCenter(std::make_unique<Spacer>(hFontMainMenu->getLineHeight()));
		
		addBackButton(Page_Options);
		
	}
	
};

class VideoOptionsMenuPage final : public MenuPage {
	
	CheckboxWidget * m_fullscreenCheckbox;
	CycleTextWidget * m_resolutionSlider;
	SliderWidget * m_gammaSlider;
	CheckboxWidget * m_minimizeOnFocusLostCheckbox;
	TextWidget * m_applyButton;
	bool m_fullscreen;
	DisplayMode m_mode;
	
public:
	
	VideoOptionsMenuPage()
		: MenuPage(Page_OptionsVideo)
		, m_fullscreenCheckbox(nullptr)
		, m_resolutionSlider(nullptr)
		, m_gammaSlider(nullptr)
		, m_minimizeOnFocusLostCheckbox(nullptr)
		, m_applyButton(nullptr)
		, m_fullscreen(false)
	{ }
	
	void init() override {
		
		reserveBottom();
		
		m_fullscreen = config.video.fullscreen;
		m_mode = config.video.mode;
		
		{
			std::string_view label = getLocalised("system_menus_options_videos_full_screen");
			auto cb = std::make_unique<CheckboxWidget>(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.video.fullscreen);
			cb->stateChanged = [this](bool checked) {
				m_fullscreen = checked;
				m_resolutionSlider->setEnabled(m_fullscreen);
				updateGammaSlider();
				updateMinimizeOnFocusLostStateCheckbox();
				updateApplyButton();
			};
			m_fullscreenCheckbox = cb.get();
			addCenter(std::move(cb));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_video_resolution");
			auto slider = std::make_unique<CycleTextWidget>(sliderSize(), hFontMenu, label, hFontControls);
			slider->valueChanged = [this](int pos, std::string_view /* string */) {
				const RenderWindow::DisplayModes & modes = mainApp->getWindow()->getDisplayModes();
				if(size_t(pos) < modes.size()) {
					m_mode = modes[size_t(pos)];
				} else {
					m_mode = DisplayMode();
				}
				updateApplyButton();
			};
			slider->setEnabled(config.video.fullscreen);
			
			for(const DisplayMode & mode : mainApp->getWindow()->getDisplayModes()) {
				
				// Find the aspect ratio
				Vec2i aspect = mode.resolution / Vec2i(std::gcd(mode.resolution.x, mode.resolution.y));
				
				std::stringstream ss;
				ss << mode;
				if(aspect.x < 50 && aspect.y < 50) {
					if(aspect.y == 5) {
						// 8:5 is conventionally called 16:10
						aspect *= 2;
					}
					ss << " (" << aspect.x << ':' << aspect.y << ')';
				}
				
				slider->addEntry(ss.str());
				if(mode == config.video.mode) {
					slider->selectLast();
				}
				
			}
			
			slider->addEntry(getLocalised("system_menus_options_video_resolution_desktop"));
			if(config.video.mode.resolution == Vec2i(0)) {
				slider->selectLast();
			}
			
			m_resolutionSlider = slider.get();
			addCenter(std::move(slider));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_video_gamma");
			auto sld = std::make_unique<SliderWidget>(sliderSize(), hFontMenu, label);
			sld->valueChanged = [this](int value) {
				if(m_gammaSlider->isEnabled()) {
					ARXMenu_Options_Video_SetGamma(float(value));
				}
			};
			m_gammaSlider = sld.get();
			addCenter(std::move(sld));
			updateGammaSlider();
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_videos_minimize_on_focus_lost");
			auto cb = std::make_unique<CheckboxWidget>(checkboxSize(), hFontMenu, label);
			cb->stateChanged = [this](bool checked) {
				if(m_minimizeOnFocusLostCheckbox->isEnabled()) {
					config.window.minimizeOnFocusLost = checked;
					mainApp->getWindow()->setMinimizeOnFocusLost(config.window.minimizeOnFocusLost);
				}
			};
			m_minimizeOnFocusLostCheckbox = cb.get();
			addCenter(std::move(cb));
			updateMinimizeOnFocusLostStateCheckbox();
		}
		
		addCenter(std::make_unique<Spacer>(hFontMenu->getLineHeight() / 2));
		
		{
			std::string_view label = getLocalised("system_menus_options_video_vsync");
			auto cb = std::make_unique<CycleTextWidget>(sliderSize(), hFontMenu, label);
			cb->valueChanged = [](int pos, std::string_view /* string */) {
				config.video.vsync = pos > 1 ? -1 : pos;
				mainApp->getWindow()->setVSync(config.video.vsync);
			};
			cb->addEntry(getLocalised("system_menus_options_video_vsync_off"));
			if(benchmark::isEnabled()) {
				cb->setValue(0);
				cb->setEnabled(false);
			} else {
				cb->addEntry(getLocalised("system_menus_options_video_vsync_on"));
				cb->addEntry(getLocalised("system_menus_options_video_vsync_auto"));
				cb->setValue(config.video.vsync < 0 ? 2 : config.video.vsync);
			}
			addCenter(std::move(cb));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_video_fps_limit");
			auto cb = std::make_unique<CycleTextWidget>(sliderSize(), hFontMenu, label);
			cb->valueChanged = [](int pos, std::string_view string) {
				if(pos == 0) {
					config.video.fpsLimit = 0;
				} else {
					config.video.fpsLimit = util::toInt(string).value_or(-1);
				}
			};
			cb->addEntry(getLocalised("system_menus_options_video_fps_limit_off"));
			cb->setValue(0);
			if(benchmark::isEnabled()) {
				cb->setEnabled(false);
			} else {
				std::vector<s32> rates;
				rates.push_back(30);
				rates.push_back(60);
				rates.push_back(72);
				rates.push_back(75);
				rates.push_back(100);
				rates.push_back(120);
				rates.push_back(144);
				rates.push_back(150);
				rates.push_back(200);
				for(const DisplayMode & mode : mainApp->getWindow()->getDisplayModes()) {
					if(mode.refresh) {
						rates.push_back(mode.refresh);
						if(mode.refresh >= 60) {
							rates.push_back(2 * mode.refresh);
						}
					}
				}
				if(config.video.fpsLimit > 0) {
					rates.push_back(config.video.fpsLimit);
				}
				std::sort(rates.begin(), rates.end());
				rates.erase(std::unique(rates.begin(), rates.end()), rates.end());
				for(s32 refresh : rates) {
					cb->addEntry(std::to_string(refresh));
					if(config.video.fpsLimit == refresh) {
						cb->selectLast();
					}
				}
				cb->addEntry(getLocalised("system_menus_options_video_fps_limit_display"));
				if(config.video.fpsLimit < 0) {
					cb->selectLast();
				}
			}
			addCenter(std::move(cb));
		}
		
		addCenter(std::make_unique<Spacer>(hFontMenu->getLineHeight() / 2));
		
		{
			std::string_view label = getLocalised("system_menus_options_video_fov");
			auto sld = std::make_unique<SliderWidget>(sliderSize(), hFontMenu, label);
			sld->valueChanged = [](int value) noexcept {
				config.video.fov = 50.f + float(value) * 5.f;
			};
			sld->setValue(glm::clamp(int((config.video.fov - 50.f) / 5.f), 0, 10));
			addCenter(std::move(sld));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_videos_view_bobbing");
			auto cb = std::make_unique<CheckboxWidget>(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.video.viewBobbing);
			cb->stateChanged = [](bool checked) noexcept {
				config.video.viewBobbing = checked;
			};
			m_fullscreenCheckbox = cb.get();
			addCenter(std::move(cb));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_videos_screen_shake");
			auto cb = std::make_unique<CheckboxWidget>(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.video.screenShake);
			cb->stateChanged = [](bool checked) noexcept {
				config.video.screenShake = checked;
			};
			m_fullscreenCheckbox = cb.get();
			addCenter(std::move(cb));
		}
		
		{
			auto txt = std::make_unique<TextWidget>(hFontMenu, getLocalised("system_menus_video_apply"));
			txt->clicked = [this](Widget * /* widget */) {
				
				config.video.mode = m_mode;
				
				if(!m_fullscreen) {
					if(config.video.mode.resolution == Vec2i(0)) {
						LogInfo << "Configuring automatic fullscreen resolution selection";
					} else {
						LogInfo << "Configuring fullscreen resolution to " << config.video.mode;
					}
				}
				
				RenderWindow * window = mainApp->getWindow();
				
				if(window->isFullScreen() != m_fullscreen || m_fullscreen) {
					GRenderer->Clear(Renderer::ColorBuffer);
					mainApp->getWindow()->showFrame();
					mainApp->setWindowSize(m_fullscreen);
				}
				
				g_mainMenu->bReInitAll = true;
				
			};
			txt->setEnabled(false);
			m_applyButton = txt.get();
			addCorner(std::move(txt), BottomRight);
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
		
		bool enable = (m_mode != config.video.mode || m_fullscreen != config.video.fullscreen);
		m_applyButton->setEnabled(enable);
		
	}
	
};

class RenderOptionsMenuPage final : public MenuPage {
	
public:
	
	RenderOptionsMenuPage()
		: MenuPage(Page_OptionsRender)
		, m_alphaCutoutAntialiasingCycleText(nullptr)
	{ }
	
	CycleTextWidget * m_alphaCutoutAntialiasingCycleText;
	
	void init() override {
		
		reserveBottom();
		
		// Renderer selection
		{
			std::string_view label = getLocalised("system_menus_options_video_renderer");
			auto slider = std::make_unique<CycleTextWidget>(sliderSize(), hFontMenu, label);
			slider->valueChanged = [](int pos, std::string_view /* string */) {
				switch(pos) {
					case 0:  config.video.renderer = "auto"; break;
					case 1:  config.video.renderer = "OpenGL";  break;
					default: arx_unreachable();
				}
			};
			slider->addEntry("Auto-Select");
			slider->selectLast();
			slider->addEntry("OpenGL");
			if(config.video.renderer == "OpenGL") {
				slider->selectLast();
			}
			addCenter(std::move(slider));
		}
		
		addCenter(std::make_unique<Spacer>(hFontMenu->getLineHeight() / 2));
		
		{
			std::string_view label = getLocalised("system_menus_options_detail");
			auto cb = std::make_unique<CycleTextWidget>(sliderSize(), hFontMenu, label);
			cb->valueChanged = [](int pos, std::string_view /* string */) {
				ARXMenu_Options_Video_SetDetailsQuality(pos);
			};
			cb->addEntry(getLocalised("system_menus_options_video_texture_low"));
			cb->addEntry(getLocalised("system_menus_options_video_texture_med"));
			cb->addEntry(getLocalised("system_menus_options_video_texture_high"));
			cb->setValue(config.video.levelOfDetail);
			addCenter(std::move(cb));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_video_brouillard");
			auto sld = std::make_unique<SliderWidget>(sliderSize(), hFontMenu, label);
			sld->valueChanged = [](int value) {
				ARXMenu_Options_Video_SetFogDistance(value);
			};
			sld->setValue(int(config.video.fogDistance));
			addCenter(std::move(sld));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_video_antialiasing");
			auto cb = std::make_unique<CheckboxWidget>(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.video.antialiasing);
			cb->stateChanged = [this](bool checked) {
				config.video.antialiasing = checked;
				setAlphaCutoutAntialisingState();
			};
			addCenter(std::move(cb));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_video_colorkey_antialiasing");
			auto cb = std::make_unique<CheckboxWidget>(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.video.colorkeyAntialiasing);
			cb->stateChanged = [](bool checked) {
				config.video.colorkeyAntialiasing = checked;
				GRenderer->reloadColorKeyTextures();
			};
			addCenter(std::move(cb));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_video_alpha_cutout_antialiasing");
			auto cb = std::make_unique<CycleTextWidget>(sliderSize(), hFontMenu, label);
			cb->valueChanged = [](int pos, std::string_view /* string */) noexcept {
				config.video.alphaCutoutAntialiasing = pos;
			};
			cb->addEntry(getLocalised("system_menus_options_video_alpha_cutout_antialiasing_off"));
			Renderer::AlphaCutoutAntialising maxAA = GRenderer->getMaxSupportedAlphaCutoutAntialiasing();
			if(maxAA >= Renderer::FuzzyAlphaCutoutAA) {
				cb->addEntry(getLocalised("system_menus_options_video_alpha_cutout_antialiasing_fuzzy"));
			}
			if(maxAA >= Renderer::CrispAlphaCutoutAA) {
				cb->addEntry(getLocalised("system_menus_options_video_alpha_cutout_antialiasing_crisp"));
			}
			m_alphaCutoutAntialiasingCycleText = cb.get();
			setAlphaCutoutAntialisingState();
			addCenter(std::move(cb));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_video_texture_filter_anisotropic");
			auto cb = std::make_unique<CycleTextWidget>(sliderSize(), hFontMenu, label);
			cb->valueChanged = [](int pos, std::string_view string) {
				int anisotropy = 1;
				if(pos > 0) {
					if(!string.empty() && string[0] == 'x') {
						anisotropy = util::toInt(string.substr(1)).value_or(1);
					} else {
						anisotropy = 9001;
					}
				}
				config.video.maxAnisotropicFiltering = anisotropy;
				GRenderer->setMaxAnisotropy(anisotropy);
			};
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
			addCenter(std::move(cb));
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
	
};

class InterfaceOptionsMenuPage final : public MenuPage {
	
public:
	
	InterfaceOptionsMenuPage()
		: MenuPage(Page_OptionsInterface)
	{ }
	
	void init() override {
		
		reserveBottom();
		
		{
			std::string_view label = getLocalised("system_menus_options_video_crosshair");
			label = getLocalised("system_menus_options_interface_crosshair", label);
			auto cb = std::make_unique<CheckboxWidget>(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.interface.showCrosshair);
			cb->stateChanged = [](bool checked) noexcept {
				config.interface.showCrosshair = checked;
			};
			addCenter(std::move(cb));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_interface_limit_speech_width");
			auto cb = std::make_unique<CheckboxWidget>(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.interface.limitSpeechWidth);
			cb->stateChanged = [](bool checked) noexcept {
				config.interface.limitSpeechWidth = checked;
			};
			addCenter(std::move(cb));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_interface_cinematic_widescreen_mode");
			auto cb = std::make_unique<CycleTextWidget>(sliderSize(), hFontMenu, label, hFontControls);
			cb->valueChanged = [](int pos, std::string_view /* string */) noexcept {
				config.interface.cinematicWidescreenMode = CinematicWidescreenMode(pos);
			};
			cb->addEntry(getLocalised("system_menus_options_interface_letterbox"));
			cb->addEntry(getLocalised("system_menus_options_interface_hard_edges"));
			cb->addEntry(getLocalised("system_menus_options_interface_fade_edges"));
			cb->setValue(config.interface.cinematicWidescreenMode);
			addCenter(std::move(cb));
		}
		
		addCenter(std::make_unique<Spacer>(hFontMenu->getLineHeight() / 2));
		
		{
			std::string_view label = getLocalised("system_menus_options_interface_hud_scale");
			auto sld = std::make_unique<SliderWidget>(sliderSize(), hFontMenu, label);
			sld->valueChanged = [](int value) {
				config.interface.hudScale = float(value) * 0.1f;
				g_hudRoot.recalcScale();
			};
			sld->setValue(int(config.interface.hudScale * 10.f));
			addCenter(std::move(sld));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_interface_hud_scale_integer");
			auto cb = std::make_unique<CheckboxWidget>(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.interface.hudScaleInteger);
			cb->stateChanged = [](bool checked ) {
				config.interface.hudScaleInteger = checked;
				g_hudRoot.recalcScale();
			};
			addCenter(std::move(cb));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_interface_book_scale");
			auto sld = std::make_unique<SliderWidget>(sliderSize(), hFontMenu, label);
			sld->valueChanged = [](int value) noexcept {
				config.interface.bookScale = float(value) * 0.1f;
			};
			sld->setValue(int(config.interface.bookScale * 10.f));
			addCenter(std::move(sld));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_interface_book_scale_integer");
			auto cb = std::make_unique<CheckboxWidget>(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.interface.bookScaleInteger);
			cb->stateChanged = [](bool checked) noexcept {
				config.interface.bookScaleInteger = checked;
			};
			addCenter(std::move(cb));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_interface_cursor_scale");
			auto sld = std::make_unique<SliderWidget>(sliderSize(), hFontMenu, label);
			sld->valueChanged = [](int value) noexcept {
				config.interface.cursorScale = float(value) * 0.1f;
			};
			sld->setValue(int(config.interface.cursorScale * 10.f));
			addCenter(std::move(sld));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_interface_cursor_scale_integer");
			auto cb = std::make_unique<CheckboxWidget>(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.interface.cursorScaleInteger);
			cb->stateChanged = [](bool checked) noexcept {
				config.interface.cursorScaleInteger = checked;
			};
			addCenter(std::move(cb));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_interface_scale_filter");
			auto cb = std::make_unique<CycleTextWidget>(sliderSize(), hFontMenu, label);
			cb->valueChanged = [](int pos, std::string_view /* string */) noexcept {
				config.interface.scaleFilter = UIScaleFilter(pos);
			};
			cb->addEntry(getLocalised("system_menus_options_video_filter_nearest"));
			cb->addEntry(getLocalised("system_menus_options_video_filter_bilinear"));
			cb->setValue(config.interface.scaleFilter);
			addCenter(std::move(cb));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_interface_font_size");
			auto sld = std::make_unique<SliderWidget>(sliderSize(), hFontMenu, label);
			sld->valueChanged = [](int value) {
				config.interface.fontSize = 0.75f + float(value) / 20.f;
				ARX_Text_Init();
			};
			sld->setValue(int(glm::clamp((config.interface.fontSize - 0.75f) * 20.f + 0.5f, 0.f, 10.f)));
			addCenter(std::move(sld));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_interface_font_weight");
			auto cb = std::make_unique<CycleTextWidget>(sliderSize(), hFontMenu, label);
			cb->valueChanged = [](int pos, std::string_view /* string */) {
				config.interface.fontWeight = pos;
				ARX_Text_Init();
			};
			cb->addEntry(getLocalised("system_menus_options_interface_font_weight_0"));
			cb->addEntry(getLocalised("system_menus_options_interface_font_weight_1"));
			cb->addEntry(getLocalised("system_menus_options_interface_font_weight_2"));
			cb->addEntry(getLocalised("system_menus_options_interface_font_weight_3"));
			cb->addEntry(getLocalised("system_menus_options_interface_font_weight_4"));
			cb->addEntry(getLocalised("system_menus_options_interface_font_weight_5"));
			cb->setValue(config.interface.fontWeight);
			addCenter(std::move(cb));
		}
		
		addBackButton(Page_Options);
		
	}
	
};


class AudioOptionsMenuPage final : public MenuPage {
	
public:
	
	AudioOptionsMenuPage()
		: MenuPage(Page_OptionsAudio)
	{ }
	
	void init() override {
		
		reserveBottom();
		
		// Audio backend selection
		{
			std::string_view label = getLocalised("system_menus_options_audio_device");
			auto slider = std::make_unique<CycleTextWidget>(sliderSize(), hFontMenu, label, hFontControls);
			slider->valueChanged = [](int pos, std::string_view string) {
				ARXMenu_Options_Audio_SetDevice((pos == 0) ? "auto" : string);
			};
			slider->addEntry("Default");
			slider->selectLast();
			for(std::string_view device : audio::getDevices()) {
				slider->addEntry(device);
				if(config.audio.device == device) {
					slider->selectLast();
				}
			}
			addCenter(std::move(slider));
		}
		
		addCenter(std::make_unique<Spacer>(hFontMenu->getLineHeight() / 2));
		
		{
			std::string_view label = getLocalised("system_menus_options_audio_master_volume");
			auto sld = std::make_unique<SliderWidget>(sliderSize(), hFontMenu, label);
			sld->valueChanged = [](int value) {
				ARXMenu_Options_Audio_SetMasterVolume(value);
			};
			sld->setValue(int(config.audio.volume)); // TODO use float sliders
			addCenter(std::move(sld));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_audio_effects_volume");
			auto sld = std::make_unique<SliderWidget>(sliderSize(), hFontMenu, label);
			sld->valueChanged = [](int value) {
				ARXMenu_Options_Audio_SetSfxVolume(value);
			};
			sld->setValue(int(config.audio.sfxVolume));
			addCenter(std::move(sld));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_audio_speech_volume");
			auto sld = std::make_unique<SliderWidget>(sliderSize(), hFontMenu, label);
			sld->valueChanged = [](int value) {
				ARXMenu_Options_Audio_SetSpeechVolume(value);
			};
			sld->setValue(int(config.audio.speechVolume));
			addCenter(std::move(sld));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_audio_ambiance_volume");
			auto sld = std::make_unique<SliderWidget>(sliderSize(), hFontMenu, label);
			sld->valueChanged = [](int value) {
				ARXMenu_Options_Audio_SetAmbianceVolume(value);
			};
			sld->setValue(int(config.audio.ambianceVolume));
			addCenter(std::move(sld));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_audio_mute_on_focus_lost");
			auto cb = std::make_unique<CheckboxWidget>(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.audio.muteOnFocusLost);
			cb->stateChanged = [](bool checked) {
				config.audio.muteOnFocusLost = checked;
				if(!mainApp->getWindow()->hasFocus()) {
					ARXMenu_Options_Audio_SetMuted(config.audio.muteOnFocusLost);
				}
			};
			addCenter(std::move(cb));
		}
		
		addCenter(std::make_unique<Spacer>(hFontMenu->getLineHeight() / 2));
		
		{
			std::string_view label = getLocalised("system_menus_options_audio_eax");
			auto cb = std::make_unique<CheckboxWidget>(checkboxSize(), hFontMenu, label);
			if(audio::isReverbSupported()) {
				cb->setChecked(config.audio.eax);
			} else {
				cb->setEnabled(false);
			}
			cb->stateChanged = [](bool checked) {
				config.audio.eax = checked;
				ARX_SOUND_SetReverb(config.audio.eax);
			};
			addCenter(std::move(cb));
		}
		
		audio::HRTFStatus hrtf = audio::getHRTFStatus();
		if(hrtf != audio::HRTFUnavailable) {
			std::string_view label = getLocalised("system_menus_options_audio_hrtf");
			auto slider = std::make_unique<CycleTextWidget>(sliderSize(), hFontMenu, label);
			slider->valueChanged = [](int pos, std::string_view /* string */) {
				switch(pos) {
					case 0: config.audio.hrtf = audio::HRTFDisable; break;
					case 1: config.audio.hrtf = audio::HRTFDefault; break;
					case 2: config.audio.hrtf = audio::HRTFEnable; break;
					default: arx_unreachable();
				}
				audio::setHRTFEnabled(config.audio.hrtf);
			};
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
			addCenter(std::move(slider));
		}
		
		addBackButton(Page_Options);
		
	}
	
};

class InputOptionsMenuPage final : public MenuPage {
	
public:
	
	InputOptionsMenuPage()
		: MenuPage(Page_OptionsInput)
	{ }
	
	void init() override {
		
		reserveBottom();
		
		{
			std::string label(getLocalised("system_menus_options_input_customize_controls"));
			label += "…";
			auto txt = std::make_unique<TextWidget>(hFontMenu, label);
			txt->setTargetPage(Page_OptionsInputCustomizeKeys1);
			addCenter(std::move(txt), false);
		}
		
		addCenter(std::make_unique<Spacer>(hFontMenu->getLineHeight() / 2));
		
		{
			std::string_view label = getLocalised("system_menus_options_input_invert_mouse");
			auto cb = std::make_unique<CheckboxWidget>(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.input.invertMouse);
			cb->stateChanged = [](bool checked) {
				config.input.invertMouse = checked;
				GInput->setInvertMouseY(config.input.invertMouse);
			};
			addCenter(std::move(cb));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_auto_ready_weapon");
			auto cb = std::make_unique<CycleTextWidget>(sliderSize(), hFontMenu, label);
			cb->valueChanged = [](int pos, std::string_view /* string */) noexcept {
				config.input.autoReadyWeapon = AutoReadyWeapon(pos);
			};
			cb->addEntry(getLocalised("system_menus_options_auto_ready_weapon_off"));
			cb->addEntry(getLocalised("system_menus_options_auto_ready_weapon_enemies"));
			cb->addEntry(getLocalised("system_menus_options_auto_ready_weapon_always"));
			cb->setValue(int(config.input.autoReadyWeapon));
			addCenter(std::move(cb));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_input_mouse_look_toggle");
			auto cb = std::make_unique<CheckboxWidget>(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.input.mouseLookToggle);
			cb->stateChanged = [](bool checked) noexcept {
				config.input.mouseLookToggle = checked;
			};
			addCenter(std::move(cb));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_input_mouse_sensitivity");
			auto sld = std::make_unique<SliderWidget>(sliderSize(), hFontMenu, label);
			sld->valueChanged = [](int value) {
				config.input.mouseSensitivity = glm::clamp(value, 0, 10);
				GInput->setMouseSensitivity(config.input.mouseSensitivity);
			};
			sld->setValue(config.input.mouseSensitivity);
			addCenter(std::move(sld));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_input_mouse_acceleration");
			auto sld = std::make_unique<SliderWidget>(sliderSize(), hFontMenu, label);
			sld->valueChanged = [](int value) {
				config.input.mouseAcceleration = glm::clamp(value, 0, 10);
				GInput->setMouseAcceleration(config.input.mouseAcceleration);
			};
			sld->setValue(config.input.mouseAcceleration);
			addCenter(std::move(sld));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_raw_mouse_input");
			auto cb = std::make_unique<CheckboxWidget>(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.input.rawMouseInput);
			cb->stateChanged = [](bool checked) {
				config.input.rawMouseInput = checked;
				GInput->setRawMouseInput(config.input.rawMouseInput);
			};
			addCenter(std::move(cb));
		}
		
		{
			std::string_view label = getLocalised("system_menus_autodescription");
			auto cb = std::make_unique<CheckboxWidget>(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.input.autoDescription);
			cb->stateChanged = [](bool checked) noexcept {
				config.input.autoDescription = checked;
			};
			addCenter(std::move(cb));
		}
		
		{
			std::string_view label = getLocalised("system_menus_options_misc_quicksave_slots");
			auto sld = std::make_unique<SliderWidget>(sliderSize(), hFontMenu, label);
			sld->setMinimum(1);
			sld->valueChanged = [](int value) noexcept {
				config.misc.quicksaveSlots = value;
			};
			sld->setValue(config.misc.quicksaveSlots);
			addCenter(std::move(sld));
		}
		
		{
			std::string_view label = getLocalised("system_menus_border_turning");
			auto cb = std::make_unique<CheckboxWidget>(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.input.borderTurning);
			cb->stateChanged = [](bool checked) noexcept {
				config.input.borderTurning = checked;
			};
			addCenter(std::move(cb));
		}
		
		{
			std::string_view label = getLocalised("system_menus_alt_rune_recognition");
			auto cb = std::make_unique<CheckboxWidget>(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.input.useAltRuneRecognition);
			cb->stateChanged = [](bool checked) noexcept {
				config.input.useAltRuneRecognition = checked;
			};
			addCenter(std::move(cb));
		}
		
		{
			std::string_view label = getLocalised("system_menus_alt_bow_aim");
			auto cb = std::make_unique<CheckboxWidget>(checkboxSize(), hFontMenu, label);
			cb->setChecked(config.input.improvedBowAim);
			cb->stateChanged = [](bool checked) noexcept {
				config.input.improvedBowAim = checked;
			};
			addCenter(std::move(cb));
		}
		
		{
			std::string_view label = getLocalised("system_menus_quick_level_transition");
			auto cb = std::make_unique<CycleTextWidget>(sliderSize(), hFontMenu, label);
			cb->valueChanged = [](int pos, std::string_view /* string */) noexcept {
				config.input.quickLevelTransition = QuickLevelTransition(pos);
			};
			cb->addEntry(getLocalised("system_menus_quick_level_transition_off"));
			cb->addEntry(getLocalised("system_menus_quick_level_transition_jump"));
			cb->addEntry(getLocalised("system_menus_quick_level_transition_immediate"));
			cb->setValue(int(config.input.quickLevelTransition));
			addCenter(std::move(cb));
		}
		
		addBackButton(Page_Options);
		
		{
			auto cb = std::make_unique<ButtonWidget>(buttonSize(16, 16), "graph/interface/menus/next");
			cb->setTargetPage(Page_OptionsInputCustomizeKeys1);
			addCorner(std::move(cb), BottomRight);
		}
		
	}
	
};

class ControlOptionsPage : public MenuPage {
	
public:
	
	explicit ControlOptionsPage(MENUSTATE state)
		: MenuPage(state)
	{
		m_rowSpacing = 3;
	}
	
	void focus() override {
		MenuPage::focus();
		reinitActionKeys();
	}
	
protected:
	
	void addControlRow(ControlAction controlAction, std::string_view text) {
		
		auto panel = std::make_unique<PanelWidget>();
		
		{
			auto txt = std::make_unique<TextWidget>(hFontControls, getLocalised(text));
			txt->setEnabled(false);
			panel->add(std::move(txt));
		}
		
		std::function<void(KeybindWidget * keybind)> keyChanged = [this](KeybindWidget * keybind) {
			
			if(keybind->action() == CONTROLS_CUST_ACTION && !(keybind->key() & int(Mouse::ButtonBase))) {
				// Only allow mouse buttons for for the action binding
				keybind->setKey(config.actions[keybind->action()].key[keybind->index()]);
				return;
			}
			
			if(config.setActionKey(keybind->action(), keybind->index(), keybind->key())) {
				// Changing one key can unbind others so we have to update them all
				reinitActionKeys();
			}
			
		};
		
		for(size_t i = 0; i < 2; i++) {
			auto keybind = std::make_unique<KeybindWidget>(controlAction, i, hFontControls);
			keybind->keyChanged = keyChanged;
			keybind->setPosition(RATIO_2(Vec2f(150.f + 95.f * float(i), 0.f)));
			panel->add(std::move(keybind));
		}
		
		addCenter(std::move(panel), false);
		
	}
	
	void reinitActionKeys() {
		
		for(Widget & widget : children()) {
			if(widget.type() == WidgetType_Panel) {
				auto & panel = static_cast<PanelWidget &>(widget);
				for(Widget & child : panel.children()) {
					if(child.type() == WidgetType_Keybind) {
						auto & keybind = static_cast<KeybindWidget &>(child);
						keybind.setKey(config.actions[keybind.action()].key[keybind.index()]);
					}
				}
			}
		}
		
	}
	
	void resetActionKeys() {
		config.setDefaultActionKeys();
		reinitActionKeys();
	}
	
};


class ControlOptionsMenuPage1 final : public ControlOptionsPage {
	
public:
	
	ControlOptionsMenuPage1()
		: ControlOptionsPage(Page_OptionsInputCustomizeKeys1)
	{ }
	
	void init() override {
		
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
		
		if(config.input.allowConsole) {
			addControlRow(CONTROLS_CUST_CONSOLE, "system_menus_options_input_customize_controls_console");
		}
		
		addBackButton(Page_OptionsInput);
		
		{
			std::string_view label = getLocalised("system_menus_options_input_customize_default");
			auto txt = std::make_unique<TextWidget>(hFontMenu, label);
			txt->clicked = [this](Widget * /* widget */) {
				resetActionKeys();
			};
			addCorner(std::move(txt), BottomCenter);
		}
		
		{
			auto cb = std::make_unique<ButtonWidget>(buttonSize(16, 16), "graph/interface/menus/next");
			cb->setTargetPage(Page_OptionsInputCustomizeKeys2);
			addCorner(std::move(cb), BottomRight);
		}
		
		reinitActionKeys();
		
	}
	
};

class ControlOptionsMenuPage2 final : public ControlOptionsPage {
	
public:
	
	ControlOptionsMenuPage2()
		: ControlOptionsPage(Page_OptionsInputCustomizeKeys2)
	{ }
	
	void init() override {
		
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
		
		addControlRow(CONTROLS_CUST_DEBUG,             "system_menus_options_input_customize_controls_debug");
		
		addBackButton(Page_OptionsInputCustomizeKeys1);
		
		{
			std::string_view label = getLocalised("system_menus_options_input_customize_default");
			auto txt = std::make_unique<TextWidget>(hFontMenu, label);
			txt->clicked = [this](Widget * /* widget */) {
				resetActionKeys();
			};
			addCorner(std::move(txt), BottomCenter);
		}
		
		reinitActionKeys();
		
	}
	
};

class QuitConfirmMenuPage final : public MenuPage {
	
public:
	
	QuitConfirmMenuPage()
		: MenuPage(Page_QuitConfirm)
	{ }
	
	void init() override {
		
		reserveBottom();
		
		{
			auto txt = std::make_unique<TextWidget>(hFontMenu, getLocalised("system_menus_main_quit"));
			txt->setEnabled(false);
			addCenter(std::move(txt));
		}
		
		{
			auto txt = std::make_unique<TextWidget>(hFontMenu, getLocalised("system_menus_main_editquest_confirm"));
			txt->setEnabled(false);
			addCenter(std::move(txt));
		}
		
		{
			auto yes = std::make_unique<TextWidget>(hFontMenu, getLocalised("system_yes"));
			yes->clicked = [](Widget * /* widget */) {
				MenuFader_start(Fade_In, Mode_InGame);
			};
			addCorner(std::move(yes), BottomRight);
		}
		
		{
			auto no = std::make_unique<TextWidget>(hFontMenu, getLocalised("system_no"));
			no->setTargetPage(Page_None);
			no->setShortcut(Keyboard::Key_Escape);
			addCorner(std::move(no), BottomLeft);
		}
		
	}
	
};

void MainMenu::initWindowPages() {
	
	m_window.reset();
	
	m_window = std::make_unique<MenuWindow>();
	
	m_window->add(std::make_unique<NewQuestMenuPage>());
	
	m_window->add(std::make_unique<ChooseLoadOrSaveMenuPage>());
	m_window->add(std::make_unique<LoadMenuPage>());
	m_window->add(std::make_unique<SaveMenuPage>());
	m_window->add(std::make_unique<SaveConfirmMenuPage>());
	
	m_window->add(std::make_unique<OptionsMenuPage>());
	m_window->add(std::make_unique<VideoOptionsMenuPage>());
	m_window->add(std::make_unique<RenderOptionsMenuPage>());
	m_window->add(std::make_unique<InterfaceOptionsMenuPage>());
	m_window->add(std::make_unique<AudioOptionsMenuPage>());
	m_window->add(std::make_unique<InputOptionsMenuPage>());
	m_window->add(std::make_unique<ControlOptionsMenuPage1>());
	m_window->add(std::make_unique<ControlOptionsMenuPage2>());
	
	m_window->add(std::make_unique<QuitConfirmMenuPage>());
	m_window->add(std::make_unique<LocalizationMenuPage>());
	
}



MainMenu::MainMenu()
	: bReInitAll(false)
	, m_window(nullptr)
	, m_requestedPage(Page_None)
	, m_background(nullptr)
	, m_resumeGame(nullptr)
	, m_selected(nullptr)
{ }

MainMenu::~MainMenu() {
	delete m_background;
}

void MainMenu::init() {
	
	m_background = TextureContainer::LoadUI("graph/interface/menus/menu_main_background", TextureContainer::NoColorKey);
	
	Vec2f pos = RATIO_2(Vec2f(370, 100));
	float yOffset = RATIO_Y(50);
	
	{
		auto txt = std::make_unique<TextWidget>(hFontMainMenu, getLocalised("system_menus_main_resumegame"));
		txt->clicked = [](Widget * /* widget */) {
			if(g_canResumeGame) {
				ARXMenu_ResumeGame();
			} else {
				ARX_QuickLoad();
			}
		};
		txt->setPosition(pos);
		m_resumeGame = txt.get();
		m_widgets.add(std::move(txt));
	}
	pos.y += yOffset;
	{
		auto txt = std::make_unique<TextWidget>(hFontMainMenu, getLocalised("system_menus_main_newquest"));
		txt->clicked = [this](Widget * /* widget */) {
			if(g_canResumeGame) {
				requestPage(Page_NewQuestConfirm);
				if(m_window) {
					m_window->setScroll(0.f);
				}
			} else {
				ARXMenu_NewQuest();
			}
		};
		txt->setPosition(pos);
		m_widgets.add(std::move(txt));
	}
	pos.y += yOffset;
	{
		auto txt = std::make_unique<TextWidget>(hFontMainMenu, getLocalised("system_menus_main_editquest"));
		txt->setTargetPage(Page_LoadOrSave);
		txt->setPosition(pos);
		m_widgets.add(std::move(txt));
	}
	pos.y += yOffset;
	{
		auto txt = std::make_unique<TextWidget>(hFontMainMenu, getLocalised("system_menus_main_options"));
		txt->setTargetPage(Page_Options);
		txt->setPosition(pos);
		m_widgets.add(std::move(txt));
	}
	pos.y += yOffset;
	{
		auto txt = std::make_unique<TextWidget>(hFontMainMenu, getLocalised("system_menus_main_credits"));
		txt->clicked = [](Widget * /* widget */) {
			MenuFader_start(Fade_In, Mode_Credits);
		};
		txt->setPosition(pos);
		m_widgets.add(std::move(txt));
	}
	pos.y += yOffset;
	{
		auto txt = std::make_unique<TextWidget>(hFontMainMenu, getLocalised("system_menus_main_quit"));
		txt->setTargetPage(Page_QuitConfirm);
		txt->setPosition(pos);
		m_widgets.add(std::move(txt));
	}
	pos.y += yOffset;
	
	{
		std::string version = arx_name + " " + arx_version;
		if(!arx_release_codename.empty()) {
			version += " \"";
			version += arx_release_codename;
			version += "\"";
		}
		float verPosX = g_size.right - 20 * g_sizeRatio.x - hFontControls->getTextSize(version).width();
		auto txt = std::make_unique<TextWidget>(hFontControls, version);
		txt->setEnabled(false);
		txt->forceDisplay(TextWidget::Disabled);
		txt->setPosition(RATIO_2(Vec2f(verPosX / g_sizeRatio.x, 80)));
		m_widgets.add(std::move(txt));
	}
	
	{
		std::unique_ptr<TextWidget> txt;
		if(g_iconFont) {
			txt = std::make_unique<TextWidget>(g_iconFont, getLocalised("system_localization"));
		} else {
			txt = std::make_unique<TextWidget>(hFontMenu, "Language");
		}
		txt->setTargetPage(Page_Localization);
		txt->setPosition(Vec2f(g_size.bottomRight() - txt->font()->getTextSize(txt->text()).size()) - Vec2f(minSizeRatio() * 25.f, 0.f));
		m_widgets.add(std::move(txt));
	}
	
}

void MainMenu::update() {
	
	if(m_resumeGame) {
		m_resumeGame->setEnabled(g_canResumeGame || !savegames.empty());
	}
	
	m_selected = m_widgets.getWidgetAt(Vec2f(GInput->getMousePosition()));
	
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
	
	m_widgets.update();
	
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
	
	m_widgets.render(m_selected);
	
	m_widgets.drawDebug();
	
	if(m_window) {
		m_window->render();
	}
	
}

MainMenu * g_mainMenu;
