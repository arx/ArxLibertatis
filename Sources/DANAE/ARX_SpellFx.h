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
//		ARX Lightning SFX
//
// Updates: (date) (person) (update)
//-----------------------------------------------------------------------------
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
//
// Code: Didier Pédreno - Sébastien Scieux
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#ifndef ARX_SPELLFX_H
#define ARX_SPELLFX_H

#include "ARX_Time.h"
#include "ARX_CSpellFx.h"
#include "ARX_SpellFx_Lvl01.h"
#include "ARX_SpellFx_Lvl02.h"
#include "ARX_SpellFx_Lvl03.h"
#include "ARX_SpellFx_Lvl04.h"
#include "ARX_SpellFx_Lvl05.h"
#include "ARX_SpellFx_Lvl06.h"
#include "ARX_SpellFx_Lvl07.h"
#include "ARX_SpellFx_Lvl08.h"
#include "ARX_SpellFx_Lvl09.h"
#include "ARX_SpellFx_Lvl10.h"


#endif
