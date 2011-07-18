
#include "io/FilePath.h"

#include <algorithm>

using std::string;
using std::copy;

static inline bool isDirSeperator(char c) {
	return (c == '\\' || c == '/'); // TODO(case-sensitive) remove backslash
}

string GetName(const string & str) {
	size_t extpos = str.find_last_of(EXT_OR_DIR_SEP);
	// No extension so far.
	if(extpos == string::npos) {
		return str;
	} else if(str[extpos] != '.') {
		return string(str, extpos + 1);
	}
	size_t dirpos = str.find_last_of(DIR_SEP, extpos);
	if(dirpos == string::npos) {
		return string(str, 0, extpos);
	}
	return string(str, dirpos + 1, extpos - dirpos - 1);
}

string GetExt(const string & str) {
	size_t extpos = str.find_last_of(EXT_OR_DIR_SEP);
	// No extension so far.
	if(extpos == string::npos || str[extpos] != '.') {
		return string();
	}
	return string(str, extpos);
}

const string & SetExt(string & str, const string & new_ext) {
	size_t extpos = str.find_last_of(EXT_OR_DIR_SEP);
	// No extension so far.
	if(extpos == string::npos || str[extpos] != '.') {
		if(!new_ext.empty() && new_ext[0] != '.') {
			str += '.';
		}
		str += new_ext;
		return str;
	}
	if(!new_ext.empty() && new_ext[0] != '.') {
		str.resize(extpos + 1 + new_ext.length());
		str[extpos] = '.';
		copy(new_ext.begin(), new_ext.end(), str.begin() + extpos + 1);
	} else {
		str.resize(extpos + new_ext.length());
		copy(new_ext.begin(), new_ext.end(), str.begin() + extpos);
	}

	return str;
}

void AddToName(string & str, const string & cat) {
	size_t extpos = str.find_last_of(EXT_OR_DIR_SEP);
	// No extension so far.
	if(extpos == string::npos || str[extpos] != '.') {
		str += cat;
		return;
	}
	size_t oldlen = str.length();
	str.resize(str.length() + cat.length());
	copy(str.begin() + extpos, str.begin() + oldlen, str.begin() + extpos + cat.length());
	copy(cat.begin(), cat.end(), str.begin() + extpos);
}

/*
 * Return the string up until and including the
 * last directory separator or the whole string
 */
string GetDirectory(const string & str) {
	size_t pos = str.find_last_of(DIR_SEP);
	return (pos == string::npos) ? string() : str.substr(0, pos + 1);
}

void RemoveName(string & str) {
	size_t pos = str.find_last_of(DIR_SEP);
	if(pos == string::npos) {
		str.clear();
	} else {
		str.resize(pos + 1);
	}
}

void File_Standardize(const string & from, string & to) {
	
	size_t in = 0;
	
	string temp;
	
	while(in != from.size()) {
		
		char c = from[in];
		if(isDirSeperator(c)) {
			
			in++;
			
			// Remove multiple consecutive slashes.
			while(in != from.size() && isDirSeperator(from[in])) {
				in++;
			}
			
			if(in == from.size()) {
				temp.push_back(c);
				break;
			}
			
			if(from[in] == '.') {
				
				in++;
				if(in == from.size()) {
					// /.<end>
					temp.push_back(c);
					continue;
				}
				
				if(from[in] == '.') {
					in++;
					
					if(in == from.size() || isDirSeperator(from[in])) {
						// /..<end> or /../
						
						size_t last = temp.find_last_of(DIR_SEP);
						
						if(last == string::npos) {
							last = 0;
						} else {
							last++;
						}
						
						if(last != temp.size() - 2 || temp[last] != '.' || temp[last + 1] != '.') {
							temp.resize(last);
							continue;
						}
						
					}
					
					// /..?
					temp.push_back(c);
					temp += "..";
					
				} else if(isDirSeperator(from[in])) {
					// /./
					// ignore
				} else {
					// /.?
					temp.push_back(c);
					temp.push_back('.');
				}
				
			} else {
				temp.push_back(c);
			}
			
		} else {
			temp.push_back(c);
			in++;
		}
	}
	
	to = temp;
}

std::string loadPath(const std::string & path) {
	string copy = path;
	makeLowercase(copy);
	std::replace(copy.begin(), copy.end(), '\\', '/');
	return copy;
}
