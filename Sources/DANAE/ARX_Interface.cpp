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
///////////////////////////////////////////////////////////////////////////////
//
// ARX_Interface.cpp
// ARX Interface Management
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//
///////////////////////////////////////////////////////////////////////////////

#include "ARX_Interface.h"
#include "ARX_Minimap.h"
#include "ARX_Paths.h"
#include "ARX_Text.h"
#include "ARX_Draw.h"
#include "ARX_Menu.h"
#include "ARX_INPUT.h"
#include "ARX_Equipment.h"
#include "ARX_Sound.h"
#include "ARX_Spells.h"
#include "ARX_Speech.h" 
#include "ARX_Levels.h"
#include "ARX_ChangeLevel.h"
#include "ARX_Particles.h"
#include "ARX_Damages.h"
#include "ARX_NPC.h"
#include "ARX_Menu.h"
#include "ARX_Menu2.h"
#include "ARX_Interactive.h"
#include "ARX_speech.h"
#include "../danae/Danae_resource.h"
#include "ARX_Text.h"
#include "ARX_Time.h"
#include "DanaeDlg.h"

#include "EERIEAnim.h"
#include "EERIELight.h"
#include "EERIELinkedObj.h"
#include "EERIEPhysicsBox.h"
#include "EERIEDRAW.h"
#include "EERIEObject.h"
#include "EERIETexture.h"
#include "ARX_collisions.h"
#include "ARX_C_Cinematique.h"
#include "Hermesmain.h"
#include "resource.h"

#include <stdio.h>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
extern float MagicSightFader;
extern long FINAL_COMMERCIAL_DEMO;
extern long NEED_TEST_TEXT;
extern float Original_framedelay;
extern long FINAL_COMMERCIAL_GAME;
extern EERIE_3DOBJ *arrowobj;
extern bool bGLOBAL_DINPUT_GAME;
extern void InitTileLights();
extern long USE_LIGHT_OPTIM;

long Manage3DCursor(long flags); // flags & 1 == simulation
long IN_BOOK_DRAW=0;
int TSU_TEST_COLLISIONS = 0;
extern float INVISIBILITY_OVERRIDE;
//-----------------------------------------------------------------------------
typedef struct
{
	INTERACTIVE_OBJ  * io;
	TextureContainer * tc;
	TextureContainer * tc2;
	float POSX;
	float POSY;
	float fRatioX;
	float fRatioY;
} ARX_INTERFACE_HALO_STRUCT;
//-----------------------------------------------------------------------------
#define GL_DECAL_ICONS		0
#define BOOKMARKS_POS_X		216.f
#define BOOKMARKS_POS_Y		60.f
#define PAGE_CHAR_SIZE		4096

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
extern TextureContainer * iconequip[];
extern TextureContainer * Movable;
extern TextureContainer * ChangeLevel;
extern TextureContainer * inventory_font;
extern TextureContainer * scursor[];
extern TextureContainer * pTCCrossHair;
extern TextureContainer * mecanism_tc;
extern TextureContainer * arrow_left_tc;
extern FOG_DEF fogparam;
extern CDirectInput *pGetInfoDirectInput;
extern CMenuConfig *pMenuConfig;
extern D3DTLVERTEX LATERDRAWHALO[];
extern EERIE_LIGHT lightparam;
extern INTERACTIVE_OBJ * CURRENT_TORCH;
extern STRUCT_SPEECH speech[];
extern _TCHAR WILLADDSPEECH[];
extern float PLAYER_ROTATION;
extern float SLID_VALUE;

extern long WILLLOADLEVEL;	// Is a LoadLevel command waiting ?
extern long WILLSAVELEVEL;	// Is a SaveLevel command waiting ?
extern long BOOKBUTTON;
extern long LASTBOOKBUTTON;
extern long FORCE_NO_HIDE;
extern long GAME_EDITOR;
extern long LastSelectedIONum;
extern long lSLID_VALUE;

extern long CHANGE_LEVEL_ICON;
extern long NO_TEXT_AT_ALL;
extern long BLOCK_PLAYER_CONTROLS;
extern long DeadTime;
extern long MOULINEX;
extern long HALOCUR;
extern long USE_CEDRIC_ANIM;
extern long ALLOW_CHEATS;
extern long LOOKING_FOR_SPELL_TARGET;
extern long WILLRETURNTOFREELOOK;
extern float BOW_FOCAL;
extern EERIE_S2D DANAEMouse;
extern short sActiveInventory;
extern unsigned long WILLADDSPEECHTIME;
extern unsigned long LOOKING_FOR_SPELL_TARGET_TIME;
extern ANIM_HANDLE * herowait_2h;

extern float ARXTimeMenu;
extern float ARXOldTimeMenu;
extern float ARXDiffTimeMenu;

extern CINEMATIQUE *ControlCinematique;
extern bool bRenderInterList;
extern bool bGToggleCombatModeWithKey;
extern long PlayerWeaponBlocked;
extern unsigned char ucFlick;

extern bool bGATI8500;
extern bool bSoftRender;

extern CARXTextManager *pTextManageFlyingOver;

BOOL IsPlayerStriking();
void OptmizeInventory(unsigned int);
long ARX_SPELLS_GetInstance(const long &);
void ARX_SPELLS_Kill(const long &);

extern long SHOW_INGAME_MINIMAP;

//-----------------------------------------------------------------------------
TextureContainer *	NoteTexture=NULL;
TextureContainer *	NoteTextureLeft=NULL;
TextureContainer *	NoteTextureRight=NULL;
TextureContainer *	BasicInventorySkin=NULL;
TextureContainer *	ThrowObject=NULL;
ARX_INTERFACE_HALO_STRUCT * aiHalo=NULL;
D3DCOLOR			BOOKINTERFACEITEMCOLOR=D3DCOLORWHITE;
E_ARX_STATE_MOUSE	eMouseState;
EERIE_S2D			bookclick;
EERIE_S2D			MemoMouse;
EERIE_LIGHT		 *	CDP_EditLight=NULL;
FOG_DEF			 *	CDP_EditFog=NULL;
HWND				CDP_PATHWAYS_Options=NULL;
HWND				CDP_FogOptions=NULL;
 
HWND				CDP_LIGHTOptions=NULL;
HWND				CDP_IOOptions=NULL;
INVENTORY_DATA *	TSecondaryInventory;
INTERACTIVE_OBJ *	CDP_EditIO=NULL;
INTERACTIVE_OBJ *	FlyingOverIO=NULL;
INTERACTIVE_OBJ *	STARTED_ACTION_ON_IO=NULL;
INTERFACE_TC		ITC;
STRUCT_NOTE			Note;
STRUCT_NOTE			QuestBook;
_TCHAR				Page_Buffer[PAGE_CHAR_SIZE+1];
bool				bBookHalo = false;
bool				bGoldHalo = false;
bool				bHitFlash = false;
bool				bInventoryClosing = false;
bool				bInventorySwitch = false;
bool				bIsAiming = false;
float				fHitFlash = 0;
unsigned long		ulHitFlash = 0;
unsigned long		ulBookHaloTime = 0;
unsigned long		ulGoldHaloTime = 0;
float				NotePosX=0;
float				NotePosY=0;
float				NoteTextMinx;
float				NoteTextMiny;
float				NoteTextMaxx;
float				NoteTextMaxy;
float				InventoryX=-60.f;
float				InventoryDir=0; // 0 stable, 1 to right, -1 to left
 
float				CINEMA_DECAL=0.f;
float				SLID_START=0.f; // Charging Weapon
float				BOOKDECX=0.f;
float				BOOKDECY=0.f;
float				PROGRESS_BAR_TOTAL=0;
float				PROGRESS_BAR_COUNT=0;
float				OLD_PROGRESS_BAR_COUNT=0;

long				TRUE_PLAYER_MOUSELOOK_ON=0;
long				LAST_PLAYER_MOUSELOOK_ON=0;
long				MEMO_PLAYER_MOUSELOOK_ON=0;
long				COMBINEGOLD=0;
long				PLAYER_MOUSELOOK_ON=0;
long				Book_Mode=0;
long				Book_MapPage=1;
long				Book_SpellPage=1;
long				CINEMASCOPE=0;
float				g_TimeStartCinemascope = 0;
long				CINEMA_INC=0;
long				SMOOTHSLID=0;
long				currpos=50;
long				DRAGGING = 0;
long				INVERTMOUSE=0; 
long				PLAYER_INTERFACE_HIDE_COUNT=0;
long				MAGICMODE=-1;
long				SpecialCursor=0;
long				FLYING_OVER		= 0;
long				OLD_FLYING_OVER	= 0;
long				LastRune=-1;
long				BOOKZOOM=0;
EERIE_3D			ePlayerAngle;
long				CURCURTIME=0;
long				CURCURDELAY=70;
long				CURCURPOS=0;
long				INTERFACE_HALO_NB=0; 
long				INTERFACE_HALO_MAX_NB=0; 
long				ObjectRotAxis=0;
long				SPECIAL_DRAW_INTER_SHADOW=0;
long				PRECAST_NUM=0;
long				LastMouseClick=0;
long				LastSelectedNode=-1;
long				LastSelectedFog=-1;
long				LastSelectedLight=-1;
long				MOVETYPE=MOVE_WAIT;
 
//used to redist points - attributes and skill
long				lCursorRedistValue = 0;
long				lFadeMapTime = 0;
float				fInterfaceRatio = 1;

unsigned long		COMBAT_MODE_ON_START_TIME = 0;
long SPECIAL_DRAW_WEAPON=0;
bool bGCroucheToggle=false;

int iHighLight=0;
float fHighLightAng=0.f;
long INTERNATIONAL_MODE=1;

float INTERFACE_RATIO(float a)
{
	return a * fInterfaceRatio;
}
float INTERFACE_RATIO_LONG(const long a)
{
	return INTERFACE_RATIO(ARX_CLEAN_WARN_CAST_FLOAT(a));
}
float INTERFACE_RATIO_DWORD(const DWORD a)
{
	return INTERFACE_RATIO(ARX_CLEAN_WARN_CAST_FLOAT(a));
}


short SHORT_INTERFACE_RATIO(const float _a) 
{
	float fRes = INTERFACE_RATIO(_a);
	ARX_CHECK_SHORT(fRes);
	return  ARX_CLEAN_WARN_CAST_SHORT(fRes);
}


bool bInverseInventory=false;
long lOldTruePlayerMouseLook=TRUE_PLAYER_MOUSELOOK_ON;
bool bForceEscapeFreeLook=false;
bool bRenderInCursorMode=true;

long lChangeWeapon=0;
INTERACTIVE_OBJ *pIOChangeWeapon=NULL;

static long lTimeToDrawMecanismCursor=0;
static long lNbToDrawMecanismCursor=0;
static long lOldInterface;


//-----------------------------------------------------------------------------
float ARX_CAST_TO_INT_THEN_FLOAT( float _f )
{
	return ( ( _f >= 0 ) ? floor( _f ) : ceil( _f ) ); 
}


//-----------------------------------------------------------------------------
BOOL MouseInBookRect(const float x, const float y, const float cx, const float cy)
{
	return ((DANAEMouse.x>=(x+BOOKDECX)*Xratio) 
		&& (DANAEMouse.x<=(cx+BOOKDECX)*Xratio) 
		&& (DANAEMouse.y>=(y+BOOKDECY)*Yratio) 
	        && (DANAEMouse.y <= (cy + BOOKDECY) * Yratio)); 
}

//-----------------------------------------------------------------------------
BOOL MouseInCam(EERIE_CAMERA * cam)
{
	return ((DANAEMouse.x>cam->clip.left) && (DANAEMouse.x<cam->clip.right) &&
		(DANAEMouse.x>cam->clip.top) && (DANAEMouse.y<cam->clip.bottom));
}

//-----------------------------------------------------------------------------
BOOL MouseInRect(const float x0, const float y0, const float x1=32, const float y1=32)
{
	return (	(DANAEMouse.x>=x0) 
		&&	(DANAEMouse.x<=x1) 
		&&	(DANAEMouse.y>=y0) 
		&&	(DANAEMouse.y<=y1) );
}

//-----------------------------------------------------------------------------
BOOL ARX_INTERFACE_MouseInBook()
{		
	if ((player.Interface & INTER_MAP ) &&  (!(player.Interface & INTER_COMBATMODE))) 
		return (MouseInBookRect(99,65,599,372));

	else return FALSE;
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_DrawItem(TextureContainer *tc, const float x, const float y, const float z, const D3DCOLOR col)
{
	if ((tc) && (tc->m_pddsSurface))
		EERIEDrawBitmap(GDevice,
		x, y,
		                INTERFACE_RATIO_DWORD(tc->m_dwWidth), INTERFACE_RATIO_DWORD(tc->m_dwHeight),
		z,
		tc,
		col);
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_DrawNumber(const float x, const float y, const long num, const int _iNb, const D3DCOLOR col)
{
	D3DTLVERTEX v[4];
	v[0]= D3DTLVERTEX( D3DVECTOR( 0, 0, 0.f ), 1.f, 1, 1, 0.f, 0.f);
	v[1]= D3DTLVERTEX( D3DVECTOR( 0, 0, 0.f ), 1.f, 1, 1, 1.f, 0.f);
	v[2]= D3DTLVERTEX( D3DVECTOR( 0, 0, 0.f ), 1.f, 1, 1, 1.f, 1.f);
	v[3]= D3DTLVERTEX( D3DVECTOR( 0, 0, 0.f ), 1.f, 1, 1, 0.f, 1.f);
	
	char format[32];

	sprintf(format, "%%0%dd", _iNb);

	v[0].sz = v[1].sz = v[2].sz = v[3].sz = 0.0000001f;

	if (inventory_font)
	{
		char tx[7];
		long tt;
		float ttx;
		float divideX = 1.f/((float) inventory_font->m_dwWidth);
		float divideY = 1.f/((float) inventory_font->m_dwHeight);

		sprintf(tx, format, num);
		long removezero=1;

		for (long i=0;i<6;i++)
		{
			
			tt=tx[i]-'0';

			if ((tt==0) && removezero) continue;

			if (tt>=0)
			{
				removezero=0;
				v[0].sx=v[3].sx=x + i*INTERFACE_RATIO(10);
				v[1].sx = v[2].sx = v[0].sx + INTERFACE_RATIO(10); 
				v[0].sy=v[1].sy=y;
				v[2].sy=v[3].sy=y + INTERFACE_RATIO(10);
				v[0].color=v[1].color=v[2].color=v[3].color=col;		
				
				ttx = (float)((float)tt * (float)11.f) + 1.5f; 
				v[3].tu=v[0].tu=ttx*divideX;
				v[1].tu=v[2].tu=(ttx+10.f)*divideX;
				
				ttx=0.5f*divideY;
				v[1].tv = v[0].tv = divideY + ttx;
				v[2].tv = v[3].tv = divideY * 12; 
				SETTC(GDevice,inventory_font);
		
				EERIEDRAWPRIM(GDevice,D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE| D3DFVF_SPECULAR ,v, 4, 0 );	
				
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Interface Texture Containers creation
//-----------------------------------------------------------------------------
void CreateInterfaceTextureContainers()
{
	char temp[512];
	memset(&ITC,0,sizeof(INTERFACE_TC));
	MakeDir(temp,"Graph\\Interface\\bars\\aim_empty.bmp");
	ITC.aim_empty = MakeTCFromFile(temp, 0);
	MakeDir(temp,"Graph\\Interface\\bars\\aim_maxi.bmp");
	ITC.aim_maxi = MakeTCFromFile(temp, 0);
	MakeDir(temp,"Graph\\Interface\\bars\\flash_gauge.bmp");
	ITC.aim_hit = MakeTCFromFile(temp, 0);
	MakeDir(temp,"Graph\\Interface\\Inventory\\hero_inventory.bmp");
	ITC.hero_inventory = MakeTCFromFile(temp, 0);
	MakeDir(temp,"Graph\\Interface\\Inventory\\Ingame_inventory.bmp");
	BasicInventorySkin = MakeTCFromFile(temp, 0);
	MakeDir(temp,"Graph\\Interface\\Inventory\\scroll_up.bmp");
	ITC.hero_inventory_up = MakeTCFromFile(temp, 0);
	MakeDir(temp,"Graph\\Interface\\Inventory\\scroll_down.bmp");
	ITC.hero_inventory_down = MakeTCFromFile(temp, 0);
	MakeDir(temp,"Graph\\Interface\\Inventory\\Hero_inventory_link.bmp");
	ITC.hero_inventory_link = MakeTCFromFile(temp, 0);

	MakeDir(temp,"Graph\\Interface\\Inventory\\inv_pick.bmp");
	ITC.inventory_pickall = MakeTCFromFile(temp, 0);
	MakeDir(temp,"Graph\\Interface\\Inventory\\inv_close.bmp");
	ITC.inventory_close = MakeTCFromFile(temp, 0);
	
	MakeDir(temp,"Graph\\Interface\\Icons\\lvl_up.bmp");
	ITC.Icon_Lvl_Up = MakeTCFromFile(temp, 0);
	ITC.ingame_sub_inv = NULL; 
	MakeDir(temp,"Graph\\Interface\\Icons\\Backpack.bmp");
	ITC.backpack=D3DTextr_GetSurfaceContainer(temp);
	MakeDir(temp,"Graph\\Interface\\Inventory\\Gold.bmp");
	ITC.gold=D3DTextr_GetSurfaceContainer(temp);
	MakeDir(temp,"Graph\\Interface\\Icons\\Book.bmp");
	ITC.book=D3DTextr_GetSurfaceContainer(temp);
	MakeDir(temp,"Graph\\Interface\\Icons\\Steal.bmp");
	ITC.steal=D3DTextr_GetSurfaceContainer(temp);
	MakeDir(temp,"Graph\\Interface\\Icons\\cant_steal_item.bmp");
	ITC.item_cant_steal=D3DTextr_GetSurfaceContainer(temp);
	MakeDir(temp,"Graph\\interface\\bars\\Empty_gauge_Red.bmp");
	ITC.empty_gauge_red=D3DTextr_GetSurfaceContainer(temp);
	MakeDir(temp,"Graph\\interface\\bars\\Empty_gauge_Blue.bmp");
	ITC.empty_gauge_blue=D3DTextr_GetSurfaceContainer(temp);
	MakeDir(temp,"Graph\\interface\\bars\\Filled_gauge_Red.bmp");
	ITC.filled_gauge_red=D3DTextr_GetSurfaceContainer(temp);
	MakeDir(temp,"Graph\\interface\\bars\\Filled_gauge_Blue.bmp");
	ITC.filled_gauge_blue=D3DTextr_GetSurfaceContainer(temp);
	MakeDir(temp,"Graph\\Interface\\cursors\\target_on.bmp");
	ITC.target_on=D3DTextr_GetSurfaceContainer(temp);
	MakeDir(temp,"Graph\\Interface\\cursors\\target_off.bmp");
	ITC.target_off=D3DTextr_GetSurfaceContainer(temp);
	MakeDir(temp,"Graph\\Interface\\cursors\\interaction_on.bmp");
	ITC.interaction_on=D3DTextr_GetSurfaceContainer(temp);
	MakeDir(temp,"Graph\\Interface\\cursors\\interaction_off.bmp");
	ITC.interaction_off=D3DTextr_GetSurfaceContainer(temp);
	MakeDir(temp,"Graph\\Interface\\cursors\\magic.bmp");
	ITC.magic=D3DTextr_GetSurfaceContainer(temp);
	MakeDir(temp,"Graph\\Interface\\cursors\\throw.bmp");
	ThrowObject = D3DTextr_GetSurfaceContainer(temp);
}

//-----------------------------------------------------------------------------

void KillInterfaceTextureContainers()
{
	D3DTextr_KillTexture(ITC.questbook);
	D3DTextr_KillTexture(ITC.bookmark_char);
	D3DTextr_KillTexture(ITC.bookmark_magic);
	D3DTextr_KillTexture(ITC.bookmark_map);
	D3DTextr_KillTexture(ITC.bookmark_quest);
	D3DTextr_KillTexture(ITC.accessible_1);
	D3DTextr_KillTexture(ITC.accessible_2);
	D3DTextr_KillTexture(ITC.accessible_3);
	D3DTextr_KillTexture(ITC.accessible_4);
	D3DTextr_KillTexture(ITC.accessible_5);
	D3DTextr_KillTexture(ITC.accessible_6);
	D3DTextr_KillTexture(ITC.accessible_7);
	D3DTextr_KillTexture(ITC.accessible_8);
	D3DTextr_KillTexture(ITC.accessible_9);
	D3DTextr_KillTexture(ITC.accessible_10);
	D3DTextr_KillTexture(ITC.current_1);
	D3DTextr_KillTexture(ITC.current_2);
	D3DTextr_KillTexture(ITC.current_3);
	D3DTextr_KillTexture(ITC.current_4);
	D3DTextr_KillTexture(ITC.current_5);
	D3DTextr_KillTexture(ITC.current_6);
	D3DTextr_KillTexture(ITC.current_7);
	D3DTextr_KillTexture(ITC.current_8);
	D3DTextr_KillTexture(ITC.current_9);
	D3DTextr_KillTexture(ITC.current_10);
	
	D3DTextr_KillTexture(ITC.heropageleft);
	D3DTextr_KillTexture(ITC.heropageright);
	
	D3DTextr_KillTexture(ITC.symbol_mega);
	D3DTextr_KillTexture(ITC.symbol_vista);
	D3DTextr_KillTexture(ITC.symbol_aam);
	D3DTextr_KillTexture(ITC.symbol_taar);
	D3DTextr_KillTexture(ITC.symbol_yok);
	D3DTextr_KillTexture(ITC.pTexCursorRedist);
	D3DTextr_KillTexture(ITC.pTexSpellBook);
	
	D3DTextr_KillTexture(ITC.pTexCornerLeft);
	D3DTextr_KillTexture(ITC.pTexCornerRight);
	
	if (ITC.lpszULevel)
	{
		free (ITC.lpszULevel);
		ITC.lpszULevel = NULL;
	}

	if (ITC.lpszUXp)
	{
		free (ITC.lpszUXp);
		ITC.lpszUXp = NULL;
	}
	
	ITC.questbook=NULL;
	ITC.bookmark_char=NULL;
	ITC.bookmark_magic=NULL;
	ITC.bookmark_map=NULL;
	ITC.bookmark_quest=NULL;
	ITC.accessible_1=NULL;
	ITC.accessible_2=NULL;
	ITC.accessible_3=NULL;
	ITC.accessible_4=NULL;
	ITC.accessible_5=NULL;
	ITC.accessible_6=NULL;
	ITC.accessible_7=NULL;
	ITC.accessible_8=NULL;
	ITC.accessible_9=NULL;
	ITC.accessible_10=NULL;
	ITC.current_1=NULL;
	ITC.current_2=NULL;
	ITC.current_3=NULL;
	ITC.current_4=NULL;
	ITC.current_5=NULL;
	ITC.current_6=NULL;
	ITC.current_7=NULL;
	ITC.current_8=NULL;
	ITC.current_9=NULL;
	ITC.current_10=NULL;
	
	ITC.heropageleft=NULL;
	ITC.heropageright=NULL;
	
	ITC.symbol_mega=NULL;
	ITC.symbol_vista=NULL;
	ITC.symbol_aam=NULL;
	ITC.symbol_taar=NULL;
	ITC.symbol_yok=NULL;
	ITC.pTexCursorRedist=NULL;
	ITC.pTexSpellBook=NULL;
	ITC.pTexCornerLeft=NULL;
	ITC.pTexCornerRight=NULL;
	ITC.lpszULevel = NULL;
	ITC.lpszUXp = NULL;
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_HALO_Render(float _fR, float _fG, float _fB,
							   long _lHaloType,
							   TextureContainer * tc,
							   TextureContainer * tc2,
							   float POSX, float POSY, float fRatioX = 1, float fRatioY = 1)
{
	float power=0.9f-EEsin(FrameTime*DIV50)*DIV10+rnd()*DIV10;

	if (power>1.f) power=1.f;

	_fR *= power;
	_fG *= power;
	_fB *= power;
	D3DCOLOR col=D3DRGB(_fR,_fG,_fB);

	if (_lHaloType & HALO_NEGATIVE)
	{
		SETBLENDMODE(GDevice,D3DBLEND_ZERO,D3DBLEND_INVSRCCOLOR);
	}
	else
	{
		SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
	}

	SETALPHABLEND(GDevice,true);
	DDSURFACEDESC2 ddsdSurfaceDescSrc;
	DDSURFACEDESC2 ddsdSurfaceDescHalo;
	ddsdSurfaceDescSrc.dwSize=sizeof(DDSURFACEDESC2);
	ddsdSurfaceDescHalo.dwSize=sizeof(DDSURFACEDESC2);
	tc->m_pddsSurface->GetSurfaceDesc(&ddsdSurfaceDescSrc);
	tc2->m_pddsSurface->GetSurfaceDesc(&ddsdSurfaceDescHalo);
	
	float fDeltaXP=((float)tc2->m_dwWidth/(float)ddsdSurfaceDescHalo.dwWidth)*((float)tc->m_dwWidth/(float)ddsdSurfaceDescSrc.dwWidth);
	float fDeltaYP=((float)tc2->m_dwHeight/(float)ddsdSurfaceDescHalo.dwHeight)*((float)tc->m_dwHeight/(float)ddsdSurfaceDescSrc.dwHeight);
	float fDeltaX=(float)tc->m_dwWidth/(float)ddsdSurfaceDescSrc.dwWidth;
	float fDeltaY=(float)tc->m_dwHeight/(float)ddsdSurfaceDescSrc.dwHeight;

	if(fDeltaX<1.f)
	{
		fDeltaXP*=(1.f/fDeltaX);
		fDeltaX=1.f;
	}

	if(fDeltaY<1.f)
	{
		fDeltaYP*=(1.f/fDeltaY);
		fDeltaY=1.f;
	}


	float fSizeX = ARX_CLEAN_WARN_CAST_FLOAT(ddsdSurfaceDescHalo.dwWidth); 
	float fSizeY = ARX_CLEAN_WARN_CAST_FLOAT(ddsdSurfaceDescHalo.dwHeight);


		fSizeX *= fRatioX;
		fSizeY *= fRatioY;
		POSX -= (fSizeX - ddsdSurfaceDescHalo.dwWidth)*0.25f;
		POSY -= (fSizeY - ddsdSurfaceDescHalo.dwHeight)*0.25f;
		fDeltaXP = 1;
		fDeltaYP = 1;
	
	EERIEDrawBitmap(GDevice,(float)(POSX-tc->halodecalX*fDeltaX),(float)(POSY-tc->halodecalY*fDeltaY)
								,((float)fSizeX)*fDeltaXP
								,((float)fSizeY)*fDeltaYP,0.00001f
								,tc2,col);
	SETALPHABLEND(GDevice,false);
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_HALO_Draw(INTERACTIVE_OBJ * io,LPDIRECT3DDEVICE7 m_pd3dDevice,TextureContainer * tc,TextureContainer * tc2,float POSX,float POSY, float _fRatioX = 1, float _fRatioY = 1)
{
	INTERFACE_HALO_NB++; 

	if (INTERFACE_HALO_NB>INTERFACE_HALO_MAX_NB)
	{
		INTERFACE_HALO_MAX_NB=INTERFACE_HALO_NB;
		aiHalo=(ARX_INTERFACE_HALO_STRUCT *)realloc(aiHalo,sizeof(ARX_INTERFACE_HALO_STRUCT)*INTERFACE_HALO_NB);
	}

	aiHalo[INTERFACE_HALO_NB-1].io=io;
	aiHalo[INTERFACE_HALO_NB-1].tc=tc;
	aiHalo[INTERFACE_HALO_NB-1].tc2=tc2;
	aiHalo[INTERFACE_HALO_NB-1].POSX=POSX;
	aiHalo[INTERFACE_HALO_NB-1].POSY=POSY;
	aiHalo[INTERFACE_HALO_NB-1].fRatioX = _fRatioX;
	aiHalo[INTERFACE_HALO_NB-1].fRatioY = _fRatioY;
}

//-----------------------------------------------------------------------------
void ReleaseHalo()
{
	if (aiHalo)
	{
		free(aiHalo);
		aiHalo=NULL;
	}
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_HALO_Flush(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	for (long i=0;i<INTERFACE_HALO_NB;i++)
		ARX_INTERFACE_HALO_Render(
		aiHalo[i].io->halo.color.r, aiHalo[i].io->halo.color.g, aiHalo[i].io->halo.color.b,
		aiHalo[i].io->halo.flags,
		aiHalo[i].tc,aiHalo[i].tc2,aiHalo[i].POSX,aiHalo[i].POSY, aiHalo[i].fRatioX, aiHalo[i].fRatioY);

	INTERFACE_HALO_NB=0;
}

//-----------------------------------------------------------------------------
bool NeedHalo(INTERACTIVE_OBJ * io)
{
	if (	(!io)
		||	(!(io->ioflags & IO_ITEM))	)
		return false;

	if (io->halo.flags & HALO_ACTIVE)
	{
		if ((io->inv) && (io->inv->TextureHalo==NULL))
		{
			io->inv->CreateHalo(GDevice);
		}

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
void InventoryOpenClose(unsigned long t) // 0 switch 1 forceopen 2 forceclose
{
	if ((t==1) && (player.Interface & INTER_INVENTORY)) return;

	if ((t==2) && (!(player.Interface & INTER_INVENTORY))) return;

	ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());

	if ((player.Interface & INTER_INVENTORY)||(player.Interface & INTER_INVENTORYALL)) 
	{
		bInventoryClosing = true;

		if (WILLRETURNTOFREELOOK)
		{
			TRUE_PLAYER_MOUSELOOK_ON |= 1;
			SLID_START=(float)ARXTime;
			WILLRETURNTOFREELOOK = 0;
		}
	}
	else
	{
		player.Interface |= INTER_INVENTORY;
		InventoryY = ARX_CLEAN_WARN_CAST_LONG(INTERFACE_RATIO(100.f));

		if (TRUE_PLAYER_MOUSELOOK_ON)
		{
			WILLRETURNTOFREELOOK = 1;
		}
	}

	if ((((player.Interface & INTER_INVENTORYALL)||(TRUE_PLAYER_MOUSELOOK_ON))&&(INTERNATIONAL_MODE))&&(player.Interface & INTER_NOTE))
	{
		ARX_INTERFACE_NoteClose();
	}

	if (!bInventoryClosing && pMenuConfig->bAutoReadyWeapon == false)
	{
		TRUE_PLAYER_MOUSELOOK_ON &= ~1;
	}
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_NoteInit()
{
	Note.type = NOTE_TYPE_UNDEFINED;
	Note.text = NULL;
	Note.textsize = 0;
	
	QuestBook.curpage=0;
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_NoteClear()
{
	Note.type = NOTE_TYPE_UNDEFINED;

	if (Note.text)
		free(Note.text);

	Note.text=NULL;
	Note.textsize=0;
	player.Interface&=~INTER_NOTE;	

	if (NoteTexture)
	{
		D3DTextr_KillTexture(NoteTexture);
		NoteTexture=NULL;
	}

	if (NoteTextureLeft)
	{
		D3DTextr_KillTexture(NoteTextureLeft);
		NoteTextureLeft=NULL;
	}

	if (NoteTextureRight)
	{
		D3DTextr_KillTexture(NoteTextureRight);
		NoteTextureRight=NULL;
	}
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_NoteOpen(ARX_INTERFACE_NOTE_TYPE type,char * tex)
{
 

	if (player.Interface & INTER_NOTE)
		ARX_INTERFACE_NoteClose();
	
	_TCHAR output[8096];
	ARX_INTERFACE_BookOpenClose(2);
	ARX_INTERFACE_NoteClear();
	Note.type=type;
	MakeLocalised(tex,output,8096);
	Note.text = (_TCHAR *)malloc((_tcslen(output)+1)*sizeof(_TCHAR));

	ZeroMemory(Note.text, (_tcslen(output)+1)*sizeof(_TCHAR));
	_tcscpy(Note.text,output); 	
	player.Interface|=INTER_NOTE;

	if (NoteTexture)
	{
		D3DTextr_KillTexture(NoteTexture);
		NoteTexture=NULL;
	}

	if (NoteTextureLeft)
	{
		D3DTextr_KillTexture(NoteTextureLeft);
		NoteTextureLeft=NULL;
	}

	if (NoteTextureRight)
	{
		D3DTextr_KillTexture(NoteTextureRight);
		NoteTextureRight=NULL;
	}
	


		Note.curpage=0;
		Note.pages[0]=0;
		long length=_tcslen(Note.text);
		long curpage=1;
		
		NoteTexture=MakeTCFromFile("Graph\\Interface\\book\\Ingame_books.bmp");
		RECT rRect;
		

		float fWidth	= NoteTexture->m_dwWidth*DIV2-10.f ; 
		float fHeight	= NoteTexture->m_dwHeight-40.f ; 
		ARX_CHECK_INT(fWidth);
		ARX_CHECK_INT(fHeight);

	int	tNoteTextMinx = 40; 
		int	tNoteTextMaxx = ARX_CLEAN_WARN_CAST_INT(fWidth);
	int	tNoteTextMiny = 40; 
		int	tNoteTextMaxy = ARX_CLEAN_WARN_CAST_INT(fHeight);
		
		float fMinx		= (tNoteTextMaxx-tNoteTextMinx)*Xratio;
		float fMiny		= (tNoteTextMaxy-tNoteTextMiny)*Yratio;
		ARX_CHECK_INT(fMinx);
		ARX_CHECK_INT(fMiny);

		SetRect(&rRect,0,0,ARX_CLEAN_WARN_CAST_INT(fMinx),ARX_CLEAN_WARN_CAST_INT(fMiny));


		int lLenghtCurr=0;

		while(length>0)
		{
			long lLengthDraw=ARX_UNICODE_ForceFormattingInRect(	hFontInGameNote,
																Note.text+lLenghtCurr,
																0,
																rRect);
			length-=lLengthDraw;
			lLenghtCurr+=lLengthDraw;
			Note.pages[curpage++]=lLenghtCurr;
		}

		Note.pages[curpage++]=-1;
		Note.totpages=curpage;
		NoteTexture=NULL;
	
	if (Note.totpages > 3)
	{
		if (Note.type == NOTE_TYPE_NOTE)
			Note.type = NOTE_TYPE_BIGNOTE;
		else
			Note.type = NOTE_TYPE_BOOK;
	}
	
	switch (Note.type)
	{
		case NOTE_TYPE_NOTICE :
			ARX_SOUND_PlayInterface(SND_MENU_CLICK, 0.9F + 0.2F * rnd());
			break;
		case NOTE_TYPE_BOOK   :
			ARX_SOUND_PlayInterface(SND_BOOK_OPEN, 0.9F + 0.2F * rnd());
			break;
		case NOTE_TYPE_NOTE   :
			ARX_SOUND_PlayInterface(SND_SCROLL_OPEN, 0.9F + 0.2F * rnd());
			break;
	}
	
	if (TRUE_PLAYER_MOUSELOOK_ON && (Note.type == NOTE_TYPE_BOOK))
	{
		TRUE_PLAYER_MOUSELOOK_ON&=~1;
	}
		
	if ((player.Interface & INTER_INVENTORYALL))
	{
		bInventoryClosing = true;
		ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
	}
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_NoteClose()
{
	if (player.Interface & INTER_NOTE)
	{
		switch (Note.type)
		{
			case NOTE_TYPE_NOTICE :
				ARX_SOUND_PlayInterface(SND_MENU_CLICK, 0.9F + 0.2F * rnd());
				break;
			case NOTE_TYPE_BOOK   :
				ARX_SOUND_PlayInterface(SND_BOOK_CLOSE, 0.9F + 0.2F * rnd());
				break;
			case NOTE_TYPE_NOTE   :
				ARX_SOUND_PlayInterface(SND_SCROLL_CLOSE, 0.9F + 0.2F * rnd());
				break;
		}
		
		ARX_INTERFACE_NoteClear();
	}

	player.Interface &= ~INTER_NOTE;
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_NoteManage()
{
	if (player.Interface & INTER_NOTE)
	{
		long clicknotmanaged=1;

		if (NoteTexture==NULL) 
		{
			switch (Note.type)
			{
			case NOTE_TYPE_NOTE:
				NoteTexture=MakeTCFromFile("Graph\\Interface\\book\\BigNote.bmp");

				if (NoteTexture)
				{
					NotePosX = 320-NoteTexture->m_dwWidth*DIV2;
					NotePosY=47.f;
					NoteTextMinx=30.f;
					NoteTextMaxx=NoteTexture->m_dwWidth-40.f;
					NoteTextMiny=30.f;
					NoteTextMaxy=NoteTexture->m_dwHeight-40.f;
				}					

				break;
			case NOTE_TYPE_NOTICE:
				NoteTexture=MakeTCFromFile("Graph\\Interface\\book\\Notice.bmp");

				if (NoteTexture)
				{
					NotePosX = 320-NoteTexture->m_dwWidth*DIV2;
					NotePosY=47.f;
					NoteTextMinx=50.f;
					NoteTextMaxx=NoteTexture->m_dwWidth-50.f;
					NoteTextMiny=50.f;
					NoteTextMaxy=NoteTexture->m_dwHeight-50.f;
				}					

				break;
			case NOTE_TYPE_BIGNOTE:
			case NOTE_TYPE_BOOK:

				if (Note.type == NOTE_TYPE_BIGNOTE)
				{
					NoteTexture=MakeTCFromFile("Graph\\Interface\\book\\Very_BigNote.bmp");
					NoteTextureLeft=MakeTCFromFile("Graph\\Interface\\book\\Left_corner.bmp");
					NoteTextureRight=MakeTCFromFile("Graph\\Interface\\book\\Right_corner.bmp");
				}
				else
				{
					NoteTexture=MakeTCFromFile("Graph\\Interface\\book\\Ingame_books.bmp");
					NoteTextureLeft=MakeTCFromFile("Graph\\Interface\\book\\Left_corner.bmp");
					NoteTextureRight=MakeTCFromFile("Graph\\Interface\\book\\Right_corner.bmp");
				}

				if (NoteTexture)
				{
					NotePosX = 320-NoteTexture->m_dwWidth*DIV2;
					NotePosY=47.f;
					NoteTextMinx = 40.f;
					NoteTextMaxx = NoteTexture->m_dwWidth*DIV2-10.f;
					NoteTextMiny = 40.f;
					NoteTextMaxy = NoteTexture->m_dwHeight-40.f;
				}					

				break;
			}
		}

		if (NoteTexture)
		{
			SETTEXTUREWRAPMODE(GDevice,D3DTADDRESS_CLAMP);
			DrawBookInterfaceItem(GDevice, NoteTexture, NotePosX, NotePosY);				
			
			if (Note.type==NOTE_TYPE_BOOK || Note.type == NOTE_TYPE_BIGNOTE)
			{

				float x0, y0;
				
				if(	(Note.curpage+4)<Note.totpages )
				{
					x0 = -14 + NotePosX + NoteTexture->m_dwWidth - NoteTextureRight->m_dwWidth - 1;
					y0 = -6  + NotePosY + NoteTexture->m_dwHeight - NoteTextureRight->m_dwHeight;

					if ( Note.type == NOTE_TYPE_BOOK )
					{

						float fWidth = ARX_CLEAN_WARN_CAST_FLOAT( x0 + NoteTextureRight->m_dwWidth )   ;
						float fHeight = ARX_CLEAN_WARN_CAST_FLOAT( y0 + NoteTextureRight->m_dwHeight ) ;

						DrawBookInterfaceItem(GDevice, NoteTextureRight,x0, y0);

						if ( MouseInBookRect(x0, y0, fWidth, fHeight) )

						{
							SpecialCursor=CURSOR_INTERACTION_ON;


							if (!(EERIEMouseButton & 1) && (LastMouseClick & 1))
							{
								clicknotmanaged=0;
								ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
								Note.curpage+=2;
							} 
						}
					}
				}

				if (Note.curpage>1)
				{
					x0 =  8 + NotePosX ;
					y0 = -6 + NotePosY + NoteTexture->m_dwHeight - NoteTextureLeft->m_dwHeight;	

					if (Note.type == NOTE_TYPE_BOOK)
					{

						float fWidth  = ARX_CLEAN_WARN_CAST_FLOAT( x0 + NoteTextureLeft->m_dwWidth )   ;
						float fHeight = ARX_CLEAN_WARN_CAST_FLOAT( y0 + NoteTextureLeft->m_dwHeight ) ;

						DrawBookInterfaceItem(GDevice, NoteTextureLeft, x0, y0);

						if ( MouseInBookRect( x0, y0, fWidth, fHeight) )

						{
							SpecialCursor=CURSOR_INTERACTION_ON;

							if (!(EERIEMouseButton & 1) && (LastMouseClick & 1))
							{					
								clicknotmanaged=0;
								ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
								Note.curpage-=2;							
							} 
						}
					}
				}

				if (Note.pages[Note.curpage]>=0)
				{
					if(Note.pages[Note.curpage+1]>0)
					{
						_tcsncpy(Page_Buffer,Note.text+Note.pages[Note.curpage],Note.pages[Note.curpage+1]-Note.pages[Note.curpage]);
						Page_Buffer[Note.pages[Note.curpage+1]-Note.pages[Note.curpage]]=_T('\0');

						danaeApp.DANAEEndRender();
						DrawBookTextInRect(NotePosX+NoteTextMinx, NotePosY+NoteTextMiny, NotePosX+NoteTextMaxx, NotePosY+NoteTextMaxy,Page_Buffer,0,0x00FF00FF, hFontInGameNote);

						danaeApp.DANAEStartRender();
						
						if(Note.pages[Note.curpage+2]>0)
						{
							_tcsncpy(Page_Buffer,Note.text+Note.pages[Note.curpage+1],Note.pages[Note.curpage+2]-Note.pages[Note.curpage+1]);
							Page_Buffer[Note.pages[Note.curpage+2]-Note.pages[Note.curpage+1]]=_T('\0');

							danaeApp.DANAEEndRender();
							DrawBookTextInRect(NotePosX+NoteTextMinx + (NoteTextMaxx-NoteTextMinx) +20, NotePosY+NoteTextMiny, NotePosX+NoteTextMaxx + (NoteTextMaxx-NoteTextMinx) +20, NotePosY+NoteTextMaxy,Page_Buffer,0,0x00FF00FF, hFontInGameNote);

							danaeApp.DANAEStartRender();
						}
					}
					else
					{
						if(Note.pages[Note.curpage]>=0)
						{
							_tcscpy(Page_Buffer,Note.text+Note.pages[Note.curpage]);

							danaeApp.DANAEEndRender();
							DrawBookTextInRect( NotePosX+NoteTextMinx, NotePosY+NoteTextMiny, NotePosX+NoteTextMaxx, NotePosY+NoteTextMaxy,Page_Buffer,0,0x00FF00FF, hFontInGameNote);

							danaeApp.DANAEStartRender();
						}
					}
				}

				SETTEXTUREWRAPMODE(GDevice,D3DTADDRESS_WRAP);
			}

			else
			{
				danaeApp.DANAEEndRender();
				DrawBookTextInRect( NotePosX+NoteTextMinx, NotePosY+NoteTextMiny, NotePosX+NoteTextMaxx, NotePosY+NoteTextMaxy,Note.text,0,0x00FF00FF, hFontInGameNote);
				danaeApp.DANAEStartRender();
			}
		}

		if (NoteTexture)
		if (MouseInBookRect(NotePosX, NotePosY, NotePosX+NoteTexture->m_dwWidth, NotePosY+NoteTexture->m_dwHeight))
		{
			if ((((EERIEMouseButton & 1) && (!(LastMouseClick & 1)) && (TRUE_PLAYER_MOUSELOOK_ON) )||(EERIEMouseButton & 2) && (!(LastMouseClick & 2))) && clicknotmanaged)
			{
				ARX_INTERFACE_NoteClose();
				EERIEMouseButton &= ~2;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_BookOpenClose(unsigned long t) // 0 switch 1 forceopen 2 forceclose
{
	if ((t==1) && (player.Interface & INTER_MAP)) return;

	if ((t==2) && (!(player.Interface & INTER_MAP))) return;
	
	
	if (player.Interface & INTER_MAP) 
	{
		ARX_SOUND_PlayInterface(SND_BOOK_CLOSE, 0.9F + 0.2F * rnd());
		SendIOScriptEvent(inter.iobj[0],SM_BOOK_CLOSE,"",NULL);	
		player.Interface &=~ INTER_MAP;
		ARX_MINIMAP_PurgeTC();
		
		if(ARXmenu.mda)
		{
			for (long i=0;i<MAX_FLYOVER;i++)
			{
				if (ARXmenu.mda->flyover[i]!=NULL)
					ARX_Menu_Release_Text(ARXmenu.mda->flyover[i]);
			}

			free((void*)ARXmenu.mda);
			ARXmenu.mda=NULL;
		}
	}
	else
	{
		SendIOScriptEvent(inter.iobj[0],0,"","BOOK_OPEN");
		
		ARX_SOUND_PlayInterface(SND_BOOK_OPEN, 0.9F + 0.2F * rnd());
		SendIOScriptEvent(inter.iobj[0],SM_BOOK_OPEN,"",NULL);
		ARX_INTERFACE_NoteClose();
		player.Interface |= INTER_MAP;
		Book_MapPage=ARX_LEVELS_GetRealNum(CURRENTLEVEL)+1;

		if (Book_MapPage>8) Book_MapPage=8;

		if (Book_MapPage<1) Book_MapPage=1;
		
		if(!ARXmenu.mda)
		{
			ARXmenu.mda = (MENU_DYNAMIC_DATA *)malloc(sizeof(MENU_DYNAMIC_DATA)); 
			memset(ARXmenu.mda,0,sizeof(MENU_DYNAMIC_DATA));
			
			ARX_Allocate_Text(ARXmenu.mda->flyover[BOOK_STRENGTH],			_T("system_charsheet_strength"));
			ARX_Allocate_Text(ARXmenu.mda->flyover[BOOK_MIND],				_T("system_charsheet_intel"));
			ARX_Allocate_Text(ARXmenu.mda->flyover[BOOK_DEXTERITY],			_T("system_charsheet_Dex"));
			ARX_Allocate_Text(ARXmenu.mda->flyover[BOOK_CONSTITUTION],		_T("system_charsheet_consti"));
			ARX_Allocate_Text(ARXmenu.mda->flyover[BOOK_STEALTH],			_T("system_charsheet_stealth"));
			ARX_Allocate_Text(ARXmenu.mda->flyover[BOOK_MECANISM],			_T("system_charsheet_mecanism"));
			ARX_Allocate_Text(ARXmenu.mda->flyover[BOOK_INTUITION],			_T("system_charsheet_intuition"));
			ARX_Allocate_Text(ARXmenu.mda->flyover[BOOK_ETHERAL_LINK],		_T("system_charsheet_etheral_link"));
			ARX_Allocate_Text(ARXmenu.mda->flyover[BOOK_OBJECT_KNOWLEDGE],	_T("system_charsheet_objknoledge"));
			ARX_Allocate_Text(ARXmenu.mda->flyover[BOOK_CASTING],			_T("system_charsheet_casting"));
			ARX_Allocate_Text(ARXmenu.mda->flyover[BOOK_PROJECTILE],		_T("system_charsheet_projectile"));
			ARX_Allocate_Text(ARXmenu.mda->flyover[BOOK_CLOSE_COMBAT],		_T("system_charsheet_closecombat"));
			ARX_Allocate_Text(ARXmenu.mda->flyover[BOOK_DEFENSE],			_T("system_charsheet_defense"));
			ARX_Allocate_Text(ARXmenu.mda->flyover[BUTTON_QUICK_GENERATION],_T("system_charsheet_quickgenerate"));
			ARX_Allocate_Text(ARXmenu.mda->flyover[BUTTON_DONE],			_T("system_charsheet_done"));
			ARX_Allocate_Text(ARXmenu.mda->flyover[BUTTON_SKIN],			_T("system_charsheet_skin"));
			ARX_Allocate_Text(ARXmenu.mda->flyover[WND_ATTRIBUTES],			_T("system_charsheet_atributes"));
			ARX_Allocate_Text(ARXmenu.mda->flyover[WND_SKILLS],				_T("system_charsheet_skills"));
			ARX_Allocate_Text(ARXmenu.mda->flyover[WND_STATUS],				_T("system_charsheet_status"));
			ARX_Allocate_Text(ARXmenu.mda->flyover[WND_LEVEL],				_T("system_charsheet_level"));
			ARX_Allocate_Text(ARXmenu.mda->flyover[WND_XP],					_T("system_charsheet_xpoints"));
			ARX_Allocate_Text(ARXmenu.mda->flyover[WND_HP],					_T("system_charsheet_hp"));
			ARX_Allocate_Text(ARXmenu.mda->flyover[WND_MANA],				_T("system_charsheet_mana"));
			ARX_Allocate_Text(ARXmenu.mda->flyover[WND_AC],					_T("system_charsheet_AC"));
			ARX_Allocate_Text(ARXmenu.mda->flyover[WND_RESIST_MAGIC],		_T("system_charsheet_res_magic"));
			ARX_Allocate_Text(ARXmenu.mda->flyover[WND_RESIST_POISON],		_T("system_charsheet_res_poison"));
			ARX_Allocate_Text(ARXmenu.mda->flyover[WND_DAMAGE],				_T("system_charsheet_damage"));
		}
	}
	
	if (player.Interface&INTER_COMBATMODE)
	{
		player.Interface&=~INTER_COMBATMODE;
		ARX_EQUIPMENT_LaunchPlayerUnReadyWeapon();
	}
	
	if (player.Interface & INTER_INVENTORYALL)
	{
		ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
		bInventoryClosing = true;
	}
	
	BOOKZOOM = 0;
	bBookHalo = false;
	ulBookHaloTime = 0;
	pTextManage->Clear();

	TRUE_PLAYER_MOUSELOOK_ON &= ~1;
}

//-------------------------------------------------------------------------------------
// Keyboard/Mouse Management
//-------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void ResetPlayerInterface()
{
	player.Interface |= INTER_LIFE_MANA;
	SLID_VALUE = 0;
	lSLID_VALUE = 0;
	SLID_START=(float)ARXTime;
}

extern long MouseDragX, MouseDragY;

//-----------------------------------------------------------------------------
void ReleaseInfosCombine()
{
	INTERACTIVE_OBJ * io = NULL;

	if (player.bag)
	for (int iNbBag=0; iNbBag<player.bag; iNbBag++) 
	for (long j=0;j<INVENTORY_Y;j++)
	for (long i=0;i<INVENTORY_X;i++)
	{
		io = inventory[iNbBag][i][j].io;

		if(io)
		{
			io->ioflags&=~IO_CAN_COMBINE;
		}
	}

	if (SecondaryInventory)
	{
		for (long j=0;j<SecondaryInventory->sizey;j++)
			for (long i=0;i<SecondaryInventory->sizex;i++)
			{
				io=SecondaryInventory->slot[i][j].io;

				if ( io )
				{
					io->ioflags&=~IO_CAN_COMBINE;
				}
			}
	}
}

//-----------------------------------------------------------------------------
void GetInfosCombineWithIO(INTERACTIVE_OBJ * _pWithIO) 
{
	if(!COMBINE)
	{
		return;
	}

	char tcIndent[256];
	char tcIsClass[256];
	sprintf(tcIndent,COMBINE->filename);
	strcpy(tcIsClass,GetName(tcIndent));
	sprintf(tcIndent,"%s_%04d",tcIsClass,COMBINE->ident);
	MakeUpcase(tcIndent);				

		char tTxtCombineDest[256];

		if(	(_pWithIO)&&
			(_pWithIO!=COMBINE)&&
			(_pWithIO->script.data) )
		{
			char* pCopyScript=new char[_pWithIO->script.size];
			memcpy(pCopyScript,_pWithIO->script.data,_pWithIO->script.size);
			
			char* pCopyOverScript=NULL;

			if(_pWithIO->over_script.data)
			{
				pCopyOverScript=new char[_pWithIO->over_script.size];
				memcpy(pCopyOverScript,_pWithIO->over_script.data,_pWithIO->over_script.size);
			}

			char* pcDataEnd=NULL;
			char *pcFound=NULL;

			if(pCopyOverScript)
			{
				pcDataEnd=((char*)pCopyOverScript)+_pWithIO->over_script.size;
				pcFound=strstr((char*)pCopyOverScript,"ON COMBINE");

				if(pcFound)
				{
					unsigned int uiNbOpen=0;

					char *pcToken=strtok(pcFound,"\r\n");

					if(strstr(pcToken,"{"))
					{
						uiNbOpen++;
					}

					while(pcToken)
					{
						pcToken=strtok(NULL,"\r\n");
						strupr(pcToken);
	
						bool bCanCombine=false;
						char* pStartString;
						char* pEndString;

						if(	strstr(pcToken,"^$PARAM1")&&
							(pStartString=strstr(pcToken,"ISCLASS")) )
						{
							pStartString=strstr(pStartString,"\"");

							if(pStartString)
							{
								pStartString++;
								pEndString=strstr(pStartString,"\"");

								if(pEndString)
								{
									memcpy(tTxtCombineDest,pStartString,pEndString-pStartString);
									tTxtCombineDest[pEndString-pStartString]=0;

									if(!stricmp(tTxtCombineDest,tcIsClass))
									{
										//same class
										bCanCombine=true;
									}
								}
							}
						}
						else
						{
							if(	strstr(pcToken,"^$PARAM1")&&
								(pStartString=strstr(pcToken,"==")) )
							{
								pStartString=strstr(pStartString,"\"");

								if(pStartString)
								{
									pStartString++;
									pEndString=strstr(pStartString,"\"");

									if(pEndString)
									{
										memcpy(tTxtCombineDest,pStartString,pEndString-pStartString);
										tTxtCombineDest[pEndString-pStartString]=0;

										if(!stricmp(tTxtCombineDest,tcIndent))
										{
											//same class
											bCanCombine=true;
										}
									}
								}
							}
							else
							{
								if(	strstr(pcToken,"^$PARAM1")&&
									(pStartString=strstr(pcToken,"ISGROUP")) )
								{
									pStartString=strstr(pStartString," ");

									if(pStartString)
									{
										pStartString++;
										pEndString=strstr(pStartString," ");
										char* pEndString2=strstr(pStartString,")");

										if(pEndString2<pEndString)
										{
											pEndString=pEndString2;
										}

										if(pEndString)
										{
											memcpy(tTxtCombineDest,pStartString,pEndString-pStartString);
											tTxtCombineDest[pEndString-pStartString]=0;

											ARX_CHECK_NOT_NEG( COMBINE->nb_iogroups );

											for(	unsigned int uiNbGroups = 0 ;
													uiNbGroups < ARX_CAST_USHORT( COMBINE->nb_iogroups ) ;
													uiNbGroups++ )
											{

												if(!stricmp(tTxtCombineDest,COMBINE->iogroups[uiNbGroups].name))
												{
													//same class
													bCanCombine=true;
												}
											}
										}
									}
								}
							}
						}

						if(strstr(pcToken,"{"))
						{
							uiNbOpen++;
						}

						if(strstr(pcToken,"}"))
						{
							uiNbOpen--;
						}

						if(bCanCombine)
						{
							uiNbOpen=0;
							_pWithIO->ioflags|=IO_CAN_COMBINE;
						}
						else
						{
							_pWithIO->ioflags&=~IO_CAN_COMBINE;
						}

						if(!uiNbOpen)
						{
							break;
						}
					}
				}
			}

			if(_pWithIO->ioflags&IO_CAN_COMBINE)
			{
				delete[] pCopyScript;
				delete[] pCopyOverScript;
				return;
			}

			pcDataEnd=((char*)pCopyScript)+_pWithIO->script.size;
			pcFound=strstr((char*)pCopyScript,"ON COMBINE");

			if(pcFound)
			{
				unsigned int uiNbOpen=0;

				char *pcToken=strtok(pcFound,"\r\n");

				if(strstr(pcToken,"{"))
				{
					uiNbOpen++;
				}

				while(pcToken)
				{
					pcToken=strtok(NULL,"\r\n");
					strupr(pcToken);
	
					bool bCanCombine=false;
					char* pStartString;
					char* pEndString;

					if(	strstr(pcToken,"^$PARAM1")&&
						(pStartString=strstr(pcToken,"ISCLASS")) )
					{
						pStartString=strstr(pStartString,"\"");

						if(pStartString)
						{
							pStartString++;
							pEndString=strstr(pStartString,"\"");

							if(pEndString)
							{
								memcpy(tTxtCombineDest,pStartString,pEndString-pStartString);
								tTxtCombineDest[pEndString-pStartString]=0;

								if(!stricmp(tTxtCombineDest,tcIsClass))
								{
									//same class
									bCanCombine=true;
								}
							}
						}
					}
					else
					{
						if(	strstr(pcToken,"^$PARAM1")&&
							(pStartString=strstr(pcToken,"==")) )
						{
							pStartString=strstr(pStartString,"\"");

							if(pStartString)
							{
								pStartString++;
								pEndString=strstr(pStartString,"\"");

								if(pEndString)
								{
									memcpy(tTxtCombineDest,pStartString,pEndString-pStartString);
									tTxtCombineDest[pEndString-pStartString]=0;

									if(!stricmp(tTxtCombineDest,tcIndent))
									{
										//same class
										bCanCombine=true;
									}
								}
							}
						}
						else
						{
							if(	strstr(pcToken,"^$PARAM1")&&
								(pStartString=strstr(pcToken,"ISGROUP")) )
							{
								pStartString=strstr(pStartString," ");

								if(pStartString)
								{
									pStartString++;
									pEndString=strstr(pStartString," ");
									char* pEndString2=strstr(pStartString,")");

									if(pEndString2<pEndString)
									{
										pEndString=pEndString2;
									}

									if(pEndString)
									{
										memcpy(tTxtCombineDest,pStartString,pEndString-pStartString);
										tTxtCombineDest[pEndString-pStartString]=0;

										ARX_CHECK_NOT_NEG( COMBINE->nb_iogroups );

										for(	unsigned int uiNbGroups = 0 ;
												uiNbGroups < ARX_CAST_USHORT( COMBINE->nb_iogroups ) ;
												uiNbGroups++ )
										{

											if(!stricmp(tTxtCombineDest,COMBINE->iogroups[uiNbGroups].name))
											{
												//same class
												bCanCombine=true;
											}
										}
									}
								}
							}
						}
					}

					if(strstr(pcToken,"{"))
					{
						uiNbOpen++;
					}

					if(strstr(pcToken,"}"))
					{
						uiNbOpen--;
					}

					if(bCanCombine)
					{
						uiNbOpen=0;
						_pWithIO->ioflags|=IO_CAN_COMBINE;
					}
					else
					{
						_pWithIO->ioflags&=~IO_CAN_COMBINE;
					}

					if(!uiNbOpen)
					{
						break;
					}
				}
			}

			delete[] pCopyScript;
			delete[] pCopyOverScript;
		}
}

//-----------------------------------------------------------------------------
void GetInfosCombine() 
{
	INTERACTIVE_OBJ * io = NULL;

	if (player.bag)
	for (int iNbBag=0; iNbBag<player.bag; iNbBag++) 
	for (long j=0;j<INVENTORY_Y;j++)
	for (long i=0;i<INVENTORY_X;i++)
	{
		io = inventory[iNbBag][i][j].io;
		GetInfosCombineWithIO(io);
	}

	if (SecondaryInventory)
	{
		for (long j=0;j<SecondaryInventory->sizey;j++)
		{
			for (long i=0;i<SecondaryInventory->sizex;i++)
			{
				io=SecondaryInventory->slot[i][j].io;
				GetInfosCombineWithIO(io);
			}
		}
	}
}

//-----------------------------------------------------------------------------
BOOL DANAE::ManageEditorControls()
{
	float val = 0.f;
	EERIE_3D trans;
	
	eMouseState = MOUSE_IN_WORLD;

	if (TRUE_PLAYER_MOUSELOOK_ON && (pMenuConfig->bAutoReadyWeapon == false) && (pMenuConfig->bMouseLookToggle))
	{

		float fX =  DANAESIZX * 0.5f;
		float fY =	DANAESIZY * 0.5f;
		ARX_CHECK_SHORT(fX);
		ARX_CHECK_SHORT(fY);

		DANAEMouse.x = ARX_CLEAN_WARN_CAST_SHORT(fX);
		DANAEMouse.y = ARX_CLEAN_WARN_CAST_SHORT(fY);


	}

	
	/////////////////////////////////////////////////////
	// begining to count time for sliding interface
	if ((!PLAYER_INTERFACE_HIDE_COUNT) && (SMOOTHSLID==0))
	{
		bool bOk = true;

		if (TRUE_PLAYER_MOUSELOOK_ON)
		{
			if (!(player.Interface & INTER_COMBATMODE) && (player.doingmagic!=2) && !InInventoryPos(&DANAEMouse))
			{
				bOk = false;

				float t=(float)ARXTime;

				if (t-SLID_START>10000.f)
				{
					SLID_VALUE+=(float)Original_framedelay*DIV10;

					if (SLID_VALUE>100.f) SLID_VALUE=100.f;

					F2L(SLID_VALUE,&lSLID_VALUE);
				}
				else
				{
					bOk = true;
 
				}
				
			}
		}

		if (bOk)
		{
			SLID_VALUE-=(float)Original_framedelay*DIV10;

			if (SLID_VALUE<0.f) SLID_VALUE=0.f;		

			F2L(SLID_VALUE,&lSLID_VALUE);
		}
	}

	// on ferme
	if ((player.Interface & INTER_COMBATMODE) || (player.doingmagic>=2))
	{
		INTERACTIVE_OBJ * io = NULL;
		
		if (SecondaryInventory!=NULL)
		{
			io = (INTERACTIVE_OBJ *)SecondaryInventory->io;
		}
		else if (player.Interface & INTER_STEAL)
		{
			io = ioSteal;
		}

		if (io!=NULL)
		{
			InventoryDir=-1;
			SendIOScriptEvent(io,SM_INVENTORY2_CLOSE,"");
			TSecondaryInventory=SecondaryInventory;
			SecondaryInventory=NULL;
			
		}
	}

	
	if (SMOOTHSLID==1)
	{
		SLID_VALUE+=(float)Original_framedelay*DIV10;

		if (SLID_VALUE > 100.f)
		{
			SLID_VALUE = 100.f;
			SMOOTHSLID = 0;
		}

		F2L(SLID_VALUE,&lSLID_VALUE);
	}
	else if (SMOOTHSLID==-1)
	{
		SLID_VALUE-=(float)Original_framedelay*DIV10;

		if (SLID_VALUE < 0.f)
		{
			SLID_VALUE = 0.f;
			SMOOTHSLID = 0;
		}

		F2L(SLID_VALUE,&lSLID_VALUE);
	}
	
	if (CINEMA_INC==1)
	{
		CINEMA_DECAL+=(float)Original_framedelay*DIV10;

		if (CINEMA_DECAL > 100.f)
		{
			CINEMA_DECAL = 100.f;
			CINEMA_INC = 0;
		}
	}
	else if (CINEMA_INC==-1)
	{
		CINEMA_DECAL-=(float)Original_framedelay*DIV10;

		if (CINEMA_DECAL < 0.f)
		{
			CINEMA_DECAL = 0.f;
			CINEMA_INC = 0;
		}
	}

	/////////////////////////////////////////////////////
	
	if  (EERIEMouseButton & 1)
	{
		if ( !(LastMouseClick &1) )
		{
			STARTDRAG.x=DANAEMouse.x;
			STARTDRAG.y=DANAEMouse.y;
			DRAGGING=0;
			
			if (pMenuConfig->bAutoReadyWeapon == false)
			{
				MouseDragX = 0;
				MouseDragY = 0;
			}
		}
		else
		{
			if ((abs(DANAEMouse.x-STARTDRAG.x)>2) && (abs(DANAEMouse.y-STARTDRAG.y)>2)
			   || ((pMenuConfig->bAutoReadyWeapon == false) && ((abs(MouseDragX) > 2) || (abs(MouseDragY) > 2))))
			{
				DRAGGING=1;
			}
		}
	}
	else
	{
		DRAGGING=0;
	}

	//-------------------------------------------------------------------------
	// interface
	//-------------------------------------------------------------------------
	// torch
	float px = 0;
	float py = 0;
	
	if (!BLOCK_PLAYER_CONTROLS)
	{
		if ((!(player.Interface & INTER_COMBATMODE)))
		{
		
			if (!TRUE_PLAYER_MOUSELOOK_ON)
			{
				px = INTERFACE_RATIO(InventoryX) + INTERFACE_RATIO(110);

				if (px < INTERFACE_RATIO(10)) px = INTERFACE_RATIO(10);

				py = DANAESIZY - INTERFACE_RATIO(158+32);

				if (CURRENT_TORCH != NULL)
				{
					if (MouseInRect(px,py, px + INTERFACE_RATIO(32), py + INTERFACE_RATIO(64)))
					{
						eMouseState=MOUSE_IN_TORCH_ICON;
						SpecialCursor=CURSOR_INTERACTION_ON;
						
						if ((LastMouseClick & 1) && (!(EERIEMouseButton & 1)) )
						{
							INTERACTIVE_OBJ * temp = CURRENT_TORCH;

							if ((temp!=NULL) && temp->locname[0])
							{
								if (((CURRENT_TORCH->ioflags & IO_ITEM) && CURRENT_TORCH->_itemdata->equipitem)
									&& (player.Full_Skill_Object_Knowledge + player.Full_Attribute_Mind
									>= CURRENT_TORCH->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Identify_Value].value) )
								{
									SendIOScriptEvent(FlyingOverIO,SM_IDENTIFY,"");
								}

								MakeLocalised(temp->locname,WILLADDSPEECH,256,-1);

								if (temp->ioflags & IO_GOLD)
								{
									_TCHAR UText[256];
									_stprintf(UText, _T("%d %s"), temp->_itemdata->price, WILLADDSPEECH);
									_tcscpy(WILLADDSPEECH, UText);
								}

								if ((temp->poisonous>0) && (temp->poisonous_count!=0))
								{
									_TCHAR Text[256];
									_TCHAR UText[256];
									MakeLocalised("[Description_Poisoned]",Text,256,-1);
									_stprintf(UText, _T("%s (%s %d)"),  WILLADDSPEECH, Text, (long)temp->poisonous);
									_tcscpy(WILLADDSPEECH, UText);
								}

								if ((temp->ioflags & IO_ITEM) && (temp->durability<100.f))
								{
									_TCHAR Text[256];
									_TCHAR UText[256];
									MakeLocalised("[Description_Durability]",Text,256,-1);
									_stprintf(UText, _T("%s %s %3.0f/%3.0f"),  WILLADDSPEECH, Text, temp->durability,temp->max_durability);
									_tcscpy(WILLADDSPEECH, UText);
								}


								WILLADDSPEECHTIME = ARXTimeUL(); 
							}
						}

						if  ((EERIEMouseButton & 1) && (LastMouseClick &1))
						{
							if ( (abs(DANAEMouse.x-STARTDRAG.x)>2) ||
								(abs(DANAEMouse.y-STARTDRAG.y)>2) )	DRAGGING = 1;
						}
						
						if ((DRAGINTER == NULL)  && (!PLAYER_MOUSELOOK_ON) && DRAGGING)
						{
							INTERACTIVE_OBJ * io=CURRENT_TORCH;
							CURRENT_TORCH->show=SHOW_FLAG_IN_SCENE;
							ARX_SOUND_PlaySFX(SND_TORCH_END);
							ARX_SOUND_Stop(SND_TORCH_LOOP);
							CURRENT_TORCH=NULL;	
							SHOW_TORCH=0;
							DynLight[0].exist=0;
							Set_DragInter(io);
							DRAGINTER->ignition=1;
						}
						else
						{
							if ((EERIEMouseButton & 4) && (COMBINE == NULL))
							{
								COMBINE = CURRENT_TORCH;
							}

							if (!(EERIEMouseButton & 2) && (LastMouseClick & 2))
							{
								ARX_PLAYER_ClickedOnTorch(CURRENT_TORCH);
								EERIEMouseButton &= ~2;
								TRUE_PLAYER_MOUSELOOK_ON&=~1;
							}
						}
					}
				}
				
				// redist
				if ((player.Skill_Redistribute) || (player.Attribute_Redistribute))
				{
					px = DANAESIZX - INTERFACE_RATIO(35) + lSLID_VALUE + GL_DECAL_ICONS;
					py = DANAESIZY - INTERFACE_RATIO(218);

					if (MouseInRect(px, py, px + INTERFACE_RATIO(32), py + INTERFACE_RATIO(32))) 
					{
						eMouseState=MOUSE_IN_REDIST_ICON;
						SpecialCursor=CURSOR_INTERACTION_ON;

						if ((EERIEMouseButton & 1) && !(LastMouseClick & 1))
						{
							ARX_INTERFACE_BookOpenClose(1);
							EERIEMouseButton &=~1;
						}

						return FALSE;
					}
				}
				
				
				// gold
				if (player.gold>0)
				{
					px = DANAESIZX - INTERFACE_RATIO(35) + lSLID_VALUE + GL_DECAL_ICONS;
					py = DANAESIZY - INTERFACE_RATIO(183);

					if (MouseInRect(px,py, px + INTERFACE_RATIO(32), py + INTERFACE_RATIO(32)))
					{
						eMouseState=MOUSE_IN_GOLD_ICON;
						SpecialCursor=CURSOR_INTERACTION_ON;

						if ((player.gold > 0)
							&& (!ARX_IMPULSE_Pressed(CONTROLS_CUST_MAGICMODE))
							&& (COMBINE==NULL) && (!COMBINEGOLD))
						{
							if (EERIEMouseButton & 4)
								COMBINEGOLD=1;
						}

						if (DRAGINTER == NULL)
							return FALSE;
					}
				}
				
				// book
				px = DANAESIZX - INTERFACE_RATIO(35) + lSLID_VALUE + GL_DECAL_ICONS;
				py = DANAESIZY - INTERFACE_RATIO(148);

				if (MouseInRect(px, py, px + INTERFACE_RATIO(32), py + INTERFACE_RATIO(32)))
				{
					eMouseState=MOUSE_IN_BOOK_ICON;
					SpecialCursor=CURSOR_INTERACTION_ON;

					if ((EERIEMouseButton & 1) && !(LastMouseClick & 1))
					{
						ARX_INTERFACE_BookOpenClose(0);
						EERIEMouseButton &=~1;
					}

					return FALSE;
				}
				
				// inventaire
				px = DANAESIZX - INTERFACE_RATIO(35) + lSLID_VALUE + GL_DECAL_ICONS;
				py = DANAESIZY - INTERFACE_RATIO(113);
				static float flDelay=0;

				if (MouseInRect(px, py, px + INTERFACE_RATIO(32), py + INTERFACE_RATIO(32))||flDelay)
				{
					eMouseState=MOUSE_IN_INVENTORY_ICON;
					SpecialCursor=CURSOR_INTERACTION_ON;

					if (EERIEMouseButton & 4)
					{
						ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());

						if (player.Interface | INTER_INVENTORYALL)
						{
							OptmizeInventory(0);
							OptmizeInventory(1);
							OptmizeInventory(2);
						}
						else
						{
							OptmizeInventory(sActiveInventory);
						}

						flDelay=0;
						EERIEMouseButton&=~4;
					}
					else if (((EERIEMouseButton & 1) && !(LastMouseClick & 1)) || flDelay)
					{
						if (!flDelay)
					{
							flDelay=ARX_TIME_Get();
							return false;
						}
						else
						{
							if ((ARX_TIME_Get() - flDelay) < 300)
							{
								return false;
							}
							else
							{
								flDelay=0;
							}
						}

						if (player.Interface & INTER_INVENTORYALL)
						{
							ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
							bInventoryClosing = true;

						}
						else
						{
							if (INTERNATIONAL_MODE)
							{
								bInverseInventory=!bInverseInventory;
								lOldTruePlayerMouseLook=TRUE_PLAYER_MOUSELOOK_ON;
							}
							else
							{
								InventoryOpenClose(0);
							}
						}

						EERIEMouseButton &=~1;
					}
					else if ((EERIEMouseButton & 2) && !(LastMouseClick & 2))
					{
						ARX_INTERFACE_BookOpenClose(2);
						ARX_INVENTORY_OpenClose(NULL);

						if (player.Interface & INTER_INVENTORYALL)
						{
							bInventoryClosing = true;
						}
						else
						{
							if ((player.Interface & INTER_INVENTORY))
							{
								ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
								bInventoryClosing = true;
								bInventorySwitch = true;
							}
							else
							{
								ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
								player.Interface |= INTER_INVENTORYALL;

								float fInventoryY	=	INTERFACE_RATIO( 121.f ) * (player.bag);
								ARX_CHECK_LONG( fInventoryY );
								InventoryY			=	ARX_CLEAN_WARN_CAST_LONG( fInventoryY );

								ARX_INTERFACE_NoteClose();

								if (TRUE_PLAYER_MOUSELOOK_ON)
								{
									WILLRETURNTOFREELOOK = 1;
								}
							}
						}

						EERIEMouseButton &= ~2;
						TRUE_PLAYER_MOUSELOOK_ON&=~1;
					}
					
					if (DRAGINTER == NULL)
						return FALSE;
				}
			}
			
			// steal
			if (player.Interface & INTER_STEAL)
			{
				px = ARX_CLEAN_WARN_CAST_FLOAT(-lSLID_VALUE);
				py = DANAESIZY - INTERFACE_RATIO(78 + 32);

				if (MouseInRect(px, py, px + INTERFACE_RATIO(32), py + INTERFACE_RATIO(32)))
				{
					eMouseState=MOUSE_IN_STEAL_ICON;
					SpecialCursor=CURSOR_INTERACTION_ON;

					if ((EERIEMouseButton & 1) && !(LastMouseClick & 1))
					{
						ARX_INVENTORY_OpenClose(ioSteal);

						if (player.Interface&(INTER_INVENTORY | INTER_INVENTORYALL))		
						{
							ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
						}

						if (SecondaryInventory != NULL)
						{
							SendIOScriptEvent(ioSteal, SM_STEAL,"");

							if (INTERNATIONAL_MODE)
							{
								bForceEscapeFreeLook=true;
							    lOldTruePlayerMouseLook=!TRUE_PLAYER_MOUSELOOK_ON;
							}
						}

						EERIEMouseButton &=~1;
					}

					if (DRAGINTER == NULL)
						return FALSE;
				}
			}
		}
	}

	// gros player book
	if (player.Interface & INTER_MAP) 
	{
		px = 97 * Xratio;
		py = 64 * Yratio;
		
		if (ITC.playerbook)
		{
			if (MouseInRect(px, py, px + ITC.playerbook->m_dwWidth * Xratio, py + ITC.playerbook->m_dwHeight * Yratio))
			{
				eMouseState = MOUSE_IN_BOOK;
			}
		}
	}
	
	// gros book/note
	if (player.Interface & INTER_NOTE)
	{
		switch (Note.type)
		{
		case NOTE_TYPE_NOTE:
			NoteTexture=MakeTCFromFile("Graph\\Interface\\book\\BigNote.bmp");

			if (NoteTexture)
			{
				NotePosX = 320-NoteTexture->m_dwWidth*DIV2;
				NotePosY=47.f;
				NoteTextMinx=30.f;
				NoteTextMaxx=NoteTexture->m_dwWidth-40.f;
				NoteTextMiny=30.f;
				NoteTextMaxy=NoteTexture->m_dwHeight-40.f;
			}					

			break;
		case NOTE_TYPE_NOTICE:
			NoteTexture=MakeTCFromFile("Graph\\Interface\\book\\Notice.bmp");

			if (NoteTexture)
			{
				NotePosX = 320-NoteTexture->m_dwWidth*DIV2;
				NotePosY=47.f;
				NoteTextMinx=50.f;
				NoteTextMaxx=NoteTexture->m_dwWidth-50.f;
				NoteTextMiny=50.f;
				NoteTextMaxy=NoteTexture->m_dwHeight-50.f;
			}

			break;
		case NOTE_TYPE_BIGNOTE:
		case NOTE_TYPE_BOOK:

			if (Note.type ==NOTE_TYPE_BIGNOTE)
			{
				NoteTexture=MakeTCFromFile("Graph\\Interface\\book\\Very_BigNote.bmp");
				NoteTextureLeft=MakeTCFromFile("Graph\\Interface\\book\\Left_corner.bmp");
				NoteTextureRight=MakeTCFromFile("Graph\\Interface\\book\\Right_corner.bmp");
			}
			else
			{
				NoteTexture=MakeTCFromFile("Graph\\Interface\\book\\Ingame_books.bmp");
				NoteTextureLeft=MakeTCFromFile("Graph\\Interface\\book\\Left_corner.bmp");
				NoteTextureRight=MakeTCFromFile("Graph\\Interface\\book\\Right_corner.bmp");
			}

			if (NoteTexture)
			{
				NotePosX = 320-NoteTexture->m_dwWidth*DIV2;
				NotePosY=47.f;
				NoteTextMinx = 40.f;
				NoteTextMaxx = NoteTexture->m_dwWidth*DIV2-10.f;
				NoteTextMiny = 40.f;
				NoteTextMaxy = NoteTexture->m_dwHeight-30.f;
			}

			break;
		}
		
		px = NotePosX * Xratio;
		py = NotePosY * Yratio;
		
		if (NoteTexture)
		if (MouseInRect(px, py, px+NoteTexture->m_dwWidth * Xratio, py+NoteTexture->m_dwHeight * Yratio))
		{
			eMouseState = MOUSE_IN_NOTE;
			return FALSE;
		}
	}

	if (!PLAYER_INTERFACE_HIDE_COUNT && (TSecondaryInventory!=NULL))
	{
		px = INTERFACE_RATIO(InventoryX) + INTERFACE_RATIO(16);
		py = INTERFACE_RATIO_DWORD(BasicInventorySkin->m_dwHeight) - INTERFACE_RATIO(16);
		INTERACTIVE_OBJ * temp=(INTERACTIVE_OBJ *)TSecondaryInventory->io;

		if (temp && !(temp->ioflags & IO_SHOP) && !(temp == ioSteal))
		{
			if (MouseInRect(px,py, px + INTERFACE_RATIO(16), py + INTERFACE_RATIO(16)))
			{
				eMouseState = MOUSE_IN_INVENTORY_PICKALL_ICON;
				SpecialCursor=CURSOR_INTERACTION_ON;

				if ((EERIEMouseButton & 1) && !(LastMouseClick & 1))
				{
					if (TSecondaryInventory)
					{
						// play un son que si un item est pris
						ARX_INVENTORY_TakeAllFromSecondaryInventory();
					}
					
					EERIEMouseButton &=~1;
				}

				if (DRAGINTER == NULL)
					return FALSE;
			}
		}

		//py = 20;
		px = INTERFACE_RATIO(InventoryX) + INTERFACE_RATIO_DWORD(BasicInventorySkin->m_dwWidth) - INTERFACE_RATIO(32);

		if (MouseInRect(px,py, px + INTERFACE_RATIO(16), py + INTERFACE_RATIO(16)))
		{
			eMouseState = MOUSE_IN_INVENTORY_CLOSE_ICON;
			SpecialCursor=CURSOR_INTERACTION_ON;

			if ((EERIEMouseButton & 1) && !(LastMouseClick & 1))
			{
				INTERACTIVE_OBJ * io = NULL;
				
				if (SecondaryInventory!=NULL)
				{
					io = (INTERACTIVE_OBJ *)SecondaryInventory->io;
				}
				else if (player.Interface & INTER_STEAL)
				{
					io = ioSteal;
				}

				if (io!=NULL)
				{
					ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd()); 
					InventoryDir=-1;
					SendIOScriptEvent(io,SM_INVENTORY2_CLOSE,"");
					TSecondaryInventory=SecondaryInventory;
					SecondaryInventory=NULL;
				}

				EERIEMouseButton &=~1;
			}

			if (DRAGINTER == NULL)
				return FALSE;
		}
	}

	//-------------------------------------------------------------------------
	
	
	// Single Click On Object
	if ( ( LastMouseClick & 1 ) && ( !( EERIEMouseButton & 1 ) ) )
	{
		if ( FlyingOverIO && ( !DRAGINTER ) )
		{
			SendIOScriptEvent( FlyingOverIO, SM_CLICKED, "" );
			bool bOk = true;
			
			if ( SecondaryInventory != NULL )
			{
				INTERACTIVE_OBJ * temp = (INTERACTIVE_OBJ *)SecondaryInventory->io;

				if (IsInSecondaryInventory(FlyingOverIO))
					if ( temp->ioflags & IO_SHOP )
						bOk = false;
			}

			if ( !( FlyingOverIO->ioflags & IO_MOVABLE ) )
				if ( ( FlyingOverIO->ioflags & IO_ITEM ) && bOk )
				{
					if ( ARX_IMPULSE_Pressed( CONTROLS_CUST_STEALTHMODE ) )
					{
						if ( !InPlayerInventoryPos( &DANAEMouse ) && !ARX_INTERFACE_MouseInBook() )
						{
							long sx, sy; 
							bool bSecondary = false;

							sx = sy = 0 ;

							if ( TSecondaryInventory && IsInSecondaryInventory( FlyingOverIO ) )
							{
								if ( SecondaryInventory )
								{
									bool bfound = true;

									for ( long j = 0 ; j < SecondaryInventory->sizey && bfound ; j++ )
									{
										for ( long i = 0 ; i < SecondaryInventory->sizex && bfound ; i++ )
										{
											if ( SecondaryInventory->slot[i][j].io == FlyingOverIO )
											{
												sx		= i;
												sy		= j;
												bfound	= false;
											}
										}
									}

									if (bfound) ARX_CHECK_NO_ENTRY();
								}

								bSecondary = true;
							}

							RemoveFromAllInventories( FlyingOverIO );
							FlyingOverIO->show = SHOW_FLAG_IN_INVENTORY;

							if ( FlyingOverIO->ioflags & IO_GOLD )
							{
								ARX_SOUND_PlayInterface( SND_GOLD );	
							}

							ARX_SOUND_PlayInterface( SND_INVSTD );

							if ( !CanBePutInInventory( FlyingOverIO ) )
							{
								if ( TSecondaryInventory && bSecondary )
								{
									extern short sInventory, sInventoryX, sInventoryY;
									sInventory	= 2;

									ARX_CHECK_SHORT(sx);
									ARX_CHECK_SHORT(sy);
									sInventoryX = ARX_CLEAN_WARN_CAST_SHORT(sx);
									sInventoryY = ARX_CLEAN_WARN_CAST_SHORT(sy);

									CanBePutInSecondaryInventory( TSecondaryInventory, FlyingOverIO, &sx, &sy );
								}

								if ( !bSecondary )
								{
									FlyingOverIO->show = SHOW_FLAG_IN_SCENE;
								}
							}

							if (DRAGINTER == FlyingOverIO)
							{
								DRAGINTER = NULL;
							}

							FlyingOverIO = NULL;
						}
					}
				}
		}
	}
	
	if (!(player.Interface & INTER_COMBATMODE))
	{
		// Dropping an Interactive Object that has been dragged
		if ((!(EERIEMouseButton & 1)) && (LastMouseClick & 1) && (DRAGINTER!=NULL)) 
		{
			//if (ARX_EQUIPMENT_PutOnPlayer(DRAGINTER))
			if (InInventoryPos(&DANAEMouse)) // Attempts to put it in inventory
			{
				if (!PutInInventory())
				{
				}			
			}
			else if (eMouseState == MOUSE_IN_INVENTORY_ICON)
			{
				if (!PutInInventory())
				{
				}
			}
			else if (ARX_INTERFACE_MouseInBook())
			{	
				if (Book_Mode == 0)
				{
					SendIOScriptEvent(DRAGINTER,SM_INVENTORYUSE,"");
					COMBINE=NULL;
				}
			}			
			else if (DRAGINTER->ioflags & IO_GOLD)
			{
					ARX_PLAYER_AddGold(DRAGINTER->_itemdata->price);
					ARX_SOUND_PlayInterface(SND_GOLD);

					if (DRAGINTER->scriptload)
					{
						RemoveFromAllInventories(DRAGINTER);

						ReleaseInter(DRAGINTER);
					}
					else
					{
						DRAGINTER->show=SHOW_FLAG_IN_INVENTORY;
						DRAGINTER->GameFlags&=~GFLAG_ISINTREATZONE;
					}	

					Set_DragInter(NULL);
				}
			else if (DRAGINTER!=NULL)
			{
				if (!EDITMODE) // test for NPC & FIX
				{	
					if ((DRAGINTER->ioflags & IO_NPC) || (DRAGINTER->ioflags & IO_FIX) )
					{
						Set_DragInter(NULL);
						goto suivant2;
					}
				}

				if (!((DRAGINTER->ioflags & IO_ITEM) && (DRAGINTER->_itemdata->count > 1)))
					if ((DRAGINTER->obj) && (DRAGINTER->obj->pbox))
					{

						if (
							(!InInventoryPos(&DANAEMouse))) //Put object in fromt of player
						{
							if (ARX_MOUSE_OVER & ARX_MOUSE_OVER_BOOK)
								 goto suivant2;

							long res=Manage3DCursor(0);
							
						if (res==0) // Throw Object
						{
							INTERACTIVE_OBJ * io=DRAGINTER;
							ARX_PLAYER_Remove_Invisibility();
							io->obj->pbox->active=1;
							io->obj->pbox->stopcount=0;
							float vx=-((float)DANAEMouse.x-(float)subj.centerx);

							vx/=3.f;
							EERIE_3D pos;
								pos.x = io->pos.x = player.pos.x; 
								pos.z = io->pos.z = player.pos.z; 
							pos.y=io->pos.y=player.pos.y+80.f;
							io->velocity.x=0.f;
							io->velocity.y=0.f;
							io->velocity.z=0.f;
							io->stopped=1;
							float y_ratio=(float)((float)DANAEMouse.y-(float)DANAECENTERY)/(float)DANAESIZY*2;
							float x_ratio=-(float)((float)DANAEMouse.x-(float)DANAECENTERX)/(float)DANAECENTERX;
							EERIE_3D viewvector;
							viewvector.x=-(float)EEsin(DEG2RAD(player.angle.b+(x_ratio*30.f)))*EEcos(DEG2RAD(player.angle.a));
								viewvector.y = EEsin(DEG2RAD(player.angle.a)) + y_ratio; 
							viewvector.z= (float)EEcos(DEG2RAD(player.angle.b+(x_ratio*30.f)))*EEcos(DEG2RAD(player.angle.a));
							io->soundtime=0;
							io->soundcount=0;
							EERIE_PHYSICS_BOX_Launch(io->obj,&pos,&viewvector);
							ARX_SOUND_PlaySFX(SND_WHOOSH, &pos);
							io->show=SHOW_FLAG_IN_SCENE;
							Set_DragInter(NULL);
						}
				}
						}

			suivant2:
				;
						}
					}
						
	if (COMBINE)
	{
		if ((!CURRENT_TORCH) || (CURRENT_TORCH && (COMBINE != CURRENT_TORCH)))
		{
			EERIE_3D pos;

			if (GetItemWorldPosition(COMBINE,&pos))
			{
				if (EEDistance3D(&pos,&player.pos)>300.f)
					COMBINE=NULL;
			}
			else COMBINE=NULL;
		}
	}
	
	if ((EERIEMouseButton & 1) && !(LastMouseClick & 1) && ((COMBINE !=NULL) || COMBINEGOLD)) 
	{
			ReleaseInfosCombine(); 

		INTERACTIVE_OBJ * io;

		if ((io=FlyingOverIO)!=NULL)
		{
			if (COMBINEGOLD)
			{
				char temp[256];
				strcpy(temp,"GOLD_COIN");
				SendIOScriptEvent(io,SM_COMBINE,temp);
			}
			else
			{
				if (io!=COMBINE)
				{
					char temp[256];
					char temp2[256];
					sprintf(temp,COMBINE->filename);
					strcpy(temp2,GetName(temp));
					sprintf(temp,"%s_%04d",temp2,COMBINE->ident);
					MakeUpcase(temp);				
					EVENT_SENDER=COMBINE;

					if (!specialstrcmp(temp2,"KEYRING"))
						ARX_KEYRING_Combine(io);
					else
						SendIOScriptEvent(io,SM_COMBINE,temp);
				}
			}			
		}

		else // GLights
		{
			float fMaxdist = 300;

			if (Project.telekinesis) fMaxdist = 850;
			
			for (long i=0;i<MAX_LIGHTS;i++)
			{
				if ((GLight[i]!=NULL) &&
					(GLight[i]->exist) &&
					(EEDistance3D(&GLight[i]->pos, &player.pos) <= fMaxdist) &&
					(!(GLight[i]->extras & EXTRAS_NO_IGNIT)))
				{
					if (MouseInRect(GLight[i]->mins.x, GLight[i]->mins.y, GLight[i]->maxs.x, GLight[i]->maxs.y))
					{
						if (COMBINE->ioflags & IO_ITEM)
						{
							if ((COMBINE == CURRENT_TORCH) || (COMBINE->_itemdata->LightValue == 1))
							{
								if (GLight[i]->status != 1)
								{
									GLight[i]->status = 1;
									ARX_SOUND_PlaySFX(SND_TORCH_START, &GLight[i]->pos);
								}
							}

							if (COMBINE->_itemdata->LightValue == 0)
							{
								if (GLight[i]->status != 0)
								{
									GLight[i]->status = 0;
									ARX_SOUND_PlaySFX(SND_TORCH_END, &GLight[i]->pos);
									SendIOScriptEvent(COMBINE, SM_CUSTOM, "DOUSE");
								}
							}
						}
					}
				}
			}
		}

		COMBINEGOLD=0;
		bool bQuitCombine = true;

		if ((player.Interface & INTER_INVENTORY))
		{
			if (player.bag)
			{

					float fCenterX	= DANAECENTERX + INTERFACE_RATIO(-320 + 35) + INTERFACE_RATIO_DWORD(ITC.hero_inventory->m_dwWidth) - INTERFACE_RATIO(32 + 3) ;
					float fSizY		= DANAESIZY - INTERFACE_RATIO(101) + INTERFACE_RATIO_LONG(InventoryY) + INTERFACE_RATIO(- 3 + 25) ;


				float posx = ARX_CAST_TO_INT_THEN_FLOAT( fCenterX );
				float posy = ARX_CAST_TO_INT_THEN_FLOAT( fSizY );


				
				if (sActiveInventory > 0)
				{
					if (MouseInRect(posx, posy, posx+INTERFACE_RATIO(32), posy+INTERFACE_RATIO(32)))
						bQuitCombine = false;
				}

				if (sActiveInventory < player.bag-1)
				{

					float fRatio	= INTERFACE_RATIO(32 + 5) ;
					ARX_CHECK_INT(posy + fRatio);

					posy += ARX_CLEAN_WARN_CAST_INT(fRatio);


					if (MouseInRect(posx, posy, posx+INTERFACE_RATIO(32), posy+INTERFACE_RATIO(32)))
						bQuitCombine = false;
				}
			}
		}

		if (bQuitCombine)
		{
			COMBINE=NULL;
			EERIEMouseButton &= ~1;
		}
	}

		//lights
	if (COMBINE)
	{
		float fMaxdist = 300;

		if (Project.telekinesis) fMaxdist = 850;
		
		for (long i=0;i<MAX_LIGHTS;i++)
		{
			if ((GLight[i]!=NULL) &&
				(GLight[i]->exist) &&

				(EEDistance3D(&GLight[i]->pos, &player.pos) <= fMaxdist) &&
				(!(GLight[i]->extras & EXTRAS_NO_IGNIT)))
			{
				if (MouseInRect(GLight[i]->mins.x, GLight[i]->mins.y, GLight[i]->maxs.x, GLight[i]->maxs.y))
				{
					SpecialCursor = CURSOR_INTERACTION_ON;
				}
			}
		}
	}

	// Double Clicked and not already combining.
	if ((EERIEMouseButton & 4) && (COMBINE==NULL)) 
	{
		long accept_combine=1;

		if ((SecondaryInventory!=NULL) && (InSecondaryInventoryPos(&DANAEMouse)))
		{
			INTERACTIVE_OBJ * io=(INTERACTIVE_OBJ *)SecondaryInventory->io;

			if (io->ioflags & IO_SHOP) accept_combine=0;
		}

		if (accept_combine)
		{
			
			if ((FlyingOverIO) &&
				( (FlyingOverIO->ioflags & IO_ITEM) && !(FlyingOverIO->ioflags & IO_MOVABLE)))
			{
				COMBINE=FlyingOverIO;
				GetInfosCombine();
				EERIEMouseButton&=~4;
			} 
			else if (InInventoryPos(&DANAEMouse)) EERIEMouseButton&=4;
		}
	}

	// Checks for Object Dragging
	if (!EDITMODE)
		if (( DRAGGING && !PLAYER_MOUSELOOK_ON &&
			(!ARX_IMPULSE_Pressed(CONTROLS_CUST_MAGICMODE)) &&
			(DRAGINTER==NULL)
			)
			|| // mode system shock
			( DRAGGING && (pMenuConfig->bAutoReadyWeapon == false) &&
			(!ARX_IMPULSE_Pressed(CONTROLS_CUST_MAGICMODE)) &&
			(DRAGINTER==NULL))
			)
		{
			if ( !TakeFromInventory(&STARTDRAG))
			{
				bool bOk = false;
				
				INTERACTIVE_OBJ *io = InterClick(&STARTDRAG); 

				if (io && !BLOCK_PLAYER_CONTROLS)
				{
					if (ARX_MOUSE_OVER & ARX_MOUSE_OVER_BOOK)
					{
						if (io->show == SHOW_FLAG_ON_PLAYER)
							bOk = true;
					}
					else
					{
						bOk = true;
					}
				}

				if (bOk)
				{
					Set_DragInter(io);

					if (io!=NULL) 
					{
						ARX_PLAYER_Remove_Invisibility();

						if (DRAGINTER->show==SHOW_FLAG_ON_PLAYER)
						{
							ARX_EQUIPMENT_UnEquip(inter.iobj[0],DRAGINTER);
							RemoveFromAllInventories(DRAGINTER);
							DRAGINTER->bbox2.x=-1;
									}

						if (!EDITMODE)
						{	
							if ((io->ioflags & IO_NPC) || (io->ioflags & IO_FIX)) 
							{
								Set_DragInter(NULL);
								goto suivant;
							}
						}

						if (io->ioflags & IO_UNDERWATER)
						{
							io->ioflags&=~IO_UNDERWATER;
							ARX_SOUND_PlayInterface(SND_PLOUF, 0.8F + 0.4F * rnd());
						}

						DRAGINTER->show=SHOW_FLAG_NOT_DRAWN;
						ARX_SOUND_PlayInterface(SND_INVSTD);
					}
				}
				else
					Set_DragInter(NULL);
				
				suivant:
					;
			}
			else ARX_PLAYER_Remove_Invisibility();
		}
		  
}

	// Load Level Command
	if (((EDITMODE)&&((this->kbd.inkey[INKEY_L]) && ((this->kbd.inkey[INKEY_LEFTSHIFT]) || (this->kbd.inkey[INKEY_RIGHTSHIFT])))) || WILLLOADLEVEL)
	{
		char loadfrom[512];

		if (WILLLOADLEVEL != 2 )
		{
			CheckIO_NOT_SAVED();
			ADDED_IO_NOT_SAVED=0;
			WILLLOADLEVEL=0;

			if (OKBox("Erase Current Level ?","Load Level..."))
			{
				strcpy(loadfrom, ""); 
				if (HERMESFileSelectorOpen(loadfrom,"Load Danae Level","Danae Level File (*.DLF)\0*.DLF\0\0",this->m_hWnd))
				{
					char pp[256];
					strcpy(pp,GetName(loadfrom));
					LoadLevelScreen(GDevice,GetLevelNumByName(pp));
					
					Pause(TRUE);

					if (CDP_LIGHTOptions!=NULL) SendMessage(CDP_LIGHTOptions,WM_CLOSE,0,0);

					if (CDP_FogOptions!=NULL) SendMessage(CDP_FogOptions,WM_CLOSE,0,0);

					CDP_LIGHTOptions=NULL;
					CDP_FogOptions=NULL;
					SetEditMode(1);
					DanaeClearLevel();
					PROGRESS_BAR_TOTAL = 108;
					OLD_PROGRESS_BAR_COUNT=PROGRESS_BAR_COUNT=0;
					LoadLevelScreen();
					DanaeLoadLevel(m_pd3dDevice,loadfrom);
					FORBID_SAVE=0;

					FirstFrame=1;
					this->kbd.inkey[INKEY_L]=0;
					this->kbd.inkey[INKEY_LEFTSHIFT]=0;
					this->kbd.inkey[INKEY_RIGHTSHIFT]=0;
					Pause(FALSE);
					return TRUE;
				}
				
			}
		}

		this->kbd.inkey[INKEY_L]=0;
		this->kbd.inkey[INKEY_LEFTSHIFT]=0;
		this->kbd.inkey[INKEY_RIGHTSHIFT]=0;
	}

	// Save Level Command
	if (EDITMODE)
		if ((this->kbd.inkey[INKEY_S]) && ((this->kbd.inkey[INKEY_LEFTSHIFT]) || (this->kbd.inkey[INKEY_RIGHTSHIFT])) || WILLSAVELEVEL)
		{
			WILLSAVELEVEL=0;

			if (FORBID_SAVE)
				ShowPopup("You can't Save Editor Level While a Game is in progress. Please reload an Editor Level to be able to save.");
			else if (OKBox("Save Current Level ?", "Save Level..."))
				{
					char saveto[512];
				strcpy(saveto, ""); 
					Pause(TRUE);
					HERMESFileSelectorSave(saveto,"Save Danae Level","Danae Level File (*.DLF)\0*.DLF\0\0",this->m_hWnd);            
					DanaeSaveLevel(saveto);
					Pause(FALSE);
				}

				this->kbd.inkey[INKEY_S]=0;
				this->kbd.inkey[INKEY_LEFTSHIFT]=0;
				this->kbd.inkey[INKEY_RIGHTSHIFT]=0;
		}

		// PATHWAYS edition Sub-Commands
		if (EDITION==EDITION_PATHWAYS)
		{
			val=10.f;

			if (EERIEMouseButton & 1)
			{
				if (CDP_PATHWAYS_Options) SendMessage(CDP_PATHWAYS_Options,WM_COMMAND,MAKEWPARAM(IDAPPLY,0),0);

				ARX_PATHS_SelectedAP=NULL;
				ARX_PATHS_SelectedNum=-1;

				if ((ARX_PATHS_FlyingOverAP!=NULL) && (ARX_PATHS_FlyingOverNum!=-1))
				{
					ARX_PATHS_SelectedAP=ARX_PATHS_FlyingOverAP;
					ARX_PATHS_SelectedNum=ARX_PATHS_FlyingOverNum;

					if (CDP_PATHWAYS_Options) SendMessage(CDP_PATHWAYS_Options,WM_INITDIALOG,0,0);
				}
			}

			if (this->kbd.inkey[INKEY_PAD8]) 
			{
				if ((ARX_PATHS_SelectedAP!=NULL) && (ARX_PATHS_SelectedNum!=-1))
				{
					trans.x=-(float)EEsin(DEG2RAD(player.angle.b))*val;
					trans.y=0.f;
					trans.z=(float)EEcos(DEG2RAD(player.angle.b))*val;
					ARX_PATHS_ModifyPathWay(ARX_PATHS_SelectedAP,ARX_PATHS_SelectedNum,ARX_PATHS_HIERARCHYMOVE | ARX_PATH_MOD_TRANSLATE,&trans,0,0);
				}

				this->kbd.inkey[INKEY_PAD8]=0;
			}

			if (this->kbd.inkey[INKEY_PAD2]) 
			{
				if ((ARX_PATHS_SelectedAP!=NULL) && (ARX_PATHS_SelectedNum!=-1))
				{
					trans.x=(float)EEsin(DEG2RAD(player.angle.b))*val;
					trans.y=0.f;
					trans.z=-(float)EEcos(DEG2RAD(player.angle.b))*val;
					ARX_PATHS_ModifyPathWay(ARX_PATHS_SelectedAP,ARX_PATHS_SelectedNum,ARX_PATHS_HIERARCHYMOVE | ARX_PATH_MOD_TRANSLATE,&trans,0,0);
				}

				this->kbd.inkey[INKEY_PAD2]=0;
			}

			if (this->kbd.inkey[INKEY_PAD6]) 
			{
				if ((ARX_PATHS_SelectedAP!=NULL) && (ARX_PATHS_SelectedNum!=-1))
				{
					trans.x=-(float)EEsin(DEG2RAD(MAKEANGLE(player.angle.b-90.f)))*val;
					trans.y=0.f;
					trans.z=(float)EEcos(DEG2RAD(MAKEANGLE(player.angle.b-90.f)))*val;
					ARX_PATHS_ModifyPathWay(ARX_PATHS_SelectedAP,ARX_PATHS_SelectedNum,ARX_PATHS_HIERARCHYMOVE | ARX_PATH_MOD_TRANSLATE,&trans,0,0);
				}

				this->kbd.inkey[INKEY_PAD6]=0;
			}		

			if (this->kbd.inkey[INKEY_PAD4]) 
			{
				if ((ARX_PATHS_SelectedAP!=NULL) && (ARX_PATHS_SelectedNum!=-1))
				{
					trans.x=-(float)EEsin(DEG2RAD(MAKEANGLE(player.angle.b+90.f)))*val;
					trans.y=0.f;
					trans.z=(float)EEcos(DEG2RAD(MAKEANGLE(player.angle.b+90.f)))*val;
					ARX_PATHS_ModifyPathWay(ARX_PATHS_SelectedAP,ARX_PATHS_SelectedNum,ARX_PATHS_HIERARCHYMOVE | ARX_PATH_MOD_TRANSLATE,&trans,0,0);
				}

				this->kbd.inkey[INKEY_PAD4]=0;
			}

			if (this->kbd.inkey[INKEY_PADADD]) 
			{
				if ((ARX_PATHS_SelectedAP!=NULL) && (ARX_PATHS_SelectedNum!=-1))
				{
					trans.x=0.f;
					trans.y=-val;
					trans.z=0.f;
					ARX_PATHS_ModifyPathWay(ARX_PATHS_SelectedAP,ARX_PATHS_SelectedNum,ARX_PATHS_HIERARCHYMOVE | ARX_PATH_MOD_TRANSLATE,&trans,0,0);
				}

				this->kbd.inkey[INKEY_PADADD]=0;
			}

			if (this->kbd.inkey[INKEY_PADMINUS]) 
			{
				if ((ARX_PATHS_SelectedAP!=NULL) && (ARX_PATHS_SelectedNum!=-1))
				{
					trans.x=0.f;
					trans.y=val;
					trans.z=0.f;
					ARX_PATHS_ModifyPathWay(ARX_PATHS_SelectedAP,ARX_PATHS_SelectedNum,ARX_PATHS_HIERARCHYMOVE | ARX_PATH_MOD_TRANSLATE,&trans,0,0);
				}

				this->kbd.inkey[INKEY_PADMINUS]=0;
			}

			if (this->kbd.inkey[INKEY_PADDIVIDE]) 
			{
				if ((ARX_PATHS_SelectedAP!=NULL) && (ARX_PATHS_SelectedNum!=-1))
				{
					ARX_PATHS_SelectedNum--;

					if (ARX_PATHS_SelectedNum<1) ARX_PATHS_SelectedNum=ARX_PATHS_SelectedAP->nb_pathways;

					if (CDP_PATHWAYS_Options) SendMessage(CDP_PATHWAYS_Options,WM_INITDIALOG,0,0);
				}

				this->kbd.inkey[INKEY_PADDIVIDE]=0;
			}

			if (this->kbd.inkey[INKEY_PADMULTIPLY]) 
			{
				if ((ARX_PATHS_SelectedAP!=NULL) && (ARX_PATHS_SelectedNum!=-1))
				{
					ARX_PATHS_SelectedNum++;

					if (ARX_PATHS_SelectedNum>ARX_PATHS_SelectedAP->nb_pathways) ARX_PATHS_SelectedNum=1;

					if (CDP_PATHWAYS_Options) SendMessage(CDP_PATHWAYS_Options,WM_INITDIALOG,0,0);
				}

				this->kbd.inkey[INKEY_PADMULTIPLY]=0;
			}

			if (this->kbd.inkey[INKEY_RETURN]) 
			{
				if (danaeApp.m_pFramework->m_bIsFullscreen)
				{
					ARX_TIME_Pause();
					Pause(TRUE);
					DialogBox( (HINSTANCE)GetWindowLong( this->m_hWnd, GWL_HINSTANCE ),
						MAKEINTRESOURCE(IDD_PATHWAYDLG), this->m_hWnd, PathwayOptionsProc);
					Pause(FALSE);
					ARX_TIME_UnPause();				
				}
				else 
					CDP_PATHWAYS_Options=(CreateDialogParam( (HINSTANCE)GetWindowLong( this->m_hWnd, GWL_HINSTANCE ),
					MAKEINTRESOURCE(IDD_PATHWAYDLG), this->m_hWnd, PathwayOptionsProc,0 ));	

				this->kbd.inkey[INKEY_RETURN]=0;
			}

			if (this->kbd.inkey[INKEY_N]) 
			{
				EERIE_3D pos;
				pos.x=player.pos.x-(float)EEsin(DEG2RAD(player.angle.b))*150.f;
				pos.z=player.pos.z+(float)EEcos(DEG2RAD(player.angle.b))*150.f;
				pos.y=player.pos.y+80.f;		

				if (CDP_PATHWAYS_Options) SendMessage(CDP_PATHWAYS_Options,WM_COMMAND,MAKEWPARAM(IDAPPLY,0),0);

				ARX_PATHS_SelectedAP=ARX_PATHS_AddNew(&pos);
				ARX_PATHS_SelectedNum=1;

				if (CDP_PATHWAYS_Options) SendMessage(CDP_PATHWAYS_Options,WM_INITDIALOG,0,0);

				this->kbd.inkey[INKEY_N]=0;
			}

			if (this->kbd.inkey[INKEY_DEL]) 
			{
				if ((ARX_PATHS_SelectedAP!=NULL) && (ARX_PATHS_SelectedNum!=-1))
					ARX_PATHS_DeletePathWay(ARX_PATHS_SelectedAP,ARX_PATHS_SelectedNum);

				this->kbd.inkey[INKEY_DEL]=0;
			}

			if (this->kbd.inkey[INKEY_SPACE]) 
			{
				if ((ARX_PATHS_SelectedAP!=NULL) && (ARX_PATHS_SelectedNum!=-1))
				{
					long v=ARX_PATHS_AddPathWay(ARX_PATHS_SelectedAP,ARX_PATHS_SelectedNum);
					EERIE_3D pos;

					if (v<=2)
					{
						pos.x=-(float)EEsin(DEG2RAD(player.angle.b))*100.f;
						pos.z=+(float)EEcos(DEG2RAD(player.angle.b))*100.f;
					}
					else
					{
						pos.x=ARX_PATHS_SelectedAP->pathways[v-2].rpos.x-(float)EEsin(DEG2RAD(player.angle.b))*100.f;
						pos.z=ARX_PATHS_SelectedAP->pathways[v-2].rpos.z+(float)EEcos(DEG2RAD(player.angle.b))*100.f;
					}

					if (v-1>=0) 
					{
						pos.y=ARX_PATHS_SelectedAP->pathways[v-1].rpos.y;
					}
					else pos.y=0.f;

					ARX_PATHS_SelectedNum=v;
					ARX_PATHS_ModifyPathWay(ARX_PATHS_SelectedAP,v,ARX_PATH_MOD_ALL,&pos,0,2000);
					
				}

				this->kbd.inkey[INKEY_SPACE]=0;
			}
		}

		// LightSources Edition Key/Mouse Management -------------------------------
	if (EDITION == EDITION_LIGHTS) 
		{
			if (EERIEMouseButton & 1)
			{
				PrecalcIOLighting(NULL,0,1);

				for (long i=0;i<MAX_LIGHTS;i++)
				{
					if (GLight[i]!=NULL)
					if (GLight[i]->exist)
						{
							if ( (DANAEMouse.x>=GLight[i]->mins.x) && (DANAEMouse.x<=GLight[i]->maxs.x)
								&& (DANAEMouse.y>=GLight[i]->mins.y) && (DANAEMouse.y<=GLight[i]->maxs.y) )
							{
								if ((CONSTANTUPDATELIGHT) && (CDP_LIGHTOptions) && CDP_EditLight)  
								{
									SendMessage(CDP_LIGHTOptions,WM_COMMAND,IDAPPLY,0);
								}

								EERIE_LIGHT_UnselectAll();						 
								GLight[i]->selected=1;
								LastSelectedLight=i;

								if (CDP_LIGHTOptions)
								{
									memcpy(&lightparam,GLight[LastSelectedLight],sizeof(EERIE_LIGHT));
									CDP_EditLight=GLight[LastSelectedLight];
									SendMessage(CDP_LIGHTOptions,WM_INITDIALOG,0,0);
									
								}

								EERIEMouseButton &=~1;
							}
						}
				}
			}

			if (this->kbd.inkey[INKEY_DEL]) 
			{
				PrecalcIOLighting(NULL,0,1);

				if ( (this->kbd.inkey[INKEY_LEFTSHIFT]) || (this->kbd.inkey[INKEY_RIGHTSHIFT]))
				{
					if (OKBox("Destroy ALL Lights from this level ???","GAIA Popup"))
					{
						if (CDP_LIGHTOptions)
						{
							SendMessage(CDP_LIGHTOptions,WM_CLOSE,0,0);
							CDP_LIGHTOptions=NULL;
						}

						EERIE_LIGHT_ClearAll();
					}

					this->kbd.inkey[INKEY_LEFTSHIFT]=0;
					this->kbd.inkey[INKEY_RIGHTSHIFT]=0;
				}
				else if (OKBox("Destroy Selected Light ?","GAIA Popup"))
				{
					if (CDP_LIGHTOptions)
					{
						SendMessage(CDP_LIGHTOptions,WM_CLOSE,0,0);
						CDP_LIGHTOptions=NULL;
					}

					EERIE_LIGHT_ClearSelected();
					LastSelectedLight=-1;
					RecalcLightZone(player.pos.x,player.pos.y,player.pos.z,12);
				}

				this->kbd.inkey[INKEY_DEL]=0;
			}

			val=10.f;

			if ((this->kbd.inkey[INKEY_LEFTSHIFT]) || (this->kbd.inkey[INKEY_RIGHTSHIFT])) 
				val=2.f;
			

			if (this->kbd.inkey[INKEY_RETURN]) 
			{
				PrecalcIOLighting(NULL,0,1);
				launchlightdialog();
				this->kbd.inkey[INKEY_RETURN]=0;
			}

			if (this->kbd.inkey[INKEY_SPACE]) 
			{
				PrecalcIOLighting(NULL,0,1);

				if ((CONSTANTUPDATELIGHT) && (CDP_LIGHTOptions) && CDP_EditLight)  
				{
					SendMessage(CDP_LIGHTOptions,WM_COMMAND,IDAPPLY,0);
				}
				
				long num = EERIE_LIGHT_Create();

				if (num != -1)
				{
					GLight[num]->exist = 1;
					GLight[num]->fallend = 600.f;
					GLight[num]->fallstart = 400.f;
					GLight[num]->intensity = 1.2f;
					GLight[num]->maxs.x = -1;
					GLight[num]->mins.x = -1;
					GLight[num]->pos.x = player.pos.x - (float)EEsin(DEG2RAD(player.angle.b)) * 150.f;
					GLight[num]->pos.z = player.pos.z + (float)EEcos(DEG2RAD(player.angle.b)) * 150.f;
					GLight[num]->pos.y = player.pos.y + 80.f;
					GLight[num]->rgb.r = 1.f;
					GLight[num]->rgb.g = 0.f;
					GLight[num]->rgb.b = 0.f;
					GLight[num]->selected = 1;
					GLight[num]->extras |= EXTRAS_NOCASTED;
					GLight[num]->tl = -1;
					GLight[num]->status = 1;
					EERIE_LIGHT_UnselectAll();
					LastSelectedLight = num;
					
					if (CDP_LIGHTOptions)
					{
						memcpy(&lightparam,GLight[LastSelectedLight],sizeof(EERIE_LIGHT));
						CDP_EditLight=GLight[LastSelectedLight];
						SendMessage(CDP_LIGHTOptions,WM_INITDIALOG,0,0);
					}
					
					GLight[num]->treat = 1;
					RecalcLightZone(GLight[LastSelectedLight]->pos.x,GLight[LastSelectedLight]->pos.y,GLight[LastSelectedLight]->pos.z,(long)(GLight[LastSelectedLight]->fallend*ACTIVEBKG->Xmul)+1);
				}
				
				this->kbd.inkey[INKEY_SPACE]=0;
			}

			if (this->kbd.inkey[INKEY_PAD8]) 
			{
				PrecalcIOLighting(NULL,0,1);	
				trans.x=-(float)EEsin(DEG2RAD(player.angle.b))*val;
				trans.y=0.f;
				trans.z=(float)EEcos(DEG2RAD(player.angle.b))*val;
				EERIE_LIGHT_TranslateSelected(&trans);
				this->kbd.inkey[INKEY_PAD8]=0;
			}

			if (this->kbd.inkey[INKEY_PAD2]) 
			{
				PrecalcIOLighting(NULL,0,1);
				trans.x=(float)EEsin(DEG2RAD(player.angle.b))*val;
				trans.y=0.f;
				trans.z=-(float)EEcos(DEG2RAD(player.angle.b))*val;
				EERIE_LIGHT_TranslateSelected(&trans);
				this->kbd.inkey[INKEY_PAD2]=0;
			}

			if (this->kbd.inkey[INKEY_PAD6]) 
			{
				PrecalcIOLighting(NULL,0,1);
				trans.x=-(float)EEsin(DEG2RAD(MAKEANGLE(player.angle.b-90.f)))*val;
				trans.y=0.f;
				trans.z=(float)EEcos(DEG2RAD(MAKEANGLE(player.angle.b-90.f)))*val;
				EERIE_LIGHT_TranslateSelected(&trans);
				this->kbd.inkey[INKEY_PAD6]=0;
			}		

			if (this->kbd.inkey[INKEY_PAD4]) 
			{
				PrecalcIOLighting(NULL,0,1);
				trans.x=-(float)EEsin(DEG2RAD(MAKEANGLE(player.angle.b+90.f)))*val;
				trans.y=0.f;
				trans.z=(float)EEcos(DEG2RAD(MAKEANGLE(player.angle.b+90.f)))*val;
				EERIE_LIGHT_TranslateSelected(&trans);
				this->kbd.inkey[INKEY_PAD4]=0;
			}

			if (this->kbd.inkey[INKEY_PADADD]) 
			{
				PrecalcIOLighting(NULL,0,1);
			trans.x = 0.f;
			trans.y = -val;
			trans.z = 0.f;
				EERIE_LIGHT_TranslateSelected(&trans);
				this->kbd.inkey[INKEY_PADADD]=0;
			}

			if (this->kbd.inkey[INKEY_PADMINUS]) 
			{
				PrecalcIOLighting(NULL,0,1);
			trans.x = 0.f;
			trans.y = val;
			trans.z = 0.f;
				EERIE_LIGHT_TranslateSelected(&trans);
				this->kbd.inkey[INKEY_PADMINUS]=0;
			}
	}
	
	// Particles Edition Key/Mouse Management -------------------------------
	if ((EDITION==EDITION_PARTICLES) && EDITMODE)
	{
		if (EERIEMouseButton & 1)
		{
		}
		
		if (this->kbd.inkey[INKEY_DEL]) 
		{
			if (OKBox("Destroy Selected Particle ?","GAIA Popup"))
			{
			}

			this->kbd.inkey[INKEY_DEL]=0;
		}

		val=10.f;

		if ((this->kbd.inkey[INKEY_LEFTSHIFT]) || (this->kbd.inkey[INKEY_RIGHTSHIFT])) 
			val=2.f;
		
		if (this->kbd.inkey[INKEY_RETURN]) 
		{
			// Launch Particle Dialog
			this->kbd.inkey[INKEY_RETURN]=0;
		}

		if (this->kbd.inkey[INKEY_SPACE]) 
		{
			// Create a Particle FX
			this->kbd.inkey[INKEY_SPACE]=0;
		}

		if (this->kbd.inkey[INKEY_PAD8]) 
		{
			// Translate FRONT
			trans.x=-(float)EEsin(DEG2RAD(player.angle.b))*val;
			trans.y=0.f;
			trans.z=(float)EEcos(DEG2RAD(player.angle.b))*val;
			this->kbd.inkey[INKEY_PAD8]=0;
		}

		if (this->kbd.inkey[INKEY_PAD2]) 
		{
			//	translate BACK
			trans.x=(float)EEsin(DEG2RAD(player.angle.b))*val;
			trans.y=0.f;
			trans.z=-(float)EEcos(DEG2RAD(player.angle.b))*val;
			this->kbd.inkey[INKEY_PAD2]=0;
		}

		if (this->kbd.inkey[INKEY_PAD6]) 
		{
			//	translate RIGHT
			trans.x=-(float)EEsin(DEG2RAD(MAKEANGLE(player.angle.b-90.f)))*val;
			trans.y=0.f;
			trans.z=(float)EEcos(DEG2RAD(MAKEANGLE(player.angle.b-90.f)))*val;
			this->kbd.inkey[INKEY_PAD6]=0;
		}		

		if (this->kbd.inkey[INKEY_PAD4]) 
		{
			// translate LEFT
			trans.x=-(float)EEsin(DEG2RAD(MAKEANGLE(player.angle.b+90.f)))*val;
			trans.y=0.f;
			trans.z=(float)EEcos(DEG2RAD(MAKEANGLE(player.angle.b+90.f)))*val;
			this->kbd.inkey[INKEY_PAD4]=0;
		}

		if (this->kbd.inkey[INKEY_PADADD]) 
		{
			//	translate UP
			trans.x = 0.f;
			trans.y = -val;
			trans.z = 0.f;
			this->kbd.inkey[INKEY_PADADD]=0;
		}

		if (this->kbd.inkey[INKEY_PADMINUS]) 
		{
			//	translate DOWN
			trans.x = 0.f;
			trans.y = val;
			trans.z = 0.f;
			this->kbd.inkey[INKEY_PADMINUS]=0;
		}
	}
	
	// NODES Edition Key/Mouse Management -------------------------------
	if ((EDITION==EDITION_NODES) && (EDITMODE))
	{
		if (EERIEMouseButton & 1) 
		{
			for (long i=0;i<nodes.nbmax;i++)
			{
				if (nodes.nodes[i].exist)
				{
					if ( (DANAEMouse.x>=nodes.nodes[i].bboxmin.x) && (DANAEMouse.x<=nodes.nodes[i].bboxmax.x)
						&& (DANAEMouse.y>=nodes.nodes[i].bboxmin.y) && (DANAEMouse.y<=nodes.nodes[i].bboxmax.y) )
					{
						UnselectAllNodes();
						SelectNode(i);
						LastSelectedNode=i;
						EERIEMouseButton &=~1;
					}
				}
			}
		}

		if (!(this->kbd.inkey[INKEY_LEFTSHIFT]) && !(this->kbd.inkey[INKEY_RIGHTSHIFT])) 
			if ((this->kbd.inkey[INKEY_L]) && (LastSelectedNode!=-1))
			{
				for (long i=0;i<nodes.nbmax;i++)
				{
					if (nodes.nodes[i].exist)
					{
						if ( (DANAEMouse.x>=nodes.nodes[i].bboxmin.x) && (DANAEMouse.x<=nodes.nodes[i].bboxmax.x)
							&& (DANAEMouse.y>=nodes.nodes[i].bboxmin.y) && (DANAEMouse.y<=nodes.nodes[i].bboxmax.y) )
						{
							LinkNodeToNode(i,LastSelectedNode);
						}
					}
				}

				this->kbd.inkey[INKEY_L]=0;
			}

			if ((this->kbd.inkey[INKEY_U]) && (LastSelectedNode!=-1))
			{
				for (long i=0;i<nodes.nbmax;i++)
				{
					if (nodes.nodes[i].exist)
					{
						if ( (DANAEMouse.x>=nodes.nodes[i].bboxmin.x) && (DANAEMouse.x<=nodes.nodes[i].bboxmax.x)
							&& (DANAEMouse.y>=nodes.nodes[i].bboxmin.y) && (DANAEMouse.y<=nodes.nodes[i].bboxmax.y) )
						{
							UnLinkNodeFromNode(i,LastSelectedNode);
						}
					}
				}

				this->kbd.inkey[INKEY_U]=0;
			}

			float val=10.f;

			if ((this->kbd.inkey[INKEY_LEFTSHIFT]) || (this->kbd.inkey[INKEY_RIGHTSHIFT])) 
			{
				val=1.f;				
			}

			if (this->kbd.inkey[INKEY_RETURN])
			{
				for (long i=0;i<nodes.nbmax;i++)
				{
					if ((nodes.nodes[i].exist) && (nodes.nodes[i].selected))
					{
						char temp[64];
				encore:
					;
	   strcpy(temp,nodes.nodes[i].name);
	   TextBox("Change Node Name",temp,63);

	   if ((ExistNodeName(temp)) && strcmp(temp,nodes.nodes[i].name))
	   {
		   ShowPopup("This Name already exists, change to new name please");
		   goto encore;
	   }

	   strcpy(nodes.nodes[i].name,temp);
					}
				}

				this->kbd.inkey[INKEY_RETURN]=0;
			}

			if (this->kbd.inkey[INKEY_N])
			{
				long n=GetFreeNode();

				if (n!=-1)
				{
					ClearNode(n,0);
					nodes.nodes[n].exist=1;
					nodes.nodes[n].pos.x=player.pos.x-(float)EEsin(DEG2RAD(player.angle.b))*150.f;
					nodes.nodes[n].pos.z=player.pos.z+(float)EEcos(DEG2RAD(player.angle.b))*150.f;
					nodes.nodes[n].pos.y=player.pos.y+80.f;
					MakeNodeName(n);
					UnselectAllNodes();
					SelectNode(n);
					LastSelectedNode=n;
					EERIEPOLY * ep=CheckInPoly(nodes.nodes[n].pos.x,nodes.nodes[n].pos.y+PLAYER_BASE_HEIGHT,nodes.nodes[n].pos.z);

					if (ep!=NULL)
					{
						float tempo;

						if (GetTruePolyY(ep,&nodes.nodes[n].pos,&tempo))
							nodes.nodes[n].pos.y=tempo-80.f;
					}
				}

				this->kbd.inkey[INKEY_N]=0;
			}

			if (this->kbd.inkey[INKEY_DEL]) 
			{
				ClearSelectedNodes();
				this->kbd.inkey[INKEY_DEL]=0;
			}

			if (this->kbd.inkey[INKEY_PAD8]) 
			{
				float ag=GetNearestSnappedAngle(player.angle.b);
				ag=DEG2RAD(ag);
				trans.x=-(float)EEsin(ag)*val;
				trans.y=0.f;
				trans.z=(float)EEcos(ag)*val;
				TranslateSelectedNodes(&trans);
				this->kbd.inkey[INKEY_PAD8]=0;
			}

			if (this->kbd.inkey[INKEY_PAD2]) 
			{
				float ag=GetNearestSnappedAngle(player.angle.b);
				ag=DEG2RAD(ag);
				trans.x=(float)EEsin(ag)*val;
				trans.y=0.f;
				trans.z=-(float)EEcos(ag)*val;
				TranslateSelectedNodes(&trans);
				this->kbd.inkey[INKEY_PAD2]=0;
			}

			if (this->kbd.inkey[INKEY_PAD6]) 
			{
				float ag=GetNearestSnappedAngle(MAKEANGLE(player.angle.b-90.f));
				ag=DEG2RAD(ag);
				trans.x=-(float)EEsin(ag)*val;
				trans.y=0.f;
				trans.z=(float)EEcos(ag)*val;
				TranslateSelectedNodes(&trans);
				this->kbd.inkey[INKEY_PAD6]=0;
			}		

			if (this->kbd.inkey[INKEY_PAD4]) 
			{
				float ag=GetNearestSnappedAngle(MAKEANGLE(player.angle.b+90.f));
				ag=DEG2RAD(ag);
				trans.x=-(float)EEsin(ag)*val;
				trans.y=0.f;
				trans.z=(float)EEcos(ag)*val;
				TranslateSelectedNodes(&trans);
				this->kbd.inkey[INKEY_PAD4]=0;
			}

			if (this->kbd.inkey[INKEY_PADADD]) 
			{
				trans.x=0.f;
				trans.y=-val;
				trans.z=0.f;
				TranslateSelectedNodes(&trans);
				this->kbd.inkey[INKEY_PADADD]=0;
			}

			if (this->kbd.inkey[INKEY_PADMINUS]) 
			{
				trans.x=0.f;
				trans.y=val;
				trans.z=0.f;
				TranslateSelectedNodes(&trans);
				this->kbd.inkey[INKEY_PADMINUS]=0;
			}
	}
	
	
	
	// Fog Key/Mouse Management -------------------------------
	if ((EDITION==EDITION_FOGS) && (EDITMODE))
	{
		if (EERIEMouseButton & 1) 
		{
			for (long i=0;i<MAX_FOG;i++)
			{
				if (fogs[i].exist)
				{
					if ( (DANAEMouse.x>=fogs[i].bboxmin.x) && (DANAEMouse.x<=fogs[i].bboxmax.x)
						&& (DANAEMouse.y>=fogs[i].bboxmin.y) && (DANAEMouse.y<=fogs[i].bboxmax.y) )
					{
						ARX_FOGS_UnselectAll();
						ARX_FOGS_Select(i);
						LastSelectedFog=i;						

						if (CDP_FogOptions)
						{
							memcpy(&fogparam,&fogs[LastSelectedFog],sizeof(FOG_DEF));
							CDP_EditFog=&fogs[LastSelectedFog];
							SendMessage(CDP_FogOptions,WM_INITDIALOG,0,0);
						}

						EERIEMouseButton &=~1;
					}
				}
			}
		}

		if ( !( this->kbd.inkey[INKEY_LEFTSHIFT] ) && !( this->kbd.inkey[INKEY_RIGHTSHIFT] ) ) 
		{
			val=10.f;
		}

		if ((this->kbd.inkey[INKEY_LEFTSHIFT]) || (this->kbd.inkey[INKEY_RIGHTSHIFT])) 
		{
			val=1.f;

			if ((LastSelectedFog!=-1) && (fogs[LastSelectedFog].special & FOG_DIRECTIONAL)
				&& ((EERIEMouseXdep) || (EERIEMouseYdep)))
			{
				fogs[LastSelectedFog].angle.a+=EERIEMouseYdep;
				fogs[LastSelectedFog].angle.b+=EERIEMouseXdep;
				
				fogs[LastSelectedFog].move.x=1.f;
				fogs[LastSelectedFog].move.y=0.f;
				fogs[LastSelectedFog].move.z=0.f;
				EERIE_3D out;
				_YRotatePoint(&fogs[LastSelectedFog].move,&out,EEcos(DEG2RAD(MAKEANGLE(fogs[LastSelectedFog].angle.b))),EEsin(DEG2RAD(MAKEANGLE(fogs[LastSelectedFog].angle.b))));
				_XRotatePoint(&out,&fogs[LastSelectedFog].move,EEcos(DEG2RAD(MAKEANGLE(fogs[LastSelectedFog].angle.a))),EEsin(DEG2RAD(MAKEANGLE(fogs[LastSelectedFog].angle.a))));
				
				
			}
		}

		if (this->kbd.inkey[INKEY_RETURN])
		{
			if ((LastSelectedFog>-1) && (!CDP_FogOptions))
			{
				
				memcpy(&fogparam,&fogs[LastSelectedFog],sizeof(FOG_DEF));
				CDP_EditFog=&fogs[LastSelectedFog];

				if (danaeApp.m_pFramework->m_bIsFullscreen)
				{
					ARX_TIME_Pause();
					Pause(TRUE);
					DialogBox( (HINSTANCE)GetWindowLong( this->m_hWnd, GWL_HINSTANCE ),
						MAKEINTRESOURCE(IDD_FOGDIALOG), this->m_hWnd, FogOptionsProc);
					Pause(FALSE);
					ARX_TIME_UnPause();				
				}
				else 
					CDP_FogOptions=(CreateDialogParam( (HINSTANCE)GetWindowLong( this->m_hWnd, GWL_HINSTANCE ),
					MAKEINTRESOURCE(IDD_FOGDIALOG), this->m_hWnd, FogOptionsProc,0 ));
			}

			this->kbd.inkey[INKEY_RETURN]=0;
		}

		if (this->kbd.inkey[INKEY_SPACE])
		{
			long n=ARX_FOGS_GetFree();

			if (n!=-1)
			{
				fogs[n].exist=1;
				ARX_FOGS_UnselectAll();
				ARX_FOGS_Select(n);
				LastSelectedFog=n;
				fogs[n].pos.x=player.pos.x-(float)EEsin(DEG2RAD(player.angle.b))*150.f;
				fogs[n].pos.z=player.pos.z+(float)EEcos(DEG2RAD(player.angle.b))*150.f;
				fogs[n].pos.y=player.pos.y+80.f;
				EERIEPOLY * ep=CheckInPoly(fogs[n].pos.x,fogs[n].pos.y+PLAYER_BASE_HEIGHT,fogs[n].pos.z);

				if (ep!=NULL)
				{
					float tempo;

					if (GetTruePolyY(ep,&fogs[n].pos,&tempo))
						fogs[n].pos.y=tempo-80.f;
				} 

				fogs[n].special=0;
				fogs[n].frequency=17.f;
				fogs[n].speed=1.f;
				fogs[n].move.x=1.f;
				fogs[n].move.y=0.f;
				fogs[n].move.z=0.f;
				fogs[n].rgb.r=0.3f;
				fogs[n].rgb.g=0.3f;
				fogs[n].rgb.b=0.5f;
				fogs[n].size=80.f;
				fogs[n].tolive=4500;
				fogs[n].scale=8.f;
				fogs[n].blend=0;
				fogs[n].angle.a=0.f;
				fogs[n].angle.b=0.f;
				fogs[n].rotatespeed=0.001f;

				if (CDP_FogOptions)
				{
					memcpy(&fogparam,&fogs[LastSelectedFog],sizeof(FOG_DEF));
					CDP_EditFog=&fogs[LastSelectedFog];
					SendMessage(CDP_FogOptions,WM_INITDIALOG,0,0);
				}
			}

			this->kbd.inkey[INKEY_SPACE]=0;
		}

		if (this->kbd.inkey[INKEY_DEL]) 
		{
			if (OKBox("Destroy Selected Fog ?","GAIA Popup"))
			{
				if (CDP_FogOptions)
				{
					SendMessage(CDP_FogOptions,WM_CLOSE,0,0);
					CDP_FogOptions=NULL;
				}

				ARX_FOGS_KillSelected();
			}

			this->kbd.inkey[INKEY_DEL]=0;
		}

		if ( this->kbd.inkey[INKEY_PAD8] ) 
		{
			trans.x = -(float)EEsin( DEG2RAD( player.angle.b ) ) * val;
			trans.y = 0.f;
			trans.z = (float)EEcos( DEG2RAD( player.angle.b ) ) * val;
			ARX_FOGS_TranslateSelected( &trans );
			this->kbd.inkey[INKEY_PAD8] = 0;
		}

		if (this->kbd.inkey[INKEY_PAD2]) 
		{
			EERIE_3D trans;
			trans.x=(float)EEsin(DEG2RAD(player.angle.b))*val;
			trans.y=0.f;
			trans.z=-(float)EEcos(DEG2RAD(player.angle.b))*val;
			ARX_FOGS_TranslateSelected(&trans);
			this->kbd.inkey[INKEY_PAD2]=0;
		}

		if (this->kbd.inkey[INKEY_PAD6]) 
		{
			EERIE_3D trans;
			trans.x=-(float)EEsin(DEG2RAD(MAKEANGLE(player.angle.b-90.f)))*val;
			trans.y=0.f;
			trans.z=(float)EEcos(DEG2RAD(MAKEANGLE(player.angle.b-90.f)))*val;
			ARX_FOGS_TranslateSelected(&trans);
			this->kbd.inkey[INKEY_PAD6]=0;
		}		

		if (this->kbd.inkey[INKEY_PAD4]) 
		{
			EERIE_3D trans;
			trans.x=-(float)EEsin(DEG2RAD(MAKEANGLE(player.angle.b+90.f)))*val;
			trans.y=0.f;
			trans.z=(float)EEcos(DEG2RAD(MAKEANGLE(player.angle.b+90.f)))*val;
			ARX_FOGS_TranslateSelected(&trans);
			this->kbd.inkey[INKEY_PAD4]=0;
		}

		if (this->kbd.inkey[INKEY_PADADD]) 
		{
			EERIE_3D trans;
			trans.x=0.f;
			trans.y=-val;
			trans.z=0.f;
			ARX_FOGS_TranslateSelected(&trans);
			this->kbd.inkey[INKEY_PADADD]=0;
		}

		if (this->kbd.inkey[INKEY_PADMINUS]) 
		{
			EERIE_3D trans;
			trans.x=0.f;
			trans.y=val;
			trans.z=0.f;
			ARX_FOGS_TranslateSelected(&trans);
			this->kbd.inkey[INKEY_PADMINUS]=0;
		}
		
	}
	
	// IO Edition Key/Mouse Management -------------------------------	
	if (EDITMODE)
	{ 
		float val;

		if ((this->kbd.inkey[INKEY_LEFTSHIFT]) || (this->kbd.inkey[INKEY_RIGHTSHIFT]))
		{
			val=1.f;		
		}
		else val=20.f;

		if (this->kbd.inkey[INKEY_HOME]) 
		{
			ResetSelectedIORot();
			this->kbd.inkey[INKEY_HOME]=0;
		}

		if (this->kbd.inkey[INKEY_DEL]) 
		{
			DeleteSelectedIO();
			this->kbd.inkey[INKEY_DEL]=0;
		}

		if (this->kbd.inkey[INKEY_PAD7]) 
		{
			GroundSnapSelectedIO();
			this->kbd.inkey[INKEY_PAD7]=0;
		}

		if (this->kbd.inkey[INKEY_PAD0]) 
		{
			if (ValidIONum(LastSelectedIONum))
			{
				inter.iobj[LastSelectedIONum]->angle.a=0.f;
				inter.iobj[LastSelectedIONum]->angle.b=0.f;
				inter.iobj[LastSelectedIONum]->angle.g=0.f;
				inter.iobj[LastSelectedIONum]->initangle.a=0.f;
				inter.iobj[LastSelectedIONum]->initangle.b=0.f;
				inter.iobj[LastSelectedIONum]->initangle.g=0.f;
			}

			this->kbd.inkey[INKEY_PAD0]=0;
		}

		if (this->kbd.inkey[INKEY_PAD8]) 
		{
			EERIE_3D trans;
			float ag=GetNearestSnappedAngle(player.angle.b);
			ag=DEG2RAD(ag);
			trans.x=-(float)EEsin(ag)*val;
			trans.y=0.f;
			trans.z=(float)EEcos(ag)*val;
			TranslateSelectedIO(&trans);
			this->kbd.inkey[INKEY_PAD8]=0;
		}

		if (this->kbd.inkey[INKEY_PAD2]) 
		{
			EERIE_3D trans;
			float ag=GetNearestSnappedAngle(player.angle.b);
			ag=DEG2RAD(ag);
			trans.x=(float)EEsin(ag)*val;
			trans.y=0.f;
			trans.z=-(float)EEcos(ag)*val;
			TranslateSelectedIO(&trans);
			this->kbd.inkey[INKEY_PAD2]=0;
		}

		if (this->kbd.inkey[INKEY_PAD6]) 
		{
			EERIE_3D trans;
			float ag=GetNearestSnappedAngle(MAKEANGLE(player.angle.b-90.f));
			ag=DEG2RAD(ag);
			trans.x=-(float)EEsin(ag)*val;
			trans.y=0.f;
			trans.z=(float)EEcos(ag)*val;
			TranslateSelectedIO(&trans);
			this->kbd.inkey[INKEY_PAD6]=0;
		}		

		if (this->kbd.inkey[INKEY_PAD4]) 
		{
			EERIE_3D trans;
			float ag=GetNearestSnappedAngle(MAKEANGLE(player.angle.b+90.f));
			ag=DEG2RAD(ag);
			trans.x=-(float)EEsin(ag)*val;
			trans.y=0.f;
			trans.z=(float)EEcos(ag)*val;
			TranslateSelectedIO(&trans);
			this->kbd.inkey[INKEY_PAD4]=0;
		}

		if (this->kbd.inkey[INKEY_PADADD]) 
		{
			EERIE_3D trans;
			trans.x=0.f;
			trans.y=-val;
			trans.z=0.f;
			TranslateSelectedIO(&trans);
			this->kbd.inkey[INKEY_PADADD]=0;
		}

		if (this->kbd.inkey[INKEY_PADMINUS]) 
		{
			EERIE_3D trans;
			trans.x=0.f;
			trans.y=val;
			trans.z=0.f;
			TranslateSelectedIO(&trans);
			this->kbd.inkey[INKEY_PADMINUS]=0;
		}

		if (this->kbd.inkey[INKEY_PAD9]) 
		{
			ObjectRotAxis++;

			if (ObjectRotAxis>2) ObjectRotAxis=0;

			this->kbd.inkey[INKEY_PAD9]=0;
		}

		if (val!=1.f) val=45.f;

		if (this->kbd.inkey[INKEY_PADMULTIPLY]) 
		{
			EERIE_3D rot;
			rot.a=rot.b=rot.g=0.f;

			if (ObjectRotAxis==0) rot.b=val;
			else if (ObjectRotAxis==1) rot.a=val;
			else rot.g=val;

			RotateSelectedIO(&rot);
			this->kbd.inkey[INKEY_PADMULTIPLY]=0;
		}

		if (this->kbd.inkey[INKEY_PADDIVIDE]) 
		{
			EERIE_3D rot;
			rot.a=rot.b=rot.g=0.f;

			if (ObjectRotAxis==0) rot.b=-val;
			else if (ObjectRotAxis==1) rot.a=-val;
			else rot.g=-val;

			RotateSelectedIO(&rot);
			this->kbd.inkey[INKEY_PADDIVIDE]=0;
		}
	}
	
	
	if (EDITMODE)
		if ((this->kbd.inkey[INKEY_RETURN])  
			&& (ValidIONum(LastSelectedIONum)))
		{
			//CDP_IOOptions			
			if (CDP_IOOptions)
			{
				SendMessage(CDP_IOOptions,WM_COMMAND,MAKELONG(IDOK,0),0);
			}

			if (!CDP_IOOptions)
			{
				CDP_EditIO=inter.iobj[LastSelectedIONum];

				if (danaeApp.m_pFramework->m_bIsFullscreen)
				{
					ARX_TIME_Pause();
					Pause(TRUE);
					DialogBox( (HINSTANCE)GetWindowLong( this->m_hWnd, GWL_HINSTANCE ),
						MAKEINTRESOURCE(IDD_SCRIPTDIALOG), this->m_hWnd, IOOptionsProc);
					Pause(FALSE);
					ARX_TIME_UnPause();				
				}
				else 
					CDP_IOOptions=(CreateDialogParam( (HINSTANCE)GetWindowLong( this->m_hWnd, GWL_HINSTANCE ),
					MAKEINTRESOURCE(IDD_SCRIPTDIALOG), this->m_hWnd, IOOptionsProc,0 ));
			}

			this->kbd.inkey[INKEY_RETURN]=0;
		}
		
		return FALSE;
}

//-----------------------------------------------------------------------------
// Switches from/to combat Mode i=-1 for switching from actual configuration
// 2 to force Draw Weapon
// 1 to force Combat Mode On
// 0 to force Combat Mode Off
//-----------------------------------------------------------------------------
void ARX_INTERFACE_Combat_Mode(long i)
{
	if ( (i>=1) &&  (player.Interface & INTER_COMBATMODE) ) return;

	if ( (i==0) &&  !(player.Interface & INTER_COMBATMODE) ) return;

	if (EDITMODE) return;

	if  ((player.Interface & INTER_COMBATMODE) && (inter.iobj[0]))
	{
		player.Interface&=~INTER_COMBATMODE;
		player.Interface&=~INTER_NO_STRIKE;
 
		
		ARX_EQUIPMENT_LaunchPlayerUnReadyWeapon();
		long weapontype=ARX_EQUIPMENT_GetPlayerWeaponType();

		if (inter.iobj[0] && arrowobj && (weapontype == WEAPON_BOW))
		{
			EERIE_LINKEDOBJ_UnLinkObjectFromObject(inter.iobj[0]->obj, arrowobj);
		}

		player.doingmagic=0;
	}

	else  if (((inter.iobj[0]->animlayer[1].cur_anim == NULL) || (inter.iobj[0]->animlayer[1].cur_anim == inter.iobj[0]->anims[ANIM_WAIT])) 
		&& (inter.iobj[0]))
	{
		ARX_INTERFACE_BookOpenClose(2);

		player.Interface|=INTER_COMBATMODE;

		if (i==2)
		{
			player.Interface|=INTER_NO_STRIKE;

			if (pMenuConfig->bMouseLookToggle)
			{
				TRUE_PLAYER_MOUSELOOK_ON |= 1;
				SLID_START=(float)ARXTime;
			}
		}

 

		ARX_EQUIPMENT_LaunchPlayerReadyWeapon();
		player.doingmagic=0;	
	}		
}
long CSEND=0;
extern void ApplySPArm();
long MOVE_PRECEDENCE=0;

extern long DISABLE_JUMP;
extern unsigned long REQUEST_JUMP;
//-----------------------------------------------------------------------------
void DANAE::ManagePlayerControls()
{
	
	if (((EERIEMouseButton & 4) && (!EDITMODE) && !(player.Interface & INTER_COMBATMODE)) 
		&& (!player.doingmagic)
		&& (!(ARX_MOUSE_OVER & ARX_MOUSE_OVER_BOOK))
		&& (eMouseState != MOUSE_IN_NOTE)
		)
	{

		INTERACTIVE_OBJ * t;
		
		t=InterClick(&DANAEMouse); 

		if (t!=NULL)
		{
			if (t->ioflags & IO_NPC) 
			{
				if (t->script.data!=NULL)
				{
					if (t->_npcdata->life>0.f)
					{
						SendIOScriptEvent(t,SM_CHAT,"");
						EERIEMouseButton&=~4;

						if (DRAGGING) DRAGGING = 0;
					}
					else 
					{
						if (t->inventory!=NULL)
						{
							if (player.Interface & INTER_STEAL)
								if (ioSteal && t != ioSteal)
								{
									SendIOScriptEvent(ioSteal, SM_STEAL,"OFF");
									player.Interface &= ~INTER_STEAL;
								}

							ARX_INVENTORY_OpenClose(t);

							if (player.Interface&(INTER_INVENTORY | INTER_INVENTORYALL))
							{
								ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
							}

							if (INTERNATIONAL_MODE)
							{
								if (SecondaryInventory)
								{
									bForceEscapeFreeLook=true;
								    lOldTruePlayerMouseLook=!TRUE_PLAYER_MOUSELOOK_ON;
								}
							}
						}							
					}
				}
			}
			else
			{
				if (t->inventory!=NULL)
				{
					if (player.Interface & INTER_STEAL)
						if (ioSteal && t != ioSteal)
						{
							SendIOScriptEvent(ioSteal, SM_STEAL,"OFF");
							player.Interface &= ~INTER_STEAL;
						}

					ARX_INVENTORY_OpenClose(t);

					if (INTERNATIONAL_MODE)
					{
						if (SecondaryInventory)
						{
							bForceEscapeFreeLook=true;
						    lOldTruePlayerMouseLook=!TRUE_PLAYER_MOUSELOOK_ON;
						}
					}
				}
				else if (t->script.data!=NULL)
					SendIOScriptEvent(t,SM_ACTION,"");

				EERIEMouseButton&=~4;

				if (DRAGGING) DRAGGING = 0;

				EERIEMouseButton = 0;
			}

			EERIEMouseButton&=~4;
			EERIEMouseButton = 0;
		}
	}

	long NOMOREMOVES=0;
	float MoveDiv;
	float FD;
	FD = 1.f;
	

	EERIE_3D tm;
	tm.x=tm.y=tm.z=0.f;

	// Checks STEALTH Key Status.
	if (ARX_IMPULSE_Pressed(CONTROLS_CUST_STEALTHMODE) )
	{			
		MoveDiv=0.02f;
		player.Current_Movement|=PLAYER_MOVE_STEALTH;
	}
	else MoveDiv=0.0333333f; 

	{
		float tr;
		
		if (eyeball.exist==2) 
		{
			FD=18.f;

			EERIE_3D old;
			Vector_Copy(&old,&eyeball.pos);
			
			// Checks WALK_FORWARD Key Status.
			if (ARX_IMPULSE_Pressed(CONTROLS_CUST_WALKFORWARD) )
			{
				tr=DEG2RAD(eyeball.angle.b);
				eyeball.pos.x+=-(float)EEsin(tr)*20.f*(float)FD*0.033f;
				eyeball.pos.z+=+(float)EEcos(tr)*20.f*(float)FD*0.033f;
				MustRefresh=TRUE;
				NOMOREMOVES=1;
			}

			// Checks WALK_BACKWARD Key Status.
			if (ARX_IMPULSE_Pressed(CONTROLS_CUST_WALKBACKWARD) )
			{
				tr=DEG2RAD(eyeball.angle.b);
				eyeball.pos.x+=(float)EEsin(tr)*20.f*(float)FD*0.033f;
				eyeball.pos.z+=-(float)EEcos(tr)*20.f*(float)FD*0.033f;
				MustRefresh=TRUE;
				NOMOREMOVES=1;
			}				

			// Checks STRAFE_LEFT Key Status.
			if( (ARX_IMPULSE_Pressed(CONTROLS_CUST_STRAFELEFT)||
				(ARX_IMPULSE_Pressed(CONTROLS_CUST_STRAFE)&&ARX_IMPULSE_Pressed(CONTROLS_CUST_TURNLEFT)))
				&& !NOMOREMOVES)
			{
				tr=DEG2RAD(MAKEANGLE(eyeball.angle.b+90.f));
				eyeball.pos.x+=-(float)EEsin(tr)*10.f*(float)FD*0.033f;
				eyeball.pos.z+=+(float)EEcos(tr)*10.f*(float)FD*0.033f;
				MustRefresh=TRUE;
				NOMOREMOVES=1;			
			}

			// Checks STRAFE_RIGHT Key Status.
			if( (ARX_IMPULSE_Pressed(CONTROLS_CUST_STRAFERIGHT)||
				(ARX_IMPULSE_Pressed(CONTROLS_CUST_STRAFE)&&ARX_IMPULSE_Pressed(CONTROLS_CUST_TURNRIGHT)))
				&& !NOMOREMOVES)
			{
				tr=DEG2RAD(MAKEANGLE(eyeball.angle.b-90.f));
				eyeball.pos.x+=-(float)EEsin(tr)*10.f*(float)FD*0.033f;
				//eyeball.pos.y+=FD*0.33f;
				eyeball.pos.z+=(float)EEcos(tr)*10.f*(float)FD*0.033f;
				MustRefresh=TRUE;
				NOMOREMOVES=1;
			}	

			IO_PHYSICS phys;			
			phys.cyl.height=-110.f;
			phys.cyl.origin.x=eyeball.pos.x;
			phys.cyl.origin.y=eyeball.pos.y+70.f;
			phys.cyl.origin.z=eyeball.pos.z;
			phys.cyl.radius=45.f;

			EERIE_CYLINDER test;
			memcpy(&test,&phys.cyl,sizeof(EERIE_CYLINDER));
			BOOL npc = AttemptValidCylinderPos(&test, NULL, CFLAG_JUST_TEST | CFLAG_NPC);
			float val=CheckAnythingInCylinder(&phys.cyl,inter.iobj[0],CFLAG_NO_NPC_COLLIDE | CFLAG_JUST_TEST);

			if ((val > -40.f))
			{
					if (val <= 70.f)
					{
						eyeball.pos.y+=val-70.f;
					}

				if (!npc)
				{
					MagicSightFader+=_framedelay*DIV200;

					if (MagicSightFader>1.f) MagicSightFader=1.f;
				}
			}
			else
			{
				memcpy(&eyeball.pos,&old,sizeof(EERIE_3D));
			}
		}
		
		
		float t;
		float multi;

		if (EDITMODE || ARXPausedTimer) FD=40.f;
		
		BOOL left=ARX_IMPULSE_Pressed(CONTROLS_CUST_STRAFELEFT);

		if(!left)
		{
			if(ARX_IMPULSE_Pressed(CONTROLS_CUST_STRAFE)&&ARX_IMPULSE_Pressed(CONTROLS_CUST_TURNLEFT))
			{
				left=TRUE;
			}
		}

		BOOL right=ARX_IMPULSE_Pressed(CONTROLS_CUST_STRAFERIGHT);

		if(!right)
		{
			if(ARX_IMPULSE_Pressed(CONTROLS_CUST_STRAFE)&&ARX_IMPULSE_Pressed(CONTROLS_CUST_TURNRIGHT))
			{
				right=TRUE;
			}
		}

		// Checks WALK_BACKWARD Key Status.
		if (	ARX_IMPULSE_Pressed(CONTROLS_CUST_WALKBACKWARD) 
			&&	!NOMOREMOVES	)
		{
			CurrFightPos=3;
			multi = 1;

			if (MoveDiv==0.02f) MOVETYPE=MOVE_WALK;
			else MOVETYPE=MOVE_RUN;

			if (left || right)
			{
				multi = 0.8f;
			}

			if (Project.improvespeed) multi+=1.2f;

			t=DEG2RAD(player.angle.b);
			multi=5.f*(float)FD*MoveDiv*multi;
			tm.x+=(float)EEsin(t)*multi;
			tm.z-=(float)EEcos(t)*multi;
		
			player.Current_Movement|=PLAYER_MOVE_WALK_BACKWARD;

			if (ARX_IMPULSE_NowPressed(CONTROLS_CUST_WALKBACKWARD) )
				MOVE_PRECEDENCE=PLAYER_MOVE_WALK_BACKWARD;			
		}
		else if (MOVE_PRECEDENCE==PLAYER_MOVE_WALK_BACKWARD) MOVE_PRECEDENCE=0;			

		// Checks WALK_FORWARD Key Status.
		if (ARX_IMPULSE_Pressed(CONTROLS_CUST_WALKFORWARD)
			&& !NOMOREMOVES)
		{
			CurrFightPos=2;
			multi = 1;

			if (MoveDiv==0.02f) MOVETYPE=MOVE_WALK;
			else MOVETYPE=MOVE_RUN;

			if (left || right) 
			{
				multi=0.8f;  				
			}

			if (Project.improvespeed) multi+=1.5;

			t=DEG2RAD(player.angle.b);
			multi=10.f*(float)FD*MoveDiv*multi;
			tm.x-=(float)EEsin(t)*multi;
			tm.z+=(float)EEcos(t)*multi;
			player.Current_Movement|=PLAYER_MOVE_WALK_FORWARD;

			if (ARX_IMPULSE_NowPressed(CONTROLS_CUST_WALKFORWARD) )
				MOVE_PRECEDENCE=PLAYER_MOVE_WALK_FORWARD;			
		}
		else if (MOVE_PRECEDENCE==PLAYER_MOVE_WALK_FORWARD) MOVE_PRECEDENCE=0;			
		
		// Checks STRAFE_LEFT Key Status.
		if (left && !NOMOREMOVES)
		{
			CurrFightPos=0;
			t=DEG2RAD(MAKEANGLE(player.angle.b+90.f));
			multi=6.f*(float)FD*MoveDiv;
			tm.x-=(float)EEsin(t)*multi;
			tm.z+=(float)EEcos(t)*multi;
			
			player.Current_Movement|=PLAYER_MOVE_STRAFE_LEFT;

			if (ARX_IMPULSE_NowPressed(CONTROLS_CUST_STRAFELEFT) )
				MOVE_PRECEDENCE=PLAYER_MOVE_STRAFE_LEFT;			
			
		}
		else if (MOVE_PRECEDENCE==PLAYER_MOVE_STRAFE_LEFT) MOVE_PRECEDENCE=0;			

		// Checks STRAFE_RIGHT Key Status.
		if ( right && !NOMOREMOVES)
		{
			CurrFightPos=1;
			t=DEG2RAD(MAKEANGLE(player.angle.b-90.f));
			multi=6.f*(float)FD*MoveDiv;
			tm.x-=(float)EEsin(t)*multi;
			tm.z+=(float)EEcos(t)*multi;			

			player.Current_Movement|=PLAYER_MOVE_STRAFE_RIGHT;

			if (ARX_IMPULSE_NowPressed(CONTROLS_CUST_STRAFERIGHT) )
				MOVE_PRECEDENCE=PLAYER_MOVE_STRAFE_RIGHT;			
		}
		else if (MOVE_PRECEDENCE==PLAYER_MOVE_STRAFE_RIGHT) MOVE_PRECEDENCE=0;			

		moveto.x=player.pos.x+tm.x;
		moveto.y=player.pos.y+tm.y;
		moveto.z=player.pos.z+tm.z;
	  }
	  
	  if (!USE_PLAYERCOLLISIONS)
	  {
		  if (this->kbd.inkey[INKEY_PAGEUP]) 
		  {
			  if (!USE_PLAYERCOLLISIONS) moveto.y=player.pos.y=player.pos.y-10.f;
		  }

		  if (this->kbd.inkey[INKEY_PAGEDOWN]) 
		  {
			  moveto.y=player.pos.y=player.pos.y+10.f;
		  }
	  }

	  // To remove for FINAL_RELEASE---------------------------------------
	  if (ALLOW_CHEATS || GAME_EDITOR)
	  {
		  if (this->kbd.inkey[INKEY_PAD5]) 
		  {
			  moveto.y=player.pos.y=FirstPolyPosY(player.pos.x,player.pos.z)-180.f;		
			  player.angle.a=0.f;
			  player.desiredangle.a=0.f;
			  this->kbd.inkey[INKEY_PAD5]=0;
		  }

		  if ((this->kbd.inkey[INKEY_A]) 
			  && ((this->kbd.inkey[INKEY_LEFTSHIFT]) || (this->kbd.inkey[INKEY_RIGHTSHIFT]) ) ) 
		  {
			  BLOCK_PLAYER_CONTROLS=0;
			  player.life=player.Full_maxlife;
			  player.mana=player.Full_maxmana;			
			  player.poison=0.f;
			  player.hunger=100;
			  DeadTime=0;
			  ARX_SOUND_PlayInterface(SND_PLAYER_FILLLIFEMANA, 0.9F + 0.2F * rnd());
		  }
		  }

	  // End of things to remove-------------------------------------------
	  
	  // Checks CROUCH Key Status.
	if (ARX_IMPULSE_NowPressed(CONTROLS_CUST_CROUCHTOGGLE))	
	  {
		  bGCroucheToggle=!bGCroucheToggle;
	  }

	  if(	ARX_IMPULSE_Pressed(CONTROLS_CUST_CROUCH)||
			bGCroucheToggle )
	  {
		  player.Current_Movement|=PLAYER_CROUCH;
	  }

	if (ARX_IMPULSE_NowPressed(CONTROLS_CUST_UNEQUIPWEAPON))	
	  {
		  ARX_EQUIPMENT_UnEquipPlayerWeapon();
	  }
	  
	  // Can only lean outside of combat mode
	  if (!(player.Interface & INTER_COMBATMODE))
	  {
		  // Checks LEAN_LEFT Key Status.
		  if (ARX_IMPULSE_Pressed(CONTROLS_CUST_LEANLEFT) )
		  {
			  player.Current_Movement|=PLAYER_LEAN_LEFT;
		  }

		  // Checks LEAN_RIGHT Key Status.
		  if (ARX_IMPULSE_Pressed(CONTROLS_CUST_LEANRIGHT) )
		  {
			  player.Current_Movement|=PLAYER_LEAN_RIGHT;
		  }
	  }

	  // Checks JUMP Key Status.
	  if ((player.jumpphase==0) &&
		  ARX_IMPULSE_NowPressed(CONTROLS_CUST_JUMP) )
	  {
		REQUEST_JUMP = ARXTimeUL(); 
	  }
	  

	  // MAGIC
	  if (ARX_IMPULSE_Pressed(CONTROLS_CUST_MAGICMODE))
	  {
		  if (!(player.Current_Movement & PLAYER_CROUCH) && (!BLOCK_PLAYER_CONTROLS)
			  && (ARXmenu.currentmode==AMCM_OFF))
		  {
			  if (!ARX_SOUND_IsPlaying(SND_MAGIC_AMBIENT))
				  ARX_SOUND_PlaySFX(SND_MAGIC_AMBIENT, NULL, 1.0F, ARX_SOUND_PLAY_LOOPED);
		  }
	  }
	  else
	  {
		  ARX_SOUND_Stop(SND_MAGIC_AMBIENT);
		  ARX_SOUND_Stop(SND_MAGIC_DRAW);
	  }
	  
	  if (ARX_IMPULSE_NowPressed(CONTROLS_CUST_DRINKPOTIONLIFE))
	  {
		  SendInventoryObjectCommand("GRAPH\\OBJ3D\\TEXTURES\\ITEM_POTION_LIFE.BMP", SM_INVENTORYUSE);
	  }
	  
	  if (ARX_IMPULSE_NowPressed(CONTROLS_CUST_DRINKPOTIONMANA))
	  {
		  SendInventoryObjectCommand("GRAPH\\OBJ3D\\TEXTURES\\ITEM_POTION_MANA.BMP", SM_INVENTORYUSE);
	  }
	  
	  if (ARX_IMPULSE_NowPressed(CONTROLS_CUST_TORCH))
	  {
		  if (CURRENT_TORCH)
		  {
			  ARX_PLAYER_KillTorch();
		  }
		  else
		  {
			  INTERACTIVE_OBJ * io = ARX_INVENTORY_GetTorchLowestDurability();

			  if (io)
			  {
				  INTERACTIVE_OBJ * ioo = io;

				  if (io->_itemdata->count>1)
				  {
					  ioo=CloneIOItem(io);
					  MakeTemporaryIOIdent(ioo);
					  ioo->show=SHOW_FLAG_NOT_DRAWN;
					  ioo->scriptload=1;
					  ioo->_itemdata->count=1;
					  io->_itemdata->count--;
				  }

				  ARX_PLAYER_ClickedOnTorch(ioo);
			  }
		  }
	  }

	  if (ARX_IMPULSE_NowPressed(CONTROLS_CUST_MINIMAP))
	  {
		  SHOW_INGAME_MINIMAP=!SHOW_INGAME_MINIMAP;
	  }

	  if (ARX_IMPULSE_NowPressed(CONTROLS_CUST_PREVIOUS))
	  {
		  if (eMouseState == MOUSE_IN_BOOK)
		  {
			  if (player.Interface & INTER_MAP)
			  {
				  if (Book_Mode > 0)
				  {
					  Book_Mode --;
					  ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
				  }
			  }
		  }
		else if (InPlayerInventoryPos(&DANAEMouse))
		  {
			  if (!PLAYER_INTERFACE_HIDE_COUNT)
			  {
				  if ((player.Interface & INTER_INVENTORY))
				  {
					  if (player.bag)
					  {
						  if (sActiveInventory > 0)
						  {
							  ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
							  sActiveInventory --;
						  }
					  }
				  }
			  }
		  }
		  else
		  {
			  if (player.Interface & INTER_MAP)
			  {
				  if (Book_Mode > 0)
				  {
					  Book_Mode --;
					  ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
				  }
			  }
			  else
			  {
				  if (!PLAYER_INTERFACE_HIDE_COUNT)
				  {
					  if ((player.Interface & INTER_INVENTORY))
					  {
						  if (player.bag)
						  {
							  if (sActiveInventory > 0)
							  {
								  ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
								  sActiveInventory --;
							  }
						  }
					  }
				  }
			  }
		  }
	  }

	  if (ARX_IMPULSE_NowPressed(CONTROLS_CUST_NEXT))
	  {
		  if (eMouseState == MOUSE_IN_BOOK)
		  {
			  if (player.Interface & INTER_MAP)
			  {
				  if (Book_Mode < 3)
				  {
					  ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
					  Book_Mode ++;
				  }
			  }
		  }
		else if (InPlayerInventoryPos(&DANAEMouse))
		  {
			  if (!PLAYER_INTERFACE_HIDE_COUNT)
			  {
				  if ((player.Interface & INTER_INVENTORY))
				  {
					  if (player.bag)
					  {
						  if (sActiveInventory < player.bag - 1)
						  {
							  ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
							  sActiveInventory ++;
						  }
					  }
				  }
			  }
		  }
		  else
		  {
			  if (player.Interface & INTER_MAP)
			  {
				  if (Book_Mode < 3)
				  {
					  ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
					  Book_Mode ++;
				  }
			  }
			  else
			  {
				  if (!PLAYER_INTERFACE_HIDE_COUNT)
				  {
					  if ((player.Interface & INTER_INVENTORY))
					  {
						  if (player.bag)
						  {
							  if (sActiveInventory < player.bag - 1)
							  {
								  ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
								  sActiveInventory ++;
							  }
						  }
					  }
				  }
			  }
		  }
	  }
	  
	  if (ARX_IMPULSE_NowPressed(CONTROLS_CUST_BOOKCHARSHEET))
	  {
		  if (!(player.Interface & INTER_MAP))
		  {
			  Book_Mode = 0;
			  ARX_INTERFACE_BookOpenClose(0);
		  }
		else if ((player.Interface & INTER_MAP) && (Book_Mode != 0))
			  {
				  Book_Mode = 0;
			  }
			  else
			  {
				  ARX_INTERFACE_BookOpenClose(2);
			  }
	  }
	  
	  if (ARX_IMPULSE_NowPressed(CONTROLS_CUST_BOOKSPELL))
	  {
		  if (!(player.Interface & INTER_MAP))
		  {
			  if (player.rune_flags)
			  {
				  Book_Mode = 1;
				  ARX_INTERFACE_BookOpenClose(0);
			  }
		  }
			  else if ((player.Interface & INTER_MAP) && (Book_Mode != 1))
			  {
				  Book_Mode = 1;
			  }
			  else
		  {
			  ARX_INTERFACE_BookOpenClose(2);
		  }	  
	  }
	  
	  if (ARX_IMPULSE_NowPressed(CONTROLS_CUST_BOOKMAP))
	  {
		  if (!(player.Interface & INTER_MAP))
		  {
			  Book_Mode = 2;
			  ARX_INTERFACE_BookOpenClose(0);
		  }
			  else if ((player.Interface & INTER_MAP) && (Book_Mode != 2))
			  {
				  Book_Mode = 2;
			  }
		  else
		  {
			  ARX_INTERFACE_BookOpenClose(2);
		  }
	  }
	  
	  if (ARX_IMPULSE_NowPressed(CONTROLS_CUST_BOOKQUEST))
	  {
		  if (!(player.Interface & INTER_MAP))
		  {
			  Book_Mode = 3;
			  ARX_INTERFACE_BookOpenClose(0);
		  }
		  			  else if ((player.Interface & INTER_MAP) && (Book_Mode != 3))
			  {
				  Book_Mode = 3;
			  }

		  else
		  {
			  ARX_INTERFACE_BookOpenClose(2);
		  }
	  }
	  
	  if (ARX_IMPULSE_NowPressed(CONTROLS_CUST_CANCELCURSPELL))
	  {
		  for (long i=MAX_SPELLS-1;i>=0;i--)
		  {
			  if ((spells[i].exist) && (spells[i].caster==0))
				  if (spellicons[spells[i].type].bDuration)
				  {
					  ARX_SPELLS_AbortSpellSound();
					  spells[i].tolive=0;
					  break;
				  }
		  }
	  }

	  if (ARX_IMPULSE_NowPressed(CONTROLS_CUST_PRECAST1))
	  {
		  if ((player.Interface & INTER_COMBATMODE) && !bIsAiming || !player.doingmagic)
			  if (Precast[0].typ != -1)
				  ARX_SPELLS_Precast_Launch(0);
	  }

	  if (ARX_IMPULSE_NowPressed(CONTROLS_CUST_PRECAST2))
	  {
		  if ((player.Interface & INTER_COMBATMODE) && !bIsAiming || !player.doingmagic)
			  if (Precast[1].typ != -1)
				  ARX_SPELLS_Precast_Launch(1);
	  }

	  if (ARX_IMPULSE_NowPressed(CONTROLS_CUST_PRECAST3))
	  {
		  if ((player.Interface & INTER_COMBATMODE) && !bIsAiming || !player.doingmagic)
			  if (Precast[2].typ != -1)
				  ARX_SPELLS_Precast_Launch(2);
	  }

	  if (ARX_IMPULSE_NowPressed(CONTROLS_CUST_WEAPON)||lChangeWeapon)
	  {
		bool bGo = true; 

		if (lChangeWeapon > 0)
		{
			if (lChangeWeapon == 2)
			{
				lChangeWeapon--;
			  }
			else
			{
				  if(	(inter.iobj[0]->animlayer[1].cur_anim==NULL)||
				        (inter.iobj[0]->animlayer[1].cur_anim == inter.iobj[0]->anims[ANIM_WAIT]))
				{
					  lChangeWeapon--;
					  
					if (pIOChangeWeapon)
					{
						SendIOScriptEvent(pIOChangeWeapon,SM_INVENTORYUSE,"");
						pIOChangeWeapon=NULL;
					  }
				  }
				else
				{
					  bGo=false;
				  }
			  }
		  }

		if (bGo)
		{
		  if (player.Interface & INTER_COMBATMODE)
		  {
			  ARX_INTERFACE_Combat_Mode(0);
			  bGToggleCombatModeWithKey=false;
			  SPECIAL_DRAW_WEAPON=0;
			  
			  if (pMenuConfig->bMouseLookToggle)
				  TRUE_PLAYER_MOUSELOOK_ON=MEMO_PLAYER_MOUSELOOK_ON;
		  }
		  else
		  {
			  MEMO_PLAYER_MOUSELOOK_ON=TRUE_PLAYER_MOUSELOOK_ON;
			  SPECIAL_DRAW_WEAPON=1;
			  TRUE_PLAYER_MOUSELOOK_ON|=1;
			  SLID_START=(float)ARXTime;
				lFadeMapTime = lARXTime;
			  ARX_INTERFACE_Combat_Mode(2);
			  bGToggleCombatModeWithKey=true;
			  
		  }
		  }
	  }

	  if(EERIEMouseButton&1) bGToggleCombatModeWithKey=false;

	if( (INTERNATIONAL_MODE)&&
	        (bForceEscapeFreeLook))
	{
		TRUE_PLAYER_MOUSELOOK_ON&=~1;

		if (!ARX_IMPULSE_Pressed(CONTROLS_CUST_FREELOOK))
		{
			bForceEscapeFreeLook=false;
		}
	}
	else
	{
		if(eMouseState!=MOUSE_IN_INVENTORY_ICON)
		{
		if (!pMenuConfig->bMouseLookToggle)
		{
			if (ARX_IMPULSE_Pressed(CONTROLS_CUST_FREELOOK))
			{
				if (!(TRUE_PLAYER_MOUSELOOK_ON & 1))
				{
					TRUE_PLAYER_MOUSELOOK_ON |= 1;
					SLID_START=(float)ARXTime;
				}
			}
			else
			{
				if(INTERNATIONAL_MODE)
				{
					TRUE_PLAYER_MOUSELOOK_ON &= ~1;
				}
			}
		}
		else
		{
			if (ARX_IMPULSE_NowPressed(CONTROLS_CUST_FREELOOK))
			{
				if (!(TRUE_PLAYER_MOUSELOOK_ON & 1))
				{
					TRUE_PLAYER_MOUSELOOK_ON |= 1;
					SLID_START=(float)ARXTime;
				}
				else
				{
					TRUE_PLAYER_MOUSELOOK_ON&=~1;

					if (player.Interface & INTER_COMBATMODE)
						ARX_INTERFACE_Combat_Mode(0);			
				}
			}
		}
		}
	}


	if(	(player.Interface&INTER_COMBATMODE)&&
		(ARX_IMPULSE_NowUnPressed(CONTROLS_CUST_FREELOOK)) )
	{
		if(INTERNATIONAL_MODE)
		{
			ARX_INTERFACE_Combat_Mode(0);
		}
		else
		{
			player.Interface&=~INTER_COMBATMODE;
			ARX_EQUIPMENT_LaunchPlayerUnReadyWeapon();
		}
	}
	  
	  if (EDITMODE) return;//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	  // Checks INVENTORY Key Status.
	  if (ARX_IMPULSE_NowPressed(CONTROLS_CUST_INVENTORY))
	  {
		  if (player.Interface & INTER_COMBATMODE)
		  {
			  ARX_INTERFACE_Combat_Mode(0);
		  }

		if (INTERNATIONAL_MODE)
		{
			  bInverseInventory=!bInverseInventory;
			  lOldTruePlayerMouseLook=TRUE_PLAYER_MOUSELOOK_ON;

			if (!pMenuConfig->bMouseLookToggle)
			{
				bForceEscapeFreeLook=true;
			  }
		  }
		else
		{
			  InventoryOpenClose(0);
		  }
	  }

	  // Checks BOOK Key Status.
	  if (ARX_IMPULSE_NowPressed(CONTROLS_CUST_BOOK))
		  ARX_INTERFACE_BookOpenClose(0);
	  
	  //	Check For Combat Mode ON/OFF
	  if (	(EERIEMouseButton & 1)
		  &&	(EDITION==EDITION_IO)
		  &&	(!(player.Interface & INTER_COMBATMODE))
	        &&	(!(ARX_MOUSE_OVER & ARX_MOUSE_OVER_BOOK)) 
		  &&	(!EDITMODE)	
		  &&	(!SpecialCursor)
		  &&  (PLAYER_MOUSELOOK_ON)
		  &&	(DRAGINTER==NULL)
		  &&	(!InInventoryPos(&DANAEMouse)
		  && (pMenuConfig->bAutoReadyWeapon))
		  )
	  {
		  if (!(LastMouseClick & 1))
		  {
			COMBAT_MODE_ON_START_TIME = ARXTimeUL();
		  }
		else 
		  {
			  if (ARXTime-COMBAT_MODE_ON_START_TIME>10)
			  {
				  ARX_INTERFACE_Combat_Mode(1);

				  if (! pMenuConfig->bAutoReadyWeapon)
					  bGToggleCombatModeWithKey=true;
			  }
		  }
	  }

	if (INTERNATIONAL_MODE)
	{

		if	(lOldTruePlayerMouseLook!=TRUE_PLAYER_MOUSELOOK_ON)
		{
			bInverseInventory=false;

			if(	TRUE_PLAYER_MOUSELOOK_ON & 1	)
			{
				if (!CSEND)
				{
					CSEND=1;
					SendIOScriptEvent(inter.iobj[0],SM_EXPLORATIONMODE,"");
				}
			}
		}
		else				
		{
			if (CSEND)
			{
				CSEND=0;
				SendIOScriptEvent(inter.iobj[0],SM_CURSORMODE,"");
			}
		}

		static long lOldInterfaceTemp=0;

		if (TRUE_PLAYER_MOUSELOOK_ON)
		{
			if(bInverseInventory)
			{
				bRenderInCursorMode=true;

				if(MAGICMODE<0)
				{
					InventoryOpenClose(1);
				}
			}
			else
			{
				if(!bInventoryClosing)
				{

					lTimeToDrawMecanismCursor=0;
					lNbToDrawMecanismCursor=0;
				
					if (player.Interface & INTER_INVENTORYALL)
					{
						ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
						bInventoryClosing = true;
						lOldInterfaceTemp=INTER_INVENTORYALL;
					}
					
					
					bRenderInCursorMode=false;
					InventoryOpenClose(2);

					if((player.Interface &INTER_INVENTORY))
					{
						INTERACTIVE_OBJ * io = NULL;

						if (SecondaryInventory!=NULL)
						{
							io = (INTERACTIVE_OBJ *)SecondaryInventory->io;
						}
						else if (player.Interface & INTER_STEAL)
						{
							io = ioSteal;
						}

						if (io!=NULL)
						{
							InventoryDir=-1;
							SendIOScriptEvent(io,SM_INVENTORY2_CLOSE,"");
							TSecondaryInventory=SecondaryInventory;
							SecondaryInventory=NULL;
						}
					}

					if(pMenuConfig->bMouseLookToggle)
					{
						TRUE_PLAYER_MOUSELOOK_ON |= 1;
						SLID_START=(float)ARXTime;
					}
				}
			}
		}
		else
		{
			if (bInverseInventory)
			{
				if (!bInventoryClosing)
				{
					
					bRenderInCursorMode=false;

					InventoryOpenClose(2);

					if ((player.Interface & INTER_INVENTORY))
					{
						INTERACTIVE_OBJ * io = NULL;

						if (SecondaryInventory!=NULL)
						{
							io = (INTERACTIVE_OBJ *)SecondaryInventory->io;
						}
						else if (player.Interface & INTER_STEAL)
						{
							io = ioSteal;
						}	

						if (io!=NULL)
						{
							InventoryDir=-1;
							SendIOScriptEvent(io,SM_INVENTORY2_CLOSE,"");
							TSecondaryInventory=SecondaryInventory;
							SecondaryInventory=NULL;
						}
					}

					if(pMenuConfig->bMouseLookToggle)
					{
						TRUE_PLAYER_MOUSELOOK_ON |= 1;
						SLID_START=(float)ARXTime;
					}
				}
			}
			else
			{

				bRenderInCursorMode=true;

				if(MAGICMODE<0)
				{

				if(lOldInterfaceTemp)
				{
					lOldInterface=lOldInterfaceTemp;
					lOldInterfaceTemp=0;
					ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
				}
				 
				if(lOldInterface)
				{
					player.Interface|=lOldInterface;
					player.Interface&=~INTER_INVENTORY;
				}
				else
				{
					InventoryOpenClose(1);	
				}
				}
			}
		}

		if (bRenderInCursorMode)
		{
			if (eyeball.exist != 0)
			{
				long lNumSpell=ARX_SPELLS_GetInstance(SPELL_FLYING_EYE);

				if (lNumSpell >= 0)
				{
					ARX_SPELLS_Kill(lNumSpell);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_Reset()
{
	SMOOTHSLID=0;
	PLAYER_INTERFACE_HIDE_COUNT=0;
	BLOCK_PLAYER_CONTROLS=0;
	SLID_VALUE=0;
	CINEMASCOPE=0;
	CINEMA_INC=0;
	CINEMA_DECAL=0;
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_SetCinemascope(long status,long smooth)
{
	if (status)
	{
		CINEMASCOPE=1;//++;
		g_TimeStartCinemascope = ARX_TIME_Get();
	}
	else
	{
		CINEMASCOPE=0;//--;
		g_TimeStartCinemascope = 0;
	}

	ARX_CHECK_NOT_NEG(CINEMASCOPE);

	if (CINEMASCOPE)
	{
		if (smooth)
		{
			CINEMA_INC=1;
		}
		else CINEMA_DECAL=100;
	}
	else
	{
		if (smooth)
		{
			CINEMA_INC=-1;
		}
		else CINEMA_DECAL=0;
	}

	if (player.Interface & INTER_INVENTORY)
	{
		player.Interface &=~ INTER_INVENTORY;
	}

	if (player.Interface & INTER_INVENTORYALL)
	{
		player.Interface &=~ INTER_INVENTORYALL;
	}
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_PlayerInterfaceModify(long showhide,long smooth)
{
	if (showhide == 0)
	{
		InventoryOpenClose(2);
		ARX_INTERFACE_BookOpenClose(2);
		ARX_INTERFACE_NoteClose();
	}

	if (showhide) PLAYER_INTERFACE_HIDE_COUNT = 0; 
	else PLAYER_INTERFACE_HIDE_COUNT = 1; 

	if (PLAYER_INTERFACE_HIDE_COUNT<0) 
		PLAYER_INTERFACE_HIDE_COUNT=0;

	if (smooth)
	{
		if (showhide) SMOOTHSLID=-1;
		else SMOOTHSLID=1;
	}
	else
	{
		if (showhide) SLID_VALUE=0.f;
		else SLID_VALUE=100.f;

		F2L(SLID_VALUE,&lSLID_VALUE);
	}
}
extern void ARX_PrepareBackgroundNRMLs();
//-----------------------------------------------------------------------------
void DANAE::ManageKeyMouse() 
{
	if (ARXmenu.currentmode == AMCM_OFF)
	{
		INTERACTIVE_OBJ * pIO = NULL;

		if (!BLOCK_PLAYER_CONTROLS)
		{
			if (TRUE_PLAYER_MOUSELOOK_ON && !(player.Interface & INTER_COMBATMODE)
				&& (eMouseState != MOUSE_IN_NOTE)
				)
			{
				POINT pos;
				pos.x=EERIEMouseX;
				pos.y=EERIEMouseY;
				
				
				EERIE_S2D poss;

				ARX_CHECK_SHORT(pos.x);
				ARX_CHECK_SHORT(pos.y);
				poss.x=ARX_CLEAN_WARN_CAST_SHORT(pos.x);
				poss.y=ARX_CLEAN_WARN_CAST_SHORT(pos.y);

				poss.x=MemoMouse.x;
				poss.y=MemoMouse.y;
				
				// mode systemshock
				if (pMenuConfig->bMouseLookToggle && pMenuConfig->bAutoReadyWeapon == false)
				{
					

					float fX =  DANAESIZX * 0.5f;
					float fY =	DANAESIZY * 0.5f;
					ARX_CHECK_SHORT(fX);
					ARX_CHECK_SHORT(fY);

					DANAEMouse.x = ARX_CLEAN_WARN_CAST_SHORT(fX);
					DANAEMouse.y = ARX_CLEAN_WARN_CAST_SHORT(fY);


					pIO = FlyingOverObject(&DANAEMouse,1);
					
					if (pIO)
					{
						FlyingOverIO = pIO;
						MemoMouse.x = DANAEMouse.x;
						MemoMouse.y = DANAEMouse.y;
					}
				}
				else
					pIO = FlyingOverObject(&poss,1);
			}
			else
				pIO = FlyingOverObject(&DANAEMouse,1);
		}
		
		if (pIO && (ARX_MOUSE_OVER & ARX_MOUSE_OVER_BOOK))
		{
			for (long i=0;i<MAX_EQUIPED;i++)
			{
				if ((player.equiped[i]!=0)
					&&	ValidIONum(player.equiped[i])
					&&	(inter.iobj[player.equiped[i]] == pIO))
					FlyingOverIO = pIO;
			}
		}
		
		if ((pIO)
			&& (pIO->GameFlags & GFLAG_INTERACTIVITY)
			&& !(ARX_MOUSE_OVER & ARX_MOUSE_OVER_BOOK)
			&& (eMouseState != MOUSE_IN_NOTE)
			)
		{
			if (!(EERIEMouseButton & 2) && (LastMouseClick & 2)
				&&  (STARTED_ACTION_ON_IO == pIO))
			{
				if (pIO->ioflags & IO_ITEM)
				{
					FlyingOverIO = pIO;
					COMBINE=NULL;

					if (DRAGINTER == NULL)
					{
						bool bOk = true;

						if (SecondaryInventory!=NULL)
						{
							INTERACTIVE_OBJ * temp=(INTERACTIVE_OBJ *)SecondaryInventory->io;

							if (IsInSecondaryInventory(FlyingOverIO))
								if (temp->ioflags & IO_SHOP)
									bOk = false;
						}
						
							INTERACTIVE_OBJ * io=inter.iobj[0];
							ANIM_USE * useanim=&io->animlayer[1];
							long type=ARX_EQUIPMENT_GetPlayerWeaponType();

							switch (type)
							{		
							case WEAPON_DAGGER:

								if(useanim->cur_anim==io->anims[ANIM_DAGGER_UNREADY_PART_1]) bOk=false;

								break;
							case WEAPON_1H:

								if(useanim->cur_anim==io->anims[ANIM_1H_UNREADY_PART_1]) bOk=false;

								break;
							case WEAPON_2H:

								if(useanim->cur_anim==io->anims[ANIM_2H_UNREADY_PART_1]) bOk=false;

								break;
							case WEAPON_BOW:

								if(useanim->cur_anim==io->anims[ANIM_MISSILE_UNREADY_PART_1]) bOk=false;

								break;
							default:
								break;
							}
						
						if (bOk)
						{
							if (!((FlyingOverIO->_itemdata->playerstacksize <= 1) && (FlyingOverIO->_itemdata->count > 1)))
							{
								SendIOScriptEvent(FlyingOverIO,SM_INVENTORYUSE,"");

								if (!((pMenuConfig->bAutoReadyWeapon == false) && (pMenuConfig->bMouseLookToggle)))
								{
									TRUE_PLAYER_MOUSELOOK_ON&=~1;
								}
							}
						}
					}

					if ((pMenuConfig->bAutoReadyWeapon == false) && (pMenuConfig->bMouseLookToggle))
					{
						EERIEMouseButton &= ~2;
					}
				}
				}
			else //!TRUE_PLAYER_MOUSELOOK_ON  
			{
				if ((EERIEMouseButton & 2) && !(LastMouseClick & 2))
				{
					STARTED_ACTION_ON_IO=FlyingOverIO;
				}
			}
		}
		
		if ((eMouseState == MOUSE_IN_WORLD) || 
			((eMouseState == MOUSE_IN_BOOK) && (!((ARX_MOUSE_OVER & ARX_MOUSE_OVER_BOOK) && (Book_Mode !=2))))
			)
		{
			if (pMenuConfig->bMouseLookToggle)
			{
				
					if (eMouseState != MOUSE_IN_NOTE)
				{
						if ((EERIEMouseButton & 2) && !(LastMouseClick & 2)&&(pMenuConfig)&&(pMenuConfig->bLinkMouseLookToUse))
						{
							if (!(FlyingOverIO && (FlyingOverIO->ioflags & IO_ITEM)) || DRAGINTER)
							{
								if (!(TRUE_PLAYER_MOUSELOOK_ON & 1))
								{
									if (!InInventoryPos(&DANAEMouse))
									{
										if (!((player.Interface & INTER_MAP) && Book_Mode != 2))
										{
											TRUE_PLAYER_MOUSELOOK_ON|=1;
											EERIEMouseButton &= ~2;
											SLID_START=(float)ARXTime;
											lFadeMapTime = lARXTime;
										}
									}
								}
								else 
								{
									if (!((pMenuConfig->bAutoReadyWeapon == false) && (pMenuConfig->bMouseLookToggle) && FlyingOverIO && (FlyingOverIO->ioflags & IO_ITEM)))
									{
										TRUE_PLAYER_MOUSELOOK_ON&=~1;

										if (player.Interface & INTER_COMBATMODE && !(player.Interface & INTER_NOTE))
											ARX_INTERFACE_Combat_Mode(0);
									}
								}
							}
						}
				}
			}
			else
			{
				if (eMouseState != MOUSE_IN_NOTE)
				{
					if(	(EERIEMouseButton & 2) &&
						(!(ARX_MOUSE_OVER & ARX_MOUSE_OVER_BOOK & (Book_Mode !=2))) &&
						(!(TRUE_PLAYER_MOUSELOOK_ON & 1) || SPECIAL_DRAW_WEAPON)&&
					        (pMenuConfig) && (pMenuConfig->bLinkMouseLookToUse)) 
					{
						if (SPECIAL_DRAW_WEAPON)
							SPECIAL_DRAW_WEAPON=0;
						else if (!InInventoryPos(&DANAEMouse))
							{
								if (SPECIAL_DRAW_WEAPON)
								{
									TRUE_PLAYER_MOUSELOOK_ON&=~1;
									SPECIAL_DRAW_WEAPON=0;
								}
								else
								{
									TRUE_PLAYER_MOUSELOOK_ON|=1;
									SLID_START=(float)ARXTime;
								lFadeMapTime = lARXTime;
								}
							}
					}
					else if ((!(EERIEMouseButton & 2)) && pMenuConfig->bLinkMouseLookToUse && (LastMouseClick & 2))
					{
						if (!SPECIAL_DRAW_WEAPON)
						{
							if (!ARX_IMPULSE_Pressed(CONTROLS_CUST_FREELOOK)) 
								TRUE_PLAYER_MOUSELOOK_ON&=~1;

							if ((player.Interface & INTER_COMBATMODE) && !ARX_IMPULSE_Pressed(CONTROLS_CUST_FREELOOK))
								ARX_INTERFACE_Combat_Mode(0);
						}

						EERIEMouseButton &= ~2;
					}
				}

				if (TRUE_PLAYER_MOUSELOOK_ON && (!(EERIEMouseButton & 2)) && !SPECIAL_DRAW_WEAPON)
				{
					if (!ARX_IMPULSE_Pressed(CONTROLS_CUST_FREELOOK))
						TRUE_PLAYER_MOUSELOOK_ON&=~1;
				}
			}
		}
		
		PLAYER_MOUSELOOK_ON=TRUE_PLAYER_MOUSELOOK_ON;

		if ((player.doingmagic==2)&& (pMenuConfig->bMouseLookToggle))
			PLAYER_MOUSELOOK_ON=0;
	}

	if(ARXmenu.currentmode!=AMCM_OFF)
	{
		PLAYER_MOUSELOOK_ON=0;
	}

	// Checks For MouseGrabbing/Restoration after Grab	
	bool bRestoreCoordMouse=true;

	if((PLAYER_MOUSELOOK_ON) && (!LAST_PLAYER_MOUSELOOK_ON))
	{
		MemoMouse.x=DANAEMouse.x;
		MemoMouse.y=DANAEMouse.y;
		EERIEMouseGrab=1;
	}
	else if ((!PLAYER_MOUSELOOK_ON) && (LAST_PLAYER_MOUSELOOK_ON))
	{
		EERIEMouseGrab=0;
		POINT	pos;
		pos.x=MemoMouse.x;
		pos.y=MemoMouse.y;

		if ( this->m_pDeviceInfo->bWindowed)
		{
			pos.x+=this->m_pFramework->Xstart;
			pos.y+=this->m_pFramework->Ystart;
		}

		ClientToScreen(this->m_hWnd,&pos);
		SetCursorPos(pos.x,pos.y);
		DANAEMouse.x=MemoMouse.x;
		DANAEMouse.y=MemoMouse.y;

		if(	(danaeApp.m_pFramework->m_bIsFullscreen)&&
			(bGLOBAL_DINPUT_GAME) )
		{
			if(pGetInfoDirectInput)
			{

				pGetInfoDirectInput->fMouseAXTemp	=	DANAEMouse.x ;
				pGetInfoDirectInput->fMouseAYTemp	=	DANAEMouse.y ;
				ARX_CHECK_INT(pGetInfoDirectInput->fMouseAXTemp);
				ARX_CHECK_INT(pGetInfoDirectInput->fMouseAYTemp);
				
				pGetInfoDirectInput->iMouseAX=ARX_CLEAN_WARN_CAST_INT(pGetInfoDirectInput->fMouseAXTemp);
				pGetInfoDirectInput->iMouseAY=ARX_CLEAN_WARN_CAST_INT(pGetInfoDirectInput->fMouseAYTemp);


			}
		}

		bRestoreCoordMouse=false;
	}

	LAST_PLAYER_MOUSELOOK_ON=PLAYER_MOUSELOOK_ON;
	
	
	
	
	
	PLAYER_ROTATION=0;

	if (Project.interpolatemouse) // mouse smoothing...
	{
		float v=EERIEMouseXdep*DIV1000;

		if (v>3.1415927f) 
		{
			v=3.1415927f;
		}
		else if (v<-3.1415927f) 
		{
			v=-3.1415927f;
		}

		F2L((float)(EEsin(v)*600.f),&EERIEMouseXdep);
		
		v=EERIEMouseYdep*DIV1000;

		if (v>3.1415927f) 
		{
			v=3.1415927f;
		}
		else if (v<-3.1415927f) 
		{
			v=-3.1415927f;
		}

		F2L((float)(EEsin(v)*600.f),&EERIEMouseYdep);
		
	}

	ARX_Menu_Manage(this->m_pd3dDevice);
	EERIE_3D tm;
	tm.x=tm.y=tm.z=0.f;
	INTERACTIVE_OBJ * t;
	
	MOVETYPE=MOVE_WAIT;

	if(bRestoreCoordMouse)
	{

		ARX_CHECK_SHORT(EERIEMouseX-this->m_pFramework->Xstart);
		ARX_CHECK_SHORT(EERIEMouseX-this->m_pFramework->Ystart);
		ARX_CHECK_SHORT(EERIEMouseX);
		ARX_CHECK_SHORT(EERIEMouseY);

		if ( this->m_pDeviceInfo->bWindowed)
		{
			DANAEMouse.x=ARX_CLEAN_WARN_CAST_SHORT(EERIEMouseX-this->m_pFramework->Xstart);
			DANAEMouse.y=ARX_CLEAN_WARN_CAST_SHORT(EERIEMouseY-this->m_pFramework->Ystart);
		}
		else 
		{
			DANAEMouse.x=ARX_CLEAN_WARN_CAST_SHORT(EERIEMouseX);
			DANAEMouse.y=ARX_CLEAN_WARN_CAST_SHORT(EERIEMouseY);
		}


				}
	
	// Player/Eyeball Freelook Management
	if (!BLOCK_PLAYER_CONTROLS)
	{
		GetInventoryObj_INVENTORYUSE(&DANAEMouse); 

		if ((!(player.Interface & INTER_MAP )) || ((player.Interface & INTER_MAP ) && ((!(ARX_MOUSE_OVER & ARX_MOUSE_OVER_BOOK & (Book_Mode != 2))/*ARX_INTERFACE_MouseInBook()*/) || (Book_Mode==2) || (Book_Mode==3) || (Book_Mode!=-1)))
			||
			(player.Interface & INTER_COMBATMODE))
		{
			static int flPushTimeX[2]={0,0};
			static int flPushTimeY[2]={0,0};
			bool bKeySpecialMove=false;

			if(!ARX_IMPULSE_Pressed(CONTROLS_CUST_STRAFE))
			{

				float fTime		= ARX_TIME_Get();	
				ARX_CHECK_INT(fTime);

				int	iTime		=  ARX_CLEAN_WARN_CAST_INT(fTime);


				if(ARX_IMPULSE_Pressed(CONTROLS_CUST_TURNLEFT))
				{
					if(!flPushTimeX[0]) 
					{
						flPushTimeX[0]	=	iTime;
					}

					bKeySpecialMove=true;
				}
				else flPushTimeX[0]=0;

				if(ARX_IMPULSE_Pressed(CONTROLS_CUST_TURNRIGHT))
				{
					if(!flPushTimeX[1]) 
					{
						flPushTimeX[1]	=	iTime;
					}

					bKeySpecialMove=true;
				}
				else flPushTimeX[1]=0;
			}

			if (USE_PLAYERCOLLISIONS)
			{

				float fTime		= ARX_TIME_Get();	
				ARX_CHECK_INT(fTime);

				int	iTime		=  ARX_CLEAN_WARN_CAST_INT(fTime);


				if(ARX_IMPULSE_Pressed(CONTROLS_CUST_LOOKUP))
				{
					if(!flPushTimeY[0]) 
					{
						flPushTimeY[0]	=	iTime;
					}

					bKeySpecialMove=true;
				}
				else flPushTimeY[0]=0;

				if(ARX_IMPULSE_Pressed(CONTROLS_CUST_LOOKDOWN))
				{
					if(!flPushTimeY[1]) 
					{
						flPushTimeY[1]	=	iTime;
					}

					bKeySpecialMove=true;
				}
				else flPushTimeY[1]=0;
			}

			if(bKeySpecialMove)
			{
				int iAction=0;

				if(	flPushTimeX[0]||
					flPushTimeX[1] )
				{
					if(flPushTimeX[0]<flPushTimeX[1]) EERIEMouseXdep=10;
					else EERIEMouseXdep=-10;

					iAction|=1;
				}

				if(	flPushTimeY[0]||
					flPushTimeY[1] )
				{
					if(flPushTimeY[0]<flPushTimeY[1]) EERIEMouseYdep=10;
					else EERIEMouseYdep=-10;

					iAction|=2;
				}

				if(!(iAction&1)) EERIEMouseXdep=0;

				if(!(iAction&2)) EERIEMouseYdep=0;
			}
			else
			{
				if (INTERNATIONAL_MODE)
				{
					if (bRenderInCursorMode)
					{
						if(	(DANAEMouse.x==(DANAESIZX-1))&&
						        (pGetInfoDirectInput->iMouseRX > 8))
						{
							EERIEMouseYdep=0;
							EERIEMouseXdep=pGetInfoDirectInput->iMouseRX;
							bKeySpecialMove=true;
						}
						else
						{
							if( (!DANAEMouse.x)&&
							        (pGetInfoDirectInput->iMouseRX < -8))
							{
								EERIEMouseYdep=0;
								EERIEMouseXdep=pGetInfoDirectInput->iMouseRX;
								bKeySpecialMove=true;
							}
						}

						if(	(DANAEMouse.y==(DANAESIZY-1))&&
						        (pGetInfoDirectInput->iMouseRY > 8))
						{
							EERIEMouseYdep=pGetInfoDirectInput->iMouseRY;
							EERIEMouseXdep=0;
							bKeySpecialMove=true;
						}
						else
						{
							if(	(!DANAEMouse.y)&&
							        (pGetInfoDirectInput->iMouseRY < -8))
							{
								EERIEMouseYdep=pGetInfoDirectInput->iMouseRY;
								EERIEMouseXdep=0;
								bKeySpecialMove=true;
							}
						}
					}
				}
			}

			if(ARX_IMPULSE_Pressed(CONTROLS_CUST_CENTERVIEW))
			{
				eyeball.angle.a=eyeball.angle.g=0.f;
				player.desiredangle.a=player.desiredangle.g=player.angle.a=player.angle.g=0.f;
			}

			float fd;

			if(	0
				&&	(danaeApp.m_pFramework->m_bIsFullscreen)
				&&	(bGLOBAL_DINPUT_GAME) )
			{
				fd = (Original_framedelay) * .3f * (640.f / (float)DANAESIZX); 
			}
			else
			{

				fd = (((float)pGetInfoDirectInput->iSensibility) + 1.f) * 0.1f * ((640.f / (float)DANAESIZX));

				if(	(pMenuConfig)&&
					(pMenuConfig->bMouseSmoothing) )
				{
					float of=Original_framedelay;

					if (of<=0.f)
						fd=0.f;
					else if (of>80.f)
					{
						of=80.f;
						fd*=of;
					}
					else
						fd*=of;
				}
				else if (fd > 200)
						fd=200;
			}

			fd *= ((float)DANAESIZX) * DIV640; 

			if ((eyeball.exist==2) && (PLAYER_MOUSELOOK_ON||bKeySpecialMove)) 
			{
				if (EERIEMouseYdep!=0)
				{
					float ia;

					if(	(pMenuConfig)&&
						(pMenuConfig->bMouseSmoothing) )
					{
						ia=((float)EERIEMouseYdep*DIV60)*fd;
					}
					else
					{
						ia=((float)EERIEMouseYdep*DIV5)*fd;
					}

					if (INVERTMOUSE) ia=-ia;

					if (eyeball.angle.a<70.f) 
					{
						if (eyeball.angle.a+ia<70.f) eyeball.angle.a+=ia;
					}
					else if (eyeball.angle.a>300.f) 
					{
						if (eyeball.angle.a+ia>300.f) eyeball.angle.a+=ia;
					}

					eyeball.angle.a=MAKEANGLE(eyeball.angle.a);
				}

				if (EERIEMouseXdep!=0)
				{
					float ib;

					if(	(pMenuConfig)&&
						(pMenuConfig->bMouseSmoothing) )
					{
						ib=((float)EERIEMouseXdep*DIV50)*fd;
					}
					else
					{
						ib=((float)EERIEMouseXdep*DIV5)*fd;
					}

					eyeball.angle.b=MAKEANGLE(eyeball.angle.b-ib);			
				}
			}
			else if (PLAYER_MOUSELOOK_ON || bKeySpecialMove) 
				if (ARXmenu.currentmode != AMCM_NEWQUEST)
					{
					if ((EERIEMouseYdep != 0)) 
						{
						float ia;
							
							if(	(pMenuConfig)&&
								(pMenuConfig->bMouseSmoothing) )
							{
								ia=((float)EERIEMouseYdep*DIV60*fd);
							}
							else
							{
							ia = ((float)EERIEMouseYdep * DIV5 * fd);
							}

							if ((inter.iobj[0]) && EEfabs(ia)>2.f) inter.iobj[0]->lastanimtime=0;
							
							if (INVERTMOUSE) ia=-ia;

							float iangle=player.angle.a;
						
							player.desiredangle.a=player.angle.a;
							player.desiredangle.a+=ia;
							player.desiredangle.a=MAKEANGLE(player.desiredangle.a);

							if ((player.desiredangle.a>=74.9f) && (player.desiredangle.a<=301.f))
							{
								if (iangle<75.f) player.desiredangle.a=74.9f; //69
								else player.desiredangle.a=301.f;
							}
							
						}

					if ((EERIEMouseXdep != 0))
						{
							float ib;

							if(	(pMenuConfig)&&
								(pMenuConfig->bMouseSmoothing) )
							{
								ib=((float)EERIEMouseXdep*DIV50*fd);
							}
							else
							{
							ib = ((float)EERIEMouseXdep * DIV5 * fd); 
							}
							
							if (ib!=0.f) player.Current_Movement|=PLAYER_ROTATE;	

							player.desiredangle.b=player.angle.b;
							player.desiredangle.b=MAKEANGLE(player.desiredangle.b-ib);			
							PLAYER_ROTATION=ib;
						}
				
				}
				
		}
	}
	
	

	{
		
		if (EDITMODE)
		{
			if (EERIEMouseButton & 1) 
			{
				t = FlyingOverIO; 

				if (t!=NULL)
				{
					SelectIO(t);
					EERIEMouseButton&=~1;
				}		
			}
		}
		////////
		else if ((!BLOCK_PLAYER_CONTROLS) && !(player.Interface & INTER_COMBATMODE)) 
			{
				if (DRAGINTER == NULL)
					if ((LastMouseClick & 1) && !(EERIEMouseButton & 1) && !(EERIEMouseButton & 4) && !(LastMouseClick & 4))
					{
						INTERACTIVE_OBJ * temp;
					temp = FlyingOverIO; 

						if ((temp!=NULL) && temp->locname[0] )
						{
							if (((FlyingOverIO->ioflags & IO_ITEM) && FlyingOverIO->_itemdata->equipitem)
								&& (player.Full_Skill_Object_Knowledge + player.Full_Attribute_Mind
								>= FlyingOverIO->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Identify_Value].value) )
							{
								SendIOScriptEvent(FlyingOverIO,SM_IDENTIFY,"");
							}

							MakeLocalised(temp->locname,WILLADDSPEECH,256,-1);

							if (temp->ioflags & IO_GOLD)
							{
								_TCHAR UText[256];
								_stprintf(UText, _T("%d %s"), temp->_itemdata->price, WILLADDSPEECH);
								_tcscpy(WILLADDSPEECH, UText);
							}

							if ((temp->poisonous>0) && (temp->poisonous_count!=0))
							{
								_TCHAR Text[256];
								_TCHAR UText[256];
								MakeLocalised("[Description_Poisoned]",Text,256,-1);
								_stprintf(UText, _T("%s (%s %d)"),  WILLADDSPEECH, Text, (long)temp->poisonous);
								_tcscpy(WILLADDSPEECH, UText);
							}

							if ((temp->ioflags & IO_ITEM) && (temp->durability<100.f))
							{
								_TCHAR Text[256];
								_TCHAR UText[256];
								MakeLocalised("[Description_Durability]",Text,256,-1);
								_stprintf(UText, _T("%s %s %3.0f/%3.0f"),  WILLADDSPEECH, Text, temp->durability,temp->max_durability);
								_tcscpy(WILLADDSPEECH, UText);
							}

					
						WILLADDSPEECHTIME = ARXTimeUL();

						if (INTERNATIONAL_MODE)
						{

							bool bAddText = true;		

								if( (temp->obj)&&
									(temp->obj->pbox)&&
							        (temp->obj->pbox->active == 1))
							{
									bAddText=false;
								}

							if (bAddText)
							{

									//------------ ONLY IN DEBUG
									ARX_CHECK_LONG( 120 * Xratio );
									ARX_CHECK_LONG( 14 * Yratio );
									ARX_CHECK_LONG( ( 120 + 500 ) * Xratio );
									ARX_CHECK_LONG( ( 14  + 200 ) * Yratio );
									//------------
									RECT rDraw	=	{	ARX_CLEAN_WARN_CAST_LONG( 120 * Xratio ), 
														ARX_CLEAN_WARN_CAST_LONG( 14 * Yratio ),
														ARX_CLEAN_WARN_CAST_LONG( ( 120 + 500 ) * Xratio ),
								                    ARX_CLEAN_WARN_CAST_LONG((14 + 200) * Yratio)
								             };

									pTextManage->Clear();
								pTextManage->AddText(InBookFont,
																		WILLADDSPEECH,
																	    rDraw,
																		RGB(232,204,143),
																		0x00FF00FF,
																		2000+_tcslen(WILLADDSPEECH)*60);
								}

								WILLADDSPEECH[0]=0;
							}
						}
					}	
					else
					{
						if(	(INTERNATIONAL_MODE)&&
							(pMenuConfig)&&
					        (pMenuConfig->bAutoDescription))
					{

								INTERACTIVE_OBJ * temp;
						temp = FlyingOverIO; 

								if ((temp!=NULL) && temp->locname[0] )
								{
									if (((FlyingOverIO->ioflags & IO_ITEM) && FlyingOverIO->_itemdata->equipitem)
										&& (player.Full_Skill_Object_Knowledge + player.Full_Attribute_Mind
										>= FlyingOverIO->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Identify_Value].value) )
									{
										SendIOScriptEvent(FlyingOverIO,SM_IDENTIFY,"");
									}

									MakeLocalised(temp->locname,WILLADDSPEECH,256,-1);

									if (temp->ioflags & IO_GOLD)
									{
										_TCHAR UText[256];
										_stprintf(UText, _T("%d %s"), temp->_itemdata->price, WILLADDSPEECH);
										_tcscpy(WILLADDSPEECH, UText);
									}

									if ((temp->poisonous>0) && (temp->poisonous_count!=0))
									{
										_TCHAR Text[256];
										_TCHAR UText[256];
										MakeLocalised("[Description_Poisoned]",Text,256,-1);
										_stprintf(UText, _T("%s (%s %d)"),  WILLADDSPEECH, Text, (long)temp->poisonous);
										_tcscpy(WILLADDSPEECH, UText);
									}

									if ((temp->ioflags & IO_ITEM) && (temp->durability<100.f))
									{
										_TCHAR Text[256];
										_TCHAR UText[256];
										MakeLocalised("[Description_Durability]",Text,256,-1);
										_stprintf(UText, _T("%s %s %3.0f/%3.0f"),  WILLADDSPEECH, Text, temp->durability,temp->max_durability);
										_tcscpy(WILLADDSPEECH, UText);
									}

						
									WILLADDSPEECHTIME = ARXTimeUL();//treat warning C4244 conversion from 'float' to 'unsigned long'
	
									bool bAddText=true;

									if( (temp->obj)&&
										(temp->obj->pbox)&&
							        (temp->obj->pbox->active == 1))
							{
										bAddText=false;
									}

							if (bAddText)
							{

										//------------ ONLY IN DEBUG
										ARX_CHECK_LONG( 120 * Xratio );
										ARX_CHECK_LONG( 14 * Yratio );
										ARX_CHECK_LONG( ( 120 + 500 ) * Xratio );
										ARX_CHECK_LONG( ( 14 + 200 ) * Yratio );
										//------------

										RECT rDraw = {	ARX_CLEAN_WARN_CAST_LONG( 120 * Xratio ), 
														ARX_CLEAN_WARN_CAST_LONG( 14 * Yratio ), 
														ARX_CLEAN_WARN_CAST_LONG( ( 120 + 500 ) * Xratio ),
								                ARX_CLEAN_WARN_CAST_LONG((14 + 200) * Yratio)
								             };

										pTextManage->Clear();
								pTextManage->AddText(InBookFont,
																		WILLADDSPEECH,
																	    rDraw,
																		RGB(232,204,143),
																		0x00FF00FF );
									}

									WILLADDSPEECH[0]=0;
								}
							}
					}
			}
			
			if ((EERIEMouseButton & 4) || (LastMouseClick & 4)) WILLADDSPEECH[0]=0;

			if (WILLADDSPEECH[0]!=0)
				if (WILLADDSPEECHTIME+300<FrameTime)
				{	
					ARX_SPEECH_Add(NULL, WILLADDSPEECH);
					WILLADDSPEECH[0]=0;
				}

				///////
		if ((EERIEMouseButton & 4) && (EDITMODE))
				{
					INTERACTIVE_OBJ * t;
					
			t = FlyingOverIO; 

					if (t!=NULL) 
					{
						SelectIO(t);
						EERIEMouseButton&=~4;
						
						if ((CDP_IOOptions) && (LastSelectedIONum>=0) && (CDP_EditIO!=inter.iobj[LastSelectedIONum]))
						{
							SendMessage(CDP_IOOptions,WM_COMMAND,MAKELONG(IDOK,0),0);
						}

						if ((!CDP_IOOptions) && (ValidIONum(LastSelectedIONum)))
						{
							CDP_EditIO=inter.iobj[LastSelectedIONum];

							if (danaeApp.m_pFramework->m_bIsFullscreen)
							{
								ARX_TIME_Pause();
								Pause(TRUE);
								DialogBox( (HINSTANCE)GetWindowLong( this->m_hWnd, GWL_HINSTANCE ),
									MAKEINTRESOURCE(IDD_SCRIPTDIALOG), this->m_hWnd, IOOptionsProc);
								Pause(FALSE);
								ARX_TIME_UnPause();				
							}
							else 
								CDP_IOOptions=(CreateDialogParam( (HINSTANCE)GetWindowLong( this->m_hWnd, GWL_HINSTANCE ),
								MAKEINTRESOURCE(IDD_SCRIPTDIALOG), this->m_hWnd, IOOptionsProc,0 ));
						}
					}
				}

				if (	(!FINAL_COMMERCIAL_DEMO) 
					&&	(!FINAL_COMMERCIAL_GAME)	
					&& (ARXmenu.currentmode == AMCM_OFF))
				{
					if (this->kbd.inkey[41])
					{
						
						if (NEED_TEST_TEXT)
						{
							NEED_TEST_TEXT=0;
							USE_PLAYERCOLLISIONS=1;						
						}
						else
						{
							NEED_TEST_TEXT=1;
							USE_PLAYERCOLLISIONS=0;
						}
					}

					this->kbd.inkey[41]=0;					
				}

#ifndef NOEDITOR

				if (!FINAL_COMMERCIAL_DEMO)
				{
					if ((ALLOW_CHEATS || GAME_EDITOR)
						&& (ARXmenu.currentmode == AMCM_OFF))
					{
						if ((this->kbd.inkey[INKEY_C]) 
							&& ((this->kbd.inkey[INKEY_LEFTSHIFT]) || (this->kbd.inkey[INKEY_RIGHTSHIFT]) ) ) 
						{
							if (USE_PLAYERCOLLISIONS) USE_PLAYERCOLLISIONS=0;
							else USE_PLAYERCOLLISIONS=1;

							this->kbd.inkey[INKEY_C]=0;
						}

				if ((this->kbd.inkey[INKEY_W]))
						{
							ARX_PLAYER_Poison(0.f);
							ARX_PLAYER_Modify_XP(1000);
							this->kbd.inkey[INKEY_W]=0;
						}
					}
				}

				if (	(GAME_EDITOR)
					&&	(ARXmenu.currentmode == AMCM_OFF))
				{
					if (this->kbd.inkey[INKEY_F7]) 
					{
						if (EDITION==EDITION_LIGHTS) 
						{
							SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B010,FALSE); //Zones
							SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B004,FALSE); //Nodes
							SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B007,FALSE); //Lights
							SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B008,FALSE); //Fogs				
							EDITION=EDITION_IO;
						}
						else 
						{
							SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B010,FALSE); //Zones
							SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B004,FALSE); //Nodes
							SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B007,TRUE); //Lights
							SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B008,FALSE); //Fogs				
							EDITION=EDITION_LIGHTS;
						}

						this->kbd.inkey[INKEY_F7]=0;
					}
					
					if	(this->kbd.inkey[INKEY_F9])
					{
						if (EDITMODE) 
						{
							SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B006,FALSE);
							SetEditMode(0);							
							ARX_TIME_Get();
							SendGameReadyMsg();
						}
						else 
						{
							SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B006,TRUE);
							SetEditMode(1);			
							RestoreAllIOInitPos();
						}

						this->kbd.inkey[INKEY_F9]=0;
					}

					if (this->kbd.inkey[INKEY_F8]) 
					{
						if (ARXPausedTimer) ARX_TIME_UnPause();
						else ARX_TIME_Pause();

						this->kbd.inkey[INKEY_F8]=0;
					}
					
#endif
					
				}

				if (GAME_EDITOR)
				{
					if ((this->kbd.inkey[INKEY_F1]) && (FirstFrame==0))
					{
						ARX_PARTICLES_ClearAll();
						ARX_SPELLS_ClearAll();

						if (CDP_PATHWAYS_Options!=NULL) SendMessage(CDP_PATHWAYS_Options,WM_CLOSE,0,0);

						if (CDP_LIGHTOptions!=NULL) SendMessage(CDP_LIGHTOptions,WM_CLOSE,0,0);

						if (CDP_FogOptions!=NULL) SendMessage(CDP_FogOptions,WM_CLOSE,0,0);

						CDP_LIGHTOptions=NULL;
						CDP_FogOptions=NULL;	
						ARX_TIME_Pause();
						DanaeSwitchFullScreen();
						ReloadAllTextures(GDevice);
						
						if(ControlCinematique)
						{
							ControlCinematique->m_pd3dDevice=GDevice;
							ActiveAllTexture(ControlCinematique);
						}
						
						ARX_TIME_UnPause();
						LaunchDummyParticle();
						this->kbd.inkey[INKEY_F1]=0;		
					}
				}

				if (	(GAME_EDITOR)
					&&	(ARXmenu.currentmode == AMCM_OFF))
				{
					if (MouseInCam(&subj)) 
					{
						if (this->kbd.nbkeydown)
						{
							if (this->kbd.inkey[INKEY_0]) 
							{

								if (USE_LIGHT_OPTIM)
									USE_LIGHT_OPTIM=0;
								else
								{
									InitTileLights();
									USE_LIGHT_OPTIM=1;
								}

								this->kbd.inkey[INKEY_0]=0;
							}			

							if (this->kbd.inkey[INKEY_7]) 
							{
								unsigned long tim = ARX_TIME_GetUL();//treat warning C4244 conversion from 'float' to 'unsigned long'
								RecalcLightZone(player.pos.x,player.pos.y,player.pos.z,12);
								tim=ARX_TIME_GetUL() - tim;//treat warning C4244 conversion from 'float' to 'unsigned long'
								this->kbd.inkey[INKEY_7]=0;
							}			

							if (this->kbd.inkey[INKEY_9]) 
							{
								ARX_SPELLS_Launch(SPELL_FLYING_EYE,0);
								this->kbd.inkey[INKEY_9]=0;
							}

							if (this->kbd.inkey[INKEY_1]) 
							{
								ARX_SPELLS_Launch(SPELL_FIREBALL,0);
								this->kbd.inkey[INKEY_1]=0;
							}

							if (this->kbd.inkey[INKEY_2]) 
							{
								if (Project.improve) 
								{
									for (long i=0;i<MAX_SPELLS;i++)
									{
										if ((spells[i].exist) && (spells[i].type==SPELL_MAGIC_SIGHT)) spells[i].tolive=0;
									}
								}
								else ARX_SPELLS_Launch(SPELL_MAGIC_SIGHT,0);

								this->kbd.inkey[INKEY_2]=0;
							}

							if (this->kbd.inkey[INKEY_6]) 
							{
								if (inter.iobj[0]->invisibility>0.f)
								{
									for (long i=0;i<MAX_SPELLS;i++)
									{
										if ((spells[i].exist) && (spells[i].type==SPELL_INVISIBILITY)) spells[i].tolive=0;
									}
								}
								else ARX_SPELLS_Launch(SPELL_INVISIBILITY,0);

								this->kbd.inkey[INKEY_6]=0;
							}

							if (this->kbd.inkey[INKEY_5]) 
							{
								if (Project.telekinesis) 
								{
									for (long i=0;i<MAX_SPELLS;i++)
									{
										if ((spells[i].exist) && (spells[i].type==SPELL_TELEKINESIS)) spells[i].tolive=0;
									}
								}
								else ARX_SPELLS_Launch(SPELL_TELEKINESIS,0);

								this->kbd.inkey[INKEY_5]=0;
							}

							if (this->kbd.inkey[INKEY_4]) 
							{
								if (Project.improvespeed) 
								{
									for (long i=0;i<MAX_SPELLS;i++)
									{
										if ((spells[i].exist) && (spells[i].type==SPELL_SPEED)) spells[i].tolive=0;
									}
								}
								else 
								{
									ARX_SPELLS_Launch(SPELL_SPEED,0);
									tm.x+=-(float)EEsin(DEG2RAD(player.pos.b))*(float)FrameDiff*DIV3;
									tm.z+=+(float)EEcos(DEG2RAD(player.pos.b))*(float)FrameDiff*DIV3;
								}

								this->kbd.inkey[INKEY_4]=0;
							}

							if (this->kbd.inkey[INKEY_3]) 
							{
								ARX_SPELLS_Launch(SPELL_HEAL,0);
								this->kbd.inkey[INKEY_3]=0;
							}

							if (this->kbd.inkey[INKEY_8]) 
							{
								ARX_SPELLS_Launch(SPELL_ICE_PROJECTILE,0);
								this->kbd.inkey[INKEY_8]=0;
							}
							
							if (this->kbd.inkey[INKEY_PAD1])
							{
								extern long TSU_TEST;
								TSU_TEST = TSU_TEST ++;

								if (TSU_TEST>2) TSU_TEST = 0;

								this->kbd.inkey[INKEY_PAD1] = 0;
							}
						}
					}
				}
	}
}

static float fDecPulse; 
//-----------------------------------------------------------------------------
void ARX_INTERFACE_DrawSecondaryInventory(bool _bSteal)
{
	// To recode in a better way...
	if (TSecondaryInventory->io && TSecondaryInventory->io->inventory_skin)
	{
		char temp[256];
		sprintf(temp,"Graph\\Interface\\Inventory\\%s.bmp",TSecondaryInventory->io->inventory_skin);
		TextureContainer * tc=MakeTCFromFile(temp);

		if (tc) ITC.ingame_inventory=tc;
		else ITC.ingame_inventory=BasicInventorySkin;
	}
	else if (ITC.ingame_inventory!=BasicInventorySkin)
	{
		ITC.ingame_inventory=BasicInventorySkin;
	}

	ARX_INTERFACE_DrawItem(ITC.ingame_inventory,INTERFACE_RATIO(InventoryX),0.f);


	long i,j;
	RECT rect;
	rect.top=0;
	rect.bottom=0;

	for (j=0;j<TSecondaryInventory->sizey;j++)
	{
		for (i=0;i<TSecondaryInventory->sizex;i++)
		{
			INTERACTIVE_OBJ * io=TSecondaryInventory->slot[i][j].io;

			if (io!=NULL) 
			{
				bool bItemSteal = false;
				TextureContainer * tc=io->inv;
				TextureContainer * tc2=NULL;

				if (NeedHalo(io)) tc2 = io->inv->TextureHalo;

				if (_bSteal)
				{
					if (!ARX_PLAYER_CanStealItem(io))
					{
						bItemSteal = true;
						tc = ITC.item_cant_steal;
						tc2 = NULL;
					}
				}

				if ((tc!=NULL) && (TSecondaryInventory->slot[i][j].show || bItemSteal) )
				{
					if (io->ioflags & IO_GOLD)
					{	
						long num=0;

						if (io->_itemdata->price<=3) 
							num=io->_itemdata->price-1;
						else if (io->_itemdata->price<=8) 
							num=3;									
						else if (io->_itemdata->price<=20) 
							num=4;
						else if (io->_itemdata->price<=50) 
							num=5;
						else
							num=6;

						io->obj=GoldCoinsObj[num];
						io->inv=GoldCoinsTC[num];
					}

					if (tc->m_pddsSurface) 
					{
						float px = INTERFACE_RATIO(InventoryX) + (float)i*INTERFACE_RATIO(32) + INTERFACE_RATIO(2);
						float py = (float)j*INTERFACE_RATIO(32) + INTERFACE_RATIO(13);

						D3DCOLOR color;

						if ((io->poisonous) && (io->poisonous_count!=0))
							color=0xFF00FF00;
						else color=D3DCOLORWHITE;

						EERIEDrawBitmap(GDevice,
							px,
							py,
						                INTERFACE_RATIO_DWORD(tc->m_dwWidth), INTERFACE_RATIO_DWORD(tc->m_dwHeight),
							0.001f,
							tc,color);

						if (!bItemSteal && (io==FlyingOverIO))
						{
							SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
							SETALPHABLEND(GDevice,true);
							
							EERIEDrawBitmap(GDevice,
								px,
								py,
							                INTERFACE_RATIO_DWORD(tc->m_dwWidth), INTERFACE_RATIO_DWORD(tc->m_dwHeight),
								0.001f,
								tc,
								D3DCOLORWHITE);
							SETALPHABLEND(GDevice,false);
						}
						else
						{
							if (!bItemSteal && (io->ioflags & IO_CAN_COMBINE)) 
							{
								SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
								SETALPHABLEND(GDevice,true);

								float fColorPulse	=	255.f * fabs( cos( DEG2RAD( fDecPulse ) ) );
								DWORD dwColor		=	ARX_CLEAN_WARN_CAST_DWORD(fColorPulse);
								
								EERIEDrawBitmap(GDevice,
									px,
									py,
								                INTERFACE_RATIO_DWORD(tc->m_dwWidth), INTERFACE_RATIO_DWORD(tc->m_dwHeight),
									0.001f,
									tc,
									(dwColor<<16)|(dwColor<<8)|dwColor);
								SETALPHABLEND(GDevice,false);
							}
						}
						
						if (tc2!=NULL) 
						{
							ARX_INTERFACE_HALO_Draw(io, GDevice,tc,tc2,
								px,
								py, INTERFACE_RATIO(1), INTERFACE_RATIO(1));
						}

						if ((io->ioflags & IO_ITEM) && (io->_itemdata->count!=1))
							ARX_INTERFACE_DrawNumber(px, py, io->_itemdata->count, 3, D3DCOLORWHITE);
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_DrawInventory(short _sNum, int _iX=0, int _iY=0)
{
	fDecPulse += FrameDiff * 0.5f; 


	float fCenterX	= DANAECENTERX - INTERFACE_RATIO(320) + INTERFACE_RATIO(35) + _iX ; 
	float fSizY		= DANAESIZY - INTERFACE_RATIO(101) + INTERFACE_RATIO_LONG(InventoryY) + _iY;
	ARX_CHECK_INT(fCenterX);
	ARX_CHECK_INT(fSizY);


	float fPosX = ARX_CAST_TO_INT_THEN_FLOAT( fCenterX );
	float fPosY = ARX_CAST_TO_INT_THEN_FLOAT( fSizY );



	ARX_INTERFACE_DrawItem(ITC.hero_inventory, fPosX, fPosY - INTERFACE_RATIO(5));
	
	RECT rect;
	rect.top=0;
	rect.bottom=0;

	for (long j=0;j<INVENTORY_Y;j++)
		for (long i=0;i<INVENTORY_X;i++)
		{
			INTERACTIVE_OBJ * io = inventory[_sNum][i][j].io;

			if ((io!=NULL) && (inventory[_sNum][i][j].show))
			{
				TextureContainer * tc=io->inv;
				TextureContainer * tc2=NULL;

				if (NeedHalo(io)) 
					tc2 = io->inv->TextureHalo; 

				if (tc!=NULL) 
				{
					float px = fPosX + i*INTERFACE_RATIO(32) + INTERFACE_RATIO(7);
					float py = fPosY + j*INTERFACE_RATIO(32) + INTERFACE_RATIO(6);

					if (tc->m_pddsSurface) 
					{
						D3DCOLOR color;

						if ((io->poisonous) && (io->poisonous_count!=0))
							color=0xFF00FF00;
						else color=D3DCOLORWHITE;

						EERIEDrawBitmap(GDevice,
							px,
							py,

							INTERFACE_RATIO_DWORD(tc->m_dwWidth),
							INTERFACE_RATIO_DWORD(tc->m_dwHeight),

							0.001f,
							tc,color);

						if (io==FlyingOverIO)
						{
							SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
							SETALPHABLEND(GDevice,true);
							EERIEDrawBitmap(GDevice,
								px,
								py,

								INTERFACE_RATIO_DWORD(tc->m_dwWidth),
								INTERFACE_RATIO_DWORD(tc->m_dwHeight),

								0.001f,
								tc,D3DCOLORWHITE);
							SETALPHABLEND(GDevice,false);
						}
						else
						{
							if (io->ioflags & IO_CAN_COMBINE) 
							{
								float fColorPulse	=	255.f * fabs( cos( DEG2RAD( fDecPulse ) ) );
								DWORD dwColor		=	ARX_CLEAN_WARN_CAST_DWORD(fColorPulse);

								SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
								SETALPHABLEND(GDevice,true);
								EERIEDrawBitmap(GDevice,
									px,
									py,

									INTERFACE_RATIO_DWORD(tc->m_dwWidth),
									INTERFACE_RATIO_DWORD(tc->m_dwHeight),

									0.001f,
									tc,(dwColor<<16)|(dwColor<<8)|dwColor);
								SETALPHABLEND(GDevice,false);
							}
						}
					}

					if (tc2)
					{
						ARX_INTERFACE_HALO_Render(
							io->halo.color.r, io->halo.color.g, io->halo.color.b,
							io->halo.flags,
							tc, tc2,
							px,
							py, INTERFACE_RATIO(1), INTERFACE_RATIO(1));
					}

					if ((io->ioflags & IO_ITEM) && (io->_itemdata->count!=1))
						ARX_INTERFACE_DrawNumber(px, py, io->_itemdata->count, 3, D3DCOLORWHITE);
				}
			}
		}
}

extern TextureContainer * stealth_gauge_tc;
extern float CURRENT_PLAYER_COLOR;
extern long EXTERNALVIEW;
//-----------------------------------------------------------------------------
// Stealth Gauge Drawing
//-----------------------------------------------------------------------------
void ARX_INTERFACE_Draw_Stealth_Gauge()
{
	if ((stealth_gauge_tc) && (!CINEMASCOPE))
	{
		float v=GetPlayerStealth();

		if (CURRENT_PLAYER_COLOR<v)
		{
			float px = INTERFACE_RATIO(InventoryX) + INTERFACE_RATIO(110);
			if (px < INTERFACE_RATIO(10)) px = INTERFACE_RATIO(10);

			float py = DANAESIZY - INTERFACE_RATIO(126 + 32);
			float t=v-CURRENT_PLAYER_COLOR;

			if (t>=15) v=1.f;
			else v=(t*DIV15)*0.9f+0.1f;

			D3DCOLOR col=_EERIERGB(v);
			SETALPHABLEND(GDevice, true);
			SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);

			GDevice->SetRenderState( D3DRENDERSTATE_ZENABLE,false);
			EERIEDrawBitmap(GDevice,
			                px, py, INTERFACE_RATIO_DWORD(stealth_gauge_tc->m_dwWidth), INTERFACE_RATIO_DWORD(stealth_gauge_tc->m_dwHeight), 0.01f,
				stealth_gauge_tc,col);

			danaeApp.EnableZBuffer();
			SETALPHABLEND(GDevice, false);
		}
	}
}

//-----------------------------------------------------------------------------
// Damaged Equipment Drawing
//-----------------------------------------------------------------------------
void ARX_INTERFACE_DrawDamagedEquipment()
{
	if (CINEMASCOPE || BLOCK_PLAYER_CONTROLS) return;

	if (player.Interface & INTER_INVENTORYALL) return;
	
	long needdraw=0;	

	for (long i=0;i<5;i++)
	{
		if (iconequip[i])
		{
			long eq=-1;

			switch (i)
			{
				case 0:
					eq = EQUIP_SLOT_WEAPON;
					break;
				case 1:
					eq = EQUIP_SLOT_SHIELD;
					break;
				case 2:
					eq = EQUIP_SLOT_HELMET;
					break;
				case 3:
					eq = EQUIP_SLOT_ARMOR;
					break;
				case 4:
					eq = EQUIP_SLOT_LEGGINGS;
					break;
			}

			if (player.equiped[eq]>0)
			{
				INTERACTIVE_OBJ * io=inter.iobj[player.equiped[eq]];
				float ratio=io->durability/io->max_durability;

				if (ratio<=0.5f) 
				{
					needdraw|=1<<i;
				}
			}
		}
	}

	if (needdraw)
	{
		SETALPHABLEND(GDevice, true);
		SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);

		danaeApp.EnableZBuffer();
		GDevice->SetRenderState( D3DRENDERSTATE_CULLMODE,D3DCULL_NONE);
		SETZWRITE(GDevice, true);
		GDevice->SetRenderState( D3DRENDERSTATE_FOGENABLE, false);
	
		float px = INTERFACE_RATIO(InventoryX) + INTERFACE_RATIO(10 + 32 + 100);

		if ( px < INTERFACE_RATIO( 10 + 32 ) ) px = INTERFACE_RATIO( 10 + 32 );

		float py = DANAESIZY - INTERFACE_RATIO(158);
		
		for (long i=0;i<5;i++)
		{
			if ( (needdraw&(1<<i))&&
				 (iconequip[i]) )
			{
				long eq=-1;

				switch (i)
				{
					case 0:
						eq = EQUIP_SLOT_WEAPON;
						break;
					case 1:
						eq = EQUIP_SLOT_SHIELD;
						break;
					case 2:
						eq = EQUIP_SLOT_HELMET;
						break;
					case 3:
						eq = EQUIP_SLOT_ARMOR;
						break;
					case 4:
						eq = EQUIP_SLOT_LEGGINGS;
						break;
				}

				if (player.equiped[eq]>0)
				{
					INTERACTIVE_OBJ * io=inter.iobj[player.equiped[eq]];
					float ratio=io->durability/io->max_durability;
					D3DCOLOR col=EERIERGB(1.f-ratio,ratio,0);
					EERIEDrawBitmap2(GDevice,
						px,py,
					                 INTERFACE_RATIO_DWORD(iconequip[i]->m_dwWidth), INTERFACE_RATIO_DWORD(iconequip[i]->m_dwHeight),
						0.001f,
						iconequip[i],col);		
				}
			}
		}

		currpos += ARX_CLEAN_WARN_CAST_LONG(INTERFACE_RATIO(33.f));
		SETALPHABLEND(GDevice, false);
	}
}



//-----------------------------------------------------------------------------
void DrawBookInterfaceItem(LPDIRECT3DDEVICE7 m_pd3dDevice,TextureContainer *tc,float x,float y,float z)
{
	if ((tc) && (tc->m_pddsSurface))
	{
		EERIEDrawBitmap2(m_pd3dDevice,
			(x+BOOKDECX)*Xratio,
			(y+BOOKDECY)*Yratio,
			(float)(tc->m_dwWidth)*Xratio,
			(float)(tc->m_dwHeight)*Yratio,
			z, tc, BOOKINTERFACEITEMCOLOR);
	}
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_RELEASESOUND()
{
	ARX_SOUND_PlayInterface(SND_MENU_RELEASE);
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_ERRORSOUND()
{
	ARX_SOUND_PlayInterface(SND_MENU_CLICK);
}

//-----------------------------------------------------------------------------
BOOL CheckAttributeClick(float x, float y, float * val, TextureContainer * tc)
{
	BOOL rval=FALSE;
	float t;
	t=*val;

	if (MouseInBookRect(x,y,x+32,y+32))
	{
		rval=TRUE;

		if (((BOOKBUTTON & 1) || (BOOKBUTTON & 2))&& (tc))
			DrawBookInterfaceItem(GDevice,tc,x,y);

		if (!(BOOKBUTTON & 1) && (LASTBOOKBUTTON & 1))
		{
			if ((player.Attribute_Redistribute > 0)) 
			{
				player.Attribute_Redistribute--;
				t++;
				*val=t;
				ARX_INTERFACE_RELEASESOUND();
			}
			else ARX_INTERFACE_ERRORSOUND();
		}

		if (!(BOOKBUTTON & 2) && (LASTBOOKBUTTON & 2))
		{
			if (ARXmenu.currentmode == AMCM_NEWQUEST)
			{
				if ((t >6) && (player.level==0))
				{
					player.Attribute_Redistribute++;
					t --;
					*val=t;
					ARX_INTERFACE_RELEASESOUND();
				}
				else ARX_INTERFACE_ERRORSOUND();
			}
			else ARX_INTERFACE_ERRORSOUND();
		}
	}

	return rval;
}

//-----------------------------------------------------------------------------
BOOL CheckSkillClick(float x, float y, float * val, TextureContainer * tc, float * oldval)
{
	BOOL rval=FALSE;
	float t,ot;
	t=*val;
	ot=*oldval;

	if ( MouseInBookRect( x, y, x + 32, y + 32 ) )
	{
		rval=TRUE;

		if (((BOOKBUTTON & 1) || (BOOKBUTTON & 2))&& (tc)) 
			DrawBookInterfaceItem(GDevice,tc,x,y);

		if (!(BOOKBUTTON & 1) && (LASTBOOKBUTTON & 1)) 
		{ 
			if ((player.Skill_Redistribute > 0)) 
			{ 
				player.Skill_Redistribute--; 
				t++; 
				*val=t;
				ARX_INTERFACE_RELEASESOUND();
			}
			else ARX_INTERFACE_ERRORSOUND();
		}

		if (!(BOOKBUTTON & 2) && (LASTBOOKBUTTON & 2)) 
		{ 
			if (ARXmenu.currentmode == AMCM_NEWQUEST)
			{
				if ((t > ot) && (player.level==0))
				{ 
					player.Skill_Redistribute++; 
					t --; 
					*val=t;
					ARX_INTERFACE_RELEASESOUND();
				}	
				else ARX_INTERFACE_ERRORSOUND();
			}
			else ARX_INTERFACE_ERRORSOUND();
		} 
	}

	return rval;
}

//#######################################################################################################
//AFFICHAGE ICONE DE SPELLS DE DURATION
//#######################################################################################################
//---------------------------------------------------------------------------
void StdDraw(float posx,float posy,D3DCOLOR color,TextureContainer * tcc,long flag,long i)
{
	TextureContainer * tc;

	if (tcc==NULL)
	{
		if (ITC.unknown==NULL) ITC.unknown= MakeTCFromFile("Graph\\interface\\icons\\spell_unknown.bmp");

		tc=ITC.unknown;
	}
	else tc=tcc;

	if (tc)
	{
		EERIEDrawBitmap(GDevice, posx, posy, INTERFACE_RATIO_DWORD(tc->m_dwWidth) * 0.5f, INTERFACE_RATIO_DWORD(tc->m_dwHeight) * 0.5f, 0.01f, tc, color);

		if (flag & 2)
		{
			SETBLENDMODE(GDevice,D3DBLEND_ZERO,D3DBLEND_ONE);

			EERIEDrawBitmap(GDevice,posx-1,posy-1,INTERFACE_RATIO_DWORD(tc->m_dwWidth)*0.5f,INTERFACE_RATIO_DWORD(tc->m_dwHeight)*0.5f,0.0001f, tc,color);
			EERIEDrawBitmap(GDevice,posx+1,posy+1,INTERFACE_RATIO_DWORD(tc->m_dwWidth)*0.5f,INTERFACE_RATIO_DWORD(tc->m_dwHeight)*0.5f,0.0001f, tc,color);

			SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
		}

		if (!(flag & 1))
		{
			if (!(player.Interface & INTER_COMBATMODE))
				if (MouseInRect(posx,posy, posx+INTERFACE_RATIO(32),posy+INTERFACE_RATIO(32)))
				{
					SpecialCursor=CURSOR_INTERACTION_ON;

					if ((LastMouseClick & 1) && (!(EERIEMouseButton & 1)) )
					{
						if (flag & 2)
						{
							if (Precast[PRECAST_NUM].typ >= 0)
								_tcscpy(WILLADDSPEECH, spellicons[Precast[PRECAST_NUM].typ].name);

							WILLADDSPEECHTIME = ARXTimeUL(); 
						}
						else
						{
							if (spells[i].type >= 0)
								_tcscpy(WILLADDSPEECH, spellicons[spells[i].type].name);

							WILLADDSPEECHTIME = ARXTimeUL(); 
						}
					}

					if (EERIEMouseButton & 4)
					{
						if (flag & 2) 
						{
							ARX_SPELLS_Precast_Launch(PRECAST_NUM);
							EERIEMouseButton&=~4;
						}
						else
						{
							ARX_SPELLS_AbortSpellSound();
							EERIEMouseButton&=~4;
							spells[i].tolive=0;							
						}
					}
				}
		}
	}
}
//---------------------------------------------------------------------------
void ManageSpellIcon(long i,float rrr,long flag)
{
	float POSX=DANAESIZX-INTERFACE_RATIO(35);
	long lPOSX;
	F2L(POSX,&lPOSX);
	D3DCOLOR color;
	float posx = POSX+lSLID_VALUE;
	float posy = (float)currpos;
	long typ=spells[i].type;
	
	if (flag & 1)
	{
		color=D3DRGB(rrr,0,0);
	}
	else if (flag & 2)
	{
		color=D3DRGB(0,rrr*DIV2,rrr);
		float px = INTERFACE_RATIO(InventoryX) + INTERFACE_RATIO(110);

		if (px < INTERFACE_RATIO(10)) px = INTERFACE_RATIO(10);

		posx = px + INTERFACE_RATIO(33 + 33 + 33) + PRECAST_NUM * INTERFACE_RATIO(33); 
		posy =DANAESIZY - INTERFACE_RATIO(126+32); // niveau du stealth
		typ = i;
	}
	else
	{
		color=D3DRGB(rrr,rrr,rrr);
	}

	bool bOk=true;

	if (spells[i].bDuration)
	{
		if(	(player.mana<20)||
			((spells[i].timcreation+spells[i].tolive-ARXTime)<2000) )
		{
			if(ucFlick&1) bOk=false;
		}
	}
	else
	{
		if(player.mana<20)
		{
			if(ucFlick&1) bOk=false;
		}
	}

	if ( ( (bOk) && (typ>=0) &&	(typ<SPELL_COUNT) ) || (flag == 2) )
		StdDraw(posx,posy,color,spellicons[typ].tc,flag,i);

	currpos += ARX_CLEAN_WARN_CAST_LONG(INTERFACE_RATIO(33.f));
}
extern float GLOBAL_LIGHT_FACTOR;
//-----------------------------------------------------------------------------
void ARX_INTERFACE_ManageOpenedBook_Finish()
{
	bool bOldGATI8500	=	bGATI8500;
	bool bOldSoftRender	=	bSoftRender;
	bGATI8500			=	false;
	bSoftRender			=	false;

	SETZWRITE(GDevice, TRUE );

	danaeApp.EnableZBuffer();

	if ((player.Interface & INTER_MAP ) &&  (!(player.Interface & INTER_COMBATMODE))) 
	{
		if (Book_Mode==1)
		{
			EERIE_3D angle;
			EERIE_3D pos;
			
			EERIE_LIGHT tl;
			memcpy(&tl,&DynLight[0],sizeof(EERIE_LIGHT));

			DynLight[0].pos.x=500.f;
			DynLight[0].pos.y=-1960.f;
			DynLight[0].pos.z=1590.f;
			
				DynLight[0].exist=1;	
				DynLight[0].rgb.r=0.6f;
				DynLight[0].rgb.g=0.7f;
				DynLight[0].rgb.b=0.9f;
				DynLight[0].intensity=1.8f;
		
			DynLight[0].fallstart=4520.f;
			DynLight[0].fallend = DynLight[0].fallstart + 600.f; 
			DynLight[0].rgb255.r=DynLight[0].rgb.r*255.f;
			DynLight[0].rgb255.g=DynLight[0].rgb.g*255.f;
			DynLight[0].rgb255.b=DynLight[0].rgb.b*255.f;
			DynLight[0].falldiff=DynLight[0].fallend-DynLight[0].fallstart;
			DynLight[0].falldiffmul=1.f/DynLight[0].falldiff;
			DynLight[0].precalc=DynLight[0].intensity*GLOBAL_LIGHT_FACTOR;
			
			EERIE_CAMERA * oldcam=ACTIVECAM;
			bookcam.centerx = DANAECENTERX; 
			bookcam.centery = DANAECENTERY; 
			SetActiveCamera(&bookcam);
			PrepareCamera(&bookcam);
			pos.x = 0.f; 
			pos.y = 0.f; 
			pos.z = 2100.f; 
			
			angle.a=0.f;
			angle.b=0.f;
			angle.g=0.f;
			
			PDL[0]=&DynLight[0];
			TOTPDL=1;
			
			long found2=0;
			float n;
			long xpos=0;
			long ypos=0;
			GDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, false);

			for (long i=0;i<NB_RUNES;i++)
			{
				if (necklace.runes[i])
				{
					F2L((382+xpos*45+BOOKDECX)*Xratio,&bookcam.centerx);
					F2L((100+ypos*64+BOOKDECY)*Yratio,&bookcam.centery);

					SetActiveCamera(&bookcam);
					PrepareCamera(&bookcam);					
					
					// First draw the lace
					angle.b=0.f;

					if (player.rune_flags & (1<<i))
					{
						DrawEERIEInter(GDevice,necklace.lacet,&angle,&pos,NULL);

						if (necklace.runes[i]->angle.b!=0.f)
						{
							if (necklace.runes[i]->angle.b>300.f)
								necklace.runes[i]->angle.b=300.f;

							angle.b=EEsin((float)ARX_TIME_Get()*DIV200)*necklace.runes[i]->angle.b*DIV40;
						}

						necklace.runes[i]->angle.b-=_framedelay*0.2f;

						if (necklace.runes[i]->angle.b<0.f) necklace.runes[i]->angle.b=0.f;
						
						DynLight[0].exist=0;	
						float tt;
						
						F2L((382+xpos*45+BOOKDECX+3)*Xratio,&bookcam.centerx);
						F2L((100+ypos*64+BOOKDECY+2)*Yratio,&bookcam.centery);
						SetActiveCamera(&bookcam);
						PrepareCamera(&bookcam);					
						
						tt=angle.b;
						angle.b=-20.f;
						angle.b=tt;
						angle.b-=20.f;
						
						angle.b+=20.f;
						SETZWRITE(GDevice,TRUE);
						SETALPHABLEND(GDevice,FALSE);
						DynLight[0].exist=1;	
						
						F2L((382+xpos*45+BOOKDECX)*Xratio,&bookcam.centerx);
						F2L((100+ypos*64+BOOKDECY)*Yratio,&bookcam.centery);
						SetActiveCamera(&bookcam);
						PrepareCamera(&bookcam);					
						
						
						// Now draw the rune
						DrawEERIEInter(GDevice,necklace.runes[i],&angle,&pos,NULL);

						if(bRenderInterList)
						{
							PopAllTriangleList(true);
						}
						
						xpos++;						

						if (xpos > 4)
						{
							xpos = 0;
							ypos++;
						}
						
						// Checks for Mouse floating over a rune...
						if ((!found2) &&
							MouseInRect(BBOXMIN.x, BBOXMIN.y, BBOXMAX.x, BBOXMAX.y))
						{
							long r=0;

							for (long j=0;j<necklace.runes[i]->nbfaces;j++)
							{
								n=PtIn2DPolyProj( necklace.runes[i], &necklace.runes[i]->facelist[j] , (float)DANAEMouse.x, (float)DANAEMouse.y);

								if (n!=0.f) 
								{ 
									r=1;
									break;
								}
							}

							if (r)
							{
								SETALPHABLEND(GDevice,TRUE);
								SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
								DrawEERIEInter(GDevice,necklace.runes[i],&angle,&pos,NULL);

								necklace.runes[i]->angle.b+=_framedelay*2.f;

								if(bRenderInterList)
								{
									PopAllTriangleList(true);
								}
								
								SETALPHABLEND(GDevice,FALSE);

								SpecialCursor=CURSOR_INTERACTION_ON;

								if ((EERIEMouseButton & 1)  && !(LastMouseClick & 1))
									if (LastRune!=i)
									{
										switch(i)
										{
										case RUNE_AAM:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "AAM", ARX_SOUND_GetDuration(SND_SYMB_AAM));
											ARX_SOUND_PlayInterface(SND_SYMB_AAM);
											break;
										case RUNE_CETRIUS:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "CETRIUS", ARX_SOUND_GetDuration(SND_SYMB_CETRIUS));
											ARX_SOUND_PlayInterface(SND_SYMB_CETRIUS);
											break;
										case RUNE_COMUNICATUM:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "COMUNICATUM", ARX_SOUND_GetDuration(SND_SYMB_COMUNICATUM));
											ARX_SOUND_PlayInterface(SND_SYMB_COMUNICATUM);
											break;
										case RUNE_COSUM:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "COSUM", ARX_SOUND_GetDuration(SND_SYMB_COSUM));
											ARX_SOUND_PlayInterface(SND_SYMB_COSUM);
											break;
										case RUNE_FOLGORA:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "FOLGORA", ARX_SOUND_GetDuration(SND_SYMB_FOLGORA));
											ARX_SOUND_PlayInterface(SND_SYMB_FOLGORA);
											break;
										case RUNE_FRIDD:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "FRIDD", ARX_SOUND_GetDuration(SND_SYMB_FRIDD));
											ARX_SOUND_PlayInterface(SND_SYMB_FRIDD);
											break;
										case RUNE_KAOM:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "KAOM", ARX_SOUND_GetDuration(SND_SYMB_KAOM));
											ARX_SOUND_PlayInterface(SND_SYMB_KAOM);
											break;
										case RUNE_MEGA:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "MEGA", ARX_SOUND_GetDuration(SND_SYMB_MEGA));
											ARX_SOUND_PlayInterface(SND_SYMB_MEGA);
											break;
										case RUNE_MORTE:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "MORTE", ARX_SOUND_GetDuration(SND_SYMB_MORTE));
											ARX_SOUND_PlayInterface(SND_SYMB_MORTE);
											break;
										case RUNE_MOVIS:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "MOVIS", ARX_SOUND_GetDuration(SND_SYMB_MOVIS));
											ARX_SOUND_PlayInterface(SND_SYMB_MOVIS);
											break;
										case RUNE_NHI:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "NHI", ARX_SOUND_GetDuration(SND_SYMB_NHI));
											ARX_SOUND_PlayInterface(SND_SYMB_NHI);
											break;
										case RUNE_RHAA:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "RHAA", ARX_SOUND_GetDuration(SND_SYMB_RHAA));
											ARX_SOUND_PlayInterface(SND_SYMB_RHAA);
											break;
										case RUNE_SPACIUM:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "SPACIUM", ARX_SOUND_GetDuration(SND_SYMB_SPACIUM));
											ARX_SOUND_PlayInterface(SND_SYMB_SPACIUM);
											break;
										case RUNE_STREGUM:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "STREGUM", ARX_SOUND_GetDuration(SND_SYMB_STREGUM));
											ARX_SOUND_PlayInterface(SND_SYMB_STREGUM);
											break;
										case RUNE_TAAR:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "TAAR", ARX_SOUND_GetDuration(SND_SYMB_TAAR));
											ARX_SOUND_PlayInterface(SND_SYMB_TAAR);
											break;
										case RUNE_TEMPUS:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "TEMPUS", ARX_SOUND_GetDuration(SND_SYMB_TEMPUS));
											ARX_SOUND_PlayInterface(SND_SYMB_TEMPUS);
											break;
										case RUNE_TERA:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "TERA", ARX_SOUND_GetDuration(SND_SYMB_TERA));
											ARX_SOUND_PlayInterface(SND_SYMB_TERA);
											break;
										case RUNE_VISTA:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "VISTA", ARX_SOUND_GetDuration(SND_SYMB_VISTA));
											ARX_SOUND_PlayInterface(SND_SYMB_VISTA);
											break;
										case RUNE_VITAE:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "VITAE", ARX_SOUND_GetDuration(SND_SYMB_VITAE));
											ARX_SOUND_PlayInterface(SND_SYMB_VITAE);
											break;
										case RUNE_YOK:
											ARX_SPELLS_RequestSymbolDraw(inter.iobj[0], "YOK", ARX_SOUND_GetDuration(SND_SYMB_YOK));
											ARX_SOUND_PlayInterface(SND_SYMB_YOK);
											break;
										}
									}

									LastRune=i;
							}
						}
					}
				}
			}

			GDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, true);
			
			SETCULL( GDevice, D3DCULL_CCW);

			if (!found2) LastRune=-1;
			
			// Now Draws Spells for this level...
			ARX_PLAYER_ComputePlayerFullStats();
			
			float posx=0;
			float posy=0;
			float fPosX = 0;
			float fPosY = 0;
			bool	bFlyingOver = false;
			
			for (int i=0; i < SPELL_COUNT; i++)
			{
				if ((spellicons[i].level==Book_SpellPage) && (!spellicons[i].bSecret))
				{
					// check if player can cast it
					bool bOk = true;
					long j = 0;

					while ((j < 4) && (spellicons[i].symbols[j] != 255))
					{
						if (!(player.rune_flags & (1<<spellicons[i].symbols[j])))
						{
							bOk = false;
						}

						j++;
					}

					if (bOk)
					{
						fPosX = 170.f+posx*85.f;
						fPosY = 135.f+posy*70.f;
						long flyingover = 0;
						
						if (MouseInBookRect(fPosX, fPosY, fPosX+48, fPosY+48))
						{
							bFlyingOver = true;
							flyingover = 1;
							
							SpecialCursor=CURSOR_INTERACTION_ON;
							FLYING_OVER = i;
							DrawBookTextCenter( 208, 90, spellicons[i].name,0,0x00FF00FF,InBookFont);
							
							for (long li=0;li<MAX_SPEECH;li++)
							{
								if (speech[li].timecreation>0) FLYING_OVER=0;
							}
							
							if(	(OLD_FLYING_OVER != FLYING_OVER)||
								(INTERNATIONAL_MODE) )
							{
								OLD_FLYING_OVER=FLYING_OVER;
								pTextManage->Clear();
								UNICODE_ARXDrawTextCenteredScroll(ARX_CLEAN_WARN_CAST_FLOAT(DANAECENTERX),
									12,
									(DANAECENTERX)*0.82f,
									spellicons[i].description,
									RGB(232,204,143),
									0x00FF00FF,
									hFontInGame,
									1000,
									0.01f,
									2,
									INTERNATIONAL_MODE?0:max(3000, 70*_tcslen(spellicons[i].description)));
							}
							
							
							long count=0;

							for (long j=0;j<6;j++)
							{
								if (spellicons[i].symbols[j]!=255)
									count++;
							}

							for (int j=0;j<6;j++)
							{
								if (spellicons[i].symbols[j]!=255)
								{
									pos.x = (240-(count*32)*DIV2+j*32);
									pos.y = (306);
									DrawBookInterfaceItem(GDevice, 
										necklace.pTexTab[spellicons[i].symbols[j]]
									                      , pos.x, pos.y);
									
								}
							}
						}

						if (spellicons[i].tc)
						{
							SETALPHABLEND(GDevice, true);
							SETBLENDMODE(GDevice,D3DBLEND_ZERO,D3DBLEND_INVSRCCOLOR);

							if (flyingover)
							{
								BOOKINTERFACEITEMCOLOR=0xFFFFFFFF;

								if ((EERIEMouseButton & 1)  && !(LastMouseClick & 1))
								{
									player.SpellToMemorize.bSpell = true;

									for (long j=0;j<6;j++)
									{
										player.SpellToMemorize.iSpellSymbols[j] = spellicons[i].symbols[j];
									}

									player.SpellToMemorize.lTimeCreation = ARXTimeUL(); 
								}
							}
							else BOOKINTERFACEITEMCOLOR = 0xFFa8d0df; 

							DrawBookInterfaceItem(GDevice, spellicons[i].tc, fPosX, fPosY);

							BOOKINTERFACEITEMCOLOR = D3DCOLORWHITE;
							SETALPHABLEND(GDevice, false);
						}
						
						posx ++;

						if (posx>=2)
						{
							posx = 0;
							posy ++;
						}
					}
				}
			}
			
			if (!bFlyingOver)
			{
				OLD_FLYING_OVER = -1;
				FLYING_OVER = -1;
			}
			
			memcpy(&DynLight[0],&tl,sizeof(EERIE_LIGHT));			
			SetActiveCamera(oldcam);
			PrepareCamera(oldcam);
		}
	}

	// Restore bGATI8500 + bSoftRender.
	bSoftRender			=	bOldSoftRender;
	bGATI8500			=	bOldGATI8500;
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_ManageOpenedBook()
{
	bool bOldGATI8500	=	bGATI8500;
	bool bOldSoftRender	=	bSoftRender;
	bGATI8500			=	false;
	bSoftRender			=	false;

	GDevice->SetRenderState( D3DRENDERSTATE_FOGENABLE, FALSE );
	
	if (ITC.questbook==NULL)
	{
		ITC.playerbook=MakeTCFromFile("Graph\\Interface\\book\\character_sheet\\char_sheet_book.bmp");
		ITC.ic_casting=MakeTCFromFile("Graph\\Interface\\book\\character_sheet\\buttons_carac\\icone_casting.bmp");
		ITC.ic_close_combat=MakeTCFromFile("Graph\\Interface\\book\\character_sheet\\buttons_carac\\icone_close_combat.bmp");
		ITC.ic_constitution =MakeTCFromFile("Graph\\Interface\\book\\character_sheet\\buttons_carac\\icone_constit.bmp");
		ITC.ic_defense=MakeTCFromFile("Graph\\Interface\\book\\character_sheet\\buttons_carac\\icone_defense.bmp");
		ITC.ic_dexterity=MakeTCFromFile("Graph\\Interface\\book\\character_sheet\\buttons_carac\\icone_dext.bmp");
		ITC.ic_etheral_link=MakeTCFromFile("Graph\\Interface\\book\\character_sheet\\buttons_carac\\icone_etheral_link.bmp");
		ITC.ic_mind=MakeTCFromFile("Graph\\Interface\\book\\character_sheet\\buttons_carac\\icone_intel.bmp");
		ITC.ic_intuition=MakeTCFromFile("Graph\\Interface\\book\\character_sheet\\buttons_carac\\icone_intuition.bmp");
		ITC.ic_mecanism=MakeTCFromFile("Graph\\Interface\\book\\character_sheet\\buttons_carac\\icone_mecanism.bmp");
		ITC.ic_object_knowledge=MakeTCFromFile("Graph\\Interface\\book\\character_sheet\\buttons_carac\\icone_obj_knowledge.bmp");
		ITC.ic_projectile=MakeTCFromFile("Graph\\Interface\\book\\character_sheet\\buttons_carac\\icone_projectile.bmp");
		ITC.ic_stealth=MakeTCFromFile("Graph\\Interface\\book\\character_sheet\\buttons_carac\\icone_stealth.bmp");
		ITC.ic_strength=MakeTCFromFile("Graph\\Interface\\book\\character_sheet\\buttons_carac\\icone_strenght.bmp");
		
		ITC.questbook=MakeTCFromFile("Graph\\Interface\\book\\questbook.bmp");
		ITC.pTexSpellBook = MakeTCFromFile("Graph\\Interface\\book\\SpellBook.bmp");
		ITC.bookmark_char=MakeTCFromFile("Graph\\Interface\\book\\bookmark_char.bmp");
		ITC.bookmark_magic=MakeTCFromFile("Graph\\Interface\\book\\bookmark_magic.bmp");
		ITC.bookmark_map=MakeTCFromFile("Graph\\Interface\\book\\bookmark_map.bmp");
		ITC.bookmark_quest=MakeTCFromFile("Graph\\Interface\\book\\bookmark_quest.bmp");

		ITC.accessible_1=MakeTCFromFile("Graph\\Interface\\book\\Accessible\\accessible_1.bmp");
		ITC.accessible_2=MakeTCFromFile("Graph\\Interface\\book\\Accessible\\accessible_2.bmp");
		ITC.accessible_3=MakeTCFromFile("Graph\\Interface\\book\\Accessible\\accessible_3.bmp");
		ITC.accessible_4=MakeTCFromFile("Graph\\Interface\\book\\Accessible\\accessible_4.bmp");
		ITC.accessible_5=MakeTCFromFile("Graph\\Interface\\book\\Accessible\\accessible_5.bmp");
		ITC.accessible_6=MakeTCFromFile("Graph\\Interface\\book\\Accessible\\accessible_6.bmp");
		ITC.accessible_7=MakeTCFromFile("Graph\\Interface\\book\\Accessible\\accessible_7.bmp");
		ITC.accessible_8=MakeTCFromFile("Graph\\Interface\\book\\Accessible\\accessible_8.bmp");
		ITC.accessible_9=MakeTCFromFile("Graph\\Interface\\book\\Accessible\\accessible_9.bmp");
		ITC.accessible_10=MakeTCFromFile("Graph\\Interface\\book\\Accessible\\accessible_10.bmp");
		ITC.current_1=MakeTCFromFile("Graph\\Interface\\book\\Current_Page\\Current_1.bmp");
		ITC.current_2=MakeTCFromFile("Graph\\Interface\\book\\Current_Page\\Current_2.bmp");
		ITC.current_3=MakeTCFromFile("Graph\\Interface\\book\\Current_Page\\Current_3.bmp");
		ITC.current_4=MakeTCFromFile("Graph\\Interface\\book\\Current_Page\\Current_4.bmp");
		ITC.current_5=MakeTCFromFile("Graph\\Interface\\book\\Current_Page\\Current_5.bmp");
		ITC.current_6=MakeTCFromFile("Graph\\Interface\\book\\Current_Page\\Current_6.bmp");
		ITC.current_7=MakeTCFromFile("Graph\\Interface\\book\\Current_Page\\Current_7.bmp");
		ITC.current_8=MakeTCFromFile("Graph\\Interface\\book\\Current_Page\\Current_8.bmp");
		ITC.current_9=MakeTCFromFile("Graph\\Interface\\book\\Current_Page\\Current_9.bmp");
		ITC.current_10=MakeTCFromFile("Graph\\Interface\\book\\Current_Page\\Current_10.bmp");
		ITC.heropageleft=MakeTCFromFile("Graph\\Interface\\book\\character_sheet\\Hero_left_X24_Y24.BMP" );
		ITC.heropageright=MakeTCFromFile("Graph\\Interface\\book\\character_sheet\\Hero_right_X305_Y270.BMP" );
		ITC.symbol_mega = NULL; 
		ITC.symbol_vista = NULL; 
		ITC.symbol_aam = NULL; 
		ITC.symbol_taar = NULL; 
		ITC.symbol_yok = NULL; 
		
		ITC.pTexCursorRedist=MakeTCFromFile("Graph\\Interface\\cursors\\add_points.bmp" );
		
		ITC.pTexCornerLeft=MakeTCFromFile("Graph\\Interface\\book\\Left_corner_original.bmp");
		ITC.pTexCornerRight=MakeTCFromFile("Graph\\Interface\\book\\Right_corner_original.bmp");
		
		ARX_Allocate_Text(ITC.lpszULevel, _T("system_charsheet_player_lvl"));
		ARX_Allocate_Text(ITC.lpszUXp, _T("system_charsheet_player_xp"));
		
		ANIM_Set(&player.useanim,herowaitbook);

		player.useanim.flags|=EA_LOOP;
		
		ARXOldTimeMenu=ARXTimeMenu=ARX_TIME_Get();
		ARXDiffTimeMenu=0;
	}
	
	BOOKDECX = 0;
	BOOKDECY = 0;
	GDevice->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTFP_LINEAR);
	GDevice->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTFP_LINEAR);
	
	if (ARXmenu.currentmode != AMCM_NEWQUEST)
	{
 
 
		GDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS);

		if (Book_Mode == 0)
		{
			DrawBookInterfaceItem(GDevice, ITC.playerbook, 97, 64, 0.9999f); 
		}
		else if (Book_Mode == 1)
		{
			DrawBookInterfaceItem(GDevice, ITC.pTexSpellBook, 97, 64, 0.9999f);
		}
		else if (Book_Mode == 2)
		{
			DrawBookInterfaceItem(GDevice, ITC.questbook, 97, 64, 0.9999f);
			  }
			  else
			  {
			DrawBookInterfaceItem(GDevice,ITC.questbook, 97, 64, 0.9999f);
		}

		GDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);
	}
	else 
	{
		float x, y;

		x = 0;
		
		if ( ITC.playerbook )
		{


			x = ARX_CLEAN_WARN_CAST_FLOAT( ( 640 - ITC.playerbook->m_dwWidth ) / 2 );
			y = ARX_CLEAN_WARN_CAST_FLOAT( ( 480 - ITC.playerbook->m_dwHeight ) / 2 );

			DrawBookInterfaceItem( GDevice, ITC.playerbook, x, y );//95.f+2.f,47.f+17.f);
		}

		BOOKDECX = x - 97;
		BOOKDECY = x - 64 + 19;
	}
	
	if (ARXmenu.currentmode != AMCM_NEWQUEST)
	{
		bool bOnglet[11];
		long max_onglet = 0;
		long Book_Page = 1;
		
		//---------------------------------------------------------------------
		// Checks Clicks in bookmarks
		
		// Character Sheet
		if (Book_Mode != 0)
		{
			float px=BOOKMARKS_POS_X;
			float py=BOOKMARKS_POS_Y;
			DrawBookInterfaceItem(GDevice,ITC.bookmark_char,px,py);

			// Check for cursor on charcter sheet bookmark
			if (	ITC.bookmark_char
				&&	MouseInBookRect(px,py,px+ITC.bookmark_char->m_dwWidth,py+ITC.bookmark_char->m_dwHeight))
			{
				// Draw highlighted Character sheet icon
				BOOKINTERFACEITEMCOLOR=0xFF555555;
				SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
				SETALPHABLEND(GDevice,TRUE);
				DrawBookInterfaceItem(GDevice,ITC.bookmark_char,px,py);
				SETALPHABLEND(GDevice,FALSE);
				BOOKINTERFACEITEMCOLOR=0xFFFFFFFF;
			
				// Set cursor to interacting
				SpecialCursor=CURSOR_INTERACTION_ON;

				// Check for click
				if (bookclick.x!=-1)
				{
					ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
					Book_Mode=0; 
					pTextManage->Clear();
				}
			}
		}

		if (Book_Mode != 1)
		{
			if (player.rune_flags)
			{
				float px=BOOKMARKS_POS_X+32;
				float py=BOOKMARKS_POS_Y;
				DrawBookInterfaceItem(GDevice,ITC.bookmark_magic,px,py);

				//////////////// TO BE REDONE/REMOVED - START
				if (NewSpell==1)
				{
					NewSpell=2;

					for (long nk=0;nk<2;nk++) MagFX(BOOKDECX+220.f,BOOKDECY+49.f,0.000001f);
				}

				//////////////// TO BE REDONE/REMOVED - END
				if (	ITC.bookmark_magic 
					&&	MouseInBookRect(px,py,px+ITC.bookmark_magic->m_dwWidth,py+ITC.bookmark_magic->m_dwHeight))				
				{
					// Draw highlighted Magic sheet icon
					BOOKINTERFACEITEMCOLOR=0xFF555555;
					SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);

					SETALPHABLEND(GDevice,TRUE);
					DrawBookInterfaceItem(GDevice,ITC.bookmark_magic,px,py);
					SETALPHABLEND(GDevice,FALSE);
					BOOKINTERFACEITEMCOLOR=0xFFFFFFFF;

					// Set cursor to interacting
					SpecialCursor=CURSOR_INTERACTION_ON;

					// Check for click
					if (bookclick.x!=-1)
					{
						ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						Book_Mode=1;
						pTextManage->Clear();
					}
				}
			}
		}

		if (Book_Mode!=2)
		{
			float px=BOOKMARKS_POS_X+64;
			float py=BOOKMARKS_POS_Y;

			DrawBookInterfaceItem(GDevice,ITC.bookmark_map,px,py);			

			if (	ITC.bookmark_map
				&&	MouseInBookRect(px,py,px+ITC.bookmark_map->m_dwWidth,py+ITC.bookmark_map->m_dwHeight))				
			{
				BOOKINTERFACEITEMCOLOR=0xFF555555;
				SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
				SETALPHABLEND(GDevice,TRUE);
				DrawBookInterfaceItem(GDevice,ITC.bookmark_map,px,py);			
				SETALPHABLEND(GDevice,FALSE);
				BOOKINTERFACEITEMCOLOR=0xFFFFFFFF;

				// Set cursor to interacting
				SpecialCursor=CURSOR_INTERACTION_ON;

				// Check for click
				if (bookclick.x!=-1)
				{
					ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
					Book_Mode=2;
					pTextManage->Clear();
				}
			}
		}

		if (Book_Mode!=3)
		{
			float px=BOOKMARKS_POS_X+96;
			float py=BOOKMARKS_POS_Y;
			DrawBookInterfaceItem(GDevice,ITC.bookmark_quest,px,py);

			if (	ITC.bookmark_quest
				&&	MouseInBookRect(px,py,px+ITC.bookmark_quest->m_dwWidth,py+ITC.bookmark_quest->m_dwHeight))				
			{
				BOOKINTERFACEITEMCOLOR=0xFF555555;
				SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
				SETALPHABLEND(GDevice,TRUE);
				DrawBookInterfaceItem(GDevice,ITC.bookmark_quest,px,py);
				SETALPHABLEND(GDevice,FALSE);
				BOOKINTERFACEITEMCOLOR=0xFFFFFFFF;

				// Set cursor to interacting
				SpecialCursor=CURSOR_INTERACTION_ON;

				// Check for click
				if (bookclick.x!=-1) 
				{
					ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
					Book_Mode=3;
					pTextManage->Clear();
				}
			}
		}
		
		if (Book_Mode==2) max_onglet=8;
		else max_onglet=10;
		
		if (Book_Mode==1) Book_Page = Book_SpellPage;
		else Book_Page = Book_MapPage;
		
		ZeroMemory(&bOnglet, 11*sizeof(bool));

		// calcul de la page de spells
		if (Book_Mode == 1)
		{
			max_onglet = 0;

			for (long i=0; i<SPELL_COUNT; i++)
			{
				if (spellicons[i].bSecret == false)
				{
					long j = 0;
					bool bOk = true;

					while ((j < 4) && (spellicons[i].symbols[j] != 255))
					{
						if (!(player.rune_flags & (1<<spellicons[i].symbols[j])))
						{
							bOk = false;
						}

						j++;
					}

					if (bOk)
					{
						bOnglet[spellicons[i].level] = true;
					}
				}
			}
		}
		else
		{
			memset(&bOnglet, true, (max_onglet+1)*sizeof(bool));
		}
		
		if ((Book_Mode==1) || (Book_Mode==2))
		{
			if (bOnglet[1])
			{
				if (Book_Page!=1) 
				{
					float px=100.f;
					float py=82.f;
					DrawBookInterfaceItem(GDevice,ITC.accessible_1,px,py);

					if (MouseInBookRect(px,py,px+32,py+32))
					{
						BOOKINTERFACEITEMCOLOR=0xFF555555;
						SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
						SETALPHABLEND(GDevice,TRUE);
						DrawBookInterfaceItem(GDevice,ITC.accessible_1,px,py);
						SETALPHABLEND(GDevice,FALSE);
						BOOKINTERFACEITEMCOLOR=0xFFFFFFFF;
						SpecialCursor=CURSOR_INTERACTION_ON;

						if (bookclick.x!=-1)
						{
							Book_Page=1;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(GDevice,ITC.current_1,102.f,82.f);
			}

			if (bOnglet[2])
			{
				if (Book_Page!=2) 
				{
					float px=98.f;
					float py=112.f;
					DrawBookInterfaceItem(GDevice,ITC.accessible_2,px,py);

					if (MouseInBookRect(px,py,px+32,py+32))
					{
						BOOKINTERFACEITEMCOLOR=0xFF555555;
						SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
						SETALPHABLEND(GDevice,TRUE);
						DrawBookInterfaceItem(GDevice,ITC.accessible_2,px,py);
						SETALPHABLEND(GDevice,FALSE);
						BOOKINTERFACEITEMCOLOR=0xFFFFFFFF;
						SpecialCursor=CURSOR_INTERACTION_ON;

						if (bookclick.x!=-1)
						{
							Book_Page=2;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(GDevice,ITC.current_2,100.f,114.f);
			}

			if (bOnglet[3])
			{
				if (Book_Page!=3) 
				{
					float px=97.f;
					float py=143.f;
					DrawBookInterfaceItem(GDevice,ITC.accessible_3,px,py);

					if (MouseInBookRect(px,py,px+32,py+32))
					{
						BOOKINTERFACEITEMCOLOR=0xFF555555;
						SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
						SETALPHABLEND(GDevice,TRUE);
						DrawBookInterfaceItem(GDevice,ITC.accessible_3,px,py);
						SETALPHABLEND(GDevice,FALSE);
						BOOKINTERFACEITEMCOLOR=0xFFFFFFFF;
						SpecialCursor=CURSOR_INTERACTION_ON;

						if (bookclick.x!=-1)
						{
							Book_Page=3;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(GDevice,ITC.current_3,101.f,141.f);
			}

			if (bOnglet[4])
			{
				if (Book_Page!=4)
				{
					float px=95.f;
					float py=170.f;
					DrawBookInterfaceItem(GDevice,ITC.accessible_4,px,py);

					if (MouseInBookRect(px,py,px+32,py+32))
					{
						BOOKINTERFACEITEMCOLOR=0xFF555555;
						SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
						SETALPHABLEND(GDevice,TRUE);
						DrawBookInterfaceItem(GDevice,ITC.accessible_4,px,py);
						SETALPHABLEND(GDevice,FALSE);
						BOOKINTERFACEITEMCOLOR=0xFFFFFFFF;
						SpecialCursor=CURSOR_INTERACTION_ON;

						if (bookclick.x!=-1)
						{
							Book_Page=4;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(GDevice,ITC.current_4,100.f,170.f);
			}

			if (bOnglet[5])
			{
				if (Book_Page!=5) 
				{
					float px=95.f;
					float py=200.f;
					DrawBookInterfaceItem(GDevice,ITC.accessible_5,px,py);

					if (MouseInBookRect(px,py,px+32,py+32))
					{
						BOOKINTERFACEITEMCOLOR=0xFF555555;
						SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
						SETALPHABLEND(GDevice,TRUE);
						DrawBookInterfaceItem(GDevice,ITC.accessible_5,px,py);
						SETALPHABLEND(GDevice,FALSE);
						BOOKINTERFACEITEMCOLOR=0xFFFFFFFF;
						SpecialCursor=CURSOR_INTERACTION_ON;

						if (bookclick.x!=-1)
						{
							Book_Page=5;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(GDevice,ITC.current_5,97.f,199.f);
			}

			if (bOnglet[6])
			{
				if (Book_Page!=6) 
				{
					float px=94.f;
					float py=229.f;
					DrawBookInterfaceItem(GDevice,ITC.accessible_6,px,py);

					if (MouseInBookRect(px,py,px+32,py+32))
					{
						BOOKINTERFACEITEMCOLOR=0xFF555555;
						SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
						SETALPHABLEND(GDevice,TRUE);
						DrawBookInterfaceItem(GDevice,ITC.accessible_6,px,py);
						SETALPHABLEND(GDevice,FALSE);
						BOOKINTERFACEITEMCOLOR=0xFFFFFFFF;
						SpecialCursor=CURSOR_INTERACTION_ON;

						if (bookclick.x!=-1)
						{
							Book_Page=6;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(GDevice,ITC.current_6,103.f,226.f);
			}

			if (bOnglet[7])
			{
				if (Book_Page!=7)
				{
					float px=94.f;
					float py=259.f;
					DrawBookInterfaceItem(GDevice,ITC.accessible_7,px,py);

					if (MouseInBookRect(px,py,px+32,py+32))
					{
						BOOKINTERFACEITEMCOLOR=0xFF555555;
						SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
						SETALPHABLEND(GDevice,TRUE);
						DrawBookInterfaceItem(GDevice,ITC.accessible_7,px,py);
						SETALPHABLEND(GDevice,FALSE);
						BOOKINTERFACEITEMCOLOR=0xFFFFFFFF;
						SpecialCursor=CURSOR_INTERACTION_ON;

						if (bookclick.x!=-1)
						{
							Book_Page=7;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(GDevice,ITC.current_7,101.f,255.f);
			}

			if (bOnglet[8])
			{
				if (Book_Page!=8) 
				{
					float px=92.f;
					float py=282.f;
					DrawBookInterfaceItem(GDevice,ITC.accessible_8,px,py);

					if (MouseInBookRect(px,py,px+32,py+32))
					{
						BOOKINTERFACEITEMCOLOR=0xFF555555;
						SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
						SETALPHABLEND(GDevice,TRUE);
						DrawBookInterfaceItem(GDevice,ITC.accessible_8,px,py);
						SETALPHABLEND(GDevice,FALSE);
						BOOKINTERFACEITEMCOLOR=0xFFFFFFFF;
						SpecialCursor=CURSOR_INTERACTION_ON;

						if (bookclick.x!=-1)
						{
							Book_Page=8;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(GDevice,ITC.current_8,99.f,283.f);
			}

			if (bOnglet[9])
			{
				if (Book_Page!=9) 
				{
					float px=90.f;
					float py=308.f;
					DrawBookInterfaceItem(GDevice,ITC.accessible_9,px,py);

					if (MouseInBookRect(px,py,px+32,py+32))
					{
						BOOKINTERFACEITEMCOLOR=0xFF555555;
						SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
						SETALPHABLEND(GDevice,TRUE);
						DrawBookInterfaceItem(GDevice,ITC.accessible_9,px,py);
						SETALPHABLEND(GDevice,FALSE);
						BOOKINTERFACEITEMCOLOR=0xFFFFFFFF;
						SpecialCursor=CURSOR_INTERACTION_ON;

						if (bookclick.x!=-1)
						{
							Book_Page=9;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(GDevice,ITC.current_9,99.f,307.f);
			}

			if (bOnglet[10])
			{
				if (Book_Page!=10) 
				{
					float px=97.f;
					float py=331.f;
					DrawBookInterfaceItem(GDevice,ITC.accessible_10,px,py);

					if (MouseInBookRect(px,py,px+32,py+32))
					{
						BOOKINTERFACEITEMCOLOR=0xFF555555;
						SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
						SETALPHABLEND(GDevice,TRUE);
						DrawBookInterfaceItem(GDevice,ITC.accessible_10,px,py);
						SETALPHABLEND(GDevice,FALSE);
						BOOKINTERFACEITEMCOLOR=0xFFFFFFFF;
						SpecialCursor=CURSOR_INTERACTION_ON;

						if (bookclick.x!=-1)
						{
							Book_Page=10;
							ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						}
					}
				}
				else DrawBookInterfaceItem(GDevice,ITC.current_10,104.f,331.f);
			}		

			if (Book_Mode==1) Book_SpellPage=Book_Page;
			else if (Book_Mode==2) Book_MapPage=Book_Page;
		}

		bookclick.x=-1;
	}
	
	if (Book_Mode == 0)
	{
		FLYING_OVER=NULL;
		_TCHAR tex[64];
		COLORREF Color = RGB(0,0,0);
		
		ARX_PLAYER_ComputePlayerFullStats();

		danaeApp.DANAEEndRender();
		_stprintf(tex, _T("%s %3d"), ITC.lpszULevel, player.level);
		DrawBookTextCenter( 398, 74, tex,Color,0x00FF00FF,InBookFont);
		
		_stprintf(tex, _T("%s %8d"), ITC.lpszUXp, player.xp);
		DrawBookTextCenter( 510, 74, tex, Color,0x00FF00FF,InBookFont);
		danaeApp.DANAEStartRender();

		if (MouseInBookRect(463, 74, 550, 94))
		{
			FLYING_OVER = WND_XP;
		}

		if (MouseInBookRect(97+41,64+62, 97+41+32, 64+62+32))
		{
			FLYING_OVER = WND_AC;
		}
		else if (MouseInBookRect(97+41,64+120, 97+41+32, 64+120+32))
		{
			FLYING_OVER = WND_RESIST_MAGIC;
		}
		else if (MouseInBookRect(97+41,64+178, 97+41+32, 64+178+32))
		{
			FLYING_OVER = WND_RESIST_POISON;
		}
		else if (MouseInBookRect(97+211,64+62, 97+211+32, 64+62+32))
		{
			FLYING_OVER = WND_HP;
		}
		else if (MouseInBookRect(97+211,64+120, 97+211+32, 64+120+32))
		{
			FLYING_OVER = WND_MANA;
		}
		else if (MouseInBookRect(97+211,64+178, 97+211+32, 64+178+32))
		{
			FLYING_OVER = WND_DAMAGE;
		}
		
		if (!((player.Attribute_Redistribute == 0) && (ARXmenu.currentmode != AMCM_NEWQUEST)))
		{
			// Main Player Attributes
			if (CheckAttributeClick(379,95,&player.Attribute_Strength,		ITC.ic_strength))
			{
				FLYING_OVER=BOOK_STRENGTH;			
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Attribute_Redistribute;
			}

			if (CheckAttributeClick(428,95,&player.Attribute_Mind,			ITC.ic_mind))
			{
				FLYING_OVER=BOOK_MIND;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Attribute_Redistribute;
			}

			if (CheckAttributeClick(477,95,&player.Attribute_Dexterity,		ITC.ic_dexterity))
			{
				FLYING_OVER=BOOK_DEXTERITY;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Attribute_Redistribute;
			}

			if (CheckAttributeClick(526,95,&player.Attribute_Constitution,	ITC.ic_constitution))
			{
				FLYING_OVER=BOOK_CONSTITUTION;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Attribute_Redistribute;
			}
		}
		
		if (!((player.Skill_Redistribute == 0) && (ARXmenu.currentmode != AMCM_NEWQUEST)))
		{
			if (CheckSkillClick(389,177,&player.Skill_Stealth,		ITC.ic_stealth,&player.Old_Skill_Stealth))
			{
				FLYING_OVER=BOOK_STEALTH;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Skill_Redistribute;
			}

			if (CheckSkillClick(453,177,&player.Skill_Mecanism,		ITC.ic_mecanism,&player.Old_Skill_Mecanism))
			{
				FLYING_OVER=BOOK_MECANISM;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Skill_Redistribute;
			}

			if (CheckSkillClick(516,177,&player.Skill_Intuition,	ITC.ic_intuition,&player.Old_Skill_Intuition))
			{
				FLYING_OVER=BOOK_INTUITION;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Skill_Redistribute;
			}

			if (CheckSkillClick(389,230,&player.Skill_Etheral_Link,	ITC.ic_etheral_link,&player.Old_Skill_Etheral_Link))
			{
				FLYING_OVER=BOOK_ETHERAL_LINK;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Skill_Redistribute;
			}

			if (CheckSkillClick(453,230,&player.Skill_Object_Knowledge,ITC.ic_object_knowledge,&player.Old_Skill_Object_Knowledge))
			{
				FLYING_OVER=BOOK_OBJECT_KNOWLEDGE;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Skill_Redistribute;
				
				if ((BOOKBUTTON & 1) && !(LASTBOOKBUTTON & 1))
				{
					ARX_INVENTORY_IdentifyAll();
					ARX_EQUIPMENT_IdentifyAll();
				}

				ARX_PLAYER_ComputePlayerFullStats();
			}

			if (CheckSkillClick(516,230,&player.Skill_Casting,		ITC.ic_casting,&player.Old_Skill_Casting))
			{
				FLYING_OVER=BOOK_CASTING;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Skill_Redistribute;
			}

			if (CheckSkillClick(389,284,&player.Skill_Close_Combat,	ITC.ic_close_combat,&player.Old_Skill_Close_Combat))
			{
				FLYING_OVER=BOOK_CLOSE_COMBAT;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Skill_Redistribute;
			}

			if (CheckSkillClick(453,284,&player.Skill_Projectile,	ITC.ic_projectile,&player.Old_Skill_Projectile))
			{
				FLYING_OVER=BOOK_PROJECTILE;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Skill_Redistribute;
			}

			if (CheckSkillClick(516,284,&player.Skill_Defense,		ITC.ic_defense,&player.Old_Skill_Defense))
			{
				FLYING_OVER=BOOK_DEFENSE;
				SpecialCursor = CURSOR_REDIST;
				lCursorRedistValue = player.Skill_Redistribute;
			}
		}
		else
		{
			//------------------------------------PRIMARY
			if (MouseInBookRect(379,95, 379+32, 95+32))
			{
				FLYING_OVER=BOOK_STRENGTH;			
			}
			else if (MouseInBookRect(428,95, 428+32, 95+32))
			{
				FLYING_OVER=BOOK_MIND;
			}
			else if (MouseInBookRect(477,95, 477+32, 95+32))
			{
				FLYING_OVER=BOOK_DEXTERITY;
			}
			else if (MouseInBookRect(526,95, 526+32, 95+32))
			{
				FLYING_OVER=BOOK_CONSTITUTION;
			}
			
			//------------------------------------SECONDARY
			if (MouseInBookRect(389,177, 389+32, 177+32))
			{
				FLYING_OVER=BOOK_STEALTH;
			}
			else if (MouseInBookRect(453,177, 453+32, 177+32))
			{
				FLYING_OVER=BOOK_MECANISM;
			}
			else if (MouseInBookRect(516,177, 516+32, 177+32))
			{
				FLYING_OVER=BOOK_INTUITION;
			}
			else if (MouseInBookRect(389,230, 389+32, 230+32))
			{
				FLYING_OVER=BOOK_ETHERAL_LINK;
			}
			else if (MouseInBookRect(453,230, 453+32, 230+32))
			{
				FLYING_OVER=BOOK_OBJECT_KNOWLEDGE;
			}
			else if (MouseInBookRect(516,230, 516+32, 230+32))
			{
				FLYING_OVER=BOOK_CASTING;
			}
			else if (MouseInBookRect(389,284, 389+32, 284+32))
			{
				FLYING_OVER=BOOK_CLOSE_COMBAT;
			}
			else if (MouseInBookRect(453,284, 453+32, 284+32))
			{
				FLYING_OVER=BOOK_PROJECTILE;
			}
			else if (MouseInBookRect(516,284, 516+32, 284+32))
			{
				FLYING_OVER=BOOK_DEFENSE;
			}
		}
		
		if (!INTERNATIONAL_MODE)
		{
			for (long i=0;i<MAX_SPEECH;i++)
			{
				if (speech[i].timecreation>0) FLYING_OVER=NULL;
			}
		}
		
		//------------------------------ SEB 04/12/2001
		if (ARXmenu.mda && ARXmenu.mda->flyover[FLYING_OVER]) //=ARXmenu.mda->flyover[FLYING_OVER];
		{
			if( (FLYING_OVER!=OLD_FLYING_OVER)||
				(INTERNATIONAL_MODE) )
			{

				float fRandom	= rnd() * 2 ;
				ARX_CHECK_INT(fRandom);

				int t = ARX_CLEAN_WARN_CAST_INT(fRandom);


				pTextManage->Clear();
				OLD_FLYING_OVER=FLYING_OVER;

				if (FLYING_OVER == WND_XP)
				{
					_TCHAR tex[512];
					_stprintf(tex, _T("%s %8d"), ARXmenu.mda->flyover[WND_XP], GetXPforLevel(player.level+1)-player.xp);
					UNICODE_ARXDrawTextCenteredScroll(	(DANAESIZX*0.5f),
						4,
						(DANAECENTERX)*0.82f,
						tex,
						RGB(232+t,204+t,143+t),
						0x00FF00FF,
						hFontInGame,
						1000,
						0.01f,
						3,
						INTERNATIONAL_MODE?0:max(3000, 70*_tcslen(tex)));
				}
				else
				{
					UNICODE_ARXDrawTextCenteredScroll(	(DANAESIZX*0.5f),
						4,
						(DANAECENTERX)*0.82f,
						ARXmenu.mda->flyover[FLYING_OVER],
						RGB(232+t,204+t,143+t),
						0x00FF00FF,
						hFontInGame,
						1000,
						0.01f,
						3,
						INTERNATIONAL_MODE?0:max(3000, 70*_tcslen(ARXmenu.mda->flyover[FLYING_OVER])));
				}
			}
		}
		else
		{
			OLD_FLYING_OVER=-1;
		}

		//------------------------------
		
		danaeApp.DANAEEndRender();
		_stprintf(tex, _T("%3.0f"),player.Full_Attribute_Strength);

		if (player.Mod_Attribute_Strength<0.f)
			Color = 0x000000FF;		
		else if (player.Mod_Attribute_Strength>0.f)
			Color = 0x00FF0000;		
		else Color = 0;

		if (ARXmenu.currentmode==AMCM_NEWQUEST)
		{
			if (player.Full_Attribute_Strength == 6)
				Color = 0x000000FF;
		}

		DrawBookTextCenter( 391, 129, tex, Color, 0x00FF00FF, InBookFont);
		
		_stprintf(tex, _T("%3.0f"),player.Full_Attribute_Mind);

		if (player.Mod_Attribute_Mind<0.f)
			Color = 0x000000FF;		
		else if (player.Mod_Attribute_Mind>0.f)
			Color = 0x00FF0000;		
		else Color = 0;

		if (ARXmenu.currentmode==AMCM_NEWQUEST)
		{
			if (player.Full_Attribute_Mind == 6)
				Color = 0x000000FF;
		}

		DrawBookTextCenter( 440, 129, tex, Color, 0x00FF00FF, InBookFont);
		
		_stprintf(tex, _T("%3.0f"),player.Full_Attribute_Dexterity);

		if (player.Mod_Attribute_Dexterity<0.f)
			Color = 0x000000FF;		
		else if (player.Mod_Attribute_Dexterity>0.f)
			Color = 0x00FF0000;		
		else Color = 0;

		if (ARXmenu.currentmode==AMCM_NEWQUEST)
		{
			if (player.Full_Attribute_Dexterity == 6)
				Color = 0x000000FF;
		}

		DrawBookTextCenter( 490, 129, tex, Color, 0x00FF00FF, InBookFont);
		
		_stprintf(tex, _T("%3.0f"),player.Full_Attribute_Constitution);

		if (player.Mod_Attribute_Constitution<0.f)
			Color = 0x000000FF;		
		else if (player.Mod_Attribute_Constitution>0.f)
			Color = 0x00FF0000;		
		else Color = 0;

		if (ARXmenu.currentmode==AMCM_NEWQUEST)
		{
			if (player.Full_Attribute_Constitution == 6)
				Color = 0x000000FF;
		}

		DrawBookTextCenter( 538, 129, tex, Color, 0x00FF00FF,InBookFont);
		
		// Player Skills
		_stprintf(tex, _T("%3.0f"),player.Full_Skill_Stealth);

		if (player.Mod_Skill_Stealth<0.f)
			Color = 0x000000FF;		
		else if (player.Mod_Skill_Stealth>0.f)
			Color = 0x00FF0000;
		else Color = 0;

		if (ARXmenu.currentmode==AMCM_NEWQUEST)
		{
			if (player.Skill_Stealth == 0)
				Color = 0x000000FF;
		}

		DrawBookTextCenter( 405, 210, tex, Color, 0x00FF00FF, InBookFont);
		
		_stprintf(tex, _T("%3.0f"),player.Full_Skill_Mecanism);

		if (player.Mod_Skill_Mecanism<0.f)
			Color = 0x000000FF;		
		else if (player.Mod_Skill_Mecanism>0.f)
			Color = 0x00FF0000;		
		else Color = 0;

		if (ARXmenu.currentmode==AMCM_NEWQUEST)
		{
			if (player.Skill_Mecanism == 0)
				Color = 0x000000FF;
		}

		DrawBookTextCenter( 469, 210, tex, Color, 0x00FF00FF, InBookFont);
		
		_stprintf(tex, _T("%3.0f"),player.Full_Skill_Intuition);

		if (player.Mod_Skill_Intuition<0.f)
			Color = 0x000000FF;		
		else if (player.Mod_Skill_Intuition>0.f)
			Color = 0x00FF0000;		
		else Color = 0;

		if (ARXmenu.currentmode==AMCM_NEWQUEST)
		{
			if (player.Skill_Intuition == 0)
				Color = 0x000000FF;
		}

		DrawBookTextCenter( 533, 210, tex, Color, 0x00FF00FF,InBookFont);
		
		_stprintf(tex, _T("%3.0f"),player.Full_Skill_Etheral_Link);

		if (player.Mod_Skill_Etheral_Link<0.f)
			Color = 0x000000FF;		
		else if (player.Mod_Skill_Etheral_Link>0.f)
			Color = 0x00FF0000;		
		else Color = 0;

		if (ARXmenu.currentmode==AMCM_NEWQUEST)
		{
			if (player.Skill_Etheral_Link == 0)
				Color = 0x000000FF;
		}

		DrawBookTextCenter( 405, 265, tex, Color, 0x00FF00FF, InBookFont);
		
		_stprintf(tex, _T("%3.0f"),player.Full_Skill_Object_Knowledge);

		if (player.Mod_Skill_Object_Knowledge<0.f)
			Color = 0x000000FF;		
		else if (player.Mod_Skill_Object_Knowledge>0.f)
			Color = 0x00FF0000;		
		else Color = 0;

		if (ARXmenu.currentmode==AMCM_NEWQUEST)
		{
			if (player.Skill_Object_Knowledge == 0)
				Color = 0x000000FF;
		}

		DrawBookTextCenter( 469, 265, tex, Color, 0x00FF00FF, InBookFont);
		
		_stprintf(tex, _T("%3.0f"),player.Full_Skill_Casting);

		if (player.Mod_Skill_Casting<0.f)
			Color = 0x000000FF;		
		else if (player.Mod_Skill_Casting>0.f)
			Color = 0x00FF0000;		
		else Color = 0;

		if (ARXmenu.currentmode==AMCM_NEWQUEST)
		{
			if (player.Skill_Casting == 0)
				Color = 0x000000FF;
		}

		DrawBookTextCenter( 533, 265, tex, Color, 0x00FF00FF, InBookFont);
		
		_stprintf(tex, _T("%3.0f"),player.Full_Skill_Close_Combat);

		if (player.Mod_Skill_Close_Combat<0.f)
			Color = 0x000000FF;		
		else if (player.Mod_Skill_Close_Combat>0.f)
			Color = 0x00FF0000;		
		else Color = 0;

		if (ARXmenu.currentmode==AMCM_NEWQUEST)
		{
			if (player.Skill_Close_Combat == 0)
				Color = 0x000000FF;
		}

		DrawBookTextCenter( 405, 319, tex, Color, 0x00FF00FF, InBookFont);
		
		_stprintf(tex, _T("%3.0f"),player.Full_Skill_Projectile);

		if (player.Mod_Skill_Projectile<0.f)
			Color = 0x000000FF;		
		else if (player.Mod_Skill_Projectile>0.f)
			Color = 0x00FF0000;		
		else Color = 0;

		if (ARXmenu.currentmode==AMCM_NEWQUEST)
		{
			if (player.Skill_Projectile == 0)
				Color = 0x000000FF;
		}

		DrawBookTextCenter( 469, 319, tex, Color, 0x00FF00FF, InBookFont);
		
		_stprintf(tex, _T("%3.0f"),player.Full_Skill_Defense);

		if (player.Mod_Skill_Defense<0.f)
			Color = 0x000000FF;		
		else if (player.Mod_Skill_Defense>0.f)
			Color = 0x00FF0000;		
		else Color = 0;

		if (ARXmenu.currentmode==AMCM_NEWQUEST)
		{
			if (player.Skill_Defense == 0)
				Color = 0x000000FF;
		}

		DrawBookTextCenter( 533, 319, tex, Color, 0x00FF00FF, InBookFont);
		
		// Secondary Attributes
		_stprintf(tex, _T("%d"),F2L_RoundUp(player.Full_maxlife));

		if ((player.Mod_maxlife<0.f) || (player.Full_maxlife < player.maxlife))
			Color = 0x000000FF;		
		else if ((player.Mod_maxlife>0.f) || (player.Full_maxlife > player.maxlife))
			Color = 0x00FF0000;		
		else Color = 0;

		DrawBookTextCenter( 324, 158, tex, Color,0x00FF00FF, InBookFont);
		
		_stprintf(tex, _T("%d"),F2L_RoundUp(player.Full_maxmana));

		if ((player.Mod_maxmana<0.f) || (player.Full_maxmana < player.maxmana))
			Color = 0x000000FF;		
		else if ((player.Mod_maxmana>0.f) || (player.Full_maxmana > player.maxmana))
			Color = 0x00FF0000;		
		else Color = 0;

		DrawBookTextCenter( 324, 218, tex, Color, 0x00FF00FF, InBookFont);
		
		_stprintf(tex, _T("%d"), F2L_RoundUp(player.Full_damages));

		if (player.Mod_damages<0.f)
			Color = 0x000000FF;		
		else if (player.Mod_damages>0.f)
			Color = 0x00FF0000;		
		else Color = 0;

		DrawBookTextCenter( 324, 278, tex, Color, 0x00FF00FF, InBookFont);
		
		float ac = player.Full_armor_class;
		_stprintf(tex, _T("%d"),F2L_RoundUp(ac));

		if (player.Mod_armor_class<0.f)
			Color = 0x000000FF;		
		else if (player.Mod_armor_class>0.f)
			Color = 0x00FF0000;		
		else Color = 0;

		DrawBookTextCenter( 153, 158, tex, Color, 0x00FF00FF, InBookFont);
		
		_stprintf(tex, _T("%3.0f"),player.Full_resist_magic);

		if (player.Mod_resist_magic<0.f)
			Color = 0x000000FF;		
		else if (player.Mod_resist_magic>0.f)
			Color = 0x00FF0000;		
		else Color = 0;

		DrawBookTextCenter( 153, 218, tex, Color, 0x00FF00FF, InBookFont);
		
		_stprintf(tex, _T("%3.0f"),player.Full_resist_poison);

		if (player.Mod_resist_poison<0.f)
			Color = 0x000000FF;		
		else if (player.Mod_resist_poison>0.f)
			Color = 0x00FF0000;		
		else Color = 0;

		DrawBookTextCenter( 153, 278, tex, Color, 0x00FF00FF, InBookFont);
		danaeApp.DANAEStartRender();
	}
	else if (Book_Mode==2)
	{
		long SHOWLEVEL = Book_MapPage - 1;

		if ((SHOWLEVEL>=0) && (SHOWLEVEL<32))
			ARX_MINIMAP_Show(GDevice,SHOWLEVEL,0);

		SHOWLEVEL = ARX_LEVELS_GetRealNum(CURRENTLEVEL);

		if ((SHOWLEVEL>=0) && (SHOWLEVEL<32))
			ARX_MINIMAP_Show(GDevice,SHOWLEVEL,1);
	}
	else if (Book_Mode==3)
	{
		if (nb_PlayerQuest > 0)
		{
			//-----------------------------------------------------------------
			// stuff pour le texte
			NotePosX = (97 );
			NotePosY = (64 );
			NoteTextMinx = 40.f;
			NoteTextMaxx = ITC.questbook->m_dwWidth*DIV2-10.f;
			NoteTextMiny = 40.f;
			NoteTextMaxy = ITC.questbook->m_dwHeight-65.f;
			float fPosX = 97;
			float fPosY = 64;
			float x0 = 0;
			float y0 = 0;
			
			// calcul du nombre de pages
			RECT rRect;
			long lCurPage = 1;


			float fMinX = (NoteTextMaxx-NoteTextMinx)*Xratio ;
			float fMinY = (NoteTextMaxy-NoteTextMiny)*Yratio ;
			ARX_CHECK_INT(fMinX);
			ARX_CHECK_INT(fMinY);
			
			SetRect(&rRect,
				0,
				0,
				ARX_CLEAN_WARN_CAST_INT(fMinX),
				ARX_CLEAN_WARN_CAST_INT(fMinY)
				);


			int lLenghtCurr=0;
			long lLenght = 0;
			_TCHAR *lpszQuests = NULL;

			QuestBook.pages[0]=0;
			
			for (long i=0; i<nb_PlayerQuest; i++)
			{
				if (PlayerQuest[i].localised != NULL)
				{
					lLenght += _tcslen(PlayerQuest[i].localised);
				}
			}
			
			lpszQuests = new _TCHAR[lLenght+nb_PlayerQuest*2+1];
			ZeroMemory(lpszQuests, (lLenght+nb_PlayerQuest*2+1)*sizeof(_TCHAR));
			
			for (int i=0; i<nb_PlayerQuest; i++)
			{
				if (PlayerQuest[i].localised != NULL)
				{
					_tcscat(lpszQuests, PlayerQuest[i].localised);
					_tcscat(lpszQuests, _T("\n\n"));
					lLenght+=2;
				}
			}
			
			while(lLenght>0)
			{
				danaeApp.DANAEEndRender();
				long lLengthDraw=ARX_UNICODE_ForceFormattingInRect(	hFontInGameNote,
					lpszQuests + lLenghtCurr,
					0,
					rRect);
				danaeApp.DANAEStartRender();
				lLenght -= lLengthDraw;
				lLenghtCurr += lLengthDraw;

				if (lCurPage + 1 < MAX_PAGES)
					QuestBook.pages[lCurPage++] = lLenghtCurr;
			}

			if (lCurPage + 1 < MAX_PAGES)
				QuestBook.pages[lCurPage++] = -1;
			else
				QuestBook.pages[MAX_PAGES-1] = -1;

			QuestBook.totpages = lCurPage;
			
			//---------------------------------------------------------------------
			// render
			if (QuestBook.curpage >= QuestBook.totpages)
				QuestBook.curpage = QuestBook.totpages-1;

			if (QuestBook.curpage < 0)
				QuestBook.curpage = 0;

			if (QuestBook.curpage>1)
			{
				x0 =   8 + fPosX;
				y0 =  -6 + fPosY + ITC.questbook->m_dwHeight - ITC.pTexCornerLeft->m_dwHeight;
				DrawBookInterfaceItem(GDevice, ITC.pTexCornerLeft,  x0, y0);

				if (MouseInBookRect(x0,y0,x0+ITC.pTexCornerLeft->m_dwWidth,y0+ITC.pTexCornerLeft->m_dwHeight))
				{
					SpecialCursor=CURSOR_INTERACTION_ON;

					if (!(EERIEMouseButton & 1) && (LastMouseClick & 1))
					{
						ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						QuestBook.curpage -= 2;
					}
				}
			}

			if ((QuestBook.curpage + 4) < QuestBook.totpages)
			{
				x0 = -15 + fPosX + ITC.questbook->m_dwWidth  - ITC.pTexCornerRight->m_dwWidth;
				y0 =  -6 + fPosY + ITC.questbook->m_dwHeight - ITC.pTexCornerRight->m_dwHeight;
				DrawBookInterfaceItem(GDevice, ITC.pTexCornerRight, x0, y0);
				
				if (MouseInBookRect(x0,y0,x0+ITC.pTexCornerRight->m_dwWidth,y0+ITC.pTexCornerRight->m_dwHeight))
				{
					SpecialCursor=CURSOR_INTERACTION_ON;

					if (!(EERIEMouseButton & 1) && (LastMouseClick & 1))
					{
						ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, 0.9F + 0.2F * rnd());
						QuestBook.curpage+=2;
					}
				}
			}

			if (QuestBook.pages[QuestBook.curpage]>=0)
			{
				if(QuestBook.pages[QuestBook.curpage+1]>0)
				{
					_tcsncpy(Page_Buffer, lpszQuests + QuestBook.pages[QuestBook.curpage], QuestBook.pages[QuestBook.curpage+1] - QuestBook.pages[QuestBook.curpage]);
					Page_Buffer[QuestBook.pages[QuestBook.curpage+1] - QuestBook.pages[QuestBook.curpage]]=_T('\0');
					DrawBookTextInRect( NotePosX + NoteTextMinx, NotePosY + NoteTextMiny, NotePosX + NoteTextMaxx, NotePosY + NoteTextMaxy, Page_Buffer, 0, 0x00FF00FF, hFontInGameNote);
					
					if(QuestBook.pages[QuestBook.curpage+2]>0)
					{
						_tcsncpy(Page_Buffer, lpszQuests + QuestBook.pages[QuestBook.curpage+1], QuestBook.pages[QuestBook.curpage+2] - QuestBook.pages[QuestBook.curpage+1]);
						Page_Buffer[QuestBook.pages[QuestBook.curpage+2] - QuestBook.pages[QuestBook.curpage+1]]=_T('\0');
						DrawBookTextInRect( NotePosX + NoteTextMinx + (NoteTextMaxx - NoteTextMinx) +20, NotePosY + NoteTextMiny, NotePosX + NoteTextMaxx + (NoteTextMaxx - NoteTextMinx) +20, NotePosY + NoteTextMaxy, Page_Buffer, 0, 0x00FF00FF, hFontInGameNote);
					}
				}
				else
				{
					if(QuestBook.pages[QuestBook.curpage]>=0)
					{
						_tcscpy(Page_Buffer, lpszQuests + QuestBook.pages[QuestBook.curpage]);
						DrawBookTextInRect( NotePosX + NoteTextMinx, NotePosY + NoteTextMiny, NotePosX+NoteTextMaxx, NotePosY + NoteTextMaxy, Page_Buffer, 0, 0x00FF00FF, hFontInGameNote);
					}
				}
			}

			delete []lpszQuests;
		}
	}
	
	if ((Book_Mode==0) && (inter.iobj[0]->obj!=NULL))
	{

		SETZWRITE(GDevice,true);
		danaeApp.EnableZBuffer();
		SetFilteringMode(GDevice,1);
		D3DRECT rec;

		if (BOOKZOOM)
		{
 
			F2L((float)((118.F+BOOKDECX)*Xratio),&rec.x1);
			F2L((float)((69.f +BOOKDECY)*Yratio),&rec.y1);
			F2L((float)((280.f + BOOKDECY)*Xratio), &rec.x2); 
			F2L((float)((310.f+BOOKDECY)*Yratio),&rec.y2);

			GDevice->Clear( 1, &rec, D3DCLEAR_ZBUFFER, 0, 1.f, 0L );

			if (ARXmenu.currentmode!=AMCM_OFF)
				danaeApp.SetClipping(139.f*Xratio,0,139.f*Xratio,310.f*Yratio);
		}
		else
		{
			F2L((float)((118.F+BOOKDECX)*Xratio),&rec.x1);
			F2L((float)((69.f +BOOKDECY)*Yratio),&rec.y1);
		
			F2L((float)((300.f+50+BOOKDECX)*Xratio),&rec.x2);
			F2L((float)((338.f+BOOKDECY)*Yratio),&rec.y2);
			
			GDevice->Clear( 1, &rec, D3DCLEAR_ZBUFFER, 0, 1.f, 0L );
			rec.x2 -= 50;
		}
		
		if (ARXmenu.currentmode==AMCM_OFF) 
			BOOKZOOM=0;
		
		EERIE_3D pos;
		EERIE_LIGHT eLight1;
		EERIE_LIGHT eLight2;
		eLight1.pos.x=50.f;
		eLight1.pos.y=50.f;
		eLight1.pos.z=200.f;
	
		eLight1.exist=1;
		eLight1.rgb.r = 0.15f; 
		eLight1.rgb.g = 0.06f; 
		eLight1.rgb.b = 0.003f; 
		eLight1.intensity = 8.8f;
		eLight1.fallstart = 2020;
		eLight1.fallend=eLight1.fallstart+60;
		eLight1.rgb255.r=eLight1.rgb.r*255.f;
		eLight1.rgb255.g=eLight1.rgb.g*255.f;
		eLight1.rgb255.b=eLight1.rgb.b*255.f;
		eLight1.falldiff=eLight1.fallend-eLight1.fallstart;
		eLight1.falldiffmul=1.f/eLight1.falldiff;
		eLight1.precalc=eLight1.intensity*GLOBAL_LIGHT_FACTOR;
		
		eLight2.exist=1;
		eLight2.pos.x=-50.f;
		eLight2.pos.y=-50.f;
		eLight2.pos.z = -200.f; 
		eLight2.rgb.r=0.6f;
		eLight2.rgb.g=0.6f;
		eLight2.rgb.b=0.6f;
		eLight2.intensity=3.8f;
		eLight2.fallstart = 0; 
		eLight2.fallend=eLight2.fallstart+3460.f;
		eLight2.rgb255.r=eLight2.rgb.r*255.f;
		eLight2.rgb255.g=eLight2.rgb.g*255.f;
		eLight2.rgb255.b=eLight2.rgb.b*255.f;
		eLight2.falldiff=eLight2.fallend-eLight2.fallstart;
		eLight2.falldiffmul=1.f/eLight2.falldiff;
		eLight2.precalc=eLight2.intensity*GLOBAL_LIGHT_FACTOR;
		
		EERIE_LIGHT * SavePDL[2];
		SavePDL[0]=PDL[0];
		SavePDL[1]=PDL[1];
		int			iSavePDL=TOTPDL;

		PDL[0] = &eLight1;
		PDL[1] = &eLight2;
		TOTPDL = 2;
		
		EERIE_CAMERA * oldcam=ACTIVECAM;
		bookcam.centerx=(rec.x2-rec.x1)/2+rec.x1;
		bookcam.centery=(rec.y2-rec.y1)/2+rec.y1;
		SetActiveCamera(&bookcam);
		PrepareCamera(&bookcam);
		
		D3DVIEWPORT7 vp;

		if (BOOKZOOM)
		{

			vp.dwX		=	ARX_CLEAN_WARN_CAST_DWORD( rec.x1 + 52.f * Xratio );
			vp.dwY		=	rec.y1;
			vp.dwWidth	=	ARX_CLEAN_WARN_CAST_DWORD( rec.x2 - rec.x1 - 73.f * Xratio );
			vp.dwHeight	=	ARX_CLEAN_WARN_CAST_DWORD( rec.y2 - rec.y1 - 17.f * Yratio );

		}
		else
		{
			vp.dwX		=	rec.x1;
			vp.dwY		=	rec.y1;
			vp.dwWidth	=	rec.x2 - rec.x1;
			vp.dwHeight	=	rec.y2 - rec.y1;
		}

		vp.dvMinZ	=	0.f;
		vp.dvMaxZ	=	1.f;
		GDevice->SetViewport(&vp);
		
		
		ePlayerAngle.a=0.f;
		ePlayerAngle.g=0.f;

		if (BOOKZOOM)
		{
			pos.x=8;
			pos.y=162.f;
			pos.z=75.f;
			eLight1.pos.z=-90.f;
			
		}
		else
		{
			ePlayerAngle.b=-20.f;
			pos.x=20.f;
			pos.y=96.f;
			pos.z=260.f;
		}
		
		if (!BOOKZOOM)
			ARX_EQUIPMENT_AttachPlayerWeaponToHand();
		
		long ti=Project.improve;
		Project.improve=0;

		if (inter.iobj[0]->invisibility>0.f) 
		{
			SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
			SETALPHABLEND(GDevice,TRUE);
		}

		INVISIBILITY_OVERRIDE=inter.iobj[0]->invisibility;

		if (INVISIBILITY_OVERRIDE>0.5f)
			INVISIBILITY_OVERRIDE=0.5f;

		FORCE_NO_HIDE=1;
		IN_BOOK_DRAW=1;
		EERIE_VERTEX *	vertexlist=(EERIE_VERTEX *)malloc(sizeof(EERIE_VERTEX)*inter.iobj[0]->obj->nbvertex);
		memcpy(vertexlist,inter.iobj[0]->obj->vertexlist3,sizeof(EERIE_VERTEX)*inter.iobj[0]->obj->nbvertex);

		if (player.useanim.cur_anim != NULL) 
		{

			ARX_CHECK_ULONG(Original_framedelay);
			EERIEDrawAnimQuat(GDevice,inter.iobj[0]->obj, &player.useanim,&ePlayerAngle,&pos,
				ARX_CLEAN_WARN_CAST_ULONG(Original_framedelay),
				NULL,D3DCOLORWHITE,NULL);

		}
		else
		{
			DrawEERIEInter(GDevice,inter.iobj[0]->obj,&ePlayerAngle,&pos,NULL);
		}

		INVISIBILITY_OVERRIDE=0;
		IN_BOOK_DRAW=0;

		if(ARXmenu.currentmode==AMCM_NEWQUEST)
		{
			if(bRenderInterList)
			{
				GDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTFP_NONE);
				SETALPHABLEND(GDevice,FALSE);
				PopAllTriangleList(true);
				SETALPHABLEND(GDevice,TRUE);
				PopAllTriangleListTransparency();
				SETALPHABLEND(GDevice,FALSE);
				GDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTFP_POINT);
			}
		}

		PDL[0]=SavePDL[0];
		PDL[1]=SavePDL[1];
		TOTPDL=iSavePDL;

		memcpy(inter.iobj[0]->obj->vertexlist3,vertexlist,sizeof(EERIE_VERTEX)*inter.iobj[0]->obj->nbvertex);
		free(vertexlist);
		FORCE_NO_HIDE=0;
		Project.improve=ti;
		

		vp.dwX		=	0;
		vp.dwY		=	0;

		vp.dwWidth	=	DANAESIZX;
		vp.dwHeight	=	DANAESIZY;
		vp.dvMinZ	=	0.f;
		vp.dvMaxZ	=	1.f;
		GDevice->SetViewport(&vp);

		if (ARXmenu.currentmode!=AMCM_OFF)
		{
			danaeApp.SetClipping(0,0,(float)DANAESIZX,(float)DANAESIZY);
		}

		SETALPHABLEND(GDevice,FALSE);
		SETCULL(GDevice,D3DCULL_NONE);
		SetActiveCamera(oldcam);		
		
		INTERACTIVE_OBJ * io=inter.iobj[0];

		if (io)
		{
			player.useanim.cur_anim = herowaitbook;

			if (	(player.equiped[EQUIP_SLOT_WEAPON]!=0)
				&&	ValidIONum(player.equiped[EQUIP_SLOT_WEAPON]	))
			{
				if (inter.iobj[player.equiped[EQUIP_SLOT_WEAPON]]->type_flags & OBJECT_TYPE_2H)
				{
					player.useanim.cur_anim = herowait_2h;
				}
			}

			SETCULL(GDevice,D3DCULL_NONE);

			if (	(player.equiped[EQUIP_SLOT_ARMOR]!=0)
				&&	ValidIONum(player.equiped[EQUIP_SLOT_ARMOR]	))
			{
				INTERACTIVE_OBJ * tod=inter.iobj[player.equiped[EQUIP_SLOT_ARMOR]];

				if (tod)
				{


					tod->bbox1.x = 195;
					tod->bbox1.y = 116;
					tod->bbox2.x = 284;
					tod->bbox2.y = 182; 

					float fX1 = (tod->bbox1.x+BOOKDECX) * Xratio ;
					float fX2 = (tod->bbox2.x+BOOKDECX) * Xratio ;
					float fY1 = (tod->bbox1.y+BOOKDECY) * Yratio ;
					float fY2 = (tod->bbox2.y+BOOKDECY) * Yratio ;

					ARX_CHECK_SHORT(fX1);
					ARX_CHECK_SHORT(fY1);
					ARX_CHECK_SHORT(fX2);
					ARX_CHECK_SHORT(fY2);
					
					tod->bbox1.x = ARX_CLEAN_WARN_CAST_SHORT(fX1);
					tod->bbox2.x = ARX_CLEAN_WARN_CAST_SHORT(fX2);
					tod->bbox1.y = ARX_CLEAN_WARN_CAST_SHORT(fY1);
					tod->bbox2.y = ARX_CLEAN_WARN_CAST_SHORT(fY2);


					tod->ioflags|=IO_ICONIC;
				}
			}

			if (	(player.equiped[EQUIP_SLOT_LEGGINGS]!=0)
				&&	ValidIONum(player.equiped[EQUIP_SLOT_LEGGINGS]	))
			{
				INTERACTIVE_OBJ * tod=inter.iobj[player.equiped[EQUIP_SLOT_LEGGINGS]];

				if (tod)
				{


					tod->bbox1.x = 218;
					tod->bbox1.y = 183;
					tod->bbox2.x = 277;
					tod->bbox2.y = 322; 

					float fX1 = (tod->bbox1.x+BOOKDECX) * Xratio ;
					float fX2 = (tod->bbox2.x+BOOKDECX) * Xratio ;
					float fY1 = (tod->bbox1.y+BOOKDECY) * Yratio ;
					float fY2 = (tod->bbox2.y+BOOKDECY) * Yratio ;

					ARX_CHECK_SHORT(fX1);
					ARX_CHECK_SHORT(fY1);
					ARX_CHECK_SHORT(fX2);
					ARX_CHECK_SHORT(fY2);

					tod->bbox1.x = ARX_CLEAN_WARN_CAST_SHORT(fX1);
					tod->bbox2.x = ARX_CLEAN_WARN_CAST_SHORT(fX2);
					tod->bbox1.y = ARX_CLEAN_WARN_CAST_SHORT(fY1);
					tod->bbox2.y = ARX_CLEAN_WARN_CAST_SHORT(fY2);


					tod->ioflags|=IO_ICONIC;
				}
			}

			if (	(player.equiped[EQUIP_SLOT_HELMET]!=0)
				&&	ValidIONum(player.equiped[EQUIP_SLOT_HELMET]	))
			{
				INTERACTIVE_OBJ * tod=inter.iobj[player.equiped[EQUIP_SLOT_HELMET]];

				if (tod)
				{


					tod->bbox1.x = 218;
					tod->bbox1.y = 75;
					tod->bbox2.x = 260;
					tod->bbox2.y = 115; 

					float fX1 = (tod->bbox1.x+BOOKDECX) * Xratio ;
					float fX2 = (tod->bbox2.x+BOOKDECX) * Xratio ;
					float fY1 = (tod->bbox1.y+BOOKDECY) * Yratio ;
					float fY2 = (tod->bbox2.y+BOOKDECY) * Yratio ;

					ARX_CHECK_SHORT(fX1);
					ARX_CHECK_SHORT(fY1);
					ARX_CHECK_SHORT(fX2);
					ARX_CHECK_SHORT(fY2);

					tod->bbox1.x = ARX_CLEAN_WARN_CAST_SHORT(fX1);
					tod->bbox2.x = ARX_CLEAN_WARN_CAST_SHORT(fX2);
					tod->bbox1.y = ARX_CLEAN_WARN_CAST_SHORT(fY1);
					tod->bbox2.y = ARX_CLEAN_WARN_CAST_SHORT(fY2);


					tod->ioflags|=IO_ICONIC;
				}
			}
			
			TextureContainer * tc;
			TextureContainer * tc2=NULL;
			SETCULL(GDevice,D3DCULL_NONE);

			if (	(player.equiped[EQUIP_SLOT_RING_LEFT]!=0)
				&&	ValidIONum(player.equiped[EQUIP_SLOT_RING_LEFT]	))
			{
				INTERACTIVE_OBJ * todraw=inter.iobj[player.equiped[EQUIP_SLOT_RING_LEFT]];
				
				tc=todraw->inv;

				if (NeedHalo(todraw)) tc2 = todraw->inv->TextureHalo;

				if (tc)
				{


					todraw->bbox1.x=146; 
					todraw->bbox1.y=312;

				
					if ((todraw->poisonous) && (todraw->poisonous_count!=0))
						BOOKINTERFACEITEMCOLOR=0xFF00FF00;
					else BOOKINTERFACEITEMCOLOR=D3DCOLORWHITE;

					DrawBookInterfaceItem(GDevice,tc,todraw->bbox1.x,todraw->bbox1.y,0);

					BOOKINTERFACEITEMCOLOR=D3DCOLORWHITE;

					if (tc2!=NULL)
					{
						ARX_INTERFACE_HALO_Draw(todraw,GDevice,tc,tc2,todraw->bbox1.x*Xratio,todraw->bbox1.y*Yratio, Xratio, Yratio);
					}



					float fWidth  = todraw->bbox1.x + ARX_CLEAN_WARN_CAST_FLOAT( tc->m_dwWidth );
					float fHeight = todraw->bbox1.y + ARX_CLEAN_WARN_CAST_FLOAT( tc->m_dwHeight );

					ARX_CHECK_SHORT(fWidth);
					ARX_CHECK_SHORT(fHeight);

					todraw->bbox2.x=ARX_CLEAN_WARN_CAST_SHORT(fWidth);
					todraw->bbox2.y=ARX_CLEAN_WARN_CAST_SHORT(fHeight);




					float fX1 = (todraw->bbox1.x+BOOKDECX) * Xratio ;
					float fX2 = (todraw->bbox2.x+BOOKDECX) * Xratio ;
					float fY1 = (todraw->bbox1.y+BOOKDECY) * Yratio ;
					float fY2 = (todraw->bbox2.y+BOOKDECY) * Yratio ;

					ARX_CHECK_SHORT(fX1);
					ARX_CHECK_SHORT(fY1);
					ARX_CHECK_SHORT(fX2);
					ARX_CHECK_SHORT(fY2);

					todraw->bbox1.x = ARX_CLEAN_WARN_CAST_SHORT(fX1);
					todraw->bbox2.x = ARX_CLEAN_WARN_CAST_SHORT(fX2);
					todraw->bbox1.y = ARX_CLEAN_WARN_CAST_SHORT(fY1);
					todraw->bbox2.y = ARX_CLEAN_WARN_CAST_SHORT(fY2);


					todraw->ioflags|=IO_ICONIC;
				}
			}

			tc2=NULL;

			if (	(player.equiped[EQUIP_SLOT_RING_RIGHT]!=0)
				&&	ValidIONum(player.equiped[EQUIP_SLOT_RING_RIGHT]	))
			{
				INTERACTIVE_OBJ * todraw=inter.iobj[player.equiped[EQUIP_SLOT_RING_RIGHT]];

				tc=todraw->inv;

				if (NeedHalo(todraw)) tc2=todraw->inv->TextureHalo;

				if (tc)
				{

					todraw->bbox1.x=296; 
					todraw->bbox1.y=312;

					
					if ((todraw->poisonous) && (todraw->poisonous_count!=0))
						BOOKINTERFACEITEMCOLOR=0xFF00FF00;
					else BOOKINTERFACEITEMCOLOR=D3DCOLORWHITE;

					DrawBookInterfaceItem(GDevice,tc,todraw->bbox1.x,todraw->bbox1.y,0);
					
					BOOKINTERFACEITEMCOLOR=D3DCOLORWHITE;

					if (tc2!=NULL) 
					{
						ARX_INTERFACE_HALO_Draw(todraw,GDevice,tc,tc2,todraw->bbox1.x*Xratio,todraw->bbox1.y*Yratio, Xratio, Yratio);
					}
					


					
					float fWidth  = todraw->bbox1.x + ARX_CLEAN_WARN_CAST_FLOAT( tc->m_dwWidth );
					float fHeight = todraw->bbox1.y + ARX_CLEAN_WARN_CAST_FLOAT( tc->m_dwHeight );


					ARX_CHECK_SHORT(fWidth);
					ARX_CHECK_SHORT(fHeight);

					todraw->bbox2.x = ARX_CLEAN_WARN_CAST_SHORT(fWidth);
					todraw->bbox2.y = ARX_CLEAN_WARN_CAST_SHORT(fHeight);



					float fX1 = (todraw->bbox1.x+BOOKDECX) * Xratio ;
					float fX2 = (todraw->bbox2.x+BOOKDECX) * Xratio ;
					float fY1 = (todraw->bbox1.y+BOOKDECY) * Yratio ;
					float fY2 = (todraw->bbox2.y+BOOKDECY) * Yratio ;

					ARX_CHECK_SHORT(fX1);
					ARX_CHECK_SHORT(fY1);
					ARX_CHECK_SHORT(fX2);
					ARX_CHECK_SHORT(fY2);

					todraw->bbox1.x = ARX_CLEAN_WARN_CAST_SHORT(fX1);
					todraw->bbox2.x = ARX_CLEAN_WARN_CAST_SHORT(fX2);
					todraw->bbox1.y = ARX_CLEAN_WARN_CAST_SHORT(fY1);
					todraw->bbox2.y = ARX_CLEAN_WARN_CAST_SHORT(fY2);


					todraw->ioflags|=IO_ICONIC;
				}
			}
			
			if (!BOOKZOOM)
				ARX_EQUIPMENT_AttachPlayerWeaponToBack();
			
			//blue halo rendering (keyword : BLUE HALO RENDERING HIGHLIGHT AURA)
			if (HALOCUR>0)
			{
				SETTC(GDevice,NULL);
				SETBLENDMODE(GDevice,D3DBLEND_SRCCOLOR,D3DBLEND_ONE);
				SETALPHABLEND(GDevice,TRUE);			
				SETCULL(GDevice,D3DCULL_NONE);
				SETZWRITE(GDevice,FALSE);

				for (int i=0;i<HALOCUR;i++)
				{
					D3DTLVERTEX * vert=&LATERDRAWHALO[(i<<2)];

					if (vert[2].color == 0)
					{
						SETBLENDMODE(GDevice,D3DBLEND_ZERO,D3DBLEND_INVSRCCOLOR);
						vert[2].color =0xFF000000;
						EERIEDRAWPRIM(GDevice,D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE , vert, 4,  0, 0 ); //>>> DO NOT USE VERTEX BUFFER HERE <<<
						SETBLENDMODE(GDevice,D3DBLEND_SRCCOLOR,D3DBLEND_ONE);
					}
					else EERIEDRAWPRIM(GDevice,D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE , vert, 4,  0, 0 ); //>>> DO NOT USE VERTEX BUFFER HERE <<<
				}

				HALOCUR=0;
				SETALPHABLEND(GDevice,FALSE);			
			}
		}
	}	

	// Restoring bSoftRender (Fix && bGATI8500).
	bSoftRender	=	bOldSoftRender;
	bGATI8500	=	bOldGATI8500;
}


//-----------------------------------------------------------------------------
void DANAE::DrawAllInterfaceFinish()
{
	currpos = ARX_CLEAN_WARN_CAST_LONG(INTERFACE_RATIO(50.f));
	float rrr;
	rrr=1.f-PULSATE*DIV2;

	if (rrr>1.f) rrr=1.f;
	else if (rrr<0.f) rrr=0.f;
	
	SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
	SETALPHABLEND(GDevice,TRUE);
	PRECAST_NUM=0;

	for (long i=0;i<MAX_SPELLS;i++) 
	{
		if ((spells[i].exist) && (spells[i].caster==0))
			if (spellicons[spells[i].type].bDuration)
				ManageSpellIcon(i,rrr,0);
	}

	if (inter.iobj[0])
	{
		for (int i=0;i<inter.iobj[0]->nb_spells_on;i++)
		{
			if (spells[inter.iobj[0]->spells_on[i]].caster!=0)
				if (spellicons[spells[i].type].bDuration)
					ManageSpellIcon(inter.iobj[0]->spells_on[i],rrr,1);
		}
	}

	if (!(player.Interface & INTER_INVENTORYALL) && !(player.Interface & INTER_MAP))
	{
		for (int i=0;i<MAX_PRECAST;i++) 
		{
			PRECAST_NUM=i;

			if (Precast[i].typ!=-1)
			{
				float val=rrr;

				if ((Precast[i].launch_time>0) &&(ARXTime>=Precast[i].launch_time))
				{
					float tt=(ARXTime-Precast[i].launch_time)*DIV1000;

					if (tt>1.f) tt=1.f;

					val*=(1.f-tt);
				}

				ManageSpellIcon(Precast[i].typ,val,2);			
			}
		}
	}
}

void ARX_INTERFACE_DrawCurrentTorch()
{
	if ((player.Interface & INTER_NOTE) && (TSecondaryInventory != NULL)
		&& ((Note.type == NOTE_TYPE_BIGNOTE) || (Note.type == NOTE_TYPE_BOOK))
		)
		return;

	float px, py;

	px = INTERFACE_RATIO(InventoryX) + INTERFACE_RATIO(110);

	if (px < INTERFACE_RATIO(10)) px = INTERFACE_RATIO(10);

	py = DANAESIZY - INTERFACE_RATIO(158+32);

	GDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,false);

	EERIEDrawBitmap(GDevice,
		px, py,
	                INTERFACE_RATIO_DWORD(CURRENT_TORCH->inv->m_dwWidth), INTERFACE_RATIO_DWORD(CURRENT_TORCH->inv->m_dwHeight),
		0.001f,
		CURRENT_TORCH->inv,D3DCOLORWHITE);
	danaeApp.EnableZBuffer();
	
	if ( rnd() > 0.2f )
	{
		long j=ARX_PARTICLES_GetFree();

		if ( ( j != -1 ) && ( !ARXPausedTimer ) )
		{
			ParticleCount++;
			PARTICLE_DEF * pd	=	&particle[j];
			pd->special			=	FIRE_TO_SMOKE;
			pd->exist			=	TRUE;
			pd->zdec			=	0;
			pd->ov.x 			=	px + INTERFACE_RATIO( 12 ) + rnd() * INTERFACE_RATIO( 3.f ) - rnd() * INTERFACE_RATIO( 6.f );
			pd->ov.y 			=	py + rnd() * INTERFACE_RATIO( 6.f );
			pd->ov.z			=	0.0000001f;
			pd->move.x 			=	INTERFACE_RATIO(1.5f)-rnd()*INTERFACE_RATIO(3.f);
			pd->move.y 			=	-INTERFACE_RATIO(5.f)+rnd()*INTERFACE_RATIO(1.f);
			pd->move.z 			=	0.f;
			pd->scale.x 		=	1.8f;
			pd->scale.y 		=	1.8f;
			pd->scale.z 		=	1.f;
			pd->timcreation		=	lARXTime;
			pd->tolive			=	500+(unsigned long)(rnd()*400.f);
			pd->tc				=	fire2;
			pd->r				=	1.f;
			pd->g				=	0.6f;
			pd->b				=	0.5f;
			pd->siz				=	INTERFACE_RATIO(14.f);
			pd->type			=	PARTICLE_2D;
		}
	}
}

extern float GLOBAL_SLOWDOWN;
extern long SPLASH_THINGS_STAGE;
//-----------------------------------------------------------------------------
void DANAE::DrawAllInterface()
{
	
	GDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_POINT );
	GDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_POINT );
	SETTEXTUREWRAPMODE(GDevice,D3DTADDRESS_CLAMP);
	
	if (!EDITMODE)
	{
		//---------------------------------------------------------------------
		if (player.Interface & INTER_COMBATMODE)
		{
			float j;

			if (AimTime==0) j=0.2f;
			else
			{
				if (BOW_FOCAL)
				{
					j=(float)(BOW_FOCAL)/710.f;
				}
				else
				{
					float at=(float)ARXTime-(float)AimTime;

					if (at>0.f)
						bIsAiming = true;
					else
						bIsAiming = false;

					at=at*(1.f+(1.f-GLOBAL_SLOWDOWN));
					float aim = ARX_CLEAN_WARN_CAST_FLOAT(player.Full_AimTime);
					j=at/aim;					
				}

				if (j>1.f) j=1.f;
				else if (j<0.2f) j=0.2f;

			}

			SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);

			GDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
			ARX_INTERFACE_DrawItem(ITC.aim_maxi, DANAECENTERX + INTERFACE_RATIO(-320+262.f), DANAESIZY + INTERFACE_RATIO(-72.f), 0.0001f, D3DRGB(j,j,j));
			GDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
			ARX_INTERFACE_DrawItem(ITC.aim_empty, DANAECENTERX + INTERFACE_RATIO(-320+262.f), DANAESIZY + INTERFACE_RATIO(-72.f), 0.0001f, D3DRGB(1,1,1));
			
			if (bHitFlash)
			{
				if (player.Full_Skill_Etheral_Link >= 40)
				{
					float j = 1.0f - fHitFlash;
					SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);

					GDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, true );
					long col = 0;

					if (j < 0.5f)
						col = D3DRGB(j*2.0f,1,0);
					else
						col = D3DRGB(1,fHitFlash,0);

					ARX_INTERFACE_DrawItem(ITC.aim_hit, DANAECENTERX + INTERFACE_RATIO(-320+262.f-25), DANAESIZY + INTERFACE_RATIO(-72.f-30), 0.0001f, col);
					GDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, false);
				}
			}
		}

		if (bHitFlash)
		{
			

			float fCalc = ulHitFlash + Original_framedelay ;
			ARX_CHECK_ULONG(fCalc);
			
			ulHitFlash = ARX_CLEAN_WARN_CAST_ULONG(fCalc);


			
			if (ulHitFlash >= 500)
			{
				bHitFlash = false;
				ulHitFlash = 0;
			}
		}
		
		//---------------------------------------------------------------------
		INTERACTIVE_OBJ * io = NULL;
		
		if (SecondaryInventory!=NULL)
		{
			io = (INTERACTIVE_OBJ *)SecondaryInventory->io;
		}
		else if (player.Interface & INTER_STEAL)
		{
			io = ioSteal;
		}

		if (io!=NULL)
		{
			float dist=Distance3D(io->pos.x,io->pos.y,io->pos.z,
				player.pos.x,player.pos.y+80.f,player.pos.z);

			if (Project.telekinesis)
			{
				if (dist > 900.f) 
				{
					if (InventoryDir != -1)
					{
						ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd()); 

						InventoryDir=-1;
						SendIOScriptEvent(io,SM_INVENTORY2_CLOSE,"");
						TSecondaryInventory=SecondaryInventory;
						SecondaryInventory=NULL;
					}
					else
					{
						if (player.Interface & INTER_STEAL)
						{
							player.Interface &= ~INTER_STEAL;
						}
					}
				}
			}
			else if (dist > 350.f) 
			{
				if (InventoryDir != -1)
				{
					ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd()); 

					InventoryDir=-1;
					SendIOScriptEvent(io,SM_INVENTORY2_CLOSE,"");
					TSecondaryInventory=SecondaryInventory;
					SecondaryInventory=NULL;
				}
				else
				{
					if (player.Interface & INTER_STEAL)
					{
						player.Interface &= ~INTER_STEAL;
					}
				}
			}
		}
		else if (InventoryDir != -1)
			{
				InventoryDir = -1;
			}

		if (!PLAYER_INTERFACE_HIDE_COUNT && (TSecondaryInventory != NULL))    
		{
			ARX_INTERFACE_DrawSecondaryInventory((bool)((player.Interface & INTER_STEAL) != 0));
		}

		if (!PLAYER_INTERFACE_HIDE_COUNT)
		{
			if ((player.Interface & INTER_INVENTORY))
			{

				if ((player.Interface & INTER_COMBATMODE) || (player.doingmagic>=2))
				{
					long t;
					F2L((float)(Original_framedelay*DIV5)+2,&t);
					InventoryY += ARX_CLEAN_WARN_CAST_LONG(INTERFACE_RATIO_LONG(t));

					if ( InventoryY > INTERFACE_RATIO( 110.f ) ) InventoryY = ARX_CLEAN_WARN_CAST_LONG( INTERFACE_RATIO( 110.f ) );


				}
				else 
				{
					if (bInventoryClosing) 
					{
						long t;
						F2L((float)(Original_framedelay*DIV5)+2,&t);
						InventoryY += ARX_CLEAN_WARN_CAST_LONG(INTERFACE_RATIO_LONG(t));

						if (InventoryY > INTERFACE_RATIO(110))
						{
							InventoryY = ARX_CLEAN_WARN_CAST_LONG( INTERFACE_RATIO( 110.f ) );
							bInventoryClosing = false;

								player.Interface &=~ INTER_INVENTORY;

							if (bInventorySwitch)
							{
								bInventorySwitch = false;
								ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
								player.Interface |= INTER_INVENTORYALL;
								ARX_INTERFACE_NoteClose();
								InventoryY = ARX_CLEAN_WARN_CAST_LONG( INTERFACE_RATIO( 121.f ) * (player.bag) );
							lOldInterface=INTER_INVENTORYALL;
							}
						}
					}
					else if (InventoryY>0)
					{
						InventoryY -= ARX_CLEAN_WARN_CAST_LONG( INTERFACE_RATIO( ( Original_framedelay * DIV5 ) + 2.f ) );


						if (InventoryY<0) InventoryY=0;
					}
				}


				
				if (player.bag)
				{
					ARX_INTERFACE_DrawInventory(sActiveInventory);


					float fCenterX	= DANAECENTERX + INTERFACE_RATIO(-320 + 35) + INTERFACE_RATIO_DWORD(ITC.hero_inventory->m_dwWidth) - INTERFACE_RATIO(32 + 3) ;
					float fSizY		= DANAESIZY - INTERFACE_RATIO(101) + INTERFACE_RATIO_LONG(InventoryY) + INTERFACE_RATIO(- 3 + 25) ;
					ARX_CHECK_INT(fCenterX);
					ARX_CHECK_INT(fSizY);


					float posx = ARX_CAST_TO_INT_THEN_FLOAT( fCenterX );
					float posy = ARX_CAST_TO_INT_THEN_FLOAT( fSizY );



					if (sActiveInventory > 0)
					{
						ARX_INTERFACE_DrawItem(ITC.hero_inventory_up,	posx, posy);

						if (MouseInRect(posx, posy, posx+INTERFACE_RATIO(32), posy+INTERFACE_RATIO(32)))
						{
							SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
							SETALPHABLEND(GDevice,true);
							SpecialCursor=CURSOR_INTERACTION_ON;
							ARX_INTERFACE_DrawItem(ITC.hero_inventory_up, posx, posy);
							SETALPHABLEND(GDevice,false);
							SpecialCursor=CURSOR_INTERACTION_ON;

							if (((EERIEMouseButton & 1)  && !(LastMouseClick & 1))
								|| ((!(EERIEMouseButton & 1)) && (LastMouseClick & 1) && (DRAGINTER!=NULL)))
							{
								if (sActiveInventory > 0)
								{
									ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
									sActiveInventory --;
								}

								EERIEMouseButton &=~1;
							}
						}
					}

					if (sActiveInventory < player.bag-1)
					{

						float fRatio = INTERFACE_RATIO(32 + 5);
						ARX_CHECK_INT(posy + fRatio);
						
						posy	+= ARX_CLEAN_WARN_CAST_INT(fRatio);


						ARX_INTERFACE_DrawItem(ITC.hero_inventory_down,	posx, DANAESIZY - INTERFACE_RATIO(101) + INTERFACE_RATIO_LONG(InventoryY) + INTERFACE_RATIO(-3 + 64));

						if (MouseInRect(posx, posy, posx+INTERFACE_RATIO(32), posy+INTERFACE_RATIO(32)))
						{
							SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
							SETALPHABLEND(GDevice,true);
							ARX_INTERFACE_DrawItem(ITC.hero_inventory_down,	posx, DANAESIZY - INTERFACE_RATIO(101) + INTERFACE_RATIO_LONG(InventoryY) + INTERFACE_RATIO(-3 + 64));
							SETALPHABLEND(GDevice,false);
							SpecialCursor=CURSOR_INTERACTION_ON;

							if (((EERIEMouseButton & 1)  && !(LastMouseClick & 1))
								|| ((!(EERIEMouseButton & 1)) && (LastMouseClick & 1) && (DRAGINTER!=NULL)))
							{
								
								if (sActiveInventory < player.bag-1)
								{
									ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
									sActiveInventory ++;
								}

								EERIEMouseButton &=~1;
							}
						}
					}
				}
			}
			else if ((player.Interface & INTER_INVENTORYALL) || bInventoryClosing)
			{
				float fSpeed = DIV3;

				if ((player.Interface & INTER_COMBATMODE) || (player.doingmagic>=2))
				{
					if (InventoryY < INTERFACE_RATIO(121)*player.bag)
					{

						InventoryY += ARX_CLEAN_WARN_CAST_LONG(INTERFACE_RATIO((Original_framedelay * fSpeed) + 2.f));
					}
				}
				else 
				{
					if (bInventoryClosing)
					{
						InventoryY += ARX_CLEAN_WARN_CAST_LONG(INTERFACE_RATIO((Original_framedelay * fSpeed) + 2.f));
						
						if ( InventoryY > INTERFACE_RATIO(121) * player.bag )
						{
							bInventoryClosing = false;

							if (player.Interface & INTER_INVENTORYALL)
							{
								player.Interface &=~ INTER_INVENTORYALL;
							}

							lOldInterface=0;
						}
					}
					else if (InventoryY>0)
					{
						InventoryY -= ARX_CLEAN_WARN_CAST_LONG(INTERFACE_RATIO((Original_framedelay * fSpeed) + 2.f));

						if (InventoryY<0)
						{
							InventoryY=0;
						}
					}
				}
				
				

				const float fBag		= (player.bag-1)*INTERFACE_RATIO(-121); 
				float fCenterX	= DANAECENTERX + INTERFACE_RATIO(-320+35);
				float fSizY		= DANAESIZY - INTERFACE_RATIO(101) + INTERFACE_RATIO_LONG(InventoryY) + INTERFACE_RATIO(-3.f + 25 - 32);
				const float fOffsetY	= INTERFACE_RATIO(121);
				ARX_CHECK_INT(fBag);
				ARX_CHECK_INT(fCenterX);
				ARX_CHECK_INT(fSizY);
				ARX_CHECK_INT(fBag + fOffsetY);
		
				
				int iOffsetY	= ARX_CLEAN_WARN_CAST_INT(fBag);
				int posx		= ARX_CLEAN_WARN_CAST_INT(fCenterX);
				int posy		= ARX_CLEAN_WARN_CAST_INT(fSizY);
				iOffsetY		+=ARX_CLEAN_WARN_CAST_INT(fOffsetY);


				for (int i=0; i<player.bag; i++)
				{
					ARX_INTERFACE_DrawItem(ITC.hero_inventory_link, posx + INTERFACE_RATIO(45), ARX_CLEAN_WARN_CAST_FLOAT(posy + iOffsetY)) ;

					ARX_INTERFACE_DrawItem(ITC.hero_inventory_link, posx+INTERFACE_RATIO_DWORD(ITC.hero_inventory->m_dwWidth)*0.5f + INTERFACE_RATIO(-16), posy+iOffsetY + INTERFACE_RATIO(-5));
					ARX_INTERFACE_DrawItem(ITC.hero_inventory_link, posx+INTERFACE_RATIO_DWORD(ITC.hero_inventory->m_dwWidth) + INTERFACE_RATIO(-45-32), posy+iOffsetY + INTERFACE_RATIO(-15));



					ARX_CHECK_INT(iOffsetY + fOffsetY);
					
					iOffsetY	+= ARX_CLEAN_WARN_CAST_INT(fOffsetY);

				
				}



				iOffsetY = ARX_CLEAN_WARN_CAST_INT(fBag);

				for (short i=0; i<player.bag; i++)
				{
					ARX_INTERFACE_DrawInventory(i, 0, iOffsetY);

					ARX_CHECK_INT(iOffsetY + fOffsetY);
					iOffsetY	+= ARX_CLEAN_WARN_CAST_INT(fOffsetY);
				}




			}
	}
	
	
	
	if ((FlyingOverIO) && !(PLAYER_MOUSELOOK_ON) && !(player.Interface & INTER_COMBATMODE)
		&& (!ARX_IMPULSE_Pressed(CONTROLS_CUST_MAGICMODE))
		        || 
		((FlyingOverIO) && (pMenuConfig->bAutoReadyWeapon == false) && !(player.Interface & INTER_COMBATMODE)
		&& (!ARX_IMPULSE_Pressed(CONTROLS_CUST_MAGICMODE)))
		)
	{
		if ((FlyingOverIO->ioflags & IO_ITEM) && (!DRAGINTER))
			if (SecondaryInventory!=NULL)
			{
				INTERACTIVE_OBJ * temp=(INTERACTIVE_OBJ *)SecondaryInventory->io;

				if (temp->ioflags & IO_SHOP)
				{
					float px	=	DANAEMouse.x;
						float py	=	ARX_CLEAN_WARN_CAST_FLOAT(DANAEMouse.y - 10);

					if (InSecondaryInventoryPos(&DANAEMouse))
					{
						long amount=ARX_INTERACTIVE_GetPrice(FlyingOverIO,temp);
						// achat
						float famount	= amount - amount * ( (float)player.Full_Skill_Intuition ) * 0.005f;
						ARX_CHECK_LONG( famount ); //Should always be OK because amount is supposed positive
						amount			= ARX_CLEAN_WARN_CAST_LONG( famount );


						if ( amount <= player.gold )
							ARX_INTERFACE_DrawNumber(px,py,amount,6,D3DRGB(0,1,0));
						else
							ARX_INTERFACE_DrawNumber(px,py,amount,6,D3DRGB(1,0,0));
					}
					else if (InPlayerInventoryPos(&DANAEMouse))
					{
						long amount = ARX_CLEAN_WARN_CAST_LONG( ARX_INTERACTIVE_GetPrice( FlyingOverIO, temp ) / 3.0f );
						// achat
						float famount	= amount + amount * ( (float) player.Full_Skill_Intuition ) * 0.005f;
						ARX_CHECK_LONG( famount ); //Should always be OK because amount is supposed positive
						amount			= ARX_CLEAN_WARN_CAST_LONG( famount );


						if (amount)
						{
							if ((!(temp->shop_category)) ||
								((temp->shop_category) && (IsIOGroup(FlyingOverIO,temp->shop_category))))
								ARX_INTERFACE_DrawNumber(px,py,amount,6,D3DRGB(0,1,0));
							else
								ARX_INTERFACE_DrawNumber(px,py,amount,6,D3DRGB(1,0,0));
						}
					}
				}
			}

			SpecialCursor=CURSOR_INTERACTION_ON;
	}
	
		ARX_INTERFACE_DrawDamagedEquipment(); 

	if ((!(player.Interface & INTER_COMBATMODE)))
	{
		if (player.Interface & INTER_MINIBACK) 
		{
			// Draw/Manage Book Icon
			float px=DANAESIZX - INTERFACE_RATIO(35) + lSLID_VALUE+GL_DECAL_ICONS;
			float py=DANAESIZY - INTERFACE_RATIO(148);
			ARX_INTERFACE_DrawItem(ITC.book, px, py);

			if (eMouseState == MOUSE_IN_BOOK_ICON)
			{
				SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
				SETALPHABLEND(GDevice,true);
				ARX_INTERFACE_DrawItem(ITC.book, px, py);
				SETALPHABLEND(GDevice,false);
			}
			
			// Draw/Manage BackPack Icon
			px=DANAESIZX - INTERFACE_RATIO(35) + lSLID_VALUE+GL_DECAL_ICONS;
			py=DANAESIZY - INTERFACE_RATIO(113);
			ARX_INTERFACE_DrawItem(ITC.backpack,px,py);

			if (eMouseState == MOUSE_IN_INVENTORY_ICON)
			{
				SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
				SETALPHABLEND(GDevice,true);
				ARX_INTERFACE_DrawItem(ITC.backpack,px,py);
				SETALPHABLEND(GDevice,false);
			}
			
			// Draw/Manage Steal Icon
			if (player.Interface & INTER_STEAL)
			{
					px = ARX_CLEAN_WARN_CAST_FLOAT(-lSLID_VALUE);
				py = DANAESIZY - INTERFACE_RATIO(78.f + 32);
				ARX_INTERFACE_DrawItem(ITC.steal, px, py);

				if (eMouseState == MOUSE_IN_STEAL_ICON)
				{
					SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
					SETALPHABLEND(GDevice,true);
					ARX_INTERFACE_DrawItem(ITC.steal, px, py);
					SETALPHABLEND(GDevice,false);
				}
			}
			
			// Draw / Manage Pick All - Close Secondary inventory icon
			if (!PLAYER_INTERFACE_HIDE_COUNT && (TSecondaryInventory!=NULL))
			{
					px = INTERFACE_RATIO(InventoryX) + INTERFACE_RATIO(16);
					py = INTERFACE_RATIO_DWORD(BasicInventorySkin->m_dwHeight) - INTERFACE_RATIO(16);
				INTERACTIVE_OBJ * temp=(INTERACTIVE_OBJ *)TSecondaryInventory->io;

				if (temp && !(temp->ioflags & IO_SHOP) && !(temp == ioSteal))
				{
					ARX_INTERFACE_DrawItem(ITC.inventory_pickall, px, py);

					if (eMouseState == MOUSE_IN_INVENTORY_PICKALL_ICON)
					{
						SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
						SETALPHABLEND(GDevice,true);
						ARX_INTERFACE_DrawItem(ITC.inventory_pickall, px, py);
						SETALPHABLEND(GDevice,false);
					}
				}
				
					px = INTERFACE_RATIO(InventoryX) + INTERFACE_RATIO_DWORD(BasicInventorySkin->m_dwWidth) - INTERFACE_RATIO(32);

				ARX_INTERFACE_DrawItem(ITC.inventory_close, px, py);

				if (eMouseState == MOUSE_IN_INVENTORY_CLOSE_ICON)
				{
					SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
					SETALPHABLEND(GDevice,true);
					ARX_INTERFACE_DrawItem(ITC.inventory_close, px, py);
					SETALPHABLEND(GDevice,false);
				}
			}
			
			// Draw/Manage Advancement Icon				
			if ((player.Skill_Redistribute) || (player.Attribute_Redistribute))
			{
				px=DANAESIZX - INTERFACE_RATIO(35) + lSLID_VALUE+GL_DECAL_ICONS;
				py=DANAESIZY - INTERFACE_RATIO(218);
				ARX_INTERFACE_DrawItem(ITC.Icon_Lvl_Up,px,py);		

				if (eMouseState == MOUSE_IN_REDIST_ICON)
				{
					SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
					SETALPHABLEND(GDevice,true);
					ARX_INTERFACE_DrawItem(ITC.Icon_Lvl_Up,px,py);		
					SETALPHABLEND(GDevice,false);	
				}			  
			}
			
			// Draw/Manage Gold Purse Icon
			if (player.gold>0)
			{
				px = DANAESIZX - INTERFACE_RATIO(35) + lSLID_VALUE+2+GL_DECAL_ICONS;
				py = DANAESIZY - INTERFACE_RATIO(183);
				ARX_INTERFACE_DrawItem(ITC.gold, px, py);

				if (eMouseState == MOUSE_IN_GOLD_ICON)
				{
					SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
					SETALPHABLEND(GDevice,true);
					SpecialCursor=CURSOR_INTERACTION_ON;
					ARX_INTERFACE_DrawItem(ITC.gold, px, py);
					SETALPHABLEND(GDevice,false);
					ARX_INTERFACE_DrawNumber(px-INTERFACE_RATIO(30),py + INTERFACE_RATIO(10-25), player.gold, 6, D3DRGB(1,1,1));
				}
			}

			if (bGoldHalo)
			{
				

				float fCalc = ulGoldHaloTime + Original_framedelay;
				ARX_CHECK_ULONG(fCalc);
				ulGoldHaloTime = ARX_CLEAN_WARN_CAST_ULONG(fCalc);


				if (ulGoldHaloTime >= 1000) //1seconde
				{
					bGoldHalo = false;
				}

				TextureContainer *tc = ITC.gold;
				TextureContainer *tc2 = NULL;

				if (ITC.gold->CreateHalo(GDevice))
					tc2=ITC.gold->TextureHalo;

				if (tc2 != NULL) 
				{
					ARX_INTERFACE_HALO_Render(0.9f, 0.9f, 0.1f, HALO_ACTIVE, tc, tc2, px, py, INTERFACE_RATIO(1), INTERFACE_RATIO(1));
				}
			}

			if (bBookHalo)
			{


				float fCalc = ulBookHaloTime + Original_framedelay;
				ARX_CHECK_ULONG(fCalc);
				ulBookHaloTime = ARX_CLEAN_WARN_CAST_ULONG(fCalc);


				if (ulBookHaloTime >= 3000) //3secondes
				{
					bBookHalo = false;
				}

				float POSX = DANAESIZX-INTERFACE_RATIO(35)+lSLID_VALUE+GL_DECAL_ICONS;
				float POSY = DANAESIZY-INTERFACE_RATIO(148);
				TextureContainer *tc = ITC.book;
					TextureContainer * tc2 = NULL; 

				if (ITC.book->CreateHalo(GDevice))
					tc2=ITC.book->TextureHalo;

				if (tc2 != NULL) 
				{
					ARX_INTERFACE_HALO_Render(0.2f, 0.4f, 0.8f, HALO_ACTIVE, tc, tc2, POSX, POSY, INTERFACE_RATIO(1), INTERFACE_RATIO(1));
				}
			}
		}
		
		if (CURRENT_TORCH)
			ARX_INTERFACE_DrawCurrentTorch();
		
	}

	if ((CHANGE_LEVEL_ICON>-1) && (ChangeLevel) )
	{

//Setting px and py as float to avoid warning on function ARX_INTERFACE_DrawItem and MouseInRect
			float px = DANAESIZX - INTERFACE_RATIO_DWORD(ChangeLevel->m_dwWidth);
		float py = 0;

		GDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,false);
		float vv = 0.9f - EEsin(FrameTime*DIV50)*DIV2+rnd()*DIV10;

		if ( vv < 0.f ) vv = 0;
		else if ( vv > 1.f ) vv = 1.f;

		ARX_INTERFACE_DrawItem( ChangeLevel, px, py, 0.0001f, D3DRGB( vv, vv, vv ) );
		danaeApp.EnableZBuffer();

			if (MouseInRect(px, py, px + INTERFACE_RATIO_DWORD(ChangeLevel->m_dwWidth), py + INTERFACE_RATIO_DWORD(ChangeLevel->m_dwHeight)))
		{
			SpecialCursor=CURSOR_INTERACTION_ON;

			if (!(EERIEMouseButton & 1) && (LastMouseClick & 1))
			{
				CHANGE_LEVEL_ICON=200;
			}
		}
	}


	// Draw stealth gauge
	if (SPLASH_THINGS_STAGE<11)
		ARX_INTERFACE_Draw_Stealth_Gauge();
	
	// book
	if ((player.Interface & INTER_MAP )&&  (!(player.Interface & INTER_COMBATMODE)))
	{
		ARX_INTERFACE_ManageOpenedBook();
		ARX_INTERFACE_ManageOpenedBook_Finish();
	}


	if (CurrSpellSymbol || player.SpellToMemorize.bSpell)
	{
		int count = 0;
		int count2 = 0; 

		for (long j=0;j<6;j++)
		{
			if (player.SpellToMemorize.iSpellSymbols[j] != 255)
				count++;

			if (SpellSymbol[j] != 255)
				count2 ++;
		}

		count = (count2>count)?count2:count;
		EERIE_3D pos;
		pos.x = DANAESIZX - ((count) * INTERFACE_RATIO(32));

		if (CHANGE_LEVEL_ICON>-1) pos.x -= INTERFACE_RATIO(32);

		pos.y = 0;

		for (int i=0; i<6; i++)
		{
			bool bHalo = false;

			if (SpellSymbol[i] != 255)
			{
				if (SpellSymbol[i] == player.SpellToMemorize.iSpellSymbols[i])
				{
					bHalo = true;
				}
				else
				{
					player.SpellToMemorize.iSpellSymbols[i] = SpellSymbol[i];
					
					for (int j=i+1; j<6; j++)
					{
						player.SpellToMemorize.iSpellSymbols[j] = 255;
					}
				}
			}

			if (player.SpellToMemorize.iSpellSymbols[i]!=255)
			{
				EERIEDrawBitmap2(GDevice, pos.x, pos.y, INTERFACE_RATIO(32), INTERFACE_RATIO(32), 0,
					necklace.pTexTab[player.SpellToMemorize.iSpellSymbols[i]]
					, D3DRGB(1,1,1));
				
				if (bHalo)
				{
					TextureContainer *tc = necklace.pTexTab[player.SpellToMemorize.iSpellSymbols[i]];
					TextureContainer *tc2;
					
					tc->CreateHalo(GDevice);
					
					tc2 = tc->TextureHalo;
						
						ARX_INTERFACE_HALO_Render(0.2f, 0.4f, 0.8f, HALO_ACTIVE, tc, tc2, pos.x, pos.y, INTERFACE_RATIO(1), INTERFACE_RATIO(1));
					}

				if (!(player.rune_flags & (1<<player.SpellToMemorize.iSpellSymbols[i])))
				{
					SETBLENDMODE(GDevice,D3DBLEND_INVDESTCOLOR,D3DBLEND_ONE);
					SETALPHABLEND(GDevice,true);
					
					EERIEDrawBitmap2(GDevice, pos.x, pos.y, INTERFACE_RATIO(32), INTERFACE_RATIO(32), 0,
						Movable
						, D3DRGB(0.8f,0.8f,0.8f));
					SETALPHABLEND(GDevice,false);
				}

				pos.x += INTERFACE_RATIO(32);
			}
		}
		
			if (ARXTime - player.SpellToMemorize.lTimeCreation > 30000) 
		{
			player.SpellToMemorize.bSpell = false;
		}
	}

	if (player.Interface & INTER_LIFE_MANA)
	{
		D3DTLVERTEX v[4];
		float px, py;
		v[0]= D3DTLVERTEX( D3DVECTOR( 0, 0, 0.001f ), 1.f, D3DCOLORWHITE, 1, 0.f, 0.f);
		v[1]= D3DTLVERTEX( D3DVECTOR( 0, 0, 0.001f ), 1.f, D3DCOLORWHITE, 1, 1.f, 0.f);
		v[2]= D3DTLVERTEX( D3DVECTOR( 0, 0, 0.001f ), 1.f, D3DCOLORWHITE, 1, 1.f, 1.f);
		v[3]= D3DTLVERTEX( D3DVECTOR( 0, 0, 0.001f ), 1.f, D3DCOLORWHITE, 1, 0.f, 1.f);
		
		GDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,false);
		px = DANAESIZX - INTERFACE_RATIO(33) + INTERFACE_RATIO(1) + lSLID_VALUE;
		py = DANAESIZY - INTERFACE_RATIO(81);
		ARX_INTERFACE_DrawItem(ITC.empty_gauge_blue, px, py, 0.f); //399

		float fnl=(float)player.life/(float)player.Full_maxlife;
		float fnm=(float)player.mana/(float)player.Full_maxmana;

		
		//---------------------------------------------------------------------
		//RED GAUGE
		unsigned long ulColor=0xFFFF0000;
			float fSLID_VALUE_neg = ARX_CLEAN_WARN_CAST_FLOAT(-lSLID_VALUE);

		if (player.poison>0.f) 
		{
				float val = __min(player.poison, 0.2f) * 255.f * 5.f; 
			long g;
			F2L(val,&g);
			ulColor=0xFF000000 | ((255-g) <<16) | (g & 255)<<8;	
		}

		if ((fInterfaceRatio>1.9f) && ITC.filled_gauge_blue)
		{
			float vuv=(1.f-fnl)*ITC.filled_gauge_red->m_dwHeight;
			long vvv;
			F2L(vuv,&vvv);
				vuv = (float)vvv / ITC.filled_gauge_red->m_dwHeight; 
			//ir=
				EERIEDrawBitmap2DecalY(GDevice, fSLID_VALUE_neg, DANAESIZY - INTERFACE_RATIO(78), INTERFACE_RATIO_DWORD(ITC.filled_gauge_red->m_dwWidth), INTERFACE_RATIO_DWORD(ITC.filled_gauge_red->m_dwHeight), 0.f, ITC.filled_gauge_red, ulColor, vuv);
		}
		else
				EERIEDrawBitmap2DecalY(GDevice, fSLID_VALUE_neg, DANAESIZY - INTERFACE_RATIO(78), INTERFACE_RATIO_DWORD(ITC.filled_gauge_red->m_dwWidth), INTERFACE_RATIO_DWORD(ITC.filled_gauge_red->m_dwHeight), 0.f, ITC.filled_gauge_red, ulColor, (1.f - fnl));

		if (!(player.Interface & INTER_COMBATMODE))
		{
				if (MouseInRect(fSLID_VALUE_neg, DANAESIZY - INTERFACE_RATIO(78), fSLID_VALUE_neg + INTERFACE_RATIO_DWORD(ITC.filled_gauge_red->m_dwWidth), DANAESIZY - INTERFACE_RATIO(78) + INTERFACE_RATIO_DWORD(ITC.filled_gauge_red->m_dwHeight)))
			{
				if(	(EERIEMouseButton & 1)&&
					(!(LastMouseClick & 1)) )
				{
					_TCHAR txt[256];


					ARX_CHECK_INT(player.life);

					ARX_SPEECH_Add(NULL,
						_itot(ARX_CLEAN_WARN_CAST_INT(player.life),txt,10)	);

				}
			}
		}

		//---------------------------------------------------------------------
		//END RED GAUGE
		
		GDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,false);
		px = 0.f-lSLID_VALUE;
		py = DANAESIZY - INTERFACE_RATIO(78);
		ARX_INTERFACE_DrawItem(ITC.empty_gauge_red, px, py, 0.001f);
		
		//---------------------------------------------------------------------
		//BLUE GAUGE

		float LARGG=INTERFACE_RATIO_DWORD(ITC.filled_gauge_blue->m_dwWidth);
		float HAUTT=INTERFACE_RATIO_DWORD(ITC.filled_gauge_blue->m_dwHeight);


		if ((fInterfaceRatio>1.9f) && ITC.filled_gauge_blue)
		{
			float vuv=(1.f-fnm)*ITC.filled_gauge_blue->m_dwHeight;
			long vvv;
			F2L(vuv,&vvv);
				vuv = (float)vvv / ITC.filled_gauge_blue->m_dwHeight; 
			//ir=
				EERIEDrawBitmap2DecalY(GDevice, DANAESIZX - INTERFACE_RATIO(33) + INTERFACE_RATIO(1) + lSLID_VALUE, DANAESIZY - INTERFACE_RATIO(81), LARGG, HAUTT, 0.f, ITC.filled_gauge_blue, ARX_OPAQUE_WHITE /*-1*/, vuv);
		}
		else
				EERIEDrawBitmap2DecalY(GDevice, DANAESIZX - INTERFACE_RATIO(33) + INTERFACE_RATIO(1) + lSLID_VALUE, DANAESIZY - INTERFACE_RATIO(81), LARGG, HAUTT, 0.f, ITC.filled_gauge_blue, ARX_OPAQUE_WHITE /*-1*/, (1.f - fnm));

		if (!(player.Interface & INTER_COMBATMODE))
		{
			if(MouseInRect(DANAESIZX - INTERFACE_RATIO(33) + lSLID_VALUE,DANAESIZY - INTERFACE_RATIO(81),DANAESIZX - INTERFACE_RATIO(33) + lSLID_VALUE+LARGG,DANAESIZY - INTERFACE_RATIO(81)+HAUTT))
			{
				if(	(EERIEMouseButton & 1)&&
					(!(LastMouseClick & 1)) )
				{
					_TCHAR txt[256];

					ARX_CHECK_INT(player.mana);

					ARX_SPEECH_Add(NULL,
						_itot(ARX_CLEAN_WARN_CAST_INT(player.mana),txt,10)	);

				}
			}
		}

		//---------------------------------------------------------------------
		//END BLUE GAUGE

			if (INTERNATIONAL_MODE)
			{
				if (bRenderInCursorMode)
				{
				SETALPHABLEND(GDevice, true);
				SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);
				
					if (mecanism_tc && (MAGICMODE < 0) && (lNbToDrawMecanismCursor < 3))
					{

					long lColorMecanism=D3DRGB(1,1,1);

					if(lTimeToDrawMecanismCursor>300)
					{
						lColorMecanism=0;

						if(lTimeToDrawMecanismCursor>400)
						{
							lTimeToDrawMecanismCursor=0;
							lNbToDrawMecanismCursor++;
						}
					}

						lTimeToDrawMecanismCursor += ARX_CLEAN_WARN_CAST_LONG(FrameDiff);

					EERIEDrawBitmap(	GDevice,
						0,0,

						INTERFACE_RATIO_DWORD(mecanism_tc->m_dwWidth),
						INTERFACE_RATIO_DWORD(mecanism_tc->m_dwHeight),

						0.01f,
						mecanism_tc,
						lColorMecanism );
				}
				
					if (arrow_left_tc)
					{

					float fSizeX=INTERFACE_RATIO_DWORD(arrow_left_tc->m_dwWidth);
					float fSizeY=INTERFACE_RATIO_DWORD(arrow_left_tc->m_dwHeight);

					long lColor=D3DRGB(.5f,.5f,.5f);
					
					static float fArrowMove=0.f;
					fArrowMove+=.5f*FrameDiff;

						if (fArrowMove > 180.f)
						{
						fArrowMove=0.f;
					}

					float fMove=fabs(sin(DEG2RAD(fArrowMove)))*fSizeX*.5f;
					
					EERIEDrawBitmap(	GDevice,					//left
						0+fMove,
						(DANAESIZY-fSizeY)*.5f,
						fSizeX,
						fSizeY,
						0.01f,
						arrow_left_tc,
						lColor );
					
					EERIEDrawBitmapUVs(	GDevice,				//right
						DANAESIZX-fSizeX-fMove,
						(DANAESIZY-fSizeY)*.5f,
						fSizeX,
						fSizeY,
						0.01f,
						arrow_left_tc,
						lColor,
						1.f,0.f,
						0.f,0.f,
						1.f,1.f,
						0.f,1.f);
					
					EERIEDrawBitmapUVs(	GDevice,				//up
						(DANAESIZX-fSizeY)*.5f,
						0.f+fMove,
						fSizeY,
						fSizeX,
						0.01f,
						arrow_left_tc,
						lColor,
						0.f,1.f,
						0.f,0.f,
						1.f,1.f,
						1.f,0.f);
					
					EERIEDrawBitmapUVs(	GDevice,				//down
						(DANAESIZX-fSizeY)*.5f,
						(DANAESIZY-fSizeX)-fMove,
						fSizeY,
						fSizeX,
						0.01f,
						arrow_left_tc,
						lColor,
						1.f,1.f,
						1.f,0.f,
						0.f,1.f,
						0.f,0.f);
				}
				
				SETALPHABLEND(GDevice, false);
			}
		}
		}
	}
	
	danaeApp.EnableZBuffer();
	GDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_LINEAR );
	GDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_LINEAR );
	SETTEXTUREWRAPMODE(GDevice,D3DTADDRESS_WRAP);
}

extern long FRAME_COUNT;
extern INTERACTIVE_OBJ * DESTROYED_DURING_RENDERING;
extern float STARTED_ANGLE;
long SPECIAL_DRAGINTER_RENDER=0;
long CANNOT_PUT_IT_HERE=0;

long Manage3DCursor(long flags) 
{
	if (BLOCK_PLAYER_CONTROLS)
		return 0;

	float ag=player.angle.a;

	if (ag>180) ag=ag-360;

	float drop_miny=(float)(DANAECENTERY)-DANAECENTERY*(ag*DIV70);

	if ((DANAEMouse.y<drop_miny) && (!EDITMODE))
		return 0;

	SetFilteringMode(GDevice,Bilinear);		
	
	INTERACTIVE_OBJ * io=DRAGINTER;

	if (!io) return 0;

	EERIE_3D temp;

	if (io->ioflags & IO_INVERTED)
	{
		temp.a=180.f;
		temp.b = -MAKEANGLE(io->angle.b - ((player.angle.b) - (STARTED_ANGLE + 90)));
	}
	else
	{
		temp.a = 0; 
		temp.b = MAKEANGLE(io->angle.b - ((player.angle.b) - (STARTED_ANGLE + 90))); 
	}

	temp.g = 0; 
	EERIE_3D pos;
	float angle=DEG2RAD(MAKEANGLE(player.angle.b));
	float angle2=DEG2RAD(MAKEANGLE(player.angle.b-90.f));
				
	float zrange=(DANAESIZY-DANAEMouse.y)/(DANAESIZY-drop_miny); //between 0 (bottom) and 1 (top)
	float va=player.angle.a;

	if (va>180) va=0;
				
				float vd=(100.f-va)*DIV90;
				float mod=va-50.f;

				if (mod<0) mod=0;

				mod*=DIV20;
	va = (vd) * (1.3f + 0.3f * mod); 
				
				vd=((1.f-zrange)*0.6f-vd)*150.f;

				if (va<0) va=0;

				if (vd<0) vd=0;
				
				float mx = DANAEMouse.x;
				float my = DANAEMouse.y;
				
				if (TRUE_PLAYER_MOUSELOOK_ON && (pMenuConfig->bAutoReadyWeapon))
				{
					mx = MemoMouse.x;
					my = MemoMouse.y;
				}
				
				pos.x=player.pos.x+EEsin(angle2)*(DANAECENTERX-mx)*0.7f*va
					-EEsin(angle)*(va*zrange*400.f+vd);

				pos.z=player.pos.z-EEcos(angle2)*(DANAECENTERX-mx)*0.7f*va
					+EEcos(angle)*(va*zrange*400.f+vd);

				pos.y=player.pos.y;
	
				{ 
					EERIE_3D objcenter;
					Vector_Init(&objcenter);
					float maxdist= 0.f;
					float miny=  99999999.f;
					float maxy= -99999999.f;
					EERIE_3D minoff;
					EERIE_3D maxoff;
					maxoff.x=minoff.x=io->obj->vertexlist[0].v.x;
					maxoff.y=minoff.y=io->obj->vertexlist[0].v.y;
					maxoff.z=minoff.z=io->obj->vertexlist[0].v.z;

					for (long i=0;i<io->obj->nbvertex;i++)
					{
						maxoff.x=__max(maxoff.x,io->obj->vertexlist[i].v.x);
						maxoff.y=__max(maxoff.y,io->obj->vertexlist[i].v.y);
						maxoff.z=__max(maxoff.z,io->obj->vertexlist[i].v.z);
						minoff.x=__min(minoff.x,io->obj->vertexlist[i].v.x);
						minoff.y=__min(minoff.y,io->obj->vertexlist[i].v.y);
						minoff.z=__min(minoff.z,io->obj->vertexlist[i].v.z);
						miny=__min(miny,io->obj->vertexlist[i].v.y);
						maxy=__max(maxy,io->obj->vertexlist[i].v.y);
					}


					EERIE_CYLINDER cyl;
					cyl.origin.x=pos.x-(maxoff.x-minoff.x)*DIV2;
					cyl.origin.y=pos.y;
					cyl.origin.z=pos.z-(maxoff.z-minoff.z)*DIV2;
					cyl.height=-50.f;
					cyl.radius=40.f;

					EERIE_3D orgn,dest,mvectx,mvecty;
		mvectx.x = mvecty.x = -(float)EEsin(DEG2RAD(player.angle.b - 90.f)); 
		mvectx.y = mvecty.y = 0; 
		mvectx.z = mvecty.z = +(float)EEcos(DEG2RAD(player.angle.b - 90.f)); 
					TRUEVector_Normalize(&mvectx);
	
					float xmod,ymod;
					xmod=(float)(DANAEMouse.x-DANAECENTERX)/(float)DANAECENTERX*160.f;
					ymod=(float)(DANAEMouse.y-DANAECENTERY)/(float)DANAECENTERY*220.f;
					mvectx.x*=xmod;
					mvectx.y*=xmod;
					mvectx.z*=xmod;
		mvecty.x = 0; 
					mvecty.y=ymod;
		mvecty.z = 0; 

					orgn.x=player.pos.x-(float)EEsin(DEG2RAD(player.angle.b))*(float)EEcos(DEG2RAD(player.angle.a))*50.f
		         + mvectx.x; 
					orgn.y=player.pos.y+(float)EEsin(DEG2RAD(player.angle.a))*50.f
							+mvectx.y+mvecty.y;
					orgn.z=player.pos.z+(float)EEcos(DEG2RAD(player.angle.b))*(float)EEcos(DEG2RAD(player.angle.a))*50.f
		         + mvectx.z; 
	
					dest.x=player.pos.x-(float)EEsin(DEG2RAD(player.angle.b))*(float)EEcos(DEG2RAD(player.angle.a))*10000.f
		         + mvectx.x; 
					dest.y=player.pos.y+(float)EEsin(DEG2RAD(player.angle.a))*10000.f
							+mvectx.y+mvecty.y*5.f;
					dest.z=player.pos.z+(float)EEcos(DEG2RAD(player.angle.b))*(float)EEcos(DEG2RAD(player.angle.a))*10000.f
		         + mvectx.z; 
					pos.x=orgn.x;
					pos.y=orgn.y;
					pos.z=orgn.z;

					EERIE_3D movev;
					movev.x=dest.x-orgn.x;
					movev.y=dest.y-orgn.y;
					movev.z=dest.z-orgn.z;
		TRUEVector_Normalize(&movev);
					
					float lastanything = 0.f;
					
					
					float height = -( maxy - miny );

					if ( height > -30.f ) height = -30.f;
					
		objcenter.x	=	minoff.x + (maxoff.x - minoff.x) * DIV2;  
		objcenter.y	=	0;	
		objcenter.z	=	minoff.z + (maxoff.z - minoff.z) * DIV2;  
					
					for ( int i = 0 ; i < io->obj->nbvertex ; i++ )
					{
						maxdist = __max(	maxdist,
											TRUEDistance2D(	objcenter.x, objcenter.z,
			                               io->obj->vertexlist[i].v.x, io->obj->vertexlist[i].v.z) - 4.f);
					}

					if (io->obj->pbox)
					{
						for (int i=1;i<io->obj->pbox->nb_physvert;i++)
						{
							maxdist=__max(maxdist,
								TRUEDistance2D(	io->obj->pbox->vert[0].initpos.x,
												io->obj->pbox->vert[0].initpos.z,
												io->obj->pbox->vert[i].initpos.x,
				                               io->obj->pbox->vert[i].initpos.z) + 14.f);
						}
					}

					VRotateY(&objcenter,temp.b);

					if ( maxdist < 15.f ) maxdist = 15.f;

					if ( maxdist > 150.f ) maxdist = 150.f;
					
		bool			bCollidposNoInit = true;
					EERIE_3D		collidpos;
					EERIE_CYLINDER	cyl2;
					float			inc			=	10.f;
					long			iterating	=	40;

		collidpos.x = collidpos.y = collidpos.z = 0;
		
		cyl2.height	=	__min(-30.f, height); 
		cyl2.radius	=	__max(20.f, maxdist); 
					

					while ( iterating>0 )
					{
						
			cyl2.origin.x = pos.x + movev.x * inc;
			cyl2.origin.y = pos.y + maxy + movev.y * inc; 
			cyl2.origin.z = pos.z + movev.z * inc;
						
						float anything = CheckAnythingInCylinder( &cyl2, io, CFLAG_JUST_TEST | CFLAG_COLLIDE_NOCOL | CFLAG_NO_NPC_COLLIDE );

						if ( anything < 0.f )
						{
							if ( iterating == 40 )
							{
								CANNOT_PUT_IT_HERE = 1;
								return -1;
							}

							iterating = 0;

							Vector_Copy( &collidpos, &cyl2.origin );
				bCollidposNoInit = false;

							if ( lastanything < 0.f )
							{
								pos.y		+= lastanything;
								collidpos.y	+= lastanything;
							}
						}
						else
						{
							pos.x			= cyl2.origin.x;
							pos.y			= cyl2.origin.y;
							pos.z			= cyl2.origin.z;
							lastanything	= anything;
						}

						iterating--;
					}
					
					collidpos.x	-=	objcenter.x;
					collidpos.z	-=	objcenter.z;
					pos.x		-=	objcenter.x;
					pos.z		-=	objcenter.z;

					if ( iterating != -1 )
					{
						CANNOT_PUT_IT_HERE = 1;
						return 0;
					}

		if ((iterating == -1) && (EEDistance3D(&player.pos, &pos) < 300.f))
					{
						if ( flags & 1 )
						{
							io->obj->drawflags |= DRAWFLAG_HIGHLIGHT;
							ARX_INTERACTIVE_Teleport( io, &pos, 1 );
							
							int iOldFrameCount	=	FRAME_COUNT;
							FRAME_COUNT			=	0;
							io->GameFlags		&=	~GFLAG_NOCOMPUTATION;

							float old			=	io->invisibility;
							

							if( bCollidposNoInit )
							{
					ARX_CHECK_NO_ENTRY();
							}



							EERIE_3D vec;
							vec.x			= collidpos.x - pos.x;
							vec.y			= collidpos.y - pos.y;
							vec.z			= collidpos.z - pos.z;
				TRUEVector_Normalize(&vec);

							if (SPECIAL_DRAGINTER_RENDER) 
							{
								
							if (((lastanything<0.f) && (EEfabs(lastanything)>EEfabs(height)))
								|| (lastanything>EEfabs(height)))
							{
						io->invisibility = 0.5f;
								DrawEERIEInter(GDevice,io->obj,&temp,&collidpos,io);
								io->invisibility=old;								
							}
							else if (lastanything>0.f)
						DrawEERIEInter(GDevice, io->obj, &temp, &pos, io);
							else 
								DrawEERIEInter(GDevice,io->obj,&temp,&pos,io);
							}
							
							PrecalcIOLighting(NULL,0,1);
							FRAME_COUNT=iOldFrameCount;
							
							if (	(!SPECIAL_DRAGINTER_RENDER) 
								&&	(INTERTRANSPOLYSPOS)	)
								ARXDRAW_DrawAllInterTransPolyPos(GDevice);

							if (!SPECIAL_DRAGINTER_RENDER && !DESTROYED_DURING_RENDERING) 							
							{
								io->obj->drawflags&=~DRAWFLAG_HIGHLIGHT;

								if (io->ignition>0.f)
									ManageIgnition(io);
							}
						}
						else
						{
				if (EEfabs(lastanything) > __min(EEfabs(height), 12))
							{
								INTERACTIVE_OBJ * io=DRAGINTER;
								ARX_PLAYER_Remove_Invisibility();
								io->obj->pbox->active=1;
								io->obj->pbox->stopcount=0;
								float vx=-((float)DANAEMouse.x-(float)subj.centerx);
 
								vx/=3.f;
								EERIE_3D pos;
					pos.x = io->pos.x = collidpos.x; 
					pos.z = io->pos.z = collidpos.z; 
								pos.y=io->pos.y=collidpos.y;
								io->velocity.x=0.f;
								io->velocity.y=0.f;
								io->velocity.z=0.f;
					
								io->stopped=1;							
								EERIE_3D viewvector;
								
								movev.x*=0.0001f;
								movev.z*=0.0001f;
								movev.y=0.1f;
								Vector_Copy(&viewvector,&movev);

								EERIE_3D angle;
								angle.a=temp.a;
								angle.b=temp.b;
								angle.g=temp.g;
								io->soundtime=0;
								io->soundcount=0;
								EERIE_PHYSICS_BOX_Launch(io->obj,&pos,&viewvector,1,&angle);
								ARX_SOUND_PlaySFX(SND_WHOOSH, &pos);
								io->show=SHOW_FLAG_IN_SCENE;
								Set_DragInter(NULL);
							}
							else
							{

								ARX_PLAYER_Remove_Invisibility();
								ARX_SOUND_PlayInterface(SND_INVSTD);
								ARX_INTERACTIVE_Teleport(io,&pos,1);

								io->angle.a=temp.a;
								io->angle.b=270.f-temp.b;
								io->angle.g=temp.g;
								io->stopped=0;
								io->show=SHOW_FLAG_IN_SCENE;
								io->obj->pbox->active=0;
								Set_DragInter(NULL);
							}
						}

						SETCULL(GDevice,D3DCULL_NONE);
						return 1;
					}
					else 
					{
						CANNOT_PUT_IT_HERE=-1;
					}
				}
	
				return 0;
				
}
//-----------------------------------------------------------------------------
void ARX_INTERFACE_RenderCursor(long flag)
{
	if (!SPECIAL_DRAGINTER_RENDER)
	{
		ManageIgnition_2(DRAGINTER);	
		GDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_POINT );
		GDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_POINT );
		SETTEXTUREWRAPMODE(GDevice,D3DTADDRESS_CLAMP);
	}
	
	TextureContainer * surf;

	if (!SPECIAL_DRAGINTER_RENDER)
	if (LOOKING_FOR_SPELL_TARGET)
	{
		if (ARXTime>LOOKING_FOR_SPELL_TARGET_TIME+7000)
		{
			ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE, &player.pos);
			ARX_SPELLS_CancelSpellTarget();
		}

		if (	(FlyingOverIO)
			&&	(	((LOOKING_FOR_SPELL_TARGET & 1) && (FlyingOverIO->ioflags & IO_NPC))
			|| ((LOOKING_FOR_SPELL_TARGET & 2) && (FlyingOverIO->ioflags & IO_ITEM))	)	)
		{
			surf=ITC.target_on;

			if (!(EERIEMouseButton & 1) && (LastMouseClick & 1))
			{
				ARX_SPELLS_LaunchSpellTarget(FlyingOverIO);
			}
		}
		else 
		{
			surf=ITC.target_off;

			if(ARX_IMPULSE_Pressed(CONTROLS_CUST_MAGICMODE))
			{
				ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE, &player.pos);
				ARX_SPELLS_CancelSpellTarget();
			}
		}

		float POSX=DANAEMouse.x;
		float POSY=DANAEMouse.y;

		if (TRUE_PLAYER_MOUSELOOK_ON)
		{
			POSX = MemoMouse.x;
			POSY = MemoMouse.y;
		}


		float fTexSizeX = INTERFACE_RATIO_DWORD(surf->m_dwWidth);
		float fTexSizeY = INTERFACE_RATIO_DWORD(surf->m_dwHeight);

		EERIEDrawBitmap(GDevice,(float)(POSX-(fTexSizeX*0.5f)),(float)(POSY-(surf->m_dwHeight*0.5f)),
			fTexSizeX, fTexSizeY, 0.f,
			surf,D3DCOLORWHITE);
		
		SETTEXTUREWRAPMODE(GDevice,D3DTADDRESS_WRAP);
		return;
	}

	SPECIAL_DRAW_INTER_SHADOW = 0;
		
	if (flag || ((!BLOCK_PLAYER_CONTROLS) && 
		(!PLAYER_INTERFACE_HIDE_COUNT)))
	{
		RECT rect;

		if (!SPECIAL_DRAGINTER_RENDER)
			SETCULL(GDevice,D3DCULL_NONE);

		if ((COMBINE) || (COMBINEGOLD))
		{
			if (SpecialCursor==CURSOR_INTERACTION_ON)
				SpecialCursor=CURSOR_COMBINEON;
			else SpecialCursor=CURSOR_COMBINEOFF;
		}
		
		if ((SpecialCursor) || !PLAYER_MOUSELOOK_ON || (DRAGINTER!=NULL)
		        || ((FlyingOverIO) && PLAYER_MOUSELOOK_ON && !(ARX_MOUSE_OVER & ARX_MOUSE_OVER_BOOK) 
			&& (eMouseState != MOUSE_IN_NOTE)
			&& (FlyingOverIO->ioflags & IO_ITEM)
			&& (FlyingOverIO->GameFlags & GFLAG_INTERACTIVITY)
			&& (pMenuConfig->bAutoReadyWeapon == false))
			|| ((MAGICMODE==1) && PLAYER_MOUSELOOK_ON))
		{
			
			if (!SPECIAL_DRAGINTER_RENDER)
			{
				if(FlyingOverIO||DRAGINTER)
				{
					fHighLightAng+=(float)(FrameDiff*0.5);

					if(fHighLightAng>90.f) fHighLightAng=90.f;


					float fHLight	= 100.f*sin(DEG2RAD(fHighLightAng));
					ARX_CHECK_INT(fHLight);
					
					iHighLight	= ARX_CLEAN_WARN_CAST_INT(fHLight);

				
				}
				else
				{
					 fHighLightAng=0.f;
					iHighLight = 0;
				}
			}
			
			
			CANNOT_PUT_IT_HERE=0;
			float ag=player.angle.a;

			if (ag>180) ag=ag-360;

			float drop_miny=(float)(DANAECENTERY)-DANAECENTERY*(ag*DIV70);

			if ((DANAEMouse.y>drop_miny) && DRAGINTER && !InInventoryPos(&DANAEMouse)
			        && !(ARX_MOUSE_OVER & ARX_MOUSE_OVER_BOOK))
			{
				if (Manage3DCursor(1)==0)
					CANNOT_PUT_IT_HERE = -1; 

				if (SPECIAL_DRAGINTER_RENDER)
				{
					
					CANNOT_PUT_IT_HERE=0;
					return;
				}
				
			}
			else CANNOT_PUT_IT_HERE = -1; 
						  
			if (SPECIAL_DRAGINTER_RENDER)
				return;

			float POSX,POSY;
			
			POSX=(float)DANAEMouse.x;
			POSY=(float)DANAEMouse.y;
			
			
			if ((SpecialCursor) && (!DRAGINTER))
			{
				if (((COMBINE!=NULL) && (COMBINE->inv!=NULL)) || COMBINEGOLD)
				{
					if (TRUE_PLAYER_MOUSELOOK_ON && (pMenuConfig->bAutoReadyWeapon))
					{
						POSX = MemoMouse.x;
						POSY = MemoMouse.y;
					}

					TextureContainer * tc;

					if (COMBINEGOLD) tc=GoldCoinsTC[5];
					else tc=COMBINE->inv;

					if (tc->m_pddsSurface==NULL)
					{
						D3DTextr_Restore(tc->m_strName,GDevice);
					}

					float MODIF=0.f;
	

					float fTexSizeX = INTERFACE_RATIO_DWORD(tc->m_dwWidth);
					float fTexSizeY = INTERFACE_RATIO_DWORD(tc->m_dwHeight);


					if (SpecialCursor==CURSOR_COMBINEON)
					{
						EERIEDrawBitmap(GDevice,(float)POSX+MODIF,(float)POSY+MODIF
							,(float)fTexSizeX
							,(float)fTexSizeY,0.00001f
							,tc,0xFFFFFFFF);

						if ((FlyingOverIO!=NULL) && (FlyingOverIO->ioflags & IO_BLACKSMITH))
						{
							float v=ARX_DAMAGES_ComputeRepairPrice(COMBINE,FlyingOverIO);

							if (v>0.f)
							{								
								long t;
								F2L(v,&t);
								ARX_INTERFACE_DrawNumber(POSX+MODIF-16,POSY+MODIF-10,t,6,0xFF00FFFF);
							}
						}
					}
					else
						EERIEDrawBitmap(GDevice,(float)POSX+MODIF,(float)POSY+MODIF
						,(float)fTexSizeX
						,(float)fTexSizeY,0.00001f
						,tc,0xFFFFAA66);
				}				
				
				switch (SpecialCursor)
				{
				case CURSOR_REDIST:
					{
						surf = ITC.pTexCursorRedist;
					}
					break;
				case CURSOR_COMBINEOFF:
					surf=ITC.target_off;
						POSX -= 16.f;
						POSY -= 16.f;
					break;
				case CURSOR_COMBINEON:
					surf=ITC.target_on;

					if (surf)
							POSX -= 16.f;

						POSY -= 16.f;
					break;
				case CURSOR_FIREBALLAIM:
					surf=ITC.target_on;

					if (surf)
					{
						rect.right	=	surf->m_dwWidth;
						rect.bottom	=	surf->m_dwHeight;
					}

					else 
					{
						ARX_CHECK_NO_ENTRY();
						rect.right = 0;
						rect.bottom = 0;
					}



					POSX = 320.f - rect.right / 2.f;
					POSY = 280.f - rect.bottom / 2.f;
					break; 
				case CURSOR_INTERACTION_ON:

					ARX_CHECK_LONG( Original_framedelay );
					CURCURTIME += ARX_CLEAN_WARN_CAST_LONG( Original_framedelay );


					if (CURCURPOS!=3) 
					{
						while(CURCURTIME>CURCURDELAY)
						{
							CURCURTIME-=CURCURDELAY;
							CURCURPOS++;
						}
					}

					if (CURCURPOS>7) CURCURPOS=0;

					surf=scursor[CURCURPOS];
					break;
				default:
					if (CURCURPOS!=0)
					{

						ARX_CHECK_LONG( Original_framedelay );
						CURCURTIME += ARX_CLEAN_WARN_CAST_LONG( Original_framedelay );


						while(CURCURTIME>CURCURDELAY)
						{
							CURCURTIME-=CURCURDELAY;
							CURCURPOS++;
						}
					}

					if (CURCURPOS>7) CURCURPOS=0;

					surf=scursor[CURCURPOS];
					break;
				}

				if ((surf))
				{

					if (SpecialCursor == CURSOR_REDIST)
					{
						EERIEDrawBitmap(GDevice,(float)POSX,(float)POSY,
							surf->m_dwWidth * Xratio,
							surf->m_dwHeight * Yratio,
							0.f,
							surf,D3DCOLORWHITE);
						
						danaeApp.DANAEEndRender();	
						_TCHAR temp[256];
						_stprintf(temp, _T("%3d"), lCursorRedistValue);
						ARX_TEXT_Draw(GDevice, InBookFont, DANAEMouse.x + 6* Xratio, DANAEMouse.y + 11* Yratio, 999, 999, temp, D3DCOLORBLACK, 0x00FF00FF);
						danaeApp.DANAEStartRender();
					}
					else
					{

						float fTexSizeX = INTERFACE_RATIO_DWORD(surf->m_dwWidth);
						float fTexSizeY = INTERFACE_RATIO_DWORD(surf->m_dwHeight);

						EERIEDrawBitmap(GDevice,(float)POSX,(float)POSY,
							fTexSizeX, fTexSizeY, 0.f,
							surf,D3DCOLORWHITE);
					}
				}

				SpecialCursor=0;
			}
			else
			{
				if (!(player.Current_Movement & PLAYER_CROUCH) && (!BLOCK_PLAYER_CONTROLS 
					&& (ARX_IMPULSE_Pressed(CONTROLS_CUST_MAGICMODE)))
					&& (ARXmenu.currentmode==AMCM_OFF))
				{
					if (MAGICMODE<0) 
					{
						if (player.Interface & INTER_MAP )
						{
							ARX_INTERFACE_BookOpenClose(2); // Forced Closing
						}

						MAGICMODE=1;
					}

					surf=ITC.magic;

					float POSX=DANAEMouse.x;
					float POSY=DANAEMouse.y;

					if (TRUE_PLAYER_MOUSELOOK_ON)
					{
						POSX = MemoMouse.x;
						POSY = MemoMouse.y;
					}


					float fTexSizeX = INTERFACE_RATIO_DWORD(surf->m_dwWidth);
					float fTexSizeY = INTERFACE_RATIO_DWORD(surf->m_dwHeight);


					EERIEDrawBitmap(	GDevice,
										(float)(POSX - (fTexSizeX*0.5f)),
										(float)(POSY - (fTexSizeY*0.5f)),
										(float)fTexSizeX,
										(float)fTexSizeY,
										0.f,
										surf,
										D3DCOLORWHITE);
				}
				else 
				{
					if (MAGICMODE>-1)
					{
						ARX_SOUND_Stop(SND_MAGIC_DRAW);
						MAGICMODE=-1;
					}

					if ((DRAGINTER!=NULL) && (DRAGINTER->inv!=NULL))
					{
						TextureContainer * tc;
						TextureContainer * tc2=NULL;
						tc=DRAGINTER->inv;

						if (NeedHalo(DRAGINTER)) tc2=DRAGINTER->inv->TextureHalo;//>_itemdata->halo_tc;
						
						if (tc->m_pddsSurface==NULL)
						{
							tc->Restore(GDevice);					
						}	

						D3DCOLOR color;

						if ((DRAGINTER->poisonous) && (DRAGINTER->poisonous_count!=0))
							color=0xFF00FF00;
						else color=D3DCOLORWHITE;
						
						float mx = POSX;
						float my = POSY;

						if (TRUE_PLAYER_MOUSELOOK_ON && (pMenuConfig->bAutoReadyWeapon))
						{
							mx = MemoMouse.x;
							my = MemoMouse.y;
						}
						

						float fTexSizeX = INTERFACE_RATIO_DWORD(tc->m_dwWidth);
						float fTexSizeY = INTERFACE_RATIO_DWORD(tc->m_dwHeight);


						if (!(DRAGINTER->ioflags & IO_MOVABLE))
						{
							EERIEDrawBitmap(GDevice,(float)mx,(float)my
								,(float)fTexSizeX
								,(float)fTexSizeY,0.00001f
								,tc,color);

							if ((DRAGINTER->ioflags & IO_ITEM) && (DRAGINTER->_itemdata->count!=1))
								ARX_INTERFACE_DrawNumber(mx+2.f,my+13.f,
								DRAGINTER->_itemdata->count, 3, D3DCOLORWHITE);
						}
						else
						{
							if ((InInventoryPos(&DANAEMouse) || InSecondaryInventoryPos(&DANAEMouse))
								||
								(CANNOT_PUT_IT_HERE != -1))
							{
								EERIEDrawBitmap(GDevice,(float)mx,(float)my
									,(float)fTexSizeX
									,(float)fTexSizeY,0.00001f
									,tc,color);
							}
						}
						
						//cross not over inventory icon
						if ((CANNOT_PUT_IT_HERE!=0) && (eMouseState != MOUSE_IN_INVENTORY_ICON))
						{
							if (!InInventoryPos(&DANAEMouse) && !InSecondaryInventoryPos(&DANAEMouse) && !ARX_INTERFACE_MouseInBook())
							{
								TextureContainer * tcc=Movable;

								if (CANNOT_PUT_IT_HERE==-1)
									tcc=ThrowObject;

								if ((tcc) && (tcc!=tc)) // to avoid movable double red cross...
									EERIEDrawBitmap(GDevice,(float)mx+16,(float)my

									,INTERFACE_RATIO_DWORD(tcc->m_dwWidth)
									,INTERFACE_RATIO_DWORD(tcc->m_dwHeight),0.00001f

									,tcc,D3DCOLORWHITE);
							}
						}
						
						if (tc2)
						{
							ARX_INTERFACE_HALO_Draw(DRAGINTER,GDevice,tc,tc2,mx,my, INTERFACE_RATIO(1), INTERFACE_RATIO(1));
						}
					}
					else 
					{
						if (CURCURPOS != 0)
						{
						
								ARX_CHECK_LONG( Original_framedelay );
								CURCURTIME	+=	ARX_CLEAN_WARN_CAST_LONG( Original_framedelay );


								while(CURCURTIME>CURCURDELAY)
								{
									CURCURTIME-=CURCURDELAY;
									CURCURPOS++;
								}
							}

							if (CURCURPOS>7) CURCURPOS=0;

							surf=scursor[CURCURPOS];
						
						
						if (surf) 
						{
							EERIEDrawBitmap(GDevice,(float)POSX,(float)POSY,
							                INTERFACE_RATIO_DWORD(surf->m_dwWidth), INTERFACE_RATIO_DWORD(surf->m_dwHeight), 0.f,
								surf,D3DCOLORWHITE);
						}
					}
				}
			}
		}
		else //mode system shock
		{
			if (SPECIAL_DRAGINTER_RENDER)
				return;

			if(FlyingOverIO||DRAGINTER)
			{
				fHighLightAng+=(float)(FrameDiff*0.5f);

				if(fHighLightAng>90.f) fHighLightAng=90.f;


				float fHLight	= 100.f*sin(DEG2RAD(fHighLightAng));
				ARX_CHECK_INT(fHLight);
				
				iHighLight	= ARX_CLEAN_WARN_CAST_INT(fHLight);

			}
			else
			{
				fHighLightAng=0.f;
				iHighLight = 0;
			}

			if (TRUE_PLAYER_MOUSELOOK_ON && pMenuConfig && pMenuConfig->bShowCrossHair)
			{
				if (!(player.Interface & INTER_COMBATMODE))
				{
					CURCURPOS=0;
					float POSX, POSY;
			
						surf = pTCCrossHair;

						if (!surf)
						{
							surf=ITC.target_off;
						}

						if (surf)
						{
							SETALPHABLEND(GDevice, true);
							SETBLENDMODE(GDevice,D3DBLEND_ONE,D3DBLEND_ONE);

							POSX = DANAESIZX*0.5f - INTERFACE_RATIO_DWORD(surf->m_dwWidth)*0.5f;
							POSY = DANAESIZY*0.5f - INTERFACE_RATIO_DWORD(surf->m_dwHeight)*0.5f;

							D3DCOLOR col=D3DRGB(0.5f, 0.5f, 0.5f);
							
							EERIEDrawBitmap(GDevice,(float)POSX,(float)POSY,

								INTERFACE_RATIO_DWORD(surf->m_dwWidth),
								INTERFACE_RATIO_DWORD(surf->m_dwHeight),

								0.f,
								surf, col);
							SETALPHABLEND(GDevice, false);
						}					
					}
				}
			}
		
		GDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_LINEAR );
		GDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_LINEAR );
		SETTEXTUREWRAPMODE(GDevice,D3DTADDRESS_WRAP);
	}
}

