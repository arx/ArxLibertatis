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
// Code: Cyril Meynier
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved

#ifndef ARX_SCENE_OBJECT_H
#define ARX_SCENE_OBJECT_H

#include "graphics/GraphicsTypes.h"

void MakeUserFlag(TextureContainer * tc);
ObjVertGroup EERIE_OBJECT_GetGroup(const EERIE_3DOBJ * obj, const std::string & groupname);
ObjSelection EERIE_OBJECT_GetSelection(const EERIE_3DOBJ * obj, const std::string & selname);

ObjVertHandle GetGroupOriginByName(const EERIE_3DOBJ * eobj, const std::string & text);
ActionPoint GetActionPointIdx(const EERIE_3DOBJ * eobj, const std::string & text);
ObjVertGroup GetActionPointGroup(const EERIE_3DOBJ * eobj, ActionPoint idx);

/*!
 * Load a possibly cached 3D object using the default texture path.
 * 
 * \param pbox true if the object should have a physics box.
 */
EERIE_3DOBJ * loadObject(const res::path & file, bool pbox = true);

EERIE_3DOBJ * Eerie_Copy(const EERIE_3DOBJ * obj);
void EERIE_Object_Precompute_Fast_Access(EERIE_3DOBJ * object);
void EERIE_3DOBJ_RestoreTextures(EERIE_3DOBJ * eobj);
void EERIE_OBJECT_CenterObjectCoordinates(EERIE_3DOBJ * ret);
void EERIE_CreateCedricData(EERIE_3DOBJ * eobj);

#endif // ARX_SCENE_OBJECT_H
