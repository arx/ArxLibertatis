
#ifndef ARX_IO_FILESTREAM_H
#define ARX_IO_FILESTREAM_H

#include <fstream>

namespace fs {

class path;

class ifstream : public std::ifstream {
	
private:
	
	ifstream(const ifstream &);
	ifstream & operator=(const ifstream &);
	
public:
	
	ifstream() { }
	
	ifstream(const path & p, openmode mode = in);
	
	void open(const path & p, openmode mode = in);
	
	virtual ~ifstream() { }
	
};

class ofstream : public std::ofstream {
	
private:
	
	ofstream(const ofstream &);
	ofstream & operator=(const ofstream &);
	
public:
	
	ofstream() { }
	
	ofstream(const path & p, openmode mode = out);
	
	void open(const path & p, openmode mode = out);
	
	virtual ~ofstream() { }
	
};

class fstream : public std::fstream {
	
private:
	
	fstream(const fstream &);
	fstream & operator=(const fstream &);
	
public:
	
	fstream() { }
	
	fstream(const path & p, openmode mode = out);
	
	void open(const path & p, openmode mode = out);
	
	virtual ~fstream() { }
	
};

} // namespace fs

#endif // ARX_IO_FILESTREAM_H
