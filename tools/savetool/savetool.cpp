
#include <string>
#include <cassert>
#include <vector>
#include <cstdio>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <cstdlib>
using std::string;
using std::vector;
using std::printf;
using std::stringstream;
using std::setfill;
using std::setw;
using std::memcpy;
using std::free;

#include "io/SaveBlock.h"
#include "io/Filesystem.h"
#include "io/FilePath.h"
#include "platform/String.h"

// TODO use structs form SaveFormat.h, but that pulls in d3d dependencies

#pragma pack(push,1)

const size_t SIZE_ID = 64;

struct SavedVec3 {
	f32 x;
	f32 y;
	f32 z;
};

struct SavedCylinder {
	SavedVec3 origin;
	f32 radius;
	f32 height;
};

struct SavedIOPhysics {
	SavedCylinder cyl;
	SavedVec3 startpos;
	SavedVec3 targetpos;
	SavedVec3 velocity;
	SavedVec3 forces;
};

struct SavedAnimUse {
	s32 next_anim;
	s32 cur_anim;
	s16 altidx_next; // idx to alternate anims...
	s16 altidx_cur; // idx to alternate anims...
	s32 ctime;
	u32 flags;
	u32 nextflags;
	s32 lastframe;
	f32 pour;
	s32 fr;
};

struct SavedSpellcastData {
	
	static const size_t SYMB_SIZE = 4;
	
	s32 castingspell; // spell being casted...
	u8 symb[SYMB_SIZE]; // symbols to draw before casting...
	s16 spell_flags;
	s16 spell_level;
	s32 target;
	s32 duration;
	
};

struct SavedModInfo {
	s32 link_origin;
	SavedVec3 link_position;
	SavedVec3 scale;
	SavedVec3 rot;
	u32 flags;
};

struct IO_LINKED_DATA {
	s32 lgroup; // linked to group n° if lgroup=-1 NOLINK
	s32 lidx;
	s32 lidx2;
	SavedModInfo modinfo;
	char linked_id[SIZE_ID];
};

struct SavedColor {
	f32 r;
	f32 g;
	f32 b;
};

struct SavedHalo {
	SavedColor color;
	f32 radius;
	u32 flags;
	s32 dynlight;
	SavedVec3 offset;
};

const size_t SAVED_MAX_ANIMS = 200;
const size_t SAVED_MAX_ANIM_LAYERS = 4;
const size_t MAX_LINKED_SAVE = 16;

struct ARX_CHANGELEVEL_IO_SAVE {
	
	s32 savesystem_type;
	s32 saveflags;
	f32 version;
	char filename[256];
	s32 ident;
	s32 ioflags;//type;
	SavedVec3 pos;
	SavedVec3 initpos;
	SavedVec3 lastpos;
	SavedVec3 move;
	SavedVec3 lastmove;
	SavedVec3 angle;
	SavedVec3 initangle;
	f32 scale;
	u32 savetime;
	f32 weight;
	
	char locname[64];
	u16 EditorFlags;
	u16 GameFlags;
	s32 material;
	s16 level;
	s16 truelevel;
	s32 nbtimers;
	// Script data
	s32 scriptload;
	s16 show;
	s16 collision;
	char mainevent[64];
	// Physics data
	SavedVec3 velocity;
	s32 stopped;
	SavedIOPhysics physics;
	f32 original_radius;
	f32 original_height;
	// Anims data
	char anims[SAVED_MAX_ANIMS][256];
	SavedAnimUse animlayer[SAVED_MAX_ANIM_LAYERS];
	// Target Info
	char id_targetinfo[SIZE_ID];
	s32 inventory;
	// Group Info
	s32 system_flags;
	f32 basespeed;
	f32 speed_modif;
	f32 frameloss;
	SavedSpellcastData spellcast_data;
	
	f32 rubber;
	f32 max_durability;
	f32 durability;
	s16 poisonous;
	s16 poisonous_count;
	
	s32 nb_linked;
	IO_LINKED_DATA linked_data[MAX_LINKED_SAVE];
	
	f32 head_rot;
	
	s16 damager_damages;
	s16 nb_iogroups;
	s32 damager_type;
	
	u32 type_flags;
	char stepmaterial[128];
	char armormaterial[128];
	char weaponmaterial[128];
	char strikespeech[128];
	s16 Tweak_nb;
	s16 padd;
	SavedHalo halo;
	char secretvalue;
	char paddd[3];
	char shop_category[128];
	f32 shop_multiply;
	s32 aflags;
	f32 ignition;
	char inventory_skin[128];
	// TO ADD:
	char usepath_name[SIZE_ID];
	u32 usepath_starttime;
	u32 usepath_curtime;
	s32 usepath_aupflags;
	SavedVec3 usepath_initpos;
	s32 usepath_lastWP;
	s32 padddd[64]; // new...
	
};

#define IO_ITEM    (1<<2)
#define IO_GOLD    (1<<10)
#define IO_MOVABLE (1<<15)

struct SavedGlobalMods {
	
	static const size_t AMBIANCE_SIZE = 128;
	
	s32 flags;
	SavedColor depthcolor;
	f32 zclip;
	char ambiance[AMBIANCE_SIZE];
	f32 ambiance_vol;
	f32 ambiance_maxvol;
	
};

struct ARX_CHANGELEVEL_INDEX {
	f32 version;
	u32 time;
	s32 nb_inter;
	s32 nb_paths;
	s32 nb_lights;
	s32 ambiances_data_size;
	SavedGlobalMods gmods_stacked;
	SavedGlobalMods gmods_current;
	SavedGlobalMods gmods_desired;
	s32 padding[256];
};

struct ARX_CHANGELEVEL_IO_INDEX {
	char filename[256];
	s32 ident;
	s32 num;
	s16 level;
	s16 truelevel;
	s32 unused;
	s32 padding[256];
};

const size_t SAVED_INVENTORY_X = 16;
const size_t SAVED_INVENTORY_Y = 3;
const size_t SAVED_MAX_MINIMAPS = 32;
const size_t SAVED_MAX_PRECAST = 3;
const size_t SAVED_MAX_EQUIPED = 12;

struct SavedPrecast {
	s32 typ;
	s32 level;
	u32 launch_time;
	s32 flags;
	s32 duration;
};

struct SavedMiniMap {
	
	static const size_t MAX_X = 50;
	static const size_t MAX_Z = 50;
	
	u32 padding;
	f32 offsetx;
	f32 offsety;
	f32 xratio;
	f32 yratio;
	f32 width;
	f32 height;
	u8 revealed[MAX_X][MAX_Z];
	
};

struct ARX_CHANGELEVEL_PLAYER {
	
	f32 version;
	s32 Current_Movement;
	s32 Last_Movement;
	s32 misc_flags;
	// Player Values
	f32 Attribute_Strength;
	f32 Attribute_Dexterity;
	f32 Attribute_Constitution;
	f32 Attribute_Mind;
	
	f32 Skill_Stealth;
	f32 Skill_Mecanism;
	f32 Skill_Intuition;
	
	f32 Skill_Etheral_Link;
	f32 Skill_Object_Knowledge;
	f32 Skill_Casting;
	
	f32 Skill_Projectile;
	f32 Skill_Close_Combat;
	f32 Skill_Defense;
	
	f32 Critical_Hit;
	s32 AimTime;
	f32 life;
	f32 maxlife;
	f32 mana;
	f32 maxmana;
	s32 level;
	s16 Attribute_Redistribute;
	s16 Skill_Redistribute;
	
	f32 armor_class;
	f32 resist_magic;
	f32 resist_poison;
	s32 xp;
	s32 skin;
	u32 rune_flags;
	f32 damages;
	f32 poison;
	f32 hunger;
	SavedVec3 pos;
	SavedVec3 angle;
	SavedVec3 size;
	
	char inzone[SIZE_ID];
	char rightIO[SIZE_ID];
	char leftIO[SIZE_ID];
	char equipsecondaryIO[SIZE_ID];
	char equipshieldIO[SIZE_ID];
	char curtorch[SIZE_ID];
	s32 gold;
	s32 falling;
	
	s16	doingmagic;
	s16	Interface;
	f32 invisibility;
	s8 useanim[36]; // padding
	SavedIOPhysics physics;
	// Jump Sub-data
	u32 jumpstarttime;
	s32 jumpphase;	// 0 no jump, 1 doing anticipation anim
	
	char id_inventory[3][SAVED_INVENTORY_X][SAVED_INVENTORY_Y][SIZE_ID];
	s32 inventory_show[3][SAVED_INVENTORY_X][SAVED_INVENTORY_Y];
	SavedMiniMap minimap[SAVED_MAX_MINIMAPS];
	char equiped[SAVED_MAX_EQUIPED][SIZE_ID];
	s32 nb_PlayerQuest;
	char anims[SAVED_MAX_ANIMS][256];
	s32 keyring_nb;
	s32 playerflags;
	char TELEPORT_TO_LEVEL[64];
	char TELEPORT_TO_POSITION[64];
	s32 TELEPORT_TO_ANGLE;
	s32 CHANGE_LEVEL_ICON;
	s16 bag;
	s16 sp_flags; // padding;
	SavedPrecast precast[SAVED_MAX_PRECAST];
	s32 Global_Magic_Mode;
	s32 Nb_Mapmarkers;
	SavedVec3 LAST_VALID_POS;
	s32 padding[253];
	
};

#pragma pack(pop)


static void print_help() {
	printf("usage: savetool <command> <savefile> [<options>...]\n"
	       "commands are:\n"
	       " - extract <savefile>\n"
	       " - add <savefile> [<files>...]\n"
				 " - fix <savefile>\n"
	);
}

static int main_extract(SaveBlock & save, int argc, char ** argv) {
	
	(void)argv;
	
	if(argc != 0) {
		print_help();
		return 1;
	}
	
	if(!save.BeginRead()) {
		return 2;
	}
	
	vector<string> files = save.getFiles();
	
	for(vector<string>::iterator file = files.begin(); file != files.end(); ++file) {
		
		size_t size;
		char * data = save.load(*file, size);
		if(!data) {
			printf("error loading %s from save\n", file->c_str());
			continue;
		}
		
		FileHandle h = FileOpenWrite(file->c_str());
		if(!h) {
			printf("error opening %s for writing\n", file->c_str());
			continue;
		}
		
		if((size_t)FileWrite(h, data, size) != size) {
			printf("error writing %s\n", file->c_str());
		}
		
		FileClose(h);
		
	}
	
	return 0;
}

static int main_add(SaveBlock & save, int argc, char ** argv) {
	
	if(!save.BeginSave()) {
		return 2;
	}
	
	for(int i = 0; i < argc; i++) {
		
		size_t size;
		char * data = (char*)FileLoadMalloc(argv[i], &size);
		
		if(!data) {
			printf("error loading %s\n", argv[i]);
		} else {
			
			string name = argv[i];
			size_t pos = name.find_last_of("/\\");
			if(pos != string::npos) {
				name = name.substr(pos + 1);
			}
			
			if(!save.save(name, data, size)) {
				printf("error writing %s to save\n", argv[i]);
			}
		}
		
	}
	
	save.flush();
	
	return 0;
}

static void fix_io(SaveBlock & save, const string & name) {
	
	if(!strcasecmp(name, "none") || name.empty()) {
		return;
	}
	
	ARX_CHANGELEVEL_IO_SAVE ais;
	
	string savefile = name + ".sav";
	
	size_t size = 0;
	char * dat = save.load(savefile, size);
	if(!dat) {
		return;
	}
	
	memcpy(&ais, dat, sizeof(ARX_CHANGELEVEL_IO_SAVE));
	
	bool changed = false;
	
	if(ais.ioflags & IO_ITEM) {
		
		string file = ais.filename;
		MakeUpcase(file);
		
		s32 flags = ais.ioflags;
		
		if(!specialstrcmp(GetName(file), "GOLD_COIN")) {
			RemoveName(file);
			file += "GOLD_COIN.asl";
			flags = ais.ioflags | IO_GOLD;
		}
		
		if(IsIn(file, "MOVABLE")) {
			flags = ais.ioflags | IO_MOVABLE;
		}
		
		if(flags != ais.ioflags) {
			printf(" - fixing %s: ioflags 0x%x -> 0x%x\n", name.c_str(), ais.ioflags, flags);
			ais.ioflags = flags;
			changed = true;
		}
		
	}
	
	if(changed) {
		memcpy(dat, &ais, sizeof(ARX_CHANGELEVEL_IO_SAVE));
		if(!save.save(savefile, dat, size)) {
			printf("error saving %s\n", savefile.c_str());
		}
	}
	
	free(dat);
	
}

static void fix_player(SaveBlock & save) {
	
	printf("player\n");
	
	const string & loadfile = "player.sav";
	
	size_t size;
	char * dat = save.load(loadfile, size);
	if(!dat) {
		return;
	}
	
	ARX_CHANGELEVEL_PLAYER asp;
	memcpy(&asp, dat, sizeof(ARX_CHANGELEVEL_PLAYER));
	
	for(size_t iNbBag = 0; iNbBag < 3; iNbBag++) {
		for(size_t m = 0; m < SAVED_INVENTORY_Y; m++) {
			for(size_t n = 0; n < SAVED_INVENTORY_X; n++) {
				fix_io(save, asp.id_inventory[iNbBag][n][m]);
			}
		}
	}
	
	fix_io(save, asp.equipsecondaryIO);
	fix_io(save, asp.equipshieldIO);
	fix_io(save, asp.leftIO);
	fix_io(save, asp.rightIO);
	fix_io(save, asp.curtorch);
	
	for(size_t k = 0; k < SAVED_MAX_EQUIPED; k++) {
		fix_io(save, asp.equiped[k]);
	}
	
}

static void fix_level(SaveBlock & save, long num) {
	
	stringstream ss;
	ss << "lvl" << setfill('0') << setw(3) << num << ".sav";
	
	if(!save.hasFile(ss.str())) {
		return;
	}
	
	size_t size;
	char * dat = save.load(ss.str(), size);
	if(!dat) {
		return;
	}
	
	printf("level %ld\n", num);
	
	size_t pos = 0;
	
	ARX_CHANGELEVEL_INDEX asi;
	memcpy(&asi, dat + pos, sizeof(ARX_CHANGELEVEL_INDEX));
	pos += sizeof(ARX_CHANGELEVEL_INDEX);
	
	ARX_CHANGELEVEL_IO_INDEX * idx_io = new ARX_CHANGELEVEL_IO_INDEX[asi.nb_inter];
	memcpy(idx_io, dat + pos, sizeof(ARX_CHANGELEVEL_IO_INDEX) * asi.nb_inter);
	pos += sizeof(ARX_CHANGELEVEL_IO_INDEX) * asi.nb_inter;
	
	free(dat);
	
	for(long i = 0; i < asi.nb_inter; i++) {
		stringstream name;
		name << GetName(idx_io[i].filename) << "_" << setw(4) << setfill('0') << idx_io[i].ident;
		fix_io(save, name.str());
		fix_io(save, name.str());
	}
	
	delete[] idx_io;
	
}

static int main_fix(SaveBlock & save, int argc, char ** argv) {
	
	(void)argv;
	
	if(argc != 0) {
		print_help();
		return 1;
	}
	
	if(!save.BeginSave()) {
		return 2;
	}
	
	fix_player(save);
	
	const long MAX_LEVEL = 24;
	for(long i = 0; i <= MAX_LEVEL; i++) {
		fix_level(save, i);
	}
	
	save.flush();
	
	return 0;
}

int main(int argc, char ** argv) {
	
	if(argc < 3) {
		print_help();
		return 1;
	}
	
	string command = argv[1];
	SaveBlock save(argv[2]);
	
	argc -= 3;
	argv += 3;
	
	if(command == "e" || command == "extract") {
		return main_extract(save, argc, argv);
	} else if(command == "a" || command == "add") {
		return main_add(save, argc, argv);
	} else if(command == "f" || command == "fix") {
		return main_fix(save, argc, argv);
	} else {
		print_help();
		return 1;
	}
	
}
