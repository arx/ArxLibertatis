
#ifndef LINUX_PORT_H
#define LINUX_PORT_H
#define LINUX_PORT

#ifdef linux

#include <ctype.h>
#include <string.h>

#define TRUE true
#define FALSE false
typedef long DWORD;
typedef void VOID;
typedef int HWND;
typedef char * PSTR;
typedef bool HRESULT;
typedef char * HKEY;
typedef char TCHAR;
typedef long LONG;
typedef int INT;
typedef int WORD;
typedef char * GUID;
typedef float FLOAT;

#define _MAX_PATH PATH_MAX

inline int stricmp(char *str1, char *str2)
{
	return strcasecmp(str1, str2);
}

inline void strupr(char *str)
{
	int i = 0;
	while(str[i] != '\0') {
		str[i] = toupper(str[i]);
	}
}

#endif
#endif
