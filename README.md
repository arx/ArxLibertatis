# Arx Fatalis Port

(Arx Libertatis?)
Port to x64_86 / GCC / OpenGL / Unix

## Dependencies

Currently wine, DevIL (libil.so), zlib (libz.so). All of these must be available as 32bit libraries.
Wine and 32bit is only a temporary solution.

## Compile

`$ cmake .`

`$ make`

## Run

Rename all .pak files in the Arx Fatalis installation to lowercase and the run from the directory containing the .pak files.

`$ WINEDEBUG=-all ./arx.exe`

You can close it with `Alt + F4` or `killall arx.exe.so`

## Debugging

Because ARX is compiled as a winelib application, normal gdb won't work. Instead use winedbg:

`$ WINEDEBUG=-all winedbg ./arx.exe.so`

## Build documentation

Run this form the root directory:

`$ make doc`

## Chat

\#arxfatalis on irc.freenode.net

## Wiki

http://arx.parpg.net/

## Reddit

http://www.reddit.com/r/ArxFatalis/
