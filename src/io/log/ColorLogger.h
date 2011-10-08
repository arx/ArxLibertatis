
#ifndef ARX_IO_LOG_COLORLOGGER_H
#define ARX_IO_LOG_COLORLOGGER_H

#include "io/log/LogBackend.h"

namespace logger {

/*!
 * ConsoleLogger that colors output message using shell color codes.
 */
class ColorConsole : public Backend {
	
public:
	
	~ColorConsole();
	
	void log(const Source & file, int line, Logger::LogLevel level, const std::string & str);
	
	void flush();
	
};

} // namespace logger

#endif // ARX_IO_LOG_COLORLOGGER_H
