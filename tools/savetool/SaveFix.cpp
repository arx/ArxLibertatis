/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "savetool/SaveFix.h"

#include <cstdio>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>

#include <boost/foreach.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/io/ios_state.hpp>

#include "Configure.h"

#include "io/SaveBlock.h"
#include "io/fs/FilePath.h"
#include "io/fs/SystemPaths.h"
#include "io/log/Logger.h"
#include "io/resource/PakReader.h"
#include "scene/SaveFormat.h"
#include "util/String.h"

typedef std::map<std::string, std::string> Idents; // ident -> where
typedef std::map<std::string, long> Remap; // ident -> newIdent

static std::string makeIdent(const std::string & file, long ident) {
	std::stringstream name;
	name << file << "_" << std::setw(4) << std::setfill('0') << ident;
	return name.str();
}

static bool fix_ident(SaveBlock & save, char (&name)[SIZE_ID], Idents & idents, const std::string & where, Remap & remap);

static void skip_script_save(const char * dat, size_t & pos) {
	const ARX_CHANGELEVEL_SCRIPT_SAVE * ass;
	ass = reinterpret_cast<const ARX_CHANGELEVEL_SCRIPT_SAVE *>(dat + pos);
	pos += sizeof(ARX_CHANGELEVEL_SCRIPT_SAVE);
	for(int i = 0; i < ass->nblvar; i++) {
		const ARX_CHANGELEVEL_VARIABLE_SAVE * avs;
		avs = reinterpret_cast<const ARX_CHANGELEVEL_VARIABLE_SAVE *>(dat + pos);
		pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);
		if(avs->type == TYPE_L_TEXT || (avs->type != TYPE_L_LONG && avs->type != TYPE_L_FLOAT && (avs->name[0] == '$' || avs->name[0] == '\xA3'))) {
			pos += size_t(avs->fval);
		}
	}
}

static bool fix_iodata(SaveBlock & save, Idents & idents, char * dat, const std::string & where, Remap & remap) {
	
	size_t pos = 0;
	ARX_CHANGELEVEL_IO_SAVE & ais = *reinterpret_cast<ARX_CHANGELEVEL_IO_SAVE *>(dat + pos);
	pos += sizeof(ARX_CHANGELEVEL_IO_SAVE);
	
	bool ioChanged = false;
	
	ioChanged |= fix_ident(save, ais.id_targetinfo, idents, where + ".id_targetinfo", remap);
	for(long i = 0; i < ais.nb_linked; i++) {
		std::stringstream where2;
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
				std::stringstream where2;
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
		
		ARX_CHANGELEVEL_INVENTORY_DATA_SAVE & aids
			= *reinterpret_cast<ARX_CHANGELEVEL_INVENTORY_DATA_SAVE *>(dat + pos);
		
		invChanged |= fix_ident(save, aids.io, idents, where + ".inventory.io", remap);
		for(long m = 0; m < aids.sizex; m++) {
			for(long n = 0; n < aids.sizey; n++) {
				std::stringstream where2;
				where2 << where << ".inventory[" << m << "][" << n << "]";
				invChanged |= fix_ident(save, aids.slot_io[m][n], idents, where2.str(), remap);
			}
		}
		invChanged |= fix_ident(save, aids.weapon, idents, where + ".inventory.weapon", remap);
		invChanged |= fix_ident(save, aids.targetinfo, idents, where + ".inventory.targetinfo", remap);
		for(long i = 0; i < ais.nb_linked; i++) {
			std::stringstream where2;
			where2 << where << ".inventory.linked_id[" << i << "]";
			invChanged |= fix_ident(save, aids.linked_id[i], idents, where2.str(), remap);
		}
		for(size_t i = 0; i < SAVED_MAX_STACKED_BEHAVIOR; i++) {
			std::stringstream where2;
			where2 << where << ".inventory.stackedtarget[" << i << "]";
			invChanged |= fix_ident(save, aids.stackedtarget[i], idents, where2.str(), remap);
		}
		
	}
	
	return ioChanged || specificsChanged || invChanged;
}

static long copy_io(SaveBlock & save, const std::string & name, Idents & idents, const std::string & where, char * dat, size_t size) {
	
	ARX_CHANGELEVEL_IO_SAVE & ais = *reinterpret_cast<ARX_CHANGELEVEL_IO_SAVE *>(dat);
	
	size_t pos = name.find_last_of('_');
	
	std::string fname = name.substr(0, pos);
	
	res::path dir = res::path::load(util::loadString(ais.filename)).parent();
	
	long i = 1;
	std::string ident;
	for(; i < 10000; i++) {
		
		ident = makeIdent(fname, i);
		
		if(save.hasFile(ident)) {
			continue;
		}
		
		if(g_resources->getDirectory(dir / ident)) {
			continue;
		}
		
		break;
	}
	
	ais.ident = i;
	
	Remap remap;
	remap[name] = i;
	idents[ident] = where;
	
	fix_iodata(save, idents, dat, where + ":" + ident, remap);
	
	LogDebug("#saving copied io " << ident);
	save.save(ident, dat, size);
	
	return i;
}

static long fix_io(SaveBlock & save, const std::string & name, Idents & idents, const std::string & where, Remap & remap) {
	
	if(name == "none" || name.empty()) {
		remap[name] = 0;
		return 0;
	}
	
	const std::string & savefile = name;
	
	std::string buffer = save.load(savefile);
	if(buffer.empty()) {
		remap[name] = 0;
		return 0;
	}
	
	char * dat = &buffer[0];
	
	Idents::iterator it = idents.find(name);
	if(it != idents.end()) {
		std::cout << "duplicate ident " << name << " detected: in " << it->second << " and " << where << '\n';
		// we already fixed this!
		long newIdent = copy_io(save, name, idents, where, dat, buffer.size());
		std::cout << " -> copied " << name << " as " << newIdent << " for " << where << '\n';
		remap[name] = newIdent;
		return newIdent;
	} else {
		idents[name] = where;
		remap[name] = 0;
	}
	
	ARX_CHANGELEVEL_IO_SAVE & ais = *reinterpret_cast<ARX_CHANGELEVEL_IO_SAVE *>(dat);
	
	bool changed = false;
	
	if(ais.ioflags & IO_ITEM) {
		
		res::path file = res::path::load(util::loadString(ais.filename));
		
		s32 flags = ais.ioflags;
		
		if(boost::starts_with(file.basename(), "gold_coin")) {
			file.up() /= "gold_coin.asl";
			flags = EntityFlags::load(ais.ioflags) | IO_GOLD; // TODO save/load flags
		}
		
		if(boost::contains(file.string(), "movable")) {
			flags = EntityFlags::load(ais.ioflags) | IO_MOVABLE; // TODO save/load flags
		}
		
		if(flags != ais.ioflags) {
			boost::io::ios_all_saver coutFlags(std::cout);
			std::cout << " - fixing " << name << ": ioflags 0x" << std::hex << ais.ioflags << " -> 0x" << std::hex << flags << '\n';
			coutFlags.restore();
			ais.ioflags = flags;
			changed = true;
		}
		
	}
	
	changed |= fix_iodata(save, idents, dat, where + ":" + name, remap);
	
	if(changed) {
		LogDebug("#saving fixed io " << savefile);
		if(!save.save(savefile, dat, buffer.size())) {
			std::cerr << "error saving " << savefile;
		}
	}
	
	return 0;
}

static bool patch_ident(char (&name)[SIZE_ID], long newIdent, const std::string & where) {
	
	if(newIdent <= 0) {
		return false;
	}
	
	std::cout << "fixing ident in " << where << ": " << name << " -> " << newIdent << '\n';
	
	std::string namestr = boost::to_lower_copy(util::loadString(name, SIZE_ID));
	
	size_t pos = namestr.find_last_of('_');
	
	util::storeString(name, makeIdent(namestr.substr(0, pos), newIdent));
	
	return true;
}

static bool fix_ident(SaveBlock & save, char (&name)[SIZE_ID], Idents & idents, const std::string & where, Remap & remap) {
	
	std::string lname = boost::to_lower_copy(util::loadString(name, SIZE_ID));
	
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
	
	std::cout << "player\n";
	
	const std::string loadfile = "player";
	
	std::string buffer = save.load(loadfile);
	if(buffer.empty()) {
		return;
	}
	
	char * dat = &buffer[0];
	
	bool changed = false;
	
	Remap remap;
	
	ARX_CHANGELEVEL_PLAYER & asp = *reinterpret_cast<ARX_CHANGELEVEL_PLAYER *>(dat);
	
	for(size_t iNbBag = 0; iNbBag < SAVED_INVENTORY_BAGS; iNbBag++) {
		for(size_t m = 0; m < SAVED_INVENTORY_Y; m++) {
			for(size_t n = 0; n < SAVED_INVENTORY_X; n++) {
				std::stringstream where;
				where << "player.inventory[" << iNbBag << "][" << n << "][" << m << "]";
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
		std::stringstream where;
		where << "player.equiped[" << k << "]";
		changed |= fix_ident(save, asp.equiped[k], idents, where.str(), remap);
	}
	
	if(changed) {
		LogDebug("saving fixed " << loadfile);
		save.save(loadfile, dat, buffer.size());
	}
	
}

static void fix_level(SaveBlock & save, long num, Idents & idents) {
	
	std::stringstream ss;
	ss << "lvl" << std::setfill('0') << std::setw(3) << num;
	
	if(!save.hasFile(ss.str())) {
		return;
	}
	
	std::string buffer = save.load(ss.str());
	if(buffer.empty()) {
		return;
	}
	
	char * dat = &buffer[0];
	
	std::cout << "level " << num << '\n';
	
	size_t pos = 0;
	
	ARX_CHANGELEVEL_INDEX & asi = *reinterpret_cast<ARX_CHANGELEVEL_INDEX *>(dat + pos);
	pos += sizeof(ARX_CHANGELEVEL_INDEX);
	
	ARX_CHANGELEVEL_IO_INDEX * idx_io = reinterpret_cast<ARX_CHANGELEVEL_IO_INDEX *>(dat + pos);
	
	Remap remap;
	
	bool changed = false;
	
	for(long i = 0; i < asi.nb_inter; i++) {
		long res;
		std::string ident = makeIdent(res::path::load(util::loadString(idx_io[i].filename)).basename(), idx_io[i].ident);
		Remap::const_iterator it = remap.find(ident);
		std::stringstream where;
		where << "level" << num << "[" << i << "]";
		if(it != remap.end()) {
			res = it->second;
		} else {
			res = fix_io(save, ident, idents, where.str(), remap);
		}
		if(res != 0) {
			std::cout << "fixing ident in " << where.str() << ": " << ident << " -> " << res << '\n';
			idx_io[i].ident = res;
			changed = true;
		}
	}
	
	if(changed) {
		LogDebug("#saving fixed " << ss.str());
		save.save(ss.str(), dat, buffer.size());
	}
	
}

int main_fix(SaveBlock & save, const std::vector<std::string> & args) {
	
	if(!args.empty()) {
		return -1;
	}
	
	g_resources = new PakReader();
	
	// TODO share this list with the game code
	static const char * const default_paks[] = { "data.pak", "data2.pak" };
	BOOST_FOREACH(const char * const filename, default_paks) {
		if(g_resources->addArchive(fs::findDataFile(filename))) {
			continue;
		}
		LogError << "Missing required data file: \"" << filename << "\"";
		return 3;
	}
	BOOST_REVERSE_FOREACH(const fs::path & base, fs::getDataDirs()) {
		const char * dirname = "graph";
		g_resources->addFiles(base / dirname, dirname);
	}
	
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
