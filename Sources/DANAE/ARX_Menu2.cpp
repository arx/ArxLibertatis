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
//-----------------------------------------------------------------------------
// Arx_Menu2.cpp
//-----------------------------------------------------------------------------
#include <windows.h>
#include <tchar.h>
#include "danae.h"
#include "arx_menu.h"
#include "arx_menu2.h"
#include "ARX_MenuPublic.h"
#include "ARX_Sound.h"
#include "ARX_Loc.h"
#include "ARX_Text.h"
#include "ARX_ViewImage.h"
#include "ARX_Interface.h"

#include "Mercury_dx_input.h"
#include "arx_time.h"

#include "eerietexture.h"
#include "eeriepoly.h"
#include "eeriedraw.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#define new new(_NORMAL_BLOCK,__FILE__, __LINE__)

extern char * GetVersionString();

#define NODEBUGZONE

//-----------------------------------------------------------------------------

#define RATIO_X(a)	(((float)a)*Xratio)
#define RATIO_Y(a)	(((float)a)*Yratio)

//-----------------------------------------------------------------------------

extern ARX_MENU_DATA ARXmenu;
extern TextureContainer * scursor[];
extern bool bGameNotFirstLaunch;
extern long DANAESIZX;
extern long DANAESIZY;

extern long LastEERIEMouseButton;

extern long save_c;
extern SaveGame *save_l;

extern bool bForceReInitAllTexture;
extern bool bALLOW_BUMP;
extern long WILL_RELOAD_ALL_TEXTURES;

extern long FINAL_RELEASE;

extern long GAME_EDITOR;
extern long ARX_DEMO;

extern long INTRO_NOT_LOADED;
extern long REFUSE_GAME_RETURN;

extern bool bGATI8500;
extern bool bForceNoEAX;

extern long _EERIEMouseXdep;
extern long _EERIEMouseYdep;

extern float PROGRESS_BAR_TOTAL;
extern float OLD_PROGRESS_BAR_COUNT;
extern float PROGRESS_BAR_COUNT;
extern float SOFTNEARCLIPPZ;

extern long GERMAN_VERSION;
extern long FRENCH_VERSION;

extern long INTERNATIONAL_MODE;

extern long CURRENT_GAME_INSTANCE;
extern char GameSavePath[];
void ARX_GAMESAVE_MakePath();

extern long GORE_MODE;

float INTERFACE_RATIO(float a);
bool bNoMenu=false;

void ARXMenu_Private_Options_Video_SetResolution(int _iWidth,int _iHeight,int _iBpp);
void ARX_SetAntiAliasing();
void ARX_MENU_LaunchAmb(char *_lpszAmb);

//-----------------------------------------------------------------------------

bool bGLOBAL_DINPUT_MENU=true;
bool bGLOBAL_DINPUT_GAME=true;

CDirectInput *pGetInfoDirectInput=NULL;
CMenuConfig *pMenuConfig;
static CWindowMenu *pWindowMenu=NULL;
CMenuState *pMenu;

CMenuElement *pMenuElementResume=NULL;
CMenuElement *pMenuElementApply=NULL;
CMenuElementText *pLoadConfirm=NULL;
CMenuSliderText *pMenuSliderResol=NULL;
CMenuSliderText *pMenuSliderBpp=NULL;
CMenuSliderText *pMenuSliderTexture=NULL;
CMenuCheckButton *pMenuCheckButtonBump=NULL;

float ARXTimeMenu;
float ARXOldTimeMenu;
float ARXDiffTimeMenu;
bool bForceGDI;

bool bFade=false;
bool bFadeInOut=false;
int iFadeAction=-1;
float fFadeInOut=0.f;

void ARX_MENU_Clicked_CREDITS();
void ARX_MENU_Clicked_NEWQUEST();
long ARX_CHANGELEVEL_Load(long);

TextureContainer *pTextureLoad=NULL;
static TextureContainer *pTextureLoadRender=NULL;

#define QUICK_SAVE_ID "ARX_QUICK_ARX"
#define QUICK_SAVE_ID1 "ARX_QUICK_ARX1"

int iTimeToDrawD7=-3000;

char pStringMod[256];
char pStringModSfx[256];
char pStringModSpeech[256];

//-----------------------------------------------------------------------------

bool isTimeBefore(SYSTEMTIME s1, SYSTEMTIME s2)
{
	if (s1.wYear < s2.wYear) return true;

	if (s1.wYear > s2.wYear) return false;

	if (s1.wYear == s2.wYear)
	{
		if (s1.wMonth < s2.wMonth) return true;

		if (s1.wMonth > s2.wMonth) return false;

		if (s1.wMonth == s2.wMonth)
		{
			if (s1.wDay < s2.wDay) return true;

			if (s1.wDay > s2.wDay) return false;

			if (s1.wDay == s2.wDay)
			{
				if (s1.wHour < s2.wHour) return true;

				if (s1.wHour > s2.wHour) return false;
				
				if (s1.wHour == s2.wHour)
				{
					if (s1.wMinute < s2.wMinute) return true;

					if (s1.wMinute > s2.wMinute) return false;

					if (s1.wMinute == s2.wMinute)
					{
						if (s1.wSecond < s2.wSecond) return true;

						if (s1.wSecond > s2.wSecond) return false;

						if (s1.wSecond == s2.wSecond)
						{
							if (s1.wMilliseconds < s2.wMilliseconds) return true;

							if (s1.wMilliseconds > s2.wMilliseconds) return false;

							if (s1.wMilliseconds == s2.wMilliseconds) return true;
						}
					}
				}
			}
		}
	}
	
	return false;
}

//-----------------------------------------------------------------------------

void ARX_QuickSave()
{
	if( REFUSE_GAME_RETURN ) return;

	FreeSaveGameList();
	CreateSaveGameList();

	_TCHAR szMenuText[256];
	_tcscpy( szMenuText, _T( QUICK_SAVE_ID ) );

	_TCHAR szMenuText1[256];
	_tcscpy( szMenuText1, _T( QUICK_SAVE_ID1 ) );

	int iOldGamma;

	ARXMenu_Options_Video_GetGamma( iOldGamma );
	ARXMenu_Options_Video_SetGamma( ( iOldGamma - 1 ) < 0 ? 0 : ( iOldGamma - 1 ) );

	ARX_SOUND_MixerPause( ARX_SOUND_MixerGame );

	bool		bFound0		=	false;
	bool		bFound1		=	false;

	int			iNbSave0	=	0; // will be used if >0 (0 will so mean NOTFOUND)
	int			iNbSave1	=	0; // will be used if >0 (0 will so mean NOTFOUND)
	SYSTEMTIME	sTime0;
	SYSTEMTIME	sTime1;
	ZeroMemory( &sTime0, sizeof(SYSTEMTIME) );// will be used if iNbSave0>0 (iNbSave0==0 will so mean NOTFOUND and sTime0 will not be used)
	ZeroMemory( &sTime1, sizeof(SYSTEMTIME) );// will be used if iNbSave0>0 (iNbSave1==0 will so mean NOTFOUND and sTime1 will not be used)


	for( int iI = 1 ; iI < (save_c) ; iI++ )
	{
		_TCHAR	tex2[256];
		_stprintf( tex2, _T( "%S"), save_l[iI].name );
		_tcsupr( tex2 );

		if( _tcsstr( szMenuText, tex2 ) )
		{
			bFound0		=	true;
			sTime0		=	save_l[iI].stime;
			iNbSave0	=	iI;
		}
		else if( _tcsstr( szMenuText1, tex2 ) )
		{
			bFound1		=	true;
			sTime1		=	save_l[iI].stime;
			iNbSave1	=	iI;
		}
	}

	if ( bFound0 && bFound1 &&
		( iNbSave0 > 0 ) && ( iNbSave0 < save_c ) &&
		( iNbSave1 > 0 ) && ( iNbSave1 < save_c ) )
	{
		int	iSave;
		
		if ( isTimeBefore( sTime0, sTime1 ) )
			iSave = iNbSave0;
		else
			iSave = iNbSave1;

		UpdateSaveGame( iSave );
		ARXMenu_Options_Video_SetGamma( iOldGamma );
		ARX_SOUND_MixerResume( ARX_SOUND_MixerGame );
		return;
	}

	char	tcSrc[256];
	char	tcDst[256];
	sprintf( tcSrc, "%sSCT_0.BMP", Project.workingdir );
	sprintf( tcDst, "%sSCT_1.BMP", Project.workingdir );
	CopyFile( tcSrc, tcDst, FALSE );

	if ( bFound0 == false )
	{
		strcpy( save_l[0].name, QUICK_SAVE_ID );
		UpdateSaveGame( 0 );
		ARXMenu_Options_Video_SetGamma( iOldGamma );
		ARX_SOUND_MixerResume( ARX_SOUND_MixerGame );

		CopyFile( tcDst, tcSrc, FALSE );
		DeleteFile( tcDst );
	}

	if ( bFound1 == false )
	{
		strcpy( save_l[0].name, QUICK_SAVE_ID1 );
		UpdateSaveGame( 0 );
		ARXMenu_Options_Video_SetGamma( iOldGamma );
		ARX_SOUND_MixerResume( ARX_SOUND_MixerGame );
	}

	DeleteFile( tcSrc );
	DeleteFile( tcDst );
}

//-----------------------------------------------------------------------------

void ARX_DrawAfterQuickLoad()
{

	ARX_CHECK_INT(iTimeToDrawD7 - FrameDiff);
	iTimeToDrawD7	-= ARX_CLEAN_WARN_CAST_INT(FrameDiff);

	
	float fColor;

	if(iTimeToDrawD7>0)
	{
		fColor=1.f;
	}
	else
	{
		int iFade=-iTimeToDrawD7;

		if(iFade>1000) return;

		fColor=1.f-(((float)iFade)/1000.f);
	}

	TextureContainer *pTex = MakeTCFromFile("\\Graph\\interface\\icons\\Menu_main_save.bmp");

	if(!pTex) return;

	GDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE);
	GDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,D3DBLEND_ONE);
	GDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,D3DBLEND_ONE);

	EERIEDrawBitmap2(	GDevice, 
						0, 
						0, 

						INTERFACE_RATIO_DWORD(pTex->m_dwWidth),
						INTERFACE_RATIO_DWORD(pTex->m_dwHeight),

						0.f, 
						pTex, 
						D3DRGB(fColor,fColor,fColor) );

	GDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,FALSE);
}

//-----------------------------------------------------------------------------

bool ARX_QuickLoad()
{
	FreeSaveGameList();
	CreateSaveGameList();

	_TCHAR szMenuText[256];
	_tcscpy( szMenuText, _T( QUICK_SAVE_ID ) );

	_TCHAR szMenuText1[256];
	_tcscpy( szMenuText1, _T( QUICK_SAVE_ID1 ) );

	bool bFound0 = false;
	bool bFound1 = false;

	int iNbSave0	=	0; // will be used if >0 (0 will so mean NOTFOUND)
	int iNbSave1	=	0; // will be used if >0 (0 will so mean NOTFOUND)
	SYSTEMTIME sTime0;
	SYSTEMTIME sTime1;
	ZeroMemory( &sTime0, sizeof(SYSTEMTIME) );// will be used if iNbSave0>0 (iNbSave0==0 will so mean NOTFOUND and sTime0 will not be used)
	ZeroMemory( &sTime1, sizeof(SYSTEMTIME) );// will be used if iNbSave0>0 (iNbSave1==0 will so mean NOTFOUND ans sTime1 will not be used)


	for( int iI = 1 ; iI < (save_c) ; iI++ )
	{
		_TCHAR	tex2[256];
		_stprintf( tex2, _T( "%S" ), save_l[iI].name );
		_tcsupr( tex2 );

		if( _tcsstr( szMenuText, tex2 ) )
		{
			bFound0		=	true;
			sTime0		=	save_l[iI].stime;
			iNbSave0	=	iI;
		}
		else if( _tcsstr( szMenuText1, tex2 ) )
		{
			bFound1		=	true;
			sTime1		=	save_l[iI].stime;
			iNbSave1	=	iI;
		}
	}

	ARX_SOUND_MixerPause( ARX_SOUND_MixerGame );

	if ( bFound0 && bFound1 &&
		( iNbSave0 > 0 ) && ( iNbSave0 < save_c ) &&
		( iNbSave1 > 0 ) && ( iNbSave1 < save_c ) )
	{
		int iSave;

		if ( isTimeBefore( sTime0, sTime1 ) )
			iSave = iNbSave1;
		else
			iSave = iNbSave0;

		INTRO_NOT_LOADED		=	1;
		LoadLevelScreen();
		PROGRESS_BAR_TOTAL		=	238;
		OLD_PROGRESS_BAR_COUNT	=	PROGRESS_BAR_COUNT=0;
		PROGRESS_BAR_COUNT		+=	1.f;
		LoadLevelScreen( GDevice, save_l[iSave].level );
		DanaeClearLevel();
		ARX_CHANGELEVEL_Load( save_l[iSave].num );
		REFUSE_GAME_RETURN		=	0;
		ARX_SOUND_MixerResume( ARX_SOUND_MixerGame );
		return true;
	}

	if ( bFound0 != false )
	{
		INTRO_NOT_LOADED		=	1;
		LoadLevelScreen();
		PROGRESS_BAR_TOTAL		=	238;
		OLD_PROGRESS_BAR_COUNT	=	PROGRESS_BAR_COUNT=0;
		PROGRESS_BAR_COUNT		+=	1.f;
		LoadLevelScreen( GDevice, save_l[iNbSave0].level );
		DanaeClearLevel();
		ARX_CHANGELEVEL_Load( save_l[iNbSave0].num );
		REFUSE_GAME_RETURN		=	0;
		ARX_SOUND_MixerResume( ARX_SOUND_MixerGame );
		return true;
	}

	if ( bFound1 != false )
	{
		INTRO_NOT_LOADED		=	1;
		LoadLevelScreen();
		PROGRESS_BAR_TOTAL		=	238;
		OLD_PROGRESS_BAR_COUNT	=	PROGRESS_BAR_COUNT=0;
		PROGRESS_BAR_COUNT		+=	1.f;
		LoadLevelScreen( GDevice, save_l[iNbSave1].level );
		DanaeClearLevel();
		ARX_CHANGELEVEL_Load( save_l[iNbSave1].num );
		REFUSE_GAME_RETURN		=	0;
		ARX_SOUND_MixerResume( ARX_SOUND_MixerGame );
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------

bool MENU_NoActiveWindow()
{
	if(	(!pWindowMenu)||
		((pWindowMenu)&&
		(pWindowMenu->eCurrentMenuState==MAIN)) ) return true;

	return false;
}

//-----------------------------------------------------------------------------

void GetTextSize(HFONT _hFont, _TCHAR *_lpszUText, int *_iWidth, int *_iHeight)
{
	HDC hDC;

	if (danaeApp.m_pddsRenderTarget)
    {
        if (SUCCEEDED( danaeApp.m_pddsRenderTarget->GetDC(&hDC)))
        {
			SelectObject(hDC, _hFont);

			SIZE sSize;

			GetTextExtentPoint32W(hDC, _lpszUText, _tcslen(_lpszUText),	&sSize);
			*_iWidth = sSize.cx;
			*_iHeight = sSize.cy;

            danaeApp.m_pddsRenderTarget->ReleaseDC(hDC);
		}
	}
}

//-----------------------------------------------------------------------------

void FontRenderText(HFONT _hFont, EERIE_3D pos, _TCHAR *_pText, COLORREF _c)
{
	if(pTextManage)
	{
		RECT rRect;


		ARX_CHECK_LONG( pos.y );
		ARX_CHECK_LONG( pos.x );
		ARX_CHECK_LONG( pos.x + 999 );
		ARX_CHECK_LONG( pos.y + 999 );
		//------------
		rRect.top	=	ARX_CLEAN_WARN_CAST_LONG( pos.y );
		rRect.left	=	ARX_CLEAN_WARN_CAST_LONG( pos.x );
		rRect.right	=	ARX_CLEAN_WARN_CAST_LONG( pos.x + 999 );
		rRect.bottom=	ARX_CLEAN_WARN_CAST_LONG( pos.y + 999 );


		ARX_TEXT pText;
		ARX_Text_Init(&pText);
		pText.lpszUText = _pText;
		pText.lCol = _c;
		pText.rRect = rRect;

		pTextManage->AddText(	_hFont,
								_pText,
								rRect,
								_c,
								0x00FF00FF);
	}
}

//-----------------------------------------------------------------------------

CMenuConfig::CMenuConfig()
{
	First();
}

//-----------------------------------------------------------------------------

void CMenuConfig::First()
{
	ResetActionKey();

	sakActionDefaultKey[CONTROLS_CUST_JUMP].iKey[0]=DIK_SPACE;
	sakActionDefaultKey[CONTROLS_CUST_JUMP].iKey[1]=-1;
	sakActionDefaultKey[CONTROLS_CUST_MAGICMODE].iKey[0]=DIK_LCONTROL;
	sakActionDefaultKey[CONTROLS_CUST_MAGICMODE].iKey[1]=DIK_RCONTROL;
	sakActionDefaultKey[CONTROLS_CUST_STEALTHMODE].iKey[0]=DIK_LSHIFT;
	sakActionDefaultKey[CONTROLS_CUST_STEALTHMODE].iKey[1]=DIK_RSHIFT;
	sakActionDefaultKey[CONTROLS_CUST_WALKFORWARD].iKey[0] = DIK_W;
	sakActionDefaultKey[CONTROLS_CUST_WALKFORWARD].iKey[1]=DIK_UP;
	sakActionDefaultKey[CONTROLS_CUST_WALKBACKWARD].iKey[0] = DIK_S;
	sakActionDefaultKey[CONTROLS_CUST_WALKBACKWARD].iKey[1]=DIK_DOWN;
	sakActionDefaultKey[CONTROLS_CUST_STRAFELEFT].iKey[0] = DIK_A;
	sakActionDefaultKey[CONTROLS_CUST_STRAFELEFT].iKey[1]=-1;
	sakActionDefaultKey[CONTROLS_CUST_STRAFERIGHT].iKey[0] = DIK_D;
	sakActionDefaultKey[CONTROLS_CUST_STRAFERIGHT].iKey[1]=-1;
	sakActionDefaultKey[CONTROLS_CUST_LEANLEFT].iKey[0] = DIK_Q;
	sakActionDefaultKey[CONTROLS_CUST_LEANLEFT].iKey[1]=-1;
	sakActionDefaultKey[CONTROLS_CUST_LEANRIGHT].iKey[0] = DIK_E;
	sakActionDefaultKey[CONTROLS_CUST_LEANRIGHT].iKey[1]=-1;
	sakActionDefaultKey[CONTROLS_CUST_CROUCH].iKey[0] = DIK_X;
	sakActionDefaultKey[CONTROLS_CUST_CROUCH].iKey[1]=-1;

	if (INTERNATIONAL_MODE)
	{
		sakActionDefaultKey[CONTROLS_CUST_MOUSELOOK].iKey[0]=DIK_F;
		sakActionDefaultKey[CONTROLS_CUST_MOUSELOOK].iKey[1]=DIK_RETURN;
	}
	else
	{
		sakActionDefaultKey[CONTROLS_CUST_MOUSELOOK].iKey[0]=DIK_BUTTON2;
		sakActionDefaultKey[CONTROLS_CUST_MOUSELOOK].iKey[1]=-1;
	}

	sakActionDefaultKey[CONTROLS_CUST_ACTION].iKey[0]=DIK_BUTTON1;
	sakActionDefaultKey[CONTROLS_CUST_ACTION].iKey[1]=-1;

	if (INTERNATIONAL_MODE)
	{
		sakActionDefaultKey[CONTROLS_CUST_INVENTORY].iKey[0] = DIK_I;
		sakActionDefaultKey[CONTROLS_CUST_INVENTORY].iKey[1]=-1;
	}
	else
	{
		sakActionDefaultKey[CONTROLS_CUST_INVENTORY].iKey[0] = DIK_I;
		sakActionDefaultKey[CONTROLS_CUST_INVENTORY].iKey[1]=-1;
	}

	sakActionDefaultKey[CONTROLS_CUST_INVENTORY].iPage=1;
	sakActionDefaultKey[CONTROLS_CUST_BOOK].iKey[0]=DIK_BACKSPACE;
	sakActionDefaultKey[CONTROLS_CUST_BOOK].iKey[1]=-1;
	sakActionDefaultKey[CONTROLS_CUST_BOOK].iPage=1;
sakActionDefaultKey[CONTROLS_CUST_BOOKCHARSHEET].iKey[0] = DIK_F1;
sakActionDefaultKey[CONTROLS_CUST_BOOKCHARSHEET].iPage=1;
sakActionDefaultKey[CONTROLS_CUST_BOOKSPELL].iKey[0] = DIK_F2;
sakActionDefaultKey[CONTROLS_CUST_BOOKSPELL].iPage=1;
sakActionDefaultKey[CONTROLS_CUST_BOOKMAP].iKey[0] = DIK_F3;
sakActionDefaultKey[CONTROLS_CUST_BOOKMAP].iPage=1;
sakActionDefaultKey[CONTROLS_CUST_BOOKQUEST].iKey[0] = DIK_F4;
sakActionDefaultKey[CONTROLS_CUST_BOOKQUEST].iPage=1;
	sakActionDefaultKey[CONTROLS_CUST_DRINKPOTIONLIFE].iKey[0] = DIK_H;
	sakActionDefaultKey[CONTROLS_CUST_DRINKPOTIONLIFE].iKey[1]=-1;
	sakActionDefaultKey[CONTROLS_CUST_DRINKPOTIONLIFE].iPage=1;
	sakActionDefaultKey[CONTROLS_CUST_DRINKPOTIONMANA].iKey[0] = DIK_G;
	sakActionDefaultKey[CONTROLS_CUST_DRINKPOTIONMANA].iKey[1]=-1;
	sakActionDefaultKey[CONTROLS_CUST_DRINKPOTIONMANA].iPage=1;
	sakActionDefaultKey[CONTROLS_CUST_TORCH].iKey[0]=DIK_T;
	sakActionDefaultKey[CONTROLS_CUST_TORCH].iKey[1]=-1;
	sakActionDefaultKey[CONTROLS_CUST_TORCH].iPage=1;

	sakActionDefaultKey[CONTROLS_CUST_CANCELCURSPELL].iKey[0] = DIK_4;
	sakActionDefaultKey[CONTROLS_CUST_CANCELCURSPELL].iKey[1] = -1;
	sakActionDefaultKey[CONTROLS_CUST_CANCELCURSPELL].iPage=1;

	sakActionDefaultKey[CONTROLS_CUST_PRECAST1].iKey[0] = DIK_1;
	sakActionDefaultKey[CONTROLS_CUST_PRECAST1].iPage=1;
	sakActionDefaultKey[CONTROLS_CUST_PRECAST2].iKey[0] = DIK_2;
	sakActionDefaultKey[CONTROLS_CUST_PRECAST2].iPage=1;
	sakActionDefaultKey[CONTROLS_CUST_PRECAST3].iKey[0] = DIK_3;
	sakActionDefaultKey[CONTROLS_CUST_PRECAST3].iPage=1;
	sakActionDefaultKey[CONTROLS_CUST_WEAPON].iKey[0] = DIK_TAB;
	sakActionDefaultKey[CONTROLS_CUST_WEAPON].iKey[1] = DIK_NUMPAD0;
	sakActionDefaultKey[CONTROLS_CUST_WEAPON].iPage=1;
	sakActionDefaultKey[CONTROLS_CUST_QUICKLOAD].iKey[0] = DIK_F9;
	sakActionDefaultKey[CONTROLS_CUST_QUICKLOAD].iPage=1;
	sakActionDefaultKey[CONTROLS_CUST_QUICKSAVE].iKey[0] = DIK_F5;
	sakActionDefaultKey[CONTROLS_CUST_QUICKSAVE].iPage=1;

	sakActionDefaultKey[CONTROLS_CUST_TURNLEFT].iKey[0]=DIK_LEFT;
	sakActionDefaultKey[CONTROLS_CUST_TURNLEFT].iKey[1]=-1;
	sakActionDefaultKey[CONTROLS_CUST_TURNRIGHT].iKey[0]=DIK_RIGHT;
	sakActionDefaultKey[CONTROLS_CUST_TURNRIGHT].iKey[1]=-1;
	sakActionDefaultKey[CONTROLS_CUST_LOOKUP].iKey[0]=DIK_PGUP;
	sakActionDefaultKey[CONTROLS_CUST_LOOKUP].iKey[1]=-1;
	sakActionDefaultKey[CONTROLS_CUST_LOOKDOWN].iKey[0]=DIK_PGDN;
	sakActionDefaultKey[CONTROLS_CUST_LOOKDOWN].iKey[1]=-1;

	sakActionDefaultKey[CONTROLS_CUST_STRAFE].iKey[0]=DIK_LALT;
	sakActionDefaultKey[CONTROLS_CUST_STRAFE].iKey[1]=-1;
	sakActionDefaultKey[CONTROLS_CUST_CENTERVIEW].iKey[0]=DIK_END;
	sakActionDefaultKey[CONTROLS_CUST_CENTERVIEW].iKey[1]=-1;

	if (INTERNATIONAL_MODE)
	{
		sakActionDefaultKey[CONTROLS_CUST_FREELOOK].iKey[0]=DIK_L;
		sakActionDefaultKey[CONTROLS_CUST_FREELOOK].iKey[1]=DIK_BUTTON2;
	}
	else
	{
		sakActionDefaultKey[CONTROLS_CUST_FREELOOK].iKey[0]=DIK_L;
		sakActionDefaultKey[CONTROLS_CUST_FREELOOK].iKey[1]=-1;
	}

	sakActionDefaultKey[CONTROLS_CUST_PREVIOUS].iKey[0] = GetDIKWithASCII(")");
	sakActionDefaultKey[CONTROLS_CUST_PREVIOUS].iPage=1;
	sakActionDefaultKey[CONTROLS_CUST_NEXT].iKey[0] = GetDIKWithASCII("=");
	sakActionDefaultKey[CONTROLS_CUST_NEXT].iPage=1;

	sakActionDefaultKey[CONTROLS_CUST_CROUCHTOGGLE].iKey[0] = GetDIKWithASCII("C");

	sakActionDefaultKey[CONTROLS_CUST_UNEQUIPWEAPON].iKey[0] = DIK_B;
	sakActionDefaultKey[CONTROLS_CUST_UNEQUIPWEAPON].iKey[1] = -1;
	sakActionDefaultKey[CONTROLS_CUST_UNEQUIPWEAPON].iPage=1;

	sakActionDefaultKey[CONTROLS_CUST_MINIMAP].iKey[0] = DIK_R;
	sakActionDefaultKey[CONTROLS_CUST_MINIMAP].iKey[1] = DIK_M;
	sakActionDefaultKey[CONTROLS_CUST_MINIMAP].iPage=1;

	bChangeResolution = false;
	bChangeTextures = false;
	bNoReturnToWindows=false;
	bLinkMouseLookToUse=false;

	bForceMetalTwoPass=false;
	bForceZBias=false;

	SetDefaultKey();
	DefaultValue();
}

//-----------------------------------------------------------------------------

CMenuConfig::CMenuConfig(char *_pName)
{
	if(!stricmp((const char*)"cfg",(const char*)_pName))
	{
		pcName=strdup("cfg.ini");
	}
	else
	{
		pcName=strdup(_pName);
	}

	First();
}

//-----------------------------------------------------------------------------

void CMenuConfig::DefaultValue()
{
	//VIDEO
	iWidth=640;
	iHeight=480;
	iNewWidth=iWidth;
	iNewHeight=iHeight;
	iBpp=16;
	iNewBpp=iBpp;
	bFullScreen=true;
	bBumpMapping=false;
	bNewBumpMapping=bBumpMapping;
	iTextureResol=2;
	iNewTextureResol=iTextureResol;
	iMeshReduction=0;
	iLevelOfDetails=2;
	iFogDistance=5;
	iLuminosite=4;
	iContrast=5;
	iGamma=5;
	bShowCrossHair=true;
	//AUDIO
	iMasterVolume=10;
	iSFXVolume=10;
	iSpeechVolume=10;
	iAmbianceVolume=8;
	bEAX=false;
	//INPUT
	bInvertMouse=false;
	bAutoReadyWeapon=false;
	bMouseLookToggle=false;
	bAutoDescription=true;
	iMouseSensitivity=4;
	bMouseSmoothing=false;
	//MISC
	INTERNATIONAL_MODE=1;
}

//-----------------------------------------------------------------------------

bool CDirectInput::GetMouseButtonDoubleClick(int _iNumButton,int _iTime)
{
	return ((iMouseTimeSet[_iNumButton]==2)&&(iMouseTime[_iNumButton]<_iTime)) ;
}

//-----------------------------------------------------------------------------

CMenuConfig::~CMenuConfig()
{
	if(pcName)
	{
		free((void*)pcName);
		pcName = NULL;
	}
}

//-----------------------------------------------------------------------------

void CMenuConfig::SetDefaultKey()
{
	int iI=MAX_ACTION_KEY;

	while(iI--)
	{
		sakActionKey[iI].iKey[0]=sakActionDefaultKey[iI].iKey[0];
		sakActionKey[iI].iKey[1]=sakActionDefaultKey[iI].iKey[1];
		sakActionKey[iI].iPage=sakActionDefaultKey[iI].iPage;
	}
	
	if (!INTERNATIONAL_MODE)
	{
		bLinkMouseLookToUse=true;
	}
	else
	{
		bLinkMouseLookToUse=false;
	}
}

//-----------------------------------------------------------------------------

int CMenuConfig::GetDIKWithASCII(char *_pcTouch)
{
_TCHAR pcT[256];

	MultiByteToWideChar(CP_ACP, 0, _pcTouch, -1, pcT, strlen(_pcTouch)+1);

	if(!_tcsicmp(pcT,_T("---")))
	{
		return -1;
	}

	for(int iI=0;iI<256;iI++)
	{
		_TCHAR *pcT1=pGetInfoDirectInput->GetFullNameTouch(iI);

		if(!_tcsicmp(pcT,pcT1))
		{
			free((void*)pcT1);
			return iI;
		}

		free((void*)pcT1);
		pcT1 = pGetInfoDirectInput->GetFullNameTouch(iI | (DIK_LSHIFT << 16));

		if(!_tcsicmp(pcT,pcT1))
		{
			free((void*)pcT1);
			return iI|(DIK_LSHIFT<<16);
		}

		free((void*)pcT1);
		pcT1=pGetInfoDirectInput->GetFullNameTouch(iI|(DIK_RSHIFT<<16));

		if(!_tcsicmp(pcT,pcT1))
		{
			free((void*)pcT1);
			return iI|(DIK_RSHIFT<<16);
		}

		free((void*)pcT1);
		pcT1=pGetInfoDirectInput->GetFullNameTouch(iI|(DIK_LCONTROL<<16));

		if(!_tcsicmp(pcT,pcT1))
		{
			free((void*)pcT1);
			return iI|(DIK_LCONTROL<<16);
		}

		free((void*)pcT1);
		pcT1=pGetInfoDirectInput->GetFullNameTouch(iI|(DIK_RCONTROL<<16));

		if(!_tcsicmp(pcT,pcT1))
		{
			free((void*)pcT1);
			return iI|(DIK_RCONTROL<<16);
		}

		free((void*)pcT1);
		pcT1=pGetInfoDirectInput->GetFullNameTouch(iI|(DIK_LALT<<16));

		if(!_tcsicmp(pcT,pcT1))
		{
			free((void*)pcT1);
			return iI|(DIK_LALT<<16);
		}

		free((void*)pcT1);
		pcT1=pGetInfoDirectInput->GetFullNameTouch(iI|(DIK_RALT<<16));

		if(!_tcsicmp(pcT,pcT1))
		{
			free((void*)pcT1);
			return iI|(DIK_RALT<<16);
		}

		free((void*)pcT1);
	}

	for(int iI=DIK_BUTTON1;iI<=DIK_BUTTON32;iI++)
	{
		_TCHAR *pcT1=pGetInfoDirectInput->GetFullNameTouch(iI);

		if(!_tcsicmp(pcT,pcT1))
		{
			free((void*)pcT1);
			return iI;
		}

		free((void*)pcT1);
	}

	for(int iI=DIK_WHEELUP;iI<=DIK_WHEELDOWN;iI++)
	{
		_TCHAR *pcT1=pGetInfoDirectInput->GetFullNameTouch(iI);

		if(!_tcsicmp(pcT,pcT1))
		{
			free((void*)pcT1);
			return iI;
		}

		free((void*)pcT1);
	}

	return -1;
}

//-----------------------------------------------------------------------------

char * CMenuConfig::ReadConfig(char *_pcSection,char *_pcKey)
{
char tcText[256];

	int iI=GetPrivateProfileString(_pcSection,_pcKey,"",tcText,256,pcName);

	if(iI<=0) return NULL;

	char *pcText=(char*)malloc(strlen(tcText)+1);

	if(!pcText) return NULL;

	strcpy(pcText,tcText);
	return pcText;
}

//-----------------------------------------------------------------------------

int CMenuConfig::ReadConfigInt(char *_pcSection,char *_pcKey,bool &_bOk)
{
	char *pcText=ReadConfig(_pcSection,_pcKey);

	if(!pcText)
	{
		_bOk=false;
		return 0;
	}

	int iI=atoi(pcText);
	free((void*)pcText);
	pcText=NULL;
	_bOk=true;
	return iI;
}

//-----------------------------------------------------------------------------
	
char* CMenuConfig::ReadConfigString(char *_pcSection,char *_pcKey)
{
	return ReadConfig(_pcSection,_pcKey);
}

//-----------------------------------------------------------------------------

bool CMenuConfig::WriteConfig(char *_pcSection,char *_pcKey,char *_pcDatas)
{
int iErreur=0;

	char tcText[256];	

	if(!GetPrivateProfileSection(_pcSection,tcText,256,pcName))
	{
		if(WritePrivateProfileSection(_pcSection,"",pcName)) iErreur++;
	}

	if(WritePrivateProfileString(_pcSection,_pcKey,_pcDatas,pcName)) iErreur++;

	return (iErreur==2);
}

//-----------------------------------------------------------------------------

bool CMenuConfig::WriteConfigInt(char *_pcSection,char *_pcKey,int _iDatas)
{
	char tcTxt[256];
	sprintf(tcTxt,"%d",_iDatas);
	return WriteConfig(_pcSection,_pcKey,(char*)tcTxt);
}

//-----------------------------------------------------------------------------

bool CMenuConfig::WriteConfigString(char *_pcSection,char *_pcKey,char *_pcDatas)
{
	return WriteConfig(_pcSection,_pcKey,_pcDatas);
}

//-----------------------------------------------------------------------------

void CMenuConfig::ResetActionKey()
{
	for (unsigned int iI=0; iI<MAX_ACTION_KEY; iI++)
	{
		sakActionKey[iI].iKey[0] = sakActionKey[iI].iKey[1]=-1;

		sakActionDefaultKey[iI].iKey[0] = sakActionDefaultKey[iI].iKey[1]=-1;

		sakActionKey[iI].iPage=0;
		sakActionDefaultKey[iI].iPage=0;
	}
}

//-----------------------------------------------------------------------------

bool CMenuConfig::SetActionKey(int _iAction,int _iActionNum,int _iVirtualKey)
{
	if(	(_iAction>=MAX_ACTION_KEY)||
		(_iActionNum>1) ) return false;

	bool bChange=false;
	bool bSecondChoice=false;
	int iOldVirtualKey=sakActionKey[_iAction].iKey[_iActionNum];
	sakActionKey[_iAction].iKey[_iActionNum]=_iVirtualKey;

	if(_iActionNum)
	{
		if(sakActionKey[_iAction].iKey[0]==-1)
		{
			sakActionKey[_iAction].iKey[0]=iOldVirtualKey;
			bSecondChoice=true;
		}

		if(sakActionKey[_iAction].iKey[0]==_iVirtualKey)
		{
			sakActionKey[_iAction].iKey[0]=-1;
		}

		bChange=true;
	}
	else
	{
		if(sakActionKey[_iAction].iKey[1]==-1)
		{
			sakActionKey[_iAction].iKey[1]=iOldVirtualKey;
			bSecondChoice=true;
		}

		if(sakActionKey[_iAction].iKey[1]==_iVirtualKey)
		{
			sakActionKey[_iAction].iKey[1]=-1;
		}

		bChange=true;
	}

	if(bSecondChoice)
	{
		bChange=true;
		iOldVirtualKey=-1;
	}

	//on remove les doublons de keys
	int iI=MAX_ACTION_KEY;

	while(iI--)
	{
		if(iI==_iAction) continue;

		if(sakActionKey[iI].iPage!=sakActionKey[_iAction].iPage) continue;

		if(sakActionKey[iI].iKey[0]==_iVirtualKey)
		{
			sakActionKey[iI].iKey[0]=iOldVirtualKey;
			bChange=true;
			break;
		}
		else
		{
			if(sakActionKey[iI].iKey[1]==_iVirtualKey)
			{
				sakActionKey[iI].iKey[1]=iOldVirtualKey;
				bChange=true;
				break;
			}
		}
	}

	return bChange;
}

//-----------------------------------------------------------------------------

bool CMenuConfig::WriteConfigKey(char *_pcKey,int _iAction)
{
char tcTxt[256];
char tcTxt2[256];
char *pcText;
bool bOk=true;
_TCHAR *pcText1;

	strcpy(tcTxt,_pcKey);

		int iL;
		pcText1=pGetInfoDirectInput->GetFullNameTouch(sakActionKey[_iAction].iKey[0]);
		iL=_tcslen(pcText1)+1;
		pcText=(char*)malloc(iL);

		while(iL--)
		{
			pcText[iL]=char(pcText1[iL]);
		}

		free((void*)pcText1);
		pcText1=NULL;
		
		if(pcText)
		{
			strcpy(tcTxt2,tcTxt);
			strcat(tcTxt2,"_k0");
			bOk&=WriteConfigString("KEY",tcTxt2,pcText);
			free((void*)pcText);
			pcText=NULL;
		}

		pcText1=pGetInfoDirectInput->GetFullNameTouch(sakActionKey[_iAction].iKey[1]);
		iL=_tcslen(pcText1)+1;
		pcText=(char*)malloc(iL);

		while(iL--)
		{
			pcText[iL]=char(pcText1[iL]);
		}

		free((void*)pcText1);
		pcText1=NULL;

		if(pcText)
		{
			strcpy(tcTxt2,tcTxt);
			strcat(tcTxt2,"_k1");
			bOk&=WriteConfigString("KEY",tcTxt2,pcText);
			free((void*)pcText);
			pcText=NULL;
		}

		
	

	return bOk;
}

//-----------------------------------------------------------------------------

void CMenuConfig::ReInitActionKey(CWindowMenuConsole *_pwmcWindowMenuConsole)
{
	int iID=BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP1;
	int iI=MAX_ACTION_KEY;
	bool bOldTouch=pGetInfoDirectInput->bTouch;
	int iOldVirtualKey=pGetInfoDirectInput->iKeyId;
	pGetInfoDirectInput->bTouch=true;

	while(iI--)
	{
		int iTab=(iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP1)>>1;

		CMenuZone *pmzMenuZone = _pwmcWindowMenuConsole->MenuAllZone.GetZoneWithID(iID);

		if (pmzMenuZone)
		{
			if(pmzMenuZone)
			{
				_pwmcWindowMenuConsole->pZoneClick=(CMenuElement*)pmzMenuZone;
				pGetInfoDirectInput->iKeyId=sakActionKey[iTab].iKey[0];
				_pwmcWindowMenuConsole->GetTouch();
			}

			pmzMenuZone=_pwmcWindowMenuConsole->MenuAllZone.GetZoneWithID(iID+1);

			if(pmzMenuZone)
			{
				_pwmcWindowMenuConsole->pZoneClick=(CMenuElement*)pmzMenuZone;
				pGetInfoDirectInput->iKeyId=sakActionKey[iTab].iKey[1];
				_pwmcWindowMenuConsole->GetTouch();
			}
		}

		iID+=2;
	}

	pGetInfoDirectInput->bTouch=bOldTouch;
	pGetInfoDirectInput->iKeyId=iOldVirtualKey;
}

//-----------------------------------------------------------------------------

bool CMenuConfig::ReadConfigKey(char *_pcKey,int _iAction)
{
char tcTxt[256];
char tcTxt2[256];
char *pcText;
bool bOk=true;
	strcpy(tcTxt, _pcKey);


		int iDIK;
		strcpy(tcTxt2,tcTxt);
		strcat(tcTxt2,"_k0");
		pcText=ReadConfigString("KEY",tcTxt2);

		if(!pcText)
		{
			bOk=false;
		}
		else
		{
			iDIK=GetDIKWithASCII(pcText);

			if(iDIK==-1)
			{
				sakActionKey[_iAction].iKey[0]=-1;
			}
			else
			{
				SetActionKey(_iAction,0,iDIK);
			}

			free((void*)pcText);
			pcText=NULL;
		}

		strcpy(tcTxt2,tcTxt);
		strcat(tcTxt2,"_k1");
		pcText=ReadConfigString("KEY",tcTxt2);

		if(!pcText)
		{
			bOk=false;
		}
		else
		{
			iDIK=GetDIKWithASCII(pcText);

			if(iDIK==-1)
			{
				sakActionKey[_iAction].iKey[1]=-1;
			}
			else
			{
				SetActionKey(_iAction,1,iDIK);
			}

			free((void*)pcText);
			pcText=NULL;
		}
		

	return bOk;
}

//-----------------------------------------------------------------------------

bool CMenuConfig::SaveAll()
{
	char tcTxt[256];
	bool bOk=true;

	//language
	strcpy(tcTxt,"\"");
	strcat(tcTxt,Project.localisationpath);
	strcat(tcTxt,"\"");
	bOk&=WriteConfigString("LANGUAGE","string",tcTxt);
	bOk&=WriteConfigInt("FIRSTRUN","int", bGameNotFirstLaunch?1:0);
	//video
	sprintf(tcTxt,"%dx%d",iWidth,iHeight);
	bOk&=WriteConfigString("VIDEO","resolution",tcTxt);
	bOk&=WriteConfigInt("VIDEO","bpp",iBpp);
	bOk&=WriteConfigInt("VIDEO","full_screen",(bFullScreen)?1:0);
	bOk&=WriteConfigInt("VIDEO","bump",(bBumpMapping)?1:0);
	bOk&=WriteConfigInt("VIDEO","texture",iTextureResol);
	bOk&=WriteConfigInt("VIDEO","mesh_reduction",iMeshReduction);
	bOk&=WriteConfigInt("VIDEO","others_details",iLevelOfDetails);
	bOk&=WriteConfigInt("VIDEO","fog",iFogDistance);
	bOk&=WriteConfigInt("VIDEO","gamma",iGamma);
	bOk&=WriteConfigInt("VIDEO","luminosity",iLuminosite);
	bOk&=WriteConfigInt("VIDEO","contrast",iContrast);
	bOk&=WriteConfigInt("VIDEO","show_crosshair",bShowCrossHair?1:0);
	bOk&=WriteConfigInt("VIDEO","antialiasing",bAntiAliasing?1:0);
	//audio 
	bOk&=WriteConfigInt("AUDIO","master_volume",iMasterVolume);
	bOk&=WriteConfigInt("AUDIO","effects_volume",iSFXVolume);
	bOk&=WriteConfigInt("AUDIO","speech_volume",iSpeechVolume);
	bOk&=WriteConfigInt("AUDIO","ambiance_volume",iAmbianceVolume);
	bOk&=WriteConfigInt("AUDIO","EAX",(bEAX)?1:0);
	//input
	bOk&=WriteConfigInt("INPUT","invert_mouse",(bInvertMouse)?1:0);
	bOk&=WriteConfigInt("INPUT","auto_ready_weapon",(bAutoReadyWeapon)?1:0);
	bOk&=WriteConfigInt("INPUT","mouse_look_toggle",(bMouseLookToggle)?1:0);
	bOk&=WriteConfigInt("INPUT","mouse_sensitivity",iMouseSensitivity);
	bOk&=WriteConfigInt("INPUT","mouse_smoothing",(bMouseSmoothing)?1:0);
	bOk&=WriteConfigInt("INPUT","auto_description",(bAutoDescription)?1:0);
	//key
	bOk&=WriteConfigKey("jump",CONTROLS_CUST_JUMP);
	bOk&=WriteConfigKey("magic_mode",CONTROLS_CUST_MAGICMODE);
	bOk&=WriteConfigKey("stealth_mode",CONTROLS_CUST_STEALTHMODE);
	bOk&=WriteConfigKey("walk_forward",CONTROLS_CUST_WALKFORWARD);
	bOk&=WriteConfigKey("walk_backward",CONTROLS_CUST_WALKBACKWARD);	
	bOk&=WriteConfigKey("strafe_left",CONTROLS_CUST_STRAFELEFT);
	bOk&=WriteConfigKey("strafe_right",CONTROLS_CUST_STRAFERIGHT);
	bOk&=WriteConfigKey("lean_left",CONTROLS_CUST_LEANLEFT);		
	bOk&=WriteConfigKey("lean_right",CONTROLS_CUST_LEANRIGHT);		
	bOk&=WriteConfigKey("crouch",CONTROLS_CUST_CROUCH);		
	bOk&=WriteConfigKey("mouselook",CONTROLS_CUST_MOUSELOOK);		
	bOk&=WriteConfigInt("INPUT","link_mouse_look_to_use",(bLinkMouseLookToUse)?1:0);
	bOk&=WriteConfigKey("action_combine",CONTROLS_CUST_ACTION);		
	bOk&=WriteConfigKey("inventory",CONTROLS_CUST_INVENTORY);	
	bOk&=WriteConfigKey("book",CONTROLS_CUST_BOOK);			
	bOk&=WriteConfigKey("char_sheet",CONTROLS_CUST_BOOKCHARSHEET);			
	bOk&=WriteConfigKey("magic_book",CONTROLS_CUST_BOOKSPELL);			
	bOk&=WriteConfigKey("map",CONTROLS_CUST_BOOKMAP);			
	bOk&=WriteConfigKey("quest_book",CONTROLS_CUST_BOOKQUEST);			
	bOk&=WriteConfigKey("drink_potion_life",CONTROLS_CUST_DRINKPOTIONLIFE);
	bOk&=WriteConfigKey("drink_potion_mana",CONTROLS_CUST_DRINKPOTIONMANA);
	bOk&=WriteConfigKey("torch",CONTROLS_CUST_TORCH);
	
	bOk&=WriteConfigKey("cancel_current_spell",CONTROLS_CUST_CANCELCURSPELL);
	bOk&=WriteConfigKey("precast_1",CONTROLS_CUST_PRECAST1);
	bOk&=WriteConfigKey("precast_2",CONTROLS_CUST_PRECAST2);
	bOk&=WriteConfigKey("precast_3",CONTROLS_CUST_PRECAST3);
	bOk&=WriteConfigKey("draw_weapon",CONTROLS_CUST_WEAPON);
	bOk&=WriteConfigKey("quicksave",CONTROLS_CUST_QUICKSAVE);
	bOk&=WriteConfigKey("quickload",CONTROLS_CUST_QUICKLOAD);

	bOk&=WriteConfigKey("turn_left",CONTROLS_CUST_TURNLEFT);
	bOk&=WriteConfigKey("turn_right",CONTROLS_CUST_TURNRIGHT);
	bOk&=WriteConfigKey("look_up",CONTROLS_CUST_LOOKUP);
	bOk&=WriteConfigKey("look_down",CONTROLS_CUST_LOOKDOWN);

	bOk&=WriteConfigKey("strafe",CONTROLS_CUST_STRAFE);
	bOk&=WriteConfigKey("center_view",CONTROLS_CUST_CENTERVIEW);

	bOk&=WriteConfigKey("freelook",CONTROLS_CUST_FREELOOK);

	bOk&=WriteConfigKey("previous",CONTROLS_CUST_PREVIOUS);
	bOk&=WriteConfigKey("next",CONTROLS_CUST_NEXT);

	bOk&=WriteConfigKey("crouch_toggle",CONTROLS_CUST_CROUCHTOGGLE);

	bOk&=WriteConfigKey("unequip_weapon",CONTROLS_CUST_UNEQUIPWEAPON);

	bOk&=WriteConfigKey("minimap",CONTROLS_CUST_MINIMAP);

	//misc
	bOk&=WriteConfigInt("MISC","softfog",(bATI)?1:0);
	bOk&=WriteConfigInt("MISC","clearnearcorrection",(bGATI8500)?1:0);
	bOk&=WriteConfigInt("MISC","forcesoftrender",(bDebugSetting)?1:0);
	bOk&=WriteConfigInt("MISC","forcenoeax",(bForceNoEAX)?1:0);
	bOk&=WriteConfigInt("MISC","forcegdi",(bForceGDI)?1:0);
	bOk&=WriteConfigInt("MISC","forcemetaltwopass",(bForceMetalTwoPass)?1:0);
	bOk&=WriteConfigInt("MISC","forcezbias",(bForceZBias)?1:0);
	bOk&=WriteConfigInt("MISC","newcontrol",(INTERNATIONAL_MODE)?1:0);
	bOk&=WriteConfigInt("MISC","forcetoggle",(bOneHanded)?1:0);
	bOk&=WriteConfigInt("MISC","fg",uiGoreMode);
	return bOk;
}

extern bool IsNoGore( void );

//-----------------------------------------------------------------------------

bool CMenuConfig::ReadAll()
{
	char	*pcText;
	bool	bOk=false;
	bool	bOkTemp;
	int		iTemp;

	//language
	if (strlen(Project.localisationpath) == 0)
	{
		if(GERMAN_VERSION)
		{
			pcText = strdup("Deutsch");
		}
		else
		{
			if(FRENCH_VERSION)
			{
				pcText=ReadConfigString("LANGUAGE","string");

				if( pcText&&
					(stricmp(pcText,"francais")&&
					stricmp(pcText,"deutsch")) )
				{
					free(pcText);
					pcText = strdup("Francais");
				}
				else
				{
					if(!pcText)
					{
						pcText = strdup("Francais");
					}
				}
			}
			else
			{
				pcText=ReadConfigString("LANGUAGE","string");
			}
		}

		if(pcText)
		{
			strcpy(Project.localisationpath,pcText);
			free((void*)pcText);
			pcText=NULL;
		}
	}

	bool bWarningGore=false;

	if (!stricmp(Project.localisationpath, "Deutsch"))
	{
		//no gore
		GERMAN_VERSION=1;
		uiGoreMode=0;
		GORE_MODE=0;
		bWarningGore=true;
	}

    ARX_Localisation_Init();

	bGameNotFirstLaunch = ReadConfigInt("FIRSTRUN","int",bOkTemp)?true:false;

	//video
	pcText=ReadConfigString("VIDEO","resolution");

	if(pcText)
	{
		char *pcTextCurr=pcText;
		int iI=strlen(pcText);

		while(iI--)
		{
			if(*pcTextCurr=='x')
			{
				*pcTextCurr=0;
				pcTextCurr++;
				bOk=true;
				break;
			}

			pcTextCurr++;
		}

		if(bOk)
		{
			iWidth = atoi(pcText);
			iHeight = atoi(pcTextCurr);
		}
		else
		{
			ARXMenu_Options_Video_GetResolution(iWidth,iHeight,iBpp);
		}

		free((void*)pcText);
		pcText=NULL;
	}
	else
	{
		ARXMenu_Options_Video_GetResolution(iWidth,iHeight,iBpp);
		bOk=false;
	}

	iNewWidth=iWidth;
	iNewHeight=iHeight;

	iTemp=ReadConfigInt("VIDEO","bpp",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		ARXMenu_Options_Video_GetBitPlane(iBpp);
		iTemp=iBpp;
	}

	iNewBpp=iBpp=iTemp;
	iTemp=ReadConfigInt("VIDEO","full_screen",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		ARXMenu_Options_Video_GetFullscreen(bFullScreen);
	}
	else
	{
		bFullScreen=(iTemp)?true:false;
	}

	iTemp=ReadConfigInt("VIDEO","bump",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		ARXMenu_Options_Video_GetBump(bBumpMapping);
		bNewBumpMapping=bBumpMapping;
	}
	else
	{
		bNewBumpMapping=bBumpMapping=(iTemp)?true:false;
	}

	bBumpMapping=bNewBumpMapping;

	if(bBumpMapping)
	{
		EERIE_ActivateBump();
	}
	else
	{
		EERIE_DesactivateBump();
	}

	bALLOW_BUMP=bBumpMapping;

	iTemp=ReadConfigInt("VIDEO","texture",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		ARXMenu_Options_Video_GetTextureQuality(iTextureResol);
		iTemp=iNewTextureResol=iTextureResol;
	}

	iTextureResol=iNewTextureResol=iTemp;

	if(iTextureResol==2) Project.TextureSize=0;

	if(iTextureResol==1) Project.TextureSize=2;

	if(iTextureResol==0) Project.TextureSize=64;

	WILL_RELOAD_ALL_TEXTURES=1;

	iTemp=ReadConfigInt("VIDEO","mesh_reduction",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		ARXMenu_Options_Video_GetLODQuality(iMeshReduction);
		iMeshReduction=iTemp;
	}

	iMeshReduction=iTemp;
	iTemp=ReadConfigInt("VIDEO","others_details",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		ARXMenu_Options_Video_GetDetailsQuality(iLevelOfDetails);
		iTemp=iLevelOfDetails;
	}

	iLevelOfDetails=iTemp;
	iTemp=ReadConfigInt("VIDEO","fog",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		ARXMenu_Options_Video_GetFogDistance(iFogDistance);
		iTemp=iFogDistance;
	}

	iFogDistance=iTemp;

	iTemp=ReadConfigInt("VIDEO","gamma",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		iTemp=5;
	}

	iGamma=iTemp;
	iTemp=ReadConfigInt("VIDEO","luminosity",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		iTemp=4;
	}

	iLuminosite=iTemp;

	if((iLuminosite<0)||(iLuminosite>10))
	{
		iLuminosite=4;
	}

	iTemp=ReadConfigInt("VIDEO","contrast",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		iTemp=5;
	}

	iContrast=iTemp;

	if((iContrast<0)||(iContrast>10))
	{
		iContrast=5;
	}

	iTemp=ReadConfigInt("VIDEO","show_crosshair",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		iTemp=1;
	}

	bShowCrossHair = iTemp?true:false;

	iTemp=ReadConfigInt("VIDEO","antialiasing",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		iTemp=0;
	}

	bAntiAliasing=iTemp?true:false;

	iTemp=ReadConfigInt("MISC","forcesoftrender",bOkTemp);
	if(!bOkTemp)
	{
		iTemp=0;
	}

	bDebugSetting=iTemp?true:false;

	//audio
	iTemp=ReadConfigInt("AUDIO","master_volume",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		iTemp=iMasterVolume;
	}

	iMasterVolume=iTemp;
	iTemp=ReadConfigInt("AUDIO","effects_volume",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		iTemp=iSFXVolume;
	}

	iSFXVolume=iTemp;
	iTemp=ReadConfigInt("AUDIO","speech_volume",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		iTemp=iSpeechVolume;
	}

	iSpeechVolume=iTemp;
	iTemp=ReadConfigInt("AUDIO","ambiance_volume",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		iTemp=iAmbianceVolume;
	}

	iAmbianceVolume=iTemp;
	iTemp=ReadConfigInt("AUDIO","EAX",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		ARXMenu_Options_Audio_GetEAX(bEAX);
	}
	else
	{
		bEAX=(iTemp)?true:false;
	}

	//input
	iTemp=ReadConfigInt("INPUT","invert_mouse",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		ARXMenu_Options_Control_GetInvertMouse(bInvertMouse);
	}
	else
	{
		bInvertMouse=(iTemp)?true:false;
	}
	
	iTemp=ReadConfigInt("INPUT","auto_ready_weapon",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		ARXMenu_Options_Control_GetAutoReadyWeapon(bAutoReadyWeapon);
	}
	else
	{
		bAutoReadyWeapon=(iTemp)?true:false;
	}

	iTemp=ReadConfigInt("INPUT","mouse_look_toggle",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		bMouseLookToggle=true;
	}
	else
	{
		bMouseLookToggle=(iTemp)?true:false;
	}

	iTemp=ReadConfigInt("INPUT","mouse_sensitivity",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		ARXMenu_Options_Control_GetMouseSensitivity(iMouseSensitivity);
		iTemp=iMouseSensitivity;
	}

	iMouseSensitivity=iTemp;

	iTemp=ReadConfigInt("INPUT","mouse_smoothing",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		bMouseSmoothing=false;
	}
	else
	{
		bMouseSmoothing=(iTemp)?true:false;
	}

	iTemp=ReadConfigInt("INPUT","auto_description",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		bAutoDescription=true;
	}
	else
	{
		bAutoDescription=(iTemp)?true:false;
	}

	//key
	bool bOk2=true;
	bOk2&=ReadConfigKey("jump",CONTROLS_CUST_JUMP);
	bOk2&=ReadConfigKey("magic_mode",CONTROLS_CUST_MAGICMODE);
	bOk2&=ReadConfigKey("stealth_mode",CONTROLS_CUST_STEALTHMODE);
	bOk2&=ReadConfigKey("walk_forward",CONTROLS_CUST_WALKFORWARD);
	bOk2&=ReadConfigKey("walk_backward",CONTROLS_CUST_WALKBACKWARD);	
	bOk2&=ReadConfigKey("strafe_left",CONTROLS_CUST_STRAFELEFT);
	bOk2&=ReadConfigKey("strafe_right",CONTROLS_CUST_STRAFERIGHT);
	bOk2&=ReadConfigKey("lean_left",CONTROLS_CUST_LEANLEFT);		
	bOk2&=ReadConfigKey("lean_right",CONTROLS_CUST_LEANRIGHT);		
	bOk2&=ReadConfigKey("crouch",CONTROLS_CUST_CROUCH);		
	bOk2&=ReadConfigKey("mouselook",CONTROLS_CUST_MOUSELOOK);		
	iTemp=ReadConfigInt("INPUT","link_mouse_look_to_use",bOkTemp);

	if(!bOkTemp)
	{
		bLinkMouseLookToUse=true;
	}
	else
	{
		bLinkMouseLookToUse=(iTemp)?true:false;
	}

	bOk2&=ReadConfigKey("action_combine",CONTROLS_CUST_ACTION);		
	bOk2&=ReadConfigKey("inventory",CONTROLS_CUST_INVENTORY);	
	bOk2&=ReadConfigKey("book",CONTROLS_CUST_BOOK);			
	bOk2&=ReadConfigKey("char_sheet",CONTROLS_CUST_BOOKCHARSHEET);			
	bOk2&=ReadConfigKey("magic_book",CONTROLS_CUST_BOOKSPELL);			
	bOk2&=ReadConfigKey("map",CONTROLS_CUST_BOOKMAP);			
	bOk2&=ReadConfigKey("quest_book",CONTROLS_CUST_BOOKQUEST);			
	bOk2&=ReadConfigKey("drink_potion_life",CONTROLS_CUST_DRINKPOTIONLIFE);
	bOk2&=ReadConfigKey("drink_potion_mana",CONTROLS_CUST_DRINKPOTIONMANA);
	bOk2&=ReadConfigKey("torch",CONTROLS_CUST_TORCH);

	bOk2&=ReadConfigKey("cancel_current_spell",CONTROLS_CUST_CANCELCURSPELL);
	bOk2&=ReadConfigKey("precast_1",CONTROLS_CUST_PRECAST1);
	bOk2&=ReadConfigKey("precast_2",CONTROLS_CUST_PRECAST2);
	bOk2&=ReadConfigKey("precast_3",CONTROLS_CUST_PRECAST3);
	bOk2&=ReadConfigKey("draw_weapon",CONTROLS_CUST_WEAPON);
	bOk2&=ReadConfigKey("quicksave",CONTROLS_CUST_QUICKSAVE);
	bOk2&=ReadConfigKey("quickload",CONTROLS_CUST_QUICKLOAD);
	bOk2&=ReadConfigKey("turn_left",CONTROLS_CUST_TURNLEFT);
	bOk2&=ReadConfigKey("turn_right",CONTROLS_CUST_TURNRIGHT);
	bOk2&=ReadConfigKey("look_up",CONTROLS_CUST_LOOKUP);
	bOk2&=ReadConfigKey("look_down",CONTROLS_CUST_LOOKDOWN);
	bOk2&=ReadConfigKey("strafe",CONTROLS_CUST_STRAFE);
	bOk2&=ReadConfigKey("center_view",CONTROLS_CUST_CENTERVIEW);

	bOk2&=ReadConfigKey("freelook",CONTROLS_CUST_FREELOOK);

	bOk2&=ReadConfigKey("previous",CONTROLS_CUST_PREVIOUS);
	bOk2&=ReadConfigKey("next",CONTROLS_CUST_NEXT);

	bOk2&=ReadConfigKey("crouch_toggle",CONTROLS_CUST_CROUCHTOGGLE);

	bOk2&=ReadConfigKey("unequip_weapon",CONTROLS_CUST_UNEQUIPWEAPON);

	bOk2&=ReadConfigKey("minimap",CONTROLS_CUST_MINIMAP);

	bOk2&=bOk;

	if(!bOk2)
	{
		int iI=MAX_ACTION_KEY;

		while(iI--)
		{
			sakActionKey[iI].iKey[0]=sakActionDefaultKey[iI].iKey[0];
			sakActionKey[iI].iKey[1]=sakActionDefaultKey[iI].iKey[1];
		}
	}

	//misc
	iTemp=ReadConfigInt("MISC","softfog",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		bATI=false;
	}
	else
	{
		bATI=(iTemp)?true:false;
	}

	iTemp=ReadConfigInt("MISC","clearnearcorrection",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		bGATI8500=false;
	}
	else
	{
		bGATI8500=(iTemp)?true:false;
	}


	if(bGATI8500)
	{
		SOFTNEARCLIPPZ	=	5.f;
	}

	iTemp=ReadConfigInt("MISC","forcenoeax",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		bForceNoEAX=false;
	}
	else
	{
		bForceNoEAX=(iTemp)?true:false;
	}

	iTemp=ReadConfigInt("MISC","forcegdi",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		bForceGDI=false;
	}
	else
	{
		bForceGDI=(iTemp)?true:false;
	}

	iTemp=ReadConfigInt("MISC","forcemetaltwopass",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		bForceMetalTwoPass=false;
	}
	else
	{
		bForceMetalTwoPass=(iTemp)?true:false;
	}

	iTemp=ReadConfigInt("MISC","forcezbias",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		bForceZBias=false;
	}
	else
	{
		bForceZBias=(iTemp)?true:false;
	}

	iTemp=ReadConfigInt("MISC","newcontrol",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		INTERNATIONAL_MODE=1;
	}
	else
	{
		INTERNATIONAL_MODE=(iTemp)?1:0;
	}

	if(INTERNATIONAL_MODE)
	{
		bLinkMouseLookToUse=false;
	}

	iTemp=ReadConfigInt("MISC","forcetoggle",bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		bOneHanded=false;
	}
	else
	{
		bOneHanded=(iTemp)?true:false;
	}

	char* pcTextMod=ReadConfigString("MISC","mod");

	if(	(pcTextMod)&&
		(strlen(pcTextMod)<256) )
	{
		strcpy(pStringMod,pcTextMod);
	}
	else
	{
		strcpy(pStringMod,"mod.pak");
	}

	free((void*)pcTextMod);

	pcTextMod=ReadConfigString("MISC","modsfx");

	if(	(pcTextMod)&&
		(strlen(pcTextMod)<256) )
	{
		strcpy(pStringModSfx,pcTextMod);
	}
	else
	{
		strcpy(pStringModSfx,"modsfx.pak");
	}

	free((void*)pcTextMod);

	pcTextMod=ReadConfigString("MISC","modspeech");

	if(	(pcTextMod)&&
		(strlen(pcTextMod)<256) )
	{
		strcpy(pStringModSpeech,pcTextMod);
	}
	else
	{
		strcpy(pStringModSpeech,"modspeech.pak");
	}

	free((void*)pcTextMod);

	uiGoreMode = ReadConfigInt("MISC", "fg", bOkTemp);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		uiGoreMode=bWarningGore?0:1;
	}

	switch(uiGoreMode)
	{
	case 0:
		{
			if(bWarningGore)
			{
				uiGoreMode=0;
				GORE_MODE=0;
			}
			else
			{
				uiGoreMode=1;
				GORE_MODE=1;
			}
		}
		break;
	case 1:
		{
			GORE_MODE=1;
		}
		break;
	case 2:
		{
			GORE_MODE=0;
		}
		break;
	default:
		{
			uiGoreMode=0;
			GORE_MODE=0;
		}
		break;
	}

	//on set les options

	ARXMenu_Options_Video_SetFogDistance(iFogDistance);
	ARXMenu_Options_Video_SetTextureQuality(iTextureResol);
	ARXMenu_Options_Video_SetBump(bBumpMapping);
	ARXMenu_Options_Video_SetLODQuality(iMeshReduction);
	ARXMenu_Options_Video_SetDetailsQuality(iLevelOfDetails);
	ARXMenu_Options_Video_SetGamma(iGamma);
	ARX_SetAntiAliasing();
	ARXMenu_Options_Video_SetSoftRender();
	ARXMenu_Options_Audio_SetMasterVolume(iMasterVolume);
	ARXMenu_Options_Audio_SetSfxVolume(iSFXVolume);
	ARXMenu_Options_Audio_SetSpeechVolume(iSpeechVolume);
	ARXMenu_Options_Audio_SetAmbianceVolume(iAmbianceVolume);

	pMenuConfig->bEAX=bEAX;
	ARXMenu_Options_Control_SetInvertMouse(bInvertMouse);
	ARXMenu_Options_Control_SetAutoReadyWeapon(bAutoReadyWeapon);
	ARXMenu_Options_Control_SetMouseLookToggleMode(bMouseLookToggle);
	ARXMenu_Options_Control_SetMouseSensitivity(iMouseSensitivity);
	ARXMenu_Options_Control_SetAutoDescription(bAutoDescription);

	if(pMenu)
	{
		pMenu->bReInitAll=true;
	}

	//mixer Game
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerGame, ARX_SOUND_MixerGetVolume(ARX_SOUND_MixerMenu));
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerGameSample, ARX_SOUND_MixerGetVolume(ARX_SOUND_MixerMenuSample));
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerGameSpeech, ARX_SOUND_MixerGetVolume(ARX_SOUND_MixerMenuSpeech));
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerGameAmbiance, ARX_SOUND_MixerGetVolume(ARX_SOUND_MixerMenuAmbiance));

	ARX_Localisation_Close();
	
	GORE_MODE = IsNoGore()? 0 : 1;
	return bOk;
}

//-----------------------------------------------------------------------------
static void CalculTextPosition( HDC& _hDC, wstring& phrase, CreditsTextInformations &infomations, float& drawpos )
{
	//Center the text on the screen
	GetTextExtentPoint32W(_hDC, phrase.c_str(), phrase.length(), &(infomations.sPos));

	if (infomations.sPos.cx < DANAESIZX) 
		infomations.sPos.cx = ARX_CLEAN_WARN_CAST_INT((DANAESIZX - infomations.sPos.cx) * DIV2);

	//Calcul height position (must be calculate after GetTextExtendPoint32 because sPos is writted)
	infomations.sPos.cy = ARX_CLEAN_WARN_CAST_INT(drawpos) ;
	drawpos += CreditsData.iFontAverageHeight ;
}

static void ExtractPhraseColor( wstring &phrase, CreditsTextInformations &infomations )
{
	//Get the good color
	if (phrase[0] == _T('~'))
	{
		phrase[0] = _T(' ');
		infomations.fColors = RGB(255,255,255);
	} 
	else //print in gold color
	{
		infomations.fColors = RGB(232,204,143);
	}
}

//Use to calculate an Average height for text fonts
static void CalculAverageWidth( HDC& _hDC )
{
		SelectObject(_hDC, hFontCredits);
		SIZE size;

		//calculate the average value
		GetTextExtentPoint32W(_hDC, _T("aA("),3, &size);
		CreditsData.iFontAverageHeight = size.cy;
}


//Use to extract string info from src buffer
static void ExtractAllCreditsTextInformations(HDC& _hDC)
{
	//Recupere les lignes à afficher
	wistringstream iss(ARXmenu.mda->str_cre_credits);
	wstring phrase;

	//Use to calculate the positions
	float drawpos	= ARX_CLEAN_WARN_CAST_FLOAT(DANAESIZY);
	bool firstLine = true ;

	while ( std::getline( iss, phrase) )
	{
		//Remove the first tild
		if (firstLine)
		{
			firstLine = false ;
			phrase[1] = ' ';
		}

		//Case of separator line
		if (phrase.length() == 0)
		{
			drawpos += CreditsData.iFontAverageHeight >> 3;
			continue ;
		}

		//Create a data containers
		CreditsTextInformations infomations ;

		ExtractPhraseColor(phrase, infomations);
		CalculTextPosition(_hDC, phrase, infomations, drawpos);

		//Assign the text modified by ExtractPhase Color
		infomations.sText = phrase;
	
		//Bufferize it
		CreditsData.aCreditsInformations.push_back(infomations);
	}
}

static void InitCredits( void )
{
	HDC hDC;

	if( SUCCEEDED( danaeApp.m_pddsRenderTarget->GetDC(&hDC) ) )
	{
		CalculAverageWidth(hDC);
		ExtractAllCreditsTextInformations(hDC);
		
		danaeApp.m_pddsRenderTarget->ReleaseDC(hDC);
	}
}


static void DrawCredits(void)
{
	int drawn = 0 ;

	//We initialize the datas
	if (CreditsData.iFontAverageHeight == -1)
	{
		InitCredits();
	}

	int iSize = CreditsData.aCreditsInformations.size() ;

	//We display them
	if (CreditsData.iFontAverageHeight != -1)
	{
		HDC hDC;
		COLORREF oldRef  = RGB(0,0,0);

		//Set the device
		if(!danaeApp.DANAEStartRender()) return;

		SETALPHABLEND(GDevice,FALSE);
		GDevice->SetRenderState( D3DRENDERSTATE_FOGENABLE, false);
		SETZWRITE(GDevice,true);
		GDevice->SetRenderState( D3DRENDERSTATE_ZENABLE,false);

		//Draw Background
		if(ARXmenu.mda->pTexCredits)
		{
			EERIEDrawBitmap2(GDevice, 0, 0, ARX_CLEAN_WARN_CAST_FLOAT(DANAESIZX), ARX_CLEAN_WARN_CAST_FLOAT(DANAESIZY + 1), .999f, ARXmenu.mda->pTexCredits, 0xFFFFFFFF);
		}	

		danaeApp.DANAEEndRender();

		//Use time passed between frame to create scroll effect
		//ARX_LOG("CreditStart %f, CreditGet ARX_TIME_Get %f  ", ARXmenu.mda->creditstart,ARX_TIME_Get( false ));
		ARXmenu.mda->creditspos-=0.025f*(float)(ARX_TIME_Get( false )-ARXmenu.mda->creditstart);
		ARXmenu.mda->creditstart=ARX_TIME_Get( false );
		
		if( SUCCEEDED( danaeApp.m_pddsRenderTarget->GetDC(&hDC) ) )
		{
			SetBkMode(hDC,TRANSPARENT);	
		

			std::vector<CreditsTextInformations>::const_iterator it = CreditsData.aCreditsInformations.begin() + CreditsData.iFirstLine ;

			for (;
				it != CreditsData.aCreditsInformations.end();
				++it)
			{
				//Update the Y word display
				float yy = it->sPos.cy + ARXmenu.mda->creditspos;

				//Display the text only if he is on the viewport
				if ((yy >= -CreditsData.iFontAverageHeight) && (yy <= DANAESIZY)) 
				{
					if (oldRef != it->fColors) //Little optimization
					{
						SetTextColor(hDC, it->fColors);
						oldRef = it->fColors;
					}

					SelectObject(hDC, hFontCredits);

					//Display the text on the screen
					TextOutW( hDC, 
						it->sPos.cx, 
						ARX_CLEAN_WARN_CAST_INT(yy), 
						it->sText.c_str(), 
						it->sText.length()	);

					++drawn;
				}
				
				if (yy <= -CreditsData.iFontAverageHeight)
				{
					++CreditsData.iFirstLine;
				}
				
				if ( yy >= DANAESIZY )
					break ; //it's useless to continue because next phrase will not be inside the viewport


			}
			danaeApp.m_pddsRenderTarget->ReleaseDC(hDC);
		}
	}



	if ( (iSize <= CreditsData.iFirstLine) && ( iFadeAction != AMCM_MAIN ) )
	{
		ARXmenu.mda->creditspos = 0;
		ARXmenu.mda->creditstart = 0 ;
		CreditsData.iFirstLine = 0 ;

		bFadeInOut = true;
		bFade = true;
		iFadeAction=AMCM_MAIN;

		ARX_MENU_LaunchAmb(AMB_MENU);
	}

	danaeApp.DANAEStartRender();

	if(ProcessFadeInOut(bFadeInOut,0.1f))
	{
		switch(iFadeAction)
		{
		case AMCM_MAIN:
			ARXmenu.currentmode=AMCM_MAIN;
			iFadeAction=-1;
			bFadeInOut=false;
			bFade=true;
			break;
		}
	}

	danaeApp.DANAEEndRender();

	SETZWRITE(GDevice,true);
	danaeApp.EnableZBuffer();	
}
//-----------------------------------------------------------------------------

void Check_Apply()
{
	if(pMenuElementApply)
	{
		if(	(pMenuConfig->bBumpMapping!=pMenuConfig->bNewBumpMapping)||
			(pMenuConfig->iTextureResol!=pMenuConfig->iNewTextureResol)||
			(pMenuConfig->iWidth!=pMenuConfig->iNewWidth)||
			(pMenuConfig->iHeight!=pMenuConfig->iNewHeight)||
			(pMenuConfig->iBpp!=pMenuConfig->iNewBpp) )
		{
			pMenuElementApply->SetCheckOn();
			((CMenuElementText*)pMenuElementApply)->lColor=((CMenuElementText*)pMenuElementApply)->lOldColor;
		}
		else
		{
			if(((CMenuElementText*)pMenuElementApply)->lColor!=RGB(127,127,127))
			{
				pMenuElementApply->SetCheckOff();
				((CMenuElementText*)pMenuElementApply)->lOldColor=((CMenuElementText*)pMenuElementApply)->lColor;
				((CMenuElementText*)pMenuElementApply)->lColor=RGB(127,127,127);
			}
		}
	}
}

//-----------------------------------------------------------------------------

static void FadeInOut(float _fVal)
{
D3DTLVERTEX d3dvertex[4];

	int iColor=D3DRGBA(_fVal,_fVal,_fVal,1.f);
	d3dvertex[0].sx=0;
	d3dvertex[0].sy=0;
	d3dvertex[0].sz=0.f;
	d3dvertex[0].rhw=0.999999f;
	d3dvertex[0].color=iColor;


	d3dvertex[1].sx=ARX_CLEAN_WARN_CAST_D3DVALUE(DANAESIZX);
	d3dvertex[1].sy=0;
	d3dvertex[1].sz=0.f;
	d3dvertex[1].rhw=0.999999f;
	d3dvertex[1].color=iColor;
	d3dvertex[2].sx=0;
	d3dvertex[2].sy=ARX_CLEAN_WARN_CAST_D3DVALUE(DANAESIZY);
	d3dvertex[2].sz=0.f;
	d3dvertex[2].rhw=0.999999f;
	d3dvertex[2].color=iColor;
	d3dvertex[3].sx=ARX_CLEAN_WARN_CAST_D3DVALUE(DANAESIZX);
	d3dvertex[3].sy=ARX_CLEAN_WARN_CAST_D3DVALUE(DANAESIZY);


	d3dvertex[3].sz=0.f;
	d3dvertex[3].rhw=0.999999f;
	d3dvertex[3].color=iColor;

	SETTC(GDevice,NULL);
	SETALPHABLEND(GDevice,TRUE);

	GDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO);
	GDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR);
	SETZWRITE(GDevice, false);
	GDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,false);
	GDevice->SetRenderState( D3DRENDERSTATE_CULLMODE,D3DCULL_NONE);

	EERIEDRAWPRIM( GDevice, D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX | D3DFVF_DIFFUSE, d3dvertex, 4, 0, EERIE_NOCOUNT );

	SETALPHABLEND(GDevice,FALSE);
	SETZWRITE(GDevice, true);

	danaeApp.EnableZBuffer();
	GDevice->SetRenderState( D3DRENDERSTATE_CULLMODE,D3DCULL_CCW);
}

//-----------------------------------------------------------------------------

bool ProcessFadeInOut(bool _bFadeIn,float _fspeed)
{
	FadeInOut(fFadeInOut);

	if(!bFade) return true;

	if(_bFadeIn)
	{
		fFadeInOut+=_fspeed*ARXDiffTimeMenu*DIV100;

		if(fFadeInOut>1.f)
		{
			fFadeInOut=1.f;
			bFade=false;
		}
	}
	else
	{
		fFadeInOut-=_fspeed*ARXDiffTimeMenu*DIV100;

		if(fFadeInOut<0.f)
		{
			fFadeInOut=0.f;
			bFade=false;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------

bool Menu2_Render()
{
	ARXOldTimeMenu = ARXTimeMenu;
	ARXTimeMenu = ARX_TIME_Get( false );
	ARXDiffTimeMenu = ARXTimeMenu-ARXOldTimeMenu;
	
	if (ARXDiffTimeMenu < 0) //this mean ArxTimeMenu is reseted 
		ARXDiffTimeMenu = 0 ;

	GDevice->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTFP_LINEAR);
	GDevice->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTFP_LINEAR);

	if ((AMCM_NEWQUEST==ARXmenu.currentmode)
		|| (AMCM_CREDITS==ARXmenu.currentmode)
		|| (AMCM_CDNOTFOUND==ARXmenu.currentmode))
	{
		if(pWindowMenu)
		{
			delete pWindowMenu;
			pWindowMenu=NULL;
		}

		if(pMenu)
		{
			delete pMenu;
			pMenu=NULL;
		}

		switch(ARXmenu.currentmode)
		{
		case AMCM_CREDITS:
			DrawCredits();
			return true;
			break;
		}

		return FALSE;
	}

	if(!danaeApp.DANAEStartRender())
	{
		return true;
	}

	if(pTextManage)
	{
		pTextManage->Clear();
	}

	GDevice->SetTextureStageState(0,D3DTSS_ADDRESS,D3DTADDRESS_CLAMP);

	GDevice->SetRenderState( D3DRENDERSTATE_FOGENABLE, false);
	SETZWRITE(GDevice, false);
	GDevice->SetRenderState( D3DRENDERSTATE_ZENABLE,false);
	GDevice->SetRenderState( D3DRENDERSTATE_CULLMODE,D3DCULL_NONE);
	
	MENUSTATE eOldMenuState=NOP;
	MENUSTATE eM;

	if(!pMenu)
	{
		eM=NOP;
	}
	else
	{
		eM=pMenu->eOldMenuWindowState;
	}

	long lColor = RGB(232, 204, 142);

	if(	(!pMenu)|| ((pMenu)&&(pMenu->bReInitAll)) )
	{
		_TCHAR szMenuText[256];
		bool bBOOL = false;
		CMenuElementText *me;

		if( (pMenu) && (pMenu->bReInitAll) )
		{
			eOldMenuState=pMenu->eOldMenuState;

			if(pWindowMenu)
			{
				delete pWindowMenu;
				pWindowMenu=NULL;
			}

			if(pMenu)
			{
				delete pMenu;
				pMenu=NULL;
			}
		}

		pMenu = new CMenuState(MAIN);
		pMenu->eOldMenuWindowState=eM;

		pMenu->pTexBackGround = MakeTCFromFile("Graph\\Interface\\menus\\menu_main_background.bmp");

		int iPosMenuPrincipaleX = 370;
int iPosMenuPrincipaleY=100;
int iDecMenuPrincipaleY=50;
#define MACRO_MENU_PRINCIPALE(MACRO_button,MACRO_menu,MACRO_locate,MACRO_check){\
		PAK_UNICODE_GetPrivateProfileString(_T(MACRO_locate), _T("string"), _T(""), szMenuText, 256, NULL);\
		me = new CMenuElementText(MACRO_button, hFontMainMenu, szMenuText, RATIO_X(iPosMenuPrincipaleX), RATIO_Y(iPosMenuPrincipaleY), lColor, 1.8f, MACRO_menu);\
		if(MACRO_check)\
		{\
			pMenuElementResume=me;\
			ARXMenu_GetResumeGame(bBOOL);\
			if (bBOOL)\
			{\
				me->SetCheckOn();\
			}\
			else\
			{\
				me->SetCheckOff();\
				me->lColor=RGB(127,127,127);\
			}\
		}\
		pMenu->AddMenuElement(me);\
		iPosMenuPrincipaleY+=iDecMenuPrincipaleY;\
}
		MACRO_MENU_PRINCIPALE(BUTTON_MENUMAIN_RESUMEGAME,RESUME_GAME,"system_menus_main_resumegame",1);
		MACRO_MENU_PRINCIPALE(BUTTON_MENUMAIN_NEWQUEST,NEW_QUEST,"system_menus_main_newquest",0);
		MACRO_MENU_PRINCIPALE(-1,EDIT_QUEST,"system_menus_main_editquest",0);
		MACRO_MENU_PRINCIPALE(BUTTON_MENUMAIN_OPTIONS,OPTIONS,"system_menus_main_options",0);
		MACRO_MENU_PRINCIPALE(BUTTON_MENUMAIN_CREDITS,CREDITS,"system_menus_main_credits",0);
		MACRO_MENU_PRINCIPALE(-1,QUIT,"system_menus_main_quit",0);
#undef MACRO_MENU_PRINCIPALE

		//version
		_TCHAR twVersion[32];
		MultiByteToWideChar(CP_ACP, 0, GetVersionString()+3, -1, twVersion, 32 );

		me = new CMenuElementText( -1, hFontControls, twVersion, RATIO_X(580), RATIO_Y(65), lColor, 1.0f, NOP );
		me->SetCheckOff();
		me->lColor=RGB(127,127,127);
		pMenu->AddMenuElement(me);
	}

	bool bScroll=true;
	{
		if(pMenuElementResume)
		{
			bool bTemp;
			ARXMenu_GetResumeGame(bTemp);

			if(bTemp)
			{
				pMenuElementResume->SetCheckOn();
				((CMenuElementText*)pMenuElementResume)->lColor=lColor;
			}
			else
			{
				pMenuElementResume->SetCheckOff();
				((CMenuElementText*)pMenuElementResume)->lColor=RGB(127,127,127);
			}
		}


		ARX_CHECK_INT(ARXDiffTimeMenu);
		MENUSTATE eMenuState = pMenu->Update(ARX_CLEAN_WARN_CAST_INT(ARXDiffTimeMenu));


		if(eOldMenuState!=NOP)
		{
			eMenuState=eOldMenuState;
			bScroll=false;
		}

		if( eMenuState == RESUME_GAME )
		{
			pTextManage->Clear();
			ARXmenu.currentmode = AMCM_OFF;
			pMenu->eMenuState = NOP;
			pMenu->pZoneClick = NULL;

			if(pWindowMenu)
			{
				delete pWindowMenu;
				pWindowMenu=NULL;
			}

			if(pMenu)
			{
				delete pMenu;
				pMenu=NULL;
			}

			SETALPHABLEND(GDevice,FALSE);
			GDevice->SetTextureStageState(0,D3DTSS_ADDRESS,D3DTADDRESS_WRAP);
			SETZWRITE(GDevice, true);
			danaeApp.EnableZBuffer();
			danaeApp.DANAEEndRender();

			return true;
		}
		else if (eMenuState!=NOP )
		{
			pMenu->eOldMenuState=eMenuState;

			if(pWindowMenu)
			{
				delete pWindowMenu;
				pWindowMenu=NULL;
			}

			//suivant la resolution
			int iWindowMenuWidth=(321);
			int iWindowMenuHeight=(430);
			int iWindowMenuPosX=(20);
			int iWindowMenuPosY=(480-iWindowMenuHeight)>>1;
			int	iWindowConsoleOffsetX=(0);
			int	iWindowConsoleOffsetY=(14-10);
			int	iWindowConsoleWidth=(iWindowMenuWidth-iWindowConsoleOffsetX);
			int	iWindowConsoleHeight=(iWindowMenuHeight-iWindowConsoleOffsetY+20);
			///////////////////////

			float fPosX1 = RATIO_X(20);
			float fPosX2 = RATIO_X(200);


			ARX_CHECK_INT(fPosX2);
			int iPosX2	= ARX_CLEAN_WARN_CAST_INT(fPosX2);


			float fPosBack	  = RATIO_X(10);
			float fPosBackY	  = RATIO_Y(190);
			float fPosNext	  = RATIO_X(140);

			float fPosApply   = RATIO_X(240);

			float fPosBDAY	  = RATIO_Y(380);

			pWindowMenu = new CWindowMenu(iWindowMenuPosX,iWindowMenuPosY,iWindowMenuWidth,iWindowMenuHeight,1);

			switch(eMenuState)
			{
			//------------------ START NEW_QUEST
			case NEW_QUEST:
				{
					_TCHAR szMenuText[256];
					bool bBOOL = false;
					ARXMenu_GetResumeGame(bBOOL);

					if (!bBOOL)
					{
						break;
					}

					CMenuElement *me = NULL;
					CWindowMenuConsole *pWindowMenuConsole=new CWindowMenuConsole(iWindowConsoleOffsetX,iWindowConsoleOffsetY,iWindowConsoleWidth,iWindowConsoleHeight,NEW_QUEST);
					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_main_editquest_confirm"), _T("string"), _T(""), szMenuText, 256, NULL);
					me=new CMenuElementText(-1, hFontMenu, szMenuText,0,0,lColor,1.f, NOP);
					me->bCheck = false;
					pWindowMenuConsole->AddMenuCenter(me);

					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_main_newquest_confirm"), _T("string"), _T(""), szMenuText, 256, NULL);
					me=new CMenuElementText(-1, hFontMenu, szMenuText,0,0,lColor,1.f, NOP);
					me->bCheck = false;
					pWindowMenuConsole->AddMenuCenter(me);

					CMenuPanel *pPanel = new CMenuPanel();
					PAK_UNICODE_GetPrivateProfileString(_T("system_yes"), _T("string"), _T(""), szMenuText, 256, NULL);
					_tcscat(szMenuText, _T("   "));
					me = new CMenuElementText(BUTTON_MENUNEWQUEST_CONFIRM, hFontMenu, szMenuText, 0, 0,lColor,1.f, NEW_QUEST_ENTER_GAME);
					me->SetPos(RATIO_X(iWindowConsoleWidth - (me->GetWidth() + 10)),0);
					pPanel->AddElementNoCenterIn(me);
					PAK_UNICODE_GetPrivateProfileString(_T("system_no"), _T("string"), _T(""), szMenuText, 256, NULL);
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosBack, 0,lColor,1.f, MAIN);
					me->SetShortCut(DIK_ESCAPE);
					pPanel->AddElementNoCenterIn(me);


					ARX_CHECK_INT(fPosBDAY);
					
					pPanel->Move(0,
								ARX_CLEAN_WARN_CAST_INT(fPosBDAY)	);

					
					pWindowMenuConsole->AddMenu(pPanel);
					pWindowMenu->AddConsole(pWindowMenuConsole);
					pWindowMenu->eCurrentMenuState=NEW_QUEST;

					}
				break;
			//------------------ END NEW_QUEST
			case EDIT_QUEST:
				{
					CMenuElement *me;
					CMenuElement *me01;
					CMenuPanel *pPanel;
					TextureContainer *pTex;
					_TCHAR szMenuText[256];
					CWindowMenuConsole *pWindowMenuConsole=new CWindowMenuConsole(iWindowConsoleOffsetX,iWindowConsoleOffsetY,iWindowConsoleWidth,iWindowConsoleHeight,EDIT_QUEST);

					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_main_editquest_load"), _T("string"), _T(""), szMenuText, 256, NULL);
					me = new CMenuElementText(BUTTON_MENUEDITQUEST_LOAD_INIT, hFontMenu, szMenuText, 0, 0, lColor, 1.f, EDIT_QUEST_LOAD);
					pWindowMenuConsole->AddMenuCenter(me);

					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_main_editquest_save"), _T("string"), _T(""), szMenuText, 256, NULL);
					me = new CMenuElementText(-1, hFontMenu, szMenuText, 0, 0, lColor, 1.f, EDIT_QUEST_SAVE);
					bool bBOOL;
					ARXMenu_GetResumeGame(bBOOL);

					if(!FINAL_RELEASE) bBOOL=true;

					if (!bBOOL)
					{
						me->SetCheckOff();
						((CMenuElementText*)me)->lColor=RGB(127,127,127);
					}

					pWindowMenuConsole->AddMenuCenter(me);

					pTex = MakeTCFromFile("\\Graph\\interface\\menus\\back.bmp");
					me = new CMenuCheckButton(-1, fPosBack, fPosBackY, pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
					me->eMenuState = MAIN;
					me->SetShortCut(DIK_ESCAPE);
					pWindowMenuConsole->AddMenu(me);

					pWindowMenu->eCurrentMenuState = EDIT_QUEST;
					pWindowMenu->AddConsole(pWindowMenuConsole);

					// LOAD ---------------------------------------------------
					pWindowMenuConsole=new CWindowMenuConsole(iWindowConsoleOffsetX,iWindowConsoleOffsetY-(40),iWindowConsoleWidth,iWindowConsoleHeight,EDIT_QUEST_LOAD);
					pWindowMenuConsole->iInterligne = 5;

					pTex = MakeTCFromFile("\\Graph\\interface\\icons\\Menu_main_Load.bmp");
					me = new CMenuCheckButton(-1, 0, 0, pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
					((CMenuCheckButton *)me)->bCheck = false;
					pWindowMenuConsole->AddMenuCenter(me);
					{
						//QUICK LOAD
						_TCHAR szMenuText[256];
						_tcscpy(szMenuText,_T(QUICK_SAVE_ID));
						_TCHAR szMenuText1[256];
						_tcscpy(szMenuText1,_T(QUICK_SAVE_ID1));

						//LOAD
						int iI;
						int iFirst=2;
						bool b1 = false;
						bool b2 = false;

						while(iFirst>=0)
						{
							for(iI=1; iI<(save_c); iI++)
							{
								_TCHAR tex[256];
								_stprintf(tex, _T("%S"),
									save_l[iI].name);
								
								CMenuElementText *me02;
								
								_TCHAR tex2[256];
								_tcscpy(tex2,tex);

								_tcsupr(tex2);

								if(!_tcscmp(szMenuText, tex2) || !_tcscmp(szMenuText1, tex2))
								{
									if(!iFirst || (b1 && b2)) continue;

									PAK_UNICODE_GetPrivateProfileString(_T("system_menus_main_quickloadsave"), _T("string"), _T("Quick"), tex, 256, NULL);

									if (_tcsstr(szMenuText, tex2))
									{
										if (b1) continue;

										b1 = true;
									}
									else if (_tcsstr(szMenuText1, tex2))
									{
										if (b2) continue;

										b2 = true;
									}

									char tex3[256];
									char tex4[256];
									strcpy(tex4,"  ");
									GetDateFormat(	LOCALE_SYSTEM_DEFAULT,
										0,
										&save_l[iI].stime,
										"MMM dd yyyy",
										tex3,
										256);
									strcat(tex4,tex3);
									GetTimeFormat(	LOCALE_SYSTEM_DEFAULT,
										0,
										&save_l[iI].stime,
										"   HH:mm",
										tex3,
										256);
									strcat(tex4,tex3);

									MultiByteToWideChar(	CP_ACP,
										0,
										tex4,
										-1,
										tex2,
										256);
									_tcscat(tex,tex2);
									
									me02 = new CMenuElementText(BUTTON_MENUEDITQUEST_LOAD, hFontControls, tex, fPosX1, 0.f, lColor, 0.8f, NOP);

									me02->lData=iI;
									pWindowMenuConsole->AddMenuCenterY((CMenuElementText*)me02);
									break;
								}
								else
								{
									if(iFirst) continue;

									char tex3[256];
									char tex4[256];
									strcpy(tex4,"  ");
									GetDateFormat(	LOCALE_SYSTEM_DEFAULT,
										0,
										&save_l[iI].stime,
										"MMM dd yyyy",
										tex3,
										256);
									strcat(tex4,tex3);
									GetTimeFormat(	LOCALE_SYSTEM_DEFAULT,
										0,
										&save_l[iI].stime,
										"   HH:mm",
										tex3,
										256);
									strcat(tex4,tex3);

									MultiByteToWideChar(	CP_ACP,
										0,
										tex4,
										-1,
										tex2,
										256);
									_tcscat(tex,tex2);
									
									me02=new CMenuElementText(BUTTON_MENUEDITQUEST_LOAD, hFontControls,tex, fPosX1,0.f,lColor, 0.8f, NOP);
								}

								me02->lData=iI;
								pWindowMenuConsole->AddMenuCenterY((CMenuElementText*)me02);
							}

							iFirst--;
						}

						me01 = new CMenuElementText(-1, hFontControls, _T(" "), fPosX1, 0.f, lColor, 0.8f, EDIT_QUEST_SAVE_CONFIRM);
							me01->SetCheckOff();
							pWindowMenuConsole->AddMenuCenterY((CMenuElementText*)me01);

						CMenuPanel *pc = new CMenuPanel();
						PAK_UNICODE_GetPrivateProfileString(_T("system_menus_main_editquest_load"), _T("string"), _T(""), szMenuText, 256, NULL);
						_tcscat(szMenuText, _T("   "));
						me = new CMenuElementText(BUTTON_MENUEDITQUEST_LOAD_CONFIRM, hFontMenu, szMenuText, 0, 0,lColor,1.f, MAIN);

						me->SetPos(RATIO_X(iWindowConsoleWidth-10)-me->GetWidth(), fPosBDAY + RATIO_Y(40));

						pLoadConfirm=(CMenuElementText*)me;
						me->SetCheckOff();
						((CMenuElementText*)me)->lOldColor=((CMenuElementText*)me)->lColor;
						((CMenuElementText*)me)->lColor=RGB(127,127,127);

						pWindowMenuConsole->AddMenu(me);
						pTex = MakeTCFromFile("\\Graph\\interface\\menus\\back.bmp");
						me = new CMenuCheckButton(-1, fPosBack, fPosBackY + RATIO_Y(20), pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
						me->eMenuState = EDIT_QUEST;
						me->SetShortCut(DIK_ESCAPE);
						pc->AddElementNoCenterIn(me);

						pWindowMenuConsole->AddMenu(pc);
					}
					pWindowMenu->AddConsole(pWindowMenuConsole);

					// SAVE----------------------------------------------------
					pWindowMenuConsole=new CWindowMenuConsole(iWindowConsoleOffsetX,iWindowConsoleOffsetY - (40), iWindowConsoleWidth,iWindowConsoleHeight,EDIT_QUEST_SAVE);
					pWindowMenuConsole->iInterligne = 5;

					pTex = MakeTCFromFile("\\Graph\\interface\\icons\\Menu_main_save.bmp");
					me = new CMenuCheckButton(-1, fPosBack-(pTex?(pTex->m_dwDeviceWidth-pTex->m_dwWidth):0), 0, pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
					((CMenuCheckButton *)me)->bCheck = false;
					pWindowMenuConsole->AddMenuCenter(me);

					//QUICK SAVE
					_TCHAR szMenuText1[256];
					_tcscpy(szMenuText,_T(QUICK_SAVE_ID));
					_tcscpy(szMenuText1,_T(QUICK_SAVE_ID1));

					//SAVE
					int iFirst=2;
					bool b1 = false;
					bool b2 = false;

					while(iFirst>=0)
					{
						if(save_c!=1)
						{
							for(int iI=1;iI<(save_c);iI++)
							{
								_TCHAR tex[256];

								_stprintf(tex, _T("%S"),
									save_l[iI].name);
								
								_TCHAR tex2[256];
								_tcscpy(tex2,tex);
								_tcsupr(tex2);

								if(!_tcscmp(szMenuText, tex2) || !_tcscmp(szMenuText1, tex2))
								{
									if(!iFirst || (b1 && b2)) continue;

									PAK_UNICODE_GetPrivateProfileString(_T("system_menus_main_quickloadsave"), _T("string"), _T("Quick"), tex, 256, NULL);

									if (_tcsstr(szMenuText, tex2))
									{
										if (b1) continue;

										b1 = true;
									}
									else if (_tcsstr(szMenuText1, tex2))
									{
										if (b2) continue;

										b2 = true;
									}									

									char tex3[256];
									char tex4[256];
									strcpy(tex4,"  ");
									GetDateFormat(	LOCALE_SYSTEM_DEFAULT,
										0,
										&save_l[iI].stime,
										"MMM dd yyyy",
										tex3,
										256);
									strcat(tex4,tex3);
									GetTimeFormat(	LOCALE_SYSTEM_DEFAULT,
										0,
										&save_l[iI].stime,
										"   HH:mm",
										tex3,
										256);
									strcat(tex4,tex3);

									MultiByteToWideChar(	CP_ACP,
										0,
										tex4,
										-1,
										tex2,
										256);
									_tcscat(tex,tex2);
									
									me = new CMenuElementText(BUTTON_MENUEDITQUEST_SAVEINFO, hFontControls, tex, fPosX1, 0.f, RGB(127, 127, 127), 0.8f, EDIT_QUEST_SAVE_CONFIRM);
									me->SetCheckOff();

									me->lData=iI;
									pWindowMenuConsole->AddMenuCenterY(me);
									break;
								}
								else
								{
									if(iFirst) continue;

									char tex3[256];
									char tex4[256];
									strcpy(tex4,"  ");
									GetDateFormat(	LOCALE_SYSTEM_DEFAULT,
										0,
										&save_l[iI].stime,
										"MMM dd yyyy",
										tex3,
										256);
									strcat(tex4,tex3);
									GetTimeFormat(	LOCALE_SYSTEM_DEFAULT,
										0,
										&save_l[iI].stime,
										"   HH:mm",
										tex3,
										256);
									strcat(tex4,tex3);

									MultiByteToWideChar(	CP_ACP,
										0,
										tex4,
										-1,
										tex2,
										256);
									_tcscat(tex,tex2);
									
									me = new CMenuElementText(BUTTON_MENUEDITQUEST_SAVEINFO, hFontControls, tex, fPosX1, 0.f, lColor, 0.8f, EDIT_QUEST_SAVE_CONFIRM);
								}

								me->lData=iI;
								pWindowMenuConsole->AddMenuCenterY(me);
							}
						}

						iFirst--;
					}

					pTex = MakeTCFromFile("\\Graph\\interface\\Icons\\Arx_logo_08.bmp");

					for(int iI=save_c; iI<=15; iI++)
					{
						_TCHAR tex[256];
						_stprintf(tex, _T("-%04d-")

							,iI);
						CMenuElementText * me01 = new CMenuElementText(-1, hFontControls, tex, fPosX1, 0.f, lColor, 0.8f, EDIT_QUEST_SAVE_CONFIRM);

						me01->eMenuState=EDIT_QUEST_SAVE_CONFIRM;
						me01->lData=0;
						pWindowMenuConsole->AddMenuCenterY((CMenuElementText*)me01);
					}

					me01 = new CMenuElementText(-1, hFontControls, _T(" "), fPosX1, 0.f, lColor, 0.8f, EDIT_QUEST_SAVE_CONFIRM);
					me01->SetCheckOff();
					pWindowMenuConsole->AddMenuCenterY((CMenuElementText*)me01);

					pTex = MakeTCFromFile("\\Graph\\interface\\menus\\back.bmp");
					me = new CMenuCheckButton(-1, fPosBack, fPosBackY + RATIO_Y(20), pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);

					me->eMenuState = EDIT_QUEST;
					me->SetShortCut(DIK_ESCAPE);
					pWindowMenuConsole->AddMenu(me);

					pWindowMenu->AddConsole(pWindowMenuConsole);

					// SAVE CONFIRM--------------------------------------------
					pWindowMenuConsole = new CWindowMenuConsole(iWindowConsoleOffsetX,iWindowConsoleOffsetY,iWindowConsoleWidth,iWindowConsoleHeight,EDIT_QUEST_SAVE_CONFIRM);

					pTex = MakeTCFromFile("\\Graph\\interface\\icons\\Menu_main_save.bmp");
					me = new CMenuCheckButton(-1, 0, 0, pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
					((CMenuCheckButton *)me)->bCheck = false;
					pWindowMenuConsole->AddMenuCenter(me);
					
					PAK_UNICODE_GetPrivateProfileString(_T("system_menu_editquest_newsavegame"), _T("string"), _T("---"), szMenuText, 256, NULL);

					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, NOP);
					me->lData=0;

					pWindowMenuConsole->AddMenuCenter(me);
					me->eState=EDIT;
					me->ePlace=CENTER;
					
					pPanel = new CMenuPanel();

					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_main_editquest_save"), _T("string"), _T(""), szMenuText, 256, NULL);

					me = new CMenuElementText(BUTTON_MENUEDITQUEST_SAVE, hFontMenu, szMenuText, 0, 0,lColor,1.f, MAIN);

					me->SetPos(RATIO_X(iWindowConsoleWidth-10)-me->GetWidth(), fPosBDAY);
					pPanel->AddElementNoCenterIn(me);

					pTex = MakeTCFromFile("\\Graph\\interface\\menus\\back.bmp");
					me = new CMenuCheckButton(-1, fPosBack, fPosBackY, pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
					me->eMenuState = EDIT_QUEST_SAVE;
					me->SetShortCut(DIK_ESCAPE);
					pPanel->AddElementNoCenterIn(me);

					pWindowMenuConsole->AddMenu(pPanel);
					
					pWindowMenu->AddConsole(pWindowMenuConsole);
					}
				break;
			//------------------ END SAVE_QUEST
			case MULTIPLAYER:
				{
				}
				break;
			//------------------ START OPTIONS
			case OPTIONS_INPUT:
				{
					MessageBox(0, "", "", 0);

				}
				break;
			case OPTIONS:
				{
					_TCHAR szMenuText[256];
					CMenuElement *me;
					CMenuPanel *pc;
					TextureContainer *pTex;

					CWindowMenuConsole *pWindowMenuConsole=new CWindowMenuConsole(iWindowConsoleOffsetX,iWindowConsoleOffsetY,iWindowConsoleWidth,iWindowConsoleHeight,OPTIONS);

					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_video"), _T("string"), _T(""), szMenuText, 256, NULL);
					me = new CMenuElementText(BUTTON_MENUOPTIONSVIDEO_INIT, hFontMenu, szMenuText, 0, 0,lColor,1.f,OPTIONS_VIDEO);
					pWindowMenuConsole->AddMenuCenter(me);
					
					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_audio"), _T("string"), _T(""), szMenuText, 256, NULL);
					me = new CMenuElementText(-1, hFontMenu, szMenuText, 0, 0,lColor,1.f,OPTIONS_AUDIO);
					pWindowMenuConsole->AddMenuCenter(me);
					
					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input"), _T("string"), _T(""), szMenuText, 256, NULL);
					me = new CMenuElementText(-1, hFontMenu, szMenuText, 0, 0,lColor,1.f,OPTIONS_INPUT);
					pWindowMenuConsole->AddMenuCenter(me);
					
					pTex = MakeTCFromFile("\\Graph\\interface\\menus\\back.bmp");
					me = new CMenuCheckButton(-1, fPosBack, fPosBackY, pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
					me->eMenuState = MAIN;
					me->SetShortCut(DIK_ESCAPE);
					pWindowMenuConsole->AddMenu(me);
				
					pWindowMenu->AddConsole(pWindowMenuConsole);
				//------------------ END OPTIONS
					
				//------------------ START VIDEO
					pWindowMenuConsole=new CWindowMenuConsole(iWindowConsoleOffsetX,iWindowConsoleOffsetY - (40),iWindowConsoleWidth,iWindowConsoleHeight, OPTIONS_VIDEO);

					pc = new CMenuPanel();
					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_video_resolution"), _T("string"), _T(""), szMenuText, 256, NULL);
					_tcscat(szMenuText, _T("  "));
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, NOP);
					me->SetCheckOff();
					pc->AddElement(me);
					int iOffsetX = iPosX2;
					int iModeX,iModeY,iModeBpp;
					ARXMenu_Options_Video_GetResolution(iModeX,iModeY,iModeBpp);
					me = new CMenuSliderText(BUTTON_MENUOPTIONSVIDEO_RESOLUTION, 0, 0);
					pMenuSliderResol =(CMenuSliderText*)me;
					int nb=danaeApp.m_pDeviceInfo->dwNumModes;
					vector<int> vBpp;
					vBpp.clear();
					int i=0;

					for(;i<nb;i++)
					{
						{
							_stprintf(szMenuText,_T("%dx%d"),danaeApp.m_pDeviceInfo->pddsdModes[i].dwWidth,danaeApp.m_pDeviceInfo->pddsdModes[i].dwHeight);
	

							ARX_CHECK_NOT_NEG( iModeBpp );

							if( danaeApp.m_pDeviceInfo->pddsdModes[i].ddpfPixelFormat.dwRGBBitCount == ARX_CAST_UINT( iModeBpp ) )
							{
								((CMenuSliderText *)me)->AddText(new CMenuElementText(-1, hFontMenu, szMenuText, 0, 0,lColor,1.f, (MENUSTATE)(OPTIONS_VIDEO_RESOLUTION_0+i)));

								ARX_CHECK_NOT_NEG( iModeX );
								ARX_CHECK_NOT_NEG( iModeY );

								if( ( danaeApp.m_pDeviceInfo->pddsdModes[i].dwWidth == ARX_CAST_UINT( iModeX ) ) &&
									( danaeApp.m_pDeviceInfo->pddsdModes[i].dwHeight == ARX_CAST_UINT( iModeY ) ) )
								{

									((CMenuSliderText*)me)->iPos = ((CMenuSliderText *)me)->vText.size()-1;
									danaeApp.m_pDeviceInfo->ddsdFullscreenMode=danaeApp.m_pDeviceInfo->pddsdModes[i];
									danaeApp.m_pDeviceInfo->dwCurrentMode=i;
								}
							}
							
							//bpp
							bool bExist=false;
							vector<int>::iterator ii;

							for(ii=vBpp.begin();ii!=vBpp.end();ii++)
							{
								if (ARX_CAST_UINT(*ii) == danaeApp.m_pDeviceInfo->pddsdModes[i].ddpfPixelFormat.dwRGBBitCount)
								{
									bExist=true;
									break;
								}
							}

							if(!bExist)
							{
								vBpp.insert(vBpp.end(),danaeApp.m_pDeviceInfo->pddsdModes[i].ddpfPixelFormat.dwRGBBitCount);
							}
						}
					}


					float fRatio	= (RATIO_X(iWindowConsoleWidth-9) - me->GetWidth()); 
					ARX_CHECK_INT(fRatio);

					me->Move(	ARX_CLEAN_WARN_CAST_INT(fRatio)	,0); 


					pc->AddElement(me);

					pWindowMenuConsole->AddMenuCenterY(pc);

					CMenuPanel *pc1 = new CMenuPanel();
					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_video_texture"), _T("string"), _T(""), szMenuText, 256, NULL);
					_tcscat(szMenuText, _T(" "));
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, NOP);
					me->SetCheckOff();
					pc1->AddElement(me);
					iOffsetX = iPosX2;
					me = new CMenuSliderText(BUTTON_MENUOPTIONSVIDEO_TEXTURES, 0, 0);
					pMenuSliderTexture = (CMenuSliderText*)me;
					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_video_texture_low"), _T("string"), _T(""), szMenuText, 256, NULL);
					((CMenuSliderText *)me)->AddText(new CMenuElementText(-1, hFontMenu, szMenuText, 0, 0,lColor,1.f, OPTIONS_VIDEO));
					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_video_texture_med"), _T("string"), _T(""), szMenuText, 256, NULL);
					((CMenuSliderText *)me)->AddText(new CMenuElementText(-1, hFontMenu, szMenuText, 0, 0,lColor,1.f, OPTIONS_VIDEO));
					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_video_texture_high"), _T("string"), _T(""), szMenuText, 256, NULL);
					((CMenuSliderText *)me)->AddText(new CMenuElementText(-1, hFontMenu, szMenuText, 0, 0,lColor,1.f, OPTIONS_VIDEO));


					fRatio	= (RATIO_X(iWindowConsoleWidth-9) - me->GetWidth()); 
					ARX_CHECK_INT(fRatio);

					me->Move(	ARX_CLEAN_WARN_CAST_INT(fRatio)	,0); 


					int iSize = me->GetWidth();
					pc1->AddElement(me);
					int iQuality = 0;
					ARXMenu_Options_Video_GetTextureQuality(iQuality);
					((CMenuSliderText *)me)->iPos = iQuality;

					pc = new CMenuPanel();
					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_video_bpp"), _T("string"), _T(""), szMenuText, 256, NULL);
					_tcscat(szMenuText, _T(" "));
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, NOP);
					me->SetCheckOff();
					pc->AddElement(me);
					me = new CMenuSliderText(BUTTON_MENUOPTIONSVIDEO_BPP, 0, 0);
					pMenuSliderBpp = (CMenuSliderText*)me;

					vector<int>::iterator ii;

					for(ii=vBpp.begin();ii!=vBpp.end();ii++)
					{
						_itot(*ii,szMenuText,10);
						((CMenuSliderText*)me)->AddText(new CMenuElementText(-1, hFontMenu, szMenuText, 0, 0, lColor, 1.f, (MENUSTATE)(BUTTON_MENUOPTIONSVIDEO_BPP+i)));

						if(*ii==iModeBpp)
						{
							((CMenuSliderText*)me)->iPos = ((CMenuSliderText*)me)->vText.size()-1;
						}
					}

					((CMenuSliderText *)me)->SetWidth(iSize);


					fRatio	= (RATIO_X(iWindowConsoleWidth-9) - me->GetWidth()); 
					ARX_CHECK_INT(fRatio);

					me->Move(	ARX_CLEAN_WARN_CAST_INT(fRatio)	,0); 


					pc->AddElement(me);
					pWindowMenuConsole->AddMenuCenterY(pc);

					pWindowMenuConsole->AddMenuCenterY(pc1);
					pc = new CMenuPanel();
					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_detail"), _T("string"), _T(""), szMenuText, 256, NULL);
					_tcscat(szMenuText, _T(" "));
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, NOP);
					me->SetCheckOff();
					pc->AddElement(me);
					iOffsetX = iPosX2;
					me = new CMenuSliderText(BUTTON_MENUOPTIONSVIDEO_OTHERSDETAILS, 0, 0);
					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_video_texture_low"), _T("string"), _T(""), szMenuText, 256, NULL);
					((CMenuSliderText *)me)->AddText(new CMenuElementText(-1, hFontMenu, szMenuText, 0, 0,lColor,1.f, OPTIONS_OTHERDETAILS));
					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_video_texture_med"), _T("string"), _T(""), szMenuText, 256, NULL);
					((CMenuSliderText *)me)->AddText(new CMenuElementText(-1, hFontMenu, szMenuText, 0, 0,lColor,1.f, OPTIONS_OTHERDETAILS));
					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_video_texture_high"), _T("string"), _T(""), szMenuText, 256, NULL);
					((CMenuSliderText *)me)->AddText(new CMenuElementText(-1, hFontMenu, szMenuText, 0, 0,lColor,1.f, OPTIONS_OTHERDETAILS));

					
					fRatio	= (RATIO_X(iWindowConsoleWidth-9) - me->GetWidth()); 
					ARX_CHECK_INT(fRatio);

					me->Move(	ARX_CLEAN_WARN_CAST_INT(fRatio)	,0); 


					pc->AddElement(me);
					iQuality = 0;
					ARXMenu_Options_Video_GetDetailsQuality(iQuality);
					((CMenuSliderText *)me)->iPos = iQuality;

					pWindowMenuConsole->AddMenuCenterY(pc);

					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_video_bump"), _T("string"), _T(""), szMenuText, 256, NULL);
					_tcscat(szMenuText, _T(" "));
					TextureContainer *pTex1 = MakeTCFromFile("\\Graph\\interface\\menus\\menu_checkbox_off.bmp");
					TextureContainer *pTex2 = MakeTCFromFile("\\Graph\\interface\\menus\\menu_checkbox_on.bmp");
					CMenuElementText * metemp = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, NOP);
					metemp->SetCheckOff();
					me = new CMenuCheckButton(BUTTON_MENUOPTIONSVIDEO_BUMP, 0, 0, pTex1->m_dwWidth, pTex1, pTex2, metemp);
					pMenuCheckButtonBump=(CMenuCheckButton*)me;
					bool bBOOL = false;
					ARXMenu_Options_Video_GetBump(bBOOL);

					if (bBOOL)
					{
						((CMenuCheckButton*)me)->iState=1;
					}
					else
					{
						((CMenuCheckButton*)me)->iState=0;
					}

					pWindowMenuConsole->AddMenuCenterY(me);

					pc = new CMenuPanel();
					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_video_brouillard"), _T("string"), _T(""), szMenuText, 256, NULL);
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, NOP);
					me->SetCheckOff();
					pc->AddElement(me);
					me = new CMenuSlider(BUTTON_MENUOPTIONSVIDEO_FOG, iPosX2, 0);
					int iFog = 5;
					ARXMenu_Options_Video_GetFogDistance(iFog);
					((CMenuSlider *)me)->iPos = iFog;
					pc->AddElement(me);

					pWindowMenuConsole->AddMenuCenterY(pc);

					pc = new CMenuPanel();
					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_video_gamma"), _T("string"), _T(""), szMenuText, 256, NULL);
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, NOP);
					me->SetCheckOff();
					pc->AddElement(me);
					me = new CMenuSlider(BUTTON_MENUOPTIONSVIDEO_GAMMA, iPosX2, 0);
					int iGamma = 0;
					ARXMenu_Options_Video_GetGamma(iGamma);
					((CMenuSlider*)me)->iPos = iGamma;
					pc->AddElement(me);
					pWindowMenuConsole->AddMenuCenterY(pc);

					pc = new CMenuPanel();
					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_video_luminosity"), _T("string"), _T("luminosity"), szMenuText, 256, NULL);
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, NOP);
					me->SetCheckOff();
					pc->AddElement(me);
					me = new CMenuSlider(BUTTON_MENUOPTIONSVIDEO_LUMINOSITY, iPosX2, 0);
					int iLum = 0;
					ARXMenu_Options_Video_GetLuminosity(iLum);
					((CMenuSlider*)me)->iPos = iLum;
					pc->AddElement(me);
					pWindowMenuConsole->AddMenuCenterY(pc);

					pc = new CMenuPanel();
					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_video_contrast"), _T("string"), _T("contrast"), szMenuText, 256, NULL);
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, NOP);
					me->SetCheckOff();
					pc->AddElement(me);
					me = new CMenuSlider(BUTTON_MENUOPTIONSVIDEO_CONTRAST, iPosX2, 0);
					int iContrast = 0;
					ARXMenu_Options_Video_GetContrast(iContrast);
					((CMenuSlider*)me)->iPos = iContrast;
					pc->AddElement(me);
					pWindowMenuConsole->AddMenuCenterY(pc);

					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_video_crosshair"), _T("string"), _T("Show Crosshair"), szMenuText, 256, NULL);
					_tcscat(szMenuText, _T(" "));
					pTex1 = MakeTCFromFile("\\Graph\\interface\\menus\\menu_checkbox_off.bmp");
					pTex2 = MakeTCFromFile("\\Graph\\interface\\menus\\menu_checkbox_on.bmp");
					metemp = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, NOP);
					metemp->SetCheckOff();
					me = new CMenuCheckButton(BUTTON_MENUOPTIONSVIDEO_CROSSHAIR, 0, 0, pTex1->m_dwWidth, pTex1, pTex2, metemp);

					if (pMenuConfig&&pMenuConfig->bShowCrossHair)
					{
						((CMenuCheckButton*)me)->iState=1;
					}
					else
					{
						((CMenuCheckButton*)me)->iState=0;
					}

					pWindowMenuConsole->AddMenuCenterY(me);

					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_video_antialiasing"), _T("string"), _T("antialiasing"), szMenuText, 256, NULL);
					_tcscat(szMenuText, _T(" "));
					pTex1 = MakeTCFromFile("\\Graph\\interface\\menus\\menu_checkbox_off.bmp");
					pTex2 = MakeTCFromFile("\\Graph\\interface\\menus\\menu_checkbox_on.bmp");
					metemp = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, NOP);
					metemp->SetCheckOff();
					me = new CMenuCheckButton(BUTTON_MENUOPTIONSVIDEO_ANTIALIASING, 0, 0, pTex1->m_dwWidth, pTex1, pTex2, metemp);

					if (pMenuConfig&&pMenuConfig->bAntiAliasing)
					{
						((CMenuCheckButton*)me)->iState=1;
					}
					else
					{
						((CMenuCheckButton*)me)->iState=0;
					}

					pWindowMenuConsole->AddMenuCenterY(me);
					ARX_SetAntiAliasing();

					metemp = new CMenuElementText(-1, hFontMenu, _T("Enable Rendering Fix"), fPosX1, 0.f, lColor, 1.f, NOP);
					metemp->SetCheckOff();
					me = new CMenuCheckButton(BUTTON_MENUOPTIONSVIDEO_DEBUGSETTING, 0, 0, pTex1->m_dwWidth, pTex1, pTex2, metemp);

 					((CMenuCheckButton*)me)->iState=ARXMenu_Options_Video_SetSoftRender();

					pWindowMenuConsole->AddMenuCenterY(me);

					pc = new CMenuPanel();
					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_video_apply"), _T("string"), _T(""), szMenuText, 256, NULL);
					_tcscat(szMenuText, _T("   "));
					pMenuElementApply = me = new CMenuElementText(BUTTON_MENUOPTIONSVIDEO_APPLY, hFontMenu, szMenuText, fPosApply, 0.f, lColor, 1.f, NOP);
					me->SetPos(RATIO_X(iWindowConsoleWidth-10)-me->GetWidth(), fPosBDAY + RATIO_Y(40));
					me->SetCheckOff();
					pc->AddElementNoCenterIn(me);

					pTex = MakeTCFromFile("\\Graph\\interface\\menus\\back.bmp");
					me = new CMenuCheckButton(BUTTON_MENUOPTIONSVIDEO_BACK, fPosBack, fPosBackY + RATIO_Y(20), pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
					me->eMenuState = OPTIONS;
					me->SetShortCut(DIK_ESCAPE);
					pc->AddElementNoCenterIn(me);

					pWindowMenuConsole->AddMenu(pc);

					pWindowMenu->AddConsole(pWindowMenuConsole);
					//------------------ END VIDEO
					
					//------------------ START AUDIO
					pWindowMenuConsole = new CWindowMenuConsole(iWindowConsoleOffsetX,iWindowConsoleOffsetY,iWindowConsoleWidth,iWindowConsoleHeight,OPTIONS_AUDIO);
					
					pc = new CMenuPanel();
					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_audio_master_volume"), _T("string"), _T(""), szMenuText, 256, NULL);
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, OPTIONS_AUDIO_VOLUME);
					me->SetCheckOff();
					pc->AddElement(me);
					me = new CMenuSlider(BUTTON_MENUOPTIONSAUDIO_MASTER, iPosX2, 0);
					int iMaster = 0;
					ARXMenu_Options_Audio_GetMasterVolume(iMaster);
					((CMenuSlider *)me)->iPos = iMaster;
					pc->AddElement(me);
					pWindowMenuConsole->AddMenuCenterY(pc);
					
					pc = new CMenuPanel();
					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_audio_effects_volume"), _T("string"), _T(""), szMenuText, 256, NULL);
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, OPTIONS_AUDIO);
					me->SetCheckOff();
					pc->AddElement(me);
					me = new CMenuSlider(BUTTON_MENUOPTIONSAUDIO_SFX, iPosX2, 0);
					int iSfx = 0;
					ARXMenu_Options_Audio_GetSfxVolume(iSfx);
					((CMenuSlider *)me)->iPos = iSfx;
					pc->AddElement(me);
					pWindowMenuConsole->AddMenuCenterY(pc);
					
					pc = new CMenuPanel();
					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_audio_speech_volume"), _T("string"), _T(""), szMenuText, 256, NULL);
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, OPTIONS_AUDIO);
					me->SetCheckOff();
					pc->AddElement(me);
					me = new CMenuSlider(BUTTON_MENUOPTIONSAUDIO_SPEECH, iPosX2, 0);
					int iSpeech = 0;
					ARXMenu_Options_Audio_GetSpeechVolume(iSpeech);
					((CMenuSlider *)me)->iPos = iSpeech;
					pc->AddElement(me);
					pWindowMenuConsole->AddMenuCenterY(pc);

					pc = new CMenuPanel();
					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_audio_ambiance_volume"), _T("string"), _T(""), szMenuText, 256, NULL);
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, OPTIONS_AUDIO);
					me->SetCheckOff();
					pc->AddElement(me);
					me = new CMenuSlider(BUTTON_MENUOPTIONSAUDIO_AMBIANCE, iPosX2, 0);
					int iAmbiance = 0;
					ARXMenu_Options_Audio_GetAmbianceVolume(iAmbiance);
					((CMenuSlider *)me)->iPos = iAmbiance;
					pc->AddElement(me);
					pWindowMenuConsole->AddMenuCenterY(pc);

					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_audio_eax"), _T("string"), _T("EAX"), szMenuText, 256, NULL);
					_tcscat(szMenuText, _T(" "));
					pTex1 = MakeTCFromFile("\\Graph\\interface\\menus\\menu_checkbox_off.bmp");
					pTex2 = MakeTCFromFile("\\Graph\\interface\\menus\\menu_checkbox_on.bmp");
					CMenuElementText * pElementText = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, OPTIONS_INPUT);
					me = new CMenuCheckButton(BUTTON_MENUOPTIONSAUDIO_EAX, 0, 0, pTex1->m_dwWidth, pTex1, pTex2, pElementText);
					bool bEAX = true;

					if (bEAX)
					{
						((CMenuCheckButton*)me)->iState=pMenuConfig->bEAX?1:0;
					}
					else
					{
						me->SetCheckOff();
						pElementText->lColor=RGB(127,127,127);
					}

					pWindowMenuConsole->AddMenuCenterY(me);
					
					pTex = MakeTCFromFile("\\Graph\\interface\\menus\\back.bmp");
					me = new CMenuCheckButton(-1, fPosBack, fPosBackY, pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
					me->eMenuState = OPTIONS;
					me->SetShortCut(DIK_ESCAPE);
					pWindowMenuConsole->AddMenu(me);

					pWindowMenu->AddConsole(pWindowMenuConsole);
					//------------------ END AUDIO
					
					//------------------ START INPUT
					pWindowMenuConsole = new CWindowMenuConsole(iWindowConsoleOffsetX,iWindowConsoleOffsetY,iWindowConsoleWidth,iWindowConsoleHeight, OPTIONS_INPUT);
					
					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls"), _T("string"), _T(""), szMenuText, 256, NULL);
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, OPTIONS_INPUT_CUSTOMIZE_KEYS_1);
					pWindowMenuConsole->AddMenuCenterY(me);
					
					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_invert_mouse"), _T("string"), _T(""), szMenuText, 256, NULL);
					_tcscat(szMenuText, _T(" "));
					pTex1 = MakeTCFromFile("\\Graph\\interface\\menus\\menu_checkbox_off.bmp");
					pTex2 = MakeTCFromFile("\\Graph\\interface\\menus\\menu_checkbox_on.bmp");
					me = new CMenuCheckButton(BUTTON_MENUOPTIONS_CONTROLS_INVERTMOUSE, 0, 0, pTex1->m_dwWidth, pTex1, pTex2, new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, OPTIONS_INPUT));
					bBOOL = false;
					ARXMenu_Options_Control_GetInvertMouse(bBOOL);

					if (bBOOL)
					{
						((CMenuCheckButton*)me)->iState=1;
					}
					else
					{
						((CMenuCheckButton*)me)->iState=0;
					}

					pWindowMenuConsole->AddMenuCenterY(me);

					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_auto_ready_weapon"), _T("string"), _T(""), szMenuText, 256, NULL);
					_tcscat(szMenuText, _T(" "));
					pTex1 = MakeTCFromFile("\\Graph\\interface\\menus\\menu_checkbox_off.bmp");
					pTex2 = MakeTCFromFile("\\Graph\\interface\\menus\\menu_checkbox_on.bmp");
					me = new CMenuCheckButton(BUTTON_MENUOPTIONS_CONTROLS_AUTOREADYWEAPON, 0, 0, pTex1->m_dwWidth, pTex1, pTex2, new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, OPTIONS_INPUT));
					bBOOL = false;
					ARXMenu_Options_Control_GetAutoReadyWeapon(bBOOL);

					if (bBOOL)
					{
						((CMenuCheckButton*)me)->iState=1;
					}
					else
					{
						((CMenuCheckButton*)me)->iState=0;
					}

					pWindowMenuConsole->AddMenuCenterY(me);

					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_mouse_look_toggle"), _T("string"), _T(""), szMenuText, 256, NULL);
					_tcscat(szMenuText, _T(" "));
					pTex1 = MakeTCFromFile("\\Graph\\interface\\menus\\menu_checkbox_off.bmp");
					pTex2 = MakeTCFromFile("\\Graph\\interface\\menus\\menu_checkbox_on.bmp");
					me = new CMenuCheckButton(BUTTON_MENUOPTIONS_CONTROLS_MOUSELOOK, 0, 0, pTex1->m_dwWidth, pTex1, pTex2, new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, OPTIONS_INPUT));
					bBOOL = false;
					ARXMenu_Options_Control_GetMouseLookToggleMode(bBOOL);

					if (bBOOL)
					{
						((CMenuCheckButton*)me)->iState=1;
					}
					else
					{
						((CMenuCheckButton*)me)->iState=0;
					}

					pWindowMenuConsole->AddMenuCenterY(me);

					pc = new CMenuPanel();
					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_mouse_sensitivity"), _T("string"), _T(""), szMenuText, 256, NULL);
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, NOP);
					me->SetCheckOff();
					pc->AddElement(me);
					me = new CMenuSlider(BUTTON_MENUOPTIONS_CONTROLS_MOUSESENSITIVITY, iPosX2, 0);
					int iSensitivity = 0;
					ARXMenu_Options_Control_GetMouseSensitivity(iSensitivity);
					((CMenuSlider*)me)->iPos = iSensitivity;
					pc->AddElement(me);
					pWindowMenuConsole->AddMenuCenterY(pc);

					if (INTERNATIONAL_MODE)
					{
						PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_mouse_smoothing"), _T("string"), _T("mouse_smoothing"), szMenuText, 256, NULL);
						_tcscat(szMenuText, _T(" "));
						pTex1 = MakeTCFromFile("\\Graph\\interface\\menus\\menu_checkbox_off.bmp");
						pTex2 = MakeTCFromFile("\\Graph\\interface\\menus\\menu_checkbox_on.bmp");
						me = new CMenuCheckButton(BUTTON_MENUOPTIONS_CONTROLS_MOUSE_SMOOTHING, 0, 0, pTex1->m_dwWidth, pTex1, pTex2, new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, OPTIONS_INPUT));
						bBOOL = false;
						ARXMenu_Options_Control_GetMouseSmoothing(bBOOL);

						if (bBOOL)
						{
							((CMenuCheckButton*)me)->iState=1;
						}
						else
						{
							((CMenuCheckButton*)me)->iState=0;
						}

						pWindowMenuConsole->AddMenuCenterY(me);

						PAK_UNICODE_GetPrivateProfileString(_T("system_menus_autodescription"), _T("string"), _T("auto_description"), szMenuText, 256, NULL);
						_tcscat(szMenuText, _T(" "));
						pTex1 = MakeTCFromFile("\\Graph\\interface\\menus\\menu_checkbox_off.bmp");
						pTex2 = MakeTCFromFile("\\Graph\\interface\\menus\\menu_checkbox_on.bmp");
						me = new CMenuCheckButton(BUTTON_MENUOPTIONS_CONTROLS_AUTODESCRIPTION, 0, 0, pTex1->m_dwWidth, pTex1, pTex2, new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, OPTIONS_INPUT));
						bBOOL = false;
						ARXMenu_Options_Control_GetAutoDescription(bBOOL);

						if (bBOOL)
						{
							((CMenuCheckButton*)me)->iState=1;
						}
						else
						{
							((CMenuCheckButton*)me)->iState=0;
						}

						pWindowMenuConsole->AddMenuCenterY(me);
					}

					pTex = MakeTCFromFile("\\Graph\\interface\\menus\\back.bmp");
					me = new CMenuCheckButton(-1, fPosBack, fPosBackY, pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
					me->eMenuState = OPTIONS;
					me->SetShortCut(DIK_ESCAPE);
					pWindowMenuConsole->AddMenu(me);
					pWindowMenu->AddConsole(pWindowMenuConsole);
				//------------------ END INPUT

				//------------------ START CUSTOM CONTROLS
				_TCHAR	pNoDef1[]=_T("---");
				_TCHAR	pNoDef2[]=_T("---");

				#define CUSTOM_CTRL_X0	RATIO_X(20)
				#define CUSTOM_CTRL_X1	RATIO_X(150)
				#define CUSTOM_CTRL_X2	RATIO_X(245)
					long fControlPosY	=	ARX_CLEAN_WARN_CAST_LONG(RATIO_Y(8.f));
				#define CUSTOM_CTRL_FUNC(a,b,c,d){\
						pc=new CMenuPanel();\
						PAK_UNICODE_GetPrivateProfileString(_T(a), _T("string"), _T("?"), szMenuText, 256, NULL);\
						me = new CMenuElementText(-1, hFontControls, szMenuText, CUSTOM_CTRL_X0, 0,lColor,.7f, NOP);\
						me->SetCheckOff();\
						pc->AddElement(me);\
						me = new CMenuElementText(c, hFontControls, pNoDef1, CUSTOM_CTRL_X1, 0,lColor,.7f, NOP);\
						me->eState=GETTOUCH;\
						if((!b)||(c<0))\
						{\
							me->SetCheckOff();\
							((CMenuElementText*)me)->lColor=RGB(127,127,127);\
						}\
						pc->AddElement(me);\
						me = new CMenuElementText(d, hFontControls, pNoDef2, CUSTOM_CTRL_X2, 0,lColor,.7f, NOP);\
						me->eState=GETTOUCH;\
						if(d<0)\
						{\
							me->SetCheckOff();\
							((CMenuElementText*)me)->lColor=RGB(127,127,127);\
						}\
						pc->AddElement(me);\
						pc->Move(0,fControlPosY);\
						pWindowMenuConsole->AddMenu(pc);\
						fControlPosY += ARX_CLEAN_WARN_CAST_LONG( pc->GetHeight() + RATIO_Y(3.f) );\
					};


				#define CUSTOM_CTRL_FUNC2(a,b,c,d){\
						pc=new CMenuPanel();\
						PAK_UNICODE_GetPrivateProfileString(_T(a), _T("string"), _T("?"), szMenuText, 256, NULL);\
						_tcscat(szMenuText,_T("2"));\
						me = new CMenuElementText(-1, hFontControls, szMenuText, CUSTOM_CTRL_X0, 0,lColor,.7f, NOP);\
						me->SetCheckOff();\
						pc->AddElement(me);\
						me = new CMenuElementText(c, hFontControls, pNoDef1, CUSTOM_CTRL_X1, 0,lColor,.7f, NOP);\
						me->eState=GETTOUCH;\
						if((!b)||(c<0))\
						{\
							me->SetCheckOff();\
							((CMenuElementText*)me)->lColor=RGB(127,127,127);\
						}\
						pc->AddElement(me);\
						me = new CMenuElementText(d, hFontControls, pNoDef2, CUSTOM_CTRL_X2, 0,lColor,.7f, NOP);\
						me->eState=GETTOUCH;\
						if(d<0)\
						{\
							me->SetCheckOff();\
							((CMenuElementText*)me)->lColor=RGB(127,127,127);\
						}\
						pc->AddElement(me);\
						pc->Move(0,fControlPosY);\
						pWindowMenuConsole->AddMenu(pc);\
						fControlPosY += ARX_CLEAN_WARN_CAST_LONG( pc->GetHeight() + RATIO_Y(3.f) );\
					};


					pWindowMenuConsole=new CWindowMenuConsole(iWindowConsoleOffsetX,iWindowConsoleOffsetY,iWindowConsoleWidth,iWindowConsoleHeight,OPTIONS_INPUT_CUSTOMIZE_KEYS_1);
					
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_mouselook",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_MOUSELOOK1, BUTTON_MENUOPTIONS_CONTROLS_CUST_MOUSELOOK2);

					if (!INTERNATIONAL_MODE)
					{
						PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_link_use_to_mouselook"), _T("string"), _T("?"), szMenuText, 256, NULL);
						\
				pTex1 = MakeTCFromFile("\\Graph\\interface\\menus\\menu_checkbox_off.bmp");
				pTex2 = MakeTCFromFile("\\Graph\\interface\\menus\\menu_checkbox_on.bmp");
				pElementText= new CMenuElementText(-1, hFontControls, szMenuText, CUSTOM_CTRL_X0, 0,lColor,.7f, NOP);
				me = new CMenuCheckButton(BUTTON_MENUOPTIONS_CONTROLS_LINK, 0, 0, pTex1->m_dwWidth>>1, pTex1, pTex2, pElementText);
				me->Move(0,fControlPosY);
						pWindowMenuConsole->AddMenu(me);
						\
							fControlPosY += ARX_CLEAN_WARN_CAST_LONG(me->GetHeight() + RATIO_Y(3.f));

				if(pMenuConfig->bLinkMouseLookToUse)
				{
					((CMenuCheckButton*)me)->iState=1;
				}
				}

					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_action_combine",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_ACTIONCOMBINE1, BUTTON_MENUOPTIONS_CONTROLS_CUST_ACTIONCOMBINE2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_jump",1,BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP1, BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_magic_mode",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_MAGICMODE1, BUTTON_MENUOPTIONS_CONTROLS_CUST_MAGICMODE2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_stealth_mode",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_STEALTHMODE1, BUTTON_MENUOPTIONS_CONTROLS_CUST_STEALTHMODE2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_walk_forward",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKFORWARD1, BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKFORWARD2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_walk_backward",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKBACKWARD1, BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKBACKWARD2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_strafe_left",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFELEFT1, BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFELEFT2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_strafe_right",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFERIGHT1, BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFERIGHT2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_lean_left",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANLEFT1, BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANLEFT2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_lean_right",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANRIGHT1, BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANRIGHT2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_crouch",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCH1, BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCH2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_crouch_toggle",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCHTOGGLE1, BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCHTOGGLE2);

					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_strafe",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFE1, BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFE2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_center_view",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_CENTERVIEW1, BUTTON_MENUOPTIONS_CONTROLS_CUST_CENTERVIEW2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_freelook",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_FREELOOK1, BUTTON_MENUOPTIONS_CONTROLS_CUST_FREELOOK2);

					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_turn_left",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNLEFT1, BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNLEFT2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_turn_right",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNRIGHT1, BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNRIGHT2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_look_up",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKUP1, BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKUP2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_look_down",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKDOWN1, BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKDOWN2);

					pc=new CMenuPanel();

					pTex = MakeTCFromFile("\\Graph\\interface\\menus\\back.bmp");
					me = new CMenuCheckButton(BUTTON_MENUOPTIONS_CONTROLS_CUST_BACK, fPosBack, fPosBackY, pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
					me->eMenuState = OPTIONS_INPUT;
					me->SetShortCut(DIK_ESCAPE);
					pc->AddElementNoCenterIn(me);
					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_default"), _T("string"), _T(""), szMenuText, 256, NULL);
					me = new CMenuElementText(BUTTON_MENUOPTIONS_CONTROLS_CUST_DEFAULT, hFontMenu, szMenuText, 0, 0,lColor,1.f, NOP);
					me->SetPos((RATIO_X(iWindowConsoleWidth) - me->GetWidth())*0.5f, fPosBDAY);
					pc->AddElementNoCenterIn(me);
					pTex = MakeTCFromFile("\\Graph\\interface\\menus\\next.bmp");
					me = new CMenuCheckButton(BUTTON_MENUOPTIONS_CONTROLS_CUST_BACK, fPosNext, fPosBackY, pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
					me->eMenuState = OPTIONS_INPUT_CUSTOMIZE_KEYS_2;
					me->SetShortCut(DIK_ESCAPE);
					pc->AddElementNoCenterIn(me);

					pWindowMenuConsole->AddMenu(pc);

					pWindowMenu->AddConsole(pWindowMenuConsole);
					pMenuConfig->ReInitActionKey(pWindowMenuConsole);

					pWindowMenuConsole=new CWindowMenuConsole(iWindowConsoleOffsetX,iWindowConsoleOffsetY,iWindowConsoleWidth,iWindowConsoleHeight,OPTIONS_INPUT_CUSTOMIZE_KEYS_2);

					fControlPosY = ARX_CLEAN_WARN_CAST_LONG(RATIO_Y(8.f));
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_inventory",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_INVENTORY1, BUTTON_MENUOPTIONS_CONTROLS_CUST_INVENTORY2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_book",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOK1, BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOK2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_bookcharsheet",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKCHARSHEET1, BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKCHARSHEET2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_bookmap",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKMAP1, BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKMAP2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_bookspell",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKSPELL1, BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKSPELL2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_bookquest",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKQUEST1, BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKQUEST2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_drink_potion_life",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONLIFE1, BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONLIFE2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_drink_potion_mana",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONMANA1, BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONMANA2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_torch",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_TORCH1, BUTTON_MENUOPTIONS_CONTROLS_CUST_TORCH2);

					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_cancelcurrentspell",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_CANCELCURSPELL1, BUTTON_MENUOPTIONS_CONTROLS_CUST_CANCELCURSPELL2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_precast1",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST1, BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST1_2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_precast2",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST2, BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST2_2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_precast3",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST3, BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST3_2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_weapon",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_WEAPON1, BUTTON_MENUOPTIONS_CONTROLS_CUST_WEAPON2);

					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_unequipweapon",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_UNEQUIPWEAPON1, BUTTON_MENUOPTIONS_CONTROLS_CUST_UNEQUIPWEAPON2);

					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_previous",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_PREVIOUS1, BUTTON_MENUOPTIONS_CONTROLS_CUST_PREVIOUS2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_next",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_NEXT1, BUTTON_MENUOPTIONS_CONTROLS_CUST_NEXT2);

					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_quickload",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKLOAD, BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKLOAD2);
					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_quicksave",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKSAVE, BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKSAVE2);

					CUSTOM_CTRL_FUNC2("system_menus_options_input_customize_controls_bookmap",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_MINIMAP1, BUTTON_MENUOPTIONS_CONTROLS_CUST_MINIMAP2);

					pc=new CMenuPanel();

					pTex = MakeTCFromFile("\\Graph\\interface\\menus\\back.bmp");
					me = new CMenuCheckButton(BUTTON_MENUOPTIONS_CONTROLS_CUST_BACK, fPosBack, fPosBackY, pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
					me->eMenuState = OPTIONS_INPUT_CUSTOMIZE_KEYS_1;
					me->SetShortCut(DIK_ESCAPE);
					pc->AddElementNoCenterIn(me);
					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_default"), _T("string"), _T(""), szMenuText, 256, NULL);
					me = new CMenuElementText(BUTTON_MENUOPTIONS_CONTROLS_CUST_DEFAULT, hFontMenu, szMenuText, 0, 0,lColor,1.f, NOP);
					me->SetPos((RATIO_X(iWindowConsoleWidth) - me->GetWidth())*0.5f, fPosBDAY);
					pc->AddElementNoCenterIn(me);

					pWindowMenuConsole->AddMenu(pc);

					pWindowMenu->AddConsole(pWindowMenuConsole);
					pMenuConfig->ReInitActionKey(pWindowMenuConsole);
					#undef CUSTOM_CTRL_X0
					#undef CUSTOM_CTRL_X1
					#undef CUSTOM_CTRL_X2
					#undef CUSTOM_CTRL_FUNC
					#undef CUSTOM_CTRL_FUNC2
				//------------------ END CUSTOM CONTROLS

					pWindowMenu->eCurrentMenuState=OPTIONS;
				}
				break;

			case QUIT:
				{
					_TCHAR szMenuText[256];
					CMenuElement *me = NULL;
					CWindowMenuConsole *pWindowMenuConsole=new CWindowMenuConsole(iWindowConsoleOffsetX,iWindowConsoleOffsetY,iWindowConsoleWidth,iWindowConsoleHeight,QUIT);
					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_main_quit"), _T("string"), _T(""), szMenuText, 256, NULL);
					me=new CMenuElementText(-1, hFontMenu, szMenuText,0,0,lColor,1.f, NOP);
					me->bCheck = false;
					pWindowMenuConsole->AddMenuCenter(me);

					PAK_UNICODE_GetPrivateProfileString(_T("system_menus_main_editquest_confirm"), _T("string"), _T(""), szMenuText, 256, NULL);
					me=new CMenuElementText(-1, hFontMenu, szMenuText,0,0,lColor,1.f, NOP);
					me->bCheck = false;
					pWindowMenuConsole->AddMenuCenter(me);

					CMenuPanel *pPanel = new CMenuPanel();
					PAK_UNICODE_GetPrivateProfileString(_T("system_yes"), _T("string"), _T(""), szMenuText, 256, NULL);

					me = new CMenuElementText(BUTTON_MENUMAIN_QUIT, hFontMenu, szMenuText, 0, 0,lColor,1.f, NEW_QUEST_ENTER_GAME);

					me->SetPos(RATIO_X(iWindowConsoleWidth-10)-me->GetWidth(), 0);
					pPanel->AddElementNoCenterIn(me);
					PAK_UNICODE_GetPrivateProfileString(_T("system_no"), _T("string"), _T(""), szMenuText, 256, NULL);
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosBack, 0,lColor,1.f, MAIN);
					me->SetShortCut(DIK_ESCAPE);
					pPanel->AddElementNoCenterIn(me);


					ARX_CHECK_INT(fPosBDAY);
					
					pPanel->Move(0,ARX_CLEAN_WARN_CAST_INT(fPosBDAY));

					
					pWindowMenuConsole->AddMenu(pPanel);
					pWindowMenu->AddConsole(pWindowMenuConsole);
					pWindowMenu->eCurrentMenuState=QUIT;

				}
				break;
			}

		}
	}
	pMenu->Render();

	if(pWindowMenu)
	{
		if(!bScroll)
		{
			pWindowMenu->fAngle=90.f;
			pWindowMenu->eCurrentMenuState=pMenu->eOldMenuWindowState;
		}


		ARX_CHECK_INT(ARXDiffTimeMenu);
		
		pWindowMenu->Update(ARX_CLEAN_WARN_CAST_INT(ARXDiffTimeMenu));


		if (pWindowMenu)
		{
			MENUSTATE eMS=pWindowMenu->Render();

			if(eMS!=NOP)
			{
				pMenu->eOldMenuWindowState=eMS;
			}
		}

		Check_Apply();
	}

	bNoMenu=false;

	danaeApp.DANAEEndRender();

	if(pTextManage)
	{
		pTextManage->Update(ARXDiffTimeMenu);
		pTextManage->Render();
	}

	danaeApp.DANAEStartRender();
	GDevice->SetTextureStageState(0,D3DTSS_ADDRESS,D3DTADDRESS_CLAMP);

	GDevice->SetRenderState( D3DRENDERSTATE_FOGENABLE, false);
	SETZWRITE(GDevice, false);
	GDevice->SetRenderState( D3DRENDERSTATE_ZENABLE,false);
	GDevice->SetRenderState( D3DRENDERSTATE_CULLMODE,D3DCULL_NONE);
	pGetInfoDirectInput->DrawCursor();

	if(pMenu->bReInitAll)
	{
		GDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DRGBA(0,0,0,0), 1.0f, 0L);

		if(bForceReInitAllTexture)
		{
			D3DTextr_RestoreAllTextures(GDevice);
			bForceReInitAllTexture=false;
		}
	}

	if (pTextureLoadRender)
	{
		GDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,FALSE);

		int iOffsetX = 0;
		int iOffsetY=0;

		if ((DANAEMouse.y + INTERFACE_RATIO_DWORD(pTextureLoad->m_dwHeight)) > DANAESIZY)
		{
			
			float fOffestY	= iOffsetY - INTERFACE_RATIO_DWORD(pTextureLoad->m_dwHeight) ;
			ARX_CHECK_INT(fOffestY);
			iOffsetY	=	ARX_CLEAN_WARN_CAST_INT(fOffestY);


		}

		EERIEDrawBitmap(	GDevice,
			ARX_CLEAN_WARN_CAST_FLOAT(DANAEMouse.x + iOffsetX),
			ARX_CLEAN_WARN_CAST_FLOAT(DANAEMouse.y + iOffsetY),

							(float)INTERFACE_RATIO_DWORD(pTextureLoad->m_dwWidth),
							(float)INTERFACE_RATIO_DWORD(pTextureLoad->m_dwHeight),

							0.001f,
							pTextureLoad,
			ARX_OPAQUE_WHITE); 

		SETTC(GDevice,NULL);
		EERIEDraw2DRect(	GDevice,
			ARX_CLEAN_WARN_CAST_FLOAT(DANAEMouse.x + iOffsetX),
			ARX_CLEAN_WARN_CAST_FLOAT(DANAEMouse.y + iOffsetY),

							DANAEMouse.x+iOffsetX+(float)INTERFACE_RATIO_DWORD(pTextureLoad->m_dwWidth),
							DANAEMouse.y+iOffsetY+(float)INTERFACE_RATIO_DWORD(pTextureLoad->m_dwHeight),

							0.01f,
			ARX_OPAQUE_WHITE); 

		pTextureLoadRender=NULL;
	}

	if(ProcessFadeInOut(bFadeInOut,0.1f))
	{
		switch(iFadeAction)
		{
		case AMCM_CREDITS:
			ARX_MENU_Clicked_CREDITS();
			iFadeAction=-1;
			bFadeInOut=false;
			bFade=true;
			break;
		case AMCM_NEWQUEST:
			ARX_MENU_Clicked_NEWQUEST();
			iFadeAction=-1;
			bFadeInOut=false;
			bFade=true;
			CINEMASCOPE = 0;
			break;
		case AMCM_OFF:

			GAME_EDITOR = 0;
			ARX_MENU_Clicked_QUIT_GAME();
			iFadeAction=-1;
			bFadeInOut=false;
			bFade=true;
			break;
		}
	}

	SETALPHABLEND(GDevice,FALSE);
	GDevice->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTFP_LINEAR);
	GDevice->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTFP_LINEAR);
	GDevice->SetTextureStageState(0,D3DTSS_ADDRESS,D3DTADDRESS_WRAP);

	SETZWRITE(GDevice, true);

	danaeApp.EnableZBuffer();
	GDevice->SetRenderState( D3DRENDERSTATE_CULLMODE,D3DCULL_CCW);

	danaeApp.DANAEEndRender();
	return true;
}

//-----------------------------------------------------------------------------

CMenuElement::CMenuElement(MENUSTATE _ms) : CMenuZone()
{
	ePlace=NOCENTER;
	eState=TNOP;
	eMenuState=_ms;
	iShortCut=-1;
}

//-----------------------------------------------------------------------------

CMenuElement::~CMenuElement()
{
	if( this == pMenuElementApply )
	{
		pMenuElementApply = NULL;
	}

	if( this == pMenuElementResume )
	{
		pMenuElementResume = NULL;
	}

	if( this == pLoadConfirm )
	{
		pLoadConfirm = NULL;
	}

	if( this == pMenuSliderResol )
	{
		pMenuSliderResol = NULL;
	}

	if( this == pMenuSliderBpp )
	{
		pMenuSliderBpp = NULL;
	}

	if( this == pMenuSliderTexture )
	{
		pMenuSliderTexture = NULL;
	}

	if( this == pMenuCheckButtonBump )
	{
		pMenuCheckButtonBump = NULL;
	}

}

//-----------------------------------------------------------------------------

CMenuElement* CMenuElement::OnShortCut()
{
	if(iShortCut==-1) return NULL;

	if(	(pGetInfoDirectInput)&&
		(pGetInfoDirectInput->IsVirtualKeyPressedNowUnPressed(iShortCut)) )
	{
		return this;
	}

	return NULL;
}

//-----------------------------------------------------------------------------

CMenuElementText::CMenuElementText(int _iID, HFONT _pHFont,_TCHAR *_pText,float _fPosX,float _fPosY,long _lColor,float _fSize,MENUSTATE _eMs) : CMenuElement(_eMs)
{
	iID = _iID;

	pHFont = _pHFont;

	if(!_tcscmp(_pText,_T("---")))
	{
		bTestYDouble=true;
	}

	lpszText=_tcsdup(_pText);
	

	ARX_CHECK_LONG(_fPosX);
	ARX_CHECK_LONG(_fPosY);

	rZone.left	= ARX_CLEAN_WARN_CAST_LONG(_fPosX);
	rZone.top	= ARX_CLEAN_WARN_CAST_LONG(_fPosY);


	GetTextSize(pHFont, _pText, (int*)&rZone.right, (int*)&rZone.bottom);

	rZone.right+=rZone.left;
	rZone.bottom+=rZone.top;
	
	lColor=_lColor;
	lColorHighlight=lOldColor=RGB(255, 255, 255);

	fSize=_fSize;
	iId=(int)this;

	bSelected = false;

	iPosCursor=_tcslen(_pText)+1;
}

//-----------------------------------------------------------------------------

CMenuElementText::~CMenuElementText()
{
	if(lpszText)
	{
		free((void*)lpszText);
		lpszText = NULL;
	}
}

//-----------------------------------------------------------------------------

void CMenuElementText::SetText(_TCHAR *_pText)
{
	if(lpszText)
	{
		free((void*)lpszText);
		lpszText = NULL;
	}

	lpszText=_tcsdup(_pText);

	GetTextSize(pHFont, _pText, (int*)&rZone.right, (int*)&rZone.bottom);

	rZone.right+=rZone.left;
	rZone.bottom+=rZone.top;
}

//-----------------------------------------------------------------------------

void CMenuElementText::Update(int _iDTime)
{
}

//-----------------------------------------------------------------------------

bool CMenuElementText::OnMouseDoubleClick(int _iMouseButton)
{
	switch(iID)
	{
	case BUTTON_MENUEDITQUEST_LOAD:
		OnMouseClick(_iMouseButton);

		if (pWindowMenu)
		{
			for (UINT i = 0 ; i < pWindowMenu->vWindowConsoleElement.size() ; i++)
			{
				CWindowMenuConsole *p = pWindowMenu->vWindowConsoleElement[i];

				if ( p->eMenuState == EDIT_QUEST_LOAD )
				{
					for (UINT j = 0 ; j < p->MenuAllZone.vMenuZone.size() ; j++)
					{
						CMenuElement *pMenuElement = (CMenuElement*) ( (CMenuElement*)p->MenuAllZone.vMenuZone[j] )->GetZoneWithID( BUTTON_MENUEDITQUEST_LOAD_CONFIRM );

						if( pMenuElement )
						{
							pMenuElement->OnMouseClick( _iMouseButton );
						}
					}
				}
			}
		}

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// true: block les zones de checks
bool CMenuElementText::OnMouseClick(int _iMouseButton)
{
	switch(eState)
	{
	case EDIT:
		eState=EDIT_TIME;
		return true;
	case GETTOUCH:
		eState=GETTOUCH_TIME;
		lOldColor=lColorHighlight;
		return true;
	}

	if (iID != BUTTON_MENUMAIN_RESUMEGAME)
	{
		ARX_SOUND_PlayMenu(SND_MENU_CLICK);
	}

	switch (iID)
	{
	case -1:
		{
			return false;
		}
		break;
	// MENUMAIN
	case BUTTON_MENUMAIN_RESUMEGAME:
		{
			pTextManage->Clear();
			ARXMenu_ResumeGame();
			ARX_SOUND_PlayMenu(SND_MENU_CLICK);
		}
		break;
	case BUTTON_MENUMAIN_NEWQUEST:
		{
			bool bBOOL = false;
			ARXMenu_GetResumeGame(bBOOL);

			if (!bBOOL)
			{
				ARXMenu_NewQuest();
			}
		}
		break;
	case BUTTON_MENUMAIN_LOADQUEST:
		{
		}break;
	case BUTTON_MENUMAIN_SAVEQUEST:
		{
		}break;
	case BUTTON_MENUMAIN_MULTIPLAYER:
		{
		}break;
	case BUTTON_MENUMAIN_OPTIONS:
		{
		}break;
	case BUTTON_MENUMAIN_CREDITS:
		{
			ARXMenu_Credits();
		}
		break;
	case BUTTON_MENUMAIN_QUIT:
		{
			ARXMenu_Quit();
		}
		break;
	case BUTTON_MENUNEWQUEST_CONFIRM:
		{
			ARXMenu_NewQuest();
		}
		break;
	// MENULOADQUEST
	case BUTTON_MENUOPTIONSVIDEO_INIT:
		{
			pMenuConfig->iNewWidth = pMenuConfig->iWidth;
			pMenuConfig->iNewHeight = pMenuConfig->iHeight;
			pMenuConfig->iNewBpp = pMenuConfig->iBpp;
			pMenuConfig->iNewTextureResol = pMenuConfig->iTextureResol;

			pMenuConfig->bChangeResolution = false;
			pMenuConfig->bChangeTextures = false;
		}
		break;
	case BUTTON_MENUEDITQUEST_LOAD_INIT:
		{
			if ( pWindowMenu )
				for (UINT i = 0 ; i < pWindowMenu->vWindowConsoleElement.size() ; i++)
				{
					CWindowMenuConsole *p = pWindowMenu->vWindowConsoleElement[i];

					if ( p->eMenuState == EDIT_QUEST_LOAD )
					{
						pWindowMenu->vWindowConsoleElement[i]->lData = lData;

						for (UINT j = 0 ; j < p->MenuAllZone.vMenuZone.size() ; j++)
						{
							CMenuZone *cz = p->MenuAllZone.vMenuZone[j];

							if ( cz->iID == BUTTON_MENUEDITQUEST_LOAD )
							{
								( (CMenuElementText *)cz )->bSelected = false;
							}
						}
					}
				}
		}
		break;
	case BUTTON_MENUEDITQUEST_LOAD:
		{
			if (pWindowMenu)
			{
			pLoadConfirm->SetCheckOn();
			pLoadConfirm->lColor=pLoadConfirm->lOldColor;

				for (UINT i = 0 ; i < pWindowMenu->vWindowConsoleElement.size() ; i++)
			{
				CWindowMenuConsole *p = pWindowMenu->vWindowConsoleElement[i];

				if ( p->eMenuState == EDIT_QUEST_LOAD )
				{
					pWindowMenu->vWindowConsoleElement[i]->lData = lData;

						for (UINT j = 0 ; j < p->MenuAllZone.vMenuZone.size(); j++)
					{
						CMenuZone *cz = p->MenuAllZone.vMenuZone[j];

						if ( cz->iID == BUTTON_MENUEDITQUEST_LOAD )
						{
							( (CMenuElementText *)cz )->bSelected = false;
						}
					}

					bSelected = true;

					}
				}
			}
			}
		break;
	case BUTTON_MENUEDITQUEST_LOAD_CONFIRM:
		{
			if (pWindowMenu)
			{
				for (UINT i = 0 ; i < pWindowMenu->vWindowConsoleElement.size() ; i++)
			{
				CWindowMenuConsole *p = pWindowMenu->vWindowConsoleElement[i];

				if ( p->eMenuState == EDIT_QUEST_LOAD )
				{
					lData = pWindowMenu->vWindowConsoleElement[i]->lData;

					if ( lData )
					{
						{
							_TCHAR szT[256];
							_stprintf(szT, _T("%lu - %S"), lData, lpszText);
							char ml[256];
							memset( ml, 0, 256 );

							eMenuState = MAIN;
							GDevice->Clear( 0, NULL, D3DCLEAR_ZBUFFER,0, 1.0f, 0L );	
							ARXMenu_LoadQuest( lData );

							bNoMenu=true;

							if( pTextManage )
							{
								pTextManage->Clear();
							}

							break;
						}
					}
				}
			}

			pLoadConfirm->SetCheckOff();
			pLoadConfirm->lColor=RGB( 127, 127, 127 );
			}
		}
		break;
	case BUTTON_MENUEDITQUEST_LOAD_CONFIRM_BACK:
		pLoadConfirm->SetCheckOff();
		pLoadConfirm->lColor=RGB(127,127,127);
		break;
	// MENUSAVEQUEST
	case BUTTON_MENUEDITQUEST_SAVE:
		{
			if (pWindowMenu)
				for (UINT i = 0 ; i < pWindowMenu->vWindowConsoleElement.size() ; i++)
			{
				CWindowMenuConsole *p = pWindowMenu->vWindowConsoleElement[i];

				if ( p->eMenuState == EDIT_QUEST_SAVE_CONFIRM )
				{
					pWindowMenu->vWindowConsoleElement[i]->lData = lData;
					CMenuElementText * me = (CMenuElementText *) p->MenuAllZone.vMenuZone[1];

					if ( me )
					{
						_TCHAR szT[256];
						_stprintf( szT, _T("%lu - %S"), me->lData, me->lpszText );
						char ml[256];
						memset( ml, 0, 256 );
						WideCharToMultiByte( CP_ACP, 0, me->lpszText, _tcslen( me->lpszText ),
							ml,  _tcslen( me->lpszText ) + 1,
							"_", NULL );

						strcpy( save_l[me->lData].name, ml );
						eMenuState = MAIN;
						ARXMenu_SaveQuest( me->lData );
						break;
					}
				}
			}
		}
		break;
	case BUTTON_MENUEDITQUEST_DELETE:
		{
			if (pWindowMenu)
				for (UINT i = 0 ; i < pWindowMenu->vWindowConsoleElement.size() ; i++)
			{
				CWindowMenuConsole *p = pWindowMenu->vWindowConsoleElement[i];

				if ( p->eMenuState == EDIT_QUEST_DELETE_CONFIRM )
				{
					pWindowMenu->vWindowConsoleElement[i]->lData = lData;
					CMenuElementText * me = (CMenuElementText *) p->MenuAllZone.vMenuZone[1];

					if ( me )
					{
						_TCHAR szT[256];
						_stprintf( szT, _T("%lu - %S"), me->lData, me->lpszText );

						char ml[256];
						memset(ml,0,256);
						WideCharToMultiByte( CP_ACP, 0, me->lpszText, _tcslen( me->lpszText ),
							ml,  _tcslen( me->lpszText ) + 1,
							"_", NULL );

						strcpy( save_l[me->lData].name, ml );
						eMenuState = MAIN;
						ARXMenu_DeleteQuest( me->lData );
						FreeSaveGameList();
						CreateSaveGameList();
							break;
							}
								}
							}
						}
						break;
	case BUTTON_MENUOPTIONSVIDEO_APPLY:
		{
			//----------BUMP
			if(pMenuConfig->bNewBumpMapping!=pMenuConfig->bBumpMapping)
			{
				pMenuConfig->bBumpMapping=pMenuConfig->bNewBumpMapping;

				if(pMenuConfig->bBumpMapping)
				{
					EERIE_ActivateBump();
				}
				else
				{
					EERIE_DesactivateBump();
				}

				if(pMenuConfig->bBumpMapping!=bALLOW_BUMP)
				{
					bForceReInitAllTexture=true;
				}

				bALLOW_BUMP=pMenuConfig->bBumpMapping;

				pMenuCheckButtonBump->iOldState=-1;
			}

			//----------END_BUMP

			//----------CHANGE_TEXTURE
			if(pMenuConfig->iNewTextureResol!=pMenuConfig->iTextureResol)
			{
				pMenuConfig->iTextureResol=pMenuConfig->iNewTextureResol;

				if(pMenuConfig->iTextureResol==2)Project.TextureSize=0;

				if(pMenuConfig->iTextureResol==1)Project.TextureSize=2;

				if(pMenuConfig->iTextureResol==0)Project.TextureSize=64;

				WILL_RELOAD_ALL_TEXTURES=1;

				pMenuSliderTexture->iOldPos=-1;
			}

			//----------END_CHANGE_TEXTURE

			//----------RESOLUTION
			if(	(pMenuConfig->iNewWidth!=pMenuConfig->iWidth)||
				(pMenuConfig->iNewHeight!=pMenuConfig->iHeight)||
				(pMenuConfig->iNewBpp!=pMenuConfig->iBpp) )
			{
				pMenuConfig->iWidth=pMenuConfig->iNewWidth;
				pMenuConfig->iHeight=pMenuConfig->iNewHeight;
				pMenuConfig->iBpp=pMenuConfig->iNewBpp;
				ARXMenu_Private_Options_Video_SetResolution(	pMenuConfig->iWidth,
																pMenuConfig->iHeight,
																pMenuConfig->iBpp);

				pMenuSliderResol->iOldPos=-1;
				pMenuSliderBpp->iOldPos=-1;
			}

			//----------END_RESOLUTION
			pMenuConfig->bChangeResolution = false;
			pMenuConfig->bChangeTextures = false;
			pMenu->bReInitAll=true;
		}
		break;
	// MENUOPTIONS_CONTROLS
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP1:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP2:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_MAGICMODE1:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_MAGICMODE2:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_STEALTHMODE1:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_STEALTHMODE2:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKFORWARD1:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKFORWARD2:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKBACKWARD1:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKBACKWARD2:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFELEFT1:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFELEFT2:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFERIGHT1:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFERIGHT2:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANLEFT1:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANLEFT2:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANRIGHT1:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANRIGHT2:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCH1:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCH2:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_MOUSELOOK1:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_ACTIONCOMBINE1:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_INVENTORY1:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_INVENTORY2:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOK1:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOK2:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONLIFE1:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONLIFE2:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONMANA1:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONMANA2:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_CUST_DEFAULT:
		{
		}break;
	case BUTTON_MENUOPTIONS_CONTROLS_BACK:
		{
			if(pMenuConfig)
				pMenuConfig->SaveAll();
		}
		break;
	}

	if ((eMenuState == EDIT_QUEST_LOAD_CONFIRM) ||
		(eMenuState == EDIT_QUEST_SAVE_CONFIRM) ||
		(eMenuState == EDIT_QUEST_DELETE_CONFIRM))
	{
		for (UINT i = 0 ; i < pWindowMenu->vWindowConsoleElement.size() ; i++)
		{
			CWindowMenuConsole *p = pWindowMenu->vWindowConsoleElement[i];

			if ( p->eMenuState == eMenuState )
			{
				p->lData = lData;
				CMenuElementText * me = (CMenuElementText *) p->MenuAllZone.vMenuZone[1];

				if ( me )
				{
					me->lData = lData;
					_TCHAR szText[256];

					if( lData )
						_stprintf( szText, _T("%S"), save_l[lData].name );
					else
					{
						PAK_UNICODE_GetPrivateProfileString( _T("system_menu_editquest_newsavegame"), _T("string"), _T(""), szText, 256, NULL );
					}

					me->SetText( szText );
					p->AlignElementCenter( me );
				}
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// true: block les zones de checks
CMenuElement* CMenuElementText::OnShortCut()
{
	if(iShortCut==-1) return NULL;

	if(	(pGetInfoDirectInput)&&
		(pGetInfoDirectInput->IsVirtualKeyPressedNowUnPressed(iShortCut)) )
	{
		return this;
	}

	return NULL;
}

//-----------------------------------------------------------------------------

void CMenuElementText::Render()
{

	if(WILL_RELOAD_ALL_TEXTURES) return;

	if(bNoMenu) return;

	EERIE_3D ePos;
	ePos.x = (float) rZone.left;
	ePos.y = (float) rZone.top;
	ePos.z = 1;

	if (bSelected)
		FontRenderText(pHFont, ePos, lpszText, lColorHighlight);

	else
		FontRenderText(pHFont, ePos, lpszText, lColor);

}

//-----------------------------------------------------------------------------

void CMenuElementText::RenderMouseOver()
{
	if(WILL_RELOAD_ALL_TEXTURES) return;

	if(bNoMenu) return;

	pGetInfoDirectInput->SetMouseOver();

	GDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE,  true);
	GDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
	GDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND,  D3DBLEND_ONE);

	EERIE_3D ePos;
	ePos.x = (float)rZone.left;
	ePos.y = (float)rZone.top;
	ePos.z = 1;

	FontRenderText(pHFont, ePos, lpszText, lColorHighlight);

	GDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE,  false);

	switch (iID)
	{
	case BUTTON_MENUEDITQUEST_LOAD:
		{
			CURRENT_GAME_INSTANCE=save_l[lData].num;
			ARX_GAMESAVE_MakePath();
			char tTxt[256];
			sprintf(tTxt,"%sGSAVE.BMP",GameSavePath+strlen(Project.workingdir));
			TextureContainer *pTextureTemp=GetTextureFile_NoRefinement(tTxt);

			if (pTextureTemp != pTextureLoad)
			{
				if (pTextureLoad)
				{
					D3DTextr_KillTexture(pTextureLoad);
				}

				pTextureLoad=pTextureTemp;

				if (pTextureLoad)
				{
					pTextureLoad->Restore(GDevice);
					pTextureLoad->bColorKey=false;
				}
			}

			pTextureLoadRender=pTextureLoad;
		}
		break;
	case BUTTON_MENUEDITQUEST_SAVEINFO:
		{
			CURRENT_GAME_INSTANCE=save_l[lData].num;
			ARX_GAMESAVE_MakePath();
			char tTxt[256];
			sprintf(tTxt,"%sGSAVE.BMP",GameSavePath+strlen(Project.workingdir));
			TextureContainer *pTextureTemp=GetTextureFile_NoRefinement(tTxt);

			if (pTextureTemp != pTextureLoad)
			{
				if (pTextureLoad)
				{
					D3DTextr_KillTexture(pTextureLoad);
				}

				pTextureLoad=pTextureTemp;

				if (pTextureLoad)
				{
					pTextureLoad->Restore(GDevice);
					pTextureLoad->bColorKey=false;
				}
			}

			pTextureLoadRender=pTextureLoad;
		}
		break;
	default:
		pTextureLoadRender=NULL;
		break;
	}
}

//-----------------------------------------------------------------------------

CMenuState::CMenuState(MENUSTATE _ms)
{
	bReInitAll=false;
	eMenuState = _ms;
	eOldMenuState = NOP;
	eOldMenuWindowState= NOP;
	pTexBackGround = NULL;
	pTexBackGround1 = NULL;
	fPos = 0;
	pMenuAllZone=new CMenuAllZone();

	iPosMenu=-1;
}

//-----------------------------------------------------------------------------

CMenuState::~CMenuState()
{
	if(pMenuAllZone) delete pMenuAllZone;

	if (pTexBackGround)
	{
		D3DTextr_KillTexture(pTexBackGround);
		pTexBackGround = NULL;
	}

	if (pTexBackGround1)
	{
		D3DTextr_KillTexture(pTexBackGround1);
		pTexBackGround1 = NULL;
	}
}

//-----------------------------------------------------------------------------

void CMenuState::AddMenuElement(CMenuElement *_me)
{
	pMenuAllZone->AddZone((CMenuZone*)_me);
}

//-----------------------------------------------------------------------------

MENUSTATE CMenuState::Update(int _iDTime)
{
	fPos += _iDTime*DIV700;

	pZoneClick=NULL;

	int iR=pMenuAllZone->CheckZone(pGetInfoDirectInput->iMouseAX,pGetInfoDirectInput->iMouseAY);

	bool bReturn=false;

	if(pGetInfoDirectInput->GetMouseButton(DXI_BUTTON0))
	{
		if(iR!=-1)
		{
			pZoneClick=(CMenuElement*)iR;

			pZoneClick->OnMouseClick(1);
			bReturn=true;
		}
	}
	else
	{
		if(iR!=-1)
		{
			pZoneClick=(CMenuElement*)iR;
		}
	}

	//GESTION DES TOUCHES
	if(bReturn)
	{
		return pZoneClick->eMenuState;
	}

	return NOP;
}

//-----------------------------------------------------------------------------

void CMenuState::Render()
{
	if(WILL_RELOAD_ALL_TEXTURES) return;

	if(bNoMenu) return;

	if (pTexBackGround)
	{
		if (pTexBackGround->m_pddsSurface)
		{
			EERIEDrawBitmap2(GDevice, 0, 0, ARX_CLEAN_WARN_CAST_FLOAT(DANAESIZX), ARX_CLEAN_WARN_CAST_FLOAT(DANAESIZY), 0.999f, pTexBackGround, D3DCOLORWHITE);
	}
		}

	//------------------------------------------------------------------------

	int t=pMenuAllZone->GetNbZone();


	ARX_CHECK_INT(ARXDiffTimeMenu);
	int iARXDiffTimeMenu	= ARX_CLEAN_WARN_CAST_INT(ARXDiffTimeMenu);


	for(int i=0;i<t;++i)
	{
		CMenuElement *pMe=(CMenuElement*)pMenuAllZone->GetZoneNum(i);
		pMe->Update(iARXDiffTimeMenu);
		pMe->Render();
	}

	//HIGHLIGHT
	if(pZoneClick)
	{
		pZoneClick->RenderMouseOver();
	}

	//DEBUG ZONE
	SETTC(GDevice,NULL);
	pMenuAllZone->DrawZone();
}

//-----------------------------------------------------------------------------

CMenuZone::CMenuZone()
{
	bActif = true;
	bCheck=true;
	bTestYDouble=false;
	iID=-1;
	lData=0;
	pData=NULL;
	lPosition=0;
	
	rZone.top = 0;
	rZone.bottom = 0;
	rZone.left = 0;
	rZone.right = 0;
}

//-----------------------------------------------------------------------------

CMenuZone::CMenuZone(int _iX1,int _iY1,int _iX2,int _iY2,int _iId)
{
	bActif=true;
	rZone.left=_iX1;
	rZone.top=_iY1;
	rZone.right=_iX2;
	rZone.bottom=_iY2;
	iId=_iId;

	iID=-1;
	lData=0;
	pData=NULL;
}

//-----------------------------------------------------------------------------

CMenuZone::~CMenuZone()
{

}

//-----------------------------------------------------------------------------

void CMenuZone::Move(int _iX,int _iY)
{
	rZone.left		+= _iX;
	rZone.top		+= _iY;
	rZone.right		+= _iX;
	rZone.bottom	+= _iY;
}

//-----------------------------------------------------------------------------

void CMenuZone::SetPos(float _fX,float _fY)
{
	int iWidth		= rZone.right - rZone.left;
	int iHeight		= rZone.bottom - rZone.top;


	ARX_CHECK_INT(_fX);
	ARX_CHECK_INT(_fY);
	int iX	= ARX_CLEAN_WARN_CAST_INT(_fX);
	int iY	= ARX_CLEAN_WARN_CAST_INT(_fY);


	rZone.left		= iX;
	rZone.top		= iY;
	rZone.right		= iX + abs(iWidth);
	rZone.bottom	= iY + abs(iHeight);
}

//-----------------------------------------------------------------------------

long CMenuZone::IsMouseOver(int _iX, int _iY)
{
	int iYDouble=0;

	if(bTestYDouble)
	{
		iYDouble=(rZone.bottom-rZone.top)>>1;
	}

	if(	bActif && 
		(_iX >= rZone.left) &&
		(_iY >= (rZone.top-iYDouble)) &&
		(_iX <= rZone.right) &&
		(_iY <= (rZone.bottom+iYDouble)) )
		return iId;

	return -1;
}

//-----------------------------------------------------------------------------

CMenuAllZone::CMenuAllZone()
{
	vMenuZone.clear();
	
	vector<CMenuZone*>::iterator i;

	for(i=vMenuZone.begin();i!=vMenuZone.end();i++)
	{
		CMenuZone *zone=*i;
		delete zone;
	}
}

//-----------------------------------------------------------------------------

CMenuAllZone::~CMenuAllZone()
{
	vector<CMenuZone*>::iterator i;

	for(i=vMenuZone.begin();i!=vMenuZone.end();i++)
	{
		CMenuZone *zone=*i;
		delete zone;
	}

	vMenuZone.clear();
}

//-----------------------------------------------------------------------------

void CMenuAllZone::AddZone(CMenuZone *_pMenuZone)
{
	vMenuZone.insert(vMenuZone.end(),_pMenuZone);
}

//-----------------------------------------------------------------------------

int CMenuAllZone::CheckZone(int _iPosX,int _iPosY)
{
	vector<CMenuZone*>::iterator i;

	for(i=vMenuZone.begin();i!=vMenuZone.end();i++)
	{
		CMenuZone *zone=*i;

		if(zone->bCheck && zone->bActif)
		{
			long iIndex = ((*i)->IsMouseOver(_iPosX, _iPosY));

			if (iIndex != -1)
				return iIndex;
		}
	}

	return -1;
}

//-----------------------------------------------------------------------------

CMenuZone * CMenuAllZone::GetZoneNum(int _iNum)
{
	vector<CMenuZone*>::iterator i;
	int iNum=0;

	for(i=vMenuZone.begin();i!=vMenuZone.end();i++)
	{
		CMenuZone *zone=*i;

		if(iNum==_iNum) return zone;

		iNum++;
	}

	return NULL;
}

//-----------------------------------------------------------------------------

CMenuZone * CMenuAllZone::GetZoneWithID(int _iID)
{
	vector<CMenuZone*>::iterator i;

	for(i=vMenuZone.begin();i!=vMenuZone.end();i++)
	{
		CMenuZone *zone;
		CMenuElement *me=(CMenuElement*)(*i);
		zone=me->GetZoneWithID(_iID);

		if(zone) return zone;
	}

	return NULL;
}

//-----------------------------------------------------------------------------

void CMenuAllZone::Move(int _iPosX,int _iPosY)
{
	vector<CMenuZone*>::iterator i;

	for(i=vMenuZone.begin();i!=vMenuZone.end();i++)
	{
		(*i)->Move(_iPosX, _iPosY);
	}
}

//-----------------------------------------------------------------------------

int CMenuAllZone::GetNbZone()
{
	return vMenuZone.size();
}

//-----------------------------------------------------------------------------

void CMenuAllZone::DrawZone()
{
#ifdef NODEBUGZONE
	return;
#endif
	
	GDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
	GDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
	SETALPHABLEND(GDevice,true);

	vector<CMenuZone*>::iterator i;

	SETTC(GDevice,NULL);

	for(i=vMenuZone.begin();i!=vMenuZone.end();i++)
	{
		CMenuZone *zone=*i;

		if(zone->bActif)
		{
			D3DTLVERTEX v1[3],v2[3];
			v1[0].sx = (float)zone->rZone.left;
			v1[0].sy = (float)zone->rZone.top;
			v1[1].sx = (float)zone->rZone.left;
			v1[1].sy = (float)zone->rZone.bottom;
			v1[2].sx = (float)zone->rZone.right;
			v1[2].sy = (float)zone->rZone.bottom;
			
			v2[0].sx = (float)zone->rZone.left;
			v2[0].sy = (float)zone->rZone.top;
			v2[1].sx = (float)zone->rZone.right;
			v2[1].sy = (float)zone->rZone.top;
			v2[2].sx = (float)zone->rZone.right;
			v2[2].sy = (float)zone->rZone.bottom;
			
			v1[0].color=v1[1].color=v1[2].color=v2[0].color=v2[1].color=v2[2].color=0xFFFFA000;	
			v1[0].sz=v1[1].sz=v1[2].sz=v2[0].sz=v2[1].sz=v2[2].sz=0.f;	
			v1[0].rhw=v1[1].rhw=v1[2].rhw=v2[0].rhw=v2[1].rhw=v2[2].rhw=0.999999f;	
			
			EERIEDRAWPRIM(GDevice,D3DPT_TRIANGLESTRIP,D3DFVF_TLVERTEX|D3DFVF_DIFFUSE,v1,3,0);
			EERIEDRAWPRIM(GDevice,D3DPT_TRIANGLESTRIP,D3DFVF_TLVERTEX|D3DFVF_DIFFUSE,v2,3,0);
		}
	}

	SETALPHABLEND(GDevice,false);
}

//-----------------------------------------------------------------------------

CMenuCheckButton::CMenuCheckButton(int _iID, float _fPosX,float _fPosY,int _iTaille,TextureContainer *_pTex1,TextureContainer *_pTex2, CMenuElementText *_pText)
:CMenuElement(NOP)
{
	iID = _iID;
	iState	= 0;
	iOldState = -1;


	ARX_CHECK_INT(_fPosX);
	ARX_CHECK_INT(_fPosY);
	iPosX	= ARX_CLEAN_WARN_CAST_INT(_fPosX);
	iPosY	= ARX_CLEAN_WARN_CAST_INT(_fPosY);


	iTaille = _iTaille;

	pText	= _pText;

	if (_pTex1)
	{

		float fRatioX = RATIO_X(_pTex1->m_dwWidth) ;
		float fRatioY = RATIO_Y(_pTex1->m_dwHeight);
		ARX_CHECK_INT(fRatioX);
		ARX_CHECK_INT(fRatioY);

		vTex.insert(vTex.end(), _pTex1);
		_iTaille = max (_iTaille, ARX_CLEAN_WARN_CAST_INT(fRatioX) );
		_iTaille = max (_iTaille, ARX_CLEAN_WARN_CAST_INT(fRatioY) );

	}

	if (_pTex2)
	{

		float fRatioX = RATIO_X(_pTex2->m_dwWidth) ;
		float fRatioY = RATIO_Y(_pTex2->m_dwHeight);
		ARX_CHECK_INT(fRatioX);
		ARX_CHECK_INT(fRatioY);

		vTex.insert(vTex.end(), _pTex2);
		_iTaille = max (_iTaille, ARX_CLEAN_WARN_CAST_INT(fRatioX));
		_iTaille = max (_iTaille, ARX_CLEAN_WARN_CAST_INT(fRatioY));

	}

	int x = 0;
	int y = 0;

	if (pText)
	{
		GetTextSize(pText->pHFont, pText->lpszText, &x, &y); 

		_iTaille = max (_iTaille, y);
		x += pText->rZone.left;
		pText->Move(iPosX, iPosY + (_iTaille - y) / 2);
	}



	ARX_CHECK_LONG( _fPosX );
	ARX_CHECK_LONG( _fPosY );
	ARX_CHECK_LONG( _fPosX + _iTaille + x );
	ARX_CHECK_LONG( _fPosY + max(_iTaille, y) );
	//CAST
	rZone.left		= ARX_CLEAN_WARN_CAST_LONG( _fPosX );
	rZone.top		= ARX_CLEAN_WARN_CAST_LONG( _fPosY );
	rZone.right		= ARX_CLEAN_WARN_CAST_LONG( _fPosX + _iTaille + x );
	rZone.bottom	= ARX_CLEAN_WARN_CAST_LONG( _fPosY + max(_iTaille, y) );
	iId=(int)this;

	if (_pTex2)
	{
		float rZoneR = ( RATIO_X(200.f) + RATIO_X(_pTex1->m_dwWidth) + (RATIO_X(12*9) - RATIO_X(_pTex1->m_dwWidth))*0.5f );
		ARX_CHECK_LONG( rZoneR );
		rZone.right = ARX_CLEAN_WARN_CAST_LONG ( rZoneR );
	}



	Move(iPosX, iPosY);
}

//-----------------------------------------------------------------------------

CMenuCheckButton::~CMenuCheckButton()
{

	vTex.clear();

	if (pText)
	{
		delete pText;
		pText = NULL;
	}
}

//-----------------------------------------------------------------------------

void CMenuCheckButton::Update(int _iDTime)
{
}

//-----------------------------------------------------------------------------

void CMenuCheckButton::Move(int _iX, int _iY)
{
	CMenuElement::Move(_iX, _iY);

	if (pText)
	{
		pText->Move(_iX, _iY);
	}
}

//-----------------------------------------------------------------------------

bool CMenuCheckButton::OnMouseClick(int _iMouseButton)
{
	if(iOldState<0)
		iOldState=iState;

	iState ++;

//NB : It seems that iState cannot be negative (used as tabular index / used as bool) but need further approval
	ARX_CHECK_NOT_NEG( iState );

	if (ARX_CAST_UINT( iState ) >= vTex.size())
	{

		iState = 0;
	}

	ARX_SOUND_PlayMenu(SND_MENU_CLICK);

	switch (iID)
	{
	case BUTTON_MENUOPTIONSVIDEO_FULLSCREEN:
		{
			ARXMenu_Options_Video_SetFullscreen((iState)?true:false);
			pMenu->bReInitAll=true;
		}
		break;
	case BUTTON_MENUOPTIONSVIDEO_BUMP:
		{
			ARXMenu_Options_Video_SetBump((iState)?true:false);
		}
		break;
	case BUTTON_MENUOPTIONSVIDEO_CROSSHAIR:
		{
			if(pMenuConfig) pMenuConfig->bShowCrossHair=(iState)?true:false;
		}
		break;
	case BUTTON_MENUOPTIONSVIDEO_ANTIALIASING:
		{
			if(pMenuConfig) pMenuConfig->bAntiAliasing=(iState)?true:false;

			ARX_SetAntiAliasing();
		}
		break;
	case BUTTON_MENUOPTIONSVIDEO_DEBUGSETTING:
		{
			if(pMenuConfig)
			{
				pMenuConfig->bDebugSetting=(iState)?true:false;
				pMenuConfig->bDebugSetting=ARXMenu_Options_Video_SetSoftRender()?1:0;
			}
		}
		break;
	case BUTTON_MENUOPTIONSAUDIO_EAX:
		{
			ARXMenu_Options_Audio_SetEAX((iState)?true:false);
		}
		break;
	case BUTTON_MENUOPTIONS_CONTROLS_INVERTMOUSE:
		{
			ARXMenu_Options_Control_SetInvertMouse((iState)?true:false);
		}
		break;
	case BUTTON_MENUOPTIONS_CONTROLS_AUTOREADYWEAPON:
		{
			ARXMenu_Options_Control_SetAutoReadyWeapon((iState)?true:false);
		}
		break;
	case BUTTON_MENUOPTIONS_CONTROLS_MOUSELOOK:
		{
			ARXMenu_Options_Control_SetMouseLookToggleMode((iState)?true:false);
		}
		break;
	case BUTTON_MENUOPTIONS_CONTROLS_AUTODESCRIPTION:
		{
			ARXMenu_Options_Control_SetAutoDescription((iState)?true:false);
		}
		break;
	case BUTTON_MENUOPTIONS_CONTROLS_MOUSE_SMOOTHING:
		{
			ARXMenu_Options_Control_SetMouseSmoothing((iState)?true:false);
		}
		break;
	case BUTTON_MENUOPTIONS_CONTROLS_LINK:
		{
			if(pMenuConfig)
			{
				pMenuConfig->bLinkMouseLookToUse=(iState)?true:false;
			}
		}
		break;
	case BUTTON_MENUOPTIONSVIDEO_BACK:
		{
			if(pMenuConfig)
			{
				if(	(pMenuSliderResol)&&
					(pMenuSliderResol->iOldPos>=0) )
				{
			 		pMenuSliderResol->iPos=pMenuSliderResol->iOldPos;
					pMenuSliderResol->iOldPos=-1;
					pMenuConfig->iNewWidth=pMenuConfig->iWidth;
					pMenuConfig->iNewHeight=pMenuConfig->iHeight;
				}

				if(	(pMenuSliderBpp)&&
					(pMenuSliderBpp->iOldPos>=0) )
				{
			 		pMenuSliderBpp->iPos=pMenuSliderBpp->iOldPos;
					pMenuSliderBpp->iOldPos=-1;
					pMenuConfig->iNewBpp=pMenuConfig->iBpp;
				}

				if(	(pMenuSliderTexture)&&
					(pMenuSliderTexture->iOldPos>=0) )
				{
			 		pMenuSliderTexture->iPos=pMenuSliderTexture->iOldPos;
					pMenuSliderTexture->iOldPos=-1;
					pMenuConfig->iNewTextureResol=pMenuConfig->iTextureResol;
				}

				if(	(pMenuCheckButtonBump)&&
					(pMenuCheckButtonBump->iOldState>=0) )
				{
			 		pMenuCheckButtonBump->iState=pMenuCheckButtonBump->iOldState;
					pMenuCheckButtonBump->iOldState=-1;
					pMenuConfig->bNewBumpMapping=pMenuConfig->bBumpMapping;
				}
			}
		}
		break;

	}

	return false;
}

//-----------------------------------------------------------------------------

void CMenuCheckButton::Render()
{
	if(WILL_RELOAD_ALL_TEXTURES) return;

	if(bNoMenu) return;

	GDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, true);
	GDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
	GDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);

	if (vTex.size())
	{
		TextureContainer *pTex = vTex[iState];
	
		D3DTLVERTEX v[4];
		unsigned long color;

		if(bCheck)
			color = ARX_OPAQUE_WHITE;
		else
			color=0xFF3F3F3F;	

		v[0].sz=v[1].sz=v[2].sz=v[3].sz=0.f;
		v[0].rhw=v[1].rhw=v[2].rhw=v[3].rhw=0.999999f;
		
		float iY = 0;

		{
			iY = ARX_CLEAN_WARN_CAST_FLOAT(rZone.bottom - rZone.top);
			iY -= iTaille;
			iY = rZone.top + iY*0.5f;
		}
		
		//carre
		EERIEDrawBitmap2(GDevice, ARX_CLEAN_WARN_CAST_FLOAT(rZone.right - iTaille), iY, RATIO_X(iTaille), RATIO_Y(iTaille), 0.f, pTex, color);
	}

	if (pText)
	{
		pText->Render();
	}

	//DEBUG
	GDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, false);
}

//-----------------------------------------------------------------------------

void CMenuCheckButton::RenderMouseOver()
{
	if(WILL_RELOAD_ALL_TEXTURES) return;

	if(bNoMenu) return;

	pGetInfoDirectInput->SetMouseOver();

	GDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, true);
	GDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
	GDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);

	TextureContainer *pTex = vTex[iState];

	if(pTex) SETTC(GDevice, pTex);
	else SETTC(GDevice,NULL);

	D3DTLVERTEX v[4];
	v[0].color = v[1].color = v[2].color = v[3].color = ARX_OPAQUE_WHITE;
	v[0].sz=v[1].sz=v[2].sz=v[3].sz=0.f;	
	v[0].rhw=v[1].rhw=v[2].rhw=v[3].rhw=0.999999f;

	float iY = 0;
	iY = ARX_CLEAN_WARN_CAST_FLOAT(rZone.bottom - rZone.top);
	iY -= iTaille;
	iY = rZone.top + iY*0.5f;

	//carre

	EERIEDrawBitmap2(GDevice, ARX_CLEAN_WARN_CAST_FLOAT(rZone.right - iTaille), iY, RATIO_X(iTaille), RATIO_Y(iTaille), 0.f, pTex, ARX_OPAQUE_WHITE); 


	//tick
	if (pText)
	{
		pText->RenderMouseOver();
	}

	//DEBUG
	GDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, false);
}

//-----------------------------------------------------------------------------

CWindowMenu::CWindowMenu(int _iPosX,int _iPosY,int _iTailleX,int _iTailleY,int _iNbButton) :
	bMouseListen (true)
{
	iPosX=(int)RATIO_X(_iPosX);
	iPosY=(int)RATIO_Y(_iPosY);
	iTailleX=(int)RATIO_X(_iTailleX);
	iTailleY=(int)RATIO_Y(_iTailleY);
	iNbButton=_iNbButton;

	pTexButton=MakeTCFromFile("Graph\\interface\\menus\\menu_left_1button.bmp",0);
	pTexButton2=MakeTCFromFile("Graph\\interface\\menus\\menu_left_2button.bmp",0);
	pTexButton3=MakeTCFromFile("Graph\\interface\\menus\\menu_left_3button.bmp",0);

	pTexMain=MakeTCFromFile("Graph\\interface\\menus\\menu_left_main.bmp",0);

	pTexGlissiere=MakeTCFromFile("Graph\\interface\\menus\\menu_left_main_glissiere.bmp",0);
	pTexGlissiereButton=MakeTCFromFile("Graph\\interface\\menus\\menu_left_main_glissiere_button.bmp",0);

	vWindowConsoleElement.clear();

	fPosXCalc=((float)-iTailleX);
	fDist=((float)(iTailleX+iPosX));
	fAngle=0.f;

	eCurrentMenuState=NOP;


	float fCalc	= fPosXCalc + (fDist * sin(DEG2RAD(fAngle)));
	ARX_CHECK_INT(fCalc);

	iPosX	= ARX_CLEAN_WARN_CAST_INT(fCalc);


	bChangeConsole=false;
}

//-----------------------------------------------------------------------------

CWindowMenu::~CWindowMenu()
{
	vector<CWindowMenuConsole*>::iterator i;

	for(i=vWindowConsoleElement.begin();i<vWindowConsoleElement.end();i++)
	{
		if( *i )
		{
			delete *i;
			*i = NULL;
		}
	}

	vWindowConsoleElement.clear();

}

//-----------------------------------------------------------------------------

void CWindowMenu::AddConsole(CWindowMenuConsole *_pMenuConsoleElement)
{
	vWindowConsoleElement.insert(vWindowConsoleElement.end(),_pMenuConsoleElement);
	_pMenuConsoleElement->iOldPosX=0;
	_pMenuConsoleElement->iOldPosY=0;
	_pMenuConsoleElement->iPosX=iPosX;
	_pMenuConsoleElement->iPosY=iPosY;
}
//-----------------------------------------------------------------------------

void CWindowMenu::Update(int _iDTime)
{


	float fCalc	= fPosXCalc + (fDist * sin(DEG2RAD(fAngle)));
	ARX_CHECK_INT(fCalc);

	iPosX	= ARX_CLEAN_WARN_CAST_INT(fCalc);


	fAngle += _iDTime * 0.08f;

	if(fAngle>90.f) fAngle=90.f;
}

//-----------------------------------------------------------------------------

MENUSTATE CWindowMenu::Render()
{
	if(WILL_RELOAD_ALL_TEXTURES) return NOP;

	if(bNoMenu) return NOP;

	if(bChangeConsole)
	{
		//TO DO: faire ce que l'on veut

		bChangeConsole=false;
	}

	SETALPHABLEND(GDevice,false);

	D3DTLVERTEX v[4];
	v[0].color = v[1].color = v[2].color = v[3].color = ARX_OPAQUE_WHITE;
	v[0].sz=v[1].sz=v[2].sz=v[3].sz=0.f;	
	v[0].rhw=v[1].rhw=v[2].rhw=v[3].rhw=0.999999f;

	GDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, false);
		GDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
		GDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);

	MENUSTATE eMS=NOP;

	if (bMouseListen)
	{
		vector<CWindowMenuConsole*>::iterator i;


		ARX_CHECK_INT(ARXDiffTimeMenu);
		int iARXDiffTimeMenu	= ARX_CLEAN_WARN_CAST_INT(ARXDiffTimeMenu) ;


		for (i = vWindowConsoleElement.begin(); i != vWindowConsoleElement.end(); ++i)
		{
			if(eCurrentMenuState==(*i)->eMenuState)
			{
				eMS=(*i)->Update(iPosX,iPosY,
					0,
					iARXDiffTimeMenu);

				if(eMS!=NOP)
				{
					break;
				}
			}
		}
	}

	vector<CWindowMenuConsole*>::iterator i;

	for (i = vWindowConsoleElement.begin(); i != vWindowConsoleElement.end(); ++i)
	{
		if(eCurrentMenuState==(*i)->eMenuState)
		{
			int iNbHide;

			if(iNbHide=((*i)->Render()))
			{
				SETALPHABLEND(GDevice,false);
			}

			break;
		}
	}

	SETALPHABLEND(GDevice,false);

	if(eMS!=NOP)
	{
		eCurrentMenuState=eMS;
		bChangeConsole=true;
	}

	return eMS;
}

//-----------------------------------------------------------------------------

CWindowMenuConsole::CWindowMenuConsole(int _iPosX,int _iPosY,int _iWidth,int _iHeight,MENUSTATE _eMenuState) :
	bMouseListen (true),
	bEdit (false),
	iInterligne (10),
	lData(0),
	pData(NULL)
{
	iOX=(int)RATIO_X(_iPosX);
	iOY=(int)RATIO_Y(_iPosY);
	iWidth=(int)RATIO_X(_iWidth);
	iHeight=(int)RATIO_Y(_iHeight);

	eMenuState=_eMenuState;

	pTexBackground = MakeTCFromFile("Graph\\interface\\menus\\menu_console_background.bmp",0);
	pTexBackgroundBorder = MakeTCFromFile("Graph\\interface\\menus\\menu_console_background_border.bmp",0);

	bFrameOdd=false;

	iPosMenu=-1;
}

//-----------------------------------------------------------------------------

void CWindowMenuConsole::AddMenu(CMenuElement *_pMenuElement)
{
	_pMenuElement->ePlace=NOCENTER;

	_pMenuElement->Move(iOX,iOY);
	MenuAllZone.AddZone((CMenuZone*)_pMenuElement);
}

//-----------------------------------------------------------------------------

void CWindowMenuConsole::AddMenuCenterY( CMenuElement * _pMenuElement )
{
	_pMenuElement->ePlace	=	CENTERY;
	int iDy					=	_pMenuElement->rZone.bottom-_pMenuElement->rZone.top;

	int iI					=	MenuAllZone.GetNbZone();

	for( int iJ = 0 ; iJ < iI ; iJ++ )
	{
		iDy +=	iInterligne;
		CMenuZone	*pZone	=	MenuAllZone.GetZoneNum(iJ);
		iDy	+=	pZone->rZone.bottom - pZone->rZone.top;
	}

	int iDepY;
	
	if( iDy < iHeight )
	{
		iDepY = iOY + ( ( iHeight - iDy ) >> 1 );
	}
	else
	{
		iDepY = iOY;
	}

	int dy = 0;
	iI = MenuAllZone.GetNbZone();

	if( iI )
	{
		dy	=	iDepY - MenuAllZone.GetZoneNum(0)->rZone.top;
	}

	//We can't go inside the for-loop
	else 
	{ 
		ARX_CHECK( !( 0 < iI ) );
	}



	for( int iJ = 0 ; iJ < iI ; iJ++ )
	{
		CMenuZone *pZone	=	MenuAllZone.GetZoneNum(iJ);
		iDy					=	pZone->rZone.bottom - pZone->rZone.top;
		iDepY				+=	iDy + iInterligne;
		pZone->Move( 0, dy );
	}

	_pMenuElement->Move( 0, iDepY );

	MenuAllZone.AddZone( (CMenuZone*) _pMenuElement );
}

//-----------------------------------------------------------------------------

void CWindowMenuConsole::AddMenuCenter( CMenuElement * _pMenuElement )
{
	_pMenuElement->ePlace	=	CENTER;

	int	iDx	=	_pMenuElement->rZone.right - _pMenuElement->rZone.left;
	int	dx	=	( ( iWidth - iDx ) >> 1 ) - _pMenuElement->rZone.left;

	if( dx < 0 )
	{
		dx = 0;
	}

	int	iDy	=	_pMenuElement->rZone.bottom - _pMenuElement->rZone.top;
	int	iI	=	MenuAllZone.GetNbZone();
	
	for( int iJ = 0 ; iJ < iI ; iJ++ )
	{
		iDy	+=	iInterligne;
		CMenuZone *pZone	=	MenuAllZone.GetZoneNum(iJ);
		iDy	+=	pZone->rZone.bottom - pZone->rZone.top;
	}

	int iDepY;

	if( iDy < iHeight )
	{
		iDepY	=	iOY + ( ( iHeight - iDy ) >> 1 );
	}
	else
	{
		iDepY	=	iOY;
	}

	int dy = 0;
	iI = MenuAllZone.GetNbZone();

	if( iI )
	{
		dy	=	iDepY - MenuAllZone.GetZoneNum(0)->rZone.top;
	}

	//We can't go inside the for-loop
	else
	{
		ARX_CHECK( !( 0 < iI ) );
	}



	for( int iJ = 0 ; iJ < iI ; iJ++ )
	{
		CMenuZone *pZone = MenuAllZone.GetZoneNum( iJ );
		iDy		=	pZone->rZone.bottom - pZone->rZone.top;
		iDepY	+=	iDy + iInterligne;
		pZone->Move( 0, dy );
	}

	iDx	=	_pMenuElement->rZone.right - _pMenuElement->rZone.left;
	iDy	=	_pMenuElement->rZone.bottom - _pMenuElement->rZone.top;

	_pMenuElement->Move( dx, iDepY );

	MenuAllZone.AddZone( (CMenuZone*) _pMenuElement );
}

//-----------------------------------------------------------------------------

void CWindowMenuConsole::AlignElementCenter(CMenuElement *_pMenuElement)
{
	_pMenuElement->Move(-_pMenuElement->rZone.left, 0);
	_pMenuElement->ePlace=CENTER;

	int iDx = _pMenuElement->rZone.right-_pMenuElement->rZone.left;
	int dx=((iWidth-iDx)>>1)-_pMenuElement->rZone.left;

	if(dx<0)
	{
		dx=0;
	}

	iDx=_pMenuElement->rZone.right-_pMenuElement->rZone.left;

	_pMenuElement->Move(dx,0);
}

//-----------------------------------------------------------------------------

static int scan2ascii(DWORD scancode, unsigned short* result)
{
   static HKL layout=GetKeyboardLayout(0);
   static unsigned char State[256];

   if (GetKeyboardState(State)==FALSE)
      return 0;
 
   UINT vk=MapVirtualKeyEx(scancode,1,layout);
   return ToAsciiEx(vk,scancode,State,result,0,layout);
}

//-----------------------------------------------------------------------------

void CWindowMenuConsole::UpdateText()
{
	if(pGetInfoDirectInput->bTouch)
	{
		pGetInfoDirectInput->iKeyId&=0xFFFF;

		if(	(pGetInfoDirectInput->IsVirtualKeyPressed(DIK_RETURN))||
			(pGetInfoDirectInput->IsVirtualKeyPressed(DIK_NUMPADENTER)) ||
			(pGetInfoDirectInput->IsVirtualKeyPressed(DIK_ESCAPE)) )
		{
			ARX_SOUND_PlayMenu(SND_MENU_CLICK);
			((CMenuElementText*)pZoneClick)->eState=EDIT;

			if(!_tcslen(((CMenuElementText*)pZoneClick)->lpszText))
			{
				_TCHAR szMenuText[256];
				PAK_UNICODE_GetPrivateProfileString(_T("system_menu_editquest_newsavegame"), _T("string"), _T(""), szMenuText, 256, NULL);

				((CMenuElementText*)pZoneClick)->SetText(szMenuText);

				int iDx=pZoneClick->rZone.right-pZoneClick->rZone.left;

				if(pZoneClick->ePlace)
				{
					pZoneClick->rZone.left=iPosX+((iWidth-iDx)>>1);

					if(pZoneClick->rZone.left<0)
					{
						pZoneClick->rZone.left=0;
					}
				}

				pZoneClick->rZone.right=pZoneClick->rZone.left+iDx;
			}

			pZoneClick=NULL;
			bEdit=false;
			return;
		}
		
		bool bKey=false;
		_TCHAR tText[256];
		
		CMenuElementText *pZoneText=(CMenuElementText*)pZoneClick;

		if(pGetInfoDirectInput->IsVirtualKeyPressedOneTouch(DIK_BACKSPACE))
		{
			_tcscpy(tText,pZoneText->lpszText);

			if(_tcslen(tText))
			{
				tText[_tcslen(tText)-1]=_T('\0');
				bKey=true;
			}
		}
		else
		{
			if(pGetInfoDirectInput->IsVirtualKeyPressedOneTouch(pGetInfoDirectInput->iKeyId))
			{
				_tcscpy(tText,pZoneText->lpszText);

				unsigned short tusOutPut[2];
				_TCHAR tCat[2];

				int iKey = pGetInfoDirectInput->iKeyId;
				int iR = scan2ascii(iKey, tusOutPut);

				if(!iR)
				{
					bKey=true;

					//touche non reconnue
					switch(iKey)
					{
					case DIK_NUMPAD0:
						tCat[0]=_T('0');
						tCat[1]=0;
						break;
					case DIK_NUMPAD1:
						tCat[0]=_T('1');
						tCat[1]=0;
						break;
					case DIK_NUMPAD2:
						tCat[0]=_T('2');
						tCat[1]=0;
						break;
					case DIK_NUMPAD3:
						tCat[0]=_T('3');
						tCat[1]=0;
						break;
					case DIK_NUMPAD4:
						tCat[0]=_T('4');
						tCat[1]=0;
						break;
					case DIK_NUMPAD5:
						tCat[0]=_T('5');
						tCat[1]=0;
						break;
					case DIK_NUMPAD6:
						tCat[0]=_T('6');
						tCat[1]=0;
						break;
					case DIK_NUMPAD7:
						tCat[0]=_T('7');
						tCat[1]=0;
						break;
					case DIK_NUMPAD8:
						tCat[0]=_T('8');
						tCat[1]=0;
						break;
					case DIK_NUMPAD9:
						tCat[0]=_T('9');
						tCat[1]=0;
						break;
					case DIK_DECIMAL:     
						tCat[0]=_T('.');
						tCat[1]=0;
						break;
					case DIK_DIVIDE:      
						tCat[0]=_T('/');
						tCat[1]=0;
						break;
					default:
						bKey=false;
						break;
					}
				}
				else
				{
					tCat[0]=_TCHAR(unsigned char(tusOutPut[0]));
					tCat[1]=0;
					bKey=true;
				}

				if(bKey)
				{
					if ((_istalnum(tCat[0]) || _istspace(tCat[0]) || _istpunct(tCat[0])) && (tCat[0]!=_T('\t')) && (tCat[0]!=_T('*')))
						_tcscat(tText,tCat);
				}
			}
		}

		if(bKey)
		{
			pZoneText->SetText(tText);

			if(	(pZoneText->rZone.right-pZoneText->rZone.left)>(iWidth-RATIO_X(64)) )
			{
				tText[_tcslen(tText)-1]=0;
				pZoneText->SetText(tText);
			}
			
			int iDx=pZoneClick->rZone.right-pZoneClick->rZone.left;

			if(pZoneClick->ePlace)
			{
				pZoneClick->rZone.left=iPosX+((iWidth-iDx)>>1);

				if(pZoneClick->rZone.left<0)
				{
					pZoneClick->rZone.left=0;
				}
			}

			pZoneClick->rZone.right=pZoneClick->rZone.left+iDx;
		}
	}
	
	if (pZoneClick->rZone.top == pZoneClick->rZone.bottom)
	{
		int w,h;
		GetTextSize(((CMenuElementText*)pZoneClick)->pHFont, _T("|"), (int*)&w, (int*)&h);
		pZoneClick->rZone.bottom += h;
	}

	//DRAW CURSOR
	D3DTLVERTEX v[4];
	SETTC(GDevice,NULL);
	float col=.5f+rnd()*.5f;
	v[0].color=v[1].color=v[2].color=v[3].color=D3DRGBA(col,col,col,1.f);
	v[0].sz=v[1].sz=v[2].sz=v[3].sz=0.f;	
	v[0].rhw=v[1].rhw=v[2].rhw=v[3].rhw=0.999999f;

	v[0].sx = (float)pZoneClick->rZone.right;
	v[0].sy = (float)pZoneClick->rZone.top;
	v[1].sx = v[0].sx+2.f;
	v[1].sy = v[0].sy;
	v[2].sx = v[0].sx;
	v[2].sy = (float)pZoneClick->rZone.bottom;
	v[3].sx = v[1].sx;
	v[3].sy = v[2].sy;
	EERIEDRAWPRIM(GDevice,D3DPT_TRIANGLESTRIP,D3DFVF_TLVERTEX|D3DFVF_DIFFUSE,v,4,0);
}

//-----------------------------------------------------------------------------

int IsMouseButtonClick()
{
	//MouseButton
	for(int i=DXI_BUTTON0;i<=DXI_BUTTON31;i++)
	{
		if(pGetInfoDirectInput->GetMouseButtonNowPressed(i))
		{
			return DIK_BUTTON1+i-DXI_BUTTON0;
		}
	}

	//Wheel UP/DOWN
	if(pGetInfoDirectInput->iWheelSens<0)
	{
		return DIK_WHEELDOWN;
	}
	else
	{
		if(pGetInfoDirectInput->iWheelSens>0)
		{
			return DIK_WHEELUP;
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------

CMenuElement * CWindowMenuConsole::GetTouch(bool _bValidateTest)
{
	int iMouseButton=0;

	if(	(pGetInfoDirectInput->bTouch)||
		(((iMouseButton = IsMouseButtonClick()) & 0xc0000000)))
	{
		if(!pGetInfoDirectInput->bTouch&&!bMouseAttack)
		{
			bMouseAttack=!bMouseAttack;
			return NULL;
		}

		CMenuElementText *pZoneText=(CMenuElementText*)pZoneClick;

		if(_bValidateTest)
		{
			if( (pZoneClick->iID==BUTTON_MENUOPTIONS_CONTROLS_CUST_ACTIONCOMBINE1)||
				(pZoneClick->iID==BUTTON_MENUOPTIONS_CONTROLS_CUST_ACTIONCOMBINE2))
			{
				bool bOk=true;

				if(	(iMouseButton&0x80000000)&&
					!(iMouseButton&0x40000000) )
				{
					bOk=false;
				}
				else
				{
					for(int iI=DIK_BUTTON1;iI<=DIK_BUTTON32;iI++)
					{
						if(pGetInfoDirectInput->iKeyId==iI)
						{
							bOk=false;
							break;
						}
					}
				}

				if(bOk) return NULL;
			}
		}

		_TCHAR *pText; 

		if (
			(pText=pGetInfoDirectInput->GetFullNameTouch((iMouseButton&0xc0000000)?iMouseButton:pGetInfoDirectInput->iKeyId)) )
		{
			pZoneText->lColorHighlight=pZoneText->lOldColor;

			pZoneText->eState=GETTOUCH;
			pZoneText->SetText(pText);
			free((void*)pText);
			pText = NULL;
			
			int iDx=pZoneClick->rZone.right-pZoneClick->rZone.left;

			if(pZoneClick->ePlace)
			{
				pZoneClick->rZone.left=(iWidth-iDx)>>1;

				if(pZoneClick->rZone.left<0)
				{
					pZoneClick->rZone.left=0;
				}
			}

			pZoneClick->rZone.right=pZoneClick->rZone.left+iDx;

			pZoneClick=NULL;
			bEdit=false;

			if(iMouseButton&0xc0000000)
			{
				pGetInfoDirectInput->iKeyId=iMouseButton;
			}

			bMouseAttack=false;

			return (CMenuElement*)pZoneText;
		}

	}

	return NULL;
}

//-----------------------------------------------------------------------------

MENUSTATE CWindowMenuConsole::Update(int _iPosX,int _iPosY,int _iOffsetY,int _FrameDiff)
{
	bFrameOdd=!bFrameOdd;

	iSavePosY=_iPosY;

	//move les zones
	if(_iOffsetY)
	{
		_iPosY-=(MenuAllZone.GetZoneNum(_iOffsetY)->rZone.top)-(MenuAllZone.GetZoneNum(0)->rZone.top);
	}

	MenuAllZone.Move((iPosX-iOldPosX),(iPosY-iOldPosY));
	
	int iI = MenuAllZone.GetNbZone();

	for(int iJ=0;iJ<iI;++iJ)
	{
		CMenuZone *pZone = MenuAllZone.GetZoneNum(iJ);

		if(	(pZone->rZone.top<iSavePosY)||
			((pZone->rZone.bottom+iInterligne)>(iSavePosY+iHeight)))
		{
			pZone->bActif=false;
		}
		else
		{
			pZone->bActif=true;
		}

		pZone->bActif=true;
	}

	iOldPosX=iPosX;
	iOldPosY=iPosY;
	iPosX=_iPosX;
	iPosY=_iPosY;
	
	// Check if mouse over
	if (bMouseListen)
	{
		if (!bEdit)
		{
			pZoneClick=NULL;
			int iR = MenuAllZone.CheckZone(pGetInfoDirectInput->iMouseAX,pGetInfoDirectInput->iMouseAY);

			if(iR!=-1)
			{
				pZoneClick=(CMenuElement*)iR;

				if( pGetInfoDirectInput->GetMouseButtonDoubleClick(DXI_BUTTON0,300) )
				{
					MENUSTATE e = pZoneClick->eMenuState;
					bEdit = pZoneClick->OnMouseDoubleClick(0);

					if (pZoneClick->iID == BUTTON_MENUEDITQUEST_LOAD)
						return MAIN;

					if(bEdit)
						return pZoneClick->eMenuState;

					return e;
				}

				if( pGetInfoDirectInput->GetMouseButton(DXI_BUTTON0) )
				{
					MENUSTATE e = pZoneClick->eMenuState;
					bEdit = pZoneClick->OnMouseClick(0);
					return e;
				}
				else
				{
					pZoneClick->EmptyFunction();
				}
			}
		}
		else
		{
			if(!pZoneClick)
			{
				int iR = MenuAllZone.CheckZone(pGetInfoDirectInput->iMouseAX,pGetInfoDirectInput->iMouseAY);

				if(iR!=-1)
				{
					pZoneClick=(CMenuElement*)iR;
					
					if( pGetInfoDirectInput->GetMouseButtonDoubleClick(DXI_BUTTON0,300) )
					{
						bEdit = pZoneClick->OnMouseDoubleClick(0);

						if(bEdit)
							return pZoneClick->eMenuState;
					}
				}
			}
		}
	}

	//check les shortcuts
	if(!bEdit)
	{
		iI=MenuAllZone.GetNbZone();

		for(int iJ=0;iJ<iI;++iJ)
		{
			CMenuElement *pMenuElement=(CMenuElement*)MenuAllZone.GetZoneNum(iJ);
			CMenuElement *CMenuElementShortCut;

			if((CMenuElementShortCut=pMenuElement->OnShortCut()))
			{
				pZoneClick=CMenuElementShortCut;
				MENUSTATE e = pZoneClick->eMenuState;
				bEdit = pZoneClick->OnMouseClick(0);
				pZoneClick=CMenuElementShortCut;
				return e;
			}
		}
		}

	return NOP;
}

//-----------------------------------------------------------------------------

static bool UpdateGameKey(bool bEdit,CMenuElement *pmeElement)
{
	bool bChange=false;

	if(	(!bEdit)&&
		(pmeElement) )
	{
		switch(pmeElement->iID)
		{
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_JUMP,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_MAGICMODE1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_MAGICMODE2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_MAGICMODE,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_MAGICMODE1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STEALTHMODE1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STEALTHMODE2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_STEALTHMODE,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_STEALTHMODE1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKFORWARD1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKFORWARD2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_WALKFORWARD,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKFORWARD1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKBACKWARD1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKBACKWARD2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_WALKBACKWARD,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKBACKWARD1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFELEFT1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFELEFT2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_STRAFELEFT,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFELEFT1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFERIGHT1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFERIGHT2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_STRAFERIGHT,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFERIGHT1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANLEFT1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANLEFT2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_LEANLEFT,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANLEFT1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANRIGHT1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANRIGHT2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_LEANRIGHT,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANRIGHT1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCH1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCH2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_CROUCH,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCH1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_MOUSELOOK1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_MOUSELOOK2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_MOUSELOOK,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_MOUSELOOK1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_ACTIONCOMBINE1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_ACTIONCOMBINE2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_ACTION,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_ACTIONCOMBINE1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_INVENTORY1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_INVENTORY2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_INVENTORY,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_INVENTORY1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOK1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOK2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_BOOK,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOK1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKCHARSHEET1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKCHARSHEET2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_BOOKCHARSHEET,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKCHARSHEET1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKSPELL1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKSPELL2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_BOOKSPELL,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKSPELL1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKMAP1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKMAP2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_BOOKMAP,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKMAP1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKQUEST1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKQUEST2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_BOOKQUEST,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKQUEST1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONLIFE1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONLIFE2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_DRINKPOTIONLIFE,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONLIFE1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONMANA1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONMANA2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_DRINKPOTIONMANA,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONMANA1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_TORCH1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_TORCH2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_TORCH,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_TORCH1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CANCELCURSPELL1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CANCELCURSPELL2:	
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_CANCELCURSPELL,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_CANCELCURSPELL1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST1_2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_PRECAST1,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST2:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST2_2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_PRECAST2,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST2,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST3:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST3_2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_PRECAST3,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST3,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_WEAPON1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_WEAPON2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_WEAPON,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_WEAPON1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKLOAD:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKLOAD2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_QUICKLOAD,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKLOAD,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKSAVE:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKSAVE2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_QUICKSAVE,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKSAVE,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNLEFT1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNLEFT2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_TURNLEFT,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNLEFT1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNRIGHT1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNRIGHT2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_TURNRIGHT,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNRIGHT1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKUP1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKUP2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_LOOKUP,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKUP1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKDOWN1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKDOWN2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_LOOKDOWN,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKDOWN1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFE1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFE2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_STRAFE,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFE1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CENTERVIEW1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CENTERVIEW2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_CENTERVIEW,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_CENTERVIEW1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_FREELOOK1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_FREELOOK2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_FREELOOK,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_FREELOOK1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PREVIOUS1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PREVIOUS2:
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_PREVIOUS,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_PREVIOUS1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_NEXT1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_NEXT2:	
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_NEXT,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_NEXT1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCHTOGGLE1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCHTOGGLE2:	
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_CROUCHTOGGLE,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCHTOGGLE1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_UNEQUIPWEAPON1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_UNEQUIPWEAPON2:	
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_UNEQUIPWEAPON,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_UNEQUIPWEAPON1,pGetInfoDirectInput->iKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_MINIMAP1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_MINIMAP2:	
			bChange=pMenuConfig->SetActionKey(CONTROLS_CUST_MINIMAP,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_MINIMAP1,pGetInfoDirectInput->iKeyId);
			break;
		}
	}

	return bChange;
}

//-----------------------------------------------------------------------------

int CWindowMenuConsole::Render()
{
	if(WILL_RELOAD_ALL_TEXTURES) return 0;

	if(bNoMenu) return 0;

	int iSlider=0;

	SETALPHABLEND(GDevice,TRUE);

	//------------------------------------------------------------------------
	//Affichage de la console

	GDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO);
	GDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR);

	GDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, false);
	EERIEDrawBitmap2(GDevice, ARX_CLEAN_WARN_CAST_FLOAT(iPosX), ARX_CLEAN_WARN_CAST_FLOAT(iSavePosY),
		RATIO_X(pTexBackground->m_dwWidth), RATIO_Y(pTexBackground->m_dwHeight),
		0, pTexBackground, ARX_OPAQUE_WHITE);

	danaeApp.EnableZBuffer();

	GDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
	GDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);

	SETALPHABLEND(GDevice, false);

	SETALPHABLEND(GDevice,FALSE);
	EERIEDrawBitmap2(GDevice, ARX_CLEAN_WARN_CAST_FLOAT(iPosX), ARX_CLEAN_WARN_CAST_FLOAT(iSavePosY),
		RATIO_X(pTexBackgroundBorder->m_dwWidth), RATIO_Y(pTexBackgroundBorder->m_dwHeight),
		0, pTexBackgroundBorder, ARX_OPAQUE_WHITE);

	//------------------------------------------------------------------------

	int t = MenuAllZone.GetNbZone();


	ARX_CHECK_INT(ARXDiffTimeMenu);
	int iARXDiffTimeMenu	= ARX_CLEAN_WARN_CAST_INT(ARXDiffTimeMenu);


	for(int i=0;i<t;++i)
	{
		CMenuElement *pMe=(CMenuElement*)MenuAllZone.GetZoneNum(i);

		if(pMe->bActif)
		{
			pMe->Update(iARXDiffTimeMenu);
			pMe->Render();
		}
		else
		{
			iSlider++;
		}
	}

	//HIGHLIGHT
	if(pZoneClick && pZoneClick->bActif)
	{
		bool bReInit=false;

		pZoneClick->RenderMouseOver();

		switch(pZoneClick->eState)
		{
		case EDIT_TIME:
			UpdateText();
			break;
		case GETTOUCH_TIME:
			{
				if(bFrameOdd)
					((CMenuElementText*)pZoneClick)->lColorHighlight=RGB(255.f, 0, 0);
				else
					((CMenuElementText*)pZoneClick)->lColorHighlight=RGB(50.f, 0, 0);

				bool bOldTouch=pGetInfoDirectInput->bTouch;

				if(	pGetInfoDirectInput->IsVirtualKeyPressed(DIK_LSHIFT)||
					pGetInfoDirectInput->IsVirtualKeyPressed(DIK_RSHIFT)||
					pGetInfoDirectInput->IsVirtualKeyPressed(DIK_LCONTROL)||
					pGetInfoDirectInput->IsVirtualKeyPressed(DIK_RCONTROL)||
					pGetInfoDirectInput->IsVirtualKeyPressed(DIK_LALT)||
					pGetInfoDirectInput->IsVirtualKeyPressed(DIK_RALT) )
				{
					if(!((pGetInfoDirectInput->iKeyId&~0x8000FFFF)>>16))
						pGetInfoDirectInput->bTouch=false;
				}
				else
				{
					if(pGetInfoDirectInput->IsVirtualKeyPressedNowUnPressed(DIK_LSHIFT))
					{
						pGetInfoDirectInput->bTouch=true;
						pGetInfoDirectInput->iKeyId=DIK_LSHIFT;
					}

					if(pGetInfoDirectInput->IsVirtualKeyPressedNowUnPressed(DIK_RSHIFT))
					{
						pGetInfoDirectInput->bTouch=true;
						pGetInfoDirectInput->iKeyId=DIK_RSHIFT;
					}

					if(pGetInfoDirectInput->IsVirtualKeyPressedNowUnPressed(DIK_LCONTROL))
					{
						pGetInfoDirectInput->bTouch=true;
						pGetInfoDirectInput->iKeyId=DIK_LCONTROL;
					}

					if(pGetInfoDirectInput->IsVirtualKeyPressedNowUnPressed(DIK_RCONTROL))
					{
						pGetInfoDirectInput->bTouch=true;
						pGetInfoDirectInput->iKeyId=DIK_RCONTROL;
					}

					if(pGetInfoDirectInput->IsVirtualKeyPressedNowUnPressed(DIK_LALT))
					{
						pGetInfoDirectInput->bTouch=true;
						pGetInfoDirectInput->iKeyId=DIK_LALT;
					}

					if(pGetInfoDirectInput->IsVirtualKeyPressedNowUnPressed(DIK_RALT))
					{
						pGetInfoDirectInput->bTouch=true;
						pGetInfoDirectInput->iKeyId=DIK_RALT;
					}
				}

				CMenuElement *pmeElement=GetTouch(true);
				pGetInfoDirectInput->bTouch=bOldTouch;

				if(pmeElement)
				{
					if(UpdateGameKey(bEdit,pmeElement))
					{
						bReInit=true;
					}
				}
			}
			break;
		default:
			{
				if(pGetInfoDirectInput->GetMouseButtonNowPressed(DXI_BUTTON0))
				{
					CMenuZone *pmzMenuZone = MenuAllZone.GetZoneWithID(BUTTON_MENUOPTIONS_CONTROLS_CUST_DEFAULT);

					if(pmzMenuZone==pZoneClick)
					{
						pMenuConfig->SetDefaultKey();
						bReInit=true;
					}
				}
			}
			break;
		}

		if(bReInit)
		{
			pMenuConfig->ReInitActionKey(this);
			bMouseAttack=false;
		}
	}

	//DEBUG ZONE
	MenuAllZone.DrawZone();

	return iSlider;
}

//-----------------------------------------------------------------------------

CMenuPanel::CMenuPanel() : CMenuElement(NOP)
{
	vElement.clear();

	iId = (int) this;
}

//-----------------------------------------------------------------------------

CMenuPanel::~CMenuPanel()
{
	vector<CMenuElement*>::iterator i;

	for(i=vElement.begin();i<vElement.end();++i)
	{
		delete (*i);
		*i = NULL;
	}

	vElement.clear();
}

//-----------------------------------------------------------------------------

void CMenuPanel::Move(int _iX, int _iY)
{
	rZone.left += _iX;
	rZone.top += _iY;
	rZone.right += _iX;
	rZone.bottom += _iY;

	vector<CMenuElement*>::iterator i;

	for(i=vElement.begin();i!=vElement.end();++i)
	{
		(*i)->Move(_iX, _iY);
	}
}

//-----------------------------------------------------------------------------
// patch on ajoute à droite en ligne
void CMenuPanel::AddElement(CMenuElement* _pElem)
{
	vElement.insert(vElement.end(), _pElem);

	if(vElement.size()==1)
	{
		rZone=_pElem->rZone;
	}
	else
	{
		rZone.left = min(rZone.left, _pElem->rZone.left);
		rZone.top = min(rZone.top, _pElem->rZone.top);
	}

	// + taille elem
	rZone.right = max(rZone.right, _pElem->rZone.right);
	rZone.bottom = max(rZone.bottom, _pElem->rZone.bottom);

	_pElem->Move(0, ((GetHeight() - _pElem->rZone.bottom) / 2));
}

//-----------------------------------------------------------------------------
// patch on ajoute à droite en ligne
void CMenuPanel::AddElementNoCenterIn(CMenuElement* _pElem)
{
	vElement.insert(vElement.end(), _pElem);

	if(vElement.size()==1)
	{
		rZone=_pElem->rZone;
	}
	else
	{
		rZone.left = min(rZone.left, _pElem->rZone.left);
		rZone.top = min(rZone.top, _pElem->rZone.top);
	}

	// + taille elem
	rZone.right = max(rZone.right, _pElem->rZone.right);
	rZone.bottom = max(rZone.bottom, _pElem->rZone.bottom);

}

//-----------------------------------------------------------------------------

CMenuElement* CMenuPanel::OnShortCut()
{
	vector<CMenuElement*>::iterator i;

	for(i=vElement.begin();i!=vElement.end();++i)
	{
		if((*i)->OnShortCut())
		{
			return *i;
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------

void CMenuPanel::Update(int _iTime)
{
	rZone.right = rZone.left;
	rZone.bottom = rZone.top;

	vector<CMenuElement*>::iterator i;

	for(i=vElement.begin();i!=vElement.end();++i)
	{
		(*i)->Update(_iTime);
		rZone.right = max(rZone.right, (*i)->rZone.right);
		rZone.bottom = max(rZone.bottom, (*i)->rZone.bottom);
	}
}

//-----------------------------------------------------------------------------

void CMenuPanel::Render()
{
	if(WILL_RELOAD_ALL_TEXTURES) return;

	if(bNoMenu) return;

	vector<CMenuElement*>::iterator i;

	for(i=vElement.begin();i!=vElement.end();++i)
	{
		(*i)->Render();
	}
}

//-----------------------------------------------------------------------------

CMenuZone * CMenuPanel::GetZoneWithID(int _iID)
{
	vector<CMenuElement*>::iterator i;

	for(i=vElement.begin();i!=vElement.end();++i)
	{
		CMenuZone *pZone;
		pZone=(*i)->GetZoneWithID(_iID);

		if(pZone) return pZone;
	}

	return NULL;
}

//-----------------------------------------------------------------------------

long CMenuPanel::IsMouseOver(int _iX, int _iY)
{
	if ((_iX >= rZone.left) &&
		(_iY >= rZone.top) &&
		(_iX <= rZone.right) &&
		(_iY <= rZone.bottom))
	{
		vector<CMenuElement *>::iterator i;
		
		for(i=vElement.begin();i!=vElement.end();++i)
		{
			if(	(*i)->bCheck &&
				(*i)->bActif && 
				(_iX >= (*i)->rZone.left) &&
				(_iY >= (*i)->rZone.top) &&
				(_iX <= (*i)->rZone.right) &&
				(_iY <= (*i)->rZone.bottom))
				return (*i)->iId;
		}
	}

	return -1;
}

//-----------------------------------------------------------------------------

CMenuButton::CMenuButton(int _iID, HFONT _pHFont,MENUSTATE _eMenuState,int _iPosX,int _iPosY,_TCHAR *_pText,float _fSize,TextureContainer *_pTex,TextureContainer *_pTexOver,int _iColor,int _iTailleX,int _iTailleY)
: CMenuElement(_eMenuState)
{
	iID = _iID;
	pHFont = _pHFont;
	fSize=_fSize;

	rZone.left=_iPosX;
	rZone.top=_iPosY;
	rZone.right  = rZone.left ;
	rZone.bottom = rZone.top ;

	vText.clear();
	iPos=0;

	if(_pText)
	{
		AddText(_pText);
	}

	pTex=_pTex;
	pTexOver=_pTexOver;


	if (pTex)
	{
		float rZoneR = rZone.left + RATIO_X(pTex->m_dwWidth);
		float rZoneB = rZone.top + RATIO_Y(pTex->m_dwHeight);

		ARX_CHECK_LONG( rZoneR );
		ARX_CHECK_LONG( rZoneB );

		rZone.right  = max(rZone.right,  ARX_CLEAN_WARN_CAST_LONG(rZoneR) );
		rZone.bottom = max(rZone.bottom, ARX_CLEAN_WARN_CAST_LONG(rZoneB) );
	}

	if (pTexOver)
	{
		float rZoneR = rZone.left + RATIO_X(pTexOver->m_dwWidth);
		float rZoneB = rZone.top + RATIO_Y(pTexOver->m_dwHeight);

		ARX_CHECK_LONG( rZoneR );
		ARX_CHECK_LONG( rZoneB );

		rZone.right  = max(rZone.right, ARX_CLEAN_WARN_CAST_LONG(rZoneR) );
		rZone.bottom = max(rZone.bottom, ARX_CLEAN_WARN_CAST_LONG(rZoneB) );
	}



	iColor=_iColor;

	iId=(int)this;
}

//-----------------------------------------------------------------------------

CMenuButton::~CMenuButton()
{
	vector<_TCHAR*>::iterator i;

	for(i=vText.begin();i!=vText.end();++i)
	{
		free((void*)(*i));
	}

	vText.clear();
}

//-----------------------------------------------------------------------------

void CMenuButton::SetPos(int _iX,int _iY)
{
	CMenuZone::SetPos(ARX_CLEAN_WARN_CAST_FLOAT(_iX), ARX_CLEAN_WARN_CAST_FLOAT(_iY));

	int iWidth		= 0;
	int iHeight		= 0;

	if (pTex)
	{

		iWidth  = max ( ARX_CAST_UINT( iWidth ), pTex->m_dwWidth );
		iHeight = max ( ARX_CAST_UINT( iHeight ), pTex->m_dwHeight );

		float fRatioX = RATIO_X(iWidth);
		float fRatioY = RATIO_Y(iHeight);
		ARX_CHECK_INT(fRatioX);
		ARX_CHECK_INT(fRatioY);

		iWidth  = ARX_CLEAN_WARN_CAST_INT(fRatioX);
		iHeight = ARX_CLEAN_WARN_CAST_INT(fRatioY);


	}

	int iWidth2		= 0;
	int iHeight2	= 0;

	if (pTexOver)
	{
		iWidth2  = max ( ARX_CAST_UINT( iWidth2 ), pTexOver->m_dwWidth );
		iHeight2 = max ( ARX_CAST_UINT( iHeight2 ), pTexOver->m_dwHeight );

		float fRatioX = RATIO_X(iWidth2) ;
		float fRatioY = RATIO_Y(iHeight2);
		ARX_CHECK_INT(fRatioX);
		ARX_CHECK_INT(fRatioY);

		iWidth2  = ARX_CLEAN_WARN_CAST_INT(fRatioX);
		iHeight2 = ARX_CLEAN_WARN_CAST_INT(fRatioY);

	}

	rZone.right		= _iX + max(iWidth,iWidth2);
	rZone.bottom	= _iY + max(iHeight,iHeight2);
}

//-----------------------------------------------------------------------------

void CMenuButton::AddText(_TCHAR *_pText)
{
	if (!_pText) return;

	_TCHAR * pText2=_tcsdup(_pText);
	vText.insert(vText.end(),pText2);

	int iSizeXButton=rZone.right-rZone.left;
	int iSizeYButton=rZone.bottom-rZone.top;
	int iSizeX=0;
	int iSizeY=0;
	GetTextSize(pHFont, _pText, &iSizeX, &iSizeY);

	if(iSizeX>iSizeXButton) iSizeXButton=iSizeX;

	if(iSizeY>iSizeYButton) iSizeYButton=iSizeY;

	rZone.right=rZone.left+iSizeXButton;
	rZone.bottom=rZone.top+iSizeYButton;
}

//-----------------------------------------------------------------------------

bool CMenuButton::OnMouseClick(int _iMouseButton)
{
	iPos++;

	ARX_CHECK_NOT_NEG( iPos );

	if( ARX_CAST_UINT( iPos ) >= vText.size() )
	{
		iPos = 0;
	}



	ARX_SOUND_PlayMenu(SND_MENU_CLICK);

	return false;
}

//-----------------------------------------------------------------------------

void CMenuButton::Update(int _iDTime)
{
}

//-----------------------------------------------------------------------------

void CMenuButton::Render()
{
	if(WILL_RELOAD_ALL_TEXTURES) return;

	if(bNoMenu) return;

	//affichage de la texture
	if(pTex)
	{
		EERIEDrawBitmap2(GDevice, ARX_CLEAN_WARN_CAST_FLOAT(rZone.left), ARX_CLEAN_WARN_CAST_FLOAT(rZone.top),
			RATIO_X(pTex->m_dwWidth),
			RATIO_Y(pTex->m_dwHeight),
			0,
			pTex,
			ARX_OPAQUE_WHITE);
	}

	//affichage de la font
	if(vText.size())
	{
		_TCHAR *pText=vText[iPos];

		GDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE,  true);
		GDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
		GDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND,  D3DBLEND_ONE);

		EERIE_3D ePos;
		ePos.x = (float)rZone.left;
		ePos.y = (float)rZone.top;
		ePos.z = 1;
		
		FontRenderText(pHFont, ePos, pText, RGB(232, 204, 142));

		GDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE,  false);
	}
}

//-----------------------------------------------------------------------------

void CMenuButton::RenderMouseOver()
{
	if(WILL_RELOAD_ALL_TEXTURES) return;

	if(bNoMenu) return;

	pGetInfoDirectInput->SetMouseOver();

	//affichage de la texture
	if(pTexOver)
	{
		D3DTLVERTEX v[4];
		v[0].color = v[1].color = v[2].color = v[3].color = ARX_OPAQUE_WHITE;
		v[0].sz=v[1].sz=v[2].sz=v[3].sz=0.f;	
		v[0].rhw=v[1].rhw=v[2].rhw=v[3].rhw=0.999999f;

		SETTC(GDevice,pTexOver);
		v[0].sx = (float)rZone.left;
		v[0].sy = (float)rZone.top;
		v[0].tu = 0.f;
		v[0].tv = 0.f;
		v[1].sx = (float)(rZone.right);
		v[1].sy = v[0].sy;
		v[1].tu = 0.999999f;
		v[1].tv = 0.f;
		v[2].sx = v[0].sx;
		v[2].sy = (float)(rZone.bottom);
		v[2].tu = 0.f;
		v[2].tv = 0.999999f;
		v[3].sx = v[1].sx;
		v[3].sy = v[2].sy;
		v[3].tu = 0.999999f;
		v[3].tv = 0.999999f;
		EERIEDRAWPRIM(GDevice,D3DPT_TRIANGLESTRIP,D3DFVF_TLVERTEX|D3DFVF_DIFFUSE,v,4,0);
	}

	if( vText.size() )
	{
		_TCHAR *pText=vText[iPos];

		GDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE,  true);
		GDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
		GDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND,  D3DBLEND_ONE);
		
		EERIE_3D ePos;
		ePos.x = (float)rZone.left;
		ePos.y = (float)rZone.top;
		ePos.z = 1;
		
		FontRenderText(pHFont, ePos, pText, RGB(255, 255, 255));
		
		GDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE,  false);
	}
}

//-----------------------------------------------------------------------------

CMenuSliderText::CMenuSliderText(int _iID, int _iPosX, int _iPosY)
: CMenuElement(NOP)
{
	iID = _iID;
	TextureContainer *pTex = MakeTCFromFile("\\Graph\\interface\\menus\\menu_slider_button_left.bmp");
	pLeftButton = new CMenuButton(-1, hFontMenu, NOP, _iPosX, _iPosY, NULL, 1, pTex, pTex, -1, pTex?pTex->m_dwWidth:0, pTex->m_dwHeight);
	pTex = MakeTCFromFile("\\Graph\\interface\\menus\\menu_slider_button_right.bmp");
	pRightButton = new CMenuButton(-1, hFontMenu, NOP, _iPosX, _iPosY, NULL, 1, pTex, pTex, -1, pTex?pTex->m_dwWidth:0, pTex->m_dwHeight);

	vText.clear();

	iPos = 0;
	iOldPos = -1;

	rZone.left   = _iPosX;
	rZone.top    = _iPosY;
	rZone.right  = _iPosX + pLeftButton->GetWidth() + pRightButton->GetWidth();
	rZone.bottom = _iPosY + max(pLeftButton->GetHeight(), pRightButton->GetHeight());
	
	iId = (int) this;
}

//-----------------------------------------------------------------------------

CMenuSliderText::~CMenuSliderText()
{
	if (pLeftButton)
	{
		delete pLeftButton;
		pLeftButton = NULL;
	}

	if (pRightButton)
	{
		delete pRightButton;
		pRightButton = NULL;
	}

	vector<CMenuElementText*>::iterator i;

	for(i=vText.begin();i!=vText.end();++i)
	{
		delete (*i);
		*i = NULL;
	}
}

//-----------------------------------------------------------------------------

void CMenuSliderText::SetWidth(int _iWidth)
{
	rZone.right  = max(rZone.right, rZone.left +  _iWidth);
	pRightButton->SetPos(rZone.right - pRightButton->GetWidth(), pRightButton->rZone.top);

	int dx=rZone.right-rZone.left-pLeftButton->GetWidth()-pRightButton->GetWidth();
	//on recentre tout
	vector<CMenuElementText*>::iterator it;

	for(it=vText.begin();it<vText.end();it++)
	{
		CMenuElementText *pMenuElementText=*it;
		int x, y;
		GetTextSize(pMenuElementText->pHFont, pMenuElementText->lpszText, &x, &y);

		int dxx=(dx-x)>>1;
		pMenuElementText->SetPos(ARX_CLEAN_WARN_CAST_FLOAT(pLeftButton->rZone.right + dxx), ARX_CLEAN_WARN_CAST_FLOAT(rZone.top));
	}
}

//-----------------------------------------------------------------------------

void CMenuSliderText::AddText(CMenuElementText *_pText)
{
	_pText->Move(rZone.left + pLeftButton->GetWidth(), rZone.top + 0);
	vText.insert(vText.end(), _pText);

	int x,y;
	GetTextSize(_pText->pHFont, _pText->lpszText, &x, &y);

	rZone.right  = max(rZone.right, rZone.left + pLeftButton->GetWidth() + pRightButton->GetWidth() + x);
	rZone.bottom = max(rZone.bottom, rZone.top + y);

	pLeftButton->SetPos(rZone.left, rZone.top+(y>>2));
	pRightButton->SetPos(rZone.right-pRightButton->GetWidth(), rZone.top+(y>>2));

	int dx=rZone.right-rZone.left-pLeftButton->GetWidth()-pRightButton->GetWidth();
	//on recentre tout
	vector<CMenuElementText*>::iterator it;

	for(it=vText.begin();it<vText.end();it++)
	{
		CMenuElementText *pMenuElementText=*it;
		GetTextSize(pMenuElementText->pHFont, pMenuElementText->lpszText, &x, &y);

		int dxx=(dx-x)>>1;
		pMenuElementText->SetPos(ARX_CLEAN_WARN_CAST_FLOAT(pLeftButton->rZone.right + dxx), ARX_CLEAN_WARN_CAST_FLOAT(rZone.top));
	}

}

//-----------------------------------------------------------------------------

void CMenuSliderText::Move(int _iX, int _iY)
{
	CMenuZone::Move(_iX, _iY);

	pLeftButton->Move(_iX, _iY);
	pRightButton->Move(_iX, _iY);

	vector<CMenuElementText*>::iterator i;

	for(i=vText.begin();i!=vText.end();++i)
	{
		(*i)->Move(_iX, _iY);
	}
}

//-----------------------------------------------------------------------------

void CMenuSliderText::EmptyFunction()
{
	//Touche pour la selection
	if(pGetInfoDirectInput->IsVirtualKeyPressedNowPressed(DIK_LEFT))
	{
		iPos--;

		if (iPos <= 0) iPos = 0;
	}
	else
	{
		if( pGetInfoDirectInput->IsVirtualKeyPressedNowPressed( DIK_RIGHT ) )
		{
			iPos++;

			ARX_CHECK_NOT_NEG(iPos);

			if ( ARX_CAST_UINT( iPos ) >= vText.size() - 1 ) iPos = vText.size() - 1;


		}
	}
}

//-----------------------------------------------------------------------------

bool CMenuSliderText::OnMouseClick(int)
{

	ARX_SOUND_PlayMenu(SND_MENU_CLICK);

	if(iOldPos<0)
		iOldPos=iPos;

	int iX = pGetInfoDirectInput->iMouseAX;
	int iY = pGetInfoDirectInput->iMouseAY;

	if ((iX >= rZone.left) &&
		(iY >= rZone.top) &&
		(iX <= rZone.right) &&
		(iY <= rZone.bottom))
	{
		if ((iX >= pLeftButton->rZone.left) &&
			(iY >= pLeftButton->rZone.top) &&
			(iX <= pLeftButton->rZone.right) &&
			(iY <= pLeftButton->rZone.bottom))
		{
			iPos--;

			if (iPos <= 0) iPos = 0;
		}
		else if ((iX >= pRightButton->rZone.left) &&
				(iY >= pRightButton->rZone.top) &&
				(iX <= pRightButton->rZone.right) &&
				(iY <= pRightButton->rZone.bottom))
			{
				iPos++;

			ARX_CHECK_NOT_NEG(iPos);

			if ( ARX_CAST_UINT( iPos ) >= vText.size() - 1 ) iPos = vText.size() - 1 ;


			}
	}

	switch (iID)
	{
	// MENUOPTIONS_VIDEO
	case BUTTON_MENUOPTIONSVIDEO_RESOLUTION:
		{
			_TCHAR *pcText;
			pcText=(vText.at(iPos))->lpszText;
			int iX = pMenuConfig->iWidth;
			int iY = pMenuConfig->iHeight;
			_stscanf(pcText, _T("%dx%d"), &iX, &iY);
			{
				pMenuConfig->iNewWidth = iX;
				pMenuConfig->iNewHeight = iY;
				pMenuConfig->bChangeResolution = true;
			}
		}
		break;
	// MENUOPTIONS_VIDEO
	case BUTTON_MENUOPTIONSVIDEO_BPP:
		{
			_TCHAR *pcText;
			pcText = vText[iPos]->lpszText;
			pMenuConfig->iNewBpp=_ttoi(pcText);
			pMenuConfig->bChangeResolution = true;
		}
		break;
	case BUTTON_MENUOPTIONSVIDEO_TEXTURES:
		{
			{
				pMenuConfig->iNewTextureResol = iPos;
				pMenuConfig->bChangeTextures = true;
			}

		}
		break;
	case BUTTON_MENUOPTIONSVIDEO_LOD:
		{
			ARXMenu_Options_Video_SetLODQuality(iPos);
		}
		break;
	case BUTTON_MENUOPTIONSVIDEO_OTHERSDETAILS:
		{
			ARXMenu_Options_Video_SetDetailsQuality(iPos);
		}
		break;
	}

	return false;
}

//-----------------------------------------------------------------------------

void CMenuSliderText::Update(int _iTime)
{
	pLeftButton->Update(_iTime);
	pRightButton->Update(_iTime);
}

//-----------------------------------------------------------------------------

void CMenuSliderText::Render()
{
	if(WILL_RELOAD_ALL_TEXTURES) return;

	if(bNoMenu) return;

	pLeftButton->Render();

	pRightButton->Render();

	GDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, false);

	if(vText[iPos])
	{
		vText[iPos]->Render();

	}

	GDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, false);

}

//-----------------------------------------------------------------------------

void CMenuSliderText::RenderMouseOver()
{
	if(WILL_RELOAD_ALL_TEXTURES) return;

	if(bNoMenu) return;

	pGetInfoDirectInput->SetMouseOver();

	int iX = pGetInfoDirectInput->iMouseAX;
	int iY = pGetInfoDirectInput->iMouseAY;

	GDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, true);
	GDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
	GDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);

	if ((iX >= rZone.left) &&
		(iY >= rZone.top) &&
		(iX <= rZone.right) &&
		(iY <= rZone.bottom))
	{
		if ((iX >= pLeftButton->rZone.left) &&
			(iY >= pLeftButton->rZone.top) &&
			(iX <= pLeftButton->rZone.right) &&
			(iY <= pLeftButton->rZone.bottom))
		{
			pLeftButton->Render();

		}
		else if ((iX >= pRightButton->rZone.left) &&
				(iY >= pRightButton->rZone.top) &&
				(iX <= pRightButton->rZone.right) &&
				(iY <= pRightButton->rZone.bottom))
			{
				pRightButton->Render();

			}
	}
}

//-----------------------------------------------------------------------------

CMenuSlider::CMenuSlider(int _iID, int _iPosX, int _iPosY)
: CMenuElement(NOP)
{
	iID = _iID;

	TextureContainer *pTexL = MakeTCFromFile("\\Graph\\interface\\menus\\menu_slider_button_left.bmp");
	TextureContainer *pTexR = MakeTCFromFile("\\Graph\\interface\\menus\\menu_slider_button_right.bmp");
	pLeftButton = new CMenuButton(-1, hFontMenu, NOP, _iPosX, _iPosY, NULL, 1, pTexL, pTexR, -1, pTexL->m_dwWidth, pTexL->m_dwHeight);
	pRightButton = new CMenuButton(-1, hFontMenu, NOP, _iPosX, _iPosY, NULL, 1, pTexR, pTexL, -1, pTexR->m_dwWidth, pTexR->m_dwHeight);
	pTex1 = MakeTCFromFile("\\Graph\\interface\\menus\\menu_slider_on.bmp");
	pTex2 = MakeTCFromFile("\\Graph\\interface\\menus\\menu_slider_off.bmp");

	iPos = 0;

	rZone.left   = _iPosX;
	rZone.top    = _iPosY;
	rZone.right  = _iPosX + pLeftButton->GetWidth() + pRightButton->GetWidth() + 10*max(pTex1->m_dwWidth, pTex2->m_dwWidth);
	rZone.bottom = _iPosY + max(pLeftButton->GetHeight(), pRightButton->GetHeight());

	ARX_CHECK_NOT_NEG( rZone.bottom );
	rZone.bottom = max( ARX_CAST_ULONG( rZone.bottom ), max( pTex1->m_dwHeight, pTex2->m_dwHeight ) );


	pRightButton->Move(pLeftButton->GetWidth() + 10*max(pTex1->m_dwWidth, pTex2->m_dwWidth), 0);

	iId = (int) this;
}

//-----------------------------------------------------------------------------

CMenuSlider::~CMenuSlider()
{
	if (pLeftButton)
	{
		delete pLeftButton;
		pLeftButton = NULL;
	}

	if (pRightButton)
	{
		delete pRightButton;
		pRightButton = NULL;
	}
}

//-----------------------------------------------------------------------------

void CMenuSlider::Move(int _iX, int _iY)
{
	CMenuZone::Move(_iX, _iY);

	pLeftButton->Move(_iX, _iY);
	pRightButton->Move(_iX, _iY);
}

//-----------------------------------------------------------------------------

void CMenuSlider::EmptyFunction()
{
	//Touche pour la selection
	if(pGetInfoDirectInput->IsVirtualKeyPressedNowPressed(DIK_LEFT))
	{
		iPos--;

		if (iPos <= 0) iPos = 0;
	}
	else
	{
		if(pGetInfoDirectInput->IsVirtualKeyPressedNowPressed(DIK_RIGHT))
		{
			iPos++;

			if (iPos >= 10) iPos = 10;
		}
	}

}

//-----------------------------------------------------------------------------

bool CMenuSlider::OnMouseClick(int)
{
	ARX_SOUND_PlayMenu(SND_MENU_CLICK);

	int iX = pGetInfoDirectInput->iMouseAX;
	int iY = pGetInfoDirectInput->iMouseAY;

	if ((iX >= rZone.left) &&
		(iY >= rZone.top) &&
		(iX <= rZone.right) &&
		(iY <= rZone.bottom))
	{
		if ((iX >= pLeftButton->rZone.left) &&
			(iY >= pLeftButton->rZone.top) &&
			(iX <= pLeftButton->rZone.right) &&
			(iY <= pLeftButton->rZone.bottom))
		{
			iPos--;

			if (iPos <= 0) iPos = 0;
		}
		else if ((iX >= pRightButton->rZone.left) &&
				(iY >= pRightButton->rZone.top) &&
				(iX <= pRightButton->rZone.right) &&
				(iY <= pRightButton->rZone.bottom))
			{
				iPos++;

				if (iPos >= 10) iPos = 10;
			}
	}

	switch (iID)
	{
	// MENUOPTIONS_VIDEO
	case BUTTON_MENUOPTIONSVIDEO_FOG:
		{
			ARXMenu_Options_Video_SetFogDistance(iPos);
		}
		break;
	case BUTTON_MENUOPTIONSVIDEO_GAMMA:
		{
			ARXMenu_Options_Video_SetGamma(iPos);
		}
		break;
	case BUTTON_MENUOPTIONSVIDEO_LUMINOSITY:
		{
			ARXMenu_Options_Video_SetLuminosity(iPos);
		}
		break;
	case BUTTON_MENUOPTIONSVIDEO_CONTRAST:
		{
			ARXMenu_Options_Video_SetContrast(iPos);
		}
		break;
	// MENUOPTIONS_AUDIO
	case BUTTON_MENUOPTIONSAUDIO_MASTER:
		{
			ARXMenu_Options_Audio_SetMasterVolume(iPos);
		}
		break;
	case BUTTON_MENUOPTIONSAUDIO_SFX:
		{
			ARXMenu_Options_Audio_SetSfxVolume(iPos);
		}
		break;
	case BUTTON_MENUOPTIONSAUDIO_SPEECH:
		{
			ARXMenu_Options_Audio_SetSpeechVolume(iPos);
		}
		break;
	case BUTTON_MENUOPTIONSAUDIO_AMBIANCE:
		{
			ARXMenu_Options_Audio_SetAmbianceVolume(iPos);
		}
		break;
	// MENUOPTIONS_CONTROLS
	case BUTTON_MENUOPTIONS_CONTROLS_MOUSESENSITIVITY:
		{
			ARXMenu_Options_Control_SetMouseSensitivity(iPos);
		}
		break;
	}

	return false;
}

//-----------------------------------------------------------------------------

void CMenuSlider::Update(int _iTime)
{
	pLeftButton->Update(_iTime);
	pRightButton->Update(_iTime);
	pRightButton->SetPos(rZone.left, rZone.top);
	

	float fWidth = pLeftButton->GetWidth() + RATIO_X(10*max(pTex1->m_dwWidth, pTex2->m_dwWidth)) ;
	ARX_CHECK_INT(fWidth);

	pRightButton->Move(	ARX_CLEAN_WARN_CAST_INT(fWidth), 0);



	ARX_CHECK_LONG( rZone.left + pLeftButton->GetWidth() + pRightButton->GetWidth() + RATIO_X(10*max(pTex1->m_dwWidth, pTex2->m_dwWidth)) );
	rZone.right  = ARX_CLEAN_WARN_CAST_LONG( rZone.left + pLeftButton->GetWidth() + pRightButton->GetWidth() + RATIO_X(10*max(pTex1->m_dwWidth, pTex2->m_dwWidth)) );

	rZone.bottom = rZone.top + max(pLeftButton->GetHeight(), pRightButton->GetHeight());

}

//-----------------------------------------------------------------------------

void CMenuSlider::Render()
{
	if(WILL_RELOAD_ALL_TEXTURES) return;

	if(bNoMenu) return;

	pLeftButton->Render();
	pRightButton->Render();


	float iX = ARX_CLEAN_WARN_CAST_FLOAT( rZone.left + pLeftButton->GetWidth() );
	float iY = ARX_CLEAN_WARN_CAST_FLOAT( rZone.top );

	float iTexW = 0;
	float iTexH = 0;

	GDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, true);
	GDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
	GDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);

	D3DTLVERTEX v[4];
	v[0].color = v[1].color = v[2].color = v[3].color = ARX_OPAQUE_WHITE;
	v[0].sz=v[1].sz=v[2].sz=v[3].sz=0.f;	
	v[0].rhw=v[1].rhw=v[2].rhw=v[3].rhw=0.999999f;

	TextureContainer *pTex = pTex1;

	for (int i=0; i<10; i++)
	{
		iTexW = 0;
		iTexH = 0;

		if (i<iPos)
		{
			if(pTex1)
			{
				pTex = pTex1;
				iTexW = RATIO_X(pTex1->m_dwWidth);
				iTexH = RATIO_Y(pTex1->m_dwHeight);
			}
		}
		else
		{
			if(pTex2)
			{
				pTex = pTex2;
				iTexW = RATIO_X(pTex2->m_dwWidth);
				iTexH = RATIO_Y(pTex2->m_dwHeight);
			}
		}

		EERIEDrawBitmap2(GDevice, iX, iY, 
			RATIO_X(pTex->m_dwWidth),
			RATIO_Y(pTex->m_dwHeight),
			0,
			pTex,
			ARX_OPAQUE_WHITE);

		iX += iTexW;
	}

	GDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, false);
}

//-----------------------------------------------------------------------------

void CMenuSlider::RenderMouseOver()
{
	if(WILL_RELOAD_ALL_TEXTURES) return;

	if(bNoMenu) return;

	pGetInfoDirectInput->SetMouseOver();

	int iX = pGetInfoDirectInput->iMouseAX;
	int iY = pGetInfoDirectInput->iMouseAY;

	GDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, true);
	GDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
	GDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);

	if ((iX >= rZone.left) &&
		(iY >= rZone.top) &&
		(iX <= rZone.right) &&
		(iY <= rZone.bottom))
	{
		if ((iX >= pLeftButton->rZone.left) &&
			(iY >= pLeftButton->rZone.top) &&
			(iX <= pLeftButton->rZone.right) &&
			(iY <= pLeftButton->rZone.bottom))
		{
			pLeftButton->Render();

		}
		else if ((iX >= pRightButton->rZone.left) &&
				(iY >= pRightButton->rZone.top) &&
				(iX <= pRightButton->rZone.right) &&
				(iY <= pRightButton->rZone.bottom))
			{
				pRightButton->Render();

			}
		}

	GDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, false);
}

//-----------------------------------------------------------------------------

CDirectInput::CDirectInput()
{
	char temp[256];
	MakeDir(temp,"graph\\interface\\cursors\\cursor00.bmp");
	pTex[0]=D3DTextr_GetSurfaceContainer(temp);
	MakeDir(temp,"graph\\interface\\cursors\\cursor01.bmp");
	pTex[1]=D3DTextr_GetSurfaceContainer(temp);
	MakeDir(temp,"graph\\interface\\cursors\\cursor02.bmp");
	pTex[2]=D3DTextr_GetSurfaceContainer(temp);
	MakeDir(temp,"graph\\interface\\cursors\\cursor03.bmp");
	pTex[3]=D3DTextr_GetSurfaceContainer(temp);
	MakeDir(temp,"graph\\interface\\cursors\\cursor04.bmp");
	pTex[4]=D3DTextr_GetSurfaceContainer(temp);
	MakeDir(temp,"graph\\interface\\cursors\\cursor05.bmp");
	pTex[5]=D3DTextr_GetSurfaceContainer(temp);
	MakeDir(temp,"graph\\interface\\cursors\\cursor06.bmp");
	pTex[6]=D3DTextr_GetSurfaceContainer(temp);
	MakeDir(temp,"graph\\interface\\cursors\\cursor07.bmp");
	pTex[7]=D3DTextr_GetSurfaceContainer(temp);

	SetCursorOff();
	SetSensibility(2);
	iMouseAX=0;
	iMouseAY=0;
	iMouseAZ=0;
	fMouseAXTemp=fMouseAYTemp=0.f;
	iNbOldCoord=0;
	iMaxOldCoord=40;

	bMouseOver=false;

	if(pTex[0])
	{
		fTailleX=(float)pTex[0]->m_dwWidth;
		fTailleY=(float)pTex[0]->m_dwHeight;
	}
	else
	{
		fTailleX=fTailleY=0.f;
	}

	iNumCursor=0;
	lFrameDiff=0;

	for(int i=DXI_BUTTON0;i<=DXI_BUTTON31;i++)
	{
		iOldMouseButton[i]=0;
		iMouseTime[i]=0;
		iMouseTimeSet[i]=0;
		bMouseButton[i]=bOldMouseButton[i]=false;
		iOldNumClick[i]=iOldNumUnClick[i]=0;
	}

	// PreCompute le ScanCode
	bTouch=false;
	HKL layout=GetKeyboardLayout(0);

	for(int iI=0;iI<256;iI++)
	{
		iKeyScanCode[iI]=MapVirtualKeyEx(iI,0,layout);
		iOneTouch[iI]=0;
	}

	bDrawCursor=true;
	bActive=false;

	iWheelSens=0;
}

//-----------------------------------------------------------------------------

CDirectInput::~CDirectInput()
{
}

//-----------------------------------------------------------------------------

void CDirectInput::SetCursorOff()
{
	eNumTex=CURSOR_OFF;
}

//-----------------------------------------------------------------------------

void CDirectInput::SetCursorOn()
{
	eNumTex=CURSOR_ON;
}

//-----------------------------------------------------------------------------

void CDirectInput::SetMouseOver()
{
	bMouseOver=true;
	SetCursorOn();
}

//-----------------------------------------------------------------------------

void CDirectInput::SetSensibility(int _iSensibility)
{
	iSensibility=_iSensibility;
}

//-----------------------------------------------------------------------------

void CDirectInput::ResetAll()
{
	for(int i=DXI_BUTTON0;i<=DXI_BUTTON31;i++)
	{
		iOldMouseButton[i]=0;
		iMouseTime[i]=0;
		iMouseTimeSet[i]=0;
		bMouseButton[i]=bOldMouseButton[i]=false;
		iOldNumClick[i]=iOldNumUnClick[i]=0;
	}

	iKeyId=-1;
	bTouch=false;

	for(int i=0;i<256;i++)
	{
		iOneTouch[i]=0;
	}

	EERIEMouseButton=LastEERIEMouseButton=0;

	iWheelSens=0;
}

//-----------------------------------------------------------------------------

void CDirectInput::GetInput()
{
int iDTime;

	DXI_ExecuteAllDevices(false);
	iKeyId=DXI_GetKeyIDPressed(DXI_KEYBOARD1);
	bTouch=(iKeyId>=0)?true:false;

	for(int i=0;i<256;i++)
	{
		if(IsVirtualKeyPressed(i))
		{
			switch(i)
			{
			case DIK_LSHIFT:
			case DIK_RSHIFT:
			case DIK_LCONTROL:
			case DIK_RCONTROL:
			case DIK_LALT:
			case DIK_RALT:

				if(i!=iKeyId)
					iKeyId|=(i<<16);

				break;
			}

			if(iOneTouch[i]<2)
			{
				iOneTouch[i]++;
			}
		}
		else
		{
			if(iOneTouch[i]>0)
			{
				iOneTouch[i]--;
			}
		}
	}

	if(bTouch)	//priorité des touches
	{
		switch(iKeyId)
		{
		case DIK_LSHIFT:
		case DIK_RSHIFT:
		case DIK_LCONTROL:
		case DIK_RCONTROL:
		case DIK_LALT:
		case DIK_RALT:
			{
				bool bFound=false;

				for(int i=0;i<256;i++)
				{
					if(bFound)
					{
						break;
					}

					switch(i&0xFFFF)
					{
					case DIK_LSHIFT:
					case DIK_RSHIFT:
					case DIK_LCONTROL:
					case DIK_RCONTROL:
					case DIK_LALT:
					case DIK_RALT:
						continue;
					default:
						{
							if(iOneTouch[i])
							{
								bFound=true;
								iKeyId&=~0xFFFF;
								iKeyId|=i;
							}
						}
						break;
					}
				}
			}
		}
	}


	ARX_CHECK_INT(ARX_TIME_Get( false ));
	const int iArxTime = ARX_CLEAN_WARN_CAST_INT(ARX_TIME_Get( false )) ;


	for(int i=DXI_BUTTON0;i<=DXI_BUTTON31;i++)
	{
		int iNumClick;
		int iNumUnClick;
		DXI_MouseButtonCountClick(DXI_MOUSE1,i,&iNumClick,&iNumUnClick);

		iOldNumClick[i]+=iNumClick+iNumUnClick;

		if(	(!bMouseButton[i])&&(iOldNumClick[i]==iNumUnClick) )
		{
			iNumUnClick=iOldNumClick[i]=0;
		}

		bOldMouseButton[i]=bMouseButton[i];

		if(bMouseButton[i])
		{
			if(iOldNumClick[i])
			{
				bMouseButton[i]=false;
			}
		}
		else
		{
			if(iOldNumClick[i])
			{
				bMouseButton[i]=true;
			}
		}

		if(iOldNumClick[i]) iOldNumClick[i]--;

		iDTime=0;
		DXI_MouseButtonPressed(DXI_MOUSE1,i,&iDTime);

		if(iDTime)
		{
			iMouseTime[i]=iDTime;
			iMouseTimeSet[i]=2;
		}
		else
		{
			if(	(iMouseTimeSet[i]>0)&&
				((ARX_TIME_Get( false )-iMouseTime[i])>300)
				)
			{
				iMouseTime[i]=0;
				iMouseTimeSet[i]=0;
			}

			if(GetMouseButtonNowPressed(i))
			{
				switch(iMouseTimeSet[i])
				{
				case 0:
					iMouseTime[i] = iArxTime;
					iMouseTimeSet[i]++;
					break;
				case 1:
					iMouseTime[i] = iArxTime - iMouseTime[i];
					iMouseTimeSet[i]++;
					break;
				}
			}
		}
		}

	iWheelSens=pGetInfoDirectInput->GetWheelSens(DXI_MOUSE1);

	if(	( danaeApp.m_pFramework->m_bIsFullscreen ) &&
		( bGLOBAL_DINPUT_MENU ) )
	{
		float fDX = 0.f;
		float fDY = 0.f;
		iMouseRX = iMouseRY = iMouseRZ = 0;

		if( DXI_GetAxeMouseXYZ(DXI_MOUSE1, &iMouseRX, &iMouseRY, &iMouseRZ) )
		{
			float fSensMax = 1.f / 6.f;
			float fSensMin = 2.f;
			float fSens = ( ( fSensMax - fSensMin ) * ( (float)iSensibility ) / 10.f ) + fSensMin;
			fSens = pow( .7f, fSens ) * 2.f;

			fDX=( (float)iMouseRX ) * fSens * ( ( (float)DANAESIZX ) / 640.f );
			fDY=( (float)iMouseRY ) * fSens * ( ( (float)DANAESIZY ) / 480.f );
			fMouseAXTemp += fDX;
			fMouseAYTemp += fDY;
			

			ARX_CHECK_INT(fMouseAXTemp);
			ARX_CHECK_INT(fMouseAYTemp);
			iMouseAX  = ARX_CLEAN_WARN_CAST_INT(fMouseAXTemp);
			iMouseAY  = ARX_CLEAN_WARN_CAST_INT(fMouseAYTemp);

			iMouseAZ += iMouseRZ;


			if(iMouseAX<0)
			{
				iMouseAX	 = 0;
				fMouseAXTemp = 0.f; 
			}


			ARX_CHECK_NOT_NEG( iMouseAX );

			if( ARX_CAST_ULONG( iMouseAX ) >= danaeApp.m_pFramework->m_dwRenderWidth )
			{

				iMouseAX = danaeApp.m_pFramework->m_dwRenderWidth - 1;
				fMouseAXTemp = ARX_CLEAN_WARN_CAST_FLOAT( iMouseAX );
			}

			if(iMouseAY<0)
			{
				fMouseAYTemp=	0.f;
				iMouseAY	=	0;
			}


			ARX_CHECK_NOT_NEG( iMouseAY );

			if( ARX_CAST_ULONG( iMouseAY ) >= danaeApp.m_pFramework->m_dwRenderHeight )
			{

				iMouseAY		= danaeApp.m_pFramework->m_dwRenderHeight - 1;
				fMouseAYTemp	= ARX_CLEAN_WARN_CAST_FLOAT( iMouseAY ); 
			}



			bMouseMove=true;
		}
		else
		{
			bMouseMove=false;
		}

		if(bGLOBAL_DINPUT_GAME)
		{
			_EERIEMouseXdep=(int)fDX;
			_EERIEMouseYdep=(int)fDY;
			EERIEMouseX=iMouseAX;
			EERIEMouseY=iMouseAY;
		}
	}
	else
	{
		bMouseMove = ((iMouseAX != DANAEMouse.x) || (iMouseAY != DANAEMouse.y));
		iMouseAX=DANAEMouse.x;
		iMouseAY=DANAEMouse.y;
		iMouseAZ=0;
	}

	int iDx;
	int iDy;

	if(pTex[eNumTex])
	{
		iDx=pTex[eNumTex]->m_dwWidth>>1;
		iDy=pTex[eNumTex]->m_dwHeight>>1;
	}
	else
	{
		iDx=0;
		iDy=0;
	}

	iOldCoord[iNbOldCoord].x=iMouseAX+iDx;
	iOldCoord[iNbOldCoord].y=iMouseAY+iDy;
	iNbOldCoord++;

	if(iNbOldCoord>=iMaxOldCoord)
	{
		iNbOldCoord=iMaxOldCoord-1;
		memmove((void*)iOldCoord,(void*)(iOldCoord+1),sizeof(EERIE_2DI)*iNbOldCoord);
	}

}

//-----------------------------------------------------------------------------

int CDirectInput::GetWheelSens(int _iIDbutton)
{
	int iX,iY,iZ=0;
	DXI_GetAxeMouseXYZ(_iIDbutton,&iX,&iY,&iZ);

	return iZ;
}

//-----------------------------------------------------------------------------

bool CDirectInput::IsVirtualKeyPressed(int _iVirtualKey)
{
	return DXI_KeyPressed(DXI_KEYBOARD1,_iVirtualKey)?true:false;
}

//-----------------------------------------------------------------------------

bool CDirectInput::IsVirtualKeyPressedOneTouch(int _iVirtualKey)
{

	return(	(DXI_KeyPressed(DXI_KEYBOARD1,_iVirtualKey))&&
			(iOneTouch[_iVirtualKey]==1) );
}

//-----------------------------------------------------------------------------

bool CDirectInput::IsVirtualKeyPressedNowPressed(int _iVirtualKey)
{
	return(	(DXI_KeyPressed(DXI_KEYBOARD1,_iVirtualKey))&&
			(iOneTouch[_iVirtualKey]==1) );
}

//-----------------------------------------------------------------------------

bool CDirectInput::IsVirtualKeyPressedNowUnPressed(int _iVirtualKey)
{
	return(	(!DXI_KeyPressed(DXI_KEYBOARD1,_iVirtualKey))&&
			(iOneTouch[_iVirtualKey]==1) );
}

//-----------------------------------------------------------------------------

void CDirectInput::DrawOneCursor(int _iPosX,int _iPosY,int _iColor)
{
	GDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_POINT );
	GDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_POINT );
	SETTEXTUREWRAPMODE(GDevice,D3DTADDRESS_CLAMP);

	EERIEDrawBitmap2(GDevice, ARX_CLEAN_WARN_CAST_FLOAT(_iPosX), ARX_CLEAN_WARN_CAST_FLOAT(_iPosY),

					INTERFACE_RATIO_DWORD(scursor[iNumCursor]->m_dwWidth),
					INTERFACE_RATIO_DWORD(scursor[iNumCursor]->m_dwHeight),

					0.00000001f,
					scursor[iNumCursor],D3DCOLORWHITE);
	GDevice->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTFP_LINEAR);
	GDevice->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTFP_LINEAR);
	SETTEXTUREWRAPMODE(GDevice,D3DTADDRESS_WRAP);
}

//-----------------------------------------------------------------------------

static bool ComputePer(EERIE_2DI *_psPoint1,EERIE_2DI *_psPoint2,D3DTLVERTEX *_psd3dv1,D3DTLVERTEX *_psd3dv2,float _fSize)
{
	EERIE_2D sTemp;
	float fTemp;

	sTemp.x=(float)(_psPoint2->x-_psPoint1->x);
	sTemp.y=(float)(_psPoint2->y-_psPoint1->y);
	fTemp=sTemp.x;
	sTemp.x=-sTemp.y;
	sTemp.y=fTemp;
	float fMag=(float)sqrt(sTemp.x*sTemp.x+sTemp.y*sTemp.y);

	if(fMag<EEdef_EPSILON)
	{
		return false;
	}

	fMag=_fSize/fMag;

	_psd3dv1->sx=(sTemp.x*fMag);
	_psd3dv1->sy=(sTemp.y*fMag);
	_psd3dv2->sx=((float)_psPoint1->x)-_psd3dv1->sx;
	_psd3dv2->sy=((float)_psPoint1->y)-_psd3dv1->sy;
	_psd3dv1->sx+=((float)_psPoint1->x);
	_psd3dv1->sy+=((float)_psPoint1->y);

	return true;
}

//-----------------------------------------------------------------------------

static void DrawLine2D(EERIE_2DI *_psPoint1,int _iNbPt,float _fSize,float _fRed,float _fGreen,float _fBlue)
{
	_iNbPt--;

	if(!_iNbPt) return;

	float fSize=_fSize/_iNbPt;
	float fTaille=fSize;
	
	float fDColorRed=_fRed/_iNbPt;
	float fColorRed=fDColorRed;
	float fDColorGreen=_fGreen/_iNbPt;
	float fColorGreen=fDColorGreen;
	float fDColorBlue=_fBlue/_iNbPt;
	float fColorBlue=fDColorBlue;

	GDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_DESTCOLOR);
	GDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVDESTCOLOR);
	SETTC(GDevice,NULL);
	SETALPHABLEND(GDevice,true);
	
	D3DTLVERTEX v[4];
	v[0].sz=v[1].sz=v[2].sz=v[3].sz=0.f;	
	v[0].rhw=v[1].rhw=v[2].rhw=v[3].rhw=0.999999f;

	EERIE_2DI *psOldPoint=_psPoint1++;
	v[0].color=v[2].color=D3DRGBA(fColorRed,fColorGreen,fColorBlue,1.f);	

	if(!ComputePer(psOldPoint,_psPoint1,&v[0],&v[2],fTaille))
	{
		v[0].sx=v[2].sx=(float)psOldPoint->x;
		v[0].sy=v[2].sy=(float)psOldPoint->y;
	}

	_iNbPt--;

	while(_iNbPt--)
	{
		fTaille+=fSize;
		fColorRed+=fDColorRed;
		fColorGreen+=fDColorGreen;
		fColorBlue+=fDColorBlue;

		if(ComputePer(psOldPoint,_psPoint1+1,&v[1],&v[3],fTaille))
		{
			v[1].color=v[3].color=D3DRGBA(fColorRed,fColorGreen,fColorBlue,1.f);	
			EERIEDRAWPRIM(GDevice,D3DPT_TRIANGLESTRIP,D3DFVF_TLVERTEX|D3DFVF_DIFFUSE,v,4,0);
			
			v[0].sx=v[1].sx;
			v[0].sy=v[1].sy;
			v[0].color=v[1].color;
			v[2].sx=v[3].sx;
			v[2].sy=v[3].sy;
			v[2].color=v[3].color;
		}

		psOldPoint=_psPoint1++;
	}

	fTaille+=fSize;
	fColorRed+=fDColorRed;
	fColorGreen+=fDColorGreen;
	fColorBlue+=fDColorBlue;

	if(ComputePer(_psPoint1,psOldPoint,&v[1],&v[3],fTaille)) 
	{
		v[1].color=v[3].color=D3DRGBA(fColorRed,fColorGreen,fColorBlue,1.f);	
		EERIEDRAWPRIM(GDevice,D3DPT_TRIANGLESTRIP,D3DFVF_TLVERTEX|D3DFVF_DIFFUSE,v,4,0);
	}

	SETALPHABLEND(GDevice,false);
}
//-----------------------------------------------------------------------------

void CDirectInput::DrawCursor()
{
	if(!bDrawCursor) return;

	GDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE,  true);
	DrawLine2D(iOldCoord,iMaxOldCoord,10.f,.725f,.619f,0.56f);

	if(pTex[iNumCursor]) SETTC(GDevice, pTex[iNumCursor]);
	else SETTC(GDevice,NULL);

	SETALPHABLEND(GDevice,false);

	GDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, false);
	DrawOneCursor(iMouseAX,iMouseAY,-1);

	danaeApp.EnableZBuffer();


	ARX_CHECK_LONG( ARXDiffTimeMenu );
	lFrameDiff += ARX_CLEAN_WARN_CAST_LONG( ARXDiffTimeMenu );


	if(lFrameDiff>70)
	{
		if(bMouseOver)
		{
			if(iNumCursor<4)
			{
				iNumCursor++;
			}
			else
			{
				if(iNumCursor>4)
				{
					iNumCursor--;
				}
			}
			
			SetCursorOff();
			bMouseOver=false;
		}
		else
		{
			if (iNumCursor > 0)
			{
				iNumCursor++;

				if(iNumCursor>7) iNumCursor=0;
			}
		}

		lFrameDiff=0;
	}

	GDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE,  false);
}

//-----------------------------------------------------------------------------

bool CDirectInput::GetMouseButton(int _iNumButton)
{

	return(	(bMouseButton[_iNumButton])&&(!bOldMouseButton[_iNumButton]));
}

//-----------------------------------------------------------------------------

bool CDirectInput::GetMouseButtonRepeat(int _iNumButton)
{
	return( bMouseButton[_iNumButton] );
}

//-----------------------------------------------------------------------------

bool CDirectInput::GetMouseButtonNowPressed(int _iNumButton)
{

	return(	(bMouseButton[_iNumButton])&&(!bOldMouseButton[_iNumButton]));
}

//-----------------------------------------------------------------------------

bool CDirectInput::GetMouseButtonNowUnPressed(int _iNumButton)
{

	return( (!bMouseButton[_iNumButton])&&(bOldMouseButton[_iNumButton]) );
}

//-----------------------------------------------------------------------------

_TCHAR * CDirectInput::GetFullNameTouch(int _iVirtualKey)
{
	_TCHAR *pText=(_TCHAR*)malloc(256*sizeof(_TCHAR));

	if(!pText) return NULL;

	long lParam;

	_TCHAR *pText2=NULL;

	if(	(_iVirtualKey!=-1)&&
		(_iVirtualKey&~0xC000FFFF) )	//COMBINAISON
	{
		pText2=GetFullNameTouch((_iVirtualKey>>16)&0x3FFF);
	}

	lParam=((_iVirtualKey)&0x7F)<<16;

	switch(_iVirtualKey)
	{
	case DIK_HOME:

		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_home"), _T("string"), _T("---"), pText, 256, NULL);
		break;
	case DIK_NEXT:

		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_pagedown"), _T("string"), _T("---"), pText, 256, NULL);
		break;
	case DIK_END:

		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_end"), _T("string"), _T("---"), pText, 256, NULL);
		break;
	case DIK_INSERT:

		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_insert"), _T("string"), _T("---"), pText, 256, NULL);
		break;
	case DIK_DELETE:

		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_delete"), _T("string"), _T("---"), pText, 256, NULL);
		break;
	case DIK_NUMLOCK:

		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_numlock"), _T("string"), _T("---"), pText, 256, NULL);
		break;
	case DIK_DIVIDE:
		_tcscpy(pText,_T("_/_"));
		break;
	case DIK_MULTIPLY:
		_tcscpy(pText,_T("_x_"));
		break;
	case DIK_SYSRQ:           
		_tcscpy(pText,_T("?"));
		break;
	case DIK_UP:                  // UpArrow on arrow keypad

		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_up"), _T("string"), _T("---"), pText, 256, NULL);
		break;
	case DIK_PRIOR:               // PgUp on arrow keypad

		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_pageup"), _T("string"), _T("---"), pText, 256, NULL);
		break;
	case DIK_LEFT:                // LeftArrow on arrow keypad

		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_left"), _T("string"), _T("---"), pText, 256, NULL);
		break;
	case DIK_RIGHT:               // RightArrow on arrow keypad

		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_right"), _T("string"), _T("---"), pText, 256, NULL);
		break;
	case DIK_DOWN:                // DownArrow on arrow keypad

		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_down"), _T("string"), _T("---"), pText, 256, NULL);
		break;
	case DIK_BUTTON1:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button0"), _T("string"), _T("b1"), pText, 256, NULL);
		break;
	case DIK_BUTTON2:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button1"), _T("string"), _T("b2"), pText, 256, NULL);
		break;
	case DIK_BUTTON3:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button2"), _T("string"), _T("b3"), pText, 256, NULL);
		break;
	case DIK_BUTTON4:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button3"), _T("string"), _T("b4"), pText, 256, NULL);
		break;
	case DIK_BUTTON5:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button4"), _T("string"), _T("b5"), pText, 256, NULL);
		break;
	case DIK_BUTTON6:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button5"), _T("string"), _T("b6"), pText, 256, NULL);
		break;
	case DIK_BUTTON7:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button6"), _T("string"), _T("b7"), pText, 256, NULL);
		break;
	case DIK_BUTTON8:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button7"), _T("string"), _T("b8"), pText, 256, NULL);
		break;
	case DIK_BUTTON9:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button8"), _T("string"), _T("b9"), pText, 256, NULL);
		break;
	case DIK_BUTTON10:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button9"), _T("string"), _T("b10"), pText, 256, NULL);
		break;
	case DIK_BUTTON11:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button10"), _T("string"), _T("b11"), pText, 256, NULL);
		break;
	case DIK_BUTTON12:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button11"), _T("string"), _T("b12"), pText, 256, NULL);
		break;
	case DIK_BUTTON13:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button12"), _T("string"), _T("b13"), pText, 256, NULL);
		break;
	case DIK_BUTTON14:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button13"), _T("string"), _T("b14"), pText, 256, NULL);
		break;
	case DIK_BUTTON15:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button14"), _T("string"), _T("b15"), pText, 256, NULL);
		break;
	case DIK_BUTTON16:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button15"), _T("string"), _T("b16"), pText, 256, NULL);
		break;
	case DIK_BUTTON17:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button16"), _T("string"), _T("b17"), pText, 256, NULL);
		break;
	case DIK_BUTTON18:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button17"), _T("string"), _T("b18"), pText, 256, NULL);
		break;
	case DIK_BUTTON19:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button18"), _T("string"), _T("b19"), pText, 256, NULL);
		break;
	case DIK_BUTTON20:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button19"), _T("string"), _T("b20"), pText, 256, NULL);
		break;
	case DIK_BUTTON21:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button20"), _T("string"), _T("b21"), pText, 256, NULL);
		break;
	case DIK_BUTTON22:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button21"), _T("string"), _T("b22"), pText, 256, NULL);
		break;
	case DIK_BUTTON23:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button22"), _T("string"), _T("b23"), pText, 256, NULL);
		break;
	case DIK_BUTTON24:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button23"), _T("string"), _T("b24"), pText, 256, NULL);
		break;
	case DIK_BUTTON25:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button24"), _T("string"), _T("b25"), pText, 256, NULL);
		break;
	case DIK_BUTTON26:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button25"), _T("string"), _T("b26"), pText, 256, NULL);
		break;
	case DIK_BUTTON27:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button26"), _T("string"), _T("b27"), pText, 256, NULL);
		break;
	case DIK_BUTTON28:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button27"), _T("string"), _T("b28"), pText, 256, NULL);
		break;
	case DIK_BUTTON29:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button28"), _T("string"), _T("b29"), pText, 256, NULL);
		break;
	case DIK_BUTTON30:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button29"), _T("string"), _T("b30"), pText, 256, NULL);
		break;
	case DIK_BUTTON31:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button30"), _T("string"), _T("b31"), pText, 256, NULL);
		break;
	case DIK_BUTTON32:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button31"), _T("string"), _T("b32"), pText, 256, NULL);
		break;
	case DIK_WHEELUP:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_wheelup"), _T("string"), _T("w0"), pText, 256, NULL);
		break;
	case DIK_WHEELDOWN:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_wheeldown"), _T("string"), _T("w1"), pText, 256, NULL);
		break;
	case -1:
		_tcscpy(pText,_T("---"));
		break;
	default:
		{
		char tAnsiText[256];
		GetKeyNameText(lParam,tAnsiText,256);
		int i=strlen(tAnsiText);

		if(!i)
		{
			_stprintf(pText,_T("Key_%d"),_iVirtualKey);
		}
		else
		{
			MultiByteToWideChar(CP_ACP, 0, tAnsiText, -1, pText, 256);

			if(_iVirtualKey==DIK_LSHIFT)
			{
				_TCHAR tText2[256];
				_TCHAR *pText3;
				PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_left"), _T("string"), _T("---"), tText2, 256, NULL);
				tText2[1]=0;
				pText3=(_TCHAR*)malloc((_tcslen(tText2)+_tcslen(pText)+1)*sizeof(_TCHAR));
				_tcscpy(pText3,tText2);
				_tcscat(pText3,pText);
				free((void*)pText);
				pText=pText3;
			}

			if(_iVirtualKey==DIK_LCONTROL)
			{
				_TCHAR tText2[256];
				_TCHAR *pText3;
				PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_left"), _T("string"), _T("---"), tText2, 256, NULL);
				tText2[1]=0;
				pText3=(_TCHAR*)malloc((_tcslen(tText2)+_tcslen(pText)+1)*sizeof(_TCHAR));
				_tcscpy(pText3,tText2);
				_tcscat(pText3,pText);
				free((void*)pText);
				pText=pText3;
			}

			if(_iVirtualKey==DIK_LALT)
			{
				_TCHAR tText2[256];
				_TCHAR *pText3;
				PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_left"), _T("string"), _T("---"), tText2, 256, NULL);
				tText2[1]=0;
				pText3=(_TCHAR*)malloc((_tcslen(tText2)+_tcslen(pText)+1)*sizeof(_TCHAR));
				_tcscpy(pText3,tText2);
				_tcscat(pText3,pText);
				free((void*)pText);
				pText=pText3;
			}

				if (_iVirtualKey == DIK_NUMPADENTER)
			{
				_TCHAR *pText3;
				pText3=(_TCHAR*)malloc((_tcslen(pText)+1+1)*sizeof(_TCHAR));
				_tcscpy(pText3,pText);
				_tcscat(pText3,_T("0"));
				free((void*)pText);
				pText=pText3;
			}
  
			if(_tcslen(pText)>8)
			{
				pText[8]=0;
				int iI=8;

				while(iI--)
				{
					if(pText[iI]==_T(' '))
					{
						pText[iI]=0;
					}
					else break;
				}
			}
		}
		}
		break;
	}

	if(pText2)
	{
		_TCHAR *pText3=(_TCHAR*)malloc((_tcslen(pText2)+_tcslen(pText)+1+1)*sizeof(_TCHAR));
		_tcscpy(pText3,pText2);
		_tcscat(pText3,_T("+"));
		_tcscat(pText3,pText);
		free((void*)pText);
		free((void*)pText2);
		pText=pText3;

	}

	return pText;
}

//-----------------------------------------------------------------------------

void Menu2_Close()
{
	ARXmenu.currentmode = AMCM_OFF;

	if (pMenu)
	{
		pMenu->eMenuState = NOP;
		pMenu->pZoneClick = NULL;
		delete pMenu;
		pMenu = NULL;
	}

	if(pWindowMenu)
	{
		delete pWindowMenu;
		pWindowMenu=NULL;
	}
}

