
#ifndef ARX_PLATFORM_ENVIRONMENT_H
#define ARX_PLATFORM_ENVIRONMENT_H

#include <string>

#include "platform/Platform.h"

std::string expandEvironmentVariables(const std::string & in);

bool getSystemConfiguration(const std::string & name, std::string & result);

#if ARX_PLATFORM != ARX_PLATFORM_WIN32
void defineXdgDirectories();
#else
#define defineXdgDirectories()
#endif

#endif // ARX_PLATFORM_ENVIRONMENT_H
