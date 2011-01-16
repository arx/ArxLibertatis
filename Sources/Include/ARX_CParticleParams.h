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
//-----------------------------------------------------------------------------
// création du fichier le mardi 22 mai18:32
//-----------------------------------------------------------------------------


#ifndef _ARX_CPARTICLE_PARAMS_H
#define _ARX_CPARTICLE_PARAMS_H

#include "EERIETypes.h"

typedef EERIE_3D Point3;

//-----------------------------------------------------------------------------
class CParticleParams
{
	public:
		Point3	p3Pos;
		Point3	p3Direction;
		Point3	p3Gravity;
		int		iNbMax;
		int		iFreq;
		bool	bRotationRandomDirection;
		bool	bRotationRandomStart;
		float	fLife;
		float	fLifeRandom;
		float	fAngle;
		float	fSpeed;
		float	fSpeedRandom;
		float	fFlash;
		float	fRotation;

	public:
		bool	bTexInfo;
		bool	bTexLoop;
		int		iTexNb;
		int		iTexTime;
		int		iBlendMode;
		char	* lpszTexName;

	public:
		bool	bStartLock;
		float	fStartSize;
		float	fStartSizeRandom;
		float	fStartColor[4];
		float	fStartColorRandom[4];

	public:
		bool	bEndLock;
		float	fEndSize;
		float	fEndSizeRandom;
		float	fEndColor[4];
		float	fEndColorRandom[4];

	public:
		CParticleParams()
		{
			bTexInfo = FALSE;
		};
};

#endif
