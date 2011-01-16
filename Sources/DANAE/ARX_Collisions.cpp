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
// ARX_Collisions
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Collisions Management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include "ARX_Collisions.h"
#include "HERMESMain.h"
#include "EERIEMath.h"
#include "ARX_Damages.h"
#include "ARX_Interactive.h"
#include "ARX_NPC.h"
#include "ARX_Time.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

//-----------------------------------------------------------------------------
#define MAX_IN_SPHERE 20

//-----------------------------------------------------------------------------
extern float FrameDiff;
long ON_PLATFORM=0;
//-----------------------------------------------------------------------------
long MAX_IN_SPHERE_Pos=0;
short EVERYTHING_IN_SPHERE[MAX_IN_SPHERE+1];
long EXCEPTIONS_LIST_Pos=0;
short EXCEPTIONS_LIST[MAX_IN_SPHERE+1];
 
long POLYIN=0;
long COLLIDED_CLIMB_POLY=0;
INTERACTIVE_OBJ * PUSHABLE_NPC=NULL;
long MOVING_CYLINDER=0;
 
EERIE_3D vector2D;
BOOL DIRECT_PATH=TRUE;
long APPLY_PUSH=0;

//-----------------------------------------------------------------------------
// Added immediate return (return anything;)
__inline float IsPolyInCylinder(EERIEPOLY *ep, EERIE_CYLINDER * cyl,long flag)
{
	long flags=flag;
	POLYIN=0;
	float min = cyl->origin.y + cyl->height; 
	float max = cyl->origin.y; 

	if (min>ep->max.y) return 999999.f;

	if (max<ep->min.y) return 999999.f;
	
	long to;

	if (ep->type & POLY_QUAD) to=4;
	else to=3;

	float nearest = 99999999.f;

	for (long num=0;num<to;num++)
	{
		float dd=Distance2D(ep->v[num].sx,ep->v[num].sz,cyl->origin.x,cyl->origin.z);

		if (dd<nearest)
		{
			nearest=dd;
		}		
	}

	if (nearest>__max(82.f,cyl->radius)) return 999999.f;

	if (	(cyl->radius<30.f)
		||	(cyl->height>-80.f)
		||	(ep->area>5000.f)	)
		flags|=CFLAG_EXTRA_PRECISION;

	if (!(flags & CFLAG_EXTRA_PRECISION))
	{
		if (ep->area<100.f) return 999999.f;
	}
	
	float anything=999999.f;

	if (PointInCylinder(cyl, &ep->center)) 
	{	
		POLYIN = 1;
		
		if (ep->norm.y<0.5f)
			anything=__min(anything,ep->min.y);
		else
			anything=__min(anything,ep->center.y);

		if (!(flags & CFLAG_EXTRA_PRECISION)) return anything;
	}

	
	long r=to-1;
	
	EERIE_3D center;
	long n;
	
	for (n=0;n<to;n++)
	{
		if (flags & CFLAG_EXTRA_PRECISION)
		{
			for (long o=0;o<5;o++)
			{
				float p=(float)o*DIV5;
				center.x=(ep->v[n].sx*p+ep->center.x*(1.f-p));
				center.y=(ep->v[n].sy*p+ep->center.y*(1.f-p));
				center.z=(ep->v[n].sz*p+ep->center.z*(1.f-p));

				if (PointInCylinder(cyl, &center)) 
				{	
					anything=__min(anything,center.y);
					POLYIN=1;

					if (!(flags & CFLAG_EXTRA_PRECISION)) return anything;
				}
			}
		}

		if ((ep->area>2000.f) 
		        || (flags & CFLAG_EXTRA_PRECISION)  )
		{
			center.x=(ep->v[n].sx+ep->v[r].sx)*DIV2;
			center.y=(ep->v[n].sy+ep->v[r].sy)*DIV2;
			center.z=(ep->v[n].sz+ep->v[r].sz)*DIV2;

			if (PointInCylinder(cyl, &center)) 
			{	
				anything=__min(anything,center.y);
				POLYIN=1;

				if (!(flags & CFLAG_EXTRA_PRECISION)) return anything;
			}

			if ((ep->area>4000.f) || (flags & CFLAG_EXTRA_PRECISION))
			{
				center.x=(ep->v[n].sx+ep->center.x)*DIV2;
				center.y=(ep->v[n].sy+ep->center.y)*DIV2;
				center.z=(ep->v[n].sz+ep->center.z)*DIV2;

				if (PointInCylinder(cyl, &center)) 
				{	
					anything=__min(anything,center.y);
					POLYIN=1;

					if (!(flags & CFLAG_EXTRA_PRECISION)) return anything;
				}
			}

			if ((ep->area>6000.f) || (flags & CFLAG_EXTRA_PRECISION))
			{
				center.x=(center.x+ep->v[n].sx)*DIV2;
				center.y=(center.y+ep->v[n].sy)*DIV2;
				center.z=(center.z+ep->v[n].sz)*DIV2;

				if (PointInCylinder(cyl, &center))
				{
					anything=__min(anything,center.y);
					POLYIN=1;

					if (!(flags & CFLAG_EXTRA_PRECISION)) return anything;
				}
			}
		}

		if (PointInCylinder(cyl, (EERIE_3D *)&ep->v[n]))
		{
			anything=__min(anything,ep->v[n].sy);			
			POLYIN=1;

			if (!(flags & CFLAG_EXTRA_PRECISION)) return anything;
		}

		r++;

		if (r>=to) r=0;
	
	}


//	To Add "more" precision

/*if (flags & CFLAG_EXTRA_PRECISION)
{

	for (long j=0;j<360;j+=90)
	{
		float xx=-EEsin(DEG2RAD((float)j))*cyl->radius;
		float yy=EEcos(DEG2RAD((float)j))*cyl->radius;
		EERIE_3D pos;
		pos.x=cyl->origin.x+xx;

		pos.z=cyl->origin.z+yy;
		//EERIEPOLY * epp;

		if (PointIn2DPolyXZ(ep, pos.x, pos.z)) 
		{
			if (GetTruePolyY(ep,&pos,&xx))
			{				
				anything=__min(anything,xx);
				return anything;
			}
		}
	} 
//}*/
	if ((anything!=999999.f) && (ep->norm.y<0.1f) && (ep->norm.y>-0.1f))
		anything=__min(anything,ep->min.y);

	return anything;
}

//-----------------------------------------------------------------------------
__inline BOOL IsPolyInSphere(EERIEPOLY *ep, EERIE_SPHERE * sph)
{
	if ((!ep) || (!sph)) return FALSE;

	if (ep->area<100.f) return FALSE;

	long to;

	if (ep->type & POLY_QUAD) to=4;
	else to=3;

	long r=to-1;
	EERIE_3D center;

	for (long n=0;n<to;n++)
	{
		if (ep->area>2000.f)
		{
			center.x=(ep->v[n].sx+ep->v[r].sx)*DIV2;
			center.y=(ep->v[n].sy+ep->v[r].sy)*DIV2;
			center.z=(ep->v[n].sz+ep->v[r].sz)*DIV2;

			if (EEDistance3D(&sph->origin, &center) < sph->radius) 
			{	
				return TRUE;
			}

			if (ep->area>4000.f)
			{
				center.x=(ep->v[n].sx+ep->center.x)*DIV2;
				center.y=(ep->v[n].sy+ep->center.y)*DIV2;
				center.z=(ep->v[n].sz+ep->center.z)*DIV2;

				if (EEDistance3D(&sph->origin, &center) < sph->radius) 
				{	
					return TRUE;
				}
			}

			if (ep->area>6000.f)
			{
				center.x=(center.x+ep->v[n].sx)*DIV2;
				center.y=(center.y+ep->v[n].sy)*DIV2;
				center.z=(center.z+ep->v[n].sz)*DIV2;

				if (EEDistance3D(&sph->origin, &center) < sph->radius) 
				{	
					return TRUE;
				}
			}
		}

		if (EEDistance3D(&sph->origin, (EERIE_3D *)&ep->v[n]) < sph->radius) 
		{	
			return TRUE;
		}

		r++;

		if (r>=to) r=0;	
	}

	return FALSE;
}

//-----------------------------------------------------------------------------
BOOL IsCollidingIO(INTERACTIVE_OBJ * io,INTERACTIVE_OBJ * ioo)
{
	if (   (ioo!=NULL)
		&& (io!=ioo)
		&& !(ioo->ioflags & IO_NO_COLLISIONS)
		&& (ioo->show==SHOW_FLAG_IN_SCENE) 
		&& (ioo->obj)
		)
	{
		if (ioo->ioflags & IO_NPC)
		{
			float old=ioo->physics.cyl.radius;
			ioo->physics.cyl.radius+=25.f;

			for (long j=0;j<io->obj->nbvertex;j++)
			{
				if (PointInCylinder(&ioo->physics.cyl,&io->obj->vertexlist3[j].v)) 
				{
					ioo->physics.cyl.radius=old;
					return TRUE;
				}
			}

			ioo->physics.cyl.radius=old;
		}
	}
	
	return FALSE;
}

extern void GetIOCyl(INTERACTIVE_OBJ * io,EERIE_CYLINDER * cyl);
void PushIO_ON_Top(INTERACTIVE_OBJ * ioo,float ydec)
{
	if (ydec!=0.f)
	for (long i=0;i<inter.nbmax;i++) 
	{
		INTERACTIVE_OBJ * io=inter.iobj[i];

		if (   (io)
			&& (io!=ioo)
			&& !(io->ioflags & IO_NO_COLLISIONS)
			&& (io->show==SHOW_FLAG_IN_SCENE) 
			&& (io->obj)
			&& (!(io->ioflags & (IO_FIX | IO_CAMERA | IO_MARKER)))
			)
		{
			if (Distance2D(io->pos.x,io->pos.z,ioo->pos.x,ioo->pos.z)<450.f)
			{
				EERIEPOLY ep;
				ep.type=0;
				float miny=9999999.f;
				float maxy=-9999999.f;

				for (long ii=0;ii<ioo->obj->nbvertex;ii++)
				{
					miny=__min(miny,ioo->obj->vertexlist3[ii].v.y);
					maxy=__max(maxy,ioo->obj->vertexlist3[ii].v.y);
				}

				float posy;

				if (io==inter.iobj[0])
					posy=player.pos.y-PLAYER_BASE_HEIGHT;					
				else 
					posy=io->pos.y;

				float modd=0;

				if (ydec>0)
					modd=-20.f;

				if ((posy<=maxy) && (posy>=miny+modd))
				{
					for (long ii=0;ii<ioo->obj->nbfaces;ii++)
					{
						float cx=0;
						float cz=0;

						for (long kk=0;kk<3;kk++)
						{
							cx+=ep.v[kk].sx=ioo->obj->vertexlist3[ioo->obj->facelist[ii].vid[kk]].v.x;
							ep.v[kk].sy=ioo->obj->vertexlist3[ioo->obj->facelist[ii].vid[kk]].v.y;
							cz+=ep.v[kk].sz=ioo->obj->vertexlist3[ioo->obj->facelist[ii].vid[kk]].v.z;
						}

						cx*=DIV3;
						cz*=DIV3;
							
						float tval=1.1f;

						for (int kk=0;kk<3;kk++)
						{
								ep.v[kk].sx = (ep.v[kk].sx - cx) * tval + cx; 
								ep.v[kk].sz = (ep.v[kk].sz - cz) * tval + cz; 
						}

						if (PointIn2DPolyXZ(&ep, io->pos.x, io->pos.z)) 
						{
							EERIE_CYLINDER cyl;

							if (io==inter.iobj[0])
							{
								if (ydec<=0)
								{
									player.pos.y+=ydec;
									moveto.y+=ydec;
									cyl.origin.x=player.pos.x;
									cyl.origin.y=player.pos.y+170.f+ydec;
									cyl.origin.z=player.pos.z;	
									cyl.height=PLAYER_BASE_HEIGHT;
									cyl.radius=PLAYER_BASE_RADIUS;
									float vv;

									if ((vv=CheckAnythingInCylinder(&cyl,inter.iobj[0],0))<0)
									{
										player.pos.y+=ydec+vv;
									}
								}
								else
								{
									cyl.origin.x=player.pos.x;
									cyl.origin.y=player.pos.y+170.f+ydec;
									cyl.origin.z=player.pos.z;	
									cyl.height=PLAYER_BASE_HEIGHT;
									cyl.radius=PLAYER_BASE_RADIUS;

									if (CheckAnythingInCylinder(&cyl,inter.iobj[0],0)>=0)
									{
										player.pos.y+=ydec;
										moveto.y+=ydec;
									}
								}
							}
							else
							{
								if (ydec<=0)
								{
									io->pos.y+=ydec;
								}
								else
								{
									GetIOCyl(io,&cyl);
									cyl.origin.y=io->pos.y+ydec;

									if (CheckAnythingInCylinder(&cyl,io,0)>=0)
										io->pos.y+=ydec;
								}
							}

							break;					
						}
					}	
				}
			}
		}
	}		
}
//-----------------------------------------------------------------------------

bool IsAnyNPCInPlatform(INTERACTIVE_OBJ * pfrm)
{
	for (long i=0;i<inter.nbmax;i++)
	{
		INTERACTIVE_OBJ * io=inter.iobj[i];

		if (	(io) 
			&&	(io!=pfrm)
			&&	(io->ioflags & IO_NPC) 
			&&	!(io->ioflags & IO_NO_COLLISIONS)			
			&&	(io->show==SHOW_FLAG_IN_SCENE)	
			)
		{
			EERIE_CYLINDER cyl;
			GetIOCyl(io,&cyl);

			if (CylinderPlatformCollide(&cyl,pfrm)!=0.f) return TRUE;
		}
	}

	return FALSE;
}
float CylinderPlatformCollide(EERIE_CYLINDER * cyl,INTERACTIVE_OBJ * io)
{
 
	float miny,maxy;
	miny=io->bbox3D.min.y;
	maxy=io->bbox3D.max.y;
			
	if (maxy <= cyl->origin.y + cyl->height) return 0; 
												
	if (miny >= cyl->origin.y) return 0; 
	
	if (In3DBBoxTolerance(&cyl->origin,&io->bbox3D,cyl->radius))
	{
		return 1.f;
				}

	return 0.f;
				}
											
long NPC_IN_CYLINDER=0;

// backup du dernier polingue
EERIEPOLY * pEPBackup = NULL;
int			iXBackup = -1;
int			iYBackup = -1;
extern int TSU_TEST_COLLISIONS;
			
	
extern void GetIOCyl(INTERACTIVE_OBJ * io,EERIE_CYLINDER * cyl);

__inline void EE_RotateY(D3DTLVERTEX *in,D3DTLVERTEX *out,float c, float s)
{										 
	out->sx = (in->sx*c) + (in->sz*s);
	out->sy = in->sy;
	out->sz = (in->sz*c) - (in->sx*s);
}

bool CollidedFromBack(INTERACTIVE_OBJ * io,INTERACTIVE_OBJ * ioo)
{
	// io was collided from back ?
	EERIEPOLY ep;
	ep.type=0;

	if (	(io )
		&&	(ioo)
		&&	(io->ioflags & IO_NPC)
		&&	(ioo->ioflags & IO_NPC)	)
	{

	
	ep.v[0].sx=io->pos.x;
	ep.v[0].sz=io->pos.z;
	float ft=DEG2RAD(135.f+90.f);
		ep.v[1].sx = EEsin(ft) * 180.f; 
		ep.v[1].sz = -EEcos(ft) * 180.f; 
	ft=DEG2RAD(225.f+90.f);
		ep.v[2].sx = EEsin(ft) * 180.f; 
		ep.v[2].sz = -EEcos(ft) * 180.f; 
	ft=DEG2RAD(270.f-io->angle.b);
	float ec=EEcos(ft);
	float es=EEsin(ft);
	EE_RotateY( &ep.v[1]  , &ep.tv[1]   , ec , es );
	EE_RotateY( &ep.v[2]  , &ep.tv[2]   , ec , es );
	ep.v[1].sx=ep.tv[1].sx+ep.v[0].sx;
	ep.v[1].sz=ep.tv[1].sz+ep.v[0].sz;
	ep.v[2].sx=ep.tv[2].sx+ep.v[0].sx;
	ep.v[2].sz=ep.tv[2].sz+ep.v[0].sz;

	// To keep if we need some visual debug
	if (PointIn2DPolyXZ(&ep,ioo->pos.x,ioo->pos.z))
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Returns 0 if nothing in cyl
// Else returns Y Offset to put cylinder in a proper place
float CheckAnythingInCylinder(EERIE_CYLINDER * cyl,INTERACTIVE_OBJ * ioo,long flags)
{	
	NPC_IN_CYLINDER=0;
	long rad;
	F2L((cyl->radius + 100)*ACTIVEBKG->Xmul, &rad); 
	long px,pz;
	F2L(cyl->origin.x*ACTIVEBKG->Xmul,&px);

	if (px>ACTIVEBKG->Xsize-2-rad)  
		return 0.f;

	if (px< 1+rad)  
		return 0.f;
	
	F2L(cyl->origin.z*ACTIVEBKG->Zmul,&pz);	

	if (pz>ACTIVEBKG->Zsize-2-rad)  
		return 0.f;

	if (pz< 1+rad)  
		return 0.f;

	float anything = 999999.f; 
	
	EERIEPOLY * ep;
	FAST_BKG_DATA * feg;
	
	int itest = 0;
	
	if (TSU_TEST_COLLISIONS)
	if (	(iXBackup >= px-rad)
		&&	(iXBackup <= px-rad)
		&&	(iYBackup >= pz-rad)
		&&	(iYBackup <= pz-rad)	
		)
	{
		float minanything = __min(anything,IsPolyInCylinder(pEPBackup,cyl,flags));

		if (anything != minanything)
		{
			anything = minanything;
			itest = 1;
		}

		if (POLYIN && (pEPBackup->type & POLY_CLIMB))
		{
			COLLIDED_CLIMB_POLY=1;
		}
	}

/* TO KEEP...
	EERIE_BKG_INFO * eg=&ACTIVEBKG->Backg[px+pz*ACTIVEBKG->Xsize];
	if (	(cyl->origin.y+cyl->height < eg->tile_miny)
			&& (cyl->origin.y > eg->tile_miny)
		//||	(cyl->origin.y > eg->tile_maxy)	
		)
	{
		return 999999.f;
	}
	*/
	for (long j=pz-rad;j<=pz+rad;j++)
	for (long i=px-rad;i<=px+rad;i++) 
	{
		float nearx,nearz;
		float nearest=99999999.f;

		for (long num=0;num<4;num++)
		{

				nearx = ARX_CLEAN_WARN_CAST_FLOAT(i * 100);  
				nearz = ARX_CLEAN_WARN_CAST_FLOAT(j * 100);  

			if ((num==1) || (num==2))
					nearx += 100;

			if ((num==2) || (num==3))
					nearz += 100; 

			float dd=Distance2D(nearx,nearz,cyl->origin.x,cyl->origin.z);

			if (dd<nearest)
			{
				nearest=dd;
			}
		}

		if (nearest>__max(82.f,cyl->radius)) continue;


		feg=&ACTIVEBKG->fastdata[i][j];
		// tsu
		bool bOk = true;
		
		if (TSU_TEST_COLLISIONS)
		{
			EERIE_BKG_INFO *eg=&ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];

			if (!(eg->tile_miny < anything))
				bOk = false;
		}

		if (bOk)
		for (long k=0;k<feg->nbpoly;k++)
		{
			ep=&feg->polydata[k];	

			if (ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL) ) continue;

			if (ep->min.y<anything)
			{

				anything= __min(anything,IsPolyInCylinder(ep,cyl,flags));

				if (POLYIN) 
				{
					if (ep->type & POLY_CLIMB)
						COLLIDED_CLIMB_POLY=1;
				}
			}
		}
	}	

	float tempo;
	
	ep=CheckInPolyPrecis(cyl->origin.x,cyl->origin.y+cyl->height,cyl->origin.z,&tempo);
	
	if (ep) 
		{
			anything=__min(anything,tempo);
	}

	long dealt = 0;

	if (!(flags & CFLAG_NO_INTERCOL))
	{
		INTERACTIVE_OBJ * io;
		long FULL_TEST=0;
		long AMOUNT=TREATZONE_CUR;

		if (	ioo
			&&	(ioo->ioflags & IO_NPC) 
			&&	(ioo->_npcdata->pathfind.flags & PATHFIND_ALWAYS))
		{
			FULL_TEST=1;
			AMOUNT=inter.nbmax;
		}

		for (long i=0;i<AMOUNT;i++) 
		{
			if (FULL_TEST)
			{
				io=inter.iobj[i];			
			}
			else
			{				
				io=treatio[i].io;				
			}

			if (	!io
				||	(io==ioo)
				||	(!io->obj)
				||	(	(io->show!=SHOW_FLAG_IN_SCENE)
					||	((io->ioflags & IO_NO_COLLISIONS)  && !(flags & CFLAG_COLLIDE_NOCOL))
					) 
				||	(EEDistance3D(&io->pos,&cyl->origin)>1000.f)) continue;
	
			{
				EERIE_CYLINDER * io_cyl=&io->physics.cyl;
				GetIOCyl(io,io_cyl);
				dealt=0;

				if (	(io->GameFlags & GFLAG_PLATFORM)
					||	((flags & CFLAG_COLLIDE_NOCOL) && (io->ioflags & IO_NPC) &&  (io->ioflags & IO_NO_COLLISIONS))
					)
				{
					float miny,maxy;
					miny=io->bbox3D.min.y;
					maxy=io->bbox3D.max.y;

					if (Distance2D(io->pos.x,io->pos.z,cyl->origin.x,cyl->origin.z)<440.f+cyl->radius)
					if (In3DBBoxTolerance(&cyl->origin,&io->bbox3D, cyl->radius+80))
					{
						if (io->ioflags & IO_FIELD)
						{
							if (In3DBBoxTolerance(&cyl->origin,&io->bbox3D, cyl->radius+10))
								anything=-99999.f;
						}
							else 
						{
						for (long ii=0;ii<io->obj->nbvertex;ii++)
						{
							long res=PointInUnderCylinder(cyl,&io->obj->vertexlist3[ii].v);

							if (res>0)
							{
								if (res==2) ON_PLATFORM=1;

										anything = __min(anything, io->obj->vertexlist3[ii].v.y - 10.f); 
							}			
						}

						for (int ii=0;ii<io->obj->nbfaces;ii++)
						{
							EERIE_3D c;
							Vector_Init(&c);
							float height=io->obj->vertexlist3[io->obj->facelist[ii].vid[0]].v.y;

							for (long kk=0;kk<3;kk++)
							{
								c.x+=io->obj->vertexlist3[io->obj->facelist[ii].vid[kk]].v.x;
								c.y+=io->obj->vertexlist3[io->obj->facelist[ii].vid[kk]].v.y;
								height=__min(height,io->obj->vertexlist3[io->obj->facelist[ii].vid[kk]].v.y);
								c.z+=io->obj->vertexlist3[io->obj->facelist[ii].vid[kk]].v.z;
							}

							c.x*=DIV3;
							c.z*=DIV3;
									c.y = io->bbox3D.min.y; 
							long res=PointInUnderCylinder(cyl,&c);

							if (res>0)
							{
								if (res==2) ON_PLATFORM=1;
										anything = __min(anything, height); 
									}
									}
								}
							}
				}
				else if (	(io->ioflags & IO_NPC)
						&&	(!(flags & CFLAG_NO_NPC_COLLIDE)) // MUST be checked here only (not before...)
						&&	(!(ioo && (ioo->ioflags & IO_NO_COLLISIONS)))	
						&&	(io->_npcdata->life>0.f)
						)
				{
					
					if (CylinderInCylinder(cyl,io_cyl))
					{
 						NPC_IN_CYLINDER=1;
						anything = __min(anything, io_cyl->origin.y + io_cyl->height); 

						if (!(flags & CFLAG_JUST_TEST) && ioo)
						{							
							if (ARXTime > io->collide_door_time + 500)
							{
								EVENT_SENDER=ioo;									
								io->collide_door_time = ARXTimeUL(); 	

								if (CollidedFromBack(io,ioo))
									SendIOScriptEvent(io,SM_COLLIDE_NPC,"BACK",NULL);
								else
									SendIOScriptEvent(io,SM_COLLIDE_NPC,"",NULL);

								EVENT_SENDER=io;
								io->collide_door_time = ARXTimeUL(); 

								if (CollidedFromBack(ioo,io))
									SendIOScriptEvent(ioo,SM_COLLIDE_NPC,"BACK",NULL);
								else
									SendIOScriptEvent(ioo,SM_COLLIDE_NPC,"",NULL);
							}

							if ((!dealt) && ((ioo->damager_damages>0) || (io->damager_damages>0)))
							{
								dealt=1;

								if (ioo->damager_damages>0)
									ARX_DAMAGES_DealDamages(i,ioo->damager_damages,GetInterNum(ioo),ioo->damager_type,&io->pos);

								if (io->damager_damages>0)
									ARX_DAMAGES_DealDamages(GetInterNum(ioo),io->damager_damages,GetInterNum(io),io->damager_type,&ioo->pos);
							}						

							PUSHABLE_NPC=io;

							if (io->targetinfo==i)
							{
								if (io->_npcdata->pathfind.listnb>0)
								{
									io->_npcdata->pathfind.listpos=0;
									io->_npcdata->pathfind.listnb=-1;

									if (io->_npcdata->pathfind.list) free(io->_npcdata->pathfind.list);

									io->_npcdata->pathfind.list=NULL;
									SendIOScriptEvent(io,0,"","PATHFINDER_END");
								}							

								if (!io->_npcdata->reachedtarget)
								{							
									EVENT_SENDER=ioo;
									SendIOScriptEvent(io,SM_REACHEDTARGET,"");
									io->_npcdata->reachedtarget=1;			
								}
							}
						}
					}
				}
				else if (io->ioflags & IO_FIX)
				{
					long nbv;
					long idx;
					EERIE_VERTEX * vlist;
					EERIE_SPHERE sp;

					float miny,maxy;
					miny=io->bbox3D.min.y;
					maxy=io->bbox3D.max.y;

					if (maxy<= cyl->origin.y+cyl->height) goto suivant;

					if (miny>= cyl->origin.y) goto suivant;	

					if (In3DBBoxTolerance(&cyl->origin,&io->bbox3D,cyl->radius+30.f))
					{
						vlist=io->obj->vertexlist3;
						nbv=io->obj->nbvertex;

						if (io->obj->nbgroups>10)
						{
							for (long ii=0;ii<io->obj->nbgroups;ii++)
							{
								idx=io->obj->grouplist[ii].origin;
								sp.origin.x=vlist[idx].v.x;
								sp.origin.y=vlist[idx].v.y;
								sp.origin.z=vlist[idx].v.z;

								if (ioo==inter.iobj[0])
								{
									sp.radius = 22.f; 
								}
								else if (ioo->ioflags & IO_NPC)
									sp.radius = 26.f; 
								else
									sp.radius = 22.f; 

								if (SphereInCylinder(cyl,&sp))
								{
									if (!(flags & CFLAG_JUST_TEST) && ioo)
									{
										if (io->GameFlags&GFLAG_DOOR)
										{
											
											if (ARXTime>io->collide_door_time+500)
											{
												EVENT_SENDER=ioo;									
												io->collide_door_time = ARXTimeUL(); 	
												SendIOScriptEvent(io,SM_COLLIDE_DOOR,"",NULL);
												EVENT_SENDER=io;
												io->collide_door_time = ARXTimeUL(); 	
												SendIOScriptEvent(ioo,SM_COLLIDE_DOOR,"",NULL);
											}
										}

										if (io->ioflags & IO_FIELD)
										{
											EVENT_SENDER=NULL;									
											io->collide_door_time = ARXTimeUL(); 	
											SendIOScriptEvent(ioo,SM_COLLIDE_FIELD,"",NULL);
										}

										if ((!dealt) && ((ioo->damager_damages>0) || (io->damager_damages>0)))
										{
											dealt=1;

											if (ioo->damager_damages>0)
												ARX_DAMAGES_DealDamages(i,ioo->damager_damages,GetInterNum(ioo),ioo->damager_type,&io->pos);

											if (io->damager_damages>0)
												ARX_DAMAGES_DealDamages(GetInterNum(ioo),io->damager_damages,GetInterNum(io),io->damager_type,&ioo->pos);
										}
									}

									anything=__min(anything,__min(sp.origin.y-sp.radius , io->bbox3D.min.y));
								}
							}
						}
						else
						{
							long step;
							
							if (ioo==inter.iobj[0])
								sp.radius = 23.f; 
							else if (ioo && !(ioo->ioflags & IO_NPC))
								sp.radius = 32.f;
							else
								sp.radius = 25.f;

							if (nbv<300)
							{
								step=1;
							}
							else if (nbv<600) step=2;
							else if (nbv<1200) step=4;
							else step=6;

							for (long ii=1;ii<nbv;ii+=step)
							{
								if (ii!=io->obj->origin)
								{
									sp.origin.x=vlist[ii].v.x;
									sp.origin.y=vlist[ii].v.y;
									sp.origin.z=vlist[ii].v.z;
									
									if (SphereInCylinder(cyl,&sp))
									{
										if (!(flags & CFLAG_JUST_TEST) && ioo)
										{
											if (io->GameFlags&GFLAG_DOOR)
											{
												if (ARXTime>io->collide_door_time+500)
												{
													EVENT_SENDER=ioo;									
													io->collide_door_time = ARXTimeUL(); 	
													SendIOScriptEvent(io,SM_COLLIDE_DOOR,"",NULL);
													EVENT_SENDER=io;
													io->collide_door_time = ARXTimeUL(); 	
													SendIOScriptEvent(ioo,SM_COLLIDE_DOOR,"",NULL);
												}
											}

										if (io->ioflags & IO_FIELD)
										{
											EVENT_SENDER=NULL;									
												io->collide_door_time = ARXTimeUL(); 	
											SendIOScriptEvent(ioo,SM_COLLIDE_FIELD,"",NULL);
										}
					
											if ((!dealt) && ioo && ((ioo->damager_damages > 0) || (io->damager_damages > 0)))
							{
								dealt=1;
											
											if (ioo->damager_damages>0)
												ARX_DAMAGES_DealDamages(i,ioo->damager_damages,GetInterNum(ioo),ioo->damager_type,&io->pos);
									
												if (io->damager_damages>0)
													ARX_DAMAGES_DealDamages(GetInterNum(ioo),io->damager_damages,GetInterNum(io),io->damager_type,&ioo->pos);
											}
										}
										anything=__min(anything,__min(sp.origin.y-sp.radius,io->bbox3D.min.y));
									}
								}
							}
						}
					}
				} 
			}
		suivant:
			;
		}
	}
	
	if (anything == 999999.f) return 0.f; 

	anything=anything-cyl->origin.y;

	return anything;	
}
//-----------------------------------------------------------------------------
__forceinline BOOL InExceptionList(long val)
{
	for (long i=0;i<EXCEPTIONS_LIST_Pos;i++)
	{
		if (val==EXCEPTIONS_LIST[i]) return TRUE;
	}

	return FALSE;
}

//-----------------------------------------------------------------------------
BOOL CheckEverythingInSphere(EERIE_SPHERE * sphere,long source,long targ) //except source...
{
	BOOL vreturn=FALSE;
	MAX_IN_SPHERE_Pos=0;
	
	EERIE_VERTEX * vlist;	
	INTERACTIVE_OBJ * io;
	long ret_idx=-1;
	
	float sr30=sphere->radius+20.f;
	float sr40=sphere->radius+30.f;
	float sr180=sphere->radius+500.f;

	for (long i=0;i<TREATZONE_CUR;i++) 
	{
		if (targ>-1) 
		{
			i=TREATZONE_CUR;
			io=inter.iobj[targ];

			if (   (!io)
				|| (InExceptionList(targ))
				|| (targ==source)
				|| (io->show!=SHOW_FLAG_IN_SCENE) 
				|| !(io->GameFlags & GFLAG_ISINTREATZONE)
				|| !(io->obj)
			)
			return FALSE;

			ret_idx=targ;
		}
		else 
		{
			if ( (treatio[i].show!=1) ||
			 (treatio[i].io==NULL) ||
			 (treatio[i].num==source) ||
			 (InExceptionList(treatio[i].num)) ) continue;

			io=treatio[i].io;
			ret_idx=treatio[i].num;
		}

		if (!(io->ioflags & IO_NPC) && (io->ioflags & IO_NO_COLLISIONS)) continue;

		if (!io->obj) continue;


				if (io->GameFlags & GFLAG_PLATFORM)					
				{
					float miny,maxy;
					miny=io->bbox3D.min.y;
					maxy=io->bbox3D.max.y;

					if (	(maxy<= sphere->origin.y+sphere->radius) 
						||	(miny>= sphere->origin.y) )
					if (In3DBBoxTolerance(&sphere->origin,&io->bbox3D,sphere->radius))
					{
						if (Distance2D(io->pos.x,io->pos.z,sphere->origin.x,sphere->origin.z)<440.f+sphere->radius)
						{
							EERIEPOLY ep;
							ep.type=0;

							for (long ii=0;ii<io->obj->nbfaces;ii++)
							{
								float cx=0;
								float cz=0;

								for (long kk=0;kk<3;kk++)
								{
									cx+=ep.v[kk].sx=io->obj->vertexlist3[io->obj->facelist[ii].vid[kk]].v.x;
										ep.v[kk].sy=io->obj->vertexlist3[io->obj->facelist[ii].vid[kk]].v.y;
									cz+=ep.v[kk].sz=io->obj->vertexlist3[io->obj->facelist[ii].vid[kk]].v.z;
								}

								cx*=DIV3;
								cz*=DIV3;

								for (int kk=0;kk<3;kk++)
								{
									ep.v[kk].sx=(ep.v[kk].sx-cx)*3.5f+cx;
									ep.v[kk].sz=(ep.v[kk].sz-cz)*3.5f+cz;
								}

								if (PointIn2DPolyXZ(&ep, sphere->origin.x, sphere->origin.z)) 
								{		
									EVERYTHING_IN_SPHERE[MAX_IN_SPHERE_Pos]=(short)ret_idx;
									MAX_IN_SPHERE_Pos++;

									if (MAX_IN_SPHERE_Pos>=MAX_IN_SPHERE) MAX_IN_SPHERE_Pos--;

									vreturn=TRUE;
									goto suivant;									
								}
							}
						}
					}
				}

			if (EEDistance3D(&io->pos,&sphere->origin)< sr180)
			{
			
				long amount=1;
				vlist=io->obj->vertexlist3;

				if (io->obj->nbgroups>4)
				{
					for (long ii=0;ii<io->obj->nbgroups;ii++)
					{								
						if (EEDistance3D(&vlist[io->obj->grouplist[ii].origin].v,&sphere->origin)< sr40)
						{
							EVERYTHING_IN_SPHERE[MAX_IN_SPHERE_Pos]=(short)ret_idx;
							MAX_IN_SPHERE_Pos++;

							if (MAX_IN_SPHERE_Pos>=MAX_IN_SPHERE) MAX_IN_SPHERE_Pos--;

							vreturn=TRUE;
							goto suivant;
						}
					}

					amount=2;
				}

					for (long ii=0;ii<io->obj->nbfaces;ii+=amount)
					{
						EERIE_FACE * ef=&io->obj->facelist[ii];

						if (ef->facetype & POLY_HIDE) continue;

						EERIE_3D fcenter;
						fcenter.x=(vlist[ef->vid[0]].v.x+vlist[ef->vid[1]].v.x+vlist[ef->vid[2]].v.x)*DIV3;
						fcenter.y=(vlist[ef->vid[0]].v.y+vlist[ef->vid[1]].v.y+vlist[ef->vid[2]].v.y)*DIV3;
						fcenter.z=(vlist[ef->vid[0]].v.z+vlist[ef->vid[1]].v.z+vlist[ef->vid[2]].v.z)*DIV3;

						if (	( EEDistance3D(&fcenter,&sphere->origin)< sr30) ||	( EEDistance3D(&vlist[ef->vid[0]].v,&sphere->origin)< sr30)
							|| (EEDistance3D(&vlist[ef->vid[1]].v,&sphere->origin)< sr30) || (EEDistance3D(&vlist[ef->vid[2]].v,&sphere->origin)< sr30))
						{
							EVERYTHING_IN_SPHERE[MAX_IN_SPHERE_Pos]=(short)ret_idx;
							MAX_IN_SPHERE_Pos++;

							if (MAX_IN_SPHERE_Pos>=MAX_IN_SPHERE) MAX_IN_SPHERE_Pos--;

							vreturn=TRUE;		
							goto suivant;
						}
					}
				}

	suivant:
	  ;
		
	}	

	return vreturn;	
}

//-----------------------------------------------------------------------------
EERIEPOLY * CheckBackgroundInSphere(EERIE_SPHERE * sphere) //except source...
{
	long rad;
	F2L(sphere->radius*ACTIVEBKG->Xmul,&rad);
	rad+=2;
	long px,pz;
	F2L(sphere->origin.x*ACTIVEBKG->Xmul,&px);	
	F2L(sphere->origin.z*ACTIVEBKG->Zmul,&pz);

	EERIEPOLY * ep;
	FAST_BKG_DATA * feg;
	long spx=__max(px-rad,0);
	long epx=__min(px+rad,ACTIVEBKG->Xsize-1);
	long spz=__max(pz-rad,0);
	long epz=__min(pz+rad,ACTIVEBKG->Zsize-1);

	for (long j=spz;j<=epz;j++)
	for (long i=spx;i<=epx;i++) 
	{
		feg=&ACTIVEBKG->fastdata[i][j];

		for (long k=0;k<feg->nbpoly;k++)
		{
			ep=&feg->polydata[k];	

			if (ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL)) continue;

			if (IsPolyInSphere(ep,sphere)) 
			{
				return ep;					
			}			
		}
	}	
	
	return NULL;	
}

//-----------------------------------------------------------------------------

BOOL CheckAnythingInSphere(EERIE_SPHERE * sphere,long source,long flags,long * num) //except source...
{
	if (num) *num=-1;

	long rad;
	F2L(sphere->radius*ACTIVEBKG->Xmul,&rad);
	rad+=2;
	long px,pz;
	F2L(sphere->origin.x*ACTIVEBKG->Xmul,&px);	
	F2L(sphere->origin.z*ACTIVEBKG->Zmul,&pz);

	EERIEPOLY * ep;
	FAST_BKG_DATA * feg;

	if (!(flags & CAS_NO_BACKGROUND_COL))
	{
		long spz=__max(pz-rad,0);
		long epz=__min(pz+rad,ACTIVEBKG->Zsize-1); 
		long spx=__max(px-rad,0);
		long epx=__min(px+rad,ACTIVEBKG->Xsize-1); 

		for (long j=spz;j<=epz;j++)
		for (long i=spx;i<=epx;i++) 
		{
			feg=&ACTIVEBKG->fastdata[i][j];

			for (long k=0;k<feg->nbpoly;k++)
			{
				ep=&feg->polydata[k];	

				if (ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL)) continue;

				if (IsPolyInSphere(ep,sphere)) 
					return TRUE;					
			}
		}	
	}

	if (flags & CAS_NO_NPC_COL) return FALSE;

	long validsource=0;

	if (flags & CAS_NO_SAME_GROUP) validsource=ValidIONum(source);

	EERIE_VERTEX * vlist;	
	INTERACTIVE_OBJ * io;
	float sr30=sphere->radius+20.f;
	float sr40=sphere->radius+30.f;
	float sr180=sphere->radius+500.f;

	for (long i=0;i<TREATZONE_CUR;i++) 
	{
		
		if ( (treatio[i].show!=1) ||
			 (treatio[i].io==NULL) ||
			 (treatio[i].num==source)
			  ) continue;

		io=treatio[i].io;

		if (!io->obj) continue;

		if (!(io->ioflags & IO_NPC) && (io->ioflags & IO_NO_COLLISIONS)) continue;

		if ((flags & CAS_NO_DEAD_COL) && (io->ioflags & IO_NPC) && (IsDeadNPC(io))) continue;

		if ((io->ioflags & IO_FIX) && (flags & CAS_NO_FIX_COL)) continue;

		if ((io->ioflags & IO_ITEM) && (flags & CAS_NO_ITEM_COL)) continue;

		if ((treatio[i].num!=0) && (source!=0) 
				&& validsource && (HaveCommonGroup(io,inter.iobj[source])))
				continue;

			if (io->GameFlags & GFLAG_PLATFORM)					
				{
					float miny,maxy;
					miny=io->bbox3D.min.y;
					maxy=io->bbox3D.max.y;

					if (	(maxy> sphere->origin.y-sphere->radius) 
						||	(miny< sphere->origin.y+sphere->radius) )
					if (In3DBBoxTolerance(&sphere->origin,&io->bbox3D,sphere->radius))
					{
						if (Distance2D(io->pos.x,io->pos.z,sphere->origin.x,sphere->origin.z)<440.f+sphere->radius)
						{
							EERIEPOLY ep;
							ep.type=0;

							for (long ii=0;ii<io->obj->nbfaces;ii++)
							{
								float cx=0;
								float cz=0;

								for (long kk=0;kk<3;kk++)
								{
									cx+=ep.v[kk].sx=io->obj->vertexlist3[io->obj->facelist[ii].vid[kk]].v.x;
										ep.v[kk].sy=io->obj->vertexlist3[io->obj->facelist[ii].vid[kk]].v.y;
									cz+=ep.v[kk].sz=io->obj->vertexlist3[io->obj->facelist[ii].vid[kk]].v.z;
								}

								cx*=DIV3;
								cz*=DIV3;

								for (int kk=0;kk<3;kk++)
								{
									ep.v[kk].sx=(ep.v[kk].sx-cx)*3.5f+cx;
									ep.v[kk].sz=(ep.v[kk].sz-cz)*3.5f+cz;
								}

								if (PointIn2DPolyXZ(&ep, sphere->origin.x, sphere->origin.z)) 
								{		
									if (num) *num=treatio[i].num;

									return TRUE;
								}
							}
						}
					}
				}

			if (EEDistance3D(&io->pos,&sphere->origin)< sr180 )
			{
				long amount=1;
				vlist=io->obj->vertexlist3;

				if (io->obj->nbgroups>4)
				{
					for (long ii=0;ii<io->obj->nbgroups;ii++)
					{								
						if (EEDistance3D(&vlist[io->obj->grouplist[ii].origin].v,&sphere->origin)< sr40)
						{
							if (num) *num=treatio[i].num;

							return TRUE;
						}
					}

					amount=2;
				}

				for (long ii=0;ii<io->obj->nbfaces;ii+=amount)
				{

					if (io->obj->facelist[ii].facetype & POLY_HIDE) continue;

					if (( EEDistance3D(&vlist[io->obj->facelist[ii].vid[0]].v,&sphere->origin)< sr30)
						|| (EEDistance3D(&vlist[io->obj->facelist[ii].vid[1]].v,&sphere->origin)< sr30))
					{
						if (num) *num=treatio[i].num;

						return TRUE;
					}
				}
			}
		
		}

	return FALSE;	
}

//-----------------------------------------------------------------------------
BOOL CheckIOInSphere(EERIE_SPHERE * sphere,long target,long flags) 
{
	if (!ValidIONum(target)) return FALSE;

	EERIE_VERTEX * vlist;		
	INTERACTIVE_OBJ * io=inter.iobj[target];
	float sr30 = sphere->radius + 22.f;
	float sr40 = sphere->radius + 27.f; 
	float sr180=sphere->radius+500.f;

	if (  			
			 ((flags & IIS_NO_NOCOL) ||  (!(flags & IIS_NO_NOCOL) && !(io->ioflags & IO_NO_COLLISIONS)))
			&& (io->show==SHOW_FLAG_IN_SCENE) 
	    && (io->GameFlags & GFLAG_ISINTREATZONE)
			&& (io->obj)
			)
		{
			if (EEDistance3D(&io->pos,&sphere->origin)< sr180)
			{
				vlist=io->obj->vertexlist3;

				if (io->obj->nbgroups>10)
				{
					long count=0;
					long ii=io->obj->nbgroups-1;

					while (ii)
					{		
						if (EEDistance3D(&vlist[io->obj->grouplist[ii].origin].v,&sphere->origin)< sr40)
						{
							count++;

							if (count>3) return TRUE;
						}

						ii--;
					}
				}

					long count=0;
					long step;

					if (io->obj->nbvertex<150) step=1;
					else if (io->obj->nbvertex<300) step=2;
					else if (io->obj->nbvertex<600) step=4;
					else if (io->obj->nbvertex<1200) step=6;
					else step=7;

					for (long ii=0;ii<io->obj->nbvertex;ii+=step)
					{
						if (EEDistance3D(&vlist[ii].v,&sphere->origin)< sr30) 
						{
							count++;

							if (count>6) return TRUE;
						}

						if (io->obj->nbvertex<120)
						{
							for (long kk=0;kk<io->obj->nbvertex;kk+=1)
							{
								if (kk!=ii)
								{
									for (float nn=0.2f;nn<1.f;nn+=0.2f)
									{
									EERIE_3D posi;
									posi.x=(vlist[ii].v.x*nn+vlist[kk].v.x*(1.f-nn));
									posi.y=(vlist[ii].v.y*nn+vlist[kk].v.y*(1.f-nn));
									posi.z=(vlist[ii].v.z*nn+vlist[kk].v.z*(1.f-nn));
									float dist=EEDistance3D(&sphere->origin,&posi);

									if (dist<=sr30+20)
									{
										count++;

										if (count>3)
									{
										if (io->ioflags & IO_FIX)
												return TRUE;

											if (count>6) 
												return TRUE;
										}										
									}
									}
								}
							}
						}
					}

					if ((count>3) && (io->ioflags & IO_FIX))	
						return TRUE;
			
				}
			}
	
	return FALSE;	
}


float MAX_ALLOWED_PER_SECOND=12.f;
//-----------------------------------------------------------------------------
// Checks if a position is valid, Modify it for height if necessary
// Returns TRUE or FALSE

BOOL AttemptValidCylinderPos(EERIE_CYLINDER * cyl,INTERACTIVE_OBJ * io,long flags)
{
	PUSHABLE_NPC=NULL;
	float anything = CheckAnythingInCylinder(cyl, io, flags); 

	if ((flags & CFLAG_LEVITATE) && (anything==0.f)) return TRUE;

	if (anything>=0.f) // Falling Cylinder but valid pos !
	{
		if (flags & CFLAG_RETURN_HEIGHT)
			cyl->origin.y+=anything;

		return TRUE;
	}
	
	EERIE_CYLINDER tmp;

	if (!(flags & CFLAG_ANCHOR_GENERATION))
	{
		
		memcpy(&tmp,cyl,sizeof(EERIE_CYLINDER));

		while (anything<0.f) 
		{
			tmp.origin.y+=anything;
			anything=CheckAnythingInCylinder(&tmp,io,flags);					
		}

		anything=tmp.origin.y-cyl->origin.y;
	}

	if (MOVING_CYLINDER)
	{
		if (flags & CFLAG_NPC)
		{		
			float tolerate;
			
			if ((flags & CFLAG_PLAYER)
				&&	(player.jumpphase)	)
			{
				tolerate=0;
			}
			else if ((io) && (io->ioflags & IO_NPC) && (io->_npcdata->pathfind.listnb > 0) && (io->_npcdata->pathfind.listpos < io->_npcdata->pathfind.listnb))
			{
				tolerate=-65-io->_npcdata->moveproblem;
			}
			else 
			{
				if ((io) &&
				        (io->_npcdata))
				{
					tolerate = -55 - io->_npcdata->moveproblem; 
				}
				else
				{
					tolerate=0.f;
				}
			}

			if (NPC_IN_CYLINDER)
			{
				tolerate=cyl->height*DIV2;
			}

			if (anything<tolerate) return FALSE;
		}

		if (io && (flags & CFLAG_PLAYER) && (anything<0.f) && (flags & CFLAG_JUST_TEST))
		{
			EERIE_CYLINDER tmpp;
			memcpy(&tmpp,cyl,sizeof(EERIE_CYLINDER));
			tmpp.radius*=0.7f;
			float tmp=CheckAnythingInCylinder(&tmpp,io,flags | CFLAG_JUST_TEST);

			if ((tmp > 50.f))
			{		
				tmpp.radius=cyl->radius*1.4f;
					tmpp.origin.y-=30.f;
					float tmp=CheckAnythingInCylinder(&tmpp,io,flags | CFLAG_JUST_TEST);

					if (tmp<0)
				return FALSE;
			}
		}

		if (io && (!(flags & CFLAG_JUST_TEST)))
		{
			if ((flags & CFLAG_PLAYER) && (anything<0.f))
			{
				if (player.jumpphase)
				{
					io->_npcdata->climb_count=MAX_ALLOWED_PER_SECOND;
					return FALSE;
				}				

				float dist;
				dist=__max(TRUEVector_Magnitude(&vector2D),1.f);
				float pente;		
				pente = EEfabs(anything) / dist * DIV2; 
				io->_npcdata->climb_count+=pente;

				if (io->_npcdata->climb_count>MAX_ALLOWED_PER_SECOND)
				{
					io->_npcdata->climb_count=MAX_ALLOWED_PER_SECOND;
				}

				if (anything < -55) 
				{
					io->_npcdata->climb_count=MAX_ALLOWED_PER_SECOND;
					return FALSE;
				}

				EERIE_CYLINDER tmpp;
				memcpy(&tmpp,cyl,sizeof(EERIE_CYLINDER));
				tmpp.radius *= 0.65f; 
				float tmp=CheckAnythingInCylinder(&tmpp,io,flags | CFLAG_JUST_TEST);

				if (tmp > 50.f)
				{				
					tmpp.radius=cyl->radius*1.45f;
					tmpp.origin.y-=30.f;
					float tmp=CheckAnythingInCylinder(&tmpp,io,flags | CFLAG_JUST_TEST);

					if (tmp<0)
						return FALSE;
				}	
			}			
		}
	}
	else if (anything<-45) return FALSE;

	if ((flags & CFLAG_SPECIAL) && (anything<-40)) 
	{
		if (flags & CFLAG_RETURN_HEIGHT)
			cyl->origin.y+=anything;

		return FALSE;
	}

	memcpy(&tmp,cyl,sizeof(EERIE_CYLINDER));
	tmp.origin.y+=anything;
	anything = CheckAnythingInCylinder(&tmp, io, flags);

	if (anything<0.f) 
	{
		if (flags & CFLAG_RETURN_HEIGHT)
		{
			while (anything<0.f) 
			{
				tmp.origin.y+=anything;
				anything=CheckAnythingInCylinder(&tmp,io,flags);
			}

			cyl->origin.y = tmp.origin.y; 
		}

		return FALSE;
	}

	cyl->origin.y=tmp.origin.y;
	return TRUE;
}

//----------------------------------------------------------------------------------------------
//flags & 1 levitate
//flags & 2 no inter col
//flags & 4 special
//flags & 8	easier sliding.
//flags & 16 climbing !!!
//flags & 32 Just Test !!!
//flags & 64 NPC mode
//----------------------------------------------------------------------------------------------
BOOL ARX_COLLISION_Move_Cylinder(IO_PHYSICS * ip,INTERACTIVE_OBJ * io,float MOVE_CYLINDER_STEP,long flags)
{
//	HERMESPerf script(HPERF_PHYSICS);
//	+5 on 15
//	
//	memcpy(&ip->cyl.origin,&ip->targetpos,sizeof(EERIE_3D));
//	return TRUE;

	ON_PLATFORM=0;
	MOVING_CYLINDER=1;
	COLLIDED_CLIMB_POLY=0;
	DIRECT_PATH=TRUE;
	IO_PHYSICS test;

	if (ip==NULL) 
	{
		MOVING_CYLINDER=0;
		return FALSE;
	}

	float distance=TRUEEEDistance3D(&ip->startpos,&ip->targetpos);

	if (distance < 0.1f) 
	{
		MOVING_CYLINDER=0;

		if (distance==0.f)
			return TRUE;

		return FALSE;
	}

	float onedist=1.f/distance;
	EERIE_3D mvector;
	mvector.x=(ip->targetpos.x-ip->startpos.x)*onedist;
	mvector.y=(ip->targetpos.y-ip->startpos.y)*onedist;
	mvector.z=(ip->targetpos.z-ip->startpos.z)*onedist;
	long count=100;

	while ((distance>0.f) && (count--))
	{
		// First We compute current increment 
		float curmovedist=__min(distance,MOVE_CYLINDER_STEP);
		distance-=curmovedist;

		// Store our cylinder desc into a test struct
		memcpy(&test,ip,sizeof(IO_PHYSICS));

		// uses test struct to simulate movement.
		vector2D.x=mvector.x*curmovedist;
		vector2D.y=0.f;
		vector2D.z=mvector.z*curmovedist;

		test.cyl.origin.x += vector2D.x; 
		test.cyl.origin.y+=mvector.y*curmovedist;
		test.cyl.origin.z += vector2D.z; 

		
		PUSHABLE_NPC=NULL;

		if ((flags & CFLAG_CHECK_VALID_POS)
			&& (CylinderAboveInvalidZone(&test.cyl)))
				return FALSE;

		if (AttemptValidCylinderPos(&test.cyl,io,flags))
		{
			// Found without complication
			 memcpy(ip,&test,sizeof(IO_PHYSICS)); 
		}
		else 
		{
			//return FALSE;
			if ((mvector.x==0.f) && (mvector.z==0.f))
				return TRUE;
			
			//goto oki;
			if (flags & CFLAG_CLIMBING)
			{
				memcpy(&test.cyl, &ip->cyl, sizeof(EERIE_CYLINDER)); 
				test.cyl.origin.y+=mvector.y*curmovedist;

				if (AttemptValidCylinderPos(&test.cyl,io,flags))
				{
					memcpy(ip,&test,sizeof(IO_PHYSICS)); 
					goto oki;
				}
			}

			DIRECT_PATH=FALSE;
			// Must Attempt To Slide along collisions
			register EERIE_3D vecatt;
			EERIE_3D rpos;
			EERIE_3D lpos;
			long RFOUND=0;
			long LFOUND=0;
			long maxRANGLE=90;
			long maxLANGLE=270;
			float ANGLESTEPP;

			if (flags & CFLAG_EASY_SLIDING)  // player sliding in fact...
			{
				ANGLESTEPP=10.f;	
				maxRANGLE=70;
				maxLANGLE=290;
			}
			else ANGLESTEPP=30.f;

			register float rangle=ANGLESTEPP;
			register float langle=360.f-ANGLESTEPP;


			while (rangle<=maxRANGLE) //tries on the Right and Left sides
			{
				memcpy(&test.cyl, &ip->cyl, sizeof(EERIE_CYLINDER)); 
				float t=DEG2RAD(MAKEANGLE(rangle));
				_YRotatePoint(&mvector,&vecatt,EEcos(t),EEsin(t));
				test.cyl.origin.x+=vecatt.x*curmovedist;
				test.cyl.origin.y+=vecatt.y*curmovedist;
				test.cyl.origin.z+=vecatt.z*curmovedist;
				float cc=io->_npcdata->climb_count;

				if (AttemptValidCylinderPos(&test.cyl, io, flags)) 
				{
					Vector_Copy(&rpos,&test.cyl.origin);
					RFOUND=1;
				}
				else io->_npcdata->climb_count=cc;

				rangle+=ANGLESTEPP;

				memcpy(&test.cyl, &ip->cyl, sizeof(EERIE_CYLINDER)); 
				t=DEG2RAD(MAKEANGLE(langle));
				_YRotatePoint(&mvector,&vecatt,EEcos(t),EEsin(t));
				test.cyl.origin.x+=vecatt.x*curmovedist;
				test.cyl.origin.y+=vecatt.y*curmovedist;
				test.cyl.origin.z+=vecatt.z*curmovedist;
				cc=io->_npcdata->climb_count;

				if (AttemptValidCylinderPos(&test.cyl, io, flags))
				{
					Vector_Copy(&lpos,&test.cyl.origin);
					LFOUND=1;
				}
				else io->_npcdata->climb_count=cc;

				langle-=ANGLESTEPP;

				if ((RFOUND) || (LFOUND)) break;
			}
			
			if ((LFOUND) && (RFOUND))
			{
				langle=360.f-langle;

				if (langle<rangle) 
				{
					Vector_Copy(&ip->cyl.origin,&lpos);
					distance -= curmovedist;
				}
				else
				{
					Vector_Copy(&ip->cyl.origin,&rpos);
					distance -= curmovedist;
				}
			}
			else if (LFOUND)
			{
				Vector_Copy(&ip->cyl.origin,&lpos);
				distance -= curmovedist; 
			}
			else if (RFOUND)
			{
				Vector_Copy(&ip->cyl.origin,&rpos);
				distance -= curmovedist; 
			}
			else  //stopped
			{ 
				ip->velocity.x=ip->velocity.y=ip->velocity.z=0.f;
				MOVING_CYLINDER=0;
				return FALSE;
			}
		}

		if (flags & CFLAG_NO_HEIGHT_MOD)
		{
			if (EEfabs(ip->startpos.y - ip->cyl.origin.y)>30.f)
				return FALSE;
		}

	oki:
		;
	}

	MOVING_CYLINDER=0;
	return TRUE;
}

//-----------------------------------------------------------------------------
BOOL IO_Visible(EERIE_3D * orgn, EERIE_3D * dest,EERIEPOLY * epp,EERIE_3D * hit)
{
	

	float x,y,z; //current ray pos
	float dx,dy,dz; // ray incs
	float adx,ady,adz; // absolute ray incs
	float ix,iy,iz;
	long px,pz;
	EERIEPOLY * ep;

	FAST_BKG_DATA * feg;
	float pas=35.f;
 

	float found_dd=999999999999.f;
	EERIE_3D found_hit;
	EERIEPOLY * found_ep=NULL;
	float iter,t;

	found_hit.x = found_hit.y = found_hit.z = 0;

	x=orgn->x;
	y=orgn->y;
	z=orgn->z;
	float distance;
	float nearest=distance=EEDistance3D( orgn,dest );

	if (distance<pas) pas=distance*DIV2;

	dx=(dest->x-orgn->x);
	adx=EEfabs(dx);
	dy=(dest->y-orgn->y);
	ady=EEfabs(dy);
	dz=(dest->z-orgn->z);
	adz=EEfabs(dz);

	if ( (adx>=ady) && (adx>=adz)) 
	{
		if (adx != dx)
		{
			ix = -pas;
		}
		else
		{
			ix = pas;
		}

		iter=adx/pas;
		t=1.f/(iter);
		iy=dy*t;
		iz=dz*t;
	}
	else if ( (ady>=adx) && (ady>=adz)) 
	{
		if (ady != dy)
		{
			iy = -pas;
		}
		else
		{
			iy = pas;
		}

		iter=ady/pas;
		t=1.f/(iter);
		ix=dx*t;
		iz=dz*t;
	}
	else 
	{
		if (adz != dz)
		{
			iz = -pas;
		}
		else
		{
			iz = pas;
		}

		iter=adz/pas;
		t=1.f/(iter);
		ix=dx*t;
		iy=dy*t;
	}

	float dd;
	x-=ix;
	y-=iy;
	z-=iz;

	while (iter>0.f) 
	{
		iter-=1.f;
		x+=ix;
		y+=iy;
		z+=iz;

		EERIE_SPHERE sphere;
		sphere.origin.x=x;
		sphere.origin.y=y;
		sphere.origin.z=z;
		sphere.radius=65.f;

		for (long num=0;num<inter.nbmax;num++)
		{
			INTERACTIVE_OBJ * io=inter.iobj[num];

			if ((io) && (io->GameFlags & GFLAG_VIEW_BLOCKER))
			{
				if ( CheckIOInSphere(&sphere,num) )
				{
					dd=EEDistance3D(orgn,&sphere.origin);

					if (dd<nearest)
					{
						hit->x=x;
						hit->y=y;
						hit->z=z;
						return FALSE;
					}
				}
			}
		}

		px=(long)(x* ACTIVEBKG->Xmul);
		pz=(long)(z* ACTIVEBKG->Zmul);

		if (px>=ACTIVEBKG->Xsize)		goto fini;
		else if (px< 0)					goto fini;

		if (pz>= ACTIVEBKG->Zsize)		goto fini;
		else if (pz< 0)					goto fini;

			feg=&ACTIVEBKG->fastdata[px][pz];

			for (long k=0;k<feg->nbpolyin;k++)
			{
				ep=feg->polyin[k];	

				if (!(ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL) ) )
				if ((ep->min.y-pas<y) && (ep->max.y+pas>y))
				if ((ep->min.x-pas<x) && (ep->max.x+pas>x))
				if ((ep->min.z-pas<z) && (ep->max.z+pas>z))
				{
					if (RayCollidingPoly(orgn,dest,ep,hit)) 
					{
						dd=EEDistance3D(orgn,hit);

						if (dd<nearest)
						{
							nearest=dd;
							found_dd=dd;
							found_ep=ep;
							found_hit.x=hit->x;
							found_hit.y=hit->y;
							found_hit.z=hit->z;
						}
					}				
				}			
			}
	
		}	

fini:
	;
	
	if ( found_ep == NULL ) return TRUE;

	if ( found_ep == epp ) return TRUE;

	hit->x = found_hit.x;
	hit->y = found_hit.y;
	hit->z = found_hit.z;
	return FALSE;
}
void ANCHOR_BLOCK_Clear()
{
	EERIE_BACKGROUND * eb=ACTIVEBKG;

	if (eb)
	{
		for (long k=0;k<eb->nbanchors;k++)
		{
			_ANCHOR_DATA * ad=&eb->anchors[k];	
			ad->flags&=~ANCHOR_FLAG_BLOCKED;
		}
	}
}

void ANCHOR_BLOCK_By_IO(INTERACTIVE_OBJ * io,long status)
{
	EERIE_BACKGROUND * eb=ACTIVEBKG;

	for (long k=0;k<eb->nbanchors;k++)
	{
		_ANCHOR_DATA * ad=&eb->anchors[k];	

		if (EEDistance3D(&ad->pos,&io->pos)>600.f) continue;

		if (Distance2D(io->pos.x,io->pos.z,ad->pos.x,ad->pos.z)<440.f)
		{
			EERIEPOLY ep;
			ep.type=0;

			for (long ii=0;ii<io->obj->nbfaces;ii++)
			{
				float cx=0;
				float cz=0;

				for (long kk=0;kk<3;kk++)
				{
					cx+=ep.v[kk].sx=io->obj->vertexlist[io->obj->facelist[ii].vid[kk]].v.x+io->pos.x;
						ep.v[kk].sy=io->obj->vertexlist[io->obj->facelist[ii].vid[kk]].v.y+io->pos.y;
					cz+=ep.v[kk].sz=io->obj->vertexlist[io->obj->facelist[ii].vid[kk]].v.z+io->pos.z;
				}

				cx*=DIV3;
				cz*=DIV3;

				for (int kk=0;kk<3;kk++)
				{
					ep.v[kk].sx=(ep.v[kk].sx-cx)*3.5f+cx;
					ep.v[kk].sz=(ep.v[kk].sz-cz)*3.5f+cz;
				}

				if (PointIn2DPolyXZ(&ep, ad->pos.x, ad->pos.z)) 
				{
					if (status)
						ad->flags|=ANCHOR_FLAG_BLOCKED;
					else
						ad->flags&=~ANCHOR_FLAG_BLOCKED;
				}
			}
		}
	}					
}
