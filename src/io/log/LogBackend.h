
#ifndef ARX_IO_LOG_LOGBACKEND_H
#define ARX_IO_LOG_LOGBACKEND_H

#include <ostream>

#include "io/log/Logger.h"

namespace logger {

struct Source {
	
	const char * file;
	
	std::string name;
	
	Logger::LogLevel level;
	
};

class Backend {
	
public:
	
	virtual ~Backend() { }
	
	virtual void log(const Source & file, int line, Logger::LogLevel level, const std::string & str) = 0;
	
	virtual void flush() = 0;
	
	/*!
	 * Format a log entry for a text-based logging backend.
	 */
	void format(std::ostream & os, const Source & file, int line, Logger::LogLevel level, const std::string & str);
	
};

} // namespace logger

#endif // ARX_IO_LOG_LOGBACKEND_H
