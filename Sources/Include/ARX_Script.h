/*
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
// ARX_Script
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Script Management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
/////////////////////////////////////////////////////////////////////////////////////
#ifndef ARX_SCRIPTING_H
#define ARX_SCRIPTING_H

#include <EERIEPoly.h>
#include <EERIEMeshTweak.h>
#include <TCHAR.h>

//-----------------------------------------------------------------------------
typedef struct
{
	char		*	name;
	short			exist;
	short			flags; 
	long			namelength;
	long			times;
	long			msecs;
	long			pos;
	long			longinfo;
	unsigned long	tim;
	INTERACTIVE_OBJ * io;
	EERIE_SCRIPT	* es;
} SCR_TIMER;

typedef struct
{
	char		*	name;
} SCRIPT_EVENT;

//-----------------------------------------------------------------------------
#define PATHFIND_ALWAYS		1
#define PATHFIND_ONCE		2
#define PATHFIND_NO_UPDATE	4

#define ANIM_DEFAULT					0
#define ANIM_WAIT						0
#define ANIM_WALK						1
#define ANIM_WALK2						2
#define ANIM_WALK3						3
#define ANIM_ACTION						8
#define ANIM_ACTION2					9
#define ANIM_ACTION3					10
#define ANIM_HIT1						11
#define ANIM_STRIKE1					12
#define ANIM_DIE						13
#define ANIM_WAIT2						14
#define ANIM_RUN						15
#define ANIM_RUN2						16
#define ANIM_RUN3						17
#define ANIM_ACTION4					18
#define ANIM_ACTION5					19
#define ANIM_ACTION6					20
#define ANIM_ACTION7					21
#define ANIM_ACTION8					22
#define ANIM_ACTION9					23
#define ANIM_ACTION10					24
#define ANIM_TALK_NEUTRAL				30
#define ANIM_TALK_HAPPY					31
#define ANIM_TALK_ANGRY					32

#define ANIM_WALK_BACKWARD				33

#define ANIM_BARE_READY					34
#define ANIM_BARE_UNREADY				(ANIM_BARE_READY+1)
#define ANIM_BARE_WAIT					(ANIM_BARE_READY+2)
#define ANIM_BARE_STRIKE_LEFT_START		(ANIM_BARE_READY+3)
#define ANIM_BARE_STRIKE_LEFT_CYCLE		(ANIM_BARE_READY+4)
#define ANIM_BARE_STRIKE_LEFT			(ANIM_BARE_READY+5)
#define ANIM_BARE_STRIKE_RIGHT_START	(ANIM_BARE_READY+6)
#define ANIM_BARE_STRIKE_RIGHT_CYCLE	(ANIM_BARE_READY+7)
#define ANIM_BARE_STRIKE_RIGHT			(ANIM_BARE_READY+8)
#define ANIM_BARE_STRIKE_TOP_START		(ANIM_BARE_READY+9)
#define ANIM_BARE_STRIKE_TOP_CYCLE		(ANIM_BARE_READY+10)
#define ANIM_BARE_STRIKE_TOP			(ANIM_BARE_READY+11)
#define ANIM_BARE_STRIKE_BOTTOM_START	(ANIM_BARE_READY+12)
#define ANIM_BARE_STRIKE_BOTTOM_CYCLE	(ANIM_BARE_READY+13)
#define ANIM_BARE_STRIKE_BOTTOM			(ANIM_BARE_READY+14)

#define ANIM_1H_READY_PART_1			(ANIM_BARE_STRIKE_BOTTOM+1)
#define ANIM_1H_READY_PART_2			(ANIM_1H_READY_PART_1+1)
#define ANIM_1H_UNREADY_PART_1			(ANIM_1H_READY_PART_1+2)
#define ANIM_1H_UNREADY_PART_2			(ANIM_1H_READY_PART_1+3)
#define ANIM_1H_WAIT					(ANIM_1H_READY_PART_1+4)
#define ANIM_1H_STRIKE_LEFT_START		(ANIM_1H_READY_PART_1+5)
#define ANIM_1H_STRIKE_LEFT_CYCLE		(ANIM_1H_READY_PART_1+6)
#define ANIM_1H_STRIKE_LEFT				(ANIM_1H_READY_PART_1+7)
#define ANIM_1H_STRIKE_RIGHT_START		(ANIM_1H_READY_PART_1+8)
#define ANIM_1H_STRIKE_RIGHT_CYCLE		(ANIM_1H_READY_PART_1+9)
#define ANIM_1H_STRIKE_RIGHT			(ANIM_1H_READY_PART_1+10)
#define ANIM_1H_STRIKE_TOP_START		(ANIM_1H_READY_PART_1+11)
#define ANIM_1H_STRIKE_TOP_CYCLE		(ANIM_1H_READY_PART_1+12)
#define ANIM_1H_STRIKE_TOP				(ANIM_1H_READY_PART_1+13)
#define ANIM_1H_STRIKE_BOTTOM_START		(ANIM_1H_READY_PART_1+14)
#define ANIM_1H_STRIKE_BOTTOM_CYCLE		(ANIM_1H_READY_PART_1+15)
#define ANIM_1H_STRIKE_BOTTOM			(ANIM_1H_READY_PART_1+16)

#define ANIM_2H_READY_PART_1			(ANIM_1H_STRIKE_BOTTOM+1) //66
#define ANIM_2H_READY_PART_2			(ANIM_2H_READY_PART_1+1)
#define ANIM_2H_UNREADY_PART_1			(ANIM_2H_READY_PART_1+2)
#define ANIM_2H_UNREADY_PART_2			(ANIM_2H_READY_PART_1+3)
#define ANIM_2H_WAIT					(ANIM_2H_READY_PART_1+4)
#define ANIM_2H_STRIKE_LEFT_START		(ANIM_2H_READY_PART_1+5)
#define ANIM_2H_STRIKE_LEFT_CYCLE		(ANIM_2H_READY_PART_1+6)
#define ANIM_2H_STRIKE_LEFT				(ANIM_2H_READY_PART_1+7)
#define ANIM_2H_STRIKE_RIGHT_START		(ANIM_2H_READY_PART_1+8)
#define ANIM_2H_STRIKE_RIGHT_CYCLE		(ANIM_2H_READY_PART_1+9)
#define ANIM_2H_STRIKE_RIGHT			(ANIM_2H_READY_PART_1+10)
#define ANIM_2H_STRIKE_TOP_START		(ANIM_2H_READY_PART_1+11)
#define ANIM_2H_STRIKE_TOP_CYCLE		(ANIM_2H_READY_PART_1+12)
#define ANIM_2H_STRIKE_TOP				(ANIM_2H_READY_PART_1+13)
#define ANIM_2H_STRIKE_BOTTOM_START		(ANIM_2H_READY_PART_1+14)
#define ANIM_2H_STRIKE_BOTTOM_CYCLE		(ANIM_2H_READY_PART_1+15)
#define ANIM_2H_STRIKE_BOTTOM			(ANIM_2H_READY_PART_1+16)

#define ANIM_DAGGER_READY_PART_1		(ANIM_2H_STRIKE_BOTTOM+1) //82
#define ANIM_DAGGER_READY_PART_2		(ANIM_DAGGER_READY_PART_1+1)
#define ANIM_DAGGER_UNREADY_PART_1		(ANIM_DAGGER_READY_PART_1+2)
#define ANIM_DAGGER_UNREADY_PART_2		(ANIM_DAGGER_READY_PART_1+3)
#define ANIM_DAGGER_WAIT				(ANIM_DAGGER_READY_PART_1+4)
#define ANIM_DAGGER_STRIKE_LEFT_START	(ANIM_DAGGER_READY_PART_1+5)
#define ANIM_DAGGER_STRIKE_LEFT_CYCLE	(ANIM_DAGGER_READY_PART_1+6)
#define ANIM_DAGGER_STRIKE_LEFT			(ANIM_DAGGER_READY_PART_1+7)
#define ANIM_DAGGER_STRIKE_RIGHT_START	(ANIM_DAGGER_READY_PART_1+8)
#define ANIM_DAGGER_STRIKE_RIGHT_CYCLE	(ANIM_DAGGER_READY_PART_1+9)
#define ANIM_DAGGER_STRIKE_RIGHT		(ANIM_DAGGER_READY_PART_1+10)
#define ANIM_DAGGER_STRIKE_TOP_START	(ANIM_DAGGER_READY_PART_1+11)
#define ANIM_DAGGER_STRIKE_TOP_CYCLE	(ANIM_DAGGER_READY_PART_1+12)
#define ANIM_DAGGER_STRIKE_TOP			(ANIM_DAGGER_READY_PART_1+13)
#define ANIM_DAGGER_STRIKE_BOTTOM_START	(ANIM_DAGGER_READY_PART_1+14)
#define ANIM_DAGGER_STRIKE_BOTTOM_CYCLE	(ANIM_DAGGER_READY_PART_1+15)
#define ANIM_DAGGER_STRIKE_BOTTOM		(ANIM_DAGGER_READY_PART_1+16)

#define ANIM_MISSILE_READY_PART_1		(ANIM_DAGGER_STRIKE_BOTTOM+1) //99
#define ANIM_MISSILE_READY_PART_2		(ANIM_MISSILE_READY_PART_1+1)
#define ANIM_MISSILE_UNREADY_PART_1		(ANIM_MISSILE_READY_PART_1+2)
#define ANIM_MISSILE_UNREADY_PART_2		(ANIM_MISSILE_READY_PART_1+3)
#define ANIM_MISSILE_WAIT				(ANIM_MISSILE_READY_PART_1+4)
#define ANIM_MISSILE_STRIKE_PART_1		(ANIM_MISSILE_READY_PART_1+5)
#define ANIM_MISSILE_STRIKE_PART_2		(ANIM_MISSILE_READY_PART_1+6)
#define ANIM_MISSILE_STRIKE_CYCLE		(ANIM_MISSILE_READY_PART_1+7)
#define ANIM_MISSILE_STRIKE				(ANIM_MISSILE_READY_PART_1+8)

#define ANIM_SHIELD_START				(ANIM_MISSILE_STRIKE+1) //108
#define ANIM_SHIELD_CYCLE				(ANIM_SHIELD_START+1)
#define ANIM_SHIELD_HIT					(ANIM_SHIELD_START+2)
#define ANIM_SHIELD_END					(ANIM_SHIELD_START+3)

#define ANIM_CAST_START					(ANIM_SHIELD_END+1) //112
#define ANIM_CAST_CYCLE					(ANIM_CAST_START+1)
#define ANIM_CAST						(ANIM_CAST_START+2)
#define ANIM_CAST_END					(ANIM_CAST_START+3)

#define ANIM_DEATH_CRITICAL				(ANIM_CAST_END+1)
#define ANIM_CROUCH						(ANIM_CAST_END+2)
#define ANIM_CROUCH_WALK				(ANIM_CAST_END+3)
#define ANIM_CROUCH_WALK_BACKWARD		(ANIM_CAST_END+4)
#define ANIM_LEAN_RIGHT					(ANIM_CAST_END+5)
#define ANIM_LEAN_LEFT					(ANIM_CAST_END+6)
#define ANIM_JUMP						(ANIM_CAST_END+7)
#define ANIM_HOLD_TORCH					(ANIM_CAST_END+8)
#define ANIM_WALK_MINISTEP				(ANIM_CAST_END+9)
#define ANIM_STRAFE_RIGHT				(ANIM_CAST_END+10)
#define ANIM_STRAFE_LEFT				(ANIM_CAST_END+11)
#define ANIM_MEDITATION					(ANIM_CAST_END+12)
#define ANIM_WAIT_SHORT					(ANIM_CAST_END+13)

#define ANIM_FIGHT_WALK_FORWARD			(ANIM_CAST_END+14)
#define ANIM_FIGHT_WALK_BACKWARD		(ANIM_CAST_END+15)
#define ANIM_FIGHT_WALK_MINISTEP		(ANIM_CAST_END+16)
#define ANIM_FIGHT_STRAFE_RIGHT			(ANIM_CAST_END+17)
#define ANIM_FIGHT_STRAFE_LEFT			(ANIM_CAST_END+18)
#define ANIM_FIGHT_WAIT					(ANIM_CAST_END+19)

#define ANIM_LEVITATE					(ANIM_CAST_END+20)
#define ANIM_CROUCH_START               (ANIM_CAST_END+21)
#define	ANIM_CROUCH_WAIT				(ANIM_CAST_END+22)
#define	ANIM_CROUCH_END					(ANIM_CAST_END+23)
#define ANIM_JUMP_ANTICIPATION			(ANIM_CAST_END+24)
#define	ANIM_JUMP_UP					(ANIM_CAST_END+25)
#define ANIM_JUMP_CYCLE					(ANIM_CAST_END+26)
#define ANIM_JUMP_END					(ANIM_CAST_END+27)
#define ANIM_TALK_NEUTRAL_HEAD			(ANIM_CAST_END+28)
#define ANIM_TALK_ANGRY_HEAD			(ANIM_CAST_END+29)
#define ANIM_TALK_HAPPY_HEAD			(ANIM_CAST_END+30)
#define ANIM_STRAFE_RUN_LEFT			(ANIM_CAST_END+31)
#define ANIM_STRAFE_RUN_RIGHT			(ANIM_CAST_END+32)
#define ANIM_CROUCH_STRAFE_LEFT			(ANIM_CAST_END+33)
#define ANIM_CROUCH_STRAFE_RIGHT		(ANIM_CAST_END+34)
#define ANIM_WALK_SNEAK					(ANIM_CAST_END+35)
#define ANIM_GRUNT						(ANIM_CAST_END+36)
#define ANIM_JUMP_END_PART2				(ANIM_CAST_END+37)
#define ANIM_HIT_SHORT					(ANIM_CAST_END+38)
#define ANIM_U_TURN_LEFT				(ANIM_CAST_END+39)
#define ANIM_U_TURN_RIGHT				(ANIM_CAST_END+40)
#define ANIM_RUN_BACKWARD				(ANIM_CAST_END+41)
#define ANIM_U_TURN_LEFT_FIGHT			(ANIM_CAST_END+42)
#define ANIM_U_TURN_RIGHT_FIGHT			(ANIM_CAST_END+43)

#define ACCEPT 1
#define REFUSE -1
#define BIGERROR -2
#define KILLBOTH 2
#define KILLCOMBINER 3
#define KILLCOMBINEDWITH 4

#define SM_NULL				0
#define SM_INIT				1
#define SM_INVENTORYIN		2
#define SM_INVENTORYOUT		3
#define SM_INVENTORYUSE		4
#define SM_SCENEUSE			5
#define SM_EQUIPIN			6
#define SM_EQUIPOUT			7
#define SM_MAIN				8
#define SM_RESET			9
#define SM_CHAT				10
#define SM_ACTION			11
#define SM_DEAD				12
#define SM_REACHEDTARGET	13
#define SM_FIGHT			14
#define SM_FLEE				15
#define SM_HIT				16
#define SM_DIE				17
#define SM_LOSTTARGET		18
#define SM_TREATIN			19
#define SM_TREATOUT			20
#define SM_MOVE				21
#define SM_DETECTPLAYER		22
#define SM_UNDETECTPLAYER	23
#define SM_COMBINE			24
#define SM_NPC_FOLLOW		25
#define SM_NPC_FIGHT		26
#define SM_NPC_STAY			27
#define SM_INVENTORY2_OPEN	28
#define SM_INVENTORY2_CLOSE 29
#define SM_CUSTOM			30
#define SM_ENTER_ZONE		31
#define SM_LEAVE_ZONE		32
#define SM_INITEND			33
#define SM_CLICKED			34
#define SM_INSIDEZONE		35
#define SM_CONTROLLEDZONE_INSIDE	36
#define SM_LEAVEZONE				37
#define SM_CONTROLLEDZONE_LEAVE		38
#define SM_ENTERZONE				39
#define SM_CONTROLLEDZONE_ENTER		40
#define SM_LOAD				41
#define SM_SPELLCAST		42
#define SM_RELOAD			43
#define SM_COLLIDE_DOOR		44
#define SM_OUCH				45
#define SM_HEAR				46
#define SM_SUMMONED			47
#define SM_SPELLEND			48
#define SM_SPELLDECISION	49
#define SM_STRIKE			50
#define SM_COLLISION_ERROR	51
#define SM_WAYPOINT			52
#define SM_PATHEND			53
#define SM_CRITICAL			54
#define SM_COLLIDE_NPC		55
#define SM_BACKSTAB			56
#define SM_AGGRESSION		57
#define SM_COLLISION_ERROR_DETAIL	58
#define SM_GAME_READY		59
#define SM_CINE_END			60
#define SM_KEY_PRESSED		61
#define SM_CONTROLS_ON		62
#define SM_CONTROLS_OFF		63
#define SM_PATHFINDER_FAILURE	64
#define SM_PATHFINDER_SUCCESS	65
#define SM_TRAP_DISARMED	66
#define SM_BOOK_OPEN		67
#define SM_BOOK_CLOSE		68
#define SM_IDENTIFY			69
#define SM_BREAK			70
#define SM_STEAL			71
#define SM_COLLIDE_FIELD	72
#define SM_CURSORMODE		73
#define SM_EXPLORATIONMODE	74
#define SM_MAXCMD			75
#define SM_EXECUTELINE		255
#define SM_DUMMY			256
#define DISABLE_HIT				1
#define DISABLE_CHAT			2
#define DISABLE_INVENTORY2_OPEN	4
#define DISABLE_HEAR			8
#define DISABLE_DETECT			16
#define DISABLE_AGGRESSION		32
#define DISABLE_MAIN			64
#define DISABLE_COLLIDE_NPC		128
#define DISABLE_CURSORMODE		256
#define DISABLE_EXPLORATIONMODE	512

#define TYPE_UNKNOWN	0  // does not exist !
#define TYPE_G_TEXT		1
#define TYPE_L_TEXT		2
#define TYPE_G_LONG		4
#define TYPE_L_LONG		8
#define TYPE_G_FLOAT	16
#define TYPE_L_FLOAT	32

#define OPER_EQUAL		1
#define OPER_NOTEQUAL	2
#define OPER_INFEQUAL	3
#define OPER_INFERIOR	4
#define OPER_SUPEQUAL	5
#define OPER_SUPERIOR	6
#define OPER_INCLASS	7
#define OPER_ISELEMENT	8
#define OPER_ISIN		9
#define OPER_ISTYPE		10
#define OPER_ISGROUP	11
#define OPER_NOTISGROUP	12

#define BIG_DEBUG_SIZE 65000
#define ONE_HANDED_WEAPON	1
#define TWO_HANDED_WEAPON	2

//-----------------------------------------------------------------------------
extern SCRIPT_VAR * svar;
extern INTERACTIVE_OBJ * EVENT_SENDER;
extern SCRIPT_EVENT AS_EVENT[];
extern SCR_TIMER * scr_timer;
extern char ShowTextWindowtext[128];
extern char ShowText[65536];
extern char ShowText2[65536];
extern char BIG_DEBUG_STRING[BIG_DEBUG_SIZE];
extern long NEED_DEBUG;
extern long BIG_DEBUG_POS;
extern long NB_GLOBALS;
extern long ActiveTimers;
extern long Event_Total_Count;
extern long FORBID_SCRIPT_IO_CREATION;
extern long MAX_TIMER_SCRIPT;

extern LRESULT CALLBACK ShowTextDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK ShowVarsDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

//-----------------------------------------------------------------------------
void InitScript(EERIE_SCRIPT * es);
void InitScriptVars(long nbvars);

void ARX_SCRIPT_Timer_Check();
void ARX_SCRIPT_Timer_FirstInit(long number);
void ARX_SCRIPT_Timer_ClearAll();
void ARX_SCRIPT_Timer_Clear_For_IO(INTERACTIVE_OBJ * io);
 
void ARX_SCRIPT_Timer_Clear_By_IO(INTERACTIVE_OBJ * io);
long ARX_SCRIPT_Timer_GetFree();
 
void ARX_SCRIPT_SetMainEvent(INTERACTIVE_OBJ * io, char * newevent);
void ARX_SCRIPT_EventStackExecute();
void ARX_SCRIPT_EventStackExecuteAll();
void ARX_SCRIPT_EventStackInit();
void ARX_SCRIPT_EventStackClear();
void ARX_SCRIPT_LaunchScriptSearch(char * search);
void ARX_SCRIPT_ResetObject(INTERACTIVE_OBJ * io, long flags);
void ARX_SCRIPT_Reset(INTERACTIVE_OBJ * io, long flags);
long ARX_SCRIPT_GetSystemIOScript(INTERACTIVE_OBJ * io, char * name);
void ARX_SCRIPT_ComputeShortcuts(EERIE_SCRIPT * es);
void ARX_SCRIPT_AllowInterScriptExec();
long ARX_SCRIPT_CountTimers();
void ARX_SCRIPT_Timer_ClearByNum(long num);
void ARX_SCRIPT_ResetAll(long flags);
void ARX_SCRIPT_EventStackClearForIo(INTERACTIVE_OBJ * io);
INTERACTIVE_OBJ * ARX_SCRIPT_Get_IO_Max_Events();
INTERACTIVE_OBJ * ARX_SCRIPT_Get_IO_Max_Events_Sent();
BOOL CheckScriptSyntax_Loading(INTERACTIVE_OBJ * io);
BOOL CheckScriptSyntax(INTERACTIVE_OBJ * io);

void ManageNPCMovement(INTERACTIVE_OBJ * io);
void ReleaseScript(EERIE_SCRIPT * es);
long GetNextWord(EERIE_SCRIPT * es, long i, char * temp, long flags = 0);
void ARX_SCRIPT_Init_Event_Stats();
void ARX_SCRIPT_SetVar(INTERACTIVE_OBJ * io, char * name, char * content);
void InitAllGlobalVars();
long SendInitScriptEvent(INTERACTIVE_OBJ * io);
void ClearSubStack(EERIE_SCRIPT * es);

//-----------------------------------------------------------------------------

long MakeLocalised(char * text, _TCHAR * output, long maxsize, long lastspeechflag = 0);

//-----------------------------------------------------------------------------
long specialstrcmp(char * text, char * seek);
void CheckHit(INTERACTIVE_OBJ * io, float ratio);
long NotifyIOEvent(INTERACTIVE_OBJ * io, long msg, char * params);
void ForceAnim(INTERACTIVE_OBJ * io, ANIM_HANDLE * ea);

long ARX_SPEECH_AddLocalised(INTERACTIVE_OBJ * io, char * text, long duration = -1);
long ARX_SPEECH_ForceLocalised(INTERACTIVE_OBJ * io, char * text, long duration = -1);

long SendIOScriptEvent(INTERACTIVE_OBJ * io, long msg, char * params, char * eventname = NULL);
long SendScriptEvent(EERIE_SCRIPT * es, long msg, char * params, INTERACTIVE_OBJ * io, char * eventname, long info = 0);
long SendMsgToAllIO(long msg, char * dat);

void Stack_SendIOScriptEvent(INTERACTIVE_OBJ * io, long msg, char * params, char * eventname);
BOOL InSubStack(EERIE_SCRIPT * es, long pos);
long GetSubStack(EERIE_SCRIPT * es);
void AttemptMoveToTarget(INTERACTIVE_OBJ * io);
void GetTargetPos(INTERACTIVE_OBJ * io, unsigned long smoothing = 0);
void ARX_IOGROUP_Release(INTERACTIVE_OBJ * io);
void CloneLocalVars(INTERACTIVE_OBJ * ioo, INTERACTIVE_OBJ * io);
BOOL IsIOGroup(INTERACTIVE_OBJ * io, char * group);
void ARX_SCRIPT_Free_All_Global_Variables();
void		MakeLocalText(EERIE_SCRIPT * es, char * tx);
void		MakeGlobalText(char * tx);

#endif
