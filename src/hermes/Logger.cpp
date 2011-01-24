#include "hermes/Logger.h"
#define BASH_COLOR 1

using std::cout;

Logger::LogLevel Logger::logLevel = Logger::Debug;

const string blackList[] = {
		"ARX_FTL.cpp",
		"ARX_Script.cpp",
		"PakManager.cpp",
		"PakReader.cpp",
		"Filesystem.cpp"
};

Logger::Logger(const std::string& file, int line, Logger::LogLevel level) {
  if( level < logLevel || isInBlackList(file)) {
    m_Print = false;
    return;
  }
  m_Print = true;
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

Logger::~Logger() {
  if (m_Print)
    std::cout<<std::endl;
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
