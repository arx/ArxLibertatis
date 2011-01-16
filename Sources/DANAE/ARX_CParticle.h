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
#ifndef ARX_CPARTICLE_H
#define ARX_CPARTICLE_H

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#include "eerietypes.h"
#include <list>
using namespace std;

//-----------------------------------------------------------------------------
typedef EERIE_3D Point3;
 

//-----------------------------------------------------------------------------
class CParticle
{
	public:
		Point3	p3Pos;
		Point3	p3OldPos;
		Point3  p3Velocity;

		// time
		float	fOneOnTTL;
		float	fTimeSec;
		long	ulTime;	// Age
		long	ulTTL;	// Time to Live

		// size
		float	fSize;
		float	fSizeStart;
		float	fSizeEnd;

		// color
		float	fColorStart[4];
		float	fColorEnd[4];
		float	fColor[4];
		unsigned long	ulColor;

		// rotation
		int		iRot;
		float	fRotStart;
		float	fRotation;

		// tex infos
		int		iTexTime;
		int		iTexNum;

	public:
		CParticle();
		~CParticle();

	public:
		void	Create();
		void	Create(Point3 _p3Pos, long, unsigned long, float);
		void	Regen();
		void	Update(long);
		bool	isAlive()
		{
			return (ulTime < ulTTL);
		};
		void	Validate();
};

#endif
