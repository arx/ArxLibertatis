
#ifndef ARX_IO_INIWRITER_H
#define ARX_IO_INIWRITER_H

#include <ostream>
#include <string>

/*!
 * Simple writer for ini-formatted data.
 */
class IniWriter {
	
private:
	
	std::ostream & output;
	
public:
	
	/*!
	 * Initialize this ini writer.
	 * @param _output Reference to an ostream that mus remain valid while the writer is used.
	 */
	IniWriter(std::ostream & _output) : output(_output) { }
	
	/*!
	 * Flush the output stream.
	 * @return true if there were no errors during writing.
	 */
	bool flush() {
		return !output.flush().bad();
	}
	
	/*!
	 * Write a section header to the output stream.
	 * @param section The section to start.
	 */
	void beginSection(const std::string & section);
	
	void writeKey(const std::string & key, const std::string & value);
	void writeKey(const std::string & key, int value);
	void writeKey(const std::string & key, float value);
	void writeKey(const std::string & key, bool value);
	
};

#endif // ARX_IO_INIWRITER_H
