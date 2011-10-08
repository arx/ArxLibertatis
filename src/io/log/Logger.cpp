
#include "io/log/Logger.h"

#include <cstdlib>
#include <cstring>

#if ARX_COMPILER_MSVC
#include <windows.h>
#endif

#include "platform/Platform.h"

#define BASH_COLOR !ARX_COMPILER_MSVC

using std::cout;
using std::string;

Logger::LogLevel Logger::logLevel = Logger::Info;

namespace {

struct LogSetting {
	string codefile;
	Logger::LogLevel logLevel;
};

const LogSetting blackList[] = {
	{ "dummy", Logger::Info },
};

};

Logger::Logger(const char* file, int line, Logger::LogLevel level) {
	writeInfo(file, line, level);
}

Logger::~Logger() {
  if (enabled)
	  log("\n");
}

void Logger::writeInfo(const char * longFile, int line, Logger::LogLevel level) {

  const char* file = std::strrchr(longFile, '/');
  if(file == 0)
    file = std::strrchr(longFile, '\\');
  ++file;
	
  LogLevel curLevel = getLogLevel(longFile);
  if(level < curLevel) {
	  enabled = false;
	  return;
  }
  
  enabled = true;
  switch(level) {
    case Info:
      log(1,32,"INFO",file, line);
      break;
    case Warning:
      log(1,33,"WARNING",file, line);
      break;
    case Error:
      log(1,31,"ERROR",file, line);
      break;
    case Debug:
      log(1,36,"DEBUG",file, line);
      break;
    default:
      log(1,32,"INFO",file, line);
  };
}

void Logger::log(int mode, int color, const string & level,
		const string & file, int line) {
	std::stringstream ss;

#if BASH_COLOR
	ss << "[\e[" << mode << ";" << color << "m" << level << "\e[m]  "
	   << "\e[0;35m" << file << "\e[m:\e[0;33m" << line << "\e[m" << "  ";
#else
	ss << "[" << level << "]  " << file << ":" << line << "  ";
#endif

	log(ss.str());
}

void Logger::log(const string& str) {
	std::cout << str;
#if ARX_COMPILER_MSVC
	if(IsDebuggerPresent())
		OutputDebugString(str.c_str());
#endif
}

Logger::LogLevel Logger::getLogLevel(const string & file) {
	for (unsigned i=0; i < ARRAY_SIZE(blackList); i++) {
		if (file.find(blackList[i].codefile) != string::npos)
			return blackList[i].logLevel;
	}
	return logLevel;
}

Logger & Logger::operator<<(const nullstr & s) {
	if(enabled) {
		if(s.str) {
			*this << "\"" << s.str << "\"";
		} else {
			*this << "NULL";
		}
	}
	return *this;
}

