#include "hermes/Logger.h"

using std::string;
using std::cout;

Logger::LogLevel Logger::logLevel = Logger::Info;

const string blackList[] = {"PakManager.cpp", "PakReader.cpp", "Filesystem.cpp"};

Logger::Logger(const std::string& file, int line, Logger::LogLevel level) {
  if( level < logLevel || isInBlackList(file)) {
    m_Print = false;
    return;
  }
  m_Print = true;
  switch(level) {
    case Info:
      cout<<"[\e[0;32mINFO\e[m "<<file<<":"<<line<<"]  ";
      break;
    case Warning:
      cout<<"[\e[1;33mWARNING\e[m "<<file<<":"<<line<<"]  ";
      break;
    case Error:
      cout<<"[\e[1;31mERROR\e[m "<<file<<":"<<line<<"]  ";
      break;
    default:
      cout<<"[\e[1;32mINFO\e[m "<<file<<":"<<line<<"]  ";
  };
}

Logger::~Logger() {
  if (m_Print)
    std::cout<<std::endl;
}

bool Logger::isInBlackList(const std::string& file) {
	for (int i=0; i < sizeof(blackList)/sizeof(string); i++) {
		if (blackList[i] == file)
			return true;
	}
	return false;
}
