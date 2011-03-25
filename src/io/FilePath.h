
#ifndef ARX_IO_FILEPATH_H
#define ARX_IO_FILEPATH_H

#include <string>

/**
 * Return only the directory part of the path given,
 * this includes everything up until the last slash
 **/
std::string GetDirectory( const std::string& );

/**
 * Remove the filename and file extension from the given path,
 * leaving only the directory and a trailing slash (or an empty string).
 **/
void RemoveName(std::string & str);

/**
 * Get the filename in the given path.
 * @return the string between the last (back-)slash and the last dot (or end of string).
 **/
std::string GetName(const std::string & str);

/**
 * Set the extension of the given path.
 * Replaces any existing extension.
 * @param new_ext The extension to set or an empty string to remove the extension.
 **/
void SetExt(std::string & str, const std::string & new_ext);

/**
 * Get the file extension of the given path.
 * @return The string after the last dot after the last (back-)slash or an empty string.
 **/
std::string GetExt(const std::string & str);

/**
 * Add a string to the given path right after the file name, but before the extension.
 **/
void AddToName(std::string & str, const std::string & cat);

/**
 * Standardize the given path.
 * Removes any ./, evaluates ../ and changes all characters to upper case.
 */
void File_Standardize(const std::string & from, std::string & to);

#endif // ARX_IO_FILEPATH_H
