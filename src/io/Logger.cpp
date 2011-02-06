
#include "io/Logger.h"

#include <cstdlib>

#define BASH_COLOR 1

using std::cout;

Logger::LogLevel Logger::logLevel = Logger::Debug;

struct LogSetting {
	string codefile;
	Logger::LogLevel logLevel;
};

const LogSetting blackList[] = {
	{ "FTL.cpp", Logger::Warning },
	{ "Script.cpp", Logger::Info },
	{ "ScriptEvent.cpp", Logger::Info },
	{ "LoadLevel.cpp", Logger::Info },
	{ "PakManager.cpp", Logger::Fatal },
	{ "PakReader.cpp", Logger::Info },
	{ "Filesystem.cpp", Logger::Fatal },
	{ "Audio.cpp", Logger::Error },
	{ "AudioInstance.cpp", Logger::Info },
	{ "Object.cpp", Logger::Warning },
	{ "Speech.cpp", Logger::Error },
	{ "Text.cpp", Logger::Info },
	{ "CinematicLoad.cpp", Logger::Info },
	{ "IO.cpp", Logger::Debug },
//	{ "MenuWidgets.cpp", Logger::Info },
	{ "MeshManipulation.cpp", Logger::Info},
	{ "Core.cpp", Logger::Info},
	{ "CinematicTexture.cpp", Logger::Info},
	{ "Mesh.cpp", Logger::Info}
};

Logger::Logger(const std::string& file, int line, Logger::LogLevel level) {
  fatal = false;
	LogLevel curLevel = getLogLevel(file);
  if(level < curLevel || curLevel == None) {
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

Logger::LogLevel Logger::getLogLevel(const std::string& file) {
	for (unsigned i=0; i < sizeof(blackList)/sizeof(*blackList); i++) {
		if (blackList[i].codefile == file)
			return blackList[i].logLevel;
	}
	return logLevel;
}

Logger & Logger::operator<<(const nullstr & s) {
	if(print) {
		if(s.str) {
			*this << "\"" << s.str << "\"";
		} else {
			*this << "NULL";
		}
	}
	return *this;
}

