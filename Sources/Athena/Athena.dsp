# Microsoft Developer Studio Project File - Name="Athena" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Athena - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Athena.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Athena.mak" CFG="Athena - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Athena - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Athena - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ARX/SOURCESS/Athena", UEEAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Athena - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ATHENA_EXPORTS" /YX /FD /c
# ADD CPP /nologo /Zp1 /MT /W4 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ATHENA_EXPORTS" /D "AAL_APIDLL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 hermes_release.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"../Danae/Release/Athena.dll" /implib:"../lib/Athena.lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Athena - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ATHENA_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /Zp1 /MTd /W4 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ATHENA_EXPORTS" /D "AAL_APIDLL" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 hermes_debug.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../Danae/Debug/Athena.dll" /implib:"../lib/Athena.lib" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "Athena - Win32 Release"
# Name "Athena - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Codec Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Athena_CoDec_ADPCM.cpp
# End Source File
# Begin Source File

SOURCE=.\Athena_Codec_RAW.cpp
# End Source File
# End Group
# Begin Group "Stream Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Athena_Stream_ASF.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Athena_Stream_WAV.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Athena.cpp
# End Source File
# Begin Source File

SOURCE=.\Athena_Ambiance.cpp
# End Source File
# Begin Source File

SOURCE=.\Athena_DLL.cpp
# End Source File
# Begin Source File

SOURCE=.\Athena_Environment.cpp
# End Source File
# Begin Source File

SOURCE=.\Athena_FileIO.cpp
# End Source File
# Begin Source File

SOURCE=.\Athena_Global.cpp
# End Source File
# Begin Source File

SOURCE=.\Athena_Instance.cpp
# End Source File
# Begin Source File

SOURCE=.\Athena_Mixer.cpp
# End Source File
# Begin Source File

SOURCE=.\Athena_Resource.cpp
# End Source File
# Begin Source File

SOURCE=.\Athena_Sample.cpp
# End Source File
# Begin Source File

SOURCE=.\Athena_Stream.cpp
# End Source File
# Begin Source File

SOURCE=.\Athena_Track.cpp
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Codec Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Athena_Codec_ADPCM.h
# End Source File
# Begin Source File

SOURCE=.\Athena_Codec_RAW.h
# End Source File
# End Group
# Begin Group "Stream Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Athena_Stream_ASF.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Athena_Stream_WAV.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Athena_Ambiance.h
# End Source File
# Begin Source File

SOURCE=.\Athena_Codec.h
# End Source File
# Begin Source File

SOURCE=.\Athena_Environment.h
# End Source File
# Begin Source File

SOURCE=.\Athena_FileIO.h
# End Source File
# Begin Source File

SOURCE=.\Athena_Global.h
# End Source File
# Begin Source File

SOURCE=.\Athena_Instance.h
# End Source File
# Begin Source File

SOURCE=.\Athena_Mixer.h
# End Source File
# Begin Source File

SOURCE=.\Athena_Resource.h
# End Source File
# Begin Source File

SOURCE=.\Athena_Sample.h
# End Source File
# Begin Source File

SOURCE=.\Athena_Stream.h
# End Source File
# Begin Source File

SOURCE=.\Athena_Track.h
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Source File

SOURCE=..\Include\Athena.h
# End Source File
# Begin Source File

SOURCE=..\Include\Athena_Types.h
# End Source File
# End Target
# End Project
