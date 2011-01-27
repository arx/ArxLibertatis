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

#ifndef EERIEDRAW_H
#define EERIEDRAW_H

#define D3D_OVERLOADS

#define EERIE_USEVB		0x8000	// using vertex buffer for EERIEDRAWPRIM
#define EERIE_NOCOUNT	0x4000	// do not incr EERIEDrawnPolys for EERIEDRAWPRIM

#include <d3d.h>
#include "EERIEPoly.h"
#include "EERIETexture.h"
#include "EERIEMath.h"
#include "EERIEApp.h"

extern long ARX_ALTERNATE_3D;
extern EERIE_3D SPRmins;
extern EERIE_3D SPRmaxs;

void MDL_AddMetalDrawLater(D3DTLVERTEX * tv);
void MDL_FlushAll(LPDIRECT3DDEVICE7 pd3dDevice);


void Delayed_EERIEDRAWPRIM(EERIEPOLY * ep);
void Delayed_FlushAll(LPDIRECT3DDEVICE7 pd3dDevice);

HRESULT EERIEDRAWPRIM(LPDIRECT3DDEVICE7 pd3dDevice,
                      D3DPRIMITIVETYPE dptPrimitiveType,
                      DWORD  dwVertexTypeDesc,
                      LPVOID lpvVertices,
                      DWORD  dwVertexCount,
                      DWORD  dwFlags,					//d3d flag
                      long flags = 0,					//eerie flag
                      EERIEPOLY * ep = NULL
                     );

void EERIE_DrawPolyBump(LPDIRECT3DDEVICE7 pd3dDevice, EERIEPOLY * ep, float alpha);
void EERIEDrawLine(float x, float y, float x1, float y1, float z, D3DCOLOR col);
void EERIEDrawCircle(float x0, float y0, float r, D3DCOLOR col, float z);
void EERIEDraw2DLine(LPDIRECT3DDEVICE7 pd3dDevice, float x0, float y0, float x1, float y1, float z, D3DCOLOR col);
void EERIEDrawBitmap(LPDIRECT3DDEVICE7 pd3dDevice, float x, float y, float sx, float sy, float z, TextureContainer * tex, D3DCOLOR col);
void EERIEDraw2DRect(LPDIRECT3DDEVICE7 pd3dDevice, float x0, float y0, float x1, float y1, float z, D3DCOLOR col);
void EERIEDrawFill2DRectDegrad(LPDIRECT3DDEVICE7 pd3dDevice, float x0, float y0, float x1, float y1, float z, D3DCOLOR cold, D3DCOLOR cole);

void DRAWLATER_ReInit();
void DRAWLATER_Render(LPDIRECT3DDEVICE7 pd3dDevice);

void EERIEDraw3DCylinder(LPDIRECT3DDEVICE7 pd3dDevice, EERIE_CYLINDER * cyl, D3DCOLOR col);
void EERIEDraw3DCylinderBase(LPDIRECT3DDEVICE7 pd3dDevice, EERIE_CYLINDER * cyl, D3DCOLOR col);
void EERIEDrawTrue3DLine(LPDIRECT3DDEVICE7 pd3dDevice, EERIE_3D * orgn, EERIE_3D * dest, D3DCOLOR col);
void EERIEDraw3DLine(LPDIRECT3DDEVICE7 pd3dDevice, EERIE_3D * orgn, EERIE_3D * dest, D3DCOLOR col);
void EERIEDrawBitmap2DecalY(LPDIRECT3DDEVICE7 pd3dDevice, float x, float y, float sx, float sy, float z, TextureContainer * tex, D3DCOLOR col, float _fDeltaY);

void EERIEOBJECT_Quadify(EERIE_3DOBJ * obj);
void EERIE_DRAW_SetTextureZMAP(long num, TextureContainer * Z_map);

void EERIEDrawSprite(LPDIRECT3DDEVICE7 pd3dDevice, D3DTLVERTEX * in, float siz, TextureContainer * tex, D3DCOLOR col, float Zpos);
void EERIEDrawRotatedSprite(LPDIRECT3DDEVICE7 pd3dDevice, D3DTLVERTEX * in, float siz, TextureContainer * tex, D3DCOLOR col, float Zpos, float rot);

void SETTEXTURE0(LPDIRECT3DDEVICE7 pd3dDevice, IDirectDrawSurface7 * tex);

__forceinline void SETTC(LPDIRECT3DDEVICE7 pd3dDevice, TextureContainer * tc)
{
	if ( (!tc) || (!tc->m_pddsSurface) )
	{
		pd3dDevice->SetTexture( 0, NULL );
	}
	else
	{
		if ( tc->bColorKey )
		{
			pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, true);

			if (	(Project.bits == 16) &&
			        (!tc->bColorKey2D)	 )
			{
				SetZBias( pd3dDevice, -4 );
			}
		}
		else
		{
			pd3dDevice->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, false );
		}
		pd3dDevice->SetTexture(0, tc->m_pddsSurface);
	}
}

void SETCULL(LPDIRECT3DDEVICE7 pd3dDevice, DWORD state);
void SETZWRITE(LPDIRECT3DDEVICE7 pd3dDevice, DWORD state);
void SETALPHABLEND(LPDIRECT3DDEVICE7 pd3dDevice, DWORD state);
void SETBLENDMODE(LPDIRECT3DDEVICE7 pd3dDevice, DWORD srcblend, DWORD destblend);
void SETTEXTUREWRAPMODE(LPDIRECT3DDEVICE7 pd3dDevice, DWORD mode);
void EERIEPOLY_DrawWired(LPDIRECT3DDEVICE7 pd3dDevice, EERIEPOLY * ep, long col = 0);
void EERIEPOLY_DrawNormals(LPDIRECT3DDEVICE7 pd3dDevice, EERIEPOLY * ep);

extern TextureContainer * EERIE_DRAW_sphere_particle;
extern TextureContainer * EERIE_DRAW_square_particle;

void EERIEDrawBitmap2(LPDIRECT3DDEVICE7 pd3dDevice, float x, float y, float sx, float sy, float z, TextureContainer * tex, D3DCOLOR col);
void EERIEDrawBitmap_uv(LPDIRECT3DDEVICE7 pd3dDevice, float x, float y, float sx, float sy, float z, TextureContainer * tex, D3DCOLOR col, float u0, float v0, float u1, float v1);
void EERIEDrawBitmapUVs(LPDIRECT3DDEVICE7 pd3dDevice, float x, float y, float sx, float sy, float z, TextureContainer * tex, D3DCOLOR col
                        , float u0, float v0
                        , float u1, float v1
                        , float u2, float v2
                        , float u3, float v3
                       );

void SET_FORCE_NO_VB( const bool& _NoVB );
bool GET_FORCE_NO_VB( );

#endif
