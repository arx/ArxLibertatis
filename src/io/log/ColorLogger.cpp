
#include "io/log/ColorLogger.h"

#include <iostream>

namespace logger {

ColorConsole::~ColorConsole() {
	// nothing to clean up
}

void ColorConsole::log(const Source & file, int line, Logger::LogLevel level, const std::string & str) {
	
	std::ostream * os;
	switch(level) {
		case Logger::Debug:   std::cout << "\e[1;36m[D]\e[0;36m", os = &std::cout; break;
		case Logger::Info:    std::cout << "\e[1;32m[I]\e[0;32m", os = &std::cout; break;
		case Logger::Warning: std::cerr << "\e[1;33m[W]\e[0;33m", os = &std::cerr; break;
		case Logger::Error:   std::cerr << "\e[1;31m[E]\e[0;31m", os = &std::cerr; break;
		case Logger::None: ARX_DEAD_CODE();
	}
	
	(*os) << ' ' << file.name << "\e[m:\e[0;33m" << line << "\e[m" << "  " << str << std::endl;
}

void ColorConsole::flush() {
	std::cout.flush(), std::cerr.flush();
}

} // namespace logger
