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

#include "graphics/GraphicsTypes.h"
#include "graphics/texture/Texture.h"
#include <vector>
#include <map>
#include <string>

using std::vector;

extern bool EERIE_USES_BUMP_MAP;
extern long GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE;

struct DELAYED_PRIM
{
	EERIEPOLY * data;
};

/** Linked list structure to hold info per texture.
 *	@todo This class is currently an hybrid between a texture class and a render batch... We should create a RenderBatch class for all vertex stuff.
 */
class TextureContainer
{
public:
	enum TCFlags
	{
		None	     = 0x00,
		NoMipmap     = 0x01,
		NoInsert     = 0x02,
		NoRefinement = 0x04,
		FakeBorder   = 0x08,		
		Level        = 0x10,		
		UI           = (NoMipmap | NoRefinement),
		All			 = 0xFFFFFFFF
	};

public:
	/** Constructor.
	 *	Only TextureContainer::Load() is allowed to create TextureContainer, but pieces of code still depend on this constructor being public.
	 *	@param  
	 *	@todo Make this constructor private.
	 */ 
	TextureContainer(const std::string& strName, TCFlags flags);

	/** Destructor
	 */
	~TextureContainer();

	/**	Load an image into a TextureContainer
	 *
	 *
	 */
	static TextureContainer * Load(const std::string& strName, TCFlags flags = None);

	/**	Load an image into a TextureContainer
	 *
	 *
	 */
	static TextureContainer * LoadUI(const std::string& strName, TCFlags flags = None);

	/** Find a TextureContainer by its name.
	 *	Searches the internal list of textures for a texture specified by
	 *	its name. Returns the structure associated with that texture.
	 *	@param strTextureName Name of the texture to find.
	 *  @return A pointer to a TextureContainer if this texture was already loaded, NULL otherwise.
	 **/
	static TextureContainer * Find(const std::string& strTextureName);

	static void DeleteAll(TCFlags flag = All);

	/**	Create a texture to display a glowing halo around a transparent texture
	 *	@todo Rewrite this feature using shaders instead of hacking a texture effect
	 */
	bool CreateHalo() { return false; }
	TextureContainer * TextureHalo;

	bool LoadFile(const std::string& strPathname);

	std::string m_texName;              // Name of texture
	DWORD   m_dwWidth;
	DWORD   m_dwHeight;
	DWORD	m_dwDeviceWidth;
	DWORD	m_dwDeviceHeight;
	DWORD   m_dwBPP;
	TCFlags m_dwFlags;
	bool    m_bHasAlpha;
	DWORD	userflags;
	bool	bColorKey;
	bool	bColorKey2D;

	Texture2D* m_pddsSurface;			// Surface of the texture
	Texture2D* m_pddsBumpMap;			// Surface of BumpMap

	// Precalculated values
	float	m_dx;						// 1.f / width
	float	m_dy;						// 1.f / height
	float	m_hdx;						// 0.5f / width
	float	m_hdy;						// 0.5f / height
	float	m_odx;						// 1.f / width
	float	m_ody;						// 1.f / height

	TextureContainer *	TextureRefinement;
	TextureContainer *	m_pNext;         // Linked list ptr
	long				systemflags;
	
	// BEGIN TODO: Move to a RenderBatch class... This RenderBatch class should contain a pointer to the TextureContainer used by the batch
	DELAYED_PRIM *	delayed;			// delayed_drawing
	long	delayed_nb;
	long	delayed_max;
	
	std::vector<EERIEPOLY *> vPolyZMap;
	std::vector<EERIEPOLY *> vPolyBump;
	std::vector<D3DTLVERTEX> vPolyInterBump;
	std::vector<SMY_ZMAPPINFO> vPolyInterZMap;

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
	// END TODO

private:
	void LookForRefinementMap(TextureContainer::TCFlags flags);

	typedef std::map<std::string, std::string> RefinementMap;
	static RefinementMap s_GlobalRefine;
	static RefinementMap s_Refine;
};


//-----------------------------------------------------------------------------
// Access functions for loaded textures. Note: these functions search
// an internal list of the textures, and use the texture associated with the
// ASCII name.
//-----------------------------------------------------------------------------
 
TextureContainer * GetTextureList();
long CountTextures( std::string& tex, long * memsize, long * memmip);
 
#define D3DCOLORWHITE 0xFFFFFFFF
#define D3DCOLORBLACK 0xFF000000

//-----------------------------------------------------------------------------
// Texture creation and deletion functions
//-----------------------------------------------------------------------------

TextureContainer * GetAnyTexture();

void EERIE_ActivateBump();
void EERIE_DesactivateBump();

#endif
