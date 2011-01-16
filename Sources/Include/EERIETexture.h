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
// EERIETextr
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//
// Updates: (date) (person) (update)
//
// Code:	Cyril Meynier
//			Sébastien Scieux	(JPEG & PNG)
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved
/////////////////////////////////////////////////////////////////////////////////////

// Desc: Functions to manage textures, including creating (loading from a
//       file), restoring lost surfaces, invalidating, and destroying.
//
//       Note: the implementation of these fucntions maintain an internal list
//       of loaded textures. After creation, individual textures are referenced
//       via their ASCII names.
#ifndef EERIETEXTR_H
#define EERIETEXTR_H

#include "EERIETypes.h"
#include <vector>

using namespace std;

extern long EERIE_USES_BUMP_MAP;
#define MAX_MIPS 2
extern long GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE;
void ReleaseAllTCWithFlag(long flag);
#define EERIETEXTUREFLAG_PERMANENT			1
#define EERIETEXTUREFLAG_LOADSCENE_RELEASE	2

typedef struct
{
	EERIEPOLY * data;
} DELAYED_PRIM;

//-----------------------------------------------------------------------------
// Name: TextureContainer
// Desc: Linked list structure to hold info per texture
//-----------------------------------------------------------------------------

class TextureContainer
{
	public:
		TextureContainer * m_pNext;			// Linked list ptr

		char	m_strName[256];				// Image filename
		char	m_texName[256];				// Name of texture
		DWORD   m_dwWidth;
		DWORD   m_dwHeight;
		DWORD	m_dwOriginalWidth;
		DWORD	m_dwOriginalHeight;
		DWORD	m_dwDeviceWidth;
		DWORD	m_dwDeviceHeight;
		DWORD   m_dwStage;					// Texture stage (for multitexture devices)
		DWORD   m_dwBPP;
		DWORD   m_dwFlags;
		BOOL    m_bHasAlpha;
		DWORD	userflags;
		bool	bColorKey;
		bool	bColorKey2D;

		LPDIRECTDRAWSURFACE7 m_pddsSurface;	// Surface of the texture
		LPDIRECTDRAWSURFACE7 m_pddsBumpMap;	// Surface of BumpMap
		HBITMAP m_hbmBitmap;				// Bitmap containing texture image
		DWORD * m_pRGBAData;
		unsigned char * m_pJPEGData;
		unsigned char * m_pJPEGData_ex;
		unsigned char	* m_pPNGData;

	public:
		LPDIRECT3DVERTEXBUFFER7		vertexbuffer;
		LPDIRECT3DVERTEXBUFFER7		vbuf;
		long	vbuf_max;

		DELAYED_PRIM *	delayed;			// delayed_drawing
		long	delayed_nb;
		long	delayed_max;

		vector<EERIEPOLY *> vPolyZMap;
		vector<EERIEPOLY *> vPolyBump;
		vector<D3DTLVERTEX> vPolyInterBump;
		vector<SMY_ZMAPPINFO> vPolyInterZMap;
		vector<D3DTLVERTEX> vPolyInterBumpTANDL;
		vector<SMY_ZMAPPINFO> vPolyInterZMapTANDL;

		SMY_ARXMAT * tMatRoom;

		unsigned long	ulMaxVertexListCull;
		unsigned long	ulNbVertexListCull;
		ARX_D3DVERTEX	* pVertexListCull;
		unsigned long	ulMaxVertexListCull_TNormalTrans;
		unsigned long	ulNbVertexListCull_TNormalTrans;
		ARX_D3DVERTEX	* pVertexListCull_TNormalTrans;
		unsigned long	ulMaxVertexListCull_TAdditive;
		unsigned long	ulNbVertexListCull_TAdditive;
		ARX_D3DVERTEX	* pVertexListCull_TAdditive;
		unsigned long	ulMaxVertexListCull_TSubstractive;
		unsigned long	ulNbVertexListCull_TSubstractive;
		ARX_D3DVERTEX	* pVertexListCull_TSubstractive;
		unsigned long	ulMaxVertexListCull_TMultiplicative;
		unsigned long	ulNbVertexListCull_TMultiplicative;
		ARX_D3DVERTEX	* pVertexListCull_TMultiplicative;
		unsigned long	ulMaxVertexListCull_TMetal;
		unsigned long	ulNbVertexListCull_TMetal;
		ARX_D3DVERTEX	* pVertexListCull_TMetal;

		//version hybride T&L
		unsigned long	ulMaxVertexListCullH;
		unsigned long	ulNbVertexListCullH;
		ARX_D3DVERTEX	* pVertexListCullH;
		unsigned long	ulMaxVertexListCull_TNormalTransH;
		unsigned long	ulNbVertexListCull_TNormalTransH;
		ARX_D3DVERTEX	* pVertexListCull_TNormalTransH;
		unsigned long	ulMaxVertexListCull_TAdditiveH;
		unsigned long	ulNbVertexListCull_TAdditiveH;
		ARX_D3DVERTEX	* pVertexListCull_TAdditiveH;
		unsigned long	ulMaxVertexListCull_TSubstractiveH;
		unsigned long	ulNbVertexListCull_TSubstractiveH;
		ARX_D3DVERTEX	* pVertexListCull_TSubstractiveH;
		unsigned long	ulMaxVertexListCull_TMultiplicativeH;
		unsigned long	ulNbVertexListCull_TMultiplicativeH;
		ARX_D3DVERTEX	* pVertexListCull_TMultiplicativeH;
		unsigned long	ulMaxVertexListCull_TMetalH;
		unsigned long	ulNbVertexListCull_TMetalH;
		ARX_D3DVERTEX	* pVertexListCull_TMetalH;

		float	m_dx;						// precalculated 1.f/width
		float	m_dy;						// precalculated 1.f/height
		float	m_hdx;						// precalculated 0.5f/width
		float	m_hdy;						// precalculated 0.5f/height
		float	m_odx;						// precalculated 1.f/width
		float	m_ody;						// precalculated 1.f/height
 
		HRESULT LoadImageData();
		HRESULT LoadBitmapFile(TCHAR * strPathname);
		HRESULT LoadTargaFile(TCHAR * strPathname);
 
		HRESULT LoadJpegFileNoDecomp(TCHAR * strPathname);
		HRESULT LoadPNGFile(TCHAR * strPathname);
		HRESULT Restore(LPDIRECT3DDEVICE7 pd3dDevice);
		HRESULT CopyBitmapToSurface(LPDIRECTDRAWSURFACE7 Surface);
		HRESULT CopyBitmapToSurface2(HBITMAP hbitmap, int depx, int depy, int largeur, int hauteur, long flag = 0);
		HRESULT CopyRGBADataToSurface(LPDIRECTDRAWSURFACE7 Surface);
		HRESULT CopyJPEGDataToSurface(LPDIRECTDRAWSURFACE7 Surface);
		HRESULT CopyPNGDataToSurface(LPDIRECTDRAWSURFACE7 Surface);
		HRESULT Use();

		TextureContainer * TextureRefinement;
		TextureContainer(TCHAR * strName, char * wd, DWORD dwStage, DWORD dwFlags);
		~TextureContainer();
		long	locks;
		long systemflags;
		long mcachecount;
		bool NoResize;
	
		bool CreateHalo(LPDIRECT3DDEVICE7 _lpDevice);
		TextureContainer * AddHalo(LPDIRECT3DDEVICE7 _lpDevice, int _iNbCouche, float _fR, float _fG, float _fB, float & _iDecalX, float & _iDecalY);
		float halodecalX;
		float halodecalY;
		TextureContainer * TextureHalo;
		int iHaloNbCouche;
		float fHaloRed;
		float fHaloGreen;
		float fHaloBlue;
};


//-----------------------------------------------------------------------------
// Access functions for loaded textures. Note: these functions search
// an internal list of the textures, and use the texture associated with the
// ASCII name.
//-----------------------------------------------------------------------------
 
TextureContainer * D3DTextr_GetSurfaceContainer(TCHAR * strName);
TextureContainer * GetTextureList();
extern TextureContainer * LastTextureContainer;
long CountTextures(char * tex, long * memsize, long * memmip);
 
//-----------------------------------------------------------------------------
// Texture invalidation and restoration functions
//-----------------------------------------------------------------------------
 
HRESULT D3DTextr_Restore(TCHAR * strName, LPDIRECT3DDEVICE7 pd3dDevice);
HRESULT D3DTextr_InvalidateAllTextures();
HRESULT D3DTextr_RestoreAllTextures(LPDIRECT3DDEVICE7 pd3dDevice);
HRESULT D3DTextr_TESTRestoreAllTextures(LPDIRECT3DDEVICE7 pd3dDevice);
void ReloadAllTextures(LPDIRECT3DDEVICE7 pd3dDevice);
void ReloadTexture(TextureContainer * tc);
#define D3DCOLORWHITE 0xFFFFFFFF
#define D3DCOLORBLACK 0xFF000000

//-----------------------------------------------------------------------------
// Texture creation and deletion functions
//-----------------------------------------------------------------------------
#define D3DTEXTR_TRANSPARENTWHITE 0x00000001
#define D3DTEXTR_TRANSPARENTBLACK 0x00000002
#define D3DTEXTR_32BITSPERPIXEL   0x00000004
#define D3DTEXTR_16BITSPERPIXEL   0x00000008
#define D3DTEXTR_CREATEWITHALPHA  0x00000010
#define D3DTEXTR_NO_MIPMAP		  (1<<5)
#define D3DTEXTR_FAKE_BORDER	  (1<<6)
#define D3DTEXTR_NO_REFINEMENT	  (1<<7)
#define D3DTEXTR_NO_INSERT		  0x80000000

TextureContainer  * D3DTextr_CreateTextureFromFile(TCHAR * strName, char * wd = NULL , DWORD dwStage = 0L,
        DWORD dwFlags = 0L , long sysflags = 0);
HRESULT D3DTextr_CreateEmptyTexture(TCHAR * strName, DWORD dwWidth,
                                    DWORD dwHeight, DWORD dwStage,
                                    DWORD dwFlags , char * wd, DWORD flags = 0);
 
HRESULT D3DTextr_DestroyContainer(TextureContainer * ptcTexture);

TextureContainer * GetAnyTexture();
TextureContainer * MakeTCFromFile(char * tex, long flag = 0);
TextureContainer * MakeTCFromFile_NoRefinement(char * tex, long flag = 0);
TextureContainer * GetTextureFile(char * tex, long flag = 0);
TextureContainer * GetTextureFile_NoRefinement(char * tex, long flag = 0);

void EERIE_ActivateBump(void);
void EERIE_DesactivateBump(void);
void D3DTextr_KillTexture(TextureContainer * tex); 
void D3DTextr_KillAllTextures();	
void SpecialBorderSurface(TextureContainer * tc, unsigned long x0, unsigned long y0);

TextureContainer * FindTexture(TCHAR * strTextureName);
TextureContainer * _FindTexture(char * strTextureName);

bool TextureContainer_Exist(TextureContainer * tc);
#endif
