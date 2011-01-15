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
// EERIETexture.cpp
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		Texture Management Functions: Create, Restore Lost Surfaces, Invalidate
//      Surfaces, Destroy Surfaces.
//
// Updates: (date) (person) (update)
//
// Code:	Cyril Meynier
//			Sébastien Scieux	(JPEG & PNG)
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////
#define STRICT
#include <tchar.h>
#include <stdio.h>

#include <jpeglib.h>
#include <jerror.h>
#include <jconfig.h>
#include <jmorecfg.h>

#include <zlib.h>

#include "EERIETexture.h"
#include "EERIEApp.h"
#include "EERIEUtil.h"
#include "EERIEJpeg.h"
#include "EERIEMath.h"

#include "HERMESMain.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

long GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE = 0;
/*-----------------------------------------------------------------------------*/
#define OR	||
#define AND &&

#define	PNG_ERROR	0
#define PNG_OK		1

#define GRAYSCALE_USED	0		//validate value:0,2,3,4,6
#define	PALETTE_USED	1
#define COLOR_USED		2
#define	ALPHA_USED		4

#define PNG_HEADER		0x52444849
#define PNG_TEXT		0x74584574
#define PNG_TIME		0x454d4974
#define PNG_PHYS		0x73594870
#define PNG_GAMMA		0x414d4167
#define PNG_DATAS		0x54414449
#define PNG_END			0x444e4549
#define PNG_PALETTE		0x45544c50

#define PNG_FNONE		0
#define PNG_FSUB		1
#define PNG_FUP			2
#define PNG_FAVERAGE	3
#define PNG_FPAETH		4

typedef struct
{
	unsigned int		width;
	unsigned int		height;
	char				bitdepth;
	char				colortype;
	char				compression;
	char				filter;
	char				interlace;
} PNG_IHDR;

typedef struct
{
	unsigned char r, g, b;
} PNG_PALETTE_ARXX;

typedef struct
{
	unsigned int		pnglzwtaille;
	unsigned int		pngbpp;
	unsigned int		pngmodulos;
	PNG_IHDR		*	pngh;
	PNG_PALETTE_ARXX	* pngpalette;
	void		*		pnglzwdatas;
} DATAS_PNG;

/*-----------------------------------------------------------------------------*/
// Local list of textures
//-----------------------------------------------------------------------------
// Macros, function prototypes and static variable
//-----------------------------------------------------------------------------
 
TextureContainer	* g_ptcTextureList = NULL;
bool				bGlobalTextureStretch;
char		*		PNGDatas;
unsigned int		ChunkType, ChunkTaille, ChunkCRC;

/*-----------------------------------------------------------------------------*/
void InverseLong(int * l)
{
	_asm
	{
		push ebx;
		mov ebx, dword ptr[l];
		mov eax, [ebx];
		bswap eax;
		mov dword ptr[ebx], eax;
		pop ebx;
	}
}








TextureContainer * MakeTCFromFile(char * tex, long flag)
{
	long old = GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE;
	GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE = -1;

	TextureContainer * tc = GetTextureFile(tex, flag);

	if (tc)
	{
		if (tc->m_pddsSurface == NULL)
			tc->Restore(GDevice);
	}

	GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE = old;
	return tc;
}

TextureContainer * MakeTCFromFile_NoRefinement(char * tex, long flag)
{
	long old = GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE;
	GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE = -1;

	TextureContainer * tc = GetTextureFile_NoRefinement(tex, flag);

	if (tc)
	{
		if (tc->m_pddsSurface == NULL)
			tc->Restore(GDevice);
	}

	GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE = old;
	return tc;
}

//-----------------------------------------------------------------------------
// Name: CD3DTextureManager
// Desc: Class used to automatically construct and destruct the static
//       texture engine class.
//-----------------------------------------------------------------------------
class CD3DTextureManager
{
		public:
		CD3DTextureManager() {}
		~CD3DTextureManager()
		{
			if (g_ptcTextureList) delete g_ptcTextureList;
		}
};

//-----------------------------------------------------------------------------
// Name: struct TEXTURESEARCHINFO
// Desc: Structure used to search for texture formats
//-----------------------------------------------------------------------------
struct TEXTURESEARCHINFO
{
	DWORD dwDesiredBPP;   // Input for texture format search
	BOOL  bUseAlpha;
	BOOL  bUsePalette;
	BOOL  bFoundGoodFormat;

	DDPIXELFORMAT * pddpf; // Output of texture format search
};

//-----------------------------------------------------------------------------
// Name: TextureSearchCallback()
// Desc: Enumeration callback routine to find a best-matching texture format.
//       The param data is the DDPIXELFORMAT of the best-so-far matching
//       texture. Note: the desired BPP is passed in the dwSize field, and the
//       default BPP is passed in the dwFlags field.
//-----------------------------------------------------------------------------
static HRESULT CALLBACK TextureSearchCallback(DDPIXELFORMAT * pddpf,
        VOID * param)
{
	if (NULL == pddpf || NULL == param)
		return DDENUMRET_OK;

	TEXTURESEARCHINFO * ptsi = (TEXTURESEARCHINFO *)param;

	// Skip any funky modes
	if (pddpf->dwFlags & (DDPF_LUMINANCE | DDPF_BUMPLUMINANCE | DDPF_BUMPDUDV))
		return DDENUMRET_OK;

	// Check for palettized formats
	if (ptsi->bUsePalette)
	{
		if (!(pddpf->dwFlags & DDPF_PALETTEINDEXED8))
			return DDENUMRET_OK;

		// Accept the first 8-bit palettized format we get
		memcpy(ptsi->pddpf, pddpf, sizeof(DDPIXELFORMAT));
		ptsi->bFoundGoodFormat = TRUE;
		return DDENUMRET_CANCEL;
	}

	// Else, skip any paletized formats (all modes under 16bpp)
	if (pddpf->dwRGBBitCount < 16)
		return DDENUMRET_OK;

	// Skip any FourCC formats
	if (pddpf->dwFourCC != 0)
		return DDENUMRET_OK;

	// Skip any ARGB 4444 formats (which are best used for pre-authored
	// content designed speciafically for an ARGB 4444 format).
	if (pddpf->dwRGBAlphaBitMask == 0x0000f000)
		return DDENUMRET_OK;

	// Make sure current alpha format agrees with requested format type
	if ((ptsi->bUseAlpha == TRUE) && !(pddpf->dwFlags & DDPF_ALPHAPIXELS))
		return DDENUMRET_OK;

	if ((ptsi->bUseAlpha == FALSE) && (pddpf->dwFlags & DDPF_ALPHAPIXELS))
		return DDENUMRET_OK;

	// Check if we found a good match
	if (pddpf->dwRGBBitCount == ptsi->dwDesiredBPP)
	{
		if ((pddpf->dwRBitMask == ptsi->pddpf->dwRBitMask) &&
		        (pddpf->dwGBitMask == ptsi->pddpf->dwGBitMask) &&
		        (pddpf->dwBBitMask == ptsi->pddpf->dwBBitMask))
		{
			memcpy(ptsi->pddpf, pddpf, sizeof(DDPIXELFORMAT));
			ptsi->bFoundGoodFormat = TRUE;
			return DDENUMRET_CANCEL;
		}
		else return DDENUMRET_OK;
	}

	return DDENUMRET_OK;
}

//-----------------------------------------------------------------------------
// Name: FindTexture()
// Desc: Searches the internal list of textures for a texture specified by
//       its name. Returns the structure associated with that texture.
//-----------------------------------------------------------------------------
TextureContainer * GetTextureList()
{
	return g_ptcTextureList;
}

TextureContainer * GetAnyTexture()
{
	return g_ptcTextureList;
}

TextureContainer * FindTexture(TCHAR * strTextureName)
{
	TextureContainer * ptcTexture = g_ptcTextureList;

	while (ptcTexture)
	{
		if (strstr(strTextureName, ptcTexture->m_texName))
			return ptcTexture;

		ptcTexture = ptcTexture->m_pNext;
	}

	return NULL;
}
TextureContainer * _FindTexture(char * strTextureName)
{
	TextureContainer * ptcTexture = g_ptcTextureList;

	while (ptcTexture)
	{
		if (strstr(strTextureName, ptcTexture->m_texName))
			return ptcTexture;

		ptcTexture = ptcTexture->m_pNext;
	}

	return NULL;
}
long BLURTEXTURES = 0;
long NOMIPMAPS = 0;

#define NB_MIPMAP_LEVELS 5

// tex Must be of sufficient size...
long CountTextures(char * tex, long * memsize, long * memmip)
{
	char temp[512];
	TextureContainer * ptcTexture = g_ptcTextureList;
	long count = 0;
	*memsize = 0;
	*memmip = 0;

	if (tex != NULL)
		strcpy(tex, "");

	while (ptcTexture)
	{
		count++;

		if (tex != NULL)
		{
			if (ptcTexture->m_dwFlags & D3DTEXTR_NO_MIPMAP)
				sprintf(temp, "%3d %s %dx%dx%d %d %s\r\n", count, ptcTexture->m_strName, ptcTexture->m_dwWidth, ptcTexture->m_dwHeight, ptcTexture->m_dwBPP, ptcTexture->locks, GetName(ptcTexture->m_texName));
			else
			{
				sprintf(temp, "%3d %s %dx%dx%d %d MIP %s\r\n", count, ptcTexture->m_strName, ptcTexture->m_dwWidth, ptcTexture->m_dwHeight, ptcTexture->m_dwBPP, ptcTexture->locks, GetName(ptcTexture->m_texName));

				for (long k = 1; k <= NB_MIPMAP_LEVELS; k++)
				{
					*memmip += ((long)(ptcTexture->m_dwWidth * ptcTexture->m_dwHeight * ptcTexture->m_dwBPP) >> 3) / (4 * k);
				}
			}

			*memsize += (long)(ptcTexture->m_dwWidth * ptcTexture->m_dwHeight * ptcTexture->m_dwBPP) >> 3;
			strcat(tex, temp);
		}

		ptcTexture = ptcTexture->m_pNext;

	}

	return count;
}
void ReloadTexture(TextureContainer * ptcTexture)
{
	if (ptcTexture)
	{
		SAFE_RELEASE(ptcTexture->m_pddsSurface);
		ptcTexture->m_pddsSurface = NULL;
		SAFE_RELEASE(ptcTexture->m_pddsBumpMap);
		ptcTexture->m_pddsBumpMap = NULL;

		SAFE_DELETE_TAB(ptcTexture->m_pRGBAData);

		if (ptcTexture->m_pJPEGData)
		{
			struct	jpeg_decompress_struct	* cinfo = (jpeg_decompress_struct *)ptcTexture->m_pJPEGData;
			jpeg_destroy_decompress(cinfo);

			if (ptcTexture->m_pJPEGData_ex)
			{
				free((void *)ptcTexture->m_pJPEGData_ex);
				ptcTexture->m_pJPEGData_ex = NULL;
			}
		}

		SAFE_DELETE_TAB(ptcTexture->m_pJPEGData);
		SAFE_DELETE_TAB(ptcTexture->m_pPNGData);

		if (ptcTexture->m_hbmBitmap)
		{
			DeleteObject(ptcTexture->m_hbmBitmap);
			ptcTexture->m_hbmBitmap = NULL;
		}

		ptcTexture->m_pddsSurface = NULL;
		ptcTexture->m_pddsBumpMap = NULL;

		ptcTexture->m_hbmBitmap   = NULL;
		ptcTexture->m_pRGBAData   = NULL;
		ptcTexture->m_pJPEGData = NULL;
		ptcTexture->m_pJPEGData_ex = NULL;
		ptcTexture->m_pPNGData = NULL;

		ptcTexture->LoadImageData();

		ptcTexture->ulNbVertexListCull = 0;
		ptcTexture->ulNbVertexListCull_TNormalTrans = 0;
		ptcTexture->ulNbVertexListCull_TAdditive = 0;
		ptcTexture->ulNbVertexListCull_TSubstractive = 0;
		ptcTexture->ulNbVertexListCull_TMultiplicative = 0;
		ptcTexture->ulNbVertexListCull_TMetal = 0;

		ptcTexture->ulMaxVertexListCull = 0;
		ptcTexture->ulMaxVertexListCull_TNormalTrans = 0;
		ptcTexture->ulMaxVertexListCull_TAdditive = 0;
		ptcTexture->ulMaxVertexListCull_TSubstractive = 0;
		ptcTexture->ulMaxVertexListCull_TMultiplicative = 0;
		ptcTexture->ulMaxVertexListCull_TMetal = 0;

		ptcTexture->vPolyBump.clear();
		ptcTexture->vPolyInterBump.clear();
		ptcTexture->vPolyInterZMap.clear();
		ptcTexture->vPolyInterBumpTANDL.clear();
		ptcTexture->vPolyInterZMapTANDL.clear();
		ptcTexture->vPolyZMap.clear();

		if (ptcTexture->pVertexListCull) free((void *)ptcTexture->pVertexListCull);

		ptcTexture->pVertexListCull = NULL;

		if (ptcTexture->pVertexListCull_TNormalTrans) free((void *)ptcTexture->pVertexListCull_TNormalTrans);

		ptcTexture->pVertexListCull_TNormalTrans = NULL;

		if (ptcTexture->pVertexListCull_TAdditive) free((void *)ptcTexture->pVertexListCull_TAdditive);

		ptcTexture->pVertexListCull_TAdditive = NULL;

		if (ptcTexture->pVertexListCull_TSubstractive) free((void *)ptcTexture->pVertexListCull_TSubstractive);

		ptcTexture->pVertexListCull_TSubstractive = NULL;

		if (ptcTexture->pVertexListCull_TMultiplicative) free((void *)ptcTexture->pVertexListCull_TMultiplicative);

		ptcTexture->pVertexListCull_TMultiplicative = NULL;

		if (ptcTexture->pVertexListCull_TMetal) free((void *)ptcTexture->pVertexListCull_TMetal);

		ptcTexture->pVertexListCull_TMetal = NULL;


		//Hybride T&L
		ptcTexture->ulNbVertexListCullH = 0;
		ptcTexture->ulNbVertexListCull_TNormalTransH = 0;
		ptcTexture->ulNbVertexListCull_TAdditiveH = 0;
		ptcTexture->ulNbVertexListCull_TSubstractiveH = 0;
		ptcTexture->ulNbVertexListCull_TMultiplicativeH = 0;
		ptcTexture->ulNbVertexListCull_TMetalH = 0;
		ptcTexture->ulMaxVertexListCullH = 0;
		ptcTexture->ulMaxVertexListCull_TNormalTransH = 0;
		ptcTexture->ulMaxVertexListCull_TAdditiveH = 0;
		ptcTexture->ulMaxVertexListCull_TSubstractiveH = 0;
		ptcTexture->ulMaxVertexListCull_TMultiplicativeH = 0;
		ptcTexture->ulMaxVertexListCull_TMetalH = 0;

		if (ptcTexture->pVertexListCullH) free((void *)ptcTexture->pVertexListCullH);

		ptcTexture->pVertexListCullH = NULL;

		if (ptcTexture->pVertexListCull_TNormalTransH) free((void *)ptcTexture->pVertexListCull_TNormalTransH);

		ptcTexture->pVertexListCull_TNormalTransH = NULL;

		if (ptcTexture->pVertexListCull_TAdditiveH) free((void *)ptcTexture->pVertexListCull_TAdditiveH);

		ptcTexture->pVertexListCull_TAdditiveH = NULL;

		if (ptcTexture->pVertexListCull_TSubstractiveH) free((void *)ptcTexture->pVertexListCull_TSubstractiveH);

		ptcTexture->pVertexListCull_TSubstractiveH = NULL;

		if (ptcTexture->pVertexListCull_TMultiplicativeH) free((void *)ptcTexture->pVertexListCull_TMultiplicativeH);

		ptcTexture->pVertexListCull_TMultiplicativeH = NULL;

		if (ptcTexture->pVertexListCull_TMetalH) free((void *)ptcTexture->pVertexListCull_TMetalH);

		ptcTexture->pVertexListCull_TMetalH = NULL;

	}
}
void ReloadAllTextures(LPDIRECT3DDEVICE7 pd3dDevice)
{
	TextureContainer * ptcTexture = g_ptcTextureList;

	while (ptcTexture)
	{
		ReloadTexture(ptcTexture);

		ptcTexture = ptcTexture->m_pNext;
	}

	D3DTextr_RestoreAllTextures(pd3dDevice);
}

//-----------------------------------------------------------------------------
// Name: TextureContainer()
// Desc: Constructor for a texture object
//-----------------------------------------------------------------------------
TextureContainer::TextureContainer(TCHAR * strName, char * wd, DWORD dwStage,
                                   DWORD dwFlags)
{
	MakeUpcase(strName);
	lstrcpy(m_strName, strName);

	if (wd != NULL)
		lstrcpy(m_texName, strName + strlen(wd));
	else lstrcpy(m_texName, strName);

	m_dwWidth		= 0;
	m_dwHeight		= 0;
	m_dwStage		= dwStage;
	m_dwBPP			= 0;
	m_dwFlags		= dwFlags;
	m_bHasAlpha		= 0;
	bColorKey		= false;
	bColorKey2D		= false;

	m_pddsSurface = NULL;
	m_pddsBumpMap = NULL;

	m_hbmBitmap   = NULL;
	m_pRGBAData   = NULL;
	m_pJPEGData = NULL;
	m_pJPEGData_ex = NULL;
	m_pPNGData = NULL;

	userflags = 0;
	TextureRefinement = NULL;

	// Add the texture to the head of the global texture list
	if (!(dwFlags & D3DTEXTR_NO_INSERT))
	{
		m_pNext = g_ptcTextureList;
		g_ptcTextureList = this;
	}

	delayed = NULL;
	delayed_nb = 0;
	delayed_max = 0;
	vertexbuffer = NULL;
	locks = 0;
	systemflags = 0;
	mcachecount = 0;
	vbuf = NULL;
	vbuf_max = 0;
	NoResize = false;

	halodecalX = 0.f;
	halodecalY = 0.f;
	TextureHalo = NULL;

	ulMaxVertexListCull = 0;
	ulNbVertexListCull = 0;
	pVertexListCull = NULL; 

	ulMaxVertexListCull_TNormalTrans = 0; 
	ulNbVertexListCull_TNormalTrans = 0;
	pVertexListCull_TNormalTrans = NULL; 

	ulMaxVertexListCull_TAdditive = 0; 
	ulNbVertexListCull_TAdditive = 0;
	pVertexListCull_TAdditive = NULL; 

	ulMaxVertexListCull_TSubstractive = 0; 
	ulNbVertexListCull_TSubstractive = 0;
	pVertexListCull_TSubstractive = NULL; 

	ulMaxVertexListCull_TMultiplicative = 0; 
	ulNbVertexListCull_TMultiplicative = 0;
	pVertexListCull_TMultiplicative = NULL; 

	ulMaxVertexListCull_TMetal = 0;
	ulNbVertexListCull_TMetal = 0;
	pVertexListCull_TMetal = NULL; 

	tMatRoom = NULL;

	vPolyBump.clear();
	vPolyInterBump.clear();
	vPolyInterZMap.clear();
	vPolyInterBumpTANDL.clear();
	vPolyInterZMapTANDL.clear();
	vPolyZMap.clear();

	//Hybride T&L
	ulMaxVertexListCullH = ulNbVertexListCullH = 0;
	pVertexListCullH = NULL;
	ulMaxVertexListCull_TNormalTransH = ulNbVertexListCull_TNormalTransH = 0;
	pVertexListCull_TNormalTransH = NULL;
	ulMaxVertexListCull_TAdditiveH = ulNbVertexListCull_TAdditiveH = 0;
	pVertexListCull_TAdditiveH = NULL;
	ulMaxVertexListCull_TSubstractiveH = ulNbVertexListCull_TSubstractiveH = 0;
	pVertexListCull_TSubstractiveH = NULL;
	ulMaxVertexListCull_TMultiplicativeH = ulNbVertexListCull_TMultiplicativeH = 0;
	pVertexListCull_TMultiplicativeH = NULL;
	ulMaxVertexListCull_TMetalH = ulNbVertexListCull_TMetalH = 0;
	pVertexListCull_TMetalH = NULL;
}

bool TextureContainer_Exist(TextureContainer * tc)
{
	TextureContainer * ptcTexture = g_ptcTextureList;

	while (ptcTexture)
	{
		if (tc == ptcTexture)
			return true;

		ptcTexture = ptcTexture->m_pNext;
	}

	return false;
}


//-----------------------------------------------------------------------------
// Name: ~TextureContainer()
// Desc: Destructs the contents of the texture container
//-----------------------------------------------------------------------------
TextureContainer::~TextureContainer()
{
	if (!TextureContainer_Exist(this))
		return;

	SAFE_RELEASE(m_pddsSurface);
	SAFE_RELEASE(m_pddsBumpMap);
	SAFE_DELETE_TAB(m_pPNGData);

	SAFE_DELETE_TAB(m_pRGBAData);

	if (m_hbmBitmap)
	{
		DeleteObject(m_hbmBitmap);
		m_hbmBitmap = NULL;
	}

	if (vertexbuffer)
	{
		free(vertexbuffer);
		vertexbuffer = NULL;
	}

	if (vbuf)
	{
		free(vbuf);
		vbuf = NULL;
	}

	if (delayed)
	{
		free(delayed);
		delayed = NULL;
	}

	if (m_pJPEGData)
	{
		struct	jpeg_decompress_struct	* cinfo = (jpeg_decompress_struct *)m_pJPEGData;

		if (cinfo)
		{
			jpeg_destroy_decompress(cinfo);
		}
		else
		{
			assert(0);
		}

		if (m_pJPEGData_ex)
		{
			free((void *)m_pJPEGData_ex);
			m_pJPEGData_ex = NULL;
		}
	}

	if (m_pJPEGData) delete [] m_pJPEGData;

	m_pJPEGData = NULL;

	// Remove the texture container from the global list
	if (g_ptcTextureList == this)
		g_ptcTextureList = m_pNext;
	else
	{
		for (TextureContainer * ptc = g_ptcTextureList; ptc; ptc = ptc->m_pNext)
			if (ptc->m_pNext == this)
				ptc->m_pNext = m_pNext;
	}

	if (pVertexListCull) free((void *)pVertexListCull);

	if (pVertexListCull_TNormalTrans) free((void *)pVertexListCull_TNormalTrans);

	if (pVertexListCull_TAdditive) free((void *)pVertexListCull_TAdditive);

	if (pVertexListCull_TSubstractive) free((void *)pVertexListCull_TSubstractive);

	if (pVertexListCull_TMultiplicative) free((void *)pVertexListCull_TMultiplicative);

	if (pVertexListCull_TMetal) free((void *)pVertexListCull_TMetal);

	if (tMatRoom) free((void *)tMatRoom);

	vPolyBump.clear();
	vPolyInterBump.clear();
	vPolyInterZMap.clear();
	vPolyInterBumpTANDL.clear();
	vPolyInterZMapTANDL.clear();
	vPolyZMap.clear();
}

//-----------------------------------------------------------------------------
// Name: LoadImageData()
// Desc: Loads the texture map's image data
//-----------------------------------------------------------------------------
HRESULT TextureContainer::LoadImageData()
{
	TCHAR * strExtension, *strExtension2;
	TCHAR  strPathname[256];
	TCHAR  tempstrPathname[256];

	// Check File
	lstrcpy(strPathname, m_strName);

	if (NULL == (strExtension = _tcsrchr(m_strName, _T('.'))))
		return DDERR_UNSUPPORTED;

	strExtension2 = _tcsrchr(m_strName, _T('_'));

	HRESULT hres;
	strcpy(tempstrPathname, strPathname);
	SetExt(tempstrPathname, ".png");

	if ((hres = LoadPNGFile(tempstrPathname)) != E_FAIL) return hres;

	SetExt(tempstrPathname, ".jpg");

	if ((hres = LoadJpegFileNoDecomp(tempstrPathname)) != E_FAIL) return hres;

	SetExt(tempstrPathname, ".jpeg");

	if ((hres = LoadJpegFileNoDecomp(tempstrPathname)) != E_FAIL) return hres;

	// Load bitmap files
	if (!lstrcmpi(strExtension, _T(".bmp")))
		return LoadBitmapFile(strPathname);

	// Load targa files
	if (!lstrcmpi(strExtension, _T(".tga")))
		return LoadTargaFile(strPathname);

	// Can add code here to check for other file formats before failing
	return DDERR_UNSUPPORTED;
}

//-----------------------------------------------------------------------------
// Name: LoadBitmapFile()
// Desc: Loads data from a .bmp file, and stores it in a bitmap structure.
//-----------------------------------------------------------------------------
HRESULT TextureContainer::LoadBitmapFile(TCHAR * strPathname)
{
	if (!PAK_FileExist(strPathname)) return E_FAIL;

	long siz = 0;
	unsigned char * dat = (unsigned char *)PAK_FileLoadMalloc(strPathname, &siz);

	if (!dat) return E_FAIL;

	BITMAPINFOHEADER sBitmapH = *((BITMAPINFOHEADER *)(dat + sizeof(BITMAPFILEHEADER)));
	HDC hHDC = CreateCompatibleDC(NULL);

	if (!hHDC)
	{
		free(dat);
		return DDERR_NOTFOUND;
	}

	char * mem = NULL;
	int iUsage;

	if (sBitmapH.biBitCount < 8)
	{
		free(dat);
		DeleteDC(hHDC);
		return DDERR_NOTFOUND;
	}

	if (sBitmapH.biBitCount == 8)
	{
		iUsage = DIB_PAL_COLORS;
	}
	else
	{
		iUsage = DIB_RGB_COLORS;
	}

	sBitmapH.biSize = sizeof(sBitmapH);
	m_hbmBitmap = CreateDIBSection(hHDC, (BITMAPINFO *)&sBitmapH, iUsage, (void **)&mem, NULL, 0);

	if (SelectObject(hHDC, m_hbmBitmap) == NULL)
	{
		DeleteDC(hHDC);
		return DDERR_NOTFOUND;
	}

	if (!mem)
	{
		free(dat);
		DeleteDC(hHDC);
		return DDERR_NOTFOUND;
	}

	if (!m_hbmBitmap)
	{
		DeleteDC(hHDC);
		free(dat);
		return DDERR_NOTFOUND;
	}


	BITMAPFILEHEADER bh;

	if (iUsage == DIB_PAL_COLORS)
	{
		RGBQUAD col[256];
		memcpy(col, dat + sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER), sizeof(RGBQUAD) * 256);
		SetDIBColorTable(hHDC,
		                 0,			// color table index of first entry
		                 256,		// number of color table entries
		                 col		// array of color table entries
		                );
	}

	bh = *((BITMAPFILEHEADER *)(dat));
	long size = sBitmapH.biSizeImage;

	if (size == 0) size = sBitmapH.biHeight * sBitmapH.biWidth * sBitmapH.biBitCount >> 3;

	memcpy(mem, dat + bh.bfOffBits, size);

	if (sBitmapH.biCompression != BI_RGB)
	{
		memset(mem, 64, size);
	}

	free(dat);
	DeleteDC(hHDC);
	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: LoadTargaFile()
// Desc: Loads RGBA data from a .tga file, and stores it in allocated memory
//       for the specified texture container
//-----------------------------------------------------------------------------
HRESULT TextureContainer::LoadTargaFile(TCHAR * strPathname)
{
	if (!PAK_FileExist(strPathname)) return E_FAIL;

	long siz = 0;
	unsigned char * dat = (unsigned char *)PAK_FileLoadMalloc(strPathname, &siz);

	if (NULL == dat)
		return E_FAIL;

	struct TargaHeader
	{
		BYTE IDLength;
		BYTE ColormapType;
		BYTE ImageType;
		BYTE ColormapSpecification[5];
		WORD XOrigin;
		WORD YOrigin;
		WORD ImageWidth;
		WORD ImageHeight;
		BYTE PixelDepth;
		BYTE ImageDescriptor;
	} tga;

	long pos = 0;
	memcpy(&tga, dat + pos, sizeof(TargaHeader));
	pos += sizeof(TargaHeader);

	// Only true color, non-mapped images are supported
	if ((0 != tga.ColormapType) ||
	        (tga.ImageType != 10 && tga.ImageType != 2))
	{
		free(dat);
		return E_FAIL;
	}

	// Skip the ID field. The first byte of the header is the length of this field
	if (tga.IDLength)
		pos += tga.IDLength;

	m_dwWidth   = tga.ImageWidth;
	m_dwHeight  = tga.ImageHeight;
	m_dwBPP     = tga.PixelDepth;
	m_pRGBAData = new DWORD[m_dwWidth*m_dwHeight];

	if (m_pRGBAData == NULL)
	{
		free(dat);
		return E_FAIL;
	}

	for (DWORD y = 0; y < m_dwHeight; y++)
	{
		DWORD dwOffset = y * m_dwWidth;

		if (0 == (tga.ImageDescriptor & 0x0010))
			dwOffset = (m_dwHeight - y - 1) * m_dwWidth;

		for (DWORD x = 0; x < m_dwWidth; x)
		{
			if (tga.ImageType == 10)
			{
				BYTE PacketInfo = dat[pos++];
				WORD PacketType = 0x80 & PacketInfo;
				WORD PixelCount = (0x007f & PacketInfo) + 1;

				if (PacketType)
				{
					DWORD b = dat[pos++];
					DWORD g = dat[pos++];
					DWORD r = dat[pos++];
					DWORD a = 0xff;

					if (m_dwBPP == 32)
						a = dat[pos++];

					while (PixelCount--)
					{
						m_pRGBAData[dwOffset+x] = (r << 24L) + (g << 16L) + (b << 8L) + (a);
						x++;
					}
				}
				else
				{
					while (PixelCount--)
					{
						BYTE b = dat[pos++];
						BYTE g = dat[pos++];
						BYTE r = dat[pos++];
						BYTE a = 0xff;

						if (m_dwBPP == 32)
							a = dat[pos++];

						m_pRGBAData[dwOffset+x] = (r << 24L) + (g << 16L) + (b << 8L) + (a);
						x++;
					}
				}
			}
			else
			{
				BYTE b = dat[pos++];
				BYTE g = dat[pos++];
				BYTE r = dat[pos++];
				BYTE a = 0xff;

				if (m_dwBPP == 32)
					a = dat[pos++];

				m_pRGBAData[dwOffset+x] = (r << 24L) + (g << 16L) + (b << 8L) + (a);
				x++;
			}
		}
	}

	// Check for alpha content
	for (DWORD i = 0; i < (m_dwWidth * m_dwHeight); i++)
	{

		if ((m_pRGBAData[i] & 0x000000ff) != 0xff)
		{
			m_bHasAlpha = TRUE;
			break;
		}
	}

	free(dat);
	return S_OK;
}


void CopySurfaceToBumpMap(LPDIRECTDRAWSURFACE7 sSurface, LPDIRECTDRAWSURFACE7 dSurface)
{
	DDSURFACEDESC2 ddesc, ddesc2;
	ddesc.dwSize  = sizeof(ddesc);
	ddesc2.dwSize = sizeof(ddesc2);
	sSurface->Lock(NULL, &ddesc , DDLOCK_WAIT, NULL);
	dSurface->Lock(NULL, &ddesc2, DDLOCK_WAIT, NULL);

	if ((32 != ddesc.ddpfPixelFormat.dwRGBBitCount)
	        &&	(16 != ddesc.ddpfPixelFormat.dwRGBBitCount)
	        ||
	        (32 != ddesc2.ddpfPixelFormat.dwRGBBitCount)
	        &&	(16 != ddesc2.ddpfPixelFormat.dwRGBBitCount))
	{
		dSurface->Unlock(NULL);
		sSurface->Unlock(NULL);
	}

	BYTE * sBytes = (BYTE *)ddesc.lpSurface;
	BYTE * dBytes = (BYTE *)ddesc2.lpSurface;

	DWORD dwRMask = ddesc.ddpfPixelFormat.dwRBitMask;
	DWORD dwGMask = ddesc.ddpfPixelFormat.dwGBitMask;
	DWORD dwBMask = ddesc.ddpfPixelFormat.dwBBitMask;
	DWORD dwAMask = ddesc.ddpfPixelFormat.dwRGBAlphaBitMask;

	DWORD dwRShiftL = 8, dwRShiftR = 0;
	DWORD dwGShiftL = 8, dwGShiftR = 0;
	DWORD dwBShiftL = 8, dwBShiftR = 0;
	DWORD dwAShiftL = 8, dwAShiftR = 0;

	DWORD dwMask;

	for (dwMask = dwRMask ; dwMask && !(dwMask & 0x1) ; dwMask >>= 1) dwRShiftR++;

	for (; dwMask ; dwMask >>= 1) dwRShiftL--;

	for (dwMask = dwGMask ; dwMask && !(dwMask & 0x1) ; dwMask >>= 1) dwGShiftR++;

	for (; dwMask ; dwMask >>= 1) dwGShiftL--;

	for (dwMask = dwBMask ; dwMask && !(dwMask & 0x1) ; dwMask >>= 1) dwBShiftR++;

	for (; dwMask ; dwMask >>= 1) dwBShiftL--;

	for (dwMask = dwAMask ; dwMask && !(dwMask & 0x1) ; dwMask >>= 1) dwAShiftR++;

	for (; dwMask ; dwMask >>= 1) dwAShiftL--;


	DWORD dwRMask2 = ddesc2.ddpfPixelFormat.dwRBitMask;
	DWORD dwGMask2 = ddesc2.ddpfPixelFormat.dwGBitMask;
	DWORD dwBMask2 = ddesc2.ddpfPixelFormat.dwBBitMask;
	DWORD dwAMask2 = ddesc2.ddpfPixelFormat.dwRGBAlphaBitMask;

	DWORD dwRShiftL2 = 8, dwRShiftR2 = 0;
	DWORD dwGShiftL2 = 8, dwGShiftR2 = 0;
	DWORD dwBShiftL2 = 8, dwBShiftR2 = 0;
	DWORD dwAShiftL2 = 8, dwAShiftR2 = 0;

	DWORD dwMask2;

	for (dwMask2 = dwRMask2 ; dwMask2 && !(dwMask2 & 0x1) ; dwMask2 >>= 1) dwRShiftR2++;

	for (; dwMask2 ; dwMask2 >>= 1) dwRShiftL2--;

	for (dwMask2 = dwGMask2 ; dwMask2 && !(dwMask2 & 0x1) ; dwMask2 >>= 1) dwGShiftR2++;

	for (; dwMask2 ; dwMask2 >>= 1) dwGShiftL2--;

	for (dwMask2 = dwBMask2 ; dwMask2 && !(dwMask2 & 0x1) ; dwMask2 >>= 1) dwBShiftR2++;

	for (; dwMask2 ; dwMask2 >>= 1) dwBShiftL2--;

	for (dwMask2 = dwAMask2 ; dwMask2 && !(dwMask2 & 0x1) ; dwMask2 >>= 1) dwAShiftR2++;

	for (; dwMask2 ; dwMask2 >>= 1) dwAShiftL2--;

	long LineOffset = ddesc.dwWidth;
	DWORD dwPixel;
	long rr, gg, bb;
	long posx, posy;
	DWORD * pDstData32;
	WORD * pDstData16;
	DWORD * pSrcData32;
	WORD * pSrcData16;
	
	DWORD dr, dg, db, da;

	for (ULONG y = 0 ; y < ddesc2.dwHeight ; y++)
	{
		pDstData32 = (DWORD *)dBytes;
		pDstData16 = (WORD *)dBytes;
		pSrcData32 = (DWORD *)sBytes;
		pSrcData16 = (WORD *)sBytes;

		for (ULONG x = 0 ; x < ddesc2.dwWidth ; x++)
		{		
			ARX_CHECK_LONG(x);
			ARX_CHECK_LONG(y  * LineOffset);
			posx = x;
			posy = y * LineOffset;

			// Original Pixel
			if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
				dwPixel = pSrcData32[posx+posy];
			else dwPixel = pSrcData16[posx+posy];

			rr = (BYTE)(((dwPixel & dwRMask2) >> dwRShiftR2) << dwRShiftL2);
			gg = (BYTE)(((dwPixel & dwGMask2) >> dwGShiftR2) << dwGShiftL2);
			bb = (BYTE)(((dwPixel & dwBMask2) >> dwBShiftR2) << dwBShiftL2);

			long val = ARX_CLEAN_WARN_CAST_LONG((rr + gg + bb) * DIV6);
			rr = gg = bb = val;

			dr = ((rr >> (dwRShiftL)) << dwRShiftR) & dwRMask;
			dg = ((gg >> (dwGShiftL)) << dwGShiftR) & dwGMask;
			db = ((bb >> (dwBShiftL)) << dwBShiftR) & dwBMask;
			da = ((255 >> (dwAShiftL)) << dwAShiftR) & dwAMask;

			if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
				pDstData32[x] = (DWORD)(dr + dg + db + da);
			else pDstData16[x] = (WORD)(dr + dg + db + da);
		}

		dBytes += ddesc2.lPitch;
	}

	dSurface->Unlock(NULL);
	sSurface->Unlock(NULL);
}

bool IsColorKeyInSurface(LPDIRECTDRAWSURFACE7 _pSurface)
{
	DDSURFACEDESC2 ddesc;
	ddesc.dwSize = sizeof(ddesc);
	_pSurface->Lock(NULL, &ddesc, DDLOCK_WAIT, NULL);

	if ((32 != ddesc.ddpfPixelFormat.dwRGBBitCount)
	        &&	(16 != ddesc.ddpfPixelFormat.dwRGBBitCount))
	{
		_pSurface->Unlock(NULL);
	}

	BYTE * sBytes = (BYTE *)ddesc.lpSurface;

	DWORD dwRMask = ddesc.ddpfPixelFormat.dwRBitMask;
	DWORD dwGMask = ddesc.ddpfPixelFormat.dwGBitMask;
	DWORD dwBMask = ddesc.ddpfPixelFormat.dwBBitMask;
	DWORD dwAMask = ddesc.ddpfPixelFormat.dwRGBAlphaBitMask;

	DWORD dwRShiftL = 8, dwRShiftR = 0;
	DWORD dwGShiftL = 8, dwGShiftR = 0;
	DWORD dwBShiftL = 8, dwBShiftR = 0;
	DWORD dwAShiftL = 8, dwAShiftR = 0;

	DWORD dwMask;

	for (dwMask = dwRMask ; dwMask && !(dwMask & 0x1) ; dwMask >>= 1) dwRShiftR++;

	for (; dwMask ; dwMask >>= 1) dwRShiftL--;

	for (dwMask = dwGMask ; dwMask && !(dwMask & 0x1) ; dwMask >>= 1) dwGShiftR++;

	for (; dwMask ; dwMask >>= 1) dwGShiftL--;

	for (dwMask = dwBMask ; dwMask && !(dwMask & 0x1) ; dwMask >>= 1) dwBShiftR++;

	for (; dwMask ; dwMask >>= 1) dwBShiftL--;

	for (dwMask = dwAMask ; dwMask && !(dwMask & 0x1) ; dwMask >>= 1) dwAShiftR++;

	for (; dwMask ; dwMask >>= 1) dwAShiftL--;

	long LineOffset = ddesc.dwWidth;
	DWORD dwPixel;
	long aa, rr, gg, bb;
	long posx, posy;
	DWORD * pSrcData32;
	WORD * pSrcData16;

	pSrcData32 = (DWORD *)sBytes;
	pSrcData16 = (WORD *) sBytes;
 
 

	for (ULONG y = 0 ; y < ddesc.dwHeight ; y++)
	{
		for (ULONG x = 0 ; x < ddesc.dwWidth ; x++)
		{
			ARX_CHECK_LONG(x);
			ARX_CHECK_LONG(y * LineOffset);
			posx = ARX_CLEAN_WARN_CAST_LONG(x);
			posy = ARX_CLEAN_WARN_CAST_LONG(y * LineOffset);

			// Original Pixel
			if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
				dwPixel = pSrcData32[posx+posy];
			else dwPixel = pSrcData16[posx+posy];

			aa = rr = gg = bb = 0;

			rr = (BYTE)(((dwPixel & dwRMask) >> dwRShiftR) << dwRShiftL);
			gg = (BYTE)(((dwPixel & dwGMask) >> dwGShiftR) << dwGShiftL);      
			bb = (BYTE)(((dwPixel & dwBMask) >> dwBShiftR) << dwBShiftL);      
			aa = (BYTE)(((dwPixel & dwAMask) >> dwAShiftR) << dwAShiftL);

			if ((rr == 0) &&
			        (gg == 0) &&
			        (bb == 0))
			{
				_pSurface->Unlock(NULL);
				return true;
			}
		}
	}

	_pSurface->Unlock(NULL);
	return false;
}


void StretchCopySurfaceToSurface(LPDIRECTDRAWSURFACE7 sSurface, LPDIRECTDRAWSURFACE7 dSurface)
{
	DDSURFACEDESC2 ddesc, ddesc2;
	ddesc.dwSize  = sizeof(ddesc);
	ddesc2.dwSize = sizeof(ddesc2);
	sSurface->Lock(NULL, &ddesc , DDLOCK_WAIT, NULL);
	dSurface->Lock(NULL, &ddesc2, DDLOCK_WAIT, NULL);

	if ((32 != ddesc.ddpfPixelFormat.dwRGBBitCount)
	        && (16 != ddesc.ddpfPixelFormat.dwRGBBitCount)
	        ||
	        (32 != ddesc2.ddpfPixelFormat.dwRGBBitCount)
	        && (16 != ddesc2.ddpfPixelFormat.dwRGBBitCount))
	{
		dSurface->Unlock(NULL);
		sSurface->Unlock(NULL);
		return;
	}

	BYTE * sBytes = (BYTE *)ddesc.lpSurface;
	BYTE * dBytes = (BYTE *)ddesc2.lpSurface;

	float rx = (float)ddesc.dwWidth / (float)ddesc2.dwWidth ;
	float ry = (float)ddesc.dwHeight / (float)ddesc2.dwHeight ;

	DWORD dwRMask = ddesc.ddpfPixelFormat.dwRBitMask;
	DWORD dwGMask = ddesc.ddpfPixelFormat.dwGBitMask;
	DWORD dwBMask = ddesc.ddpfPixelFormat.dwBBitMask;
	DWORD dwAMask = ddesc.ddpfPixelFormat.dwRGBAlphaBitMask;

	DWORD dwRShiftL = 8, dwRShiftR = 0;
	DWORD dwGShiftL = 8, dwGShiftR = 0;
	DWORD dwBShiftL = 8, dwBShiftR = 0;
	DWORD dwAShiftL = 8, dwAShiftR = 0;

	DWORD dwMask;

	for (dwMask = dwRMask ; dwMask && !(dwMask & 0x1) ; dwMask >>= 1) dwRShiftR++;

	for (; dwMask ; dwMask >>= 1) dwRShiftL--;

	for (dwMask = dwGMask ; dwMask && !(dwMask & 0x1) ; dwMask >>= 1) dwGShiftR++;

	for (; dwMask ; dwMask >>= 1) dwGShiftL--;

	for (dwMask = dwBMask ; dwMask && !(dwMask & 0x1) ; dwMask >>= 1) dwBShiftR++;

	for (; dwMask ; dwMask >>= 1) dwBShiftL--;

	for (dwMask = dwAMask ; dwMask && !(dwMask & 0x1) ; dwMask >>= 1) dwAShiftR++;

	for (; dwMask ; dwMask >>= 1) dwAShiftL--;


	DWORD dwRMask2 = ddesc2.ddpfPixelFormat.dwRBitMask;
	DWORD dwGMask2 = ddesc2.ddpfPixelFormat.dwGBitMask;
	DWORD dwBMask2 = ddesc2.ddpfPixelFormat.dwBBitMask;
	DWORD dwAMask2 = ddesc2.ddpfPixelFormat.dwRGBAlphaBitMask;

	DWORD dwRShiftL2 = 8, dwRShiftR2 = 0;
	DWORD dwGShiftL2 = 8, dwGShiftR2 = 0;
	DWORD dwBShiftL2 = 8, dwBShiftR2 = 0;
	DWORD dwAShiftL2 = 8, dwAShiftR2 = 0;

	DWORD dwMask2;

	for (dwMask2 = dwRMask2 ; dwMask2 && !(dwMask2 & 0x1) ; dwMask2 >>= 1) dwRShiftR2++;

	for (; dwMask2 ; dwMask2 >>= 1) dwRShiftL2--;

	for (dwMask2 = dwGMask2 ; dwMask2 && !(dwMask2 & 0x1) ; dwMask2 >>= 1) dwGShiftR2++;

	for (; dwMask2 ; dwMask2 >>= 1) dwGShiftL2--;

	for (dwMask2 = dwBMask2 ; dwMask2 && !(dwMask2 & 0x1) ; dwMask2 >>= 1) dwBShiftR2++;

	for (; dwMask2 ; dwMask2 >>= 1) dwBShiftL2--;

	for (dwMask2 = dwAMask2 ; dwMask2 && !(dwMask2 & 0x1) ; dwMask2 >>= 1) dwAShiftR2++;

	for (; dwMask2 ; dwMask2 >>= 1) dwAShiftL2--;

	long LineOffset = ddesc.dwWidth;
	DWORD dwPixel[10];
	BYTE r[10];
	BYTE g[10];
	BYTE b[10];
	BYTE a[10];
	long rr, gg, bb;
	long posx, posy, offset;
	DWORD * pDstData32;
	WORD * pDstData16;
	DWORD * pSrcData32;
	WORD * pSrcData16;

	DWORD dr, dg, db, da;

	for (ULONG y = 0 ; y < ddesc2.dwHeight ; y++)
	{
		pDstData32 = (DWORD *)dBytes;
		pDstData16 = (WORD *)dBytes;
		pSrcData32 = (DWORD *)sBytes;
		pSrcData16 = (WORD *)sBytes;

		for (ULONG x = 0 ; x < ddesc2.dwWidth ; x++)
		{
			F2L((float)(x * rx), &posx);
			F2L((float)(y * ry) * LineOffset, &posy);

			// Pixel Up Left
			if ((y <= 0.f) || (x <= 0.f)) offset = posx + posy;
			else offset = posx + posy - LineOffset - 1;

			if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
				dwPixel[6] = pSrcData32[offset];
			else dwPixel[6] = pSrcData16[offset];

			// Pixel Up
			if (y <= 0) offset = posx + posy;
			else offset = posx + posy - LineOffset;

			if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
				dwPixel[5] = pSrcData32[offset];
			else dwPixel[5] = pSrcData16[offset];

			// Pixel Up Right
			if ((y <= 0.f) || (x >= ddesc2.dwWidth - 1)) offset = posx + posy;
			else offset = posx + posy - LineOffset + 1;

			if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
				dwPixel[8] = pSrcData32[offset];
			else dwPixel[8] = pSrcData16[offset];

			// Pixel Left
			if (x <= 0) offset = posx + posy;
			else offset = posx + posy - 1;

			if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
				dwPixel[4] = pSrcData32[offset];
			else dwPixel[4] = pSrcData16[offset];

			// Original Pixel
			if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
				dwPixel[0] = pSrcData32[posx+posy];
			else dwPixel[0] = pSrcData16[posx+posy];

			// Pixel Right
			if (x >= ddesc2.dwWidth - 1) offset = posx + posy;
			else offset = posx + posy + 1;

			if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
				dwPixel[1] = pSrcData32[offset];
			else dwPixel[1] = pSrcData16[offset];

			// Pixel Down Left
			if ((x <= 0) || (y >= ddesc2.dwHeight - 1)) offset = posx + posy;
			else offset = posx + posy + LineOffset - 1;

			if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
				dwPixel[7] = pSrcData32[offset];
			else dwPixel[7] = pSrcData16[offset];

			// Pixel Down
			if (y >= ddesc2.dwHeight - 1) offset = posx + posy;
			else offset = posx + posy + LineOffset;

			if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
				dwPixel[2] = pSrcData32[offset];
			else dwPixel[2] = pSrcData16[offset];

			// Pixel Down Right
			if ((x >= ddesc2.dwWidth - 1) || (y >= ddesc2.dwHeight - 1)) offset = posx + posy;
			else offset = posx + posy + LineOffset + 1;

			if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
				dwPixel[3] = pSrcData32[offset];
			else dwPixel[3] = pSrcData16[offset];

			rr = gg = bb = 0;
			long aa = 0;
 
			for (long n = 0 ; n < 9 ; n++)
			{
				r[n] = (BYTE)(((dwPixel[n] & dwRMask2) >> dwRShiftR2) << dwRShiftL2);
				g[n] = (BYTE)(((dwPixel[n] & dwGMask2) >> dwGShiftR2) << dwGShiftL2);      
				b[n] = (BYTE)(((dwPixel[n] & dwBMask2) >> dwBShiftR2) << dwBShiftL2);     
				a[n] = (BYTE)(((dwPixel[n] & dwAMask2) >> dwAShiftR2) << dwAShiftL2);

				rr += r[n];
				gg += g[n];
				bb += b[n];
				aa += a[n];
			}

			rr += r[4] + r[5] + r[0] * 3 + r[2] + r[1];
			gg += g[4] + g[5] + g[0] * 3 + g[2] + g[1];
			bb += b[4] + b[5] + b[0] * 3 + b[2] + b[1];
			aa += a[4] + a[5] + a[0] * 3 + a[2] + a[1];
			rr >>= 4;
			gg >>= 4;
			bb >>= 4;
			aa >>= 4;

			dr = ((rr >> (dwRShiftL)) << dwRShiftR) & dwRMask;
			dg = ((gg >> (dwGShiftL)) << dwGShiftR) & dwGMask;
			db = ((bb >> (dwBShiftL)) << dwBShiftR) & dwBMask;
			da = ((255 >> (dwAShiftL)) << dwAShiftR) & dwAMask;

			if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
				pDstData32[x] = (DWORD)(dr + dg + db + da);
			else pDstData16[x] = (WORD)(dr + dg + db + da);
		}

		dBytes += ddesc2.lPitch;
	}

	dSurface->Unlock(NULL);
	sSurface->Unlock(NULL);
}

long SPECIAL_PNUX = 0; 

void PnuxSurface(LPDIRECTDRAWSURFACE7 sSurface)
{
	DDSURFACEDESC2 ddesc;
	ddesc.dwSize = sizeof(ddesc);
	sSurface->Lock(NULL, &ddesc, DDLOCK_WAIT, NULL);

	BYTE * sBytes = (BYTE *)ddesc.lpSurface;

	DWORD dwRMask = ddesc.ddpfPixelFormat.dwRBitMask;
	DWORD dwGMask = ddesc.ddpfPixelFormat.dwGBitMask;
	DWORD dwBMask = ddesc.ddpfPixelFormat.dwBBitMask;
	DWORD dwAMask = ddesc.ddpfPixelFormat.dwRGBAlphaBitMask;

	DWORD dwRShiftL = 8, dwRShiftR = 0;
	DWORD dwGShiftL = 8, dwGShiftR = 0;
	DWORD dwBShiftL = 8, dwBShiftR = 0;
	DWORD dwAShiftL = 8, dwAShiftR = 0;

	DWORD dwMask;

	for (dwMask = dwRMask ; dwMask && !(dwMask & 0x1) ; dwMask >>= 1) dwRShiftR++;

	for (; dwMask ; dwMask >>= 1) dwRShiftL--;

	for (dwMask = dwGMask ; dwMask && !(dwMask & 0x1) ; dwMask >>= 1) dwGShiftR++;

	for (; dwMask ; dwMask >>= 1) dwGShiftL--;

	for (dwMask = dwBMask ; dwMask && !(dwMask & 0x1) ; dwMask >>= 1) dwBShiftR++;

	for (; dwMask ; dwMask >>= 1) dwBShiftL--;

	for (dwMask = dwAMask ; dwMask && !(dwMask & 0x1) ; dwMask >>= 1) dwAShiftR++;

	for (; dwMask ; dwMask >>= 1) dwAShiftL--;


	long LineOffset = ddesc.dwWidth;
	DWORD dwPixel;
	BYTE r;
	BYTE g;
	BYTE b;
	long posx, posy, offset;
	long aa, rr, gg, bb;
	DWORD * pSrcData32;
	WORD * pSrcData16;

	DWORD dr, dg, db, da;
	pSrcData32 = (DWORD *)sBytes;
	pSrcData16 = (WORD *)sBytes;

	for (ULONG y = 0 ; y < ddesc.dwHeight ; y++)
	{

		for (ULONG x = 0 ; x < ddesc.dwWidth ; x++)
		{
			posx = x;
			posy = y * LineOffset;

			// Original Pixel
			offset = posx + posy;

			if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
				dwPixel = pSrcData32[offset];
			else dwPixel = pSrcData16[offset];

			r = (BYTE)(((dwPixel & dwRMask) >> dwRShiftR) << dwRShiftL);
			g = (BYTE)(((dwPixel & dwGMask) >> dwGShiftR) << dwGShiftL);       
			b = (BYTE)(((dwPixel & dwBMask) >> dwBShiftR) << dwBShiftL);      
			aa = 255;
			float fr, fg, fb;

			rr = r;
			gg = g;
			bb = b;

			fr = ARX_CLEAN_WARN_CAST_FLOAT(rr);
			fg = ARX_CLEAN_WARN_CAST_FLOAT(gg);
			fb = ARX_CLEAN_WARN_CAST_FLOAT(bb);

			if (SPECIAL_PNUX == 3)
			{
				float power = (fr + fg + fb) * DIV3 * 1.2f;
				fr = power;
				fg = power;
				fb = power;
			}
			else if (SPECIAL_PNUX == 2)
			{
				float power = (fr + fg + fb) * DIV3;

				if (power > fr * 0.75f)
				{
					fg = fr * 0.6f;
					fb = fr * 0.5f;
				}
				else
				{
					fg = fr * 0.3f;
					fb = fr * 0.1f;
				}

				fr *= 1.3f;
				fg *= 1.5f;
				fb *= 1.5f;

				if (power > 200.f)
				{
					fr += (power - 200.f) * DIV5;
					fg += (power - 200.f) * DIV4;
					fb += (power - 200.f) * DIV3;
				}
			}
			else
			{
				float power = (fr + fg + fb) * 0.6f;

				if (power > 190.f)
				{
					fr = power;
					fg = power * 0.3f;
					fb = power * 0.7f;
				}
				else
				{
					fr = power * 0.7f;
					fg = power * 0.5f;
					fb = power;
				}

			}

			if (fr > 255.f) fr = 255.f;

			if (fg > 255.f) fg = 255.f;

			if (fb > 255.f) fb = 255.f;

			F2L(fr, &rr);
			F2L(fg, &gg);
			F2L(fb, &bb);
			dr = ((rr >> (dwRShiftL)) << dwRShiftR) & dwRMask;
			dg = ((gg >> (dwGShiftL)) << dwGShiftR) & dwGMask;
			db = ((bb >> (dwBShiftL)) << dwBShiftR) & dwBMask;
			da = ((aa >> (dwAShiftL)) << dwAShiftR) & dwAMask;
			offset = posx + posy;

			if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
				pSrcData32[offset] = (DWORD)(dr + dg + db + da);
			else pSrcData16[offset] = (WORD)(dr + dg + db + da);

		}
	}

	sSurface->Unlock(NULL);
}

void SmoothSurface(LPDIRECTDRAWSURFACE7 sSurface)
{
	DDSURFACEDESC2 ddesc;
	ddesc.dwSize = sizeof(ddesc);
	sSurface->Lock(NULL, &ddesc, DDLOCK_WAIT, NULL);

	BYTE * sBytes = (BYTE *)ddesc.lpSurface;

	DWORD dwRMask = ddesc.ddpfPixelFormat.dwRBitMask;
	DWORD dwGMask = ddesc.ddpfPixelFormat.dwGBitMask;
	DWORD dwBMask = ddesc.ddpfPixelFormat.dwBBitMask;
	DWORD dwAMask = ddesc.ddpfPixelFormat.dwRGBAlphaBitMask;

	DWORD dwRShiftL = 8, dwRShiftR = 0;
	DWORD dwGShiftL = 8, dwGShiftR = 0;
	DWORD dwBShiftL = 8, dwBShiftR = 0;
	DWORD dwAShiftL = 8, dwAShiftR = 0;

	DWORD dwMask;

	for (dwMask = dwRMask ; dwMask && !(dwMask & 0x1) ; dwMask >>= 1) dwRShiftR++;

	for (; dwMask ; dwMask >>= 1) dwRShiftL--;

	for (dwMask = dwGMask ; dwMask && !(dwMask & 0x1) ; dwMask >>= 1) dwGShiftR++;

	for (; dwMask ; dwMask >>= 1) dwGShiftL--;

	for (dwMask = dwBMask ; dwMask && !(dwMask & 0x1) ; dwMask >>= 1) dwBShiftR++;

	for (; dwMask ; dwMask >>= 1) dwBShiftL--;

	for (dwMask = dwAMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwAShiftR++;

	for (; dwMask; dwMask >>= 1) dwAShiftL--;


	long LineOffset = ddesc.dwWidth;
	DWORD dwPixel[10];
	BYTE r[10];
	BYTE g[10];
	BYTE b[10];
	long posx, posy, offset;
	long rr, gg, bb;
	DWORD * pSrcData32;
	WORD * pSrcData16;

	DWORD dr, dg, db, da;
	pSrcData32 = (DWORD *)sBytes;
	pSrcData16 = (WORD *)sBytes;

	for (ULONG y = 0 ; y < ddesc.dwHeight ; y++)
	{

		for (ULONG x = 0 ; x < ddesc.dwWidth ; x++)
		{
			posx = x;
			posy = y * LineOffset;

			// Pixel Up Left
			if ((y <= 0.f) || (x <= 0.f)) offset = posx + posy;
			else offset = posx + posy - LineOffset - 1;

			if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
				dwPixel[6] = pSrcData32[offset];
			else dwPixel[6] = pSrcData16[offset];

			// Pixel Up
			if (y <= 0) offset = posx + posy;
			else offset = posx + posy - LineOffset;

			if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
				dwPixel[5] = pSrcData32[offset];
			else dwPixel[5] = pSrcData16[offset];

			// Pixel Up Right
			if ((y <= 0.f) || (x >= ddesc.dwWidth - 1)) offset = posx + posy;
			else offset = posx + posy - LineOffset + 1;

			if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
				dwPixel[8] = pSrcData32[offset];
			else dwPixel[8] = pSrcData16[offset];

			// Pixel Left
			if (x <= 0) offset = posx + posy;
			else offset = posx + posy - 1;

			if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
				dwPixel[4] = pSrcData32[offset];
			else dwPixel[4] = pSrcData16[offset];

			// Original Pixel
			offset = posx + posy;

			if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
				dwPixel[0] = pSrcData32[offset];
			else dwPixel[0] = pSrcData16[offset];

			// Pixel Right
			if (x >= ddesc.dwWidth - 1) offset = posx + posy;
			else offset = posx + posy + 1;

			if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
				dwPixel[1] = pSrcData32[offset];
			else dwPixel[1] = pSrcData16[offset];

			// Pixel Down Left
			if ((x <= 0) || (y >= ddesc.dwHeight - 1)) offset = posx + posy;
			else offset = posx + posy + LineOffset - 1;

			if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
				dwPixel[7] = pSrcData32[offset];
			else dwPixel[7] = pSrcData16[offset];

			// Pixel Down
			if (y >= ddesc.dwHeight - 1) offset = posx + posy;
			else offset = posx + posy + LineOffset;

			if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
				dwPixel[2] = pSrcData32[offset];
			else dwPixel[2] = pSrcData16[offset];

			// Pixel Down Right
			if ((x >= ddesc.dwWidth - 1) || (y >= ddesc.dwHeight - 1)) offset = posx + posy;
			else offset = posx + posy + LineOffset + 1;

			if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
				dwPixel[3] = pSrcData32[offset];
			else dwPixel[3] = pSrcData16[offset];

			rr = gg = bb = 0;
			long nbincrust = 0;

			for (long n = 0 ; n < 9 ; n++)
			{
				r[n] = (BYTE)(((dwPixel[n] & dwRMask) >> dwRShiftR) << dwRShiftL);
				g[n] = (BYTE)(((dwPixel[n] & dwGMask) >> dwGShiftR) << dwGShiftL);      
				b[n] = (BYTE)(((dwPixel[n] & dwBMask) >> dwBShiftR) << dwBShiftL);      

				rr += r[n];
				gg += g[n];
				bb += b[n];

				if ((r[n] == 0) && (g[n] == 0) && (b[n] == 0))
				{
					nbincrust++;
				}

				
			}
			
			rr += r[0] * 7;
			gg += g[0] * 7;
			bb += b[0] * 7; 

			rr >>= 4;
			gg >>= 4;
			bb >>= 4;
	
			long aa = 255 - nbincrust * 28;

			if (aa < 30) aa = 0;

			dr = ((rr >> (dwRShiftL)) << dwRShiftR) & dwRMask;
			dg = ((gg >> (dwGShiftL)) << dwGShiftR) & dwGMask;
			db = ((bb >> (dwBShiftL)) << dwBShiftR) & dwBMask;
			da = ((aa >> (dwAShiftL)) << dwAShiftR) & dwAMask;
			offset = posx + posy;

			if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
				pSrcData32[offset] = (DWORD)(dr + dg + db + da);
			else pSrcData16[offset] = (WORD)(dr + dg + db + da);
			
		}
	}

	sSurface->Unlock(NULL);
}

void SpecialBorderSurface(TextureContainer * tc, unsigned long x0, unsigned long y0)
{
	if (!tc->m_pddsSurface)
		return;

	LPDIRECTDRAWSURFACE7 sSurface = tc->m_pddsSurface;
	tc->m_dwFlags |= D3DTEXTR_FAKE_BORDER;
	DDSURFACEDESC2 ddesc;
	memset(&ddesc, 0, sizeof(ddesc));
	ddesc.dwSize = sizeof(ddesc);

	if (FAILED(sSurface->Lock(NULL, &ddesc, DDLOCK_WAIT, NULL))) return;

	BYTE * sBytes = (BYTE *)ddesc.lpSurface;

	DWORD dwRMask = ddesc.ddpfPixelFormat.dwRBitMask;
	DWORD dwGMask = ddesc.ddpfPixelFormat.dwGBitMask;
	DWORD dwBMask = ddesc.ddpfPixelFormat.dwBBitMask;
	DWORD dwAMask = ddesc.ddpfPixelFormat.dwRGBAlphaBitMask;

	DWORD dwRShiftL = 8, dwRShiftR = 0;
	DWORD dwGShiftL = 8, dwGShiftR = 0;
	DWORD dwBShiftL = 8, dwBShiftR = 0;
	DWORD dwAShiftL = 8, dwAShiftR = 0;

	DWORD dwMask;

	for (dwMask = dwRMask ; dwMask && !(dwMask & 0x1) ; dwMask >>= 1) dwRShiftR++;

	for (; dwMask ; dwMask >>= 1) dwRShiftL--;

	for (dwMask = dwGMask ; dwMask && !(dwMask & 0x1) ; dwMask >>= 1) dwGShiftR++;

	for (; dwMask ; dwMask >>= 1) dwGShiftL--;

	for (dwMask = dwBMask ; dwMask && !(dwMask & 0x1) ; dwMask >>= 1) dwBShiftR++;

	for (; dwMask ; dwMask >>= 1) dwBShiftL--;

	for (dwMask = dwAMask ; dwMask && !(dwMask & 0x1) ; dwMask >>= 1) dwAShiftR++;

	for (; dwMask ; dwMask >>= 1) dwAShiftL--;


	long LineOffset = ddesc.dwWidth;
	DWORD dwPixel;
	long posx, posy, offset;
	DWORD * pSrcData32;
	WORD * pSrcData16;

	pSrcData32 = (DWORD *)sBytes;
	pSrcData16 = (WORD *)sBytes;

	if (ddesc.dwHeight > y0)
	{
		for (ULONG x = 0 ; x < ddesc.dwWidth ; x++)
		{
			posx = x;
			posy = y0 * LineOffset;

			// Pixel Up
			if (y0 <= 0) offset = posx + posy;
			else offset = posx + posy - LineOffset;

			if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
				dwPixel = (DWORD)pSrcData32[offset];
			else dwPixel = (WORD)pSrcData16[offset];

			for (ULONG y = y0 ; y < ddesc.dwHeight ; y++)
			{
				posy = y * LineOffset;
				offset = posx + posy;

				if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
					pSrcData32[offset] = (DWORD)(dwPixel);
				else pSrcData16[offset] = (WORD)(dwPixel);
			}
		}
	}

	if (ddesc.dwWidth > x0)
	{
		for (ULONG y = 0 ; y < ddesc.dwHeight ; y++)
		{
			posx = x0 - 1;
			posy = y * LineOffset;

			// Pixel Up
			offset = posx + posy;

			if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
				dwPixel = pSrcData32[offset];
			else dwPixel = pSrcData16[offset];

			for (ULONG x = x0 ; x < ddesc.dwWidth ; x++)
			{
				posx = x;
				offset = posx + posy;

				if (32 == ddesc.ddpfPixelFormat.dwRGBBitCount)
					pSrcData32[offset] = (DWORD)(dwPixel);
				else pSrcData16[offset] = (WORD)(dwPixel);
			}
		}
	}

	sSurface->Unlock(NULL);
}

long EERIE_USES_BUMP_MAP = 0;
void EERIE_ActivateBump(void)
{
	EERIE_USES_BUMP_MAP = 0;		//pas de bump

	if (g_pD3DApp)
	{
		EERIE_USES_BUMP_MAP = 1;	//defaut: bump 2pass

		if (g_pD3DApp->m_pDeviceInfo->wNbBlendStage > 2 && g_pD3DApp->m_pDeviceInfo->wNbTextureSimultaneous > 1)
		{
			//bump 1pass
			if (g_pD3DApp->m_pDeviceInfo->dwTextureOpCaps & D3DTEXOPCAPS_DOTPRODUCT3)
			{
				//dot
				EERIE_USES_BUMP_MAP = 2;
			}
			else
			{
				if (g_pD3DApp->m_pDeviceInfo->dwTextureOpCaps & D3DTEXOPCAPS_ADDSIGNED)
				{
					//addsigned
					EERIE_USES_BUMP_MAP = 3;
				}
			}
		}
	}
}

void EERIE_DesactivateBump(void)
{
	EERIE_USES_BUMP_MAP = 0;
}
HRESULT TextureContainer::Use()
{
	this->locks++;
	return S_OK;
}

extern long GLOBAL_FORCE_MINI_TEXTURE;
extern long GORE_MODE;
//-----------------------------------------------------------------------------
// Name: Restore()
// Desc: Rebuilds the texture surface using the new device.
//-----------------------------------------------------------------------------
HRESULT TextureContainer::Restore(LPDIRECT3DDEVICE7 pd3dDevice)
{
	if (!pd3dDevice) return E_FAIL;

	bGlobalTextureStretch = true;
	bool bActivateBump = true;
	char tTxt[256];
	strcpy(tTxt, (const char *)m_strName);
	strupr(tTxt);

	//TEXTURE_STRETCH
	if (strstr(tTxt, "INTERFACE") ||
	        strstr(tTxt, "LEVELS") ||
	        strstr(tTxt, "ITEMS") ||
	        strstr(tTxt, "REFINEMENT") ||
	        strstr(tTxt, "LOGO.BMP"))
	{
		bGlobalTextureStretch = false;
		this->m_dwFlags |= D3DTEXTR_NO_MIPMAP;
	}

	//END_TEXTURE_STRETCH

	//BUMP
	if (strstr(tTxt, "ITEMS") ||
	        strstr(tTxt, "INTERFACE") ||
	        strstr(tTxt, "REFINEMENT") ||
	        strstr(tTxt, "LOGO.BMP"))
	{
		bActivateBump = false;
	}

	//END_BUMP

	LPDIRECTDRAWSURFACE7 t_m_pddsSurface;
	// Release any previously created objects

	SAFE_RELEASE(m_pddsSurface);
	SAFE_RELEASE(m_pddsBumpMap);

	// Check params
	if (NULL == pd3dDevice)
		return DDERR_INVALIDPARAMS;

	// Get the device caps
	D3DDEVICEDESC7 ddDesc;

	if (FAILED(pd3dDevice->GetCaps(&ddDesc)))
		return E_FAIL;

	// Setup the new surface desc
	DDSURFACEDESC2 ddsd;
	D3DUtil_InitSurfaceDesc(ddsd);

	if (((this->m_dwFlags & D3DTEXTR_NO_MIPMAP)) || NOMIPMAPS || !bGlobalTextureStretch)
	{
		ddsd.dwFlags         = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH |
		                       DDSD_PIXELFORMAT | DDSD_TEXTURESTAGE | DDSD_CKSRCBLT | D3DTEXTR_TRANSPARENTBLACK;
		ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE ;
		ddsd.dwTextureStage  = m_dwStage;
		ddsd.dwWidth         = m_dwWidth;
		ddsd.dwHeight        = m_dwHeight;
	}
	else
	{
		ddsd.dwFlags         = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH |
		                       DDSD_PIXELFORMAT | DDSD_TEXTURESTAGE | DDSD_CKSRCBLT | D3DTEXTR_TRANSPARENTBLACK
		                       | DDSD_MIPMAPCOUNT ;
		ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP | DDSCAPS_COMPLEX ;
		ddsd.dwTextureStage  = m_dwStage;
		ddsd.dwWidth         = m_dwWidth;
		ddsd.dwHeight        = m_dwHeight;
		ddsd.dwMipMapCount   = NB_MIPMAP_LEVELS;
	}

	ddsd.ddckCKSrcBlt.dwColorSpaceHighValue = RGB(0, 0, 0);
	ddsd.ddckCKSrcBlt.dwColorSpaceLowValue = RGB(0, 0, 0);


	// Turn on texture management for hardware devices
	if (ddDesc.deviceGUID == IID_IDirect3DHALDevice)
		ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
	else if (ddDesc.deviceGUID == IID_IDirect3DTnLHalDevice)
		ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
	else
		ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

	// Adjust width and height to be powers of 2, if the device requires it
	if ((ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_POW2) || 1)
	{
		for (ddsd.dwWidth = 1;  m_dwWidth > ddsd.dwWidth;   ddsd.dwWidth <<= 1);

		for (ddsd.dwHeight = 1; m_dwHeight > ddsd.dwHeight; ddsd.dwHeight <<= 1);
	}

	// Limit max texture sizes, if the driver can't handle large textures
	DWORD dwMaxWidth  = ddDesc.dwMaxTextureWidth;
	DWORD dwMaxHeight = ddDesc.dwMaxTextureHeight;

	if ((Project.TextureSize) && bGlobalTextureStretch && (!NoResize))
	{
		// Max quality
		if (Project.TextureSize == 0)
		{
			ddsd.dwWidth  = m_dwWidth;
			ddsd.dwHeight = m_dwHeight;
		}
		else if (Project.TextureSize == 2)
		{
			float fRatio = 1;

			if ((ddsd.dwWidth > 128) || (ddsd.dwHeight > 128))
			{
				float fVal = ARX_CLEAN_WARN_CAST_FLOAT(max(ddsd.dwWidth, ddsd.dwHeight));
				fRatio = 128.0f / fVal;
			}

			ddsd.dwWidth  = ARX_CLEAN_WARN_CAST_DWORD(ddsd.dwWidth * fRatio);
			ddsd.dwHeight = ARX_CLEAN_WARN_CAST_DWORD(ddsd.dwHeight * fRatio);
		}
		else if (Project.TextureSize == 64)
		{
			float fRatio = 1;

			if ((ddsd.dwWidth > 64) || (ddsd.dwHeight > 64))
			{
				fRatio = 0.5f;
			}

			ddsd.dwWidth  = ARX_CLEAN_WARN_CAST_DWORD(ddsd.dwWidth * fRatio);
			ddsd.dwHeight = ARX_CLEAN_WARN_CAST_DWORD(ddsd.dwHeight * fRatio);
		}
	}

	if (ddsd.dwWidth > dwMaxWidth)
		ddsd.dwWidth = dwMaxWidth;

	if (ddsd.dwHeight > dwMaxHeight)
		ddsd.dwHeight = dwMaxHeight;

	if (GLOBAL_FORCE_MINI_TEXTURE)
	{
		ddsd.dwWidth	= 8;
		ddsd.dwHeight	= 8;
	}

	if (strstr(tTxt, "FILLED_GAUGE_BLUE")
	        ||	strstr(tTxt, "FILLED_GAUGE_RED"))
	{
		ddsd.dwWidth = 32;

		if (ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_POW2)
			ddsd.dwHeight = 128;
		else
			ddsd.dwHeight = 96;
	}

	//-------------------------------------------------------------------------

	// Make the texture square, if the driver requires it
	if (ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY)
	{
		if (ddsd.dwWidth > ddsd.dwHeight) ddsd.dwHeight = ddsd.dwWidth;
		else                               ddsd.dwWidth  = ddsd.dwHeight;
	}

	// Setup the structure to be used for texture enumration.
	TEXTURESEARCHINFO tsi;
	tsi.bFoundGoodFormat = FALSE;
	tsi.pddpf            = &ddsd.ddpfPixelFormat;
	tsi.dwDesiredBPP     = m_dwBPP;
	tsi.bUsePalette      = FALSE;
	tsi.bUseAlpha        = m_bHasAlpha;

	tsi.dwDesiredBPP = Project.TextureBits;

	if (m_dwFlags & (D3DTEXTR_TRANSPARENTWHITE | D3DTEXTR_TRANSPARENTBLACK))
	{
		if (tsi.bUsePalette)
		{
			if (ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_ALPHAPALETTE)
			{
				tsi.bUseAlpha   = TRUE;
				tsi.bUsePalette = TRUE;
			}
			else
			{
				tsi.bUseAlpha   = TRUE;
				tsi.bUsePalette = FALSE;
			}
		}
	}

	// Enumerate the texture formats, and find the closest device-supported
	// texture pixel format
	tsi.pddpf->dwRBitMask = 0x0000F800;
	tsi.pddpf->dwGBitMask = 0x000007E0;
	tsi.pddpf->dwBBitMask = 0x0000001F;
	pd3dDevice->EnumTextureFormats(TextureSearchCallback, &tsi);

	if (! tsi.bFoundGoodFormat)
	{
		tsi.pddpf->dwRBitMask = 0x00007C00;
		tsi.pddpf->dwGBitMask = 0x000003E0;
		tsi.pddpf->dwBBitMask = 0x0000001F;
		pd3dDevice->EnumTextureFormats(TextureSearchCallback, &tsi);
	}

	// If we couldn't find a format, let's try a default format
	if (FALSE == tsi.bFoundGoodFormat)
	{
		tsi.bUsePalette  = FALSE;
		tsi.dwDesiredBPP = 16;
		tsi.pddpf->dwRBitMask = 0x0000F800;
		tsi.pddpf->dwGBitMask = 0x000007E0;
		tsi.pddpf->dwBBitMask = 0x0000001F;
		pd3dDevice->EnumTextureFormats(TextureSearchCallback, &tsi);

		if (! tsi.bFoundGoodFormat)
		{
			tsi.pddpf->dwRBitMask = 0x00007C00;
			tsi.pddpf->dwGBitMask = 0x000003E0;
			tsi.pddpf->dwBBitMask = 0x0000001F;
			pd3dDevice->EnumTextureFormats(TextureSearchCallback, &tsi);
		}

		// If we still fail, we cannot create this texture
		if (FALSE == tsi.bFoundGoodFormat)
			return E_FAIL;
	}

	// Get the DirectDraw interface for creating surface
	LPDIRECTDRAW7        pDD;
	LPDIRECTDRAWSURFACE7 pddsRender;
	pd3dDevice->GetRenderTarget(&pddsRender);
	pddsRender->GetDDInterface((VOID **)&pDD);
	pddsRender->Release();

	if ((bActivateBump) &&
	        (EERIE_USES_BUMP_MAP))
	{
		DDSURFACEDESC2 ddsdbump = ddsd;
		ddsdbump.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_TEXTURESTAGE | DDSD_CKSRCBLT | D3DTEXTR_TRANSPARENTBLACK;
		ddsdbump.ddsCaps.dwCaps  = DDSCAPS_TEXTURE ;

		HRESULT hr = pDD->CreateSurface(&ddsdbump, &m_pddsBumpMap, NULL);

		if (FAILED(hr))
		{
			m_pddsBumpMap = NULL;
		}
	}
	else
	{
		m_pddsBumpMap = NULL;
	}

	// Create a new surface for the texture
	HRESULT hr = pDD->CreateSurface(&ddsd, &t_m_pddsSurface, NULL);

	// Done with DDraw
	pDD->Release();

	if (FAILED(hr))
		return hr;

	if (m_hbmBitmap)
		 CopyBitmapToSurface(t_m_pddsSurface);

	if (m_pRGBAData)
		CopyRGBADataToSurface(t_m_pddsSurface);

	if (m_pJPEGData) CopyJPEGDataToSurface(t_m_pddsSurface);

	if (m_pPNGData) CopyPNGDataToSurface(t_m_pddsSurface);

	if ((!(this->m_dwFlags & D3DTEXTR_NO_MIPMAP)) && BLURTEXTURES) SmoothSurface(t_m_pddsSurface);

	if (SPECIAL_PNUX) PnuxSurface(t_m_pddsSurface);

	if (strstr(tTxt, "ARKANE."))
	{
		bColorKey = false;
		bColorKey2D = true;
	}
	else
	{
		if (IsColorKeyInSurface(t_m_pddsSurface))
		{
			bColorKey = true;

			if (strstr(tTxt, "INTERFACE") ||
			        strstr(tTxt, "ICON"))
			{
				bColorKey2D = true;
			}


			t_m_pddsSurface->SetColorKey(DDCKEY_SRCBLT , &ddsd.ddckCKSrcBlt);

			if (m_pddsBumpMap) 
			{
				m_pddsBumpMap->Release();
				m_pddsBumpMap = NULL;
			}
		}
		else
		{
			if ((EERIE_USES_BUMP_MAP) &&
			        (m_pddsBumpMap)) CopySurfaceToBumpMap(t_m_pddsSurface, m_pddsBumpMap);
		}
	}

	/**************/
	long ox = ddsd.dwWidth;
	long oy = ddsd.dwHeight;

	this->m_dwDeviceWidth = ox;
	this->m_dwDeviceHeight = oy;

	// Loop through each surface in the mipmap, copying the bitmap to the temp
	// surface, and then blitting the temp surface to the real one.

	LPDIRECTDRAWSURFACE7 pddsDest = t_m_pddsSurface;
	LPDIRECTDRAWSURFACE7 MipDest[NB_MIPMAP_LEVELS];
	LPDIRECTDRAWSURFACE7 pddsNextDest;

	if (bGlobalTextureStretch)
	{
		if ((!(this->m_dwFlags & D3DTEXTR_NO_MIPMAP)) && (!NOMIPMAPS))
		{
			pddsDest->AddRef();

			for (WORD wNum = 0; wNum < NB_MIPMAP_LEVELS; wNum++)
			{
				if (wNum)
				{
					StretchCopySurfaceToSurface(MipDest[wNum-1], pddsDest);
				}

				// Get the next surface in the chain. Do a Release() call, though, to
				// avoid increasing the ref counts on the surfaces.
				DDSCAPS2 ddsCaps;
				ddsCaps.dwCaps  = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
				ddsCaps.dwCaps2 = 0;
				ddsCaps.dwCaps3 = 0;
				ddsCaps.dwCaps4 = 0;
				MipDest[wNum] = pddsDest;

				if (SUCCEEDED(pddsDest->GetAttachedSurface(&ddsCaps, &pddsNextDest)))
				{
					if (wNum + 1 < NB_MIPMAP_LEVELS)
						MipDest[wNum+1] = pddsDest;

					pddsDest->Release();
					pddsDest = pddsNextDest;
				}
				else if (wNum + 1 < NB_MIPMAP_LEVELS)
					MipDest[wNum+1] = NULL;
			}
		}
	}

	m_pddsSurface = t_m_pddsSurface;

	this->m_odx = (1.f / (float)m_dwWidth);
	this->m_hdx = 0.5f * (1.f / (float)ddsd.dwWidth);
	this->m_dx += this->m_hdx;
	this->m_ody = (1.f / (float)m_dwHeight);
	this->m_hdy = 0.5f * (1.f / (float)ddsd.dwHeight);
	this->m_dy += this->m_hdy;

	if (TextureHalo)
	{
		D3DTextr_KillTexture(TextureHalo);
		TextureHalo = AddHalo(pd3dDevice, iHaloNbCouche, fHaloRed, fHaloGreen, fHaloBlue, halodecalX, halodecalY);
	}

	if (m_dwFlags & D3DTEXTR_FAKE_BORDER)
		SpecialBorderSurface(this, this->m_dwOriginalWidth, this->m_dwOriginalHeight);

	return S_OK;
}

void RemoveFakeBlack(BITMAP bm)
{
	if (bm.bmBits == NULL) return;

	if (bm.bmBitsPixel == 32)
	{
		BYTE * sBytes = (BYTE *)bm.bmBits;

		DWORD dwRShiftL		= 0;
		DWORD dwRShiftR		= 0;
		DWORD dwRMask		= 0x000000FF;
		DWORD dwGShiftL		= 0;
		DWORD dwGShiftR		= 8;
		DWORD dwGMask		= 0x0000FF00;
		DWORD dwBShiftL		= 0;
		DWORD dwBShiftR		= 16;
		DWORD dwBMask		= 0x00FF0000;
		DWORD dwAShiftL		= 0;
		DWORD dwAShiftR		= 24;
		DWORD dwAMask		= 0xFF000000;
		DWORD * pSrcData32	= (DWORD *)sBytes;

		ARX_CHECK_NOT_NEG(bm.bmHeight);

		for (DWORD y = 0 ; y < ARX_CAST_ULONG(bm.bmHeight) ; y++)
		{
			ARX_CHECK_NOT_NEG(bm.bmWidth);

			for (DWORD x = 0 ; x < ARX_CAST_ULONG(bm.bmWidth) ; x++)
			{
				// Original Pixel
				DWORD dwPixel;
				long offset = x + y * bm.bmWidthBytes ;

				dwPixel = pSrcData32[offset];

				BYTE r1 = (BYTE)(((dwPixel & dwRMask) >> dwRShiftR) << dwRShiftL);    
				BYTE g1 = (BYTE)(((dwPixel & dwGMask) >> dwGShiftR) << dwGShiftL);     
				BYTE b1 = (BYTE)(((dwPixel & dwBMask) >> dwBShiftR) << dwBShiftL); 
				BYTE a1 = (BYTE)(((dwPixel & dwAMask) >> dwAShiftR) << dwAShiftL);

				if ((r1 != 0) && (r1 < 15)) r1 = 15;

				if ((g1 != 0) && (g1 < 15)) g1 = 15;

				if ((b1 != 0) && (b1 < 15)) b1 = 15;

				long rr	= r1;
				BYTE r	= (BYTE)rr;
				long gg	= g1;
				BYTE g	= (BYTE)gg;
				long bb	= b1;
				BYTE b	= (BYTE)bb;
				long aa	= a1;
				BYTE a	= (BYTE)aa;

				if ((r1 == 0) && (g1 == 0) && (b1 == 0)) a = 0;

				DWORD dr = ((r >> (dwRShiftL)) << dwRShiftR) & dwRMask;
				DWORD dg = ((g >> (dwGShiftL)) << dwGShiftR) & dwGMask;
				DWORD db = ((b >> (dwBShiftL)) << dwBShiftR) & dwBMask;
				DWORD da = ((a >> (dwAShiftL)) << dwAShiftR) & dwAMask;

				pSrcData32[offset] = (DWORD)(dr + dg + db + da);

			}
		}
	}
	else if (bm.bmBitsPixel == 24)
	{
		unsigned char * sBytes = (unsigned char *)bm.bmBits;

		ARX_CHECK_NOT_NEG(bm.bmHeight);

		for (DWORD y = 0 ; y < ARX_CAST_ULONG(bm.bmHeight) ; y++)
		{
			ARX_CHECK_NOT_NEG(bm.bmWidth);

			for (DWORD x = 0 ; x < ARX_CAST_ULONG(bm.bmWidth) ; x++)
			{
				long offset			  = x * 3 + y * bm.bmWidthBytes;
				unsigned char dwPixel = sBytes[offset];

				if ((dwPixel != 0) && (dwPixel < 15)) sBytes[offset] = 15;

				dwPixel = sBytes[offset+1];

				if ((dwPixel != 0) && (dwPixel < 15)) sBytes[offset+1] = 15;

				dwPixel = sBytes[offset+2];

				if ((dwPixel != 0) && (dwPixel < 15)) sBytes[offset+2] = 15;

			}
		}
	}
}

//-----------------------------------------------------------------------------
// Name: CopyBitmapToSurface()
// Desc: Copies the image of a bitmap into a surface
//-----------------------------------------------------------------------------
HRESULT TextureContainer::CopyBitmapToSurface(LPDIRECTDRAWSURFACE7 Surface)
{
	// Get a DDraw object to create a temporary surface
	LPDIRECTDRAW7 pDD;
	Surface->GetDDInterface((VOID **)&pDD);

	// Get the bitmap structure (to extract width, height, and bpp)
	BITMAP bm;
	GetObject(m_hbmBitmap, sizeof(BITMAP), &bm);

	// Setup the new surface desc
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	Surface->GetSurfaceDesc(&ddsd);
	float fWidthSurface = (float)ddsd.dwWidth;
	float fHeightSurface = (float)ddsd.dwHeight;
	ddsd.dwFlags          = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT |
	                        DDSD_TEXTURESTAGE;
	ddsd.ddsCaps.dwCaps   = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;

	ddsd.ddsCaps.dwCaps2  = 0L;
	ddsd.dwWidth          = bm.bmWidth;
	ddsd.dwHeight         = bm.bmHeight;

	// Create a new surface for the texture
	LPDIRECTDRAWSURFACE7 pddsTempSurface;
	HRESULT hr;

	if (FAILED(hr = pDD->CreateSurface(&ddsd, &pddsTempSurface, NULL)))
	{
		pDD->Release();
		return hr;
	}

	// Get a DC for the bitmap
	HDC hdcBitmap = CreateCompatibleDC(NULL);

	if (NULL == hdcBitmap)
	{
		pddsTempSurface->Release();
		pDD->Release();
		return hr;
	}

	SelectObject(hdcBitmap, m_hbmBitmap);

	// Handle palettized textures. Need to attach a palette
	if (ddsd.ddpfPixelFormat.dwRGBBitCount == 8)
	{
		LPDIRECTDRAWPALETTE  pPalette;
		DWORD dwPaletteFlags = DDPCAPS_8BIT | DDPCAPS_ALLOW256;
		DWORD pe[256];
		UINT uiRes = GetDIBColorTable(hdcBitmap, 0, 256, (RGBQUAD *)pe);
		ARX_CHECK_WORD(uiRes);
		WORD  wNumColors     = ARX_CLEAN_WARN_CAST_WORD(uiRes);

		// Create the color table
		for (WORD i = 0; i < wNumColors; i++)
		{
			pe[i] = RGB(GetBValue(pe[i]), GetGValue(pe[i]), GetRValue(pe[i]));

			// Handle textures with transparent pixels
			if (m_dwFlags & (D3DTEXTR_TRANSPARENTWHITE | D3DTEXTR_TRANSPARENTBLACK))
			{
				// Set alpha for opaque pixels
				if (m_dwFlags & D3DTEXTR_TRANSPARENTBLACK)
				{
					if (pe[i] != 0x00000000)
						pe[i] |= 0xff000000;
				}
				else if (m_dwFlags & D3DTEXTR_TRANSPARENTWHITE)
				{
					if (pe[i] != 0x00ffffff)
						pe[i] |= 0xff000000;
				}
			}
		}

		// Add DDPCAPS_ALPHA flag for textures with transparent pixels
		if (m_dwFlags & (D3DTEXTR_TRANSPARENTWHITE | D3DTEXTR_TRANSPARENTBLACK))
			dwPaletteFlags |= DDPCAPS_ALPHA;

		// Create & attach a palette
		pDD->CreatePalette(dwPaletteFlags, (PALETTEENTRY *)pe, &pPalette, NULL);
		pddsTempSurface->SetPalette(pPalette);
		Surface->SetPalette(pPalette);
		SAFE_RELEASE(pPalette);
	}

	
	RemoveFakeBlack(bm);

	// Copy the bitmap image to the surface.
	HDC hdcSurface;

	if (SUCCEEDED(pddsTempSurface->GetDC(&hdcSurface)))
	{
		BitBlt(hdcSurface, 0, 0, bm.bmWidth, bm.bmHeight, hdcBitmap, 0, 0,
		       SRCCOPY);
		pddsTempSurface->ReleaseDC(hdcSurface);
	}

	DeleteDC(hdcBitmap);



	// Copy the temp surface to the real texture surface
	if (bGlobalTextureStretch)
	{
		m_dx = .999999f;
		m_dy = .999999f;
		Surface->Blt(NULL, pddsTempSurface, NULL, DDBLT_WAIT, NULL);
	}
	else
	{
		RECT rBlt;
		SetRect(&rBlt, 0, 0, ddsd.dwWidth, ddsd.dwHeight);

		if (ddsd.dwWidth < fWidthSurface)
		{
			m_dx = ((float)ddsd.dwWidth) / fWidthSurface;
		}
		else
		{
			rBlt.right = (int)fWidthSurface;
			m_dx = .999999f;
		}

		if (ddsd.dwHeight < fHeightSurface)
		{
			m_dy = ((float)ddsd.dwHeight) / fHeightSurface;
		}
		else
		{
			rBlt.bottom = (int)fHeightSurface;
			m_dy = 0.999999f;
		}

		Surface->Blt(&rBlt, pddsTempSurface, NULL, DDBLT_WAIT, NULL);
	}

	DDSURFACEDESC2 ddesc, ddesc2;
	ddesc.dwSize = sizeof(ddesc);
	ddesc2.dwSize = sizeof(ddesc2);

	// Done with the temp surface
	pddsTempSurface->Release();

	// For textures with real alpha (not palettized), set transparent bits
	if (ddsd.ddpfPixelFormat.dwRGBAlphaBitMask)
	{
		if (m_dwFlags & (D3DTEXTR_TRANSPARENTWHITE | D3DTEXTR_TRANSPARENTBLACK))
		{
			// Lock the texture surface
			DDSURFACEDESC2 ddsd;
			ddsd.dwSize = sizeof(ddsd);

			while (Surface->Lock(NULL, &ddsd, 0, NULL) ==
			        DDERR_WASSTILLDRAWING);

			DWORD dwAlphaMask = ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;
			DWORD dwRGBMask   = (ddsd.ddpfPixelFormat.dwRBitMask |
			                     ddsd.ddpfPixelFormat.dwGBitMask |
			                     ddsd.ddpfPixelFormat.dwBBitMask);
			DWORD dwColorkey  = 0x00000000; // Colorkey on black

			if (m_dwFlags & D3DTEXTR_TRANSPARENTWHITE)
				dwColorkey = dwRGBMask;     // Colorkey on white

			// Add an opaque alpha value to each non-colorkeyed pixel
			for (DWORD y = 0; y < ddsd.dwHeight; y++)
			{
				WORD * p16 = (WORD *)((BYTE *)ddsd.lpSurface + y * ddsd.lPitch);
				DWORD * p32 = (DWORD *)((BYTE *)ddsd.lpSurface + y * ddsd.lPitch);

				for (DWORD x = 0; x < ddsd.dwWidth; x++)
				{
					if (ddsd.ddpfPixelFormat.dwRGBBitCount == 16)
					{
						if ((*p16 &= dwRGBMask) != dwColorkey)
							* p16 |= dwAlphaMask;

						p16++;
					}

					if (ddsd.ddpfPixelFormat.dwRGBBitCount == 32)
					{
						if ((*p32 &= dwRGBMask) != dwColorkey)
							* p32 |= dwAlphaMask;

						p32++;
					}
				}
			}

			Surface->Unlock(NULL);
		}
	}

	pDD->Release();

	return S_OK;;
}


//-----------------------------------------------------------------------------
// Name: CopyBitmapToSurface2()
// Desc: Copie une partie de bitmap sur la surface, un restore doit avoir ete fait avant!!!  seb
//-----------------------------------------------------------------------------
HRESULT TextureContainer::CopyBitmapToSurface2(HBITMAP hbitmap, int depx, int depy, int largeur, int hauteur, long flag)
{
	LPDIRECTDRAWSURFACE7 Surface;

	Surface = this->m_pddsSurface;

	// Get a DDraw object to create a temporary surface
	LPDIRECTDRAW7 pDD;
	Surface->GetDDInterface((VOID **)&pDD);

	// Get the bitmap structure (to extract width, height, and bpp)
	BITMAP bm;
	GetObject(hbitmap, sizeof(BITMAP), &bm);

	// Setup the new surface desc
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	Surface->GetSurfaceDesc(&ddsd);
	float fWidthSurface = (float)ddsd.dwWidth;
	float fHeightSurface = (float)ddsd.dwHeight;
	ddsd.dwFlags          = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT |
	                        DDSD_TEXTURESTAGE;
	ddsd.ddsCaps.dwCaps   = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;
	ddsd.ddsCaps.dwCaps2  = 0L;
	ddsd.dwWidth          = largeur;
	ddsd.dwHeight         = hauteur;

	// Create a new surface for the texture
	LPDIRECTDRAWSURFACE7 pddsTempSurface;
	HRESULT hr;

	if (FAILED(hr = pDD->CreateSurface(&ddsd, &pddsTempSurface, NULL)))
	{
		pDD->Release();
		return hr;
	}

	// Get a DC for the bitmap
	HDC hdcBitmap = CreateCompatibleDC(NULL);

	if (NULL == hdcBitmap)
	{
		pddsTempSurface->Release();
		pDD->Release();
		return hr;
	}

	SelectObject(hdcBitmap, hbitmap);

	// Handle palettized textures. Need to attach a palette
	if (ddsd.ddpfPixelFormat.dwRGBBitCount == 8)
	{
		LPDIRECTDRAWPALETTE  pPalette;
		DWORD dwPaletteFlags = DDPCAPS_8BIT | DDPCAPS_ALLOW256;
		DWORD pe[256];

		UINT uiRes = GetDIBColorTable(hdcBitmap, 0, 256, (RGBQUAD *)pe);
		ARX_CHECK_WORD(uiRes);
		WORD  wNumColors     = ARX_CLEAN_WARN_CAST_WORD(uiRes);

		// Create the color table
		for (WORD i = 0; i < wNumColors; i++)
		{
			pe[i] = RGB(GetBValue(pe[i]), GetGValue(pe[i]), GetRValue(pe[i]));

			// Handle textures with transparent pixels
			if (m_dwFlags & (D3DTEXTR_TRANSPARENTWHITE | D3DTEXTR_TRANSPARENTBLACK))
			{
				// Set alpha for opaque pixels
				if (m_dwFlags & D3DTEXTR_TRANSPARENTBLACK)
				{
					if (pe[i] != 0x00000000)
						pe[i] |= 0xff000000;
				}
				else if (m_dwFlags & D3DTEXTR_TRANSPARENTWHITE)
				{
					if (pe[i] != 0x00ffffff)
						pe[i] |= 0xff000000;
				}
			}
		}

		// Add DDPCAPS_ALPHA flag for textures with transparent pixels
		if (m_dwFlags & (D3DTEXTR_TRANSPARENTWHITE | D3DTEXTR_TRANSPARENTBLACK))
			dwPaletteFlags |= DDPCAPS_ALPHA;

		// Create & attach a palette
		pDD->CreatePalette(dwPaletteFlags, (PALETTEENTRY *)pe, &pPalette, NULL);
		pddsTempSurface->SetPalette(pPalette);
		Surface->SetPalette(pPalette);
		SAFE_RELEASE(pPalette);
	}

	if (!(flag & 1)) RemoveFakeBlack(bm);

	// Copy the bitmap image to the surface.
	HDC hdcSurface;

	if (SUCCEEDED(pddsTempSurface->GetDC(&hdcSurface)))
	{
		BitBlt(hdcSurface, 0, 0, largeur, hauteur, hdcBitmap, depx, depy,
		       SRCCOPY);
		pddsTempSurface->ReleaseDC(hdcSurface);

	}

	DeleteDC(hdcBitmap);

	if (bGlobalTextureStretch)
	{
		m_dx = .999999f;
		m_dy = .999999f;
		Surface->Blt(NULL, pddsTempSurface, NULL, DDBLT_WAIT, NULL);
	}
	else
	{
		RECT rBlt;
		SetRect(&rBlt, 0, 0, ddsd.dwWidth, ddsd.dwHeight);

		if (ddsd.dwWidth < fWidthSurface)
		{
			m_dx = ((float)ddsd.dwWidth) / fWidthSurface;
		}
		else
		{
			rBlt.right = (int)fWidthSurface;
			m_dx = .999999f;
		}

		if (ddsd.dwHeight < fHeightSurface)
		{
			m_dy = ((float)ddsd.dwHeight) / fHeightSurface;
		}
		else
		{
			rBlt.bottom = (int)fHeightSurface;
			m_dy = 0.999999f;
		}

		Surface->Blt(&rBlt, pddsTempSurface, NULL, DDBLT_WAIT, NULL);
	}

	DDSURFACEDESC2 ddesc, ddesc2;
	ddesc.dwSize = sizeof(ddesc);
	ddesc2.dwSize = sizeof(ddesc2);

	// Done with the temp surface
	pddsTempSurface->Release();

	// For textures with real alpha (not palettized), set transparent bits
	if (ddsd.ddpfPixelFormat.dwRGBAlphaBitMask)
	{
		if (m_dwFlags & (D3DTEXTR_TRANSPARENTWHITE | D3DTEXTR_TRANSPARENTBLACK))
		{
			// Lock the texture surface
			DDSURFACEDESC2 ddsd;
			ddsd.dwSize = sizeof(ddsd);

			while (Surface->Lock(NULL, &ddsd, 0, NULL) ==
			        DDERR_WASSTILLDRAWING);

			DWORD dwAlphaMask = ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;
			DWORD dwRGBMask   = (ddsd.ddpfPixelFormat.dwRBitMask |
			                     ddsd.ddpfPixelFormat.dwGBitMask |
			                     ddsd.ddpfPixelFormat.dwBBitMask);
			DWORD dwColorkey  = 0x00000000; // Colorkey on black

			if (m_dwFlags & D3DTEXTR_TRANSPARENTWHITE)
				dwColorkey = dwRGBMask;     // Colorkey on white

			// Add an opaque alpha value to each non-colorkeyed pixel
			for (DWORD y = 0; y < ddsd.dwHeight; y++)
			{
				WORD * p16 = (WORD *)((BYTE *)ddsd.lpSurface + y * ddsd.lPitch);
				DWORD * p32 = (DWORD *)((BYTE *)ddsd.lpSurface + y * ddsd.lPitch);

				for (DWORD x = 0; x < ddsd.dwWidth; x++)
				{
					if (ddsd.ddpfPixelFormat.dwRGBBitCount == 16)
					{
						if ((*p16 &= dwRGBMask) != dwColorkey)
							* p16 |= dwAlphaMask;

						p16++;
					}

					if (ddsd.ddpfPixelFormat.dwRGBBitCount == 32)
					{
						if ((*p32 &= dwRGBMask) != dwColorkey)
							* p32 |= dwAlphaMask;

						p32++;
					}
				}
			}

			Surface->Unlock(NULL);
		}
	}

	pDD->Release();

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CopyRGBADataToSurface()
// Desc: Invalidates the current texture objects and rebuilds new ones
//       using the new device.
//-----------------------------------------------------------------------------
HRESULT TextureContainer::CopyRGBADataToSurface(LPDIRECTDRAWSURFACE7 Surface)
{
	// Get a DDraw object to create a temporary surface
	LPDIRECTDRAW7 pDD;
	Surface->GetDDInterface((VOID **)&pDD);

	// Setup the new surface desc
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	Surface->GetSurfaceDesc(&ddsd);
	float fWidthSurface = (float)ddsd.dwWidth;
	float fHeightSurface = (float)ddsd.dwHeight;
	ddsd.dwFlags         = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT |
	                       DDSD_TEXTURESTAGE;
	ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;
	ddsd.ddsCaps.dwCaps2 = 0L;
	ddsd.dwWidth         = m_dwWidth;
	ddsd.dwHeight        = m_dwHeight;

	// Create a new surface for the texture
	LPDIRECTDRAWSURFACE7 pddsTempSurface;
	HRESULT hr;

	if (FAILED(hr = pDD->CreateSurface(&ddsd, &pddsTempSurface, NULL)))
	{
		pDD->Release();
		return NULL;
	}

	while (pddsTempSurface->Lock(NULL, &ddsd, 0, 0) == DDERR_WASSTILLDRAWING);

	//    DWORD lPitch = ddsd.lPitch;
	BYTE * pBytes = (BYTE *)ddsd.lpSurface;

	DWORD dwRMask = ddsd.ddpfPixelFormat.dwRBitMask;
	DWORD dwGMask = ddsd.ddpfPixelFormat.dwGBitMask;
	DWORD dwBMask = ddsd.ddpfPixelFormat.dwBBitMask;
	DWORD dwAMask = ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;

	DWORD dwRShiftL = 8, dwRShiftR = 0;
	DWORD dwGShiftL = 8, dwGShiftR = 0;
	DWORD dwBShiftL = 8, dwBShiftR = 0;
	DWORD dwAShiftL = 8, dwAShiftR = 0;

	DWORD dwMask;

	for (dwMask = dwRMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwRShiftR++;

	for (; dwMask; dwMask >>= 1) dwRShiftL--;

	for (dwMask = dwGMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwGShiftR++;

	for (; dwMask; dwMask >>= 1) dwGShiftL--;

	for (dwMask = dwBMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwBShiftR++;

	for (; dwMask; dwMask >>= 1) dwBShiftL--;

	for (dwMask = dwAMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwAShiftR++;

	for (; dwMask; dwMask >>= 1) dwAShiftL--;

	for (DWORD y = 0; y < ddsd.dwHeight; y++)
	{
		DWORD * pDstData32 = (DWORD *)pBytes;
		WORD * pDstData16 = (WORD *)pBytes;

		for (DWORD x = 0; x < ddsd.dwWidth; x++)
		{
			DWORD dwPixel = m_pRGBAData[y*ddsd.dwWidth+x];

			BYTE r = (BYTE)((dwPixel >> 24) & 0x000000ff);
			BYTE g = (BYTE)((dwPixel >> 16) & 0x000000ff);
			BYTE b = (BYTE)((dwPixel >> 8) & 0x000000ff);
			BYTE a = (BYTE)((dwPixel >> 0) & 0x000000ff);

			DWORD dr = ((r >> (dwRShiftL)) << dwRShiftR)&dwRMask;
			DWORD dg = ((g >> (dwGShiftL)) << dwGShiftR)&dwGMask;
			DWORD db = ((b >> (dwBShiftL)) << dwBShiftR)&dwBMask;
			DWORD da = ((a >> (dwAShiftL)) << dwAShiftR)&dwAMask;

			if (32 == ddsd.ddpfPixelFormat.dwRGBBitCount)
				pDstData32[x] = (DWORD)(dr + dg + db + da);
			else
				pDstData16[x] = (WORD)(dr + dg + db + da);
		}

		pBytes += ddsd.lPitch;
	}

	pddsTempSurface->Unlock(0);

	// Copy the temp surface to the real texture surface
	if (bGlobalTextureStretch)
	{
		m_dx = .999999f;
		m_dy = .999999f;
		Surface->Blt(NULL, pddsTempSurface, NULL, DDBLT_WAIT, NULL);
	}
	else
	{
		RECT rBlt;
		SetRect(&rBlt, 0, 0, ddsd.dwWidth, ddsd.dwHeight);

		if (ddsd.dwWidth < fWidthSurface)
		{
			m_dx = ((float)ddsd.dwWidth) / fWidthSurface;
		}
		else
		{
			rBlt.right = (int)fWidthSurface;
			m_dx = .999999f;
		}

		if (ddsd.dwHeight < fHeightSurface)
		{
			m_dy = ((float)ddsd.dwHeight) / fHeightSurface;
		}
		else
		{
			rBlt.bottom = (int)fHeightSurface;
			m_dy = 0.999999f;
		}

		Surface->Blt(&rBlt, pddsTempSurface, NULL, DDBLT_WAIT, NULL);
	}

	// Done with the temp objects
	pddsTempSurface->Release();
	pDD->Release();

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CopyJPEGDataToSurface()
// Desc: Invalidates the current texture objects and rebuilds new ones
//       using the new device.
//-----------------------------------------------------------------------------
int JPEGError;
METHODDEF(void)
my_error_exit(j_common_ptr cinfo)
{
	char txt[256];
	(*cinfo->err->output_message)(cinfo); 
	sprintf(txt, "truc zarb: %s", cinfo->err->jpeg_message_table[cinfo->err->msg_code]);
	MessageBox(NULL, txt, "EERIE JPEG Error", 0);

	JPEGError = 1;
	return;
}
BOOL JPEG_NO_TRUE_BLACK = TRUE;
/*--------------------------------------------------------------------------------*/
HRESULT TextureContainer::CopyJPEGDataToSurface(LPDIRECTDRAWSURFACE7 Surface)
{
	// Get a DDraw object to create a temporary surface
	LPDIRECTDRAW7 pDD;
	Surface->GetDDInterface((VOID **)&pDD);

	// Setup the new surface desc
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	Surface->GetSurfaceDesc(&ddsd);
	float fWidthSurface = (float)ddsd.dwWidth;
	float fHeightSurface = (float)ddsd.dwHeight;
	ddsd.dwFlags         = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT |
	                       DDSD_TEXTURESTAGE;
	ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;
	ddsd.ddsCaps.dwCaps2 = 0L;
	ddsd.dwWidth         = m_dwWidth;
	ddsd.dwHeight        = m_dwHeight;

	// Create a new surface for the texture
	LPDIRECTDRAWSURFACE7 pddsTempSurface;
	HRESULT hr;

	if (FAILED(hr = pDD->CreateSurface(&ddsd, &pddsTempSurface, NULL)))
	{
		pDD->Release();
		return NULL;
	}

	while (pddsTempSurface->Lock(NULL, &ddsd, 0, 0) == DDERR_WASSTILLDRAWING);

	BYTE * pBytes = (BYTE *)ddsd.lpSurface;

	DWORD dwRMask = ddsd.ddpfPixelFormat.dwRBitMask;
	DWORD dwGMask = ddsd.ddpfPixelFormat.dwGBitMask;
	DWORD dwBMask = ddsd.ddpfPixelFormat.dwBBitMask;
	DWORD dwAMask = ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;

	DWORD dwRShiftL = 8, dwRShiftR = 0;
	DWORD dwGShiftL = 8, dwGShiftR = 0;
	DWORD dwBShiftL = 8, dwBShiftR = 0;
	DWORD dwAShiftL = 8, dwAShiftR = 0;

	DWORD dwMask;

	for (dwMask = dwRMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwRShiftR++;

	for (; dwMask; dwMask >>= 1) dwRShiftL--;

	for (dwMask = dwGMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwGShiftR++;

	for (; dwMask; dwMask >>= 1) dwGShiftL--;

	for (dwMask = dwBMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwBShiftR++;

	for (; dwMask; dwMask >>= 1) dwBShiftL--;

	for (dwMask = dwAMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwAShiftR++;

	for (; dwMask; dwMask >>= 1) dwAShiftL--;

	struct	jpeg_decompress_struct	* cinfo = (jpeg_decompress_struct *)m_pJPEGData;
	long	dx, dy;

	//initialisé d'abord le format output
	cinfo->out_color_space = JCS_RGB;
	cinfo->output_components = 3;
	jpeg_start_decompress(cinfo);

	if (JPEGError)
	{
		JPEGError = 0;
		pddsTempSurface->Unlock(0);
		pddsTempSurface->Release();
		pDD->Release();
		return NULL;
	}


	char * buffer = new char[cinfo->output_width*cinfo->output_height*cinfo->output_components];

	if (!buffer)
	{
		(void)jpeg_finish_decompress(cinfo);
		pddsTempSurface->Unlock(0);
		pddsTempSurface->Release();
		pDD->Release();
		return NULL;
	}

	unsigned char * bufferconv;

	dy = m_dwHeight;

	while (dy)
	{
		DWORD * pDstData32 = (DWORD *)pBytes;
		WORD * pDstData16 = (WORD *)pBytes;

		jpeg_read_scanlines(cinfo, (unsigned char **)&buffer, 1);

		if (JPEGError)
		{
			JPEGError = 0;
			pddsTempSurface->Unlock(0);
			pddsTempSurface->Release();
			pDD->Release();
			(void)jpeg_finish_decompress(cinfo);
			return NULL;
		}

		bufferconv = (unsigned char *)buffer;
		dx = m_dwWidth;

		while (dx)
		{
			BYTE r = (BYTE) * bufferconv++;
			BYTE g = (BYTE) * bufferconv++;
			BYTE b = (BYTE) * bufferconv++;
			BYTE a = 0xFF;

			if ((JPEG_NO_TRUE_BLACK) && (r < 15) && (g < 15) && (b < 15))
			{
				r = 15;
				g = 15;
				b = 15;
			}

			DWORD dr = ((r >> (dwRShiftL)) << dwRShiftR)&dwRMask;
			DWORD dg = ((g >> (dwGShiftL)) << dwGShiftR)&dwGMask;
			DWORD db = ((b >> (dwBShiftL)) << dwBShiftR)&dwBMask;
			DWORD da = ((a >> (dwAShiftL)) << dwAShiftR)&dwAMask;

			if (32 == ddsd.ddpfPixelFormat.dwRGBBitCount)
				*pDstData32++ = (DWORD)(dr + dg + db + da);
			else
				*pDstData16++ = (WORD)(dr + dg + db + da);

			dx--;
		}

		pBytes += ddsd.lPitch;
		dy--;
	}

	(void)jpeg_finish_decompress(cinfo);
	(void)jpeg_mem_reinitsrc((void *)cinfo);
	delete buffer;

	pddsTempSurface->Unlock(0);

	// Copy the temp surface to the real texture surface
	if (bGlobalTextureStretch)
	{
		m_dx = .999999f;
		m_dy = .999999f;
		Surface->Blt(NULL, pddsTempSurface, NULL, DDBLT_WAIT, NULL);
	}
	else
	{
		RECT rBlt;
		SetRect(&rBlt, 0, 0, ddsd.dwWidth, ddsd.dwHeight);

		if (ddsd.dwWidth < fWidthSurface)
		{
			m_dx = ((float)ddsd.dwWidth) / fWidthSurface;
		}
		else
		{
			rBlt.right = (int)fWidthSurface;
			m_dx = .999999f;
		}

		if (ddsd.dwHeight < fHeightSurface)
		{
			m_dy = ((float)ddsd.dwHeight) / fHeightSurface;
		}
		else
		{
			rBlt.bottom = (int)fHeightSurface;
			m_dy = 0.999999f;
		}

		Surface->Blt(&rBlt, pddsTempSurface, NULL, DDBLT_WAIT, NULL);
	}

	// Done with the temp objects
	pddsTempSurface->Release();
	pDD->Release();

	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: D3DTextr_SetTexturePath()
// Desc: Enumeration callback routine to find a best-matching texture format.
//-----------------------------------------------------------------------------

void ConvertData(char * dat)
{
	if (dat[0] == '"') strcpy(dat, dat + 1);

	for (ULONG i = 1 ; i < strlen(dat) ; i++)
	{
		if (dat[i] == '"') dat[i] = 0;
	}
}

void LookForRefinementMap(TextureContainer * tc)
{
	char * GlobalRefine = NULL;
	long GlobalRefine_size = 0;
	char * Refine = NULL;
	long Refine_size = 0;
	char inifile[512];
	long count = 0;
	char str1[1024];
	char str2[1024];
	tc->TextureRefinement = NULL;

	if (GlobalRefine == NULL)
	{
		if (GlobalRefine_size == 0)
		{
			sprintf(inifile, "%sGraph\\Obj3D\\Textures\\Refinement\\GlobalRefinement.ini", Project.workingdir);

			if (PAK_FileExist(inifile))
			{
				GlobalRefine = (char *)PAK_FileLoadMallocZero(inifile, &GlobalRefine_size);
			}

			if (GlobalRefine == NULL) GlobalRefine_size = -1;
		}
	}

	if (Refine == NULL)
	{
		if (Refine_size == 0)
		{
			sprintf(inifile, "%sGraph\\Obj3D\\Textures\\Refinement\\Refinement.ini", Project.workingdir);

			if (PAK_FileExist(inifile))
			{
				Refine = (char *)PAK_FileLoadMallocZero(inifile, &Refine_size);
			}

			if (Refine == NULL) Refine_size = -1;
		}
	}

	if (GlobalRefine)
	{
		unsigned char * from = (unsigned char *)GlobalRefine;
		long fromsize = GlobalRefine_size;
		unsigned char data[256];
		long pos = 0;
		char name[256];
		strcpy(name, GetName(tc->m_strName));

		while (pos < GlobalRefine_size)
		{
			long pos2 = 0;

			while ((from[pos] != '\n') && (pos < fromsize))
			{
				data[pos2++] = from[pos++];

				if (pos2 > 255) pos2 = 255;

				if (pos >= GlobalRefine_size) break;
			}

			data[pos2] = 0;

			while ((pos < fromsize) && (from[pos] < 32)) pos++;

			if (count == 2)
			{
				count = 0;
				continue;
			}

			ConvertData((char *)data);

			if (count == 0) strcpy(str1, (char *)data);

			if (count == 1)
			{
				sprintf(str2, "%sGraph\\Obj3D\\Textures\\Refinement\\%s.bmp", Project.workingdir, data);
				MakeUpcase(str1);
				MakeUpcase(name);

				if (strstr(name, str1))
				{
					if (!_stricmp((char *)data, "NONE")) tc->TextureRefinement = NULL;
					else tc->TextureRefinement =
						    D3DTextr_CreateTextureFromFile(str2, Project.workingdir, 0, D3DTEXTR_16BITSPERPIXEL);

				}
			}

			count++;
		}
	}

	if (Refine)
	{
		unsigned char * from = (unsigned char *)Refine;
		long fromsize = Refine_size;
		unsigned char data[256];
		long pos = 0;
		char name[256];
		strcpy(name, GetName(tc->m_strName));

		while (pos < Refine_size)
		{
			long pos2 = 0;

			while ((from[pos] != '\n') && (pos < fromsize))
			{
				data[pos2++] = from[pos++];

				if (pos2 > 255) pos2 = 255;

				if (pos >= Refine_size) break;
			}

			data[pos2] = 0;

			while ((pos < fromsize) && (from[pos] < 32)) pos++;

			if (count == 2)
			{
				count = 0;
				continue;
			}

			ConvertData((char *)data);

			if (count == 0) strcpy(str1, (char *)data);

			if (count == 1)
			{
				sprintf(str2, "%sGraph\\Obj3D\\Textures\\Refinement\\%s.bmp", Project.workingdir, data);
				MakeUpcase(str1);
				MakeUpcase(name);

				if (!strcmp(name, str1))
				{
					if (!_stricmp((char *)data, "NONE")) tc->TextureRefinement = NULL;
					else tc->TextureRefinement =
						    D3DTextr_CreateTextureFromFile(str2, Project.workingdir, 0, D3DTEXTR_16BITSPERPIXEL);

				}
			}

			count++;
		}
	}

	if (GlobalRefine)
	{
		free((void *)GlobalRefine);
		GlobalRefine = NULL;
	}

	if (Refine)
	{
		free((void *)Refine);
		Refine = NULL;
	}
}
void ReleaseAllTCWithFlag(long flag)
{
	restart:
	;
	TextureContainer * ptcTexture = g_ptcTextureList;

	while (ptcTexture)
	{
		if (ptcTexture->systemflags & flag)
		{
			D3DTextr_KillTexture(ptcTexture);
			goto restart;
		}
		ptcTexture = ptcTexture->m_pNext;
	}

}

extern void MakeUserFlag(TextureContainer * tc);
TextureContainer * LastTextureContainer = NULL;
//-----------------------------------------------------------------------------
// Name: D3DTextr_CreateTextureFromFile()
// Desc: Is passed a filename and creates a local Bitmap from that file.
//       The texture can not be used until it is restored, however.
//-----------------------------------------------------------------------------
#include "hermesmain.h"
extern long DEBUGSYS;

TextureContainer * D3DTextr_CreateTextureFromFile(TCHAR * strName, char * wd , DWORD dwStage,
        DWORD dwFlags, long sysflags)
{
	TextureContainer * ReturnValue = NULL;
	// Check parameters
	char texx[300];

	if (DEBUGSYS)
	{
		sprintf(texx, "LoadTextureFF %s", strName);
		ForceSendConsole(texx, 1, 0, (HWND)1);
	}

	LastTextureContainer = NULL;

	if (NULL == strName)
		return ReturnValue;

	// Check first to see if the texture is already loaded
	MakeUpcase(strName);

	if (NULL != (LastTextureContainer = FindTexture(strName)))
	{
		if (DEBUGSYS)
		{
			sprintf(texx, "CreateTex: ALREADY LOADED  - %s ", strName);
			SendConsole(texx, 3, 0, (HWND)1);
		}

		return LastTextureContainer;
	}

	// Allocate and add the texture to the linked list of textures;
	TextureContainer * ptcTexture = new TextureContainer(strName, wd , dwStage,
	        dwFlags);

	if (NULL == ptcTexture)
	{
		if (DEBUGSYS)
		{
			sprintf(texx, "CreateTex: FAILED Out of Memory - %s ", strName);
			SendConsole(texx, 3, 0, (HWND)1);
		}

		return ReturnValue;
	}

	ptcTexture->systemflags = sysflags;

	if (GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE == 1)
		ptcTexture->systemflags |= EERIETEXTUREFLAG_LOADSCENE_RELEASE;
	else if (GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE == -1)
		ptcTexture->systemflags &= ~EERIETEXTUREFLAG_LOADSCENE_RELEASE;

	// Create a bitmap and load the texture file into it,
	if (FAILED(ptcTexture->LoadImageData()))
	{
		delete ptcTexture;

		if (DEBUGSYS)
		{
			sprintf(texx, "CreateTex: FAILED Unable to Load Image Data - %s ", strName);
			SendConsole(texx, 3, 0, (HWND)1);
		}

		return ReturnValue;
	}

	// Save the image's dimensions
	if (ptcTexture->m_hbmBitmap)
	{
		BITMAP bm;
		GetObject(ptcTexture->m_hbmBitmap, sizeof(BITMAP), &bm);
		ptcTexture->m_dwWidth  = (DWORD)bm.bmWidth;
		ptcTexture->m_dwHeight = (DWORD)bm.bmHeight;
		ptcTexture->m_dwBPP    = (DWORD)bm.bmBitsPixel;
	}

	ptcTexture->m_dwDeviceWidth = 0;
	ptcTexture->m_dwDeviceHeight = 0;
	ptcTexture->m_dwOriginalWidth = ptcTexture->m_dwWidth;
	ptcTexture->m_dwOriginalHeight = ptcTexture->m_dwHeight;

	if (ptcTexture->m_dwWidth > 0.f)
	{
		ptcTexture->m_odx = 1.f / (float)ptcTexture->m_dwWidth;
		ptcTexture->m_dx = ptcTexture->m_odx;
	}
	else
		ptcTexture->m_odx = ptcTexture->m_dx = DIV256;

	ptcTexture->m_hdx = 0.5f * ptcTexture->m_dx;

	if (ptcTexture->m_dwHeight > 0.f)
	{
		ptcTexture->m_ody = 1.f / (float)ptcTexture->m_dwHeight;
		ptcTexture->m_dy = ptcTexture->m_ody;
	}
	else
		ptcTexture->m_ody = ptcTexture->m_dy = DIV256;

	ptcTexture->m_hdy = 0.5f * ptcTexture->m_dy;

	ReturnValue = LastTextureContainer = ptcTexture;

	if (!(dwFlags & D3DTEXTR_NO_REFINEMENT))
		LookForRefinementMap(ReturnValue);

	ReturnValue = LastTextureContainer = ptcTexture;

	if (DEBUGSYS)
	{
		sprintf(texx, "CreateTex: SUCCESS !!! - %s", strName);
		SendConsole(texx, 3, 0, (HWND)1);
	}

	ptcTexture->Use();
	MakeUserFlag(ptcTexture);
	return ReturnValue;
}



//-----------------------------------------------------------------------------
// Name: D3DTextr_CreateEmptyTexture()
// Desc: Creates an empty texture.
//-----------------------------------------------------------------------------
HRESULT D3DTextr_CreateEmptyTexture(TCHAR * strName, DWORD dwWidth,
                                    DWORD dwHeight, DWORD dwStage,
                                    DWORD dwFlags, char * wd , DWORD flags)
{
	if (!(flags & 1)) // no name check
	{
		// Check parameters
		if (NULL == strName)
			return E_INVALIDARG;

		// Check first to see if the texture is already loaded
		MakeUpcase(strName);

		if (NULL != FindTexture(strName))
			return E_FAIL;
	}

	// Allocate and add the texture to the linked list of textures;
	TextureContainer * ptcTexture = new TextureContainer(strName, wd, dwStage,
	        dwFlags);

	if (NULL == ptcTexture)
		return E_OUTOFMEMORY;

	// Save dimensions
	ptcTexture->m_dwWidth  = dwWidth;
	ptcTexture->m_dwHeight = dwHeight;
	ptcTexture->m_dwBPP    = 32;

	// Save alpha usage flag
	if (dwFlags & D3DTEXTR_CREATEWITHALPHA)
		ptcTexture->m_bHasAlpha = TRUE;

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: D3DTextr_Restore()
// Desc: Invalidates the current texture objects and rebuilds new ones
//       using the new device.
//-----------------------------------------------------------------------------
HRESULT D3DTextr_Restore(TCHAR * strName, LPDIRECT3DDEVICE7 pd3dDevice)
{
	MakeUpcase(strName);
	TextureContainer * ptcTexture = FindTexture(strName);

	if (NULL == ptcTexture)
		return DDERR_NOTFOUND;

	// Restore the texture (this recreates the new surface for this device).
	return ptcTexture->Restore(pd3dDevice);
}




//-----------------------------------------------------------------------------
// Name: D3DTextr_RestoreAllTextures()
// Desc: This function is called when a mode is changed. It updates all
//       texture objects to be valid with the new device.
//-----------------------------------------------------------------------------
HRESULT D3DTextr_RestoreAllTextures(LPDIRECT3DDEVICE7 pd3dDevice)
{
	TextureContainer * ptcTexture = g_ptcTextureList;

	while (ptcTexture)
	{
		D3DTextr_Restore(ptcTexture->m_strName, pd3dDevice);
		ptcTexture = ptcTexture->m_pNext;
	}

	return S_OK;
}

HRESULT D3DTextr_TESTRestoreAllTextures(LPDIRECT3DDEVICE7 pd3dDevice)
{
	TextureContainer * ptcTexture = g_ptcTextureList;

	while (ptcTexture)
	{
		if (!ptcTexture->m_pddsSurface)
			D3DTextr_Restore(ptcTexture->m_strName, pd3dDevice);

		ptcTexture = ptcTexture->m_pNext;
	}

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: D3DTextr_InvalidateAllTextures()
// Desc: This function is called when a mode is changed. It invalidates
//       all texture objects so their device can be safely released.
//-----------------------------------------------------------------------------
HRESULT D3DTextr_InvalidateAllTextures()
{
	TextureContainer * ptcTexture = g_ptcTextureList;

	while (ptcTexture)
	{
		if (ptcTexture->m_pddsSurface)
		{
			ptcTexture->m_pddsSurface->DeleteAttachedSurface(0, NULL);
			ptcTexture->m_pddsSurface->Release();
			ptcTexture->m_pddsSurface = NULL;
		}

		if (ptcTexture->m_pddsBumpMap)
		{
			ptcTexture->m_pddsBumpMap->Release();
			ptcTexture->m_pddsBumpMap = NULL;
		}

		ptcTexture = ptcTexture->m_pNext;
	}

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: D3DTextr_DestroyTexture()
// Desc: Frees the resources for the specified texture container
//-----------------------------------------------------------------------------

/*
	Detruit une surface chargée
	seb
*/
void D3DTextr_KillTexture(TextureContainer * tex)
{
	D3DTextr_DestroyContainer(tex);
	tex = NULL;
}

/*
	Detruit toutes les surfaces chargées
*/
void D3DTextr_KillAllTextures()
{
	while (g_ptcTextureList)
	{
		D3DTextr_KillTexture(g_ptcTextureList);
	}
}


//-----------------------------------------------------------------------------
// Name: D3DTextr_DestroyContainer()
// Desc: Frees the resources for the specified texture container
//-----------------------------------------------------------------------------
HRESULT D3DTextr_DestroyContainer(TextureContainer * ptcTexture)
{
	SAFE_DELETE(ptcTexture);

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: D3DTextr_GetSurface()
// Desc: Returns a pointer to a d3dSurface from the name of the texture
//-----------------------------------------------------------------------------

TextureContainer * D3DTextr_GetSurfaceContainer(TCHAR * strName)
{
	MakeUpcase(strName);
	TextureContainer * ptcTexture = FindTexture(strName);
	return ptcTexture;
}

//-----------------------------------------------------------------------------
// Name: LoadJPEGFileNoDeComp()
// Desc: Loads a .jpeg file, and stores it in allocated memory
//       for the specified texture container
//-----------------------------------------------------------------------------
HRESULT TextureContainer::LoadJpegFileNoDecomp(TCHAR * strPathname)
{

	if (!PAK_FileExist(strPathname))
		return E_FAIL;

	long	taille;
	unsigned char * memjpeg = (unsigned char *)PAK_FileLoadMalloc(strPathname, &taille);

	if (!memjpeg)
		return E_FAIL;

	m_pJPEGData = new unsigned char[sizeof(struct	jpeg_decompress_struct)+sizeof(struct jpeg_error_mgr)];

	if (!m_pJPEGData)
	{
		free(memjpeg);
		return E_FAIL;
	}

	struct jpeg_decompress_struct * cinfo = (struct jpeg_decompress_struct *)m_pJPEGData;

	struct jpeg_error_mgr * jerr = (struct jpeg_error_mgr *)(m_pJPEGData + sizeof(struct jpeg_decompress_struct));

	cinfo->err = jpeg_std_error(jerr);

	if (JPEGError)
	{
		JPEGError = 0;
		delete memjpeg;
		delete[] m_pJPEGData;
		m_pJPEGData = NULL;
		return E_FAIL;
	}

	jerr->error_exit = my_error_exit;

	jpeg_create_decompress(cinfo);

	if (JPEGError)
	{
		JPEGError = 0;
		free(memjpeg);
		delete[] m_pJPEGData;
		m_pJPEGData = NULL;
		return E_FAIL;
	}

	jpeg_mem_src(cinfo, (char *)memjpeg, taille);

	if (JPEGError)
	{
		JPEGError = 0;
		jpeg_destroy_decompress(cinfo);
		free(memjpeg);
		delete[] m_pJPEGData;
		m_pJPEGData = NULL;
		return E_FAIL;
	}

	jpeg_read_header(cinfo, TRUE);

	if (JPEGError)
	{
		JPEGError = 0;
		jpeg_destroy_decompress(cinfo);
		free(memjpeg);
		delete[] m_pJPEGData;
		m_pJPEGData = NULL;
		return E_FAIL;
	}

	m_dwWidth = cinfo->image_width;
	m_dwHeight = cinfo->image_height;
	m_dwBPP = 24;
	m_pJPEGData_ex = memjpeg;
	(void)jpeg_mem_reinitsrc((void *)cinfo);

	return S_OK;
}
/*-----------------------------------------------------------------------------*/
/*
	<< Datas du PNG en memoire;
	>> PNG_OK si format valide sinon PNG_ERROR
*/
int	Read_PNG_Signature(void)
{
	int	* dat;

	dat = (int *)PNGDatas;

	if (*dat++ != 0x474e5089) return PNG_ERROR;

	if (*dat++ != 0x0a1a0a0d) return PNG_ERROR;

	PNGDatas = (char *)dat;
	return PNG_OK;
}
/*-----------------------------------------------------------------------------*/
/*
	un chunk est composé de:
	4 byte:taille datas
	4 byte:type
	des datas
	4 byte:CRC(a testé peut etre)

	<< Datas PNG en memoire.
	>> Type du chunk, taille du chunk, et son CRC
*/
void Read_PNG_Chunk(void)
{
	unsigned int	* dat;

	dat = (unsigned int *)PNGDatas;
	ChunkTaille = *dat++;
	ChunkType = *dat++;
	PNGDatas = (char *)dat;

	InverseLong((int *)&ChunkTaille);
	dat = (unsigned int *)(PNGDatas + ChunkTaille);
	ChunkCRC = *dat;
}
/*-----------------------------------------------------------------------------*/
/*
	<< Datas PNG en memoire.
	>> Header crepie+code erreur
*/
int Read_PNG_Header(DATAS_PNG * dpng)
{
	PNG_IHDR * pngh = dpng->pngh = (PNG_IHDR *)PNGDatas;

	if ((!pngh->width)OR(!pngh->height)OR(pngh->interlace)) return PNG_ERROR;

	InverseLong((int *)&pngh->width);
	InverseLong((int *)&pngh->height);
	PNGDatas += ChunkTaille + 4;

	switch (pngh->colortype)
	{
		case GRAYSCALE_USED:
				dpng->pngbpp = pngh->bitdepth >> 3;

			if (!dpng->pngbpp) dpng->pngbpp = 1;

			break;
		case COLOR_USED:
				dpng->pngbpp = (pngh->bitdepth >> 3) * 3;
			break;
		case COLOR_USED|PALETTE_USED:
				dpng->pngbpp = (pngh->bitdepth >> 3);

			if (!dpng->pngbpp) dpng->pngbpp = 1;

			dpng->pngbpp *= 3;
			break;
		case ALPHA_USED:
				dpng->pngbpp = (pngh->bitdepth >> 3) * 2;
			break;
		case COLOR_USED|ALPHA_USED:
				dpng->pngbpp = (pngh->bitdepth >> 3) * 4;
			break;
		default:
				return PNG_ERROR;
			break;
	}

	return PNG_OK;
}
/*-----------------------------------------------------------------------------*/
/*
	<< Datas PNG en memoire.
*/
void Read_PNG_Palette(DATAS_PNG * dpng)
{
	dpng->pngpalette = (PNG_PALETTE_ARXX *)PNGDatas;
	PNGDatas += ChunkTaille + 4;
}
/*-----------------------------------------------------------------------------*/
/*
	<< Datas PNG en memoire.
	>> datas ptr init
*/
void Read_PNG_Datas(DATAS_PNG * dpng)
{
	dpng->pnglzwdatas = (void *)PNGDatas;
	dpng->pnglzwtaille = ChunkTaille;
	PNGDatas += ChunkTaille + 4;
}
/*-----------------------------------------------------------------------------*/
/*
	<< Datas PNG en memoire.
	>> Datas Decompresse.
*/
void * PNG_Inflate(DATAS_PNG * dpng)
{
	void	*	mem;
	int			err, tdecomp;
	z_stream	d_stream;	//stream de decompression

	d_stream.zalloc = (alloc_func)0;
	d_stream.zfree = (free_func)0;
	d_stream.opaque = (voidpf)0;

	d_stream.next_in = (Bytef *)dpng->pnglzwdatas;
	d_stream.avail_in = (uInt)dpng->pnglzwtaille;

	err = inflateInit(&d_stream);

	if (err != Z_OK) return NULL;

	PNG_IHDR * pngh = dpng->pngh;

	tdecomp = ((pngh->width + 1) * pngh->height * dpng->pngbpp);
	mem = (void *)new char[tdecomp];

	if (!mem) return NULL;

	while (1)
	{
		d_stream.next_out = (Bytef *)mem;
		d_stream.avail_out = (uInt)tdecomp;
		err = inflate(&d_stream, Z_NO_FLUSH);

		if (err == Z_STREAM_END) break;
	}

	err = inflateEnd(&d_stream);

	if (err != Z_OK)
	{
		free(mem);
		mem = NULL;
		return NULL;
	}

	return mem;
}
/*-----------------------------------------------------------------------------*/
int Decomp_PNG(void * mems, DATAS_PNG * dpng)
{
	int	noerr = 1, stop = 1;

	PNGDatas = (char *)mems;
	dpng->pngpalette = NULL;
	dpng->pnglzwdatas = NULL;

	if (!Read_PNG_Signature()) return PNG_ERROR;

	while ((noerr)AND(stop))
	{
		Read_PNG_Chunk();

		switch (ChunkType)
		{
			case PNG_HEADER:
					noerr = Read_PNG_Header(dpng);
				break;
			case PNG_PALETTE:
					Read_PNG_Palette(dpng);
				break;
			case PNG_DATAS:
					Read_PNG_Datas(dpng);
				break;
			case PNG_END:
					stop = 0;
				break;
			default:
					PNGDatas += ChunkTaille + 4;
				break;
		}
	}

	if (!noerr) return PNG_ERROR;

	return PNG_OK;
}
//-----------------------------------------------------------------------------
// Name: LoadPNGFile()
// Desc: Loads a .jpeg file, and stores it in allocated memory
//       for the specified texture container, no interlace required
//-----------------------------------------------------------------------------
HRESULT TextureContainer::LoadPNGFile(TCHAR * strPathname)
{
	int	taille;

	FILE * file = fopen(strPathname, "rb");

	if (NULL == file) return E_FAIL;

	fseek(file, 0, SEEK_END);
	m_pPNGData = new unsigned char[(taille=ftell(file))+sizeof(DATAS_PNG)];

	if (!m_pPNGData)
	{
		fclose(file);
		return E_FAIL;
	}

	fseek(file, 0, SEEK_SET);

	char * mempng = (char *)(m_pPNGData + sizeof(DATAS_PNG));
	fread((void *)mempng, 1, taille, file);
	fclose(file);

	if (Decomp_PNG((void *)mempng, (DATAS_PNG *)m_pPNGData) == PNG_ERROR)
	{
		delete m_pPNGData;
		m_pPNGData = NULL;
		return E_FAIL;
	}

	PNG_IHDR * pngh = (PNG_IHDR *)(((DATAS_PNG *)m_pPNGData)->pngh);

	if (pngh->colortype & ALPHA_USED)
	{
		m_bHasAlpha = TRUE;
	}

	m_dwWidth = pngh->width;
	m_dwHeight = pngh->height;
	m_dwBPP = 24;

	return S_OK;
}
/*-----------------------------------------------------------------------------*/
char * FilteringNone(char * mem, char ** memd, DATAS_PNG * dpng)
{
	memcpy((void *)*memd, (const void *)mem, dpng->pngmodulos);
	*memd += dpng->pngmodulos;

	return (mem + dpng->pngmodulos);
}
/*-----------------------------------------------------------------------------*/
char * FilteringSub(unsigned char * mem, char ** memd, DATAS_PNG * dpng)
{
	unsigned char	* ms, *md, *mddep;
	int				dx;

	ms = mem;
	md = (unsigned char *) * memd;
	mddep = md;

	dx = dpng->pngbpp;

	while (dx)
	{
		*md++ = *ms++;
		dx--;
	}

	dx = dpng->pngmodulos - dpng->pngbpp;

	while (dx)
	{
		*md++ = (*ms++) + (*mddep++);
		dx--;
	}


	*memd = (char *)md;
	return (char *)ms;
}
/*-----------------------------------------------------------------------------*/
char * FilteringUp(unsigned char * mem, char ** memd, int flg, DATAS_PNG * dpng)
{
	unsigned char	* md, *ms, *mdm;
	int				dx;

	md = (unsigned char *) * memd;
	ms = mem;

	if (flg)
	{
		dx = dpng->pngmodulos;

		while (dx)
		{
			*md++ = *ms++;
			dx--;
		}
	}
	else
	{
		mdm = md - dpng->pngmodulos;
		dx = dpng->pngmodulos;

		while (dx)
		{
			*md++ = (*ms++) + (*mdm++);
			dx--;
		}
	}


	*memd = (char *)md;
	return (char *)ms;
}
/*-----------------------------------------------------------------------------*/
char * FilteringAverage(char * mem, char ** memd, int first, DATAS_PNG * dpng)
{
	unsigned char	* md, *ms, *mdm, *mddep;
	int				dx;

	md = (unsigned char *) * memd;
	mddep = md;
	ms = (unsigned char *)mem;

	if (first)
	{
		dx = dpng->pngbpp;

		while (dx--)
		{
			*md++ = *ms++;
		}

		dx = dpng->pngmodulos - dpng->pngbpp;

		while (dx--)
		{
			*md++ = (*ms++) + ((*mddep++) >> 1);
		}
	}
	else
	{
		mdm = md - dpng->pngmodulos;
		dx = dpng->pngbpp;

		while (dx)
		{
			*md++ = (*ms++) + ((*mdm++) >> 1);
			dx--;
		}

		dx = dpng->pngmodulos - dpng->pngbpp;

		while (dx)
		{
			*md++ = (*ms++) + (((*mdm++) + (*mddep++)) >> 1);
			dx--;
		}
	}

	*memd = (char *)md;
	return (char *)ms;
}
/*-----------------------------------------------------------------------------*/
unsigned char PaethPredictor(unsigned char a, unsigned char b, unsigned char c)
{
	int p, pa, pb, pc;

	p = a + b - c;
	pa = abs(p - a);
	pb = abs(p - b);
	pc = abs(p - c);

	if ((pa <= pb)AND(pa <= pc))
	{
		return a;
	}
	else
	{
		if (pb <= pc)
		{
			return b;
		}
		else
		{
			return c;
		}
	}
}
/*-----------------------------------------------------------------------------*/
char * FilteringPaeth(unsigned char * mem, char ** memd, int first, DATAS_PNG * dpng)
{
	char	* md, *ms, *mddep, *mdm, *mdmdep;
	int		dx;

	md = *memd;
	mddep = md;
	ms = (char *)mem;

	if (first)
	{
		dx = dpng->pngbpp;

		while (dx)
		{
			*md++ = *ms++;
			dx--;
		}

		dx = dpng->pngmodulos - dpng->pngbpp;

		while (dx)
		{
			*md++ = (*ms++) + PaethPredictor(*mddep, 0, 0);
			mddep++;
			dx--;
		}
	}
	else
	{
		mdm = md - dpng->pngmodulos;
		mdmdep = mdm;
		dx = dpng->pngbpp;

		while (dx)
		{
			*md++ = (*ms++) + PaethPredictor(0, *mdm, 0);
			mdm++;
			dx--;
		}

		dx = dpng->pngmodulos - dpng->pngbpp;

		while (dx)
		{
			*md++ = (*ms++) + PaethPredictor(*mddep, *mdm, *mdmdep);
			mddep++;
			mdmdep++;
			mdm++;
			dx--;
		}
	}

	*memd = md;
	return ms;
}
/*-----------------------------------------------------------------------------*/
HRESULT TextureContainer::CopyPNGDataToSurface(LPDIRECTDRAWSURFACE7 Surface)
{
	int				dx, dy, t, dec;
	char		*	memd, *memdc;
	unsigned char	cc;
	unsigned short	* memdcw;
	char			filtre, *mt;

	// Get a DDraw object to create a temporary surface
	LPDIRECTDRAW7 pDD;
	Surface->GetDDInterface((VOID **)&pDD);

	// Setup the new surface desc
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	Surface->GetSurfaceDesc(&ddsd);
	float fWidthSurface = (float)ddsd.dwWidth;
	float fHeightSurface = (float)ddsd.dwHeight;
	ddsd.dwFlags         = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT |
	                       DDSD_TEXTURESTAGE;
	ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;
	ddsd.ddsCaps.dwCaps2 = 0L;
	ddsd.dwWidth         = m_dwWidth;
	ddsd.dwHeight        = m_dwHeight;

	// Create a new surface for the texture
	LPDIRECTDRAWSURFACE7 pddsTempSurface;
	HRESULT hr;

	if (FAILED(hr = pDD->CreateSurface(&ddsd, &pddsTempSurface, NULL)))
	{
		pDD->Release();
		return NULL;
	}

	while (pddsTempSurface->Lock(NULL, &ddsd, 0, 0) == DDERR_WASSTILLDRAWING);

	BYTE * pBytes = (BYTE *)ddsd.lpSurface;

	DWORD dwRMask = ddsd.ddpfPixelFormat.dwRBitMask;
	DWORD dwGMask = ddsd.ddpfPixelFormat.dwGBitMask;
	DWORD dwBMask = ddsd.ddpfPixelFormat.dwBBitMask;
	DWORD dwAMask = ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;

	DWORD dwRShiftL = 8, dwRShiftR = 0;
	DWORD dwGShiftL = 8, dwGShiftR = 0;
	DWORD dwBShiftL = 8, dwBShiftR = 0;
	DWORD dwAShiftL = 8, dwAShiftR = 0;

	DWORD dwMask;

	for (dwMask = dwRMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwRShiftR++;

	for (; dwMask; dwMask >>= 1) dwRShiftL--;

	for (dwMask = dwGMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwGShiftR++;

	for (; dwMask; dwMask >>= 1) dwGShiftL--;

	for (dwMask = dwBMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwBShiftR++;

	for (; dwMask; dwMask >>= 1) dwBShiftL--;

	for (dwMask = dwAMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwAShiftR++;

	for (; dwMask; dwMask >>= 1) dwAShiftL--;

	DATAS_PNG * dpng = (DATAS_PNG *)m_pPNGData;
	PNG_IHDR * pngh = dpng->pngh;

	switch (pngh->colortype)
	{
		case GRAYSCALE_USED:
				dec = (pngh->bitdepth >> 3);

			if (!dec) dec = 1;

			t = (pngh->width * pngh->height) * dec;
			break;
		case COLOR_USED:
				dec = (pngh->bitdepth >> 3) * 3;
			t = (pngh->width * pngh->height) * dec;
			break;
		case COLOR_USED|PALETTE_USED:
				dec = (pngh->bitdepth >> 3);

			if (!dec) dec = 1;

			t = (pngh->width * pngh->height) * dec;
			break;
		case ALPHA_USED:
				dec = (pngh->bitdepth >> 3) * 2;
			t = (pngh->width * pngh->height) * dec;
			break;
		case COLOR_USED|ALPHA_USED:
				dec = (pngh->bitdepth >> 3) * 4;
			t = (pngh->width * pngh->height) * dec;
			break;
		default:
				pDD->Release();
			return NULL;
			break;
	}

	memd = new char[t];

	if (!memd)
	{
		pDD->Release();
		return NULL;
	}

	//depak
	dpng->pngmodulos = dec * pngh->width;

	char * mems = (char *)PNG_Inflate(dpng);
	mt = mems;
	memdc = memd;
	dy = pngh->height;

	while (dy)
	{
		filtre = *mt++;

		switch (filtre)
		{
			case PNG_FNONE:
					mt = FilteringNone(mt, &memdc, dpng);
				break;
			case PNG_FSUB:
					mt = FilteringSub((unsigned char *)mt, &memdc, dpng);
				break;
			case PNG_FUP:
					mt = FilteringUp((unsigned char *)mt, &memdc, (dy == (int)pngh->height) ? 1 : 0, dpng);
				break;
			case PNG_FAVERAGE:
					mt = FilteringAverage(mt, &memdc, (dy == (int)pngh->height) ? 1 : 0, dpng);
				break;
			case PNG_FPAETH:
					mt = FilteringPaeth((unsigned char *)mt, &memdc, (dy == (int)pngh->height) ? 1 : 0, dpng);
				break;
			default:
					delete memd;
				pDD->Release();
				return NULL;
				break;
		}

		dy--;
	}

	//convertion format RGBA 32 bits
	switch (pngh->colortype)
	{
		case GRAYSCALE_USED:

				switch (pngh->bitdepth)		//TO DO:1/2/4
				{
					case 8:
							memdc = memd;
						dy = m_dwHeight;

						while (dy)
						{
							DWORD * pDstData32 = (DWORD *)pBytes;
							WORD * pDstData16 = (WORD *)pBytes;

							dx = m_dwWidth;

							while (dx)
							{
								cc = *memdc++;
								BYTE r = (BYTE)cc;
								BYTE g = (BYTE)cc;
								BYTE b = (BYTE)cc;
								BYTE a = 0xFF;

								if ((r != 0) && (r < 15)) r = 15;

								if ((g != 0) && (g < 15)) g = 15;

								if ((b != 0) && (b < 15)) b = 15;

								DWORD dr = ((r >> (dwRShiftL)) << dwRShiftR)&dwRMask;
								DWORD dg = ((g >> (dwGShiftL)) << dwGShiftR)&dwGMask;
								DWORD db = ((b >> (dwBShiftL)) << dwBShiftR)&dwBMask;
								DWORD da = ((a >> (dwAShiftL)) << dwAShiftR)&dwAMask;

								if (32 == ddsd.ddpfPixelFormat.dwRGBBitCount)
									*pDstData32++ = (DWORD)(dr + dg + db + da);
								else
									*pDstData16++ = (WORD)(dr + dg + db + da);

								dx--;
							}

							pBytes += ddsd.lPitch;
							dy--;
						}

						break;
					case 16:
							memdcw = (unsigned short *)memd;
						dy = m_dwHeight;

						while (dy)
						{
							DWORD * pDstData32 = (DWORD *)pBytes;
							WORD * pDstData16 = (WORD *)pBytes;

							dx = m_dwWidth;

							while (dx)
							{
								cc = ARX_CLEAN_WARN_CAST_UCHAR((*memdcw++) >> 8);
								BYTE r = (BYTE)cc;
								BYTE g = (BYTE)cc;
								BYTE b = (BYTE)cc;
								BYTE a = 0xFF;

								if ((r != 0) && (r < 15)) r = 15;

								if ((g != 0) && (g < 15)) g = 15;

								if ((b != 0) && (b < 15)) b = 15;

								DWORD dr = ((r >> (dwRShiftL)) << dwRShiftR)&dwRMask;
								DWORD dg = ((g >> (dwGShiftL)) << dwGShiftR)&dwGMask;
								DWORD db = ((b >> (dwBShiftL)) << dwBShiftR)&dwBMask;
								DWORD da = ((a >> (dwAShiftL)) << dwAShiftR)&dwAMask;

								if (32 == ddsd.ddpfPixelFormat.dwRGBBitCount)
									*pDstData32++ = (DWORD)(dr + dg + db + da);
								else
									*pDstData16++ = (WORD)(dr + dg + db + da);

								dx--;
							}

							pBytes += ddsd.lPitch;
							dy--;
						}

						break;
				}

			break;
		case COLOR_USED:

				switch (pngh->bitdepth)
				{
					case 8:
							memdc = memd;
						dy = m_dwHeight;

						while (dy)
						{
							DWORD * pDstData32 = (DWORD *)pBytes;
							WORD * pDstData16 = (WORD *)pBytes;

							dx = m_dwWidth;

							while (dx)
							{
								BYTE r = (BYTE) * memdc++;
								BYTE g = (BYTE) * memdc++;
								BYTE b = (BYTE) * memdc++;
								BYTE a = 0xFF;

								if ((r != 0) && (r < 15)) r = 15;

								if ((g != 0) && (g < 15)) g = 15;

								if ((b != 0) && (b < 15)) b = 15;

								DWORD dr = ((r >> (dwRShiftL)) << dwRShiftR)&dwRMask;
								DWORD dg = ((g >> (dwGShiftL)) << dwGShiftR)&dwGMask;
								DWORD db = ((b >> (dwBShiftL)) << dwBShiftR)&dwBMask;
								DWORD da = ((a >> (dwAShiftL)) << dwAShiftR)&dwAMask;

								if (32 == ddsd.ddpfPixelFormat.dwRGBBitCount)
									*pDstData32++ = (DWORD)(dr + dg + db + da);
								else
									*pDstData16++ = (WORD)(dr + dg + db + da);

								dx--;
							}

							pBytes += ddsd.lPitch;
							dy--;
						}

						break;
					case 16:
							memdcw = (unsigned short *)memd;
						dy = m_dwHeight;

						while (dy)
						{
							DWORD * pDstData32 = (DWORD *)pBytes;
							WORD * pDstData16 = (WORD *)pBytes;

							dx = m_dwWidth;

							while (dx)
							{
								BYTE r = (BYTE)((*memdcw++) >> 8);
								BYTE g = (BYTE)((*memdcw++) >> 8);
								BYTE b = (BYTE)((*memdcw++) >> 8);
								BYTE a = 0xFF;

								if ((r != 0) && (r < 15)) r = 15;

								if ((g != 0) && (g < 15)) g = 15;

								if ((b != 0) && (b < 15)) b = 15;

								DWORD dr = ((r >> (dwRShiftL)) << dwRShiftR)&dwRMask;
								DWORD dg = ((g >> (dwGShiftL)) << dwGShiftR)&dwGMask;
								DWORD db = ((b >> (dwBShiftL)) << dwBShiftR)&dwBMask;
								DWORD da = ((a >> (dwAShiftL)) << dwAShiftR)&dwAMask;

								if (32 == ddsd.ddpfPixelFormat.dwRGBBitCount)
									*pDstData32++ = (DWORD)(dr + dg + db + da);
								else
									*pDstData16++ = (WORD)(dr + dg + db + da);

								dx--;
							}

							pBytes += ddsd.lPitch;
							dy--;
						}

						break;
				}

			break;
		case COLOR_USED|PALETTE_USED:

				switch (pngh->bitdepth)	//TO_DO 1/2/4
				{
					case 8:
							memdc = memd;
						dy = m_dwHeight;

						while (dy)
						{
							DWORD * pDstData32 = (DWORD *)pBytes;
							WORD * pDstData16 = (WORD *)pBytes;

							dx = m_dwWidth;

							while (dx)
							{
								cc = *memdc++;
								BYTE r = (BYTE)dpng->pngpalette[cc].r;
								BYTE g = (BYTE)dpng->pngpalette[cc].g;
								BYTE b = (BYTE)dpng->pngpalette[cc].b;
								BYTE a = 0xFF;

								if ((r != 0) && (r < 15)) r = 15;

								if ((g != 0) && (g < 15)) g = 15;

								if ((b != 0) && (b < 15)) b = 15;

								DWORD dr = ((r >> (dwRShiftL)) << dwRShiftR)&dwRMask;
								DWORD dg = ((g >> (dwGShiftL)) << dwGShiftR)&dwGMask;
								DWORD db = ((b >> (dwBShiftL)) << dwBShiftR)&dwBMask;
								DWORD da = ((a >> (dwAShiftL)) << dwAShiftR)&dwAMask;

								if (32 == ddsd.ddpfPixelFormat.dwRGBBitCount)
									*pDstData32++ = (DWORD)(dr + dg + db + da);
								else
									*pDstData16++ = (WORD)(dr + dg + db + da);

								dx--;
							}

							pBytes += ddsd.lPitch;
							dy--;
						}

						break;
				}

			break;
		case ALPHA_USED:

				switch (pngh->bitdepth)
				{
					case 8:
							memdc = memd;
						dy = m_dwHeight;

						while (dy)
						{
							DWORD * pDstData32 = (DWORD *)pBytes;
							WORD * pDstData16 = (WORD *)pBytes;

							dx = m_dwWidth;

							while (dx)
							{
								cc = *memdc++;
								BYTE r = (BYTE)cc;
								BYTE g = (BYTE)cc;
								BYTE b = (BYTE)cc;
								BYTE a = (BYTE) * memdc++;

								if ((r != 0) && (r < 15)) r = 15;

								if ((g != 0) && (g < 15)) g = 15;

								if ((b != 0) && (b < 15)) b = 15;

								DWORD dr = ((r >> (dwRShiftL)) << dwRShiftR)&dwRMask;
								DWORD dg = ((g >> (dwGShiftL)) << dwGShiftR)&dwGMask;
								DWORD db = ((b >> (dwBShiftL)) << dwBShiftR)&dwBMask;
								DWORD da = ((a >> (dwAShiftL)) << dwAShiftR)&dwAMask;

								if (32 == ddsd.ddpfPixelFormat.dwRGBBitCount)
									*pDstData32++ = (DWORD)(dr + dg + db + da);
								else
									*pDstData16++ = (WORD)(dr + dg + db + da);

								dx--;
							}

							pBytes += ddsd.lPitch;
							dy--;
						}

						break;
					case 16:
							memdcw = (unsigned short *)memd;
						dy = m_dwHeight;

						while (dy)
						{
							DWORD * pDstData32 = (DWORD *)pBytes;
							WORD * pDstData16 = (WORD *)pBytes;

							dx = m_dwWidth;

							while (dx)
							{
								cc = ARX_CLEAN_WARN_CAST_UCHAR((*memdcw++) >> 8);
								BYTE r = (BYTE)cc;
								BYTE g = (BYTE)cc;
								BYTE b = (BYTE)cc;
								BYTE a = (BYTE)((*memdcw++) >> 8);

								if ((r != 0) && (r < 15)) r = 15;

								if ((g != 0) && (g < 15)) g = 15;

								if ((b != 0) && (b < 15)) b = 15;

								DWORD dr = ((r >> (dwRShiftL)) << dwRShiftR)&dwRMask;
								DWORD dg = ((g >> (dwGShiftL)) << dwGShiftR)&dwGMask;
								DWORD db = ((b >> (dwBShiftL)) << dwBShiftR)&dwBMask;
								DWORD da = ((a >> (dwAShiftL)) << dwAShiftR)&dwAMask;

								if (32 == ddsd.ddpfPixelFormat.dwRGBBitCount)
									*pDstData32++ = (DWORD)(dr + dg + db + da);
								else
									*pDstData16++ = (WORD)(dr + dg + db + da);

								dx--;
							}

							pBytes += ddsd.lPitch;
							dy--;
						}

						break;
				}

			break;
		case COLOR_USED|ALPHA_USED:

				switch (pngh->bitdepth)
				{
					case 8:
							memdc = memd;
						dy = m_dwHeight;

						while (dy)
						{
							DWORD * pDstData32 = (DWORD *)pBytes;
							WORD * pDstData16 = (WORD *)pBytes;

							dx = m_dwWidth;

							while (dx)
							{
								BYTE r = (BYTE) * memdc++;
								BYTE g = (BYTE) * memdc++;
								BYTE b = (BYTE) * memdc++;
								BYTE a = (BYTE) * memdc++;

								if ((r != 0) && (r < 15)) r = 15;

								if ((g != 0) && (g < 15)) g = 15;

								if ((b != 0) && (b < 15)) b = 15;

								DWORD dr = ((r >> (dwRShiftL)) << dwRShiftR)&dwRMask;
								DWORD dg = ((g >> (dwGShiftL)) << dwGShiftR)&dwGMask;
								DWORD db = ((b >> (dwBShiftL)) << dwBShiftR)&dwBMask;
								DWORD da = ((a >> (dwAShiftL)) << dwAShiftR)&dwAMask;

								if (32 == ddsd.ddpfPixelFormat.dwRGBBitCount)
									*pDstData32++ = (DWORD)(dr + dg + db + da);
								else
									*pDstData16++ = (WORD)(dr + dg + db + da);

								dx--;
							}

							pBytes += ddsd.lPitch;
							dy--;
						}

						break;
					case 16:
							memdcw = (unsigned short *)memd;
						dy = m_dwHeight;

						while (dy)
						{
							DWORD * pDstData32 = (DWORD *)pBytes;
							WORD * pDstData16 = (WORD *)pBytes;

							dx = m_dwWidth;

							while (dx)
							{
								BYTE r = (BYTE)((*memdcw++) >> 8);
								BYTE g = (BYTE)((*memdcw++) >> 8);
								BYTE b = (BYTE)((*memdcw++) >> 8);
								BYTE a = (BYTE)((*memdcw++) >> 8);

								if ((r != 0) && (r < 15)) r = 15;

								if ((g != 0) && (g < 15)) g = 15;

								if ((b != 0) && (b < 15)) b = 15;

								DWORD dr = ((r >> (dwRShiftL)) << dwRShiftR)&dwRMask;
								DWORD dg = ((g >> (dwGShiftL)) << dwGShiftR)&dwGMask;
								DWORD db = ((b >> (dwBShiftL)) << dwBShiftR)&dwBMask;
								DWORD da = ((a >> (dwAShiftL)) << dwAShiftR)&dwAMask;

								if (32 == ddsd.ddpfPixelFormat.dwRGBBitCount)
									*pDstData32++ = (DWORD)(dr + dg + db + da);
								else
									*pDstData16++ = (WORD)(dr + dg + db + da);

								dx--;
							}

							pBytes += ddsd.lPitch;
							dy--;
						}

						break;
				}

			break;
	}

	delete mems;
	delete memd;

	pddsTempSurface->Unlock(0);

	// Copy the temp surface to the real texture surface
	if (bGlobalTextureStretch)
	{
		m_dx = .999999f;
		m_dy = .999999f;
		Surface->Blt(NULL, pddsTempSurface, NULL, DDBLT_WAIT, NULL);
	}
	else
	{
		RECT rBlt;
		SetRect(&rBlt, 0, 0, ddsd.dwWidth, ddsd.dwHeight);

		if (ddsd.dwWidth < fWidthSurface)
		{
			m_dx = ((float)ddsd.dwWidth) / fWidthSurface;
		}
		else
		{
			rBlt.right = (int)fWidthSurface;
			m_dx = .999999f;
		}

		if (ddsd.dwHeight < fHeightSurface)
		{
			m_dy = ((float)ddsd.dwHeight) / fHeightSurface;
		}
		else
		{
			rBlt.bottom = (int)fHeightSurface;
			m_dy = 0.999999f;
		}

		Surface->Blt(&rBlt, pddsTempSurface, NULL, DDBLT_WAIT, NULL);
	}

	// Done with the temp objects
	pddsTempSurface->Release();
	pDD->Release();

	return S_OK;
}

//*************************************************************************************
//*************************************************************************************
TextureContainer * GetTextureFile(char * tex, long flag)
{
	char dir[256];
	char dir2[256];

	long old = GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE;
	GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE = -1;
	sprintf(dir2, "%s%s", Project.workingdir, tex);
	File_Standardize(dir2, dir);
	TextureContainer * returnTc;

	if (flag & 1)
		returnTc = D3DTextr_CreateTextureFromFile(dir, Project.workingdir, 0, D3DTEXTR_32BITSPERPIXEL);
	else
		returnTc = D3DTextr_CreateTextureFromFile(dir, Project.workingdir, 0, D3DTEXTR_NO_MIPMAP);

	GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE = old;
	return returnTc;
}
//*************************************************************************************
//*************************************************************************************
TextureContainer * GetTextureFile_NoRefinement(char * tex, long flag)
{
	char dir[256];
	char dir2[256];

	long old = GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE;
	GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE = -1;
	sprintf(dir2, "%s%s", Project.workingdir, tex);
	File_Standardize(dir2, dir);
	TextureContainer * returnTc;

	if (flag & 1)
		returnTc = D3DTextr_CreateTextureFromFile(dir, Project.workingdir, 0, D3DTEXTR_32BITSPERPIXEL | D3DTEXTR_NO_REFINEMENT);
	else
		returnTc = D3DTextr_CreateTextureFromFile(dir, Project.workingdir, 0, D3DTEXTR_NO_MIPMAP | D3DTEXTR_NO_REFINEMENT);

	GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE = old;
	return returnTc;
}
//*************************************************************************************
//*************************************************************************************
bool TextureContainer::CreateHalo(LPDIRECT3DDEVICE7 _lpDevice)
{
	if (this->TextureHalo) return true; 

	this->TextureHalo = this->AddHalo(_lpDevice, 8, 255.f, 255.f, 255.f, this->halodecalX, this->halodecalY);

	if (this->TextureHalo) return true;

	return false;
}

//-----------------------------------------------------------------------------
TextureContainer * TextureContainer::AddHalo(LPDIRECT3DDEVICE7 _lpDevice, int _iNbCouche, float _fR, float _fG, float _fB, float & _iDecalX, float & _iDecalY)
{
	if ((!m_pddsSurface) ||
	        (!_iNbCouche)) return NULL;

	iHaloNbCouche = _iNbCouche;
	fHaloRed = _fR;
	fHaloGreen = _fG;
	fHaloBlue = _fB;

	D3DDEVICEDESC7 ddDesc;

	if (FAILED(_lpDevice->GetCaps(&ddDesc))) return NULL;

	LPDIRECTDRAW7 pDD;

	if (FAILED(m_pddsSurface->GetDDInterface((VOID **)&pDD)))
	{
		return NULL;
	}


	float fDR = _fR / ((float)_iNbCouche);
	float fDG = _fG / ((float)_iNbCouche);
	float fDB = _fB / ((float)_iNbCouche);

	DDSURFACEDESC2 ddsdSurfaceDesc;
	ddsdSurfaceDesc.dwSize = sizeof(ddsdSurfaceDesc);
	m_pddsSurface->GetSurfaceDesc(&ddsdSurfaceDesc);

	ddsdSurfaceDesc.dwFlags          = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	ddsdSurfaceDesc.ddsCaps.dwCaps   = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	ddsdSurfaceDesc.ddsCaps.dwCaps2	= 0;

	DWORD dwRMask = ddsdSurfaceDesc.ddpfPixelFormat.dwRBitMask;
	DWORD dwGMask = ddsdSurfaceDesc.ddpfPixelFormat.dwGBitMask;
	DWORD dwBMask = ddsdSurfaceDesc.ddpfPixelFormat.dwBBitMask;
	DWORD dwAMask = ddsdSurfaceDesc.ddpfPixelFormat.dwRGBAlphaBitMask;
	DWORD dwRShiftL = 8, dwRShiftR = 0;
	DWORD dwGShiftL = 8, dwGShiftR = 0;
	DWORD dwBShiftL = 8, dwBShiftR = 0;
	DWORD dwAShiftL = 8, dwAShiftR = 0;
	DWORD dwMask;

	for (dwMask = dwRMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwRShiftR++;

	for (; dwMask; dwMask >>= 1) dwRShiftL--;

	for (dwMask = dwGMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwGShiftR++;

	for (; dwMask; dwMask >>= 1) dwGShiftL--;

	for (dwMask = dwBMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwBShiftR++;

	for (; dwMask; dwMask >>= 1) dwBShiftL--;

	for (dwMask = dwAMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwAShiftR++;

	for (; dwMask; dwMask >>= 1) dwAShiftL--;

	int iOldWidth	= ddsdSurfaceDesc.dwWidth;
	int iOldHeight	= ddsdSurfaceDesc.dwHeight;
	UINT iHaloWidth	= ddsdSurfaceDesc.dwWidth + _iNbCouche;
	UINT iHaloHeight = ddsdSurfaceDesc.dwHeight + _iNbCouche;

	if (ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_POW2)
	{
		for (ddsdSurfaceDesc.dwWidth  = 1 ; iHaloWidth > ddsdSurfaceDesc.dwWidth ; ddsdSurfaceDesc.dwWidth  <<= 1);

		for (ddsdSurfaceDesc.dwHeight = 1 ; iHaloHeight > ddsdSurfaceDesc.dwHeight ; ddsdSurfaceDesc.dwHeight <<= 1);
	}

	if (ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY)
	{
		if (ddsdSurfaceDesc.dwWidth > ddsdSurfaceDesc.dwHeight)
			ddsdSurfaceDesc.dwHeight = ddsdSurfaceDesc.dwWidth;
		else
			ddsdSurfaceDesc.dwWidth = ddsdSurfaceDesc.dwHeight;
	}

	iHaloWidth = ddsdSurfaceDesc.dwWidth;
	iHaloHeight = ddsdSurfaceDesc.dwHeight;

	LPDIRECTDRAWSURFACE7 pddsTempSurface;

	if (FAILED(pDD->CreateSurface(&ddsdSurfaceDesc, &pddsTempSurface, NULL)))
	{
		pDD->Release();
		return NULL;
	}

	_iDecalX = (float)((iHaloWidth - iOldWidth) >> 1);
	_iDecalY = (float)((iHaloHeight - iOldHeight) >> 1);

	if (FAILED(pddsTempSurface->BltFast((int)_iDecalX,
	                                    (int)_iDecalY,
	                                    m_pddsSurface,
	                                    NULL,
	                                    DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT)))
	{
		pddsTempSurface->Release();
		pDD->Release();
		return NULL;
	}

	ddsdSurfaceDesc.dwSize = sizeof(ddsdSurfaceDesc);

	if (FAILED(pddsTempSurface->Lock(NULL,
	                                 &ddsdSurfaceDesc,
	                                 DDLOCK_WRITEONLY | DDLOCK_WAIT,
	                                 NULL)))
	{
		pddsTempSurface->Release();
		pDD->Release();
		return NULL;
	}

	int iNb = ddsdSurfaceDesc.dwWidth * ddsdSurfaceDesc.dwHeight;
	int iNb2 = iNb;

	if (ddsdSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 32)
	{
		unsigned int uiColorMask;
		uiColorMask =	((((int)(255.f - _fR)) >> dwRShiftL) << dwRShiftR) |
		                ((((int)(255.f - _fG)) >> dwGShiftL) << dwGShiftR) |
		                ((((int)(255.f - _fB)) >> dwBShiftL) << dwBShiftR) |
		                (0xFF000000);

		unsigned int * puiMem = (unsigned int *)ddsdSurfaceDesc.lpSurface;

		while (iNb--)
		{
			if (*puiMem) *puiMem = uiColorMask;

			puiMem++;
		}

		char * pcMem;
		unsigned int uiColorCmp;
		unsigned int uiColorWrite;
		uiColorCmp = uiColorMask;

		while (_iNbCouche--)
		{
			uiColorWrite =	((((int)_fR) >> dwRShiftL) << dwRShiftR) |
			                ((((int)_fG) >> dwGShiftL) << dwGShiftR) |
			                ((((int)_fB) >> dwBShiftL) << dwBShiftR) |
			                (0xFF000000);

			unsigned int * puiOneLineDown;
			unsigned int * puiOneLineUp;
			pcMem = (char *)ddsdSurfaceDesc.lpSurface;

			puiMem = (unsigned int *)pcMem;

			if (*puiMem == uiColorCmp)
			{
				puiOneLineUp = (unsigned int *)(((char *)puiMem) + ddsdSurfaceDesc.lPitch);

				if (!(*(puiMem + 1))) *(puiMem + 1) = uiColorWrite;

				if (!(*puiOneLineUp)) *puiOneLineUp = uiColorWrite;
			}

			puiMem++;

			for (UINT iX = 1 ; iX < (ddsdSurfaceDesc.dwWidth - 1) ; iX++)
			{
				if (*puiMem == uiColorCmp)
				{
					puiOneLineUp = (unsigned int *)(((char *) puiMem) + ddsdSurfaceDesc.lPitch);

					if (!(*(puiMem - 1))) *(puiMem - 1) = uiColorWrite;

					if (!(*(puiMem + 1))) *(puiMem + 1) = uiColorWrite;

					if (!(*puiOneLineUp)) *puiOneLineUp = uiColorWrite;
				}

				puiMem++;
			}

			if (*puiMem == uiColorCmp)
			{
				puiOneLineUp = (unsigned int *)(((char *)puiMem) + ddsdSurfaceDesc.lPitch);

				if (!(*(puiMem - 1))) *(puiMem - 1) = uiColorWrite;

				if (!(*puiOneLineUp)) *puiOneLineUp = uiColorWrite;
			}

			for (UINT iY = 1 ; iY < (ddsdSurfaceDesc.dwHeight - 1) ; iY++)
			{
				pcMem += ddsdSurfaceDesc.lPitch;
				puiMem = (unsigned int *)pcMem;

				if (*puiMem == uiColorCmp)
				{
					puiOneLineDown = (unsigned int *)(((char *)puiMem) - ddsdSurfaceDesc.lPitch);
					puiOneLineUp   = (unsigned int *)(((char *)puiMem) + ddsdSurfaceDesc.lPitch);

					if (!(*(puiMem + 1))) *(puiMem + 1) = uiColorWrite;

					if (!(*puiOneLineDown)) *puiOneLineDown = uiColorWrite;

					if (!(*puiOneLineUp)) *puiOneLineUp	 = uiColorWrite;
				}

				puiMem++;

				for (UINT iX = 1 ; iX < (ddsdSurfaceDesc.dwWidth - 1) ; iX++)
				{
					if (*puiMem == uiColorCmp)
					{
						puiOneLineDown	= (unsigned int *)(((char *)puiMem) - ddsdSurfaceDesc.lPitch);
						puiOneLineUp	= (unsigned int *)(((char *)puiMem) + ddsdSurfaceDesc.lPitch);

						if (!(*(puiMem - 1))) *(puiMem - 1) = uiColorWrite;

						if (!(*(puiMem + 1))) *(puiMem + 1) = uiColorWrite;

						if (!(*puiOneLineDown)) *puiOneLineDown = uiColorWrite;

						if (!(*puiOneLineUp)) *puiOneLineUp   = uiColorWrite;
					}

					puiMem++;
				}

				if (*puiMem == uiColorCmp)
				{
					puiOneLineDown = (unsigned int *)(((char *)puiMem) - ddsdSurfaceDesc.lPitch);
					puiOneLineUp = (unsigned int *)(((char *)puiMem) + ddsdSurfaceDesc.lPitch);

					if (!(*(puiMem - 1))) *(puiMem - 1) = uiColorWrite;

					if (!(*puiOneLineDown)) *puiOneLineDown = uiColorWrite;

					if (!(*puiOneLineUp)) *puiOneLineUp   = uiColorWrite;
				}
			}

			pcMem += ddsdSurfaceDesc.lPitch;
			puiMem = (unsigned int *)pcMem;

			if (*puiMem == uiColorCmp)
			{
				puiOneLineDown = (unsigned int *)(((char *)puiMem) - ddsdSurfaceDesc.lPitch);

				if (!(*(puiMem + 1))) *(puiMem + 1) = uiColorWrite;

				if (!(*puiOneLineDown)) *puiOneLineDown = uiColorWrite;
			}

			puiMem++;

			for (UINT iX = 1 ; iX < (ddsdSurfaceDesc.dwWidth - 1) ; iX++)
			{
				if (*puiMem == uiColorCmp)
				{
					puiOneLineDown = (unsigned int *)(((char *)puiMem) - ddsdSurfaceDesc.lPitch);

					if (!(*(puiMem - 1))) *(puiMem - 1) = uiColorWrite;

					if (!(*(puiMem + 1))) *(puiMem + 1) = uiColorWrite;

					if (!(*puiOneLineDown)) *puiOneLineDown = uiColorWrite;
				}

				puiMem++;
			}

			if (*puiMem == uiColorCmp)
			{
				puiOneLineDown = (unsigned int *)(((char *)puiMem) - ddsdSurfaceDesc.lPitch);

				if (!(*(puiMem - 1))) *(puiMem - 1) = uiColorWrite;

				if (!(*puiOneLineDown)) *puiOneLineDown = uiColorWrite;
			}

			uiColorCmp = uiColorWrite;
			_fR -= fDR;
			_fG -= fDG;
			_fB -= fDB;
		}

		puiMem = (unsigned int *)ddsdSurfaceDesc.lpSurface;

		while (iNb2--)
		{
			if (*puiMem == uiColorMask) *puiMem = 0;

			puiMem++;
		}
	}
	else
	{
		unsigned short usColorMask;

		unsigned int uiDec = ((((unsigned int)(255.f - _fR)) >> dwRShiftL) << dwRShiftR) |
		                     ((((unsigned int)(255.f - _fG)) >> dwGShiftL) << dwGShiftR) |
		                     ((((unsigned int)(255.f - _fB)) >> dwBShiftL) << dwBShiftR) |
		                     (0x8000);
		ARX_CHECK_USHORT(uiDec);
		usColorMask =	ARX_CLEAN_WARN_CAST_USHORT(uiDec);

		unsigned short * pusMem = (unsigned short *)ddsdSurfaceDesc.lpSurface;

		while (iNb--)
		{
			if (*pusMem) *pusMem = usColorMask;

			pusMem++;
		}

		char * pcMem;
		unsigned short usColorCmp;
		unsigned short usColorWrite;
		usColorCmp = usColorMask;

		while (_iNbCouche--)
		{
			int iColor = ((((int)_fR) >> dwRShiftL) << dwRShiftR) |
			             ((((int)_fG) >> dwGShiftL) << dwGShiftR) |
			             ((((int)_fB) >> dwBShiftL) << dwBShiftR) |
			             (0x8000);
			ARX_CHECK_USHORT(iColor);

			usColorWrite =	ARX_CLEAN_WARN_CAST_USHORT(iColor);

			unsigned short * pusOneLineDown;
			unsigned short * pusOneLineUp;
			pcMem = (char *)ddsdSurfaceDesc.lpSurface;

			pusMem = (unsigned short *)pcMem;

			if (*pusMem == usColorCmp)
			{
				pusOneLineUp = (unsigned short *)(((char *)pusMem) + ddsdSurfaceDesc.lPitch);

				if (!(*(pusMem + 1))) *(pusMem + 1) = usColorWrite;

				if (!(*pusOneLineUp)) *pusOneLineUp = usColorWrite;
			}

			pusMem++;

			for (UINT iX = 1 ; iX < (ddsdSurfaceDesc.dwWidth - 1) ; iX++)
			{
				if (*pusMem == usColorCmp)
				{
					pusOneLineUp = (unsigned short *)(((char *) pusMem) + ddsdSurfaceDesc.lPitch);

					if (!(*(pusMem - 1))) *(pusMem - 1) = usColorWrite;

					if (!(*(pusMem + 1))) *(pusMem + 1) = usColorWrite;

					if (!(*pusOneLineUp)) *pusOneLineUp   = usColorWrite;
				}

				pusMem++;
			}

			if (*pusMem == usColorCmp)
			{
				pusOneLineUp = (unsigned short *)(((char *)pusMem) + ddsdSurfaceDesc.lPitch);

				if (!(*(pusMem - 1))) *(pusMem - 1) = usColorWrite;

				if (!(*pusOneLineUp)) *pusOneLineUp = usColorWrite;
			}

			for (UINT iY = 1 ; iY < (ddsdSurfaceDesc.dwHeight - 1) ; iY++)
			{
				pcMem += ddsdSurfaceDesc.lPitch;
				pusMem = (unsigned short *)pcMem;

				if (*pusMem == usColorCmp)
				{
					pusOneLineDown = (unsigned short *)(((char *)pusMem) - ddsdSurfaceDesc.lPitch);
					pusOneLineUp   = (unsigned short *)(((char *)pusMem) + ddsdSurfaceDesc.lPitch);

					if (!(*(pusMem + 1))) *(pusMem + 1) = usColorWrite;

					if (!(*pusOneLineDown)) *pusOneLineDown = usColorWrite;

					if (!(*pusOneLineUp)) *pusOneLineUp = usColorWrite;
				}

				pusMem++;

				for (UINT iX = 1 ; iX < (ddsdSurfaceDesc.dwWidth - 1) ; iX++)
				{
					if (*pusMem == usColorCmp)
					{
						pusOneLineDown	= (unsigned short *)(((char *)pusMem) - ddsdSurfaceDesc.lPitch);
						pusOneLineUp	= (unsigned short *)(((char *)pusMem) + ddsdSurfaceDesc.lPitch);

						if (!(*(pusMem - 1))) *(pusMem - 1)	= usColorWrite;

						if (!(*(pusMem + 1))) *(pusMem + 1)	= usColorWrite;

						if (!(*pusOneLineDown)) *pusOneLineDown	= usColorWrite;

						if (!(*pusOneLineUp)) *pusOneLineUp	= usColorWrite;
					}

					pusMem++;
				}

				if (*pusMem == usColorCmp)
				{
					pusOneLineDown = (unsigned short *)(((char *)pusMem) - ddsdSurfaceDesc.lPitch);
					pusOneLineUp = (unsigned short *)(((char *)pusMem) + ddsdSurfaceDesc.lPitch);

					if (!(*(pusMem - 1))) *(pusMem - 1) = usColorWrite;

					if (!(*pusOneLineDown)) *pusOneLineDown = usColorWrite;

					if (!(*pusOneLineUp)) *pusOneLineUp = usColorWrite;
				}
			}

			pcMem += ddsdSurfaceDesc.lPitch;
			pusMem = (unsigned short *)pcMem;

			if (*pusMem == usColorCmp)
			{
				pusOneLineDown = (unsigned short *)(((char *)pusMem) - ddsdSurfaceDesc.lPitch);

				if (!(*(pusMem + 1))) *(pusMem + 1) = usColorWrite;

				if (!(*pusOneLineDown)) *pusOneLineDown = usColorWrite;
			}

			pusMem++;

			for (UINT iX = 1 ; iX < (ddsdSurfaceDesc.dwWidth - 1) ; iX++)
			{
				if (*pusMem == usColorCmp)
				{
					pusOneLineDown = (unsigned short *)(((char *)pusMem) - ddsdSurfaceDesc.lPitch);

					if (!(*(pusMem - 1))) *(pusMem - 1) = usColorWrite;

					if (!(*(pusMem + 1))) *(pusMem + 1) = usColorWrite;

					if (!(*pusOneLineDown)) *pusOneLineDown = usColorWrite;
				}

				pusMem++;
			}

			if (*pusMem == usColorCmp)
			{
				pusOneLineDown = (unsigned short *)(((char *)pusMem) - ddsdSurfaceDesc.lPitch);

				if (!(*(pusMem - 1))) *(pusMem - 1) = usColorWrite;

				if (!(*pusOneLineDown)) *pusOneLineDown = usColorWrite;
			}


			usColorCmp = usColorWrite;
			_fR -= fDR;
			_fG -= fDG;
			_fB -= fDB;
		}

		pusMem = (unsigned short *)ddsdSurfaceDesc.lpSurface;

		while (iNb2--)
		{
			if (*pusMem == usColorMask) *pusMem = 0;

			pusMem++;
		}
	}

	if (FAILED(pddsTempSurface->Unlock(NULL)))
	{
		pddsTempSurface->Release();
		pDD->Release();
		return NULL;
	}

	SmoothSurface(pddsTempSurface);
	SmoothSurface(pddsTempSurface);

	//on cree un bitmap
	void * pData = NULL;

	BITMAPINFO bmi;
	bmi.bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth			= iHaloWidth;
	bmi.bmiHeader.biHeight			= iHaloHeight;
	bmi.bmiHeader.biPlanes			= 1;
	bmi.bmiHeader.biBitCount		= 24;
	bmi.bmiHeader.biCompression		= BI_RGB;
	bmi.bmiHeader.biSizeImage		= iHaloWidth * iHaloHeight * 3;
	bmi.bmiHeader.biXPelsPerMeter	= 0;
	bmi.bmiHeader.biYPelsPerMeter	= 0;
	bmi.bmiHeader.biClrUsed			= 0;
	bmi.bmiHeader.biClrImportant	= 0;

	HDC memDC = CreateCompatibleDC(NULL);
	HBITMAP hBitmap = CreateDIBSection(memDC, &bmi, DIB_RGB_COLORS, &pData, NULL, 0);

	if (!hBitmap)
	{
		DeleteDC(memDC);
		pddsTempSurface->Release();
		pDD->Release();
		return NULL;
	}

	SelectObject(memDC, hBitmap);

	ddsdSurfaceDesc.dwSize = sizeof(ddsdSurfaceDesc);

	if (FAILED(pddsTempSurface->Lock(NULL,
	                                 &ddsdSurfaceDesc,
	                                 DDLOCK_WRITEONLY | DDLOCK_WAIT,
	                                 NULL)))
	{
		DeleteDC(memDC);
		DeleteObject(hBitmap);
		pddsTempSurface->Release();
		pDD->Release();
		return NULL;
	}

	ddsdSurfaceDesc.ddpfPixelFormat.dwRBitMask >>= dwRShiftR;
	ddsdSurfaceDesc.ddpfPixelFormat.dwGBitMask >>= dwGShiftR;
	ddsdSurfaceDesc.ddpfPixelFormat.dwBBitMask >>= dwBShiftR;
	dwRShiftL = dwGShiftL = dwBShiftL = 0;

	while (!(ddsdSurfaceDesc.ddpfPixelFormat.dwRBitMask & 0x80))
	{
		ddsdSurfaceDesc.ddpfPixelFormat.dwRBitMask <<= 1;
		dwRShiftL++;
	}

	while (!(ddsdSurfaceDesc.ddpfPixelFormat.dwGBitMask & 0x80))
	{
		ddsdSurfaceDesc.ddpfPixelFormat.dwGBitMask <<= 1;
		dwGShiftL++;
	}

	while (!(ddsdSurfaceDesc.ddpfPixelFormat.dwBBitMask & 0x80))
	{
		ddsdSurfaceDesc.ddpfPixelFormat.dwBBitMask <<= 1;
		dwBShiftL++;
	}

	unsigned char * pucData = (unsigned char *)pData;
	unsigned char usR, usG, usB;
	ddsdSurfaceDesc.lpSurface = (void *)(((char *)ddsdSurfaceDesc.lpSurface) + (ddsdSurfaceDesc.lPitch * (ddsdSurfaceDesc.dwHeight - 1)));

	if (ddsdSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 16)
	{
		for (UINT iY = 0 ; iY < iHaloHeight ; iY++)
		{
			unsigned short * pusMem = (unsigned short *) ddsdSurfaceDesc.lpSurface;

			for (UINT iX = 0 ; iX < iHaloWidth ; iX++)
			{
				usR = (unsigned char)((*pusMem >> dwRShiftR) << dwRShiftL);
				usG = (unsigned char)((*pusMem >> dwGShiftR) << dwGShiftL);
				usB = (unsigned char)((*pusMem >> dwBShiftR) << dwBShiftL);
				*pucData++ = usB;
				*pucData++ = usG;
				*pucData++ = usR;
				pusMem++;
			}

			ddsdSurfaceDesc.lpSurface = (void *)(((char *)ddsdSurfaceDesc.lpSurface) - ddsdSurfaceDesc.lPitch);
		}
	}
	else
	{
		for (UINT iY = 0 ; iY < iHaloHeight ; iY++)
		{
			unsigned int * puiMem = (unsigned int *)ddsdSurfaceDesc.lpSurface;

			for (UINT iX = 0 ; iX < iHaloWidth ; iX++)
			{
				usR = (unsigned char)((*puiMem >> dwRShiftR) << dwRShiftL);
				usG = (unsigned char)((*puiMem >> dwGShiftR) << dwGShiftL);
				usB = (unsigned char)((*puiMem >> dwBShiftR) << dwBShiftL);
				*pucData++ = usB;
				*pucData++ = usG;
				*pucData++ = usR;
				puiMem++;
			}

			ddsdSurfaceDesc.lpSurface = (void *)(((char *)ddsdSurfaceDesc.lpSurface) - ddsdSurfaceDesc.lPitch);
		}
	}

	if (FAILED(pddsTempSurface->Unlock(NULL)))
	{
		DeleteDC(memDC);
		DeleteObject(hBitmap);
		pddsTempSurface->Release();
		pDD->Release();
		return NULL;
	}

	char tText[256];
	strcpy((char *)tText, (const char *)m_strName);
	strcat((char *)tText, "_H");
	TextureContainer * pTex;

	if (FAILED(D3DTextr_CreateEmptyTexture(
	               (char *)tText,
	               iHaloWidth,
	               iHaloHeight,
	               0,
	               0,
	               NULL, 1)))
	{
		DeleteDC(memDC);
		DeleteObject(hBitmap);
		pddsTempSurface->Release();
		pDD->Release();
		return NULL;
	}


	pTex = FindTexture((TCHAR *)tText);

	if (!pTex)
	{
		DeleteDC(memDC);
		DeleteObject(hBitmap);
		pddsTempSurface->Release();
		pDD->Release();
		return NULL;
	}

	pTex->m_hbmBitmap = hBitmap;

	DeleteDC(memDC);
	pddsTempSurface->Release();
	pDD->Release();

	pTex->Restore(_lpDevice);
	ddsdSurfaceDesc.dwSize = sizeof(ddsdSurfaceDesc);
	pTex->m_pddsSurface->GetSurfaceDesc(&ddsdSurfaceDesc);
	return pTex;
}

