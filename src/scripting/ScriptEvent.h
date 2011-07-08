/*
 * ScriptEvent.h
 *
 *  Created on: Jan 31, 2011
 *      Author: bmonkey
 */

#ifndef ARX_SCRIPTING_SCRIPTEVENT_H
#define ARX_SCRIPTING_SCRIPTEVENT_H

#include <map>

#include "scripting/Script.h"

struct SCRIPT_EVENT {
	SCRIPT_EVENT(const std::string & str): name(str) {}
	std::string name;
};

class ScriptEvent;

class ScriptContext {
	
private:
	
	const EERIE_SCRIPT * script;
	size_t pos;
	INTERACTIVE_OBJ * io;
	
public:
	
	ScriptContext(const EERIE_SCRIPT * script, size_t pos = 0, INTERACTIVE_OBJ * io = NULL);
	
	std::string getStringVar(const std::string & var);
	std::string getFlags();
	std::string getWord();
	
	void skipWhitespace();
	
	inline INTERACTIVE_OBJ * getIO() { return io; }
	
	std::string getLowercase();
	
	float getFloat();
	
	float getFloatVar(const std::string & name);
	
	friend class ScriptEvent;
};

class ScriptCommand {
	
public:
	
	virtual ScriptResult execute(ScriptContext & context) = 0;
	
	virtual ~ScriptCommand() { }
	
};

class ScriptEvent {
	
public:
	
	static long totalCount;
	
	ScriptEvent();
	virtual ~ScriptEvent();
	
	static ScriptResult send(EERIE_SCRIPT * es, ScriptMessage msg, const std::string & params, INTERACTIVE_OBJ * io, const std::string & eventname, long info = 0);
	
	static void registerCommand(const std::string & name, ScriptCommand * command);
	
	static void init();
	
private:
	
	typedef std::map<std::string, ScriptCommand *> Commands;
	
	static Commands commands;
	
};

#endif // ARX_SCRIPTING_SCRIPTEVENT_H
