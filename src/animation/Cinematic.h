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

#ifndef ARX_ANIMATION_CINEMATIC_H
#define ARX_ANIMATION_CINEMATIC_H

#include <windows.h> // for HRESULT

#include "graphics/d3dwrapper.h"
#include "graphics/GraphicsTypes.h" // for EERIE_3D

// TODO macros
//fx
#define FX_FADEIN  1
#define FX_FADEOUT 2
#define FX_BLUR    3
//prefx
#define FX_DREAM   (1<<8)
//post fx
#define FX_FLASH   (1<<16)
#define FX_APPEAR  (2<<16)
#define FX_APPEAR2 (3<<16)
//all time
#define FX_LIGHT   (1<<24)


struct C_KEY;
struct CinematicGrid;

// TODO used for loading
#pragma pack(push,1)
class CinematicLight {
	
public:
	
	EERIE_3D pos;
	float fallin;
	float fallout;
	float r, g, b;
	float intensity;
	float intensiternd;
	C_KEY * prev;
	C_KEY * next;
	
	CinematicLight() {
		pos.x = pos.y = pos.z = 0.f;
		fallin = 100.f;
		fallout = 200.f;
		r = g = b = 255.f;
		intensity = 1.f;
		intensiternd = 0.2f;
		next = NULL;
	};
	
};
#pragma pack(pop)

class Cinematic {
	
public:
	EERIE_3D pos;
	float angz;
	EERIE_3D possuiv; // in the case of a non-fade interpolation
	float angzsuiv;
	int numbitmap;
	int numbitmapsuiv;
	float a;
	int fx;
	int fxsuiv;
	bool changekey;
	C_KEY * key;
	bool projectload;
	short ti;
	short tichoose;
	short force;
	int color;
	int colord;
	int colorflash;
	float speed;
	int colorchoose;
	int colorchoosed;
	int colorflashchoose;
	float speedchoose;
	int idsound;
	CinematicLight light;
	CinematicLight lightchoose;
	CinematicLight lightd;
	EERIE_3D posgrille;
	float angzgrille;
	EERIE_3D posgrillesuiv;
	float angzgrillesuiv;
	float speedtrack;
	float flTime;
	float m_flIntensityRND;
	
	Cinematic(int, int);
	bool ActiveTexture(int id);
	HRESULT InitDeviceObjects();
	HRESULT OneTimeSceneReInit();
	HRESULT Render(float framediff);
	HRESULT New();
	void ReInitMapp(int id);
	HRESULT DeleteDeviceObjects();
	
};

void DrawGrille(CinematicGrid * grille, int col, int fx, CinematicLight * light, EERIE_3D * posgrillesuiv, float angzgrillesuiv);
void FillKeyTemp(EERIE_3D * pos, float az, int frame, int numbitmap, int numfx, short ti, int color, int colord, int colorf, float speed, int idsound, short force, CinematicLight * light, EERIE_3D * posgrille, float angzgrille, float speedtrack);

#endif // ARX_ANIMATION_CINEMATIC_H
