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

#ifndef __MINOS_LIST_H__
#define __MINOS_LIST_H__

#define __MINOS_LIST_VERSION__ "0000"

#include "Minos_Common.h"

using namespace MINOS;

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//                                                                           //
// Class List                                                                //
//                                                                           //
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
template<class T>
class List
{
	public:
		//Constructor and destructor
		inline List();
		inline ~List();
		//Methods
		inline T & operator[](const UInt &);
		inline SBool Append(const T &);
		inline Void Remove();
		inline Void Remove(const UInt &);
		inline Void Free();
		inline UInt Count();
	private:
		//Implementation
		UInt count;
		T * data;
};

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//                                                                           //
// Constructor and destructor                                                //
//                                                                           //
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
template<class T>
inline List<T>::List() : count(0), data(NULL)
{
}

template<class T>
inline List<T>::~List()
{
	free(data);
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//                                                                           //
// Methods                                                                   //
//                                                                           //
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
template <class T>
inline T & List<T>::operator[](const UInt & i)
{
	return data[i];
}

template<class T>
inline SBool List<T>::Append(const T & t)
{
	Void * ptr = NULL;
	ptr = realloc(data, sizeof(T) * (count + 1));

	if (!ptr) return SFALSE;

	data = (T *)ptr;
	data[count++] = t;

	return STRUE;
}

template<class T>
inline Void List<T>::Remove()
{
	data = (T *)realloc(data, sizeof(T) * --count);
}

template<class T>
inline Void List<T>::Remove(const UInt & i)
{
	memcpy(&data[i], &data[i + 1], sizeof(T) *(--count - i));
	data = (T *)realloc(data, sizeof(T) * count);
}

template<class T>
inline Void List<T>::Free()
{
	free(data), data = NULL, count = 0;
}

template<class T>
inline UInt List<T>::Count()
{
	return count;
}

#endif//__MINOS_LIST_H__
