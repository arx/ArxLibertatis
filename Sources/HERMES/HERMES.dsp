# Microsoft Developer Studio Project File - Name="HERMES" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=HERMES - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "HERMES.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "HERMES.mak" CFG="HERMES - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "HERMES - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "HERMES - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Gaia", KVAAAAAA"
# PROP Scc_LocalPath "\\arkaneserver\msdn98\gaiass\gaia"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "HERMES - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /Zp1 /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\HERMES_release.lib"

!ELSEIF  "$(CFG)" == "HERMES - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /Zp1 /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\HERMES_debug.lib"

!ENDIF 

# Begin Target

# Name "HERMES - Win32 Release"
# Name "HERMES - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\HERMES_ClusterSave.cpp
# End Source File
# Begin Source File

SOURCE=.\HERMES_hachage.cpp
# End Source File
# Begin Source File

SOURCE=.\HERMES_pack.cpp
# End Source File
# Begin Source File

SOURCE=.\HERMES_pack_public.cpp
# End Source File
# Begin Source File

SOURCE=.\HERMES_PAK.cpp
# End Source File
# Begin Source File

SOURCE=.\HERMESconsole.cpp
# End Source File
# Begin Source File

SOURCE=.\HERMESDDE.cpp
# End Source File
# Begin Source File

SOURCE=.\HERMESmain.cpp
# End Source File
# Begin Source File

SOURCE=.\HERMESnet.cpp
# End Source File
# Begin Source File

SOURCE=.\HERMESPerf.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\Include\HERMES_ClusterSave.h
# End Source File
# Begin Source File

SOURCE=..\Include\HERMES_hachage.h
# End Source File
# Begin Source File

SOURCE=..\Include\HERMES_pack.h
# End Source File
# Begin Source File

SOURCE=..\Include\HERMES_pack_public.h
# End Source File
# Begin Source File

SOURCE=..\Include\HERMES_pack_types.h
# End Source File
# Begin Source File

SOURCE=..\Include\HERMES_PAK.h
# End Source File
# Begin Source File

SOURCE=..\Include\HERMESconsole.h
# End Source File
# Begin Source File

SOURCE=..\Include\HERMESDDE.h
# End Source File
# Begin Source File

SOURCE=..\Include\HERMESmain.h
# End Source File
# Begin Source File

SOURCE=..\Include\HERMESnet.h
# End Source File
# Begin Source File

SOURCE=..\Include\HERMESPerf.h
# End Source File
# Begin Source File

SOURCE=..\Include\OtherLibs\IMPLODE.H
# End Source File
# Begin Source File

SOURCE=..\Include\resource.h
# End Source File
# Begin Source File

SOURCE=..\Include\ResourceHERMESnet.h
# End Source File
# End Group
# End Target
# End Project
