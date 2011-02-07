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
// HERMESMain
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		HUM...hum...
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

// Desc: HERMES main functionalities   //FILES MEMORY
#include <cstring>
#include <cstdio>

#include <algorithm>
#include <iostream>
#include <algorithm>

#include <time.h>
#include "io/IO.h"
#include "io/Registry.h"
#include "io/Filesystem.h"
#include "io/Logger.h"
#include "core/Common.h"

#if ARX_COMPILER_MSVC
    #include <shlobj.h>
    #include <windows.h>
#else
    extern "C" {
    #undef __cplusplus
    #include <shlobj.h>
    #include <windows.h>
    #include <unistd.h>
    }

// TODO(lubosz): temporary include replacement
    #define _MAX_EXT 3
    #define _MAX_FNAME 512
    #define _MAX_DRIVE 1
    #define _MAX_DIR _MAX_FNAME
#endif

using namespace std;


char HermesBufferWork[MAX_PATH];    // Used by FileStandardize (avoid malloc/free per call)

UINT GaiaWM = 0;
HWND MAIN_PROGRAM_HANDLE = NULL;
long DEBUGG = 1;

void SAFEstrcpy(char * dest, const char * src, unsigned long max)
{
	if (strlen(src) > max)
	{
		memcpy(dest, src, max);
	}
	else
	{
		strcpy(dest, src);
	}
}

bool NC_IsIn( std::string t1, std::string t2 )
{
	MakeUpcase(t1);
	MakeUpcase(t2);

	return ( t1.find(t2) != string::npos );
}

bool IsIn(const std::string& strin, const std::string& str)
{
	return ( strin.find( str ) != std::string::npos );
}

void File_Standardize(const std::string& from, std::string& to)
{
	long pos = 0;
	long pos2 = 0;
	long size = from.length();
	std::string temp = from; /*HermesBufferWork;	

	while (pos < size)
	{
		if (((from[pos] == '\\') || (from[pos] == '/')) && (pos != 0))
		{
			while ((pos < size - 1)
					&& ((from[pos+1] == '\\') || (from[pos+1] == '/')))
				pos++;
		}

		temp[pos2++]	= (char)toupper(from[pos]);
		pos++;
	}

	temp[pos2] = 0;*/

	again:
	;
	size = temp.length();
	pos = 0;

	while (pos < size - 2)
	{
		if ((temp[pos] == '\\' || temp[pos] == '/') && (temp[pos+1] == '.') && (temp[pos+2] == '.'))
		{
			long fnd = pos;

			while (pos > 0)
			{
				pos--;

				if (temp[pos] == '\\' || temp[pos] == '/')
				{
					// Remove /descend/ascend bit found in path, e.g /dir/../
					temp = temp.substr( 0, pos ) + temp.substr( fnd + 3 );
					//strcpy(temp + pos, temp + fnd + 3);
					goto again;
				}
			}
		}

		pos++;
	}

	to = temp;
}

void GetDate(HERMES_DATE_TIME * hdt)
{
	struct tm * newtime;
 
	time_t long_time;

	time(&long_time);                  // Get time as long integer.
	newtime = localtime(&long_time);   // Convert to local time.

	if (newtime)
	{
		hdt->secs = newtime->tm_sec;
		hdt->mins = newtime->tm_min;
		hdt->hours = newtime->tm_hour;
		hdt->months = newtime->tm_mon + 1;
		hdt->days = newtime->tm_mday;
		hdt->years = newtime->tm_year + 1900;
	}
	else
	{
		hdt->secs = 0;
		hdt->mins = 0;
		hdt->hours = 0;
		hdt->months = 5;
		hdt->days = 32;
		hdt->years = 2002;
	}
}

long DebugLvl[6];
void HERMES_InitDebug()
{
	DebugLvl[0] = 1;
	DebugLvl[1] = 1;
	DebugLvl[2] = 1;
	DebugLvl[3] = 1;
	DebugLvl[4] = 1;
	DebugLvl[5] = 1;
	DEBUGG = 0;
}

void MakeUpcase( std::string& str ) {
	std::transform( str.begin(), str.end(), str.begin(), ::toupper );
}

HKEY    ConsoleKey = NULL;
#define CONSOLEKEY_KEY     TEXT("Software\\Arkane_Studios\\ASMODEE")

void ConsoleSend( const std::string& dat, long level, HWND source, long flag)
{
	printf("ConsoleSend: %s %ld %p %ld\n", dat.c_str(), level, source, flag);
	RegCreateKeyEx(HKEY_CURRENT_USER, CONSOLEKEY_KEY, 0, NULL,
				   REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
				   &ConsoleKey, NULL);
	WriteRegKey(ConsoleKey, "ConsInfo", dat.c_str() );
	WriteRegKeyValue(ConsoleKey, "ConsHwnd", (DWORD)source);
	WriteRegKeyValue(ConsoleKey, "ConsLevel", (DWORD)level);
	WriteRegKeyValue(ConsoleKey, "ConsFlag", (DWORD)flag);
	RegCloseKey(ConsoleKey);
}

void SendConsole( const std::string& dat, long level, long flag, HWND source) {
	if (GaiaWM != 0)
	{
		if (DebugLvl[0]) return;

		if (level < 1) return;

		if (level > 5) return;

		if (DebugLvl[level])
		{
			ConsoleSend(dat, level, source, flag);
			SendMessage(HWND_BROADCAST, GaiaWM, 33, 33);
		}
	}
}

long FINAL_COMMERCIAL_DEMO_bis = 1;

void ForceSendConsole( const std::string& dat, long level, long flag, HWND source)
{
	if (!FINAL_COMMERCIAL_DEMO_bis)
	{
		if (GaiaWM != 0)
		{
			ConsoleSend(dat, level, source, flag);
			SendMessage(HWND_BROADCAST, GaiaWM, 33, 33);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
HKEY    ComKey = NULL;
#define COMKEY_KEY     TEXT("Software\\Arkane_Studios\\GaiaCom")
char * HERMES_GaiaCOM_Receive()
{
	static char dat[1024];
	RegCreateKeyEx(HKEY_CURRENT_USER, COMKEY_KEY, 0, NULL,
				   REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
				   &ComKey, NULL);
	ReadRegKey(ComKey, "ComInfo",
			   dat, 512, "");
	RegCloseKey(ComKey);
	return dat;
}

//******************************************************************************
// MEMORY MANAGEMENT
//******************************************************************************
#ifdef _DEBUG
long HERMES_KEEP_MEMORY_TRACE = 0;
#else
long HERMES_KEEP_MEMORY_TRACE = 0;
#endif
#define SIZE_STRING_MEMO_TRACE 24
typedef struct
{
	void 	*		ptr;
	char			string1[SIZE_STRING_MEMO_TRACE];
	unsigned long	size;
} MEMO_TRACE;

MEMO_TRACE * MemoTraces = NULL;
long nb_MemoTraces = 0;
 
void HERMES_UnRegister_Memory(void * adr)
{
	long pos = -1;

	for (long i = 0; i < nb_MemoTraces; i++)
	{
		if (MemoTraces[i].ptr == adr)
		{
			pos = i;
			break;
		}
	}

	if (pos == -1) return;

	if (nb_MemoTraces == 1)
	{
		free(MemoTraces);
		MemoTraces = NULL;
		nb_MemoTraces = 0;
		return;
	}

	MEMO_TRACE * tempo = (MEMO_TRACE *)malloc(sizeof(MEMO_TRACE) * (nb_MemoTraces - 1));

	if (tempo == NULL)
	{
		HERMES_KEEP_MEMORY_TRACE = 0;
		free(MemoTraces);
		return;
	}

	for (int i = 0; i < nb_MemoTraces - 1; i++)
	{
		if (i >= pos)
		{
			memcpy(&MemoTraces[i], &MemoTraces[i+1], sizeof(MEMO_TRACE));
		}
	}

	memcpy(tempo, MemoTraces, sizeof(MEMO_TRACE)*(nb_MemoTraces - 1));
	free(MemoTraces);
	MemoTraces = (MEMO_TRACE *)tempo;
	nb_MemoTraces--;
}

unsigned long MakeMemoryText(char * text)
{
	if (HERMES_KEEP_MEMORY_TRACE == 0)
	{
		strcpy(text, "Memory Tracing OFF.");
		return 0;
	}

	bool * ignore;
	unsigned long TotMemory = 0;
	unsigned long TOTTotMemory = 0;
	char header[128];
	char theader[128];

	if (nb_MemoTraces == 0)
	{
		text[0] = 0;
		return 0;
	}

	ignore = (bool *)malloc(sizeof(bool) * nb_MemoTraces);

	for (long i = 0; i < nb_MemoTraces; i++) ignore[i] = false;

	strcpy(text, "");

	for (int i = 0; i < nb_MemoTraces; i++)
	{
		if (!ignore[i])
		{
			TotMemory = MemoTraces[i].size;

			if (MemoTraces[i].string1[0] != 0)
				strcpy(header, MemoTraces[i].string1);
			else strcpy(header, "<Unknown>");

			for (long j = i + 1; j < nb_MemoTraces; j++)
			{
				if (!ignore[j])
				{
					if (MemoTraces[j].string1[0] != 0)
						strcpy(theader, MemoTraces[j].string1);
					else strcpy(theader, "<Unknown>");

					if (!strcmp(header, theader))
					{
						ignore[j] = true;
						TotMemory += MemoTraces[j].size;
					}
				}
			}

			sprintf(theader, "%12lu %s\r\n", TotMemory, header);

			if (strlen(text) + strlen(theader) + 4 < 64000)
			{
				strcat(text, theader);
			}

			TOTTotMemory += TotMemory;
		}
	}

	free(ignore);
	return TOTTotMemory;
}

void MemFree(void * adr)
{
	if (HERMES_KEEP_MEMORY_TRACE)
		HERMES_UnRegister_Memory(adr);

	free(adr);
}
 
bool HERMES_CreateFileCheck(const char * name, char * scheck, size_t size, const float id)
{
	LogWarning << "partially unimplemented HERMES_CreateFileCheck";
	
	
	printf("HERMES_CreateFileCheck(%s, ...)\n", name);
	WIN32_FILE_ATTRIBUTE_DATA attrib;
	FileHandle file;
	long length(size >> 2), i = 7 * 4;

	// TODO port
	if (!GetFileAttributesEx(name, GetFileExInfoStandard, &attrib)) return true;

	if (!(file = FileOpenRead(name))) return true;

	FileSeek(file, 0, SEEK_END);

	memset(scheck, 0, size);
	((float *)scheck)[0] = id;
	((long *)scheck)[1] = size;
	memcpy(&((long *)scheck)[2], &attrib.ftCreationTime, sizeof(FILETIME));
	memcpy(&((long *)scheck)[4], &attrib.ftLastWriteTime, sizeof(FILETIME));
	((long *)scheck)[6] = FileTell(file);
	memcpy(&scheck[i], name, strlen(name) + 1);
	i += strlen(name) + 1;
	memset(&scheck[i], 0, i % 4);
	i += i % 4;
	i >>= 2;

	FileSeek(file, 0, SEEK_SET);

	while (i < length)
	{
		long crc = 0;
		
		for (long j = 0; j < 256; j++)
		{
			char read;

			if (!FileRead(file, &read, 1)) break;

			crc += read;
		}

		((long *)scheck)[i++] = crc;

		// TODO is this actually needed?
		//if (feof(file))
		//{
		//	memset(&((long *)scheck)[i], 0, (length - i) << 2);
		//	break;
		//}
	}

	FileCloseRead(file);

	return false;
}

	#define GetLastChar(x)      strchr(x, '\0')
	#define NullTerminate(x)    x[i] = '\0'
	
	// from http://acmlm.kafuka.org/board/thread.php?id=3930
	static void splitpath(const char * path, char * drive, char * dir, char * fName, char * ext)
	{
		//LogDebug << "Path: " << path <<  " || Drive: " << drive << " dir: " << dir << " fName: " << fName << " ext: " << ext;
		char separator = '\\';
		
		const char    *endPoint = NULL,
				*pos = (char*) path,
				*temp = NULL,
				*lastChar = NULL;
 
		unsigned int    i = 0,
						unixStyle = 0;
		
		/* initialize all the output strings in case we have to abort */
		if(drive) { strcpy(drive, ""); } 
		if(dir)   { strcpy(dir, "");   }
		if(fName) { strcpy(fName, "");  }
		if(ext)   { strcpy(ext, "");   }

		/* find the end of the string */
		lastChar = GetLastChar(path);

		if(path[0] == '/')
		{   separator = '/'; unixStyle = 1; }
		else
			separator = '\\';
	
		/* first figure out whether it contains a drive name */
		endPoint = strchr(path, separator);
		
		/* unix style drives are of the form "/drivename/" */
		if(unixStyle)
			endPoint = strchr(endPoint + 1, separator);

	/* unix style drives are of the form "/drivename/" */
	if(unixStyle)
		endPoint = strchr(endPoint + 1, separator);

	/* we found a drive name */
	if(endPoint && (endPoint < lastChar))
	{
		if(drive)
		{
			for(i = 0; pos + i < endPoint; i++)
				drive[i] = pos[i];

			NullTerminate(drive); /* null terminate the the drive string */
		}

		pos = endPoint;
	}
	else if(unixStyle)
	{

		if(drive)
		{
			for(i = 0; (pos + i) < lastChar; i++)
					drive[i] = pos[i];

			NullTerminate(drive);
		}
			
		return;
	}
	else
		/* this happens when there's no separators in the path name */
		endPoint = pos; 

	/* next, find the directory name, if any */
	temp = pos;

	while(temp && (endPoint < lastChar) )
	{
		temp = strchr(endPoint + 1, separator);

		if(temp) { endPoint = temp; }
	}

	/* if true, it means there's an intermediate directory name */
	if( (endPoint) && (endPoint > pos) && (endPoint < lastChar))
	{
		if(dir)
		{
			for(i = 0; (pos + i) <= endPoint; i++)
				dir[i] = pos[i];

			NullTerminate(dir);
		}

		pos = ++endPoint;
	}
	else
		/* this happens when there's no separators in the path name */
		endPoint = pos;

	/* find the file name */
	temp = pos;

	while(temp && (endPoint < lastChar))
	{
		temp = strchr(endPoint + 1, '.');

		if(temp) { endPoint = temp; }
	}

	if( (endPoint > pos) && (endPoint < lastChar))
	{
		if(fName)
		{
			for(i = 0; pos + i < endPoint; i++)
				fName[i] = pos[i];

			NullTerminate(fName);
		}

		pos = endPoint;
	}
	else if(endPoint == pos)
	{
		/* in this case there is no extension */
		if(fName)
		{
			for(i = 0; (pos + i) < lastChar; i++)
				fName[i] = pos[i];

			fName[i] = '\0';
		}

		return;
	}

	/* the remaining characters just get dumped as the extension */
	if(ext)
	{
		for(i = 0; pos + i < lastChar; i++)
			ext[i] = pos[i];

		NullTerminate(ext);
	}
		
	/* finished! :) */
	}
	
		// from http://acmlm.kafuka.org/board/thread.php?id=3930
	static void makepath(char *path, char *drive, char *dir, char *fName, const char *ext)
	{
		char separator = '\\';
		const char * lastChar = NULL;
		char *pos = NULL;

		unsigned int i = 0,
						unixStyle = 0,
						sepCount = 0; /* number of consecutive seperators */

	if(!path)
		return;
	
	/* Initialize the path to nothing */
	strcpy(path, "");
	
	if(drive)
	{
		if(drive[0] == '/')
		{   
			unixStyle = 1; separator = '/';
		}

		sepCount = 0;
		pos = (char*) drive;
		lastChar = GetLastChar(drive);

		if(lastChar == pos)
			goto directory;

		for(; pos < lastChar; pos++)
		{
			sepCount = ( (*pos) == separator ) ? sepCount + 1 : 0;

			/* filter out any extra separators */
			if(sepCount > 1) { continue; }
		
			path[i++] = (*pos);
		}
		
		if( (i) && path[i-1] != separator)
			path[i++] = separator;
		   
		NullTerminate(path);
	}

directory:

	if(dir)
	{
		sepCount = 0;
		pos = (char*) dir;
		lastChar = GetLastChar(dir);
	
		if(pos == lastChar)
			goto fileName;

		/* no character in the path yet? have to add that first separator */
		if(!i)
			path[i++] = separator; sepCount++;

		/* getting rid of any extra separators */
		while( ((*pos) == separator) && (pos < lastChar) )
			pos++;
		
		for( ; pos < lastChar; pos++)
		{
			sepCount = ( (*pos) == separator ) ? sepCount + 1 : 0;
		
			if(sepCount > 1) { continue; }
		
			path[i++] = (*pos);
		}
		
		if( (i) && path[i-1] != separator)
			path[i++] = separator;            
		
		NullTerminate(path);
	}

fileName:
	
	if(fName)
	{
		pos = (char*) fName;
		lastChar = GetLastChar(fName);
		
		if(lastChar == pos)
			goto extension;

		for(sepCount = 0; pos < lastChar; pos++)
		{
			sepCount = ( (*pos) == '.' ) ? sepCount + 1 : 0;
			
			if(sepCount > 1) { continue; }
			
			path[i++] = (*pos);
		}
	
		NullTerminate(path);
	}

extension:

	if(ext)
	{
		sepCount = 0;
		pos = (char*) ext;
		lastChar = GetLastChar(ext);

		if(lastChar == pos)
			return;

		if(i && (path[i - 1] != '.'))
		{ path[i++] = '.'; sepCount++; }
	
		for(; pos < lastChar; pos++)
		{
			sepCount = ( (*pos) == '.' ) ? sepCount + 1 : 0;
			
			if(sepCount > 1) { continue; }

			path[i++] = (*pos);
		}

		NullTerminate(path);
	}
	else
	{
		char * lastPathChar = GetLastChar(path) - 1;

		/* backpedal until we get rid of all the dots b/c what's the use of a dot on an extensionless file? */
		while(lastPathChar > path)
		{
			if((*lastPathChar) != '.')
				break;

			(*lastPathChar) = '\0';
			lastPathChar--;
		}
	}
}

#undef GetLastChar
#undef NullTerminate

//******************************************************************************
// FILES MANAGEMENT
//******************************************************************************
char _drv[256];
char _dir[256];
char _name[256];
char _ext[256];

std::string GetName( const std::string& str )
{
	splitpath(str.c_str(), _drv, _dir, _name, _ext);
	return _name;
}

char* GetExt( const std::string& str )
{
	splitpath(str.c_str(), _drv, _dir, _name, _ext);
	return _ext;
}

void SetExt(string & str, const string & new_ext) {
	size_t extpos = str.find_last_of("./\\");
	// No extension so far.
	if(extpos == string::npos || str[extpos] != '.') {
		if(!new_ext.empty() && new_ext[0] != '.') {
			str += '.';
		}
		str += new_ext;
		return;
	}
	if(!new_ext.empty() && new_ext[0] != '.') {
		str.resize(extpos + 1 + new_ext.length());
		str[extpos] = '.';
		std::copy(new_ext.begin(), new_ext.end(), str.begin() + extpos + 1);
	} else {
		str.resize(extpos + new_ext.length());
		std::copy(new_ext.begin(), new_ext.end(), str.begin() + extpos);
	}
}

void AddToName( std::string& str, const std::string& cat )
{
	splitpath(str.c_str(), _drv, _dir, _name, _ext);
	strcat(_name, cat.c_str() );
	char temp[512];
	strcpy( temp, str.c_str() );
	makepath(temp, _drv, _dir, _name, _ext);
	str = temp;
}

void RemoveName( std::string& str )
{
	char temp[512];
	strcpy( temp, str.c_str() );
	splitpath( temp, _drv, _dir, _name, _ext);
	makepath(temp, _drv, _dir, NULL, NULL);
	str = temp;
}

bool CreateFullPath( const std::string& path ) {
	printf("CreateFullPath(%s)", path.c_str() );

	char temp[512];
	strcpy( temp, path.c_str() );
	
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[MAX_PATH];

	splitpath( temp, drive, dir, fname, ext);

	if (strlen(dir) == 0) return false;

	char curpath[256];
	curpath[0] = '\0';
	//strcpy(curpath, drive);
	strcat(curpath, PATH_SEPERATOR_STR);
	long start = 1;
	long pos = 1;
	long size = strlen(dir);

	while (pos < size)
	{
		if ((dir[pos] == '\\') || (dir[pos] == '/'))
		{
			dir[pos] = 0;
			memcpy(curpath + strlen(curpath), dir + start, pos - start + 1);
			strcat(curpath, PATH_SEPERATOR_STR);
			CreateDirectory(curpath, NULL);
			start = pos + 1;
		}

		pos++;
	}

	if (DirectoryExist(temp)) return true;

	return false;
}

void ExitApp(int v)
{
	if (MAIN_PROGRAM_HANDLE != NULL)
		SendMessage(MAIN_PROGRAM_HANDLE, WM_CLOSE, 0, 0);

	exit(v);
}

//******************************************************************************
// OPEN/SAVE FILES DIALOGS
//******************************************************************************
char	LastFolder[MAX_PATH];		// Last Folder used
static OPENFILENAME ofn;

bool HERMESFolderBrowse(const char * str)
{
	BROWSEINFO		bi;
	LPITEMIDLIST	liil;

	bi.hwndOwner	= NULL;//MainFrameWindow;
	bi.pidlRoot		= NULL;
	bi.pszDisplayName = LastFolder;
	bi.lpszTitle	= str;
	bi.ulFlags		= 0;
	bi.lpfn			= NULL;
	bi.lParam		= 0;
	bi.iImage		= 0;


	liil = SHBrowseForFolder(&bi);

	if (liil)
	{
		if (SHGetPathFromIDList(liil, LastFolder))	return true;
		else return false;
	}
	else return false;
}


bool HERMESFolderSelector(char * file_name, const char * title)
{
	if (HERMESFolderBrowse(title))
	{
		sprintf(file_name, "%s\\", LastFolder);
		return true;
	}
	else
	{
		strcpy(file_name, " ");
		return false;
	}
}
bool HERMES_WFSelectorCommon(const char * pstrFileName, const char * pstrTitleName, const char * filter, long flag, long flag_operation, long max_car, HWND hWnd)
{
	BOOL	value;
	char	cwd[MAX_PATH];

	ofn.lStructSize		= sizeof(OPENFILENAME) ;
	ofn.hInstance			= NULL ;
	ofn.lpstrCustomFilter	= NULL ;
	ofn.nMaxCustFilter		= 0 ;
	ofn.nFilterIndex		= 0 ;
	ofn.lpstrFileTitle		= NULL ;
	ofn.nMaxFileTitle		= _MAX_FNAME + MAX_PATH ;
	ofn.nFileOffset		= 0 ;
	ofn.nFileExtension		= 0 ;
	ofn.lpstrDefExt		= "txt" ;
	ofn.lCustData			= 0L ;
	ofn.lpfnHook			= NULL ;
	ofn.lpTemplateName		= NULL ;

	ofn.lpstrFilter			= filter ;
	ofn.hwndOwner			= hWnd;
	ofn.lpstrFile			= strdup(pstrFileName);
	ofn.lpstrTitle			= pstrTitleName ;
	ofn.Flags				= flag;

	GetCurrentDirectory(MAX_PATH, cwd);
	ofn.lpstrInitialDir = cwd;
	ofn.nMaxFile = max_car;

	if (flag_operation)
	{
		value = GetOpenFileName(&ofn);
	}
	else
	{
		value = GetSaveFileName(&ofn);
	}
	
	free(ofn.lpstrFile);

	return value == TRUE;
}

int HERMESFileSelectorOpen(const char * pstrFileName, const char * pstrTitleName, const char * filter, HWND hWnd)
{
	return HERMES_WFSelectorCommon(pstrFileName, pstrTitleName, filter, OFN_HIDEREADONLY | OFN_CREATEPROMPT, 1, MAX_PATH, hWnd);
}
 
int HERMESFileSelectorSave(const char * pstrFileName, const char * pstrTitleName, const char * filter, HWND hWnd)
{
	return HERMES_WFSelectorCommon(pstrFileName, pstrTitleName, filter, OFN_OVERWRITEPROMPT, 0, MAX_PATH, hWnd);
}
 
//-------------------------------------------------------------------------------------
// SP funcs
#define hrnd()  (((float)rand() ) * 0.00003051850947599f)

//-------------------------------------------------------------------------------------
// Error Logging Funcs...
char * HERMES_MEMORY_SECURITY = NULL;
void HERMES_Memory_Security_On(long size)
{
	if (size < 128000) size = 128000;

	HERMES_MEMORY_SECURITY = (char *)malloc(size);
}
void HERMES_Memory_Security_Off()
{
	if (HERMES_MEMORY_SECURITY)
		free(HERMES_MEMORY_SECURITY);

	HERMES_MEMORY_SECURITY = NULL;
}

long HERMES_Memory_Emergency_Out(long size, const char * info)
{
	if (HERMES_MEMORY_SECURITY)
		free(HERMES_MEMORY_SECURITY);

	HERMES_MEMORY_SECURITY = NULL;
	char out[512];

	if (info)
	{
		if (size > 0)
			sprintf(out, "FATAL ERROR: Unable To Allocate %ld bytes... %s", size, info);
		else
			sprintf(out, "FATAL ERROR: Unable To Allocate Memory... %s", info);
	}
	else if (size > 0)
		sprintf(out, "FATAL ERROR: Unable To Allocate %ld bytes...", size);
	else
		sprintf(out, "FATAL ERROR: Unable To Allocate Memory...");

	int re = MessageBox(NULL, out, "ARX Fatalis", MB_RETRYCANCEL    | MB_ICONSTOP);

	if ((re == IDRETRY) && (size > 0))
		return 1;

	exit(0);
	return 0;
}

LARGE_INTEGER	start_chrono;
long NEED_BENCH = 0;
void StartBench()
{
	if (NEED_BENCH)
	{
		QueryPerformanceCounter(&start_chrono);
	}
}
unsigned long EndBench()
{
	if (NEED_BENCH)
	{
		LARGE_INTEGER	end_chrono;
		QueryPerformanceCounter(&end_chrono);
		unsigned long ret = (unsigned long)(end_chrono.QuadPart - start_chrono.QuadPart);
		return ret;
	}

	return 0;
}

