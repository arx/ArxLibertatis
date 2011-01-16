# Microsoft Developer Studio Project File - Name="EErie" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=EErie - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "EErie.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "EErie.mak" CFG="EErie - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "EErie - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "EErie - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Gaia", KVAAAAAA"
# PROP Scc_LocalPath "\\arkaneserver\msdn98\gaiass\gaia"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "EErie - Win32 Release"

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
# ADD CPP /nologo /Zp1 /MT /W3 /GX /Zi /O2 /Oy- /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FAs /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\EErie_release.lib"

!ELSEIF  "$(CFG)" == "EErie - Win32 Debug"

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
# ADD CPP /nologo /Zp1 /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\EErie_debug.lib"

!ENDIF 

# Begin Target

# Name "EErie - Win32 Release"
# Name "EErie - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\EERIE_AVI.cpp
# End Source File
# Begin Source File

SOURCE=.\EERIEAnchors.cpp
# End Source File
# Begin Source File

SOURCE=.\EERIEAnim.cpp
# End Source File
# Begin Source File

SOURCE=.\EERIEapp.cpp
# End Source File
# Begin Source File

SOURCE=.\EERIEClothes.cpp
# End Source File
# Begin Source File

SOURCE=.\EERIECollisionSpheres.cpp
# End Source File
# Begin Source File

SOURCE=.\EERIEDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\EERIEenum.cpp
# End Source File
# Begin Source File

SOURCE=.\EERIEFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\EERIELight.cpp
# End Source File
# Begin Source File

SOURCE=.\EERIELinkedObj.cpp
# End Source File
# Begin Source File

SOURCE=.\EERIEmath.cpp
# End Source File
# Begin Source File

SOURCE=.\EERIEMeshTweak.cpp
# End Source File
# Begin Source File

SOURCE=.\EERIEobject.cpp
# End Source File
# Begin Source File

SOURCE=.\EERIEPathfinder.cpp
# End Source File
# Begin Source File

SOURCE=.\EERIEPhysicsBox.cpp
# End Source File
# Begin Source File

SOURCE=.\EERIEPoly.cpp
# End Source File
# Begin Source File

SOURCE=.\EERIEProgressive.cpp
# End Source File
# Begin Source File

SOURCE=.\EERIETexture.cpp
# End Source File
# Begin Source File

SOURCE=.\EERIEutil.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\Include\EERIE_AVI.h
# End Source File
# Begin Source File

SOURCE=..\Include\EERIEAnchors.h
# End Source File
# Begin Source File

SOURCE=..\Include\EERIEAnim.h
# End Source File
# Begin Source File

SOURCE=..\Include\EERIEapp.h
# End Source File
# Begin Source File

SOURCE=..\Include\EERIEClothes.h
# End Source File
# Begin Source File

SOURCE=..\Include\EERIECollisionSpheres.h
# End Source File
# Begin Source File

SOURCE=..\Include\EERIEDraw.h
# End Source File
# Begin Source File

SOURCE=..\Include\EERIEenum.h
# End Source File
# Begin Source File

SOURCE=..\Include\EERIEframe.h
# End Source File
# Begin Source File

SOURCE=..\Include\EERIEJpeg.h
# End Source File
# Begin Source File

SOURCE=..\Include\EERIELight.h
# End Source File
# Begin Source File

SOURCE=..\Include\EERIELinkedObj.h
# End Source File
# Begin Source File

SOURCE=..\Include\EERIEmath.h
# End Source File
# Begin Source File

SOURCE=..\Include\EERIEMeshTweak.h
# End Source File
# Begin Source File

SOURCE=..\Include\EERIEobject.h
# End Source File
# Begin Source File

SOURCE=..\Include\EERIEPathfinder.h
# End Source File
# Begin Source File

SOURCE=..\Include\EERIEPhysicsBox.h
# End Source File
# Begin Source File

SOURCE=..\Include\EERIEPoly.h
# End Source File
# Begin Source File

SOURCE=..\Include\EERIEProgressive.h
# End Source File
# Begin Source File

SOURCE=..\Include\EERIEres.h
# End Source File
# Begin Source File

SOURCE=..\Include\EERIETexture.h
# End Source File
# Begin Source File

SOURCE=..\Include\EERIETypes.h
# End Source File
# Begin Source File

SOURCE=..\Include\EERIEutil.h
# End Source File
# Begin Source File

SOURCE=..\Include\OtherLibs\Jconfig.h
# End Source File
# Begin Source File

SOURCE=..\Include\OtherLibs\Jerror.h
# End Source File
# Begin Source File

SOURCE=..\Include\OtherLibs\Jmorecfg.h
# End Source File
# Begin Source File

SOURCE=..\Include\OtherLibs\Jpeglib.h
# End Source File
# Begin Source File

SOURCE=..\Include\TheoData.h
# End Source File
# Begin Source File

SOURCE=..\Include\OtherLibs\zconf.h
# End Source File
# Begin Source File

SOURCE=..\Include\OtherLibs\zlib.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\lib\OtherLibs\zlib.lib
# End Source File
# Begin Source File

SOURCE=..\lib\OtherLibs\Jpeglib.lib
# End Source File
# End Target
# End Project
