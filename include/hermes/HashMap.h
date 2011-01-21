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
#ifndef HACHAGE_H
#define HACHAGE_H

#include <cstddef>

class HashMap {
	
private:
	
	struct Entry {
		const char * name;
		void * value;
	};
	
	std::size_t size;
	std::size_t mask;
	std::size_t fill;
	
	Entry * data;
	
private:
	
	std::size_t FuncH1(std::size_t);
	std::size_t FuncH2(std::size_t);
	std::size_t getHash(const char * );
	
public:
	
	HashMap(std::size_t size = 256);
	~HashMap();
	
	bool add(const char * name, void * value = NULL);
	
	void * get(const char * name);
	
};

#endif
