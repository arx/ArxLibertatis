
#include "io/String.h"

#include <cstring>
#include <algorithm>

using std::string;
using std::transform;

void MakeUpcase( std::string& str ) {
	transform(str.begin(), str.end(), str.begin(), ::toupper);
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

void SAFEstrcpy(char * dest, const char * src, unsigned long max) {
	if(strlen(src) > max) {
		memcpy(dest, src, max);
	} else {
		strcpy(dest, src);
	}
}
