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
// EERIEUtil
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

// Desc: Helper functions and typing shortcuts for Direct3D programming.
#ifndef D3DUTIL_H
#define D3DUTIL_H

#define D3D_OVERLOADS
#include <d3d.h>
#include <ARX_Common.h>

//-----------------------------------------------------------------------------
// Miscellaneous helper functions
//-----------------------------------------------------------------------------
 

#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_TAB(p)  { if(p) { delete[] (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

//-----------------------------------------------------------------------------
// Short cut functions for creating and using DX structures
//-----------------------------------------------------------------------------
VOID D3DUtil_InitDeviceDesc(D3DDEVICEDESC7 & ddDevDesc);
VOID D3DUtil_InitSurfaceDesc(DDSURFACEDESC2 & ddsd, DWORD dwFlags = 0,
                             DWORD dwCaps = 0);
VOID D3DUtil_InitMaterial(D3DMATERIAL7 & mtrl, FLOAT r = 0.0f, FLOAT g = 0.0f,
                          FLOAT b = 0.0f, FLOAT a = 1.0f);
 
//-----------------------------------------------------------------------------
// D3D Matrix functions. For performance reasons, some functions are inline.
//-----------------------------------------------------------------------------
HRESULT D3DUtil_SetViewMatrix(D3DMATRIX & mat, D3DVECTOR & vFrom,
                              D3DVECTOR & vAt, D3DVECTOR & vUp);

inline VOID D3DUtil_SetIdentityMatrix(D3DMATRIX & m)
{
	m._12 = m._13 = m._14 = m._21 = m._23 = m._24 = 0.0f;
	m._31 = m._32 = m._34 = m._41 = m._42 = m._43 = 0.0f;
	m._11 = m._22 = m._33 = m._44 = 1.0f;
}

inline VOID D3DUtil_SetTranslateMatrix(D3DMATRIX & m, FLOAT tx, FLOAT ty,
                                       FLOAT tz)
{
	D3DUtil_SetIdentityMatrix(m);
	m._41 = tx;
	m._42 = ty;
	m._43 = tz;
}

inline VOID D3DUtil_SetTranslateMatrix(D3DMATRIX & m, D3DVECTOR & v)
{
	D3DUtil_SetTranslateMatrix(m, v.x, v.y, v.z);
}

//-----------------------------------------------------------------------------
// Debug printing support
//-----------------------------------------------------------------------------

HRESULT _DbgOut(TCHAR *, DWORD, HRESULT, TCHAR *);

#if defined(DEBUG) | defined(_DEBUG)
#define DEBUG_MSG(str)    _DbgOut( __FILE__, (DWORD)__LINE__, 0, str )
#define DEBUG_ERR(hr,str) _DbgOut( __FILE__, (DWORD)__LINE__, hr, str )
#else
#define DEBUG_MSG(str)    (0L)
#define DEBUG_ERR(hr,str) (hr)
#endif


#endif // D3DUTIL_H
