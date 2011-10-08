
#ifndef ARX_IO_LOG_LOGGER_H
#define ARX_IO_LOG_LOGGER_H

#include <sstream>
#include <string>

#include "platform/Platform.h"

#ifdef _DEBUG
//! Log a Debug message. Arguments are only evaluated if their results will be used.
#define LogDebug(...)    \
	if(const ::logger::Source * logger_source = ::Logger::get(__FILE__, ::Logger::Debug)) \
		::Logger(logger_source, __LINE__, ::Logger::Debug) << __VA_ARGS__
#else
#define LogDebug(...)    ARX_DISCARD(__VA_ARGS__)
#endif

//! Log an Info message. Arguments are always evaluated.
#define LogInfo    ::Logger(__FILE__, __LINE__, ::Logger::Info)

//! Log a Warning message. Arguments are always evaluated.
#define LogWarning ::Logger(__FILE__, __LINE__, ::Logger::Warning)

//! Log an Error message. Arguments are always evaluated.
#define LogError   ::Logger(__FILE__, __LINE__, ::Logger::Error)

//! Test if the Debug log level is enabled for the current file.
#define LogDebugEnabled   ::Logger::isEnabled(__FILE__, ::Logger::Debug)

//! Test if the Info log level is enabled for the current file.
#define LogInfoEnabled    ::Logger::isEnabled(__FILE__, ::Logger::Info)

//! Test if the Warning log level is enabled for the current file.
#define LogWarningEnabled ::Logger::isEnabled(__FILE__, ::Logger::Warning)

//! Test if the Error log level is enabled for the current file.
#define LogErrorEnabled   ::Logger::isEnabled(__FILE__, ::Logger::Error)

namespace logger {

class Backend;
class Source;

} // namespace logger

/*!
 * Logger class that allows longging via the stream operator.
 */
class Logger {
	
public:
	
	enum LogLevel {
		Debug,
		Info,
		Warning,
		Error,
		None //!< A special pseudo log level used to completely disable logging for a source file.
	};
	
private:
	static void log(const logger::Source & file, int line, LogLevel level, const std::string & str);
	
	const logger::Source * const file;
	const int line;
	const LogLevel level;
	
	std::ostringstream buffer; //! Buffer for the log message excluding level, file and line.
	
public:
	
	/*!
	 * If the given log level is es enabled for the given file, return an
	 * internal handle for the file. Otherwise return null.
	 * If not null, the return value is independent of the log level.
	 */
	static const logger::Source * get(const char * file, LogLevel level);
	
	inline Logger(const char * _file, int _line, LogLevel _level)
	       : file(get(_file, _level)), line(_line), level(_level) { }
	inline Logger(const logger::Source * _file, int _line, LogLevel _level)
	       : file(_file), line(_line), level(_level) { }
	
	template<class T>
	inline Logger & operator<<(const T & i) {
		if(file) {
			buffer << i;
		}
		return *this;
	}
	
	inline ~Logger() {
		if(file) {
			log(*file, line, level, buffer.str());
		}
	}
	
	/*!
	 * Add a backend to receive log messages.
	 * The backend will be automatically freed on exit unless it is removed first.
	 * Adding the same backend instance twice results in undefined behaviour.
	 * A NULL parameter is ignored.
	 */
	static void add(logger::Backend * backend);
	
	/*!
	 * Remove a logging backend.
	 * The backend is not automatically deleted.
	 */
	static void remove(logger::Backend * backend);
	
	/*!
	 * Set the log level for a file or group of files.
	 * @param component Path component of all files to set the log level for.
	 *               This must be either 'src', 'tools' or a subdirectory,
	 *               or the basename of a contained file (excluding the extension).
	 *               The order in which log levels are set is ignored,
	 *               but more specific prefixes overwrite more general ones.
	 * @param level The log level to set. Use log level none to disable logging completely.
	 */
	static void set(const std::string & component, LogLevel level);
	
	/*!
	 * Reset the log level for a file or group of files.
	 * Return the logger state to be if set() was never called for the given component.
	 */
	static void reset(const std::string & component);
	
	/*!
	 * @return true if the given log level is currently enabled for the current level.
	 *         Log levels inherit their enabled state: e.g. if Info is enabled,
	 *         Warning and Error are also enabled.
	 */
	inline static bool isEnabled(const char * file, LogLevel level) {
		return (get(file, level) != NULL);
	}
	
	/*!
	 * Flush buffered output in all logging backends.
	 */
		static void flush();
	
	/*!
	* Helper class to pass a C string that might be NULL to the logger.
	* If the pointer is NULL, the string "NULL" is logged.
	* Otherwise the contents of the string are logged, surrounded by double quotes. 
	*/
	struct nullstr {
		
		const char * str;
		
		inline nullstr(const char * s) : str(s) { };
		
	};
	
	/*!
	 * Initialize standard log backends.
	 */
	static void init();
	
};

inline Logger & operator<<(Logger & logger, const Logger::nullstr & s) {
	if(s.str) {
		return logger << "\"" << s.str << "\"";
	} else {
		return logger << "NULL";
	}
}

#endif // ARX_IO_LOG_LOGGER_H
