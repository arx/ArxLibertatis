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
/*
  vendredi 23 novembre
    màj update (check if size > 0)
  vendredi 15 juin 2001
	amélorations, rajouts de trucs genre blend (nouvel alpha pour blood par ex)
	insertion dans l'éditeur de Danae
	affichage dans l'éditeur
	mise à jour avec la dll
	calcul du point pour selection souris
  jeudi 31 mai code cosmetics + optims
  mercredi 30 mai nettoyage + préparations des VB + optims draw sprite + rotated
			-> passage de trianglefan à loop
  mardi 29 mai optims diverses + fix spawn
  lundi 28 mai fix des upates par la dll + random color lock + fix speed (slider -> edit)
			+ fix set params deg2rad angle à faire en amont
			+ blend + flash + texture loop
  vendredi 25 mai DLL dans danae
  mercredi 23 mai 2001 interface revue + stuff added
			life + rand2 -> life + rnd
			size + rand2 -> size + rnd
			color + rand2 -> color + rnd
			alpha + rand2 -> alpha + rnd
			changement des slideurs -> rand2 -> rnd (genre min max)
  lundi 21 mai 2001 code cosmetics + fire and forget particles + texture loop on/off
			  + interface revue
  semaine e3 sens rotation + cone revu
  lundi 07 mai 2001 time textures animées + spray cone plus ou moins + alpha
			  + rotation
  04 mai 2001 textures animées + fix random velocity
				base size
  03 mai 2001 interface (random) + intégration danae ok
  02 mai 2001 classes + interface (base) + début intégration danae
  30 avril 2001 classes

todo:
	keys
	freq
	cf eeriedrawsprite pour la taille
	add & forget
	swpan type
	freq
	toto random tex start (qd multi)
	bounding box
*/


//-----------------------------------------------------------------------------
#include "danae.h"
#include "arx_cspellfx.h"
#include "arx_cparticles.h"
#include "arx_cparticleParams.h"

#include "EERIEDraw.h"
#include "EERIEPoly.h"

#include <list>
using namespace std;


#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

//-----------------------------------------------------------------------------
CParticleManager::CParticleManager()
{
	listParticleSystem.clear();
}

//-----------------------------------------------------------------------------
CParticleManager::~CParticleManager()
{
	Clear();
}

//-----------------------------------------------------------------------------
void CParticleManager::Clear()
{
	list<CParticleSystem *>::iterator i;

	i = listParticleSystem.begin();
	CParticleSystem * pP;

	while (i != listParticleSystem.end())
	{
		pP = *i;
		++i;

		if (pP)
		{
			delete pP;
			listParticleSystem.remove(pP);
		}
	}

	listParticleSystem.clear();
}

//-----------------------------------------------------------------------------
void CParticleManager::AddSystem(CParticleSystem * _pPS)
{
	listParticleSystem.insert(listParticleSystem.end(), _pPS);
}

//-----------------------------------------------------------------------------
void CParticleManager::Update(long _lTime)
{
	if (!listParticleSystem.size()) return;

	list<CParticleSystem *>::iterator i;
	i = listParticleSystem.begin();

	while (i != listParticleSystem.end())
	{
		CParticleSystem * p = *i;
		++i;

		if (!p->IsAlive())
		{
			delete p;
			listParticleSystem.remove(p);
		}
		else
		{
			p->Update(_lTime);
		}
	}
}

//-----------------------------------------------------------------------------

void CParticleManager::Render(LPDIRECT3DDEVICE7 _lpD3DDevice)
{
	int ilekel = 0;
	list<CParticleSystem *>::iterator i;

	for (i = listParticleSystem.begin(); i != listParticleSystem.end(); ++i)
	{
		CParticleSystem * p = *i;
		p->Render(_lpD3DDevice);
		ilekel++;
	}
}

