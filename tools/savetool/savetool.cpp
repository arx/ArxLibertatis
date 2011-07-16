
#include <string>
#include <cassert>
#include <vector>
#include <cstdio>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <map>
#include <set>
using std::string;
using std::vector;
using std::printf;
using std::stringstream;
using std::setfill;
using std::setw;
using std::memcpy;
using std::free;
using std::map;
using std::set;
using std::transform;

#include "io/SaveBlock.h"
#include "io/Filesystem.h"
#include "io/FilePath.h"
#include "io/PakReader.h"
#include "platform/String.h"
#include "platform/Platform.h"

// TODO use structs form SaveFormat.h, but that pulls in d3d dependencies

#pragma pack(push,1)

const size_t SIZE_ID = 64;
const s32 SYSTEM_FLAG_INVENTORY = 2;
const size_t SAVED_MAX_STACKED_BEHAVIOR = 5;

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

struct ARX_CHANGELEVEL_INVENTORY_DATA_SAVE {
	
	char io[SIZE_ID];
	s32 sizex;
	s32 sizey;
	char slot_io[20][20][SIZE_ID];
	s32 slot_show[20][20];
	char initio[20][20][SIZE_ID];
	/// limit...
	char weapon[SIZE_ID];
	char targetinfo[SIZE_ID];
	char linked_id[MAX_LINKED_SAVE][SIZE_ID];
	char stackedtarget[SAVED_MAX_STACKED_BEHAVIOR][SIZE_ID];
	
};

struct ARX_CHANGELEVEL_TIMERS_SAVE {
	
	char name[SIZE_ID];
	s32 times;
	s32 msecs;
	s32 pos;
	s32 tim;
	s32 script; // 0 = global ** 1 = local
	s32 longinfo;
	s32 flags;
	
};

struct ARX_CHANGELEVEL_SCRIPT_SAVE {
	s32 nblvar;
	u32 lastcall;
	s32 allowevents;
};

struct ARX_CHANGELEVEL_VARIABLE_SAVE {
	s32 type;
	f32 fval;
	char name[SIZE_ID];
};

#define TYPE_L_TEXT  2
#define TYPE_L_LONG  8
#define TYPE_L_FLOAT 32

struct SavedBehaviour {
	
	s32 exist;
	u32 behavior;
	f32 behavior_param;
	s32 tactics; // 0=none ; 1=side ; 2=side+back
	s32 target;
	s32 movemode;
	SavedAnimUse animlayer[SAVED_MAX_ANIM_LAYERS];
	
};

struct SavedPathfindTarget {
	
	u32 padding[4];
	s32 truetarget;
	
};

const size_t SAVED_MAX_EXTRA_ROTATE = 4;

struct SavedExtraRotate {
	
	s32 flags;
	s16 group_number[SAVED_MAX_EXTRA_ROTATE];
	SavedVec3 group_rotate[SAVED_MAX_EXTRA_ROTATE];
	
};

struct ARX_CHANGELEVEL_NPC_IO_SAVE {
	
	f32 maxlife;
	f32 life;
	f32 maxmana;
	f32 mana;
	s32 reachedtarget;
	char id_weapon[SIZE_ID];
	s32 detect;
	s32 movemode;
	f32 armor_class;
	f32 absorb;
	f32 damages;
	f32 tohit;
	f32 aimtime;
	u32 behavior;
	f32 behavior_param;
	s32 tactics;
	s32 xpvalue;
	s32 cut;
	f32 moveproblem;
	s32 weapontype;
	s32 weaponinhand;
	s32 fightdecision;
	char weaponname[256];
	f32 look_around_inc;
	u32 collid_time;
	s32 collid_state;
	f32 speakpitch;
	f32 lastmouth;
	SavedBehaviour stacked[SAVED_MAX_STACKED_BEHAVIOR];
	char weapon[SIZE_ID];
	
	f32 critical;
	f32 reach;
	f32 backstab_skill;
	f32 poisonned;
	u8 resist_poison;
	u8 resist_magic;
	u8 resist_fire;
	u8 padd;
	
	s16 strike_time;
	s16 walk_start_time;
	s32 aiming_start;
	s32 npcflags;
	SavedPathfindTarget pathfind;
	SavedExtraRotate ex_rotate;
	u32 blood_color;
	char stackedtarget[SAVED_MAX_STACKED_BEHAVIOR][SIZE_ID];
	f32 fDetect;
	s16 cuts;
	s16 spadd;
	s32 paddd[63]; // new...
	
};

#define TYPE_NPC	1
#define TYPE_ITEM	2
#define TYPE_FIX	3
#define TYPE_CAMERA	4
#define TYPE_MARKER	5

const size_t SAVED_IO_EQUIPITEM_ELEMENT_Number = 29;

struct SavedEquipItemElement {
	
	f32 value;
	s16 flags;
	s16 special;
	
};

struct SavedEquipItem {
	
	SavedEquipItemElement elements[SAVED_IO_EQUIPITEM_ELEMENT_Number];
	
};

struct ARX_CHANGELEVEL_ITEM_IO_SAVE {
	s32 price;
	s16 maxcount;
	s16 count;
	char food_value;
	char stealvalue;
	s16 playerstacksize;
	s16 LightValue;
	SavedEquipItem equipitem;
	s32 padd[64]; // new...
};

struct ARX_CHANGELEVEL_FIX_IO_SAVE {
	char trapvalue;
	char padd[3];
	s32 paddd[64]; // new...
};

struct ARX_CHANGELEVEL_MARKER_IO_SAVE {
	s32 dummy;
};

struct SavedRect {
	
	s32 left;
	s32 top;
	s32 right;
	s32 bottom;
	
};

struct SavedMatrix {
	
	f32 _11, _12, _13, _14;
	f32 _21, _22, _23, _24;
	f32 _31, _32, _33, _34;
	f32 _41, _42, _43, _44;
	
};

struct SavedTransform {
	
	f32 posx;
	f32 posy;
	f32 posz;
	f32 ycos;
	f32 ysin;
	f32 xsin;
	f32 xcos;
	f32 use_focal;
	f32 xmod;
	f32 ymod;
	f32 zmod;
	
};

struct SavedCamera {
	
	SavedTransform transform;
	SavedVec3 pos;
	f32 Ycos;
	f32 Ysin;
	f32 Xcos;
	f32 Xsin;
	f32 Zcos;
	f32 Zsin;
	f32 focal;
	f32 use_focal;
	f32 Zmul;
	f32 posleft;
	f32 postop;
	
	f32 xmod;
	f32 ymod;
	SavedMatrix matrix;
	SavedVec3 angle;
	
	SavedVec3 d_pos;
	SavedVec3 d_angle;
	SavedVec3 lasttarget;
	SavedVec3 lastpos;
	SavedVec3 translatetarget;
	s32 lastinfovalid;
	SavedVec3 norm;
	SavedColor fadecolor;
	SavedRect clip;
	f32 clipz0;
	f32 clipz1;
	s32 centerx;
	s32 centery;
	
	f32 smoothing;
	f32 AddX;
	f32 AddY;
	s32 Xsnap;
	s32 Zsnap;
	f32 Zdiv;
	
	s32 clip3D;
	s32 type;
	s32 bkgcolor;
	s32 nbdrawn;
	f32 cdepth;
	
	SavedVec3 size;
	
};

struct ARX_CHANGELEVEL_CAMERA_IO_SAVE {
	SavedCamera cam;
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
			printf("error writing to %s\n", file->c_str());
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

typedef map<string, string> Idents; // ident -> where
typedef map<string, long> Remap; // ident -> newIdent

static string makeIdent(const string & file, long ident) {
	stringstream name;
	name << file << "_" << setw(4) << setfill('0') << ident;
	string namestr = name.str();
	transform(namestr.begin(), namestr.end(), namestr.begin(), ::toupper);
	return namestr;
}

static bool fix_ident(SaveBlock & save, char name[SIZE_ID], Idents & idents, const string & where, Remap & remap);

static bool fix_iodata(SaveBlock & save, Idents & idents, ARX_CHANGELEVEL_IO_SAVE & ais, char * dat, const string & where, Remap & remap) {
	
	size_t pos = 0;
	
	bool ioChanged = false;
	
	ioChanged |= fix_ident(save, ais.id_targetinfo, idents, where + ".id_targetinfo", remap);
	for(long i = 0; i < ais.nb_linked; i++) {
		stringstream where2;
		where2 << where << ".linked_data[" << i << "].linked_id";
		ioChanged |= fix_ident(save, ais.linked_data[i].linked_id, idents, where2.str(), remap);
	}
	if(ioChanged) {
		memcpy(dat + pos, &ais, sizeof(ARX_CHANGELEVEL_IO_SAVE));
	}
	pos += sizeof(ARX_CHANGELEVEL_IO_SAVE);
	
	pos += ais.nbtimers * sizeof(ARX_CHANGELEVEL_TIMERS_SAVE);
	
	ARX_CHANGELEVEL_SCRIPT_SAVE ass;
	
	memcpy(&ass, dat + pos, sizeof(ARX_CHANGELEVEL_SCRIPT_SAVE));
	pos += sizeof(ARX_CHANGELEVEL_SCRIPT_SAVE);
	for(int i = 0; i < ass.nblvar; i++) {
		ARX_CHANGELEVEL_VARIABLE_SAVE avs;
		memcpy(&avs, dat + pos, sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE));
		pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);
		if(avs.type == TYPE_L_TEXT || (avs.type != TYPE_L_LONG && avs.type != TYPE_L_FLOAT && (avs.name[0] == '$' || avs.name[0] == '\xA3'))) {
			pos += (size_t)avs.fval;
		}
	}
	
	memcpy(&ass, dat + pos, sizeof(ARX_CHANGELEVEL_SCRIPT_SAVE));
	pos += sizeof(ARX_CHANGELEVEL_SCRIPT_SAVE);
	for(int i = 0; i < ass.nblvar; i++) {
		ARX_CHANGELEVEL_VARIABLE_SAVE avs;
		memcpy(&avs, dat + pos, sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE));
		pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);
		if(avs.type == TYPE_L_TEXT || (avs.type != TYPE_L_LONG && avs.type != TYPE_L_FLOAT && (avs.name[0] == '$' || avs.name[0] == '\xA3'))) {
			pos += (size_t)avs.fval;
		}
	}
	
	bool specificsChanged = false;
	
	switch(ais.savesystem_type) {
		case TYPE_NPC:
			ARX_CHANGELEVEL_NPC_IO_SAVE anis;
			memcpy(&anis, dat + pos, sizeof(ARX_CHANGELEVEL_NPC_IO_SAVE));
			specificsChanged |= fix_ident(save, anis.id_weapon, idents, where + ".npc.id_weapon", remap);
			specificsChanged |= fix_ident(save, anis.weapon, idents, where + ".npc.weapon", remap);
			for(size_t i = 0; i < SAVED_MAX_STACKED_BEHAVIOR; i++) {
				stringstream where2;
				where2 << where << ".npc.stackedtarget[" << i << "]";
				specificsChanged |= fix_ident(save, anis.stackedtarget[i], idents, where2.str(), remap);
			}
			if(specificsChanged) {
				memcpy(dat + pos, &anis, sizeof(ARX_CHANGELEVEL_NPC_IO_SAVE));
			}
			pos += sizeof(ARX_CHANGELEVEL_NPC_IO_SAVE);
			break;
		case TYPE_ITEM:
			pos += sizeof(ARX_CHANGELEVEL_ITEM_IO_SAVE);
			break;
		case TYPE_FIX:
			pos += sizeof(ARX_CHANGELEVEL_FIX_IO_SAVE);
			break;
		case TYPE_CAMERA:
			pos += sizeof(ARX_CHANGELEVEL_CAMERA_IO_SAVE);
			break;
		case TYPE_MARKER:
			pos += sizeof(ARX_CHANGELEVEL_MARKER_IO_SAVE);
			break;
	}
	
	// The items should have been in the level index but might not be if an item in the player inventory has the same ident.
	
	bool invChanged = false;
	
	if(ais.system_flags & SYSTEM_FLAG_INVENTORY) {
		
		ARX_CHANGELEVEL_INVENTORY_DATA_SAVE aids;
		memcpy(&aids, dat + pos, sizeof(ARX_CHANGELEVEL_INVENTORY_DATA_SAVE));
		
		invChanged |= fix_ident(save, aids.io, idents, where + ".inventory.io", remap);
		for(long m = 0; m < aids.sizex; m++) {
			for(long n = 0; n < aids.sizey; n++) {
				stringstream where2;
				where2 << where << ".inventory.slot_io[" << m << "][" << n << "]";
				invChanged |= fix_ident(save, aids.slot_io[m][n], idents, where2.str(), remap);
			}
		}
		invChanged |= fix_ident(save, aids.weapon, idents, where + ".inventory.weapon", remap);
		invChanged |= fix_ident(save, aids.targetinfo, idents, where + ".inventory.targetinfo", remap);
		for(long i = 0; i < ais.nb_linked; i++) {
			stringstream where2;
			where2 << where << ".inventory.linked_id[" << i << "]";
			invChanged |= fix_ident(save, aids.linked_id[i], idents, where2.str(), remap);
		}
		for(size_t i = 0; i < SAVED_MAX_STACKED_BEHAVIOR; i++) {
			stringstream where2;
			where2 << where << ".inventory.stackedtarget[" << i << "]";
			invChanged |= fix_ident(save, aids.stackedtarget[i], idents, where2.str(), remap);
		}
		if(invChanged) {
			memcpy(dat + pos, &aids, sizeof(ARX_CHANGELEVEL_INVENTORY_DATA_SAVE));
		}
		pos +=  sizeof(ARX_CHANGELEVEL_INVENTORY_DATA_SAVE);
		
	}
	
	return ioChanged || specificsChanged || invChanged;
}

static long copy_io(SaveBlock & save, const string & name, Idents & idents, const string & where, char * dat, size_t size) {
	
	ARX_CHANGELEVEL_IO_SAVE ais;
	memcpy(&ais, dat, sizeof(ARX_CHANGELEVEL_IO_SAVE));
	
	size_t pos = name.find_last_of('_');
	
	string fname = name.substr(0, pos);
	
	long i = 1;
	string ident;
	for(; i < 10000; i++) {
		
		ident = makeIdent(fname, i);
		
		if(save.hasFile(ident + ".sav")) {
			continue;
		}
		
		string file = ais.filename;
		RemoveName(file);
		file += ident;
		
		if(resources->getDirectory(file)) {
			continue;
		}
		
		break;
	}
	
	ais.ident = i;
	memcpy(dat, &ais, sizeof(ARX_CHANGELEVEL_IO_SAVE));
	
	Remap remap;
	remap[name] = i;
	idents[ident] = where;
	
	fix_iodata(save, idents, ais, dat, where + ":" + ident, remap);
	
	save.save(ident + ".sav", dat, size);
	
	return i;
}

static long fix_io(SaveBlock & save, const string & name, Idents & idents, const string & where, Remap & remap) {
	
	if(!strcasecmp(name, "none") || name.empty()) {
		remap[name] = 0;
		return 0;
	}
	
	string savefile = name + ".sav";
	
	size_t size = 0;
	char * dat = save.load(savefile, size);
	if(!dat) {
		remap[name] = 0;
		return 0;
	}
	
	Idents::iterator it = idents.find(name);
	if(it != idents.end()) {
		printf("duplicate ident %s detected: in %s and %s\n", name.c_str(), it->second.c_str(), where.c_str());
		// we already fixed this!
		long newIdent = copy_io(save, name, idents, where, dat, size);
		printf(" -> copied %s as %ld for %s\n", name.c_str(), newIdent, where.c_str());
		free(dat);
		remap[name] = newIdent;
		return newIdent;
	} else {
		idents[name] = where;
		remap[name] = 0;
	}
	
	ARX_CHANGELEVEL_IO_SAVE ais;
	memcpy(&ais, dat, sizeof(ARX_CHANGELEVEL_IO_SAVE));
	
	bool changed = false;
	
	if(ais.ioflags & IO_ITEM) {
		
		string file = toLowercase(ais.filename);
		
		s32 flags = ais.ioflags;
		
		if(!specialstrcmp(GetName(file), "gold_coin")) {
			RemoveName(file);
			file += "gold_coin.asl";
			flags = ais.ioflags | IO_GOLD;
		}
		
		if(IsIn(file, "movable")) {
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
	}
	
	changed |= fix_iodata(save, idents, ais, dat, where + ":" + name, remap);
	
	if(changed) {
		if(!save.save(savefile, dat, size)) {
			printf("error saving %s\n", savefile.c_str());
		}
	}
	
	free(dat);
	
	return 0;
}

static bool patch_ident(char name[SIZE_ID], long newIdent, const string & where) {
	
	if(newIdent <= 0) {
		return false;
	}
	
	printf("fixing ident in %s: %s -> %ld\n", where.c_str(), name, newIdent);
	
	string namestr = name;
	
	size_t pos = namestr.find_last_of('_');
	
	strcpy(name, makeIdent(namestr.substr(0, pos), newIdent).c_str());
	
	return true;
}

static bool fix_ident(SaveBlock & save, char name[SIZE_ID], Idents & idents, const string & where, Remap & remap) {
	
	if(name[0] == '\0' || !strcasecmp(name, "none") || !strcasecmp(name, "player") || !strcasecmp(name, "self")) {
		return false;
	}
	
	string namestr = name;
	transform(namestr.begin(), namestr.end(), namestr.begin(), ::toupper);
	
	Remap::const_iterator it = remap.find(namestr);
	if(it != remap.end()) {
		return patch_ident(name, it->second, where);
	}
	
	long newIdent = fix_io(save, namestr, idents, where, remap);
	
	return patch_ident(name, newIdent, where);
}

static void fix_player(SaveBlock & save, Idents & idents) {
	
	printf("player\n");
	
	const string & loadfile = "player.sav";
	
	size_t size;
	char * dat = save.load(loadfile, size);
	if(!dat) {
		return;
	}
	
	bool changed = false;
	
	Remap remap;
	
	ARX_CHANGELEVEL_PLAYER asp;
	memcpy(&asp, dat, sizeof(ARX_CHANGELEVEL_PLAYER));
	
	for(size_t iNbBag = 0; iNbBag < 3; iNbBag++) {
		for(size_t m = 0; m < SAVED_INVENTORY_Y; m++) {
			for(size_t n = 0; n < SAVED_INVENTORY_X; n++) {
				stringstream where;
				where << "player.id_inventory[" << iNbBag << "][" << n << "][" << m << "]"; 
				changed |= fix_ident(save, asp.id_inventory[iNbBag][n][m], idents, where.str(), remap);
			}
		}
	}
	
	changed |= fix_ident(save, asp.inzone, idents, "player.inzone", remap);
	changed |= fix_ident(save, asp.rightIO, idents, "player.rightIO", remap);
	changed |= fix_ident(save, asp.leftIO, idents, "player.leftIO", remap);
	changed |= fix_ident(save, asp.equipsecondaryIO, idents, "player.equipsecondaryIO", remap);
	changed |= fix_ident(save, asp.equipshieldIO, idents, "player.equipshieldIO", remap);
	changed |= fix_ident(save, asp.curtorch, idents, "player.torch", remap);
	
	for(size_t k = 0; k < SAVED_MAX_EQUIPED; k++) {
		stringstream where;
		where << "player.equiped[" << k << "]"; 
		changed |= fix_ident(save, asp.equiped[k], idents, where.str(), remap);
	}
	
	if(changed) {
		memcpy(dat, &asp, sizeof(ARX_CHANGELEVEL_PLAYER));
		save.save(loadfile, dat, size);
	}
	
	free(dat);
	
}

static void fix_level(SaveBlock & save, long num, Idents & idents) {
	
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
	
	Remap remap;
	
	bool changed = false;
	
	for(long i = 0; i < asi.nb_inter; i++) {
		long res;
		string ident = makeIdent(GetName(idx_io[i].filename), idx_io[i].ident);
		Remap::const_iterator it = remap.find(ident);
		stringstream where;
		where << "level" << num << "[" << i << "]";
		if(it != remap.end()) {
			res = it->second;
		} else {
			res = fix_io(save, ident, idents, where.str(), remap);
		}
		if(res != 0) {
			printf("fixing ident in %s: %s -> %ld\n", where.str().c_str(), ident.c_str(), res);
			idx_io[i].ident = res;
			changed = true;
			memcpy(dat + sizeof(ARX_CHANGELEVEL_INDEX) + (sizeof(ARX_CHANGELEVEL_IO_INDEX) * i), &idx_io[i], sizeof(ARX_CHANGELEVEL_IO_INDEX));
		}
	}
	
	if(changed) {
		save.save(ss.str(), dat, size);
	}
	
	free(dat);
	
	delete[] idx_io;
	
}

static int main_fix(SaveBlock & save, int argc, char ** argv) {
	
	(void)argv;
	
	if(argc != 0) {
		print_help();
		return 1;
	}
	
	resources = new PakReader();
	
	if(!resources->addArchive("data.pak") || !resources->addArchive("data2.pak") || !resources->addFiles(".")) {
		printf("could not open pak files, run 'savetool fix' from the game directory\n");
		return 3;
	}
	
	if(!save.BeginSave()) {
		return 2;
	}
	
	Idents idents;
	
	fix_player(save, idents);
	
	const long MAX_LEVEL = 24;
	for(long i = 0; i <= MAX_LEVEL; i++) {
		fix_level(save, i, idents);
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
