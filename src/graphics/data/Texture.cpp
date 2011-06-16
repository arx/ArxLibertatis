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

#include "graphics/data/Texture.h"

#include <cstdio>
#include <cassert>
#include <limits>
#include <iomanip>
#include <sstream>

#include <zlib.h>

#include <il.h>

#include "core/Application.h"

#include "graphics/GraphicsUtility.h"
#include "graphics/GraphicsEnum.h"
#include "graphics/Math.h"
#include "graphics/Renderer.h"

#include "io/FilePath.h"
#include "io/PakManager.h"
#include "io/Logger.h"

#include "platform/Platform.h"
#include "platform/String.h"

using std::string;
using std::map;

long GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE = 0;

/*-----------------------------------------------------------------------------*/
// Local list of textures
//-----------------------------------------------------------------------------


const TextureContainer::TCFlags TextureContainer::UI = (TextureContainer::NoMipmap | TextureContainer::NoRefinement);
const TextureContainer::TCFlags TextureContainer::All = Flag(0xffffffff);

TextureContainer * g_ptcTextureList = NULL;

TextureContainer * GetTextureList()
{
	return g_ptcTextureList;
}

TextureContainer * GetAnyTexture()
{
	return g_ptcTextureList;
}

#define NB_MIPMAP_LEVELS 5

// tex Must be of sufficient size...
long CountTextures( std::string& tex, long * memsize, long * memmip)
{
	std::string temp;
	TextureContainer * ptcTexture = g_ptcTextureList;
	long count = 0;
	*memsize = 0;
	*memmip = 0;

	tex.clear();

	while (ptcTexture)
	{
		count++;

		if (ptcTexture->m_dwFlags & TextureContainer::NoMipmap) {
			std::stringstream ss;
			ss << std::setw(3) << count << std::setw(0) << ' ' << ptcTexture->m_texName << ' ' << ptcTexture->m_dwWidth << 'x' << ptcTexture->m_dwHeight << 'x' << ptcTexture->m_dwBPP << ' ' << GetName(ptcTexture->m_texName) << "\r\n";
			temp = ss.str();
		}
		else
		{
			std::stringstream ss;
			ss << std::setw(3) << count << ' ' << std::setw(0) << ptcTexture->m_texName << ' ' << ptcTexture->m_dwWidth << 'x' << ptcTexture->m_dwHeight << 'x' << ptcTexture->m_dwBPP << " MIP " << GetName(ptcTexture->m_texName) << "\r\n";
			temp = ss.str();

			for (long k = 1; k <= NB_MIPMAP_LEVELS; k++)
			{
				*memmip += ((long)(ptcTexture->m_dwWidth * ptcTexture->m_dwHeight * ptcTexture->m_dwBPP) >> 3) / (4 * k);
			}
		}

		*memsize += (long)(ptcTexture->m_dwWidth * ptcTexture->m_dwHeight * ptcTexture->m_dwBPP) >> 3;
		tex = temp;
	}

	ptcTexture = ptcTexture->m_pNext;

	return count;
}

void ResetVertexLists(TextureContainer * ptcTexture)
{
	if(!ptcTexture)
		return;

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

	ptcTexture->vPolyInterZMap.clear();
	ptcTexture->vPolyZMap.clear();

	if (ptcTexture->pVertexListCull)
	{
		free((void *)ptcTexture->pVertexListCull);
		ptcTexture->pVertexListCull = NULL;
	}

	if (ptcTexture->pVertexListCull_TNormalTrans)
	{
		free((void *)ptcTexture->pVertexListCull_TNormalTrans);
		ptcTexture->pVertexListCull_TNormalTrans = NULL;
	}

	if (ptcTexture->pVertexListCull_TAdditive)
	{
		free((void *)ptcTexture->pVertexListCull_TAdditive);
		ptcTexture->pVertexListCull_TAdditive = NULL;
	}

	if (ptcTexture->pVertexListCull_TSubstractive)
	{
		free((void *)ptcTexture->pVertexListCull_TSubstractive);
		ptcTexture->pVertexListCull_TSubstractive = NULL;
	}

	if (ptcTexture->pVertexListCull_TMultiplicative)
	{
		free((void *)ptcTexture->pVertexListCull_TMultiplicative);
		ptcTexture->pVertexListCull_TMultiplicative = NULL;
	}

	if (ptcTexture->pVertexListCull_TMetal)
	{
		free((void *)ptcTexture->pVertexListCull_TMetal);
		ptcTexture->pVertexListCull_TMetal = NULL;
	}
	
	if(ptcTexture->tMatRoom) {
		free(ptcTexture->tMatRoom);
		ptcTexture->tMatRoom = NULL;
	}
}

//-----------------------------------------------------------------------------
// Name: TextureContainer()
// Desc: Constructor for a texture object
//-----------------------------------------------------------------------------
TextureContainer::TextureContainer(const std::string& strName, TCFlags flags) {
	
	m_texName = strName;
	MakeUpcase( m_texName );

	m_dwWidth		= 0;
	m_dwHeight		= 0;
	m_dwBPP			= 0;
	m_dwFlags		= flags;

	m_pTexture = NULL;

	userflags = 0;
	TextureRefinement = NULL;
	TextureHalo = NULL;

	// Add the texture to the head of the global texture list
	if(!(flags & NoInsert)) {
		m_pNext = g_ptcTextureList;
		g_ptcTextureList = this;
	}

	delayed = NULL;
	delayed_nb = 0;
	delayed_max = 0;

	systemflags = 0;

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

	vPolyInterZMap.clear();
	vPolyZMap.clear();
}

TextureContainer::~TextureContainer()
{
	delete m_pTexture;

	if (delayed)
	{
		free(delayed);
		delayed = NULL;
	}

	// Remove the texture container from the global list
	if (g_ptcTextureList == this)
		g_ptcTextureList = m_pNext;
	else
	{
		for (TextureContainer * ptc = g_ptcTextureList; ptc; ptc = ptc->m_pNext)
			if (ptc->m_pNext == this)
				ptc->m_pNext = m_pNext;
	}

	ResetVertexLists(this);
}

bool TextureContainer::LoadFile(const std::string& strPathname)
{
	bool bLoaded = false;

	m_texName = strPathname;
	MakeUpcase( m_texName );
	
	std::string tempPath = m_texName;
	bool foundPath = PAK_FileExist(SetExt(tempPath, ".png"));
	foundPath = foundPath || PAK_FileExist(SetExt(tempPath, ".jpg"));
	foundPath = foundPath || PAK_FileExist(SetExt(tempPath, ".jpeg"));
	foundPath = foundPath || PAK_FileExist(SetExt(tempPath, ".bmp"));
	foundPath = foundPath || PAK_FileExist(SetExt(tempPath, ".tga"));

	if(!foundPath)
	{
		LogError << m_texName << " not found";
		return false;
	}
	
	if(m_pTexture)
		delete m_pTexture;

	m_pTexture = GRenderer->CreateTexture2D();
	if(m_pTexture)
	{
		bool bMipmaps = !(m_dwFlags & NoMipmap);
		bLoaded = m_pTexture->Init(tempPath, bMipmaps);
		if(bLoaded)
		{
			m_dwWidth   = m_pTexture->GetImage().GetWidth();
			m_dwHeight  = m_pTexture->GetImage().GetHeight();
			m_dwBPP     = m_pTexture->GetImage().GetNumChannels() * 8;

			// Do not keep image data in memory
			m_pTexture->GetImage().Reset();

			m_dwDeviceWidth = m_pTexture->GetWidth();
			m_dwDeviceHeight = m_pTexture->GetHeight();

			m_odx = (1.f / (float)m_dwWidth);
			m_hdx = 0.5f * (1.f / (float)m_dwDeviceWidth);
			m_dx  = (m_dwWidth / (float)m_dwDeviceWidth) + this->m_hdx;
			m_ody = (1.f / (float)m_dwHeight);
			m_hdy = 0.5f * (1.f / (float)m_dwDeviceHeight);
			m_dy  = (m_dwHeight / (float)m_dwDeviceHeight) + this->m_hdy;
		}
	}

	return bLoaded;
}

extern void MakeUserFlag(TextureContainer * tc);

TextureContainer * TextureContainer::Load(const string & name, TCFlags flags) {
	
	// Check first to see if the texture is already loaded
	TextureContainer * newTexture = Find(name);
	if(newTexture) {
		// TODO don't we need to check the texture's systemflags?
		return newTexture;
	}
	
	// Allocate and add the texture to the linked list of textures;
	newTexture = new TextureContainer(name, flags);
	if(!newTexture) {
		return NULL;
	}
	
	newTexture->systemflags = flags;
	
	if(GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE == -1) {
		newTexture->systemflags &= ~Level;
	}
	
	// Create a bitmap and load the texture file into it,
	if(!newTexture->LoadFile(name)) {
		delete newTexture;
		return NULL;
	}
	
	if(!(flags & NoRefinement)) {
		newTexture->LookForRefinementMap(flags);
	}
	
	MakeUserFlag(newTexture);
	
	return newTexture;
}

TextureContainer * TextureContainer::LoadUI(const string & strName, TCFlags flags) {
	return Load(strName, flags | UI);
}

TextureContainer * TextureContainer::Find(const std::string& strTextureName) {
	
	TextureContainer * ptcTexture = g_ptcTextureList;
	
	std::string strTexNameUpper = strTextureName;
	MakeUpcase(strTexNameUpper);
	
	while(ptcTexture) {
		
		if(strTexNameUpper.find(ptcTexture->m_texName) != std::string::npos) {
			return ptcTexture;
		}
		
		ptcTexture = ptcTexture->m_pNext;
	}
	
	return NULL;
}

void TextureContainer::DeleteAll(TCFlags flag)
{
	TextureContainer * pCurrentTexture = g_ptcTextureList;
	TextureContainer * pNextTexture = NULL;

	while (pCurrentTexture)
	{
		pNextTexture = pCurrentTexture->m_pNext;

		if (pCurrentTexture->systemflags & flag)
			delete pCurrentTexture;

		pCurrentTexture = pNextTexture;
	}
}

TextureContainer::RefinementMap TextureContainer::s_GlobalRefine;
TextureContainer::RefinementMap TextureContainer::s_Refine;

static void ConvertData(string & dat) {
	
	size_t substrStart = 0;
	size_t substrLen = string::npos;
	
	size_t posStart = dat.find_first_of('"');
	if(posStart != string::npos) {
		substrStart = posStart + 1;
		
		size_t posEnd = dat.find_last_of('"');
		arx_assert(posEnd != string::npos);
		
		if(posEnd != posStart) {
			substrLen = posEnd - substrStart;
		}
	}
	
	dat = dat.substr(substrStart, substrLen);
}

static void LoadRefinementMap(const string & fileName, map<string, string> & refinementMap) {
	
	size_t fileSize = 0;
	char * from = (char *)PAK_FileLoadMalloc(fileName, fileSize);
	if(!from) {
		return;
	}
	
	size_t pos = 0;
	long count = 0;
	
	std::string name;
	
	while(pos < fileSize) {
		
		string data;
		
		while(pos < fileSize && from[pos] != '\r' && from[pos] != '\n') {
			data += from[pos++];
		}
		while(pos < fileSize && (from[pos] == '\r' || from[pos] == '\n')) {
			pos++;
		}
		
		if(count == 2) {
			count = 0;
			continue;
		}
		
		ConvertData(data);
		
		if(count == 0) {
			name = data;
		} else if(count == 1) {
			
			MakeUpcase(name);
			MakeUpcase(data);
			
			if(data != "NONE") {
				refinementMap[name] = data;
			} else {
				refinementMap[name].clear();
			}
		}
		
		count++;
	}
	
	free(from);
}

void TextureContainer::LookForRefinementMap(TCFlags flags) {
	
	TextureRefinement = NULL;
	
	static bool loadedRefinements = false;
	if(!loadedRefinements) {
		const char INI_REFINEMENT_GLOBAL[] = "Graph\\Obj3D\\Textures\\Refinement\\GlobalRefinement.ini";
		const char INI_REFINEMENT[] = "Graph\\Obj3D\\Textures\\Refinement\\Refinement.ini";
		LoadRefinementMap(INI_REFINEMENT_GLOBAL, s_GlobalRefine);
		LoadRefinementMap(INI_REFINEMENT, s_Refine);
		loadedRefinements = true;
	}
	
	std::string name = GetName(m_texName);
	MakeUpcase(name);
	
	RefinementMap::const_iterator it = s_Refine.find(name);
	if(it != s_Refine.end()) {
		if(!it->second.empty()) {
			string file = "Graph\\Obj3D\\Textures\\Refinement\\" + it->second + ".bmp";
			TextureRefinement = Load(file, flags);
		}
		return;
	}
	
	it = s_GlobalRefine.find(name);
	if(it != s_GlobalRefine.end() && !it->second.empty()) {
		string file = "Graph\\Obj3D\\Textures\\Refinement\\" + it->second + ".bmp";
		TextureRefinement = Load(file, flags);
	}
	
	
}
