
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

#include "Configure.h"
#ifdef BUILD_EDITOR
#undef BUILD_EDITOR
#endif

#include "io/SaveBlock.h"
#include "io/FilePath.h"
#include "io/PakReader.h"
#include "io/Filesystem.h"
#include "platform/String.h"
#include "platform/Platform.h"
#include "scene/SaveFormat.h"

using std::string;
using std::vector;
using std::printf;
using std::stringstream;
using std::setfill;
using std::setw;
using std::free;
using std::map;
using std::set;
using std::transform;

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
	
	if(!save.open()) {
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
		
		std::ofstream h(file->c_str(), std::ios_base::out | std::ios_base::binary);
		if(!h.is_open()) {
			printf("error opening %s for writing\n", file->c_str());
			continue;
		}
		
		if(h.write(data, size).fail()) {
			printf("error writing to %s\n", file->c_str());
		}
		
	}
	
	return 0;
}

static int main_add(SaveBlock & save, int argc, char ** argv) {
	
	if(!save.open(true)) {
		return 2;
	}
	
	for(int i = 0; i < argc; i++) {
		
		size_t size;
		char * data = read_file(argv[i], size);
		
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
			
			delete[] data;
		}
		
	}
	
	save.flush("pld");
	
	return 0;
}

typedef map<string, string> Idents; // ident -> where
typedef map<string, long> Remap; // ident -> newIdent

static string makeIdent(const string & file, long ident) {
	stringstream name;
	name << file << "_" << setw(4) << setfill('0') << ident;
	return name.str();
}

static bool fix_ident(SaveBlock & save, char name[SIZE_ID], Idents & idents, const string & where, Remap & remap);

static void skip_script_save(const char * dat, size_t & pos) {
	const ARX_CHANGELEVEL_SCRIPT_SAVE * ass;
	ass = reinterpret_cast<const ARX_CHANGELEVEL_SCRIPT_SAVE *>(dat + pos);
	pos += sizeof(ARX_CHANGELEVEL_SCRIPT_SAVE);
	for(int i = 0; i < ass->nblvar; i++) {
		const ARX_CHANGELEVEL_VARIABLE_SAVE * avs;
		avs = reinterpret_cast<const ARX_CHANGELEVEL_VARIABLE_SAVE *>(dat + pos);
		pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);
		if(avs->type == TYPE_L_TEXT || (avs->type != TYPE_L_LONG && avs->type != TYPE_L_FLOAT && (avs->name[0] == '$' || avs->name[0] == '\xA3'))) {
			pos += (size_t)avs->fval;
		}
	}
}

static bool fix_iodata(SaveBlock & save, Idents & idents, char * dat, const string & where, Remap & remap) {
	
	size_t pos = 0;
	ARX_CHANGELEVEL_IO_SAVE & ais = *reinterpret_cast<ARX_CHANGELEVEL_IO_SAVE *>(dat + pos);
	pos += sizeof(ARX_CHANGELEVEL_IO_SAVE);
	
	bool ioChanged = false;
	
	ioChanged |= fix_ident(save, ais.id_targetinfo, idents, where + ".id_targetinfo", remap);
	for(long i = 0; i < ais.nb_linked; i++) {
		stringstream where2;
		where2 << where << ".linked_data[" << i << "].linked_id";
		ioChanged |= fix_ident(save, ais.linked_data[i].linked_id, idents, where2.str(), remap);
	}
	
	pos += ais.nbtimers * sizeof(ARX_CHANGELEVEL_TIMERS_SAVE);
	
	skip_script_save(dat, pos);
	skip_script_save(dat, pos);
	
	bool specificsChanged = false;
	
	switch(ais.savesystem_type) {
		
		case TYPE_NPC: {
			ARX_CHANGELEVEL_NPC_IO_SAVE & anis = *reinterpret_cast<ARX_CHANGELEVEL_NPC_IO_SAVE *>(dat + pos);
			pos += sizeof(ARX_CHANGELEVEL_NPC_IO_SAVE);
			
			specificsChanged |= fix_ident(save, anis.id_weapon, idents, where + ".npc.id_weapon", remap);
			specificsChanged |= fix_ident(save, anis.weapon, idents, where + ".npc.weapon", remap);
			for(size_t i = 0; i < SAVED_MAX_STACKED_BEHAVIOR; i++) {
				stringstream where2;
				where2 << where << ".npc.stackedtarget[" << i << "]";
				specificsChanged |= fix_ident(save, anis.stackedtarget[i], idents, where2.str(), remap);
			}
			break;
		}
		
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
		
		ARX_CHANGELEVEL_INVENTORY_DATA_SAVE & aids = *reinterpret_cast<ARX_CHANGELEVEL_INVENTORY_DATA_SAVE *>(dat + pos);
		pos +=  sizeof(ARX_CHANGELEVEL_INVENTORY_DATA_SAVE);
		
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
		
	}
	
	return ioChanged || specificsChanged || invChanged;
}

static long copy_io(SaveBlock & save, const string & name, Idents & idents, const string & where, char * dat, size_t size) {
	
	ARX_CHANGELEVEL_IO_SAVE & ais = *reinterpret_cast<ARX_CHANGELEVEL_IO_SAVE *>(dat);
	
	size_t pos = name.find_last_of('_');
	
	string fname = name.substr(0, pos);
	
	long i = 1;
	string ident;
	for(; i < 10000; i++) {
		
		ident = makeIdent(fname, i);
		
		if(save.hasFile(ident)) {
			continue;
		}
		
		string file = loadPath(safestring(ais.filename));
		RemoveName(file);
		file += ident;
		
		if(resources->getDirectory(file)) {
			continue;
		}
		
		break;
	}
	
	ais.ident = i;
	
	Remap remap;
	remap[name] = i;
	idents[ident] = where;
	
	fix_iodata(save, idents, dat, where + ":" + ident, remap);
	
	save.save(ident, dat, size);
	
	return i;
}

static long fix_io(SaveBlock & save, const string & name, Idents & idents, const string & where, Remap & remap) {
	
	if(name == "none" || name.empty()) {
		remap[name] = 0;
		return 0;
	}
	
	string savefile = name;
	
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
	
	ARX_CHANGELEVEL_IO_SAVE & ais = *reinterpret_cast<ARX_CHANGELEVEL_IO_SAVE *>(dat);
	
	bool changed = false;
	
	if(ais.ioflags & IO_ITEM) {
		
		string file = toLowercase(safestring(ais.filename));
		
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
	
	changed |= fix_iodata(save, idents, dat, where + ":" + name, remap);
	
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
	
	string namestr = toLowercase(safestring(name, SIZE_ID));
	
	size_t pos = namestr.find_last_of('_');
	
	strncpy(name, makeIdent(namestr.substr(0, pos), newIdent).c_str(), sizeof(name));
	
	return true;
}

static bool fix_ident(SaveBlock & save, char name[SIZE_ID], Idents & idents, const string & where, Remap & remap) {
	
	string lname = toLowercase(safestring(name, SIZE_ID));
	
	if(lname.empty() || lname == "none" || lname == "player" || lname == "self") {
		return false;
	}
	
	Remap::const_iterator it = remap.find(lname);
	if(it != remap.end()) {
		return patch_ident(name, it->second, where);
	}
	
	long newIdent = fix_io(save, lname, idents, where, remap);
	
	return patch_ident(name, newIdent, where);
}

static void fix_player(SaveBlock & save, Idents & idents) {
	
	printf("player\n");
	
	const string & loadfile = "player";
	
	size_t size;
	char * dat = save.load(loadfile, size);
	if(!dat) {
		return;
	}
	
	bool changed = false;
	
	Remap remap;
	
	ARX_CHANGELEVEL_PLAYER & asp = *reinterpret_cast<ARX_CHANGELEVEL_PLAYER *>(dat);
	
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
		save.save(loadfile, dat, size);
	}
	
	free(dat);
	
}

static void fix_level(SaveBlock & save, long num, Idents & idents) {
	
	stringstream ss;
	ss << "lvl" << setfill('0') << setw(3) << num;
	
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
	
	ARX_CHANGELEVEL_INDEX & asi = *reinterpret_cast<ARX_CHANGELEVEL_INDEX *>(dat + pos);
	pos += sizeof(ARX_CHANGELEVEL_INDEX);
	
	ARX_CHANGELEVEL_IO_INDEX * idx_io = reinterpret_cast<ARX_CHANGELEVEL_IO_INDEX *>(dat + pos);
	pos += sizeof(ARX_CHANGELEVEL_IO_INDEX) * asi.nb_inter;
	
	Remap remap;
	
	bool changed = false;
	
	for(long i = 0; i < asi.nb_inter; i++) {
		long res;
		string ident = makeIdent(GetName(loadPath(safestring(idx_io[i].filename))), idx_io[i].ident);
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
		}
	}
	
	if(changed) {
		save.save(ss.str(), dat, size);
	}
	
	free(dat);
	
}

static int main_fix(SaveBlock & save, int argc, char ** argv) {
	
	(void)argv;
	
	if(argc != 0) {
		print_help();
		return 1;
	}
	
	resources = new PakReader();
	
	if(!resources->addArchive("data.pak") || !resources->addArchive("data2.pak")) {
		printf("could not open pak files, run 'savetool fix' from the game directory\n");
		return 3;
	}
	
	resources->addFiles("graph", "graph");
	
	if(!save.open(true)) {
		return 2;
	}
	
	Idents idents;
	
	fix_player(save, idents);
	
	const long MAX_LEVEL = 24;
	for(long i = 0; i <= MAX_LEVEL; i++) {
		fix_level(save, i, idents);
	}
	
	save.flush("pld");
	
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
