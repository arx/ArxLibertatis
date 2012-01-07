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

/*!
 * Functions to manage textures, including creating (loading from a
 * file), restoring lost surfaces, invalidating, and destroying.
 *
 * Note: the implementation of these fucntions maintain an internal list
 * of loaded textures. After creation, individual textures are referenced
 * via their ASCII names.
 */

#ifndef ARX_GRAPHICS_DATA_TEXTURECONTAINER_H
#define ARX_GRAPHICS_DATA_TEXTURECONTAINER_H

#include <vector>
#include <map>

#include "io/resource/ResourcePath.h"
#include "math/Vector2.h"
#include "platform/Flags.h"

struct SMY_ARXMAT;
struct SMY_ZMAPPINFO;
struct EERIEPOLY;
struct TexturedVertex;
class Texture2D;

extern long GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE;

struct DELAYED_PRIM {
	EERIEPOLY * data;
};

/** Linked list structure to hold info per texture.
 * @todo This class is currently an hybrid between a texture class and a render batch... We should create a RenderBatch class for all vertex stuff.
 */
class TextureContainer {
	
public:
	
	enum TCFlag {
		NoMipmap     = (1<<0),
		NoInsert     = (1<<1),
		NoRefinement = (1<<2),
		Level        = (1<<3),
		NoColorKey   = (1<<4)
	};
	
	DECLARE_FLAGS(TCFlag, TCFlags)
	static const TCFlags UI;
	
	/** Constructor.
	 *	Only TextureContainer::Load() is allowed to create TextureContainer, but pieces of code still depend on this constructor being public.
	 *	@param  
	 *	@todo Make this constructor private.
	 */ 
	TextureContainer(const res::path & strName, TCFlags flags);
	
	/** Destructor
	 */
	~TextureContainer();
	
	/**	Load an image into a TextureContainer
	 *
	 *
	 */
	static TextureContainer * Load(const res::path & strName, TCFlags flags = 0);
	
	/**	Load an image into a TextureContainer
	 *
	 *
	 */
	static TextureContainer * LoadUI(const res::path & strName, TCFlags flags = 0);
	
	/** Find a TextureContainer by its name.
	 *	Searches the internal list of textures for a texture specified by
	 *	its name. Returns the structure associated with that texture.
	 *	@param strTextureName Name of the texture to find.
	 *  @return A pointer to a TextureContainer if this texture was already loaded, NULL otherwise.
	 **/
	static TextureContainer * Find(const res::path & strTextureName);
	
	static void DeleteAll(TCFlags flag = TCFlags::all());
	
	/**	Create a texture to display a glowing halo around a transparent texture
	 *	@todo Rewrite this feature using shaders instead of hacking a texture effect
	 */
	bool CreateHalo() { return false; }
	TextureContainer * TextureHalo;
	
	bool LoadFile(const res::path & strPathname);
	
	const res::path m_texName; // Name of texture
	u32 m_dwWidth;
	u32 m_dwHeight;
	TCFlags m_dwFlags;
	u32 userflags;
	
	Texture2D * m_pTexture; // Diffuse
	
	/*! End of the image in texture coordinates (image size divided by stored size).
	 * This is usually Vec2f::ONE but may differ if only power of two textures are supported.
	 */
	Vec2f uv;
	
	//! Size of half a pixel in normalized texture coordinates.
	Vec2f hd;
	
	TextureContainer * TextureRefinement;
	TextureContainer * m_pNext; // Linked list ptr
	TCFlags systemflags;
	
	// BEGIN TODO: Move to a RenderBatch class... This RenderBatch class should contain a pointer to the TextureContainer used by the batch
	DELAYED_PRIM * delayed; // delayed_drawing
	long	delayed_nb;
	long	delayed_max;
	
	std::vector<EERIEPOLY *> vPolyZMap;
	std::vector<SMY_ZMAPPINFO> vPolyInterZMap;
	
	SMY_ARXMAT * tMatRoom;
	
	unsigned long ulMaxVertexListCull;
	unsigned long ulNbVertexListCull;
	TexturedVertex * pVertexListCull;
	
	unsigned long ulMaxVertexListCull_TNormalTrans;
	unsigned long ulNbVertexListCull_TNormalTrans;
	TexturedVertex * pVertexListCull_TNormalTrans;
	
	unsigned long ulMaxVertexListCull_TAdditive;
	unsigned long ulNbVertexListCull_TAdditive;
	TexturedVertex * pVertexListCull_TAdditive;
	
	unsigned long ulMaxVertexListCull_TSubstractive;
	unsigned long ulNbVertexListCull_TSubstractive;
	TexturedVertex * pVertexListCull_TSubstractive;
	
	unsigned long ulMaxVertexListCull_TMultiplicative;
	unsigned long ulNbVertexListCull_TMultiplicative;
	TexturedVertex * pVertexListCull_TMultiplicative;
	
	unsigned long ulMaxVertexListCull_TMetal;
	unsigned long ulNbVertexListCull_TMetal;
	TexturedVertex * pVertexListCull_TMetal;
	// END TODO
	
private:
	
	void LookForRefinementMap(TCFlags flags);
	
	typedef std::map<res::path, res::path> RefinementMap;
	static RefinementMap s_GlobalRefine;
	static RefinementMap s_Refine;
	
};

DECLARE_FLAGS_OPERATORS(TextureContainer::TCFlags)

// Access functions for loaded textures. Note: these functions search
// an internal list of the textures, and use the texture associated with the
// ASCII name.
 
TextureContainer * GetTextureList();

// Texture creation and deletion functions

TextureContainer * GetAnyTexture();

#endif // ARX_GRAPHICS_DATA_TEXTURECONTAINER_H
