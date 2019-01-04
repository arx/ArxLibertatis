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
/* Based on:
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

#include "script/ScriptedPlayer.h"

#include "core/Core.h"
#include "game/EntityManager.h"
#include "game/Inventory.h"
#include "game/Item.h"
#include "game/Player.h"
#include "graphics/Math.h"
#include "graphics/data/Mesh.h"
#include "graphics/particle/ParticleEffects.h"

#include "gui/Hud.h"
#include "gui/Interface.h"
#include "gui/hud/SecondaryInventory.h"
#include "io/resource/ResourcePath.h"
#include "scene/Interactive.h"
#include "scene/GameSound.h"
#include "script/ScriptEvent.h"
#include "script/ScriptUtils.h"


namespace script {

namespace {

class AddBagCommand : public Command {
	
public:
	
	AddBagCommand() : Command("addbag") { }
	
	Result execute(Context & context) {
		
		ARX_UNUSED(context);
		
		DebugScript("");
		
		ARX_PLAYER_AddBag();
		
		return Success;
	}
	
};

class AddXpCommand : public Command {
	
public:
	
	AddXpCommand() : Command("addxp") { }
	
	Result execute(Context & context) {
		
		float val = context.getFloat();
		
		DebugScript(' ' << val);
		
		ARX_PLAYER_Modify_XP(static_cast<long>(val));
		
		return Success;
	}
	
};

class AddGoldCommand : public Command {
	
public:
	
	AddGoldCommand() : Command("addgold") { }
	
	Result execute(Context & context) {
		
		float val = context.getFloat();
		
		DebugScript(' ' << val);
		
		if(val != 0.f) {
			ARX_SOUND_PlayInterface(g_snd.GOLD);
		}
		
		ARX_PLAYER_AddGold(checked_range_cast<long>(val));
		
		return Success;
	}
	
};

class RidiculousCommand : public Command {
	
public:
	
	RidiculousCommand() : Command("ridiculous") { }
	
	Result execute(Context & context) {
		
		ARX_UNUSED(context);
		
		DebugScript("");
		
		ARX_PLAYER_MakeFreshHero();
		
		return Success;
	}
	
};

class RuneCommand : public Command {
	
	typedef std::map<std::string, RuneFlag> Runes;
	Runes runes;
	
public:
	
	RuneCommand() : Command("rune") {
		runes["aam"] = FLAG_AAM;
		runes["cetrius"] = FLAG_CETRIUS;
		runes["comunicatum"] = FLAG_COMUNICATUM;
		runes["cosum"] = FLAG_COSUM;
		runes["folgora"] = FLAG_FOLGORA;
		runes["fridd"] = FLAG_FRIDD;
		runes["kaom"] = FLAG_KAOM;
		runes["mega"] = FLAG_MEGA;
		runes["morte"] = FLAG_MORTE;
		runes["movis"] = FLAG_MOVIS;
		runes["nhi"] = FLAG_NHI;
		runes["rhaa"] = FLAG_RHAA;
		runes["spacium"] = FLAG_SPACIUM;
		runes["stregum"] = FLAG_STREGUM;
		runes["taar"] = FLAG_TAAR;
		runes["tempus"] = FLAG_TEMPUS;
		runes["tera"] = FLAG_TERA;
		runes["vista"] = FLAG_VISTA;
		runes["vitae"] = FLAG_VITAE;
		runes["yok"] = FLAG_YOK;
	}
	
	Result execute(Context & context) {
		
		long add = 0;
		HandleFlags("ar") {
			add = (flg & flag('r')) ? -1 : ((flg & flag('a')) ? 1 : 0);
		}
		
		std::string name = context.getWord();
		
		DebugScript(' ' << options << ' ' << name);
		
		if(name == "all") {
			
			if(add) {
				ScriptWarning << "unexpected flags: " << options << " all";
				return Failed;
			}
			
			ARX_PLAYER_Rune_Add_All();
			
		} else {
			
			if(!add) {
				ScriptWarning << "missing flags:  " << options << ' ' << name << "; expected -a or -r";
				return Failed;
			}
			
			Runes::const_iterator it = runes.find(name);
			if(it == runes.end()) {
				ScriptWarning << "unknown rune name: " << options << ' ' << name;
				return Failed;
			}
			
			if(add == 1) {
				ARX_Player_Rune_Add(it->second);
			} else if(add == -1) {
				ARX_Player_Rune_Remove(it->second);
			}
		}
		
		return Success;
	}
	
};

class QuestCommand : public Command {
	
public:
	
	QuestCommand() : Command("quest") { }
	
	Result execute(Context & context) {
		
		std::string name = loadUnlocalized(context.getWord());
		
		DebugScript(' ' << name);
		
		ARX_PLAYER_Quest_Add(name);
		g_hudRoot.bookIconGui.requestHalo();
		
		return Success;
	}
	
};

class SetPlayerTweakCommand : public Command {
	
public:
	
	SetPlayerTweakCommand() : Command("setplayertweak", AnyEntity) { }
	
	Result execute(Context & context) {
		
		std::string command = context.getWord();
		
		Entity * io = context.getEntity();
		if(!io->tweakerinfo) {
			io->tweakerinfo = new IO_TWEAKER_INFO;
			if(!io->tweakerinfo) {
				return Failed;
			}
		}
		
		if(command == "skin") {
			
			io->tweakerinfo->skintochange = context.getWord();
			io->tweakerinfo->skinchangeto = res::path::load(context.getWord());
			
			DebugScript(" skin " << io->tweakerinfo->skintochange << ' ' << io->tweakerinfo->skinchangeto);
			
		} else {
			
			io->tweakerinfo->filename = res::path::load(context.getWord());
			
			DebugScript(" mesh " << io->tweakerinfo->filename);
		}
		
		return Success;
	}
	
};

class SetHungerCommand : public Command {
	
public:
	
	SetHungerCommand() : Command("sethunger") { }
	
	Result execute(Context & context) {
		
		player.hunger = context.getFloat();
		player.hunger = std::min(player.hunger, 100.f);
		
		DebugScript(' ' << player.hunger);
		
		return Success;
	}
	
};

class SetPlayerControlsCommand : public Command {
	
	static void Stack_SendMsgToAllNPC_IO(Entity * sender, const ScriptEventName & event) {
		for(size_t i = 0; i < entities.size(); i++) {
			const EntityHandle handle = EntityHandle(i);
			Entity * entity = entities[handle];
			if(entity && (entity->ioflags & IO_NPC)) {
				Stack_SendIOScriptEvent(sender, entity, event);
			}
		}
	}
	
public:
	
	SetPlayerControlsCommand() : Command("setplayercontrols") { }
	
	Result execute(Context & context) {
		
		bool enable = context.getBool();
		
		DebugScript(' ' << enable);
		
		if(enable) {
			if(BLOCK_PLAYER_CONTROLS) {
				Stack_SendMsgToAllNPC_IO(context.getEntity(), SM_CONTROLS_ON);
			}
			BLOCK_PLAYER_CONTROLS = false;
		} else {
			if(!BLOCK_PLAYER_CONTROLS) {
				ARX_PLAYER_PutPlayerInNormalStance();
				
				for(size_t i = 0; i < MAX_SPELLS; i++) {
					SpellBase * spell = spells[SpellHandle(i)];
					
					if(spell && (spell->m_caster == EntityHandle_Player || spell->m_target == EntityHandle_Player)) {
						switch(spell->m_type) {
							case SPELL_MAGIC_SIGHT:
							case SPELL_LEVITATE:
							case SPELL_SPEED:
							case SPELL_FLYING_EYE:
								spells.endSpell(spell);
								break;
							default: break;
						}
					}
				}
				
				Stack_SendMsgToAllNPC_IO(context.getEntity(), SM_CONTROLS_OFF);
				spells.endByCaster(EntityHandle_Player);
			}
			BLOCK_PLAYER_CONTROLS = true;
			player.Interface &= ~INTER_COMBATMODE;
		}
		
		return Success;
	}
	
};

class StealNPCCommand : public Command {
	
public:
	
	StealNPCCommand() : Command("stealnpc") { }
	
	Result execute(Context & context) {
		
		DebugScript("");
		
		if(player.Interface & INTER_STEAL) {
			// TODO This sender does not make sense
			SendIOScriptEvent(context.getSender(), ioSteal, SM_STEAL, "off");
		}
		
		player.Interface |= INTER_STEAL;
		g_secondaryInventoryHud.m_fadeDirection = SecondaryInventoryHud::Fade_right;
		ioSteal = context.getEntity();
		
		return Success;
	}
	
};

class SpecialFXCommand : public Command {
	
public:
	
	SpecialFXCommand() : Command("specialfx") { }
	
	Result execute(Context & context) {
		
		std::string type = context.getWord();
		
		Entity * io = context.getEntity();
		
		if(type == "ylside_death") {
			DebugScript(" ylside_death");
			if(!io) {
				ScriptWarning << "can only use 'specialfx ylside_death' in IO context";
				return Failed;
			}
			SetYlsideDeath(io);
			
		} else if(type == "player_appears") {
			DebugScript(" player_appears");
			if(!io) {
				ScriptWarning << "can only use 'specialfx player_appears' in IO context";
				return Failed;
			}
			MakePlayerAppearsFX(*io);
			
		} else if(type == "heal") {
			
			float val = context.getFloat();
			
			DebugScript(" heal " << val);
			
			if(!BLOCK_PLAYER_CONTROLS) {
				player.lifePool.current += val;
			}
			player.lifePool.current = glm::clamp(player.lifePool.current, 0.f, player.Full_maxlife);
			
		} else if(type == "mana") {
			
			float val = context.getFloat();
			
			DebugScript(" mana " << val);
			
			player.manaPool.current = glm::clamp(player.manaPool.current + val, 0.f, player.Full_maxmana);
			
		} else if(type == "newspell") {
			
			context.skipWord();
			
			DebugScript(" newspell");
			
			g_hudRoot.bookIconGui.requestFX();
		} else if(type == "torch") {
			
			DebugScript(" torch");
			
			if(!io || !(io->ioflags & IO_ITEM)) {
				ScriptWarning << "can only use 'specialfx torch' for items";
				return Failed;
			}
			
			Entity * ioo = io;
			if(io->_itemdata->count > 1) {
				ioo = CloneIOItem(io);
				ioo->show = SHOW_FLAG_IN_INVENTORY;
				ioo->scriptload = 1;
				ioo->_itemdata->count = 1;
				io->_itemdata->count--;
			}
			
			ARX_PLAYER_ClickedOnTorch(ioo);
			
		} else if(type == "fiery") {
			DebugScript(" fiery");
			if(!io) {
				ScriptWarning << "can only use 'specialfx fiery' in IO context";
				return Failed;
			}
			io->ioflags |= IO_FIERY;
			
		} else if(type == "fieryoff") {
			DebugScript(" fieryoff");
			if(!io) {
				ScriptWarning << "can only use 'specialfx fieryoff' in IO context";
				return Failed;
			}
			io->ioflags &= ~IO_FIERY;
			
		} else if(type == "torchon") {
			DebugScript(" torchon");
			// do nothing
			
		} else if(type == "torchoff") {
			DebugScript(" torchoff");
			if(player.torch) {
				ARX_PLAYER_ClickedOnTorch(player.torch);
			}
			
		} else {
			ScriptWarning << "unknown fx: " << type;
			return Failed;
		}
		
		return Success;
	}
	
};

class KeyringAddCommand : public Command {
	
public:
	
	KeyringAddCommand() : Command("keyringadd") { }
	
	Result execute(Context & context) {
		
		std::string key = context.getStringVar(context.getWord());
		
		DebugScript(' ' << key);
		
		ARX_KEYRING_Add(key);
		
		return Success;
	}
	
};

class PlayerLookAtCommand : public Command {
	
public:
	
	PlayerLookAtCommand() : Command("playerlookat") { }
	
	Result execute(Context & context) {
		
		std::string target = context.getWord();
		
		DebugScript(' ' << target);
		
		Entity * t = entities.getById(target, context.getEntity());
		if(!t) {
			ScriptWarning << "unknown target: " << target;
			return Failed;
		}
		
		ForcePlayerLookAtIO(t);
		
		return Success;
	}
	
};

class PrecastCommand : public Command {
	
public:
	
	PrecastCommand() : Command("precast") { }
	
	Result execute(Context & context) {
		
		SpellcastFlags spflags = SPELLCAST_FLAG_PRECAST | SPELLCAST_FLAG_NOANIM;
		bool dur = false;
		long duration = -1;
		HandleFlags("df") {
			if(flg & flag('d')) {
				spflags |= SPELLCAST_FLAG_NOCHECKCANCAST;
				duration = long(context.getFloat());
				dur = true;
			}
			if(flg & flag('f')) {
				spflags |= SPELLCAST_FLAG_NOCHECKCANCAST | SPELLCAST_FLAG_NOMANA;
			}
		}
		
		long level = glm::clamp(long(context.getFloat()), 1l, 10l);
		
		std::string spellname = context.getWord();
		
		DebugScript(' ' << options << ' ' << duration << ' ' << level << ' ' << spellname);
		
		SpellType spellid = GetSpellId(spellname);
		if(spellid == SPELL_NONE) {
			ScriptWarning << "unknown spell: " << spellname;
			return Failed;
		}
		
		if(!dur) {
			duration = 2000 + level * 2000;
		}
		
		if(context.getEntity() != entities.player()) {
			spflags |= SPELLCAST_FLAG_NOCHECKCANCAST;
		}
		
		TryToCastSpell(entities.player(), spellid, level, EntityHandle(), spflags, GameDurationMs(duration));
		
		return Success;
	}
	
};

class PoisonCommand : public Command {
	
public:
	
	PoisonCommand() : Command("poison") { }
	
	Result execute(Context & context) {
		
		float fval = context.getFloat();
		
		DebugScript(' ' << fval);
		
		ARX_PLAYER_Poison(fval);
		
		return Success;
	}
	
};

class PlayerManaDrainCommand : public Command {
	
public:
	
	PlayerManaDrainCommand() : Command("playermanadrain") { }
	
	Result execute(Context & context) {
		
		bool enable = context.getBool();
		
		DebugScript(' ' << enable);
		
		if(enable) {
			player.playerflags &= ~PLAYERFLAGS_NO_MANA_DRAIN;
		} else {
			player.playerflags |= PLAYERFLAGS_NO_MANA_DRAIN;
		}
		
		return Success;
	}
	
};

class InvulnerabilityCommand : public Command {
	
public:
	
	InvulnerabilityCommand() : Command("invulnerability") { }
	
	Result execute(Context & context) {
		
		bool player = false;
		HandleFlags("p") {
			player = test_flag(flg, 'p');
		}
		
		bool enable = context.getBool();
		
		DebugScript(' ' << options << ' ' << enable);
		
		Entity * io = context.getEntity();
		if(!player && !io) {
			ScriptWarning << "must either use -p or execute in IO context";
			return Failed;
		}
		
		
		if(enable) {
			if(player) {
				ARX_PLAYER_Invulnerability(1);
			} else {
				io->ioflags |= IO_INVULNERABILITY;
			}
		} else {
			if(player) {
				ARX_PLAYER_Invulnerability(0);
			} else {
				io->ioflags &= ~IO_INVULNERABILITY;
			}
		}
		
		return Success;
	}
	
};

} // anonymous namespace

void setupScriptedPlayer() {
	
	ScriptEvent::registerCommand(new AddBagCommand);
	ScriptEvent::registerCommand(new AddXpCommand);
	ScriptEvent::registerCommand(new AddGoldCommand);
	ScriptEvent::registerCommand(new RidiculousCommand);
	ScriptEvent::registerCommand(new RuneCommand);
	ScriptEvent::registerCommand(new QuestCommand);
	ScriptEvent::registerCommand(new SetPlayerTweakCommand);
	ScriptEvent::registerCommand(new SetHungerCommand);
	ScriptEvent::registerCommand(new SetPlayerControlsCommand);
	ScriptEvent::registerCommand(new StealNPCCommand);
	ScriptEvent::registerCommand(new SpecialFXCommand);
	ScriptEvent::registerCommand(new KeyringAddCommand);
	ScriptEvent::registerCommand(new PlayerLookAtCommand);
	ScriptEvent::registerCommand(new PrecastCommand);
	ScriptEvent::registerCommand(new PoisonCommand);
	ScriptEvent::registerCommand(new PlayerManaDrainCommand);
	ScriptEvent::registerCommand(new InvulnerabilityCommand);
	
}

} // namespace script
