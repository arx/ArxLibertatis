/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

#include <boost/algorithm/string/case_conv.hpp>

#include "graphics/Renderer.h"
#include "graphics/texture/Texture.h"

#include "io/resource/ResourcePath.h"
#include "io/resource/PakReader.h"
#include "io/log/Logger.h"
#include "io/fs/FilePath.h"
#include "io/fs/Filesystem.h"

#include "platform/Platform.h"

#include "scene/Object.h"

long GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE = 0;

/*-----------------------------------------------------------------------------*/
// Local list of textures
//-----------------------------------------------------------------------------

static TextureContainer * g_ptcTextureList = NULL;

TextureContainer * GetTextureList() {
	return g_ptcTextureList;
}

TextureContainer * GetAnyTexture() {
	return g_ptcTextureList;
}

static void ResetModelBatch(ModelBatch * tex) {
	
	if(!tex) {
		return;
	}
	
	tex->count[BatchBucket_Opaque] = 0;
	tex->count[BatchBucket_Blended] = 0;
	tex->count[BatchBucket_Additive] = 0;
	tex->count[BatchBucket_Subtractive] = 0;
	tex->count[BatchBucket_Multiplicative] = 0;
	
	tex->max[BatchBucket_Opaque] = 0;
	tex->max[BatchBucket_Blended] = 0;
	tex->max[BatchBucket_Additive] = 0;
	tex->max[BatchBucket_Subtractive] = 0;
	tex->max[BatchBucket_Multiplicative] = 0;
	
	free(tex->list[BatchBucket_Opaque]);
	tex->list[BatchBucket_Opaque] = NULL;
	free(tex->list[BatchBucket_Blended]);
	tex->list[BatchBucket_Blended] = NULL;
	free(tex->list[BatchBucket_Additive]);
	tex->list[BatchBucket_Additive] = NULL;
	free(tex->list[BatchBucket_Subtractive]);
	tex->list[BatchBucket_Subtractive] = NULL;
	free(tex->list[BatchBucket_Multiplicative]);
	tex->list[BatchBucket_Multiplicative] = NULL;
}

static void ResetRoomBatches(RoomBatches & roomBatches) {
	roomBatches.tMatRoomSize = 0;
	free(roomBatches.tMatRoom);
	roomBatches.tMatRoom = NULL;
}

TextureContainer::TextureContainer(const res::path & strName, TCFlags flags)
	: m_texName(strName)
{
	arx_assert(!strName.has_ext("bmp") && !strName.has_ext("tga"),
	           "bad texture name: \"%s\"", strName.string().c_str());
	
	m_size = Vec2i_ZERO;
	m_dwFlags = flags;

	m_pTexture = NULL;

	userflags = 0;
	TextureHalo = NULL;
	
	m_pNext = NULL;
	
	// Add the texture to the head of the global texture list
	if(!(flags & NoInsert)) {
		m_pNext = g_ptcTextureList;
		g_ptcTextureList = this;
	}

	systemflags = 0;

	m_roomBatches = RoomBatches();
	m_modelBatch = ModelBatch();
}

TextureContainer::~TextureContainer() {
	
	delete m_pTexture;
	delete TextureHalo;
		
	// Remove the texture container from the global list
	if(g_ptcTextureList == this) {
		g_ptcTextureList = m_pNext;
	} else {
		for(TextureContainer * ptc = g_ptcTextureList; ptc; ptc = ptc->m_pNext) {
			if(ptc->m_pNext == this) {
				ptc->m_pNext = m_pNext;
			}
		}
	}
	
	ResetModelBatch(&m_modelBatch);
	ResetRoomBatches(m_roomBatches);
}

bool TextureContainer::LoadFile(const res::path & strPathname) {
	
	res::path tempPath = strPathname;
	bool foundPath = resources->getFile(tempPath.append(".png")) != NULL;
	foundPath = foundPath || resources->getFile(tempPath.set_ext("jpg"));
	foundPath = foundPath || resources->getFile(tempPath.set_ext("jpeg"));
	foundPath = foundPath || resources->getFile(tempPath.set_ext("bmp"));
	foundPath = foundPath || resources->getFile(tempPath.set_ext("tga"));

	if(!foundPath) {
		LogError << strPathname << " not found";
		return false;
	}
	
	delete m_pTexture, m_pTexture = NULL;
	m_pTexture = GRenderer->CreateTexture2D();
	if(!m_pTexture) {
		return false;
	}
	
	Texture::TextureFlags flags = 0;
	
	if(!(m_dwFlags & NoColorKey) && tempPath.ext() == ".bmp") {
		flags |= Texture::HasColorKey;
	}
	
	if(!(m_dwFlags & NoMipmap)) {
		flags |= Texture::HasMipmaps;
	}
	
	if(m_dwFlags & Intensity) {
		flags |= Texture::Intensity;
	}
	
	if(!m_pTexture->Init(tempPath, flags)) {
		LogError << "Error creating texture " << tempPath;
		return false;
	}
	
	m_size = m_pTexture->getSize();
	
	Vec2f storedSize = Vec2f(m_pTexture->getStoredSize());
	uv = Vec2f(m_size) / storedSize;
	hd = Vec2f(.5f, .5f) / storedSize;
	
	return true;
}

bool TextureContainer::hasColorKey() {
	return m_pTexture != NULL && m_pTexture->hasColorKey();
}

TextureContainer * TextureContainer::Load(const res::path & name, TCFlags flags) {
	
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
	
	MakeUserFlag(newTexture);
	
	return newTexture;
}

TextureContainer * TextureContainer::LoadUI(const res::path & strName, TCFlags flags) {
	return Load(strName, flags | TextureContainer::NoMipmap);
}

bool TextureContainer::CreateHalo() {
	
	Image srcImage;
	if(!srcImage.LoadFromFile(m_pTexture->getFileName())) {
		return false;
	}
	
	// Allocate and add the texture to the linked list of textures;
	res::path haloName = m_texName.string();
	haloName.append("_halo");
	TextureHalo = new TextureContainer(haloName, NoMipmap | NoColorKey);
	if(!TextureHalo) {
		return false;
	}
	
	TextureHalo->m_pTexture = GRenderer->CreateTexture2D();
	if(!TextureHalo->m_pTexture) {
		return true;
	}
	
	Image im;
	
	int width = m_size.x + HALO_RADIUS * 2;
	int height = m_size.y + HALO_RADIUS * 2;
	im.Create(width, height, srcImage.GetFormat());
	
	// Center the image, offset by radius to contain the edges of the blur
	im.Clear();
	im.Copy(srcImage, HALO_RADIUS, HALO_RADIUS);
	
	// Keep a copy of the image at this stage, in order to apply proper alpha masking later
	Image copy = im;
	
	// Convert image to grayscale, and turn it to black & white
	im.ToGrayscale(Image::Format_L8A8);
	im.ApplyThreshold(0, ~0);

	// Blur the image
	im.Blur(HALO_RADIUS);

	// Increase the gamma of the blur outline
	im.QuakeGamma(10.0f);

	// Set alpha to inverse of original image alpha
	copy.ApplyColorKeyToAlpha();
	im.SetAlpha(copy, true);
	
	TextureHalo->m_pTexture->Init(im, 0);
	
	TextureHalo->m_size.x = TextureHalo->m_pTexture->getSize().x;
	TextureHalo->m_size.y = TextureHalo->m_pTexture->getSize().y;
	
	Vec2i storedSize = TextureHalo->m_pTexture->getStoredSize();
	TextureHalo->uv = Vec2f(
		float(TextureHalo->m_size.x) / storedSize.x,
		float(TextureHalo->m_size.y) / storedSize.y
	);
	TextureHalo->hd = Vec2f(.5f / storedSize.x, .5f / storedSize.y);
	
	return true;
}

TextureContainer * TextureContainer::Find(const res::path & strTextureName) {
	
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
