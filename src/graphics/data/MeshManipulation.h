/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GRAPHICS_DATA_MESHMANIPULATION_H
#define ARX_GRAPHICS_DATA_MESHMANIPULATION_H

#include "graphics/GraphicsTypes.h"
#include "io/resource/ResourcePath.h"
#include "util/Flags.h"

struct EERIE_3DOBJ;
class TextureContainer;
class Entity;

enum TweakFlag {
	TWEAK_REMOVE    = 1 << 0,
	TWEAK_HEAD      = 1 << 1,
	TWEAK_TORSO     = 1 << 2,
	TWEAK_LEGS      = 1 << 3,
	TWEAK_TYPE_SKIN = 1 << 4,
	TWEAK_TYPE_ICON = 1 << 5,
	TWEAK_TYPE_MESH = 1 << 6
};
DECLARE_FLAGS(TweakFlag, TweakType)
DECLARE_FLAGS_OPERATORS(TweakType)

struct TWEAK_INFO {
	
	TweakType type;
	res::path param1;
	res::path param2;
	
};

void EERIE_MESH_TWEAK_Do(Entity * io, TweakType tw, const res::path & path);
bool IsInSelection(const EERIE_3DOBJ * obj, size_t vert, ObjSelection tw);
void AddVertexIdxToGroup(EERIE_3DOBJ * obj, size_t group, size_t val);
void EERIE_MESH_TWEAK_Skin(EERIE_3DOBJ * obj, const res::path & s1, const res::path & s2);
long ObjectAddMap(EERIE_3DOBJ * obj, TextureContainer * tc);

#endif // ARX_GRAPHICS_DATA_MESHMANIPULATION_H
