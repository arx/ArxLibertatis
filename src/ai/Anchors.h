/*
 * Copyright 2011-2017 Arx Libertatis Team (see the AUTHORS file)
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
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#ifndef ARX_AI_ANCHORS_H
#define ARX_AI_ANCHORS_H

#include <vector>

#include "math/Vector.h"
#include "util/Flags.h"

struct BackgroundData;
struct Cylinder;
class Entity;

enum AnchorFlag {
	ANCHOR_FLAG_BLOCKED = 1 << 3
};
DECLARE_FLAGS(AnchorFlag, AnchorFlags)
DECLARE_FLAGS_OPERATORS(AnchorFlags)

struct ANCHOR_DATA {
	
	Vec3f pos;
	AnchorFlags flags;
	std::vector<long> linked;
	float radius;
	float height;
	
	ANCHOR_DATA()
		: pos(Vec3f_ZERO)
		, flags(0)
		, radius(0)
		, height(0)
	{ }
	
};

/*!
 * Clears all Anchor data from a Background
 */
void AnchorData_ClearAll(BackgroundData * eb);

void ANCHOR_BLOCK_By_IO(Entity * io, long status);
void ANCHOR_BLOCK_Clear();

#endif // ARX_AI_ANCHORS_H
