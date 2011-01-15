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
#ifndef ARX_CEDRIC_H
#define ARX_CEDRIC_H
#include "danae.h"


#define		CEDRIC				1
#define		REFERENCE_FPS		(20.0f)
#define 	ANIMQUATTYPE_FIRST_PERSON	2
#define 	ANIMQUATTYPE_NO_RENDER		4
#define 	MIPMESH_START 				380.f
#define 	MIPMESH_DIV	  				DIV190

#if CEDRIC
extern	void	Cedric_AnimateDrawEntity(LPDIRECT3DDEVICE7 pd3dDevice, EERIE_3DOBJ * eobj,
        ANIM_USE * animuse, EERIE_3D * angle, EERIE_3D * pos,
        INTERACTIVE_OBJ * io, D3DCOLOR col, long typ);

#endif
#endif