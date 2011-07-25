
#include "io/FileStream.h"

#include "io/FilePath.h"

namespace fs {

ifstream::ifstream(const path & p, openmode mode) : std::ifstream(p.string().c_str(), mode) { }

void ifstream::open(const path & p, openmode mode) {
	std::ifstream::open(p.string().c_str(), mode);
}

ofstream::ofstream(const path & p, openmode mode) : std::ofstream(p.string().c_str(), mode) { }

void ofstream::open(const path & p, openmode mode) {
	std::ofstream::open(p.string().c_str(), mode);
}

fstream::fstream(const path & p, openmode mode) : std::fstream(p.string().c_str(), mode) { }

void fstream::open(const path & p, openmode mode) {
	std::fstream::open(p.string().c_str(), mode);
}

} // namespace fs
