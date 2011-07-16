
#ifndef ARX_PLATFORM_STRING_H
#define ARX_PLATFORM_STRING_H

#include <string>
#include <cstring>

void makeLowercase(std::string & str);
std::string toLowercase(const std::string & str);

/**
 * Load an std::string from a const char * that may not be null-terminated.
 */
std::string safestring(const char * data, size_t maxLength);

template <size_t N>
std::string safestring(const char (&data)[N]) {
	return safestring(data, N);
}

bool IsIn(const std::string & strin, const std::string & str);
int strcasecmp(const std::string & str1, const std::string & str2);
int strcmp(const std::string & str1, const std::string & str2);

/**
 * Converts a given string to an integer using stringstream
 * @param str The string to be converted
 * @return The integer created by stringstream
 */
int atoi( const std::string& str );

/**
 * Converts a given integer to a string using stringstream
 * @param i The integer to be converted
 * @return The string created by stringstream
 */
std::string itoa( int i );

/**
 * Converts a given bool value to a string using stringstream
 * @param b The bool to be converted
 * @return The string string created by stringstream
 */
std::string btoa( bool b );

/**
 * Converts a given string to a bool using stringstream
 * @param str The string to be converted
 * @return The bool created by stringstream
 */
bool atob( const std::string& str );

/**
 * Checks if a string (seek) is at the start of another string (text)
 * returns 0 if "seek" is at the start of "text"
 * else returns 1
 */
long specialstrcmp(const std::string & text, const std::string & seek);

void SAFEstrcpy(char * dest, const char * src, unsigned long max);

/**
 * Read-only reference to (part of) a string that is stored somewhere else.
 */
class strref {
	
private:
	
	const char * const _data;
	const size_t _length;
	
public:
	
	inline strref() : _data(NULL), _length(0) { };
	inline strref(const char * data) : _data(data), _length(std::strlen(data)) { };
	inline strref(const char * data, size_t length) : _data(data), _length(length) { };
	inline strref(const std::string & str) : _data(str.data()), _length(str.length()) { };
	inline strref(const strref & o) : _data(o._data), _length(o._length) { };
	inline strref(const std::string & str, size_t offset) : _data(str.data() + offset), _length(str.length() - offset) { };
	inline strref(const std::string & str, size_t offset, size_t length) : _data(str.data() + offset), _length(length) { };
	
	inline operator std::string() const {
		return std::string(_data, _length);
	}
	
	inline bool operator==(const std::string & str) const {
		return (str.compare(0, str.length(), _data, _length) == 0);
	}
	
	inline size_t length() const {
		return _length;
	}
	
	inline const char * begin() const {
		return _data;
	}
	
	inline const char * end() const {
		return _data + _length;
	}
	
	inline strref substr(size_t offset) const {
		return strref(_data + offset, _length - offset);
	}
	
	inline strref substr(size_t offset, size_t length) const {
		return strref(_data + offset, length);
	}
	
	inline bool empty() const {
		return (_length == 0);
	}
	
};

inline bool operator==(const std::string & str1, strref str2) {
	return (str2 == str1);
}

#endif // ARX_PLATFORM_STRING_H
