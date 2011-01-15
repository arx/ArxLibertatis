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

// Desc: Shortcut macros and functions for using DX objects
#define D3D_OVERLOADS
#define STRICT
#include <math.h>
#include <stdio.h>
#include <tchar.h>

#include "EerieUtil.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

//-----------------------------------------------------------------------------
// Name: D3DUtil_InitSurfaceDesc()
// Desc: Helper function called to build a DDSURFACEDESC2 structure,
//       typically before calling CreateSurface() or GetSurfaceDesc()
//-----------------------------------------------------------------------------
VOID D3DUtil_InitSurfaceDesc(DDSURFACEDESC2 & ddsd, DWORD dwFlags,
                             DWORD dwCaps)
{
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize                 = sizeof(ddsd);
	ddsd.dwFlags                = dwFlags;
	ddsd.ddsCaps.dwCaps         = dwCaps;
	ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
}

//-----------------------------------------------------------------------------
// Name: D3DUtil_InitMaterial()
// Desc: Helper function called to build a D3DMATERIAL7 structure
//-----------------------------------------------------------------------------
VOID D3DUtil_InitMaterial(D3DMATERIAL7 & mtrl, FLOAT r, FLOAT g, FLOAT b,
                          FLOAT a)
{
	ZeroMemory(&mtrl, sizeof(D3DMATERIAL7));
	mtrl.dcvDiffuse.r = mtrl.dcvAmbient.r = r;
	mtrl.dcvDiffuse.g = mtrl.dcvAmbient.g = g;
	mtrl.dcvDiffuse.b = mtrl.dcvAmbient.b = b;
	mtrl.dcvDiffuse.a = mtrl.dcvAmbient.a = a;
}

//-----------------------------------------------------------------------------
// Name: D3DUtil_SetViewMatrix()
// Desc: Given an eye point, a lookat point, and an up vector, this
//       function builds a 4x4 view matrix.
//-----------------------------------------------------------------------------
HRESULT D3DUtil_SetViewMatrix(D3DMATRIX & mat, D3DVECTOR & vFrom,
                              D3DVECTOR & vAt, D3DVECTOR & vWorldUp)
{
	// Get the z basis vector, which points straight ahead. This is the
	// difference from the eyepoint to the lookat point.
	D3DVECTOR vView = vAt - vFrom;

	FLOAT fLength = Magnitude(vView);

	if (fLength < 1e-6f)
		return E_INVALIDARG;

	// Normalize the z basis vector
	vView /= fLength;

	// Get the dot product, and calculate the projection of the z basis
	// vector onto the up vector. The projection is the y basis vector.
	FLOAT fDotProduct = DotProduct(vWorldUp, vView);

	D3DVECTOR vUp = vWorldUp - fDotProduct * vView;

	// If this vector has near-zero length because the input specified a
	// bogus up vector, let's try a default up vector
	if (1e-6f > (fLength = Magnitude(vUp)))
	{
		vUp = D3DVECTOR(0.0f, 1.0f, 0.0f) - vView.y * vView;

		// If we still have near-zero length, resort to a different axis.
		if (1e-6f > (fLength = Magnitude(vUp)))
		{
			vUp = D3DVECTOR(0.0f, 0.0f, 1.0f) - vView.z * vView;

			if (1e-6f > (fLength = Magnitude(vUp)))
				return E_INVALIDARG;
		}
	}

	// Normalize the y basis vector
	vUp /= fLength;

	// The x basis vector is found simply with the cross product of the y
	// and z basis vectors
	D3DVECTOR vRight = CrossProduct(vUp, vView);

	// Start building the matrix. The first three rows contains the basis
	// vectors used to rotate the view to point at the lookat point
	D3DUtil_SetIdentityMatrix(mat);
	mat._11 = vRight.x;
	mat._12 = vUp.x;
	mat._13 = vView.x;
	mat._21 = vRight.y;
	mat._22 = vUp.y;
	mat._23 = vView.y;
	mat._31 = vRight.z;
	mat._32 = vUp.z;
	mat._33 = vView.z;

	// Do the translation values (rotations are still about the eyepoint)
	mat._41 = - DotProduct(vFrom, vRight);
	mat._42 = - DotProduct(vFrom, vUp);
	mat._43 = - DotProduct(vFrom, vView);

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: _DbgOut()
// Desc: Outputs a message to the debug stream
//-----------------------------------------------------------------------------
HRESULT _DbgOut(TCHAR * strFile, DWORD dwLine, HRESULT hr, TCHAR * strMsg)
{
	TCHAR buffer[256];
	wsprintf(buffer, _T("%s(%ld): "), strFile, dwLine);
	OutputDebugString(buffer);
	OutputDebugString(strMsg);

	if (hr)
	{
		wsprintf(buffer, _T("(hr=%08lx)\n"), hr);
		OutputDebugString(buffer);
	}

	OutputDebugString(_T("\n"));

	return hr;
}

