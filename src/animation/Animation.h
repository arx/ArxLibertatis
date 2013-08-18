/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved

#ifndef  ARX_ANIMATION_ANIMATION_H
#define  ARX_ANIMATION_ANIMATION_H

#include <stddef.h>
#include <string>

#include "math/MathFwd.h"
#include "graphics/BaseGraphicsTypes.h"
#include "graphics/GraphicsTypes.h"

class Entity;
struct ANIM_USE;

const size_t MAX_ANIMATIONS = 900;


struct EERIE_FRAME
{
	long		num_frame;
	long		flag;
	int			master_key_frame;
	short		f_translate; //int
	short		f_rotate; //int
	float		time;
	Vec3f	translate;
	EERIE_QUAT	quat;
	audio::SampleId	sample;
};

struct EERIE_GROUP
{
	int		key;
	Vec3f	translate;
	EERIE_QUAT	quat;
	Vec3f	zoom;
};

struct EERIE_ANIM
{
	long		anim_time;
	unsigned long	flag;
	long		nb_groups;
	long		nb_key_frames;
	EERIE_FRAME *	frames;
	EERIE_GROUP  *  groups;
	unsigned char *	voidgroups;
};

struct ANIM_HANDLE {

	ANIM_HANDLE();

	res::path path; // empty path means an unallocated slot
	EERIE_ANIM ** anims;
	short alt_nb;
	long locks;
};

short ANIM_GetAltIdx(ANIM_HANDLE * ah,long old);
void ANIM_Set(ANIM_USE * au,ANIM_HANDLE * anim);

void GetAnimTotalTranslate( ANIM_HANDLE * eanim,long alt_idx,Vec3f * pos);

long EERIE_ANIMMANAGER_Count(std::string & tex, long * memsize);
void EERIE_ANIMMANAGER_ClearAll();
void EERIE_ANIMMANAGER_PurgeUnused();
void EERIE_ANIMMANAGER_ReleaseHandle(ANIM_HANDLE * anim);
ANIM_HANDLE * EERIE_ANIMMANAGER_Load(const res::path & path);
ANIM_HANDLE * EERIE_ANIMMANAGER_Load_NoWarning(const res::path & path);

void PrepareAnim(ANIM_USE *eanim, unsigned long time, Entity *io);
void ResetAnim(ANIM_USE * eanim);

void EERIE_ANIMMANAGER_ReloadAll();

void AcquireLastAnim(Entity * io);
void FinishAnim(Entity * io,ANIM_HANDLE * eanim);

#endif // ARX_ANIMATION_ANIMATION_H
