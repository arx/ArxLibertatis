
#ifndef ARX_SCRIPTING_SCRIPTEDLANG_H
#define ARX_SCRIPTING_SCRIPTEDLANG_H

#include <string>

namespace script {

class Context;

void setupScriptedLang();

void timerCommand(const std::string & timer, Context & context);

} // namespace script

#endif // ARX_SCRIPTING_SCRIPTEDLANG_H
