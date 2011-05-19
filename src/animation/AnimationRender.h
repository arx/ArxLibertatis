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

#ifndef ARX_ANIMATION_ANIMATIONRENDER_H
#define ARX_ANIMATION_ANIMATIONRENDER_H

#include "graphics/d3dwrapper.h"
#include "platform/math/Vector3.h"
#include "platform/math/Angle.h"

struct EERIE_3DOBJ;
struct ANIM_USE;
struct INTERACTIVE_OBJ;
struct EERIE_VERTEX;
struct EERIE_FACE;
struct EERIE_RGB;
struct EERIEMATRIX;
struct EERIE_QUAT;
class TextureContainer;

extern float LPpower;

#define CEDRIC 1
#define REFERENCE_FPS (20.0f)
#define ANIMQUATTYPE_FIRST_PERSON 2
#define ANIMQUATTYPE_NO_RENDER 4
#define MIPMESH_START 380.f
#define MIPMESH_DIV (1.0f / 190)

#if CEDRIC

void Cedric_AnimateDrawEntity(EERIE_3DOBJ * eobj, ANIM_USE * animuse, Anglef * angle, Vec3f * pos, INTERACTIVE_OBJ * io, long typ);

int ARX_SoftClippZ(EERIE_VERTEX * _pVertex1, EERIE_VERTEX * _pVertex2, EERIE_VERTEX * _pVertex3, D3DTLVERTEX ** _ptV, EERIE_FACE * _pFace, float _fInvibility, TextureContainer * _pTex, bool _bBump, bool _bZMapp, EERIE_3DOBJ * _pObj, int _iNumFace, long * _pInd, INTERACTIVE_OBJ * _pioInteractive, bool _bNPC, long _lSpecialColorFlag, EERIE_RGB * _pRGB);

#endif // CEDRIC

void MakeCLight(INTERACTIVE_OBJ * io, EERIE_RGB * infra, Anglef * angle, Vec3f * pos, EERIE_3DOBJ * eobj, EERIEMATRIX * BIGMAT, EERIE_QUAT * BIGQUAT);
void MakeCLight2(INTERACTIVE_OBJ * io,EERIE_RGB * infra, Anglef * angle,Vec3f * pos, EERIE_3DOBJ * eobj, EERIEMATRIX * BIGMAT, EERIE_QUAT * BIGQUAT, long i);

#endif // ARX_ANIMATION_ANIMATIONRENDER_H
