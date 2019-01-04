/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include <boost/noncopyable.hpp>

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

/*!
 * Overload to give compilers a chance to calculate flag masks at
 * compile-time for string constants.
 * 
 * This should probably be done using constexpr in c++11.
 * We could force compile-time calculation with a template-based
 * implementation, but that will be much uglier and limited.
 */
template <size_t N>
u64 flags(const char (&flags)[N]) {
	
	u64 result = 0ul;
	for(size_t i = (flags[0] == '-') ? 1 : 0; i < N - 1; i++) {
		result |= flag(flags[i]);
	}
	
	return result;
}

class Context {
	
	const EERIE_SCRIPT * m_script;
	size_t m_pos;
	Entity * m_sender;
	Entity * m_entity;
	ScriptMessage m_message;
	ScriptParameters m_parameters;
	std::vector<size_t> m_stack;
	
public:
	
	explicit Context(const EERIE_SCRIPT * script, size_t pos, Entity * sender, Entity * entity,
	                 ScriptMessage msg, const ScriptParameters & parameters);
	
	std::string getStringVar(const std::string & name) const;
	std::string getFlags();
	std::string getWord();
	void skipWord();
	
	std::string getCommand(bool skipNewlines = true);
	
	void skipWhitespace(bool skipNewlines = false, bool warnNewlines = false);
	
	Entity * getSender() const { return m_sender; }
	Entity * getEntity() const { return m_entity; }
	ScriptMessage getMessage() const { return m_message; }
	const ScriptParameters & getParameters() const { return m_parameters; }
	std::string getParameter(size_t i) const { return m_parameters.get(i); }
	
	bool getBool();
	
	float getFloat();
	
	float getFloatVar(const std::string & name) const;
	
	/*!
	 * Skip input until the end of the current line.
	 * \return the current position or (size_t)-1 if we are already at the line end
	 */
	size_t skipCommand();
	
	void skipStatement();
	
	bool jumpToLabel(const std::string & target, bool substack = false);
	bool returnToCaller();
	
	const EERIE_SCRIPT * getScript() const { return m_script; }
	
	size_t getPosition() const { return m_pos; }
	
	
};

class Command : private boost::noncopyable {
	
	const std::string m_name;
	const long m_entityFlags;
	
public:
	
	enum Result {
		Success,
		Failed,
		AbortAccept,
		AbortRefuse,
		AbortError,
		AbortDestructive,
		Jumped
	};
	
	static const long AnyEntity = -1;
	
	explicit Command(const std::string & name, long entityFlags = 0)
		: m_name(name), m_entityFlags(entityFlags) { }
	
	virtual Result execute(Context & context) = 0;
	
	virtual Result peek(Context & context) {
		
		ARX_UNUSED(context);
		
		return AbortDestructive;
	}
	
	virtual ~Command() { }
	
	const std::string & getName() const { return m_name; }
	long getEntityFlags() const { return m_entityFlags; }
	
};

bool isSuppressed(const Context & context, const std::string & command);

bool isBlockEndSuprressed(const Context & context, const std::string & command);

size_t initSuppressions();

#define ScriptContextPrefix(context) '[' << ((context).getEntity() ? (((context).getScript() == &(context).getEntity()->script) ? (context).getEntity()->className() : (context).getEntity()->idString()) : "unknown") << ':' << (context).getPosition() << "] "
#define ScriptPrefix ScriptContextPrefix(context) << getName() <<
#define DebugScript(args) LogDebug(ScriptPrefix args)
#define ScriptInfo(args) LogInfo << ScriptPrefix args
#define ScriptWarning ARX_LOG(isSuppressed(context, getName()) ? Logger::Debug : Logger::Warning) << ScriptPrefix ": "
#define ScriptError   ARX_LOG(isSuppressed(context, getName()) ? Logger::Debug : Logger::Error) << ScriptPrefix ": "

#define HandleFlags(expected) std::string options = context.getFlags(); \
	for(u64 run = !options.empty(), flg = 0; run && ((flg = flags(options), (flg && !(flg & ~flags(expected)))) || (ScriptWarning << "unexpected flags: " << options, true)); run = 0)

} // namespace script

#endif // ARX_SCRIPT_SCRIPTUTILS_H
