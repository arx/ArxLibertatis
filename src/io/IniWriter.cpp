
#include "io/IniWriter.h"

using std::string;
using std::endl;
using std::boolalpha;

void IniWriter::beginSection(const string & section) {
	output << endl << '[' << section << ']' << endl;
}

void IniWriter::writeKey(const string & key, const string & value) {
	output << key << '=' << '"' << value << '"' << endl;
}

void IniWriter::writeKey(const string & key, int value) {
	output << key << '=' << value << endl;
}

void IniWriter::writeKey(const string & key, float value) {
	output << key << '=' << value << endl;
}

void IniWriter::writeKey(const string & key, bool value) {
	output << key << '=' << boolalpha << value << endl;
}
