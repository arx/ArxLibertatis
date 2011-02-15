# Arx Fatalis Port

(Arx Libertis?)
Port to GCC / OpenGL / Unix

## Dependencies

Currently wine and libjpeg, and maybe some other headers like zlib.h.
Wine is only a temporary solution.

## Compile

`$ cmake .`

`$ make`

## Run

Rename all .pak files in the Arx Fatalis installation to lowercase and the run from the directory containing the .pak files (replacing "path/to/" with the path to the executable): 
`$ WINEDEBUG=-all wine path/to/arx.exe.so`

## Chat

\#arxfatalis on irc.freenode.net

## Reddit

http://www.reddit.com/r/ArxFatalis/
