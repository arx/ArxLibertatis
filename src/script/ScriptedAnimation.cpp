
#include "script/ScriptedAnimation.h"

#include "ai/Paths.h"
#include "core/GameTime.h"
#include "game/NPC.h"
#include "graphics/Math.h"
#include "graphics/data/Mesh.h"
#include "io/FilePath.h"
#include "scene/Interactive.h"
#include "script/ScriptUtils.h"

using std::string;

namespace script {

namespace {

class RotateCommand : public Command {
	
public:
	
	RotateCommand() : Command("rotate", ANY_IO) { }
	
	Result execute(Context & context) {
		
		INTERACTIVE_OBJ * io = context.getIO();
		
		float t1 = context.getFloat();
		float t2 = context.getFloat();
		float t3 = context.getFloat();
		
		DebugScript(' ' << t1 << ' ' << t2 << ' ' << t3);
		
		io->angle.a += t1;
		io->angle.b += t2;
		io->angle.g += t3;
		
		if((size_t)io->nb_lastanimvertex != io->obj->vertexlist.size()) {
			free(io->lastanimvertex);
			io->lastanimvertex = NULL;
		}
		io->lastanimtime = 0;
		
		return Success;
	}
	
};

class ForceAnimCommand : public Command {
	
	static void forceAnim(INTERACTIVE_OBJ & io, ANIM_HANDLE * ea) {
		
		if(io.animlayer[0].cur_anim
		   && io.animlayer[0].cur_anim != io.anims[ANIM_DIE]
		   && io.animlayer[0].cur_anim != io.anims[ANIM_HIT1]) {
			AcquireLastAnim(&io);
		}
		
		FinishAnim(&io, io.animlayer[0].cur_anim);
		io.lastmove = Vec3f::ZERO;
		ANIM_Set(&io.animlayer[0], ea);
		io.animlayer[0].flags |= EA_FORCEPLAY;
		io.animlayer[0].nextflags = 0;
		
		CheckSetAnimOutOfTreatZone(&io, 0);
	}
	
public:
	
	ForceAnimCommand() : Command("forceanim", ANY_IO) { }
	
	Result execute(Context & context) {
		
		string anim = context.getLowercase();
		
		DebugScript(' ' << anim);
		
		AnimationNumber num = GetNumAnim(anim);
		if(num == ANIM_NONE) {
			ScriptWarning << "unknown animation: " << anim;
			return Failed;
		}
		
		INTERACTIVE_OBJ & io = *context.getIO();
		if(!io.anims[num]) {
			ScriptWarning << "animation " << anim << " not set";
			return Failed;
		}
		
		forceAnim(io, io.anims[num]);
		
		return Success;
	}
	
};

class ForceAngleCommand : public Command {
	
public:
	
	ForceAngleCommand() : Command("forceangle", ANY_IO) { }
	
	Result execute(Context & context) {
		
		float angle = MAKEANGLE(context.getFloat());
		
		DebugScript(' ' << angle);
		
		context.getIO()->angle.b = angle;
		
		return Success;
	}
	
};

class PlayAnimCommand : public Command {
	
	static void SetNextAnim(INTERACTIVE_OBJ * io, ANIM_HANDLE * ea, long layer, bool loop, bool nointerpol) {
		
		if(IsDeadNPC(io)) {
			return;
		}
		
		if(nointerpol) {
			AcquireLastAnim(io);
		}
		
		FinishAnim(io, io->animlayer[layer].cur_anim);
		ANIM_Set(&io->animlayer[layer], ea);
		io->animlayer[layer].next_anim = NULL;
		
		if(loop) {
			io->animlayer[layer].flags |= EA_LOOP;
		} else {
			io->animlayer[layer].flags &= ~EA_LOOP;
		}
		io->animlayer[layer].flags |= EA_FORCEPLAY;
	}
	
public:
	
	PlayAnimCommand() : Command("playanim") { }
	
	Result execute(Context & context) {
		
		INTERACTIVE_OBJ * iot = context.getIO();
		long nu = 0;
		bool loop = false;
		bool nointerpol = false;
		bool execute = false;
		
		HandleFlags("123lnep") {
			if(flg & flag('1')) {
				nu = 0;
			}
			if(flg & flag('2')) {
				nu = 1;
			}
			if(flg & flag('3')) {
				nu = 2;
			}
			loop = (flg & flag('l'));
			nointerpol = (flg & flag('n'));
			execute = (flg & flag('e'));
			if(flg & flag('p')) {
				iot = inter.iobj[0];
				iot->move = iot->lastmove = Vec3f::ZERO;
			}
		}
		
		string anim = context.getLowercase();
		
		DebugScript(' ' << options << ' ' << anim);
		
		if(!iot) {
			ScriptWarning << "must either use -p or use with IO";
			return Failed;
		}
		
		if(anim == "none") {
			iot->animlayer[nu].cur_anim = NULL;
			iot->animlayer[nu].next_anim = NULL;
			return Success;
		}
		
		AnimationNumber num = GetNumAnim(anim);
		if(num == ANIM_NONE) {
			ScriptWarning << "unknown anim: " << anim;
			return Failed;
		}
		
		if(!iot->anims[num]) {
			return Success;
		}
		
		iot->ioflags |= IO_NO_PHYSICS_INTERPOL;
		SetNextAnim(iot, iot->anims[num], nu, loop, nointerpol);
		
		if(!loop) {
			CheckSetAnimOutOfTreatZone(iot, nu);
		}
		
		if(iot == inter.iobj[0]) {
			iot->animlayer[nu].flags &= ~EA_STATICANIM;
		}
		
		if(execute) {
			
			string timername = "anim_" + ARX_SCRIPT_Timer_GetDefaultName();
			long num2 = ARX_SCRIPT_Timer_GetFree();
			if(num2 < 0) {
				ScriptError << "no free timer";
				return Failed;
			}
			
			size_t pos = context.skipCommand();
			if(pos != (size_t)-1) {
				scr_timer[num2].reset();
				ActiveTimers++;
				scr_timer[num2].es = context.getScript();
				scr_timer[num2].exist = 1;
				scr_timer[num2].io = context.getIO();
				scr_timer[num2].msecs = max(iot->anims[num]->anims[iot->animlayer[nu].altidx_cur]->anim_time, 1000.f);
				scr_timer[num2].name = timername;
				scr_timer[num2].pos = pos;
				scr_timer[num2].tim = ARXTimeUL();
				scr_timer[num2].times = 1;
				scr_timer[num2].longinfo = 0;
			}
		}
		
		return Success;
	}
	
};

class LoadAnimCommand : public Command {
	
public:
	
	LoadAnimCommand() : Command("loadanim") { }
	
	Result execute(Context & context) {
		
		INTERACTIVE_OBJ * iot = context.getIO();
		
		HandleFlags("p") {
			if(flg & flag('p')) {
				iot = inter.iobj[0];
			}
		}
		
		string anim = context.getLowercase();
		
		string file = loadPath(context.getLowercase());
		
		DebugScript(' ' << options << ' ' << anim << ' ' << file);
		
		
		if(!iot) {
			ScriptWarning << "must either use -p or use with IO";
			return Failed;
		}
		
		AnimationNumber num = GetNumAnim(anim);
		if(num == ANIM_NONE) {
			ScriptWarning << "unknown anim: " << anim;
			return Failed;
		}
		
		if(iot->anims[num]) {
			ReleaseAnimFromIO(iot, num);
		}
		
		if(file == "none") {
			iot->anims[num] = NULL;
			return Success;
		}
		
		if(iot == inter.iobj[0] || (iot->ioflags & IO_NPC)) {
			file = "graph\\obj3d\\anims\\npc\\" + file;
		} else {
			file = "graph\\obj3d\\anims\\fix_inter\\" + file;
		}
		SetExt(file, ".tea");
		string path;
		File_Standardize(file, path);
		
		iot->anims[num] = EERIE_ANIMMANAGER_Load_NoWarning(path);
		
		if(!iot->anims[num]) {
			ScriptWarning << "animation not found: " << path;
			return Failed;
		}
		
		return Success;
	}
	
};

class MoveCommand : public Command {
	
public:
	
	MoveCommand() : Command("move", ANY_IO) { }
	
	Result execute(Context & context) {
		
		float dx = context.getFloat();
		float dy = context.getFloat();
		float dz = context.getFloat();
		
		DebugScript(' ' << dx << ' ' << dy << ' ' << dz);
		
		context.getIO()->pos += Vec3f(dx, dy, dz);
		
		return Success;
	}
	
};

class UsePathCommand : public Command {
	
public:
	
	UsePathCommand() : Command("usepath", ANY_IO) { }
	
	Result execute(Context & context) {
		
		string type = context.getLowercase();
		
		DebugScript(' ' << type);
		
		ARX_USE_PATH * aup = context.getIO()->usepath;
		if(!aup) {
			ScriptWarning << "no path set";
			return Failed;
		}
		
		if(type == "b") {
			aup->aupflags &= ~ARX_USEPATH_PAUSE;
			aup->aupflags &= ~ARX_USEPATH_FORWARD;
			aup->aupflags |= ARX_USEPATH_BACKWARD;
		} else if(type == "f") {
			aup->aupflags &= ~ARX_USEPATH_PAUSE;
			aup->aupflags |= ARX_USEPATH_FORWARD;
			aup->aupflags &= ~ARX_USEPATH_BACKWARD;
		} else if(type == "p") {
			aup->aupflags |= ARX_USEPATH_PAUSE;
			aup->aupflags &= ~ARX_USEPATH_FORWARD;
			aup->aupflags &= ~ARX_USEPATH_BACKWARD;
		} else {
			ScriptWarning << "unknown usepath type: " << type;
			return Failed;
		}
		
		return Success;
	}
	
};

class UnsetControlledZoneCommand : public Command {
	
public:
	
	UnsetControlledZoneCommand() : Command("unsetcontrolledzone") { }
	
	Result execute(Context & context) {
		
		string zone = context.getLowercase();
		
		DebugScript(' ' << zone);
		
		ARX_PATH * ap = ARX_PATH_GetAddressByName(zone);
		if(ap) {
			ap->controled[0] = 0;
		}
		
		return Success;
	}
	
};

class SetPathCommand : public Command {
	
public:
	
	SetPathCommand() : Command("setpath", ANY_IO) { }
	
	Result execute(Context & context) {
		
		bool wormspecific = false;
		bool followdir = false;
		HandleFlags("wf") {
			wormspecific = (flg & flag('w'));
			followdir = (flg & flag('f'));
		}
		
		string name = context.getLowercase();
		
		DebugScript(' ' << options << ' ' << name);
		
		INTERACTIVE_OBJ * io = context.getIO();
		if(name == "none") {
			if(io->usepath) {
				free(io->usepath), io->usepath = NULL;
			}
		} else {
			
			ARX_PATH * ap = ARX_PATH_GetAddressByName(name);
			if(!ap) {
				ScriptWarning << "unknown path: " << name;
				return Failed;
			}
			
			if(io->usepath != NULL) {
				free(io->usepath), io->usepath = NULL;
			}
			
			ARX_USE_PATH * aup = (ARX_USE_PATH *)malloc(sizeof(ARX_USE_PATH));
			aup->_starttime = aup->_curtime = ARXTime;
			aup->aupflags = ARX_USEPATH_FORWARD;
			if(wormspecific) {
				aup->aupflags |= ARX_USEPATH_WORM_SPECIFIC | ARX_USEPATH_FLAG_ADDSTARTPOS;
			}
			if(followdir) {
				aup->aupflags |= ARX_USEPATH_FOLLOW_DIRECTION;
			}
			aup->initpos = io->initpos;
			aup->lastWP = -1;
			aup->path = ap;
			io->usepath = aup;
		}
		
		return Success;
	}
	
};

class SetControlledZoneCommand : public Command {
	
public:
	
	SetControlledZoneCommand() : Command("setcontrolledzone") { }
	
	Result execute(Context & context) {
		
		string name = context.getLowercase();
		
		DebugScript(' ' << name);
		
		ARX_PATH * ap = ARX_PATH_GetAddressByName(name);
		if(!ap) {
			ScriptWarning << "unknown zone: " << name;
			return Failed;
		}
		
		strcpy(ap->controled, context.getIO()->long_name().c_str());
		
		return Success;
	}
	
};

}

void setupScriptedAnimation() {
	
	ScriptEvent::registerCommand(new RotateCommand);
	ScriptEvent::registerCommand(new ForceAnimCommand);
	ScriptEvent::registerCommand(new ForceAngleCommand);
	ScriptEvent::registerCommand(new PlayAnimCommand);
	ScriptEvent::registerCommand(new LoadAnimCommand);
	ScriptEvent::registerCommand(new MoveCommand);
	ScriptEvent::registerCommand(new SetControlledZoneCommand);
	ScriptEvent::registerCommand(new SetPathCommand);
	ScriptEvent::registerCommand(new UsePathCommand);
	ScriptEvent::registerCommand(new UnsetControlledZoneCommand);
	
}

} // namespace script
