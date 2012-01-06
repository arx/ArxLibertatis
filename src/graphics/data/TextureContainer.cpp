/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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
// Code: Cyril Meynier
//       Sébastien Scieux	(JPEG & PNG)
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved

#include "graphics/data/TextureContainer.h"

#include <stddef.h>
#include <cstdlib>
#include <string>
#include <utility>

#include "graphics/GraphicsTypes.h"
#include "graphics/Renderer.h"
#include "graphics/texture/Texture.h"

#include "io/FilePath.h"
#include "io/PakReader.h"
#include "io/log/Logger.h"

#include "platform/Platform.h"
#include "platform/String.h"

using std::string;
using std::map;

long GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE = 0;

/*-----------------------------------------------------------------------------*/
// Local list of textures
//-----------------------------------------------------------------------------


const TextureContainer::TCFlags TextureContainer::UI = (TextureContainer::NoMipmap | TextureContainer::NoRefinement);

TextureContainer * g_ptcTextureList = NULL;

TextureContainer * GetTextureList()
{
	return g_ptcTextureList;
}

TextureContainer * GetAnyTexture()
{
	return g_ptcTextureList;
}

void ResetVertexLists(TextureContainer * ptcTexture) {
	
	if(!ptcTexture) {
		return;
	}
	
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
	
	if(ptcTexture->pVertexListCull) {
		free(ptcTexture->pVertexListCull);
		ptcTexture->pVertexListCull = NULL;
	}
	
	if(ptcTexture->pVertexListCull_TNormalTrans) {
		free(ptcTexture->pVertexListCull_TNormalTrans);
		ptcTexture->pVertexListCull_TNormalTrans = NULL;
	}
	
	if(ptcTexture->pVertexListCull_TAdditive) {
		free(ptcTexture->pVertexListCull_TAdditive);
		ptcTexture->pVertexListCull_TAdditive = NULL;
	}
	
	if(ptcTexture->pVertexListCull_TSubstractive) {
		free(ptcTexture->pVertexListCull_TSubstractive);
		ptcTexture->pVertexListCull_TSubstractive = NULL;
	}
	
	if(ptcTexture->pVertexListCull_TMultiplicative) {
		free(ptcTexture->pVertexListCull_TMultiplicative);
		ptcTexture->pVertexListCull_TMultiplicative = NULL;
	}
	
	if(ptcTexture->pVertexListCull_TMetal) {
		free(ptcTexture->pVertexListCull_TMetal);
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
TextureContainer::TextureContainer(const fs::path & strName, TCFlags flags) : m_texName(strName) {
	
	arx_assert_msg(!strName.has_ext("bmp") && !strName.has_ext("tga"), "bad texture name: \"%s\"", strName.string().c_str()); // TODO(case-sensitive) remove
	
	m_dwWidth = 0;
	m_dwHeight = 0;
	m_dwFlags = flags;

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

bool TextureContainer::LoadFile(const fs::path & strPathname) {
	
	bool bLoaded = false;
	
	fs::path tempPath = strPathname;
	bool foundPath = resources->getFile(tempPath.append(".png")) != NULL;
	foundPath = foundPath || resources->getFile(tempPath.set_ext("jpg"));
	foundPath = foundPath || resources->getFile(tempPath.set_ext("jpeg"));
	foundPath = foundPath || resources->getFile(tempPath.set_ext("bmp"));
	foundPath = foundPath || resources->getFile(tempPath.set_ext("tga"));

	if(!foundPath) {
		LogError << strPathname << " not found";
		return false;
	}
	
	if(m_pTexture)
		delete m_pTexture;

	m_pTexture = GRenderer->CreateTexture2D();
	if(m_pTexture)
	{
		
		Texture::TextureFlags flags = 0;
		
		if(!(m_dwFlags & NoColorKey) && tempPath.ext() == ".bmp") {
			flags |= Texture::HasColorKey;
		}
		
		if(!(m_dwFlags & NoMipmap)) {
			flags |= Texture::HasMipmaps;
		}
		
		bLoaded = m_pTexture->Init(tempPath, flags);
		if(bLoaded)
		{
			m_dwWidth = m_pTexture->getSize().x;
			m_dwHeight = m_pTexture->getSize().y;
			
			Vec2i storedSize = m_pTexture->getStoredSize();
			uv = Vec2f(float(m_dwWidth) / storedSize.x, float(m_dwHeight) / storedSize.y);
			hd = Vec2f(.5f / storedSize.x, .5f / storedSize.y);
		}
	}

	return bLoaded;
}

extern void MakeUserFlag(TextureContainer * tc);

TextureContainer * TextureContainer::Load(const fs::path & name, TCFlags flags) {
	
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

TextureContainer * TextureContainer::LoadUI(const fs::path & strName, TCFlags flags) {
	return Load(strName, flags | UI);
}

TextureContainer * TextureContainer::Find(const fs::path & strTextureName) {
	
	TextureContainer * ptcTexture = g_ptcTextureList;
	
	while(ptcTexture) {
		
		if(strTextureName == ptcTexture->m_texName) {
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

static void LoadRefinementMap(const fs::path & fileName, map<fs::path, fs::path> & refinementMap) {
	
	size_t fileSize = 0;
	char * from = resources->readAlloc(fileName, fileSize);
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
			
			
			makeLowercase(data);
			
			if(data != "none") {
				refinementMap[fs::path::load(name)] = fs::path::load(data);
			} else {
				refinementMap[fs::path::load(name)].clear();
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
		const char INI_REFINEMENT_GLOBAL[] = "graph/obj3d/textures/refinement/globalrefinement.ini";
		const char INI_REFINEMENT[] = "graph/obj3d/textures/refinement/refinement.ini";
		LoadRefinementMap(INI_REFINEMENT_GLOBAL, s_GlobalRefine);
		LoadRefinementMap(INI_REFINEMENT, s_Refine);
		loadedRefinements = true;
	}
	
	RefinementMap::const_iterator it = s_Refine.find(m_texName);
	if(it != s_Refine.end()) {
		if(!it->second.empty()) {
			TextureRefinement = Load("graph/obj3d/textures/refinement" / it->second, flags);
		}
		return;
	}
	
	it = s_GlobalRefine.find(m_texName);
	if(it != s_GlobalRefine.end() && !it->second.empty()) {
		TextureRefinement = Load("graph/obj3d/textures/refinement" / it->second, flags);
	}
	
	
}
