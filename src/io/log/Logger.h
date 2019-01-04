/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ARX_IO_LOG_LOGGER_H
#define ARX_IO_LOG_LOGGER_H

#include <sstream>
#include <string>

#include "platform/Platform.h"

#define ARX_LOG(Level)         ::Logger(ARX_FILE, __LINE__, Level)
#define ARX_LOG_FORCED(Level)  ::Logger(ARX_FILE, __LINE__, Level, true)
#define ARX_LOG_ENABLED(Level) ::Logger::isEnabled(ARX_FILE, Level)

#ifdef ARX_DEBUG
//! Log a Debug message. Arguments are only evaluated if their results will be used.
#define LogDebug(...)    \
	if(ARX_LOG_ENABLED(::Logger::Debug)) \
		ARX_LOG_FORCED(::Logger::Debug) << __VA_ARGS__
#else
#define LogDebug(...)    ARX_DISCARD(__VA_ARGS__)
#endif

//! Log an Info message. Arguments are always evaluated.
#define LogInfo     ARX_LOG(::Logger::Info)

//! Log a Warning message. Arguments are always evaluated.
#define LogWarning  ARX_LOG(::Logger::Warning)

//! Log an Error message. Arguments are always evaluated.
#define LogError    ARX_LOG(::Logger::Error)

//! Log a Critical message - will cause the application to exit with a dialog. Arguments are always evaluated.
#define LogCritical ARX_LOG(::Logger::Critical)

//! Test if the Debug log level is enabled for the current file.
#define LogDebugEnabled   ARX_LOG_ENABLED(::Logger::Debug)

//! Test if the Info log level is enabled for the current file.
#define LogInfoEnabled    ARX_LOG_ENABLED(::Logger::Info)

//! Test if the Warning log level is enabled for the current file.
#define LogWarningEnabled ARX_LOG_ENABLED(::Logger::Warning)

//! Test if the Error log level is enabled for the current file.
#define LogErrorEnabled   ARX_LOG_ENABLED(::Logger::Error)

namespace logger { class Backend; }

/*!
 * Logger class that allows longging via the stream operator.
 */
class Logger {
	
public:
	
	enum LogLevel {
		Debug,
		Info,
		Console,
		Warning,
		Error,
		Critical,
		None //!< A special pseudo log level used to completely disable logging for a source file.
	};
	
private:
	
	static void log(const char * file, int line, LogLevel level, const std::string & str);
	
	const char * const file;
	const int line;
	const LogLevel level;
	const bool enabled;
	
	std::ostringstream buffer; //! Buffer for the log message excluding level, file and line.
	
public:
	
	Logger(const char * _file, int _line, LogLevel _level, bool _enabled)
		: file(_file), line(_line), level(_level), enabled(_enabled) { }
	Logger(const char * _file, int _line, LogLevel _level)
		: file(_file), line(_line), level(_level), enabled(isEnabled(_file, _level)) { }
	
	template <class T>
	Logger & operator<<(const T & i) {
		if(enabled) {
			buffer << i;
		}
		return *this;
	}
	
	~Logger() {
		if(enabled) {
			log(file, line, level, buffer.str());
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
	 * \param component Path component of all files to set the log level for.
	 *                  This must be either 'src', 'tools' or a subdirectory,
	 *                  or the basename of a contained file (excluding the extension).
	 *                  The order in which log levels are set is ignored,
	 *                  but more specific prefixes overwrite more general ones.
	 * \param level The log level to set. Use log level none to disable logging completely.
	 */
	static void set(const std::string & component, LogLevel level);
	
	/*!
	 * Reset the log level for a file or group of files.
	 * Return the logger state to be if set() was never called for the given component.
	 */
	static void reset(const std::string & component);
	
	/*!
	 * Configure log levels for multiple components.
	 * Config is a comma-seperated list of compontent log level assignments,
	 * Each assignment is of the form "component=level" or "component".
	 * Valid levels are:
	 *  - "debug" / "d" / "D" (default if level is omitted)
	 *  - "info" / "i" / "I"
	 *  - "warning" / "warn" / "w" / "W"
	 *  - "error" / "e" / "E"
	 *  - "none" / "n" / "N"
	 *  - "reset" / "r" / "R" / "-"
	 */
	static void configure(const std::string & settings);
	
	/*!
	 * \return true if the given log level is currently enabled for the current level.
	 *         Log levels inherit their enabled state: e.g. if Info is enabled,
	 *         Warning and Error are also enabled.
	 */
	static bool isEnabled(const char * file, LogLevel level);
	
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
		
		explicit nullstr(const char * s) : str(s) { }
		
	};
	
	/*!
	 * Initialize standard log backends.
	 */
	static void initialize();
	
	/*!
	 * Shutdown the logging and free all registered backends.
	 */
	static void shutdown();
	
	static void quickShutdown();
};

inline Logger & operator<<(Logger & logger, const Logger::nullstr & s) {
	if(s.str) {
		return logger << "\"" << s.str << "\"";
	} else {
		return logger << "NULL";
	}
}

#endif // ARX_IO_LOG_LOGGER_H
