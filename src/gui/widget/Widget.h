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

#include "math/Rectangle.h"
#include "util/HandleType.h"

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
	OPTIONS_AUDIO,
	OPTIONS_INPUT,
	OPTIONS_INPUT_CUSTOMIZE_KEYS_1,
	OPTIONS_INPUT_CUSTOMIZE_KEYS_2,
	CREDITS,
	QUIT,
	NOP,
	OPTIONS_VIDEO_RENDERER_OPENGL,
	OPTIONS_VIDEO_RENDERER_AUTOMATIC,
	OPTIONS_AUDIO_BACKEND_OPENAL,
	OPTIONS_AUDIO_BACKEND_AUTOMATIC,
};

ARX_HANDLE_TYPEDEF(long, SavegameHandle, -1);

class Widget {
	
public:

	bool	bTestYDouble;
	Widget *	pRef;
	Rect	rZone;
	int			iID;
	
	SavegameHandle m_savegame;
	
	ELEMPOS     ePlace;			//placement de la zone
	ELEMSTATE   eState;			//etat de l'element en cours
	MENUSTATE   eMenuState;		//etat de retour de l'element
	int         iShortCut;
	
public:
	explicit Widget(MENUSTATE);
	virtual ~Widget();
	
	virtual Widget * OnShortCut();
	virtual bool OnMouseClick() = 0;
	virtual void Update(int time) = 0;
	virtual void Render() = 0;
	virtual void RenderMouseOver() { }
	virtual void EmptyFunction() { }
	virtual bool OnMouseDoubleClick() { return false; }
	virtual Widget * GetZoneWithID(int zoneId);
	
	void SetShortCut(int _iShortCut);
	
	virtual void setEnabled(bool enable);
	
	virtual void Move(const Vec2i & offset);
	virtual void SetPos(Vec2i pos);
	
	void SetCheckOff();
	void SetCheckOn();
	
	bool getCheck();
	
	virtual Widget * IsMouseOver(const Vec2s& mousePos) const;
	
protected:
	bool enabled;
	bool bCheck;
};

#endif // ARX_GUI_WIDGET_WIDGET_H
