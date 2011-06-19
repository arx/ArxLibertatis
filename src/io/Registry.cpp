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

#include "io/Registry.h"

#include <algorithm>

#ifdef BUILD_EDITOR

/* Declare registry access functions */
HRESULT WriteRegKey(HKEY hKey, const char * strName, const char * strValue);
HRESULT WriteRegKeyValue(HKEY hKey, const char * strName, DWORD val);
HRESULT ReadRegKeyValue(HKEY hKey, const char * strName, long * val);
HRESULT ReadRegKey(HKEY hKey, const char * strName, char* strValue, DWORD dwLength, const char * strDefault);

//-----------------------------------------------------------------------------
// Name: WriteRegKey()
// Desc: Writes a registry key 
//-----------------------------------------------------------------------------
HRESULT WriteRegKey( HKEY hKey, const char * strName, const char * strValue )
{
	LONG bResult;

	bResult = RegSetValueEx( hKey, strName, 0, REG_SZ, 
							 (LPBYTE) strValue, strlen(strValue) + 1 );
	if ( bResult != ERROR_SUCCESS )
		return E_FAIL;

    return S_OK;
}

HRESULT WriteRegKeyValue( HKEY hKey, const char * strName, DWORD val )
{
	LONG bResult;

	bResult = RegSetValueEx( hKey, strName, 0, REG_DWORD, 
							 (LPBYTE) &val, 4 );
	if ( bResult != ERROR_SUCCESS )
		return E_FAIL;

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: ReadRegKey()
// Desc: Read a registry key 
//-----------------------------------------------------------------------------
HRESULT ReadRegKey( HKEY hKey, const char * strName, char* strValue, 
                    DWORD dwLength, const char * strDefault )
{
	DWORD dwType;
	LONG bResult;

	bResult = RegQueryValueEx( hKey, strName, 0, &dwType, 
							 (LPBYTE) strValue, &dwLength );
	if ( bResult != ERROR_SUCCESS )
		strcpy( strValue, strDefault );

    return S_OK;
}
HRESULT ReadRegKeyValue( HKEY hKey, const char * strName, long * val) {
	
	DWORD dwType;
	DWORD dwLength=4;
	
	RegQueryValueEx( hKey, strName, 0, &dwType, (LPBYTE) val, &dwLength );

	return S_OK;
}

// START - DANAE Registery Funcs ****************************************************************

//-----------------------------------------------------------------------------------------------
HKEY    DanaeKey= NULL;
#define DANAEKEY_KEY     TEXT("Software\\Arkane_Studios\\DANAE")
//-----------------------------------------------------------------------------------------------

void Danae_Registry_Open()
{
	RegCreateKeyEx( HKEY_CURRENT_USER, DANAEKEY_KEY, 0, NULL,
	                REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
	                &DanaeKey, NULL );
}

//-----------------------------------------------------------------------------------------------

void Danae_Registry_Close()
{
	RegCloseKey(DanaeKey);
	DanaeKey=NULL;
}

//-----------------------------------------------------------------------------------------------

void Danae_Registry_WriteValue(const char * string, DWORD value)
{
	if (DanaeKey == NULL) Danae_Registry_Open();

	if (DanaeKey != NULL)
	{
		WriteRegKeyValue( DanaeKey, string, value);
		Danae_Registry_Close();
	}
}

//-----------------------------------------------------------------------------------------------

void Danae_Registry_Write(const char * string, const char * text)
{
	if ( DanaeKey == NULL ) Danae_Registry_Open();

	if ( DanaeKey != NULL )
	{
		WriteRegKey( DanaeKey, string, text);
		Danae_Registry_Close();
	}
}

//-----------------------------------------------------------------------------------------------

void Danae_Registry_Read(const char * string, char * text, const char * defaultstr,long maxsize)
{
	if (DanaeKey==NULL) Danae_Registry_Open();

	if (DanaeKey!=NULL)
	{
		ReadRegKey( DanaeKey, string, text,maxsize,defaultstr);
		Danae_Registry_Close();
	}
	else
	{
		if ((defaultstr) && (defaultstr[0]!=0))
		{
			memcpy(text,defaultstr,std::min(maxsize+1,(long)strlen(defaultstr)+1));
			text[std::min(maxsize,(long)strlen(defaultstr))]=0;
		}
		else text[0]=0;
	}
}

//-----------------------------------------------------------------------------------------------

void Danae_Registry_ReadValue(const char * string, long * value, long defaultvalue)
{
	if (DanaeKey==NULL) Danae_Registry_Open();

	if (DanaeKey!=NULL)
	{
		ReadRegKeyValue( DanaeKey, string, value);
		Danae_Registry_Close();
	}
	else 
		*value = defaultvalue;
}

// END - DANAE Registery Funcs ******************************************************************
//-----------------------------------------------------------------------------------------------
#endif // BUILD_EDITOR

