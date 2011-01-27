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
#include <windows.h>
#include <shlobj.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <io.h>
#include <time.h>
#include <direct.h>			// _getcwd
#include "HERMESmain.h"
#include "HERMESNet.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>


char HermesBufferWork[_MAX_PATH];	// Used by FileStandardize (avoid malloc/free per call)

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


unsigned char IsIn(char * strin, char * str)
{
	char * tmp;
	tmp = strstr(strin, str);

	if (tmp == NULL) return 0;

	return 1;
}

unsigned char NC_IsIn(char * strin, char * str)
{
	char * tmp;
	char t1[4096];
	char t2[4096];
	strcpy(t1, strin);
	strcpy(t2, str);
	MakeUpcase(t1);
	MakeUpcase(t2);
	tmp = strstr(t1, t2);

	if (tmp == NULL) return 0;

	return 1;
}

void File_Standardize(char * from, char * to)
{
	long pos = 0;
	long pos2 = 0;
	long size = strlen(from);
	char * temp = HermesBufferWork;	

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

	temp[pos2] = 0;

again:
	;
	size = strlen(temp);
	pos = 0;

	while (pos < size - 2)
	{
		if ((temp[pos] == '\\') && (temp[pos+1] == '.') && (temp[pos+2] == '.'))
		{
			long fnd = pos;

			while (pos > 0)
			{
				pos--;

				if (temp[pos] == '\\')
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

long KillAllDirectory(char * path)
{
	long idx;
	struct _finddata_t fl;
	char pathh[512];
	sprintf(pathh, "%s*.*", path);

	if ((idx = _findfirst(pathh, &fl)) != -1)
	{
		do
		{
			if (fl.name[0] != '.')
			{
				if (fl.attrib & _A_SUBDIR)
				{
					sprintf(pathh, "%s%s\\", path, fl.name);
					KillAllDirectory(pathh);
					RemoveDirectory(pathh);
				}
				else
				{
					sprintf(pathh, "%s%s", path, fl.name);
					DeleteFile(pathh);
				}
			}

		}
		while (_findnext(idx, &fl) != -1);

		_findclose(idx);
	}

	RemoveDirectory(path);
	return 1;
}

void GetDate(HERMES_DATE_TIME * hdt)
{
	struct tm * newtime;
 
	time_t long_time;

	time(&long_time);                  /* Get time as long integer. */
	newtime = localtime(&long_time);   /* Convert to local time. */

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
{
	strupr(str);
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

void SendConsole(char * dat, long level, long flag, HWND source)
{
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

	BOOL * ignore;
	unsigned long TotMemory = 0;
	unsigned long TOTTotMemory = 0;
	char header[128];
	char theader[128];

	if (nb_MemoTraces == 0)
	{
		text[0] = 0;
		return 0;
	}

	ignore = (BOOL *)malloc(sizeof(BOOL) * nb_MemoTraces);

	for (long i = 0; i < nb_MemoTraces; i++) ignore[i] = FALSE;

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
						ignore[j] = TRUE;
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

//******************************************************************************
// FILES MANAGEMENT
//******************************************************************************
char _drv[256];
char _dir[256];
char _name[256];
char _ext[256];

char * GetName(char * str)
{
	_splitpath(str, _drv, _dir, _name, _ext);
	return _name;
}

char * GetExt(char * str)
{
	_splitpath(str, _drv, _dir, _name, _ext);
	return _ext;
}

void SetExt(char * str, char * new_ext)
{
	_splitpath(str, _drv, _dir, _name, _ext);
	_makepath(str, _drv, _dir, _name, new_ext);
}

void AddToName(char * str, char * cat)
{
	_splitpath(str, _drv, _dir, _name, _ext);
	strcat(_name, cat);
	_makepath(str, _drv, _dir, _name, _ext);
}

void RemoveName(char * str)
{
	_splitpath(str, _drv, _dir, _name, _ext);
	_makepath(str, _drv, _dir, NULL, NULL);
}

long DirectoryExist(char * name)
{
	long idx;
	struct _finddata_t fd;

	if ((idx = _findfirst(name, &fd)) == -1)
	{
		_findclose(idx);
		char initial[256];
		_getcwd(initial, 255);

		if (_chdir(name) == 0) // success
		{
			_chdir(initial);
			return 1;
		}

		_chdir(initial);
		return 0;
	}

	_findclose(idx);
	return 1;
}

BOOL CreateFullPath(char * path)
{

	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	_splitpath(path, drive, dir, fname, ext);

	if (strlen(dir) == 0) return FALSE;

	char curpath[256];
	strcpy(curpath, drive);
	strcat(curpath, "\\");
	long start = 1;
	long pos = 1;
	long size = strlen(dir);

	while (pos < size)
	{
		if ((dir[pos] == '\\') || (dir[pos] == '/'))
		{
			dir[pos] = 0;
			memcpy(curpath + strlen(curpath), dir + start, pos - start + 1);
			strcat(curpath, "\\");
			_mkdir(curpath);
			start = pos + 1;
		}

		pos++;
	}

	if (DirectoryExist(path)) return TRUE;

	return FALSE;
}
long FileExist(char * name)
{
	long i;

	if ((i = FileOpenRead(name)) == 0) return 0;

	FileCloseRead(i);
	return 1;
}

long	FileOpenRead(char * name)
{
	long	handle;
	handle = _open((const char *)name, (int)_O_BINARY | _O_RDONLY);

	if (handle < 0)	return(0);

	return(handle + 1);
}

long	FileSizeHandle(long handle)
{
	return(_tell((int)handle - 1));
}

long	FileOpenWrite(char * name)
{
	int	handle;

	handle = _open((const char *)name, (int)_O_BINARY | _O_CREAT | _O_TRUNC, (int)_S_IWRITE);

	if (handle < 0)	return(0);

	_close(handle);
	handle = _open((const char *)name, (int)_O_BINARY | _O_WRONLY);

	if (handle < 0)	return(0);

	return(handle + 1);
}
long	FileCloseRead(long handle)
{
	return(_close((int)handle - 1));
}

long	FileCloseWrite(long handle)
{
	_commit((int)handle - 1);
	return(_close((int)handle - 1));
}

long	FileRead(long handle, void * adr, long size)
{
	return(_read(handle - 1, adr, size));
}

long	FileWrite(long handle, void * adr, long size)
{
	return(_write(handle - 1, adr, size));
}
long	FileSeek(long handle, long offset, long mode)
{
	return(_lseek((int)handle - 1, (long)offset, (int)mode));
}
void ExitApp(int v)
{
	if (MAIN_PROGRAM_HANDLE != NULL)
		SendMessage(MAIN_PROGRAM_HANDLE, WM_CLOSE, 0, 0);

	exit(v);
}

// Finishes by a 0 (for text)
void	* FileLoadMallocZero(char * name, long * SizeLoadMalloc)
{
	long	handle;
	long	size1, size2;
	unsigned char	* adr;

retry:
	;
	handle = FileOpenRead(name);

	if (!handle)
	{
		char str[256];
		strcpy(str, name);
		int mb;
		mb = ShowError("FileLoadMalloc", str, 4);

		switch (mb)
		{
			case IDABORT:
				ExitApp(1);
				break;
			case IDRETRY:
				goto  retry;
				break;
			case IDIGNORE:
				if (SizeLoadMalloc != NULL) *SizeLoadMalloc = 0;

				return(NULL);
				break;
		}
	}

	FileSeek(handle, 0, FILE_SEEK_END);
	size1 = FileSizeHandle(handle) + 2;
	adr = (unsigned char *)malloc(size1);

	if (!adr)
	{
		int mb;
		mb = ShowError("FileLoadMalloc", name, 4);

		switch (mb)
		{
			case IDABORT:
				ExitApp(1);
				break;
			case IDRETRY:
				FileCloseRead(handle);
				goto  retry;
				break;
			case IDIGNORE:
				FileCloseRead(handle);

				if (SizeLoadMalloc != NULL) *SizeLoadMalloc = 0;

				return(0);
				break;
		}
	}

	FileSeek(handle, 0, FILE_SEEK_START);
	size2 = FileRead(handle, adr, size1 - 2);
	FileCloseRead(handle);

	if (size1 != size2 + 2)
	{
		free(adr);
		int mb;
		mb = ShowError("FileLoadMalloc", name, 4);

		switch (mb)
		{
			case IDABORT:
				ExitApp(1);
				break;
			case IDRETRY:
				goto  retry;
				break;
			case IDIGNORE:
				if (SizeLoadMalloc != NULL) *SizeLoadMalloc = 0;

				return(0);
				break;
		}
	}

	if (SizeLoadMalloc != NULL) *SizeLoadMalloc = size1;

	adr[size1-1] = 0;
	adr[size1-2] = 0;
	return(adr);
}
void	* FileLoadMalloc(char * name, long * SizeLoadMalloc)
{
	long	handle;
	long	size1, size2;
	unsigned char	* adr;

retry:
	handle = FileOpenRead(name);

	if (!handle)
	{
		char str[256];
		strcpy(str, name);
		int mb;
		mb = ShowError("FileLoadMalloc", str, 4);

		switch (mb)
		{
			case IDABORT:
				ExitApp(1);
				break;
			case IDRETRY:
				goto  retry;
				break;
			case IDIGNORE:
				if (SizeLoadMalloc != NULL) *SizeLoadMalloc = 0;
				return(NULL);
				break;
		}

	}

	FileSeek(handle, 0, FILE_SEEK_END);
	size1 = FileSizeHandle(handle);
	adr = (unsigned char *)malloc(size1);

	if (!adr)
	{
		int mb;
		mb = ShowError("FileLoadMalloc", name, 4);

		switch (mb)
		{
			case IDABORT:
				ExitApp(1);
				break;
			case IDRETRY:
				FileCloseRead(handle);
				goto  retry;
				break;
			case IDIGNORE:
				FileCloseRead(handle);

				if (SizeLoadMalloc != NULL) *SizeLoadMalloc = 0;

				return(0);
				break;
		}
	}

	FileSeek(handle, 0, FILE_SEEK_START);
	size2 = FileRead(handle, adr, size1);
	FileCloseRead(handle);

	if (size1 != size2)
	{
		free(adr);
		int mb;
		mb = ShowError("FileLoadMalloc", name, 4);

		switch (mb)
		{
			case IDABORT:
				ExitApp(1);
				break;
			case IDRETRY:
				goto  retry;
				break;
			case IDIGNORE:
				if (SizeLoadMalloc != NULL) *SizeLoadMalloc = 0;
				return(0);
				break;
		}

	}

	if (SizeLoadMalloc != NULL) *SizeLoadMalloc = size1;

	return(adr);
}
//******************************************************************************
// OPEN/SAVE FILES DIALOGS
//******************************************************************************
char	LastFolder[_MAX_PATH];		// Last Folder used
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
		if (SHGetPathFromIDList(liil, LastFolder))	return TRUE;
		else return FALSE;
	}
	else return FALSE;
}


bool HERMESFolderSelector(char * file_name, char * title)
{
	if (HERMESFolderBrowse(title))
	{
		sprintf(file_name, "%s\\", LastFolder);
		return TRUE;
	}
	else
	{
		strcpy(file_name, " ");
		return FALSE;
	}
}
BOOL HERMES_WFSelectorCommon(PSTR pstrFileName, PSTR pstrTitleName, char * filter, long flag, long flag_operation, long max_car, HWND hWnd)
{
	LONG	value;
	char	cwd[_MAX_PATH];

	ofn.lStructSize		= sizeof(OPENFILENAME) ;
	ofn.hInstance			= NULL ;
	ofn.lpstrCustomFilter	= NULL ;
	ofn.nMaxCustFilter		= 0 ;
	ofn.nFilterIndex		= 0 ;
	ofn.lpstrFileTitle		= NULL ;
	ofn.nMaxFileTitle		= _MAX_FNAME + _MAX_EXT ;
	ofn.nFileOffset		= 0 ;
	ofn.nFileExtension		= 0 ;
	ofn.lpstrDefExt		= "txt" ;
	ofn.lCustData			= 0L ;
	ofn.lpfnHook			= NULL ;
	ofn.lpTemplateName		= NULL ;

	ofn.lpstrFilter			= filter ;
	ofn.hwndOwner			= hWnd;
	ofn.lpstrFile			= pstrFileName ;
	ofn.lpstrTitle			= pstrTitleName ;
	ofn.Flags				= flag;

	_getcwd(cwd, _MAX_PATH);
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

	return value;
}

int HERMESFileSelectorOpen(PSTR pstrFileName, PSTR pstrTitleName, char * filter, HWND hWnd)
{
	return HERMES_WFSelectorCommon(pstrFileName, pstrTitleName, filter, OFN_HIDEREADONLY | OFN_CREATEPROMPT, 1, _MAX_PATH, hWnd);
}
 
int HERMESFileSelectorSave(PSTR pstrFileName, PSTR pstrTitleName, char * filter, HWND hWnd)
{
	return HERMES_WFSelectorCommon(pstrFileName, pstrTitleName, filter, OFN_OVERWRITEPROMPT, 0, _MAX_PATH, hWnd);
}
 
//////////////////////////////////////// PACKING
//Always on for now...

char WorkBuff[CMP_BUFFER_SIZE];

/* Routine to read uncompressed data.  Used only by implode().
** This routine reads the data that is to be compressed.
*/

unsigned int
ReadUnCompressed(char * buff, unsigned int * size, void * Param)
{
	PARAM * Ptr = (PARAM *) Param;

	if (Ptr->UnCompressedSize == 0L)
	{
		/* This will terminate the compression or extraction process */
		return(0);
	}

	if (Ptr->UnCompressedSize < (unsigned long)*size)
	{
		*size = (unsigned int)Ptr->UnCompressedSize;
	}

	memcpy(buff, Ptr->pSource + Ptr->SourceOffset, *size);
	Ptr->SourceOffset += (unsigned long) * size;
	Ptr->UnCompressedSize -= (unsigned long) * size;

	return(*size);
}

/* Routine to read compressed data.  Used only by explode().
** This routine reads the compressed data that is to be uncompressed.
*/

unsigned int
ReadCompressed(char * buff, unsigned int * size, void * Param)
{
	PARAM * Ptr = (PARAM *) Param;

	if (Ptr->CompressedSize == 0L)
	{
		/* This will terminate the compression or extraction process */
		return(0);
	}

	if (Ptr->CompressedSize < (unsigned long)*size)
	{
		*size = (unsigned int)Ptr->CompressedSize;
	}

	memcpy(buff, Ptr->pSource + Ptr->SourceOffset, *size);
	Ptr->SourceOffset += (unsigned long) * size;
	Ptr->CompressedSize -= (unsigned long) * size;

	return(*size);
}

/* Routime to write compressed data.  Used only by implode().
** This routine writes the compressed data to a memory buffer.
*/

void
WriteCompressed(char * buff, unsigned int * size, void * Param)
{
	PARAM * Ptr = (PARAM *) Param;

	if (Ptr->CompressedSize + (unsigned long)*size > Ptr->BufferSize)
	{
		Ptr->BufferSize = Ptr->CompressedSize + (unsigned long) * size;
		Ptr->pDestination = (char *)realloc(Ptr->pDestination, Ptr->BufferSize);
	}

	if (Ptr->pDestination)
	{
		memcpy(Ptr->pDestination + Ptr->DestinationOffset, buff, *size);
		Ptr->DestinationOffset += (unsigned long) * size;
		Ptr->CompressedSize += (unsigned long) * size;
	}
}

/* Routine to write uncompressed data. Used only by explode().
** This routine writes the uncompressed data to a memory buffer.
*/

void
WriteUnCompressed(char * buff, unsigned int * size, void * Param)
{
	PARAM * Ptr = (PARAM *) Param;

	if (Ptr->UnCompressedSize + (unsigned long)*size > Ptr->BufferSize)
	{
		Ptr->BufferSize = Ptr->UnCompressedSize + ((unsigned long) * size) * 10;
		Ptr->pDestination = (char *)realloc(Ptr->pDestination, Ptr->BufferSize);
	}

	if (Ptr->pDestination)
	{
		memcpy(Ptr->pDestination + Ptr->DestinationOffset, buff, *size);
		Ptr->DestinationOffset += (unsigned long) * size;
		Ptr->UnCompressedSize += (unsigned long) * size;
	}
}

void
WriteUnCompressedNoAlloc(char * buff, unsigned int * size, void * Param)
{
	PARAM * Ptr = (PARAM *) Param;

	if (Ptr->UnCompressedSize + (unsigned long)*size > Ptr->BufferSize)
	{
		Ptr->BufferSize = Ptr->UnCompressedSize + ((unsigned long) * size) * 10;
	}

	if (Ptr->pDestination)
	{
		memcpy(Ptr->pDestination + Ptr->DestinationOffset, buff, *size);
		Ptr->DestinationOffset += (unsigned long) * size;
		Ptr->UnCompressedSize += (unsigned long) * size;
	}
}

char * STD_Implode(char * from, long from_size, long * to_size)
{
	if (WorkBuff == NULL)
	{
		return NULL;
	}

	unsigned int type  = CMP_BINARY;
	unsigned int dsize;// = 2048;

	if (from_size <= 32768)
		dsize = 1024;
	else if (from_size <= 131072)
		dsize = 2048;
	else dsize = 4096;

	PARAM Param;
	memset(&Param, 0, sizeof(PARAM));

	Param.pSource = from;
	Param.pDestination = (char *)malloc(from_size);

	bool bFirst = true; 

	while (1)
	{
		Param.CompressedSize = 0;
		Param.UnCompressedSize = from_size;
		Param.BufferSize = Param.UnCompressedSize; 
		Param.SourceOffset      = 0L;
		Param.DestinationOffset = 0L;
		Param.Crc               = (unsigned long) - 1;
		long lResult = implode(ReadUnCompressed, WriteCompressed, WorkBuff, &Param, &type, &dsize);

		if (!lResult)
		{
			break;
		}

		if (bFirst)
		{
			bFirst = false;
			continue;
		}

		lResult = MessageBox(NULL, "Failed while saving...", "Save Error", MB_RETRYCANCEL | MB_ICONERROR);

		if (lResult == IDCANCEL)
		{
			break;
		}
	}

	*to_size = Param.CompressedSize;
	return Param.pDestination;
}
char * STD_Explode(char * from, long from_size, long * to_size)
{
	if (WorkBuff == NULL)
	{
		return NULL;
	}

	PARAM Param;
	memset(&Param, 0, sizeof(PARAM));
	Param.BufferSize = 0;
	Param.pSource = from;
	Param.pDestination = NULL; 
	Param.CompressedSize = from_size;

	Param.Crc               = (unsigned long) - 1;
	unsigned int error = explode(ReadCompressed, WriteUnCompressed, WorkBuff, &Param);

	if (error)
	{
		*to_size = 0; 
		return NULL;
	}

	*to_size = Param.UnCompressedSize;
	return Param.pDestination;
}

void STD_ExplodeNoAlloc(char * from, long from_size, char * to, long * to_size)
{
	if (WorkBuff == NULL)
	{
		return;
	}

	PARAM Param;
	memset(&Param, 0, sizeof(PARAM));
	Param.BufferSize = 0;
	Param.pSource = from;
	Param.pDestination = to; 
	Param.CompressedSize = from_size;
	Param.Crc               = (unsigned long) - 1;
	unsigned int error = explode(ReadCompressed, WriteUnCompressedNoAlloc, WorkBuff, &Param);

	if (error)
	{
		*to_size = 0; 
		return;
	}

	*to_size = Param.UnCompressedSize;
}
//-------------------------------------------------------------------------------------
// SP funcs
#define hrnd()  (((FLOAT)rand() ) * 0.00003051850947599f)

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
 
