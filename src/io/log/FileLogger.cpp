
#include "io/log/FileLogger.h"

#include "io/log/ColorLogger.h"

#include "Configure.h"

namespace logger {

File::~File() {
	// nothing to clean up
}

void File::log(const Source & file, int line, Logger::LogLevel level, const std::string & str) {
	
	switch(level) {
		case Logger::Debug:   ofs << "[D]"; break;
		case Logger::Info:    ofs << "[I]"; break;
		case Logger::Warning: ofs << "[W]"; break;
		case Logger::Error:   ofs << "[E]"; break;
		case Logger::None: ARX_DEAD_CODE();
	}
	
	ofs << ' ' << file.name << ':' << line << "  " << str << std::endl;
}

void File::flush() {
	ofs.flush();
}

} // namespace logger
