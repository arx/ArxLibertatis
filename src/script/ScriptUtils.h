/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ARX_SCRIPT_SCRIPTUTILS_H
#define ARX_SCRIPT_SCRIPTUTILS_H

#include <string>
#include <vector>

#include "platform/Platform.h"
#include "script/ScriptEvent.h"
#include "io/log/Logger.h"

namespace script {

inline u64 flag(char c) {
	if(c >= '0' && c <= '9') {
		return (u64(1) << (c - '0'));
	} else if(c >= 'a' && c <= 'z') {
		return (u64(1) << (c - 'a' + 10));
	} else {
		return (u64(1) << 63);
	}
}

inline bool test_flag(u64 flg, char c) {
	return (flg & flag(c)) != 0;
}

inline u64 flags(const std::string & flags) {
	
	
	size_t i = 0;
	if(flags.length() > 0 && flags[0] == '-') {
		i++;
	}
	
	u64 result = 0ul;
	for(; i < flags.length(); i++) {
		result |= flag(flags[i]);
	}
	
	return result;
}

/**
 * Overload to give compilers a chance to calculate flag masks at compile-time for string constants.
 * 
 * This should probably be done using constexpr in c++11.
 * We could force compile-time calculation with a template-based implementation, but that will be much uglier and limited.
 */
template <size_t N>
inline u64 flags(const char (&flags)[N]) {
	
	u64 result = 0ul;
	for(size_t i = (flags[0] == '-') ? 1 : 0; i < N - 1; i++) {
		result |= flag(flags[i]);
	}
	
	return result;
}

class Context {
	
private:
	
	EERIE_SCRIPT * script;
	size_t pos;
	INTERACTIVE_OBJ * io;
	ScriptMessage message;
	std::vector<size_t> stack;
	
public:
	
	Context(EERIE_SCRIPT * script, size_t pos = 0, INTERACTIVE_OBJ * io = NULL, ScriptMessage msg = SM_NULL);
	
	std::string getStringVar(const std::string & var) const;
	std::string getFlags();
	std::string getWord();
	void skipWord();
	
	std::string getCommand(bool skipNewlines = true);
	
	void skipWhitespace(bool skipNewlines = false);
	
	inline INTERACTIVE_OBJ * getIO() const { return io; }
	
	bool getBool();
	
	float getFloat();
	
	float getFloatVar(const std::string & name) const;
	
	/*!
	 * Skip input until the end of the current line.
	 * @return the current position or (size_t)-1 if we are already at the line end
	 */
	size_t skipCommand();
	
	void skipStatement();
	
	bool jumpToLabel(const std::string & target, bool substack = false);
	bool returnToCaller();
	
	inline EERIE_SCRIPT * getScript() const { return script; }
	inline EERIE_SCRIPT * getMaster() const { return script->master ? script->master : script; }
	
	inline size_t getPosition() const { return pos; }
	
	inline ScriptMessage getMessage() const { return message; }
	
	friend class ::ScriptEvent;
};

class Command {
	
	const std::string name;
	const long ioflags;
	
public:
	
	enum Result {
		Success,
		Failed,
		AbortAccept,
		AbortRefuse,
		AbortError,
		Jumped
	};
	
	static const long ANY_IO = -1;
	
	inline Command(const std::string & _name, long _ioflags = 0) : name(_name), ioflags(_ioflags) { }
	
	virtual Result execute(Context & context) = 0;
	
	virtual ~Command() { }
	
	inline const std::string & getName() const { return name; }
	inline long getIOFlags() const { return ioflags; }
	
};

bool isSuppressed(const Context & context, const std::string & command);

bool isBlockEndSuprressed(const Context & context, const std::string & command);

size_t initSuppressions();

#define ScriptContextPrefix(context) '[' << ((context).getIO() ? (((context).getScript() == &(context).getIO()->script) ? (context).getIO()->short_name() : (context).getIO()->long_name()) : "unknown") << ':' << (context).getPosition() << "] "
#define ScriptPrefix ScriptContextPrefix(context) << getName() <<
#define DebugScript(args) LogDebug(ScriptPrefix args)
#define ScriptInfo(args) LogInfo << ScriptPrefix args
#define ScriptWarning Logger(__FILE__,__LINE__, isSuppressed(context, getName()) ? Logger::Debug : Logger::Warning) << ScriptPrefix ": "
#define ScriptError Logger(__FILE__,__LINE__, isSuppressed(context, getName()) ? Logger::Debug : Logger::Error) << ScriptPrefix ": "

#define HandleFlags(expected) string options = context.getFlags(); \
	for(u64 run = !options.empty(), flg; run && ((flg = flags(options), (flg && !(flg & ~flags(expected)))) || (ScriptWarning << "unexpected flags: " << options, true)); run = 0)

} // namespace script

#endif // ARX_SCRIPT_SCRIPTUTILS_H
