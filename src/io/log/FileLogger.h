
#ifndef ARX_IO_LOG_FILELOGGER_H
#define ARX_IO_LOG_FILELOGGER_H

#include "io/FileStream.h"
#include "io/log/LogBackend.h"

namespace logger {

/*!
 * Simple logger that prints plain text to standard output.
 */
class File : public Backend {
	
	fs::ofstream ofs;
	
public:
	
	inline File(const fs::path & path, std::ios_base::openmode mode)
	                  : ofs(path, mode) { }
	
	~File();
	
	void log(const Source & file, int line, Logger::LogLevel level, const std::string & str);
	
	void flush();
	
};

} // namespace logger

#endif // ARX_IO_LOG_FILELOGGER_H
