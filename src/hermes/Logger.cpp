#include <stdlib.h>
#include "hermes/Logger.h"
#define BASH_COLOR 1

using std::cout;

Logger::LogLevel Logger::logLevel = Logger::Debug;

const string blackList[] = {
		/*"ARX_FTL.cpp",*/
		"ARX_Script.cpp",
		"PakManager.cpp",
		"PakReader.cpp",
		"Filesystem.cpp",
		"Athena.cpp"
};

Logger::Logger(const std::string& file, int line, Logger::LogLevel level) {
  fatal = false;
  if( level < logLevel || isInBlackList(file)) {
    print = false;
    return;
  }
  print = true;
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
    case Fatal:
      log(4,31,"FATAL",file, line);
      fatal = true;
      break;
    default:
      log(1,32,"INFO",file, line);
  };
}

Logger::~Logger() {
  if (print)
    std::cout<<std::endl;
  if (fatal)
	  exit(0);
}

void Logger::log(int mode, int color, const string & level,
		const string & file, int line) {
#ifdef BASH_COLOR
	cout<<"[\e["<< mode <<";"<< color <<"m" << level << "\e[m]  "
			<<"\e[0;35m"<<file<<"\e[m:\e[0;33m"<<line<<"\e[m"<<"  ";
#else
	cout<<"["<< level << "]  "<<file<<":"<<line<<"  ";
#endif
}

bool Logger::isInBlackList(const std::string& file) {
	for (unsigned i=0; i < sizeof(blackList)/sizeof(string); i++) {
		if (blackList[i] == file)
			return true;
	}
	return false;
}
