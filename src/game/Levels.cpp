/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */
/* Based on:
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
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#include "game/Levels.h"

#include <cstring>

#include <boost/algorithm/string/predicate.hpp>

int ARX_LEVELS_GetRealNum(long num) {
	
	if(num < 0) {
		return -1;
	}
	
	switch(num) {
		case 0:
		case 8:
		case 11:
		case 12:
			return 0;
		case 1:
		case 13:
		case 14:
			return 1;
		case 2:
		case 15:
			return 2;
		case 3:
		case 16:
		case 17:
			return 3;
		case 4:
		case 18:
		case 19:
			return 4;
		case 5:
		case 21:
			return 5;
		case 6:
		case 22:
			return 6;
		case 7:
		case 23:
			return 7;
		default:
			return num;
	}
	
}

long GetLevelNumByName(const std::string & name) {
	
	if ( !name.empty() )
	{
		if (boost::contains(name, "level10")) return 10;
		if (boost::contains(name, "level11")) return 11;
		if (boost::contains(name, "level12")) return 12;
		if (boost::contains(name, "level13")) return 13;
		if (boost::contains(name, "level14")) return 14;
		if (boost::contains(name, "level15")) return 15;
		if (boost::contains(name, "level16")) return 16;
		if (boost::contains(name, "level17")) return 17;
		if (boost::contains(name, "level18")) return 18;
		if (boost::contains(name, "level19")) return 19;
		if (boost::contains(name, "level20")) return 20;
		if (boost::contains(name, "level21")) return 21;
		if (boost::contains(name, "level22")) return 22;
		if (boost::contains(name, "level23")) return 23;
		if (boost::contains(name, "level24")) return 24;
		if (boost::contains(name, "level25")) return 25;
		if (boost::contains(name, "level26")) return 26;
		if (boost::contains(name, "level27")) return 27;
		if (boost::contains(name, "leveldemo2")) return 29;
		if (boost::contains(name, "leveldemo3")) return 30;
		if (boost::contains(name, "leveldemo4")) return 31;
		if (boost::contains(name, "leveldemo")) return 28;
		if (boost::contains(name, "level0")) return 0;
		if (boost::contains(name, "level1")) return 1;
		if (boost::contains(name, "level2")) return 2;
		if (boost::contains(name, "level3")) return 3;
		if (boost::contains(name, "level4")) return 4;
		if (boost::contains(name, "level5")) return 5;
		if (boost::contains(name, "level6")) return 6;
		if (boost::contains(name, "level7")) return 7;
		if (boost::contains(name, "level8")) return 8;
		if (boost::contains(name, "level9")) return 9;
	}

	return -1;
}

const char * GetLevelNameByNum(long num) {
	
	switch(num) {
		case 0: return "0";
		case 1: return "1";
		case 2: return "2";
		case 3: return "3";
		case 4: return "4";
		case 5: return "5";
		case 6: return "6";
		case 7: return "7";
		case 8: return "8";
		case 9: return "9";
		case 10: return "10";
		case 11: return "11";
		case 12: return "12";
		case 13: return "13";
		case 14: return "14";
		case 15: return "15";
		case 16: return "16";
		case 17: return "17";
		case 18: return "18";
		case 19: return "19";
		case 20: return "20";
		case 21: return "21";
		case 22: return "22";
		case 23: return "23";
		case 24: return "24";
		case 25: return "25";
		case 26: return "26";
		case 27: return "27";
		case 28: return "demo";
		case 29: return "demo2";
		case 30: return "demo3";
		case 31: return "demo4";
		default: return "none";
	}
}
