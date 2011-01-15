/*
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
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
// ARX_Menu
//////////////////////////////////////////////////////////////////////////////////////

#ifndef ARX_MENU2_H
#define ARX_MENU2_H

#include <windows.h>
#include <vector>
#include <d3d.h>
#include "arx_text.h"

using namespace std;

//-----------------------------------------------------------------------------

#define BUTTON_MENUMAIN_RESUMEGAME			1
#define BUTTON_MENUMAIN_NEWQUEST			(1+BUTTON_MENUMAIN_RESUMEGAME)
#define BUTTON_MENUMAIN_LOADQUEST			(1+BUTTON_MENUMAIN_NEWQUEST)
#define BUTTON_MENUMAIN_SAVEQUEST			(1+BUTTON_MENUMAIN_LOADQUEST)
#define BUTTON_MENUMAIN_MULTIPLAYER			(1+BUTTON_MENUMAIN_SAVEQUEST)
#define BUTTON_MENUMAIN_OPTIONS				(1+BUTTON_MENUMAIN_MULTIPLAYER)
#define BUTTON_MENUMAIN_CREDITS				(1+BUTTON_MENUMAIN_OPTIONS)
#define BUTTON_MENUMAIN_QUIT				(1+BUTTON_MENUMAIN_CREDITS)

#define BUTTON_MENUNEWQUEST_CONFIRM			(1+BUTTON_MENUMAIN_QUIT)

#define BUTTON_MENUEDITQUEST_LOAD_INIT		(1+BUTTON_MENUNEWQUEST_CONFIRM)
#define BUTTON_MENUEDITQUEST_LOAD			(1+BUTTON_MENUEDITQUEST_LOAD_INIT)
#define BUTTON_MENUEDITQUEST_LOAD_CONFIRM	(1+BUTTON_MENUEDITQUEST_LOAD)
#define BUTTON_MENUEDITQUEST_SAVE			(1+BUTTON_MENUEDITQUEST_LOAD_CONFIRM)
#define BUTTON_MENUEDITQUEST_DELETE			(1+BUTTON_MENUEDITQUEST_SAVE)


#define BUTTON_MENUOPTIONSVIDEO_INIT		(1+BUTTON_MENUEDITQUEST_DELETE)
#define BUTTON_MENUOPTIONSVIDEO_RESOLUTION	(1+BUTTON_MENUOPTIONSVIDEO_INIT)
#define BUTTON_MENUOPTIONSVIDEO_BPP			(1+BUTTON_MENUOPTIONSVIDEO_RESOLUTION)
#define BUTTON_MENUOPTIONSVIDEO_FULLSCREEN	(1+BUTTON_MENUOPTIONSVIDEO_BPP)
#define BUTTON_MENUOPTIONSVIDEO_APPLY		(1+BUTTON_MENUOPTIONSVIDEO_FULLSCREEN)
#define BUTTON_MENUOPTIONSVIDEO_BUMP		(1+BUTTON_MENUOPTIONSVIDEO_APPLY)
#define BUTTON_MENUOPTIONSVIDEO_TEXTURES	(1+BUTTON_MENUOPTIONSVIDEO_BUMP)
#define BUTTON_MENUOPTIONSVIDEO_LOD			(1+BUTTON_MENUOPTIONSVIDEO_TEXTURES)
#define BUTTON_MENUOPTIONSVIDEO_FOG			(1+BUTTON_MENUOPTIONSVIDEO_LOD)
#define BUTTON_MENUOPTIONSVIDEO_GAMMA		(1+BUTTON_MENUOPTIONSVIDEO_FOG)
#define BUTTON_MENUOPTIONSVIDEO_LUMINOSITY	(1+BUTTON_MENUOPTIONSVIDEO_GAMMA)
#define BUTTON_MENUOPTIONSVIDEO_CONTRAST	(1+BUTTON_MENUOPTIONSVIDEO_LUMINOSITY)
#define BUTTON_MENUOPTIONSVIDEO_CROSSHAIR	(1+BUTTON_MENUOPTIONSVIDEO_CONTRAST)
#define BUTTON_MENUOPTIONSVIDEO_ANTIALIASING (1+BUTTON_MENUOPTIONSVIDEO_CROSSHAIR)
#define BUTTON_MENUOPTIONSVIDEO_OTHERSDETAILS	(1+BUTTON_MENUOPTIONSVIDEO_ANTIALIASING)
#define BUTTON_MENUOPTIONSVIDEO_DEBUGSETTING	(1+BUTTON_MENUOPTIONSVIDEO_OTHERSDETAILS)


#define BUTTON_MENUOPTIONSAUDIO_MASTER		(1+BUTTON_MENUOPTIONSVIDEO_OTHERSDETAILS)
#define BUTTON_MENUOPTIONSAUDIO_SFX			(1+BUTTON_MENUOPTIONSAUDIO_MASTER)
#define BUTTON_MENUOPTIONSAUDIO_SPEECH		(1+BUTTON_MENUOPTIONSAUDIO_SFX)
#define BUTTON_MENUOPTIONSAUDIO_AMBIANCE	(1+BUTTON_MENUOPTIONSAUDIO_SPEECH)
#define BUTTON_MENUOPTIONSAUDIO_EAX			(1+BUTTON_MENUOPTIONSAUDIO_AMBIANCE)

#define BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP1				(BUTTON_MENUOPTIONSAUDIO_EAX+1)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP2				(BUTTON_MENUOPTIONSAUDIO_EAX+2)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_MAGICMODE1			(BUTTON_MENUOPTIONSAUDIO_EAX+3)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_MAGICMODE2			(BUTTON_MENUOPTIONSAUDIO_EAX+4)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_STEALTHMODE1		(BUTTON_MENUOPTIONSAUDIO_EAX+5)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_STEALTHMODE2		(BUTTON_MENUOPTIONSAUDIO_EAX+6)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKFORWARD1		(BUTTON_MENUOPTIONSAUDIO_EAX+7)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKFORWARD2		(BUTTON_MENUOPTIONSAUDIO_EAX+8)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKBACKWARD1		(BUTTON_MENUOPTIONSAUDIO_EAX+9)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKBACKWARD2		(BUTTON_MENUOPTIONSAUDIO_EAX+10)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFELEFT1		(BUTTON_MENUOPTIONSAUDIO_EAX+11)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFELEFT2		(BUTTON_MENUOPTIONSAUDIO_EAX+12)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFERIGHT1		(BUTTON_MENUOPTIONSAUDIO_EAX+13)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFERIGHT2		(BUTTON_MENUOPTIONSAUDIO_EAX+14)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANLEFT1			(BUTTON_MENUOPTIONSAUDIO_EAX+15)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANLEFT2			(BUTTON_MENUOPTIONSAUDIO_EAX+16)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANRIGHT1			(BUTTON_MENUOPTIONSAUDIO_EAX+17)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANRIGHT2			(BUTTON_MENUOPTIONSAUDIO_EAX+18)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCH1			(BUTTON_MENUOPTIONSAUDIO_EAX+19)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCH2			(BUTTON_MENUOPTIONSAUDIO_EAX+20)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_MOUSELOOK1			(BUTTON_MENUOPTIONSAUDIO_EAX+21)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_MOUSELOOK2			(BUTTON_MENUOPTIONSAUDIO_EAX+22)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_ACTIONCOMBINE1		(BUTTON_MENUOPTIONSAUDIO_EAX+23)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_ACTIONCOMBINE2		(BUTTON_MENUOPTIONSAUDIO_EAX+24)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_INVENTORY1			(BUTTON_MENUOPTIONSAUDIO_EAX+25)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_INVENTORY2			(BUTTON_MENUOPTIONSAUDIO_EAX+26)

#define BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOK1				(BUTTON_MENUOPTIONSAUDIO_EAX+27)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOK2				(BUTTON_MENUOPTIONSAUDIO_EAX+28)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKCHARSHEET1		(BUTTON_MENUOPTIONSAUDIO_EAX+29)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKCHARSHEET2		(BUTTON_MENUOPTIONSAUDIO_EAX+30)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKSPELL1			(BUTTON_MENUOPTIONSAUDIO_EAX+31)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKSPELL2			(BUTTON_MENUOPTIONSAUDIO_EAX+32)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKMAP1			(BUTTON_MENUOPTIONSAUDIO_EAX+33)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKMAP2			(BUTTON_MENUOPTIONSAUDIO_EAX+34)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKQUEST1			(BUTTON_MENUOPTIONSAUDIO_EAX+35)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKQUEST2			(BUTTON_MENUOPTIONSAUDIO_EAX+36)

#define BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONLIFE1	(BUTTON_MENUOPTIONSAUDIO_EAX+37)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONLIFE2	(BUTTON_MENUOPTIONSAUDIO_EAX+38)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONMANA1	(BUTTON_MENUOPTIONSAUDIO_EAX+39)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONMANA2	(BUTTON_MENUOPTIONSAUDIO_EAX+40)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_TORCH1				(BUTTON_MENUOPTIONSAUDIO_EAX+41)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_TORCH2				(BUTTON_MENUOPTIONSAUDIO_EAX+42)

#define BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST1			(BUTTON_MENUOPTIONSAUDIO_EAX+43)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST1_2			(BUTTON_MENUOPTIONSAUDIO_EAX+44)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST2			(BUTTON_MENUOPTIONSAUDIO_EAX+45)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST2_2			(BUTTON_MENUOPTIONSAUDIO_EAX+46)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST3			(BUTTON_MENUOPTIONSAUDIO_EAX+47)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST3_2			(BUTTON_MENUOPTIONSAUDIO_EAX+48)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_WEAPON1			(BUTTON_MENUOPTIONSAUDIO_EAX+49)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_WEAPON2			(BUTTON_MENUOPTIONSAUDIO_EAX+50)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKLOAD			(BUTTON_MENUOPTIONSAUDIO_EAX+51)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKLOAD2			(BUTTON_MENUOPTIONSAUDIO_EAX+52)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKSAVE			(BUTTON_MENUOPTIONSAUDIO_EAX+53)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKSAVE2			(BUTTON_MENUOPTIONSAUDIO_EAX+54)

#define BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNLEFT1			(BUTTON_MENUOPTIONSAUDIO_EAX+55)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNLEFT2			(BUTTON_MENUOPTIONSAUDIO_EAX+56)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNRIGHT1			(BUTTON_MENUOPTIONSAUDIO_EAX+57)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNRIGHT2			(BUTTON_MENUOPTIONSAUDIO_EAX+58)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKUP1			(BUTTON_MENUOPTIONSAUDIO_EAX+59)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKUP2			(BUTTON_MENUOPTIONSAUDIO_EAX+60)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKDOWN1			(BUTTON_MENUOPTIONSAUDIO_EAX+61)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKDOWN2			(BUTTON_MENUOPTIONSAUDIO_EAX+62)

#define BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFE1			(BUTTON_MENUOPTIONSAUDIO_EAX+63)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFE2			(BUTTON_MENUOPTIONSAUDIO_EAX+64)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_CENTERVIEW1		(BUTTON_MENUOPTIONSAUDIO_EAX+65)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_CENTERVIEW2		(BUTTON_MENUOPTIONSAUDIO_EAX+66)

#define BUTTON_MENUOPTIONS_CONTROLS_CUST_FREELOOK1			(BUTTON_MENUOPTIONSAUDIO_EAX+67)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_FREELOOK2			(BUTTON_MENUOPTIONSAUDIO_EAX+68)

#define BUTTON_MENUOPTIONS_CONTROLS_CUST_PREVIOUS1			(BUTTON_MENUOPTIONSAUDIO_EAX+69)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_PREVIOUS2			(BUTTON_MENUOPTIONSAUDIO_EAX+70)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_NEXT1				(BUTTON_MENUOPTIONSAUDIO_EAX+71)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_NEXT2				(BUTTON_MENUOPTIONSAUDIO_EAX+72)

#define BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCHTOGGLE1		(BUTTON_MENUOPTIONSAUDIO_EAX+73)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCHTOGGLE2		(BUTTON_MENUOPTIONSAUDIO_EAX+74)

#define BUTTON_MENUOPTIONS_CONTROLS_CUST_UNEQUIPWEAPON1		(BUTTON_MENUOPTIONSAUDIO_EAX+75)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_UNEQUIPWEAPON2		(BUTTON_MENUOPTIONSAUDIO_EAX+76)

#define BUTTON_MENUOPTIONS_CONTROLS_CUST_CANCELCURSPELL1	(BUTTON_MENUOPTIONSAUDIO_EAX+77)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_CANCELCURSPELL2	(BUTTON_MENUOPTIONSAUDIO_EAX+78)

#define BUTTON_MENUOPTIONS_CONTROLS_CUST_MINIMAP1			(BUTTON_MENUOPTIONSAUDIO_EAX+79)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_MINIMAP2			(BUTTON_MENUOPTIONSAUDIO_EAX+80)

#define BUTTON_MENUOPTIONS_CONTROLS_CUST_BACK				(BUTTON_MENUOPTIONSAUDIO_EAX+81)
#define BUTTON_MENUOPTIONS_CONTROLS_CUST_DEFAULT			(BUTTON_MENUOPTIONSAUDIO_EAX+82)

#define BUTTON_MENUOPTIONS_CONTROLS_INVERTMOUSE				(1+BUTTON_MENUOPTIONS_CONTROLS_CUST_DEFAULT)
#define BUTTON_MENUOPTIONS_CONTROLS_AUTOREADYWEAPON			(1+BUTTON_MENUOPTIONS_CONTROLS_INVERTMOUSE)
#define BUTTON_MENUOPTIONS_CONTROLS_MOUSELOOK				(1+BUTTON_MENUOPTIONS_CONTROLS_AUTOREADYWEAPON)
#define BUTTON_MENUOPTIONS_CONTROLS_MOUSESENSITIVITY		(1+BUTTON_MENUOPTIONS_CONTROLS_MOUSELOOK)
#define BUTTON_MENUOPTIONS_CONTROLS_AUTODESCRIPTION			(1+BUTTON_MENUOPTIONS_CONTROLS_MOUSESENSITIVITY)
#define BUTTON_MENUOPTIONS_CONTROLS_MOUSE_SMOOTHING			(1+BUTTON_MENUOPTIONS_CONTROLS_AUTODESCRIPTION)

#define BUTTON_MENUEDITQUEST_LOAD_CONFIRM_BACK				(1+BUTTON_MENUOPTIONS_CONTROLS_MOUSE_SMOOTHING)

#define BUTTON_MENUOPTIONS_CONTROLS_BACK					(1+BUTTON_MENUEDITQUEST_LOAD_CONFIRM_BACK)

#define BUTTON_MENUOPTIONS_CONTROLS_LINK					(1+BUTTON_MENUOPTIONS_CONTROLS_BACK)

#define BUTTON_MENUOPTIONSVIDEO_BACK						(1+BUTTON_MENUOPTIONS_CONTROLS_LINK)

#define BUTTON_MENUEDITQUEST_SAVEINFO						(1+BUTTON_MENUOPTIONSVIDEO_BACK)

#define	CONTROLS_CUST_JUMP				0
#define	CONTROLS_CUST_MAGICMODE			1
#define	CONTROLS_CUST_STEALTHMODE		2
#define	CONTROLS_CUST_WALKFORWARD		3
#define	CONTROLS_CUST_WALKBACKWARD		4
#define	CONTROLS_CUST_STRAFELEFT		5
#define	CONTROLS_CUST_STRAFERIGHT		6
#define	CONTROLS_CUST_LEANLEFT			7
#define	CONTROLS_CUST_LEANRIGHT			8
#define	CONTROLS_CUST_CROUCH			9
#define	CONTROLS_CUST_MOUSELOOK			10
#define	CONTROLS_CUST_ACTION			11
#define	CONTROLS_CUST_INVENTORY			12
#define	CONTROLS_CUST_BOOK				13
#define CONTROLS_CUST_BOOKCHARSHEET		14
#define CONTROLS_CUST_BOOKSPELL			15
#define CONTROLS_CUST_BOOKMAP			16
#define CONTROLS_CUST_BOOKQUEST			17
#define	CONTROLS_CUST_DRINKPOTIONLIFE	18
#define	CONTROLS_CUST_DRINKPOTIONMANA	19
#define	CONTROLS_CUST_TORCH				20

#define	CONTROLS_CUST_PRECAST1			21
#define	CONTROLS_CUST_PRECAST2			22
#define	CONTROLS_CUST_PRECAST3			23
#define	CONTROLS_CUST_WEAPON			24
#define	CONTROLS_CUST_QUICKLOAD			25
#define	CONTROLS_CUST_QUICKSAVE			26

#define	CONTROLS_CUST_TURNLEFT			27
#define	CONTROLS_CUST_TURNRIGHT			28
#define	CONTROLS_CUST_LOOKUP			29
#define	CONTROLS_CUST_LOOKDOWN			30

#define	CONTROLS_CUST_STRAFE			31
#define	CONTROLS_CUST_CENTERVIEW		32

#define	CONTROLS_CUST_FREELOOK			33

#define	CONTROLS_CUST_PREVIOUS			34
#define	CONTROLS_CUST_NEXT				35

#define	CONTROLS_CUST_CROUCHTOGGLE		36

#define	CONTROLS_CUST_UNEQUIPWEAPON		37

#define	CONTROLS_CUST_CANCELCURSPELL	38

#define	CONTROLS_CUST_MINIMAP			39

#define MAX_ACTION_KEY					40

//-----------------------------------------------------------------------------

#define DIK_BUTTON1		(0x80000000|DXI_BUTTON0)
#define DIK_BUTTON2		(0x80000000|DXI_BUTTON1)
#define DIK_BUTTON3		(0x80000000|DXI_BUTTON2)
#define DIK_BUTTON4		(0x80000000|DXI_BUTTON3)
#define DIK_BUTTON5		(0x80000000|DXI_BUTTON4)
#define DIK_BUTTON6		(0x80000000|DXI_BUTTON5)
#define DIK_BUTTON7		(0x80000000|DXI_BUTTON6)
#define DIK_BUTTON8		(0x80000000|DXI_BUTTON7)
#define DIK_BUTTON9		(0x80000000|DXI_BUTTON8)
#define DIK_BUTTON10	(0x80000000|DXI_BUTTON9)
#define DIK_BUTTON11	(0x80000000|DXI_BUTTON10)
#define DIK_BUTTON12	(0x80000000|DXI_BUTTON11)
#define DIK_BUTTON13	(0x80000000|DXI_BUTTON12)
#define DIK_BUTTON14	(0x80000000|DXI_BUTTON13)
#define DIK_BUTTON15	(0x80000000|DXI_BUTTON14)
#define DIK_BUTTON16	(0x80000000|DXI_BUTTON15)
#define DIK_BUTTON17	(0x80000000|DXI_BUTTON16)
#define DIK_BUTTON18	(0x80000000|DXI_BUTTON17)
#define DIK_BUTTON19	(0x80000000|DXI_BUTTON18)
#define DIK_BUTTON20	(0x80000000|DXI_BUTTON19)
#define DIK_BUTTON21	(0x80000000|DXI_BUTTON20)
#define DIK_BUTTON22	(0x80000000|DXI_BUTTON21)
#define DIK_BUTTON23	(0x80000000|DXI_BUTTON22)
#define DIK_BUTTON24	(0x80000000|DXI_BUTTON23)
#define DIK_BUTTON25	(0x80000000|DXI_BUTTON24)
#define DIK_BUTTON26	(0x80000000|DXI_BUTTON25)
#define DIK_BUTTON27	(0x80000000|DXI_BUTTON26)
#define DIK_BUTTON28	(0x80000000|DXI_BUTTON27)
#define DIK_BUTTON29	(0x80000000|DXI_BUTTON28)
#define DIK_BUTTON30	(0x80000000|DXI_BUTTON29)
#define DIK_BUTTON31	(0x80000000|DXI_BUTTON30)
#define DIK_BUTTON32	(0x80000000|DXI_BUTTON31)
#define ARX_MAXBUTTON	32

#define DIK_WHEELUP		(0x40000000|0)
#define DIK_WHEELDOWN	(0x40000000|1)

typedef enum _MENUSTATE
{
	MAIN,
	RESUME_GAME,
	NEW_QUEST,
	NEW_QUEST_ENTER_GAME,
	EDIT_QUEST,
	EDIT_QUEST_LOAD,
	EDIT_QUEST_LOAD_CONFIRM,
	EDIT_QUEST_SAVE,
	EDIT_QUEST_SAVE_CONFIRM,
	EDIT_QUEST_DELETE,
	EDIT_QUEST_DELETE_CONFIRM,
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
} MENUSTATE;

//-----------------------------------------------------------------------------
class CMenuZone
{
	public:
		bool	bActif;
		bool	bCheck;
		bool	bTestYDouble;
		int		iId;
		RECT	rZone;
		int			iID;
		long		lData;
		long	*	pData;
		long		lPosition;
	public:
		CMenuZone();
		CMenuZone(int, int, int, int, int);
		virtual ~CMenuZone();

		int GetWidth()
		{
			return (rZone.right - rZone.left);
		}
		int GetHeight()
		{
			return (rZone.bottom - rZone.top);
		}
		virtual void Move(int, int);
		virtual void SetPos(float, float); 
 
		void SetCheckOff()
		{
			bCheck = false;
		};
		void SetCheckOn()
		{
			bCheck = true;
		};
 
		virtual long IsMouseOver(int, int);
};

//-----------------------------------------------------------------------------
class CMenuAllZone
{
	public:
		vector<CMenuZone *>	vMenuZone;
	public:
		CMenuAllZone();
		virtual ~CMenuAllZone();

		void AddZone(CMenuZone *);
		int CheckZone(int, int);
 
		CMenuZone * GetZoneNum(int);
		CMenuZone * GetZoneWithID(int);
		void Move(int, int);
		void DrawZone();
		int GetNbZone();
};

typedef enum _ELEMSTATE
{
	TNOP,
	//Element Text
	EDIT,				//type d'etat
	GETTOUCH,
	EDIT_TIME,			//etat en cours
	GETTOUCH_TIME
} ELEMSTATE;

typedef enum _ELEMPOS
{
	NOCENTER,
	CENTER,
	CENTERY
} ELEMPOS;

//-----------------------------------------------------------------------------
class CMenuElement : public CMenuZone
{
	public:
		ELEMPOS		ePlace;			//placement de la zone
		ELEMSTATE	eState;			//etat de l'element en cours
		MENUSTATE	eMenuState;		//etat de retour de l'element
		int			iShortCut;
	public:
		CMenuElement(MENUSTATE);
		virtual ~CMenuElement();

		virtual CMenuElement * OnShortCut();
		virtual bool OnMouseClick(int) = 0;
		virtual void Update(int) = 0;
		virtual void Render() = 0;
		virtual void RenderMouseOver() {};
		virtual void EmptyFunction() {};
		virtual bool OnMouseDoubleClick(int)
		{
			return false;
		};
		virtual CMenuZone * GetZoneWithID(int _iID)
		{
			if (iID == _iID)
			{
				return (CMenuZone *)this;
			}
			else
			{
				return NULL;
			}
		};
 
		void SetShortCut(int _iShortCut)
		{
			iShortCut = _iShortCut;
		};
};

//-----------------------------------------------------------------------------
// faire une classe
// like a container in java

//-----------------------------------------------------------------------------
class CMenuPanel : public CMenuElement
{
	public:
		vector<CMenuElement *>	vElement;
	public:
		CMenuPanel();
		virtual ~CMenuPanel();

		void Move(int, int);
		void AddElement(CMenuElement *);
		void AddElementNoCenterIn(CMenuElement *);
 
		void Update(int);
		void Render();
		bool OnMouseClick(int)
		{
			return false;
		};
		CMenuElement * OnShortCut();
		void RenderMouseOver() {};
		long IsMouseOver(int, int);
		CMenuZone * GetZoneWithID(int);
};

//-----------------------------------------------------------------------------
class CMenuElementText: public CMenuElement
{
	public:
		_TCHAR	* lpszText;
		HFONT	pHFont;
		long	lColor;
		long	lOldColor;
		long	lColorHighlight;
		float	fSize;
		bool	bSelected;
		int		iPosCursor;

	public:

		CMenuElementText(int, HFONT, _TCHAR *, float, float, long, float, MENUSTATE); 
		virtual ~CMenuElementText();

		CMenuElement * OnShortCut();
		bool OnMouseClick(int);
		void Update(int);
		void Render();
		void SetText(_TCHAR * _pText);
		void RenderMouseOver();
 
 
 
 
		bool OnMouseDoubleClick(int);
};

//-----------------------------------------------------------------------------
class CMenuButton: public CMenuElement
{
	public:
		vector<_TCHAR *>		vText;
		int					iPos;
		TextureContainer	* pTex;
		TextureContainer	* pTexOver;
		HFONT				pHFont;
		int					iColor;
		float				fSize;

	public:
		CMenuButton(int, HFONT, MENUSTATE, int, int, _TCHAR *, float _fSize = 1.f, TextureContainer * _pTex = NULL, TextureContainer * _pTexOver = NULL, int _iColor = -1, int _iTailleX = 0, int _iTailleY = 0);
		~CMenuButton();

	public:
		void SetPos(int, int);
		void AddText(_TCHAR *);
		CMenuElement * OnShortCut()
		{
			return NULL;
		};
		bool OnMouseClick(int);
		void Update(int);
		void Render();
		void RenderMouseOver();
};

//-----------------------------------------------------------------------------
class CMenuSliderText: public CMenuElement
{
	public:
		CMenuButton		*	pLeftButton;
		CMenuButton		*	pRightButton;
		vector<CMenuElementText *>	vText;
		int					iPos;
		int					iOldPos;
	public:
		CMenuSliderText(int, int, int);
		virtual ~CMenuSliderText();

	public:
		void AddText(CMenuElementText *);

	public:
		void Move(int, int);
		bool OnMouseClick(int);
		CMenuElement * OnShortCut()
		{
			return NULL;
		};
		void Update(int);
		void Render();
		void RenderMouseOver();
		void EmptyFunction();
		void SetWidth(int);
};

//-----------------------------------------------------------------------------
class CMenuSlider: public CMenuElement
{
	public:
		CMenuButton		*	pLeftButton;
		CMenuButton		*	pRightButton;
		TextureContainer	* pTex1;
		TextureContainer	* pTex2;
		int					iPos;

	public:
		CMenuSlider(int, int, int);//, CMenuButton*, CMenuButton*, TextureContainer*, TextureContainer*);
		virtual ~CMenuSlider();

	public:
		void Move(int, int);
		bool OnMouseClick(int);
		CMenuElement * OnShortCut()
		{
			return NULL;
		};
		void Update(int);
		void Render();
		void RenderMouseOver();
		void EmptyFunction();
};

//-----------------------------------------------------------------------------
class CMenuCheckButton : public CMenuElement
{
	public:
		int					iState;
		int					iOldState;
		int					iPosX;
		int					iPosY;
		int					iTaille;
		CMenuAllZone	*	pAllCheckZone;
		vector<TextureContainer *> vTex;
		CMenuElementText	* pText;

	public:
		CMenuCheckButton(int, float, float, int, TextureContainer *, TextureContainer *, CMenuElementText * _pText = NULL); 
		virtual ~CMenuCheckButton();

 
		void Move(int, int);
		bool OnMouseClick(int);
		void Update(int);
		void Render();
		void RenderMouseOver();
};

//-----------------------------------------------------------------------------

class CMenuState
{
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
		CMenuState(MENUSTATE);
		virtual ~CMenuState();

		void AddMenuElement(CMenuElement *);
		MENUSTATE Update(int);
		void Render();
};

//-----------------------------------------------------------------------------
class CWindowMenuConsole
{
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
		CWindowMenuConsole(int, int, int, int, MENUSTATE);
 

		void AddMenu(CMenuElement *);
		void AddMenuCenter(CMenuElement *);
		void AddMenuCenterY(CMenuElement *);
		void AlignElementCenter(CMenuElement *);
		MENUSTATE Update(int, int, int, int);
		int Render();
 
		CMenuElement * GetTouch(bool _bValidateTest = false);
};

//-----------------------------------------------------------------------------
class CWindowMenu
{
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
		vector<CWindowMenuConsole *>	vWindowConsoleElement;
		float				fPosXCalc;
		float				fDist;
		float				fAngle;
		MENUSTATE			eCurrentMenuState;
		bool				bChangeConsole;
	public:
		CWindowMenu(int, int, int, int, int);
		virtual ~CWindowMenu();

		void AddConsole(CWindowMenuConsole *);
		void Update(int);
		MENUSTATE Render();
};

//-----------------------------------------------------------------------------
typedef enum _CURSORSTATE
{
	CURSOR_OFF,
	CURSOR_ON,
} CURSORSTATE;

//-----------------------------------------------------------------------------
typedef struct
{
	int	x;
	int	y;
} EERIE_2DI;

//-----------------------------------------------------------------------------
class CDirectInput
{
	public:
		bool				bActive;
		bool				bTouch;
		int					iKeyId;
		int					iKeyScanCode[256];
		int					iOneTouch[256];
		bool				bMouseMove;
		int					iMouseRX;
		int					iMouseRY;
		int					iMouseRZ;
		int					iMouseAX;
		int					iMouseAY;
		int					iMouseAZ;
		float				fMouseAXTemp;
		float				fMouseAYTemp;
		int					iSensibility;
		int					iOldMouseButton[ARX_MAXBUTTON];
		bool				bMouseButton[ARX_MAXBUTTON];
		bool				bOldMouseButton[ARX_MAXBUTTON];
		int					iMouseTime[ARX_MAXBUTTON];
		int					iMouseTimeSet[ARX_MAXBUTTON];
		int					iNbOldCoord;
		int					iMaxOldCoord;
		EERIE_2DI			iOldCoord[256];

		TextureContainer	* pTex[8];
		long				lFrameDiff;
		CURSORSTATE			eNumTex;
		int					iNumCursor;
		float				fTailleX;
		float				fTailleY;
		bool				bMouseOver;
		bool				bDrawCursor;

		int					iOldNumClick[ARX_MAXBUTTON];
		int					iOldNumUnClick[ARX_MAXBUTTON];

		int					iWheelSens;
	private:
		void DrawOneCursor(int, int, int);
 
	public:
		CDirectInput();
		virtual ~CDirectInput();

		void SetMouseOver();
		void SetCursorOn();
		void SetCursorOff();
		void SetSensibility(int);
		void GetInput();
		void DrawCursor();
		bool GetMouseButton(int);
		bool GetMouseButtonRepeat(int);
		bool GetMouseButtonNowPressed(int);
		bool GetMouseButtonNowUnPressed(int);
		bool GetMouseButtonDoubleClick(int, int);
 
		bool IsVirtualKeyPressed(int);
		bool IsVirtualKeyPressedOneTouch(int);
		bool IsVirtualKeyPressedNowPressed(int);
		bool IsVirtualKeyPressedNowUnPressed(int);
		_TCHAR * GetFullNameTouch(int);
 
 
		void ResetAll();
		int GetWheelSens(int);
};

//-----------------------------------------------------------------------------
typedef struct
{
	int	iKey[2];
	int iPage;
} SACTION_KEY;

class CMenuConfig
{
	public:
		char	*	pcName;
		//LANGUAGE
		//VIDEO
		int			iWidth;
		int			iHeight;
		int			iNewWidth;
		int			iNewHeight;
		int			iBpp;
		int			iNewBpp;
		bool		bFullScreen;
		bool		bBumpMapping;
		bool		bNewBumpMapping;
		bool		bMouseSmoothing;
		int			iTextureResol;
		int			iNewTextureResol;
		int			iMeshReduction;
		int			iLevelOfDetails;
		int			iFogDistance;
		int			iGamma;
		int			iLuminosite;
		int			iContrast;
		bool		bShowCrossHair;
		bool		bAntiAliasing;
		bool		bChangeResolution;
		bool		bChangeTextures;
		bool		bDebugSetting;
		//AUDIO
		int			iMasterVolume;
		int			iSFXVolume;
		int			iSpeechVolume;
		int			iAmbianceVolume;
		bool		bEAX;
		//INPUT
		bool		bInvertMouse;
		bool		bAutoReadyWeapon;
		bool		bMouseLookToggle;
		bool		bAutoDescription;
		int			iMouseSensitivity;
		SACTION_KEY sakActionKey[MAX_ACTION_KEY];
		SACTION_KEY sakActionDefaultKey[MAX_ACTION_KEY];

		bool		bLinkMouseLookToUse;
		//MISC
		bool		bATI;
		bool		bForceMetalTwoPass;
		bool		bForceZBias;
		bool		bOneHanded;
		unsigned int uiGoreMode;

		bool		bNoReturnToWindows;
	private:
		int GetDIKWithASCII(char * _pcTouch);
		char * ReadConfig(char * _pcSection, char * _pcKey);
		bool WriteConfig(char * _pcSection, char * _pcKey, char * _pcDatas);
	public:
		CMenuConfig();
		CMenuConfig(char *);
		virtual ~CMenuConfig();

		bool SetActionKey(int _iAction, int _iActionNum, int _iVirtualKey);
		int ReadConfigInt(char * _pcSection, char * _pcKey, bool & _bOk);
 
		char * ReadConfigString(char * _pcSection, char * _pcKey);
		bool WriteConfigInt(char * _pcSection, char * _pcKey, int _iDatas);
 
		bool WriteConfigString(char * _pcSection, char * _pcKey, char * _pcDatas);
 
 
 
		void ResetActionKey();
		bool WriteConfigKey(char * _pcKey, int _iAction);
		bool ReadConfigKey(char * _pcKey, int _iAction);
		void ReInitActionKey(CWindowMenuConsole * _pwmcWindowMenuConsole);
		void SetDefaultKey();
		void DefaultValue();
		void First();

		bool SaveAll();
		bool ReadAll();
};

//-----------------------------------------------------------------------------
typedef struct CreditsTextInformations
{
	CreditsTextInformations()
	{
		sPos.cx = 0 ;
		sPos.cy = 0 ;
		fColors = 0 ;
	}

	wstring  sText ;
	COLORREF fColors ;
	SIZE sPos;
} CreditsTextInformations;


typedef struct CreditsInformations
{
  CreditsInformations()
  {
	iFontAverageHeight = -1;
	iFirstLine = 0 ;
  }

  int iFirstLine ;
  int iFontAverageHeight ;
  std::vector<CreditsTextInformations> aCreditsInformations ;
} CreditsInformations;


static CreditsInformations CreditsData ;

static void InitCredits(void);
static void CalculAverageWidth(HDC& _hDC) ;
static void ExtractAllCreditsTextInformations();
static void ExtractPhraseColor( wstring &phrase, CreditsTextInformations &infomations );
static void CalculTextPosition( HDC& _hDC, wstring& phrase, CreditsTextInformations &infomations, float& drawpos );


//-----------------------------------------------------------------------------
bool Menu2_Render();
void Menu2_Close();

bool ProcessFadeInOut(bool _bFadeIn, float _fspeed);

//-----------------------------------------------------------------------------
void ARX_MENU_Clicked_NEWQUEST();
void ARX_MENU_Clicked_QUIT();
void ARX_MENU_Clicked_CREDITS();

bool ARX_QuickLoad();
void ARX_QuickSave();
void ARX_DrawAfterQuickLoad();

#endif
