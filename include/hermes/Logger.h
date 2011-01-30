#ifndef LOGGER_H
#define LOGGER_H

#include <ARX_Common.h>
#include <cstring>

#if !ARX_COMPILER_MSVC
    #define ShortFile std::strrchr(__FILE__, '/')+1
#else
    #define ShortFile std::strrchr(__FILE__, '\\')+1
#endif

#define LogDebug    Logger(ShortFile,__LINE__, Logger::Debug)
#define LogError    Logger(ShortFile,__LINE__, Logger::Error)
#define LogWarning  Logger(ShortFile,__LINE__, Logger::Warning)
#define LogInfo     Logger(ShortFile,__LINE__, Logger::Info)
#define LogFatal    Logger(ShortFile,__LINE__, Logger::Fatal)

#include <iostream>
#include <string>

using std::string;

class Logger {

public:
  enum LogLevel {
	Debug = 0,
    Info,
    Warning,
    Error,
    Fatal
  };
  Logger(const char* file, int line, LogLevel level);
  Logger(const std::string& file, int line, LogLevel level);
  virtual ~Logger();
  template<class T>
  Logger& operator<< (T i) {
    if (print)
      std::cout<<i;
    return *this;
  }

  static LogLevel logLevel;
private:
  bool print, fatal;
  bool isInBlackList(const std::string& file);
  void writeInfo(const char* file, int line, Logger::LogLevel level);
  void log(int mode, int color, const string & level,
			const string & file, int line);
};

#endif // LOGGER_H
