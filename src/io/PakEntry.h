/*
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/

#ifndef ARX_IO_PAKENTRY_H
#define ARX_IO_PAKENTRY_H

#include <string>
#include <map>

namespace fs {
class path;
}

class PakFileHandle;

class PakFile {
	
private:
	
	size_t _size;
	
	PakFile * _alternative;
	
protected:
	
	inline PakFile(size_t size) :  _size(size), _alternative(NULL) { };
	
	virtual ~PakFile();
	
	friend class PakReader;
	friend class PakDirectory;
	
public:
	
	inline size_t size() const { return _size; }
	inline PakFile * alternative() const { return _alternative; }
	
	virtual void read(void * buf) const = 0;
	char * readAlloc() const;
	
	virtual PakFileHandle * open() const = 0;
	
};

class PakDirectory {
	
private:
	
	// TODO hash maps might be a better fit
	std::map<std::string, PakFile *> files;
	std::map<std::string, PakDirectory> dirs;
	
	PakDirectory * addDirectory(const fs::path & path);
	
	void addFile(const std::string & name, PakFile * file);
	void removeFile(const std::string & name);
	
	friend class PakReader;
	friend class std::map<std::string, PakDirectory>;
	friend struct std::pair<const std::string, PakDirectory>;
	
public:
	
	PakDirectory();
	~PakDirectory();
	
	typedef std::map<std::string, PakDirectory>::iterator dirs_iterator;
	typedef std::map<std::string, PakFile *>::const_iterator files_iterator;
	
	PakDirectory * getDirectory(const fs::path & path);
	
	PakFile * getFile(const fs::path & path);
	
	inline dirs_iterator dirs_begin() { return dirs.begin(); }
	inline dirs_iterator dirs_end() { return dirs.end(); }
	
	inline files_iterator files_begin() { return files.begin(); }
	inline files_iterator files_end() { return files.end(); }
	
};

#endif // ARX_IO_PAKENTRY_H
