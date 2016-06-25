/*
 * Copyright 2015 Arx Libertatis Team (see the AUTHORS file)
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
#include "math/Rectangle.h"
#include "util/HandleType.h"

// Enum for all the buttons in the menu
enum MenuButton {
	BUTTON_INVALID = -1,

	BUTTON_MENUEDITQUEST_LOAD = 1,
	BUTTON_MENUEDITQUEST_LOAD_CONFIRM,
	BUTTON_MENUEDITQUEST_SAVE,
	BUTTON_MENUEDITQUEST_DELETE,
	BUTTON_MENUEDITQUEST_DELETE_CONFIRM,

	BUTTON_MENUOPTIONS_CONTROLS_CUST_DEFAULT,

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
	MAIN,
	RESUME_GAME,
	NEW_QUEST,
	EDIT_QUEST,
	EDIT_QUEST_LOAD,
	EDIT_QUEST_SAVE,
	EDIT_QUEST_SAVE_CONFIRM,
	OPTIONS,
	OPTIONS_VIDEO,
	OPTIONS_INTERFACE,
	OPTIONS_AUDIO,
	OPTIONS_INPUT,
	OPTIONS_INPUT_CUSTOMIZE_KEYS_1,
	OPTIONS_INPUT_CUSTOMIZE_KEYS_2,
	CREDITS,
	QUIT,
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
	Widget *	pRef;
	
	MenuButton m_id;
	
	SavegameHandle m_savegame;
	
	ELEMPOS     ePlace;			//placement de la zone
	ELEMSTATE   eState;			//etat de l'element en cours
	MENUSTATE   m_targetMenu;		//etat de retour de l'element
	int         m_shortcut;
	
public:
	explicit Widget();
	virtual ~Widget();
	
	virtual bool OnMouseClick() = 0;
	virtual void Update() = 0;
	virtual void Render() = 0;
	virtual void RenderMouseOver() { }
	virtual void EmptyFunction() { }
	virtual bool OnMouseDoubleClick() { return false; }
	virtual Widget * GetZoneWithID(MenuButton zoneId);
	
	void SetShortCut(int _iShortCut);
	
	virtual void setEnabled(bool enable);
	
	virtual void Move(const Vec2f & offset);
	virtual void SetPos(Vec2f pos);
	
	void SetCheckOff();
	void SetCheckOn();
	
	bool getCheck();
	
	virtual Widget * IsMouseOver(const Vec2f & mousePos) const;
	
	virtual WidgetType type() const = 0;
	
protected:
	bool enabled;
	bool bCheck;
};

#endif // ARX_GUI_WIDGET_WIDGET_H
