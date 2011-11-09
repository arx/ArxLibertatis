
#include "LogBackend.h"

void logger::Backend::format(std::ostream & os, const logger::Source & file, int line, Logger::LogLevel level, const std::string & str) {
	
	switch(level) {
		case Logger::Debug:   os << "[D]"; break;
		case Logger::Info:    os << "[I]"; break;
		case Logger::Warning: os << "[W]"; break;
		case Logger::Error:   os << "[E]"; break;
		case Logger::None: ARX_DEAD_CODE();
	}
	
	os << ' ' << file.name << ':' << line << "  " << str << std::endl;
}
