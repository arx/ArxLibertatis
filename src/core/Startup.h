
#ifndef ARX_CORE_STARTUP_H
#define ARX_CORE_STARTUP_H

#include "platform/Platform.h"

/*!
 * Parse the command line.
 * @return true if the program should exit immediately.
 */
#if ARX_PLATFORM != ARX_PLATFORM_WIN32
void parseCommandLine(int argc, const char * const * argv);
#else
void parseCommandLine(const char * command_line);
#endif

#endif // ARX_CORE_STARTUP_H
