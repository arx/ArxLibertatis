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
	
	static long totalCount;
	
	ScriptEvent();
	virtual ~ScriptEvent();
	static ScriptResult send(EERIE_SCRIPT * es, ScriptMessage msg, const std::string & params, INTERACTIVE_OBJ * io, const std::string & eventname, long info = 0);
	
};

#endif // ARX_SCRIPTING_SCRIPTEVENT_H
