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
#include "input/InputKey.h"
#include "math/Vector.h"
#include "math/Rectangle.h"

class TextureContainer;
class Font;

// Enum for all the buttons in the menu
enum MenuButton {
	
	BUTTON_MENUMAIN_RESUMEGAME = 1,
	BUTTON_MENUMAIN_NEWQUEST,
	BUTTON_MENUMAIN_LOADQUEST,
	BUTTON_MENUMAIN_SAVEQUEST,
	BUTTON_MENUMAIN_MULTIPLAYER,
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

	BUTTON_MENUOPTIONSAUDIO_BACKEND,
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

//-----------------------------------------------------------------------------
enum MENUSTATE
{
	MAIN,
	RESUME_GAME,
	NEW_QUEST,
	NEW_QUEST_ENTER_GAME,
	EDIT_QUEST,
	EDIT_QUEST_LOAD,
	EDIT_QUEST_SAVE,
	EDIT_QUEST_SAVE_CONFIRM,
	MULTIPLAYER,
	OPTIONS,
	OPTIONS_VIDEO,
	OPTIONS_VIDEO_RESOLUTION,
	OPTIONS_AUDIO,
	OPTIONS_INPUT,
	OPTIONS_INPUT_CUSTOMIZE_KEYS_1,
	OPTIONS_INPUT_CUSTOMIZE_KEYS_2,
	CREDITS,
	BACK,
	QUIT,
	NOP,
	OPTIONS_LOD,
	OPTIONS_OTHERDETAILS,
	OPTIONS_VIDEO_RENDERER_OPENGL,
	OPTIONS_VIDEO_RENDERER_AUTOMATIC,
	OPTIONS_AUDIO_BACKEND_OPENAL,
	OPTIONS_AUDIO_BACKEND_AUTOMATIC,
	SAVE_QUEST_0 = 100,
	SAVE_QUEST_1 = 101,
	SAVE_QUEST_2 = 102,
	SAVE_QUEST_3 = 103,
	SAVE_QUEST_4 = 104,
	SAVE_QUEST_5 = 105,
	SAVE_QUEST_6 = 106,
	SAVE_QUEST_7 = 107,
	SAVE_QUEST_8 = 108,
	SAVE_QUEST_9 = 109,
	OPTIONS_VIDEO_RESOLUTION_0 = 200,
	OPTIONS_AUDIO_VOLUME = 300,
	OPTIONS_INPUT_KEY_0 = 400
};

//-----------------------------------------------------------------------------
class CMenuZone
{
	public:
		bool	bActif;
		bool	bCheck;
		bool	bTestYDouble;
        CMenuZone *	pRef;
		Rect	rZone;
		int			iID;
		long		lData;
		long	*	pData;
		long		lPosition;
	public:
		CMenuZone();
		CMenuZone(int, int, int, int, CMenuZone *);
		virtual ~CMenuZone();

		int GetWidth() const
		{
			return (rZone.right - rZone.left);
		}
		int GetHeight() const
		{
			return (rZone.bottom - rZone.top);
		}
		virtual void Move(int, int);
		virtual void SetPos(float, float);
 
		void SetCheckOff()
		{
			bCheck = false;
		}
		void SetCheckOn()
		{
			bCheck = true;
		};
 
		virtual CMenuZone * IsMouseOver(const Vec2s& mousePos) const;
};

//-----------------------------------------------------------------------------
class CMenuAllZone
{
	public:
		std::vector<CMenuZone *>	vMenuZone;
	public:
		CMenuAllZone();
		virtual ~CMenuAllZone();

		void AddZone(CMenuZone * menuZone);
		CMenuZone * CheckZone(const Vec2s & mousePos) const;
 
		CMenuZone * GetZoneNum(int zoneNumber);
		CMenuZone * GetZoneWithID(int zoneId);
		void Move(int dx, int dy);
		void DrawZone();
		int GetNbZone();
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

//-----------------------------------------------------------------------------
class CMenuElement : public CMenuZone
{
	public:
		ELEMPOS     ePlace;			//placement de la zone
		ELEMSTATE   eState;			//etat de l'element en cours
		MENUSTATE   eMenuState;		//etat de retour de l'element
		int         iShortCut;
	public:
		explicit CMenuElement(MENUSTATE);
		virtual ~CMenuElement();

		virtual CMenuElement * OnShortCut();
		virtual bool OnMouseClick(int button) = 0;
		virtual void Update(int time) = 0;
		virtual void Render() = 0;
		virtual void RenderMouseOver() { }
		virtual void EmptyFunction() { }
		virtual bool OnMouseDoubleClick(int button) { ARX_UNUSED(button); return false; }
		virtual CMenuZone * GetZoneWithID(int zoneId) {
			return (iID == zoneId) ? (CMenuZone *)this : NULL;
		}

		void SetShortCut(int _iShortCut)
		{
			iShortCut = _iShortCut;
		};
		virtual void setEnabled(bool enable) {
			enabled = enable;
		}
protected:
	bool enabled;
};

//-----------------------------------------------------------------------------
// faire une classe
// like a container in java

class CMenuPanel : public CMenuElement {
	
	public:
		std::vector<CMenuElement *>	vElement;
	public:
		CMenuPanel();
		virtual ~CMenuPanel();

		void Move(int dx, int dy);
		void AddElement(CMenuElement * element);
		void AddElementNoCenterIn(CMenuElement * element);
 
		void Update(int time);
		void Render();
		bool OnMouseClick(int button) { ARX_UNUSED(button); return false; }
		CMenuElement * OnShortCut();
		void RenderMouseOver() { }
		CMenuZone * IsMouseOver(const Vec2s & mousePos) const;
		CMenuZone * GetZoneWithID(int zoneId);
};

class CMenuElementText: public CMenuElement {
	
	public:
		std::string lpszText;
		Font*	pFont;
		Color lColor;
		Color lOldColor;
		Color lColorHighlight;
		float	fSize;
		bool	bSelected;
		int		iPosCursor;

	public:

		CMenuElementText(int id, Font * font, const std::string & text, float px, float py,
		                 Color color, float size, MENUSTATE state);
		virtual ~CMenuElementText();

		CMenuElement * OnShortCut();
		bool OnMouseClick(int button);
		void Update(int time);
		void Render();
		void SetText(const std::string & _pText);
		void RenderMouseOver();
 
		Vec2i GetTextSize() const;

		bool OnMouseDoubleClick(int button);
};

class CMenuButton: public CMenuElement {
	
	public:
		std::string         vText;
		int                 iPos;
		TextureContainer*   pTex;
		TextureContainer*   pTexOver;
		Font*               pFont;
		int                 iColor;
		float               fSize;

	public:
		CMenuButton(int id, Font * font, MENUSTATE state, int px, int py,
		            const std::string & label, float size = 1.f, TextureContainer * tex = NULL, 
		            TextureContainer * texOver = NULL, int color = -1);
		~CMenuButton();

	public:
		void SetPos(float px, float py);
		void AddText(const std::string & label);
		CMenuElement * OnShortCut() { return NULL; }
		bool OnMouseClick(int button);
		void Update(int time);
		void Render();
		void RenderMouseOver();
};

class CMenuSliderText: public CMenuElement {
	public:
		CMenuButton		*	pLeftButton;
		CMenuButton		*	pRightButton;
		std::vector<CMenuElementText*>	vText;
		int					iPos;
		int					iOldPos;
	public:
		CMenuSliderText(int, int, int);
		virtual ~CMenuSliderText();

	public:
		void AddText(CMenuElementText * text);

	public:
		void Move(int dx, int dy);
		bool OnMouseClick(int button);
		CMenuElement * OnShortCut() { return NULL; }
		void Update(int time);
		void Render();
		void RenderMouseOver();
		void EmptyFunction();
		void SetWidth(int width);
		virtual void setEnabled(bool enable);
};

//! Slider with value in the range [0..10]
class CMenuSlider: public CMenuElement {
	
	public:
		CMenuButton		*	pLeftButton;
		CMenuButton		*	pRightButton;
		TextureContainer	* pTex1;
		TextureContainer	* pTex2;
		int					iPos;

		void setValue(int value) { iPos = value; }
		int getValue() const { return iPos; }

	public:
		CMenuSlider(int id, int px, int py);
		virtual ~CMenuSlider();

	public:
		void Move(int dx, int dy);
		bool OnMouseClick(int button);
		CMenuElement * OnShortCut() { return NULL; }
		void Update(int time);
		void Render();
		void RenderMouseOver();
		void EmptyFunction();
};

class CMenuCheckButton : public CMenuElement {
	
	void ComputeTexturesPosition();

	public:
		int					iState;
		int					iOldState;
		int					iPosX;
		int					iPosY;
		int					iTaille;
		CMenuAllZone	*	pAllCheckZone;
		std::vector<TextureContainer *> vTex;
		CMenuElementText	* pText;

	public:
		CMenuCheckButton(int id, float px, float py, int size, TextureContainer * tex1,
		                 TextureContainer * tex2, CMenuElementText * label = NULL); 
		virtual ~CMenuCheckButton();

		void Move(int dx, int dy);
		bool OnMouseClick(int button);
		void Update(int time);
		void Render();
		void RenderMouseOver();
};

class CMenuState {
	
	public:
		bool					bReInitAll;
		MENUSTATE				eMenuState;
		MENUSTATE				eOldMenuState;
		MENUSTATE				eOldMenuWindowState;
		TextureContainer	*	pTexBackGround;
		TextureContainer	*	pTexBackGround1;
		CMenuAllZone		*	pMenuAllZone;
		CMenuElement		*	pZoneClick;
		float					fPos;

		int						iPosMenu;
	public:
		explicit CMenuState(MENUSTATE state);
		virtual ~CMenuState();

		void AddMenuElement(CMenuElement * element);
		MENUSTATE Update(int time);
		void Render();
};

class CWindowMenuConsole {
	
	public:
		bool					bMouseListen;
		bool					bFrameOdd;

		int						iPosX;
		int						iPosY;
		int						iSavePosY;
		int						iOldPosX;
		int						iOldPosY;
		int						iOX;
		int						iOY;
		int						iWidth;
		int						iHeight;
		int						iInterligne;
		MENUSTATE				eMenuState;
		CMenuAllZone			MenuAllZone;
		CMenuElement		*	pZoneClick;
		bool					bEdit;
		TextureContainer	*	pTexBackground;
		TextureContainer	*	pTexBackgroundBorder;

		long					lData;
		long			*		pData;

		int						iPosMenu;
		bool				bMouseAttack;
	private:
		void UpdateText();
	public:
		CWindowMenuConsole(int px, int py, int width, int height, MENUSTATE state);
 
		void AddMenu(CMenuElement * menu);
		void AddMenuCenter(CMenuElement * menu);
		void AddMenuCenterY(CMenuElement * menu);
		void AlignElementCenter(CMenuElement * element);
		MENUSTATE Update(int px, int py, int offsety);
		int Render();
 
		CMenuElement * GetTouch(bool keyTouched, int keyId, InputKeyId* pInputKeyId = NULL, bool _bValidateTest = false);
		void ReInitActionKey();
};

class CWindowMenu {
	
	public:
		bool				bMouseListen;
		int					iPosX;
		int					iPosY;
		int					iNbButton;
		int					iTailleX;
		int					iTailleY;
		TextureContainer	* pTexButton;
		TextureContainer	* pTexButton2;
		TextureContainer	* pTexButton3;
		TextureContainer	* pTexMain;
		TextureContainer	* pTexMainShadow;
		TextureContainer	* pTexGlissiere;
		TextureContainer	* pTexGlissiereButton;
		std::vector<CWindowMenuConsole *>	vWindowConsoleElement;
		float				fPosXCalc;
		float				fDist;
		float				fAngle;
		MENUSTATE			eCurrentMenuState;
		bool				bChangeConsole;
	public:
		CWindowMenu(int px, int py, int width, int height, int nButton);
		virtual ~CWindowMenu();

		void AddConsole(CWindowMenuConsole * console);
		void Update(float time);
		MENUSTATE Render();
};

class MenuCursor {

public:
	
	enum CURSORSTATE {
		CURSOR_OFF,
		CURSOR_ON,
	};
	
	MenuCursor();
	virtual ~MenuCursor();
	
	void Update();
	void SetMouseOver();
	void SetCursorOn();
	void SetCursorOff();
	void DrawCursor();
	
private:
	
	void DrawOneCursor(const Vec2s & mousePos);
	
	bool exited; //! Has the mouse exited the window
	
	// Cursor
	TextureContainer	* pTex[8];
	long				lFrameDiff;
	CURSORSTATE			eNumTex;
	int					iNumCursor;
	float				fTailleX;
	float				fTailleY;
	bool				bMouseOver;
	bool				bDrawCursor;
	
	// For the ribbon effect
	int					iNbOldCoord;
	int					iMaxOldCoord;
	Vec2s				iOldCoord[256];
	
};

bool Menu2_Render();
void Menu2_Close();

bool ProcessFadeInOut(bool _bFadeIn, float _fspeed);

void ARX_MENU_Clicked_NEWQUEST();
void ARX_MENU_Clicked_QUIT();

bool ARX_QuickLoad();
void ARX_QuickSave();
bool ARX_SlotLoad(int slotIndex);
void ARX_DrawAfterQuickLoad();

#endif // ARX_GUI_MENUWIDGETS_H
