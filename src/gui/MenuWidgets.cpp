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

#include "gui/MenuWidgets.h"

#include <cstring>
#include <cstdio>

#include <algorithm>
#include <sstream>

#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"
#include "core/Localisation.h"

#include "gui/Menu.h"
#include "gui/MenuPublic.h"
#include "gui/Text.h"
#include "gui/Interface.h"
#include "gui/Credits.h"
#include "gui/TextManager.h"

#include "graphics/Draw.h"
#include "graphics/Frame.h"
#include "graphics/GraphicsEnum.h"
#include "graphics/data/Texture.h"
#include "graphics/data/Mesh.h"
#include "graphics/font/Font.h"
#include "graphics/texture/TextureStage.h"

#include "input/Input.h"

#include "io/Logger.h"

#include "platform/String.h"

#include "scene/GameSound.h"
#include "scene/LoadLevel.h"

using std::wistringstream;
using std::min;
using std::max;
using std::string;

int newTextureSize;
int newWidth;
int newHeight;
int newBpp;
bool changeResolution = false;
bool changeTextures = false;

extern char* GetVersionString();

#define NODEBUGZONE

//-----------------------------------------------------------------------------

#define RATIO_X(a)    (((float)a)*Xratio)
#define RATIO_Y(a)    (((float)a)*Yratio)


//-----------------------------------------------------------------------------
// Imported global variables and functions
extern ARX_MENU_DATA ARXmenu;
extern TextureContainer * scursor[];
extern long DANAESIZX;
extern long DANAESIZY;

extern long LastEERIEMouseButton;

extern bool bForceReInitAllTexture;
extern long WILL_RELOAD_ALL_TEXTURES;

extern long FINAL_RELEASE;

extern long INTRO_NOT_LOADED;
extern long REFUSE_GAME_RETURN;

extern float PROGRESS_BAR_TOTAL;
extern float OLD_PROGRESS_BAR_COUNT;
extern float PROGRESS_BAR_COUNT;

extern long CURRENT_GAME_INSTANCE;
extern char GameSavePath[];
void ARX_GAMESAVE_MakePath();


float INTERFACE_RATIO(float a);
bool bNoMenu=false;

void ARXMenu_Private_Options_Video_SetResolution(int _iWidth,int _iHeight,int _bpp );

//-----------------------------------------------------------------------------

MenuCursor* pMenuCursor=NULL;

static CWindowMenu *pWindowMenu=NULL;
CMenuState *pMenu;

CMenuElement *pMenuElementResume=NULL;
CMenuElement *pMenuElementApply=NULL;
CMenuElementText *pLoadConfirm=NULL;
CMenuSliderText *pMenuSliderResol=NULL;
CMenuSliderText *pMenuSliderBpp=NULL;
CMenuSliderText *pMenuSliderTexture=NULL;

float ARXTimeMenu;
float ARXOldTimeMenu;
float ARXDiffTimeMenu;

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

//-----------------------------------------------------------------------------
// Local functions

namespace
{

bool isTimeBefore(const SYSTEMTIME& s1, const SYSTEMTIME& s2)
{
	if (s1.wYear < s2.wYear) return true;
	if (s1.wYear > s2.wYear) return false;
	if (s1.wMonth < s2.wMonth) return true;
	if (s1.wMonth > s2.wMonth) return false;
	if (s1.wDay < s2.wDay) return true;
	if (s1.wDay > s2.wDay) return false;
	if (s1.wHour < s2.wHour) return true;
	if (s1.wHour > s2.wHour) return false;
	if (s1.wMinute < s2.wMinute) return true;
	if (s1.wMinute > s2.wMinute) return false;
	if (s1.wSecond < s2.wSecond) return true;
	if (s1.wSecond > s2.wSecond) return false;
	if (s1.wMilliseconds < s2.wMilliseconds) return true;
	if (s1.wMilliseconds > s2.wMilliseconds) return false;
	return true;
}

} // \namespace

//-----------------------------------------------------------------------------

void ARX_QuickSave()
{
	if( REFUSE_GAME_RETURN ) return;

	CreateSaveGameList();

	std::string szMenuText = QUICK_SAVE_ID;
	std::string szMenuText1 = QUICK_SAVE_ID1;

	int iOldGamma;

	ARXMenu_Options_Video_GetGamma( iOldGamma );
	ARXMenu_Options_Video_SetGamma( ( iOldGamma - 1 ) < 0 ? 0 : ( iOldGamma - 1 ) );

	ARX_SOUND_MixerPause( ARX_SOUND_MixerGame );

	bool        bFound0        =    false;
	bool        bFound1        =    false;

	size_t            iNbSave0    =    0; // will be used if >0 (0 will so mean NOTFOUND)
	size_t            iNbSave1    =    0; // will be used if >0 (0 will so mean NOTFOUND)
	SYSTEMTIME    sTime0;
	SYSTEMTIME    sTime1;
	ZeroMemory( &sTime0, sizeof(SYSTEMTIME) );// will be used if iNbSave0>0 (iNbSave0==0 will so mean NOTFOUND and sTime0 will not be used)
	ZeroMemory( &sTime1, sizeof(SYSTEMTIME) );// will be used if iNbSave0>0 (iNbSave1==0 will so mean NOTFOUND and sTime1 will not be used)


	for( size_t iI = 1 ; iI < save_l.size() ; iI++ )
	{
		std::string tex2 = save_l[iI].name;
		MakeUpcase( tex2 );

		if( szMenuText.find( tex2 ) != std::string::npos )
		{
			bFound0     = true;
			sTime0      = save_l[iI].stime;
			iNbSave0    = iI;
		}
		else if( szMenuText1.find( tex2 ) != std::string::npos )
		{
			bFound1     = true;
			sTime1      = save_l[iI].stime;
			iNbSave1    = iI;
		}
	}

	if ( bFound0 && bFound1 &&
		( iNbSave0 > 0 ) && ( iNbSave0 < save_l.size() ) &&
		( iNbSave1 > 0 ) && ( iNbSave1 < save_l.size() ) )
	{
		size_t iSave;
		
		if ( isTimeBefore( sTime0, sTime1 ) )
			iSave = iNbSave0;
		else
			iSave = iNbSave1;

		UpdateSaveGame( iSave );
		ARXMenu_Options_Video_SetGamma( iOldGamma );
		ARX_SOUND_MixerResume( ARX_SOUND_MixerGame );
		return;
	}

	const char    tcSrc[256] = "SCT_0.BMP";
	const char    tcDst[256] = "SCT_1.BMP";
	CopyFile( tcSrc, tcDst, false );

	if ( bFound0 == false )
	{
		save_l[0].name = QUICK_SAVE_ID;
		UpdateSaveGame( 0 );
		ARXMenu_Options_Video_SetGamma( iOldGamma );
		ARX_SOUND_MixerResume( ARX_SOUND_MixerGame );
		CopyFile( tcDst, tcSrc, false );
		DeleteFile( tcDst );
	}

	if ( bFound1 == false )
	{
		save_l[0].name = QUICK_SAVE_ID1;
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
	iTimeToDrawD7    -= ARX_CLEAN_WARN_CAST_INT(FrameDiff);
	
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

	TextureContainer *pTex = TextureContainer::Load("\\Graph\\interface\\icons\\Menu_main_save.bmp");

	if(!pTex) return;

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);

	EERIEDrawBitmap2(0, 0, INTERFACE_RATIO_DWORD(pTex->m_dwWidth),
	                 INTERFACE_RATIO_DWORD(pTex->m_dwHeight), 0.f, pTex, Color::gray(fColor));

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}

bool ARX_QuickLoad()
{
	CreateSaveGameList();

	std::string szMenuText = QUICK_SAVE_ID;
	std::string szMenuText1 = QUICK_SAVE_ID1;

	bool bFound0 = false;
	bool bFound1 = false;

	size_t iNbSave0    =    0; // will be used if >0 (0 will so mean NOTFOUND)
	size_t iNbSave1    =    0; // will be used if >0 (0 will so mean NOTFOUND)
	SYSTEMTIME sTime0;
	SYSTEMTIME sTime1;
	ZeroMemory( &sTime0, sizeof(SYSTEMTIME) );// will be used if iNbSave0>0 (iNbSave0==0 will so mean NOTFOUND and sTime0 will not be used)
	ZeroMemory( &sTime1, sizeof(SYSTEMTIME) );// will be used if iNbSave1>0 (iNbSave1==0 will so mean NOTFOUND ans sTime1 will not be used)

	for( size_t iI = 1 ; iI < save_l.size() ; iI++ )
	{
		std::string tex2 = save_l[iI].name;
		MakeUpcase( tex2 );

		if( szMenuText.find( tex2 ) != std::string::npos )
		{
			bFound0     = true;
			sTime0      = save_l[iI].stime;
			iNbSave0    = iI;
		}
		else if( szMenuText1.find( tex2 ) != std::string::npos )
		{
			bFound1     = true;
			sTime1      = save_l[iI].stime;
			iNbSave1    = iI;
		}
	}

	ARX_SOUND_MixerPause( ARX_SOUND_MixerGame );

	if ( bFound0 && bFound1 &&
		( iNbSave0 > 0 ) && ( iNbSave0 < save_l.size() ) &&
		( iNbSave1 > 0 ) && ( iNbSave1 < save_l.size() ) )
	{
		size_t iSave;

		if ( isTimeBefore( sTime0, sTime1 ) )
			iSave = iNbSave1;
		else
			iSave = iNbSave0;

		INTRO_NOT_LOADED        =    1;
		LoadLevelScreen();
		PROGRESS_BAR_TOTAL        =    238;
		OLD_PROGRESS_BAR_COUNT    =    PROGRESS_BAR_COUNT=0;
		PROGRESS_BAR_COUNT        +=    1.f;
		LoadLevelScreen( save_l[iSave].level );
		DanaeClearLevel();
		ARX_CHANGELEVEL_Load( save_l[iSave].num );
		REFUSE_GAME_RETURN        =    0;
		ARX_SOUND_MixerResume( ARX_SOUND_MixerGame );
		return true;
	}

	if ( bFound0 != false )
	{
		INTRO_NOT_LOADED        =    1;
		LoadLevelScreen();
		PROGRESS_BAR_TOTAL        =    238;
		OLD_PROGRESS_BAR_COUNT    =    PROGRESS_BAR_COUNT=0;
		PROGRESS_BAR_COUNT        +=    1.f;
		LoadLevelScreen( save_l[iNbSave0].level );
		DanaeClearLevel();
		ARX_CHANGELEVEL_Load( save_l[iNbSave0].num );
		REFUSE_GAME_RETURN        =    0;
		ARX_SOUND_MixerResume( ARX_SOUND_MixerGame );
		return true;
	}

	if ( bFound1 != false )
	{
		INTRO_NOT_LOADED        =    1;
		LoadLevelScreen();
		PROGRESS_BAR_TOTAL        =    238;
		OLD_PROGRESS_BAR_COUNT    =    PROGRESS_BAR_COUNT=0;
		PROGRESS_BAR_COUNT        +=    1.f;
		LoadLevelScreen( save_l[iNbSave1].level );
		DanaeClearLevel();
		ARX_CHANGELEVEL_Load( save_l[iNbSave1].num );
		REFUSE_GAME_RETURN        =    0;
		ARX_SOUND_MixerResume( ARX_SOUND_MixerGame );
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------

bool MENU_NoActiveWindow()
{
	if( (!pWindowMenu)||
		((pWindowMenu)&&
		(pWindowMenu->eCurrentMenuState==MAIN)) ) return true;

	return false;
}

void FontRenderText(Font* _pFont, Vec3f pos, const std::string& _pText, Color _c)
{
	if(pTextManage)
	{
		ARX_CHECK_LONG( pos.y );
		ARX_CHECK_LONG( pos.x );
		pTextManage->AddText( _pFont, _pText, pos.x, pos.y, _c);
	}
}

//-----------------------------------------------------------------------------

void to_lower(std::string & str) {
	std::transform( str.begin(), str.end(), str.begin(), ::tolower );
}

//-----------------------------------------------------------------------------

	
void Check_Apply()
{
	if(pMenuElementApply)
	{
		if((config.video.textureSize!=newTextureSize)||
		   (config.video.width!=newWidth)||
		   (config.video.height!=newHeight)||
		   (config.video.bpp!=newBpp)) {
			pMenuElementApply->SetCheckOn();
			((CMenuElementText*)pMenuElementApply)->lColor=((CMenuElementText*)pMenuElementApply)->lOldColor;
		}
		else
		{
			if(((CMenuElementText*)pMenuElementApply)->lColor!=Color(127,127,127))
			{
				pMenuElementApply->SetCheckOff();
				((CMenuElementText*)pMenuElementApply)->lOldColor=((CMenuElementText*)pMenuElementApply)->lColor;
				((CMenuElementText*)pMenuElementApply)->lColor=Color(127,127,127);
			}
		}
	}
}

//-----------------------------------------------------------------------------

static void FadeInOut(float _fVal)
{
	TexturedVertex d3dvertex[4];

	u32 iColor = Color::gray(_fVal).toBGR();
	d3dvertex[0].sx=0;
	d3dvertex[0].sy=0;
	d3dvertex[0].sz=0.f;
	d3dvertex[0].rhw=0.999999f;
	d3dvertex[0].color=iColor;

	d3dvertex[1].sx=static_cast<float>(DANAESIZX);
	d3dvertex[1].sy=0;
	d3dvertex[1].sz=0.f;
	d3dvertex[1].rhw=0.999999f;
	d3dvertex[1].color=iColor;

	d3dvertex[2].sx=0;
	d3dvertex[2].sy=static_cast<float>(DANAESIZY);
	d3dvertex[2].sz=0.f;
	d3dvertex[2].rhw=0.999999f;
	d3dvertex[2].color=iColor;

	d3dvertex[3].sx=static_cast<float>(DANAESIZX);
	d3dvertex[3].sy=static_cast<float>(DANAESIZY);
	d3dvertex[3].sz=0.f;
	d3dvertex[3].rhw=0.999999f;
	d3dvertex[3].color=iColor;

	GRenderer->ResetTexture(0);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::DepthTest, false);
	GRenderer->SetCulling(Renderer::CullNone);

	EERIEDRAWPRIM(Renderer::TriangleStrip, d3dvertex, 4, true);

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);

	GRenderer->SetRenderState(Renderer::DepthTest, true);
	GRenderer->SetCulling(Renderer::CullCCW);
}

//-----------------------------------------------------------------------------

bool ProcessFadeInOut(bool _bFadeIn,float _fspeed)
{
	FadeInOut(fFadeInOut);

	if(!bFade) return true;

	if(_bFadeIn)
	{
		fFadeInOut+=_fspeed*ARXDiffTimeMenu*( 1.0f / 100 );

		if(fFadeInOut>1.f)
		{
			fFadeInOut=1.f;
			bFade=false;
		}
	}
	else
	{
		fFadeInOut-=_fspeed*ARXDiffTimeMenu*( 1.0f / 100 );

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
	if(pMenuCursor == NULL)
		pMenuCursor = new MenuCursor();

	pMenuCursor->Update();

	ARXOldTimeMenu = ARXTimeMenu;
	ARXTimeMenu = ARX_TIME_Get( false );
	ARXDiffTimeMenu = ARXTimeMenu-ARXOldTimeMenu;

	if (ARXDiffTimeMenu < 0) //this mean ArxTimeMenu is reset
		ARXDiffTimeMenu = 0 ;

	GRenderer->GetTextureStage(0)->SetMinFilter(TextureStage::FilterLinear);
	GRenderer->GetTextureStage(0)->SetMagFilter(TextureStage::FilterLinear);

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

		if(ARXmenu.currentmode == AMCM_CREDITS){
			Credits::render();
			return true;
		}

		return false;
	}

	if(!GRenderer->BeginScene())
	{
		return true;
	}

	if(pTextManage)
	{
		pTextManage->Clear();
	}

	GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapClamp);

	GRenderer->SetRenderState(Renderer::Fog, false);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::DepthTest, false);
	GRenderer->SetCulling(Renderer::CullNone);

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

	Color lColor = Color(232, 204, 142);

	if(    (!pMenu)|| ((pMenu)&&(pMenu->bReInitAll)) )
	{
		std::string szMenuText;
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

		pMenu->pTexBackGround = TextureContainer::LoadUI("Graph\\Interface\\menus\\menu_main_background.bmp");

		int iPosMenuPrincipaleX = 370;
	int iPosMenuPrincipaleY=100;
	int iDecMenuPrincipaleY=50;
#define MACRO_MENU_PRINCIPALE(MACRO_button,MACRO_menu,MACRO_locate,MACRO_check){\
		szMenuText = getLocalised( MACRO_locate );\
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
				me->lColor=Color(127,127,127);\
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
		me = new CMenuElementText( -1, hFontControls, arxVersion, RATIO_X(580), RATIO_Y(65), lColor, 1.0f, NOP );
		me->SetCheckOff();
		me->lColor=Color(127,127,127);
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
				((CMenuElementText*)pMenuElementResume)->lColor=Color(127,127,127);
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

			GRenderer->SetRenderState(Renderer::AlphaBlending, false);
			GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapRepeat);
			GRenderer->SetRenderState(Renderer::DepthWrite, true);
			GRenderer->SetRenderState(Renderer::DepthTest, true);
			GRenderer->EndScene();

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
			int    iWindowConsoleOffsetX=(0);
			int    iWindowConsoleOffsetY=(14-10);
			int    iWindowConsoleWidth=(iWindowMenuWidth-iWindowConsoleOffsetX);
			int    iWindowConsoleHeight=(iWindowMenuHeight-iWindowConsoleOffsetY+20);
			///////////////////////

			float fPosX1 = RATIO_X(20);
			float fPosX2 = RATIO_X(200);


			ARX_CHECK_INT(fPosX2);
			int iPosX2    = ARX_CLEAN_WARN_CAST_INT(fPosX2);


			float fPosBack      = RATIO_X(10);
			float fPosBackY      = RATIO_Y(190);
			float fPosNext      = RATIO_X(140);

			float fPosApply   = RATIO_X(240);

			float fPosBDAY      = RATIO_Y(380);

			pWindowMenu = new CWindowMenu(iWindowMenuPosX,iWindowMenuPosY,iWindowMenuWidth,iWindowMenuHeight,1);

			switch(eMenuState)
			{
			//------------------ START NEW_QUEST
			case NEW_QUEST:
				{
					std::string szMenuText;
					bool bBOOL = false;
					ARXMenu_GetResumeGame(bBOOL);

					if (!bBOOL)
					{
						break;
					}

					CMenuElement *me = NULL;
					CWindowMenuConsole *pWindowMenuConsole=new CWindowMenuConsole(iWindowConsoleOffsetX,iWindowConsoleOffsetY,iWindowConsoleWidth,iWindowConsoleHeight,NEW_QUEST);
					szMenuText = getLocalised( "system_menus_main_editquest_confirm" );
					me=new CMenuElementText(-1, hFontMenu, szMenuText,0,0,lColor,1.f, NOP);
					me->bCheck = false;
					pWindowMenuConsole->AddMenuCenter(me);

					szMenuText = getLocalised( "system_menus_main_newquest_confirm" );
					me=new CMenuElementText(-1, hFontMenu, szMenuText,0,0,lColor,1.f, NOP);
					me->bCheck = false;
					pWindowMenuConsole->AddMenuCenter(me);

					CMenuPanel *pPanel = new CMenuPanel();
					szMenuText = getLocalised( "system_yes" );
					szMenuText += "   "; // TODO This space can probably go
					me = new CMenuElementText(BUTTON_MENUNEWQUEST_CONFIRM, hFontMenu, szMenuText, 0, 0,lColor,1.f, NEW_QUEST_ENTER_GAME);
					me->SetPos(RATIO_X(iWindowConsoleWidth - (me->GetWidth() + 10)),0);
					pPanel->AddElementNoCenterIn(me);
					szMenuText = getLocalised( "system_no" );
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosBack, 0,lColor,1.f, MAIN);
					me->SetShortCut(Keyboard::Key_Escape);
					pPanel->AddElementNoCenterIn(me);


					ARX_CHECK_INT(fPosBDAY);

					pPanel->Move(0,
								ARX_CLEAN_WARN_CAST_INT(fPosBDAY)    );

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
					std::string szMenuText;
					CWindowMenuConsole *pWindowMenuConsole=new CWindowMenuConsole(iWindowConsoleOffsetX,iWindowConsoleOffsetY,iWindowConsoleWidth,iWindowConsoleHeight,EDIT_QUEST);

					szMenuText = getLocalised( "system_menus_main_editquest_load");
					me = new CMenuElementText(BUTTON_MENUEDITQUEST_LOAD_INIT, hFontMenu, szMenuText, 0, 0, lColor, 1.f, EDIT_QUEST_LOAD);
					pWindowMenuConsole->AddMenuCenter(me);

					szMenuText = getLocalised( "system_menus_main_editquest_save");
					me = new CMenuElementText(-1, hFontMenu, szMenuText, 0, 0, lColor, 1.f, EDIT_QUEST_SAVE);
					bool bBOOL;
					ARXMenu_GetResumeGame(bBOOL);

					if(!FINAL_RELEASE) bBOOL=true;

					if (!bBOOL)
					{
						me->SetCheckOff();
						((CMenuElementText*)me)->lColor=Color(127,127,127);
					}

					pWindowMenuConsole->AddMenuCenter(me);

					pTex = TextureContainer::Load("\\Graph\\interface\\menus\\back.bmp");
					me = new CMenuCheckButton(-1, fPosBack, fPosBackY, pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
					me->eMenuState = MAIN;
					me->SetShortCut(Keyboard::Key_Escape);
					pWindowMenuConsole->AddMenu(me);

					pWindowMenu->eCurrentMenuState = EDIT_QUEST;
					pWindowMenu->AddConsole(pWindowMenuConsole);

					// LOAD ---------------------------------------------------
					pWindowMenuConsole=new CWindowMenuConsole(iWindowConsoleOffsetX,iWindowConsoleOffsetY-(40),iWindowConsoleWidth,iWindowConsoleHeight,EDIT_QUEST_LOAD);
					pWindowMenuConsole->iInterligne = 5;

					pTex = TextureContainer::Load("\\Graph\\interface\\icons\\Menu_main_Load.bmp");
					me = new CMenuCheckButton(-1, 0, 0, pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
					((CMenuCheckButton *)me)->bCheck = false;
					pWindowMenuConsole->AddMenuCenter(me);
					{
						//QUICK LOAD
						std::string szMenuText = QUICK_SAVE_ID;
						std::string szMenuText1 = QUICK_SAVE_ID1;

						//LOAD
						int iFirst=2;
						bool b1 = false;
						bool b2 = false;

						while(iFirst>=0)
						{
							for(size_t iI = 1; iI < save_l.size(); iI++) {
								std::string tex = save_l[iI].name;

								CMenuElementText *me02;

								std::string tex2;
								tex2.resize(tex.size());
								std::transform(tex.begin(), tex.end(), tex2.begin(), ::toupper);

								if( !szMenuText.compare( tex2 ) || !szMenuText1.compare( tex2 ) )
								{
									if(!iFirst || (b1 && b2)) continue;

									tex = getLocalised( "system_menus_main_quickloadsave", "Quick");

									if ( szMenuText.find( tex2) != std::string::npos )
									{
										if (b1) continue;

										b1 = true;
									}
									else if ( szMenuText1.find( tex2 ) != std::string::npos )
									{
										if (b2) continue;

										b2 = true;
									}

									char tex3[256];
									tex += "  ";
									GetDateFormat(    LOCALE_SYSTEM_DEFAULT,
										0,
										&save_l[iI].stime,
										"MMM dd yyyy",
										tex3,
										256);
									tex +=  tex3;
									GetTimeFormatA(    LOCALE_SYSTEM_DEFAULT,
										0,
										&save_l[iI].stime,
										"   HH:mm",
										tex3,
										256);
									tex += tex3;
									
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
									GetDateFormat(    LOCALE_SYSTEM_DEFAULT,
										0,
										&save_l[iI].stime,
										"MMM dd yyyy",
										tex3,
										256);
									strcat(tex4,tex3);
									GetTimeFormatA(    LOCALE_SYSTEM_DEFAULT,
										0,
										&save_l[iI].stime,
										"   HH:mm",
										tex3,
										256);
									strcat(tex4,tex3);
									tex += tex4;
									
									me02=new CMenuElementText(BUTTON_MENUEDITQUEST_LOAD, hFontControls,tex, fPosX1,0.f,lColor, 0.8f, NOP);
								}

								me02->lData=iI;
								pWindowMenuConsole->AddMenuCenterY((CMenuElementText*)me02);
							}

							iFirst--;
						}

						me01 = new CMenuElementText(-1, hFontControls, " ", fPosX1, 0.f, lColor, 0.8f, EDIT_QUEST_SAVE_CONFIRM);
							me01->SetCheckOff();
							pWindowMenuConsole->AddMenuCenterY((CMenuElementText*)me01);

						CMenuPanel *pc = new CMenuPanel();
						szMenuText = getLocalised("system_menus_main_editquest_load");
						szMenuText += "   ";
						me = new CMenuElementText(BUTTON_MENUEDITQUEST_LOAD_CONFIRM, hFontMenu, szMenuText, 0, 0,lColor,1.f, MAIN);

						me->SetPos(RATIO_X(iWindowConsoleWidth-10)-me->GetWidth(), fPosBDAY + RATIO_Y(40));

						pLoadConfirm=(CMenuElementText*)me;
						me->SetCheckOff();
						((CMenuElementText*)me)->lOldColor=((CMenuElementText*)me)->lColor;
						((CMenuElementText*)me)->lColor=Color(127,127,127);

						pWindowMenuConsole->AddMenu(me);
						pTex = TextureContainer::Load("\\Graph\\interface\\menus\\back.bmp");
						me = new CMenuCheckButton(-1, fPosBack, fPosBackY + RATIO_Y(20), pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
						me->eMenuState = EDIT_QUEST;
						me->SetShortCut(Keyboard::Key_Escape);
						pc->AddElementNoCenterIn(me);

						pWindowMenuConsole->AddMenu(pc);
					}
					pWindowMenu->AddConsole(pWindowMenuConsole);

					// SAVE----------------------------------------------------
					pWindowMenuConsole=new CWindowMenuConsole(iWindowConsoleOffsetX,iWindowConsoleOffsetY - (40), iWindowConsoleWidth,iWindowConsoleHeight,EDIT_QUEST_SAVE);
					pWindowMenuConsole->iInterligne = 5;

					pTex = TextureContainer::Load("\\Graph\\interface\\icons\\Menu_main_save.bmp");
					me = new CMenuCheckButton(-1, fPosBack-(pTex?(pTex->m_dwDeviceWidth-pTex->m_dwWidth):0), 0, pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
					((CMenuCheckButton *)me)->bCheck = false;
					pWindowMenuConsole->AddMenuCenter(me);

					//QUICK SAVE
					std::string szMenuText1;
					szMenuText = QUICK_SAVE_ID;
					szMenuText1 = QUICK_SAVE_ID1;

					//SAVE
					int iFirst=2;
					bool b1 = false;
					bool b2 = false;

					while(iFirst>=0)
					{
						if(save_l.size()!=1)
						{
							for(size_t iI = 1; iI < save_l.size(); iI++) {
								std::string tex = save_l[iI].name;
								std::string tex2;
								tex2.resize(tex.size());
								std::transform(tex.begin(), tex.end(), tex2.begin(), ::toupper);

								if(!szMenuText.compare( tex2 ) || !szMenuText1.compare( tex2 ) )
								{
									if(!iFirst || (b1 && b2)) continue;

									tex = getLocalised( "system_menus_main_quickloadsave", "Quick" );

									if ( szMenuText.find( tex2 ) != std::string::npos )
									{
										if (b1) continue;

										b1 = true;
									}
									else if ( szMenuText1.find( tex2 ) != std::string::npos )
									{
										if (b2) continue;

										b2 = true;
									}                                    

									char tex3[256];
									char tex4[256];
									strcpy(tex4,"  ");
									GetDateFormat(    LOCALE_SYSTEM_DEFAULT,
										0,
										&save_l[iI].stime,
										"MMM dd yyyy",
										tex3,
										256);
									strcat(tex4,tex3);
									GetTimeFormatA(    LOCALE_SYSTEM_DEFAULT,
										0,
										&save_l[iI].stime,
										"   HH:mm",
										tex3,
										256);
									strcat(tex4,tex3);
									
									tex += tex4;
									
									me = new CMenuElementText(BUTTON_MENUEDITQUEST_SAVEINFO, hFontControls, tex, fPosX1, 0.f, Color(127, 127, 127), 0.8f, EDIT_QUEST_SAVE_CONFIRM);
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
									GetDateFormat(    LOCALE_SYSTEM_DEFAULT,
										0,
										&save_l[iI].stime,
										"MMM dd yyyy",
										tex3,
										256);
									strcat(tex4,tex3);
									GetTimeFormatA(    LOCALE_SYSTEM_DEFAULT,
										0,
										&save_l[iI].stime,
										"   HH:mm",
										tex3,
										256);
									strcat(tex4,tex3);
									tex += tex4;
									
									me = new CMenuElementText(BUTTON_MENUEDITQUEST_SAVEINFO, hFontControls, tex, fPosX1, 0.f, lColor, 0.8f, EDIT_QUEST_SAVE_CONFIRM);
								}

								me->lData=iI;
								pWindowMenuConsole->AddMenuCenterY(me);
							}
						}

						iFirst--;
					}

					pTex = TextureContainer::Load("\\Graph\\interface\\Icons\\Arx_logo_08.bmp");

					for(int iI=save_l.size(); iI<=15; iI++)
					{
						char tex[256];
						sprintf(tex, "-%04d-"

							,iI);
						CMenuElementText * me01 = new CMenuElementText(-1, hFontControls, tex, fPosX1, 0.f, lColor, 0.8f, EDIT_QUEST_SAVE_CONFIRM);

						me01->eMenuState=EDIT_QUEST_SAVE_CONFIRM;
						me01->lData=0;
						pWindowMenuConsole->AddMenuCenterY((CMenuElementText*)me01);
					}

					me01 = new CMenuElementText(-1, hFontControls, " ", fPosX1, 0.f, lColor, 0.8f, EDIT_QUEST_SAVE_CONFIRM);
					me01->SetCheckOff();
					pWindowMenuConsole->AddMenuCenterY((CMenuElementText*)me01);

					pTex = TextureContainer::Load("\\Graph\\interface\\menus\\back.bmp");
					me = new CMenuCheckButton(-1, fPosBack, fPosBackY + RATIO_Y(20), pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);

					me->eMenuState = EDIT_QUEST;
					me->SetShortCut(Keyboard::Key_Escape);
					pWindowMenuConsole->AddMenu(me);

					pWindowMenu->AddConsole(pWindowMenuConsole);

					// SAVE CONFIRM--------------------------------------------
					pWindowMenuConsole = new CWindowMenuConsole(iWindowConsoleOffsetX,iWindowConsoleOffsetY,iWindowConsoleWidth,iWindowConsoleHeight,EDIT_QUEST_SAVE_CONFIRM);

					pTex = TextureContainer::Load("\\Graph\\interface\\icons\\Menu_main_save.bmp");
					me = new CMenuCheckButton(-1, 0, 0, pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
					((CMenuCheckButton *)me)->bCheck = false;
					pWindowMenuConsole->AddMenuCenter(me);
					
					szMenuText = getLocalised("system_menu_editquest_newsavegame", "---");

					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, NOP);
					me->lData=0;

					pWindowMenuConsole->AddMenuCenter(me);
					me->eState=EDIT;
					me->ePlace=CENTER;

					pPanel = new CMenuPanel();

					szMenuText = getLocalised("system_menus_main_editquest_save", "");

					me = new CMenuElementText(BUTTON_MENUEDITQUEST_SAVE, hFontMenu, szMenuText, 0, 0,lColor,1.f, MAIN);

					me->SetPos(RATIO_X(iWindowConsoleWidth-10)-me->GetWidth(), fPosBDAY);
					pPanel->AddElementNoCenterIn(me);

					pTex = TextureContainer::Load("\\Graph\\interface\\menus\\back.bmp");
					me = new CMenuCheckButton(-1, fPosBack, fPosBackY, pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
					me->eMenuState = EDIT_QUEST_SAVE;
					me->SetShortCut(Keyboard::Key_Escape);
					pPanel->AddElementNoCenterIn(me);

					pWindowMenuConsole->AddMenu(pPanel);

					pWindowMenu->AddConsole(pWindowMenuConsole);
					}
				break;
			case OPTIONS:
				{
					std::string szMenuText;
					CMenuElement *me;
					CMenuPanel *pc;
					TextureContainer *pTex;

					CWindowMenuConsole *pWindowMenuConsole=new CWindowMenuConsole(iWindowConsoleOffsetX,iWindowConsoleOffsetY,iWindowConsoleWidth,iWindowConsoleHeight,OPTIONS);

					szMenuText = getLocalised( "system_menus_options_video", "");
					me = new CMenuElementText(BUTTON_MENUOPTIONSVIDEO_INIT, hFontMenu, szMenuText, 0, 0,lColor,1.f,OPTIONS_VIDEO);
					pWindowMenuConsole->AddMenuCenter(me);
					
					szMenuText = getLocalised( "system_menus_options_audio", "");
					me = new CMenuElementText(-1, hFontMenu, szMenuText, 0, 0,lColor,1.f,OPTIONS_AUDIO);
					pWindowMenuConsole->AddMenuCenter(me);
					
					szMenuText = getLocalised( "system_menus_options_input", "");
					me = new CMenuElementText(-1, hFontMenu, szMenuText, 0, 0,lColor,1.f,OPTIONS_INPUT);
					pWindowMenuConsole->AddMenuCenter(me);

					pTex = TextureContainer::Load("\\Graph\\interface\\menus\\back.bmp");
					me = new CMenuCheckButton(-1, fPosBack, fPosBackY, pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
					me->eMenuState = MAIN;
					me->SetShortCut(Keyboard::Key_Escape);
					pWindowMenuConsole->AddMenu(me);

					pWindowMenu->AddConsole(pWindowMenuConsole);
				//------------------ END OPTIONS

				//------------------ START VIDEO
					pWindowMenuConsole=new CWindowMenuConsole(iWindowConsoleOffsetX,iWindowConsoleOffsetY - (40),iWindowConsoleWidth,iWindowConsoleHeight, OPTIONS_VIDEO);

					pc = new CMenuPanel();
					szMenuText = getLocalised( "system_menus_options_video_resolution", "");
					szMenuText += "  ";
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, NOP);
					me->SetCheckOff();
					pc->AddElement(me);
					int iModeX,iModeY,iModeBpp;
					ARXMenu_Options_Video_GetResolution(iModeX,iModeY,iModeBpp);
					me = new CMenuSliderText(BUTTON_MENUOPTIONSVIDEO_RESOLUTION, 0, 0);
					pMenuSliderResol =(CMenuSliderText*)me;
					int nb=danaeApp.m_pDeviceInfo->dwNumModes;
					std::vector<int> vBpp;
					vBpp.clear();
					int i=0;

					for(;i<nb;i++)
					{
						{
							std::stringstream ss;
							ss << danaeApp.m_pDeviceInfo->pddsdModes[i].dwWidth << 'x' << danaeApp.m_pDeviceInfo->pddsdModes[i].dwHeight;
							szMenuText = ss.str();


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
							std::vector<int>::iterator ii;

							for(ii=vBpp.begin();ii!=vBpp.end();++ii)
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


					float fRatio    = (RATIO_X(iWindowConsoleWidth-9) - me->GetWidth()); 
					ARX_CHECK_INT(fRatio);

					me->Move(    ARX_CLEAN_WARN_CAST_INT(fRatio)    ,0); 


					pc->AddElement(me);

					pWindowMenuConsole->AddMenuCenterY(pc);

					CMenuPanel *pc1 = new CMenuPanel();
					szMenuText = getLocalised("system_menus_options_video_texture");
					szMenuText += " ";
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, NOP);
					me->SetCheckOff();
					pc1->AddElement(me);
					me = new CMenuSliderText(BUTTON_MENUOPTIONSVIDEO_TEXTURES, 0, 0);
					pMenuSliderTexture = (CMenuSliderText*)me;
					szMenuText = getLocalised("system_menus_options_video_texture_low");
					((CMenuSliderText *)me)->AddText(new CMenuElementText(-1, hFontMenu, szMenuText, 0, 0,lColor,1.f, OPTIONS_VIDEO));
					szMenuText = getLocalised("system_menus_options_video_texture_med");
					((CMenuSliderText *)me)->AddText(new CMenuElementText(-1, hFontMenu, szMenuText, 0, 0,lColor,1.f, OPTIONS_VIDEO));
					szMenuText = getLocalised("system_menus_options_video_texture_high");
					((CMenuSliderText *)me)->AddText(new CMenuElementText(-1, hFontMenu, szMenuText, 0, 0,lColor,1.f, OPTIONS_VIDEO));


					fRatio    = (RATIO_X(iWindowConsoleWidth-9) - me->GetWidth()); 
					ARX_CHECK_INT(fRatio);

					me->Move(    ARX_CLEAN_WARN_CAST_INT(fRatio)    ,0); 


					int iSize = me->GetWidth();
					pc1->AddElement(me);
					int iQuality = 0;
					ARXMenu_Options_Video_GetTextureQuality(iQuality);
					((CMenuSliderText *)me)->iPos = iQuality;

					pc = new CMenuPanel();
					szMenuText = getLocalised("system_menus_options_video_bpp");
					szMenuText += " ";
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, NOP);
					me->SetCheckOff();
					pc->AddElement(me);
					me = new CMenuSliderText(BUTTON_MENUOPTIONSVIDEO_BPP, 0, 0);
					pMenuSliderBpp = (CMenuSliderText*)me;

					std::vector<int>::iterator ii;

					for(ii=vBpp.begin();ii!=vBpp.end();++ii)
					{
						std::stringstream bpp;
						bpp << *ii;
						((CMenuSliderText*)me)->AddText(new CMenuElementText(-1, hFontMenu, bpp.str(), 0, 0, lColor, 1.f, (MENUSTATE)(BUTTON_MENUOPTIONSVIDEO_BPP+i)));

						if(*ii==iModeBpp)
						{
							((CMenuSliderText*)me)->iPos = ((CMenuSliderText*)me)->vText.size()-1;
						}
					}

					((CMenuSliderText *)me)->SetWidth(iSize);


					fRatio    = (RATIO_X(iWindowConsoleWidth-9) - me->GetWidth()); 
					ARX_CHECK_INT(fRatio);

					me->Move(    ARX_CLEAN_WARN_CAST_INT(fRatio)    ,0); 


					pc->AddElement(me);
					pWindowMenuConsole->AddMenuCenterY(pc);

					pWindowMenuConsole->AddMenuCenterY(pc1);
					pc = new CMenuPanel();
					szMenuText = getLocalised("system_menus_options_detail");
					szMenuText += " ";
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, NOP);
					me->SetCheckOff();
					pc->AddElement(me);
					me = new CMenuSliderText(BUTTON_MENUOPTIONSVIDEO_OTHERSDETAILS, 0, 0);
					szMenuText = getLocalised("system_menus_options_video_texture_low");
					((CMenuSliderText *)me)->AddText(new CMenuElementText(-1, hFontMenu, szMenuText, 0, 0,lColor,1.f, OPTIONS_OTHERDETAILS));
					szMenuText = getLocalised("system_menus_options_video_texture_med");
					((CMenuSliderText *)me)->AddText(new CMenuElementText(-1, hFontMenu, szMenuText, 0, 0,lColor,1.f, OPTIONS_OTHERDETAILS));
					szMenuText = getLocalised("system_menus_options_video_texture_high");
					((CMenuSliderText *)me)->AddText(new CMenuElementText(-1, hFontMenu, szMenuText, 0, 0,lColor,1.f, OPTIONS_OTHERDETAILS));

					
					fRatio    = (RATIO_X(iWindowConsoleWidth-9) - me->GetWidth()); 
					ARX_CHECK_INT(fRatio);

					me->Move(    ARX_CLEAN_WARN_CAST_INT(fRatio)    ,0); 


					pc->AddElement(me);
					iQuality = 0;
					ARXMenu_Options_Video_GetDetailsQuality(iQuality);
					((CMenuSliderText *)me)->iPos = iQuality;

					pWindowMenuConsole->AddMenuCenterY(pc);

					bool bBOOL = false;
					
					pc = new CMenuPanel();
					szMenuText = getLocalised("system_menus_options_video_brouillard");
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, NOP);
					me->SetCheckOff();
					pc->AddElement(me);
					me = new CMenuSlider(BUTTON_MENUOPTIONSVIDEO_FOG, iPosX2, 0);
					int iFog = 5;
					ARXMenu_Options_Video_GetFogDistance(iFog);
					((CMenuSlider *)me)->setValue(iFog);
					pc->AddElement(me);

					pWindowMenuConsole->AddMenuCenterY(pc);

					pc = new CMenuPanel();
					szMenuText = getLocalised("system_menus_options_video_gamma");
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, NOP);
					me->SetCheckOff();
					pc->AddElement(me);
					me = new CMenuSlider(BUTTON_MENUOPTIONSVIDEO_GAMMA, iPosX2, 0);
					int iGamma = 0;
					ARXMenu_Options_Video_GetGamma(iGamma);
					((CMenuSlider*)me)->setValue(iGamma);
					pc->AddElement(me);
					pWindowMenuConsole->AddMenuCenterY(pc);

					pc = new CMenuPanel();
					szMenuText = getLocalised("system_menus_options_video_luminosity", "luminosity");
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, NOP);
					me->SetCheckOff();
					pc->AddElement(me);
					me = new CMenuSlider(BUTTON_MENUOPTIONSVIDEO_LUMINOSITY, iPosX2, 0);
					int iLum = 0;
					ARXMenu_Options_Video_GetLuminosity(iLum);
					((CMenuSlider*)me)->setValue(iLum);
					pc->AddElement(me);
					pWindowMenuConsole->AddMenuCenterY(pc);

					pc = new CMenuPanel();
					szMenuText = getLocalised("system_menus_options_video_contrast", "contrast");
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, NOP);
					me->SetCheckOff();
					pc->AddElement(me);
					me = new CMenuSlider(BUTTON_MENUOPTIONSVIDEO_CONTRAST, iPosX2, 0);
					int iContrast = 0;
					ARXMenu_Options_Video_GetContrast(iContrast);
					((CMenuSlider*)me)->setValue(iContrast);
					pc->AddElement(me);
					pWindowMenuConsole->AddMenuCenterY(pc);

					szMenuText = getLocalised("system_menus_options_video_crosshair", "Show Crosshair");
					szMenuText += " ";
					TextureContainer *pTex1 = TextureContainer::Load("\\Graph\\interface\\menus\\menu_checkbox_off.bmp");
					TextureContainer *pTex2 = TextureContainer::Load("\\Graph\\interface\\menus\\menu_checkbox_on.bmp");
					CMenuElementText * metemp = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, NOP);
					metemp->SetCheckOff();
					me = new CMenuCheckButton(BUTTON_MENUOPTIONSVIDEO_CROSSHAIR, 0, 0, pTex1->m_dwWidth, pTex1, pTex2, metemp);

					((CMenuCheckButton*)me)->iState= config.video.showCrosshair ? 1 : 0;

					pWindowMenuConsole->AddMenuCenterY(me);

					szMenuText = getLocalised("system_menus_options_video_antialiasing", "antialiasing");
					szMenuText += " ";
					pTex1 = TextureContainer::Load("\\Graph\\interface\\menus\\menu_checkbox_off.bmp");
					pTex2 = TextureContainer::Load("\\Graph\\interface\\menus\\menu_checkbox_on.bmp");
					metemp = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, NOP);
					metemp->SetCheckOff();
					me = new CMenuCheckButton(BUTTON_MENUOPTIONSVIDEO_ANTIALIASING, 0, 0, pTex1->m_dwWidth, pTex1, pTex2, metemp);

					((CMenuCheckButton*)me)->iState= config.video.antialiasing ? 1 : 0;

					pWindowMenuConsole->AddMenuCenterY(me);
					ARX_SetAntiAliasing();

					pc = new CMenuPanel();
					szMenuText = getLocalised("system_menus_video_apply");
					szMenuText += "   ";
					pMenuElementApply = me = new CMenuElementText(BUTTON_MENUOPTIONSVIDEO_APPLY, hFontMenu, szMenuText, fPosApply, 0.f, lColor, 1.f, NOP);
					me->SetPos(RATIO_X(iWindowConsoleWidth-10)-me->GetWidth(), fPosBDAY + RATIO_Y(40));
					me->SetCheckOff();
					pc->AddElementNoCenterIn(me);

					pTex = TextureContainer::Load("\\Graph\\interface\\menus\\back.bmp");
					me = new CMenuCheckButton(BUTTON_MENUOPTIONSVIDEO_BACK, fPosBack, fPosBackY + RATIO_Y(20), pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
					me->eMenuState = OPTIONS;
					me->SetShortCut(Keyboard::Key_Escape);
					pc->AddElementNoCenterIn(me);

					pWindowMenuConsole->AddMenu(pc);

					pWindowMenu->AddConsole(pWindowMenuConsole);
					//------------------ END VIDEO

					//------------------ START AUDIO
					pWindowMenuConsole = new CWindowMenuConsole(iWindowConsoleOffsetX,iWindowConsoleOffsetY,iWindowConsoleWidth,iWindowConsoleHeight,OPTIONS_AUDIO);

					pc = new CMenuPanel();
					szMenuText = getLocalised("system_menus_options_audio_master_volume");
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, OPTIONS_AUDIO_VOLUME);
					me->SetCheckOff();
					pc->AddElement(me);
					me = new CMenuSlider(BUTTON_MENUOPTIONSAUDIO_MASTER, iPosX2, 0);
					((CMenuSlider *)me)->setValue((int)config.audio.volume); // TODO use float sliders
					pc->AddElement(me);
					pWindowMenuConsole->AddMenuCenterY(pc);

					pc = new CMenuPanel();
					szMenuText = getLocalised("system_menus_options_audio_effects_volume");
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, OPTIONS_AUDIO);
					me->SetCheckOff();
					pc->AddElement(me);
					me = new CMenuSlider(BUTTON_MENUOPTIONSAUDIO_SFX, iPosX2, 0);
					((CMenuSlider *)me)->setValue((int)config.audio.sfxVolume);
					pc->AddElement(me);
					pWindowMenuConsole->AddMenuCenterY(pc);

					pc = new CMenuPanel();
					szMenuText = getLocalised("system_menus_options_audio_speech_volume");
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, OPTIONS_AUDIO);
					me->SetCheckOff();
					pc->AddElement(me);
					me = new CMenuSlider(BUTTON_MENUOPTIONSAUDIO_SPEECH, iPosX2, 0);
					((CMenuSlider *)me)->setValue((int)config.audio.speechVolume);
					pc->AddElement(me);
					pWindowMenuConsole->AddMenuCenterY(pc);

					pc = new CMenuPanel();
					szMenuText = getLocalised("system_menus_options_audio_ambiance_volume");
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, OPTIONS_AUDIO);
					me->SetCheckOff();
					pc->AddElement(me);
					me = new CMenuSlider(BUTTON_MENUOPTIONSAUDIO_AMBIANCE, iPosX2, 0);
					((CMenuSlider *)me)->setValue((int)config.audio.ambianceVolume);
					pc->AddElement(me);
					pWindowMenuConsole->AddMenuCenterY(pc);

					szMenuText = getLocalised("system_menus_options_audio_eax", "EAX");
					szMenuText += " ";
					pTex1 = TextureContainer::Load("\\Graph\\interface\\menus\\menu_checkbox_off.bmp");
					pTex2 = TextureContainer::Load("\\Graph\\interface\\menus\\menu_checkbox_on.bmp");
					CMenuElementText * pElementText = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, OPTIONS_INPUT);
					me = new CMenuCheckButton(BUTTON_MENUOPTIONSAUDIO_EAX, 0, 0, pTex1->m_dwWidth, pTex1, pTex2, pElementText);
					((CMenuCheckButton*)me)->iState = config.audio.eax ? 1 : 0;

					pWindowMenuConsole->AddMenuCenterY(me);

					pTex = TextureContainer::Load("\\Graph\\interface\\menus\\back.bmp");
					me = new CMenuCheckButton(-1, fPosBack, fPosBackY, pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
					me->eMenuState = OPTIONS;
					me->SetShortCut(Keyboard::Key_Escape);
					pWindowMenuConsole->AddMenu(me);

					pWindowMenu->AddConsole(pWindowMenuConsole);
					//------------------ END AUDIO

					//------------------ START INPUT
					pWindowMenuConsole = new CWindowMenuConsole(iWindowConsoleOffsetX,iWindowConsoleOffsetY,iWindowConsoleWidth,iWindowConsoleHeight, OPTIONS_INPUT);
					
					szMenuText = getLocalised("system_menus_options_input_customize_controls");
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, OPTIONS_INPUT_CUSTOMIZE_KEYS_1);
					pWindowMenuConsole->AddMenuCenterY(me);
					
					szMenuText = getLocalised("system_menus_options_input_invert_mouse");
					szMenuText += " ";
					pTex1 = TextureContainer::Load("\\Graph\\interface\\menus\\menu_checkbox_off.bmp");
					pTex2 = TextureContainer::Load("\\Graph\\interface\\menus\\menu_checkbox_on.bmp");
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

					szMenuText = getLocalised("system_menus_options_auto_ready_weapon");
					szMenuText += " ";
					pTex1 = TextureContainer::Load("\\Graph\\interface\\menus\\menu_checkbox_off.bmp");
					pTex2 = TextureContainer::Load("\\Graph\\interface\\menus\\menu_checkbox_on.bmp");
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

					szMenuText = getLocalised( "system_menus_options_input_mouse_look_toggle" );
					szMenuText += " ";
					pTex1 = TextureContainer::Load("\\Graph\\interface\\menus\\menu_checkbox_off.bmp");
					pTex2 = TextureContainer::Load("\\Graph\\interface\\menus\\menu_checkbox_on.bmp");
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
					szMenuText = getLocalised( "system_menus_options_input_mouse_sensitivity" );
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosX1, 0.f, lColor, 1.f, NOP);
					me->SetCheckOff();
					pc->AddElement(me);
					me = new CMenuSlider(BUTTON_MENUOPTIONS_CONTROLS_MOUSESENSITIVITY, iPosX2, 0);
					int iSensitivity = 0;
					ARXMenu_Options_Control_GetMouseSensitivity(iSensitivity);
					((CMenuSlider*)me)->setValue(iSensitivity);
					pc->AddElement(me);
					pWindowMenuConsole->AddMenuCenterY(pc);

					if (config.misc.newControl)
					{
						szMenuText = getLocalised( "system_menus_options_input_mouse_smoothing", "mouse_smoothing" );
						szMenuText += " ";
						pTex1 = TextureContainer::Load("\\Graph\\interface\\menus\\menu_checkbox_off.bmp");
						pTex2 = TextureContainer::Load("\\Graph\\interface\\menus\\menu_checkbox_on.bmp");
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

						szMenuText = getLocalised( "system_menus_autodescription", "auto_description" );
						szMenuText += " ";
						pTex1 = TextureContainer::Load("\\Graph\\interface\\menus\\menu_checkbox_off.bmp");
						pTex2 = TextureContainer::Load("\\Graph\\interface\\menus\\menu_checkbox_on.bmp");
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

					pTex = TextureContainer::Load("\\Graph\\interface\\menus\\back.bmp");
					me = new CMenuCheckButton(-1, fPosBack, fPosBackY, pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
					me->eMenuState = OPTIONS;
					me->SetShortCut(Keyboard::Key_Escape);
					pWindowMenuConsole->AddMenu(me);
					pWindowMenu->AddConsole(pWindowMenuConsole);
				//------------------ END INPUT

				//------------------ START CUSTOM CONTROLS
				char pNoDef1[]="---";
				char pNoDef2[]="---";

				#define CUSTOM_CTRL_X0    RATIO_X(20)
				#define CUSTOM_CTRL_X1    RATIO_X(150)
				#define CUSTOM_CTRL_X2    RATIO_X(245)
					long fControlPosY    =    ARX_CLEAN_WARN_CAST_LONG(RATIO_Y(8.f));
				#define CUSTOM_CTRL_FUNC(a,b,c,d){\
						pc=new CMenuPanel();\
						szMenuText = getLocalised(a, "?");\
						me = new CMenuElementText(-1, hFontControls, szMenuText, CUSTOM_CTRL_X0, 0,lColor,.7f, NOP);\
						me->SetCheckOff();\
						pc->AddElement(me);\
						me = new CMenuElementText(c, hFontControls, pNoDef1, CUSTOM_CTRL_X1, 0,lColor,.7f, NOP);\
						me->eState=GETTOUCH;\
						if((!b)||(c<0))\
						{\
							me->SetCheckOff();\
							((CMenuElementText*)me)->lColor=Color(127,127,127);\
						}\
						pc->AddElement(me);\
						me = new CMenuElementText(d, hFontControls, pNoDef2, CUSTOM_CTRL_X2, 0,lColor,.7f, NOP);\
						me->eState=GETTOUCH;\
						if(d<0)\
						{\
							me->SetCheckOff();\
							((CMenuElementText*)me)->lColor=Color(127,127,127);\
						}\
						pc->AddElement(me);\
						pc->Move(0,fControlPosY);\
						pWindowMenuConsole->AddMenu(pc);\
						fControlPosY += ARX_CLEAN_WARN_CAST_LONG( pc->GetHeight() + RATIO_Y(3.f) );\
					};


				#define CUSTOM_CTRL_FUNC2(a,b,c,d){\
						pc=new CMenuPanel();\
						szMenuText = getLocalised(a, "?");\
						szMenuText += "2";\
						me = new CMenuElementText(-1, hFontControls, szMenuText, CUSTOM_CTRL_X0, 0,lColor,.7f, NOP);\
						me->SetCheckOff();\
						pc->AddElement(me);\
						me = new CMenuElementText(c, hFontControls, pNoDef1, CUSTOM_CTRL_X1, 0,lColor,.7f, NOP);\
						me->eState=GETTOUCH;\
						if((!b)||(c<0))\
						{\
							me->SetCheckOff();\
							((CMenuElementText*)me)->lColor=Color(127,127,127);\
						}\
						pc->AddElement(me);\
						me = new CMenuElementText(d, hFontControls, pNoDef2, CUSTOM_CTRL_X2, 0,lColor,.7f, NOP);\
						me->eState=GETTOUCH;\
						if(d<0)\
						{\
							me->SetCheckOff();\
							((CMenuElementText*)me)->lColor=Color(127,127,127);\
						}\
						pc->AddElement(me);\
						pc->Move(0,fControlPosY);\
						pWindowMenuConsole->AddMenu(pc);\
						fControlPosY += ARX_CLEAN_WARN_CAST_LONG( pc->GetHeight() + RATIO_Y(3.f) );\
					};


					pWindowMenuConsole=new CWindowMenuConsole(iWindowConsoleOffsetX,iWindowConsoleOffsetY,iWindowConsoleWidth,iWindowConsoleHeight,OPTIONS_INPUT_CUSTOMIZE_KEYS_1);

					CUSTOM_CTRL_FUNC("system_menus_options_input_customize_controls_mouselook",1, BUTTON_MENUOPTIONS_CONTROLS_CUST_MOUSELOOK1, BUTTON_MENUOPTIONS_CONTROLS_CUST_MOUSELOOK2);

					if (!config.misc.newControl)
					{
						szMenuText = getLocalised( "system_menus_options_input_customize_controls_link_use_to_mouselook", "?" );
						\
				pTex1 = TextureContainer::Load("\\Graph\\interface\\menus\\menu_checkbox_off.bmp");
				pTex2 = TextureContainer::Load("\\Graph\\interface\\menus\\menu_checkbox_on.bmp");
				pElementText= new CMenuElementText(-1, hFontControls, szMenuText, CUSTOM_CTRL_X0, 0,lColor,.7f, NOP);
				me = new CMenuCheckButton(BUTTON_MENUOPTIONS_CONTROLS_LINK, 0, 0, pTex1->m_dwWidth>>1, pTex1, pTex2, pElementText);
				me->Move(0,fControlPosY);
						pWindowMenuConsole->AddMenu(me);
						fControlPosY += ARX_CLEAN_WARN_CAST_LONG(me->GetHeight() + RATIO_Y(3.f));

						if(config.input.linkMouseLookToUse)
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

					pTex = TextureContainer::Load("\\Graph\\interface\\menus\\back.bmp");
					me = new CMenuCheckButton(BUTTON_MENUOPTIONS_CONTROLS_CUST_BACK, fPosBack, fPosBackY, pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
					me->eMenuState = OPTIONS_INPUT;
					me->SetShortCut(Keyboard::Key_Escape);
					pc->AddElementNoCenterIn(me);
					szMenuText = getLocalised( "system_menus_options_input_customize_default" );
					me = new CMenuElementText(BUTTON_MENUOPTIONS_CONTROLS_CUST_DEFAULT, hFontMenu, szMenuText, 0, 0,lColor,1.f, NOP);
					me->SetPos((RATIO_X(iWindowConsoleWidth) - me->GetWidth())*0.5f, fPosBDAY);
					pc->AddElementNoCenterIn(me);
					pTex = TextureContainer::Load("\\Graph\\interface\\menus\\next.bmp");
					me = new CMenuCheckButton(BUTTON_MENUOPTIONS_CONTROLS_CUST_BACK, fPosNext, fPosBackY, pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
					me->eMenuState = OPTIONS_INPUT_CUSTOMIZE_KEYS_2;
					me->SetShortCut(Keyboard::Key_Escape);
					pc->AddElementNoCenterIn(me);

					pWindowMenuConsole->AddMenu(pc);

					pWindowMenu->AddConsole(pWindowMenuConsole);
					pWindowMenuConsole->ReInitActionKey();

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

					pTex = TextureContainer::Load("\\Graph\\interface\\menus\\back.bmp");
					me = new CMenuCheckButton(BUTTON_MENUOPTIONS_CONTROLS_CUST_BACK, fPosBack, fPosBackY, pTex?pTex->m_dwWidth:0, pTex, NULL, NULL);
					me->eMenuState = OPTIONS_INPUT_CUSTOMIZE_KEYS_1;
					me->SetShortCut(Keyboard::Key_Escape);
					pc->AddElementNoCenterIn(me);
					szMenuText = getLocalised( "system_menus_options_input_customize_default" );
					me = new CMenuElementText(BUTTON_MENUOPTIONS_CONTROLS_CUST_DEFAULT, hFontMenu, szMenuText, 0, 0,lColor,1.f, NOP);
					me->SetPos((RATIO_X(iWindowConsoleWidth) - me->GetWidth())*0.5f, fPosBDAY);
					pc->AddElementNoCenterIn(me);

					pWindowMenuConsole->AddMenu(pc);

					pWindowMenu->AddConsole(pWindowMenuConsole);
					pWindowMenuConsole->ReInitActionKey();
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
					std::string szMenuText;
					CMenuElement *me = NULL;
					CWindowMenuConsole *pWindowMenuConsole=new CWindowMenuConsole(iWindowConsoleOffsetX,iWindowConsoleOffsetY,iWindowConsoleWidth,iWindowConsoleHeight,QUIT);
					szMenuText = getLocalised( "system_menus_main_quit" );
					me=new CMenuElementText(-1, hFontMenu, szMenuText,0,0,lColor,1.f, NOP);
					me->bCheck = false;
					pWindowMenuConsole->AddMenuCenter(me);

					szMenuText = getLocalised( "system_menus_main_editquest_confirm" );
					me=new CMenuElementText(-1, hFontMenu, szMenuText,0,0,lColor,1.f, NOP);
					me->bCheck = false;
					pWindowMenuConsole->AddMenuCenter(me);

					CMenuPanel *pPanel = new CMenuPanel();
					szMenuText = getLocalised( "system_yes" ); // TODO Is case sensitive, fix pak

					me = new CMenuElementText(BUTTON_MENUMAIN_QUIT, hFontMenu, szMenuText, 0, 0,lColor,1.f, NEW_QUEST_ENTER_GAME);

					me->SetPos(RATIO_X(iWindowConsoleWidth-10)-me->GetWidth(), 0);
					pPanel->AddElementNoCenterIn(me);
					szMenuText = getLocalised( "system_no" ); // TODO Is case sensitive, fix pak
					me = new CMenuElementText(-1, hFontMenu, szMenuText, fPosBack, 0,lColor,1.f, MAIN);
					me->SetShortCut(Keyboard::Key_Escape);
					pPanel->AddElementNoCenterIn(me);


					ARX_CHECK_INT(fPosBDAY);
					pPanel->Move(0,ARX_CLEAN_WARN_CAST_INT(fPosBDAY));
					pWindowMenuConsole->AddMenu(pPanel);
					pWindowMenu->AddConsole(pWindowMenuConsole);
					pWindowMenu->eCurrentMenuState=QUIT;

				}
				break;
				
			default: break; // Unhandled menu state.
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

	// If the menu needs to be reinitialized, then the text in the TextManager is probably using bad fonts that were deleted already
	// Skip one update in this case
	if(pTextManage && !pMenu->bReInitAll)
	{
		pTextManage->Update(ARXDiffTimeMenu);
		pTextManage->Render();
	}

	GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapClamp);

	GRenderer->SetRenderState(Renderer::Fog, false);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::DepthTest, false);
	GRenderer->SetCulling(Renderer::CullNone);
	pMenuCursor->DrawCursor();

	if(pMenu->bReInitAll)
	{
		GRenderer->Clear(Renderer::ColorBuffer | Renderer::DepthBuffer);

		if(bForceReInitAllTexture)
		{
			GRenderer->RestoreAllTextures();
			bForceReInitAllTexture=false;
		}
	}

	if (pTextureLoadRender)
	{
		GRenderer->SetRenderState(Renderer::DepthTest, false);

		int iOffsetX = 0;
		int iOffsetY=0;

		if ((DANAEMouse.y + INTERFACE_RATIO_DWORD(pTextureLoad->m_dwHeight)) > DANAESIZY)
		{
			
			float fOffestY    = iOffsetY - INTERFACE_RATIO_DWORD(pTextureLoad->m_dwHeight) ;
			ARX_CHECK_INT(fOffestY);
			iOffsetY    =    ARX_CLEAN_WARN_CAST_INT(fOffestY);


		}

		EERIEDrawBitmap(static_cast<float>(DANAEMouse.x + iOffsetX),
		                static_cast<float>(DANAEMouse.y + iOffsetY),
		                (float)INTERFACE_RATIO_DWORD(pTextureLoad->m_dwWidth),
		                (float)INTERFACE_RATIO_DWORD(pTextureLoad->m_dwHeight),
		                0.001f, pTextureLoad, Color::white);

		GRenderer->ResetTexture(0);
		EERIEDraw2DRect(static_cast<float>(DANAEMouse.x + iOffsetX),
		                static_cast<float>(DANAEMouse.y + iOffsetY),
		                DANAEMouse.x + iOffsetX + (float)INTERFACE_RATIO_DWORD(pTextureLoad->m_dwWidth),
		                DANAEMouse.y + iOffsetY + (float)INTERFACE_RATIO_DWORD(pTextureLoad->m_dwHeight),
		                0.01f, Color::white);

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
#ifdef BUILD_EDITOR
			GAME_EDITOR = 0;
#endif
			ARX_MENU_Clicked_QUIT_GAME();
			iFadeAction=-1;
			bFadeInOut=false;
			bFade=true;
			break;
		}
	}

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->GetTextureStage(0)->SetMinFilter(TextureStage::FilterLinear);
	GRenderer->GetTextureStage(0)->SetMagFilter(TextureStage::FilterLinear);
	GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapRepeat);

	GRenderer->SetRenderState(Renderer::DepthWrite, true);

	GRenderer->SetRenderState(Renderer::DepthTest, true);
	GRenderer->SetCulling(Renderer::CullCCW);

	GRenderer->EndScene();
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
}

//-----------------------------------------------------------------------------

CMenuElement* CMenuElement::OnShortCut()
{
	if(iShortCut==-1) return NULL;

	if( GInput->isKeyPressedNowUnPressed(iShortCut) )
	{
		return this;
	}

	return NULL;
}

//-----------------------------------------------------------------------------

CMenuElementText::CMenuElementText(int _iID, Font* _pFont, const std::string& _pText,float _fPosX,float _fPosY,Color _lColor,float _fSize,MENUSTATE _eMs) : CMenuElement(_eMs)
{
	iID = _iID;

	pFont = _pFont;

	if( !_pText.compare( "---") )
	{
		bTestYDouble=true;
	}

	lpszText= _pText;

	ARX_CHECK_LONG(_fPosX);
	ARX_CHECK_LONG(_fPosY);

	Vec2i textSize = _pFont->GetTextSize(_pText);
	rZone.left = ARX_CLEAN_WARN_CAST_LONG(_fPosX);
	rZone.top = ARX_CLEAN_WARN_CAST_LONG(_fPosY);
	rZone.right  = rZone.left + textSize.x;
	rZone.bottom = rZone.top + textSize.y;

	lColor=_lColor;
	lColorHighlight=lOldColor=Color(255, 255, 255);

	fSize=_fSize;
	pRef=this;

	bSelected = false;

	iPosCursor = _pText.length() + 1;
}

//-----------------------------------------------------------------------------

CMenuElementText::~CMenuElementText()
{
}

//-----------------------------------------------------------------------------

void CMenuElementText::SetText( const std::string& _pText )
{
	lpszText = _pText;

	Vec2i textSize = pFont->GetTextSize(_pText);

	rZone.right  = textSize.x + rZone.left;
	rZone.bottom = textSize.y + rZone.top;
}

//-----------------------------------------------------------------------------

void CMenuElementText::Update(int _iDTime) {
	(void)_iDTime;
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
bool CMenuElementText::OnMouseClick(int _iMouseButton) {
	
	(void)_iMouseButton;
	
	switch(eState)
	{
	case EDIT:
		eState=EDIT_TIME;
		return true;
	case GETTOUCH:
		eState=GETTOUCH_TIME;
		lOldColor=lColorHighlight;
		return true;
	default: break;
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
			newWidth = config.video.width;
			newHeight = config.video.height;
			newBpp = config.video.bpp;
			newTextureSize = config.video.textureSize;

			changeResolution = false;
			changeTextures = false;
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
						eMenuState = MAIN;
						GRenderer->Clear(Renderer::DepthBuffer);
						ARXMenu_LoadQuest( lData );
						bNoMenu=true;
						if(pTextManage) {
							pTextManage->Clear();
						}
						break;
					}
				}
			}

			pLoadConfirm->SetCheckOff();
			pLoadConfirm->lColor=Color( 127, 127, 127 );
			}
		}
		break;
	case BUTTON_MENUEDITQUEST_LOAD_CONFIRM_BACK:
		pLoadConfirm->SetCheckOff();
		pLoadConfirm->lColor=Color(127,127,127);
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

					if(me) {
						save_l[me->lData].name = me->lpszText;
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

					if(me) {
						save_l[me->lData].name = me->lpszText;
						eMenuState = MAIN;
						ARXMenu_DeleteQuest( me->lData );
						CreateSaveGameList();
						break;
					}
				}
			}
		}
		break;
	case BUTTON_MENUOPTIONSVIDEO_APPLY:
		{
			//----------CHANGE_TEXTURE
			if(newTextureSize!=config.video.textureSize)
			{
				config.video.textureSize=newTextureSize;

				if(config.video.textureSize==2)Project.TextureSize=0;

				if(config.video.textureSize==1)Project.TextureSize=2;

				if(config.video.textureSize==0)Project.TextureSize=64;

				WILL_RELOAD_ALL_TEXTURES=1;

				pMenuSliderTexture->iOldPos=-1;
			}

			//----------END_CHANGE_TEXTURE

			//----------RESOLUTION
			if(    (newWidth!=config.video.width)||
				(newHeight!=config.video.height)||
				(newBpp!=config.video.bpp) )
			{
				config.video.width=newWidth;
				config.video.height=newHeight;
				config.video.bpp=newBpp;
				ARXMenu_Private_Options_Video_SetResolution(    config.video.width,
																config.video.height,
																config.video.bpp);

				pMenuSliderResol->iOldPos=-1;
				pMenuSliderBpp->iOldPos=-1;
			}

			//----------END_RESOLUTION
			changeResolution = false;
			changeTextures = false;
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
			config.save();
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
					std::string szText;

					if( lData )
						szText = save_l[lData].name;
					else
					{
						szText = getLocalised( "system_menu_editquest_newsavegame" );
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

	if( GInput->isKeyPressedNowUnPressed(iShortCut) )
	{
		return this;
	}

	return NULL;
}

//-----------------------------------------------------------------------------

void CMenuElementText::Render()
{
	if (WILL_RELOAD_ALL_TEXTURES) return;

	if(bNoMenu) return;

	Vec3f ePos;
	ePos.x = (float) rZone.left;
	ePos.y = (float) rZone.top;
	ePos.z = 1;

	if (bSelected)
		FontRenderText(pFont, ePos, lpszText, lColorHighlight);

	else
		FontRenderText(pFont, ePos, lpszText, lColor);

}

//-----------------------------------------------------------------------------

void CMenuElementText::RenderMouseOver()
{
	if(WILL_RELOAD_ALL_TEXTURES) return;

	if(bNoMenu) return;

	pMenuCursor->SetMouseOver();

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);

	Vec3f ePos;
	ePos.x = (float)rZone.left;
	ePos.y = (float)rZone.top;
	ePos.z = 1;

	FontRenderText(pFont, ePos, lpszText, lColorHighlight);

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);

	switch (iID)
	{
	case BUTTON_MENUEDITQUEST_LOAD:
		{
			CURRENT_GAME_INSTANCE=save_l[lData].num;
			ARX_GAMESAVE_MakePath();
			char tTxt[256];
			sprintf(tTxt,"%sGSAVE.BMP",GameSavePath);
			TextureContainer *pTextureTemp=TextureContainer::LoadUI(tTxt);

			if (pTextureTemp != pTextureLoad)
			{
				if (pTextureLoad)
					delete pTextureLoad;

				pTextureLoad=pTextureTemp;
			}

			pTextureLoadRender=pTextureLoad;
		}
		break;
	case BUTTON_MENUEDITQUEST_SAVEINFO:
		{
			CURRENT_GAME_INSTANCE=save_l[lData].num;
			ARX_GAMESAVE_MakePath();
			char tTxt[256];
			sprintf(tTxt,"%sGSAVE.BMP",GameSavePath);
			TextureContainer *pTextureTemp=TextureContainer::LoadUI(tTxt);

			if (pTextureTemp != pTextureLoad)
			{
				if (pTextureLoad)
					delete pTextureLoad;

				pTextureLoad=pTextureTemp;
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

Vec2i CMenuElementText::GetTextSize() const
{
	return pFont->GetTextSize(lpszText);
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
		delete pTexBackGround;
		pTexBackGround = NULL;
	}

	if (pTexBackGround1)
	{
		delete pTexBackGround1;
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
	fPos += _iDTime*( 1.0f / 700 );

	pZoneClick=NULL;

	CMenuZone * iR=pMenuAllZone->CheckZone(GInput->getMousePosAbs());

	if(GInput->getMouseButton(Mouse::Button_0)) {
		if(iR) {
			pZoneClick = (CMenuElement*)iR;
			pZoneClick->OnMouseClick(1);
			return pZoneClick->eMenuState;
		}
	} else {
		if(iR) {
			pZoneClick = (CMenuElement*)iR;
		}
	}

	return NOP;
}

//-----------------------------------------------------------------------------

void CMenuState::Render()
{
	if(WILL_RELOAD_ALL_TEXTURES) return;

	if(bNoMenu) return;

	if (pTexBackGround)
		EERIEDrawBitmap2(0, 0, ARX_CLEAN_WARN_CAST_FLOAT(DANAESIZX), ARX_CLEAN_WARN_CAST_FLOAT(DANAESIZY), 0.999f, pTexBackGround, Color::white);

	//------------------------------------------------------------------------

	int t=pMenuAllZone->GetNbZone();


	ARX_CHECK_INT(ARXDiffTimeMenu);
	int iARXDiffTimeMenu    = ARX_CLEAN_WARN_CAST_INT(ARXDiffTimeMenu);


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
	GRenderer->ResetTexture(0);
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

CMenuZone::CMenuZone(int _iX1,int _iY1,int _iX2,int _iY2, CMenuZone * _pRef)
{
	bActif=true;
	rZone.left=_iX1;
	rZone.top=_iY1;
	rZone.right=_iX2;
	rZone.bottom=_iY2;
	pRef=_pRef;

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
	rZone.left        += _iX;
	rZone.top        += _iY;
	rZone.right        += _iX;
	rZone.bottom    += _iY;
}

//-----------------------------------------------------------------------------

void CMenuZone::SetPos(float _fX,float _fY)
{
	int iWidth        = rZone.right - rZone.left;
	int iHeight        = rZone.bottom - rZone.top;


	ARX_CHECK_INT(_fX);
	ARX_CHECK_INT(_fY);
	int iX    = ARX_CLEAN_WARN_CAST_INT(_fX);
	int iY    = ARX_CLEAN_WARN_CAST_INT(_fY);


	rZone.left        = iX;
	rZone.top        = iY;
	rZone.right        = iX + abs(iWidth);
	rZone.bottom    = iY + abs(iHeight);
}

//-----------------------------------------------------------------------------

CMenuZone * CMenuZone::IsMouseOver(const Vec2s& mousePos) const
{
	int iYDouble=0;

	if(bTestYDouble)
	{
		iYDouble=(rZone.bottom-rZone.top)>>1;
	}

	if(    bActif && 
		(mousePos.x >= rZone.left) &&
		(mousePos.y >= (rZone.top-iYDouble)) &&
		(mousePos.x <= rZone.right) &&
		(mousePos.y <= (rZone.bottom+iYDouble)) )
		return pRef;

	return NULL;
}

//-----------------------------------------------------------------------------

CMenuAllZone::CMenuAllZone()
{
	vMenuZone.clear();

	vector<CMenuZone*>::iterator i;

	for(i=vMenuZone.begin();i!=vMenuZone.end();++i)
	{
		CMenuZone *zone=*i;
		delete zone;
	}
}

//-----------------------------------------------------------------------------

CMenuAllZone::~CMenuAllZone()
{
	for(std::vector<CMenuZone*>::iterator it = vMenuZone.begin(), it_end = vMenuZone.end(); it != it_end; ++it)
		delete *it;
}

//-----------------------------------------------------------------------------

void CMenuAllZone::AddZone(CMenuZone *_pMenuZone)
{
	vMenuZone.push_back(_pMenuZone);
}

//-----------------------------------------------------------------------------

CMenuZone * CMenuAllZone::CheckZone(const Vec2s& mousePos) const
{
	std::vector<CMenuZone*>::const_iterator i;

	for(i=vMenuZone.begin();i!=vMenuZone.end();++i)
	{
		CMenuZone *zone=*i;

		if(zone->bCheck && zone->bActif)
		{
			CMenuZone * pRef = ((*i)->IsMouseOver(mousePos));

            if (pRef)
                return pRef;
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------

CMenuZone * CMenuAllZone::GetZoneNum(int _iNum)
{
	vector<CMenuZone*>::iterator i;
	int iNum=0;

	for(i=vMenuZone.begin();i!=vMenuZone.end();++i)
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
	for (std::vector<CMenuZone*>::iterator i = vMenuZone.begin(), i_end = vMenuZone.end(); i != i_end; ++i)
		if (CMenuZone *zone = ((CMenuElement*)(*i))->GetZoneWithID(_iID))
			return zone;

	return NULL;
}

//-----------------------------------------------------------------------------

void CMenuAllZone::Move(int _iPosX,int _iPosY)
{
	for (std::vector<CMenuZone*>::iterator i = vMenuZone.begin(), i_end = vMenuZone.end(); i != i_end; ++i)
		(*i)->Move(_iPosX, _iPosY);
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

	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	GRenderer->ResetTexture(0);

	for(std::vector<CMenuZone*>::const_iterator i = vMenuZone.begin(), i_end = vMenuZone.end(); i != i_end; ++i)
	{
		CMenuZone *zone=*i;

		if(zone->bActif)
		{
			TexturedVertex v1[3],v2[3];
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
			
			EERIEDRAWPRIM(Renderer::TriangleStrip, v1);
			EERIEDRAWPRIM(Renderer::TriangleStrip, v2);
		}
	}

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}

//-----------------------------------------------------------------------------

CMenuCheckButton::CMenuCheckButton(int _iID, float _fPosX,float _fPosY,int _iTaille,TextureContainer *_pTex1,TextureContainer *_pTex2, CMenuElementText *_pText)
	:CMenuElement(NOP)
{
	iID = _iID;
	iState    = 0;
	iOldState = -1;


	ARX_CHECK_INT(_fPosX);
	ARX_CHECK_INT(_fPosY);
	iPosX    = ARX_CLEAN_WARN_CAST_INT(_fPosX);
	iPosY    = ARX_CLEAN_WARN_CAST_INT(_fPosY);


	iTaille = _iTaille;

	pText    = _pText;

	if (_pTex1)
	{
		float fRatioX = RATIO_X(_pTex1->m_dwWidth) ;
		float fRatioY = RATIO_Y(_pTex1->m_dwHeight);
		ARX_CHECK_INT(fRatioX);
		ARX_CHECK_INT(fRatioY);

		vTex.push_back(_pTex1);
		_iTaille = std::max(_iTaille, ARX_CLEAN_WARN_CAST_INT(fRatioX));
		_iTaille = std::max(_iTaille, ARX_CLEAN_WARN_CAST_INT(fRatioY));
	}

	if (_pTex2)
	{

		float fRatioX = RATIO_X(_pTex2->m_dwWidth) ;
		float fRatioY = RATIO_Y(_pTex2->m_dwHeight);
		ARX_CHECK_INT(fRatioX);
		ARX_CHECK_INT(fRatioY);

		vTex.push_back(_pTex2);
		_iTaille = std::max(_iTaille, ARX_CLEAN_WARN_CAST_INT(fRatioX));
		_iTaille = std::max(_iTaille, ARX_CLEAN_WARN_CAST_INT(fRatioY));
	}

	Vec2i textSize(0,0);

	if ( pText )
	{
		textSize = pText->pFont->GetTextSize(pText->lpszText); 

		_iTaille = std::max<int>(_iTaille, textSize.y);
		textSize.x += pText->rZone.left;
		pText->Move(iPosX, iPosY + (_iTaille - textSize.y) / 2);
	}



	ARX_CHECK_LONG( _fPosX );
	ARX_CHECK_LONG( _fPosY );
	ARX_CHECK_LONG( _fPosX + _iTaille + textSize.x );
	ARX_CHECK_LONG( _fPosY + std::max<int>(_iTaille, textSize.y) );
	//CAST
	rZone.left = ARX_CLEAN_WARN_CAST_LONG( _fPosX );
	rZone.top = ARX_CLEAN_WARN_CAST_LONG( _fPosY );
	rZone.right = ARX_CLEAN_WARN_CAST_LONG( _fPosX + _iTaille + textSize.x );
	rZone.bottom = ARX_CLEAN_WARN_CAST_LONG( _fPosY + std::max<int>(_iTaille, textSize.y) );
	pRef=this;

	if (_pTex2) // TODO should this be _pTex1?
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

void CMenuCheckButton::Move(int _iX, int _iY)
{
	CMenuElement::Move(_iX, _iY);

	if (pText)
		pText->Move(_iX, _iY);

	ComputeTexturesPosition();
}

//-----------------------------------------------------------------------------

bool CMenuCheckButton::OnMouseClick(int _iMouseButton) {
	
	(void)_iMouseButton;
	
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
	case BUTTON_MENUOPTIONSVIDEO_CROSSHAIR:
		{
			config.video.showCrosshair=(iState)?true:false;
		}
		break;
	case BUTTON_MENUOPTIONSVIDEO_ANTIALIASING:
		{
			config.video.antialiasing=(iState)?true:false;

			ARX_SetAntiAliasing();
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
			config.input.linkMouseLookToUse=(iState)?true:false;
		}
		break;
	case BUTTON_MENUOPTIONSVIDEO_BACK:
	{
		if(    (pMenuSliderResol)&&
			(pMenuSliderResol->iOldPos>=0) )
		{
			pMenuSliderResol->iPos=pMenuSliderResol->iOldPos;
			pMenuSliderResol->iOldPos=-1;
			newWidth=config.video.width;
			newHeight=config.video.height;
		}
		
		if(    (pMenuSliderBpp)&&
			(pMenuSliderBpp->iOldPos>=0) )
		{
			pMenuSliderBpp->iPos=pMenuSliderBpp->iOldPos;
			pMenuSliderBpp->iOldPos=-1;
			newBpp=config.video.bpp;
		}
		
		if(    (pMenuSliderTexture)&&
			(pMenuSliderTexture->iOldPos>=0) )
		{
			pMenuSliderTexture->iPos=pMenuSliderTexture->iOldPos;
			pMenuSliderTexture->iOldPos=-1;
			newTextureSize=config.video.textureSize;
		}
		break;
	}

	}

	return false;
}

//-----------------------------------------------------------------------------

void CMenuCheckButton::Update(int /*_iDTime*/)
{
}

//-----------------------------------------------------------------------------

void CMenuCheckButton::Render()
{
	if(WILL_RELOAD_ALL_TEXTURES) return;

	if(bNoMenu) return;

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);

	if (!vTex.empty())
	{
		TextureContainer *pTex = vTex[iState];

		TexturedVertex v[4];
		Color color = (bCheck) ? Color::white : Color::fromBGRA(0xFF3F3F3F);

		v[0].sz=v[1].sz=v[2].sz=v[3].sz=0.f;
		v[0].rhw=v[1].rhw=v[2].rhw=v[3].rhw=0.999999f;
		
		float iY = 0;

		{
			iY = ARX_CLEAN_WARN_CAST_FLOAT(rZone.bottom - rZone.top);
			iY -= iTaille;
			iY = rZone.top + iY*0.5f;
		}
		
		//carre
		EERIEDrawBitmap2(static_cast<float>(rZone.right - iTaille), iY, RATIO_X(iTaille), RATIO_Y(iTaille), 0.f, pTex, color);
	}

	if (pText)
		pText->Render();

	//DEBUG
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}

//-----------------------------------------------------------------------------

void CMenuCheckButton::RenderMouseOver()
{
	if(WILL_RELOAD_ALL_TEXTURES) return;

	if(bNoMenu) return;

	pMenuCursor->SetMouseOver();

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);

	TextureContainer *pTex = vTex[iState];

	if(pTex) GRenderer->SetTexture(0, pTex);
	else GRenderer->ResetTexture(0);

	TexturedVertex v[4];
	v[0].color = v[1].color = v[2].color = v[3].color = Color::white.toBGR();
	v[0].sz=v[1].sz=v[2].sz=v[3].sz=0.f;    
	v[0].rhw=v[1].rhw=v[2].rhw=v[3].rhw=0.999999f;

	float iY = 0;
	iY = ARX_CLEAN_WARN_CAST_FLOAT(rZone.bottom - rZone.top);
	iY -= iTaille;
	iY = rZone.top + iY*0.5f;

	//carre

	EERIEDrawBitmap2(static_cast<float>(rZone.right - iTaille), iY, RATIO_X(iTaille), RATIO_Y(iTaille), 0.f, pTex, Color::white); 


	//tick
	if (pText)
		pText->RenderMouseOver();

	//DEBUG
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}

//-----------------------------------------------------------------------------

// Nuky - merges common code of Render() and RenderMouseOver()
/// Compute members fTexX, fTexY, fTexSX and fTexSY according to rZone and iTaille
void CMenuCheckButton::ComputeTexturesPosition()
{
	// Nuky - for now I split into 2 cases, because I don't know yet how to fix position with text
	// TODO Merge with master
	/*if (!pText)
	{
		fTexX_ = ARX_CLEAN_WARN_CAST_FLOAT(rZone.left);
		fTexY_ = ARX_CLEAN_WARN_CAST_FLOAT(rZone.top);
		fTexSX_ = RATIO_X(iTaille);
		fTexSY_ = RATIO_Y(iTaille);
	}
	else
	{
		fTexX_ = ARX_CLEAN_WARN_CAST_FLOAT(rZone.right - iTaille);
		fTexY_ = rZone.top + (ARX_CLEAN_WARN_CAST_FLOAT(rZone.bottom - rZone.top) - iTaille) * 0.5f;
		fTexSX_ = RATIO_X(iTaille);
		fTexSY_ = RATIO_Y(iTaille);
	}*/
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

	pTexButton=TextureContainer::LoadUI("Graph\\interface\\menus\\menu_left_1button.bmp");
	pTexButton2=TextureContainer::LoadUI("Graph\\interface\\menus\\menu_left_2button.bmp");
	pTexButton3=TextureContainer::LoadUI("Graph\\interface\\menus\\menu_left_3button.bmp");

	pTexMain=TextureContainer::LoadUI("Graph\\interface\\menus\\menu_left_main.bmp");

	pTexGlissiere=TextureContainer::LoadUI("Graph\\interface\\menus\\menu_left_main_glissiere.bmp");
	pTexGlissiereButton=TextureContainer::LoadUI("Graph\\interface\\menus\\menu_left_main_glissiere_button.bmp");

	vWindowConsoleElement.clear();

	fPosXCalc=((float)-iTailleX);
	fDist=((float)(iTailleX+iPosX));
	fAngle=0.f;

	eCurrentMenuState=NOP;


	float fCalc	= fPosXCalc + (fDist * sin(radians(fAngle)));
	ARX_CHECK_INT(fCalc);

	iPosX    = ARX_CLEAN_WARN_CAST_INT(fCalc);


	bChangeConsole=false;
}

//-----------------------------------------------------------------------------

CWindowMenu::~CWindowMenu()
{
	for (std::vector<CWindowMenuConsole*>::iterator it = vWindowConsoleElement.begin(), it_end = vWindowConsoleElement.end(); it < it_end; ++it)
		delete *it;
}

//-----------------------------------------------------------------------------

void CWindowMenu::AddConsole(CWindowMenuConsole *_pMenuConsoleElement)
{
	vWindowConsoleElement.push_back(_pMenuConsoleElement);
	_pMenuConsoleElement->iOldPosX = 0;
	_pMenuConsoleElement->iOldPosY = 0;
	_pMenuConsoleElement->iPosX = iPosX;
	_pMenuConsoleElement->iPosY = iPosY;
}

//-----------------------------------------------------------------------------

void CWindowMenu::Update(int _iDTime)
{


	float fCalc	= fPosXCalc + (fDist * sin(radians(fAngle)));
	ARX_CHECK_INT(fCalc);

	iPosX    = ARX_CLEAN_WARN_CAST_INT(fCalc);


	fAngle += _iDTime * 0.08f;

	if (fAngle>90.f) fAngle=90.f;
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

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);

	TexturedVertex v[4];
	v[0].color = v[1].color = v[2].color = v[3].color = Color::white.toBGR();
	v[0].sz=v[1].sz=v[2].sz=v[3].sz=0.f;    
	v[0].rhw=v[1].rhw=v[2].rhw=v[3].rhw=0.999999f;

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);

	MENUSTATE eMS=NOP;

	if (bMouseListen)
	{
		vector<CWindowMenuConsole*>::iterator i;

		for (i = vWindowConsoleElement.begin(); i != vWindowConsoleElement.end(); ++i)
		{
			if(eCurrentMenuState==(*i)->eMenuState)
			{
				eMS=(*i)->Update(iPosX, iPosY, 0);
				
				if (eMS != NOP)
					break;
			}
		}
	}

	for (std::vector<CWindowMenuConsole*>::iterator i = vWindowConsoleElement.begin(); i != vWindowConsoleElement.end(); ++i)
	{
		if(eCurrentMenuState==(*i)->eMenuState)
		{
			if ((*i)->Render())
				GRenderer->SetRenderState(Renderer::AlphaBlending, false);

			break;
		}
	}

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);

	if (eMS != NOP)
	{
		eCurrentMenuState=eMS;
		bChangeConsole=true;
	}

	return eMS;
}

CWindowMenuConsole::CWindowMenuConsole(int _iPosX,int _iPosY,int _iWidth,int _iHeight,MENUSTATE _eMenuState) :
	bMouseListen (true),
	iInterligne (10),
	bEdit (false),
	lData(0),
	pData(NULL)
{
	iOX=(int)RATIO_X(_iPosX);
	iOY=(int)RATIO_Y(_iPosY);
	iWidth=(int)RATIO_X(_iWidth);
	iHeight=(int)RATIO_Y(_iHeight);

	eMenuState=_eMenuState;

	pTexBackground = TextureContainer::LoadUI("Graph\\interface\\menus\\menu_console_background.bmp");
	pTexBackgroundBorder = TextureContainer::LoadUI("Graph\\interface\\menus\\menu_console_background_border.bmp");

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
	_pMenuElement->ePlace    =    CENTERY;
	int iDy                    =    _pMenuElement->rZone.bottom-_pMenuElement->rZone.top;

	int iI                    =    MenuAllZone.GetNbZone();

	for( int iJ = 0 ; iJ < iI ; iJ++ )
	{
		iDy +=    iInterligne;
		CMenuZone    *pZone    =    MenuAllZone.GetZoneNum(iJ);
		iDy    +=    pZone->rZone.bottom - pZone->rZone.top;
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
		dy    =    iDepY - MenuAllZone.GetZoneNum(0)->rZone.top;
	}

	//We can't go inside the for-loop
	else
	{
		ARX_CHECK( !( 0 < iI ) );
	}

	for( int iJ = 0 ; iJ < iI ; iJ++ )
	{
		CMenuZone *pZone    =    MenuAllZone.GetZoneNum(iJ);
		iDy                    =    pZone->rZone.bottom - pZone->rZone.top;
		iDepY                +=    iDy + iInterligne;
		pZone->Move( 0, dy );
	}

	_pMenuElement->Move( 0, iDepY );

	MenuAllZone.AddZone( (CMenuZone*) _pMenuElement );
}

//-----------------------------------------------------------------------------

void CWindowMenuConsole::AddMenuCenter( CMenuElement * _pMenuElement )
{
	_pMenuElement->ePlace    =    CENTER;

	int    iDx    =    _pMenuElement->rZone.right - _pMenuElement->rZone.left;
	int    dx    =    ( ( iWidth - iDx ) >> 1 ) - _pMenuElement->rZone.left;

	if( dx < 0 )
	{
		dx = 0;
	}

	int    iDy    =    _pMenuElement->rZone.bottom - _pMenuElement->rZone.top;
	int    iI    =    MenuAllZone.GetNbZone();

	for( int iJ = 0 ; iJ < iI ; iJ++ )
	{
		iDy    +=    iInterligne;
		CMenuZone *pZone    =    MenuAllZone.GetZoneNum(iJ);
		iDy    +=    pZone->rZone.bottom - pZone->rZone.top;
	}

	int iDepY;

	if( iDy < iHeight )
	{
		iDepY    =    iOY + ( ( iHeight - iDy ) >> 1 );
	}
	else
	{
		iDepY    =    iOY;
	}

	int dy = 0;
	iI = MenuAllZone.GetNbZone();

	if( iI )
	{
		dy    =    iDepY - MenuAllZone.GetZoneNum(0)->rZone.top;
	}

	//We can't go inside the for-loop
	else
	{
		ARX_CHECK( !( 0 < iI ) );
	}

	for( int iJ = 0 ; iJ < iI ; iJ++ )
	{
		CMenuZone *pZone = MenuAllZone.GetZoneNum( iJ );
		iDepY += pZone->rZone.bottom - pZone->rZone.top + iInterligne;
		pZone->Move( 0, dy );
	}

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

void CWindowMenuConsole::UpdateText()
{
	if(GInput->isAnyKeyPressed())
	{
		if( (GInput->isKeyPressed(Keyboard::Key_Enter)) ||
			(GInput->isKeyPressed(Keyboard::Key_NumPadEnter)) ||
			(GInput->isKeyPressed(Keyboard::Key_Escape)) )
		{
			ARX_SOUND_PlayMenu(SND_MENU_CLICK);
			((CMenuElementText*)pZoneClick)->eState=EDIT;

			if( ((CMenuElementText*)pZoneClick)->lpszText.empty() )
			{
				std::string szMenuText;
				szMenuText = getLocalised("system_menu_editquest_newsavegame");

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
		std::string tText;
		
		CMenuElementText *pZoneText=(CMenuElementText*)pZoneClick;

		if(GInput->isKeyPressedNowPressed(Keyboard::Key_Backspace))
		{
			tText = pZoneText->lpszText;

			if( !tText.empty() )
			{
				tText.resize(tText.size() - 1);
				bKey=true;
			}
		}
		else
		{
			int iKey = GInput->getKeyPressed();
			iKey&=0xFFFF;

			if(GInput->isKeyPressedNowPressed(iKey))
			{
				tText = pZoneText->lpszText;

				char tCat;
				
				bKey = GInput->getKeyAsText(iKey, tCat);

				if(bKey)
				{
					int iChar = tCat & 0x000000FF; // To prevent ascii chars between [128, 255] from causing an assertion in the functions below...
					if ((isalnum(iChar) || isspace(iChar) || ispunct(iChar)) && (tCat != '\t') && (tCat != '*'))
						tText += tCat;
				}
			}
		}

		if(bKey)
		{
			pZoneText->SetText(tText);

			if(    (pZoneText->rZone.right-pZoneText->rZone.left)>(iWidth-RATIO_X(64)) )
			{
				if(!tText.empty()) {
					tText.resize(tText.size() - 1);
					pZoneText->SetText(tText);
				}
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
		Vec2i textSize = ((CMenuElementText*)pZoneClick)->pFont->GetTextSize("|");
		pZoneClick->rZone.bottom += textSize.y;
	}

	//DRAW CURSOR
	TexturedVertex v[4];
	GRenderer->ResetTexture(0);
	float col=.5f+rnd()*.5f;
	v[0].color = v[1].color = v[2].color = v[3].color = Color::gray(col).toBGR();
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
	EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);
}

//-----------------------------------------------------------------------------

CMenuElement * CWindowMenuConsole::GetTouch(bool keyTouched, int keyId, InputKeyId* pInputKeyId, bool _bValidateTest)
{
	int iMouseButton = keyTouched ? 0 : GInput->getMouseButtonClicked();
	
	if(pInputKeyId)
		*pInputKeyId = keyId;

	if(keyTouched || (iMouseButton & (Mouse::ButtonBase | Mouse::WheelBase))) {
		if(!keyTouched && !bMouseAttack)
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

				if(  (iMouseButton & Mouse::ButtonBase)&&
					!(iMouseButton & Mouse::WheelBase) )
				{
					bOk=false;
				}
				else
				{
					for(int buttonId = Mouse::ButtonBase; buttonId < Mouse::ButtonMax; buttonId++)
					{
						if(keyId == buttonId)
						{
							bOk=false;
							break;
						}
					}
				}

				if(bOk) return NULL;
			}
		}

		std::string pText;
		if(iMouseButton & (Mouse::ButtonBase | Mouse::WheelBase))
			pText = GInput->getKeyName(iMouseButton, true); 
		else
			pText = GInput->getKeyName(keyId, true);

		if ( !pText.empty() )
		{
			pZoneText->lColorHighlight=pZoneText->lOldColor;

			pZoneText->eState=GETTOUCH;
			pZoneText->SetText(pText);
			
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

			if(iMouseButton & (Mouse::ButtonBase | Mouse::WheelBase))
			{
				if(pInputKeyId)
					*pInputKeyId = iMouseButton;
			}

			bMouseAttack=false;

			return (CMenuElement*)pZoneText;
		}

	}

	return NULL;
}

//-----------------------------------------------------------------------------

MENUSTATE CWindowMenuConsole::Update(int _iPosX,int _iPosY,int _iOffsetY)
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

		if(    (pZone->rZone.top<iSavePosY)||
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
			CMenuZone * iR = MenuAllZone.CheckZone(GInput->getMousePosAbs());

			if(iR) {
				pZoneClick=(CMenuElement*)iR;

				if( GInput->getMouseButtonDoubleClick(Mouse::Button_0,300) )
				{
					MENUSTATE e = pZoneClick->eMenuState;
					bEdit = pZoneClick->OnMouseDoubleClick(0);

					if (pZoneClick->iID == BUTTON_MENUEDITQUEST_LOAD)
						return MAIN;

					if(bEdit)
						return pZoneClick->eMenuState;

					return e;
				}

				if( GInput->getMouseButton(Mouse::Button_0) )
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
				CMenuZone * iR = MenuAllZone.CheckZone(GInput->getMousePosAbs());

				if(iR) {
					pZoneClick=(CMenuElement*)iR;

					if( GInput->getMouseButtonDoubleClick(Mouse::Button_0,300) )
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
			CMenuElement *CMenuElementShortCut = pMenuElement->OnShortCut();

			if(CMenuElementShortCut)
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

static bool UpdateGameKey(bool bEdit, CMenuElement *pmeElement, InputKeyId inputKeyId)
{
	bool bChange=false;

	if(    (!bEdit)&&
		(pmeElement) )
	{
		switch(pmeElement->iID)
		{
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP2:
			bChange=config.setActionKey(CONTROLS_CUST_JUMP,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_MAGICMODE1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_MAGICMODE2:
			bChange=config.setActionKey(CONTROLS_CUST_MAGICMODE,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_MAGICMODE1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STEALTHMODE1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STEALTHMODE2:
			bChange=config.setActionKey(CONTROLS_CUST_STEALTHMODE,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_STEALTHMODE1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKFORWARD1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKFORWARD2:
			bChange=config.setActionKey(CONTROLS_CUST_WALKFORWARD,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKFORWARD1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKBACKWARD1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKBACKWARD2:
			bChange=config.setActionKey(CONTROLS_CUST_WALKBACKWARD,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_WALKBACKWARD1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFELEFT1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFELEFT2:
			bChange=config.setActionKey(CONTROLS_CUST_STRAFELEFT,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFELEFT1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFERIGHT1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFERIGHT2:
			bChange=config.setActionKey(CONTROLS_CUST_STRAFERIGHT,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFERIGHT1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANLEFT1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANLEFT2:
			bChange=config.setActionKey(CONTROLS_CUST_LEANLEFT,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANLEFT1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANRIGHT1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANRIGHT2:
			bChange=config.setActionKey(CONTROLS_CUST_LEANRIGHT,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_LEANRIGHT1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCH1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCH2:
			bChange=config.setActionKey(CONTROLS_CUST_CROUCH,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCH1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_MOUSELOOK1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_MOUSELOOK2:
			bChange=config.setActionKey(CONTROLS_CUST_MOUSELOOK,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_MOUSELOOK1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_ACTIONCOMBINE1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_ACTIONCOMBINE2:
			bChange=config.setActionKey(CONTROLS_CUST_ACTION,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_ACTIONCOMBINE1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_INVENTORY1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_INVENTORY2:
			bChange=config.setActionKey(CONTROLS_CUST_INVENTORY,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_INVENTORY1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOK1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOK2:
			bChange=config.setActionKey(CONTROLS_CUST_BOOK,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOK1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKCHARSHEET1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKCHARSHEET2:
			bChange=config.setActionKey(CONTROLS_CUST_BOOKCHARSHEET,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKCHARSHEET1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKSPELL1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKSPELL2:
			bChange=config.setActionKey(CONTROLS_CUST_BOOKSPELL,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKSPELL1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKMAP1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKMAP2:
			bChange=config.setActionKey(CONTROLS_CUST_BOOKMAP,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKMAP1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKQUEST1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKQUEST2:
			bChange=config.setActionKey(CONTROLS_CUST_BOOKQUEST,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_BOOKQUEST1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONLIFE1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONLIFE2:
			bChange=config.setActionKey(CONTROLS_CUST_DRINKPOTIONLIFE,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONLIFE1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONMANA1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONMANA2:
			bChange=config.setActionKey(CONTROLS_CUST_DRINKPOTIONMANA,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_DRINKPOTIONMANA1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_TORCH1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_TORCH2:
			bChange=config.setActionKey(CONTROLS_CUST_TORCH,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_TORCH1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CANCELCURSPELL1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CANCELCURSPELL2:    
			bChange=config.setActionKey(CONTROLS_CUST_CANCELCURSPELL,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_CANCELCURSPELL1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST1_2:
			bChange=config.setActionKey(CONTROLS_CUST_PRECAST1,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST2:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST2_2:
			bChange=config.setActionKey(CONTROLS_CUST_PRECAST2,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST2,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST3:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST3_2:
			bChange=config.setActionKey(CONTROLS_CUST_PRECAST3,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_PRECAST3,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_WEAPON1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_WEAPON2:
			bChange=config.setActionKey(CONTROLS_CUST_WEAPON,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_WEAPON1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKLOAD:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKLOAD2:
			bChange=config.setActionKey(CONTROLS_CUST_QUICKLOAD,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKLOAD,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKSAVE:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKSAVE2:
			bChange=config.setActionKey(CONTROLS_CUST_QUICKSAVE,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_QUICKSAVE,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNLEFT1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNLEFT2:
			bChange=config.setActionKey(CONTROLS_CUST_TURNLEFT,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNLEFT1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNRIGHT1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNRIGHT2:
			bChange=config.setActionKey(CONTROLS_CUST_TURNRIGHT,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_TURNRIGHT1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKUP1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKUP2:
			bChange=config.setActionKey(CONTROLS_CUST_LOOKUP,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKUP1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKDOWN1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKDOWN2:
			bChange=config.setActionKey(CONTROLS_CUST_LOOKDOWN,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_LOOKDOWN1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFE1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFE2:
			bChange=config.setActionKey(CONTROLS_CUST_STRAFE,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_STRAFE1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CENTERVIEW1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CENTERVIEW2:
			bChange=config.setActionKey(CONTROLS_CUST_CENTERVIEW,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_CENTERVIEW1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_FREELOOK1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_FREELOOK2:
			bChange=config.setActionKey(CONTROLS_CUST_FREELOOK,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_FREELOOK1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PREVIOUS1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_PREVIOUS2:
			bChange=config.setActionKey(CONTROLS_CUST_PREVIOUS,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_PREVIOUS1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_NEXT1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_NEXT2:    
			bChange=config.setActionKey(CONTROLS_CUST_NEXT,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_NEXT1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCHTOGGLE1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCHTOGGLE2:    
			bChange=config.setActionKey(CONTROLS_CUST_CROUCHTOGGLE,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_CROUCHTOGGLE1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_UNEQUIPWEAPON1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_UNEQUIPWEAPON2:    
			bChange=config.setActionKey(CONTROLS_CUST_UNEQUIPWEAPON,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_UNEQUIPWEAPON1,inputKeyId);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_MINIMAP1:
		case BUTTON_MENUOPTIONS_CONTROLS_CUST_MINIMAP2:    
			bChange=config.setActionKey(CONTROLS_CUST_MINIMAP,pmeElement->iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_MINIMAP1,inputKeyId);
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

	//------------------------------------------------------------------------
	// Console display
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
	GRenderer->SetRenderState(Renderer::DepthTest, false);

	EERIEDrawBitmap2(static_cast<float>(iPosX), static_cast<float>(iSavePosY),
	                 RATIO_X(pTexBackground->m_dwWidth), RATIO_Y(pTexBackground->m_dwHeight),
	                 0, pTexBackground, Color::white);

	GRenderer->SetRenderState(Renderer::DepthTest, true);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	EERIEDrawBitmap2(static_cast<float>(iPosX), static_cast<float>(iSavePosY),
	                 RATIO_X(pTexBackgroundBorder->m_dwWidth), RATIO_Y(pTexBackgroundBorder->m_dwHeight),
	                 0, pTexBackgroundBorder, Color::white);

	//------------------------------------------------------------------------

	int t = MenuAllZone.GetNbZone();


	ARX_CHECK_INT(ARXDiffTimeMenu);
	int iARXDiffTimeMenu    = ARX_CLEAN_WARN_CAST_INT(ARXDiffTimeMenu);


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
					((CMenuElementText*)pZoneClick)->lColorHighlight = Color(255, 0, 0);
				else
					((CMenuElementText*)pZoneClick)->lColorHighlight = Color(50, 0, 0);

				bool keyTouched = GInput->isAnyKeyPressed();
				int keyId = GInput->getKeyPressed();
				
				if( GInput->isKeyPressed(Keyboard::Key_LeftShift)||
					GInput->isKeyPressed(Keyboard::Key_RightShift)||
					GInput->isKeyPressed(Keyboard::Key_LeftCtrl)||
					GInput->isKeyPressed(Keyboard::Key_RightCtrl)||
					GInput->isKeyPressed(Keyboard::Key_LeftAlt)||
					GInput->isKeyPressed(Keyboard::Key_RightAlt) )
				{
					if(!((keyId & INPUT_COMBINATION_MASK )>>16))
						keyTouched = false;
				}
				else
				{
					if(GInput->isKeyPressedNowUnPressed(Keyboard::Key_LeftShift))
					{
						keyTouched = true;
						keyId = Keyboard::Key_LeftShift;
					}

					if(GInput->isKeyPressedNowUnPressed(Keyboard::Key_RightShift))
					{
						keyTouched = true;
						keyId = Keyboard::Key_RightShift;
					}

					if(GInput->isKeyPressedNowUnPressed(Keyboard::Key_LeftCtrl))
					{
						keyTouched = true;
						keyId = Keyboard::Key_LeftCtrl;
					}

					if(GInput->isKeyPressedNowUnPressed(Keyboard::Key_RightCtrl))
					{
						keyTouched = true;
						keyId = Keyboard::Key_RightCtrl;
					}

					if(GInput->isKeyPressedNowUnPressed(Keyboard::Key_LeftAlt))
					{
						keyTouched = true;
						keyId = Keyboard::Key_LeftAlt;
					}

					if(GInput->isKeyPressedNowUnPressed(Keyboard::Key_RightAlt))
					{
						keyTouched = true;
						keyId = Keyboard::Key_RightAlt;
					}
				}

				InputKeyId inputKeyId;
				CMenuElement *pmeElement = GetTouch(keyTouched, keyId, &inputKeyId, true);

				if(pmeElement)
				{
					if(UpdateGameKey(bEdit,pmeElement, inputKeyId))
					{
						bReInit=true;
					}
				}
			}
			break;
		default:
			{
				if(GInput->getMouseButtonNowPressed(Mouse::Button_0))
				{
					CMenuZone *pmzMenuZone = MenuAllZone.GetZoneWithID(BUTTON_MENUOPTIONS_CONTROLS_CUST_DEFAULT);

					if(pmzMenuZone==pZoneClick)
					{
						config.setDefaultActionKeys();
						bReInit=true;
					}
				}
			}
			break;
		}

		if(bReInit)
		{
			ReInitActionKey();
			bMouseAttack=false;
		}
	}

	//DEBUG ZONE
	MenuAllZone.DrawZone();

	return iSlider;
}

void CWindowMenuConsole::ReInitActionKey()
{
	int iID=BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP1;
	int iI=NUM_ACTION_KEY;
	
	while(iI--)
	{
		int iTab=(iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP1)>>1;

		CMenuZone *pmzMenuZone = MenuAllZone.GetZoneWithID(iID);

		if (pmzMenuZone)
		{
			if(pmzMenuZone)
			{
				pZoneClick = (CMenuElement*)pmzMenuZone;
				GetTouch(true, config.actions[iTab].key[0]);
			}

			pmzMenuZone = MenuAllZone.GetZoneWithID(iID+1);

			if( pmzMenuZone )
			{
				pZoneClick = (CMenuElement*)pmzMenuZone;
				GetTouch(true, config.actions[iTab].key[1]);
			}
		}

		iID+=2;
	}
}

//-----------------------------------------------------------------------------

CMenuPanel::CMenuPanel()
: CMenuElement(NOP)
{
	vElement.clear();

	pRef = this;
}

//-----------------------------------------------------------------------------

CMenuPanel::~CMenuPanel()
{
	for (std::vector<CMenuElement*>::iterator it = vElement.begin(), it_end = vElement.end(); it != it_end; ++it)
		delete (*it);
}

//-----------------------------------------------------------------------------

void CMenuPanel::Move(int _iX, int _iY)
{
	rZone.left += _iX;
	rZone.top += _iY;
	rZone.right += _iX;
	rZone.bottom += _iY;

	for (std::vector<CMenuElement*>::iterator it = vElement.begin(), it_end = vElement.end(); it != it_end; ++it)
		(*it)->Move(_iX, _iY);
}

//-----------------------------------------------------------------------------
// patch on ajoute � droite en ligne
void CMenuPanel::AddElement(CMenuElement* _pElem)
{
	vElement.push_back(_pElem);

	if (vElement.size() == 1)
		rZone = _pElem->rZone;
	else
	{
		rZone.left = std::min(rZone.left, _pElem->rZone.left);
		rZone.top = std::min(rZone.top, _pElem->rZone.top);
	}

	// + taille elem
	rZone.right = std::max(rZone.right, _pElem->rZone.right);
	rZone.bottom = std::max(rZone.bottom, _pElem->rZone.bottom);

	_pElem->Move(0, ((GetHeight() - _pElem->rZone.bottom) / 2));
}

//-----------------------------------------------------------------------------
// patch on ajoute � droite en ligne
void CMenuPanel::AddElementNoCenterIn(CMenuElement* _pElem)
{
	vElement.push_back(_pElem);

	if (vElement.size() == 1)
		rZone=_pElem->rZone;
	else
	{
		rZone.left = std::min(rZone.left, _pElem->rZone.left);
		rZone.top = std::min(rZone.top, _pElem->rZone.top);
	}

	// + taille elem
	rZone.right = std::max(rZone.right, _pElem->rZone.right);
	rZone.bottom = std::max(rZone.bottom, _pElem->rZone.bottom);

}

//-----------------------------------------------------------------------------

CMenuElement* CMenuPanel::OnShortCut()
{
	for (std::vector<CMenuElement*>::iterator it = vElement.begin(), it_end = vElement.end(); it != it_end; ++it)
		if ((*it)->OnShortCut())
			return *it;

	return NULL;
}

//-----------------------------------------------------------------------------

void CMenuPanel::Update(int _iTime)
{
	rZone.right = rZone.left;
	rZone.bottom = rZone.top;

	;

	for (std::vector<CMenuElement*>::iterator it = vElement.begin(), it_end = vElement.end(); it != it_end; ++it)
	{
		(*it)->Update(_iTime);
		rZone.right = std::max(rZone.right, (*it)->rZone.right);
		rZone.bottom = std::max(rZone.bottom, (*it)->rZone.bottom);
	}
}

//-----------------------------------------------------------------------------

void CMenuPanel::Render()
{
	if(WILL_RELOAD_ALL_TEXTURES) return;

	if(bNoMenu) return;

	for (std::vector<CMenuElement*>::iterator it = vElement.begin(), it_end = vElement.end(); it != it_end; ++it)
		(*it)->Render();
}

//-----------------------------------------------------------------------------

CMenuZone * CMenuPanel::GetZoneWithID(int _iID)
{
	for (std::vector<CMenuElement*>::iterator it = vElement.begin(), it_end = vElement.end(); it != it_end; ++it)
		if (CMenuZone* pZone = (*it)->GetZoneWithID(_iID))
			return pZone;

	return NULL;
}

//-----------------------------------------------------------------------------

CMenuZone * CMenuPanel::IsMouseOver(const Vec2s& mousePos) const
{
	if ((mousePos.x >= rZone.left) &&
		(mousePos.y >= rZone.top) &&
		(mousePos.x <= rZone.right) &&
		(mousePos.y <= rZone.bottom))
	{
		vector<CMenuElement *>::const_iterator i;
		
		for(i=vElement.begin();i!=vElement.end();++i)
		{
			if ((*i)->bCheck &&
				(*i)->bActif && 
				(mousePos.x >= (*i)->rZone.left) &&
				(mousePos.y >= (*i)->rZone.top) &&
				(mousePos.x <= (*i)->rZone.right) &&
				(mousePos.y <= (*i)->rZone.bottom))
				return (*i)->pRef;
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------

CMenuButton::CMenuButton(int _iID, Font* _pFont,MENUSTATE _eMenuState,int _iPosX,int _iPosY, const std::string& _pText,float _fSize,TextureContainer *_pTex,TextureContainer *_pTexOver,int _iColor)
	: CMenuElement(_eMenuState)
{
	iID = _iID;
	pFont = _pFont;
	fSize=_fSize;

	rZone.left=_iPosX;
	rZone.top=_iPosY;
	rZone.right  = rZone.left ;
	rZone.bottom = rZone.top ;

	vText.clear();
	iPos=0;

	if( !_pText.empty() )
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

		rZone.right  = max((long)rZone.right,  ARX_CLEAN_WARN_CAST_LONG(rZoneR) );
		rZone.bottom = max((long)rZone.bottom, ARX_CLEAN_WARN_CAST_LONG(rZoneB) );
	}

	if (pTexOver)
	{
		float rZoneR = rZone.left + RATIO_X(pTexOver->m_dwWidth);
		float rZoneB = rZone.top + RATIO_Y(pTexOver->m_dwHeight);

		ARX_CHECK_LONG( rZoneR );
		ARX_CHECK_LONG( rZoneB );

		rZone.right  = max((long)rZone.right, ARX_CLEAN_WARN_CAST_LONG(rZoneR) );
		rZone.bottom = max((long)rZone.bottom, ARX_CLEAN_WARN_CAST_LONG(rZoneB) );
	}



	iColor=_iColor;

	pRef=this;
}

//-----------------------------------------------------------------------------

CMenuButton::~CMenuButton() {
}

//-----------------------------------------------------------------------------

void CMenuButton::SetPos(float _iX,float _iY)
{
	CMenuZone::SetPos(_iX, _iY);

	int iWidth = 0;
	int iHeight = 0;

	if (pTex)
	{
		iWidth = pTex->m_dwWidth;
		iHeight = pTex->m_dwHeight;

		float fRatioX = RATIO_X(iWidth);
		float fRatioY = RATIO_Y(iHeight);

		ARX_CHECK_INT(fRatioX);
		ARX_CHECK_INT(fRatioY);

		iWidth = ARX_CLEAN_WARN_CAST_INT(fRatioX);
		iHeight = ARX_CLEAN_WARN_CAST_INT(fRatioY);
	}

	int iWidth2 = 0;
	int iHeight2 = 0;

	if (pTexOver)
	{
		iWidth2 = pTexOver->m_dwWidth;
		iHeight2 = pTexOver->m_dwHeight;

		float fRatioX = RATIO_X(iWidth2);
		float fRatioY = RATIO_Y(iHeight2);

		ARX_CHECK_INT(fRatioX);
		ARX_CHECK_INT(fRatioY);

		iWidth2 = ARX_CLEAN_WARN_CAST_INT(fRatioX);
		iHeight2 = ARX_CLEAN_WARN_CAST_INT(fRatioY);
	}

	rZone.right = static_cast<int>(_iX) + max(iWidth,iWidth2);
	rZone.bottom = static_cast<int>(_iY) + max(iHeight,iHeight2);
}

//-----------------------------------------------------------------------------

void CMenuButton::AddText( const std::string& _pText)
{
	if ( _pText.empty() )
		return;

	vText += _pText;

	int iSizeXButton=rZone.right-rZone.left;
	int iSizeYButton=rZone.bottom-rZone.top;
	
	Vec2i textSize = pFont->GetTextSize(_pText);

	if(textSize.x>iSizeXButton) iSizeXButton=textSize.x;
	if(textSize.y>iSizeYButton) iSizeYButton=textSize.y;

	rZone.right=rZone.left+iSizeXButton;
	rZone.bottom=rZone.top+iSizeYButton;
}

bool CMenuButton::OnMouseClick(int _iMouseButton) {
	
	(void)_iMouseButton;
	
	iPos++;

	ARX_CHECK_NOT_NEG( iPos );

	if( ARX_CAST_UINT( iPos ) >= vText.size() )
		iPos = 0;

	ARX_SOUND_PlayMenu(SND_MENU_CLICK);

	return false;
}

//-----------------------------------------------------------------------------

void CMenuButton::Update(int _iDTime) {
	(void)_iDTime;
}

//-----------------------------------------------------------------------------

void CMenuButton::Render()
{
	if(WILL_RELOAD_ALL_TEXTURES) return;

	if(bNoMenu) return;

	//affichage de la texture
	if(pTex) {
		EERIEDrawBitmap2(static_cast<float>(rZone.left), static_cast<float>(rZone.top),
		                 RATIO_X(pTex->m_dwWidth), RATIO_Y(pTex->m_dwHeight), 0, pTex, Color::white);
	}

	//affichage de la font
	if(vText.size())
	{
		char pText = vText[iPos];

		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);

		Vec3f ePos;
		ePos.x = (float)rZone.left;
		ePos.y = (float)rZone.top;
		ePos.z = 1;
		
		FontRenderText(pFont, ePos, &pText, Color(232, 204, 142));

		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	}
}

//-----------------------------------------------------------------------------

void CMenuButton::RenderMouseOver()
{
	if(WILL_RELOAD_ALL_TEXTURES) return;

	if(bNoMenu) return;

	pMenuCursor->SetMouseOver();

	//affichage de la texture
	if(pTexOver)
	{
		TexturedVertex v[4];
		v[0].color = v[1].color = v[2].color = v[3].color = Color::white.toBGR();
		v[0].sz=v[1].sz=v[2].sz=v[3].sz=0.f;
		v[0].rhw=v[1].rhw=v[2].rhw=v[3].rhw=0.999999f;

		GRenderer->SetTexture(0, pTexOver);
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
		EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);
	}

	if( vText.size() )
	{
		char pText=vText[iPos];

		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
		
		Vec3f ePos;
		ePos.x = (float)rZone.left;
		ePos.y = (float)rZone.top;
		ePos.z = 1;
		
		FontRenderText(pFont, ePos, &pText, Color(255, 255, 255));
		
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	}
}

//-----------------------------------------------------------------------------

CMenuSliderText::CMenuSliderText(int _iID, int _iPosX, int _iPosY)
	: CMenuElement(NOP)
{
	iID = _iID;
	TextureContainer *pTex = TextureContainer::Load("\\Graph\\interface\\menus\\menu_slider_button_left.bmp");
	pLeftButton = new CMenuButton(-1, hFontMenu, NOP, _iPosX, _iPosY, string(), 1, pTex, pTex, -1);
	pTex = TextureContainer::Load("\\Graph\\interface\\menus\\menu_slider_button_right.bmp");
	pRightButton = new CMenuButton(-1, hFontMenu, NOP, _iPosX, _iPosY, string(), 1, pTex, pTex, -1);

	vText.clear();

	iPos = 0;
	iOldPos = -1;

	rZone.left   = _iPosX;
	rZone.top    = _iPosY;
	rZone.right  = _iPosX + pLeftButton->GetWidth() + pRightButton->GetWidth();
	rZone.bottom = _iPosY + max(pLeftButton->GetHeight(), pRightButton->GetHeight());

	pRef = this;
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

	for(it=vText.begin();it<vText.end();++it)
	{
		CMenuElementText *pMenuElementText=*it;
		Vec2i textSize = pMenuElementText->GetTextSize();

		int dxx=(dx-textSize.x)>>1;
		pMenuElementText->SetPos(ARX_CLEAN_WARN_CAST_FLOAT(pLeftButton->rZone.right + dxx), ARX_CLEAN_WARN_CAST_FLOAT(rZone.top));
	}
}

//-----------------------------------------------------------------------------

void CMenuSliderText::AddText(CMenuElementText *_pText)
{
	_pText->Move(rZone.left + pLeftButton->GetWidth(), rZone.top + 0);
	vText.insert(vText.end(), _pText);

	Vec2i textSize = _pText->GetTextSize();

	rZone.right  = max(rZone.right, rZone.left + pLeftButton->GetWidth() + pRightButton->GetWidth() + textSize.x);
	rZone.bottom = max(rZone.bottom, rZone.top + textSize.y);

	pLeftButton->SetPos(rZone.left, rZone.top+(textSize.y>>2));
	pRightButton->SetPos(rZone.right-pRightButton->GetWidth(), rZone.top+(textSize.y>>2));

	int dx=rZone.right-rZone.left-pLeftButton->GetWidth()-pRightButton->GetWidth();
	//on recentre tout
	vector<CMenuElementText*>::iterator it;

	for(it=vText.begin();it<vText.end();++it)
	{
		CMenuElementText *pMenuElementText=*it;
		
		textSize = pMenuElementText->GetTextSize();

		int dxx=(dx-textSize.x)>>1;
		pMenuElementText->SetPos(ARX_CLEAN_WARN_CAST_FLOAT(pLeftButton->rZone.right + dxx), ARX_CLEAN_WARN_CAST_FLOAT(rZone.top));
	}
}

//-----------------------------------------------------------------------------

void CMenuSliderText::Move(int _iX, int _iY)
{
	CMenuZone::Move(_iX, _iY);

	pLeftButton->Move(_iX, _iY);
	pRightButton->Move(_iX, _iY);

	for (std::vector<CMenuElementText*>::const_iterator i = vText.begin(), i_end = vText.end(); i != i_end; ++i)
		(*i)->Move(_iX, _iY);
}

//-----------------------------------------------------------------------------

void CMenuSliderText::EmptyFunction()
{
	//Touche pour la selection
	if(GInput->isKeyPressedNowPressed(Keyboard::Key_LeftArrow))
	{
		iPos--;

		if (iPos <= 0) iPos = 0;
	}
	else
	{
		if( GInput->isKeyPressedNowPressed( Keyboard::Key_RightArrow ) )
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

	int iX = GInput->getMousePosAbs().x;
	int iY = GInput->getMousePosAbs().y;

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

			if ( ARX_CAST_UINT( iPos ) >= vText.size() - 1 )
				iPos = vText.size() - 1 ;
		}
	}

	switch (iID)
	{
	// MENUOPTIONS_VIDEO
	case BUTTON_MENUOPTIONSVIDEO_RESOLUTION:
		{
			std::string pcText = (vText.at(iPos))->lpszText;
			std::stringstream ss( pcText );
			int iX = config.video.width;
			int iY = config.video.height;
			char tmp;
			ss >> iX >> tmp >> iY;
//            pcText, "%dx%d"), &iX, &iY);
			{
				newWidth = iX;
				newHeight = iY;
				changeResolution = true;
			}
		}
		break;
	// MENUOPTIONS_VIDEO
	case BUTTON_MENUOPTIONSVIDEO_BPP:
		{
			std::string pcText;
			std::stringstream ss;
			pcText = vText[iPos]->lpszText;
			ss << pcText;
			ss >> newBpp;
			changeResolution = true;
		}
		break;
	case BUTTON_MENUOPTIONSVIDEO_TEXTURES:
		{
			{
				newTextureSize = iPos;
				changeTextures = true;
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

	if (vText[iPos])
	{
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		vText[iPos]->Render();
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	}
}

//-----------------------------------------------------------------------------

void CMenuSliderText::RenderMouseOver()
{
	if(WILL_RELOAD_ALL_TEXTURES) return;

	if(bNoMenu) return;

	pMenuCursor->SetMouseOver();

	int iX = GInput->getMousePosAbs().x;
	int iY = GInput->getMousePosAbs().y;

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);

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
// CMenuSlider
//-----------------------------------------------------------------------------

CMenuSlider::CMenuSlider(int _iID, int _iPosX, int _iPosY)
	: CMenuElement(NOP)
{
	iID = _iID;

	TextureContainer *pTexL = TextureContainer::Load("\\Graph\\interface\\menus\\menu_slider_button_left.bmp");
	TextureContainer *pTexR = TextureContainer::Load("\\Graph\\interface\\menus\\menu_slider_button_right.bmp");
	pLeftButton = new CMenuButton(-1, hFontMenu, NOP, _iPosX, _iPosY, string(), 1, pTexL, pTexR, -1);
	pRightButton = new CMenuButton(-1, hFontMenu, NOP, _iPosX, _iPosY, string(), 1, pTexR, pTexL, -1);
	pTex1 = TextureContainer::Load("\\Graph\\interface\\menus\\menu_slider_on.bmp");
	pTex2 = TextureContainer::Load("\\Graph\\interface\\menus\\menu_slider_off.bmp");

	iPos = 0;

	rZone.left   = _iPosX;
	rZone.top    = _iPosY;
	rZone.right  = _iPosX + pLeftButton->GetWidth() + pRightButton->GetWidth() + 10*max(pTex1->m_dwWidth, pTex2->m_dwWidth);
	rZone.bottom = _iPosY + max(pLeftButton->GetHeight(), pRightButton->GetHeight());

	ARX_CHECK_NOT_NEG( rZone.bottom );
	rZone.bottom = max( ARX_CAST_ULONG( rZone.bottom ), (unsigned long)max( pTex1->m_dwHeight, pTex2->m_dwHeight ) );


	pRightButton->Move(pLeftButton->GetWidth() + 10*max(pTex1->m_dwWidth, pTex2->m_dwWidth), 0);

	pRef = this;
}

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
	if(GInput->isKeyPressedNowPressed(Keyboard::Key_LeftArrow))
	{
		iPos--;

		if (iPos <= 0) iPos = 0;
	}
	else
	{
		if(GInput->isKeyPressedNowPressed(Keyboard::Key_RightArrow))
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

	int iX = GInput->getMousePosAbs().x;
	int iY = GInput->getMousePosAbs().y;

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
		ARXMenu_Options_Video_SetFogDistance(iPos);
		break;
	case BUTTON_MENUOPTIONSVIDEO_GAMMA:
		ARXMenu_Options_Video_SetGamma(iPos);
		break;
	case BUTTON_MENUOPTIONSVIDEO_LUMINOSITY:
		ARXMenu_Options_Video_SetLuminosity(iPos);
		break;
	case BUTTON_MENUOPTIONSVIDEO_CONTRAST:
		ARXMenu_Options_Video_SetContrast(iPos);
		break;
	// MENUOPTIONS_AUDIO
	case BUTTON_MENUOPTIONSAUDIO_MASTER:
		ARXMenu_Options_Audio_SetMasterVolume(iPos);
		break;
	case BUTTON_MENUOPTIONSAUDIO_SFX:
		ARXMenu_Options_Audio_SetSfxVolume(iPos);
		break;
	case BUTTON_MENUOPTIONSAUDIO_SPEECH:
		ARXMenu_Options_Audio_SetSpeechVolume(iPos);
		break;
	case BUTTON_MENUOPTIONSAUDIO_AMBIANCE:
		ARXMenu_Options_Audio_SetAmbianceVolume(iPos);
		break;
	// MENUOPTIONS_CONTROLS
	case BUTTON_MENUOPTIONS_CONTROLS_MOUSESENSITIVITY:
		ARXMenu_Options_Control_SetMouseSensitivity(iPos);
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

	pRightButton->Move(    ARX_CLEAN_WARN_CAST_INT(fWidth), 0);

	ARX_CHECK_LONG( rZone.left + pLeftButton->GetWidth() + pRightButton->GetWidth() + RATIO_X(10*std::max(pTex1->m_dwWidth, pTex2->m_dwWidth)) );
	rZone.right  = ARX_CLEAN_WARN_CAST_LONG( rZone.left + pLeftButton->GetWidth() + pRightButton->GetWidth() + RATIO_X(10*std::max(pTex1->m_dwWidth, pTex2->m_dwWidth)) );

	rZone.bottom = rZone.top + std::max(pLeftButton->GetHeight(), pRightButton->GetHeight());

}

void CMenuSlider::Render()
{
	if(WILL_RELOAD_ALL_TEXTURES) return;

	if(bNoMenu) return;

	pLeftButton->Render();
	pRightButton->Render();


	float iX = ARX_CLEAN_WARN_CAST_FLOAT( rZone.left + pLeftButton->GetWidth() );
	float iY = ARX_CLEAN_WARN_CAST_FLOAT( rZone.top );

	float iTexW = 0;

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);

	TexturedVertex v[4];
	v[0].color = v[1].color = v[2].color = v[3].color = Color::white.toBGR();
	v[0].sz=v[1].sz=v[2].sz=v[3].sz=0.f;    
	v[0].rhw=v[1].rhw=v[2].rhw=v[3].rhw=0.999999f;

	TextureContainer *pTex = pTex1;

	for (int i=0; i<10; i++)
	{
		iTexW = 0;

		if (i<iPos)
		{
			if(pTex1)
			{
				pTex = pTex1;
				iTexW = RATIO_X(pTex1->m_dwWidth);
			}
		}
		else
		{
			if(pTex2)
			{
				pTex = pTex2;
				iTexW = RATIO_X(pTex2->m_dwWidth);
			}
		}

		EERIEDrawBitmap2(iX, iY, RATIO_X(pTex->m_dwWidth), RATIO_Y(pTex->m_dwHeight), 0, pTex, Color::white);

		iX += iTexW;
	}

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}

void CMenuSlider::RenderMouseOver()
{
	if(WILL_RELOAD_ALL_TEXTURES) return;

	if(bNoMenu) return;

	pMenuCursor->SetMouseOver();

	int iX = GInput->getMousePosAbs().x;
	int iY = GInput->getMousePosAbs().y;

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);

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

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}

//-----------------------------------------------------------------------------

MenuCursor::MenuCursor()
{
	pTex[0]=TextureContainer::Find("graph\\interface\\cursors\\cursor00.bmp");
	pTex[1]=TextureContainer::Find("graph\\interface\\cursors\\cursor01.bmp");
	pTex[2]=TextureContainer::Find("graph\\interface\\cursors\\cursor02.bmp");
	pTex[3]=TextureContainer::Find("graph\\interface\\cursors\\cursor03.bmp");
	pTex[4]=TextureContainer::Find("graph\\interface\\cursors\\cursor04.bmp");
	pTex[5]=TextureContainer::Find("graph\\interface\\cursors\\cursor05.bmp");
	pTex[6]=TextureContainer::Find("graph\\interface\\cursors\\cursor06.bmp");
	pTex[7]=TextureContainer::Find("graph\\interface\\cursors\\cursor07.bmp");

	SetCursorOff();
	
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
	
	bDrawCursor=true;
}

//-----------------------------------------------------------------------------

MenuCursor::~MenuCursor()
{
}

//-----------------------------------------------------------------------------

void MenuCursor::SetCursorOff()
{
	eNumTex=CURSOR_OFF;
}

//-----------------------------------------------------------------------------

void MenuCursor::SetCursorOn()
{
	eNumTex=CURSOR_ON;
}

//-----------------------------------------------------------------------------

void MenuCursor::SetMouseOver()
{
	bMouseOver=true;
	SetCursorOn();
}

//-----------------------------------------------------------------------------

void MenuCursor::DrawOneCursor(const Vec2s& mousePos) {
	
	GRenderer->GetTextureStage(0)->SetMinFilter(TextureStage::FilterNearest);
	GRenderer->GetTextureStage(0)->SetMagFilter(TextureStage::FilterNearest);
	GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapClamp);
	EERIEDrawBitmap2(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y),
	                 INTERFACE_RATIO_DWORD(scursor[iNumCursor]->m_dwWidth),
	                 INTERFACE_RATIO_DWORD(scursor[iNumCursor]->m_dwHeight),
	                 0.00000001f, scursor[iNumCursor], Color::white);
	GRenderer->GetTextureStage(0)->SetMinFilter(TextureStage::FilterLinear);
	GRenderer->GetTextureStage(0)->SetMagFilter(TextureStage::FilterLinear);
	GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapRepeat);
}

//-----------------------------------------------------------------------------

void MenuCursor::Update()
{
    Vec2s iDiff;

	if(pTex[eNumTex])
	{
		iDiff.x=pTex[eNumTex]->m_dwWidth>>1;
		iDiff.y=pTex[eNumTex]->m_dwHeight>>1;
	}
	else
	{
		iDiff.x=0;
		iDiff.y=0;
	}

	iOldCoord[iNbOldCoord] = GInput->getMousePosAbs() + iDiff;
	iNbOldCoord++;

	if(iNbOldCoord>=iMaxOldCoord)
	{
		iNbOldCoord=iMaxOldCoord-1;
		memmove((void*)iOldCoord,(void*)(iOldCoord+1),sizeof(Vec2s)*iNbOldCoord);
	}

}

//-----------------------------------------------------------------------------

static bool ComputePer(const Vec2s & _psPoint1, const Vec2s & _psPoint2, TexturedVertex * _psd3dv1, TexturedVertex * _psd3dv2, float _fSize) {
	
	Vec2f sTemp((float)(_psPoint2.x - _psPoint1.x), (float)(_psPoint2.y - _psPoint1.y));
	float fTemp = sTemp.x;
	sTemp.x = -sTemp.y;
	sTemp.y = fTemp;
	float fMag = sTemp.length();
	if(fMag < EEdef_EPSILON) {
		return false;
	}

	fMag = _fSize / fMag;

	_psd3dv1->sx=(sTemp.x*fMag);
	_psd3dv1->sy=(sTemp.y*fMag);
	_psd3dv2->sx=((float)_psPoint1.x)-_psd3dv1->sx;
	_psd3dv2->sy=((float)_psPoint1.y)-_psd3dv1->sy;
	_psd3dv1->sx+=((float)_psPoint1.x);
	_psd3dv1->sy+=((float)_psPoint1.y);

	return true;
}

//-----------------------------------------------------------------------------

static void DrawLine2D(const Vec2s * _psPoint1, int _iNbPt, float _fSize, float _fRed, float _fGreen, float _fBlue) {
	
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

	GRenderer->SetBlendFunc(Renderer::BlendDstColor, Renderer::BlendInvDstColor);
	GRenderer->ResetTexture(0);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	TexturedVertex v[4];
	v[0].sz=v[1].sz=v[2].sz=v[3].sz=0.f;    
	v[0].rhw=v[1].rhw=v[2].rhw=v[3].rhw=0.999999f;

	const Vec2s * psOldPoint = _psPoint1++;
	v[0].color = v[2].color = Color3f(fColorRed, fColorGreen, fColorBlue).toBGR();

	if(!ComputePer(*psOldPoint, *_psPoint1, &v[0], &v[2], fTaille)) {
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

		if(ComputePer(*psOldPoint, *(_psPoint1 + 1), &v[1], &v[3], fTaille)) {
			v[1].color = v[3].color = Color3f(fColorRed, fColorGreen, fColorBlue).toBGR();
			EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);

			v[0].sx=v[1].sx;
			v[0].sy=v[1].sy;
			v[0].color=v[1].color;
			v[2].sx=v[3].sx;
			v[2].sy=v[3].sy;
			v[2].color=v[3].color;
		}

		psOldPoint = _psPoint1++;
	}

	fTaille+=fSize;
	fColorRed+=fDColorRed;
	fColorGreen+=fDColorGreen;
	fColorBlue+=fDColorBlue;

	if(ComputePer(*_psPoint1, *psOldPoint, &v[1], &v[3], fTaille)) {
		v[1].color = v[3].color = Color3f(fColorRed, fColorGreen, fColorBlue).toBGR();
		EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);
	}

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}

//-----------------------------------------------------------------------------

void MenuCursor::DrawCursor()
{
	if(!bDrawCursor)
		return;

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	DrawLine2D(iOldCoord,iNbOldCoord + 1,10.f,.725f,.619f,0.56f);

	if(pTex[iNumCursor]) 
		GRenderer->SetTexture(0, pTex[iNumCursor]);
	else 
		GRenderer->ResetTexture(0);

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);

	GRenderer->SetRenderState(Renderer::DepthTest, false);
	DrawOneCursor(GInput->getMousePosAbs());
	GRenderer->SetRenderState(Renderer::DepthTest, true);


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

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
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

	if(pMenuCursor)
	{
		delete pMenuCursor;
		pMenuCursor = NULL;
	}
}
