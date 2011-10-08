
#ifndef ARX_IO_LOGGER_H
#define ARX_IO_LOGGER_H

#include <sstream>
#include <iostream>
#include <string>

#include "platform/Platform.h"

#ifdef _DEBUG
#define LogDebug(...)    Logger(__FILE__,__LINE__, Logger::Debug) << __VA_ARGS__
#else
#define LogDebug(...)    ARX_DISCARD(__VA_ARGS__)
#endif

#define LogError    Logger(__FILE__,__LINE__, Logger::Error)
#define LogWarning  Logger(__FILE__,__LINE__, Logger::Warning)
#define LogInfo     Logger(__FILE__,__LINE__, Logger::Info)

class Logger {
	
public:
	
	enum LogLevel {
		Debug,
		Info,
		Warning,
		Error
	};
	
	Logger(const char * file, int line, LogLevel level);
	
	virtual ~Logger();
	
	template<class T>
	Logger & operator<<(const T & i) {
		if(enabled) {
			std::stringstream ss;
			ss << i;
			log(ss.str());
		}
		return *this;
	}
	
	struct nullstr {
		
		const char * str;
		
		inline nullstr(const char * s) : str(s) { };
		
	};
	
	/* Log a string that may be null. */
	Logger & operator<<(const nullstr & s);
	
	static LogLevel logLevel;
	
private:
	
	bool enabled;
	bool isInBlackList(const std::string & file);
	void writeInfo(const char * file, int line, LogLevel level);
	LogLevel getLogLevel(const std::string & file);
	void log(int mode, int color, const std::string & level, const std::string & file, int line);
	void log(const std::string& str);
	
};

#endif // ARX_IO_LOGGER_H
