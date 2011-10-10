
#include "io/log/FileLogger.h"

#include "io/log/ColorLogger.h"

namespace logger {

File::~File() {
	// nothing to clean up
}

void File::log(const Source & file, int line, Logger::LogLevel level, const std::string & str) {
	format(ofs, file, line, level, str);
}

void File::flush() {
	ofs.flush();
}

} // namespace logger
