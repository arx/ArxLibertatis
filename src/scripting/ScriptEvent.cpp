/*
 * ScriptEvent.cpp
 *
 *  Created on: Jan 31, 2011
 *      Author: bmonkey
 */

#include "ScriptEvent.h"

#include "scene/Interactive.h"
#include "core/Time.h"
#include "core/Resource.h"
#include "game/Damage.h"
#include "game/Equipment.h"
#include "game/Missile.h"
#include "game/NPC.h"
#include "gui/Speech.h"
#include "gui/MiniMap.h"
#include "ai/Paths.h"
#include "scene/Scene.h"
#include "scene/GameSound.h"
#include "physics/Actors.h"
#include "physics/CollisionShapes.h"
#include "physics/Collisions.h"
#include "gui/Text.h"
#include "gui/Menu.h"
#include "io/Logger.h"
#include "io/IO.h"
#include "io/PakManager.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/GraphicsModes.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/Math.h"
#include "graphics/data/MeshManipulation.h"

using std::max;
using std::min;

extern long FINAL_COMMERCIAL_DEMO;
extern long GLOBAL_MAGIC_MODE;
extern INTERACTIVE_OBJ * CURRENT_TORCH;
extern EERIE_3D LASTCAMPOS, LASTCAMANGLE;
extern INTERACTIVE_OBJ * CAMERACONTROLLER;
extern char WILL_LAUNCH_CINE[256];
extern float InventoryDir;
extern long REFUSE_GAME_RETURN;
extern long FINAL_RELEASE;
extern long GAME_EDITOR;
extern long TELEPORT_TO_CONFIRM;
extern long CINE_PRELOAD;
extern long PLAY_LOADED_CINEMATIC;
extern long ARX_CONVERSATION;
extern long CHANGE_LEVEL_ICON;
extern long FRAME_COUNT;
extern float g_TimeStartCinemascope;

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
	std::string("")
};

void ARX_SCRIPT_ComputeShortcuts(EERIE_SCRIPT * es)
{
	LogDebug << "ARX_SCRIPT_ComputeShortcuts start";
	long nb = min(MAX_SHORTCUT, SM_MAXCMD);

	for (long j = 1; j < nb; j++)
	{
		es->shortcut[j] = FindScriptPos(es, AS_EVENT[j].name);

		if (es->shortcut[j] >= 0)
		{
			std::string dest;
			GetNextWord(es, es->shortcut[j], dest);

			if (!strcasecmp(dest.c_str(), "{"))
			{
				GetNextWord(es, es->shortcut[j], dest);

				if (!strcasecmp(dest.c_str(), "ACCEPT"))
				{
					es->shortcut[j] = -1;
				}
			}
		}

	}

	LogDebug << "ARX_SCRIPT_ComputeShortcuts end";
}

void ComputeACSPos(ARX_CINEMATIC_SPEECH * acs, INTERACTIVE_OBJ * io, long ionum)
{
	if (!acs) return;

	if (io)
	{

		long id = io->obj->fastaccess.view_attach;

		if (id != -1)
		{
			acs->pos1.x = io->obj->vertexlist3[id].v.x;
			acs->pos1.y = io->obj->vertexlist3[id].v.y;
			acs->pos1.z = io->obj->vertexlist3[id].v.z;
		}
		else
		{
			acs->pos1.x = io->pos.x;
			acs->pos1.y = io->pos.y + io->physics.cyl.height;
			acs->pos1.z = io->pos.z;
		}
	}

	if (ValidIONum(ionum))
	{
		INTERACTIVE_OBJ *	ioo =	inter.iobj[ionum];
		long				id	=	ioo->obj->fastaccess.view_attach;

		if (id != -1)
		{
			acs->pos2.x	=	ioo->obj->vertexlist3[id].v.x;
			acs->pos2.y	=	ioo->obj->vertexlist3[id].v.y;
			acs->pos2.z	=	ioo->obj->vertexlist3[id].v.z;
		}
		else
		{
			acs->pos2.x	=	ioo->pos.x;
			acs->pos2.y	=	ioo->pos.y + ioo->physics.cyl.height;
			acs->pos2.z	=	ioo->pos.z;
		}
	}
}

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
}
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

bool HasVisibility(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * ioo)
{
	register float x0, y0, z0;
	register float x1, y1, z1;
	x0 = io->pos.x;
	y0 = io->pos.y;
	z0 = io->pos.z;
	x1 = ioo->pos.x;
	y1 = ioo->pos.y;
	z1 = ioo->pos.z;
	float dist = Distance3D(x0, y0, z0, x1, y1, z1);

	if (dist > 20000) return false;

	float ab = MAKEANGLE(io->angle.b);
	EERIE_3D orgn, dest;

	orgn.x = x0;
	orgn.y = y0 - 90.f;
	orgn.z = z0;
	dest.x = x1;
	dest.y = y1 - 90.f;
	dest.z = z1;
	float aa = GetAngle(orgn.x, orgn.z, dest.x, dest.z);
	aa = MAKEANGLE(degrees(aa));

	if ((aa < ab + 90.f) && (aa > ab - 90.f))
	{
		//font
		ARX_TEXT_Draw(GDevice, InBookFont, 300, 320, 0, 0, "VISIBLE", D3DRGB(1.f, 0.f, 0.f));
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

void Stack_SendMsgToAllNPC_IO(long msg, const char * dat)
{
	for (long i = 0; i < inter.nbmax; i++)
	{
		if ((inter.iobj[i]) && (inter.iobj[i]->ioflags & IO_NPC))
		{
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

bool UNSETVar(SCRIPT_VAR * svf, long* nb, const std::string& name)
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


//TODO(lubosz): THIS IS DEFINETLY TOO LONG FOR ONE FUNCTION
long ScriptEvent::send(EERIE_SCRIPT * es, long msg, const std::string& params, INTERACTIVE_OBJ * io, const std::string& evname, long info)
{
	//TODO(lubosz): globals
	INTERACTIVE_OBJ * LASTSPAWNED = NULL;
	long LINEEND = 0;
	long PauseScript = 0;
	long GLOB = 0;
	long RELOADING = 0;

	LogDebug << "ScriptEvent::send msg=" << msg << " ("
	         << ((msg < sizeof(AS_EVENT)/sizeof(*AS_EVENT) - 1) ? AS_EVENT[msg].name : "unknown")
	         << ")" << " params=" << params
	         << " io=" << Logger::nullstr(io ? io->filename : NULL)
	         << " evame=" << evname << " info=" << info;

	if (io)
	{
		if ((io->GameFlags & GFLAG_MEGAHIDE)
				&&	(msg != SM_RELOAD))
			return ACCEPT;

		if (io->show == SHOW_FLAG_DESTROYED) // destroyed
			return ACCEPT;
	}

	Event_Total_Count++;

	if (io)
	{
		io->stat_count++;

		if (io->ioflags & IO_FREEZESCRIPT)
		{
			if (msg == SM_LOAD) return ACCEPT;

			return REFUSE;
		}

		if (io->ioflags & IO_NPC)
		{
			if ((io->_npcdata->life <= 0.f) && (msg != SM_DEAD) && (msg != SM_DIE) && (msg != SM_EXECUTELINE) && (msg != SM_RELOAD)
					&& (msg != SM_EXECUTELINE)
					&& (msg != SM_INVENTORY2_OPEN) && (msg != SM_INVENTORY2_CLOSE))
				return ACCEPT;
		}

		//changement d'armes si on casse
		if (((io->ioflags & IO_FIX)	||	(io->ioflags & IO_ITEM))
				&&	(msg == SM_BREAK))
		{
			ManageCasseDArme(io);
		}
	}

	if (((EDITMODE) || (PauseScript))
			&& (msg != SM_LOAD)
			&& (msg != SM_INIT)
			&& (msg != SM_INITEND))
		return ACCEPT;

	long ret = ACCEPT;
	std::string temp;
	char cmd[256];
	char eventname[64];
	long brackets = 0;
	long pos;

	// Retrieves in esss script pointer to script holding variables.
	EERIE_SCRIPT * esss = (EERIE_SCRIPT *)es->master;

	if (esss == NULL) esss = es;

	// Finds script position to execute code...
	if ( !evname.empty())
	{
		strcpy(eventname, "ON ");
		strcat(eventname, evname.c_str());
		pos = FindScriptPos( es, eventname );
	}
	else
	{
		if (msg == SM_EXECUTELINE)
			pos = info;
		else
		{
			switch (msg)
			{
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
				case SM_KEY_PRESSED:
				{
					float dwCurrTime = ARX_TIME_Get();

					if ((dwCurrTime - g_TimeStartCinemascope) < 3000)
					{
						return REFUSE;
					}
				}
				break;
			}

			if (msg < MAX_SHORTCUT)
				pos = es->shortcut[msg];
			else
			{
				if (((msg >= SM_MAXCMD))
						&& (msg != SM_EXECUTELINE) && (evname.empty()))
				{

					return ACCEPT;
				}

				pos = FindScriptPos(es, AS_EVENT[msg].name);
			}
		}
	}

	if (pos <= -1)
	{
		GLOB = 1;
		return ACCEPT;
	}

	GLOB = 0;



	MakeSSEPARAMS(params.c_str());


	if (msg != SM_EXECUTELINE)
	{
		if (!evname.empty())
		{
			pos += strlen(eventname); // adding 'ON ' length
			LogDebug << eventname << " received";

		}
		else
		{
			pos += AS_EVENT[msg].name.length();
			LogDebug << AS_EVENT[msg].name << " received";

		}

		if ((pos = GetNextWord(es, pos, temp)) < 0) return ACCEPT;

		if (temp[0] != '{')
		{
			LogError << "ERROR: No bracket after event";
			return ACCEPT;
		}
		else brackets = 1;
	}
	else
	{
		LogDebug << "EXECUTELINE received";
		brackets = 0;
	}

	while (pos >= 0)
	{

		cmd[0] = 0;

		if (pos >= es->size - 1) 	return ACCEPT;

		if ((pos = GetNextWord(es, pos, temp)) < 0) 	return ACCEPT;

		if ((msg == SM_EXECUTELINE) && (LINEEND == 1)) return ACCEPT;

		MakeStandard(temp);

		switch (temp[0])
		{
			case '}':
				brackets--;
				break;
			case '{':
				brackets++;
				break;
			case '/':

				if (temp[1] == '/') pos = GotoNextLine(es, pos);

				break;
			case '>':

				if (temp[1] == '>') pos = GotoNextLine(es, pos);

				break;
			case 'B':

				if (!strcmp(temp, "BEHAVIOR"))
				{
					unsigned long behavior = 0; //BEHAVIOUR_NONE;
					float behavior_param = 0.f;
					pos = GetNextWord(es, pos, temp);
					LogDebug << "BEHAVIOR "<< temp;

					if (!strcasecmp(temp, "STACK"))
					{
						ARX_NPC_Behaviour_Stack(io);
					}
					else if (!strcasecmp(temp, "UNSTACK"))
					{
						ARX_NPC_Behaviour_UnStack(io);
					}
					else if (!strcasecmp(temp, "UNSTACKALL"))
					{
						ARX_NPC_Behaviour_Reset(io);
					}
					else
					{
						if (temp[0] == '-')
						{

							if (iCharIn(temp, 'L'))
								behavior |= BEHAVIOUR_LOOK_AROUND;

							if (iCharIn(temp, 'S'))
								behavior |= BEHAVIOUR_SNEAK;

							if (iCharIn(temp, 'D'))
								behavior |= BEHAVIOUR_DISTANT;

							if (iCharIn(temp, 'M'))
								behavior |= BEHAVIOUR_MAGIC;

							if (iCharIn(temp, 'F'))
								behavior |= BEHAVIOUR_FIGHT;

							if (iCharIn(temp, 'A'))
								behavior |= BEHAVIOUR_STARE_AT;

							if (CharIn(temp, '0') && io && (io->ioflags & IO_NPC))
								io->_npcdata->tactics = 0;

							if (CharIn(temp, '1') && io && (io->ioflags & IO_NPC))
								io->_npcdata->tactics = 0;

							if (CharIn(temp, '2') && io && (io->ioflags & IO_NPC))
								io->_npcdata->tactics = 0;

							pos = GetNextWord(es, pos, temp);

							LogDebug <<  temp;

						}


						if (!strcasecmp(temp, "GO_HOME"))
							behavior |= BEHAVIOUR_GO_HOME;
						else if (!strcasecmp(temp, "FRIENDLY"))
						{
							if ((io) && (io->ioflags & IO_NPC)) io->_npcdata->movemode = NOMOVEMODE;

							behavior |= BEHAVIOUR_FRIENDLY;
						}
						else if (!strcasecmp(temp, "MOVE_TO"))
						{
							if ((io) && (io->ioflags & IO_NPC)) io->_npcdata->movemode = WALKMODE;

							behavior |= BEHAVIOUR_MOVE_TO;
						}
						else if (!strcasecmp(temp, "FLEE"))
						{
							behavior |= BEHAVIOUR_FLEE;
							pos = GetNextWord(es, pos, temp);
							behavior_param = GetVarValueInterpretedAsFloat(temp, esss, io);

							if ((io) && (io->ioflags & IO_NPC)) io->_npcdata->movemode = RUNMODE;
						}
						else if (!strcasecmp(temp, "LOOK_FOR"))
						{
							behavior |= BEHAVIOUR_LOOK_FOR;
							pos = GetNextWord(es, pos, temp);
							behavior_param = GetVarValueInterpretedAsFloat(temp, esss, io);

							if ((io) && (io->ioflags & IO_NPC)) io->_npcdata->movemode = WALKMODE;
						}
						else if (!strcasecmp(temp, "HIDE"))
						{
							behavior |= BEHAVIOUR_HIDE;
							pos = GetNextWord(es, pos, temp);
							behavior_param = GetVarValueInterpretedAsFloat(temp, esss, io);

							if ((io) && (io->ioflags & IO_NPC)) io->_npcdata->movemode = WALKMODE;
						}
						else if (!strcasecmp(temp, "WANDER_AROUND"))
						{
							behavior |= BEHAVIOUR_WANDER_AROUND;
							pos = GetNextWord(es, pos, temp);
							behavior_param = GetVarValueInterpretedAsFloat(temp, esss, io);

							if ((io) && (io->ioflags & IO_NPC)) io->_npcdata->movemode = WALKMODE;
						}
						else if (!strcasecmp(temp, "GUARD"))
						{
							behavior |= BEHAVIOUR_GUARD;

							if (io)
							{
								io->targetinfo = -2;

								if (io->ioflags & IO_NPC) io->_npcdata->movemode = NOMOVEMODE;
							}
						}

						if ((io) && (io->ioflags & IO_NPC))
						{

							ARX_CHECK_LONG(behavior_param);
							ARX_NPC_Behaviour_Change(io, behavior, ARX_CLEAN_WARN_CAST_LONG(behavior_param));


						}
					}
				}

				if (!strcmp(temp, "BOOK"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'A')) //MAGIC
							Book_Mode = 2;

						if (iCharIn(temp, 'E')) //Equip
							Book_Mode = 1;

						if (iCharIn(temp, 'M')) //Map
							Book_Mode = 3;

						pos = GetNextWord(es, pos, temp);
					}

					if (!strcasecmp(temp, "OPEN"))
					{
						ARX_INTERFACE_BookOpenClose(1);
					}
					else if (!strcasecmp(temp, "CLOSE"))
					{
						ARX_INTERFACE_BookOpenClose(2);
					}
				}

				break;
			case 'A':

				if (!strcmp(temp, "ACCEPT"))
				{
					ret = ACCEPT;
					ClearSubStack(es);

					LogDebug << "  ACCEPT";
					goto end;
				}
				else if (!strcmp(temp, "ADDBAG"))
				{
					ARX_PLAYER_AddBag();
					LogDebug << "ADD_BAG " << temp;

				}
				else if (!strcmp(temp, "ACTIVATEPHYSICS"))
				{
					ARX_INTERACTIVE_ActivatePhysics(GetInterNum(io));
				}
				else if (!strcmp(temp, "ADDXP"))
				{
					pos = GetNextWord(es, pos, temp);
					float val = GetVarValueInterpretedAsFloat(temp, esss, io);
					ARX_PLAYER_Modify_XP((long)val);
					LogDebug << "ADD_XP "<< temp;
				}
				else if (!strcmp(temp, "ADDGOLD"))
				{
					pos = GetNextWord(es, pos, temp);
					float val = GetVarValueInterpretedAsFloat(temp, esss, io);

					if (val != 0) ARX_SOUND_PlayInterface(SND_GOLD);


					ARX_CHECK_LONG(val);
					ARX_PLAYER_AddGold(ARX_CLEAN_WARN_CAST_LONG(val));

					LogDebug << "ADD_GOLD " << temp;
				}
				else if (!strcmp(temp, "ATTRACTOR"))
				{
					pos = GetNextWord(es, pos, temp);
					long t = GetTargetByNameTarget(temp.c_str());

					if (t == -2) t = GetInterNum(io);

					pos = GetNextWord(es, pos, temp);
					float val = 0.f;
					float val2 = 0.f;

					if (strcasecmp(temp, "OFF"))
					{
						val = GetVarValueInterpretedAsFloat(temp, esss, io);
						pos = GetNextWord(es, pos, temp);
						val2 = GetVarValueInterpretedAsFloat(temp, esss, io);
					}

					ARX_SPECIAL_ATTRACTORS_Add(t, val, val2);
				}
				else if (!strcmp(temp, "AMBIANCE"))
				{
					float volume(1.0F);

					pos = GetNextWord(es, pos, temp);

					LogDebug << "AMBIANCE " << temp;

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'V'))
						{
							pos = GetNextWord(es, pos, temp);
							LogDebug << temp;
							volume = GetVarValueInterpretedAsFloat(temp, esss, io);

							pos = GetNextWord(es, pos, temp);
							LogDebug << temp;

							ARX_SOUND_PlayScriptAmbiance(temp.c_str(), ARX_SOUND_PLAY_LOOPED, volume * ( 1.0f / 100 ));
						}
						else if (iCharIn(temp, 'N'))
						{
							pos = GetNextWord(es, pos, temp);
							LogDebug << temp;

							ARX_SOUND_PlayScriptAmbiance(temp.c_str(), ARX_SOUND_PLAY_ONCE);
						}
						else if (iCharIn(temp, 'M'))
						{
							std::string temp2;

							pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, temp2);
								strcat(cmd, " ");
							}

#endif

							ARX_SOUND_SetAmbianceTrackStatus(temp.c_str(), temp2.c_str(), 1); //1 = Mute

							pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, temp);
								strcat(cmd, " ");
							}

#endif
						}
						else if (iCharIn(temp, 'U'))
						{
							std::string temp2;

							pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, temp2.c_str());
								strcat(cmd, " ");
							}

#endif

							ARX_SOUND_SetAmbianceTrackStatus(temp.c_str(), temp2.c_str(), 0);//0 = unmute

							pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, temp);
								strcat(cmd, " ");
							}

#endif
						}
					}
					else if (!strcasecmp(temp, "KILL"))
						ARX_SOUND_KillAmbiances();
					else
						ARX_SOUND_PlayScriptAmbiance(temp.c_str());
				}
				else if (!strcmp(temp, "ANCHORBLOCK"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io)
					{
						if ((!strcasecmp(temp, "ON")) || (!strcasecmp(temp, "YES")))
						{
							ANCHOR_BLOCK_By_IO(io, 1);
						}
						else
							ANCHOR_BLOCK_By_IO(io, 0);
					}
				}
				else if (!strcmp(temp, "ATTACHNPCTOPLAYER"))
				{
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "ATTACH_NPC_TO_PLAYER ...OBSOLETE...");

#endif
				}
				else if (!strcmp(temp, "ATTACH"))
				{
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "ATTACH ");
					}

#endif
					std::string temp1;
					std::string temp2;
					pos = GetNextWord(es, pos, temp); // Source IO
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcat(cmd, temp.c_str());
						strcat(cmd, " ");
					}

#endif
					long t = GetTargetByNameTarget(temp.c_str());

					if (t == -2) t = GetInterNum(io); //self

					pos = GetNextWord(es, pos, temp1); // source action_point
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcat(cmd, temp1);
						strcat(cmd, " ");
					}

#endif
					pos = GetNextWord(es, pos, temp); // target IO
					long t2 = GetTargetByNameTarget(temp.c_str());
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcat(cmd, temp);
						strcat(cmd, " ");
					}

#endif

					if (t2 == -2) t2 = GetInterNum(io); //self

					pos = GetNextWord(es, pos, temp2); // target action_point
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcat(cmd, temp2);
						strcat(cmd, " ");
					}

#endif

					if (ARX_INTERACTIVE_Attach(t, t2, temp1.c_str(), temp2.c_str()))
					{
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, "--> success");
						}

#endif
					}

#ifdef NEEDING_DEBUG
					else if (NEED_DEBUG)
					{
						strcat(cmd, "--> failure");
					}

#endif
				}

				break;
			case 'G':

				if (!strcmp(temp, "GOTO"))
				{
					if (msg == SM_EXECUTELINE) msg = SM_DUMMY;

					if ((pos = GetNextWord(es, pos, temp)) == -1)
					{
						ret = ACCEPT;
						goto end;
					}

					pos = FindLabelPos(es, temp);

					if (pos == -1) return ACCEPT;

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "GOTO %s", temp);

#endif
				}
				else if (!strcmp(temp, "GOSUB"))
				{
					if (msg == SM_EXECUTELINE) msg = SM_DUMMY;

					if ((pos = GetNextWord(es, pos, temp)) == -1) return ACCEPT;

					if (!InSubStack(es, pos)) return BIGERROR;

					pos = FindLabelPos(es, temp);

					if (pos == -1) return ACCEPT;

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "GOSUB %s", temp);

#endif
				}
				else if (!strcmp(temp, "GMODE"))
				{
					pos = GetNextWord(es, pos, temp);
				}

				break;
			case 'R':

				if (!strcmp(temp, "REFUSE"))
				{
					ClearSubStack(es);
					ret = REFUSE;
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) LogDebug << "  REFUSE";

#endif
					goto end;
				}
				else if (!strcmp(temp, "REVIVE"))
				{
					long tmp = GetNextWord(es, pos, temp);
					long init = 0;

					if (temp[0] == '-')
					{
						pos = tmp;

						if ((iCharIn(temp, 'I'))) init = 1;
					}

					ARX_NPC_Revive(io, init);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) LogDebug << "REVIVE";

#endif
					goto end;
				}
				else if (!strcmp(temp, "RIDICULOUS"))
				{
					ARX_PLAYER_MakeFreshHero();
				}
				else if (!strcmp(temp, "REPAIR"))
				{
					pos = GetNextWord(es, pos, temp);
					long t = GetTargetByNameTarget(temp);

					if (t == -2) t = GetInterNum(io); //self

					pos = GetNextWord(es, pos, temp);
					float val = GetVarValueInterpretedAsFloat(temp, esss, io);

					if (val < 0.f) val = 0.f;
					else if (val > 100.f) val = 100.f;

					if (ValidIONum(t))
						ARX_DAMAGES_DurabilityRestore(inter.iobj[t], val);
				}
				else if (!strcmp(temp, "RANDOM"))
				{
					std::string temp1;
					pos = GetNextWord(es, pos, temp1);
					float val = (float)atof(temp1.c_str());

					if (val < 0.f) val = 0.f;
					else if (val > 100.f) val = 100.f;

					float t = rnd() * 100.f;

					if (val < t)
					{
						pos = SkipNextStatement(es, pos);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "RANDOM %s", temp1);

#endif
				}
				else if (!strcmp(temp, "RETURN"))
				{
					if ((pos = GetSubStack(es)) == -1) return BIGERROR;

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "RETURN");

#endif
				}
				else if (!strcmp(temp, "REPLACEME"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io)
					{

						std::string tex;
						std::string tex2;

						if (io->ioflags & IO_NPC)
							tex2 = "Graph\\Obj3D\\Interactive\\NPC\\" + temp + ".teo";
						else if (io->ioflags & IO_FIX)
							tex2 = "Graph\\Obj3D\\Interactive\\FIX_INTER\\" + temp + ".teo";
						else
							tex2 = "Graph\\Obj3D\\Interactive\\Items\\" + temp + ".teo";

						File_Standardize(tex2, tex);
						EERIE_3D last_angle;
						memcpy(&last_angle, &io->angle, sizeof(EERIE_3D));
						INTERACTIVE_OBJ * ioo = (INTERACTIVE_OBJ *)AddInteractive(GDevice, tex.c_str(), -1); //AddItem(GDevice,tex);

						if (ioo != NULL)
						{
							LASTSPAWNED = ioo;
							ioo->scriptload = 1;
							ioo->initpos.x = io->initpos.x;
							ioo->initpos.y = io->initpos.y;
							ioo->initpos.z = io->initpos.z;
							ioo->pos.x = io->pos.x;
							ioo->pos.y = io->pos.y;
							ioo->pos.z = io->pos.z;
							ioo->angle.a = io->angle.a;
							ioo->angle.b = io->angle.b;
							ioo->angle.g = io->angle.g;
							ioo->move.x = io->move.x;
							ioo->move.y = io->move.y;
							ioo->move.z = io->move.z;
							ioo->show = io->show;

							if (io == DRAGINTER)
								Set_DragInter(ioo);

							long neww = GetInterNum(ioo);
							long oldd = GetInterNum(io);

							if (io->ioflags & IO_ITEM)
							{
								if (io->_itemdata->count > 1)
								{
									io->_itemdata->count--;
									SendInitScriptEvent(ioo);
									CheckForInventoryReplaceMe(ioo, io);
								}
								else goto finishit;
							}
							else
							{
							finishit:
								;

								for (long i = 0; i < MAX_SPELLS; i++)
								{
									if ((spells[i].exist) && (spells[i].caster == oldd))
									{
										spells[i].caster = neww;
									}
								}

								io->show = SHOW_FLAG_KILLED;
								ReplaceInAllInventories(io, ioo);
								SendInitScriptEvent(ioo);
								memcpy(&ioo->angle, &last_angle, sizeof(EERIE_3D));
								TREATZONE_AddIO(ioo, neww);

								for (int i = 0; i < MAX_EQUIPED; i++)
								{
									if	((player.equiped[i] != 0)
											&&	ValidIONum(player.equiped[i]))
									{
										INTERACTIVE_OBJ * equiped = inter.iobj[player.equiped[i]];

										if	(equiped == io)
										{
											ARX_EQUIPMENT_UnEquip(inter.iobj[0], io, 1);
											ARX_EQUIPMENT_Equip(inter.iobj[0], ioo);
										}
									}
								}

								if (io->scriptload)
								{
									ReleaseInter(io);
									return REFUSE;
								}

								TREATZONE_RemoveIO(io);
								return REFUSE;
							}
						}

#ifdef NEEDING_DEBUG

						if (NEED_DEBUG) sprintf(cmd, "REPLACE_ME %s", temp);

#endif
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "REPLACE_ME %s --> Failure Not An IO", temp);

#endif
				}
				else if (!strcmp(temp, "ROTATE"))
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
						io->angle.a += t1;
						io->angle.b += t2;
						io->angle.g += t3;

						if (io->nb_lastanimvertex != io->obj->nbvertex)
						{
							free(io->lastanimvertex);
							io->lastanimvertex = NULL;
						}

						io->lastanimtime = 0;
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG) sprintf(cmd, "%s %s %s %s", temp, temp1, temp2, temp3);

#endif
					}
				}
				else if (!strcmp(temp, "RUNE"))
				{
					pos		 = GetNextWord(es, pos, temp);
					long add = 0;

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'A')) add = 1;

						if (iCharIn(temp, 'R')) add = -1;

						pos = GetNextWord(es, pos, temp);
					}
					if( !add && strcmp( temp, "ALL" ) )	//ARX: jycorbel (2010-07-19) - In case of "RUNE ALL" cheat, add is useless it is no risky to keep it uninitialized.
					{
						ARX_CHECK_NO_ENTRY(); //add used without being initialized, remove rune
					}

					if (!strcasecmp(temp, "AAM"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_AAM);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_AAM);
					}
					else if (!strcasecmp(temp, "CETRIUS"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_CETRIUS);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_CETRIUS);
					}
					else if (!strcasecmp(temp, "COMUNICATUM"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_COMUNICATUM);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_COMUNICATUM);
					}
					else if (!strcasecmp(temp, "COSUM"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_COSUM);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_COSUM);
					}
					else if (!strcasecmp(temp, "FOLGORA"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_FOLGORA);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_FOLGORA);
					}
					else if (!strcasecmp(temp, "FRIDD"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_FRIDD);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_FRIDD);
					}
					else if (!strcasecmp(temp, "KAOM"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_KAOM);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_KAOM);
					}
					else if (!strcasecmp(temp, "MEGA"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_MEGA);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_MEGA);
					}
					else if (!strcasecmp(temp, "MORTE"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_MORTE);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_MORTE);
					}
					else if (!strcasecmp(temp, "MOVIS"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_MOVIS);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_MOVIS);
					}
					else if (!strcasecmp(temp, "NHI"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_NHI);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_NHI);
					}
					else if (!strcasecmp(temp, "RHAA"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_RHAA);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_RHAA);
					}
					else if (!strcasecmp(temp, "SPACIUM"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_SPACIUM);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_SPACIUM);
					}
					else if (!strcasecmp(temp, "STREGUM"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_STREGUM);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_STREGUM);
					}
					else if (!strcasecmp(temp, "TAAR"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_TAAR);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_TAAR);
					}
					else if (!strcasecmp(temp, "TEMPUS"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_TEMPUS);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_TEMPUS);
					}
					else if (!strcasecmp(temp, "TERA"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_TERA);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_TERA);
					}
					else if (!strcasecmp(temp, "VISTA"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_VISTA);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_VISTA);
					}
					else if (!strcasecmp(temp, "VITAE"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_VITAE);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_VITAE);
					}
					else if (!strcasecmp(temp, "YOK"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_YOK);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_YOK);
					}
					else if (!strcasecmp(temp, "ALL"))
						ARX_PLAYER_Rune_Add_All();

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "RUNE %ld %s", add, temp);

#endif
				}

				break;
			case 'C':

				if (!strcmp(temp, "CINE")) //CAMERA_ACTIVATE
				{
					long preload = 0;
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "CINE ");
					}

#endif
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcat(cmd, temp);
					}

#endif

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'P'))
							preload = 1;

						pos = GetNextWord(es, pos, temp);
					}

					if (!strcasecmp(temp, "KILL"))
					{
						DANAE_KillCinematic();
					}
					else if (!strcasecmp(temp, "PLAY"))
					{
						PLAY_LOADED_CINEMATIC = 1;
						ARX_TIME_Pause();
					}
					else
					{
						{
							char temp2[256];
							strcpy(temp2, "Graph\\interface\\illustrations\\");
							strcat(temp2, temp.c_str());
							strcat(temp2, ".cin");
							temp += ".cin";

							if (PAK_FileExist(temp2))
							{
								strcpy(WILL_LAUNCH_CINE, temp.c_str());
								CINE_PRELOAD = preload;
							}
						}
					}
				}
				else if (!strcmp(temp, "COLLISION"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "COLLISION ");
						strcat(cmd, temp);
					}

#endif

					if (io)
					{
						if ((!strcasecmp(temp, "ON")) || (!strcasecmp(temp, "YES")))
						{
							if (io->ioflags & IO_NO_COLLISIONS)
							{
								long col = 0;

								for (long kkk = 0; kkk < inter.nbmax; kkk++)
								{
									INTERACTIVE_OBJ * ioo = inter.iobj[kkk];

									if (ioo)
									{
										if (IsCollidingIO(io, ioo))
										{
											INTERACTIVE_OBJ * oes = EVENT_SENDER;
											EVENT_SENDER = ioo;
											Stack_SendIOScriptEvent(io, SM_COLLISION_ERROR_DETAIL);
											EVENT_SENDER = oes;
											col = 1;
										}
									}
								}

								if (col)
								{
									INTERACTIVE_OBJ * oes = EVENT_SENDER;
									EVENT_SENDER = NULL;
									Stack_SendIOScriptEvent(io, SM_COLLISION_ERROR);
									EVENT_SENDER = oes;
								}
							}

							io->ioflags &= ~IO_NO_COLLISIONS;
						}
						else
							io->ioflags |= IO_NO_COLLISIONS;
					}
				}
				else if (!strcmp(temp, "CAMERACONTROL"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "CAMERA_CONTROL ");
						strcat(cmd, temp);
					}

#endif

					if (!strcasecmp(temp, "ON")) CAMERACONTROLLER = io;
					else CAMERACONTROLLER = NULL;
				}
				else if (!strcmp(temp, "CONVERSATION"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "CONVERSATION ");
						strcat(cmd, temp);
					}

#endif
					long nb_people = 0;

					if (temp[0] == '-')
					{
						if (CharIn(temp, '0')) nb_people = 0;

						if (CharIn(temp, '1')) nb_people = 1;

						if (CharIn(temp, '2')) nb_people = 2;

						if (CharIn(temp, '3')) nb_people = 3;

						if (CharIn(temp, '4')) nb_people = 4;

						if (CharIn(temp, '5')) nb_people = 5;

						if (CharIn(temp, '6')) nb_people = 6;

						if (CharIn(temp, '7')) nb_people = 7;

						if (CharIn(temp, '8')) nb_people = 8;

						if (CharIn(temp, '9')) nb_people = 9;

						pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp);
						}

#endif
					}

					if (!strcmp(temp, "ON"))
					{
						ARX_CONVERSATION = 1;
					}
					else
					{
						ARX_CONVERSATION = 0;
					}

					if ((nb_people) && ARX_CONVERSATION)
					{
						main_conversation.actors_nb = nb_people;

						for (long j = 0; j < nb_people; j++)
						{
							pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp);
							}

#endif
							long t = GetTargetByNameTarget(temp);

							if (t == -2) //self
							{
								for (long k = 0; k < inter.nbmax; k++)
								{
									if (io == inter.iobj[k])
									{
										t = k;
										break;
									}
								}
							}

							main_conversation.actors[j] = t;
						}
					}
				}
				else if (!strcmp(temp, "CAMERAACTIVATE"))
				{
					FRAME_COUNT = -1;
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "CAMERA_ACTIVATE ");
						strcat(cmd, temp);
					}

#endif

					if (!strcasecmp(temp, "NONE"))
					{
						MasterCamera.exist = 0;
					}
					else
					{
						FRAME_COUNT = 0;
						long t = GetTargetByNameTarget(temp);

						if (t == -2) t = GetInterNum(io);

						if (t != -1)
						{
							if (inter.iobj[t]->ioflags & IO_CAMERA)
							{
								MasterCamera.exist |= 2;
								MasterCamera.want_io = inter.iobj[t];
								MasterCamera.want_aup = (ARX_USE_PATH *)inter.iobj[t]->usepath;
								MasterCamera.want_cam = &inter.iobj[t]->_camdata->cam;
							}

						}
					}
				}
				else if (!strcmp(temp, "CAMERASMOOTHING"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "CAMERA_SMOOTHING ");
						strcat(cmd, temp);
					}

#endif

					if (io != NULL)
					{
						if (io->ioflags & IO_CAMERA)
						{
							float fo = GetVarValueInterpretedAsFloat(temp, esss, io);
							EERIE_CAMERA * cam = (EERIE_CAMERA *)io->_camdata;
							cam->smoothing = fo;
						}
					}
				}
				else if (!strcmp(temp, "CINEMASCOPE"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "CINEMASCOPE ");
						strcat(cmd, temp);
					}

#endif
					long smooth = 0;

					if (temp[0] == '-')
					{
						if ((temp[1] == 's') || (temp[1] == 'S'))
						{
							smooth = 1;
						}

						pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp);
						}

#endif
					}

					if (!strcasecmp(temp, "ON"))
					{
						ARX_INTERFACE_SetCinemascope(1, smooth);
					}
					else ARX_INTERFACE_SetCinemascope(0, smooth);
				}
				else if (!strcmp(temp, "CAMERAFOCAL"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "CAMERA_FOCAL ");
						strcat(cmd, temp);
					}

#endif

					if (io != NULL)
					{
						if (io->ioflags & IO_CAMERA)
						{
							float fo = GetVarValueInterpretedAsFloat(temp, esss, io);
							EERIE_CAMERA * cam = (EERIE_CAMERA *)io->_camdata;
							cam->focal = fo;

							if (cam->focal < 100) cam->focal = 100;
							else if (cam->focal > 800) cam->focal = 800;
						}
					}
				}
				else if (!strcmp(temp, "CAMERATRANSLATETARGET"))
				{
					std::string temp2;
					std::string temp3;

					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp2);
					pos = GetNextWord(es, pos, temp3);

					if (io != NULL)
					{
						if (io->ioflags & IO_CAMERA)
						{
							EERIE_3D fo;
							fo.x = GetVarValueInterpretedAsFloat(temp, esss, io);
							fo.y = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fo.z = GetVarValueInterpretedAsFloat(temp3, esss, io);
							EERIE_CAMERA * cam = (EERIE_CAMERA *)io->_camdata;
							cam->translatetarget.x = fo.x;
							cam->translatetarget.y = fo.y;
							cam->translatetarget.z = fo.z;
						}
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "CAMERA_TRANSLATE_TARGET %s %s %s", temp, temp2, temp3);

#endif
				}
				else if (!strcmp(temp, "CLOSESTEALBAG"))
				{
					if ((io) && (io->ioflags & IO_NPC))
					{
						if (player.Interface & INTER_STEAL)
						{
							INTERACTIVE_OBJ * pio = NULL;

							if (SecondaryInventory != NULL)
							{
								pio = (INTERACTIVE_OBJ *)SecondaryInventory->io;
							}
							else if (player.Interface & INTER_STEAL)
							{
								pio = ioSteal;
							}

							if ((pio != NULL) && (pio == ioSteal))
							{
								InventoryDir = -1;
								SendIOScriptEvent(pio, SM_INVENTORY2_CLOSE);
								TSecondaryInventory = SecondaryInventory;
								SecondaryInventory = NULL;
							}

						}
					}
				}

				break;
			case 'Q':

				if (!strcmp(temp, "QUAKE"))
				{
					float f1, f2, f3; // intensity duration period
					pos = GetNextWord(es, pos, temp);
					f1 = GetVarValueInterpretedAsFloat(temp, esss, io);
					pos = GetNextWord(es, pos, temp);
					f2 = GetVarValueInterpretedAsFloat(temp, esss, io);
					pos = GetNextWord(es, pos, temp);
					f3 = GetVarValueInterpretedAsFloat(temp, esss, io);
					AddQuakeFX(f1, f2, f3, 1);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "QUAKE");

#endif
				}
				else if (!strcmp(temp, "QUEST"))
				{
					pos = GetNextWord(es, pos, temp);
					ARX_PLAYER_Quest_Add(temp.c_str());
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "QUEST %s", temp);

#endif
				}

				break;
			case 'N':

				if (!strcmp(temp, "NOP"))
				{
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "NOP");

#endif
				}
				else if (!strcmp(temp, "NOTE"))
				{
					ARX_INTERFACE_NOTE_TYPE type = NOTE_TYPE_UNDEFINED;
					pos = GetNextWord(es, pos, temp);

					if (!strcasecmp(temp, "NOTE"))
						type = NOTE_TYPE_NOTE;

					if (!strcasecmp(temp, "NOTICE"))
						type = NOTE_TYPE_NOTICE;

					if (!strcasecmp(temp, "BOOK"))
						type = NOTE_TYPE_BOOK;

					pos = GetNextWord(es, pos, temp);

					if (player.Interface & INTER_NOTE)
						ARX_INTERFACE_NoteClose();
					else
					{
						ARX_INTERFACE_NoteOpen(type, temp.c_str());
					}
				}

				break;
			case 'S':

				if (!strcmp(temp, "SPELLCAST"))
				{

					std::string temp2;
					long duration = -1;
					long flags = 0;
					long dur = 0;
					pos = GetNextWord(es, pos, temp); // switch or level

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'K'))
						{
							pos = GetNextWord(es, pos, temp); //spell id
							long spellid = GetSpellId(temp);
							long from = GetInterNum(io);

							if (ValidIONum(from))
							{
								long sp = ARX_SPELLS_GetInstanceForThisCaster(spellid, from);

								if (sp >= 0)
								{
									spells[sp].tolive = 0;
								}
							}

							goto suite;
						}

						if (iCharIn(temp, 'D'))
						{
							flags |= SPELLCAST_FLAG_NOCHECKCANCAST;
							pos = GetNextWord(es, pos, temp2); // duration
							duration = (long)GetVarValueInterpretedAsFloat(temp2, esss, io);

							if (duration <= 0)
							{
								duration = 99999999;
							}

							dur = 1;
						}

						if (iCharIn(temp, 'X'))
						{
							flags |= SPELLCAST_FLAG_NOSOUND;
						}

						if (iCharIn(temp, 'M'))
						{
							flags |= SPELLCAST_FLAG_NOCHECKCANCAST;
							flags |= SPELLCAST_FLAG_NODRAW;
						}

						if (iCharIn(temp, 'S'))
						{
							flags |= SPELLCAST_FLAG_NOCHECKCANCAST;
							flags |= SPELLCAST_FLAG_NOANIM;
						}

						if (iCharIn(temp, 'F'))
						{
							flags |= SPELLCAST_FLAG_NOCHECKCANCAST;
							flags |= SPELLCAST_FLAG_NOMANA;
						}

						if (iCharIn(temp, 'Z'))
						{
							flags |= SPELLCAST_FLAG_RESTORE;
						}

						pos = GetNextWord(es, pos, temp); // level
					}

					long level;
					level = (long)GetVarValueInterpretedAsFloat(temp, esss, io);

					if (level < 1) level = 1;
					else if (level > 10) level = 10;

					if (!dur)
						duration = 1000 + level * 2000;

					pos = GetNextWord(es, pos, temp); //spell id
					long spellid;
					spellid = GetSpellId(temp);
					pos = GetNextWord(es, pos, temp); //spell target
					long t;
					t = GetTargetByNameTarget(temp);

					if (t <= -1) t = GetInterNum(io);

					if ((t >= 0)
							&&	(t < inter.nbmax)
							&&	(spellid != -1))
					{
						if (io != inter.iobj[0])
						{
							flags |= SPELLCAST_FLAG_NOCHECKCANCAST;
						}

						TryToCastSpell(io, spellid, level, t, flags, duration);
					}

				suite:
					;
				}
				else if (!strcmp(temp, "SPEAK")) // speak say_ident actions
				{
					ARX_CINEMATIC_SPEECH acs;
					acs.type = ARX_CINE_SPEECH_NONE;


					std::string temp2;
					long ttt;

					long player		=	0;
					long voixoff	=	0;
					long notext		=	0;

					pos = GetNextWord(es, pos, temp2);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "SPEAK ");
						strcat(cmd, temp2);
					}

#endif

					long mood			=	ANIM_TALK_NEUTRAL;
					long unbreakable	=	0;
					MakeUpcase(temp2);

					if (!strcasecmp(temp2, "KILLALL"))
					{
						ARX_SPEECH_Reset();
					}
					else
					{
						if (temp2[0] == '-')
						{
							if (iCharIn(temp2, 'T')) notext		=	1;

							if (iCharIn(temp2, 'U')) unbreakable =	1;

							if (iCharIn(temp2, 'P')) player		=	1;

							if (iCharIn(temp2, 'H')) mood		=	ANIM_TALK_HAPPY;

							if (iCharIn(temp2, 'A')) mood		=	ANIM_TALK_ANGRY;

							if (iCharIn(temp2, 'O'))
							{
								voixoff	=	2;

								//Crash when we set speak pitch to 1,
								//Variable use for a division, 0 is not possible
								//To find
							}

							if (iCharIn(temp2, 'C'))
							{
								FRAME_COUNT	=	0;
								pos			=	GetNextWord(es, pos, temp2);

								if (!strcasecmp(temp2, "KEEP"))
								{
									acs.type	=	ARX_CINE_SPEECH_KEEP;
									acs.pos1.x	=	LASTCAMPOS.x;
									acs.pos1.y	=	LASTCAMPOS.y;
									acs.pos1.z	=	LASTCAMPOS.z;
									acs.pos2.a	=	LASTCAMANGLE.a;
									acs.pos2.b	=	LASTCAMANGLE.b;
									acs.pos2.g	=	LASTCAMANGLE.g;
								}

								if (!strcasecmp(temp2, "ZOOM"))
								{
									acs.type			=	ARX_CINE_SPEECH_ZOOM;
									pos					=	GetNextWord(es, pos, temp2);
									acs.startangle.a	=	GetVarValueInterpretedAsFloat(temp2, esss, io);
									pos					=	GetNextWord(es, pos, temp2);
									acs.startangle.b	=	GetVarValueInterpretedAsFloat(temp2, esss, io);
									pos					=	GetNextWord(es, pos, temp2);
									acs.endangle.a		=	GetVarValueInterpretedAsFloat(temp2, esss, io);
									pos					=	GetNextWord(es, pos, temp2);
									acs.endangle.b		=	GetVarValueInterpretedAsFloat(temp2, esss, io);
									pos					=	GetNextWord(es, pos, temp2);
									acs.startpos		=	GetVarValueInterpretedAsFloat(temp2, esss, io);
									pos					=	GetNextWord(es, pos, temp2);
									acs.endpos			=	GetVarValueInterpretedAsFloat(temp2, esss, io);


									//ARX_CHECK_NO_ENTRY(); //ARX: xrichter (2010-07-20) - temp2 is often (always?) a string number and GetTargetByNameTarget return -1. To be careful if temp2 is not a string number, we choose to test GetTargetByNameTarget return value.
									acs.ionum			=	GetTargetByNameTarget(temp2);

									if (acs.ionum == -2)   //means temp2 is "me" or "self"
										acs.ionum		=	GetInterNum(io);



									if (player)
										ComputeACSPos(&acs, inter.iobj[0], acs.ionum);
									else ComputeACSPos(&acs, io, -1);
								}
								else if ((!strcasecmp(temp2, "CCCTALKER_L"))
										 || (!strcasecmp(temp2, "CCCTALKER_R")))
								{
									if (!strcasecmp(temp2, "CCCTALKER_R"))
										acs.type = ARX_CINE_SPEECH_CCCTALKER_R;
									else acs.type = ARX_CINE_SPEECH_CCCTALKER_L;

									pos = GetNextWord(es, pos, temp2);
									acs.ionum = GetTargetByNameTarget(temp2);

									if (acs.ionum == -2) acs.ionum = GetInterNum(io);

									pos = GetNextWord(es, pos, temp2);
									acs.startpos = GetVarValueInterpretedAsFloat(temp2, esss, io);
									pos = GetNextWord(es, pos, temp2);
									acs.endpos = GetVarValueInterpretedAsFloat(temp2, esss, io);

									if (player)
										ComputeACSPos(&acs, inter.iobj[0], acs.ionum);
									else ComputeACSPos(&acs, io, acs.ionum);
								}
								else if ((!strcasecmp(temp2, "CCCLISTENER_L"))
										 || (!strcasecmp(temp2, "CCCLISTENER_R")))
								{
									if (!strcasecmp(temp2, "CCCLISTENER_R"))
										acs.type = ARX_CINE_SPEECH_CCCLISTENER_R;
									else acs.type = ARX_CINE_SPEECH_CCCLISTENER_L;

									pos = GetNextWord(es, pos, temp2);
									acs.ionum = GetTargetByNameTarget(temp2);

									if (acs.ionum == -2) acs.ionum = GetInterNum(io);

									pos = GetNextWord(es, pos, temp2);
									acs.startpos = GetVarValueInterpretedAsFloat(temp2, esss, io);
									pos = GetNextWord(es, pos, temp2);
									acs.endpos = GetVarValueInterpretedAsFloat(temp2, esss, io);

									if (player)
										ComputeACSPos(&acs, inter.iobj[0], acs.ionum);
									else ComputeACSPos(&acs, io, acs.ionum);
								}
								else if ((!strcasecmp(temp2, "SIDE"))
										 || (!strcasecmp(temp2, "SIDE_L"))
										 || (!strcasecmp(temp2, "SIDE_R")))
								{
									if (!strcasecmp(temp2, "SIDE_L"))
										acs.type = ARX_CINE_SPEECH_SIDE_LEFT;
									else acs.type = ARX_CINE_SPEECH_SIDE;

									pos = GetNextWord(es, pos, temp2);
									acs.ionum = GetTargetByNameTarget(temp2);

									if (acs.ionum == -2) acs.ionum = GetInterNum(io);

									pos = GetNextWord(es, pos, temp2);
									acs.startpos = GetVarValueInterpretedAsFloat(temp2, esss, io);
									pos = GetNextWord(es, pos, temp2);
									acs.endpos = GetVarValueInterpretedAsFloat(temp2, esss, io);
									//startdist
									pos = GetNextWord(es, pos, temp2);
									acs.f0 = GetVarValueInterpretedAsFloat(temp2, esss, io);
									//enddist
									pos = GetNextWord(es, pos, temp2);
									acs.f1 = GetVarValueInterpretedAsFloat(temp2, esss, io);
									//height modifier
									pos = GetNextWord(es, pos, temp2);
									acs.f2 = GetVarValueInterpretedAsFloat(temp2, esss, io);

									if (player)
										ComputeACSPos(&acs, inter.iobj[0], acs.ionum);
									else ComputeACSPos(&acs, io, acs.ionum);
								}
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

						long speechnum;
						std::string temp1 = GetVarValueInterpretedAsText(temp2, esss, io);

						if (!strcmp(temp2, "[]"))
						{
							ARX_SPEECH_ClearIOSpeech(io);
						}
						else
						{
							if (notext) voixoff |= ARX_SPEECH_FLAG_NOTEXT;

							if (!CINEMASCOPE) voixoff |= ARX_SPEECH_FLAG_NOTEXT;

							if (player)
							{
								speechnum = ARX_SPEECH_AddSpeech(inter.iobj[0], temp1.c_str(), PARAM_LOCALISED, mood, voixoff);
							}
							else
								speechnum = ARX_SPEECH_AddSpeech(io, temp1.c_str(), PARAM_LOCALISED, mood, voixoff);

							ttt = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp2);
							}

#endif

							if ((!LINEEND) && (speechnum >= 0))
							{
								char timername[128];
								char timername2[128];
								ARX_SCRIPT_Timer_GetDefaultName(timername2);
								sprintf(timername, "SPEAK_%s", timername2);
								aspeech[speechnum].scrpos = pos;
								aspeech[speechnum].es = es;
								aspeech[speechnum].ioscript = io;

								if (unbreakable) aspeech[speechnum].flags |= ARX_SPEECH_FLAG_UNBREAKABLE;

								memcpy(&aspeech[speechnum].cine, &acs, sizeof(ARX_CINEMATIC_SPEECH));
								pos = GotoNextLine(es, pos);
							}

							LINEEND = 0;
						}
					}
				}
				else if (!strcmp(temp, "SHOPCATEGORY"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io->shop_category) free(io->shop_category);

					io->shop_category = NULL;
					io->shop_category = (char *)malloc(temp.length() + 1);

					if (io->shop_category)
						strcpy(io->shop_category, temp.c_str());
				}
				else if (!strcmp(temp, "SHOPMULTIPLY"))
				{
					pos = GetNextWord(es, pos, temp);
					io->shop_multiply = GetVarValueInterpretedAsFloat(temp, esss, io);
				}
				else if (!strcmp(temp, "SETPOISONOUS"))
				{
					float poisonous = 0.f;
					float poisonous_count = 0.f;
					pos = GetNextWord(es, pos, temp);
					poisonous = GetVarValueInterpretedAsFloat(temp, esss, io);;
					pos = GetNextWord(es, pos, temp);
					poisonous_count = GetVarValueInterpretedAsFloat(temp, esss, io);

					if (poisonous_count == 0)
					{
						io->poisonous_count = 0;
					}
					else
					{

						ARX_CHECK_SHORT(poisonous);
						ARX_CHECK_SHORT(poisonous_count);

						io->poisonous		= ARX_CLEAN_WARN_CAST_SHORT(poisonous);
						io->poisonous_count = ARX_CLEAN_WARN_CAST_SHORT(poisonous_count);

					}

				}
				else if (!strcmp(temp, "SETPLATFORM"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!strcasecmp(temp, "ON"))
					{
						io->GameFlags |= GFLAG_PLATFORM;
					}
					else io->GameFlags &= ~GFLAG_PLATFORM;
				}
				else if (!strcmp(temp, "SETGORE"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!strcasecmp(temp, "ON"))
					{
						io->GameFlags &= ~GFLAG_NOGORE;
					}
					else io->GameFlags |= GFLAG_NOGORE;
				}
				else if (!strcmp(temp, "SETUNIQUE"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!strcasecmp(temp, "ON"))
					{
						io->ioflags |= IO_UNIQUE;
					}
					else io->ioflags &= ~IO_UNIQUE;
				}
				else if (!strcmp(temp, "SETBLACKSMITH"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!strcasecmp(temp, "ON"))
					{
						io->ioflags |= IO_BLACKSMITH;
					}
					else io->ioflags &= ~IO_BLACKSMITH;
				}
				else if (!strcmp(temp, "SETELEVATOR"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!strcasecmp(temp, "ON"))
					{
						io->GameFlags |= GFLAG_ELEVATOR;
					}
					else io->GameFlags &= ~GFLAG_ELEVATOR;
				}
				else if (!strcmp(temp, "SETTRAP")) // -1 = off
				{
					pos = GetNextWord(es, pos, temp);

					if ((io) && (io->ioflags & IO_FIX))
					{
						if (!strcasecmp(temp, "off"))
						{
							io->_fixdata->trapvalue = -1;
						}
						else
						{
							io->_fixdata->trapvalue = (char)GetVarValueInterpretedAsFloat(temp, esss, io);

							if (io->_fixdata->trapvalue < -1) io->_fixdata->trapvalue = -1;

							if (io->_fixdata->trapvalue > 100) io->_fixdata->trapvalue = 100;
						}
					}
				}
				else if (!strcmp(temp, "SETSECRET")) // -1 = off
				{
					pos = GetNextWord(es, pos, temp);

					if ((io) && (io->ioflags & IO_FIX))
					{
						if (!strcasecmp(temp, "off"))
						{
							io->secretvalue = -1;
						}
						else
						{
							io->secretvalue = (char)GetVarValueInterpretedAsFloat(temp, esss, io);

							if (io->secretvalue < -1) io->secretvalue = -1;

							if (io->secretvalue > 100) io->secretvalue = 100;
						}
					}
				}
				else if (!strcmp(temp, "SETDETECT")) // -1 = off
				{
					pos = GetNextWord(es, pos, temp);

					if ((io) && (io->ioflags & IO_NPC))
					{
						if (!strcasecmp(temp, "off"))
						{
							io->_npcdata->fDetect = -1;
						}
						else
						{
							io->_npcdata->fDetect = (char)GetVarValueInterpretedAsFloat(temp, esss, io);

							if (io->_npcdata->fDetect < -1)	io->_npcdata->fDetect = -1;

							if (io->_npcdata->fDetect > 100) io->_npcdata->fDetect = 100;
						}
					}
				}
				else if (!strcmp(temp, "SETSTEAL")) // -1 = off
				{
					pos = GetNextWord(es, pos, temp);

					if ((io) && (io->ioflags & IO_ITEM))
					{
						if (!strcasecmp(temp, "off"))
						{
							io->_itemdata->stealvalue = -1;
						}
						else
						{
							io->_itemdata->stealvalue = (char)GetVarValueInterpretedAsFloat(temp, esss, io);

							if (io->_itemdata->stealvalue < -1)	io->_itemdata->stealvalue = -1;

							if (io->_itemdata->stealvalue > 100) io->_itemdata->stealvalue = 100;

							if (io->_itemdata->stealvalue == 100) io->_itemdata->stealvalue = -1;
						}
					}
				}
				else if (!strcmp(temp, "SETLIGHT")) // -1 = off  for ITEM only
				{
					pos = GetNextWord(es, pos, temp);

					if ((io) && (io->ioflags & IO_ITEM))
					{
						if (!strcasecmp(temp, "off"))
						{
							io->_itemdata->stealvalue = -1;
						}
						else
						{
							io->_itemdata->LightValue = (char)GetVarValueInterpretedAsFloat(temp, esss, io);

							if (io->_itemdata->LightValue < -1)  io->_itemdata->LightValue = -1;

							if (io->_itemdata->LightValue > 1) io->_itemdata->LightValue = 1;
						}
					}
				}
				else if (!strcmp(temp, "SETBLOOD"))
				{
					pos = GetNextWord(es, pos, temp);
					float r = GetVarValueInterpretedAsFloat(temp, esss, io);
					pos = GetNextWord(es, pos, temp);
					float g = GetVarValueInterpretedAsFloat(temp, esss, io);
					pos = GetNextWord(es, pos, temp);
					float b = GetVarValueInterpretedAsFloat(temp, esss, io);

					if (io->ioflags & IO_NPC)
					{
						io->_npcdata->blood_color = D3DRGB(r, g, b);
					}
				}
				else if (!strcmp(temp, "SETMATERIAL"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io)
					{
						io->material = ARX_MATERIAL_GetIdByName(temp.c_str());
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_MATERIAL %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETSPEAKPITCH"))
				{
					pos = GetNextWord(es, pos, temp);

					if ((io) && (io->ioflags & IO_NPC))
					{
						io->_npcdata->speakpitch = GetVarValueInterpretedAsFloat(temp, esss, io);

						if (io->_npcdata->speakpitch < 0.6f) io->_npcdata->speakpitch = 0.6f;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_SPEAK_PITCH %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETFOOD"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io->ioflags & IO_ITEM)
					{
						io->_itemdata->food_value = (char)GetVarValueInterpretedAsFloat(temp, esss, io);
					}
				}
				else if (!strcmp(temp, "SETSPEED"))
				{
					pos = GetNextWord(es, pos, temp);
					io->basespeed = GetVarValueInterpretedAsFloat(temp, esss, io);

					if (io->basespeed < 0.f) io->basespeed = 0.f;

					if (io->basespeed > 10.f) io->basespeed = 10.f;
				}
				else if (!strcmp(temp, "SETSTAREFACTOR"))
				{
					pos = GetNextWord(es, pos, temp);

					if ((io) && (io->ioflags & IO_NPC))
					{
						io->_npcdata->stare_factor = GetVarValueInterpretedAsFloat(temp, esss, io);
					}
				}
				else if (!strcmp(temp, "SETGROUP"))
				{
					pos = GetNextWord(es, pos, temp);
					long remove = 0;

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'R'))
							remove = 1;

						pos = GetNextWord(es, pos, temp);
					}

					std::string temp1 = GetVarValueInterpretedAsText(temp, esss, io);

					if (remove)
					{
						if (!strcasecmp(temp1, "DOOR")) io->GameFlags &= ~GFLAG_DOOR;

						ARX_IOGROUP_Remove(io, temp1.c_str());
					}
					else
					{
						if (!strcasecmp(temp1, "DOOR")) io->GameFlags |= GFLAG_DOOR;

						ARX_IOGROUP_Add(io, temp1.c_str());
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_GROUP %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETNPCSTAT"))
				{
					std::string temp2;
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp2);
					ARX_NPC_SetStat(io, temp.c_str(), GetVarValueInterpretedAsFloat(temp2, esss, io));
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_NPC_STAT %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETXPVALUE"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io && (io->ioflags & IO_NPC))
					{
						io->_npcdata->xpvalue = (long)GetVarValueInterpretedAsFloat(temp, esss, io);

						if (io->_npcdata->xpvalue < 0) io->_npcdata->xpvalue = 0;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_XP_VALUE %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETNAME"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io != NULL)
					{
						strcpy(io->locname, temp.c_str());
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SETNAME %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETPLAYERTWEAK"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "SET_PLAYER_TWEAK ");
						strcat(cmd, temp.c_str());
					}

#endif

					if (io->tweakerinfo == NULL)
					{
						io->tweakerinfo = (IO_TWEAKER_INFO *)malloc(sizeof(IO_TWEAKER_INFO));

						if (io->tweakerinfo)
						{
							memset(io->tweakerinfo, 0, sizeof(IO_TWEAKER_INFO));
						}
					}

					if (!strcasecmp(temp, "SKIN"))
					{
						std::string temp2;
						pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp.c_str());
						}

#endif
						pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp2.c_str());
						}

#endif

						if (io->tweakerinfo)
						{
							strcpy(io->tweakerinfo->skintochange, temp.c_str());
							strcpy(io->tweakerinfo->skinchangeto, temp2.c_str());
						}
					}
					else	// Mesh Tweaker...
					{
						pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp.c_str());
						}

#endif

						if (io->tweakerinfo)
						{
							strcpy(io->tweakerinfo->filename, temp.c_str());
						}
					}
				}
				else if (!strcmp(temp, "SETCONTROLLEDZONE"))
				{
					pos = GetNextWord(es, pos, temp);
					ARX_PATH * ap = ARX_PATH_GetAddressByName(temp.c_str());

					if (ap != NULL)
					{
						char title[64];
						temp = GetName(io->filename);
						sprintf(title, "%s_%04ld", temp.c_str(), io->ident);
						strcpy(ap->controled, title);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_CONTROLLED_ZONE %s", temp);

#endif
				}
				else if ((!strcmp(temp, "SETSTATUS")) || (!strcmp(temp, "SETMAINEVENT")))
				{
					pos = GetNextWord(es, pos, temp);
					ARX_SCRIPT_SetMainEvent(io, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SETMAINEVENT %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETMOVEMODE"))
				{
					pos = GetNextWord(es, pos, temp);

					if ((io != NULL) && (io->ioflags & IO_NPC))
					{
						if (!strcmp(temp, "WALK"))	ARX_NPC_ChangeMoveMode(io, WALKMODE);

						if (!strcmp(temp, "RUN"))	ARX_NPC_ChangeMoveMode(io, RUNMODE);

						if (!strcmp(temp, "NONE"))	ARX_NPC_ChangeMoveMode(io, NOMOVEMODE);

						if (!strcmp(temp, "SNEAK"))	ARX_NPC_ChangeMoveMode(io, SNEAKMODE);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SETMOVEMODE %s", temp);

#endif
				}
				else if (!strcmp(temp, "SPAWN"))
				{
					std::string temp2;
					std::string tmptext;
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "SPAWN ");
						strcat(cmd, temp);
					}

#endif
					MakeUpcase(temp);

					if (!strcmp(temp, "NPC"))
					{
						pos = GetNextWord(es, pos, temp); // object to spawn.
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp);
						}

#endif
						pos = GetNextWord(es, pos, temp2); // object ident for position
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp2);
						}

#endif
						long t = GetTargetByNameTarget(temp2);

						if (t == -2) t = GetInterNum(io);

						if ((t >= 0) && (t < inter.nbmax))
						{
							char tex2[256];
							sprintf(tex2, "Graph\\Obj3D\\Interactive\\NPC\\%s", temp.c_str());
							File_Standardize(tex2, tmptext);
							INTERACTIVE_OBJ * ioo;

							if (FORBID_SCRIPT_IO_CREATION == 0)
							{
								ioo = AddNPC(GDevice, tmptext.c_str(), IO_IMMEDIATELOAD);

								if (ioo)
								{
									LASTSPAWNED = ioo;
									ioo->scriptload = 1;
									ioo->pos.x = inter.iobj[t]->pos.x;
									ioo->pos.y = inter.iobj[t]->pos.y;
									ioo->pos.z = inter.iobj[t]->pos.z;

									ioo->angle.a = inter.iobj[t]->angle.a;
									ioo->angle.b = inter.iobj[t]->angle.b;
									ioo->angle.g = inter.iobj[t]->angle.g;
									MakeTemporaryIOIdent(ioo);
									SendInitScriptEvent(ioo);

									if (inter.iobj[t]->ioflags & IO_NPC)
									{
										float dist = inter.iobj[t]->physics.cyl.radius + ioo->physics.cyl.radius + 10;
										EERIE_3D ofs;
										ofs.x = -EEsin(radians(inter.iobj[t]->angle.b)) * dist;
										ofs.y = 0.f;
										ofs.z = EEcos(radians(inter.iobj[t]->angle.b)) * dist;
										ioo->pos.x += ofs.x;
										ioo->pos.z += ofs.z;
									}

									TREATZONE_AddIO(ioo, GetInterNum(ioo));
								}
							}
						}
					}
					else if (!strcmp(temp, "ITEM"))
					{
						pos = GetNextWord(es, pos, temp); // object to spawn.
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp);
						}

#endif
						pos = GetNextWord(es, pos, temp2); // object ident for position
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp2);
						}

#endif
						long t = GetTargetByNameTarget(temp2);

						if (t == -2) t = GetInterNum(io);

						if ((t >= 0) && (t < inter.nbmax))
						{
							char tex2[256];
							sprintf(tex2, "Graph\\Obj3D\\Interactive\\ITEMS\\%s", temp.c_str());
							File_Standardize(tex2, tmptext);
							INTERACTIVE_OBJ * ioo;

							if (FORBID_SCRIPT_IO_CREATION == 0)
							{
								ioo = AddItem(GDevice, tmptext.c_str(), IO_IMMEDIATELOAD);

								if (ioo)
								{
									MakeTemporaryIOIdent(ioo);
									LASTSPAWNED = ioo;
									ioo->scriptload = 1;
									ioo->pos.x = inter.iobj[t]->pos.x;
									ioo->pos.y = inter.iobj[t]->pos.y;
									ioo->pos.z = inter.iobj[t]->pos.z;
									ioo->angle.a = inter.iobj[t]->angle.a;
									ioo->angle.b = inter.iobj[t]->angle.b;
									ioo->angle.g = inter.iobj[t]->angle.g;
									MakeTemporaryIOIdent(ioo);
									SendInitScriptEvent(ioo);
								}

								TREATZONE_AddIO(ioo, GetInterNum(ioo));
							}
						}
					}
					else if (!strcmp(temp, "FIREBALL"))
					{
						GetTargetPos(io);
						EERIE_3D pos;
						pos.x = io->pos.x;
						pos.y = io->pos.y;
						pos.z = io->pos.z;

						if (io->ioflags & IO_NPC) pos.y -= 80.f;

						ARX_MISSILES_Spawn(io, MISSILE_FIREBALL, &pos, &io->target);
					}
				}
				else if (!strcmp(temp, "SETOBJECTTYPE"))
				{
					pos = GetNextWord(es, pos, temp);
					long val = 1; // flag to add

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'R'))
						{
							val = 0; // flag to remove
						}

						pos = GetNextWord(es, pos, temp);
					}

					ARX_EQUIPMENT_SetObjectType(io, temp.c_str(), val);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_OBJECT_TYPE %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETRIGHTHAND"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_RIGHT_HAND %s ...OBSOLETE...", temp);

#endif
				}
				else if (!strcmp(temp, "SETHUNGER"))
				{
					pos = GetNextWord(es, pos, temp);
					player.hunger = GetVarValueInterpretedAsFloat(temp, esss, io);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_HUNGER %s", temp);

#endif
				}

				else if (!strcmp(temp, "SETLEFTHAND"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_LEFT_HAND %s ...OBSOLETE...", temp);

#endif
				}
				else if (!strcmp(temp, "SETSHIELD"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_SHIELD %s ...OBSOLETE...", temp);

#endif
				}
				else if (!strcmp(temp, "SETTWOHANDED"))
				{
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_TWO_HANDED ...OBSOLETE...");

#endif
				}
				else if (!strcmp(temp, "SETINTERACTIVITY"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!strcasecmp(temp, "NONE"))
					{
						io->GameFlags &= ~GFLAG_INTERACTIVITY;
						io->GameFlags &= ~GFLAG_INTERACTIVITYHIDE;
					}
					else if (!strcasecmp(temp, "HIDE"))
					{
						io->GameFlags &= ~GFLAG_INTERACTIVITY;
						io->GameFlags |= GFLAG_INTERACTIVITYHIDE;
					}
					else
					{
						io->GameFlags |= GFLAG_INTERACTIVITY;
						io->GameFlags &= ~GFLAG_INTERACTIVITYHIDE;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_INTERACTIVITY %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETEQUIP"))
				{
					std::string temp2;
					temp2[0] = 0;
					std::string temp3;
					pos = GetNextWord(es, pos, temp3);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "SET_EQUIP ");
						strcat(cmd, temp3.c_str());
					}

#endif

					if (temp3[0] == '-')
					{
						if (!strcasecmp(temp3, "-r"))  ARX_EQUIPMENT_Remove_All_Special(io);
						else
						{
							pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp);
							}

#endif
							pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp2.c_str());
							}

#endif
						}
					}
					else
					{
						temp = temp3;
						pos = GetNextWord(es, pos, temp2);
						temp3[0] = 0;
					}

					short flag = 0;

					if (temp2[0])
					{
						if (temp2[temp2.length()-1] == '%') flag = 1;
					}
					else flag = 0;

					ARX_EQUIPMENT_SetEquip(io, temp3.c_str(), temp.c_str(), GetVarValueInterpretedAsFloat(temp2, esss, io), flag);
				}
				else if (!strcmp(temp, "SETONEHANDED"))
				{

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_ONE_HANDED ...OBSOLETE...");

#endif
				}
				else if (!strcmp(temp, "SETWEAPON"))
				{
					pos = GetNextWord(es, pos, temp);
					io->GameFlags &= ~GFLAG_HIDEWEAPON;

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'H'))	// Hide Weapon
						{
							io->GameFlags |= GFLAG_HIDEWEAPON;
						}

						pos = GetNextWord(es, pos, temp);
					}

					if ((io) && (io->ioflags & IO_NPC))
					{
						// temporarily removed for Alpha
						strcpy(io->_npcdata->weaponname, temp.c_str());
						Prepare_SetWeapon(io, temp);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_WEAPON %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETLIFE"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io != NULL)
					{
						if (io->ioflags & IO_NPC)
						{
							io->_npcdata->maxlife = io->_npcdata->life = GetVarValueInterpretedAsFloat(temp, esss, io);
						}
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_LIFE %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETDURABILITY"))
				{
					long current = 0;
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'C')) current = 1;

						pos = GetNextWord(es, pos, temp);
					}

					if (io != NULL)
					{
						if (!(io->ioflags & IO_NPC))
						{
							if (current) io->durability = GetVarValueInterpretedAsFloat(temp, esss, io);
							else io->max_durability = io->durability = GetVarValueInterpretedAsFloat(temp, esss, io);
						}
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_LIFE %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETPATH"))
				{
					long wormspecific = 0;
					long followdir = 0;
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'W'))
						{
							wormspecific = 1;
						}

						if (iCharIn(temp, 'F'))
						{
							followdir = 1;
						}

						pos = GetNextWord(es, pos, temp);
					}

					if (io != NULL)
					{

						if (!strcasecmp(temp, "NONE"))
						{
							if (io->usepath != NULL)
							{
								free(io->usepath);
								io->usepath = NULL;
							}
						}
						else
						{
							ARX_PATH * ap = ARX_PATH_GetAddressByName(temp.c_str());

							if ((ap != NULL) && (ap != io->usepath))
							{
								if (io->usepath != NULL)
								{
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

								aup->initpos.x = io->initpos.x;
								aup->initpos.y = io->initpos.y;
								aup->initpos.z = io->initpos.z;
								aup->lastWP = -1;
								aup->path = ap;
								io->usepath = (void *)aup;
							}
						}
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_PATH %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETTARGET"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "SET_TARGET ");
						strcat(cmd, temp);
					}

#endif

					if ((io) && (io->ioflags & IO_NPC))
					{
						io->_npcdata->pathfind.flags &= ~PATHFIND_ALWAYS;
						io->_npcdata->pathfind.flags &= ~PATHFIND_ONCE;
						io->_npcdata->pathfind.flags &= ~PATHFIND_NO_UPDATE;
					}

					if (temp[0] == '-')
					{
						if ((io) && (io->ioflags & IO_NPC))
						{
							if (iCharIn(temp, 'S'))
								io->_npcdata->pathfind.flags |= PATHFIND_ONCE;

							if (iCharIn(temp, 'A'))
								io->_npcdata->pathfind.flags |= PATHFIND_ALWAYS;

							if (iCharIn(temp, 'N'))
								io->_npcdata->pathfind.flags |= PATHFIND_NO_UPDATE;
						}

						pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp);
						}

#endif
					}

					std::string temp1 = GetVarValueInterpretedAsText(temp, esss, io);

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

						if (!strcasecmp(temp, "OBJECT"))
						{
							pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp);
							}

#endif
							temp1 = GetVarValueInterpretedAsText(temp, esss, io);
						}

						long t = GetTargetByNameTarget(temp1);

						if (t == -2) t = GetInterNum(io);

						if (io->ioflags & IO_CAMERA)
						{
							EERIE_CAMERA * cam = (EERIE_CAMERA *)io->_camdata;
							cam->translatetarget.x = 0.f;
							cam->translatetarget.y = 0.f;
							cam->translatetarget.z = 0.f;
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
				else if (!strcmp(temp, "STARTTIMER"))
				{
					pos = GetNextWord(es, pos, temp);
					long t = -1;

					if (!strcmp(temp, "TIMER1")) t = 0;

					if (!strcmp(temp, "TIMER2")) t = 1;

					if (!strcmp(temp, "TIMER3")) t = 2;

					if (!strcmp(temp, "TIMER4")) t = 3;

					if (t > -1)
					{
						esss->timers[t] = ARXTimeUL();

						if (esss->timers[t] == 0) esss->timers[t] = 1;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "START_TIMER %s", temp);

#endif
				}
				else if (!strcmp(temp, "STOPTIMER"))
				{
					pos = GetNextWord(es, pos, temp);
					long t = -1;

					if (!strcmp(temp, "TIMER1")) t = 0;

					if (!strcmp(temp, "TIMER2")) t = 1;

					if (!strcmp(temp, "TIMER3")) t = 2;

					if (!strcmp(temp, "TIMER4")) t = 3;

					if (t > -1)
					{
						esss->timers[t] = 0;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "STOP_TIMER %s", temp);

#endif
				}
				else if (!strcmp(temp, "SENDEVENT"))
				{
					std::string evt;
					std::string temp1;
					std::string temp2;
					std::string temp3;
					char zonename[128];
					pos = GetNextWord(es, pos, temp1);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "SEND_EVENT ");
						strcat(cmd, temp);
					}

#endif
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
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp1);
						}

#endif

						if (group)
						{
							std::string temp6 = GetVarValueInterpretedAsText(temp1, esss, io);
							strcpy(groupname, temp6.c_str());
							pos = GetNextWord(es, pos, temp1);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp1);
							}

#endif
						}
					}

					float rad = 0;

					if ((group) && (!zone) && (!radius))
					{
					}
					else
					{
						pos = GetNextWord_Interpreted(io, es, pos, temp2);

#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp2);
						}

#endif

						if (zone) strcpy(zonename, temp2.c_str());

						if (radius) rad = GetVarValueInterpretedAsFloat(temp2, esss, io);
					}

					pos = GetNextWord(es, pos, temp3);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcat(cmd, " ");
						strcat(cmd, temp3);
					}

#endif

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
						EERIE_3D _pos, _pos2;

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

									if (EEDistance3D(&_pos, &_pos2) <= rad)
									{
										io->stat_sent++;
										Stack_SendIOScriptEvent(inter.iobj[l], 0, temp3, evt);
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
							EERIE_3D _pos;

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

										if (ARX_PATH_IsPosInZone(ap, _pos.x, _pos.y, _pos.z))
										{
											io->stat_sent++;
											Stack_SendIOScriptEvent(inter.iobj[l], 0, temp3, evt);
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
							if ((inter.iobj[l] != NULL)
									&& (inter.iobj[l] != io)
									&& (IsIOGroup(inter.iobj[l], groupname))
							   )
							{
								io->stat_sent++;
								Stack_SendIOScriptEvent(inter.iobj[l], 0, temp3, evt);
							}
						}
					}
					else // SINGLE OBJECT EVENT
					{
						long t = GetTargetByNameTarget(temp2);

						if (t == -2) t = GetInterNum(io);



						if (ValidIONum(t))
						{
							io->stat_sent++;
							Stack_SendIOScriptEvent(inter.iobj[t], 0, temp3, evt);
						}
					}

					EVENT_SENDER = oes;
				}
				else if (!strcmp(temp, "SET"))
				{
					std::string temp2;
					char tempp[256];
					long ival;
					float fval;
					SCRIPT_VAR * sv = NULL;
					long a = 0;
					pos = GetNextWord(es, pos, temp, 1);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "SET ");
						strcat(cmd, temp);
					}

#endif

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'A')) a = 1;

						pos = GetNextWord(es, pos, temp, 1);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp);
						}

#endif
					}

					pos = GetNextWord(es, pos, temp2);

					switch (temp[0])
					{
						case '$': // GLOBAL TEXT
							strcpy(tempp, GetVarValueInterpretedAsText(temp2, esss, io).c_str());

							if (a) RemoveNumerics(tempp);

							sv = SETVarValueText(svar, NB_GLOBALS, temp, tempp);

							if (sv == NULL)
							{
								ShowScriptError("Unable to Set Variable Value", cmd);
							}
							else sv->type = TYPE_G_TEXT;

							break;
						case '\xA3': // LOCAL TEXT
							strcpy(tempp, GetVarValueInterpretedAsText(temp2, esss, io).c_str());

							if (a) RemoveNumerics(tempp);

							sv = SETVarValueText(esss->lvar, esss->nblvar, temp, tempp);

							if (sv == NULL)
							{
								ShowScriptError("Unable to Set Variable Value", cmd);
							}
							else sv->type = TYPE_L_TEXT;

							break;
						case '#': // GLOBAL LONG
							ival = (long)GetVarValueInterpretedAsFloat(temp2, esss, io);
							sv = SETVarValueLong(svar, NB_GLOBALS, temp, ival);

							if (sv == NULL)
							{
								ShowScriptError("Unable to Set Variable Value", cmd);
							}
							else sv->type = TYPE_G_LONG;

							break;
						case '\xA7': // LOCAL LONG
							ival = (long)GetVarValueInterpretedAsFloat(temp2, esss, io);
							sv = SETVarValueLong(esss->lvar, esss->nblvar, temp, ival);

							if (sv == NULL)
							{
								ShowScriptError("Unable to Set Variable Value", cmd);
							}
							else sv->type = TYPE_L_LONG;

							break;
						case '&': // GLOBAL float
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							sv = SETVarValueFloat(svar, NB_GLOBALS, temp, fval);

							if (sv == NULL)
							{
								ShowScriptError("Unable to Set Variable Value", cmd);
							}
							else sv->type = TYPE_G_FLOAT;

							break;
						case '@': // LOCAL float
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							sv = SETVarValueFloat(esss->lvar, esss->nblvar, temp, fval);

							if (sv == NULL)
							{
								ShowScriptError("Unable to Set Variable Value", cmd);
							}
							else sv->type = TYPE_L_FLOAT;

							break;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET %s %s", temp.c_str(), temp2);

#endif
				}
				else if (!strcmp(temp, "SAY"))
				{
					//DO NOTHING
				}
				else if (!strcmp(temp, "SETANGULAR"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!strcasecmp(temp, "ON"))	io->ioflags |= IO_ANGULAR;
					else	io->ioflags &= ~IO_ANGULAR;
				}
				else if (!strcmp(temp, "SETPLAYERCOLLISION"))
				{
					pos = GetNextWord(es, pos, temp);
					MakeUpcase(temp);

					if (!strcmp(temp, "ON"))	io->collision |= 1;
					else	io->collision &= ~1;

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_PLAYER_COLLISION %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETSTEPMATERIAL"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io)
					{
						if (io->stepmaterial)
						{
							free((void *)io->stepmaterial);
							io->stepmaterial = NULL;
						}

						io->stepmaterial = (char *)malloc(temp.length() + 1);
						strcpy(io->stepmaterial, temp.c_str());
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_STEP_MATERIAL %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETARMORMATERIAL"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io)
					{
						if (io->armormaterial)
						{
							free((void *)io->armormaterial);
							io->armormaterial = NULL;
						}

						io->armormaterial = (char *)malloc(temp.length() + 1);
						strcpy(io->armormaterial, temp.c_str());
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_ARMOR_MATERIAL %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETWEAPONMATERIAL"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io)
					{
						if (io->weaponmaterial)
							free(io->weaponmaterial);

						io->weaponmaterial = NULL;
						io->weaponmaterial = strdup(temp.c_str());
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_STEP_MATERIAL %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETSTRIKESPEECH"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io)
					{
						if (io->strikespeech) free(io->strikespeech);

						io->strikespeech = NULL;
						io->strikespeech = strdup(temp.c_str());
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_STEP_MATERIAL %s", temp.c_str());

#endif
				}
				else if (!strcmp(temp, "SETPLAYERCONTROLS"))
				{
					INTERACTIVE_OBJ * oes = EVENT_SENDER;
					EVENT_SENDER = io;
					pos = GetNextWord(es, pos, temp);
					MakeUpcase(temp);

					if (!strcmp(temp, "ON"))
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
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_PLAYER_CONTROLS %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETWORLDCOLLISION"))
				{
					pos = GetNextWord(es, pos, temp);
					MakeUpcase(temp);

					if (!strcmp(temp, "ON"))	io->collision |= 2;
					else io->collision &= ~2;

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_WORLD_COLLISION %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETSHADOW"))
				{
					pos = GetNextWord(es, pos, temp);
					MakeUpcase(temp);

					if (!strcmp(temp, "ON"))	io->ioflags &= ~IO_NOSHADOW;
					else io->ioflags |= IO_NOSHADOW;

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_SHADOW %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETDETACHABLE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETSTACKABLE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETSHOP"))
				{
					pos = GetNextWord(es, pos, temp);
					MakeUpcase(temp);

					if ((!strcmp(temp, "ON")) || (!strcmp(temp, "YES")))
						io->ioflags |= IO_SHOP;
					else	io->ioflags &= ~IO_SHOP;

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_SHOP %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETMAXCOUNT"))
				{
					pos = GetNextWord(es, pos, temp);

					if ((io != NULL) && (io->ioflags & IO_ITEM))
					{
						io->_itemdata->maxcount = (short)GetVarValueInterpretedAsFloat(temp, esss, io);

						if (io->_itemdata->maxcount < 1) io->_itemdata->maxcount = 1;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_MAX_COUNT %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETCOUNT"))
				{
					pos = GetNextWord(es, pos, temp);
					float c = (float)atof(temp.c_str());

					if (c < 1.f) c = 1.f;

					if ((io != NULL) && (io->ioflags & IO_ITEM))
					{
						io->_itemdata->count = (short)GetVarValueInterpretedAsFloat(temp, esss, io);

						if (io->_itemdata->count < 1) io->_itemdata->count = 1;

						if (io->_itemdata->count > io->_itemdata->maxcount) io->_itemdata->count = io->_itemdata->maxcount;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_COUNT %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETWEIGHT"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io != NULL)
					{
						io->weight = GetVarValueInterpretedAsFloat(temp, esss, io);

						if (io->weight < 0.f) io->weight = 0.f;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_WEIGHT %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETTRANSPARENCY"))
				{
					pos = GetNextWord(es, pos, temp);

					io->invisibility = 1.f + GetVarValueInterpretedAsFloat(temp, esss, io) * ( 1.0f / 100 );

					if (io->invisibility == 1.f) io->invisibility = 0;
				}
				else if (!strcmp(temp, "SETEVENT"))
				{
					std::string temp2;
					long t;
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp2);
					MakeUpcase(temp);
					MakeUpcase(temp2);

					if ((!strcmp(temp2, "ON")) || (!strcmp(temp2, "YES"))) t = 1;
					else t = 0;

					if (!strcmp(temp, "COLLIDE_NPC"))
					{
						if (t) esss->allowevents &= ~DISABLE_COLLIDE_NPC;
						else esss->allowevents |= DISABLE_COLLIDE_NPC;
					}

					if (!strcmp(temp, "CHAT"))
					{
						if (t) esss->allowevents &= ~DISABLE_CHAT;
						else esss->allowevents |= DISABLE_CHAT;
					}

					if (!strcmp(temp, "HIT"))
					{
						if (t) esss->allowevents &= ~DISABLE_HIT;
						else esss->allowevents |= DISABLE_HIT;
					}

					if (!strcmp(temp, "INVENTORY2_OPEN"))
					{
						if (t) esss->allowevents &= ~DISABLE_INVENTORY2_OPEN ;
						else esss->allowevents |= DISABLE_INVENTORY2_OPEN ;
					}

					if (!strcmp(temp, "DETECTPLAYER"))
					{
						if (t) esss->allowevents &= ~DISABLE_DETECT ;
						else esss->allowevents |= DISABLE_DETECT;
					}

					if (!strcmp(temp, "HEAR"))
					{
						if (t) esss->allowevents &= ~DISABLE_HEAR ;
						else esss->allowevents |= DISABLE_HEAR ;
					}

					if (!strcmp(temp, "AGGRESSION"))
					{
						if (t) esss->allowevents &= ~DISABLE_AGGRESSION ;
						else esss->allowevents |= DISABLE_AGGRESSION ;
					}

					if (!strcmp(temp, "MAIN"))
					{
						if (t) esss->allowevents &= ~DISABLE_MAIN ;
						else esss->allowevents |= DISABLE_MAIN ;
					}

					if (!strcmp(temp, "CURSORMODE"))
					{
						if (t) esss->allowevents &= ~DISABLE_CURSORMODE ;
						else esss->allowevents |= DISABLE_CURSORMODE ;
					}

					if (!strcmp(temp, "EXPLORATIONMODE"))
					{
						if (t) esss->allowevents &= ~DISABLE_EXPLORATIONMODE ;
						else esss->allowevents |= DISABLE_EXPLORATIONMODE ;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_EVENT %s %s", temp, temp2);

#endif
				}
				else if (!strcmp(temp, "SETPRICE"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io)
					{
						if (io->ioflags & IO_ITEM)
						{
							io->_itemdata->price = (long)GetVarValueInterpretedAsFloat(temp, esss, io);

							if (io->_itemdata->price < 0) io->_itemdata->price = 0;
						}
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_PRICE %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETINTERNALNAME"))
				{
					pos = GetNextWord(es, pos, temp);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "ERROR: SETINTERNALNAME %s - NOT AN IO !!!", temp);

#endif
				}
				else if (!strcmp(temp, "SHOWGLOBALS"))
				{
					ShowText = "";
					MakeGlobalText(ShowText);
					ShowTextWindowtext = "Global Variables";

					if (!(danaeApp.kbd.inkey[INKEY_LEFTSHIFT]) && !(danaeApp.kbd.inkey[INKEY_RIGHTSHIFT]))
						DialogBox(hInstance, (LPCTSTR)IDD_SHOWTEXT, NULL, (DLGPROC)ShowTextDlg);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SHOWGLOBALS");

#endif
				}
				else if (!strcmp(temp, "SHOWLOCALS"))
				{
					ShowText = "";
					MakeLocalText(es, ShowText);
					ShowTextWindowtext = "Local Variables";

					if (!(danaeApp.kbd.inkey[INKEY_LEFTSHIFT]) && !(danaeApp.kbd.inkey[INKEY_RIGHTSHIFT]))
						DialogBox(hInstance, (LPCTSTR)IDD_SHOWTEXT, NULL, (DLGPROC)ShowTextDlg);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SHOWLOCALS");

#endif
				}
				else if (!strcmp(temp, "SHOWVARS"))
				{
					ShowText = "";
					ShowText2 = "";
					MakeGlobalText(ShowText);
					MakeLocalText(es, ShowText2);
					ShowTextWindowtext = "Variables";

					if (!(danaeApp.kbd.inkey[INKEY_LEFTSHIFT]) && !(danaeApp.kbd.inkey[INKEY_RIGHTSHIFT]))
						DialogBox(hInstance, (LPCTSTR)IDD_SHOWVARS, NULL, (DLGPROC)ShowVarsDlg);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SHOWVARS");

#endif
				}
				else if (!strcmp(temp, "SETIRCOLOR"))
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

						if (NEED_DEBUG) sprintf(cmd, "%s %s %s %s", temp, temp1, temp2, temp3);

#endif
					}
				}
				else if (!strcmp(temp, "SETSCALE"))
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
				else if (!strcmp(temp, "STEALNPC"))
				{
					if (player.Interface & INTER_STEAL)
					{
						SendIOScriptEvent(ioSteal, SM_STEAL, "OFF");
					}

					player.Interface |= INTER_STEAL;
					InventoryDir = 1;
					ioSteal = io;
				}
				else if (!strcmp(temp, "SPECIALFX"))
				{
					std::string temp1;
					pos = GetNextWord(es, pos, temp1);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "SPECIAL_FX ");
						strcat(cmd, temp);
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
				else if (!strcmp(temp, "SETBUMP"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!strcasecmp(temp, "ON"))
					{
						io->ioflags |= IO_BUMP;
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG) sprintf(cmd, "SETBUMP ON");

#endif
					}
					else if (!strcasecmp(temp, "OFF"))
					{
						io->ioflags &= ~IO_BUMP;
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG) sprintf(cmd, "SETBUMP OFF");

#endif
					}
				}
				else if (!strcmp(temp, "SETZMAP"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!strcasecmp(temp, "ON"))
					{
						io->ioflags |= IO_ZMAP;
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG) sprintf(cmd, "SETMAP ON");

#endif
					}
					else if (!strcasecmp(temp, "OFF"))
					{
						io->ioflags &= ~IO_ZMAP;
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG) sprintf(cmd, "SETMAP OFF");

#endif
					}
				}

				break;
			case 'Z':

				if (!strcmp(temp, "ZONEPARAM"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "ZONE_PARAM ");
						strcat(cmd, temp);
					}

#endif

					if (!strcasecmp(temp, "STACK"))
					{
						ARX_GLOBALMODS_Stack();
					}
					else if (!strcasecmp(temp, "UNSTACK"))
					{
						ARX_GLOBALMODS_UnStack();
					}
					else
					{
						if (temp[0] == '-')
						{
							pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp);
							}

#endif
						}

						if (!strcasecmp(temp, "RGB"))
						{
							pos = GetNextWord(es, pos, temp);
							desired.depthcolor.r = GetVarValueInterpretedAsFloat(temp, esss, io);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp);
							}

#endif
							pos = GetNextWord(es, pos, temp);
							desired.depthcolor.g = GetVarValueInterpretedAsFloat(temp, esss, io);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp);
							}

#endif
							pos = GetNextWord(es, pos, temp);
							desired.depthcolor.b = GetVarValueInterpretedAsFloat(temp, esss, io);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp);
							}

#endif
							desired.flags |= GMOD_DCOLOR;
						}
						else if (!strcasecmp(temp, "ZCLIP"))
						{
							pos = GetNextWord(es, pos, temp);
							desired.zclip = GetVarValueInterpretedAsFloat(temp, esss, io);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp);
							}

#endif
							desired.flags |= GMOD_ZCLIP;
						}
						else if (!strcasecmp(temp, "AMBIANCE"))
						{
							pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp);
							}

#endif
							ARX_SOUND_PlayZoneAmbiance(temp.c_str());
						}

					}
				}

				break;
			case 'K':

				if (!strcmp(temp, "KILLME"))
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
				else if (!strcmp(temp, "KEYRINGADD"))
				{
					std::string temp2;
					pos = GetNextWord(es, pos, temp2);
					temp = GetVarValueInterpretedAsText(temp2, esss, io);
					ARX_KEYRING_Add(temp.c_str());
				}

				break;
			case 'F':

				if (!strcmp(temp, "FORCEANIM"))
				{
					std::string temp2;
					long num;
					pos = GetNextWord(es, pos, temp2);
					num = GetNumAnim(temp2.c_str());

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
				else if (!strcmp(temp, "FORCEANGLE"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io != NULL)
					{

						io->angle.b = MAKEANGLE(GetVarValueInterpretedAsFloat(temp, esss, io));
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "FORCEANGLE %s", temp);

#endif
				}
				else if (!strcmp(temp, "FORCEDEATH"))
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

				if (!strcmp(temp, "PLAYERLOOKAT"))
				{
					pos = GetNextWord(es, pos, temp);
					long t = GetTargetByNameTarget(temp);

					if (t == -2) t = GetInterNum(io);

					if (ValidIONum(t))
					{
						ForcePlayerLookAtIO(inter.iobj[t]);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "PLAYERLOOKAT %s", temp);

#endif
				}
				else if (!strcmp(temp, "PLAYERSTACKSIZE"))
				{
					pos = GetNextWord(es, pos, temp);

					if ((io) && (io->ioflags & IO_ITEM))
					{
						io->_itemdata->playerstacksize = (short)GetVarValueInterpretedAsFloat(temp, esss, io);

						if (io->_itemdata->playerstacksize < 1)
							io->_itemdata->playerstacksize = 1;

						if (io->_itemdata->playerstacksize > 100)
							io->_itemdata->playerstacksize = 100;
					}
				}
				else if (!strcmp(temp, "PRECAST"))
				{
					std::string temp2;
					long duration = -1;
					long flags = 0;
					long dur = 0;
					pos = GetNextWord(es, pos, temp); // switch or level

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'D'))
						{
							flags |= SPELLCAST_FLAG_NOCHECKCANCAST;
							pos = GetNextWord(es, pos, temp2); // duration
							duration = GetVarValueInterpretedAsFloat(temp2, esss, io);
							dur = 1;
						}

						if (iCharIn(temp, 'F'))
						{
							flags |= SPELLCAST_FLAG_NOCHECKCANCAST;
							flags |= SPELLCAST_FLAG_NOMANA;
						}

						pos = GetNextWord(es, pos, temp); // level
					}

					long level = GetVarValueInterpretedAsFloat(temp, esss, io);

					if (level < 1) level = 1;
					else if (level > 10) level = 10;

					pos = GetNextWord(es, pos, temp); //spell id
					long spellid;
					spellid = GetSpellId(temp);

					if (spellid != -1)
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
				else if (!strcmp(temp, "POISON"))
				{
					pos = GetNextWord(es, pos, temp);
					float fval = GetVarValueInterpretedAsFloat(temp, esss, io);
					ARX_PLAYER_Poison(fval);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "POISON %s", temp);

#endif
				}
				else if (!strcmp(temp, "PLAYERMANADRAIN"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!strcasecmp(temp, "ON"))
						player.playerflags &= ~PLAYERFLAGS_NO_MANA_DRAIN;
					else
						player.playerflags |= PLAYERFLAGS_NO_MANA_DRAIN;
				}
				else if (!strcmp(temp, "PATHFIND"))
				{
					std::string temp2;
					pos = GetNextWord(es, pos, temp2);
					long t = GetTargetByNameTarget(temp2);
					ARX_NPC_LaunchPathfind(io, t);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "PATHFIND %s", temp2);

#endif
				}
				else if (!strcmp(temp, "PLAYANIM"))
				{
					INTERACTIVE_OBJ * iot = io;
					std::string temp2;
					long num;
					long nu = 0;
					long loop = 0;
					long execute = 0;
					long nointerpol = 0;
					pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "PLAY_ANIM ");
						strcat(cmd, temp);
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

						if (iCharIn(temp2, 'P'))
						{
							iot = inter.iobj[0];
							iot->move.x = iot->lastmove.x = 0.f;
							iot->move.y = iot->lastmove.y = 0.f;
							iot->move.z = iot->lastmove.z = 0.f;

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
						num = GetNumAnim(temp2);

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
											char timername[64];
											char timername2[64];
											ARX_SCRIPT_Timer_GetDefaultName(timername2);
											sprintf(timername, "ANIM_%s", timername2);
											long num2 = ARX_SCRIPT_Timer_GetFree();

											if (num2 > -1)
											{
												memset(&scr_timer[num2], 0, sizeof(SCR_TIMER));
												ActiveTimers++;
												scr_timer[num2].es = es;
												scr_timer[num2].exist = 1;
												scr_timer[num2].io = io;
												scr_timer[num2].msecs = max(iot->anims[num]->anims[iot->animlayer[nu].altidx_cur]->anim_time, 1000.0f);
												scr_timer[num2].namelength = strlen(timername) + 1;
												scr_timer[num2].name = (char *)malloc(scr_timer[num2].namelength);
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
				else if (!strcmp(temp, "PLAYERINTERFACE"))
				{
					std::string temp2;
					pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "PLAYER_INTERFACE ");
						strcat(cmd, temp);
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
				else if (!strcmp(temp, "PLAY"))
				{
					unsigned long loop(ARX_SOUND_PLAY_ONCE);
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
						strcat(cmd, temp);
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
				else if (!strcmp(temp, "PLAYSPEECH"))
				{
					std::string temp2;

					pos = GetNextWord(es, pos, temp2);

					long num = ARX_SOUND_PlaySpeech(temp2.c_str(), io && io->show == 1 ? io : NULL);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						if (num == ARX_SOUND_INVALID_RESOURCE)
							sprintf(cmd, "PLAYSPEECH %s - UNABLE TO LOAD FILE", temp2);
						else
							sprintf(cmd, "PLAYSPEECH %s - Success DanaePlaySample", temp2);
					}

#endif
				}
				else if (!strcmp(temp, "POPUP"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!(danaeApp.kbd.inkey[INKEY_LEFTSHIFT]) && !(danaeApp.kbd.inkey[INKEY_RIGHTSHIFT]))
					{
						ARX_TIME_Pause();
						LogError << (temp.c_str());
						ARX_TIME_UnPause();
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "POPUP %s", temp);

#endif
				}
				else if (!strcmp(temp, "PHYSICAL"))
				{
					std::string temp2;
					pos = GetNextWord(es, pos, temp);

					if (!strcasecmp(temp, "ON"))
					{
						io->ioflags &= ~IO_PHYSICAL_OFF;
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG) sprintf(cmd, "PHYSICAL ON");

#endif
					}
					else if (!strcasecmp(temp, "OFF"))
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

						if (!strcasecmp(temp, "HEIGHT"))
						{
							if (io)
							{
								io->original_height = -fval;

								if (io->original_height > -30) io->original_height = -30;

								if (io->original_height < -165) io->original_height = -165;

								io->physics.cyl.height = io->original_height * io->scale;
							}
						}
						else if (!strcasecmp(temp, "RADIUS"))
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

						if (NEED_DEBUG) sprintf(cmd, "PHYSICAL %s %s", temp, temp2);

#endif
					}
				}

				break;
			case 'L':

				if (!strcmp(temp, "LOADANIM"))
				{
					INTERACTIVE_OBJ * iot = io;
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'P'))
							iot = inter.iobj[0];

						pos = GetNextWord(es, pos, temp);
					}

					std::string temp2;
					long flag;

					pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "LOADANIM %s %s", temp, temp2);

#endif

					if (iot != NULL)
					{
						long num = -1;
						MakeUpcase(temp);
						num = GetNumAnim(temp);

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

								if ((iot == inter.iobj[0]) || (iot->ioflags & IO_NPC))
								{
									flag = TEA_NPC_SAMPLES;
									tex3 = "Graph\\Obj3D\\Anims\\npc\\" + temp2;
								}
								else
								{
									flag = TEA_FIX_SAMPLES;
									tex3 = "Graph\\Obj3D\\Anims\\Fix_Inter\\" + temp2;
								}

								SetExt(tex3, ".tea");
								File_Standardize(tex3, tex2);

								if (PAK_FileExist(tex2.c_str()))
								{
									iot->anims[num] = EERIE_ANIMMANAGER_Load(tex2.c_str());

									if (iot->anims[num] == NULL)
									{
										char ttmp[512];
										sprintf(ttmp, "LOADANIM %s %s FAILED", temp.c_str(), temp2.c_str());
										ForceSendConsole(ttmp, 1, 0, (HWND)1);
									}
								}
							}
							else
							{
								char ttmp[512];
								sprintf(ttmp, "LOADANIM %s %s FAILED", temp.c_str(), temp2.c_str());
								ForceSendConsole(ttmp, 1, 0, (HWND)1);
							}
						}
					}
				}
				else if (!strcmp(temp, "LINKOBJTOME"))
				{
					pos = GetNextWord_Interpreted(io, es, pos, temp);
					long t = GetTargetByNameTarget(temp);
					pos = GetNextWord(es, pos, temp);

					if (ValidIONum(t))
						LinkObjToMe(io, inter.iobj[t], temp.c_str());

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "LINKOBJTOME %ld %s", t, temp);

#endif
				}

				break;
			case 'I':

				if ((temp[1] == 'F') && (temp[2] == 0))
				{
					const unsigned int tvSize = 256 ;
					std::string temp3;
					short oper = 0;
					short failed = 0;
					short typ1, typ2;
					std::string tvar1;
					std::string tvar2;
					float fvar1, fvar2;
					std::string tempo;

					fvar1 = fvar2 = 0;
					pos = GetNextWord(es, pos, temp);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "IF (");
						strcat(cmd, temp);
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

					switch (temp[0])
					{
						case '^':
						{

							long lv; float fv; std::string tv;	//Arx: xrichter (2010-08-04) - fix a crash when $OBJONTOP return to many object name inside tv
							switch ( GetSystemVar( esss, io, temp, tv,tvSize, &fv, &lv ) )
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
							fvar1	=	GETVarValueLong(svar, NB_GLOBALS, temp);
							break;
						case '\xA7':
							typ1	=	TYPE_FLOAT;
							fvar1	=	GETVarValueLong(esss->lvar, esss->nblvar, temp);
							break;
						case '&':
							typ1	=	TYPE_FLOAT;
							fvar1	=	GETVarValueFloat(svar, NB_GLOBALS, temp);
							break;
						case '@':
							typ1	=	TYPE_FLOAT;
							fvar1	=	GETVarValueFloat(esss->lvar, esss->nblvar, temp);
							break;
						case '$':
							typ1	=	TYPE_TEXT;
							tempo	=	GETVarValueText(svar, NB_GLOBALS, temp);

							if ( tempo.empty()) tvar1[0] = 0;
							else tvar1 = tempo;

							break;
						case '\xA3':
							typ1	=	TYPE_TEXT;
							tempo	=	GETVarValueText(esss->lvar, esss->nblvar, temp);

							if ( tempo.empty() ) tvar1[0] = 0;
							else tvar1 = tempo;

							break;
						default:

							if ((oper == OPER_ISTYPE) || (oper == OPER_ISGROUP) || (oper == OPER_NOTISGROUP))
							{
								typ1 =	TYPE_TEXT;
								tvar1 = temp;
							}
							else
							{
								typ1 =	TYPE_FLOAT;
								fvar1 =	(float)atof(temp.c_str());
							}
					}

					switch ( temp3[0] )
					{
						case '^':
						{

							long lv; float fv; std::string tv;
							switch ( GetSystemVar( esss, io, temp3, tv,tvSize, &fv, &lv ) )
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

							if (tempo.empty()) tvar2[0] = 0;
							else tvar2 = tempo;

							break;
						case '\xA3':
							typ2			=	TYPE_TEXT;
							tempo			=	GETVarValueText(esss->lvar, esss->nblvar, temp3);

							if (tempo.empty()) tvar2[0] = 0;
							else tvar2 = tempo;

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

									long flagg	=	ARX_EQUIPMENT_GetObjectTypeFlag(tvar2.c_str());

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
				else if (!strcmp(temp, "INC"))
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

					if (NEED_DEBUG) sprintf(cmd, "%s %s %s", temp, temp1, temp2);

#endif
				}
				else if (!strcmp(temp, "IFEXISTINTERNAL"))
				{
					long failed = 1;
					pos = GetNextWord(es, pos, temp);
					long t = GetTargetByNameTarget(temp);

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
						if (failed) sprintf(cmd, "IFEXISTINTERNAL (%s) -> false", temp);
						else sprintf(cmd, "IFEXISTINTERNAL (%s) -> true", temp);
					}

#endif
				}
				else if (!strcmp(temp, "INVULNERABILITY"))
				{
					pos = GetNextWord(es, pos, temp);
					long player = 0;

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'P'))
							player = 1;

						pos = GetNextWord(es, pos, temp);
					}

					if (!strcasecmp(temp, "ON"))
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
				else if (!strcmp(temp, "INVERTEDOBJECT"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!strcasecmp(temp, "ON"))
					{
						io->ioflags |= IO_INVERTED;
					}
					else io->ioflags &= ~IO_INVERTED;
				}
				else if (!strcmp(temp, "IFVISIBLE"))
				{
					long failed = 1;
					pos = GetNextWord(es, pos, temp);
					long t = GetTargetByNameTarget(temp);

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
						if (failed) sprintf(cmd, "IFVISIBLE (%s) -> false", temp);
						else sprintf(cmd, "IFVISIBLE (%s) -> true", temp);
					}

#endif
				}
				else if (!strcmp(temp, "INVENTORY"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "INVENTORY ");
						strcat(cmd, temp);
					}

#endif
					MakeStandard(temp);
					long ion;
					ion = GetInterNum(io);

					if ((io != NULL) && (ion != -1))
						if (!strcasecmp(temp, "CREATE"))
						{
							if (inter.iobj[ion]->inventory != NULL)
							{
								INVENTORY_DATA * id = (INVENTORY_DATA *)inter.iobj[ion]->inventory;

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
								inter.iobj[ion]->inventory = malloc(sizeof(INVENTORY_DATA));
								memset(inter.iobj[ion]->inventory, 0, sizeof(INVENTORY_DATA));
								INVENTORY_DATA * id = (INVENTORY_DATA *)inter.iobj[ion]->inventory;
								id->sizex = 3;
								id->sizey = 11;
								id->io = inter.iobj[ion];
							}
						}
						else if (!strcmp(temp, "SKIN"))
						{
							std::string temp2;
							pos = GetNextWord(es, pos, temp2);

							if (io)
							{
								if (io->inventory_skin) free(io->inventory_skin);

								io->inventory_skin = strdup(temp2.c_str());
							}
						}
						else if (!strcmp(temp, "PLAYERADDFROMSCENE"))
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

							if (!RELOADING)
							{
								long t = GetTargetByNameTarget(temp2);

								if (t == -2) t = GetInterNum(io);

								if (ValidIONum(t))
								{
									RemoveFromAllInventories(inter.iobj[t]);
									inter.iobj[t]->show = SHOW_FLAG_IN_INVENTORY;

									if (!CanBePutInInventory(inter.iobj[t]))
									{
										PutInFrontOfPlayer(inter.iobj[t], 1);
									}
								}
							}
						}
						else if ((!strcmp(temp, "PLAYERADD")) || (!strcmp(temp, "PLAYERADDMULTI")))
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

								if (RELOADING)
								{
									if (!strcmp(temp, "PLAYERADDMULTI"))
									{
										pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

										if (NEED_DEBUG)
										{
											strcat(cmd, " ");
											strcat(cmd, temp2);
										}

#endif
									}
								}
								else
								{
									std::string tex2;
									tex2 = "Graph\\Obj3D\\Interactive\\Items\\" + temp2 + ".teo";
									File_Standardize(tex2, tex);

									if (FORBID_SCRIPT_IO_CREATION == 0)
									{
										INTERACTIVE_OBJ * ioo = (INTERACTIVE_OBJ *)AddItem(GDevice, tex.c_str(), IO_IMMEDIATELOAD);

										if (ioo != NULL)
										{
											LASTSPAWNED = ioo;
											ioo->scriptload = 1;
											MakeTemporaryIOIdent(ioo);
											SendInitScriptEvent(ioo);

											if (!strcmp(temp, "PLAYERADDMULTI"))
											{
												pos = GetNextWord(es, pos, temp2);

												if (ioo->ioflags & IO_GOLD)
												{
													ioo->_itemdata->price = atoi(temp2.c_str());
												}
												else
												{
													ioo->_itemdata->maxcount = 9999;

													int iTemp = atoi(temp2.c_str());
													ARX_CHECK_SHORT(iTemp);

													ioo->_itemdata->count = ARX_CLEAN_WARN_CAST_SHORT(iTemp);


													if (ioo->_itemdata->count < 1) ioo->_itemdata->count = 1;
												}
											}

											ioo->show = SHOW_FLAG_IN_INVENTORY;

											if (!CanBePutInInventory(ioo))
											{
												PutInFrontOfPlayer(ioo, 1);
											}
										}
									}
									else
									{
										if (!strcmp(temp, "PLAYERADDMULTI"))
											pos = GetNextWord(es, pos, temp2);
									}
								}
							}
						}
						else if (!strcmp(temp, "ADDFROMSCENE"))
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

							if (!RELOADING)
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

									if (!CanBePutInSecondaryInventory((INVENTORY_DATA *)inter.iobj[ion]->inventory, inter.iobj[t], &xx, &yy))
									{
										PutInFrontOfPlayer(inter.iobj[t], 1);
									}
								}
							}

						}
						else if ((!strcmp(temp, "ADD")) || (!strcmp(temp, "ADDMULTI")))
						{
							std::string temp2;

							if (inter.iobj[ion]->inventory == NULL)
							{
								pos = GetNextWord(es, pos, temp2);

								if (!strcmp(temp, "ADDMULTI"))
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

								if (RELOADING)
								{
									if (!strcmp(temp, "ADDMULTI"))
									{
										pos = GetNextWord(es, pos, temp2);
									}
								}
								else
								{
									std::string tex2;
									tex2 = "Graph\\Obj3D\\Interactive\\Items\\" + temp2 + ".teo";
									File_Standardize(tex2, tex);

									if (FORBID_SCRIPT_IO_CREATION == 0)
									{
										long multi = -1;

										if (!strcmp(temp, "ADDMULTI"))
										{
											pos = GetNextWord(es, pos, temp2);
											multi = atoi(temp2.c_str());
										}

										INTERACTIVE_OBJ * ioo = (INTERACTIVE_OBJ *)AddItem(GDevice, tex.c_str(), IO_IMMEDIATELOAD);
										long xx, yy;

										if ((ioo != NULL)
												&&	(multi != 0))
										{
											LASTSPAWNED = ioo;
											ioo->scriptload = 1;
											MakeTemporaryIOIdent(ioo);
											SendInitScriptEvent(ioo);


											if (!strcmp(temp, "ADDMULTI"))
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

											if (!CanBePutInSecondaryInventory((INVENTORY_DATA *)inter.iobj[ion]->inventory, ioo, &xx, &yy))
											{
												PutInFrontOfPlayer(ioo, 1);
											}
										}
									}
									else
									{
										if (!strcmp(temp, "ADDMULTI"))
											pos = GetNextWord(es, pos, temp2);
									}
								}
							}
						}
						else if (!strcmp(temp, "DESTROY"))
						{
							if (inter.iobj[ion]->inventory != NULL)
							{
								if (SecondaryInventory == (INVENTORY_DATA *)inter.iobj[ion]->inventory)
									SecondaryInventory = NULL;

								free(inter.iobj[ion]->inventory);
								inter.iobj[ion]->inventory = NULL;
							}
						}
						else if (!strcmp(temp, "OPEN"))
						{
							if (SecondaryInventory != (INVENTORY_DATA *)inter.iobj[ion]->inventory)
							{
								SecondaryInventory = (INVENTORY_DATA *)inter.iobj[ion]->inventory;
								ARX_SOUND_PlayInterface(SND_BACKPACK);
							}
						}
						else if (!strcmp(temp, "CLOSE"))
						{
							if (inter.iobj[ion]->inventory != NULL)
							{
								SecondaryInventory = NULL;
								ARX_SOUND_PlayInterface(SND_BACKPACK);
							}
						}
				}

				break;
			case 'O':

				if (!strcmp(temp, "OBJECTHIDE"))
				{
					std::string temp1;
					long megahide = 0;
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'M'))
							megahide = 1;

						pos = GetNextWord(es, pos, temp);
					}

					long t = GetTargetByNameTarget(temp);

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

					if (NEED_DEBUG) sprintf(cmd, "OBJECT_HIDE %s %s", temp, temp1);

#endif
				}

				break;
			case 'H':
			{
				if (!strcmp(temp, "HEROSAY"))
				{
					char tempp[256];
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'D'))
						{
							// do not show (debug say)
							if (FINAL_RELEASE)
							{
								pos = GetNextWord(es, pos, temp);
								goto nodraw;
							}
						}

						pos = GetNextWord(es, pos, temp);
					}

					strcpy(tempp, GetVarValueInterpretedAsText(temp, esss, io).c_str());

					if (tempp[0] == '[')
					{
						ARX_SPEECH_AddLocalised(NULL, tempp);
					}
					else
					{
						_TCHAR UText[512];
						MultiByteToWideChar(CP_ACP, 0, tempp, -1, (wchar_t*)UText, 256);
						ARX_SPEECH_Add(NULL, UText);
					}

				nodraw:
					;
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "HEROSAY %s", temp);

#endif
				}
				else if (!strcmp(temp, "HALO"))
				{
					pos = GetNextWord(es, pos, temp);
					MakeUpcase(temp);

					if (iCharIn(temp, 'O'))
					{
						if (io) io->halo_native.flags |= HALO_ACTIVE;
					}

					if (iCharIn(temp, 'F'))
					{
						if (io) io->halo_native.flags &= ~HALO_ACTIVE;
					}

					if (iCharIn(temp, 'N'))
					{
						if (io) io->halo_native.flags |= HALO_NEGATIVE;
					}
					else if (io) io->halo_native.flags &= ~HALO_NEGATIVE;

					if (iCharIn(temp, 'L'))
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

					if (iCharIn(temp, 'C'))
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

					if (iCharIn(temp, 'S'))
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

					if (NEED_DEBUG) sprintf(cmd, "HALO %s", temp);

#endif
				}
			}
			break;
			case 'T':

				if (!strcmp(temp, "TELEPORT"))
				{
					std::string temp2;
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "TELEPORT ");
						strcat(cmd, temp);
					}

#endif

					if (!strcasecmp(temp, "behind"))
					{
						ARX_INTERACTIVE_TeleportBehindTarget(io);
					}
					else
					{
						TELEPORT_TO_CONFIRM = 1;
						long playr = 0;
						long initpos = 0;

						if (temp[0] == '-')
						{
							long angle = -1;

							if (iCharIn(temp, 'A'))
							{
								pos = GetNextWord(es, pos, temp2);

								float fangle = GetVarValueInterpretedAsFloat(temp2, esss, io);
								angle = fangle;

								if (!iCharIn(temp, 'L'))
									player.desiredangle.b = player.angle.b = fangle;


							}

							if (iCharIn(temp, 'N'))
							{
								TELEPORT_TO_CONFIRM = 0;
							}

							if (iCharIn(temp, 'L'))
							{
								pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

								if (NEED_DEBUG)
								{
									strcat(cmd, " ");
									strcat(cmd, temp);
								}

#endif
								strcpy(TELEPORT_TO_LEVEL, temp.c_str());
								pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

								if (NEED_DEBUG)
								{
									strcat(cmd, " ");
									strcat(cmd, temp);
								}

#endif
								strcpy(TELEPORT_TO_POSITION, temp.c_str());

								if (angle == -1) TELEPORT_TO_ANGLE	=	ARX_CLEAN_WARN_CAST_LONG(player.angle.b);
								else TELEPORT_TO_ANGLE = angle;

								CHANGE_LEVEL_ICON = 1;

								if (!TELEPORT_TO_CONFIRM) CHANGE_LEVEL_ICON = 200;

								goto finishteleport;
							}

							if (iCharIn(temp, 'P'))
							{
								playr = 1;
								pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

								if (NEED_DEBUG)
								{
									strcat(cmd, " ");
									strcat(cmd, temp);
								}

#endif
							}

							if (iCharIn(temp, 'I'))
								initpos = 1;

						}

						if (!GAME_EDITOR) TELEPORT_TO_CONFIRM = 0;

						if (initpos == 0)
						{
							long t = GetTargetByNameTarget(temp);

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
										EERIE_3D pos;

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
									EERIE_3D pos;

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
				}
				else if (!strcmp(temp, "TARGETPLAYERPOS"))
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
				else if (!strcmp(temp, "TWEAK"))
				{
					if (io != NULL)
					{
						pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcpy(cmd, "TWEAK ");
							strcat(cmd, temp);
						}

#endif

						long tw;

						if (!strcasecmp(temp, "SKIN"))
						{
							std::string temp1;
							pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp);
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
								ARX_INTERACTIVE_MEMO_TWEAK(io, TWEAK_TYPE_SKIN, temp, temp1);
								EERIE_MESH_TWEAK_Skin(io->obj, temp, temp1);
							}
						}
						else if (!strcasecmp(temp, "ICON"))
						{
							pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp);
							}

#endif

							if (io)
							{
								ARX_INTERACTIVE_MEMO_TWEAK(io, TWEAK_TYPE_ICON, temp, NULL);
								ARX_INTERACTIVE_TWEAK_Icon(io, temp);
							}
						}
						else
						{
							tw = TWEAK_ERROR;

							if (!strcasecmp(temp, "HEAD"))
							{
								if (io->ident == 33)
								{
									tw = tw;
								}

								tw = TWEAK_HEAD;
							}
							else if (!strcasecmp(temp, "TORSO"))
								tw = TWEAK_TORSO;
							else if (!strcasecmp(temp, "LEGS"))
								tw = TWEAK_LEGS;
							else if (!strcasecmp(temp, "ALL"))
								tw = TWEAK_ALL;
							else if (!strcasecmp(temp, "UPPER"))
								tw = TWEAK_UPPER;
							else if (!strcasecmp(temp, "LOWER"))
								tw = TWEAK_LOWER;
							else if (!strcasecmp(temp, "UP_LO"))
								tw = TWEAK_UP_LO;

							if (!strcasecmp(temp, "REMOVE"))
							{
								tw = TWEAK_REMOVE;
								ARX_INTERACTIVE_MEMO_TWEAK(io, tw, NULL, NULL);
								EERIE_MESH_TWEAK_Do(io, tw, NULL);
							}
							else
							{
								pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

								if (NEED_DEBUG)
								{
									strcat(cmd, " ");
									strcat(cmd, temp);
								}

#endif
								std::string path;

								if (io->usemesh != NULL)
									path = io->usemesh;
								else path = io->filename;

								RemoveName(path);
								path += "Tweaks\\";
								path += temp;
								path += ".teo";

								if (tw != TWEAK_ERROR)
								{
									ARX_INTERACTIVE_MEMO_TWEAK(io, tw, path, NULL);
									EERIE_MESH_TWEAK_Do(io, tw, path);
								}
							}
						}
					}

#ifdef NEEDING_DEBUG
					else if (NEED_DEBUG) sprintf(cmd, "ERROR: TWEAK - NOT AN IO !!!");

#endif
				}
				else if ((temp[1] == 'I') && (temp[2] == 'M') && (temp[3] == 'E') && (temp[4] == 'R'))
				{
					// Timer -m nbtimes duration commands
					char timername[64];
					std::string temp2;
					std::string temp3;

					long times = 0;
					long msecs = 0;

					// Checks if the timer is named by caller of if it needs a default name
					if (temp.length() > 5)
						strcpy(timername, temp.c_str() + 5);
					else ARX_SCRIPT_Timer_GetDefaultName(timername);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, temp);
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
						long mili = 0;
						long idle = 0;
						ARX_SCRIPT_Timer_Clear_By_Name_And_IO(timername, io);

						if (strcasecmp(temp2, "OFF"))
						{
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

							times = atoi(temp2.c_str());
							pos = GetNextWord(es, pos, temp3);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp3);
							}

#endif
							msecs = atoi(temp3.c_str());

							if (!mili) msecs *= 1000;

							long num = ARX_SCRIPT_Timer_GetFree();

							if (num != -1)
							{
								ActiveTimers++;
								scr_timer[num].es = es;
								scr_timer[num].exist = 1;
								scr_timer[num].io = io;
								scr_timer[num].msecs = msecs;
								scr_timer[num].namelength = strlen(timername) + 1;
								scr_timer[num].name = (char *)malloc(scr_timer[num].namelength);
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
			case 'V':

				if (!strcmp(temp, "VIEWBLOCK"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "VIEWBLOCK ");
						strcat(cmd, temp);
					}

#endif

					if (io)
					{
						if (!strcasecmp(temp, "ON"))
						{
							io->GameFlags |= GFLAG_VIEW_BLOCKER;
						}
						else io->GameFlags &= ~GFLAG_VIEW_BLOCKER;
					}
				}

				break;
			case 'W':

				if (!strcmp(temp, "WORLDFADE"))
				{
					std::string temp1;
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp1); //duration
					FADEDURATION = GetVarValueInterpretedAsFloat(temp1, esss, io);
					FADESTART = ARX_TIME_GetUL();

					if (!strcasecmp(temp, "OUT"))
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

					if (NEED_DEBUG) sprintf(cmd, "WORLD_FADE %s %s", temp, temp1);

#endif
				}
				else if (!strcmp(temp, "WEAPON"))
				{
					pos = GetNextWord(es, pos, temp);

					if ((io) && (io->ioflags & IO_NPC))
					{
						if ((!strcasecmp(temp, "DRAW")) || (!strcasecmp(temp, "ON")))
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

				if (!strcmp(temp, "USEMESH"))
				{
					pos = GetNextWord(es, pos, temp);
					ARX_INTERACTIVE_MEMO_TWEAK(io, TWEAK_TYPE_MESH, temp, NULL);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "USE_MESH ");
						strcat(cmd, temp);
					}

#endif
					ARX_INTERACTIVE_USEMESH(io, temp);

					std::string tex;
					std::string tex1;
					std::string tex2;

					if (io->ioflags & IO_NPC)	tex2 = "Graph\\Obj3D\\Interactive\\NPC\\" + temp;
					else if (io->ioflags & IO_FIX)	tex2 = "Graph\\Obj3D\\Interactive\\FIX_INTER\\" + temp;
					else if (io->ioflags & IO_ITEM)	tex2 = "Graph\\Obj3D\\Interactive\\Items\\" + temp;
					else tex2[0] = 0;

					File_Standardize(tex2, tex);

					if (tex[0] != 0)
					{
						if (io->usemesh == NULL)
							io->usemesh = (char *)malloc(256);

						strcpy(io->usemesh, tex.c_str());

						if (io->obj != NULL)
						{
							ReleaseEERIE3DObj(io->obj);
							io->obj = NULL;
						}

						const char texpath[] = "Graph\\Obj3D\\Textures\\";

						if (io->ioflags & IO_FIX)
							io->obj = TheoToEerie_Fast(texpath, tex.c_str(), TTE_NO_NDATA | TTE_NO_PHYSICS_BOX, GDevice);
						else if (io->ioflags & IO_NPC)
							io->obj = TheoToEerie_Fast(texpath, tex.c_str(), TTE_NO_PHYSICS_BOX | TTE_NPC, GDevice);
						else
							io->obj = TheoToEerie_Fast(texpath, tex.c_str(), 0, GDevice);

						EERIE_COLLISION_Cylinder_Create(io);
					}
				}

				if (!strcmp(temp, "UNSET"))
				{
					pos = GetNextWord(es, pos, temp, 1);

					if (IsGlobal(temp[0]))
					{
						UNSETVar(svar, &NB_GLOBALS, temp);
					}
					else
					{
						UNSETVar(esss->lvar, &esss->nblvar, temp);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "UNSET %s", temp);

#endif
				}
				else if (!strcmp(temp, "USEPATH"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io->usepath)
					{
						ARX_USE_PATH * aup = (ARX_USE_PATH *)io->usepath;

						if (iCharIn(temp, 'B'))
						{
							aup->aupflags &= ~ARX_USEPATH_PAUSE;
							aup->aupflags &= ~ARX_USEPATH_FORWARD;
							aup->aupflags |= ARX_USEPATH_BACKWARD;
						}

						if (iCharIn(temp, 'F'))
						{
							aup->aupflags &= ~ARX_USEPATH_PAUSE;
							aup->aupflags |= ARX_USEPATH_FORWARD;
							aup->aupflags &= ~ARX_USEPATH_BACKWARD;
						}

						if (iCharIn(temp, 'P'))
						{
							aup->aupflags |= ARX_USEPATH_PAUSE;
							aup->aupflags &= ~ARX_USEPATH_FORWARD;
							aup->aupflags &= ~ARX_USEPATH_BACKWARD;
						}
					}
				}
				else if (!strcmp(temp, "UNSETCONTROLLEDZONE"))
				{
					pos = GetNextWord(es, pos, temp);
					ARX_PATH * ap = ARX_PATH_GetAddressByName(temp.c_str());

					if (ap != NULL)
					{
						ap->controled[0] = 0;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "UNSET_CONTROLLED_ZONE %s", temp);

#endif
				}

				break;
			case 'E':

				if (!strcmp(temp, "ELSE"))
				{
					pos = SkipNextStatement(es, pos);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "ELSE");

#endif
				}
				else if (!strcmp(temp, "ENDINTRO"))
				{
					ARX_INTERFACE_EndIntro();
				}
				else if (!strcmp(temp, "ENDGAME"))
				{
					REFUSE_GAME_RETURN = 1;

					if (FINAL_COMMERCIAL_DEMO)
						ARX_INTERFACE_EndIntro();
					else
					{
						ARX_SOUND_MixerStop(ARX_SOUND_MixerGame);
						ARX_MENU_Launch(GDevice);
						ARX_MENU_Clicked_CREDITS();
					}
				}
				else if (!strcmp(temp, "EATME"))
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
				else if (!strcmp(temp, "EQUIP"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "EQUIP ");
						strcat(cmd, temp);
					}

#endif
					long unequip = 0;

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'R')) unequip = 1;

						pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp);
						}

#endif
					}

					long t = GetTargetByNameTarget(temp);

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
					(!strcmp(temp, "MUL")))
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

					if (NEED_DEBUG) sprintf(cmd, "%s %s %s", temp, temp1, temp2);

#endif

				}
				else if (!strcmp(temp, "MOVE"))
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

						if (NEED_DEBUG) sprintf(cmd, "%s %s %s %s", temp, temp1, temp2, temp3);

#endif
					}
				}
				else if (!strcmp(temp, "MAGIC"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!strcasecmp(temp, "OFF"))
						GLOBAL_MAGIC_MODE = 0;
					else
						GLOBAL_MAGIC_MODE = 1;
				}
				else if (!strcmp(temp, "MAPMARKER"))
				{
					float x, y, t;
					long lvl;

					pos = GetNextWord(es, pos, temp);

					if ((!strcasecmp(temp, "remove")) || (!strcasecmp(temp, "-r")))
					{
						pos = GetNextWord(es, pos, temp);
						ARX_MAPMARKER_Remove(temp);
					}
					else
					{
						x = GetVarValueInterpretedAsFloat(temp, esss, io);
						pos = GetNextWord(es, pos, temp);
						y = GetVarValueInterpretedAsFloat(temp, esss, io);
						pos = GetNextWord(es, pos, temp);
						t = GetVarValueInterpretedAsFloat(temp, esss, io);
						lvl = t;
						pos = GetNextWord(es, pos, temp);
						ARX_MAPMARKER_Add(x, y, lvl, temp);
					}
				}

				break;
			case '-':
			case '+':

				if ((!strcmp(temp, "++")) ||
						(!strcmp(temp, "--")))
				{
					SCRIPT_VAR * sv = NULL;
					std::string temp1;
					long	ival;
					float	fval;
					pos = GetNextWord(es, pos, temp1);

					switch (temp1[0])
					{
						case '#':
							ival = GETVarValueLong(svar, NB_GLOBALS, temp1);

							if (!strcmp(temp, "--"))
							{
								sv = SETVarValueLong(svar, NB_GLOBALS, temp1, ival - 1);
							}
							else
							{
								sv = SETVarValueLong(svar, NB_GLOBALS, temp1, ival + 1);
							}

							break;
						case '\xA3':
							ival = GETVarValueLong(esss->lvar, esss->nblvar, temp1);

							if (!strcmp(temp, "--"))
							{
								sv = SETVarValueLong(esss->lvar, esss->nblvar, temp1, ival - 1);
							}
							else
							{
								sv = SETVarValueFloat(esss->lvar, esss->nblvar, temp1, ival + 1.f);
							}

							break;
						case '&':
							fval = GETVarValueFloat(svar, NB_GLOBALS, temp1);
							ARX_CHECK_NO_ENTRY();

							if (!strcmp(temp, "--"))
							{
								sv = SETVarValueFloat(svar, NB_GLOBALS, temp1, fval  - 1.f);
							}
							else
							{
								sv = SETVarValueFloat(esss->lvar, esss->nblvar, temp1, fval + 1.f);
							}

							break;
						case '@':
							fval = GETVarValueFloat(esss->lvar, esss->nblvar, temp1);

							if (!strcmp(temp, "--"))
							{
								sv = SETVarValueFloat(esss->lvar, esss->nblvar, temp1, fval - 1.f);
							}
							else
							{
								sv = SETVarValueFloat(esss->lvar, esss->nblvar, temp1, fval + 1.f);
							}

							break;
						default:
							return BIGERROR;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "%s %s", temp, temp1);

#endif
				}

			case 'D':

				if (
					(!strcmp(temp, "DEC")) ||
					(!strcmp(temp, "DIV")))
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
						strcpy(cmd, temp);
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

					switch (temp1[0])
					{
						case '$': // GLOBAL TEXT
						case '\xA3': // LOCAL TEXT
							ShowScriptError("Unable to execute this\nOperation on a String", cmd);
							break;
						case '#': // GLOBAL LONG
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fdval = (float)GETVarValueLong(svar, NB_GLOBALS, temp1);

							if (!strcmp(temp, "DEC")) fval = fdval - fval;
							else if (!strcmp(temp, "DIV"))
							{
								if (fval != 0.f)	fval = fdval / fval;
							}

							sv = SETVarValueLong(svar, NB_GLOBALS, temp1, (long)fval);

							if (sv != NULL) sv->type = TYPE_G_LONG;

							break;
						case '\xA7': // LOCAL LONG
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fdval = (float)GETVarValueLong(esss->lvar, esss->nblvar, temp1);

							if (!strcmp(temp, "DEC")) fval = fdval - fval;
							else if (!strcmp(temp, "DIV"))
							{
								if (fval != 0.f)	fval = fdval / fval;
							}

							sv = SETVarValueLong(esss->lvar, esss->nblvar, temp1, (long)fval);

							if (sv != NULL) sv->type = TYPE_L_LONG;

							break;
						case '&': // GLOBAL float
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fdval = GETVarValueFloat(svar, NB_GLOBALS, temp1);

							if (!strcmp(temp, "DEC")) fval = fdval - fval;
							else if (!strcmp(temp, "DIV"))
							{
								if (fval != 0.f)	fval = fdval / fval;
							}

							sv = SETVarValueFloat(svar, NB_GLOBALS, temp1, fval);

							if (sv != NULL) sv->type = TYPE_G_FLOAT;

							break;
						case '@': // LOCAL float
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fdval = GETVarValueFloat(esss->lvar, esss->nblvar, temp1);

							if (!strcmp(temp, "DEC")) fval = fdval - fval;
							else if (!strcmp(temp, "DIV"))
							{
								if (fval != 0.f)	fval = fdval / fval;
							}

							sv = SETVarValueFloat(esss->lvar, esss->nblvar, temp1, fval);

							if (sv != NULL) sv->type = TYPE_L_FLOAT;

							break;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "%s %s %s", temp, temp1, temp2);

#endif
				}
				else if (!strcmp(temp, "DESTROY"))
				{
					std::string temp2;
					pos = GetNextWord(es, pos, temp2); // Source IO
					temp = GetVarValueInterpretedAsText(temp2, esss, io);
					long t = GetTargetByNameTarget(temp);

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
				else if (!strcmp(temp, "DETACHNPCFROMPLAYER"))
				{
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "DETACH_NPC_FROM_PLAYER ...OBSOLETE...");

#endif
				}
				else if (!strcmp(temp, "DODAMAGE"))
				{
					pos = GetNextWord(es, pos, temp); // Source IO
					long type = 0;

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'F'))
							type |= DAMAGE_TYPE_FIRE;

						if (iCharIn(temp, 'M'))
							type |= DAMAGE_TYPE_MAGICAL;

						if (iCharIn(temp, 'P'))
							type |= DAMAGE_TYPE_POISON;

						if (iCharIn(temp, 'L'))
							type |= DAMAGE_TYPE_LIGHTNING;

						if (iCharIn(temp, 'C'))
							type |= DAMAGE_TYPE_COLD;

						if (iCharIn(temp, 'G'))
							type |= DAMAGE_TYPE_GAS;

						if (iCharIn(temp, 'E'))
							type |= DAMAGE_TYPE_METAL;

						if (iCharIn(temp, 'W'))
							type |= DAMAGE_TYPE_WOOD;

						if (iCharIn(temp, 'S'))
							type |= DAMAGE_TYPE_STONE;

						if (iCharIn(temp, 'A'))
							type |= DAMAGE_TYPE_ACID;

						if (iCharIn(temp, 'O'))
							type |= DAMAGE_TYPE_ORGANIC;

						if (iCharIn(temp, 'R'))
							type |= DAMAGE_TYPE_DRAIN_LIFE;

						if (iCharIn(temp, 'N'))
							type |= DAMAGE_TYPE_DRAIN_MANA;

						if (iCharIn(temp, 'U'))
							type |= DAMAGE_TYPE_PUSH;

						pos = GetNextWord(es, pos, temp);
					}

					long t = GetTargetByNameTarget(temp);

					if (t == -2) t = GetInterNum(io); //self

					pos = GetNextWord(es, pos, temp);
					float fval = GetVarValueInterpretedAsFloat(temp, esss, io);

					if (ValidIONum(t))
						ARX_DAMAGES_DealDamages(t, fval, GetInterNum(io), type, &inter.iobj[t]->pos);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "DODAMAGE");

#endif
				}
				else if (!strcmp(temp, "DAMAGER"))
				{
					io->damager_type = DAMAGE_TYPE_PER_SECOND;
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'F'))
							io->damager_type |= DAMAGE_TYPE_FIRE;

						if (iCharIn(temp, 'M'))
							io->damager_type |= DAMAGE_TYPE_MAGICAL;

						if (iCharIn(temp, 'P'))
							io->damager_type |= DAMAGE_TYPE_POISON;

						if (iCharIn(temp, 'L'))
							io->damager_type |= DAMAGE_TYPE_LIGHTNING;

						if (iCharIn(temp, 'C'))
							io->damager_type |= DAMAGE_TYPE_COLD;

						if (iCharIn(temp, 'G'))
							io->damager_type |= DAMAGE_TYPE_GAS;

						if (iCharIn(temp, 'E'))
							io->damager_type |= DAMAGE_TYPE_METAL;

						if (iCharIn(temp, 'W'))
							io->damager_type |= DAMAGE_TYPE_WOOD;

						if (iCharIn(temp, 'S'))
							io->damager_type |= DAMAGE_TYPE_STONE;

						if (iCharIn(temp, 'A'))
							io->damager_type |= DAMAGE_TYPE_ACID;

						if (iCharIn(temp, 'O'))
							io->damager_type |= DAMAGE_TYPE_ORGANIC;

						if (iCharIn(temp, 'R'))
							io->damager_type |= DAMAGE_TYPE_DRAIN_LIFE;

						if (iCharIn(temp, 'N'))
							io->damager_type |= DAMAGE_TYPE_DRAIN_MANA;

						if (iCharIn(temp, 'U'))
							io->damager_type |= DAMAGE_TYPE_PUSH;

						pos = GetNextWord(es, pos, temp);
					}

					float fval = GetVarValueInterpretedAsFloat(temp, esss, io);



					ARX_CHECK_SHORT(fval);

					io->damager_damages = ARX_CLEAN_WARN_CAST_SHORT(fval);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "DAMAGER");

#endif
				}
				else if (!strcmp(temp, "DETACH"))
				{
					pos = GetNextWord(es, pos, temp); // Source IO
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "DETACH ");
						strcat(cmd, temp);
					}

#endif
					long t = GetTargetByNameTarget(temp);

					if (t == -2) t = GetInterNum(io); //self

					pos = GetNextWord(es, pos, temp); // target IO
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcat(cmd, " ");
						strcat(cmd, tempo);
					}

#endif
					long t2 = GetTargetByNameTarget(temp);

					if (t2 == -2) t2 = GetInterNum(io); //self

					ARX_INTERACTIVE_Detach(t, t2);
				}

				else if (!strcmp(temp, "DRAWSYMBOL")) // DRAWSYMBOL symbol duration
				{
					std::string temp1;
					std::string temp2;
					pos = GetNextWord(es, pos, temp1);
					pos = GetNextWord(es, pos, temp2);

					if (io != NULL)
					{
						MakeUpcase(temp1);
						float dur = GetVarValueInterpretedAsFloat(temp2, esss, io);
						ARX_SPELLS_RequestSymbolDraw(io, temp1.c_str(), dur);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "DRAW_SYMBOL %s %s", temp1, temp2);

#endif
				}

				break;
			default:

				if (io)
				{
					std::string temp2;
					std::string temp3;
					std::string temp4;
					long ppos = pos;
					pos = GetNextWord(es, pos, temp2);
					pos = GetNextWord(es, pos, temp3);
					pos = GetNextWord(es, pos, temp4);
					sprintf(cmd, "SCRIPT ERROR: %s_%04ld %s %s %s %s [char %ld]", GetName(io->filename).c_str(), io->ident, temp.c_str(), temp2.c_str(), temp3.c_str(), temp4.c_str(), ppos);

					LogError << cmd;

					io->ioflags |= IO_FREEZESCRIPT;
					return REFUSE;
				}

#ifdef NEEDING_DEBUG

				if (NEED_DEBUG)
				{
					LogError << "unknown command: " << temp;
				}

#endif

		}

#ifdef NEEDING_DEBUG

		if ((NEED_DEBUG) && (cmd[0] != 0))
		{
			LogDebug << "  " << cmd;
		}

#endif
	}

end:
	;

#ifdef NEEDING_DEBUG

	if (NEED_DEBUG)
	{
		if (msg != SM_EXECUTELINE)
		{
			if (evname) LogDebug << eventname << " EVENT Successfully Finished";
			else if (msg != SM_DUMMY) LogDebug << AS_EVENT[msg].name << " EVENT Successfully Finished";
			else LogDebug << "Dummy EVENT Successfully Finished";
		}
		else
		{
			LogDebug << "EXECUTELINE Successfully Finished";
		}
	}

#endif
	return ret;
}
