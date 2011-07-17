
#include "platform/String.h"
#include "platform/Platform.h"

#include <cstring>
#include <algorithm>
#include <sstream>

using std::string;
using std::transform;

void makeLowercase(string & str) {
	transform(str.begin(), str.end(), str.begin(), ::tolower);
}

string toLowercase(const string & str) {
	string copy = str;
	makeLowercase(copy);
	return copy;
}

string safestring(const char * data, size_t maxLength) {
	return string(data, std::find(data, data + maxLength, '\0'));
}

bool IsIn(const string & strin, const string & str) {
	return (strin.find( str ) != string::npos);
}

long specialstrcmp(const string & text, const string & seek) {
	return text.compare(0, seek.length(), seek) ? 1 : 0;
}

int atoi( const std::string& str )
{
	std::stringstream ss( str );
	int out;
	ss >> out;
	return out;
}

std::string itoa( int i )
{
	std::stringstream ss;
	ss << i;
	std::string out;
	ss >> out;
	return out;
}

bool atob( const std::string& str )
{
	std::stringstream ss( str );
	bool out;
	ss >> out;
	return out;
}

std::string btoa( bool i )
{
	std::stringstream ss;
	ss << i;
	std::string out;
	ss >> out;
	return out;
}

void SAFEstrcpy(char * dest, const char * src, unsigned long max) {
	if(strlen(src) > max) {
		memcpy(dest, src, max);
	} else {
		strcpy(dest, src);
	}
}
