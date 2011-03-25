
#ifndef ARX_IO_STRING_H
#define ARX_IO_STRING_H

#include <string>

void MakeUpcase(std::string & str);
bool IsIn(const std::string & strin, const std::string & str);
bool NC_IsIn(std::string strin, std::string str);
int strcasecmp(const std::string & str1, const std::string & str2);
int strcmp(const std::string & str1, const std::string & str2);

int atoi( const std::string& str );
std::string itoa( int i );

/**
 * Checks if a string (seek) is at the start of another string (text)
 * returns 0 if "seek" is at the start of "text"
 * else returns 1
 */
long specialstrcmp(const std::string & text, const std::string & seek);

void SAFEstrcpy(char * dest, const char * src, unsigned long max);

#endif // ARX_IO_STRING_H
