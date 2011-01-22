# Arx Fatalis Port

(Arx Libertatis?)
Port to x64_86 / GCC / OpenGL / Unix

## Dependencies

Currently wine and libjpeg8 32bit, and maybe some other headers like zlib.h.
Wine and 32bit is only a temporary solution.

## Compile

`$ cmake .`

`$ make`

## Ubuntu / Debian (and other distros with default libjpeg version 6)

If you get following build error:

/usr/include/jmorecfg.h:161: error: conflicting declaration ‘typedef long int INT32’

`$ sudo aptitude install libjpeg8-dev`

For 64bit:

Some hints from cybersphinx on getting the 32bit libjpeg8 library (for linking)

(Ubuntu has only libjpeg6 in it's 32bit libs package)

http://paste.pocoo.org/show/323329/ 

I will make the Ubuntu build readme section more straight forward, next time I build it in Ubuntu.

## Run

Rename all .pak files in the Arx Fatalis installation to lowercase and the run from the directory containing the .pak files.

`$ WINEDEBUG=-all ./arx.exe`

## Debugging

Because ARX is compiled as a winelib application, normal gdb won't work. Instead use winedbg:

`$ WINEDEBUG=-all winedbg ./arx.exe.so`

## Chat

\#arxfatalis on irc.freenode.net

## Reddit

http://www.reddit.com/r/ArxFatalis/
