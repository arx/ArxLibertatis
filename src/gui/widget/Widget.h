/*
 * Copyright 2015-2016 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GUI_WIDGET_WIDGET_H
#define ARX_GUI_WIDGET_WIDGET_H

#include <boost/noncopyable.hpp>

#include "core/SaveGame.h"
#include "input/InputKey.h"
#include "math/Rectangle.h"
#include "util/HandleType.h"

// Enum for all the buttons in the menu
enum MenuButton {
	BUTTON_INVALID = -1,

	BUTTON_MENUEDITQUEST_LOAD = 1,

	BUTTON_MENUEDITQUEST_SAVEINFO,
};

enum ELEMSTATE
{
	TNOP,
	//Element Text
	EDIT,           //type d'etat
	GETTOUCH,
	EDIT_TIME,      //etat en cours
	GETTOUCH_TIME
};

enum ELEMPOS
{
	NOCENTER,
	CENTER,
	CENTERY
};

enum MENUSTATE
{
	Page_None,
	RESUME_GAME,
	Page_NewQuestConfirm,
	Page_LoadOrSave,
	Page_Load,
	Page_Save,
	Page_SaveConfirm,
	Page_Options,
	Page_OptionsVideo,
	Page_OptionsRender,
	Page_OptionsInterface,
	Page_OptionsAudio,
	Page_OptionsInput,
	Page_OptionsInputCustomizeKeys1,
	Page_OptionsInputCustomizeKeys2,
	CREDITS,
	Page_QuitConfirm,
	NOP
};

enum WidgetType {
	WidgetType_Button,
	WidgetType_Checkbox,
	WidgetType_CycleText,
	WidgetType_Panel,
	WidgetType_Slider,
	WidgetType_Text
};

class Widget : private boost::noncopyable {
	
public:
	Rectf m_rect;
	
	MenuButton m_id;
	
	SavegameHandle m_savegame;
	
	ELEMPOS     ePlace;
	ELEMSTATE   eState;
	MENUSTATE   m_targetMenu;
	InputKeyId  m_shortcut;
	
public:
	explicit Widget();
	virtual ~Widget();
	
	virtual bool OnMouseClick() = 0;
	virtual void Update() = 0;
	virtual void Render() = 0;
	virtual void RenderMouseOver() { }
	virtual void EmptyFunction() { }
	virtual void OnMouseDoubleClick() { }
	
	void SetShortCut(int _iShortCut);
	
	virtual void setEnabled(bool enable);
	
	virtual void Move(const Vec2f & offset);
	virtual void SetPos(Vec2f pos);
	
	void SetCheckOff();
	void SetCheckOn();
	
	bool getCheck();
	
	virtual Widget * IsMouseOver(const Vec2f & mousePos);
	
	virtual WidgetType type() const = 0;
	
protected:
	bool enabled;
	bool bCheck;
};

#endif // ARX_GUI_WIDGET_WIDGET_H
