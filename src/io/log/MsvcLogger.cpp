
#include "io/log/MsvcLogger.h"

#include <sstream>

#include <windows.h>

namespace logger {

MsvcDebugger::~MsvcDebugger() {
	// nothing to clean up
}

void MsvcDebugger::log(const Source & file, int line, Logger::LogLevel level, const std::string & str) {
	
	std::ostringstream oss;
	switch(level) {
		case Logger::Debug:   oss << "[D]"; break;
		case Logger::Info:    oss << "[I]"; break;
		case Logger::Warning: oss << "[W]"; break;
		case Logger::Error:   oss << "[E]"; break;
		case Logger::None: ARX_DEAD_CODE();
	}
	
	oss << ' ' << file.name << ':' << line << "  " << str << "\n";
	
	OutputDebugString(oss.str().c_str());
}

void MsvcDebugger::flush() {
	// nothing to do
}

MsvcDebugger * MsvcDebugger::get() {
	return IsDebuggerPresent() ? new MsvcDebugger : NULL;
}

} // namespace logger
