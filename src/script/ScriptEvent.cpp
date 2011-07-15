/*
 * ScriptEvent.cpp
 *
 *  Created on: Jan 31, 2011
 *      Author: bmonkey
 */

#include "script/ScriptEvent.h"

#include <cstdio>
#include <cassert>

#include "ai/Paths.h"

#include "core/GameTime.h"
#include "core/Resource.h"
#include "core/Core.h"

#include "game/Damage.h"
#include "game/Equipment.h"
#include "game/Missile.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "game/Inventory.h"

#include "gui/Speech.h"
#include "gui/MiniMap.h"
#include "gui/Text.h"
#include "gui/Menu.h"

#include "graphics/GraphicsModes.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/Math.h"
#include "graphics/data/MeshManipulation.h"

#include "io/Logger.h"
#include "io/FilePath.h"
#include "io/PakReader.h"

#include "physics/Attractors.h"
#include "physics/CollisionShapes.h"
#include "physics/Collisions.h"

#include "platform/String.h"

#include "scene/Scene.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"
#include "scene/Object.h"
#include "scene/Light.h"

#include "script/ScriptUtils.h"
#include "script/ScriptedCamera.h"
#include "script/ScriptedControl.h"
#include "script/ScriptedInteractiveObject.h"
#include "script/ScriptedInterface.h"
#include "script/ScriptedItem.h"
#include "script/ScriptedLang.h"
#include "script/ScriptedNPC.h"
#include "script/ScriptedPlayer.h"

using std::max;
using std::min;
using std::string;

extern float g_TimeStartCinemascope;

#ifdef NEEDING_DEBUG
long NEED_DEBUG = 0;
#endif // NEEDING_DEBUG

long ScriptEvent::totalCount = 0;

SCRIPT_EVENT AS_EVENT[] =
{
	std::string("ON NULL"),
	std::string("ON INIT"),
	std::string("ON INVENTORYIN"),
	std::string("ON INVENTORYOUT"),
	std::string("ON INVENTORYUSE"),
	std::string("ON SCENEUSE"),
	std::string("ON EQUIPIN"),
	std::string("ON EQUIPOUT"),
	std::string("ON MAIN"),
	std::string("ON RESET"),
	std::string("ON CHAT"),
	std::string("ON ACTION"),
	std::string("ON DEAD"),
	std::string("ON REACHEDTARGET"),
	std::string("ON FIGHT"),
	std::string("ON FLEE"),
	std::string("ON HIT"),
	std::string("ON DIE"),
	std::string("ON LOSTTARGET"),
	std::string("ON TREATIN"),
	std::string("ON TREATOUT"),
	std::string("ON MOVE"),
	std::string("ON DETECTPLAYER"),
	std::string("ON UNDETECTPLAYER"),
	std::string("ON COMBINE"),
	std::string("ON NPC_FOLLOW"),
	std::string("ON NPC_FIGHT"),
	std::string("ON NPC_STAY"),
	std::string("ON INVENTORY2_OPEN"),
	std::string("ON INVENTORY2_CLOSE"),
	std::string("ON CUSTOM"),
	std::string("ON ENTER_ZONE"),
	std::string("ON LEAVE_ZONE"),
	std::string("ON INITEND") ,
	std::string("ON CLICKED") ,
	std::string("ON INSIDEZONE"),
	std::string("ON CONTROLLEDZONE_INSIDE"),
	std::string("ON LEAVEZONE"),
	std::string("ON CONTROLLEDZONE_LEAVE"),
	std::string("ON ENTERZONE"),
	std::string("ON CONTROLLEDZONE_ENTER"),
	std::string("ON LOAD"),
	std::string("ON SPELLCAST"),
	std::string("ON RELOAD"),
	std::string("ON COLLIDE_DOOR"),
	std::string("ON OUCH"),
	std::string("ON HEAR"),
	std::string("ON SUMMONED"),
	std::string("ON SPELLEND"),
	std::string("ON SPELLDECISION"),
	std::string("ON STRIKE"),
	std::string("ON COLLISION_ERROR"),
	std::string("ON WAYPOINT"),
	std::string("ON PATHEND"),
	std::string("ON CRITICAL"),
	std::string("ON COLLIDE_NPC"),
	std::string("ON BACKSTAB"),
	std::string("ON AGGRESSION"),
	std::string("ON COLLISION_ERROR_DETAIL"),
	std::string("ON GAME_READY"),
	std::string("ON CINE_END"),
	std::string("ON KEY_PRESSED"),
	std::string("ON CONTROLS_ON"),
	std::string("ON CONTROLS_OFF"),
	std::string("ON PATHFINDER_FAILURE"),
	std::string("ON PATHFINDER_SUCCESS"),
	std::string("ON TRAP_DISARMED"),
	std::string("ON BOOK_OPEN"),
	std::string("ON BOOK_CLOSE"),
	std::string("ON IDENTIFY"),
	std::string("ON BREAK"),
	std::string("ON STEAL"),
	std::string("ON COLLIDE_FIELD"),
	std::string("ON CURSORMODE"),
	std::string("ON EXPLORATIONMODE"),
	std::string("") // TODO is this really needed?
};

void ARX_SCRIPT_ComputeShortcuts(EERIE_SCRIPT& es)
{
	long nb = min((long)MAX_SHORTCUT, (long)SM_MAXCMD);
	// LogDebug << "Compute " << nb << " shortcuts";

	for (long j = 1; j < nb; j++) {
		es.shortcut[j] = FindScriptPos(&es, AS_EVENT[j].name);

		if (es.shortcut[j] >= 0) {
			std::string dest;
			GetNextWord(&es, es.shortcut[j], dest);

			if (!strcasecmp(dest, "{")) {
				GetNextWord(&es, es.shortcut[j], dest);

				if (!strcasecmp(dest, "ACCEPT")) {
					es.shortcut[j] = -1;
				}
			}
		}

	}
}

void ShowScriptError(const char * tx, const char * cmd)
{
	char text[512];
	sprintf(text, "SCRIPT ERROR\n%s\n\n%s", tx, cmd);
	LogError << (text);
}

ScriptEvent::ScriptEvent() {
	// TODO Auto-generated constructor stub

}

ScriptEvent::~ScriptEvent() {
	// TODO Auto-generated destructor stub
}

static bool checkInteractiveObject(INTERACTIVE_OBJ * io, ScriptMessage msg, ScriptResult & ret) {
	
	io->stat_count++;
	
	if((io->GameFlags & GFLAG_MEGAHIDE) && msg != SM_RELOAD) {
		ret = ACCEPT;
		return true;
	}
	
	if(io->show == SHOW_FLAG_DESTROYED) {
		ret = ACCEPT;
		return true;
	}
	
	if(io->ioflags & IO_FREEZESCRIPT) {
		ret = (msg == SM_LOAD) ? ACCEPT : REFUSE;
		return true;
	}
	
	if(io->ioflags & IO_NPC
	  && io->_npcdata->life <= 0.f
	  && msg != SM_DEAD
	  && msg != SM_DIE
	  && msg != SM_EXECUTELINE
	  && msg != SM_RELOAD
	  && msg != SM_INVENTORY2_OPEN
	  && msg != SM_INVENTORY2_CLOSE) {
		ret = ACCEPT;
		return true;
	}
	
	//change weapons if you break
	if((io->ioflags & IO_FIX || io->ioflags & IO_ITEM) && msg == SM_BREAK) {
		ManageCasseDArme(io);
	}
	
	return false;
}

extern long LINEEND; // set by GetNextWord
extern INTERACTIVE_OBJ * LASTSPAWNED;
extern long PauseScript;

namespace script {

namespace {

class ObsoleteCommand : public Command {
	
private:
	
	size_t nargs;
	
public:
	
	ObsoleteCommand(const string & command, size_t _nargs = 0) : Command(command), nargs(_nargs) { }
	
	Result execute(Context & context) {
		
		for(size_t i = 0; i < nargs; i++) {
			context.skipWord();
		}
		
		LogWarning << "obsolete command: " << getName();
		
		return Failed;
	}
	
	~ObsoleteCommand() { }
	
};

}

} // namespace script

using namespace script; // TODO remove once everythng has been moved to the script namespace

ScriptResult ScriptEvent::send(EERIE_SCRIPT * es, ScriptMessage msg, const std::string& params, INTERACTIVE_OBJ * io, const std::string& evname, long info) {
	
	ScriptResult ret = ACCEPT;
	std::string word = "";
	char cmd[256];
	char eventname[64];
	long brackets = 0;
	long pos;
	
	totalCount++;
	
	if(io && checkInteractiveObject(io, msg, ret)) {
		return ret;
	}

	if ((EDITMODE || PauseScript)
			&& msg != SM_LOAD
			&& msg != SM_INIT
			&& msg != SM_INITEND) {
		return ACCEPT;
	}
	
	// Retrieves in esss script pointer to script holding variables.
	EERIE_SCRIPT * esss = (EERIE_SCRIPT *)es->master;
	if(esss == NULL) {
		esss = es;
	}
	
	// Finds script position to execute code...
	if (!evname.empty()) {
		strcpy(eventname, "on ");
		strcat(eventname, evname.c_str());
		pos = FindScriptPos( es, eventname );
	} else {
		if (msg == SM_EXECUTELINE) {
			pos = info;
		} else {
			switch(msg) {
				case SM_COLLIDE_NPC:
					if (esss->allowevents & DISABLE_COLLIDE_NPC) return REFUSE;
					break;
				case SM_CHAT:
					if (esss->allowevents & DISABLE_CHAT) return REFUSE;
					break;
				case SM_HIT:
					if (esss->allowevents & DISABLE_HIT) return REFUSE;
					break;
				case SM_INVENTORY2_OPEN:
					if (esss->allowevents & DISABLE_INVENTORY2_OPEN) return REFUSE;
					break;
				case SM_HEAR:
					if (esss->allowevents & DISABLE_HEAR) return REFUSE;
					break;
				case SM_UNDETECTPLAYER:
				case SM_DETECTPLAYER:
					if (esss->allowevents & DISABLE_DETECT) return REFUSE;
					break;
				case SM_AGGRESSION:
					if (esss->allowevents & DISABLE_AGGRESSION) return REFUSE;
					break;
				case SM_MAIN:
					if (esss->allowevents & DISABLE_MAIN) return REFUSE;
					break;
				case SM_CURSORMODE:
					if (esss->allowevents & DISABLE_CURSORMODE) return REFUSE;
					break;
				case SM_EXPLORATIONMODE:
					if (esss->allowevents & DISABLE_EXPLORATIONMODE) return REFUSE;
					break;
				case SM_KEY_PRESSED: {
					float dwCurrTime = ARX_TIME_Get();
					if ((dwCurrTime - g_TimeStartCinemascope) < 3000) {
						LogDebug << "refusing SM_KEY_PRESSED";
						return REFUSE;
					}
					break;
				}
				default: break;
			}

			if (msg < (long)MAX_SHORTCUT) {
				pos = es->shortcut[msg];
			} else {
				
				assert(msg != SM_EXECUTELINE && evname.empty());
				
				if(msg >= SM_MAXCMD) {
					LogDebug << "unknown message " << msg;
					return ACCEPT;
				}
				
				// TODO will never be reached as MAX_SHORTCUT > SM_MAXCMD
				
				pos = FindScriptPos(es, AS_EVENT[msg].name);
			}
		}
	}

	if (pos <= -1) {
		// TODO very noisy LogDebug << "cannot find event handler";
		return ACCEPT;
	}
	
	
	LogDebug << "SendScriptEvent msg=" << msg << " ("
	         << (((size_t)msg < sizeof(AS_EVENT)/sizeof(*AS_EVENT) - 1) ? AS_EVENT[msg].name : "unknown")
	         << ")" << " params=\"" << params << "\""
	         << " evame=\"" << evname << "\" info=" << info;
	LogDebug << " io=" << Logger::nullstr(io ? io->filename : NULL)
	         << ":" << io->ident
	         << (io == NULL ? "" : es == &io->script ? " base" : " overriding");

	MakeSSEPARAMS(params.c_str());

	if (msg != SM_EXECUTELINE) {
		if (!evname.empty()) {
			pos += strlen(eventname); // adding 'ON ' length
			LogDebug << eventname << " received";
		} else {
			pos += AS_EVENT[msg].name.length();
			LogDebug << AS_EVENT[msg].name << " received";
		}

		if ((pos = GetNextWord(es, pos, word)) < 0) return ACCEPT;

		if(word.empty() || word[0] != '{') {
			LogError << "ERROR: No bracket after event, got \"" << word << "\"";
			return ACCEPT;
		}
		else brackets = 1;
	} else {
		LogDebug << "EXECUTELINE received";
		brackets = 0;
	}

	while (pos >= 0) {

		cmd[0] = 0;

		if (pos >= (int)es->size - 1)
			return ACCEPT;

		if ((pos = GetNextWord(es, pos, word)) < 0)
			return ACCEPT;

		if ((msg == SM_EXECUTELINE) && (LINEEND == 1))
			return ACCEPT;

		MakeStandard(word);

		//TODO(lubosz): this is one mega switch
		LogDebug << "Switching! current word '" << word << "'";
		
		Commands::const_iterator it = commands.find(toLowercase(word));
		
		if(it != commands.end()) {
			
			Context context(es, pos, io);
			
			context.message = msg;
			
			Command & command = *(it->second);
			
			Command::Result res;
			if(command.getIOFlags() && (!io || (command.getIOFlags() != Command::ANY_IO && !(command.getIOFlags() & io->ioflags)))) {
				LogWarning << "command " << command.getName() << " needs an IO of type " << command.getIOFlags();
				context.skipCommand();
				res = Command::Failed;
			} else {
				res = it->second->execute(context);
			}
			
			pos = context.pos;
			
			LINEEND = (context.pos >= es->size || es->data[pos] == '\n') ? 1 : 0;
			
			if(res == Command::AbortAccept) {
				ClearSubStack(es);
				return ACCEPT;
			} else if(res == Command::AbortRefuse) {
				ClearSubStack(es);
				return REFUSE;
			} else if(res == Command::AbortError) {
				ClearSubStack(es);
				return BIGERROR;
			} else if(res == Command::Jumped) {
				if(msg == SM_EXECUTELINE) {
					msg = SM_DUMMY;
				}
			}
			
		} else switch (word[0]) {
			case '}':
				brackets--;
				break;
			case '{':
				brackets++;
				break;
			case '/':
				if (word[1] == '/') pos = GotoNextLine(es, pos);
				break;
			case '>':
				if (word[1] == '>') pos = GotoNextLine(es, pos);
				break;

			case 'T':

				if ((word[1] == 'I') && (word[2] == 'M') && (word[3] == 'E') && (word[4] == 'R'))
				{
					// Timer -m nbtimes duration commands
					string timername;
					std::string temp2;
					std::string temp3;

					// Checks if the timer is named by caller of if it needs a default name
					if(word.length() > 5) {
						timername = toLowercase(word.substr(5));
					} else {
						timername = ARX_SCRIPT_Timer_GetDefaultName();
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, word);
					}

#endif
					pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcat(cmd, " ");
						strcat(cmd, temp2);
					}

#endif

					// We start by clearing instances of this timer. (Timers are unique by
					// a combination of name & IO).
					if (!strcasecmp(temp2, "KILL_LOCAL"))
					{
						ARX_SCRIPT_Timer_Clear_All_Locals_For_IO(io);
					}
					else
					{
						ARX_SCRIPT_Timer_Clear_By_Name_And_IO(timername, io);

						if (strcasecmp(temp2, "OFF"))
						{
							long mili = 0;
							long idle = 0;
							
							if (temp2[0] == '-')
							{
								if (iCharIn(temp2, 'M'))
									mili = 1;

								if (iCharIn(temp2, 'I'))
									idle = 1;

								pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

								if (NEED_DEBUG)
								{
									strcat(cmd, " ");
									strcat(cmd, temp2);
								}

#endif
							}

							long times = atoi(temp2);
							pos = GetNextWord(es, pos, temp3);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp3);
							}

#endif
							long msecs = atoi(temp3);

							if (!mili) msecs *= 1000;

							long num = ARX_SCRIPT_Timer_GetFree();

							if (num != -1)
							{
								ActiveTimers++;
								scr_timer[num].es = es;
								scr_timer[num].exist = 1;
								scr_timer[num].io = io;
								scr_timer[num].msecs = msecs;
								scr_timer[num].name = timername;
								scr_timer[num].pos = pos;
								scr_timer[num].tim = ARXTimeUL();
								scr_timer[num].times = times;

								if ((idle) && io)
									scr_timer[num].flags = 1;
								else
									scr_timer[num].flags = 0;
							}

							pos = GotoNextLine(es, pos);
						}
					}
				}

				break;
			case 'D':

				if (!strcmp(word, "DAMAGER"))
				{
					io->damager_type = DAMAGE_TYPE_PER_SECOND;
					pos = GetNextWord(es, pos, word);

					if (word[0] == '-')
					{
						if (iCharIn(word, 'F'))
							io->damager_type |= DAMAGE_TYPE_FIRE;

						if (iCharIn(word, 'M'))
							io->damager_type |= DAMAGE_TYPE_MAGICAL;

						if (iCharIn(word, 'P'))
							io->damager_type |= DAMAGE_TYPE_POISON;

						if (iCharIn(word, 'L'))
							io->damager_type |= DAMAGE_TYPE_LIGHTNING;

						if (iCharIn(word, 'C'))
							io->damager_type |= DAMAGE_TYPE_COLD;

						if (iCharIn(word, 'G'))
							io->damager_type |= DAMAGE_TYPE_GAS;

						if (iCharIn(word, 'E'))
							io->damager_type |= DAMAGE_TYPE_METAL;

						if (iCharIn(word, 'W'))
							io->damager_type |= DAMAGE_TYPE_WOOD;

						if (iCharIn(word, 'S'))
							io->damager_type |= DAMAGE_TYPE_STONE;

						if (iCharIn(word, 'A'))
							io->damager_type |= DAMAGE_TYPE_ACID;

						if (iCharIn(word, 'O'))
							io->damager_type |= DAMAGE_TYPE_ORGANIC;

						if (iCharIn(word, 'R'))
							io->damager_type |= DAMAGE_TYPE_DRAIN_LIFE;

						if (iCharIn(word, 'N'))
							io->damager_type |= DAMAGE_TYPE_DRAIN_MANA;

						if (iCharIn(word, 'U'))
							io->damager_type |= DAMAGE_TYPE_PUSH;

						pos = GetNextWord(es, pos, word);
					}

					float fval = GetVarValueInterpretedAsFloat(word, esss, io);



					ARX_CHECK_SHORT(fval);

					io->damager_damages = ARX_CLEAN_WARN_CAST_SHORT(fval);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "DAMAGER");

#endif
				}
				else if (!strcmp(word, "DETACH"))
				{
					pos = GetNextWord(es, pos, word); // Source IO
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "DETACH ");
						strcat(cmd, word);
					}

#endif
					long t = GetTargetByNameTarget(word);

					if (t == -2) t = GetInterNum(io); //self

					pos = GetNextWord(es, pos, word); // target IO
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcat(cmd, " ");
						strcat(cmd, tempo);
					}

#endif
					long t2 = GetTargetByNameTarget(word);

					if (t2 == -2) t2 = GetInterNum(io); //self

					ARX_INTERACTIVE_Detach(t, t2);
				}

				else if (!strcmp(word, "DRAWSYMBOL")) // DRAWSYMBOL symbol duration
				{
					std::string temp1;
					std::string temp2;
					pos = GetNextWord(es, pos, temp1);
					pos = GetNextWord(es, pos, temp2);

					if (io != NULL)
					{
						MakeUpcase(temp1);
						float dur = GetVarValueInterpretedAsFloat(temp2, esss, io);
						ARX_SPELLS_RequestSymbolDraw(io, temp1, dur);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "DRAW_SYMBOL %s %s", temp1, temp2);

#endif
				}

				break;
			default:
				if (io) {
					std::string temp2;
					std::string temp3;
					std::string temp4;
					long ppos = pos;
					pos = GetNextWord(es, pos, temp2);
					pos = GetNextWord(es, pos, temp3);
					GetNextWord(es, pos, temp4);
					sprintf(cmd, "Script Error for token #%ld '%s' (%s|%s|%s) in file %s_%04ld",
							ppos,word.c_str(),temp2.c_str(), temp3.c_str(), temp4.c_str(),GetName(io->filename).c_str(),io->ident);
					LogError << cmd;

					io->ioflags |= IO_FREEZESCRIPT;
					return REFUSE;
				}

				LogError << "unknown command: " << word;
		}

		if (cmd[0] != 0) LogDebug << "CMD " << cmd;

	}

	LogDebug << "goto end";

	if (msg != SM_EXECUTELINE) {
		if (evname != "")
			LogDebug << eventname << " EVENT Successfully Finished";
		else if (msg != SM_DUMMY)
			LogDebug << AS_EVENT[msg].name << " EVENT Successfully Finished";
		else LogDebug << "Dummy EVENT Successfully Finished";
	} else {
		LogDebug << "EXECUTELINE Successfully Finished";
	}

	return ret;
}

void ScriptEvent::registerCommand(Command * command) {
	
	typedef std::pair<Commands::iterator, bool> Res;
	
	Res res = commands.insert(std::make_pair(command->getName(), command));
	
	if(!res.second) {
		LogError << "duplicate script command name: " + command->getName();
		delete command;
	}
	
}

void ScriptEvent::init() {
	
	initSuppressions();
	initAnimationNumbers();
	
	setupScriptedCamera();
	setupScriptedControl();
	setupScriptedInteractiveObject();
	setupScriptedInterface();
	setupScriptedItem();
	setupScriptedLang();
	setupScriptedNPC();
	setupScriptedPlayer();
	
	registerCommand(new ObsoleteCommand("attachnpctoplayer"));
	registerCommand(new ObsoleteCommand("gmode", 1));
	registerCommand(new ObsoleteCommand("setrighthand", 1));
	registerCommand(new ObsoleteCommand("setlefthand", 1));
	registerCommand(new ObsoleteCommand("setshield", 1));
	registerCommand(new ObsoleteCommand("settwohanded"));
	registerCommand(new ObsoleteCommand("setonehanded"));
	registerCommand(new ObsoleteCommand("say"));
	registerCommand(new ObsoleteCommand("setdetachable", 1));
	registerCommand(new ObsoleteCommand("setstackable", 1));
	registerCommand(new ObsoleteCommand("setinternalname", 1));
	registerCommand(new ObsoleteCommand("detachnpcfromplayer"));
	
	LogInfo << "scripting system initialized with " << commands.size() << " commands";
}

ScriptEvent::Commands ScriptEvent::commands;
