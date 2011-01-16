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
#ifndef __ATHENA_RESOURCE_H__
#define __ATHENA_RESOURCE_H__

#include <stdio.h>
#include <Athena_Types.h>
#include <windows.h>
#include <dsound.h>


namespace ATHENA
{

	const aalULong ALIGNMENT(0x10);

	FILE * OpenResource(const char * name, const char * resource_path);


	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Class ResourceHandle                                                      //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	class ResourceHandle
	{
		public:
			//Constructors and destructor
			ResourceHandle()
			{
				__count = 0;
			}
			virtual ~ResourceHandle() {};
			//Operators
			inline aalVoid Catch()
			{
				++__count;
			}
			inline aalVoid Release()
			{
				--__count;
			}
			inline aalSInt IsHandled()
			{
				return __count;
			}
		private:
			aalSInt __count;
	};

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Class ResourceList                                                        //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	template <class T>
	class ResourceList
	{
		public:
			//Constructor and destructor
			ResourceList();
			~ResourceList();
			//Operaators
			inline aalUBool IsValid(const aalSLong & index);
			inline aalUBool IsNotValid(const aalSLong & index);
			inline T * operator[](const aalSLong & index);
			inline aalULong Size()
			{
				return size;
			}
			inline aalSLong Add(T * element);
			inline aalVoid Delete(const aalSLong & index);
			inline aalVoid Clean(bool bOk);
		private:
			//Data
			aalULong size;
			T ** list;
	};

	template <class T>
	inline ResourceList<T>::ResourceList() : size(0), list(NULL)
	{
	}

	template <class T>
	inline ResourceList<T>::~ResourceList()
	{
		for (aalULong i(0); i < size; i++) if (list[i]) delete list[i];
	}

	template <class T>
	inline aalUBool ResourceList<T>::IsValid(const aalSLong & index)
	{
		return (aalULong(index) < size && list[index]) ? AAL_UTRUE : AAL_UFALSE;
	}

	template <class T>
	inline aalUBool ResourceList<T>::IsNotValid(const aalSLong & index)
	{
		return (aalULong(index) >= size || !list[index]) ? AAL_UTRUE : AAL_UFALSE;
	}

	template <class T>
	inline T * ResourceList<T>::operator[](const aalSLong & index)
	{
		return list[index];
	}

	template <class T>
	inline aalSLong ResourceList<T>::Add(T * element)
	{
		aalVoid * ptr;
		aalULong i(0);

		for (; i < size; i++)
		{
			if (!list[i])
			{
				list[i] = element;
				return i;
			}
		}

		ptr = realloc(list, (size + ALIGNMENT) * sizeof(T *));

		if (!ptr) return AAL_SFALSE;

		list = (T **)ptr, size += ALIGNMENT;

		memset(&list[i], 0, ALIGNMENT * sizeof(T *));
		list[i] = element;

		return i;
	}

	template <class T>
	inline aalVoid ResourceList<T>::Delete(const aalSLong & index)
	{
		if (aalULong(index) >= size  || !list[index]) return;

		delete list[index];
		list[index] = NULL;

		if (size <= ALIGNMENT) return;

		for (aalULong j(size - ALIGNMENT); j < size; j++) if (list[j]) return;

		list = (T **)realloc(list, (size -= ALIGNMENT) * sizeof(T *));
	}

	template <class T>
	inline aalVoid ResourceList<T>::Clean(bool bOk)
	{

		for (aalULong i(0); i < size; i++)
			if (list[i])
			{
				delete list[i];
			}

		free(list), list = NULL, size = 0;
	}

}//ATHENA::

#endif//__ATHENA_RESOURCE_H__
