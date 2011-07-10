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

extern long FINAL_COMMERCIAL_DEMO;
extern long GLOBAL_MAGIC_MODE;
extern INTERACTIVE_OBJ * CURRENT_TORCH;
extern float InventoryDir;
extern long REFUSE_GAME_RETURN;
extern long FINAL_RELEASE;
extern long TELEPORT_TO_CONFIRM;
extern long CHANGE_LEVEL_ICON;
extern float g_TimeStartCinemascope;

enum ScriptOperator {
	OPER_UNKNOWN = 0,
	OPER_EQUAL = 1,
	OPER_NOTEQUAL = 2,
	OPER_INFEQUAL = 3,
	OPER_INFERIOR = 4,
	OPER_SUPEQUAL = 5,
	OPER_SUPERIOR = 6,
	OPER_INCLASS = 7,
	OPER_ISELEMENT = 8,
	OPER_ISIN = 9,
	OPER_ISTYPE = 10,
	OPER_ISGROUP = 11,
	OPER_NOTISGROUP = 12
};

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

/* TODO broken and not really used
void RemoveNumerics(char * tx)
{
	char dest[512];
	strcpy(dest, tx);
	long			pos			= 0;
	unsigned int	size_dest	= strlen(dest);

	for (unsigned long i = 0 ; i < size_dest ; i++)
	{
		if ((dest[i] < '0')
				&&	(dest[i] > '9')
		   )
		{
			tx[pos++] = dest[i];
		}
	}

	tx[pos] = 0;
}*/

bool IsElement( const char * seek, const char * text)
{
	char tex[1024];
	memcpy(tex, text, 1023);
	tex[1023] = 0;
	char * token = strtok(tex, " ");

	while (token != NULL)
	{
		if (!strcmp(token, seek)) return true;

		token = strtok(NULL, " ");
	}

	return false;

}

bool HasVisibility(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * ioo) {
	
	if(distSqr(io->pos, ioo->pos) > square(20000)) {
		return false;
	}
	
	float ab = MAKEANGLE(io->angle.b);
	float aa = GetAngle(io->pos.x, io->pos.z, ioo->pos.x, ioo->pos.z);
	aa = MAKEANGLE(degrees(aa));

	if((aa < ab + 90.f) && (aa > ab - 90.f)) {
		//font
		ARX_TEXT_Draw(hFontInBook, 300, 320, "VISIBLE", Color::red);
		return true;
	}

	return false;
}

void ShowScriptError(const char * tx, const char * cmd)
{
	char text[512];
	sprintf(text, "SCRIPT ERROR\n%s\n\n%s", tx, cmd);
	LogError << (text);
}

static void Stack_SendMsgToAllNPC_IO(ScriptMessage msg, const char * dat) {
	for(long i = 0; i < inter.nbmax; i++) {
		if(inter.iobj[i] && (inter.iobj[i]->ioflags & IO_NPC)) {
			Stack_SendIOScriptEvent(inter.iobj[i], msg, dat);
		}
	}
}

void SetNextAnim(INTERACTIVE_OBJ * io, ANIM_HANDLE * ea, long layer, long loop, long flags)
{
	if (!ea) return;

	if (!io) return;

	if (IsDeadNPC(io)) return;

	if (!(flags & 1))
		AcquireLastAnim(io);

	FinishAnim(io, io->animlayer[layer].cur_anim);
	ANIM_Set(&io->animlayer[layer], ea);
	io->animlayer[layer].next_anim = NULL;

	if (loop)
		io->animlayer[layer].flags |= EA_LOOP;
	else
		io->animlayer[layer].flags &= ~EA_LOOP;

	io->animlayer[layer].flags |= EA_FORCEPLAY;
}

bool IsGlobal(char c)
{
	if ((c == '$') || (c == '#') || (c == '&')) return true;

	return false;
}

long GetVarNum(SCRIPT_VAR * svf, long* nb, const std::string& name)
{
	if (!svf) return -1;

	for (long i = 0; i < *nb; i++)
	{
		if ((svf[i].type != 0) && (svf[i].name))
		{
			if (!strcmp(name.c_str(), svf[i].name)) return i;
		}
	}

	return -1;
}

bool UNSETVar(SCRIPT_VAR * & svf, long* nb, const std::string& name)
{
	long i = GetVarNum(svf, nb, name);

	if (i < 0) return false;

	long n = *nb;

	if (svf[i].text)
	{
		free((void *)svf[i].text);
		svf[i].text = NULL;
	}

	if (n - 1 - i > 0)
	{
		memcpy(&svf[i], &svf[i+1], sizeof(SCRIPT_VAR)*(n - i - 1));
	}

	svf = (SCRIPT_VAR *)realloc(svf, sizeof(SCRIPT_VAR) * (n - 1));
	(*nb)--;
	return true;
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
		strcpy(eventname, "ON ");
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

				break;
			case 'S':

				if (!strcmp(word, "SETSECRET")) // -1 = off
				{
					pos = GetNextWord(es, pos, word);

					if ((io) && (io->ioflags & IO_FIX))
					{
						if (!strcasecmp(word, "off"))
						{
							io->secretvalue = -1;
						}
						else
						{
							io->secretvalue = (char)GetVarValueInterpretedAsFloat(word, esss, io);

							if (io->secretvalue < -1) io->secretvalue = -1;

							if (io->secretvalue > 100) io->secretvalue = 100;
						}
					}
				}
				else if (!strcmp(word, "SETDETECT")) // -1 = off
				{
					pos = GetNextWord(es, pos, word);

					if ((io) && (io->ioflags & IO_NPC))
					{
						if (!strcasecmp(word, "off"))
						{
							io->_npcdata->fDetect = -1;
						}
						else
						{
							io->_npcdata->fDetect = (char)GetVarValueInterpretedAsFloat(word, esss, io);

							if (io->_npcdata->fDetect < -1)	io->_npcdata->fDetect = -1;

							if (io->_npcdata->fDetect > 100) io->_npcdata->fDetect = 100;
						}
					}
				}
				else if (!strcmp(word, "SETSTEAL")) // -1 = off
				{
					pos = GetNextWord(es, pos, word);

					if ((io) && (io->ioflags & IO_ITEM))
					{
						if (!strcasecmp(word, "off"))
						{
							io->_itemdata->stealvalue = -1;
						}
						else
						{
							io->_itemdata->stealvalue = (char)GetVarValueInterpretedAsFloat(word, esss, io);

							if (io->_itemdata->stealvalue < -1)	io->_itemdata->stealvalue = -1;

							if (io->_itemdata->stealvalue > 100) io->_itemdata->stealvalue = 100;

							if (io->_itemdata->stealvalue == 100) io->_itemdata->stealvalue = -1;
						}
					}
				}
				else if (!strcmp(word, "SETLIGHT")) // -1 = off  for ITEM only
				{
					pos = GetNextWord(es, pos, word);

					if ((io) && (io->ioflags & IO_ITEM))
					{
						if (!strcasecmp(word, "off"))
						{
							io->_itemdata->stealvalue = -1;
						}
						else
						{
							io->_itemdata->LightValue = (char)GetVarValueInterpretedAsFloat(word, esss, io);

							if (io->_itemdata->LightValue < -1)  io->_itemdata->LightValue = -1;

							if (io->_itemdata->LightValue > 1) io->_itemdata->LightValue = 1;
						}
					}
				}
				else if (!strcmp(word, "SETBLOOD"))
				{
					pos = GetNextWord(es, pos, word);
					float r = GetVarValueInterpretedAsFloat(word, esss, io);
					pos = GetNextWord(es, pos, word);
					float g = GetVarValueInterpretedAsFloat(word, esss, io);
					pos = GetNextWord(es, pos, word);
					float b = GetVarValueInterpretedAsFloat(word, esss, io);

					if(io->ioflags & IO_NPC) {
						io->_npcdata->blood_color = Color3f(r, g, b).to<u8>();
					}
				}
				else if (!strcmp(word, "SETMATERIAL"))
				{
					pos = GetNextWord(es, pos, word);

					if (io)
					{
						io->material = ARX_MATERIAL_GetIdByName(word);
					}

					LogDebug <<  "SET_MATERIAL "<< word;
				}
				else if (!strcmp(word, "SETSPEAKPITCH"))
				{
					pos = GetNextWord(es, pos, word);

					if ((io) && (io->ioflags & IO_NPC))
					{
						io->_npcdata->speakpitch = GetVarValueInterpretedAsFloat(word, esss, io);

						if (io->_npcdata->speakpitch < 0.6f) io->_npcdata->speakpitch = 0.6f;
					}

					LogDebug <<  "SET_SPEAK_PITCH "<< word;
				}
				else if (!strcmp(word, "SETFOOD"))
				{
					pos = GetNextWord(es, pos, word);

					if (io->ioflags & IO_ITEM)
					{
						io->_itemdata->food_value = (char)GetVarValueInterpretedAsFloat(word, esss, io);
					}
				}
				else if (!strcmp(word, "SETSPEED"))
				{
					pos = GetNextWord(es, pos, word);
					io->basespeed = GetVarValueInterpretedAsFloat(word, esss, io);

					if (io->basespeed < 0.f) io->basespeed = 0.f;

					if (io->basespeed > 10.f) io->basespeed = 10.f;
				}
				else if (!strcmp(word, "SETSTAREFACTOR"))
				{
					pos = GetNextWord(es, pos, word);

					if ((io) && (io->ioflags & IO_NPC))
					{
						io->_npcdata->stare_factor = GetVarValueInterpretedAsFloat(word, esss, io);
					}
				}
				else if (!strcmp(word, "SETGROUP"))
				{
					pos = GetNextWord(es, pos, word);
					long remove = 0;

					if (word[0] == '-')
					{
						if (iCharIn(word, 'R'))
							remove = 1;

						pos = GetNextWord(es, pos, word);
					}

					std::string temp1 = GetVarValueInterpretedAsText(word, esss, io);

					if (remove)
					{
						if (!strcasecmp(temp1, "DOOR")) io->GameFlags &= ~GFLAG_DOOR;

						ARX_IOGROUP_Remove(io, temp1);
					}
					else
					{
						if (!strcasecmp(temp1, "DOOR")) io->GameFlags |= GFLAG_DOOR;

						ARX_IOGROUP_Add(io, temp1);
					}

					LogDebug <<  "SET_GROUP "<< word;
				}
				else if (!strcmp(word, "SETNPCSTAT"))
				{
					std::string temp2;
					pos = GetNextWord(es, pos, word);
					pos = GetNextWord(es, pos, temp2);
					ARX_NPC_SetStat( *io, word, GetVarValueInterpretedAsFloat(temp2, esss, io));
					LogDebug <<  "SET_NPC_STAT "<< word;
				}
				else if (!strcmp(word, "SETXPVALUE"))
				{
					pos = GetNextWord(es, pos, word);

					if (io && (io->ioflags & IO_NPC))
					{
						io->_npcdata->xpvalue = (long)GetVarValueInterpretedAsFloat(word, esss, io);

						if (io->_npcdata->xpvalue < 0) io->_npcdata->xpvalue = 0;
					}

					LogDebug << "SET_XP_VALUE " << word;
				}
				else if (!strcmp(word, "SETNAME"))
				{
					pos = GetNextWord(es, pos, word);

					if (io != NULL)
					{
						strcpy(io->locname, word.c_str());
					}

					LogDebug <<  "SETNAME "<< word;
				}
				else if (!strcmp(word, "SETPLAYERTWEAK"))
				{
					pos = GetNextWord(es, pos, word);
					LogDebug <<  "SET_PLAYER_TWEAK "<< word;

					if (io->tweakerinfo == NULL)
					{
						io->tweakerinfo = (IO_TWEAKER_INFO *)malloc(sizeof(IO_TWEAKER_INFO));

						if (io->tweakerinfo)
						{
							memset(io->tweakerinfo, 0, sizeof(IO_TWEAKER_INFO));
						}
					}

					if (!strcasecmp(word, "SKIN"))
					{
						std::string temp2;
						pos = GetNextWord(es, pos, word);
						LogDebug << word;
						pos = GetNextWord(es, pos, temp2);
						LogDebug << temp2;

						if (io->tweakerinfo)
						{
							strcpy(io->tweakerinfo->skintochange, word.c_str());
							strcpy(io->tweakerinfo->skinchangeto, temp2.c_str());
						}
					}
					else	// Mesh Tweaker...
					{
						pos = GetNextWord(es, pos, word);
						LogDebug << word;

						if (io->tweakerinfo)
						{
							strcpy(io->tweakerinfo->filename, word.c_str());
						}
					}
				}
				else if (!strcmp(word, "SETCONTROLLEDZONE"))
				{
					pos = GetNextWord(es, pos, word);
					ARX_PATH * ap = ARX_PATH_GetAddressByName(word);

					if (ap != NULL)
					{
						std::string str = io->long_name();
						strcpy(ap->controled, str.c_str() );
					}

					LogDebug << "SET_CONTROLLED_ZONE "<< word;
				}
				else if ((!strcmp(word, "SETSTATUS")) || (!strcmp(word, "SETMAINEVENT")))
				{
					pos = GetNextWord(es, pos, word);
					ARX_SCRIPT_SetMainEvent(io, word);
					LogDebug << "SETMAINEVENT "<< word;
				}
				else if (!strcmp(word, "SETMOVEMODE"))
				{
					pos = GetNextWord(es, pos, word);

					if ((io != NULL) && (io->ioflags & IO_NPC))
					{
						if (!strcmp(word, "WALK"))	ARX_NPC_ChangeMoveMode(io, WALKMODE);

						if (!strcmp(word, "RUN"))	ARX_NPC_ChangeMoveMode(io, RUNMODE);

						if (!strcmp(word, "NONE"))	ARX_NPC_ChangeMoveMode(io, NOMOVEMODE);

						if (!strcmp(word, "SNEAK"))	ARX_NPC_ChangeMoveMode(io, SNEAKMODE);
					}

					LogDebug << "SETMOVEMODE "<< word;
				}
				else if (!strcmp(word, "SPAWN"))
				{
					std::string temp2;
					std::string tmptext;
					pos = GetNextWord(es, pos, word);
					LogDebug <<  "SPAWN "<< word;
					MakeUpcase(word);

					if (!strcmp(word, "NPC"))
					{
						pos = GetNextWord(es, pos, word); // object to spawn.
						LogDebug <<  word;
						pos = GetNextWord(es, pos, temp2); // object ident for position
						LogDebug << temp2;
						long t = GetTargetByNameTarget(temp2);

						if (t == -2) t = GetInterNum(io);

						if ((t >= 0) && (t < inter.nbmax))
						{
							char tex2[256];
							sprintf(tex2, "Graph\\Obj3D\\Interactive\\NPC\\%s", word.c_str());
							File_Standardize(tex2, tmptext);
							INTERACTIVE_OBJ * ioo;

							if (FORBID_SCRIPT_IO_CREATION == 0)
							{
								ioo = AddNPC(tmptext, IO_IMMEDIATELOAD);

								if (ioo)
								{
									LASTSPAWNED = ioo;
									ioo->scriptload = 1;
									ioo->pos = inter.iobj[t]->pos;

									ioo->angle = inter.iobj[t]->angle;
									MakeTemporaryIOIdent(ioo);
									SendInitScriptEvent(ioo);

									if(inter.iobj[t]->ioflags & IO_NPC) {
										float dist = inter.iobj[t]->physics.cyl.radius + ioo->physics.cyl.radius + 10;
										ioo->pos.x += -EEsin(radians(inter.iobj[t]->angle.b)) * dist;
										ioo->pos.z += EEcos(radians(inter.iobj[t]->angle.b)) * dist;
									}

									TREATZONE_AddIO(ioo, GetInterNum(ioo));
								}
							}
						}
					}
					else if (!strcmp(word, "ITEM"))
					{
						pos = GetNextWord(es, pos, word); // object to spawn.
						LogDebug << word;
						pos = GetNextWord(es, pos, temp2); // object ident for position
						LogDebug <<  temp2;
						long t = GetTargetByNameTarget(temp2);

						if (t == -2) t = GetInterNum(io);

						if ((t >= 0) && (t < inter.nbmax))
						{
							char tex2[256];
							sprintf(tex2, "Graph\\Obj3D\\Interactive\\ITEMS\\%s", word.c_str());
							File_Standardize(tex2, tmptext);
							INTERACTIVE_OBJ * ioo;

							if (FORBID_SCRIPT_IO_CREATION == 0)
							{
								ioo = AddItem( tmptext, IO_IMMEDIATELOAD);

								if (ioo)
								{
									MakeTemporaryIOIdent(ioo);
									LASTSPAWNED = ioo;
									ioo->scriptload = 1;
									ioo->pos = inter.iobj[t]->pos;
									ioo->angle = inter.iobj[t]->angle;
									MakeTemporaryIOIdent(ioo);
									SendInitScriptEvent(ioo);
								}

								TREATZONE_AddIO(ioo, GetInterNum(ioo));
							}
						}
					}
					else if (!strcmp(word, "FIREBALL"))
					{
						GetTargetPos(io);
						Vec3f pos = io->pos;

						if (io->ioflags & IO_NPC) pos.y -= 80.f;

						ARX_MISSILES_Spawn(io, MISSILE_FIREBALL, &pos, &io->target);
					}
				}
				else if (!strcmp(word, "SETOBJECTTYPE"))
				{
					pos = GetNextWord(es, pos, word);
					long val = 1; // flag to add

					if (word[0] == '-')
					{
						if (iCharIn(word, 'R'))
						{
							val = 0; // flag to remove
						}

						pos = GetNextWord(es, pos, word);
					}

					ARX_EQUIPMENT_SetObjectType(io, word, val);
					LogDebug <<  "SET_OBJECT_TYPE "<< word;
				}
				else if (!strcmp(word, "SETRIGHTHAND"))
				{
					pos = GetNextWord(es, pos, word);
					LogDebug <<  "SET_RIGHT_HAND ...OBSOLETE...: " << word;
				}
				else if (!strcmp(word, "SETHUNGER"))
				{
					pos = GetNextWord(es, pos, word);
					player.hunger = GetVarValueInterpretedAsFloat(word, esss, io);
					LogDebug <<  "SET_HUNGER "<< word;
				}

				else if (!strcmp(word, "SETLEFTHAND"))
				{
					pos = GetNextWord(es, pos, word);
					LogDebug <<  "SET_LEFT_HAND ...OBSOLETE...: "<< word;
				}
				else if (!strcmp(word, "SETSHIELD"))
				{
					pos = GetNextWord(es, pos, word);
					LogDebug <<  "SET_SHIELD ...OBSOLETE...: "<< word;
				}
				else if (!strcmp(word, "SETTWOHANDED"))
				{
					LogDebug <<  "SET_TWO_HANDED ...OBSOLETE...";
				}
				else if (!strcmp(word, "SETINTERACTIVITY"))
				{
					pos = GetNextWord(es, pos, word);

					if (!strcasecmp(word, "NONE"))
					{
						io->GameFlags &= ~GFLAG_INTERACTIVITY;
						io->GameFlags &= ~GFLAG_INTERACTIVITYHIDE;
					}
					else if (!strcasecmp(word, "HIDE"))
					{
						io->GameFlags &= ~GFLAG_INTERACTIVITY;
						io->GameFlags |= GFLAG_INTERACTIVITYHIDE;
					}
					else
					{
						io->GameFlags |= GFLAG_INTERACTIVITY;
						io->GameFlags &= ~GFLAG_INTERACTIVITYHIDE;
					}

					LogDebug <<  "SET_INTERACTIVITY "<< word;
				}
				else if (!strcmp(word, "SETEQUIP"))
				{
					std::string temp2;
					std::string temp3;
					pos = GetNextWord(es, pos, temp3);
					LogDebug <<  "SET_EQUIP "<< temp3;

					if (temp3[0] == '-')
					{
						if (!strcasecmp(temp3, "-r"))  ARX_EQUIPMENT_Remove_All_Special(io);
						else
						{
							pos = GetNextWord(es, pos, word);
							LogDebug <<  word;
							pos = GetNextWord(es, pos, temp2);
							LogDebug <<  temp2;
						}
					}
					else
					{
						word = temp3;
						pos = GetNextWord(es, pos, temp2);
						temp3.clear();
					}

					short flag = 0;

					if (!temp2.empty())
					{
						if (temp2[temp2.length()-1] == '%') flag = 1;
					}
					else flag = 0;

					ARX_EQUIPMENT_SetEquip(io, temp3, word, GetVarValueInterpretedAsFloat(temp2, esss, io), flag);
				}
				else if (!strcmp(word, "SETONEHANDED"))
				{

					LogDebug <<  "SET_ONE_HANDED ...OBSOLETE...";
				}
				else if (!strcmp(word, "SETWEAPON"))
				{
					pos = GetNextWord(es, pos, word);
					io->GameFlags &= ~GFLAG_HIDEWEAPON;

					if (word[0] == '-')
					{
						if (iCharIn(word, 'H'))	// Hide Weapon
						{
							io->GameFlags |= GFLAG_HIDEWEAPON;
						}

						pos = GetNextWord(es, pos, word);
					}

					if ((io) && (io->ioflags & IO_NPC))
					{
						// temporarily removed for Alpha
						strcpy(io->_npcdata->weaponname, word.c_str());
						Prepare_SetWeapon(io, word);
					}

					LogDebug <<  "SET_WEAPON "<< word;
				}
				else if (!strcmp(word, "SETLIFE"))
				{
					pos = GetNextWord(es, pos, word);

					if (io != NULL)
					{
						if (io->ioflags & IO_NPC)
						{
							io->_npcdata->maxlife = io->_npcdata->life = GetVarValueInterpretedAsFloat(word, esss, io);
						}
					}

					LogDebug <<  "SET_LIFE "<< word;
				}
				else if (!strcmp(word, "SETDURABILITY"))
				{
					long current = 0;
					pos = GetNextWord(es, pos, word);

					if (word[0] == '-')
					{
						if (iCharIn(word, 'C')) current = 1;

						pos = GetNextWord(es, pos, word);
					}

					if (io != NULL)
					{
						if (!(io->ioflags & IO_NPC))
						{
							if (current) io->durability = GetVarValueInterpretedAsFloat(word, esss, io);
							else io->max_durability = io->durability = GetVarValueInterpretedAsFloat(word, esss, io);
						}
					}

					LogDebug <<  "SET_LIFE "<< word;
				}
				else if (!strcmp(word, "SETPATH"))
				{
					long wormspecific = 0;
					long followdir = 0;
					pos = GetNextWord(es, pos, word);

					if (word[0] == '-')
					{
						if (iCharIn(word, 'W'))
						{
							wormspecific = 1;
						}

						if (iCharIn(word, 'F'))
						{
							followdir = 1;
						}

						pos = GetNextWord(es, pos, word);
					}

					if (io != NULL)
					{

						if (!strcasecmp(word, "NONE"))
						{
							if (io->usepath != NULL)
							{
								free(io->usepath);
								io->usepath = NULL;
							}
						}
						else
						{
							ARX_PATH * ap = ARX_PATH_GetAddressByName(word);

							if(ap) {
								
								if(io->usepath != NULL) {
									free(io->usepath);
									io->usepath = NULL;
								}

								ARX_USE_PATH * aup = (ARX_USE_PATH *)malloc(sizeof(ARX_USE_PATH));
								aup->_starttime = aup->_curtime = ARXTime;
								aup->aupflags = ARX_USEPATH_FORWARD;

								if (wormspecific)
								{
									aup->aupflags |= ARX_USEPATH_WORM_SPECIFIC | ARX_USEPATH_FLAG_ADDSTARTPOS;
								}

								if (followdir) aup->aupflags |= ARX_USEPATH_FOLLOW_DIRECTION;

								aup->initpos = io->initpos;
								aup->lastWP = -1;
								aup->path = ap;
								io->usepath = aup;
							}
						}
					}

					LogDebug << "SET_PATH "<< word;
				}
				else if (!strcmp(word, "SETTARGET"))
				{
					pos = GetNextWord(es, pos, word);
					LogDebug << "SET_TARGET "<< word;
					if ((io) && (io->ioflags & IO_NPC))
					{
						io->_npcdata->pathfind.flags &= ~PATHFIND_ALWAYS;
						io->_npcdata->pathfind.flags &= ~PATHFIND_ONCE;
						io->_npcdata->pathfind.flags &= ~PATHFIND_NO_UPDATE;
					}

					if (word[0] == '-')
					{
						if ((io) && (io->ioflags & IO_NPC))
						{
							if (iCharIn(word, 'S'))
								io->_npcdata->pathfind.flags |= PATHFIND_ONCE;

							if (iCharIn(word, 'A'))
								io->_npcdata->pathfind.flags |= PATHFIND_ALWAYS;

							if (iCharIn(word, 'N'))
								io->_npcdata->pathfind.flags |= PATHFIND_NO_UPDATE;
						}

						pos = GetNextWord(es, pos, word);
						LogDebug << word;
					}

					std::string temp1 = GetVarValueInterpretedAsText(word, esss, io);

					if (io != NULL)
					{
						long old_target = -12;

						if ((io) && (io->ioflags & IO_NPC))
						{
							if (io->_npcdata->reachedtarget)
								old_target = io->targetinfo;

							if ((io->_npcdata->behavior & BEHAVIOUR_FLEE) ||
									(io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND))
								old_target = -12;
						}

						if (!strcasecmp(word, "OBJECT"))
						{
							pos = GetNextWord(es, pos, word);
							LogDebug <<  word;
							temp1 = GetVarValueInterpretedAsText(word, esss, io);
						}

						long t = GetTargetByNameTarget(temp1);

						if (t == -2) t = GetInterNum(io);

						if (io->ioflags & IO_CAMERA)
						{
							EERIE_CAMERA * cam = (EERIE_CAMERA *)io->_camdata;
							cam->translatetarget = Vec3f::ZERO;
						}

						if (ValidIONum(t))
						{
							io->targetinfo = t; //TARGET_PATH;
							GetTargetPos(io);
						}

						if (!strcasecmp(temp1, "PATH"))
						{
							io->targetinfo = TARGET_PATH;
							GetTargetPos(io);
						}
						else if (!strcasecmp(temp1, "NONE"))
						{
							io->targetinfo = TARGET_NONE;
						}

						if (old_target != t)
						{
							if (io->ioflags & IO_NPC) io->_npcdata->reachedtarget = 0;

							ARX_NPC_LaunchPathfind(io, t);
						}
					}
				}
				else if (!strcmp(word, "STARTTIMER"))
				{
					pos = GetNextWord(es, pos, word);
					long t = -1;

					if (!strcmp(word, "TIMER1")) t = 0;

					if (!strcmp(word, "TIMER2")) t = 1;

					if (!strcmp(word, "TIMER3")) t = 2;

					if (!strcmp(word, "TIMER4")) t = 3;

					if (t > -1)
					{
						esss->timers[t] = ARXTimeUL();

						if (esss->timers[t] == 0) esss->timers[t] = 1;
					}

					LogDebug <<  "START_TIMER "<< word;
				}
				else if (!strcmp(word, "STOPTIMER"))
				{
					pos = GetNextWord(es, pos, word);
					long t = -1;

					if (!strcmp(word, "TIMER1")) t = 0;

					if (!strcmp(word, "TIMER2")) t = 1;

					if (!strcmp(word, "TIMER3")) t = 2;

					if (!strcmp(word, "TIMER4")) t = 3;

					if (t > -1)
					{
						esss->timers[t] = 0;
					}

					LogDebug <<  "STOP_TIMER "<< word;
				}
				else if (!strcmp(word, "SENDEVENT"))
				{
					std::string evt;
					std::string temp1;
					std::string temp2;
					std::string temp3;
					char zonename[128];
					pos = GetNextWord(es, pos, temp1);
					LogDebug << "SEND_EVENT "<< word;
					long radius = 0;
					long zone = 0;
					long group = 0;
					char groupname[64];
#define SEND_NPC	1
#define SEND_ITEM	2
#define SEND_FIX	4
					long sendto = SEND_NPC;

					if (temp1[0] == '-')
					{
						if (iCharIn(temp1, 'G'))
							group = 1;

						if (iCharIn(temp1, 'F'))
							sendto = SEND_FIX;

						if (iCharIn(temp1, 'I'))
						{
							if (sendto == SEND_NPC)
								sendto = SEND_ITEM;
							else sendto |= SEND_ITEM;
						}

						if (iCharIn(temp1, 'N'))
							sendto |= SEND_NPC;

						if (iCharIn(temp1, 'R'))
							radius = 1;

						if (iCharIn(temp1, 'Z'))
						{
							zone = 1;
						}

						pos = GetNextWord(es, pos, temp1);
						LogDebug << temp1;

						if (group)
						{
							std::string temp6 = GetVarValueInterpretedAsText(temp1, esss, io);
							strcpy(groupname, temp6.c_str());
							pos = GetNextWord(es, pos, temp1);
							LogDebug << temp1;						}
					}

					float rad = 0;

					if ((group) && (!zone) && (!radius))
					{
					}
					else
					{
						pos = GetNextWord_Interpreted(io, es, pos, temp2);

						LogDebug << temp2;
						if (zone) strcpy(zonename, temp2.c_str());

						if (radius) rad = GetVarValueInterpretedAsFloat(temp2, esss, io);
					}

					pos = GetNextWord(es, pos, temp3);

					LogDebug <<  temp3;

					long i = 0;

					while (i < SM_MAXCMD)
					{
						if (!strcmp(temp1, AS_EVENT[i].name.c_str() + 3))
						{
							break;
						}

						i++;
					}

					if (i >= SM_MAXCMD)
					{
						evt = temp1;
					}
					else
					{
						evt = AS_EVENT[i].name.c_str() + 3;
					}

					INTERACTIVE_OBJ * oes = EVENT_SENDER;
					EVENT_SENDER = io;

					if (radius)   // SEND EVENT TO ALL OBJECTS IN A RADIUS
					{
						Vec3f _pos, _pos2;

						for (long l = 0 ; l < inter.nbmax ; l++)
						{
							if ((inter.iobj[l] != NULL)
									&& (inter.iobj[l] != io)
									&&	!(inter.iobj[l]->ioflags & IO_CAMERA)
									&&	!(inter.iobj[l]->ioflags & IO_MARKER)
									&& ((!group) || (IsIOGroup(inter.iobj[l], groupname)))
							   )
							{
								if (((sendto & SEND_NPC) && (inter.iobj[l]->ioflags & IO_NPC))
										||	((sendto & SEND_FIX) && (inter.iobj[l]->ioflags & IO_FIX))
										||	((sendto & SEND_ITEM) && (inter.iobj[l]->ioflags & IO_ITEM)))
								{
									GetItemWorldPosition(inter.iobj[l], &_pos);
									GetItemWorldPosition(io, &_pos2);

									if(distSqr(_pos, _pos2) <= square(rad)) {
										io->stat_sent++;
										Stack_SendIOScriptEvent(inter.iobj[l], SM_NULL, temp3, evt);
									}
								}
							}
						}
					}
					else if (zone) // SEND EVENT TO ALL OBJECTS IN A ZONE
					{
						ARX_PATH * ap = ARX_PATH_GetAddressByName(zonename);

						if (ap != NULL)
						{
							Vec3f _pos;

							for (long l = 0; l < inter.nbmax; l++)
							{
								if ((inter.iobj[l])
										&&	!(inter.iobj[l]->ioflags & IO_CAMERA)
										&&	!(inter.iobj[l]->ioflags & IO_MARKER)
										&&	((!group) || (IsIOGroup(inter.iobj[l], groupname)))
								   )
								{
									if (((sendto & SEND_NPC) && (inter.iobj[l]->ioflags & IO_NPC))
											||	((sendto & SEND_FIX) && (inter.iobj[l]->ioflags & IO_FIX))
											||	((sendto & SEND_ITEM) && (inter.iobj[l]->ioflags & IO_ITEM)))
									{
										GetItemWorldPosition(inter.iobj[l], &_pos);

										if(ARX_PATH_IsPosInZone(ap, _pos.x, _pos.y, _pos.z)) {
											io->stat_sent++;
											Stack_SendIOScriptEvent(inter.iobj[l], SM_NULL, temp3, evt);
										}
									}
								}
							}
						}
					}
					else if	(group) // sends an event to all members of a group
					{
						for (long l = 0; l < inter.nbmax; l++)
						{
							if((inter.iobj[l] != NULL) && (inter.iobj[l] != io) && (IsIOGroup(inter.iobj[l], groupname))) {
								io->stat_sent++;
								Stack_SendIOScriptEvent(inter.iobj[l], SM_NULL, temp3, evt);
							}
						}
					}
					else // SINGLE OBJECT EVENT
					{
						long t = GetTargetByNameTarget(temp2);

						if (t == -2) t = GetInterNum(io);



						if(ValidIONum(t)) {
							io->stat_sent++;
							Stack_SendIOScriptEvent(inter.iobj[t], SM_NULL, temp3, evt);
						}
					}

					EVENT_SENDER = oes;
				}
				else if (!strcmp(word, "SET"))
				{
					std::string temp2;
					char tempp[256];
					long ival;
					float fval;
					SCRIPT_VAR * sv = NULL;
					//long a = 0;
					pos = GetNextWord(es, pos, word, 1);
					LogDebug <<  "SET "<< word;

					if (word[0] == '-')
					{
						if (iCharIn(word, 'A')) {
							LogWarning << "Broken 'set -a' script command used.";
							//a = 1;
						}

						pos = GetNextWord(es, pos, word, 1);
						LogDebug <<  word;
					}

					pos = GetNextWord(es, pos, temp2);

					switch (word[0])
					{
						case '$': // GLOBAL TEXT
							strcpy(tempp, GetVarValueInterpretedAsText(temp2, esss, io).c_str());

							//if (a) RemoveNumerics(tempp);

							sv = SETVarValueText(svar, NB_GLOBALS, word, tempp);

							if (sv == NULL)
							{
								ShowScriptError("Unable to Set Variable Value", cmd);
							}
							else sv->type = TYPE_G_TEXT;

							break;
						case '\xA3': // LOCAL TEXT
							strcpy(tempp, GetVarValueInterpretedAsText(temp2, esss, io).c_str());

							//if (a) RemoveNumerics(tempp);

							sv = SETVarValueText(esss->lvar, esss->nblvar, word, tempp);

							if (sv == NULL)
							{
								ShowScriptError("Unable to Set Variable Value", cmd);
							}
							else sv->type = TYPE_L_TEXT;

							break;
						case '#': // GLOBAL LONG
							ival = (long)GetVarValueInterpretedAsFloat(temp2, esss, io);
							sv = SETVarValueLong(svar, NB_GLOBALS, word, ival);

							if (sv == NULL)
							{
								ShowScriptError("Unable to Set Variable Value", cmd);
							}
							else sv->type = TYPE_G_LONG;

							break;
						case '\xA7': // LOCAL LONG
							ival = (long)GetVarValueInterpretedAsFloat(temp2, esss, io);
							sv = SETVarValueLong(esss->lvar, esss->nblvar, word, ival);

							if (sv == NULL)
							{
								ShowScriptError("Unable to Set Variable Value", cmd);
							}
							else sv->type = TYPE_L_LONG;

							break;
						case '&': // GLOBAL float
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							sv = SETVarValueFloat(svar, NB_GLOBALS, word, fval);

							if (sv == NULL)
							{
								ShowScriptError("Unable to Set Variable Value", cmd);
							}
							else sv->type = TYPE_G_FLOAT;

							break;
						case '@': // LOCAL float
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							sv = SETVarValueFloat(esss->lvar, esss->nblvar, word, fval);

							if (sv == NULL)
							{
								ShowScriptError("Unable to Set Variable Value", cmd);
							}
							else sv->type = TYPE_L_FLOAT;

							break;
					}

					LogDebug <<  "SET "<<word<<" "<< temp2;
				}
				else if (!strcmp(word, "SAY"))
				{
					//DO NOTHING
				}
				else if (!strcmp(word, "SETPLAYERCOLLISION"))
				{
					pos = GetNextWord(es, pos, word);
					MakeUpcase(word);

					if (!strcmp(word, "ON"))	io->collision |= 1;
					else	io->collision &= ~1;

					LogDebug << "SET_PLAYER_COLLISION "<< word;
				}
				else if (!strcmp(word, "SETSTEPMATERIAL"))
				{
					pos = GetNextWord(es, pos, word);

					if (io)
					{
						if (io->stepmaterial)
						{
							free((void *)io->stepmaterial);
							io->stepmaterial = NULL;
						}

						io->stepmaterial = (char *)malloc(word.length() + 1);
						strcpy(io->stepmaterial, word.c_str());
					}

					LogDebug << "SET_STEP_MATERIAL "<< word;
				}
				else if (!strcmp(word, "SETARMORMATERIAL"))
				{
					pos = GetNextWord(es, pos, word);

					if (io)
					{
						if (io->armormaterial)
						{
							free((void *)io->armormaterial);
							io->armormaterial = NULL;
						}

						io->armormaterial = (char *)malloc(word.length() + 1);
						strcpy(io->armormaterial, word.c_str());
					}

					LogDebug << "SET_ARMOR_MATERIAL "<< word;
				}
				else if (!strcmp(word, "SETWEAPONMATERIAL"))
				{
					pos = GetNextWord(es, pos, word);

					if (io)
					{
						if (io->weaponmaterial)
							free(io->weaponmaterial);

						io->weaponmaterial = NULL;
						io->weaponmaterial = strdup(word.c_str());
					}

					LogDebug << "SET_STEP_MATERIAL "<< word;
				}
				else if (!strcmp(word, "SETSTRIKESPEECH"))
				{
					pos = GetNextWord(es, pos, word);

					if (io)
					{
						if (io->strikespeech) free(io->strikespeech);

						io->strikespeech = NULL;
						io->strikespeech = strdup(word.c_str());
					}

					LogDebug << "SET_STEP_MATERIAL "<< word;
				}
				else if (!strcmp(word, "SETPLAYERCONTROLS"))
				{
					INTERACTIVE_OBJ * oes = EVENT_SENDER;
					EVENT_SENDER = io;
					pos = GetNextWord(es, pos, word);
					MakeUpcase(word);

					if (!strcmp(word, "ON"))
					{
						if (BLOCK_PLAYER_CONTROLS)
						{
							Stack_SendMsgToAllNPC_IO(SM_CONTROLS_ON, "");
						}

						BLOCK_PLAYER_CONTROLS = 0;
					}
					else
					{
						if (!BLOCK_PLAYER_CONTROLS)
						{
							ARX_PLAYER_PutPlayerInNormalStance(0);
							Stack_SendMsgToAllNPC_IO(SM_CONTROLS_OFF, "");
							ARX_SPELLS_FizzleAllSpellsFromCaster(0);
						}

						BLOCK_PLAYER_CONTROLS = 1;
						player.Interface &= ~INTER_COMBATMODE;
					}

					EVENT_SENDER = oes;
					LogDebug << "SET_PLAYER_CONTROLS "<< word;
				}
				else if (!strcmp(word, "SETWORLDCOLLISION"))
				{
					pos = GetNextWord(es, pos, word);
					MakeUpcase(word);

					if (!strcmp(word, "ON"))	io->collision |= 2;
					else io->collision &= ~2;

					LogDebug << "SET_WORLD_COLLISION "<< word;
				}
				else if (!strcmp(word, "SETDETACHABLE"))
				{
					pos = GetNextWord(es, pos, word);
				}
				else if (!strcmp(word, "SETSTACKABLE"))
				{
					pos = GetNextWord(es, pos, word);
				}
				else if (!strcmp(word, "SETMAXCOUNT"))
				{
					pos = GetNextWord(es, pos, word);

					if ((io != NULL) && (io->ioflags & IO_ITEM))
					{
						io->_itemdata->maxcount = (short)GetVarValueInterpretedAsFloat(word, esss, io);

						if (io->_itemdata->maxcount < 1) io->_itemdata->maxcount = 1;
					}

					LogDebug << "SET_MAX_COUNT "<<word;
				}
				else if (!strcmp(word, "SETCOUNT"))
				{
					pos = GetNextWord(es, pos, word);

					if ((io != NULL) && (io->ioflags & IO_ITEM))
					{
						io->_itemdata->count = (short)GetVarValueInterpretedAsFloat(word, esss, io);

						if (io->_itemdata->count < 1) io->_itemdata->count = 1;

						if (io->_itemdata->count > io->_itemdata->maxcount) io->_itemdata->count = io->_itemdata->maxcount;
					}

					LogDebug << "SET_COUNT "<< word;
				}
				else if (!strcmp(word, "SETWEIGHT"))
				{
					pos = GetNextWord(es, pos, word);

					if (io != NULL)
					{
						io->weight = GetVarValueInterpretedAsFloat(word, esss, io);

						if (io->weight < 0.f) io->weight = 0.f;
					}

					LogDebug << "SET_WEIGHT "<< word;
				}
				else if (!strcmp(word, "SETTRANSPARENCY"))
				{
					pos = GetNextWord(es, pos, word);

					io->invisibility = 1.f + GetVarValueInterpretedAsFloat(word, esss, io) * ( 1.0f / 100 );

					if (io->invisibility == 1.f) io->invisibility = 0;
				}
				else if (!strcmp(word, "SETEVENT"))
				{
					std::string temp2;
					long t;
					pos = GetNextWord(es, pos, word);
					pos = GetNextWord(es, pos, temp2);
					MakeUpcase(word);
					MakeUpcase(temp2);

					if ((!strcmp(temp2, "ON")) || (!strcmp(temp2, "YES"))) t = 1;
					else t = 0;

					if (!strcmp(word, "COLLIDE_NPC"))
					{
						if (t) esss->allowevents &= ~DISABLE_COLLIDE_NPC;
						else esss->allowevents |= DISABLE_COLLIDE_NPC;
					}

					if (!strcmp(word, "CHAT"))
					{
						if (t) esss->allowevents &= ~DISABLE_CHAT;
						else esss->allowevents |= DISABLE_CHAT;
					}

					if (!strcmp(word, "HIT"))
					{
						if (t) esss->allowevents &= ~DISABLE_HIT;
						else esss->allowevents |= DISABLE_HIT;
					}

					if (!strcmp(word, "INVENTORY2_OPEN"))
					{
						if (t) esss->allowevents &= ~DISABLE_INVENTORY2_OPEN ;
						else esss->allowevents |= DISABLE_INVENTORY2_OPEN ;
					}

					if (!strcmp(word, "DETECTPLAYER"))
					{
						if (t) esss->allowevents &= ~DISABLE_DETECT ;
						else esss->allowevents |= DISABLE_DETECT;
					}

					if (!strcmp(word, "HEAR"))
					{
						if (t) esss->allowevents &= ~DISABLE_HEAR ;
						else esss->allowevents |= DISABLE_HEAR ;
					}

					if (!strcmp(word, "AGGRESSION"))
					{
						if (t) esss->allowevents &= ~DISABLE_AGGRESSION ;
						else esss->allowevents |= DISABLE_AGGRESSION ;
					}

					if (!strcmp(word, "MAIN"))
					{
						if (t) esss->allowevents &= ~DISABLE_MAIN ;
						else esss->allowevents |= DISABLE_MAIN ;
					}

					if (!strcmp(word, "CURSORMODE"))
					{
						if (t) esss->allowevents &= ~DISABLE_CURSORMODE ;
						else esss->allowevents |= DISABLE_CURSORMODE ;
					}

					if (!strcmp(word, "EXPLORATIONMODE"))
					{
						if (t) esss->allowevents &= ~DISABLE_EXPLORATIONMODE ;
						else esss->allowevents |= DISABLE_EXPLORATIONMODE ;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_EVENT %s %s", word, temp2);

#endif
				}
				else if (!strcmp(word, "SETPRICE"))
				{
					pos = GetNextWord(es, pos, word);

					if (io)
					{
						if (io->ioflags & IO_ITEM)
						{
							io->_itemdata->price = (long)GetVarValueInterpretedAsFloat(word, esss, io);

							if (io->_itemdata->price < 0) io->_itemdata->price = 0;
						}
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_PRICE %s", word);

#endif
				}
				else if (!strcmp(word, "SETINTERNALNAME"))
				{
					pos = GetNextWord(es, pos, word);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "ERROR: SETINTERNALNAME %s - NOT AN IO !!!", word);

#endif
				}
				else if (!strcmp(word, "SHOWGLOBALS"))
				{
					ShowText = "";
					MakeGlobalText(ShowText);
					ShowTextWindowtext = "Global Variables";

#ifdef BUILD_EDITOR
					if (!(danaeApp.kbd.inkey[INKEY_LEFTSHIFT]) && !(danaeApp.kbd.inkey[INKEY_RIGHTSHIFT]))
						DialogBox(hInstance, (LPCTSTR)IDD_SHOWTEXT, NULL, (DLGPROC)ShowTextDlg);
#endif

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SHOWGLOBALS");

#endif
				}
				else if (!strcmp(word, "SHOWLOCALS"))
				{
					ShowText = "";
					MakeLocalText(es, ShowText);
					ShowTextWindowtext = "Local Variables";

#ifdef BUILD_EDITOR
					if (!(danaeApp.kbd.inkey[INKEY_LEFTSHIFT]) && !(danaeApp.kbd.inkey[INKEY_RIGHTSHIFT]))
						DialogBox(hInstance, (LPCTSTR)IDD_SHOWTEXT, NULL, (DLGPROC)ShowTextDlg);
#endif

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SHOWLOCALS");

#endif
				}
				else if (!strcmp(word, "SHOWVARS"))
				{
					ShowText = "";
					ShowText2 = "";
					MakeGlobalText(ShowText);
					MakeLocalText(es, ShowText2);
					ShowTextWindowtext = "Variables";

#ifdef BUILD_EDITOR
					if (!(danaeApp.kbd.inkey[INKEY_LEFTSHIFT]) && !(danaeApp.kbd.inkey[INKEY_RIGHTSHIFT]))
						DialogBox(hInstance, (LPCTSTR)IDD_SHOWVARS, NULL, (DLGPROC)ShowVarsDlg);
#endif

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SHOWVARS");

#endif
				}
				else if (!strcmp(word, "SETIRCOLOR"))
				{
					if (io != NULL)
					{
						std::string temp1;
						std::string temp2;
						std::string temp3;
						float t1, t2, t3;

						pos = GetNextWord(es, pos, temp1);
						pos = GetNextWord(es, pos, temp2);
						pos = GetNextWord(es, pos, temp3);
						t1 = GetVarValueInterpretedAsFloat(temp1, esss, io);
						t2 = GetVarValueInterpretedAsFloat(temp2, esss, io);
						t3 = GetVarValueInterpretedAsFloat(temp3, esss, io);
						io->infracolor.r = t1;
						io->infracolor.g = t2;
						io->infracolor.b = t3;
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG) sprintf(cmd, "%s %s %s %s", word, temp1, temp2, temp3);

#endif
					}
				}
				else if (!strcmp(word, "SETSCALE"))
				{
					if (io != NULL)
					{
						std::string temp1;
						float t1;
						pos = GetNextWord(es, pos, temp1);
						t1 = GetVarValueInterpretedAsFloat(temp1, esss, io);
						io->scale = t1 * ( 1.0f / 100 );
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG) sprintf(cmd, "SET_SCALE %s", temp1);

#endif
					}
				}
				else if (!strcmp(word, "STEALNPC"))
				{
					if (player.Interface & INTER_STEAL)
					{
						SendIOScriptEvent(ioSteal, SM_STEAL, "OFF");
					}

					player.Interface |= INTER_STEAL;
					InventoryDir = 1;
					ioSteal = io;
				}
				else if (!strcmp(word, "SPECIALFX"))
				{
					std::string temp1;
					pos = GetNextWord(es, pos, temp1);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "SPECIAL_FX ");
						strcat(cmd, word);
					}

#endif

					if (!strcasecmp(temp1, "YLSIDE_DEATH"))
					{
						SetYlsideDeath(io);
					}
					else if (!strcasecmp(temp1, "PLAYER_APPEARS"))
					{
						MakePlayerAppearsFX(io);
					}
					else if (!strcmp(temp1, "HEAL"))
					{
						pos = GetNextWord(es, pos, temp1);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp1);
						}

#endif

						if (!BLOCK_PLAYER_CONTROLS)
							player.life += (float)atof(temp1.c_str());

						if (player.life > player.Full_maxlife) player.life = player.Full_maxlife;

						if (player.life < 0.f) player.life = 0.f;
					}
					else if (!strcmp(temp1, "MANA"))
					{
						pos = GetNextWord(es, pos, temp1);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp1);
						}

#endif
						player.mana += (float)atof(temp1.c_str());

						if (player.mana > player.Full_maxmana) player.mana = player.Full_maxmana;

						if (player.mana < 0.f) player.mana = 0.f;
					}
					else if (!strcmp(temp1, "NEWSPELL"))
					{
						MakeBookFX(DANAESIZX - INTERFACE_RATIO(35), DANAESIZY - INTERFACE_RATIO(148), 0.00001f);
						pos = GetNextWord(es, pos, temp1);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp1);
						}

#endif
					}
					else if (!strcmp(temp1, "TORCH"))
					{
						if ((io) && (io->ioflags & IO_ITEM))
						{
							INTERACTIVE_OBJ * ioo = io;

							if (io->_itemdata->count > 1)
							{
								ioo = CloneIOItem(io);
								MakeTemporaryIOIdent(ioo);
								ioo->show = SHOW_FLAG_IN_INVENTORY;
								ioo->scriptload = 1;
								ioo->_itemdata->count = 1;
								io->_itemdata->count--;
							}

							ARX_PLAYER_ClickedOnTorch(ioo);
						}
					}
					else if (!strcmp(temp1, "FIERY"))
					{
						io->ioflags |= IO_FIERY;
					}
					else if (!strcmp(temp1, "FIERYOFF"))
					{
						io->ioflags &= ~IO_FIERY;
					}
					else if (!strcmp(temp1, "TORCHON"))
					{
						//DO NOTHING
					}
					else if (!strcmp(temp1, "TORCHOFF"))
					{
						if (CURRENT_TORCH)
							ARX_PLAYER_ClickedOnTorch(CURRENT_TORCH);
					}
				}

				break;
			case 'Z':

				if (!strcmp(word, "ZONEPARAM"))
				{
					pos = GetNextWord(es, pos, word);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "ZONE_PARAM ");
						strcat(cmd, word);
					}

#endif

					if (!strcasecmp(word, "STACK"))
					{
						ARX_GLOBALMODS_Stack();
					}
					else if (!strcasecmp(word, "UNSTACK"))
					{
						ARX_GLOBALMODS_UnStack();
					}
					else
					{
						if (word[0] == '-')
						{
							pos = GetNextWord(es, pos, word);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, word);
							}

#endif
						}

						if (!strcasecmp(word, "RGB"))
						{
							pos = GetNextWord(es, pos, word);
							desired.depthcolor.r = GetVarValueInterpretedAsFloat(word, esss, io);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, word);
							}

#endif
							pos = GetNextWord(es, pos, word);
							desired.depthcolor.g = GetVarValueInterpretedAsFloat(word, esss, io);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, word);
							}

#endif
							pos = GetNextWord(es, pos, word);
							desired.depthcolor.b = GetVarValueInterpretedAsFloat(word, esss, io);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, word);
							}

#endif
							desired.flags |= GMOD_DCOLOR;
						}
						else if (!strcasecmp(word, "ZCLIP"))
						{
							pos = GetNextWord(es, pos, word);
							desired.zclip = GetVarValueInterpretedAsFloat(word, esss, io);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, word);
							}

#endif
							desired.flags |= GMOD_ZCLIP;
						}
						else if (!strcasecmp(word, "AMBIANCE"))
						{
							pos = GetNextWord(es, pos, word);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, word);
							}

#endif
							ARX_SOUND_PlayZoneAmbiance(word.c_str());
						}

					}
				}

				break;
			case 'K':

				if (!strcmp(word, "KILLME"))
				{
					if (io) // can only kill IOs
					{
						if ((io->ioflags & IO_ITEM) && (io->_itemdata->count > 1))
						{
							io->_itemdata->count--;
						}
						else
						{
							io->show = SHOW_FLAG_KILLED;
							io->GameFlags &= ~GFLAG_ISINTREATZONE;
							RemoveFromAllInventories(io);
							ARX_DAMAGES_ForceDeath(io, EVENT_SENDER);
						}
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "KILLME");

#endif
				}
				else if (!strcmp(word, "KEYRINGADD"))
				{
					std::string temp2;
					pos = GetNextWord(es, pos, temp2);
					word = GetVarValueInterpretedAsText(temp2, esss, io);
					ARX_KEYRING_Add(word);
				}

				break;
			case 'F':

				if (!strcmp(word, "FORCEANIM"))
				{
					std::string temp2;
					long num;
					pos = GetNextWord(es, pos, temp2);
					num = GetNumAnim(temp2);

					if (num > -1)
					{
						if (io != NULL)
							if (io->anims[num] != NULL)
							{
								ForceAnim(io, io->anims[num]);
								CheckSetAnimOutOfTreatZone(io, 0);
							}
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "FORCEANIM %s", temp2.c_str());

#endif
				}
				else if (!strcmp(word, "FORCEANGLE"))
				{
					pos = GetNextWord(es, pos, word);

					if (io != NULL)
					{

						io->angle.b = MAKEANGLE(GetVarValueInterpretedAsFloat(word, esss, io));
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "FORCEANGLE %s", word);

#endif
				}
				else if (!strcmp(word, "FORCEDEATH"))
				{
					std::string temp2;
					long t;
					pos = GetNextWord(es, pos, temp2);
					MakeUpcase(temp2);

					if ((!strcmp(temp2, "ME")) || (!strcmp(temp2, "SELF")))
					{
						t = GetInterNum(io);
					}
					else
					{
						t = GetTargetByNameTarget(temp2);

						if (t == -2) t = GetInterNum(io);
					}

					if (t > 0)
					{
						ARX_DAMAGES_ForceDeath(inter.iobj[t], io);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "FORCE_DEATH %s", temp2);

#endif
				}

				break;
			case 'P':

				if (!strcmp(word, "PLAYERLOOKAT"))
				{
					pos = GetNextWord(es, pos, word);
					long t = GetTargetByNameTarget(word);

					if (t == -2) t = GetInterNum(io);

					if (ValidIONum(t))
					{
						ForcePlayerLookAtIO(inter.iobj[t]);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "PLAYERLOOKAT %s", word);

#endif
				}
				else if (!strcmp(word, "PLAYERSTACKSIZE"))
				{
					pos = GetNextWord(es, pos, word);

					if ((io) && (io->ioflags & IO_ITEM))
					{
						io->_itemdata->playerstacksize = (short)GetVarValueInterpretedAsFloat(word, esss, io);

						if (io->_itemdata->playerstacksize < 1)
							io->_itemdata->playerstacksize = 1;

						if (io->_itemdata->playerstacksize > 100)
							io->_itemdata->playerstacksize = 100;
					}
				}
				else if (!strcmp(word, "PRECAST"))
				{
					std::string temp2;
					long duration = -1;
					SpellcastFlags flags = 0;
					long dur = 0;
					pos = GetNextWord(es, pos, word); // switch or level

					if (word[0] == '-')
					{
						if (iCharIn(word, 'D'))
						{
							flags |= SPELLCAST_FLAG_NOCHECKCANCAST;
							pos = GetNextWord(es, pos, temp2); // duration
							duration = GetVarValueInterpretedAsFloat(temp2, esss, io);
							dur = 1;
						}

						if (iCharIn(word, 'F'))
						{
							flags |= SPELLCAST_FLAG_NOCHECKCANCAST;
							flags |= SPELLCAST_FLAG_NOMANA;
						}

						pos = GetNextWord(es, pos, word); // level
					}

					long level = GetVarValueInterpretedAsFloat(word, esss, io);

					if (level < 1) level = 1;
					else if (level > 10) level = 10;

					pos = GetNextWord(es, pos, word); //spell id
					Spell spellid;
					spellid = GetSpellId(word);

					if (spellid != SPELL_NONE)
					{
						flags |= SPELLCAST_FLAG_PRECAST;

						if (!dur)
							duration = 2000 + level * 2000;

						if (io != inter.iobj[0])
						{
							flags |= SPELLCAST_FLAG_NOCHECKCANCAST;
						}

						flags |= SPELLCAST_FLAG_NOANIM;
						TryToCastSpell(inter.iobj[0], spellid, level, -1, flags, duration);
					}
				}
				else if (!strcmp(word, "POISON"))
				{
					pos = GetNextWord(es, pos, word);
					float fval = GetVarValueInterpretedAsFloat(word, esss, io);
					ARX_PLAYER_Poison(fval);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "POISON %s", word);

#endif
				}
				else if (!strcmp(word, "PLAYERMANADRAIN"))
				{
					pos = GetNextWord(es, pos, word);

					if (!strcasecmp(word, "ON"))
						player.playerflags &= ~PLAYERFLAGS_NO_MANA_DRAIN;
					else
						player.playerflags |= PLAYERFLAGS_NO_MANA_DRAIN;
				}
				else if (!strcmp(word, "PATHFIND"))
				{
					std::string temp2;
					pos = GetNextWord(es, pos, temp2);
					long t = GetTargetByNameTarget(temp2);
					ARX_NPC_LaunchPathfind(io, t);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "PATHFIND %s", temp2);

#endif
				}
				else if (!strcmp(word, "PLAYANIM"))
				{
					INTERACTIVE_OBJ * iot = io;
					std::string temp2;
					long nu = 0;
					long loop = 0;
					long execute = 0;
					long nointerpol = 0;
					pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "PLAY_ANIM ");
						strcat(cmd, word);
					}

#endif

					if (temp2[0] == '-')
					{
						if (CharIn(temp2, '1')) nu = 0;

						if (CharIn(temp2, '2')) nu = 1;

						if (CharIn(temp2, '3')) nu = 2;

						if (iCharIn(temp2, 'L')) loop = 1;

						if (iCharIn(temp2, 'N')) nointerpol = 1;

						if (iCharIn(temp2, 'E')) execute = 1;

						if(iCharIn(temp2, 'P')) {
							iot = inter.iobj[0];
							iot->move = iot->lastmove = Vec3f::ZERO;
						}

						pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp2);
						}

#endif
					}

					if (!strcasecmp(temp2, "NONE"))
					{
						if (iot != NULL)
						{
							iot->animlayer[nu].cur_anim = NULL;
							iot->animlayer[nu].next_anim = NULL;
						}
					}
					else
					{
						long num = GetNumAnim(temp2);

						if (num > -1)
						{
							if (iot != NULL)
								if (iot->anims[num] != NULL)
								{
									iot->ioflags |= IO_NO_PHYSICS_INTERPOL;
									SetNextAnim(iot, iot->anims[num], nu, loop, nointerpol);

									if (!loop)
										CheckSetAnimOutOfTreatZone(iot, nu);

									{
										if (iot == inter.iobj[0])
											iot->animlayer[nu].flags &= ~EA_STATICANIM;

										if (execute)
										{
											string timername = "anim_" + ARX_SCRIPT_Timer_GetDefaultName();
											long num2 = ARX_SCRIPT_Timer_GetFree();

											if (num2 > -1)
											{
												scr_timer[num2].reset();
												ActiveTimers++;
												scr_timer[num2].es = es;
												scr_timer[num2].exist = 1;
												scr_timer[num2].io = io;
												scr_timer[num2].msecs = max(iot->anims[num]->anims[iot->animlayer[nu].altidx_cur]->anim_time, 1000.0f);
												scr_timer[num2].name = timername;
												scr_timer[num2].pos = pos;
												scr_timer[num2].tim = ARXTimeUL();
												scr_timer[num2].times = 1;
												scr_timer[num2].longinfo = 0; //numsound;
											}

											pos = GotoNextLine(es, pos);
										}
									}
								}
						}
					}
				}
				else if (!strcmp(word, "PLAYERINTERFACE"))
				{
					std::string temp2;
					pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "PLAYER_INTERFACE ");
						strcat(cmd, word);
					}

#endif
					long smooth = 0;

					if (temp2[0] == '-')
					{
						if ((temp2[1] == 's') || (temp2[1] == 'S'))
						{
							smooth = 1;
						}

						pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp2);
						}

#endif
					}

					if (!strcasecmp(temp2, "HIDE"))
						ARX_INTERFACE_PlayerInterfaceModify(0, smooth);

					if (!strcasecmp(temp2, "SHOW"))
						ARX_INTERFACE_PlayerInterfaceModify(1, smooth);

				}
				else if (!strcmp(word, "PLAY"))
				{
					SoundLoopMode loop(ARX_SOUND_PLAY_ONCE);
					std::string temp2;
					float pitch(1.0F);
					bool unique(false);
					bool stop(false);
					bool no_pos(false);

					pos = GetNextWord(es, pos, temp2);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "PLAY ");
						strcat(cmd, word);
					}

#endif

					if (temp2[0] == '-')
					{
						if (iCharIn(temp2, 'I')) unique = true;
						else if (iCharIn(temp2, 'L')) loop = ARX_SOUND_PLAY_LOOPED;
						else if (iCharIn(temp2, 'P')) pitch = 0.9F + 0.2F * rnd();
						else if (iCharIn(temp2, 'S')) stop = true;
						else if (iCharIn(temp2, 'O')) no_pos = true;

						pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp2);
						}

#endif
					}

					std::string temp1 = GetVarValueInterpretedAsText(temp2, esss, io);

					if (io)
					{
						SetExt(temp1, ".wav");

						if (stop)
						{
							ARX_SOUND_Stop(io->sound);
							io->sound = ARX_SOUND_INVALID_RESOURCE;
						}
						else
						{
							if (unique && io->sound != ARX_SOUND_INVALID_RESOURCE)
								ARX_SOUND_Stop(io->sound);

							long num;

							if (no_pos || SM_INVENTORYUSE == msg)
								num = ARX_SOUND_PlayScript(temp1.c_str(), NULL, pitch, loop);
							else
								num = ARX_SOUND_PlayScript(temp1.c_str(), io, pitch, loop);

							if (unique) io->sound = num;

#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								if (num == ARX_SOUND_INVALID_RESOURCE)
									sprintf(cmd, "PLAY %s - Success DanaePlaySample", temp1);
								else
									sprintf(cmd, "PLAY %s - UNABLE TO LOAD FILE", temp1);
							}

#endif
						}
					}

#ifdef NEEDING_DEBUG
					else if (NEED_DEBUG) strcat(cmd, "ERROR: - NOT AN IO !!!");

#endif
				}
				else if (!strcmp(word, "PLAYSPEECH"))
				{
					std::string temp2;

					pos = GetNextWord(es, pos, temp2);

					long num = ARX_SOUND_PlaySpeech(temp2.c_str(), io && io->show == 1 ? io : NULL);

					if (num == ARX_SOUND_INVALID_RESOURCE)
						sprintf(cmd, "PLAYSPEECH %s - UNABLE TO LOAD FILE", temp2.c_str());
					else
						sprintf(cmd, "PLAYSPEECH %s - Success DanaePlaySample", temp2.c_str());
					
				}
				else if (!strcmp(word, "POPUP"))
				{
					pos = GetNextWord(es, pos, word);

					if (!(danaeApp.kbd.inkey[INKEY_LEFTSHIFT]) && !(danaeApp.kbd.inkey[INKEY_RIGHTSHIFT]))
					{
						ARX_TIME_Pause();
						LogError << word;
						ARX_TIME_UnPause();
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "POPUP %s", word);

#endif
				}
				else if (!strcmp(word, "PHYSICAL"))
				{
					std::string temp2;
					pos = GetNextWord(es, pos, word);

					if (!strcasecmp(word, "ON"))
					{
						io->ioflags &= ~IO_PHYSICAL_OFF;
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG) sprintf(cmd, "PHYSICAL ON");

#endif
					}
					else if (!strcasecmp(word, "OFF"))
					{
						io->ioflags |= IO_PHYSICAL_OFF;
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG) sprintf(cmd, "PHYSICAL OFF");

#endif
					}
					else
					{
						pos = GetNextWord(es, pos, temp2);
						float fval = GetVarValueInterpretedAsFloat(temp2, esss, io);

						if (!strcasecmp(word, "HEIGHT"))
						{
							if (io)
							{
								io->original_height = -fval;

								if (io->original_height > -30) io->original_height = -30;

								if (io->original_height < -165) io->original_height = -165;

								io->physics.cyl.height = io->original_height * io->scale;
							}
						}
						else if (!strcasecmp(word, "RADIUS"))
						{
							if (io)
							{
								io->original_radius = fval;

								if (io->original_radius < 10) io->original_radius = 10;
								else if (io->original_radius > 40) io->original_radius = 40;

								io->physics.cyl.radius = io->original_radius * io->scale;

							}
						}

#ifdef NEEDING_DEBUG

						if (NEED_DEBUG) sprintf(cmd, "PHYSICAL %s %s", word, temp2);

#endif
					}
				}

				break;
			case 'L':

				if (!strcmp(word, "LOADANIM"))
				{
					INTERACTIVE_OBJ * iot = io;
					pos = GetNextWord(es, pos, word);

					if (word[0] == '-')
					{
						if (iCharIn(word, 'P'))
							iot = inter.iobj[0];

						pos = GetNextWord(es, pos, word);
					}

					std::string temp2;

					pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "LOADANIM %s %s", word, temp2);

#endif

					if (iot != NULL)
					{
						long num = -1;
						MakeUpcase(word);
						num = GetNumAnim(word);

						if ((num > -1) && (num < MAX_ANIMS))
						{


							if (iot->anims[num] != NULL)
							{
								ReleaseAnimFromIO(iot, num);
							}

							if (iot->anims[num] == NULL)
							{
								std::string tex2;
								std::string tex3;

								if((iot == inter.iobj[0]) || (iot->ioflags & IO_NPC)) {
									tex3 = "Graph\\Obj3D\\Anims\\npc\\" + temp2;
								} else {
									tex3 = "Graph\\Obj3D\\Anims\\Fix_Inter\\" + temp2;
								}

								SetExt(tex3, ".tea");
								File_Standardize(tex3, tex2);

								if (resources->getFile(tex2))
								{
									iot->anims[num] = EERIE_ANIMMANAGER_Load(tex2);

									if(iot->anims[num] == NULL) {
										LogWarning << "LOADANIM " << word << " " << temp2 << " FAILED";
									}
								}
							} else {
								LogWarning << "LOADANIM " << word << " " << temp2 << " FAILED";
							}
						}
					}
				}
				else if (!strcmp(word, "LINKOBJTOME"))
				{
					pos = GetNextWord_Interpreted(io, es, pos, word);
					long t = GetTargetByNameTarget(word);
					pos = GetNextWord(es, pos, word);

					if (ValidIONum(t))
						LinkObjToMe(io, inter.iobj[t], word);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "LINKOBJTOME %ld %s", t, word);

#endif
				}

				break;
			case 'I':

				if ((word[1] == 'F') && (word.size() == 2))
				{
					std::string temp3;
					ScriptOperator oper = OPER_UNKNOWN;
					short failed = 0;
					short typ1, typ2;
					std::string tvar1;
					std::string tvar2;
					float fvar1, fvar2;
					std::string tempo;

					fvar1 = fvar2 = 0;
					pos = GetNextWord(es, pos, word);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "IF (");
						strcat(cmd, word);
					}

#endif

					pos = GetNextWord(es, pos, temp3);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcat(cmd, " ");
						strcat(cmd, temp3);
					}

#endif

					if	(!strcmp(temp3, "=="))			oper = OPER_EQUAL;
					else if (!strcmp(temp3, "!="))			oper = OPER_NOTEQUAL;
					else if (!strcmp(temp3, "<="))			oper = OPER_INFEQUAL;
					else if (!strcmp(temp3, "<"))			oper = OPER_INFERIOR;
					else if (!strcmp(temp3, ">="))			oper = OPER_SUPEQUAL;
					else if (!strcmp(temp3, ">"))			oper = OPER_SUPERIOR;
					else if	(!strcasecmp(temp3, "ISCLASS"))	oper = OPER_INCLASS;
					else if	(!strcasecmp(temp3, "ISELEMENT"))	oper = OPER_ISELEMENT;
					else if	(!strcasecmp(temp3, "ISIN"))		oper = OPER_ISIN;
					else if	(!strcasecmp(temp3, "ISTYPE"))		oper = OPER_ISTYPE;
					else if	(!strcasecmp(temp3, "ISGROUP"))	oper = OPER_ISGROUP;
					else if	(!strcasecmp(temp3, "!ISGROUP"))	oper = OPER_NOTISGROUP;


					pos = GetNextWord(es, pos, temp3);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcat(cmd, " ");
						strcat(cmd, temp3);
						strcat(cmd, ") ");
					}

#endif

					switch (word[0])
					{
						case '^':
						{

							long lv; float fv; std::string tv;	//Arx: xrichter (2010-08-04) - fix a crash when $OBJONTOP return to many object name inside tv
							switch ( GetSystemVar( esss, io, word, tv, &fv, &lv ) )
							{
								case TYPE_TEXT:
									typ1	=	TYPE_TEXT;
									tvar1 = tv;
									break;
								case TYPE_LONG:
									typ1	=	TYPE_FLOAT;
									fvar1	=	(float)lv;
									break;
								case TYPE_FLOAT:
									typ1	=	TYPE_FLOAT;
									fvar1	=	fv;
									break;

								default:
									ARX_CHECK_NO_ENTRY(); //typ1 are not initialized
									typ1	=	TYPE_TEXT;
									break;

							}
						}
							break;
						case '#':
							typ1	=	TYPE_FLOAT;
							fvar1	=	GETVarValueLong(svar, NB_GLOBALS, word);
							break;
						case '\xA7':
							typ1	=	TYPE_FLOAT;
							fvar1	=	GETVarValueLong(esss->lvar, esss->nblvar, word);
							break;
						case '&':
							typ1	=	TYPE_FLOAT;
							fvar1	=	GETVarValueFloat(svar, NB_GLOBALS, word);
							break;
						case '@':
							typ1	=	TYPE_FLOAT;
							fvar1	=	GETVarValueFloat(esss->lvar, esss->nblvar, word);
							break;
						case '$':
							typ1	=	TYPE_TEXT;
							tempo	=	GETVarValueText(svar, NB_GLOBALS, word);

							tvar1 = tempo;

							break;
						case '\xA3':
							typ1	=	TYPE_TEXT;
							tempo	=	GETVarValueText(esss->lvar, esss->nblvar, word);

							tvar1 = tempo;

							break;
						default:

							if ((oper == OPER_ISTYPE) || (oper == OPER_ISGROUP) || (oper == OPER_NOTISGROUP))
							{
								typ1 =	TYPE_TEXT;
								tvar1 = word;
							}
							else
							{
								typ1 =	TYPE_FLOAT;
								fvar1 =	(float)atof(word.c_str());
							}
					}

					switch ( temp3[0] )
					{
						case '^':
						{

							long lv; float fv; std::string tv;
							switch ( GetSystemVar( esss, io, temp3, tv, &fv, &lv ) )
							{
								case TYPE_TEXT:
									typ2	=	TYPE_TEXT;
									tvar2 = tv;
									break;
								case TYPE_LONG:
									typ2	=	TYPE_FLOAT;
									fvar2	=	(float)lv;
									break;
								case TYPE_FLOAT:
									typ2	=	TYPE_FLOAT;
									fvar2	=	fv;
									break;

								default:
									ARX_CHECK_NO_ENTRY(); //typ1 is not initialized
									typ2	=	TYPE_TEXT;
									break;

							}
						}
							break;
						case '#':
							typ2			=	TYPE_FLOAT;
							fvar2			=	(float)GETVarValueLong(svar, NB_GLOBALS, temp3);
							break;
						case '\xA7':
							typ2			=	TYPE_FLOAT;
							fvar2			=	(float)GETVarValueLong(esss->lvar, esss->nblvar, temp3);
							break;
						case '&':
							typ2			=	TYPE_FLOAT;
							fvar2			=	GETVarValueFloat(svar, NB_GLOBALS, temp3);
							break;
						case '@':
							typ2			=	TYPE_FLOAT;
							fvar2			=	GETVarValueFloat(esss->lvar, esss->nblvar, temp3);
							break;
						case '$':
							typ2			=	TYPE_TEXT;
							tempo			=	GETVarValueText(svar, NB_GLOBALS, temp3);

							tvar2 = tempo;

							break;
						case '\xA3':
							typ2			=	TYPE_TEXT;
							tempo			=	GETVarValueText(esss->lvar, esss->nblvar, temp3);

							tvar2 = tempo;

							break;
						default:

							if (typ1 == TYPE_TEXT)
							{
								typ2		=	TYPE_TEXT;
								tvar2 = temp3;
							}
							else
							{
								typ2		=	TYPE_FLOAT;
								fvar2		=	(float)atof(temp3.c_str());
							}
					}

					failed = 0;

					switch (oper)
					{
						case OPER_ISELEMENT:

							if (typ1 != typ2) failed = 1;
							else
							{
								if (typ1 == TYPE_TEXT)
								{
									MakeUpcase(tvar1);
									MakeUpcase(tvar2);

									if (IsElement(tvar1.c_str(), tvar2.c_str())) failed = 0;
									else failed = 1;
								}
							}

							break;
						case OPER_INCLASS:

							if (typ1 != typ2) failed = 1;
							else
							{
								if (typ1 == TYPE_TEXT)
								{
									MakeUpcase(tvar1);
									MakeUpcase(tvar2);

									if ((IsIn(tvar2, tvar1)) || (IsIn(tvar1, tvar2))) failed = 0;
									else failed = 1;
								}
							}

							break;
						case OPER_ISGROUP:

							if (typ1 != typ2) failed = 1;
							else
							{
								if (typ1 == TYPE_TEXT)
								{
									MakeUpcase(tvar1);
									long t = GetTargetByNameTarget(tvar1);

									if (t == -2) t = GetInterNum(io);

									if (ValidIONum(t))
									{
										if (IsIOGroup(inter.iobj[t], tvar2)) failed = 0;
										else failed = 1;
									}
									else failed = 1;
								}
							}

							break;
						case OPER_NOTISGROUP:

							if (typ1 != typ2) failed = 1;
							else
							{
								if (typ1 == TYPE_TEXT)
								{
									MakeUpcase(tvar1);

									long t = GetTargetByNameTarget(tvar1);

									if (t == -2) t = GetInterNum(io);

									if (ValidIONum(t))
									{
										if (IsIOGroup(inter.iobj[t], tvar2)) failed = 1;
										else failed = 0;
									}
									else failed = 1;
								}
							}

							break;
						case OPER_ISTYPE:

							if (typ1 != typ2) failed = 1;
							else
							{
								if (typ1 == TYPE_TEXT)
								{
									MakeUpcase(tvar1);

									long t		=	GetTargetByNameTarget(tvar1);

									if (t == -2) t = GetInterNum(io);

									ItemType flagg = ARX_EQUIPMENT_GetObjectTypeFlag(tvar2);

									if ((flagg != 0) && (ValidIONum(t)))
									{
										if (inter.iobj[t]->type_flags & flagg) failed = 0;
										else failed = 1;
									}
									else failed = 1;
								}
							}

							break;
						case OPER_ISIN:

							if (typ1 != typ2) failed = 1;
							else
							{
								if (typ1 == TYPE_TEXT)
								{
									MakeUpcase(tvar1);
									MakeUpcase(tvar2);

									if (IsIn(tvar2, tvar1))  failed = 0;
									else failed = 1;
								}
							}

							break;
						case OPER_EQUAL:

							if (typ1 != typ2) failed = 1;
							else
							{
								if (typ1 == TYPE_TEXT)
								{
									MakeUpcase(tvar1);
									MakeUpcase(tvar2);

									if (strcmp(tvar1, tvar2)) failed = 1;
								}
								else
								{
									if (fvar1 != fvar2) failed = 1;
								}
							}

							break;
						case OPER_NOTEQUAL:

							if (typ1 != typ2) failed = 1;
							else
							{
								if (typ1 == TYPE_TEXT)
								{
									if (strcmp(tvar1, tvar2)) failed = 1;
								}
								else
								{
									if (fvar1 != fvar2) failed = 1;
								}
							}

							if (failed) failed = 0;
							else failed = 1;

							break;
						case OPER_INFEQUAL:

							if (typ1 != typ2) failed = 1;
							else
							{
								if (typ1 == TYPE_TEXT)	 failed = 1;
								else
								{
									if (fvar1 > fvar2) failed = 1;
								}
							}

							break;
						case OPER_INFERIOR:

							if (typ1 != typ2) failed = 1;
							else
							{
								if (typ1 == TYPE_TEXT)		failed = 1;
								else
								{
									if (fvar1 >= fvar2)	failed = 1;
								}
							}

							break;
						case OPER_SUPEQUAL:

							if (typ1 != typ2) failed = 1;
							else
							{
								if (typ1 == TYPE_TEXT)		failed = 1;
								else
								{
									if (fvar1 < fvar2)	failed = 1;
								}
							}

							break;
						case OPER_SUPERIOR:

							if (typ1 != typ2) failed = 1;
							else
							{
								if (typ1 == TYPE_TEXT)		failed = 1;
								else
								{
									if (fvar1 <= fvar2)	failed = 1;
								}
							}

							break;
						case OPER_UNKNOWN: break;
					}

					if (failed)
					{
						pos = SkipNextStatement(es, pos);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						if (failed) strcat(cmd, " -> false");
						else strcat(cmd, " -> true");
					}

#endif
				}
				else if (!strcmp(word, "INC"))
				{
					std::string temp1;
					std::string temp2;
					float fval;
					float fdval;
					SCRIPT_VAR * sv = NULL;

					pos = GetNextWord(es, pos, temp1, 1);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "INC ");
						strcat(cmd, temp1);
					}

#endif
					pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcat(cmd, temp2);
					}

#endif

					switch (temp1[0])
					{
						case '$': // GLOBAL TEXT
						case '\xA3': // LOCAL TEXT
							ShowScriptError("Unable to execute this\nOperation on a String", cmd);
							break;
						case '#': // GLOBAL LONG
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fdval = (float)GETVarValueLong(svar, NB_GLOBALS, temp1);
							fval = fdval + fval;
							sv = SETVarValueLong(svar, NB_GLOBALS, temp1, (long)fval);

							if (sv != NULL) sv->type = TYPE_G_LONG;

							break;
						case '\xA7': // LOCAL LONG
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fdval = (float)GETVarValueLong(esss->lvar, esss->nblvar, temp1);
							fval = fdval + fval;
							sv = SETVarValueLong(esss->lvar, esss->nblvar, temp1, (long)fval);

							if (sv != NULL) sv->type = TYPE_L_LONG;

							break;
						case '&': // GLOBAL float
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fdval = GETVarValueFloat(svar, NB_GLOBALS, temp1);
							fval = fdval + fval;
							sv = SETVarValueFloat(svar, NB_GLOBALS, temp1, fval);

							if (sv != NULL) sv->type = TYPE_G_FLOAT;

							break;
						case '@': // LOCAL float
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fdval = GETVarValueFloat(esss->lvar, esss->nblvar, temp1);
							fval = fdval + fval;
							sv = SETVarValueFloat(esss->lvar, esss->nblvar, temp1, fval);

							if (sv != NULL) sv->type = TYPE_L_FLOAT;

							break;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "%s %s %s", word, temp1, temp2);

#endif
				}
				else if (!strcmp(word, "IFEXISTINTERNAL"))
				{
					long failed = 1;
					pos = GetNextWord(es, pos, word);
					long t = GetTargetByNameTarget(word);

					if (t != -1)
					{
						failed = 0;
					}

					if (failed)
					{
						pos = SkipNextStatement(es, pos);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						if (failed) sprintf(cmd, "IFEXISTINTERNAL (%s) -> false", word);
						else sprintf(cmd, "IFEXISTINTERNAL (%s) -> true", word);
					}

#endif
				}
				else if (!strcmp(word, "INVULNERABILITY"))
				{
					pos = GetNextWord(es, pos, word);
					long player = 0;

					if (word[0] == '-')
					{
						if (iCharIn(word, 'P'))
							player = 1;

						pos = GetNextWord(es, pos, word);
					}

					if (!strcasecmp(word, "ON"))
					{
						if (player)
							ARX_PLAYER_Invulnerability(1);
						else
							io->ioflags |= IO_INVULNERABILITY;
					}
					else
					{
						if (player)
							ARX_PLAYER_Invulnerability(0);
						else
							io->ioflags &= ~IO_INVULNERABILITY;
					}
				}
				else if (!strcmp(word, "IFVISIBLE"))
				{
					long failed = 1;
					pos = GetNextWord(es, pos, word);
					long t = GetTargetByNameTarget(word);

					if (ValidIONum(t))
					{
						if (HasVisibility(io, inter.iobj[t]))
							failed = 0;
					}

					if (failed)
					{
						pos = SkipNextStatement(es, pos);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						if (failed) sprintf(cmd, "IFVISIBLE (%s) -> false", word);
						else sprintf(cmd, "IFVISIBLE (%s) -> true", word);
					}

#endif
				}
				else if (!strcmp(word, "INVENTORY"))
				{
					pos = GetNextWord(es, pos, word);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "INVENTORY ");
						strcat(cmd, word);
					}

#endif
					MakeStandard(word);
					long ion;
					ion = GetInterNum(io);

					if ((io != NULL) && (ion != -1)) {
						if (!strcasecmp(word, "CREATE"))
						{
							if (inter.iobj[ion]->inventory != NULL)
							{
								INVENTORY_DATA * id = inter.iobj[ion]->inventory;

								for (long nj = 0; nj < id->sizey; nj++)
									for (long ni = 0; ni < id->sizex; ni++)
									{
										if (id->slot[ni][nj].io != NULL)
										{
											long tmp = GetInterNum(id->slot[ni][nj].io);

											if (tmp != -1)
											{
												if (inter.iobj[tmp]->scriptload)
												{
													RemoveFromAllInventories(inter.iobj[tmp]);
													ReleaseInter(inter.iobj[tmp]);
													inter.iobj[tmp] = NULL;
												}
												else inter.iobj[tmp]->show = SHOW_FLAG_KILLED;
											}

											id->slot[ni][nj].io = NULL;
										}
									}

								free(io->inventory);
								inter.iobj[ion]->inventory = NULL;
							}

							if (inter.iobj[ion]->inventory == NULL)
							{
								inter.iobj[ion]->inventory = (INVENTORY_DATA *)malloc(sizeof(INVENTORY_DATA));
								memset(inter.iobj[ion]->inventory, 0, sizeof(INVENTORY_DATA));
								INVENTORY_DATA * id = inter.iobj[ion]->inventory;
								id->sizex = 3;
								id->sizey = 11;
								id->io = inter.iobj[ion];
							}
						}
						else if (!strcmp(word, "SKIN"))
						{
							std::string temp2;
							pos = GetNextWord(es, pos, temp2);

							if (io)
							{
								if (io->inventory_skin) free(io->inventory_skin);

								io->inventory_skin = strdup(temp2.c_str());
							}
						}
						else if (!strcmp(word, "PLAYERADDFROMSCENE"))
						{
							std::string temp2;
							pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp2);
							}

#endif

							{
								long t = GetTargetByNameTarget(temp2);

								if (t == -2) t = GetInterNum(io);

								if (ValidIONum(t))
								{
									RemoveFromAllInventories(inter.iobj[t]);
									inter.iobj[t]->show = SHOW_FLAG_IN_INVENTORY;

									if (!CanBePutInInventory(inter.iobj[t]))
									{
										PutInFrontOfPlayer(inter.iobj[t]);
									}
								}
							}
						}
						else if ((!strcmp(word, "PLAYERADD")) || (!strcmp(word, "PLAYERADDMULTI")))
						{
							{
								std::string temp2;
								std::string tex;
								pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

								if (NEED_DEBUG)
								{
									strcat(cmd, " ");
									strcat(cmd, temp2);
								}

#endif

								{
									std::string tex2;
									tex2 = "Graph\\Obj3D\\Interactive\\Items\\" + temp2 + ".teo";
									File_Standardize(tex2, tex);

									if (FORBID_SCRIPT_IO_CREATION == 0)
									{
										INTERACTIVE_OBJ * ioo = (INTERACTIVE_OBJ *)AddItem( tex, IO_IMMEDIATELOAD);

										if (ioo != NULL)
										{
											LASTSPAWNED = ioo;
											ioo->scriptload = 1;
											MakeTemporaryIOIdent(ioo);
											SendInitScriptEvent(ioo);

											if (!strcmp(word, "PLAYERADDMULTI"))
											{
												pos = GetNextWord(es, pos, temp2);

												if (ioo->ioflags & IO_GOLD)
												{
													ioo->_itemdata->price = atoi(temp2);
												}
												else
												{
													ioo->_itemdata->maxcount = 9999;

													int iTemp = atoi(temp2);
													ARX_CHECK_SHORT(iTemp);

													ioo->_itemdata->count = ARX_CLEAN_WARN_CAST_SHORT(iTemp);


													if (ioo->_itemdata->count < 1) ioo->_itemdata->count = 1;
												}
											}

											ioo->show = SHOW_FLAG_IN_INVENTORY;

											if (!CanBePutInInventory(ioo))
											{
												PutInFrontOfPlayer(ioo);
											}
										}
									}
									else
									{
										if (!strcmp(word, "PLAYERADDMULTI"))
											pos = GetNextWord(es, pos, temp2);
									}
								}
							}
						}
						else if (!strcmp(word, "ADDFROMSCENE"))
						{
							long xx, yy;
							std::string temp2;

							pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp2);
							}

#endif

							{
								long t = GetTargetByNameTarget(temp2);

								if (t == -2) t = GetInterNum(io);

								if (ValidIONum(t))
								{
									if (ARX_EQUIPMENT_IsPlayerEquip(inter.iobj[t]))
									{
										ARX_EQUIPMENT_UnEquip(inter.iobj[0], inter.iobj[t], 1);
									}
									else
									{
										RemoveFromAllInventories(inter.iobj[t]);
									}

									inter.iobj[t]->scriptload = 0;
									inter.iobj[t]->show = SHOW_FLAG_IN_INVENTORY;

									if(!CanBePutInSecondaryInventory(inter.iobj[ion]->inventory, inter.iobj[t], &xx, &yy)) {
										PutInFrontOfPlayer(inter.iobj[t]);
									}
								}
							}

						}
						else if ((!strcmp(word, "ADD")) || (!strcmp(word, "ADDMULTI")))
						{
							std::string temp2;

							if (inter.iobj[ion]->inventory == NULL)
							{
								pos = GetNextWord(es, pos, temp2);

								if (!strcmp(word, "ADDMULTI"))
									pos = GetNextWord(es, pos, temp2);
							}
							else if (inter.iobj[ion]->inventory != NULL)
							{
								std::string tex;

								pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

								if (NEED_DEBUG)
								{
									strcat(cmd, " ");
									strcat(cmd, temp2);
								}

#endif

								{
									std::string tex2;
									tex2 = "Graph\\Obj3D\\Interactive\\Items\\" + temp2 + ".teo";
									File_Standardize(tex2, tex);

									if (FORBID_SCRIPT_IO_CREATION == 0)
									{
										long multi = -1;

										if (!strcmp(word, "ADDMULTI"))
										{
											pos = GetNextWord(es, pos, temp2);
											multi = atoi(temp2);
										}

										INTERACTIVE_OBJ * ioo = (INTERACTIVE_OBJ *)AddItem( tex, IO_IMMEDIATELOAD);
										long xx, yy;

										if ((ioo != NULL)
												&&	(multi != 0))
										{
											LASTSPAWNED = ioo;
											ioo->scriptload = 1;
											MakeTemporaryIOIdent(ioo);
											SendInitScriptEvent(ioo);


											if (!strcmp(word, "ADDMULTI"))
											{
												if (ioo->ioflags & IO_GOLD)
												{
													ioo->_itemdata->price = multi;
												}
												else
												{
													ioo->_itemdata->maxcount = 9999;

													ARX_CHECK_SHORT(multi);
													ioo->_itemdata->count = ARX_CLEAN_WARN_CAST_SHORT(multi);


													if (ioo->_itemdata->count < 1) ioo->_itemdata->count = 1;
												}
											}

											ioo->show = SHOW_FLAG_IN_INVENTORY;

											if(!CanBePutInSecondaryInventory(inter.iobj[ion]->inventory, ioo, &xx, &yy)) {
												PutInFrontOfPlayer(ioo);
											}
										}
									}
									else
									{
										if (!strcmp(word, "ADDMULTI"))
											pos = GetNextWord(es, pos, temp2);
									}
								}
							}
						}
						else if (!strcmp(word, "DESTROY"))
						{
							if (inter.iobj[ion]->inventory != NULL)
							{
								if(SecondaryInventory == inter.iobj[ion]->inventory) {
									SecondaryInventory = NULL;
								}

								free(inter.iobj[ion]->inventory);
								inter.iobj[ion]->inventory = NULL;
							}
						}
						else if (!strcmp(word, "OPEN"))
						{
							if(SecondaryInventory != inter.iobj[ion]->inventory) {
								SecondaryInventory = inter.iobj[ion]->inventory;
								ARX_SOUND_PlayInterface(SND_BACKPACK);
							}
						}
						else if (!strcmp(word, "CLOSE"))
						{
							if (inter.iobj[ion]->inventory != NULL)
							{
								SecondaryInventory = NULL;
								ARX_SOUND_PlayInterface(SND_BACKPACK);
							}
						}
					}
				}

				break;
			case 'O':

				if (!strcmp(word, "OBJECTHIDE"))
				{
					std::string temp1;
					long megahide = 0;
					pos = GetNextWord(es, pos, word);

					if (word[0] == '-')
					{
						if (iCharIn(word, 'M'))
							megahide = 1;

						pos = GetNextWord(es, pos, word);
					}

					long t = GetTargetByNameTarget(word);

					if (t == -2) t = GetInterNum(io);

					pos = GetNextWord(es, pos, temp1);

					if (ValidIONum(t))
					{
						inter.iobj[t]->GameFlags &= ~GFLAG_MEGAHIDE;

						if ((!strcasecmp(temp1, "ON")) || (!strcasecmp(temp1, "YES")))
						{
							if (megahide)
							{
								inter.iobj[t]->GameFlags |= GFLAG_MEGAHIDE;
								inter.iobj[t]->show = SHOW_FLAG_MEGAHIDE;
							}
							else inter.iobj[t]->show = SHOW_FLAG_HIDDEN;
						}
						else if ((inter.iobj[t]->show == SHOW_FLAG_MEGAHIDE)
								 ||	(inter.iobj[t]->show == SHOW_FLAG_HIDDEN))
						{
							inter.iobj[t]->show = SHOW_FLAG_IN_SCENE;

							if ((inter.iobj[t]->ioflags & IO_NPC)
									&&	(inter.iobj[t]->_npcdata->life <= 0.f)
							   )
							{
								inter.iobj[t]->animlayer[0].cur_anim = inter.iobj[t]->anims[ANIM_DIE];
								inter.iobj[t]->animlayer[1].cur_anim = NULL;
								inter.iobj[t]->animlayer[2].cur_anim = NULL;
								inter.iobj[t]->animlayer[0].ctime = 9999999;
							}
						}
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "OBJECT_HIDE %s %s", word, temp1);

#endif
				}

				break;
			case 'H':
			{
				if (!strcmp(word, "HEROSAY"))
				{
					char tempp[256];
					pos = GetNextWord(es, pos, word);

					if (word[0] == '-')
					{
						if (iCharIn(word, 'D'))
						{
							// do not show (debug say)
							if (FINAL_RELEASE)
							{
								pos = GetNextWord(es, pos, word);
								goto nodraw;
							}
						}

						pos = GetNextWord(es, pos, word);
					}

					strcpy(tempp, GetVarValueInterpretedAsText(word, esss, io).c_str());

					if(tempp[0] == '[') {
						ARX_SPEECH_AddLocalised(tempp);
					} else {
						ARX_SPEECH_Add(tempp);
					}

				nodraw:
					;
					LogDebug << "goto nodraw";
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "HEROSAY %s", word);

#endif
				}
				else if (!strcmp(word, "HALO"))
				{
					pos = GetNextWord(es, pos, word);
					MakeUpcase(word);

					if (iCharIn(word, 'O'))
					{
						if (io) io->halo_native.flags |= HALO_ACTIVE;
					}

					if (iCharIn(word, 'F'))
					{
						if (io) io->halo_native.flags &= ~HALO_ACTIVE;
					}

					if (iCharIn(word, 'N'))
					{
						if (io) io->halo_native.flags |= HALO_NEGATIVE;
					}
					else if (io) io->halo_native.flags &= ~HALO_NEGATIVE;

					if (iCharIn(word, 'L'))
					{
						if (io) io->halo_native.flags |= HALO_DYNLIGHT;
					}
					else if (io)
					{
						io->halo_native.flags &= ~HALO_DYNLIGHT;

						if (ValidDynLight(io->halo_native.dynlight))
							DynLight[io->halo_native.dynlight].exist = 0;

						io->halo_native.dynlight = -1;
					}

					if (iCharIn(word, 'C'))
					{
						std::string temp1;
						std::string temp2;
						std::string temp3;
						pos = GetNextWord(es, pos, temp1);
						pos = GetNextWord(es, pos, temp2);
						pos = GetNextWord(es, pos, temp3);

						if (io)
						{
							io->halo_native.color.r = GetVarValueInterpretedAsFloat(temp1, esss, io);
							io->halo_native.color.g = GetVarValueInterpretedAsFloat(temp2, esss, io);
							io->halo_native.color.b = GetVarValueInterpretedAsFloat(temp3, esss, io);
						}
					}

					if (iCharIn(word, 'S'))
					{
						std::string temp1;
						pos = GetNextWord(es, pos, temp1);

						if (io)
						{
							io->halo_native.radius = GetVarValueInterpretedAsFloat(temp1, esss, io);
						}
					}

					ARX_HALO_SetToNative(io);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "HALO %s", word);

#endif
				}
			}
			break;
			case 'T':

				if (!strcmp(word, "TELEPORT"))
				{
					std::string temp2;
					pos = GetNextWord(es, pos, word);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "TELEPORT ");
						strcat(cmd, word);
					}

#endif

					if (!strcasecmp(word, "behind"))
					{
						ARX_INTERACTIVE_TeleportBehindTarget(io);
					}
					else
					{
						TELEPORT_TO_CONFIRM = 1;
						long playr = 0;
						long initpos = 0;

						if (word[0] == '-')
						{
							long angle = -1;

							if (iCharIn(word, 'A'))
							{
								pos = GetNextWord(es, pos, temp2);

								float fangle = GetVarValueInterpretedAsFloat(temp2, esss, io);
								angle = fangle;

								if (!iCharIn(word, 'L'))
									player.desiredangle.b = player.angle.b = fangle;


							}

							if (iCharIn(word, 'N'))
							{
								TELEPORT_TO_CONFIRM = 0;
							}

							if (iCharIn(word, 'L'))
							{
								pos = GetNextWord(es, pos, word);
#ifdef NEEDING_DEBUG

								if (NEED_DEBUG)
								{
									strcat(cmd, " ");
									strcat(cmd, word);
								}

#endif
								strcpy(TELEPORT_TO_LEVEL, word.c_str());
								pos = GetNextWord(es, pos, word);
#ifdef NEEDING_DEBUG

								if (NEED_DEBUG)
								{
									strcat(cmd, " ");
									strcat(cmd, word);
								}

#endif
								strcpy(TELEPORT_TO_POSITION, word.c_str());

								if (angle == -1) TELEPORT_TO_ANGLE	=	ARX_CLEAN_WARN_CAST_LONG(player.angle.b);
								else TELEPORT_TO_ANGLE = angle;

								CHANGE_LEVEL_ICON = 1;

								if (!TELEPORT_TO_CONFIRM) CHANGE_LEVEL_ICON = 200;

								goto finishteleport;
							}

							if (iCharIn(word, 'P'))
							{
								playr = 1;
								pos = GetNextWord(es, pos, word);
#ifdef NEEDING_DEBUG

								if (NEED_DEBUG)
								{
									strcat(cmd, " ");
									strcat(cmd, word);
								}

#endif
							}

							if (iCharIn(word, 'I'))
								initpos = 1;

						}

#ifdef BUILD_EDITOR
						if(!GAME_EDITOR) {
#endif
							TELEPORT_TO_CONFIRM = 0;
#ifdef BUILD_EDITOR
						}
#endif

						if (initpos == 0)
						{
							long t = GetTargetByNameTarget(word);

							if (t == -2) t = GetInterNum(io);

							if ((t != -1) && (t != -2))
							{
								if (t == -3)
								{
									if ((io->show != SHOW_FLAG_LINKED)
											&&	(io->show != SHOW_FLAG_HIDDEN)
											&&	(io->show != SHOW_FLAG_MEGAHIDE)
											&&	(io->show != SHOW_FLAG_DESTROYED)
											&&	(io->show != SHOW_FLAG_KILLED))
										io->show = SHOW_FLAG_IN_SCENE;

									ARX_INTERACTIVE_Teleport(io, &player.pos);
								}
								else
								{
									if (inter.iobj[t] != NULL)
									{
										Vec3f pos;

										if (GetItemWorldPosition(inter.iobj[t], &pos))
										{
											if (playr)
											{
												ARX_INTERACTIVE_Teleport(inter.iobj[0], &pos);
											}
											else
											{
												if ((io->ioflags & IO_NPC)
														&&	(io->_npcdata->life <= 0))
												{
												}
												else
												{
													if ((io->show != SHOW_FLAG_HIDDEN)
															&&	(io->show != SHOW_FLAG_MEGAHIDE))
														io->show = SHOW_FLAG_IN_SCENE;

													ARX_INTERACTIVE_Teleport(io, &pos);
												}
											}
										}
									}
								}
							}
						}
						else
						{
							if (io)
							{
								if (playr)
								{
									Vec3f pos;

									if (GetItemWorldPosition(io, &pos))
									{
										ARX_INTERACTIVE_Teleport(inter.iobj[0], &pos);
									}
								}
								else
								{
									if ((io->ioflags & IO_NPC)
											&&	(io->_npcdata->life <= 0))
									{
									}
									else
									{
										if ((io->show != SHOW_FLAG_HIDDEN)
												&&	(io->show != SHOW_FLAG_MEGAHIDE))
											io->show = SHOW_FLAG_IN_SCENE;

										ARX_INTERACTIVE_Teleport(io, &io->initpos);
									}
								}
							}
						}
					}

				finishteleport:
					;
					LogDebug << "goto finishteleport";
				}
				else if (!strcmp(word, "TARGETPLAYERPOS"))
				{
					if (io != NULL)
					{
						io->targetinfo = TARGET_PLAYER;
						GetTargetPos(io);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG) sprintf(cmd, "TARGETPLAYERPOS");

#endif
					}

#ifdef NEEDING_DEBUG
					else if (NEED_DEBUG) sprintf(cmd, "ERROR: TARGETPLAYERPOS - NOT AN IO !!!");

#endif
				}
				else if (!strcmp(word, "TWEAK"))
				{
					if (io != NULL)
					{
						pos = GetNextWord(es, pos, word);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcpy(cmd, "TWEAK ");
							strcat(cmd, word);
						}

#endif

						if (!strcasecmp(word, "SKIN"))
						{
							std::string temp1;
							pos = GetNextWord(es, pos, word);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, word);
							}

#endif
							pos = GetNextWord(es, pos, temp1);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp1);
							}

#endif

							if (io)
							{
								ARX_INTERACTIVE_MEMO_TWEAK(io, TWEAK_TYPE_SKIN, word, temp1);
								EERIE_MESH_TWEAK_Skin(io->obj, word, temp1);
							}
						}
						else if (!strcasecmp(word, "ICON"))
						{
							pos = GetNextWord(es, pos, word);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, word);
							}

#endif

							if (io)
							{
								ARX_INTERACTIVE_MEMO_TWEAK(io, TWEAK_TYPE_ICON, word, string());
								ARX_INTERACTIVE_TWEAK_Icon(io, word);
							}
						}
						else
						{
							
							long tw = TWEAK_ERROR;
							if(!strcasecmp(word, "HEAD")) {
								tw = TWEAK_HEAD;
							} else if (!strcasecmp(word, "TORSO")) {
								tw = TWEAK_TORSO;
							} else if (!strcasecmp(word, "LEGS")) {
								tw = TWEAK_LEGS;
							} else if (!strcasecmp(word, "ALL")) {
								tw = TWEAK_ALL;
							} else if (!strcasecmp(word, "UPPER")) {
								tw = TWEAK_UPPER;
							} else if (!strcasecmp(word, "LOWER")) {
								tw = TWEAK_LOWER;
							} else if (!strcasecmp(word, "UP_LO")) {
								tw = TWEAK_UP_LO;
							}

							if (!strcasecmp(word, "REMOVE"))
							{
								tw = TWEAK_REMOVE;
								ARX_INTERACTIVE_MEMO_TWEAK(io, tw, string(), string());
								EERIE_MESH_TWEAK_Do(io, tw, string());
							}
							else
							{
								pos = GetNextWord(es, pos, word);
#ifdef NEEDING_DEBUG

								if (NEED_DEBUG)
								{
									strcat(cmd, " ");
									strcat(cmd, word);
								}

#endif
								std::string path;

								if (io->usemesh != NULL)
									path = io->usemesh;
								else path = io->filename;

								RemoveName(path);
								path += "Tweaks\\";
								path += word;
								path += ".teo";

								if (tw != TWEAK_ERROR)
								{
									ARX_INTERACTIVE_MEMO_TWEAK(io, tw, path, string());
									EERIE_MESH_TWEAK_Do(io, tw, path);
								}
							}
						}
					}

#ifdef NEEDING_DEBUG
					else if (NEED_DEBUG) sprintf(cmd, "ERROR: TWEAK - NOT AN IO !!!");

#endif
				}
				else if ((word[1] == 'I') && (word[2] == 'M') && (word[3] == 'E') && (word[4] == 'R'))
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
			case 'W':

				if (!strcmp(word, "WORLDFADE"))
				{
					std::string temp1;
					pos = GetNextWord(es, pos, word);
					pos = GetNextWord(es, pos, temp1); //duration
					FADEDURATION = GetVarValueInterpretedAsFloat(temp1, esss, io);
					FADESTART = ARX_TIME_GetUL();

					if (!strcasecmp(word, "OUT"))
					{

						std::string temp2;
						std::string temp3;

						pos = GetNextWord(es, pos, temp1);
						pos = GetNextWord(es, pos, temp2);
						pos = GetNextWord(es, pos, temp3);
						FADECOLOR.r = GetVarValueInterpretedAsFloat(temp1, esss, io);
						FADECOLOR.g = GetVarValueInterpretedAsFloat(temp2, esss, io);
						FADECOLOR.b = GetVarValueInterpretedAsFloat(temp3, esss, io);
						FADEDIR = -1;
					}
					else
					{
						FADEDIR = 1;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "WORLD_FADE %s %s", word, temp1);

#endif
				}
				else if (!strcmp(word, "WEAPON"))
				{
					pos = GetNextWord(es, pos, word);

					if ((io) && (io->ioflags & IO_NPC))
					{
						if ((!strcasecmp(word, "DRAW")) || (!strcasecmp(word, "ON")))
						{
							if (io->_npcdata->weaponinhand == 0)
							{
								AcquireLastAnim(io);
								FinishAnim(io, io->animlayer[1].cur_anim);
								io->animlayer[1].cur_anim = NULL;
								io->_npcdata->weaponinhand = -1;
							}
						}
						else
						{
							if (io->_npcdata->weaponinhand == 1)
							{
								AcquireLastAnim(io);
								FinishAnim(io, io->animlayer[1].cur_anim);
								io->animlayer[1].cur_anim = NULL;
								io->_npcdata->weaponinhand = 2;
							}
						}
					}
				}

				break;
			case 'U':

				if (!strcmp(word, "USEMESH"))
				{
					pos = GetNextWord(es, pos, word);
					ARX_INTERACTIVE_MEMO_TWEAK(io, TWEAK_TYPE_MESH, word, string());
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "USE_MESH ");
						strcat(cmd, word);
					}

#endif
					ARX_INTERACTIVE_USEMESH(io, word);

					std::string tex;
					std::string tex1;
					std::string tex2;

					if (io->ioflags & IO_NPC)	tex2 = "Graph\\Obj3D\\Interactive\\NPC\\" + word;
					else if (io->ioflags & IO_FIX)	tex2 = "Graph\\Obj3D\\Interactive\\FIX_INTER\\" + word;
					else if (io->ioflags & IO_ITEM)	tex2 = "Graph\\Obj3D\\Interactive\\Items\\" + word;
					else tex2[0] = 0;

					File_Standardize(tex2, tex);

					if (!tex.empty())
					{
						if (io->usemesh == NULL)
							io->usemesh = (char *)malloc(256);

						strcpy(io->usemesh, tex.c_str());

						if(io->obj) {
							delete io->obj;
							io->obj = NULL;
						}
						
						bool pbox = (!(io->ioflags & IO_FIX) && !(io->ioflags & IO_NPC));
						io->obj = loadObject(tex, pbox);
						
						EERIE_COLLISION_Cylinder_Create(io);
					}
				}

				if (!strcmp(word, "UNSET"))
				{
					pos = GetNextWord(es, pos, word, 1);

					if (IsGlobal(word[0]))
					{
						UNSETVar(svar, &NB_GLOBALS, word);
					}
					else
					{
						UNSETVar(esss->lvar, &esss->nblvar, word);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "UNSET %s", word);

#endif
				}
				else if (!strcmp(word, "USEPATH"))
				{
					pos = GetNextWord(es, pos, word);

					if (io->usepath)
					{
						ARX_USE_PATH * aup = io->usepath;

						if (iCharIn(word, 'B'))
						{
							aup->aupflags &= ~ARX_USEPATH_PAUSE;
							aup->aupflags &= ~ARX_USEPATH_FORWARD;
							aup->aupflags |= ARX_USEPATH_BACKWARD;
						}

						if (iCharIn(word, 'F'))
						{
							aup->aupflags &= ~ARX_USEPATH_PAUSE;
							aup->aupflags |= ARX_USEPATH_FORWARD;
							aup->aupflags &= ~ARX_USEPATH_BACKWARD;
						}

						if (iCharIn(word, 'P'))
						{
							aup->aupflags |= ARX_USEPATH_PAUSE;
							aup->aupflags &= ~ARX_USEPATH_FORWARD;
							aup->aupflags &= ~ARX_USEPATH_BACKWARD;
						}
					}
				}
				else if (!strcmp(word, "UNSETCONTROLLEDZONE"))
				{
					pos = GetNextWord(es, pos, word);
					ARX_PATH * ap = ARX_PATH_GetAddressByName(word);

					if (ap != NULL)
					{
						ap->controled[0] = 0;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "UNSET_CONTROLLED_ZONE %s", word);

#endif
				}

				break;
			case 'E':

				if (!strcmp(word, "ELSE"))
				{
					pos = SkipNextStatement(es, pos);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "ELSE");

#endif
				}
				else if (!strcmp(word, "ENDINTRO"))
				{
					ARX_INTERFACE_EndIntro();
				}
				else if (!strcmp(word, "ENDGAME"))
				{
					REFUSE_GAME_RETURN = 1;

					if (FINAL_COMMERCIAL_DEMO)
						ARX_INTERFACE_EndIntro();
					else
					{
						ARX_SOUND_MixerStop(ARX_SOUND_MixerGame);
						ARX_MENU_Launch();
						ARX_MENU_Clicked_CREDITS();
					}
				}
				else if (!strcmp(word, "EATME"))
				{
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "EATME");

#endif

					if (io) // can only kill IOs
					{
						if (io->ioflags & IO_ITEM)
						{
							player.hunger += io->_itemdata->food_value * 4;

							if (player.hunger > 100.f) player.hunger = 100.f;
						}

						if ((io->ioflags & IO_ITEM) && (io->_itemdata->count > 1))
						{
							io->_itemdata->count--;
						}
						else
						{
							io->show = SHOW_FLAG_KILLED;
							io->GameFlags &= ~GFLAG_ISINTREATZONE;
							RemoveFromAllInventories(io);
							ARX_DAMAGES_ForceDeath(io, EVENT_SENDER);
						}
					}
				}
				else if (!strcmp(word, "EQUIP"))
				{
					pos = GetNextWord(es, pos, word);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "EQUIP ");
						strcat(cmd, word);
					}

#endif
					long unequip = 0;

					if (word[0] == '-')
					{
						if (iCharIn(word, 'R')) unequip = 1;

						pos = GetNextWord(es, pos, word);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, word);
						}

#endif
					}

					long t = GetTargetByNameTarget(word);

					if (t == -3) t = 0;

					if (ValidIONum(t))
					{
						if (unequip)
						{
							INTERACTIVE_OBJ * oes = EVENT_SENDER;
							EVENT_SENDER = inter.iobj[t];
							Stack_SendIOScriptEvent(io, SM_EQUIPOUT);
							EVENT_SENDER = oes;
							ARX_EQUIPMENT_UnEquip(inter.iobj[t], io);
						}
						else
						{
							INTERACTIVE_OBJ * oes = EVENT_SENDER;
							EVENT_SENDER = inter.iobj[t];
							Stack_SendIOScriptEvent(io, SM_EQUIPIN);
							EVENT_SENDER = oes;
							ARX_EQUIPMENT_Equip(inter.iobj[t], io);
						}
					}
				}

				break;
			case 'M':

				if (
					(!strcmp(word, "MUL")))
				{
					std::string temp1;
					std::string temp2;
					float fval;
					float fdval;
					SCRIPT_VAR * sv = NULL;

					pos = GetNextWord(es, pos, temp1, 1);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "MUL ");
						strcat(cmd, temp1);
					}

#endif
					pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcat(cmd, temp2);
					}

#endif

					switch (temp1[0])
					{
						case '$': // GLOBAL TEXT
						case '\xA3': // LOCAL TEXT
							ShowScriptError("Unable to execute this\nOperation on a String", cmd);
							break;
						case '#': // GLOBAL LONG
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fdval = (float)GETVarValueLong(svar, NB_GLOBALS, temp1);
							fval = fval * fdval;
							sv = SETVarValueLong(svar, NB_GLOBALS, temp1, (long)fval);

							if (sv)
								sv->type = TYPE_G_LONG;

							break;
						case '\xA7': // LOCAL LONG
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fdval = (float)GETVarValueLong(esss->lvar, esss->nblvar, temp1);
							fval = fval * fdval;
							sv = SETVarValueLong(esss->lvar, esss->nblvar, temp1, (long)fval);

							if (sv)
								sv->type = TYPE_L_LONG;

							break;
						case '&': // GLOBAL float
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fdval = GETVarValueFloat(svar, NB_GLOBALS, temp1);
							fval = fdval * fval;
							sv = SETVarValueFloat(svar, NB_GLOBALS, temp1, fval);

							if (sv)
								sv->type = TYPE_G_FLOAT;

							break;
						case '@': // LOCAL float
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fdval = GETVarValueFloat(esss->lvar, esss->nblvar, temp1);
							fval = fdval * fval;
							sv = SETVarValueFloat(esss->lvar, esss->nblvar, temp1, fval);

							if (sv)
								sv->type = TYPE_L_FLOAT;

							break;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "%s %s %s", word, temp1, temp2);

#endif

				}
				else if (!strcmp(word, "MOVE"))
				{
					if (io != NULL)
					{
						std::string temp1;
						std::string temp2;
						std::string temp3;
						float t1, t2, t3;

						pos = GetNextWord(es, pos, temp1);
						pos = GetNextWord(es, pos, temp2);
						pos = GetNextWord(es, pos, temp3);

						t1 = GetVarValueInterpretedAsFloat(temp1, esss, io);
						t2 = GetVarValueInterpretedAsFloat(temp2, esss, io);
						t3 = GetVarValueInterpretedAsFloat(temp3, esss, io);

						io->pos.x += t1;
						io->pos.y += t2;
						io->pos.z += t3;
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG) sprintf(cmd, "%s %s %s %s", word, temp1, temp2, temp3);

#endif
					}
				}
				else if (!strcmp(word, "MAGIC"))
				{
					pos = GetNextWord(es, pos, word);

					if (!strcasecmp(word, "OFF"))
						GLOBAL_MAGIC_MODE = 0;
					else
						GLOBAL_MAGIC_MODE = 1;
				}
				else if (!strcmp(word, "MAPMARKER"))
				{
					pos = GetNextWord(es, pos, word);

					if ((!strcasecmp(word, "remove")) || (!strcasecmp(word, "-r")))
					{
						pos = GetNextWord(es, pos, word);
						ARX_MAPMARKER_Remove(word);
					}
					else
					{
						float x = GetVarValueInterpretedAsFloat(word, esss, io);
						pos = GetNextWord(es, pos, word);
						float y = GetVarValueInterpretedAsFloat(word, esss, io);
						pos = GetNextWord(es, pos, word);
						float t = GetVarValueInterpretedAsFloat(word, esss, io);
						long lvl = t;
						pos = GetNextWord(es, pos, word);
						ARX_MAPMARKER_Add(x, y, lvl, word);
					}
				}

				break;
			case '-':
			case '+':

				if ((!strcmp(word, "++")) ||
						(!strcmp(word, "--")))
				{
					std::string temp1;
					long	ival;
					float	fval;
					pos = GetNextWord(es, pos, temp1);

					switch (temp1[0])
					{
						case '#':
							ival = GETVarValueLong(svar, NB_GLOBALS, temp1);

							if (!strcmp(word, "--"))
							{
								SETVarValueLong(svar, NB_GLOBALS, temp1, ival - 1);
							}
							else
							{
								SETVarValueLong(svar, NB_GLOBALS, temp1, ival + 1);
							}

							break;
						case '\xA3':
							ival = GETVarValueLong(esss->lvar, esss->nblvar, temp1);

							if (!strcmp(word, "--"))
							{
								SETVarValueLong(esss->lvar, esss->nblvar, temp1, ival - 1);
							}
							else
							{
								SETVarValueFloat(esss->lvar, esss->nblvar, temp1, ival + 1.f);
							}

							break;
						case '&':
							fval = GETVarValueFloat(svar, NB_GLOBALS, temp1);
							ARX_CHECK_NO_ENTRY();

							if (!strcmp(word, "--"))
							{
								SETVarValueFloat(svar, NB_GLOBALS, temp1, fval  - 1.f);
							}
							else
							{
								SETVarValueFloat(esss->lvar, esss->nblvar, temp1, fval + 1.f);
							}

							break;
						case '@':
							fval = GETVarValueFloat(esss->lvar, esss->nblvar, temp1);

							if (!strcmp(word, "--"))
							{
								SETVarValueFloat(esss->lvar, esss->nblvar, temp1, fval - 1.f);
							}
							else
							{
								SETVarValueFloat(esss->lvar, esss->nblvar, temp1, fval + 1.f);
							}

							break;
						default:
							return BIGERROR;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "%s %s", word, temp1);

#endif
				}

			case 'D':

				if (
					(!strcmp(word, "DEC")) ||
					(!strcmp(word, "DIV")))
				{
					std::string temp1;
					std::string temp2;
					float fval;
					float fdval;
					SCRIPT_VAR * sv = NULL;

					pos = GetNextWord(es, pos, temp1, 1);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, word);
						strcat(cmd, " ");
						strcat(cmd, temp1);
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

					switch (temp1[0]) {
						case '$': // GLOBAL TEXT
						case '\xA3': // LOCAL TEXT
							ShowScriptError("Unable to execute this\nOperation on a String", cmd);
							break;
						case '#': // GLOBAL LONG
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fdval = (float)GETVarValueLong(svar, NB_GLOBALS, temp1);

							if (!strcmp(word, "DEC")) fval = fdval - fval;
							else if (!strcmp(word, "DIV"))
							{
								if (fval != 0.f)	fval = fdval / fval;
							}

							sv = SETVarValueLong(svar, NB_GLOBALS, temp1, (long)fval);

							if (sv != NULL) sv->type = TYPE_G_LONG;

							break;
						case '\xA7': // LOCAL LONG
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fdval = (float)GETVarValueLong(esss->lvar, esss->nblvar, temp1);

							if (!strcmp(word, "DEC")) fval = fdval - fval;
							else if (!strcmp(word, "DIV"))
							{
								if (fval != 0.f)	fval = fdval / fval;
							}

							sv = SETVarValueLong(esss->lvar, esss->nblvar, temp1, (long)fval);

							if (sv != NULL) sv->type = TYPE_L_LONG;

							break;
						case '&': // GLOBAL float
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fdval = GETVarValueFloat(svar, NB_GLOBALS, temp1);

							if (!strcmp(word, "DEC")) fval = fdval - fval;
							else if (!strcmp(word, "DIV"))
							{
								if (fval != 0.f)	fval = fdval / fval;
							}

							sv = SETVarValueFloat(svar, NB_GLOBALS, temp1, fval);

							if (sv != NULL) sv->type = TYPE_G_FLOAT;

							break;
						case '@': // LOCAL float
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fdval = GETVarValueFloat(esss->lvar, esss->nblvar, temp1);

							if (!strcmp(word, "DEC")) fval = fdval - fval;
							else if (!strcmp(word, "DIV"))
							{
								if (fval != 0.f)	fval = fdval / fval;
							}

							sv = SETVarValueFloat(esss->lvar, esss->nblvar, temp1, fval);

							if (sv != NULL) sv->type = TYPE_L_FLOAT;

							break;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "%s %s %s", word, temp1, temp2);

#endif
				}
				else if (!strcmp(word, "DESTROY"))
				{
					std::string temp2;
					pos = GetNextWord(es, pos, temp2); // Source IO
					word = GetVarValueInterpretedAsText(temp2, esss, io);
					long t = GetTargetByNameTarget(word);

					if (t == -2) t = GetInterNum(io); //self

					if (ValidIONum(t)) // can only kill IOs
					{
						long self = 0;
						INTERACTIVE_OBJ * ioo = inter.iobj[t];

						if (io == ioo) self = 1;

						ARX_INTERACTIVE_DestroyIO(ioo);

						if (self) return ACCEPT; // Cannot process further...
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "DESTROY");

#endif
				}
				else if (!strcmp(word, "DETACHNPCFROMPLAYER"))
				{
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "DETACH_NPC_FROM_PLAYER ...OBSOLETE...");

#endif
				}
				else if (!strcmp(word, "DODAMAGE"))
				{
					pos = GetNextWord(es, pos, word); // Source IO
					DamageType type = 0;

					if (word[0] == '-')
					{
						if (iCharIn(word, 'F'))
							type |= DAMAGE_TYPE_FIRE;

						if (iCharIn(word, 'M'))
							type |= DAMAGE_TYPE_MAGICAL;

						if (iCharIn(word, 'P'))
							type |= DAMAGE_TYPE_POISON;

						if (iCharIn(word, 'L'))
							type |= DAMAGE_TYPE_LIGHTNING;

						if (iCharIn(word, 'C'))
							type |= DAMAGE_TYPE_COLD;

						if (iCharIn(word, 'G'))
							type |= DAMAGE_TYPE_GAS;

						if (iCharIn(word, 'E'))
							type |= DAMAGE_TYPE_METAL;

						if (iCharIn(word, 'W'))
							type |= DAMAGE_TYPE_WOOD;

						if (iCharIn(word, 'S'))
							type |= DAMAGE_TYPE_STONE;

						if (iCharIn(word, 'A'))
							type |= DAMAGE_TYPE_ACID;

						if (iCharIn(word, 'O'))
							type |= DAMAGE_TYPE_ORGANIC;

						if (iCharIn(word, 'R'))
							type |= DAMAGE_TYPE_DRAIN_LIFE;

						if (iCharIn(word, 'N'))
							type |= DAMAGE_TYPE_DRAIN_MANA;

						if (iCharIn(word, 'U'))
							type |= DAMAGE_TYPE_PUSH;

						pos = GetNextWord(es, pos, word);
					}

					long t = GetTargetByNameTarget(word);

					if (t == -2) t = GetInterNum(io); //self

					pos = GetNextWord(es, pos, word);
					float fval = GetVarValueInterpretedAsFloat(word, esss, io);

					if (ValidIONum(t))
						ARX_DAMAGES_DealDamages(t, fval, GetInterNum(io), type, &inter.iobj[t]->pos);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "DODAMAGE");

#endif
				}
				else if (!strcmp(word, "DAMAGER"))
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
	
	typedef std::pair<std::map<string, Command *>::iterator, bool> Res;
	
	Res res = commands.insert(std::make_pair(command->getName(), command));
	
	if(!res.second) {
		LogError << "duplicate script command name: " + command->getName();
		delete command;
	}
	
}

void ScriptEvent::init() {
	
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
	
	LogInfo << "scripting system initialized with " << commands.size() << " commands";
}

ScriptEvent::Commands ScriptEvent::commands;
