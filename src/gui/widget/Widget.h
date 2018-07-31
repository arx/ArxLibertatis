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

enum MENUSTATE {
	NOP,
	Page_None,
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
	Page_QuitConfirm,
};

enum WidgetType {
	WidgetType_Spacer,
	WidgetType_Button,
	WidgetType_Checkbox,
	WidgetType_CycleText,
	WidgetType_Panel,
	WidgetType_Slider,
	WidgetType_Text,
	WidgetType_TextInput,
	WidgetType_Keybind,
	WidgetType_SaveSlot,
};

class Widget : private boost::noncopyable {
	
public:
	Rectf m_rect;
	
	MENUSTATE   m_targetMenu;
	
protected:
	
	bool m_enabled;
	
public:
	
	Widget();
	virtual ~Widget();
	
	virtual bool click();
	virtual bool doubleClick() { return click(); }
	virtual void hover() { }
	virtual void update() { }
	virtual void render(bool mouseOver = false) = 0;
	virtual bool wantFocus() const { return false; }
	virtual void unfocus() { }
	
	InputKeyId shortcut() const { return m_shortcut; }
	void setShortcut(int key);
	
	virtual void setEnabled(bool enable);
	bool isEnabled() const { return m_enabled; }
	
	virtual void move(const Vec2f & offset);
	void SetPos(Vec2f pos) { move(pos - m_rect.topLeft()); }
	
	virtual Widget * getWidgetAt(const Vec2f & mousePos);
	
	virtual WidgetType type() const = 0;
	
private:
	
	InputKeyId m_shortcut;
	
};

#endif // ARX_GUI_WIDGET_WIDGET_H
