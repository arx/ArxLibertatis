
#include "platform/String.h"

#include <cstring>
#include <algorithm>
#include <sstream>

using std::string;
using std::transform;

std::string MakeUpcase( std::string& str ) {
	transform(str.begin(), str.end(), str.begin(), ::toupper);
	return str;
}

bool NC_IsIn(string t1, string t2) {
	MakeUpcase(t1);
	MakeUpcase(t2);
	return (t1.find(t2) != string::npos);
}

bool IsIn(const string & strin, const string & str) {
	return (strin.find( str ) != string::npos);
}

int strcasecmp(const string & str1, const string & str2) {
	return strcasecmp(str1.c_str(), str2.c_str());
}

int strcmp(const string & str1, const string & str2) {
	return str1.compare(str2);
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

void SAFEstrcpy(char * dest, const char * src, unsigned long max) {
	if(strlen(src) > max) {
		memcpy(dest, src, max);
	} else {
		strcpy(dest, src);
	}
}
