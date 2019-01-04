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
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#include "core/Core.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <iomanip>
#include <sstream>
#include <vector>

#include <boost/version.hpp>
#include <boost/foreach.hpp>

#include "Configure.h"

#include "ai/Paths.h"
#include "ai/PathFinderManager.h"

#include "animation/Animation.h"
#include "animation/AnimationRender.h"

#include "cinematic/Cinematic.h"
#include "cinematic/CinematicLoad.h"
#include "cinematic/CinematicKeyframer.h"

#include "core/Application.h"
#include "core/ArxGame.h"
#include "core/Benchmark.h"
#include "core/Config.h"
#include "core/Localisation.h"
#include "core/GameTime.h"
#include "core/Version.h"

#include "game/Camera.h"
#include "game/Damage.h"
#include "game/EntityManager.h"
#include "game/Equipment.h"
#include "game/Inventory.h"
#include "game/Levels.h"
#include "game/Missile.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "game/npc/Dismemberment.h"
#include "game/spell/FlyingEye.h"
#include "game/spell/Cheat.h"
#include "game/effect/Quake.h"
#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Draw.h"
#include "graphics/DrawLine.h"
#include "graphics/DrawDebug.h"
#include "graphics/font/Font.h"
#include "graphics/GlobalFog.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"
#include "graphics/Renderer.h"
#include "graphics/Vertex.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/PolyBoom.h"
#include "graphics/effects/Fade.h"
#include "graphics/effects/Fog.h"
#include "graphics/image/Image.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/ParticleManager.h"
#include "graphics/particle/MagicFlare.h"
#include "graphics/particle/Spark.h"
#include "graphics/texture/TextureStage.h"

#include "gui/Cursor.h"
#include "gui/Hud.h"
#include "gui/Interface.h"
#include "gui/LoadLevelScreen.h"
#include "gui/Menu.h"
#include "gui/MenuWidgets.h"
#include "gui/Speech.h"
#include "gui/MiniMap.h"
#include "gui/Text.h"
#include "gui/TextManager.h"
#include "gui/debug/DebugKeys.h"

#include "input/Input.h"
#include "input/Keyboard.h"
#include "input/Mouse.h"

#include "io/fs/SystemPaths.h"
#include "io/resource/ResourcePath.h"
#include "io/resource/PakReader.h"
#include "io/Screenshot.h"
#include "io/log/Logger.h"

#include "math/Angle.h"
#include "math/Rectangle.h"
#include "math/Vector.h"
 
#include "physics/Collisions.h"
#include "physics/Projectile.h"

#include "platform/CrashHandler.h"
#include "platform/Platform.h"
#include "platform/ProgramOptions.h"

#include "scene/LinkedObject.h"
#include "cinematic/CinematicSound.h"
#include "scene/ChangeLevel.h"
#include "scene/Scene.h"
#include "scene/GameSound.h"
#include "scene/LoadLevel.h"
#include "scene/Interactive.h"
#include "scene/Light.h"
#include "scene/Object.h"

#include "script/Script.h"

#include "window/RenderWindow.h"

class TextManager;

Image savegame_thumbnail;

extern long DONT_WANT_PLAYER_INZONE;

//-----------------------------------------------------------------------------

TextureContainer * GoldCoinsTC[MAX_GOLD_COINS_VISUALS]; // Gold Coins Icons

Vec2s DANAEMouse;
Vec3f g_moveto;
EERIE_3DOBJ * GoldCoinsObj[MAX_GOLD_COINS_VISUALS];// 3D Objects For Gold Coins
EERIE_3DOBJ * arrowobj = NULL; // 3D Object for arrows
EERIE_3DOBJ * cameraobj = NULL; // Camera 3D Object // NEEDTO: Remove for Final
EERIE_3DOBJ * markerobj = NULL; // Marker 3D Object // NEEDTO: Remove for Final

Vec2s g_dragStartPos;
Entity * COMBINE = NULL;

bool GMOD_RESET = true;

Vec3f LastValidPlayerPos;

// START - Information for Player Teleport between/in Levels-------------------------------------
std::string TELEPORT_TO_LEVEL;
std::string TELEPORT_TO_POSITION;
long TELEPORT_TO_ANGLE;
// END -   Information for Player Teleport between/in Levels---------------------------------------
res::path LastLoadedScene;

float g_framedelay = 0.f;

bool LOAD_N_ERASE = true;

Rect g_size(640, 480);
Vec2f g_sizeRatio(1.f, 1.f);

bool REQUEST_SPEECH_SKIP = false;
long CURRENTLEVEL = -1;
bool DONT_ERASE_PLAYER = false;

ChangeLevelIcon CHANGE_LEVEL_ICON = NoChangeLevel;

bool g_cursorOverBook = false;
//-----------------------------------------------------------------------------
// DEBUG FLAGS/Vars
//-----------------------------------------------------------------------------
bool g_requestLevelInit = false;

bool START_NEW_QUEST = false;
static long LAST_WEAPON_TYPE = -1;

float PULSATE;

bool AdjustUI() {
	
	// Sets Danae Screen size depending on windowed/full-screen state
	g_size = Rect(mainApp->getWindow()->getSize().x, mainApp->getWindow()->getSize().y);
		
	// Computes X & Y screen ratios compared to a standard 640x480 screen
	
	g_sizeRatio.x = float(g_size.width()) / 640.f;
	g_sizeRatio.y = float(g_size.height()) / 480.f;
	
	if(!ARX_Text_Init()) {
		return false;
	}
	
	MenuReInitAll();
	
	return true;
}

void runGame() {
	
	mainApp = new ArxGame();
	if(mainApp->initialize()) {
		// Init all done, start the main loop
		mainApp->run();
	} else {
		// Fallback to a generic critical error in case none was set yet...
		LogCritical << "Initialization failed";
	}
	
	if(mainApp) {
		mainApp->shutdown();
		delete mainApp;
		mainApp = NULL;
	}
}

Entity * FlyingOverObject(const Vec2s & pos) {
	
	// TODO do this properly!
	if(player.torch && eMouseState == MOUSE_IN_TORCH_ICON) {
		return player.torch;
	}
	
	if(Entity * entity = GetFromInventory(pos).first) {
		return entity;
	}
	
	if(InInventoryPos(pos)) {
		return NULL;
	}
	
	if(Entity * entity = InterClick(pos)) {
		return entity;
	}
	
	return NULL;
}

static void PlayerLaunchArrow_Test(float aimratio, float poisonous, const Vec3f & pos, const Anglef & angle) {
	
	Vec3f vect = angleToVector(angle);
	Vec3f position = pos;
	float velocity = aimratio + 0.3f;

	if(velocity < 0.9f)
		velocity = 0.9f;
	
	glm::quat quat = angleToQuatForArrow(angle);

	float wd = getEquipmentBaseModifier(IO_EQUIPITEM_ELEMENT_Damages);
	// TODO Why ignore relative modifiers? Why not just use player.Full_damages?
	
	float damages = wd * (1.f + (player.m_skillFull.projectile + player.m_attributeFull.dexterity) * 0.02f);

	ARX_THROWN_OBJECT_Throw(EntityHandle_Player, position, vect, quat, velocity, damages, poisonous);
}

void SetEditMode() {
	
	LAST_JUMP_ENDTIME = 0;
	
	if(!DONT_ERASE_PLAYER) {
		player.lifePool.current = 0.1f;
	}
	
	RestoreAllLightsInitialStatus();

	RestoreInitialIOStatus();
	
	if(!DONT_ERASE_PLAYER) {
		ARX_PLAYER_MakeFreshHero();
	}
	
}

void levelInit() {
	
	arx_assert(entities.player());
	
	LogDebug("Initializing level ...");
	
	ARX_PARTICLES_FirstInit();
	g_renderBatcher.reset();
	
	progressBarAdvance(2.f);
	LoadLevelScreen();
	
	g_particleManager.Clear();
	
	if(GMOD_RESET)
		ARX_GLOBALMODS_Reset();

	GMOD_RESET = true;
	
	g_dragStartPos = Vec2s(0);
	DANAEMouse = Vec2s(0);
	
	PolyBoomClear();
	ARX_DAMAGES_Reset();
	ARX_MISSILES_ClearAll();
	spells.clearAll();
	ARX_SPELLS_ClearAllSymbolDraw();
	
	ARX_PARTICLES_ClearAll();
	ParticleSparkClear();
	
	if(LOAD_N_ERASE) {
		CleanScriptLoadedIO();
		RestoreInitialIOStatus();
		DRAGINTER = NULL;
	}
	
	ARX_SPELLS_ResetRecognition();
	
	eyeball.exist = 0;
	
	resetDynLights();
	
	if(LOAD_N_ERASE) {
		CleanInventory();
		ARX_SCRIPT_Timer_ClearAll();
		UnlinkAllLinkedObjects();
		ARX_SCRIPT_ResetAll(false);
	}
	
	SecondaryInventory = NULL;
	TSecondaryInventory = NULL;
	ARX_FOGS_Render();
	
	if(LOAD_N_ERASE) {
		
		if(!DONT_ERASE_PLAYER)
			ARX_PLAYER_InitPlayer();

		g_hudRoot.playerInterfaceFader.resetSlid();

		player.lifePool.current = player.lifePool.max;
		player.manaPool.current = player.manaPool.max;
		if(!DONT_ERASE_PLAYER) {
			ARX_PLAYER_MakeFreshHero();
		}
	}
	
	InitSnapShot(fs::getUserDir() / "snapshot");
	
	progressBarAdvance(4.f);
	LoadLevelScreen();

	if(player.torch) {
		player.torch_loop = ARX_SOUND_PlaySFX_loop(g_snd.TORCH_LOOP, NULL, 1.f);
	}
	
	g_playerCamera.m_pos = g_moveto = player.pos;
	g_playerCamera.angle = player.angle;
	
	RestoreLastLoadedLightning(*ACTIVEBKG);

	progressBarAdvance();
	LoadLevelScreen();

	if(LOAD_N_ERASE) {
		SetEditMode();
		ARX_SOUND_MixerStop(ARX_SOUND_MixerGame);
		ARX_SCRIPT_ResetAll(true);
		EERIE_ANIMMANAGER_PurgeUnused();
	}

	progressBarAdvance();
	LoadLevelScreen();

	LOAD_N_ERASE = true;
	DONT_ERASE_PLAYER = false;

	progressBarAdvance();
	LoadLevelScreen();
	
	PrepareIOTreatZone(1);
	
	progressBarAdvance();
	LoadLevelScreen();
	
	if(DONT_WANT_PLAYER_INZONE) {
		player.inzone = NULL;
		DONT_WANT_PLAYER_INZONE = 0;
	}
	
	progressBarAdvance();
	LoadLevelScreen();

	player.desiredangle.setPitch(0.f);
	player.angle.setPitch(0.f);
	ARX_PLAYER_RectifyPosition();

	entities.player()->_npcdata->vvpos = -99999;
	
	SendMsgToAllIO(NULL, SM_GAME_READY);
	
	PLAYER_MOUSELOOK_ON = false;
	
	g_note.clear();
	
	EntityHandle t = entities.getById("seat_stool1_0012");
	if(ValidIONum(t)) {
		entities[t]->ioflags |= IO_FORCEDRAW;
	}
	
	ARX_NPC_RestoreCuts();
	
	ResetVVPos(entities.player());
	
	progressBarAdvance();
	LoadLevelScreen();
	LoadLevelScreen(-2);
	
	if(!CheckInPoly(player.pos) && LastValidPlayerPos.x != 0.f
	   && LastValidPlayerPos.y != 0.f && LastValidPlayerPos.z != 0.f) {
		player.pos = LastValidPlayerPos;
	}
	
	LastValidPlayerPos = player.pos;
	
	g_platformTime.updateFrame();
	
	g_gameTime.resume(GameTime::PauseInitial | GameTime::PauseMenu);
}

void ManageNONCombatModeAnimations() {
	arx_assert(entities.player());
	
	Entity * io = entities.player();
	
	AnimLayer & layer3 = io->animlayer[3];
	ANIM_HANDLE ** alist = io->anims;
	
	if(player.m_currentMovement & (PLAYER_LEAN_LEFT | PLAYER_LEAN_RIGHT))
		return;
	
	if(ValidIONum(player.equiped[EQUIP_SLOT_SHIELD]) && !BLOCK_PLAYER_CONTROLS) {
		if(layer3.cur_anim == NULL || (layer3.cur_anim != alist[ANIM_SHIELD_CYCLE]
		                               && layer3.cur_anim != alist[ANIM_SHIELD_HIT]
		                               && layer3.cur_anim != alist[ANIM_SHIELD_START])) {
			changeAnimation(io, 3, alist[ANIM_SHIELD_START]);
		} else if(layer3.cur_anim == alist[ANIM_SHIELD_START] && (layer3.flags & EA_ANIMEND)) {
			changeAnimation(io, 3, alist[ANIM_SHIELD_CYCLE], EA_LOOP);
		}
	} else {
		if(layer3.cur_anim == alist[ANIM_SHIELD_CYCLE]) {
			changeAnimation(io, 3, alist[ANIM_SHIELD_END]);
		} else if(layer3.cur_anim == alist[ANIM_SHIELD_END] && (layer3.flags & EA_ANIMEND)) {
			layer3.cur_anim = NULL;
		}
	}
}

static bool StrikeAimtime() {
	
	ARX_PLAYER_Remove_Invisibility();
	
	player.m_strikeAimRatio = glm::clamp(player.m_aimTime / player.Full_AimTime, 0.1f, 1.0f);
	
	return (player.m_strikeAimRatio > 0.8f);
}

static void strikeSpeak(Entity * io) {
	
	if(!StrikeAimtime()) {
		return;
	}
	
	const std::string * str;
	EntityHandle equiped = player.equiped[EQUIP_SLOT_WEAPON];
	if(ValidIONum(equiped) && !entities[equiped]->strikespeech.empty()) {
		str = &entities[equiped]->strikespeech;
	} else if(!io->strikespeech.empty()) {
		str = &io->strikespeech;
	} else {
		return;
	}
	
	ARX_SPEECH_AddSpeech(io, *str, ANIM_TALK_NEUTRAL, ARX_SPEECH_FLAG_NOTEXT);
}

void ManageCombatModeAnimations() {
	
	arx_assert(entities.player());
	
	if(player.m_aimTime > 0) {
		player.m_aimTime += g_platformTime.lastFrameDuration();
	}
	
	Entity * const io = entities.player();
	
	AnimLayer & layer1 = io->animlayer[1];
	
	ANIM_HANDLE ** alist = io->anims;
	WeaponType weapontype = ARX_EQUIPMENT_GetPlayerWeaponType();
	
	if(weapontype == WEAPON_BARE && LAST_WEAPON_TYPE != weapontype) {
		if(layer1.cur_anim != alist[ANIM_BARE_WAIT]) {
			changeAnimation(io, 1, alist[ANIM_BARE_WAIT]);
			player.m_aimTime = 0;
		}
	}
	
	switch(weapontype) {
		case WEAPON_BARE: { // BARE HANDS PLAYER MANAGEMENT
			if(layer1.cur_anim == alist[ANIM_BARE_WAIT]) {
				player.m_aimTime = 0;
				if(eeMousePressed1()) {
					changeAnimation(io, 1, alist[ANIM_BARE_STRIKE_LEFT_START + player.m_strikeDirection * 3]);
					io->isHit = false;
				}
			}
			
			// Now go for strike cycle...
			for(long j = 0; j < 4; j++) {
				if(layer1.cur_anim == alist[ANIM_BARE_STRIKE_LEFT_START + j * 3] && (layer1.flags & EA_ANIMEND)) {
					changeAnimation(io, 1, alist[ANIM_BARE_STRIKE_LEFT_CYCLE + j * 3], EA_LOOP);
					player.m_aimTime = PlatformDuration::ofRaw(1);
				} else if(layer1.cur_anim == alist[ANIM_BARE_STRIKE_LEFT_CYCLE + j * 3] && !eeMousePressed1()) {
					changeAnimation(io, 1, alist[ANIM_BARE_STRIKE_LEFT + j * 3]);
					strikeSpeak(io);
					SendIOScriptEvent(NULL, io, SM_STRIKE, "bare");
					player.m_weaponBlocked = AnimationDuration::ofRaw(-1); // TODO inband signaling AnimationDuration
					player.m_strikeDirection = 0;
					player.m_aimTime = 0;
				} else if(layer1.cur_anim == alist[ANIM_BARE_STRIKE_LEFT + j * 3]) {
					if(layer1.flags & EA_ANIMEND) {
						changeAnimation(io, 1, alist[ANIM_BARE_WAIT], EA_LOOP);
						player.m_strikeDirection = 0;
						player.m_aimTime = PlatformDuration::ofRaw(1);
						player.m_weaponBlocked = AnimationDuration::ofRaw(-1);
					} else if( layer1.ctime > layer1.currentAltAnim()->anim_time * 0.2f
					        && layer1.ctime < layer1.currentAltAnim()->anim_time * 0.8f
					        && player.m_weaponBlocked == AnimationDuration::ofRaw(-1)) {
						
						ActionPoint id = ActionPoint();
						if(layer1.cur_anim == alist[ANIM_BARE_STRIKE_LEFT]) {
							id = io->obj->fastaccess.left_attach;
						} else { // Strike Right
							id = io->obj->fastaccess.primary_attach;
						}
						
						if(id != ActionPoint()) {
							Sphere sphere;
							sphere.origin = actionPointPosition(io->obj, id);
							sphere.radius = 25.f;
							
							EntityHandle num;
							
							if(CheckAnythingInSphere(sphere, EntityHandle_Player, 0, &num)) {
								float dmgs = (player.m_miscFull.damages + 1) * player.m_strikeAimRatio;
								
								if(ARX_DAMAGES_TryToDoDamage(actionPointPosition(io->obj, id), dmgs, 40, EntityHandle_Player)) {
									player.m_weaponBlocked = layer1.ctime;
								}
								
								ParticleSparkSpawnContinous(sphere.origin, unsigned(dmgs), SpawnSparkType_Success);
								
								if(ValidIONum(num)) {
									static PlatformInstant lastHit = 0;
									PlatformInstant now = g_platformTime.frameStart();
									if(now - lastHit > toPlatformDuration(layer1.ctime)) {
										ARX_SOUND_PlayCollision(entities[num]->material, MATERIAL_FLESH, 1.f, 1.f, sphere.origin, NULL);
										lastHit = now;
									}
								}
							}
						}
					}
				}
			}
			break;
		}
		case WEAPON_DAGGER: { // DAGGER PLAYER MANAGEMENT
			// Waiting and receiving Strike Impulse
			if(layer1.cur_anim == alist[ANIM_DAGGER_WAIT]) {
				player.m_aimTime = 0;
				if(eeMousePressed1()) {
					changeAnimation(io, 1, alist[ANIM_DAGGER_STRIKE_LEFT_START + player.m_strikeDirection * 3]);
					io->isHit = false;
				}
			}
			
			// Now go for strike cycle...
			for(long j = 0; j < 4; j++) {
				if(layer1.cur_anim == alist[ANIM_DAGGER_STRIKE_LEFT_START + j * 3] && (layer1.flags & EA_ANIMEND)) {
					changeAnimation(io, 1, alist[ANIM_DAGGER_STRIKE_LEFT_CYCLE + j * 3], EA_LOOP);
					player.m_aimTime = PlatformDuration::ofRaw(1);
				} else if(layer1.cur_anim == alist[ANIM_DAGGER_STRIKE_LEFT_CYCLE + j * 3] && !eeMousePressed1()) {
					changeAnimation(io, 1, alist[ANIM_DAGGER_STRIKE_LEFT + j * 3]);
					strikeSpeak(io);
					SendIOScriptEvent(NULL, io, SM_STRIKE, "dagger");
					player.m_strikeDirection = 0;
					player.m_aimTime = 0;
				} else if(layer1.cur_anim == alist[ANIM_DAGGER_STRIKE_LEFT + j * 3]) {
					if(   layer1.ctime > layer1.currentAltAnim()->anim_time * 0.3f
					   && layer1.ctime < layer1.currentAltAnim()->anim_time * 0.7f
					) {
						Entity * weapon = entities[player.equiped[EQUIP_SLOT_WEAPON]];
						
						if(player.m_weaponBlocked == AnimationDuration::ofRaw(-1)
							&& ARX_EQUIPMENT_Strike_Check(io, weapon, player.m_strikeAimRatio, 0))
						{
							player.m_weaponBlocked = layer1.ctime;
						}
					}
					
					if(layer1.flags & EA_ANIMEND) {
						changeAnimation(io, 1, alist[ANIM_DAGGER_WAIT], EA_LOOP);
						layer1.flags &= ~(EA_PAUSED | EA_REVERSE);
						player.m_strikeDirection = 0;
						player.m_aimTime = PlatformDuration::ofRaw(1);
						player.m_weaponBlocked = AnimationDuration::ofRaw(-1);
					}
					
					if(   player.m_weaponBlocked != AnimationDuration::ofRaw(-1)
					   && layer1.ctime < layer1.currentAltAnim()->anim_time * 0.9f
					) {
						Entity * weapon = entities[player.equiped[EQUIP_SLOT_WEAPON]];
						ARX_EQUIPMENT_Strike_Check(io, weapon, player.m_strikeAimRatio, 1);
					}
				}
			}
			break;
		}
		case WEAPON_1H: { // 1HANDED PLAYER MANAGEMENT
			// Waiting and Received Strike Impulse
			if(layer1.cur_anim == alist[ANIM_1H_WAIT]) {
				player.m_aimTime = 0;
				if(eeMousePressed1()) {
					changeAnimation(io, 1, alist[ANIM_1H_STRIKE_LEFT_START + player.m_strikeDirection * 3]);
					io->isHit = false;
				}
			}
			
			// Now go for strike cycle...
			for(long j = 0; j < 4; j++) {
				if(layer1.cur_anim == alist[ANIM_1H_STRIKE_LEFT_START + j * 3] && (layer1.flags & EA_ANIMEND)) {
					changeAnimation(io, 1, alist[ANIM_1H_STRIKE_LEFT_CYCLE + j * 3], EA_LOOP);
					player.m_aimTime = PlatformDuration::ofRaw(1);
				} else if(layer1.cur_anim == alist[ANIM_1H_STRIKE_LEFT_CYCLE + j * 3] && !eeMousePressed1()) {
					changeAnimation(io, 1, alist[ANIM_1H_STRIKE_LEFT + j * 3]);
					strikeSpeak(io);
					SendIOScriptEvent(NULL, io, SM_STRIKE, "1h");
					player.m_strikeDirection = 0;
					player.m_aimTime = 0;
				} else if(layer1.cur_anim == alist[ANIM_1H_STRIKE_LEFT + j * 3]) {
					if(   layer1.ctime > layer1.currentAltAnim()->anim_time * 0.3f
					   && layer1.ctime < layer1.currentAltAnim()->anim_time * 0.7f
					) {
						Entity * weapon = entities[player.equiped[EQUIP_SLOT_WEAPON]];
						
						if(player.m_weaponBlocked == AnimationDuration::ofRaw(-1)
							&& ARX_EQUIPMENT_Strike_Check(io, weapon, player.m_strikeAimRatio, 0))
						{
							player.m_weaponBlocked = layer1.ctime;
						}
					}
					
					if(layer1.flags & EA_ANIMEND) {
						changeAnimation(io, 1, alist[ANIM_1H_WAIT], EA_LOOP);
						layer1.flags &= ~(EA_PAUSED | EA_REVERSE);
						player.m_strikeDirection = 0;
						player.m_aimTime = 0;
						player.m_weaponBlocked = AnimationDuration::ofRaw(-1);
					}
					
					if(   player.m_weaponBlocked != AnimationDuration::ofRaw(-1)
					   && layer1.ctime < layer1.currentAltAnim()->anim_time * 0.9f
					) {
						Entity * weapon = entities[player.equiped[EQUIP_SLOT_WEAPON]];
						ARX_EQUIPMENT_Strike_Check(io, weapon, player.m_strikeAimRatio, 1);
					}
				}
			}
			break;
		}
		case WEAPON_2H: { // 2HANDED PLAYER MANAGEMENT
			// Waiting and Receiving Strike Impulse
			if(layer1.cur_anim == alist[ANIM_2H_WAIT]) {
				player.m_aimTime = 0;
				if(eeMousePressed1()) {
					changeAnimation(io, 1, alist[ANIM_2H_STRIKE_LEFT_START + player.m_strikeDirection * 3]);
					io->isHit = false;
				}
			}
			
			// Now go for strike cycle...
			for(long j = 0; j < 4; j++) {
				if(layer1.cur_anim == alist[ANIM_2H_STRIKE_LEFT_START + j * 3] && (layer1.flags & EA_ANIMEND)) {
					changeAnimation(io, 1, alist[ANIM_2H_STRIKE_LEFT_CYCLE + j * 3], EA_LOOP);
					player.m_aimTime = PlatformDuration::ofRaw(1);
				} else if(layer1.cur_anim == alist[ANIM_2H_STRIKE_LEFT_CYCLE + j * 3] && !eeMousePressed1()) {
					changeAnimation(io, 1, alist[ANIM_2H_STRIKE_LEFT + j * 3]);
					strikeSpeak(io);
					SendIOScriptEvent(NULL, io, SM_STRIKE, "2h");
					player.m_strikeDirection = 0;
					player.m_aimTime = 0;
				} else if(layer1.cur_anim == alist[ANIM_2H_STRIKE_LEFT + j * 3]) {
					if(   layer1.ctime > layer1.currentAltAnim()->anim_time * 0.3f
					   && layer1.ctime < layer1.currentAltAnim()->anim_time * 0.7f
					) {
						Entity * weapon = entities[player.equiped[EQUIP_SLOT_WEAPON]];
						
						if(player.m_weaponBlocked == AnimationDuration::ofRaw(-1)
							&& ARX_EQUIPMENT_Strike_Check(io, weapon, player.m_strikeAimRatio, 0))
						{
							player.m_weaponBlocked = layer1.ctime;
						}
					}
					
					if(layer1.flags & EA_ANIMEND) {
						changeAnimation(io, 1, alist[ANIM_2H_WAIT], EA_LOOP);
						layer1.flags &= ~(EA_PAUSED | EA_REVERSE);
						player.m_strikeDirection = 0;
						player.m_aimTime = 0;
						player.m_weaponBlocked = AnimationDuration::ofRaw(-1);
					}
					
					if(   player.m_weaponBlocked != AnimationDuration::ofRaw(-1)
					   && layer1.ctime < layer1.currentAltAnim()->anim_time * 0.9f
					) {
						Entity * weapon = entities[player.equiped[EQUIP_SLOT_WEAPON]];
						ARX_EQUIPMENT_Strike_Check(io, weapon, player.m_strikeAimRatio, 1);
					}
				}
			}
			break;
		}
		case WEAPON_BOW: { // MISSILE PLAYER MANAGEMENT
			if(layer1.cur_anim == alist[ANIM_MISSILE_STRIKE_CYCLE]) {
				player.m_bowAimRatio += bowZoomFromDuration(toMs(g_platformTime.lastFrameDuration()));
				
				if(player.m_bowAimRatio > 1.f)
					player.m_bowAimRatio = 1.f;
			}
			
			// Waiting and Receiving Strike Impulse
			if(layer1.cur_anim == alist[ANIM_MISSILE_WAIT]) {
				player.m_aimTime = PlatformDuration::ofRaw(1);
				
				if(eeMousePressed1() && Player_Arrow_Count() > 0) {
					changeAnimation(io, 1, alist[ANIM_MISSILE_STRIKE_PART_1]);
					io->isHit = false;
				}
			}
			
			if(layer1.cur_anim == alist[ANIM_MISSILE_STRIKE_PART_1] && (layer1.flags & EA_ANIMEND)) {
				player.m_aimTime = 0;
				changeAnimation(io, 1, alist[ANIM_MISSILE_STRIKE_PART_2]);
				EERIE_LINKEDOBJ_LinkObjectToObject(io->obj, arrowobj, "left_attach", "attach", NULL);
			}
			
			// Now go for strike cycle...
			if(layer1.cur_anim == alist[ANIM_MISSILE_STRIKE_PART_2] && (layer1.flags & EA_ANIMEND)) {
				changeAnimation(io, 1, alist[ANIM_MISSILE_STRIKE_CYCLE], EA_LOOP);
				player.m_aimTime = PlatformDuration::ofRaw(1);
			} else if(layer1.cur_anim == alist[ANIM_MISSILE_STRIKE_CYCLE] && !eeMousePressed1()) {
				EERIE_LINKEDOBJ_UnLinkObjectFromObject(io->obj, arrowobj);
				changeAnimation(io, 1, alist[ANIM_MISSILE_STRIKE]);
				SendIOScriptEvent(NULL, io, SM_STRIKE, "bow");
				StrikeAimtime();
				player.m_strikeAimRatio = player.m_bowAimRatio;
				Entity * quiver = Player_Arrow_Count_Decrease();
				float poisonous = 0.f;
				
				if(quiver) {
					poisonous = quiver->poisonous;
					if(quiver->poisonous_count > 0) {
						quiver->poisonous_count--;
						
						if(quiver->poisonous_count <= 0)
							quiver->poisonous = 0;
					}
					
					ARX_DAMAGES_DurabilityLoss(quiver, 1.f);
					
					// TODO is this needed ?, quivers seem to self destruct via script, but maybe not all
					if(ValidIOAddress(quiver) && quiver->durability <= 0.f) {
						ARX_INTERACTIVE_DestroyIOdelayed(quiver);
					}
				}
				
				float aimratio = player.m_strikeAimRatio;
				
				if(sp_max && poisonous < 3.f)
					poisonous = 3.f;
				
				Vec3f orgPos = player.pos + Vec3f(0.f, 40.f, 0.f);
				
				if(io->obj->fastaccess.left_attach != ActionPoint()) {
					orgPos = actionPointPosition(io->obj, io->obj->fastaccess.left_attach);
				}
				
				Anglef orgAngle = player.angle;
				
				PlayerLaunchArrow_Test(aimratio, poisonous, orgPos, orgAngle);
				
				if(sp_max) {
					Anglef angle;
					Vec3f pos = player.pos + Vec3f(0.f, 40.f, 0.f);
					
					angle.setPitch(player.angle.getPitch());
					angle.setYaw(player.angle.getYaw() + 8);
					angle.setRoll(player.angle.getRoll());
					PlayerLaunchArrow_Test(aimratio, poisonous, pos, angle);
					angle.setPitch(player.angle.getPitch());
					angle.setYaw(player.angle.getYaw() - 8);
					PlayerLaunchArrow_Test(aimratio, poisonous, pos, angle);
					angle.setPitch(player.angle.getPitch());
					angle.setYaw(player.angle.getYaw() + 4.f);
					PlayerLaunchArrow_Test(aimratio, poisonous, pos, angle);
					angle.setPitch(player.angle.getPitch());
					angle.setYaw(player.angle.getYaw() - 4.f);
					PlayerLaunchArrow_Test(aimratio, poisonous, pos, angle);
				}
				
				player.m_aimTime = 0;
			} else if(layer1.cur_anim == alist[ANIM_MISSILE_STRIKE]) {
				player.m_bowAimRatio -= bowZoomFromDuration(toMs(g_platformTime.lastFrameDuration()));
				
				if(player.m_bowAimRatio < 0)
					player.m_bowAimRatio = 0;
				
				if(layer1.flags & EA_ANIMEND) {
					player.m_bowAimRatio = 0;
					changeAnimation(io, 1, alist[ANIM_MISSILE_WAIT], EA_LOOP);
					player.m_aimTime = 0;
					player.m_weaponBlocked = AnimationDuration::ofRaw(-1);
					EERIE_LINKEDOBJ_UnLinkObjectFromObject(io->obj, arrowobj);
				}
			}
			break;
		}
	}
	
	LAST_WEAPON_TYPE = weapontype;
}

void ManageCombatModeAnimationsEND() {
	
	Entity * io = entities.player();
	
	AnimLayer & layer1 = io->animlayer[1];
	AnimLayer & layer3 = io->animlayer[3];
	
	ANIM_HANDLE ** alist = io->anims;
	
	if(layer1.cur_anim
	   && (layer1.cur_anim == alist[ANIM_BARE_READY]
	       || layer1.cur_anim == alist[ANIM_DAGGER_READY_PART_2]
	       || layer1.cur_anim == alist[ANIM_DAGGER_READY_PART_1]
	       || layer1.cur_anim == alist[ANIM_1H_READY_PART_2]
	       || layer1.cur_anim == alist[ANIM_1H_READY_PART_1]
	       || layer1.cur_anim == alist[ANIM_2H_READY_PART_2]
	       || layer1.cur_anim == alist[ANIM_2H_READY_PART_1]
	       || layer1.cur_anim == alist[ANIM_MISSILE_READY_PART_1]
	       || layer1.cur_anim == alist[ANIM_MISSILE_READY_PART_2])) {
		player.m_aimTime = PlatformDuration::ofRaw(1);
	}
	
	if(layer1.flags & EA_ANIMEND) {
		
		WeaponType weapontype = ARX_EQUIPMENT_GetPlayerWeaponType();
		
		if(layer1.cur_anim
		   && (layer1.cur_anim == io->anims[ANIM_BARE_UNREADY]
		       || layer1.cur_anim == io->anims[ANIM_DAGGER_UNREADY_PART_2]
		       || layer1.cur_anim == io->anims[ANIM_1H_UNREADY_PART_2]
		       || layer1.cur_anim == io->anims[ANIM_2H_UNREADY_PART_2]
		       || layer1.cur_anim == io->anims[ANIM_MISSILE_UNREADY_PART_2])) {
			AcquireLastAnim(io);
			layer1.cur_anim = NULL;
		}
		
		switch(weapontype) {
			case WEAPON_BARE: {
				// Is Weapon Ready ? In this case go to Fight Wait anim
				if(layer1.cur_anim == alist[ANIM_BARE_READY]) {
					if(player.Interface & INTER_NO_STRIKE) {
						player.Interface &= ~INTER_NO_STRIKE;
						changeAnimation(io, 1, alist[ANIM_BARE_WAIT], EA_LOOP);
					} else {
						changeAnimation(io, 1, alist[ANIM_BARE_STRIKE_LEFT_START + player.m_strikeDirection * 3]);
					}
					player.m_aimTime = PlatformDuration::ofRaw(1);
					io->isHit = false;
				}
				break;
			}
			case WEAPON_DAGGER: { // DAGGER ANIMS end

				if(alist[ANIM_DAGGER_READY_PART_1]) {
					if(layer1.cur_anim == alist[ANIM_DAGGER_READY_PART_1]) {
						ARX_EQUIPMENT_AttachPlayerWeaponToHand();
						changeAnimation(io, 1, alist[ANIM_DAGGER_READY_PART_2]);
					} else if(layer1.cur_anim == alist[ANIM_DAGGER_READY_PART_2]) {
						if(player.Interface & INTER_NO_STRIKE) {
							player.Interface &= ~INTER_NO_STRIKE;
							changeAnimation(io, 1, alist[ANIM_DAGGER_WAIT], EA_LOOP);
						} else {
							changeAnimation(io, 1, alist[ANIM_DAGGER_STRIKE_LEFT_START + player.m_strikeDirection * 3]);
						}
						player.m_aimTime = PlatformDuration::ofRaw(1);
						io->isHit = false;
					} else if(layer1.cur_anim == alist[ANIM_DAGGER_UNREADY_PART_1]) {
						ARX_EQUIPMENT_AttachPlayerWeaponToBack();
						changeAnimation(io, 1, alist[ANIM_DAGGER_UNREADY_PART_2]);
					}
				}

			break;
			}
			case WEAPON_1H: { // 1H ANIMS end

				if(alist[ANIM_1H_READY_PART_1]) {
					if(layer1.cur_anim == alist[ANIM_1H_READY_PART_1]) {
						ARX_EQUIPMENT_AttachPlayerWeaponToHand();
						changeAnimation(io, 1, alist[ANIM_1H_READY_PART_2]);
					} else if(layer1.cur_anim == alist[ANIM_1H_READY_PART_2]) {
						if(player.Interface & INTER_NO_STRIKE) {
							player.Interface &= ~INTER_NO_STRIKE;
							changeAnimation(io, 1, alist[ANIM_1H_WAIT], EA_LOOP);
						} else {
							changeAnimation(io, 1, alist[ANIM_1H_STRIKE_LEFT_START + player.m_strikeDirection * 3]);
						}
						player.m_aimTime = PlatformDuration::ofRaw(1);
						io->isHit = false;
					} else if(layer1.cur_anim == alist[ANIM_1H_UNREADY_PART_1]) {
						ARX_EQUIPMENT_AttachPlayerWeaponToBack();
						changeAnimation(io, 1, alist[ANIM_1H_UNREADY_PART_2]);
					}
				}

			break;
			}
			case WEAPON_2H: { // 2H ANIMS end

				if(alist[ANIM_2H_READY_PART_1]) {
					if(layer1.cur_anim == alist[ANIM_2H_READY_PART_1]) {
						ARX_EQUIPMENT_AttachPlayerWeaponToHand();
						changeAnimation(io, 1, alist[ANIM_2H_READY_PART_2]);
					} else if(layer1.cur_anim == alist[ANIM_2H_READY_PART_2]) {
						if(player.Interface & INTER_NO_STRIKE) {
							player.Interface &= ~INTER_NO_STRIKE;
							changeAnimation(io, 1, alist[ANIM_2H_WAIT], EA_LOOP);
						} else {
							changeAnimation(io, 1, alist[ANIM_2H_STRIKE_LEFT_START + player.m_strikeDirection * 3]);
						}
						player.m_aimTime = PlatformDuration::ofRaw(1);
						io->isHit = false;
					} else if(layer1.cur_anim == alist[ANIM_2H_UNREADY_PART_1]) {
						ARX_EQUIPMENT_AttachPlayerWeaponToBack();
						changeAnimation(io, 1, alist[ANIM_2H_UNREADY_PART_2]);
					}
				}

			break;
			}
			case WEAPON_BOW: { // MISSILE Weapon ANIMS end

				if(alist[ANIM_MISSILE_READY_PART_1]) {
					if(layer1.cur_anim == alist[ANIM_MISSILE_READY_PART_1]) {
						ARX_EQUIPMENT_AttachPlayerWeaponToHand();
						changeAnimation(io, 1, alist[ANIM_MISSILE_READY_PART_2]);
					} else if(layer1.cur_anim == alist[ANIM_MISSILE_READY_PART_2]) {
						if(Player_Arrow_Count() > 0) {
							if(player.Interface & INTER_NO_STRIKE) {
								player.Interface &= ~INTER_NO_STRIKE;
								changeAnimation(io, 1, alist[ANIM_MISSILE_WAIT], EA_LOOP);
							} else {
								changeAnimation(io, 1, alist[ANIM_MISSILE_STRIKE_PART_1]);
							}
							io->isHit = false;
						} else {
							changeAnimation(io, 1, alist[ANIM_MISSILE_WAIT]);
						}
						EERIE_LINKEDOBJ_UnLinkObjectFromObject(io->obj, arrowobj);
					} else if(layer1.cur_anim == alist[ANIM_MISSILE_STRIKE_PART_1]) {
						// TODO why no AcquireLastAnim()?
						ANIM_Set(layer1, alist[ANIM_MISSILE_STRIKE_PART_2]);
					} else if(layer1.cur_anim == alist[ANIM_MISSILE_STRIKE_PART_2]) {
						// TODO why no AcquireLastAnim()?
						ANIM_Set(layer1, alist[ANIM_MISSILE_STRIKE_CYCLE]);
					} else if(layer1.cur_anim == alist[ANIM_MISSILE_UNREADY_PART_1]) {
						ARX_EQUIPMENT_AttachPlayerWeaponToBack();
						changeAnimation(io, 1, alist[ANIM_MISSILE_UNREADY_PART_2]);
					}
				}

			break;
			}
		}

		// Spell casting anims
		if(alist[ANIM_CAST] && layer1.cur_anim == alist[ANIM_CAST]) {
			if(alist[ANIM_CAST_END]) {
				changeAnimation(io, 1, alist[ANIM_CAST_END]);
			}
		} else if(alist[ANIM_CAST_END] && layer1.cur_anim == alist[ANIM_CAST_END]) {
			AcquireLastAnim(io);
			layer1.cur_anim = NULL;
			player.doingmagic = 0;

			if(WILLRETURNTOCOMBATMODE) {
				player.Interface |= INTER_COMBATMODE;
				player.Interface |= INTER_NO_STRIKE;

				ARX_EQUIPMENT_LaunchPlayerReadyWeapon();
				WILLRETURNTOCOMBATMODE = false;
			}
		}
	}

	// Is the shield off ?
	if(layer3.flags & EA_ANIMEND) {
		if(io->anims[ANIM_SHIELD_END] && layer3.cur_anim == io->anims[ANIM_SHIELD_END]) {
			AcquireLastAnim(io);
			layer3.cur_anim = NULL;
		}
	}
}

extern TextureContainer * ombrignon;

void DrawImproveVisionInterface() {
	
	if(ombrignon) {
		float mod = 0.6f + PULSATE * 0.35f;
		UseRenderState state(render2D().blendAdditive());
		EERIEDrawBitmap(Rectf(g_size), 0.0001f, ombrignon, Color::red * ((0.5f + PULSATE * 0.1f) * mod));
	}
	
}

void DANAE_StartNewQuest()
{
	benchmark::begin(benchmark::LoadLevel);
	player.Interface = INTER_LIFE_MANA | INTER_MINIBACK | INTER_MINIBOOK;
	progressBarSetTotal(108);
	progressBarReset();
	LoadLevelScreen(1);
	DONT_ERASE_PLAYER = true;
	DanaeClearLevel();
	progressBarAdvance(2.f);
	LoadLevelScreen();
	DanaeLoadLevel("graph/levels/level1/level1.dlf");
	START_NEW_QUEST = false;
	BLOCK_PLAYER_CONTROLS = false;
	fadeReset();
	player.Interface = INTER_LIFE_MANA | INTER_MINIBACK | INTER_MINIBOOK;
}
