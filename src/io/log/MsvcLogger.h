
#ifndef ARX_IO_LOG_MSVCLOGGER_H
#define ARX_IO_LOG_MSVCLOGGER_H

#include "io/log/LogBackend.h"

#include "Configure.h"

#ifdef HAVE_WINAPI

namespace logger {

/*!
 * Logger that uses the windows OutputDebugString() function.
 */
class MsvcDebugger : public Backend {
	
public:
	
	~MsvcDebugger();
	
	void log(const Source & file, int line, Logger::LogLevel level, const std::string & str);
	
	void flush();
	
	/*!
	 * Returns a MsvcDebugger instance if a debugger is attached or NULL otherwise.
	 */
	static MsvcDebugger * get();
	
};

} // namespace logger

#endif // HAVE_WINAPI

#endif // ARX_IO_LOG_MSVCLOGGER_H
