#ifndef LOGGER_H
#define LOGGER_H

#define ShortFile strrchr(__FILE__, '/')+1
#define LogError Logger(ShortFile,__LINE__, Logger::Error)
#define LogWarning Logger(ShortFile,__LINE__, Logger::Warning)
#define LogInfo Logger(ShortFile,__LINE__, Logger::Info)

#include <iostream>

class Logger {

public:
  enum LogLevel {
    Info=0,
    Warning,
    Error
  };
  Logger(const std::string& file, int line, LogLevel level);
  virtual ~Logger();
  template<class T>
  Logger& operator<< (T i) {
    if (m_Print)
      std::cout<<i;
    return *this;
  }

  static LogLevel logLevel;
private:
  bool m_Print;
  bool isInBlackList(const std::string& file);
};

#endif // LOGGER_H
