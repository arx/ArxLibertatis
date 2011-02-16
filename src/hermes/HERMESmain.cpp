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

#include <time.h>
#include "HERMESMain.h"
#include "HERMESNet.h"
#include <hermes/Filesystem.h>

extern "C" {
#undef __cplusplus
#include <shlobj.h>
#include <windows.h>
#include <unistd.h>
}



#include <hermes/blast.h>

using namespace std;

// TODO is this correct?
#define _MAX_EXT 3
#define _MAX_FNAME 512
#define _MAX_DRIVE 1
#define _MAX_DIR _MAX_FNAME

char HermesBufferWork[MAX_PATH];	// Used by FileStandardize (avoid malloc/free per call)

UINT GaiaWM = 0;
HWND MAIN_PROGRAM_HANDLE = NULL;
long DEBUGG = 1;

void SAFEstrcpy(char * dest, char * src, unsigned long max)
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


bool IsIn(const char * strin, const char * str)
{
	const char * tmp = strstr(strin, str);
	
	if (tmp == NULL) return false;
	
	return true;
}

bool NC_IsIn(const char * strin, const char * str)
{
	char * tmp;
	char t1[4096];
	char t2[4096];
	strcpy(t1, strin);
	strcpy(t2, str);
	MakeUpcase(t1);
	MakeUpcase(t2);
	tmp = strstr(t1, t2);
	
	if (tmp == NULL) return false;
	
	return true;
}

void File_Standardize(const char * from, char * to)
{
	
	long pos = 0;
	long pos2 = 0;
	long size = strlen(from);
	char * temp = HermesBufferWork;
	
	strcpy(temp, from);
	/*

	while (pos < size)
	{
		if (((from[pos] == '\\') || (from[pos] == '/')) && (pos != 0))
		{
			while ((pos < size - 1)
			        && ((from[pos+1] == '\\') || (from[pos+1] == '/')))
				pos++;
		}

		temp[pos2++]	= ARX_CLEAN_WARN_CAST_CHAR(toupper(from[pos]));
		pos++;
	}

	temp[pos2] = 0;*/

again:
	;
	size = strlen(temp);
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
					strcpy(temp + pos, temp + fnd + 3);
					goto again;
				}
			}
		}

		pos++;
	}

	strcpy(to, temp);
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

void MakeUpcase(char * str)
{/* TODO 
	while(*str != '\0') {
		// islower is needed as the are read-only strings passed that are already in upper case?
		if(islower(*str)) {
			*str = toupper(*str);
		}
		str++;
	}*/
}

HKEY    ConsoleKey = NULL;
#define CONSOLEKEY_KEY     TEXT("Software\\Arkane_Studios\\ASMODEE")

void ConsoleSend(char * dat, long level, HWND source, long flag)
{
	RegCreateKeyEx(HKEY_CURRENT_USER, CONSOLEKEY_KEY, 0, NULL,
	               REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
	               &ConsoleKey, NULL);
	WriteRegKey(ConsoleKey, "ConsInfo", dat);
	WriteRegKeyValue(ConsoleKey, "ConsHwnd", (DWORD)source);
	WriteRegKeyValue(ConsoleKey, "ConsLevel", (DWORD)level);
	WriteRegKeyValue(ConsoleKey, "ConsFlag", (DWORD)flag);
	RegCloseKey(ConsoleKey);
}

void SendConsole(char * dat, long level, long flag, HWND source) {
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
void ForceSendConsole(char * dat, long level, long flag, HWND source)
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
struct MEMO_TRACE
{
	void 	*		ptr;
	char			string1[SIZE_STRING_MEMO_TRACE];
	unsigned long	size;
};

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

			sprintf(theader, "%12u %s\r\n", TotMemory, header);

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
 
long HERMES_CreateFileCheck(const char * name, char * scheck, const long & size, const float & id)
{
	printf("HERMES_CreateFileCheck(%s, ...)\n", name);
	WIN32_FILE_ATTRIBUTE_DATA attrib;
	FILE * file;
	long length(size >> 2), i = 7 * 4;

	if (!GetFileAttributesEx(name, GetFileExInfoStandard, &attrib)) return true;

	if (!(file = fopen(name, "rb"))) return true;

	fseek(file, 0, SEEK_END);

	memset(scheck, 0, size);
	((float *)scheck)[0] = id;
	((long *)scheck)[1] = size;
	memcpy(&((long *)scheck)[2], &attrib.ftCreationTime, sizeof(FILETIME));
	memcpy(&((long *)scheck)[4], &attrib.ftLastWriteTime, sizeof(FILETIME));
	((long *)scheck)[6] = ftell(file);
	memcpy(&scheck[i], name, strlen(name) + 1);
	i += strlen(name) + 1;
	memset(&scheck[i], 0, i % 4);
	i += i % 4;
	i >>= 2;

	fseek(file, 0, SEEK_SET);

	while (i < length)
	{
		long crc = 0;

		for (long j(0); j < 256; j++)
		{
			char read;

			if (!fread(&read, 1, 1, file)) break;

			crc += read;
		}

		((long *)scheck)[i++] = crc;

		if (feof(file))
		{
			memset(&((long *)scheck)[i], 0, (length - i) << 2);
			break;
		}
	}

	fclose(file);

	return false;
}

    #define GetLastChar(x)      strchr(x, '\0')
    #define NullTerminate(x)    x[i] = '\0'
    
    // from http://acmlm.kafuka.org/board/thread.php?id=3930
    static void splitpath(const char * path, char * drive, char * dir, char * fName, char * ext)
    {
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
    static void makepath(char *path, char *drive, char *dir, char *fName, char *ext)
    {
        char separator = '\\';
        char *lastChar = NULL;
        char *pos      = NULL;
        
        unsigned int    i = 0,
                        unixStyle = 0,
                        sepCount = 0; /* number of consecutive separators */
        

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
            {   path[i++] = '.'; sepCount++; }
            
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
            lastChar = GetLastChar(path) - 1;
            
            /* backpedal until we get rid of all the dots b/c what's the use of a dot on an extensionless file? */
            while(lastChar > path)
            {
                if((*lastChar) != '.')
                    break;
            
                (*lastChar) = '\0';
                lastChar--;
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

char * GetName(const char * str)
{
	splitpath(str, _drv, _dir, _name, _ext);
	return _name;
}

char * GetExt(const char * str)
{
	splitpath(str, _drv, _dir, _name, _ext);
	return _ext;
}

void SetExt(char * str, char * new_ext)
{
	splitpath(str, _drv, _dir, _name, _ext);
	makepath(str, _drv, _dir, _name, new_ext);
}

void AddToName(char * str, char * cat)
{
	splitpath(str, _drv, _dir, _name, _ext);
	strcat(_name, cat);
	makepath(str, _drv, _dir, _name, _ext);
}

void RemoveName(char * str)
{
	splitpath(str, _drv, _dir, _name, _ext);
	makepath(str, _drv, _dir, NULL, NULL);
}

bool CreateFullPath(const char * path) {
	printf("CreateFullPath(%s)", path);
	
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[MAX_PATH];

	splitpath(path, drive, dir, fname, ext);

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

	if (DirectoryExist(path)) return true;

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

bool HERMESFolderBrowse(char * str)
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


bool HERMESFolderSelector(char * file_name, char * title)
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
bool HERMES_WFSelectorCommon(const char * pstrFileName, const char * pstrTitleName, char * filter, long flag, long flag_operation, long max_car, HWND hWnd)
{
	LONG	value;
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

	return value;
}

int HERMESFileSelectorOpen(const char * pstrFileName, const char * pstrTitleName, char * filter, HWND hWnd)
{
	return HERMES_WFSelectorCommon(pstrFileName, pstrTitleName, filter, OFN_HIDEREADONLY | OFN_CREATEPROMPT, 1, MAX_PATH, hWnd);
}
 
int HERMESFileSelectorSave(const char * pstrFileName, const char * pstrTitleName, char * filter, HWND hWnd)
{
	return HERMES_WFSelectorCommon(pstrFileName, pstrTitleName, filter, OFN_OVERWRITEPROMPT, 0, MAX_PATH, hWnd);
}
 
//////////////////////////////////////// PACKING
//Always on for now...


/* Routine to read compressed data.  Used only by blast().
** This routine reads the compressed data that is to be uncompressed.
*/
size_t ReadCompressed(void * Param, const unsigned char ** buf) {
	
	PARAM * Ptr = (PARAM *) Param;
	
	*buf = (const unsigned char *)Ptr->pSource + Ptr->SourceOffset;
	
	size_t size = Ptr->CompressedSize;
	
	Ptr->SourceOffset += size;
	
	Ptr->CompressedSize = 0;

	return size;
}

/* Routine to write uncompressed data. Used only by explode().
** This routine writes the uncompressed data to a memory buffer.
*/
int WriteUnCompressed(void * Param, unsigned char * buf, size_t len) {
	
	PARAM * Ptr = (PARAM *) Param;
	
	if(Ptr->UnCompressedSize + len > Ptr->BufferSize) {
		Ptr->BufferSize = Ptr->UnCompressedSize + len * 10;
		Ptr->pDestination = (char *)realloc(Ptr->pDestination, Ptr->BufferSize);
	}
	
	if(Ptr->pDestination) {
		memcpy(Ptr->pDestination + Ptr->DestinationOffset, buf, len);
		Ptr->DestinationOffset += len;
		Ptr->UnCompressedSize += len;
		return 0;
	} else {
		return 1;
	}
	
}

int WriteUnCompressedNoAlloc(void * Param, unsigned char * buf, size_t len) {
	
	PARAM * Ptr = (PARAM *) Param;
	
	if(Ptr->UnCompressedSize + len > Ptr->BufferSize) {
		return 1;
	}
	
	if(Ptr->pDestination) {
		memcpy(Ptr->pDestination + Ptr->DestinationOffset, buf, len);
		Ptr->DestinationOffset += len;
		Ptr->UnCompressedSize += len;
		return 0;
	} else {
		return 1;
	}
	
}


char * STD_Explode(char * from, size_t from_size, size_t * to_size)
{

	PARAM Param;
	memset(&Param, 0, sizeof(PARAM));
	Param.BufferSize = 0;
	Param.pSource = from;
	Param.pDestination = NULL; 
	Param.CompressedSize = from_size;

	Param.Crc               = (unsigned long) - 1;
	unsigned int error = blast(ReadCompressed, &Param, WriteUnCompressed, &Param);

	if (error)
	{
		*to_size = 0; 
		return NULL;
	}

	*to_size = Param.UnCompressedSize;
	return Param.pDestination;
}

void STD_ExplodeNoAlloc(char * from, size_t from_size, char * to, size_t * to_size)
{

	PARAM Param;
	memset(&Param, 0, sizeof(PARAM));
	Param.BufferSize = *to_size;
	Param.pSource = from;
	Param.pDestination = to; 
	Param.CompressedSize = from_size;
	Param.Crc               = (unsigned long) - 1;
	unsigned int error = blast(ReadCompressed, &Param, WriteUnCompressedNoAlloc, &Param);

	if (error)
	{
		*to_size = 0; 
		return;
	}

	*to_size = Param.UnCompressedSize;
}
//-------------------------------------------------------------------------------------
// SP funcs
#define hrnd()  (((float)rand() ) * 0.00003051850947599f)

//-------------------------------------------------------------------------------------
// Error Logging Funcs...
char ERROR_Log_FIC[256];
long ERROR_Log_Write = 0;
void ERROR_Log_Init(char * fich)
{
	strcpy(ERROR_Log_FIC, fich);
	FILE * fic;

	if ((fic = fopen(ERROR_Log_FIC, "w")) != NULL)
	{
		ERROR_Log_Write = 1;
		fclose(fic);
	}
	else ERROR_Log_Write = 0;
}
bool ERROR_Log(char * fich)
{
	FILE * fic;

	if (ERROR_Log_Write)
	{
		if ((fic = fopen(ERROR_Log_FIC, "a+")) != NULL)
		{
			fprintf(fic, "Error: %s\n", fich);
			fclose(fic);
			return true;
		}
	}

	return false;
}
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

long HERMES_Memory_Emergency_Out(long size, char * info)
{
	if (HERMES_MEMORY_SECURITY)
		free(HERMES_MEMORY_SECURITY);

	HERMES_MEMORY_SECURITY = NULL;
	char out[512];

	if (info)
	{
		if (size > 0)
			sprintf(out, "FATAL ERROR: Unable To Allocate %d bytes... %s", size, info);
		else
			sprintf(out, "FATAL ERROR: Unable To Allocate Memory... %s", info);
	}
	else if (size > 0)
		sprintf(out, "FATAL ERROR: Unable To Allocate %d bytes...", size);
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

