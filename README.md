# Arx Fatalis Port

(Arx Libertatis?)
Port to x64_86 / GCC / OpenGL / Unix

## Dependencies

Currently wine and libjpeg8 32bit, and maybe some other headers like zlib.h.
Wine and 32bit is only a temporary solution.

## Compile

`$ cmake .`

`$ make`

## Run

Rename all .pak files in the Arx Fatalis installation to lowercase and the run from the directory containing the .pak files.

`$ WINEDEBUG=-all ./arx.exe`

## Debuggung

Because ARX is compiled as a winelib application, normal gdb won't work. Instead use winedbg:

`$ WINEDEBUG=-all winedbg ./arx.exe.so`

## Chat

\#arxfatalis on irc.freenode.net

## Reddit

http://www.reddit.com/r/ArxFatalis/
