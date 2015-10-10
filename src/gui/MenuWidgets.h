/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GUI_MENUWIDGETS_H
#define ARX_GUI_MENUWIDGETS_H

#include <vector>
#include <string>

#include "graphics/Color.h"
#include "gui/widget/Widget.h"
#include "gui/widget/WidgetContainer.h"
#include "input/InputKey.h"
#include "math/Vector.h"
#include "math/Rectangle.h"
#include "util/HandleType.h"

class TextureContainer;
class Font;

//-----------------------------------------------------------------------------
// faire une classe
// like a container in java

class HorizontalPanelWidget : public Widget {
	
public:
	HorizontalPanelWidget();
	virtual ~HorizontalPanelWidget();
	
	void Move(const Vec2i & offset);
	void AddElement(Widget * element);
	void AddElementNoCenterIn(Widget * element);
	
	void Update(int time);
	void Render();
	bool OnMouseClick() { return false; }
	Widget * OnShortCut();
	void RenderMouseOver() { }
	Widget * IsMouseOver(const Vec2s & mousePos) const;
	Widget * GetZoneWithID(int zoneId);
	
private:
	std::vector<Widget *>	vElement;
};

class TextWidget: public Widget {
	
public:
	std::string lpszText;
	Font*	pFont;
	Color lColor;
	Color lOldColor;
	Color lColorHighlight;
	bool	bSelected;
	
public:
	TextWidget(MenuButton id, Font * font, const std::string & text, Vec2i pos = Vec2i_ZERO, MENUSTATE state = NOP);
	virtual ~TextWidget();
	
	void setColor(Color color) { lColor = color; }
	
	Widget * OnShortCut();
	bool OnMouseClick();
	void Update(int time);
	void Render();
	void SetText(const std::string & _pText);
	void RenderMouseOver();
	
	bool OnMouseDoubleClick();
};

class ButtonWidget: public Widget {
	
public:
	ButtonWidget(Vec2i pos, const char * texturePath);
	~ButtonWidget();
	
public:
	void SetPos(Vec2i pos);
	void AddText(const std::string & label);
	Widget * OnShortCut() { return NULL; }
	bool OnMouseClick();
	void Update(int time);
	void Render();
	void RenderMouseOver();
	
private:
	TextureContainer * m_texture;
};

class CycleTextWidget: public Widget {
	
public:
	explicit CycleTextWidget(MenuButton _iID);
	virtual ~CycleTextWidget();
	
	void setValue(int value) { iPos = value; }
	int getValue() const { return iPos; }
	void setOldValue(int value) { iOldPos = value; }
	int getOldValue() const { return iOldPos; }
	
	void selectLast();
	
	void AddText(TextWidget * text);
	
	void Move(const Vec2i & offset);
	bool OnMouseClick();
	Widget * OnShortCut() { return NULL; }
	void Update(int time);
	void Render();
	void RenderMouseOver();
	void EmptyFunction();
	virtual void setEnabled(bool enable);
	
private:
	ButtonWidget		*	pLeftButton;
	ButtonWidget		*	pRightButton;
	std::vector<TextWidget*>	vText;
	int					iPos;
	int					iOldPos;
};

//! Slider with value in the range [0..10]
class SliderWidget: public Widget {
	
public:
	SliderWidget(MenuButton id, Vec2i pos);
	virtual ~SliderWidget();
	
	void setValue(int value) { m_value = value; }
	int getValue() const { return m_value; }
	
	void Move(const Vec2i & offset);
	bool OnMouseClick();
	Widget * OnShortCut() { return NULL; }
	void Update(int time);
	void Render();
	void RenderMouseOver();
	void EmptyFunction();
	
private:
	ButtonWidget		*	pLeftButton;
	ButtonWidget		*	pRightButton;
	TextureContainer	* pTex1;
	TextureContainer	* pTex2;
	int					m_value;
};

class CheckboxWidget : public Widget {
	
public:
	explicit CheckboxWidget(TextWidget * label);
	virtual ~CheckboxWidget();
	
	void Move(const Vec2i & offset);
	bool OnMouseClick();
	void Update(int time);
	
	void renderCommon();
	void Render();
	void RenderMouseOver();
	
	int					iState;
	int					iOldState;
	
private:
	TextureContainer * m_textureOff;
	TextureContainer * m_textureOn;
	TextWidget	* pText;
};

class CWindowMenuConsole {
	
public:
	CWindowMenuConsole(Vec2i pos, Vec2i size, MENUSTATE state);
	
	void AddMenu(Widget * element);
	void AddMenuCenter(Widget * element, bool centerX = false);
	void AlignElementCenter(Widget * element);
	MENUSTATE Update(Vec2i pos);
	void Render();
	
	Widget * GetTouch(bool keyTouched, int keyId, InputKeyId* pInputKeyId = NULL, bool _bValidateTest = false);
	void ReInitActionKey();
	
	Vec2i m_pos;
	Vec2i m_oldPos;
	int m_rowSpacing;
	SavegameHandle m_savegame;
	MENUSTATE eMenuState;
	WidgetContainer MenuAllZone;
	
private:
	void UpdateText();
	
	bool					bFrameOdd;
	
	Vec2i m_offset;
	Vec2i m_size;
	
	Widget		*	pZoneClick;
	bool					bEdit;
	TextureContainer	*	pTexBackground;
	TextureContainer	*	pTexBackgroundBorder;
	
	bool				bMouseAttack;
	
	static const int m_textCursorFlashDuration = 300;
	float m_textCursorCurrentTime;
};

class CWindowMenu {
	
private:
	Vec2i m_pos;
	Vec2i m_size;
	float				fPosXCalc;
	float				fDist;
	
public:
	CWindowMenu(Vec2i pos, Vec2i size);
	virtual ~CWindowMenu();
	
	void AddConsole(CWindowMenuConsole * console);
	void Update(float time);
	MENUSTATE Render();
	
	std::vector<CWindowMenuConsole *>	vWindowConsoleElement;
	float				fAngle;
	MENUSTATE			eCurrentMenuState;
};

struct TexturedVertex;



void MenuReInitAll();

void Menu2_Open();
bool Menu2_Render();
void Menu2_Close();

bool ProcessFadeInOut(bool _bFadeIn, float _fspeed);

void ARX_MENU_Clicked_QUIT();

bool ARX_QuickLoad();
void ARX_QuickSave();
bool ARX_SlotLoad(int slotIndex);

bool MENU_NoActiveWindow();

#endif // ARX_GUI_MENUWIDGETS_H
