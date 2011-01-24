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

#include "hermes/HashMap.h"

#include <cstring>
#include <cassert>

#include <algorithm>
using std::transform;

#include <string>
using std::string;

HashMap::HashMap(size_t sz) {
	
	data = new Entry[sz];
	
	size = sz;
	fill = 0;
	
	/*for(size_t i = 0; i < sz; i++) {
		data[i].name = NULL;
	}*/
	
	// TODO size must be a power of two
	mask = sz - 1;
}

HashMap::~HashMap() {
	
	/*while(size--) {
		if(data[size].name) {
			free((void *)data[size].name);
			data[size].name = NULL;
		}
	}*/

	delete[] data;
}

bool HashMap::add(const std::string& name, void* value) {
	
	string lname = name;
	transform(lname.begin(), lname.end(), lname.begin(), tolower);
	
	if(fill >= (size * 3) / 4) {
		// TODO recreate the table
		//size <<= 1;
		//mask = size - 1;
		//data = (Entry *)realloc(data, size * sizeof(Entry));
		return false;
	}
	
	size_t hash = getHash(lname.c_str());
	size_t h1 = FuncH1(hash);
	size_t h2 = FuncH2(hash);
	
	for(size_t i = 0; i < size; i++) {
		
		h1 &= mask;
		
		if ( data[h1].name.empty() ) {
			data[h1].name = lname;
			data[h1].value = value;
			assert( !data[h1].name.empty() );
			fill++;
			return true;
		}
		
		h1 += h2;
	}
	
	return false;
}

void * HashMap::get(const std::string& name) {
	
	string lname = name;
	transform(lname.begin(), lname.end(), lname.begin(), tolower);
	
	size_t hash = getHash(lname.c_str());
	size_t h1 = FuncH1(hash);
	size_t h2 = FuncH2(hash);
	
	for(size_t i = 0; i < size; i++) {
		h1 &= mask;
		
		if( !data[h1].name.empty() ) {
			if(!strcmp(lname.c_str(), data[h1].name.c_str())) {
				return data[h1].value;
			}
		}
		
		h1 += h2;
	}
	
	return NULL;
}

size_t HashMap::FuncH1(size_t hash) {
	return hash;
}

size_t HashMap::FuncH2(size_t hash) {
	return ((hash >> 1) | 1);
}

size_t HashMap::getHash(const std::string& name) {
	
	size_t iKey = 0;
	size_t len = name.length();
	
	for(size_t i = 0; i < len; i++) {
		iKey += name[i] * (i + 1) + name[i] * len;
	}
	
	return iKey;
}
