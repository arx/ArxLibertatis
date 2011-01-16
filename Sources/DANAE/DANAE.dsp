# Microsoft Developer Studio Project File - Name="DANAE" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=DANAE - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DANAE.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DANAE.mak" CFG="DANAE - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DANAE - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "DANAE - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Gaia", KVAAAAAA"
# PROP Scc_LocalPath "\\arkaneserver\msdn98\gaiass\gaia"
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DANAE - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /Zp1 /MT /W3 /GX /Zi /O2 /Ob2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /FAs /FD /D /QaxW /NODEFAULTLIB:LIBC /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 implode.lib Mercury_Release.lib Ceditor_Release.lib Minos_release.lib HERMES_release.LIB EERIE_release.LIB eaxguid.lib dinput.lib zlib.lib jpeglib.lib wsock32.lib dxguid.lib dsound.lib winmm.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib ddraw.lib Advapi32.lib shell32.lib ole32.lib /nologo /subsystem:windows /incremental:yes /map /debug /machine:I386

!ELSEIF  "$(CFG)" == "DANAE - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /Zp1 /MTd /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "NODEFAULTLIB:" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 implode.lib Mercury_Debug.lib Ceditor_Debug.lib Minos_Debug.lib HERMES_debug.LIB EERIE_debug.LIB eaxguid.lib dinput.lib zlib.lib jpeglib.lib wsock32.lib dxguid.lib dsound.lib winmm.lib comctl32.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib ddraw.lib Advapi32.lib shell32.lib ole32.lib /nologo /subsystem:windows /map /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "DANAE - Win32 Release"
# Name "DANAE - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ARX_C_cinematique.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_C_fx.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_C_keyframer.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_C_loadsave.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_C_mapp.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_C_sound.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_carte.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Cedric.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_ChangeLevel.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Collisions.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Damages.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Draw.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Equipment.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Fogs.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_FTL.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_GlobalMods.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_HWTransform.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Input.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Interactive.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Interface.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Intro.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Inventory.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Levels.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Menu.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Menu2.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_MenuPublic.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Minimap.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Missile.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Network.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_NPC.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Particles.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Paths.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Physics.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Player.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Scene.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Script.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Snapshot.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Sound.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Special.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Speech.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Text.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Time.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_ViewImage.cpp
# End Source File
# Begin Source File

SOURCE=.\Danae.cpp
# End Source File
# Begin Source File

SOURCE=.\DANAE.rc
# End Source File
# Begin Source File

SOURCE=.\DANAE_Debugger.cpp
# End Source File
# Begin Source File

SOURCE=.\DanaeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\DanaeSaveLoad.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\Include\ARX_C_cinematique.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_carte.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_Cedric.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_ChangeLevel.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_Collisions.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_Damages.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_Draw.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_Equipment.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_Fogs.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_FTL.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_GlobalMods.h
# End Source File
# Begin Source File

SOURCE=.\ARX_HWTransform.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_Input.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_Interactive.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_Interface.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_Levels.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_Menu.h
# End Source File
# Begin Source File

SOURCE=.\ARX_Menu2.h
# End Source File
# Begin Source File

SOURCE=.\ARX_MenuPublic.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_Minimap.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_Missile.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_Network.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_NPC.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_Particles.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_Paths.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_Physics.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_Player.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_Scene.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_Script.h
# End Source File
# Begin Source File

SOURCE=.\ARX_SnapShot.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_Sound.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_Special.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_Speech.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_Text.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_Time.h
# End Source File
# Begin Source File

SOURCE=.\ARX_ViewImage.h
# End Source File
# Begin Source File

SOURCE=..\Include\Danae.h
# End Source File
# Begin Source File

SOURCE=..\Include\DANAE_Debugger.h
# End Source File
# Begin Source File

SOURCE=.\Danae_resource.h
# End Source File
# Begin Source File

SOURCE=..\Include\DANAE_VERSION.h
# End Source File
# Begin Source File

SOURCE=..\Include\DanaeDlg.h
# End Source File
# Begin Source File

SOURCE=..\Include\DanaeSaveLoad.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\arkane_logo.bmp
# End Source File
# Begin Source File

SOURCE=.\ARX.bmp
# End Source File
# Begin Source File

SOURCE=.\Arx_icon_64X64.ico
# End Source File
# Begin Source File

SOURCE=.\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmap2.bmp
# End Source File
# Begin Source File

SOURCE=.\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\Danae.bmp
# End Source File
# Begin Source File

SOURCE=.\Danae.ico
# End Source File
# Begin Source File

SOURCE=.\Danae_TB.bmp
# End Source File
# Begin Source File

SOURCE=.\directx.ico
# End Source File
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\icon2.ico
# End Source File
# Begin Source File

SOURCE=.\pass_logo.bmp
# End Source File
# Begin Source File

SOURCE=.\Presentation.bmp
# End Source File
# Begin Source File

SOURCE=.\toolbar1.bmp
# End Source File
# Begin Source File

SOURCE=.\winmain.rc
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "Spells"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ARX_CSpellFx.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_CSpellFx.h
# End Source File
# Begin Source File

SOURCE=.\ARX_SpellFx.h
# End Source File
# Begin Source File

SOURCE=.\ARX_SpellFX_Lvl01.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_SpellFx_Lvl01.h
# End Source File
# Begin Source File

SOURCE=.\ARX_SpellFX_Lvl02.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_SpellFx_Lvl02.h
# End Source File
# Begin Source File

SOURCE=.\ARX_SpellFX_Lvl03.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_SpellFx_Lvl03.h
# End Source File
# Begin Source File

SOURCE=.\ARX_SpellFX_Lvl04.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_SpellFx_Lvl04.h
# End Source File
# Begin Source File

SOURCE=.\ARX_SpellFX_Lvl05.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_SpellFx_Lvl05.h
# End Source File
# Begin Source File

SOURCE=.\ARX_SpellFX_Lvl06.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_SpellFx_Lvl06.h
# End Source File
# Begin Source File

SOURCE=.\ARX_SpellFX_Lvl07.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_SpellFx_Lvl07.h
# End Source File
# Begin Source File

SOURCE=.\ARX_SpellFX_Lvl08.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_SpellFx_Lvl08.h
# End Source File
# Begin Source File

SOURCE=.\ARX_SpellFX_Lvl09.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_SpellFx_Lvl09.h
# End Source File
# Begin Source File

SOURCE=.\ARX_SpellFX_Lvl10.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_SpellFx_Lvl10.h
# End Source File
# Begin Source File

SOURCE=.\ARX_Spells.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_Spells.h
# End Source File
# End Group
# Begin Group "Particles"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ARX_CParticle.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_CParticle.h
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_CParticleParams.h
# End Source File
# Begin Source File

SOURCE=.\ARX_CParticles.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_CParticles.h
# End Source File
# Begin Source File

SOURCE=.\ARX_CParticleSystem.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\ARX_CParticleSystem.h
# End Source File
# End Group
# Begin Group "Localisation"

# PROP Default_Filter ""
# Begin Group "Config popup"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Arx_Config.cpp
# End Source File
# Begin Source File

SOURCE=.\Arx_Config.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ARX_Loc.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_Loc.h
# End Source File
# Begin Source File

SOURCE=.\ARX_LocHash.cpp
# End Source File
# Begin Source File

SOURCE=.\ARX_LocHash.h
# End Source File
# End Group
# End Target
# End Project
