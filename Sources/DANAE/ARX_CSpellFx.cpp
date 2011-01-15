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
// ARX_Lightning
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//
// Updates: (date) (person) (update)
//-----------------------------------------------------------------------------
// jeudi 5 juillet 2001
// mercredi 4 juillet 2001
//		fix fireball + progressif
//		tests poison proj
//		fix tex rise dead
//		split sol bless
// mardi 3 juillet 2001
//		summon 10% poulet 90% demon
//		confuse devient -> rhaa vista au lieu de méga comunicatum
//		fix des cost des spells
// lundi 2 juillet 2001
//		paralyse + poufs + fix time 5 secondes
// vendredi 29 juin 2001
// jeudi 28
//		fix fireball pos not réinit
//		rise dead + summon fix tex qui wrap -> mirror
// jeudi 21 juin 2001
//		virer ombrignon magic sight
//		create food virer marmite -> cf heal en jaune orange
//		cure poison zone sur soi -> cf heal en vert
//		fire protec, cold protec -> virer rubans -> halo
//		ice field -> virer en haut
//		bless ya plus la couronne
//		summon trop haut
//		rise dead uniqmt zombie et momie
//		fire protection, cold, armor, lower sont mutuellement exclusifs
// jeudi 14 juin 2001
//		intégration des particules dand danae, poser translate, édit
// mercredi 13 juin 2001
//		spells
// Mardi 12 juin 2001
//		poison projectile
// Lundi 11 juin 2001
//		cure poison + armor + lower armor
//		aurevoir le spell activate portal
// Mercredi Jeudi Vendredi 06 07 08 juin 2001
//		pleins de trucs sur les spells
//		fire field + armor + bless (partoches)
// Mardi 05 juin 2001
//		Harm
//		CreateFood
// Jeudi 10 mai 2001
//		Ignit & Douse rayon 3 -> 6m
//		Magic Missile / version bonus / version sans bulles
//		Heal Particules
//		Ice Projectile dist + nb de pics en fct player stat + path
// Mercredi 9 mai 2001
//		Apres Editeur de particules
//		Finition des sorts pour l'E3
//		Ignit & douse ya plus le son si ya pas de lumières dans le rayon d'action
//		Ignit & Douse rayon 2m50
//		Ignit & Douse 500 ms
//		Ignit & Douse pas de son si rien à allumer / éteindre
//		Magic Missile + gros au milieu + fix orientation obj 3d sur le path
// Lundi 23 avril 2001
//		Finition, paufinage, binding avec le monde
// Vendredi 19 avril 2001
//		Mega Merge dans danae
// La semaine -> spells spells spells
// Jeudi 12 avril 2001
//		Curse ok
//		Confuse ok
// Mecredi 11 avril 2001
//		Magic Missile
//		Poison Projectile
// Mardi 10 avril 2001
//		Les classes
//		Bless
//		Incinerate
// Lundi 9 avril 2001
//		Truc générique de mouche sur path Herminte terminé avec alignement sur path
//		Spell Make Friend (confuse)
//		Spell Magic Missile
// Vendredi 6 avril 2001
//		Truc générique de mouche sur path Hermite (seb)
// Jeudi 5 avril 2001
//		Début truc générique de mouche sur path Hermite (seb)
//		Tests du path avec bézier -> pas cool paske passe pas par les points, enfin si
//		mais fo des ptains de tangeantes qui font chier
//		Negate Magic
//		Paralyse
// Mercredi 4 avril 2001
//		Tests béziers
//		Create Food -> ajout chaudron + particules
// Mardi 3 avril 2001
//		Spells Armor, Speed, Dispell Illusion, CreateFood
// Lundi 2 avril 2001
//		Spells Curse, Detect Traps, Make Friend, Fire Field
// Vendredi 30 mars 2001
//		Rise Dead -> fixer dans tous les angles
// Jeudi 29 mars 2001
//		Rune of Guarding -> + particles + 3 meshs -> trop chargé à voir
// Mardi 27 mars 2001
//		Cure Poison
//		Rune of Guarding
// Lundi 26 mars 2001
//		Début Incinerate
//		Réunion sur les spells
// Vendredi 23 mars 2001
//		Ice Terminé
//		Mesh Motte de glace
// Jeudi 22 mars 2001
//		Mesh Stalagmite
// Mercredi 21 mars 2001
//		Nouveaux Spells à faire: Incinerate & Ice projectile
// Mardi 20 mars 2001
//		Lightning -> texture, donc division du nombre de faces par 2
//		Lightning -> billboard 100% correct now par rapport à beta création
//		Summon deuxième timer pour la fermeture
// Lundi 19 mars 2001
//		summon nettoyage + terminé
// Vendredi 16 mars 2001
//		summon debug + nettoyage + améliorations + bordure
// Jeudi 15 mars 2001
//		summon creature déchirure + lum vol + intro + bevel + fin plat
//		+ billboard	+ vol light quad plats + bug fix
// Mercredi 14 mars 2001
//		summon creature déchirure
// Mardi 13 mar 2001
//		summon creature
// Lundi 12 mar 2001
//		cube force field -> moins speed, rotate bitmap à la pougne
//		+ jelly mauve / vert + intro
// Mardi 06 mar 2001
//		recherche sur cube force field
// Lundi 26 fev 2001
//		lightning color + frame update
// Vendredi 25 fev 2001
//		lightning objectifization et classification
// Jeudi 24 fev 2001
//		lightning integration danae + galere avec les angles (repere)
// Mercredi 23 fev 2001
//		lightning structure memoire + scintillement
// Mardi 22 fev 2001
//		lightning tests sympas + tweakage
// Lundi 21 fev 2001
//		lightning 1ers tests + vecteurs math
//-----------------------------------------------------------------------------
// Todo: Lightning -> collisions
//		 Ice -> pouf comme un train à vapeur à la progression
//		 Ice -> limiter la distance
//		 Ice -> intégrer dans la scène (avec les lumières dessus)
//		 ForceField -> rune qui tourne
//		 Summon -> linker avec le earthquake
//		 Cure Poison -> changer les tex + 1 variante
//		 Rune Of Guarding -> mesh qui va + boule + lum dyn
//		 Rise Dead -> fixer dans tous les angles
//		 Paralyse -> perso en noir et blanc figé
//		 -> life drain, mana et harm faire une grazubu de classe
//		 bless -> halo doré
//		 armor -> tex sur les y avec dégradé genre super power :o) sinon modif spécular
//		 detect trap, disarm trap, dispell illusion, enchant weapon
//		 magic sight, changer fov + doux, couleur, textruche (en prévoir ptet +sieures)
//		 poison proj finir les collisions
//		 cure poison bouffe mana par pj guéri
//		 paralyse -> plantes grimpantes + perso en noir et blanc
//		 negate magic
//		 todo checker fire protec, armor, cold -> annulent lower que si sort supérieur en niveau
//
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include "danae.h"
#include "ARX_CSpellFx.h"
#include "EERIEDRAW.h"

//-----------------------------------------------------------------------------
CSpellFx::CSpellFx() :
	lLightId(-1),
	fManaCostToLaunch(10),
	fManaCostPerSecond(10),
	fBeta(0)
{
	SetDuration(1000);
	SetAngle(fBeta);
	lSrc = -1;
	spellinstance = -1;
};

//-----------------------------------------------------------------------------
void CSpellFx::SetDuration(const unsigned long ulaDuration)
{
	ulDuration = ulaDuration;

	if (ulDuration <= 0) ulDuration = 100;

	fOneOnDuration = 1.f / (float)(ulDuration);

	ulCurrentTime = 0;
};

//-----------------------------------------------------------------------------
unsigned long CSpellFx::GetCurrentTime()
{
	return ulCurrentTime;
};

//-----------------------------------------------------------------------------
unsigned long CSpellFx::GetDuration()
{
	return ulDuration;
};

//-----------------------------------------------------------------------------
void CSpellFx::SetAngle(float afAngle)
{
	fBeta = afAngle;
	fBetaRad = DEG2RAD(fBeta);
	fBetaRadCos = (float) cos(fBetaRad);
	fBetaRadSin = (float) sin(fBetaRad);
}


//-----------------------------------------------------------------------------

void CSpellFx::Update(float _fParam)
{
	ARX_CHECK_ULONG(_fParam);
	Update(ARX_CLEAN_WARN_CAST_ULONG(_fParam));
}


//-----------------------------------------------------------------------------
void Draw3DLineTex(LPDIRECT3DDEVICE7 m_pd3dDevice, EERIE_3D s, EERIE_3D e, float fSize, int color)
{
	float fBeta = MAKEANGLE(player.angle.b);
	float zz = fSize; // size
	float xx = (float)(fSize * cos(DEG2RAD(fBeta)));

	D3DTLVERTEX v[4];
	D3DTLVERTEX v2[4];

	v2[0].color = v2[1].color = v2[2].color = v2[3].color = color;

	// version 2 faces
	v2[0].tu = 0;
	v2[0].tv = 0;
	v2[1].tu = 1;
	v2[1].tv = 0;
	v2[2].tu = 1;
	v2[2].tv = 1;
	v2[3].tu = 0;
	v2[3].tv = 1;

	v[0].sx = s.x;
	v[0].sy = s.y + zz;
	v[0].sz = s.z;

	v[1].sx = s.x;
	v[1].sy = s.y - zz;
	v[1].sz = s.z;

	v[2].sx = e.x;
	v[2].sy = e.y - zz;
	v[2].sz = e.z;

	v[3].sx = e.x;
	v[3].sy = e.y + zz;
	v[3].sz = e.z;
	
	EE_RT2(&v[0], &v2[0]);
	EE_RT2(&v[1], &v2[1]);
	EE_RT2(&v[2], &v2[2]);
	EE_RT2(&v[3], &v2[3]);
	ARX_DrawPrimitive_SoftClippZ(&v2[0],
	                             &v2[1],
	                             &v2[2]);
	ARX_DrawPrimitive_SoftClippZ(&v2[0],
	                             &v2[2],
	                             &v2[3]);

	zz *= (float) sin(DEG2RAD(fBeta));

	v[0].sx = s.x + xx;
	v[0].sy = s.y;
	v[0].sz = s.z + zz;

	v[1].sx = s.x - xx;
	v[1].sy = s.y;
	v[1].sz = s.z - zz;

	v[2].sx = e.x - xx;
	v[2].sy = e.y;
	v[2].sz = e.z - zz;

	v[3].sx = e.x + xx;
	v[3].sy = e.y;
	v[3].sz = e.z + zz;
	
	EE_RT2(&v[0], &v2[0]);
	EE_RT2(&v[1], &v2[1]);
	EE_RT2(&v[2], &v2[2]);
	EE_RT2(&v[3], &v2[3]);
	ARX_DrawPrimitive_SoftClippZ(&v2[0],
	                             &v2[1],
	                             &v2[2]);
	ARX_DrawPrimitive_SoftClippZ(&v2[0],
	                             &v2[2],
	                             &v2[3]);
}

//-----------------------------------------------------------------------------
void Draw3DLineTex(LPDIRECT3DDEVICE7 m_pd3dDevice, EERIE_3D s, EERIE_3D e, int color, float fStartSize, float fEndSize)
{
	float fBeta = MAKEANGLE(player.angle.b);
	float xxs = (float)(fStartSize * cos(DEG2RAD(fBeta)));
	float xxe = (float)(fEndSize * cos(DEG2RAD(fBeta)));
	float zzs = fStartSize;
	float zze = fEndSize;

	D3DTLVERTEX v[4];
	D3DTLVERTEX v2[4];

	v2[0].color = v2[1].color = v2[2].color = v2[3].color = color;

	// version 2 faces
	v2[0].tu = 0;
	v2[0].tv = 0;
	v2[1].tu = 1;
	v2[1].tv = 0;
	v2[2].tu = 1;
	v2[2].tv = 1;
	v2[3].tu = 0;
	v2[3].tv = 1;

	v[0].sx = s.x;
	v[0].sy = s.y + zzs;
	v[0].sz = s.z;

	v[1].sx = s.x;
	v[1].sy = s.y - zzs;
	v[1].sz = s.z;

	v[2].sx = e.x;
	v[2].sy = e.y - zze;
	v[2].sz = e.z;

	v[3].sx = e.x;
	v[3].sy = e.y + zze;
	v[3].sz = e.z;

	EE_RT2(&v[0], &v2[0]);
	EE_RT2(&v[1], &v2[1]);
	EE_RT2(&v[2], &v2[2]);
	EE_RT2(&v[3], &v2[3]);
	ARX_DrawPrimitive_SoftClippZ(&v2[0],
	                             &v2[1],
	                             &v2[2]);
	ARX_DrawPrimitive_SoftClippZ(&v2[0],
	                             &v2[2],
	                             &v2[3]);

	zzs *= (float) sin(DEG2RAD(fBeta));
	zze *= (float) sin(DEG2RAD(fBeta));

	v[0].sx = s.x + xxs;
	v[0].sy = s.y;
	v[0].sz = s.z + zzs;

	v[1].sx = s.x - xxs;
	v[1].sy = s.y;
	v[1].sz = s.z - zzs;

	v[2].sx = e.x - xxe;
	v[2].sy = e.y;
	v[2].sz = e.z - zze;

	v[3].sx = e.x + xxe;
	v[3].sy = e.y;
	v[3].sz = e.z + zze;
	
	EE_RT2(&v[0], &v2[0]);
	EE_RT2(&v[1], &v2[1]);
	EE_RT2(&v[2], &v2[2]);
	EE_RT2(&v[3], &v2[3]);
	ARX_DrawPrimitive_SoftClippZ(&v2[0],
	                             &v2[1],
	                             &v2[2]);
	ARX_DrawPrimitive_SoftClippZ(&v2[0],
	                             &v2[2],
	                             &v2[3]);
}

//-----------------------------------------------------------------------------
void Draw3DLineTex2(LPDIRECT3DDEVICE7 m_pd3dDevice, EERIE_3D s, EERIE_3D e, float fSize, int color, int color2)
{
	float fBeta = MAKEANGLE(player.angle.b);
	float zz = fSize; 
	float xx = (float)(fSize * cos(DEG2RAD(fBeta)));

	D3DTLVERTEX v[4];
	D3DTLVERTEX v2[4];

	v2[0].color = v2[1].color = color;
	v2[2].color = v2[3].color = color2;

	// version 2 faces
	v2[0].tu = 0;
	v2[0].tv = 0;
	v2[1].tu = 1;
	v2[1].tv = 0;
	v2[2].tu = 1;
	v2[2].tv = 1;
	v2[3].tu = 0;
	v2[3].tv = 1;

	v[0].sx = s.x;
	v[0].sy = s.y + zz;
	v[0].sz = s.z;

	v[1].sx = s.x;
	v[1].sy = s.y - zz;
	v[1].sz = s.z;

	v[2].sx = e.x;
	v[2].sy = e.y - zz;
	v[2].sz = e.z;

	v[3].sx = e.x;
	v[3].sy = e.y + zz;
	v[3].sz = e.z;

	EE_RT2(&v[0], &v2[0]);
	EE_RT2(&v[1], &v2[1]);
	EE_RT2(&v[2], &v2[2]);
	EE_RT2(&v[3], &v2[3]);
	ARX_DrawPrimitive_SoftClippZ(&v2[0],
	                             &v2[1],
	                             &v2[3]);
	ARX_DrawPrimitive_SoftClippZ(&v2[1],
	                             &v2[2],
	                             &v2[3]);


	zz *= (float) sin(DEG2RAD(fBeta));

	v[0].sx = s.x + xx;
	v[0].sy = s.y;
	v[0].sz = s.z + zz;

	v[1].sx = s.x - xx;
	v[1].sy = s.y;
	v[1].sz = s.z - zz;

	v[2].sx = e.x - xx;
	v[2].sy = e.y;
	v[2].sz = e.z - zz;

	v[3].sx = e.x + xx;
	v[3].sy = e.y;
	v[3].sz = e.z + zz;

	EE_RT2(&v[0], &v2[0]);
	EE_RT2(&v[1], &v2[1]);
	EE_RT2(&v[2], &v2[2]);
	EE_RT2(&v[3], &v2[3]);
	ARX_DrawPrimitive_SoftClippZ(&v2[0],
	                             &v2[1],
	                             &v2[3]);
	ARX_DrawPrimitive_SoftClippZ(&v2[1],
	                             &v2[2],
	                             &v2[3]);
}

//-----------------------------------------------------------------------------
void Split(D3DTLVERTEX * v, int a, int b, float fX, float fMulX, float fY, float fMulY, float fZ, float fMulZ)
{
	if (a != b)
	{
		int i = (int)((a + b) * 0.5f);

		if ((i != a) && (i != b))
		{
			v[i].sx = (v[a].sx + v[b].sx) * 0.5f + fX * frand2();
			v[i].sy = (v[a].sy + v[b].sy) * 0.5f + fY * frand2(); 
			v[i].sz = (v[a].sz + v[b].sz) * 0.5f + fZ * frand2(); 
			Split(v, a, i, fX, fMulX, fY, fMulY, fZ, fMulZ);
			Split(v, i, b, fX, fMulX, fY, fMulY, fZ, fMulZ);
		}
	}
}

//-----------------------------------------------------------------------------
void Split(D3DTLVERTEX * v, int a, int b, float yo, float fMul)
{
	if (a != b)
	{
		int i = (int)((a + b) * 0.5f);

		if ((i != a) && (i != b))
		{
			v[i].sx = (v[a].sx + v[b].sx) * 0.5f + yo * frand2(); 
			v[i].sy = (v[a].sy + v[b].sy) * 0.5f + yo * frand2(); 
			v[i].sz = (v[a].sz + v[b].sz) * 0.5f + yo * frand2(); 
			Split(v, a, i, yo * fMul);
			Split(v, i, b, yo * fMul);
		}
	}
}

