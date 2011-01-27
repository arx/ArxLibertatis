
  ==============================
| Arx Fatalis GPL source release |
  ==============================

Source code released on January the 14 of 2011.


This file contains the following sections:

LICENSE
GENERAL NOTES
COMPILING ON WIN32


_______

LICENSE
_______


See ARX_PUBLIC_LICENSE.txt for the GNU GENERAL PUBLIC LICENSE

ADDITIONAL TERMS:  The Arx Fatalis GPL Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU GPL which accompanied the Arx Fatalis Source Code.  If not, please request a copy in writing from Arkane Studios at Arkane Studios, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

EXCLUDED CODE:  The code described below and contained in the Arx Fatalis GPL Source Code release is not part of the Program covered by the GPL and is expressly excluded from its terms.  You are solely responsible for obtaining from the copyright holder a license for such code and complying with the applicable license terms.


PKZIP
-----
IMPLODE.LIB - PKWARE Data Compression Library (R) for Win32
Copyright 1991,1992,1994,1995 PKWARE Inc.  All Rights Reserved.
PKWARE Data Compression Library Reg. U.S. Pat. and Tm. Off.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files, to deal in the Software without restriction, including without limitation the rights to use, copy, modify and distribute the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL PKWARE Inc. BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


EAX 2.0 & EAX-AC3
-----------------
EAX.H - DirectSound3D Environmental Audio Extensions version 2.0
Copyright (c) 1999 Creative Technology Ltd. All rights reserved. 

Permission to use, copy, modify, and distribute this software and its documentation for any purpose and without fee is hereby granted, provided that the above copyright notice appear in all copies and that both that copyright notice and this permission notice appear in supporting documentation, and that the name of Creative Technology Ltd. is not be used in advertising or publicity pertaining to distribution of the software without specific, written prior permission.


JPEG library
------------
jpeglib.h
Copyright (C) 1991-1998, Thomas G. Lane.
This file is part of the Independent JPEG Group (IJG)'s software.

Permission is hereby granted to use, copy, modify, and distribute this software (or portions thereof) for any purpose, without fee, subject to these conditions:
1. If any part of the source code for this software is distributed, then this README file must be included, with this copyright and no-warranty notice unaltered; and any additions, deletions, or changes to the original files must be clearly indicated in accompanying documentation.
2. If only executable code is distributed, then the accompanying documentation must state that "this software is based in part on the work of the Independent JPEG Group".
3. Permission for use of this software is granted only if the user accepts full responsibility for any undesirable consequences; the authors accept NO LIABILITY for damages of any kind.

These conditions apply to any software derived from or based on the IJG code, not just to the unmodified library. If you use our work, you ought to acknowledge us.


Zlib
----
zlib.h -- interface of the 'zlib' general purpose compression library version 1.1.3, July 9th, 1998
Copyright (C) 1995-1998 Jean-loup Gailly and Mark Adler

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.


_____________

GENERAL NOTES
_____________

Short summary of the file layout in src/ folder:
Athena/        source code (.cpp); also includes sound-related files (.h)
DANAE/         Arx Engine source code (renderer, game code, etc.)
EERIE/         math-related source code, geometry utilities, mesh animation and texturing
HERMES/        HDD I/O management (load/save functions)
Mercury/       DirectX devices management (D3D, inputs)
HERMES/        Arx pathfinding
DX/            DirectX 7 include files (Include/) and lib (Lib/)
Include/       gathers all Arx include files (except Athena's) and misc libs described hereabove (OtherLibs/)
lib/           contains libs for Arx and utilities

While we made sure we were still able to compile the game on Windows, this build didn't get any kind of extensive testing so it may not work properly on your computer. After Arkane Studios released the game under GPL, it is very likely that several projects will start over, trying to make the source code more friendly to new compilers and environments. If you are picking up this release weeks/months/years after we uploaded it, you probably want to look around on the net for updated versions of this codebase as well.


__________________

COMPILING ON WIN32
__________________


Prerequisites
-------------
You need a commercial installation of Arx to use these source files. Output binaries files (.exe, .dll) should target the game/ folder. Arx.exe will not launch unless you previously copied all Arx Fatalis content from install folder into the game/ folder. 

An alternative solution is to set output files in Arx Fatalis install folder. But be careful, overriding Arx files may corrupt your game installation.

Compiling :
-----------
VC9 / Visual Studio 2008 solution file is provided:
src/Gaia.sln

VC9 / Visual Studio 2008 project files referenced are:
PROJECT FILES PATH          # Quick Comment
============================#===============================
src/Athena/Athena.vcproj    # sound-related
src/DANAE/DANAE.vcproj      # game engine stuff
src/EERIE/EErie.vcproj      # math/geometry-related
src/HERMES/HERMES.vcproj    # data loading/saving stuff
src/Mercury/Mercury.vcproj  # DirectX devices
src/MINOS/MINOS.vcproj      # Pathfinding

To compile Arx, use VC9 default options.
In case of trouble, make sure you do not have forgot to include a directory in the src folder.

You can lauch Arx in debug mode using windows assert (take a look at ARX_Common.h for more details).
If you launch Arx directly from VC8, make sure to set DANAE as your Start-Up project with:
Debugging Command               : $(TargetPath) = game/arx.exe
Debugging Working Directory     : game/ 

