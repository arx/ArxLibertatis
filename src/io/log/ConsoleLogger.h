
#ifndef ARX_IO_LOG_CONSOLELOGGER_H
#define ARX_IO_LOG_CONSOLELOGGER_H

#include "io/log/LogBackend.h"

namespace logger {

/*!
 * Simple logger that prints plain text to standard output.
 */
class Console : public Backend {
	
public:
	
	~Console();
	
	void log(const Source & file, int line, Logger::LogLevel level, const std::string & str);
	
	void flush();
	
	/*!
	 * @return a ColorLogger instance if colors are supported or a ConsoleLogger otherwise.
	 *         May return null if standard output / error is discarded.
	 */
	static Backend * get();
	
};

} // namespace logger

#endif // ARX_IO_LOG_CONSOLELOGGER_H
