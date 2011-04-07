/*
 * ScriptEvent.h
 *
 *  Created on: Jan 31, 2011
 *      Author: bmonkey
 */

#ifndef ARX_SCRIPTING_SCRIPTEVENT_H
#define ARX_SCRIPTING_SCRIPTEVENT_H

#include "scripting/Script.h"

struct SCRIPT_EVENT {
	SCRIPT_EVENT(const std::string & str): name(str) {}
	std::string name;
};

class ScriptEvent {
public:
	ScriptEvent();
	virtual ~ScriptEvent();
	static long checkInteractiveObject(INTERACTIVE_OBJ * io, long msg);
	static long send(EERIE_SCRIPT * es, long msg, const std::string & params, INTERACTIVE_OBJ * io, const std::string & eventname, long info = 0);
};

#endif // ARX_SCRIPTING_SCRIPTEVENT_H
