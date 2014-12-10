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
#include "gui/MainMenu.h"
#include "input/InputKey.h"
#include "math/Vector.h"
#include "math/Rectangle.h"
#include "util/HandleType.h"

class TextureContainer;
class Font;

// Enum for all the buttons in the menu
enum MenuButton {
	
	BUTTON_MENUMAIN_RESUMEGAME = 1,
	BUTTON_MENUMAIN_NEWQUEST,
	BUTTON_MENUMAIN_OPTIONS,
	BUTTON_MENUMAIN_CREDITS,
	BUTTON_MENUMAIN_QUIT,

	BUTTON_MENUNEWQUEST_CONFIRM,

	BUTTON_MENUEDITQUEST_LOAD_INIT,
	BUTTON_MENUEDITQUEST_LOAD,
	BUTTON_MENUEDITQUEST_LOAD_CONFIRM,
	BUTTON_MENUEDITQUEST_SAVE,
	BUTTON_MENUEDITQUEST_DELETE,
	BUTTON_MENUEDITQUEST_DELETE_CONFIRM,

	BUTTON_MENUOPTIONSVIDEO_INIT,
	BUTTON_MENUOPTIONSVIDEO_RENDERER,
	BUTTON_MENUOPTIONSVIDEO_RESOLUTION,
	BUTTON_MENUOPTIONSVIDEO_FULLSCREEN,
	BUTTON_MENUOPTIONSVIDEO_APPLY,
	BUTTON_MENUOPTIONSVIDEO_FOG,
	BUTTON_MENUOPTIONSVIDEO_CROSSHAIR,
	BUTTON_MENUOPTIONSVIDEO_ANTIALIASING,
	BUTTON_MENUOPTIONSVIDEO_VSYNC,
	BUTTON_MENUOPTIONSVIDEO_OTHERSDETAILS,

	BUTTON_MENUOPTIONSAUDIO_DEVICE,
	BUTTON_MENUOPTIONSAUDIO_MASTER,
	BUTTON_MENUOPTIONSAUDIO_SFX,
	BUTTON_MENUOPTIONSAUDIO_SPEECH,
	BUTTON_MENUOPTIONSAUDIO_AMBIANCE,
	BUTTON_MENUOPTIONSAUDIO_EAX,

	BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_MAGICMODE1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_MAGICMODE2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_STEALTHMODE1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_STEALTHMODE2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKFORWARD1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKFORWARD2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKBACKWARD1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKBACKWARD2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFELEFT1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFELEFT2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFERIGHT1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFERIGHT2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANLEFT1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANLEFT2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANRIGHT1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANRIGHT2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCH1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCH2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_USE1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_USE2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_ACTIONCOMBINE1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_ACTIONCOMBINE2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_INVENTORY1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_INVENTORY2,

	BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOK1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOK2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKCHARSHEET1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKCHARSHEET2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKSPELL1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKSPELL2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKMAP1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKMAP2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKQUEST1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKQUEST2,

	BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONLIFE1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONLIFE2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONMANA1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONMANA2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_TORCH1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_TORCH2,

	BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST1_2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST2_2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST3,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST3_2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_WEAPON1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_WEAPON2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKLOAD,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKLOAD2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKSAVE,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKSAVE2,

	BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNLEFT1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNLEFT2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNRIGHT1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNRIGHT2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKUP1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKUP2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKDOWN1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKDOWN2,

	BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFE1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFE2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_CENTERVIEW1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_CENTERVIEW2,

	BUTTON_MENUOPTIONS_CONTROLS_CUST_FREELOOK1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_FREELOOK2,

	BUTTON_MENUOPTIONS_CONTROLS_CUST_PREVIOUS1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_PREVIOUS2,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_NEXT1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_NEXT2,

	BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCHTOGGLE1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCHTOGGLE2,

	BUTTON_MENUOPTIONS_CONTROLS_CUST_UNEQUIPWEAPON1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_UNEQUIPWEAPON2,

	BUTTON_MENUOPTIONS_CONTROLS_CUST_CANCELCURSPELL1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_CANCELCURSPELL2,

	BUTTON_MENUOPTIONS_CONTROLS_CUST_MINIMAP1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_MINIMAP2,
	
	BUTTON_MENUOPTIONS_CONTROLS_CUST_TOGGLE_FULLSCREEN1,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_TOGGLE_FULLSCREEN2,

	BUTTON_MENUOPTIONS_CONTROLS_CUST_BACK,
	BUTTON_MENUOPTIONS_CONTROLS_CUST_DEFAULT,

	BUTTON_MENUOPTIONS_CONTROLS_INVERTMOUSE,
	BUTTON_MENUOPTIONS_CONTROLS_AUTOREADYWEAPON,
	BUTTON_MENUOPTIONS_CONTROLS_MOUSELOOK,
	BUTTON_MENUOPTIONS_CONTROLS_MOUSESENSITIVITY,
	BUTTON_MENUOPTIONS_CONTROLS_AUTODESCRIPTION,
	BUTTON_MENUOPTIONS_CONTROLS_QUICKSAVESLOTS,
	BUTTON_MENUEDITQUEST_LOAD_CONFIRM_BACK,

	BUTTON_MENUOPTIONS_CONTROLS_BACK,

	BUTTON_MENUOPTIONS_CONTROLS_LINK,

	BUTTON_MENUOPTIONSVIDEO_BACK,

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
	virtual Widget * GetZoneWithID(int zoneId) {
		return (iID == zoneId) ? this : NULL;
	}
	
	void SetShortCut(int _iShortCut)
	{
		iShortCut = _iShortCut;
	};
	virtual void setEnabled(bool enable) {
		enabled = enable;
	}
	
	virtual void Move(const Vec2i & offset);
	virtual void SetPos(Vec2i pos);
	
	void SetCheckOff()
	{
		bCheck = false;
	}
	void SetCheckOn()
	{
		bCheck = true;
	};
	
	bool getCheck() { return bCheck; }
	
	virtual Widget * IsMouseOver(const Vec2s& mousePos) const;
	
protected:
	bool enabled;
	bool bCheck;
};

class CMenuAllZone
{
public:
	std::vector<Widget *>	vMenuZone;
public:
	CMenuAllZone();
	virtual ~CMenuAllZone();
	
	void AddZone(Widget * menuZone);
	Widget * CheckZone(const Vec2s & mousePos) const;
	
	Widget * GetZoneNum(size_t index);
	Widget * GetZoneWithID(int zoneId);
	void Move(const Vec2i & offset);
	void DrawZone();
	size_t GetNbZone();
};

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
	
	TextWidget(int id, Font * font, const std::string & text, Vec2i pos = Vec2i_ZERO, MENUSTATE state = NOP);
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
	explicit CycleTextWidget(int _iID);
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
	SliderWidget(int id, Vec2i pos);
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

class CMenuState {
	
public:
	bool					bReInitAll;
	MENUSTATE				eOldMenuState;
	MENUSTATE				eOldMenuWindowState;
	TextureContainer	*	pTexBackGround;
	CMenuAllZone		*	pMenuAllZone;
	Widget		*	pZoneClick;
	
public:
	explicit CMenuState();
	virtual ~CMenuState();
	
	void createChildElements();
	
	void AddMenuElement(Widget * element);
	MENUSTATE Update();
	void Render();
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
	CMenuAllZone MenuAllZone;
	
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

class MenuCursor {

public:
	
	enum CURSORSTATE {
		CURSOR_OFF,
		CURSOR_ON,
	};
	
	MenuCursor();
	virtual ~MenuCursor();
	
	void reset();
	void update(float time);
	void SetMouseOver();
	void SetCursorOn();
	void SetCursorOff();
	void DrawCursor();
	
private:
	bool ComputePer(const Vec2s & _psPoint1, const Vec2s & _psPoint2, TexturedVertex * _psd3dv1, TexturedVertex * _psd3dv2, float _fSize);
	void DrawLine2D(float _fSize, Color3f color);
	
	void DrawOneCursor(const Vec2s & mousePos);
	
	Vec2s m_size;
	bool exited; //! Has the mouse exited the window
	
	float m_storedTime;
	
	// Cursor
	long				lFrameDiff;
	CURSORSTATE			eNumTex;
	int					m_currentFrame;
	bool				bMouseOver;
	
	// For the ribbon effect
	int					iNbOldCoord;
	int					iMaxOldCoord;
	Vec2s				iOldCoord[256];
	
};

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
