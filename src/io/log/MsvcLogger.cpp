
#include "io/log/MsvcLogger.h"

#include <sstream>

#include <windows.h>

namespace logger {

MsvcDebugger::~MsvcDebugger() {
	// nothing to clean up
}

void MsvcDebugger::log(const Source & file, int line, Logger::LogLevel level, const std::string & str) {
	std::ostringstream oss;
	format(oss, file, line, level, str);
	OutputDebugString(oss.str().c_str());
}

void MsvcDebugger::flush() {
	// nothing to do
}

MsvcDebugger * MsvcDebugger::get() {
	return IsDebuggerPresent() ? new MsvcDebugger : NULL;
}

} // namespace logger
