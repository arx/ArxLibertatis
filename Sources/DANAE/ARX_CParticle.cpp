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
#include "EERIEMath.h"
#include "arx_cspellfx.h"
#include "arx_cparticle.h"

#include <list>
using namespace std;

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

//-----------------------------------------------------------------------------
CParticle::CParticle()
{
	p3Pos.x = frand2() * 5;
	p3Pos.y = frand2() * 5;
	p3Pos.z = frand2() * 5;

	p3Velocity.x = frand2() * 10;
	p3Velocity.y = frand2() * 10;
	p3Velocity.z = frand2() * 10;

	float frnd	=	2000 + rnd() * 3000;
	ARX_CHECK_LONG(frnd);
	ulTTL		=	ARX_CLEAN_WARN_CAST_LONG(frnd);
	fOneOnTTL = 1.0f / (float) ulTTL;
	ulTime		=	0;


	fSizeStart = 1;
	fSizeEnd = 1;
	fColorStart[0] = 1;
	fColorStart[1] = 1;
	fColorStart[2] = 1;
	fColorStart[3] = 0.5f;
	fColorEnd[0] = 1;
	fColorEnd[1] = 1;
	fColorEnd[2] = 1;
	fColorEnd[3] = 0.1f;

	iTexTime = 0;
	iTexNum = 0;
}

//-----------------------------------------------------------------------------
CParticle::~CParticle()
{
 
}

//-----------------------------------------------------------------------------
void CParticle::Regen()
{
	p3OldPos.x = p3Pos.x = 0;
	p3OldPos.y = p3Pos.y = 0;
	p3OldPos.z = p3Pos.z = 0;

	ulTime = 0;
	fSize = 1;
	iTexTime = 0;
	iTexNum = 0;
}

//-----------------------------------------------------------------------------
void CParticle::Validate()
{
	if (fSize < 1)
		fSize = 1;

	if (fSizeStart < 0)
		fSizeStart = 0;

	if (fSizeEnd < 0)
		fSizeEnd = 0;

	for (int i = 0; i < 4; i++)
	{
		if (fColorStart[i] < 0)
			fColorStart[i] = 0;

		if (fColorStart[i] > 1)
			fColorStart[i] = 1;

		if (fColorEnd[i] < 0)
			fColorEnd[i] = 0;

		if (fColorEnd[i] > 1)
			fColorEnd[i] = 1;
	}

	if (ulTTL < 100)
	{
		ulTTL = 100;
		fOneOnTTL = 1.0f / (float)ulTTL;
	}
}

//-----------------------------------------------------------------------------
void CParticle::Update(long _lTime)
{
	ulTime += _lTime;
	iTexTime += _lTime;
	fTimeSec = _lTime * DIV1000;

	if (ulTime < ulTTL)
	{
		float ft = fOneOnTTL * ulTime;

		// backup old pos
		p3OldPos.x = p3Pos.x;
		p3OldPos.y = p3Pos.y;
		p3OldPos.z = p3Pos.z;

		// update new pos
		p3Pos.x += p3Velocity.x * fTimeSec; 
		p3Pos.y += p3Velocity.y * fTimeSec; 
		p3Pos.z += p3Velocity.z * fTimeSec;

		fSize = fSizeStart + (fSizeEnd - fSizeStart) * ft;
		fColor[0] = fColorStart[0] + (fColorEnd[0] - fColorStart[0]) * ft;
		fColor[1] = fColorStart[1] + (fColorEnd[1] - fColorStart[1]) * ft;
		fColor[2] = fColorStart[2] + (fColorEnd[2] - fColorStart[2]) * ft;
		fColor[3] = fColorStart[3] + (fColorEnd[3] - fColorStart[3]) * ft;

		ulColor = D3DRGBA(fColor[0], fColor[1], fColor[2], fColor[3]);
	}
}
