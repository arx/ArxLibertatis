proj=Eerieframe

INCLUDE=$(INCLUDE);$(DXSDKROOT)\samples\multimedia\d3dim\include;

!if "$(nodebug)" == "1"
OBJ_DIR = Retail
!else
OBJ_DIR = Debug
!endif

!include "$(DXSDKROOT)\include\dxsdk.inc"

mycflags=

OBJ_LIST = $(OBJ_DIR)\$(proj).obj $(OBJ_DIR)\eerieenum.obj $(OBJ_DIR)\eeriemath.obj $(OBJ_DIR)\eerietextr.obj $(OBJ_DIR)\eerieutil.obj $(OBJ_DIR)\eerieapp.obj $(OBJ_DIR)\eeriefile.obj

all: mkdir $(OBJ_DIR)\$(proj).lib copyfile

$(OBJ_DIR)\$(proj).obj:  $(proj).cpp
    $(cc) $(cdebug) $(cflags) $(mycflags) $(outobj)$(OBJ_DIR)\$(proj).obj $(proj).cpp

$(OBJ_DIR)\eerieenum.obj:  eerieenum.cpp
    $(cc) $(cdebug) $(cflags) $(mycflags) $(outobj)$(OBJ_DIR)\eerieenum.obj eerieenum.cpp

$(OBJ_DIR)\eeriefile.obj:  eeriefile.cpp
    $(cc) $(cdebug) $(cflags) $(mycflags) $(outobj)$(OBJ_DIR)\eeriefile.obj eeriefile.cpp

$(OBJ_DIR)\eeriemath.obj:  eeriemath.cpp
    $(cc) $(cdebug) $(cflags) $(mycflags) $(outobj)$(OBJ_DIR)\eeriemath.obj eeriemath.cpp

$(OBJ_DIR)\eerietextr.obj:  eerietextr.cpp
    $(cc) $(cdebug) $(cflags) $(mycflags) $(outobj)$(OBJ_DIR)\eerietextr.obj eerietextr.cpp

$(OBJ_DIR)\eerieutil.obj:  eerieutil.cpp
    $(cc) $(cdebug) $(cflags) $(mycflags) $(outobj)$(OBJ_DIR)\eerieutil.obj eerieutil.cpp

$(OBJ_DIR)\eerieapp.obj:  eerieapp.cpp
    $(cc) $(cdebug) $(cflags) $(mycflags) $(outobj)$(OBJ_DIR)\eerieapp.obj eerieapp.cpp

$(OBJ_DIR)\$(proj).lib: $(OBJ_LIST)
    $(bldlib) $(libflags) $(borlib)$(OBJ_DIR)\$(proj).obj $(borlib)$(OBJ_DIR)\eerieenum.obj $(borlib)$(OBJ_DIR)\eeriemath.obj $(borlib)$(OBJ_DIR)\eerietextr.obj $(borlib)$(OBJ_DIR)\eerieutil.obj $(borlib)$(OBJ_DIR)\eerieapp.obj $(borlib)$(OBJ_DIR)\eeriefile.obj

!IFDEF BORLAND
copyfile:
    copy /Y $(OBJ_DIR)\$(proj).lib $(OBJ_DIR)\..\..\..\lib\Borland

mkdir:
    @if not exist $(OBJ_DIR)\NUL md $(OBJ_DIR)
    @if not exist $(OBJ_DIR)\..\..\..\lib md $(OBJ_DIR)\..\..\..\lib
    @if not exist $(OBJ_DIR)\..\..\..\lib\Borland md $(OBJ_DIR)\..\..\..\lib\Borland
!ELSE
!IFDEF WATCOM
copyfile:
    copy /Y $(OBJ_DIR)\$(proj).lib $(OBJ_DIR)\..\..\..\lib\Watcom

mkdir:
    @if not exist $(OBJ_DIR)\NUL md $(OBJ_DIR)
    @if not exist $(OBJ_DIR)\..\..\..\lib md $(OBJ_DIR)\..\..\..\lib
    @if not exist $(OBJ_DIR)\..\..\..\lib\Watcom md $(OBJ_DIR)\..\..\..\lib\Watcom
!ELSE
copyfile:
    copy /Y $(OBJ_DIR)\$(proj).lib $(OBJ_DIR)\..\..\..\lib\

mkdir:
    @if not exist $(OBJ_DIR)\NUL md $(OBJ_DIR)
    @if not exist $(OBJ_DIR)\..\..\..\lib md $(OBJ_DIR)\..\..\..\lib
!ENDIF
!ENDIF

# Rules for cleaning out those old files
clean:
    -@echo y | del Retail
    -@echo y | del Debug
    -@rd Retail
    -@rd Debug
