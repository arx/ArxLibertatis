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
///////////////////////////////////////////////////////////////////////////////
//
// ARX_Sound.cpp
// ARX Sound Management
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//
///////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <list>

#include <ARX_SOUND.h>
#include <HERMESMain.h>
#include <HERMES_Pak.h>
#include <EERIEMath.h>
#include <ARX_NPC.h>
#include <ARX_Interactive.h>
#include <ARX_Player.h>
#include <ARX_Script.h>
#include <ARX_Particles.h>
#include <../danae/arx_menu2.h>
#include <Athena.h>

#include "eerieapp.h"

using namespace std;

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

using namespace ATHENA;

extern long FINAL_RELEASE;
extern long EXTERNALVIEW;
extern INTERACTIVE_OBJ * CAMERACONTROLLER;

extern CMenuConfig * pMenuConfig;

typedef struct
{
	char * name;
	unsigned long variant_i;
	unsigned long variant_c;
	long * variant_l;
} ARX_SOUND_Material;

typedef struct
{
	char * name;
	unsigned long material_c;
	ARX_SOUND_Material * material_l;
} ARX_SOUND_CollisionMap;

typedef struct
{
	char * name;
	long name_size;
	float factor;
} ARX_SOUND_Presence;

enum ParseIniFileEnum
{
	PARSE_INI_FILE_CONTINUE,
	PARSE_INI_FILE_SKIP,
	PARSE_INI_FILE_STOP
};

typedef unsigned long(* ParseIniFileCallback)(const char * lpszText);

static enum PlayingAmbianceType
{
	PLAYING_AMBIANCE_MENU,
	PLAYING_AMBIANCE_SCRIPT,
	PLAYING_AMBIANCE_ZONE
};

typedef struct
{
	char name[256];
	float volume;
	long loop;
	long type;
} PlayingAmbiance;

static const unsigned long ARX_SOUND_UPDATE_INTERVAL(100);  
static const unsigned long ARX_SOUND_STREAMING_LIMIT(2000); 
static const unsigned long MAX_MATERIALS(17);
static const unsigned long MAX_VARIANTS(5);
static const unsigned long AMBIANCE_FADE_TIME(2000);
static const float ARX_SOUND_UNIT_FACTOR(0.01F);
static const float ARX_SOUND_ROLLOFF_FACTOR(1.3F);
static const float ARX_SOUND_DEFAULT_FALLSTART(200.0F);

static const float ARX_SOUND_DEFAULT_FALLEND(2200.0F);
static const float ARX_SOUND_REFUSE_DISTANCE(2500.0F);

static const char ARX_SOUND_PATH_INI[] = "localisation\\";
static const char ARX_SOUND_PATH_SAMPLE[] = "sfx\\";
static const char ARX_SOUND_PATH_AMBIANCE[] = "sfx\\ambiance\\";
static const char ARX_SOUND_PATH_ENVIRONMENT[] = "sfx\\environment\\";
static const char ARX_SOUND_PRESENCE_NAME[] = "presence";
static const char ARX_SOUND_FILE_EXTENSION_WAV[] = ".wav";
static const char ARX_SOUND_FILE_EXTENSION_INI[] = ".ini";

static const unsigned long ARX_SOUND_COLLISION_MAP_COUNT = 3;
static const char * ARX_SOUND_COLLISION_MAP_NAME[] =
{
	"snd_armor",
	"snd_step",
	"snd_weapon"
};

static bool bIsActive(false);
static HANDLE hUpdateThread(NULL);
static bool bExitUpdateThread(false);


static long ambiance_zone(AAL_SFALSE);
static long ambiance_menu(AAL_SFALSE);

static long Inter_Materials[MAX_MATERIALS][MAX_MATERIALS][MAX_VARIANTS];
static unsigned long collision_map_c(0);
static ARX_SOUND_CollisionMap * collision_map_l = NULL;

static unsigned long presence_c(0);
static ARX_SOUND_Presence * presence_l = NULL;

 

// ARX mixers
long ARX_SOUND_MixerGame(AAL_SFALSE);
long ARX_SOUND_MixerGameSample(AAL_SFALSE);
long ARX_SOUND_MixerGameSpeech(AAL_SFALSE);
long ARX_SOUND_MixerGameAmbiance(AAL_SFALSE);
long ARX_SOUND_MixerMenu(AAL_SFALSE);
long ARX_SOUND_MixerMenuSample(AAL_SFALSE);
long ARX_SOUND_MixerMenuSpeech(AAL_SFALSE);
long ARX_SOUND_MixerMenuAmbiance(AAL_SFALSE);

// Menu ambiances
char AMB_MENU[] = "ambient_menu.amb";
char AMB_CREDITS[] = "ambient_credits.amb";

// Menu samples
long SND_MENU_CLICK(AAL_SFALSE);
long SND_MENU_CREDITS_LOOP(AAL_SFALSE);
long SND_MENU_LOOP(AAL_SFALSE);
long SND_MENU_OPTIONS_LOOP(AAL_SFALSE);
long SND_MENU_PUSH(AAL_SFALSE);
long SND_MENU_RELEASE(AAL_SFALSE);

// Interface samples
long SND_BACKPACK(AAL_SFALSE);
long SND_BOOK_OPEN(AAL_SFALSE);
long SND_BOOK_CLOSE(AAL_SFALSE);
long SND_BOOK_PAGE_TURN(AAL_SFALSE);
long SND_GOLD(AAL_SFALSE);
long SND_INVSTD(AAL_SFALSE);
long SND_MAP(AAL_SFALSE);
long SND_SCROLL_OPEN(AAL_SFALSE);
long SND_SCROLL_CLOSE(AAL_SFALSE);
long SND_TORCH_START(AAL_SFALSE);
long SND_TORCH_LOOP(AAL_SFALSE);
long SND_TORCH_END(AAL_SFALSE);

// Other SFX samples
long SND_FIREPLACE(AAL_SFALSE);
long SND_PLOUF(AAL_SFALSE);
long SND_QUAKE(AAL_SFALSE);
long SND_WHOOSH(AAL_SFALSE);

// Player samples
long SND_PLAYER_DEATH(AAL_SFALSE);
long SND_PLAYER_DEATH_BY_FIRE(AAL_SFALSE);

long SND_PLAYER_FILLLIFEMANA(AAL_SFALSE);
long SND_PLAYER_HEART_BEAT(AAL_SFALSE);
long SND_PLAYER_JUMP(AAL_SFALSE);
long SND_PLAYER_JUMP_END(AAL_SFALSE);
long SND_PLAYER_LEVEL_UP(AAL_SFALSE);
long SND_PLAYER_POISONED(AAL_SFALSE);

// Magic drawing samples
long SND_MAGIC_AMBIENT(AAL_SFALSE);
long SND_MAGIC_DRAW(AAL_SFALSE);
long SND_MAGIC_FIZZLE(AAL_SFALSE);

// Magic symbols samples
long SND_SYMB_AAM(AAL_SFALSE);
long SND_SYMB_CETRIUS(AAL_SFALSE);
long SND_SYMB_COSUM(AAL_SFALSE);
long SND_SYMB_COMUNICATUM(AAL_SFALSE);
long SND_SYMB_FOLGORA(AAL_SFALSE);
long SND_SYMB_FRIDD(AAL_SFALSE);
long SND_SYMB_KAOM(AAL_SFALSE);
long SND_SYMB_MEGA(AAL_SFALSE);
long SND_SYMB_MORTE(AAL_SFALSE);
long SND_SYMB_MOVIS(AAL_SFALSE);
long SND_SYMB_NHI(AAL_SFALSE);
long SND_SYMB_RHAA(AAL_SFALSE);
long SND_SYMB_SPACIUM(AAL_SFALSE);
long SND_SYMB_STREGUM(AAL_SFALSE);
long SND_SYMB_TAAR(AAL_SFALSE);
long SND_SYMB_TEMPUS(AAL_SFALSE);
long SND_SYMB_TERA(AAL_SFALSE);
long SND_SYMB_VISTA(AAL_SFALSE);
long SND_SYMB_VITAE(AAL_SFALSE);
long SND_SYMB_YOK(AAL_SFALSE);

// Spells samples
long SND_SPELL_ACTIVATE_PORTAL(AAL_SFALSE);
long SND_SPELL_ARMOR_START(AAL_SFALSE);
long SND_SPELL_ARMOR_END(AAL_SFALSE);
long SND_SPELL_ARMOR_LOOP(AAL_SFALSE);
long SND_SPELL_LOWER_ARMOR(AAL_SFALSE);
long SND_SPELL_BLESS(AAL_SFALSE);
long SND_SPELL_COLD_PROTECTION_START(AAL_SFALSE);
long SND_SPELL_COLD_PROTECTION_LOOP(AAL_SFALSE);
long SND_SPELL_COLD_PROTECTION_END(AAL_SFALSE);
long SND_SPELL_CONFUSE(AAL_SFALSE);
long SND_SPELL_CONTROL_TARGET(AAL_SFALSE);
long SND_SPELL_CREATE_FIELD(AAL_SFALSE);
long SND_SPELL_CREATE_FOOD(AAL_SFALSE);
long SND_SPELL_CURE_POISON(AAL_SFALSE);
long SND_SPELL_CURSE(AAL_SFALSE);
long SND_SPELL_DETECT_TRAP(AAL_SFALSE);
long SND_SPELL_DETECT_TRAP_LOOP(AAL_SFALSE);
long SND_SPELL_DISARM_TRAP(AAL_SFALSE);
long SND_SPELL_DISPELL_FIELD(AAL_SFALSE);
long SND_SPELL_DISPELL_ILLUSION(AAL_SFALSE);
long SND_SPELL_DOUSE(AAL_SFALSE);
long SND_SPELL_ELECTRIC(AAL_SFALSE);
long SND_SPELL_ENCHANT_WEAPON(AAL_SFALSE);
long SND_SPELL_EXPLOSION(AAL_SFALSE);
long SND_SPELL_EYEBALL_IN(AAL_SFALSE);
long SND_SPELL_EYEBALL_OUT(AAL_SFALSE);
long SND_SPELL_FIRE_FIELD(AAL_SFALSE);
long SND_SPELL_FIRE_HIT(AAL_SFALSE);
long SND_SPELL_FIRE_LAUNCH(AAL_SFALSE);
long SND_SPELL_FIRE_PROTECTION(AAL_SFALSE);
long SND_SPELL_FIRE_WIND(AAL_SFALSE);
long SND_SPELL_FREEZETIME(AAL_SFALSE);
long SND_SPELL_HARM(AAL_SFALSE);
long SND_SPELL_HEALING(AAL_SFALSE);
long SND_SPELL_ICE_FIELD(AAL_SFALSE);
long SND_SPELL_ICE_PROJECTILE_LAUNCH(AAL_SFALSE);
long SND_SPELL_INCINERATE(AAL_SFALSE);
long SND_SPELL_IGNITE(AAL_SFALSE);
long SND_SPELL_INVISIBILITY_START(AAL_SFALSE);
long SND_SPELL_INVISIBILITY_END(AAL_SFALSE);
long SND_SPELL_LEVITATE_START(AAL_SFALSE);
long SND_SPELL_LIGHTNING_START(AAL_SFALSE);
long SND_SPELL_LIGHTNING_LOOP(AAL_SFALSE);
long SND_SPELL_LIGHTNING_END(AAL_SFALSE);
long SND_SPELL_MAGICAL_HIT(AAL_SFALSE);

long SND_SPELL_MASS_LIGHTNING_END(AAL_SFALSE);
long SND_SPELL_FIRE_FIELD_START(AAL_SFALSE);
long SND_SPELL_FIRE_FIELD_LOOP(AAL_SFALSE);
long SND_SPELL_FIRE_FIELD_END(AAL_SFALSE);


long SND_SPELL_MAGICAL_SHIELD(AAL_SFALSE);
long SND_SPELL_MASS_INCINERATE(AAL_SFALSE);
long SND_SPELL_MASS_PARALYSE(AAL_SFALSE);
long SND_SPELL_MM_CREATE(AAL_SFALSE);
long SND_SPELL_MM_HIT(AAL_SFALSE);
long SND_SPELL_MM_LAUNCH(AAL_SFALSE);
long SND_SPELL_MM_LOOP(AAL_SFALSE);
long SND_SPELL_NEGATE_MAGIC(AAL_SFALSE);
long SND_SPELL_NO_EFFECT(AAL_SFALSE);
long SND_SPELL_PARALYSE(AAL_SFALSE);
long SND_SPELL_PARALYSE_END(AAL_SFALSE);
long SND_SPELL_POISON_PROJECTILE_LAUNCH(AAL_SFALSE);
long SND_SPELL_RAISE_DEAD(AAL_SFALSE);
long SND_SPELL_REPEL_UNDEAD(AAL_SFALSE);
long SND_SPELL_REPEL_UNDEAD_LOOP(AAL_SFALSE);
long SND_SPELL_RUNE_OF_GUARDING(AAL_SFALSE);
long SND_SPELL_SLOW_DOWN(AAL_SFALSE);
long SND_SPELL_SPARK(AAL_SFALSE);
long SND_SPELL_SPEED_START(AAL_SFALSE);
long SND_SPELL_SPEED_LOOP(AAL_SFALSE);
long SND_SPELL_SPEED_END(AAL_SFALSE);
long SND_SPELL_SUMMON_CREATURE(AAL_SFALSE);
long SND_SPELL_TELEKINESIS_START(AAL_SFALSE);
long SND_SPELL_TELEKINESIS_END(AAL_SFALSE);
long SND_SPELL_TELEPORT(AAL_SFALSE);
long SND_SPELL_TELEPORTED(AAL_SFALSE);
long SND_SPELL_VISION_START(AAL_SFALSE);
long SND_SPELL_VISION_LOOP(AAL_SFALSE);

bool bForceNoEAX = false;

static void ARX_SOUND_CreateEnvironment();
static void ARX_SOUND_CreateEnvironments();
static void ARX_SOUND_CreateStaticSamples();
static void ARX_SOUND_ReleaseStaticSamples();
static void ARX_SOUND_LoadCollision(const long & mat1, const long & mat2, const char * name);
static void ARX_SOUND_CreateCollisionMap(const char * file_name);
static void ARX_SOUND_CreateCollisionMaps();
static void ARX_SOUND_DeleteCollisionMaps();
static void ARX_SOUND_CreateMaterials();
static void ARX_SOUND_CreatePresenceMap();
static void ARX_SOUND_DeletePresenceMap();
static float GetSamplePresenceFactor(const char * name);
LPTHREAD_START_ROUTINE UpdateSoundThread(char *);
static void ARX_SOUND_LaunchUpdateThread();
static void ARX_SOUND_KillUpdateThread();
static void ARX_SOUND_ParseIniFile(char * _lpszTextFile, const unsigned long _ulFileSize, ParseIniFileCallback lpSectionCallback, ParseIniFileCallback lpStringCallback);
void ARX_SOUND_PreloadAll();

extern char pStringModSfx[];
extern char pStringModSpeech[];

long ARX_SOUND_Init(HWND hwnd)
{
	if (bIsActive) ARX_SOUND_Release();

	if ((bForceNoEAX) ||
	        (pMenuConfig && (!pMenuConfig->bEAX)))
	{
		if (aalInitForceNoEAX(hwnd) || aalEnable(AAL_FLAG_MULTITHREAD))
		{
			aalClean();
			return -1;
		}
	}
	else
	{
		if (aalInit(hwnd) || aalEnable(AAL_FLAG_MULTITHREAD))
		{
			aalClean();
			return -1;
		}
	}

	if (aalSetRootPath(Project.workingdir) ||
	        aalSetSamplePath(ARX_SOUND_PATH_SAMPLE) ||
	        aalSetAmbiancePath(ARX_SOUND_PATH_AMBIANCE) ||
	        aalSetEnvironmentPath(ARX_SOUND_PATH_ENVIRONMENT))
	{
		aalClean();
		return -1;
	}

	// Create game mixers
	ARX_SOUND_MixerGame = aalCreateMixer();
	ARX_SOUND_MixerGameSample = aalCreateMixer();
	aalSetMixerParent(ARX_SOUND_MixerGameSample, ARX_SOUND_MixerGame);
	ARX_SOUND_MixerGameSpeech = aalCreateMixer();
	aalSetMixerParent(ARX_SOUND_MixerGameSpeech, ARX_SOUND_MixerGame);
	ARX_SOUND_MixerGameAmbiance = aalCreateMixer();
	aalSetMixerParent(ARX_SOUND_MixerGameAmbiance, ARX_SOUND_MixerGame);

	// Create menu mixers
	ARX_SOUND_MixerMenu = aalCreateMixer();
	ARX_SOUND_MixerMenuSample = aalCreateMixer();
	aalSetMixerParent(ARX_SOUND_MixerMenuSample, ARX_SOUND_MixerMenu);
	ARX_SOUND_MixerMenuSpeech = aalCreateMixer();
	aalSetMixerParent(ARX_SOUND_MixerMenuSpeech, ARX_SOUND_MixerMenu);
	ARX_SOUND_MixerMenuAmbiance = aalCreateMixer();
	aalSetMixerParent(ARX_SOUND_MixerMenuAmbiance, ARX_SOUND_MixerMenu);

	if ((ARX_SOUND_MixerGame == AAL_SFALSE) ||
	        (ARX_SOUND_MixerGameSample == AAL_SFALSE) ||
	        (ARX_SOUND_MixerGameSpeech == AAL_SFALSE) ||
	        (ARX_SOUND_MixerGameAmbiance == AAL_SFALSE) ||
	        (ARX_SOUND_MixerMenu == AAL_SFALSE) ||
	        (ARX_SOUND_MixerMenuSample == AAL_SFALSE) ||
	        (ARX_SOUND_MixerMenuSpeech == AAL_SFALSE) ||
	        (ARX_SOUND_MixerMenuAmbiance == AAL_SFALSE))
	{
		aalClean();
		return -1;
	}

	// Standard Mixer: 22050 16 Stereo
	aalFormat af;

	af.frequency = 22050;
	af.quality = 16;
	af.channels = 2;
	aalSetOutputFormat(af);

	// Enable 3D positionning system
	aalEnable(AAL_FLAG_POSITION);

	aalSetStreamLimit(ARX_SOUND_STREAMING_LIMIT);

	aalSetListenerUnitFactor(ARX_SOUND_UNIT_FACTOR);
	aalSetListenerRolloffFactor(ARX_SOUND_ROLLOFF_FACTOR);

	if (FINAL_RELEASE)
	{
		char pakfile[256];

		aalEnable(AAL_FLAG_PACKEDRESOURCES);

		if (pStringModSfx[0])
		{
			sprintf(pakfile, "%s%s", Project.workingdir, pStringModSfx);

			if (FileExist(pakfile)) aalAddResourcePack(pakfile);
		}

		if (pStringModSpeech[0])
		{
			sprintf(pakfile, "%s%s", Project.workingdir, pStringModSpeech);

			if (FileExist(pakfile)) aalAddResourcePack(pakfile);
		}

		sprintf(pakfile, "%ssfx.pak", Project.workingdir);

		if (FileExist(pakfile)) aalAddResourcePack(pakfile);
		else
		{
			MessageBox(NULL, "Unable to Find Data File\nPlease Reinstall ARX Fatalis", "Arx Fatalis - Error", MB_ICONEXCLAMATION | MB_OK);
			exit(0);
		}

		sprintf(pakfile, "%sspeech.pak", Project.workingdir);

		if (FileExist(pakfile))
		{
			aalAddResourcePack(pakfile);
		}
		else
		{
			sprintf(pakfile, "%sspeech_default.pak", Project.workingdir);

			if (FileExist(pakfile))
			{
				aalAddResourcePack(pakfile);
			}
			else
			{
				MessageBox(NULL, "Unable to Find Data File\nPlease Reinstall ARX Fatalis", "Arx Fatalis - Error", MB_ICONEXCLAMATION | MB_OK);
				exit(0);
			}
		}
	}

	// Load samples
	ARX_SOUND_CreateStaticSamples();
	ARX_SOUND_CreateMaterials();
	ARX_SOUND_CreateCollisionMaps();
	ARX_SOUND_CreatePresenceMap();

	// Load environments, enable environment system and set default one if required
	ARX_SOUND_CreateEnvironments();

	if (Project.soundmode & ARX_SOUND_REVERB)
	{
		aalEnable(AAL_FLAG_REVERBERATION);
		ARX_SOUND_EnvironmentSet("alley.aef");
	}

	ARX_SOUND_LaunchUpdateThread();

	bIsActive = true;
	ARX_SOUND_PreloadAll();

	return 0;
}
 
void ARX_SOUND_PreloadAll()
{

}


void ARX_SOUND_Release()
{
	ARX_SOUND_ReleaseStaticSamples();
	ARX_SOUND_DeleteCollisionMaps();
	ARX_SOUND_DeletePresenceMap();
	ARX_SOUND_KillUpdateThread();
	aalClean();
	bIsActive = false;
	Project.soundmode &= ~(ARX_SOUND_ON | ARX_SOUND_REVERB);
}

long ARX_SOUND_IsEnabled()
{
	return bIsActive ? 1 : 0;
}

void ARX_SOUND_EnableReverb(const long & status)
{
	if (bIsActive)
	{
		if (status)
		{
			if (aalEnable(AAL_FLAG_REVERBERATION))
				Project.soundmode &= ~ARX_SOUND_REVERB;
			else
				Project.soundmode |= ARX_SOUND_REVERB;
		}
		else
		{
			aalDisable(AAL_FLAG_REVERBERATION);
			Project.soundmode &= ~ARX_SOUND_REVERB;
		}
	}
}

long ARX_SOUND_IsReverbEnabled()
{
	if (!bIsActive) return 0;

	return aalIsEnabled(AAL_FLAG_REVERBERATION);
}

void ARX_SOUND_MixerSetVolume(const long & mixer_id, const float & volume)
{
	if (bIsActive) aalSetMixerVolume(mixer_id, volume);
}

float ARX_SOUND_MixerGetVolume(const long & mixer_id)
{
	float volume(0.0F);

	if (bIsActive) aalGetMixerVolume(mixer_id, &volume);

	return volume;
}

void ARX_SOUND_MixerStop(const long & mixer_id)
{
	if (bIsActive) aalMixerStop(mixer_id);
}

void ARX_SOUND_MixerPause(const long & mixer_id)
{
	if (bIsActive) aalMixerPause(mixer_id);
}

void ARX_SOUND_MixerResume(const long & mixer_id)
{
	if (bIsActive) aalMixerResume(mixer_id);
}

void ARX_SOUND_MixerSwitch(const long & from, const long & to)
{
	ARX_SOUND_MixerPause(from);
	ARX_SOUND_MixerSetVolume(to, ARX_SOUND_MixerGetVolume(from));
	ARX_SOUND_MixerSetVolume(to, ARX_SOUND_MixerGetVolume(from));
	ARX_SOUND_MixerSetVolume(to, ARX_SOUND_MixerGetVolume(from));
	ARX_SOUND_MixerResume(to);
}

// Sets the position of the listener
void ARX_SOUND_SetListener(const EERIE_3D * position, const EERIE_3D * front, const EERIE_3D * up)
{
	if (bIsActive)
	{
		aalSetListenerPosition(*(aalVector *)position);
		aalSetListenerDirection(*(aalVector *)front, *(aalVector *)up);
	}
}

void ARX_SOUND_EnvironmentSet(const char * name)
{
	if (bIsActive)
	{
		aalSLong e_id(aalGetEnvironment(name));

		if (e_id != AAL_SFALSE)
		{
			aalSetListenerEnvironment(e_id);
			aalSetEnvironmentRolloffFactor(e_id, ARX_SOUND_ROLLOFF_FACTOR);
		}
	}
}

long ARX_SOUND_PlaySFX(long & sample_id, const EERIE_3D * position, const float & pitch, const long & loop)
{
	if (!bIsActive || sample_id == AAL_SFALSE) return AAL_SFALSE;

	aalChannel channel;
	char sample_name[256];
	float presence;

	channel.mixer = ARX_SOUND_MixerGameSample;
	channel.flags = AAL_FLAG_VOLUME | AAL_FLAG_POSITION | AAL_FLAG_REVERBERATION | AAL_FLAG_FALLOFF;
	channel.volume = 1.0F;

	if (position)
	{
		if (ACTIVECAM && EEDistance3D(&ACTIVECAM->pos, position) > ARX_SOUND_REFUSE_DISTANCE)
			return -1;
	}

	aalGetSampleName(sample_id, sample_name);
	presence = GetSamplePresenceFactor(sample_name);
	channel.falloff.start = ARX_SOUND_DEFAULT_FALLSTART * presence;
	channel.falloff.end = ARX_SOUND_DEFAULT_FALLEND * presence;

	if (pitch != 1.0F)
	{
		channel.flags |= AAL_FLAG_PITCH;
		channel.pitch = pitch;
	}

	if (position)
	{
		channel.position.x = position->x;
		channel.position.y = position->y;
		channel.position.z = position->z;
	}
	else
	{
		channel.flags |= AAL_FLAG_RELATIVE;
		channel.position.x = 0.0F;
		channel.position.y = 0.0F;
		channel.position.z = 1.0F;
	}

	aalSamplePlay(sample_id, channel, loop);

	return sample_id;
}


long ARX_SOUND_PlayInterface(long & sample_id, const float & pitch, const long & loop)
{
	if (!bIsActive || sample_id == AAL_SFALSE) return AAL_SFALSE;

	aalChannel channel;

	channel.mixer = ARX_SOUND_MixerGameSample;
	channel.flags = AAL_FLAG_VOLUME;
	channel.volume = 1.0F;

	if (pitch != 1.0F) channel.flags |= AAL_FLAG_PITCH, channel.pitch = pitch;

	aalSamplePlay(sample_id, channel, loop);

	return sample_id;
}

long ARX_SOUND_PlayMenu(long & sample_id, const float & pitch, const long & loop)
{
	if (!bIsActive || sample_id == AAL_SFALSE) return AAL_SFALSE;

	aalChannel channel;

	channel.mixer = ARX_SOUND_MixerMenuSample;
	channel.flags = AAL_FLAG_VOLUME;
	channel.volume = 1.0F;

	if (pitch != 1.0F) channel.flags |= AAL_FLAG_PITCH, channel.pitch = pitch;

	aalSamplePlay(sample_id, channel, loop);

	return sample_id;
}


void ARX_SOUND_IOFrontPos(const INTERACTIVE_OBJ * io, aalVector & pos)
{
	if (io)
	{
		pos.x = io->pos.x - EEsin(DEG2RAD(MAKEANGLE(io->angle.b))) * 100.0F;
		pos.y = io->pos.y - 100.0F;
		pos.z = io->pos.z + EEcos(DEG2RAD(MAKEANGLE(io->angle.b))) * 100.0F;
	}
	else if (ACTIVECAM)
	{
		pos.x = ACTIVECAM->pos.x - EEsin(DEG2RAD(MAKEANGLE(ACTIVECAM->angle.b))) * 100.0F;
		pos.y = ACTIVECAM->pos.y - 100.0F;
		pos.z = ACTIVECAM->pos.z + EEcos(DEG2RAD(MAKEANGLE(ACTIVECAM->angle.b))) * 100.0F;
	}
	else
	{
		Vector_Init((EERIE_3D *)&pos);
	}
}

long ARX_SOUND_PlaySpeech(const char * name, const INTERACTIVE_OBJ * io)
{
	if (!bIsActive) return AAL_SFALSE;

	char file_name[256];
	aalChannel channel;
	aalSLong sample_id;

	sprintf(file_name, "speech\\%s\\%s.wav", Project.localisationpath, name);

	sample_id = aalCreateSample(file_name);

	channel.mixer = ARX_SOUND_MixerGameSpeech;
	channel.flags = AAL_FLAG_VOLUME | AAL_FLAG_POSITION | AAL_FLAG_REVERBERATION | AAL_FLAG_AUTOFREE | AAL_FLAG_FALLOFF;
	channel.volume = 1.0F;
	channel.falloff.start = ARX_SOUND_DEFAULT_FALLSTART;
	channel.falloff.end = ARX_SOUND_DEFAULT_FALLEND;

	if (io)
	{
		if (((io == inter.iobj[0]) && !EXTERNALVIEW) ||
		        (io->ioflags & IO_CAMERA && io == CAMERACONTROLLER))
			ARX_SOUND_IOFrontPos(io, channel.position);
		else
		{
			channel.position.x = io->pos.x;
			channel.position.y = io->pos.y;
			channel.position.z = io->pos.z;
		}

		if (ACTIVECAM && EEDistance3D(&ACTIVECAM->pos, &io->pos) > ARX_SOUND_REFUSE_DISTANCE)
			return -1;

		if (io->ioflags & IO_NPC && io->_npcdata->speakpitch != 1.0F)
		{
			channel.flags |= AAL_FLAG_PITCH;
			channel.pitch = io->_npcdata->speakpitch;
		}

	}
	else
	{
		channel.flags |= AAL_FLAG_RELATIVE;
		channel.position.x = 0.0F;
		channel.position.y = 0.0F;
		channel.position.z = 100.0F;
	}

	aalSamplePlay(sample_id, channel);

	return sample_id;
}

long ARX_SOUND_PlayCollision(const long & mat1, const long & mat2, const float & volume, const float & power, EERIE_3D * position, INTERACTIVE_OBJ * source)
{
	if (!bIsActive) return 0;

	if (mat1 == MATERIAL_NONE || mat2 == MATERIAL_NONE) return 0;

	if (mat1 == MATERIAL_WATER || mat2 == MATERIAL_WATER)
		ARX_PARTICLES_SpawnWaterSplash(position);

	long sample_id;

	sample_id = Inter_Materials[mat1][mat2][0];

	if (sample_id == AAL_SFALSE) return 0;

	aalChannel channel;
	char sample_name[256];
	float presence;

	channel.mixer = ARX_SOUND_MixerGameSample;

	channel.flags = AAL_FLAG_VOLUME | AAL_FLAG_PITCH | AAL_FLAG_POSITION | AAL_FLAG_REVERBERATION | AAL_FLAG_FALLOFF;

	aalGetSampleName(sample_id, sample_name);
	presence = GetSamplePresenceFactor(sample_name);
	channel.falloff.start = ARX_SOUND_DEFAULT_FALLSTART * presence;
	channel.falloff.end = ARX_SOUND_DEFAULT_FALLEND * presence;

	if (position)
	{
		if (ACTIVECAM && EEDistance3D(&ACTIVECAM->pos, position) > ARX_SOUND_REFUSE_DISTANCE)
			return -1;
	}

	//Launch 'ON HEAR' script event
	ARX_NPC_SpawnAudibleSound(position, source, power, presence);

	if (position)
	{
		channel.position.x = position->x;
		channel.position.y = position->y;
		channel.position.z = position->z;
	}
	else ARX_PLAYER_FrontPos((EERIE_3D *)&channel.position);

	channel.pitch = 0.9F + 0.2F * rnd();
	channel.volume = volume;
	aalSamplePlay(sample_id, channel);

	unsigned long length;
	aalGetSampleLength(sample_id, length);

	return (long)(channel.pitch * length);
}

long ARX_SOUND_PlayCollision(const char * name1, const char * name2, const float & volume, const float & power, EERIE_3D * position, INTERACTIVE_OBJ * source)
{
	if (!bIsActive) return 0;

	if (!name1 || !name2) return 0;

	if (stricmp(name2, "WATER") == 0)
		ARX_PARTICLES_SpawnWaterSplash(position);


	for (unsigned long i(0); i < collision_map_c; i++)
	{
		ARX_SOUND_CollisionMap * c_map = &collision_map_l[i];

		if (!stricmp(name1, c_map->name))
			for (unsigned long j(0); j < c_map->material_c; j++)
			{
				ARX_SOUND_Material * c_material = &c_map->material_l[j];

				if (!stricmp(name2, c_material->name))
				{
					long sample_id;

					sample_id = c_material->variant_l[c_material->variant_i];

					if (++c_material->variant_i >= c_material->variant_c) c_material->variant_i = 0;

					if (sample_id == AAL_SFALSE)
						return 0;

					aalChannel channel;
					channel.mixer = ARX_SOUND_MixerGameSample;
					char sample_name[256];
					float presence;

					channel.flags = AAL_FLAG_VOLUME | AAL_FLAG_PITCH | AAL_FLAG_POSITION | AAL_FLAG_REVERBERATION | AAL_FLAG_FALLOFF;

					aalGetSampleName(sample_id, sample_name);
					presence = GetSamplePresenceFactor(sample_name);
					channel.falloff.start = ARX_SOUND_DEFAULT_FALLSTART * presence;
					channel.falloff.end = ARX_SOUND_DEFAULT_FALLEND * presence;

					//Launch 'ON HEAR' script event
					ARX_NPC_SpawnAudibleSound(position, source, power, presence);

					if (position)
					{
						channel.position.x = position->x;
						channel.position.y = position->y;
						channel.position.z = position->z;

						if (ACTIVECAM && EEDistance3D(&ACTIVECAM->pos, position) > ARX_SOUND_REFUSE_DISTANCE)
							return -1;
					}
					else
						ARX_PLAYER_FrontPos((EERIE_3D *)&channel.position);


					channel.pitch = 0.975F + 0.5F * rnd();
					channel.volume = volume;
					aalSamplePlay(sample_id, channel);

					unsigned long length;
					aalGetSampleLength(sample_id, length);

					return (long)(channel.pitch * length);
				}
			}
	}

	return 0;
}

long ARX_SOUND_PlayScript(const char * name, const INTERACTIVE_OBJ * io, const float & pitch, const long & loop)
{
	if (!bIsActive) return AAL_SFALSE;

	aalChannel channel;
	long sample_id;

	sample_id = aalCreateSample(name);

	if (sample_id == AAL_SFALSE) return AAL_SFALSE;

	channel.mixer = ARX_SOUND_MixerGameSample;
	channel.flags = AAL_FLAG_VOLUME | AAL_FLAG_AUTOFREE | AAL_FLAG_POSITION | AAL_FLAG_REVERBERATION | AAL_FLAG_FALLOFF;
	channel.volume = 1.0F;
	channel.falloff.start = ARX_SOUND_DEFAULT_FALLSTART * GetSamplePresenceFactor(name);
	channel.falloff.end = ARX_SOUND_DEFAULT_FALLEND;

	if (io)
	{
		GetItemWorldPositionSound((INTERACTIVE_OBJ *)io, (EERIE_3D *)&channel.position);

		if (loop != ARX_SOUND_PLAY_LOOPED)
		{
			EERIE_3D ePos;
			ePos.x = channel.position.x;
			ePos.y = channel.position.y;
			ePos.z = channel.position.z;

			if (ACTIVECAM && EEDistance3D(&ACTIVECAM->pos, &ePos) > ARX_SOUND_REFUSE_DISTANCE)
				return -1;
		}
	}
	else
	{
		channel.flags |= AAL_FLAG_RELATIVE;
		channel.position.x = 0.0F;
		channel.position.y = 0.0F;
		channel.position.z = 100.0F;
	}

	if (pitch != 1.0F)
	{
		channel.flags |= AAL_FLAG_PITCH;
		channel.pitch = pitch;
	}

	aalSamplePlay(sample_id, channel, loop);

	return sample_id;
}

long ARX_SOUND_PlayAnim(long & sample_id, const EERIE_3D * position)
{
	if (!bIsActive || sample_id == AAL_SFALSE) return AAL_SFALSE;

	aalChannel channel;

	channel.mixer = ARX_SOUND_MixerGameSample;
	channel.flags = AAL_FLAG_VOLUME;
	channel.volume = 1.0F;

	if (position)
	{
		char sample_name[256];
		float presence;

		channel.flags |= AAL_FLAG_POSITION | AAL_FLAG_REVERBERATION | AAL_FLAG_FALLOFF;
		aalGetSampleName(sample_id, sample_name);
		presence = GetSamplePresenceFactor(sample_name);
		channel.falloff.start = ARX_SOUND_DEFAULT_FALLSTART * presence;
		channel.falloff.end = ARX_SOUND_DEFAULT_FALLEND * presence;
		channel.position.x = position->x;
		channel.position.y = position->y;
		channel.position.z = position->z;
	}

	if (ACTIVECAM && EEDistance3D(&ACTIVECAM->pos, position) > ARX_SOUND_REFUSE_DISTANCE)
		return -1;

	aalSamplePlay(sample_id, channel);

	return sample_id;
}

long ARX_SOUND_PlayCinematic(const char * name)
{
	long sample_id;
	aalChannel channel;

	sample_id = aalCreateSample(name);

	if (sample_id == AAL_SFALSE) return AAL_SFALSE;

	channel.mixer = ARX_SOUND_MixerGameSpeech;
	channel.flags = AAL_FLAG_VOLUME | AAL_FLAG_AUTOFREE | AAL_FLAG_POSITION | AAL_FLAG_FALLOFF | AAL_FLAG_REVERBERATION | AAL_FLAG_POSITION;
	channel.volume = 1.0F;
	channel.falloff.start = ARX_SOUND_DEFAULT_FALLSTART;
	channel.falloff.end = ARX_SOUND_DEFAULT_FALLEND;

	if (ACTIVECAM)
	{
		EERIE_3D front, up;
		float t;
		t = DEG2RAD(MAKEANGLE(ACTIVECAM->angle.b));
		front.x = -EEsin(t);
		front.y = 0.f;
		front.z = EEcos(t);
		TRUEVector_Normalize(&front);
		up.x = 0.f;
		up.y = 1.f;
		up.z = 0.f;
		ARX_SOUND_SetListener(&ACTIVECAM->pos, &front, &up);
	}

	ARX_SOUND_IOFrontPos(NULL, channel.position); 

	aalSamplePlay(sample_id, channel);

	return sample_id;
}

long ARX_SOUND_IsPlaying(long & sample_id)
{
	return bIsActive ? aalIsSamplePlaying(sample_id) : 0;
}


float ARX_SOUND_GetDuration(long & sample_id)
{
	if (bIsActive && sample_id != AAL_SFALSE)
	{
		aalULong length;

		aalGetSampleLength(sample_id, length);
		return ARX_CLEAN_WARN_CAST_FLOAT(length);
	}

	return 0.f;
}

void ARX_SOUND_RefreshVolume(long & sample_id, const float & volume)
{
	if (bIsActive && sample_id != AAL_SFALSE)
		aalSetSampleVolume(sample_id, volume);
}

void ARX_SOUND_RefreshPitch(long & sample_id, const float & pitch)
{
	if (bIsActive && sample_id != AAL_SFALSE)
		aalSetSamplePitch(sample_id, pitch);
}

void ARX_SOUND_RefreshPosition(long & sample_id, const EERIE_3D * position)
{
	if (bIsActive && sample_id != AAL_SFALSE)
	{
		if (position)
			aalSetSamplePosition(sample_id, *(aalVector *)position);
		else
		{
			EERIE_3D pos;

			ARX_PLAYER_FrontPos(&pos);
			aalSetSamplePosition(sample_id, *(aalVector *)&pos);
		}
	}
}

void ARX_SOUND_RefreshSpeechPosition(long & sample_id, const INTERACTIVE_OBJ * io)
{
	if (!bIsActive || !io || sample_id == AAL_SFALSE) return;

	aalVector position;

	if (io)
	{
		if (((io == inter.iobj[0]) && !EXTERNALVIEW) ||
		        (io->ioflags & IO_CAMERA && io == CAMERACONTROLLER))
		{
			ARX_SOUND_IOFrontPos(io, position);
		}
		else
		{
			position.x = io->pos.x;
			position.y = io->pos.y;
			position.z = io->pos.z;
		}
	}

	aalSetSamplePosition(sample_id, position);
}

long ARX_SOUND_Load(const char * name)
{
	if (!bIsActive) return AAL_SFALSE;

	char sample_name[256];

	sprintf(sample_name, "%s%s", name, ARX_SOUND_FILE_EXTENSION_WAV);

	return aalCreateSample(sample_name);
}

void ARX_SOUND_Free(const long & sample)
{
	if (!bIsActive || sample == ARX_SOUND_INVALID_RESOURCE) return;

	aalDeleteSample(sample);
}

void ARX_SOUND_Stop(long & sample_id)
{
	if (bIsActive && sample_id != AAL_SFALSE) aalSampleStop(sample_id);
}

long ARX_SOUND_PlayScriptAmbiance(const char * name, const long & loop, const float & volume) //, const EERIE_3D *position)
{
	if (!bIsActive) return AAL_SFALSE;

	char temp[512];

	strcpy(temp, name);
	SetExt(temp, ".amb");

	long ambiance_id(aalGetAmbiance(temp));

	if (ambiance_id == AAL_SFALSE)
	{
		if (volume == 0.0F) return AAL_SFALSE;

		ambiance_id = aalCreateAmbiance(temp);
		aalSetAmbianceUserData(ambiance_id, (void *)PLAYING_AMBIANCE_SCRIPT);

		aalChannel channel;

		channel.mixer = ARX_SOUND_MixerGameAmbiance;
		channel.flags = AAL_FLAG_VOLUME | AAL_FLAG_AUTOFREE;
		channel.volume = volume;

		aalAmbiancePlay(ambiance_id, channel, loop);
	}
	else
	{
		if (volume <= 0.0F)
		{
			aalDeleteAmbiance(ambiance_id);
			return AAL_SFALSE;
		}

		aalSetAmbianceVolume(ambiance_id, volume);
	}

	return ambiance_id;
}

long ARX_SOUND_PlayZoneAmbiance(const char * name, const long & loop, const float & volume) 
{
	if (!bIsActive) return AAL_SFALSE;

	char temp[512];

	strcpy(temp, name);
	SetExt(temp, ".amb");

	if (!stricmp(name, "NONE"))
	{
		aalAmbianceStop(ambiance_zone, AMBIANCE_FADE_TIME);
		ambiance_zone = AAL_SFALSE;
		return AAL_SFALSE;
	}

	long ambiance_id(aalGetAmbiance(temp));

	if (ambiance_id == AAL_SFALSE)
	{
		ambiance_id = aalCreateAmbiance(temp);
		aalSetAmbianceUserData(ambiance_id, (void *)PLAYING_AMBIANCE_ZONE);
	}
	else if (ambiance_id == ambiance_zone)
		return ambiance_zone;

	aalChannel channel;

	channel.mixer = ARX_SOUND_MixerGameAmbiance;
	channel.flags = AAL_FLAG_VOLUME | AAL_FLAG_AUTOFREE;
	channel.volume = volume;

	aalAmbianceStop(ambiance_zone, AMBIANCE_FADE_TIME);
	aalAmbiancePlay(ambiance_zone = ambiance_id, channel, loop, AMBIANCE_FADE_TIME);

	return ambiance_zone;
}

long ARX_SOUND_SetAmbianceTrackStatus(const char * ambiance_name, const char * track_name, const unsigned long & status)
{
	if (!bIsActive || !ambiance_name) return AAL_SFALSE;

	long ambiance_id, track_id;
	char temp[512];

	strcpy(temp, ambiance_name);
	sprintf(temp, "%s", ambiance_name);
	SetExt(temp, ".amb");

	ambiance_id = aalGetAmbiance(temp);

	if (ambiance_id == AAL_SFALSE) return AAL_SFALSE;

	if (aalGetAmbianceTrackID(ambiance_id, track_name, track_id)) return AAL_SFALSE;

	aalMuteAmbianceTrack(ambiance_id, track_id, (aalUBool)status);

	return ambiance_id;
}

void ARX_SOUND_KillAmbiances()
{
	if (!bIsActive) return;

	aalSLong ambiance_id = aalGetAmbiance();

	while (ambiance_id != AAL_SFALSE)
	{
		aalDeleteAmbiance(ambiance_id);
		ambiance_id = aalGetAmbiance();
	}

	ambiance_zone = AAL_SFALSE;
}

long ARX_SOUND_PlayMenuAmbiance(const char * ambiance_name)
{
	if (!bIsActive) return AAL_SFALSE;

	aalDeleteAmbiance(ambiance_menu);
	ambiance_menu = aalCreateAmbiance(ambiance_name);

	aalSetAmbianceUserData(ambiance_menu, (void *)PLAYING_AMBIANCE_MENU);

	aalChannel channel;

	channel.mixer = ARX_SOUND_MixerMenuAmbiance;
	channel.flags = AAL_FLAG_VOLUME;
	channel.volume = 1.0F;

	aalAmbiancePlay(ambiance_menu, channel, 0);

	return ambiance_menu;
}

long nbelems = 0;
char ** elems = NULL;
long * numbers = NULL;

void ARX_SOUND_FreeAnimSamples()
{
	if (elems)
	{
		for (long i = 0; i < nbelems; i++)
		{
			if (elems[i])
			{
				free(elems[i]);
				elems[i] = NULL;
			}
		}

		free(elems);
		elems = NULL;
	}

	if (numbers)
	{
		free(numbers);
		numbers = NULL;
	}

	nbelems = 0;

}
#define MAX_ANIMATIONS 900

extern ANIM_HANDLE animations[];
void ARX_SOUND_PushAnimSamples()
{
	ARX_SOUND_FreeAnimSamples();

	long number = 0;

	for (long i = 0; i < MAX_ANIMATIONS; i++)
	{
		if (animations[i].path[0])
		{
			for (long j = 0; j < animations[i].alt_nb; j++)
			{
				EERIE_ANIM * anim = animations[i].anims[j];

				for (long k = 0; k < anim->nb_key_frames; k++)
				{
					number++;

					if (anim->frames[k].sample != -1)
					{
						char dest[256];
						aalGetSampleName(anim->frames[k].sample, dest);

						if (dest[0])
						{
							elems = (char **)realloc(elems, sizeof(char *) * (nbelems + 1));
							elems[nbelems] = strdup(dest);
							numbers = (long *)realloc(numbers, sizeof(long) * (nbelems + 1));
							numbers[nbelems] = number;
							nbelems++;
						}
					}
				}
			}
		}
	}
}
void ARX_SOUND_PopAnimSamples()
{
	if ((!elems) ||
	        (!bIsActive))
	{
		return;
	}

	long curelem = 0;
	long number = 0;

	for (long i = 0; i < MAX_ANIMATIONS; i++)
	{
		if (animations[i].path[0])
		{
			for (long j = 0; j < animations[i].alt_nb; j++)
			{
				EERIE_ANIM * anim = animations[i].anims[j];

				for (long k = 0; k < anim->nb_key_frames; k++)
				{
					number++;

					if (number == numbers[curelem]) 
					{
						anim->frames[k].sample = aalCreateSample(elems[curelem++]);
					}
				}
			}
		}
	}


	ARX_SOUND_FreeAnimSamples();
}

void ARX_SOUND_AmbianceSavePlayList(void ** _play_list, unsigned long * size)
{
	unsigned long count(0);
	PlayingAmbiance * play_list = NULL;
	long ambiance_id(AAL_SFALSE);

	ambiance_id = aalGetNextAmbiance();

	while (ambiance_id != AAL_SFALSE)
	{
		long type;
		aalGetAmbianceUserData(ambiance_id, (void **)&type);

		if (type == PLAYING_AMBIANCE_SCRIPT || type == PLAYING_AMBIANCE_ZONE)
		{
			void * ptr;
			PlayingAmbiance * playing;

			ptr = realloc(play_list, (count + 1) * sizeof(PlayingAmbiance));

			if (!ptr) break;

			play_list = (PlayingAmbiance *)ptr;
			playing = &play_list[count];

			aalGetAmbianceName(ambiance_id, playing->name);
			aalGetAmbianceVolume(ambiance_id, playing->volume);
			playing->loop = aalIsAmbianceLooped(ambiance_id) ? ARX_SOUND_PLAY_LOOPED : ARX_SOUND_PLAY_ONCE;
			playing->type = type;

			count++;
		}

		ambiance_id = aalGetNextAmbiance(ambiance_id);
	}

	*_play_list = play_list;
	*size = count * sizeof(PlayingAmbiance);
}

void ARX_SOUND_AmbianceRestorePlayList(void * _play_list, unsigned long size)
{
	unsigned long count = size / sizeof(PlayingAmbiance);
	PlayingAmbiance * play_list = (PlayingAmbiance *)_play_list;

	for (unsigned long i(0); i < count; i++)
	{
		PlayingAmbiance * playing = &play_list[i];

		switch (playing->type)
		{
			case PLAYING_AMBIANCE_SCRIPT :
				ARX_SOUND_PlayScriptAmbiance(playing->name, playing->loop, playing->volume);
				break;

			case PLAYING_AMBIANCE_ZONE :
				ARX_SOUND_PlayZoneAmbiance(playing->name, playing->loop, playing->volume);
				break;
		}
	}
}

// P‚BÙMÈJËMÁA
extern PakManager * pPakManager;
static void ARX_SOUND_CreateEnvironments()
{
	if (FINAL_RELEASE)
	{
		vector<EVE_REPERTOIRE *> *pvDirectory = NULL;
		char lpszPakPath[512] = "";

		sprintf(lpszPakPath, "%ssfx.pak", Project.workingdir);

		if (!pPakManager) pPakManager = new PakManager;

		if (!pPakManager->AddPak(lpszPakPath)) return;

		pvDirectory = pPakManager->ExistDirectory((char *)ARX_SOUND_PATH_ENVIRONMENT);

		if (!pvDirectory)
		{
			pPakManager->RemovePak(lpszPakPath);
			return;
		}

		vector<EVE_REPERTOIRE *>::iterator iv;

		for (iv = pvDirectory->begin(); iv < pvDirectory->end(); iv++)
		{
			int nb = (*iv)->nbfiles;
			EVE_TFILE * et = (*iv)->fichiers;

			while (nb--)
			{
				aalCreateEnvironment((const char *)et->name);
				et = et->fnext;
			}
		}

		pPakManager->RemovePak(lpszPakPath);
		pvDirectory->clear();
		delete pvDirectory;
	}
	else
	{
		char path[512] = "";
		_finddata_t fdata;
		long fhandle;

		sprintf(path, "%ssfx\\environment\\*.aef", Project.workingdir);

		if ((fhandle = _findfirst(path, &fdata)) != -1)
		{
			do
			{
				aalCreateEnvironment(fdata.name);
			}
			while (_findnext(fhandle, &fdata));

			_findclose(fhandle);
		}
	}
}

static void ARX_SOUND_CreateStaticSamples()
{
	// Interface
	SND_BACKPACK                       = aalCreateSample("interface_backpack.wav");
	SND_MAP                            = aalCreateSample("interface_map.wav");
	SND_BOOK_OPEN                      = aalCreateSample("book_open.wav");
	SND_BOOK_CLOSE                     = aalCreateSample("book_close.wav");
	SND_BOOK_PAGE_TURN                 = aalCreateSample("book_page_turn.wav");
	SND_SCROLL_OPEN                    = aalCreateSample("scroll_open.wav");
	SND_SCROLL_CLOSE                   = aalCreateSample("scroll_close.wav");
	SND_TORCH_START                    = aalCreateSample("torch_start.wav");
	SND_TORCH_LOOP                     = aalCreateSample("sfx_torch_11khz.wav");
	SND_TORCH_END                      = aalCreateSample("torch_end.wav");
	SND_INVSTD                         = aalCreateSample("interface_invstd.wav");
	SND_GOLD                           = aalCreateSample("drop_coin.wav");

	//Menu
	SND_MENU_CLICK                     = aalCreateSample("menu_click.wav");
	SND_MENU_CREDITS_LOOP              = aalCreateSample("menu_credits_loop.wav");
	SND_MENU_LOOP                      = aalCreateSample("menu_loop.wav");
	SND_MENU_OPTIONS_LOOP              = aalCreateSample("menu_options_loop.wav");
	SND_MENU_PUSH                      = aalCreateSample("menu_push.wav");
	SND_MENU_RELEASE                   = aalCreateSample("menu_release.wav");

	//Other SFX samples
	SND_FIREPLACE                      = aalCreateSample("fire_place.wav");
	SND_PLOUF                          = aalCreateSample("fishing_plouf.wav");
	SND_QUAKE                          = aalCreateSample("sfx_quake.wav");
	SND_WHOOSH							= aalCreateSample("whoosh07.wav");

	// Player
	SND_PLAYER_FILLLIFEMANA            = aalCreateSample("player_filllifemana.wav");
	SND_PLAYER_HEART_BEAT              = aalCreateSample("player_heartb.wav");
	SND_PLAYER_JUMP                    = aalCreateSample("player_jump.wav");
	SND_PLAYER_JUMP_END                = aalCreateSample("player_jumpend.wav");
	SND_PLAYER_LEVEL_UP                = aalCreateSample("player_level_up.wav");
	SND_PLAYER_POISONED                = aalCreateSample("player_poisoned.wav");
	SND_PLAYER_DEATH_BY_FIRE           = aalCreateSample("lava_death.wav");

	// Magic draw
	SND_MAGIC_AMBIENT                  = aalCreateSample("magic_ambient.wav");
	SND_MAGIC_DRAW                     = aalCreateSample("magic_draw.wav");
	SND_MAGIC_FIZZLE                   = aalCreateSample("magic_fizzle.wav");

	// Magic symbols
	SND_SYMB_AAM                       = aalCreateSample("magic_aam.wav");
	SND_SYMB_CETRIUS                   = aalCreateSample("magic_citrius.wav");
	SND_SYMB_COSUM                     = aalCreateSample("magic_cosum.wav");
	SND_SYMB_COMUNICATUM               = aalCreateSample("magic_comunicatum.wav");
	SND_SYMB_FOLGORA                   = aalCreateSample("magic_folgora.wav");
	SND_SYMB_FRIDD                     = aalCreateSample("magic_fridd.wav");
	SND_SYMB_KAOM                      = aalCreateSample("magic_kaom.wav");
	SND_SYMB_MEGA                      = aalCreateSample("magic_mega.wav");
	SND_SYMB_MORTE                     = aalCreateSample("magic_morte.wav");
	SND_SYMB_MOVIS                     = aalCreateSample("magic_movis.wav");
	SND_SYMB_NHI                       = aalCreateSample("magic_nhi.wav");
	SND_SYMB_RHAA                      = aalCreateSample("magic_rhaa.wav");
	SND_SYMB_SPACIUM                   = aalCreateSample("magic_spacium.wav");
	SND_SYMB_STREGUM                   = aalCreateSample("magic_stregum.wav");
	SND_SYMB_TAAR                      = aalCreateSample("magic_taar.wav");
	SND_SYMB_TEMPUS                    = aalCreateSample("magic_tempus.wav");
	SND_SYMB_TERA                      = aalCreateSample("magic_tera.wav");
	SND_SYMB_VISTA                     = aalCreateSample("magic_vista.wav");
	SND_SYMB_VITAE                     = aalCreateSample("magic_vitae.wav");
	SND_SYMB_YOK                       = aalCreateSample("magic_yok.wav");

	// Spells
	SND_SPELL_ACTIVATE_PORTAL          = aalCreateSample("magic_spell_activate_portal.wav");
	SND_SPELL_ARMOR_START              = aalCreateSample("magic_spell_armor_start.wav");
	SND_SPELL_ARMOR_END                = aalCreateSample("magic_spell_armor_end.wav");
	SND_SPELL_ARMOR_LOOP               = aalCreateSample("magic_spell_armor_loop.wav");
	SND_SPELL_LOWER_ARMOR              = aalCreateSample("Magic_Spell_decrease_Armor.wav");
	SND_SPELL_BLESS                    = aalCreateSample("magic_spell_bless.wav");
	SND_SPELL_COLD_PROTECTION_START    = aalCreateSample("Magic_Spell_Cold_Protection.wav");
	SND_SPELL_COLD_PROTECTION_LOOP     = aalCreateSample("Magic_Spell_Cold_Protection_loop.wav");
	SND_SPELL_COLD_PROTECTION_END      = aalCreateSample("Magic_Spell_Cold_Protection_end.wav");
	SND_SPELL_CONFUSE                  = aalCreateSample("magic_spell_confuse.wav");
	SND_SPELL_CONTROL_TARGET           = aalCreateSample("magic_spell_control_target.wav");
	SND_SPELL_CREATE_FIELD             = aalCreateSample("magic_spell_create_field.wav");
	SND_SPELL_CREATE_FOOD              = aalCreateSample("magic_spell_create_food.wav");
	SND_SPELL_CURE_POISON              = aalCreateSample("magic_spell_cure_poison.wav");
	SND_SPELL_CURSE                    = aalCreateSample("magic_spell_curse.wav");
	SND_SPELL_DETECT_TRAP              = aalCreateSample("magic_spell_detect_trap.wav");
	SND_SPELL_DETECT_TRAP_LOOP         = aalCreateSample("magic_spell_detect_trap_Loop.wav");
	SND_SPELL_DISARM_TRAP              = aalCreateSample("magic_spell_disarm_trap.wav");
	SND_SPELL_DISPELL_FIELD            = aalCreateSample("magic_spell_dispell_field.wav");
	SND_SPELL_DISPELL_ILLUSION         = aalCreateSample("magic_spell_dispell_illusion.wav");
	SND_SPELL_DOUSE                    = aalCreateSample("magic_spell_douse.wav");
	SND_SPELL_ELECTRIC                 = aalCreateSample("sfx_electric.wav");
	SND_SPELL_ENCHANT_WEAPON           = aalCreateSample("magic_spell_enchant_weapon.wav");
	SND_SPELL_EXPLOSION                = aalCreateSample("magic_spell_explosion.wav");
	SND_SPELL_EYEBALL_IN               = aalCreateSample("magic_spell_eyeball_in.wav");
	SND_SPELL_EYEBALL_OUT              = aalCreateSample("magic_spell_eyeball_out.wav");
	SND_SPELL_FIRE_HIT                 = aalCreateSample("magic_spell_firehit.wav");
	SND_SPELL_FIRE_LAUNCH              = aalCreateSample("magic_spell_firelaunch.wav");
	SND_SPELL_FIRE_PROTECTION          = aalCreateSample("magic_spell_fire_protection.wav");
	SND_SPELL_FIRE_WIND                = aalCreateSample("magic_spell_firewind.wav");
	SND_SPELL_FREEZETIME               = aalCreateSample("magic_spell_freezetime.wav");
	SND_SPELL_HARM                     = aalCreateSample("magic_spell_harm.wav");
	SND_SPELL_HEALING                  = aalCreateSample("magic_spell_healing.wav");
	SND_SPELL_ICE_FIELD                = aalCreateSample("magic_spell_ice_field.wav");
	SND_SPELL_ICE_PROJECTILE_LAUNCH    = aalCreateSample("magic_spell_ice_projectile_launch.wav");
	SND_SPELL_INCINERATE               = aalCreateSample("magic_spell_incinerate.wav");
	SND_SPELL_IGNITE                   = aalCreateSample("magic_spell_ignite.wav");
	SND_SPELL_INVISIBILITY_START       = aalCreateSample("magic_spell_invisibilityon.wav");
	SND_SPELL_INVISIBILITY_END         = aalCreateSample("magic_spell_invisibilityoff.wav");
	SND_SPELL_LEVITATE_START           = aalCreateSample("magic_spell_levitate_start.wav");
	SND_SPELL_LIGHTNING_START          = aalCreateSample("magic_spell_lightning_start.wav");
	SND_SPELL_LIGHTNING_LOOP           = aalCreateSample("magic_spell_lightning_loop.wav");
	SND_SPELL_LIGHTNING_END            = aalCreateSample("magic_spell_lightning_end.wav");
	SND_SPELL_MAGICAL_HIT              = aalCreateSample("magic_spell_magicalhit.wav");

	SND_SPELL_MASS_LIGHTNING_END		= aalCreateSample("magic_spell_mass_lightning_end.wav");
	SND_SPELL_FIRE_FIELD_START			= aalCreateSample("magic_spell_fire_field.wav");
	SND_SPELL_FIRE_FIELD_LOOP			= aalCreateSample("magic_spell_fire_field_loop.wav");
	SND_SPELL_FIRE_FIELD_END			= aalCreateSample("magic_spell_fire_field_end.wav");

	SND_SPELL_MAGICAL_SHIELD           = aalCreateSample("magic_spell_magicalshield.wav");
	SND_SPELL_MASS_INCINERATE          = aalCreateSample("magic_spell_mass_incinerate.wav");
	SND_SPELL_MASS_PARALYSE            = aalCreateSample("magic_spell_mass_paralyse.wav");
	SND_SPELL_MM_CREATE                = aalCreateSample("magic_spell_missilecreate.wav");
	SND_SPELL_MM_HIT                   = aalCreateSample("magic_spell_missilehit.wav");
	SND_SPELL_MM_LAUNCH                = aalCreateSample("magic_spell_missilelaunch.wav");
	SND_SPELL_MM_LOOP                  = aalCreateSample("magic_spell_missileloop.wav");
	SND_SPELL_NEGATE_MAGIC             = aalCreateSample("magic_spell_negate_magic.wav");
	SND_SPELL_NO_EFFECT                = aalCreateSample("magic_spell_noeffect.wav");
	SND_SPELL_PARALYSE                 = aalCreateSample("magic_spell_paralyse.wav");
	SND_SPELL_PARALYSE_END             = aalCreateSample("magic_spell_paralyse_end.wav");
	SND_SPELL_POISON_PROJECTILE_LAUNCH = aalCreateSample("magic_spell_poison_projectile_launch.wav");
	SND_SPELL_RAISE_DEAD               = aalCreateSample("magic_spell_raise_dead.wav");
	SND_SPELL_REPEL_UNDEAD             = aalCreateSample("magic_spell_repel_undead.wav");
	SND_SPELL_REPEL_UNDEAD_LOOP        = aalCreateSample("magic_spell_repell_loop.wav");
	SND_SPELL_RUNE_OF_GUARDING         = aalCreateSample("magic_spell_rune_of_guarding.wav");
	SND_SPELL_SLOW_DOWN                = aalCreateSample("magic_spell_slow_down.wav");
	SND_SPELL_SPARK                    = aalCreateSample("sfx_spark.wav");
	SND_SPELL_SPEED_START              = aalCreateSample("magic_spell_speedstart.wav");
	SND_SPELL_SPEED_LOOP               = aalCreateSample("magic_spell_speed.wav");
	SND_SPELL_SPEED_END				   = aalCreateSample("magic_spell_speedend.wav");
	SND_SPELL_SUMMON_CREATURE          = aalCreateSample("magic_spell_summon_creature.wav");
	SND_SPELL_TELEKINESIS_START        = aalCreateSample("magic_spell_telekinesison.wav");
	SND_SPELL_TELEKINESIS_END          = aalCreateSample("magic_spell_telekinesisoff.wav");
	SND_SPELL_TELEPORT                 = aalCreateSample("magic_spell_teleport.wav");
	SND_SPELL_TELEPORTED               = aalCreateSample("magic_spell_teleported.wav");
	SND_SPELL_VISION_START             = aalCreateSample("magic_spell_vision2.wav");
	SND_SPELL_VISION_LOOP              = aalCreateSample("magic_spell_vision.wav");
}

// Reset each static sample to ARX_SOUND_INVALID_RESOURCE
// Those samples are freed from memory when Athena is deleted
static void ARX_SOUND_ReleaseStaticSamples()
{
	// Interface samples
	SND_BACKPACK = AAL_SFALSE;
	SND_BOOK_OPEN = AAL_SFALSE;
	SND_BOOK_CLOSE = AAL_SFALSE;
	SND_BOOK_PAGE_TURN = AAL_SFALSE;
	SND_GOLD = AAL_SFALSE;
	SND_INVSTD = AAL_SFALSE;
	SND_MAP = AAL_SFALSE;
	SND_SCROLL_OPEN = AAL_SFALSE;
	SND_SCROLL_CLOSE = AAL_SFALSE;
	SND_TORCH_START = AAL_SFALSE;
	SND_TORCH_LOOP = AAL_SFALSE;
	SND_TORCH_END = AAL_SFALSE;

	// Other SFX samples
	SND_FIREPLACE = AAL_SFALSE;
	SND_PLOUF = AAL_SFALSE;
	SND_QUAKE = AAL_SFALSE;

	// Menu samples
	SND_MENU_CLICK = AAL_SFALSE;
	SND_MENU_CREDITS_LOOP = AAL_SFALSE;
	SND_MENU_LOOP = AAL_SFALSE;
	SND_MENU_OPTIONS_LOOP = AAL_SFALSE;
	SND_MENU_PUSH = AAL_SFALSE;
	SND_MENU_RELEASE = AAL_SFALSE;

	// Player samples
	SND_PLAYER_DEATH = AAL_SFALSE;
	SND_PLAYER_DEATH_BY_FIRE = AAL_SFALSE;
	SND_PLAYER_FILLLIFEMANA = AAL_SFALSE;
	SND_PLAYER_HEART_BEAT = AAL_SFALSE;
	SND_PLAYER_JUMP = AAL_SFALSE;
	SND_PLAYER_JUMP_END = AAL_SFALSE;
	SND_PLAYER_LEVEL_UP = AAL_SFALSE;
	SND_PLAYER_POISONED = AAL_SFALSE;

	// Magic drawing samples
	SND_MAGIC_AMBIENT = AAL_SFALSE;
	SND_MAGIC_DRAW = AAL_SFALSE;
	SND_MAGIC_FIZZLE = AAL_SFALSE;

	// Magic symbols samples
	SND_SYMB_AAM = AAL_SFALSE;
	SND_SYMB_CETRIUS = AAL_SFALSE;
	SND_SYMB_COSUM = AAL_SFALSE;
	SND_SYMB_COMUNICATUM = AAL_SFALSE;
	SND_SYMB_FOLGORA = AAL_SFALSE;
	SND_SYMB_FRIDD = AAL_SFALSE;
	SND_SYMB_KAOM = AAL_SFALSE;
	SND_SYMB_MEGA = AAL_SFALSE;
	SND_SYMB_MORTE = AAL_SFALSE;
	SND_SYMB_MOVIS = AAL_SFALSE;
	SND_SYMB_NHI = AAL_SFALSE;
	SND_SYMB_RHAA = AAL_SFALSE;
	SND_SYMB_SPACIUM = AAL_SFALSE;
	SND_SYMB_STREGUM = AAL_SFALSE;
	SND_SYMB_TAAR = AAL_SFALSE;
	SND_SYMB_TEMPUS = AAL_SFALSE;
	SND_SYMB_TERA = AAL_SFALSE;
	SND_SYMB_VISTA = AAL_SFALSE;
	SND_SYMB_VITAE = AAL_SFALSE;
	SND_SYMB_YOK = AAL_SFALSE;

	// Spells samples
	SND_SPELL_ACTIVATE_PORTAL = AAL_SFALSE;
	SND_SPELL_ARMOR_START	= AAL_SFALSE;
	SND_SPELL_ARMOR_END		= AAL_SFALSE;
	SND_SPELL_ARMOR_LOOP	= AAL_SFALSE;
	SND_SPELL_LOWER_ARMOR = AAL_SFALSE;
	SND_SPELL_BLESS = AAL_SFALSE;
	SND_SPELL_COLD_PROTECTION_START = AAL_SFALSE;
	SND_SPELL_COLD_PROTECTION_LOOP = AAL_SFALSE;
	SND_SPELL_COLD_PROTECTION_END = AAL_SFALSE;
	SND_SPELL_CONFUSE = AAL_SFALSE;
	SND_SPELL_CONTROL_TARGET = AAL_SFALSE;
	SND_SPELL_CREATE_FIELD = AAL_SFALSE;
	SND_SPELL_CREATE_FOOD = AAL_SFALSE;
	SND_SPELL_CURE_POISON = AAL_SFALSE;
	SND_SPELL_CURSE = AAL_SFALSE;
	SND_SPELL_DETECT_TRAP = AAL_SFALSE;
	SND_SPELL_DETECT_TRAP_LOOP = AAL_SFALSE;
	SND_SPELL_DISARM_TRAP = AAL_SFALSE;
	SND_SPELL_DISPELL_FIELD = AAL_SFALSE;
	SND_SPELL_DISPELL_ILLUSION = AAL_SFALSE;
	SND_SPELL_DOUSE = AAL_SFALSE;
	SND_SPELL_ELECTRIC = AAL_SFALSE;
	SND_SPELL_ENCHANT_WEAPON = AAL_SFALSE;
	SND_SPELL_EXPLOSION = AAL_SFALSE;
	SND_SPELL_EYEBALL_IN = AAL_SFALSE;
	SND_SPELL_EYEBALL_OUT = AAL_SFALSE;
	SND_SPELL_FIRE_FIELD = AAL_SFALSE;
	SND_SPELL_FIRE_HIT = AAL_SFALSE;
	SND_SPELL_FIRE_LAUNCH = AAL_SFALSE;
	SND_SPELL_FIRE_PROTECTION = AAL_SFALSE;
	SND_SPELL_FIRE_WIND = AAL_SFALSE;
	SND_SPELL_FREEZETIME = AAL_SFALSE;
	SND_SPELL_HARM = AAL_SFALSE;
	SND_SPELL_HEALING = AAL_SFALSE;
	SND_SPELL_ICE_FIELD = AAL_SFALSE;
	SND_SPELL_ICE_PROJECTILE_LAUNCH = AAL_SFALSE;
	SND_SPELL_INCINERATE = AAL_SFALSE;
	SND_SPELL_IGNITE = AAL_SFALSE;
	SND_SPELL_INVISIBILITY_START = AAL_SFALSE;
	SND_SPELL_INVISIBILITY_END = AAL_SFALSE;
	SND_SPELL_LEVITATE_START = AAL_SFALSE;
	SND_SPELL_LIGHTNING_START = AAL_SFALSE;
	SND_SPELL_LIGHTNING_LOOP = AAL_SFALSE;
	SND_SPELL_LIGHTNING_END = AAL_SFALSE;
	SND_SPELL_MAGICAL_HIT = AAL_SFALSE;

	SND_SPELL_MASS_LIGHTNING_END = AAL_SFALSE;
	SND_SPELL_FIRE_FIELD_START = AAL_SFALSE;
	SND_SPELL_FIRE_FIELD_LOOP = AAL_SFALSE;
	SND_SPELL_FIRE_FIELD_END = AAL_SFALSE;

	SND_SPELL_MAGICAL_SHIELD = AAL_SFALSE;
	SND_SPELL_MASS_INCINERATE = AAL_SFALSE;
	SND_SPELL_MASS_PARALYSE = AAL_SFALSE;
	SND_SPELL_MM_CREATE = AAL_SFALSE;
	SND_SPELL_MM_HIT = AAL_SFALSE;
	SND_SPELL_MM_LAUNCH = AAL_SFALSE;
	SND_SPELL_MM_LOOP = AAL_SFALSE;
	SND_SPELL_NEGATE_MAGIC = AAL_SFALSE;
	SND_SPELL_PARALYSE = AAL_SFALSE;
	SND_SPELL_PARALYSE_END = AAL_SFALSE;
	SND_SPELL_POISON_PROJECTILE_LAUNCH = AAL_SFALSE;
	SND_SPELL_RAISE_DEAD = AAL_SFALSE;
	SND_SPELL_REPEL_UNDEAD = AAL_SFALSE;
	SND_SPELL_REPEL_UNDEAD_LOOP = AAL_SFALSE;
	SND_SPELL_RUNE_OF_GUARDING = AAL_SFALSE;
	SND_SPELL_SLOW_DOWN = AAL_SFALSE;
	SND_SPELL_SPARK = AAL_SFALSE;
	SND_SPELL_SPEED_START = AAL_SFALSE;
	SND_SPELL_SPEED_LOOP = AAL_SFALSE;
	SND_SPELL_SPEED_END = AAL_SFALSE;
	SND_SPELL_SUMMON_CREATURE = AAL_SFALSE;
	SND_SPELL_TELEKINESIS_START = AAL_SFALSE;
	SND_SPELL_TELEKINESIS_END = AAL_SFALSE;
	SND_SPELL_TELEPORT = AAL_SFALSE;
	SND_SPELL_TELEPORTED = AAL_SFALSE;
	SND_SPELL_VISION_START = AAL_SFALSE;
	SND_SPELL_VISION_LOOP = AAL_SFALSE;
}

long ARX_MATERIAL_GetIdByName(char * name)
{
	if (!stricmp(name, "WEAPON"))	      return MATERIAL_WEAPON;

	if (!stricmp(name, "FLESH"))		    return MATERIAL_FLESH;

	if (!stricmp(name, "METAL"))		    return MATERIAL_METAL;

	if (!stricmp(name, "GLASS"))		    return MATERIAL_GLASS;

	if (!stricmp(name, "CLOTH"))		    return MATERIAL_CLOTH;

	if (!stricmp(name, "WOOD"))		      return MATERIAL_WOOD;

	if (!stricmp(name, "EARTH"))		    return MATERIAL_EARTH;

	if (!stricmp(name, "WATER"))		    return MATERIAL_WATER;

	if (!stricmp(name, "ICE"))		      return MATERIAL_ICE;

	if (!stricmp(name, "GRAVEL"))	      return MATERIAL_GRAVEL;

	if (!stricmp(name, "STONE"))		    return MATERIAL_STONE;

	if (!stricmp(name, "FOOT_LARGE"))   return MATERIAL_FOOT_LARGE;

	if (!stricmp(name, "FOOT_BARE"))    return MATERIAL_FOOT_BARE;

	if (!stricmp(name, "FOOT_SHOE"))    return MATERIAL_FOOT_SHOE;

	if (!stricmp(name, "FOOT_METAL"))   return MATERIAL_FOOT_METAL;

	if (!stricmp(name, "FOOT_STEALTH")) return MATERIAL_FOOT_STEALTH;

	return MATERIAL_NONE;
}
bool ARX_MATERIAL_GetNameById(long id, char * name)
{
	switch (id)
	{
		case MATERIAL_WEAPON:
			strcpy(name, "WEAPON");
			return true;
			break;
		case MATERIAL_FLESH:
			strcpy(name, "FLESH");
			return true;
			break;
		case MATERIAL_METAL:
			strcpy(name, "METAL");
			return true;
			break;
		case MATERIAL_GLASS:
			strcpy(name, "GLASS");
			return true;
			break;
		case MATERIAL_CLOTH:
			strcpy(name, "CLOTH");
			return true;
			break;
		case MATERIAL_WOOD:
			strcpy(name, "WOOD");
			return true;
			break;
		case MATERIAL_EARTH:
			strcpy(name, "EARTH");
			return true;
			break;
		case MATERIAL_WATER:
			strcpy(name, "WATER");
			return true;
			break;
		case MATERIAL_ICE:
			strcpy(name, "ICE");
			return true;
			break;
		case MATERIAL_GRAVEL:
			strcpy(name, "GRAVEL");
			return true;
			break;
		case MATERIAL_STONE:
			strcpy(name, "STONE");
			return true;
			break;
		case MATERIAL_FOOT_LARGE:
			strcpy(name, "FOOT_LARGE");
			return true;
			break;
		case MATERIAL_FOOT_BARE:
			strcpy(name, "FOOT_BARE");
			return true;
			break;
		case MATERIAL_FOOT_SHOE:
			strcpy(name, "FOOT_SHOE");
			return true;
			break;
		case MATERIAL_FOOT_METAL:
			strcpy(name, "FOOT_METAL");
			return true;
			break;
		case MATERIAL_FOOT_STEALTH:
			strcpy(name, "FOOT_STEALTH");
			return true;
			break;
	}

	strcpy(name, "NONE");
	return false;
}
static void ARX_SOUND_LoadCollision(const long & mat1, const long & mat2, const char * name)
{
	char path[256];

	for (long i(0); i < MAX_VARIANTS; i++)
	{
		sprintf(path, "%s_%d.wav", name, i + 1);
		Inter_Materials[mat1][mat2][i] = aalCreateSample(path);

		if (mat1 != mat2)
			Inter_Materials[mat2][mat1][i] = Inter_Materials[mat1][mat2][i];
	}
}

unsigned long CollisionMapSectionCallback(const char * lpszSection)
{
	void * ptr;
	unsigned long ulSectionSize(strlen(lpszSection) + 1);
	ARX_SOUND_CollisionMap * current_map;

	//Resize the collision map list
	ptr = realloc(collision_map_l, sizeof(ARX_SOUND_CollisionMap) * (collision_map_c + 1));

	if (!ptr) return PARSE_INI_FILE_STOP;

	collision_map_l = (ARX_SOUND_CollisionMap *)ptr;
	current_map = &collision_map_l[collision_map_c];


	//Initialize the current collision map
	current_map->name = NULL;
	current_map->name = (char *)malloc(ulSectionSize);

	if (!current_map->name)
	{
		collision_map_l = (ARX_SOUND_CollisionMap *)realloc(collision_map_l, sizeof(ARX_SOUND_CollisionMap) * collision_map_c);
		return PARSE_INI_FILE_STOP;
	}

	memcpy(current_map->name, lpszSection, ulSectionSize);
	current_map->material_c = 0;
	current_map->material_l = NULL;

	collision_map_c++;

	return PARSE_INI_FILE_CONTINUE;
}

unsigned long CollisionMapStringCallback(const char * lpszString)
{
	ARX_SOUND_Material * current_material;
	unsigned long ulKeySize;
	const char * lpszValue;
	void * ptr;
	ARX_SOUND_CollisionMap * current_map = &collision_map_l[collision_map_c - 1];

	//Find value position in current string and compute key size
	lpszValue = strchr(lpszString, '=');

	if (!lpszValue) return PARSE_INI_FILE_CONTINUE;

	ulKeySize = ++lpszValue - lpszString;

	if (!strlen(lpszValue)) return PARSE_INI_FILE_CONTINUE;

	//Allocate the material
	ptr = realloc(current_map->material_l, sizeof(ARX_SOUND_Material) * (current_map->material_c + 1));

	if (!ptr) return PARSE_INI_FILE_STOP;

	current_map->material_l = (ARX_SOUND_Material *)ptr;
	current_material = &current_map->material_l[current_map->material_c];

	//Initialize it
	current_material->name = (char *)malloc(ulKeySize);

	if (!current_material->name)
	{
		current_map->material_l = (ARX_SOUND_Material *)realloc(current_map->material_l, sizeof(ARX_SOUND_Material) * current_map->material_c);
		return PARSE_INI_FILE_STOP;
	}

	memcpy(current_material->name, lpszString, ulKeySize);
	current_material->name[ulKeySize - 1] = 0;
	//current_material->name = strdup(lpszString);
	current_material->variant_c = current_material->variant_i = 0;
	current_material->variant_l = NULL;

	//Find and create samples for the current material
	char path[256];

	for (unsigned long i(0); i < MAX_VARIANTS; i++)
	{
		long sample_id;

		if (i)
			sprintf(path, "%s%u%s", lpszValue, i, ARX_SOUND_FILE_EXTENSION_WAV);
		else
			sprintf(path, "%s%s", lpszValue, ARX_SOUND_FILE_EXTENSION_WAV);

		sample_id = aalCreateSample(path);

		if (sample_id == ARX_SOUND_INVALID_RESOURCE)
		{
			sprintf(path, "%s_%u%s", lpszValue, i, ARX_SOUND_FILE_EXTENSION_WAV);
			sample_id = aalCreateSample(path);
		}

		if (sample_id != ARX_SOUND_INVALID_RESOURCE)
		{
			ptr = realloc(current_material->variant_l, sizeof(long) * (current_material->variant_c + 1));

			if (!ptr) break;

			current_material->variant_l = (long *)ptr;
			current_material->variant_l[current_material->variant_c] = sample_id;
			current_material->variant_c++;
		}
	}

	if (!current_material->variant_c)
	{
		free(current_material->name);
		current_map->material_l = (ARX_SOUND_Material *)realloc(current_map->material_l, sizeof(ARX_SOUND_Material) * current_map->material_c);
	}
	else current_map->material_c++;

	return PARSE_INI_FILE_CONTINUE;
}

static void ARX_SOUND_CreateCollisionMaps()
{
	char path[256];

	for (unsigned long i = 0; i < ARX_SOUND_COLLISION_MAP_COUNT; i++)
	{
		char * lpszFileText;
		long lFileSize;

		sprintf(path, "%s%s%s%s",
		        Project.workingdir,
		        ARX_SOUND_PATH_INI, ARX_SOUND_COLLISION_MAP_NAME[i], ARX_SOUND_FILE_EXTENSION_INI);

		lpszFileText = (char *)PAK_FileLoadMallocZero(path, &lFileSize);

		if (!lpszFileText) return;

		ARX_SOUND_ParseIniFile(lpszFileText, lFileSize, CollisionMapSectionCallback, CollisionMapStringCallback);

		free(lpszFileText);
	}
}

static void ARX_SOUND_DeleteCollisionMaps()
{
	for (unsigned long i(0); i < collision_map_c; i++)
	{
		ARX_SOUND_CollisionMap * current_map = &collision_map_l[i];

		for (unsigned long j(0); j < current_map->material_c; j++)
		{
			ARX_SOUND_Material * current_material = &current_map->material_l[j];

			for (unsigned long k(0); k < current_material->variant_c; k++)
				aalDeleteSample(current_material->variant_l[k]);

			free(current_material->variant_l);
			free(current_material->name);
		}

		free(current_map->material_l);
		free(current_map->name);
	}

	free(collision_map_l), collision_map_l = NULL, collision_map_c = 0;
}

static void ARX_SOUND_CreateMaterials()
{
	memset(Inter_Materials, -1, sizeof(long) * MAX_MATERIALS * MAX_MATERIALS * MAX_VARIANTS);

	ARX_SOUND_LoadCollision(MATERIAL_WEAPON, MATERIAL_WEAPON,       "WEAPON_on_WEAPON");
	ARX_SOUND_LoadCollision(MATERIAL_WEAPON, MATERIAL_FLESH,        "WEAPON_on_FLESH");
	ARX_SOUND_LoadCollision(MATERIAL_WEAPON, MATERIAL_METAL,        "WEAPON_on_METAL");
	ARX_SOUND_LoadCollision(MATERIAL_WEAPON, MATERIAL_GLASS,        "WEAPON_on_GLASS");
	ARX_SOUND_LoadCollision(MATERIAL_WEAPON, MATERIAL_CLOTH,        "WEAPON_on_CLOTH");
	ARX_SOUND_LoadCollision(MATERIAL_WEAPON, MATERIAL_WOOD,         "WEAPON_on_WOOD");
	ARX_SOUND_LoadCollision(MATERIAL_WEAPON, MATERIAL_EARTH,        "WEAPON_on_EARTH");
	ARX_SOUND_LoadCollision(MATERIAL_WEAPON, MATERIAL_WATER,        "WEAPON_on_WATER");
	ARX_SOUND_LoadCollision(MATERIAL_WEAPON, MATERIAL_ICE,          "WEAPON_on_ICE");
	ARX_SOUND_LoadCollision(MATERIAL_WEAPON, MATERIAL_GRAVEL,       "WEAPON_on_GRAVEL");
	ARX_SOUND_LoadCollision(MATERIAL_WEAPON, MATERIAL_STONE,        "WEAPON_on_STONE");

	ARX_SOUND_LoadCollision(MATERIAL_FLESH,  MATERIAL_FLESH,        "FLESH_on_FLESH");
	ARX_SOUND_LoadCollision(MATERIAL_FLESH,  MATERIAL_METAL,        "FLESH_on_METAL");
	ARX_SOUND_LoadCollision(MATERIAL_FLESH,  MATERIAL_GLASS,        "FLESH_on_GLASS");
	ARX_SOUND_LoadCollision(MATERIAL_FLESH,  MATERIAL_CLOTH,        "FLESH_on_CLOTH");
	ARX_SOUND_LoadCollision(MATERIAL_FLESH,  MATERIAL_WOOD,         "FLESH_on_WOOD");
	ARX_SOUND_LoadCollision(MATERIAL_FLESH,  MATERIAL_EARTH,        "FLESH_on_EARTH");
	ARX_SOUND_LoadCollision(MATERIAL_FLESH,  MATERIAL_WATER,        "FLESH_on_WATER");
	ARX_SOUND_LoadCollision(MATERIAL_FLESH,  MATERIAL_ICE,          "FLESH_on_ICE");
	ARX_SOUND_LoadCollision(MATERIAL_FLESH,  MATERIAL_GRAVEL,       "FLESH_on_GRAVEL");
	ARX_SOUND_LoadCollision(MATERIAL_FLESH,  MATERIAL_STONE,        "FLESH_on_STONE");

	ARX_SOUND_LoadCollision(MATERIAL_METAL,  MATERIAL_METAL,        "METAL_on_METAL");
	ARX_SOUND_LoadCollision(MATERIAL_METAL,  MATERIAL_GLASS,        "METAL_on_GLASS");
	ARX_SOUND_LoadCollision(MATERIAL_METAL,  MATERIAL_CLOTH,        "METAL_on_CLOTH");
	ARX_SOUND_LoadCollision(MATERIAL_METAL,  MATERIAL_WOOD,         "METAL_on_WOOD");
	ARX_SOUND_LoadCollision(MATERIAL_METAL,  MATERIAL_EARTH,        "METAL_on_EARTH");
	ARX_SOUND_LoadCollision(MATERIAL_METAL,  MATERIAL_WATER,        "METAL_on_WATER");
	ARX_SOUND_LoadCollision(MATERIAL_METAL,  MATERIAL_ICE,          "METAL_on_ICE");
	ARX_SOUND_LoadCollision(MATERIAL_METAL,  MATERIAL_GRAVEL,       "METAL_on_GRAVEL");
	ARX_SOUND_LoadCollision(MATERIAL_METAL,  MATERIAL_STONE,        "METAL_on_STONE");

	ARX_SOUND_LoadCollision(MATERIAL_GLASS,  MATERIAL_GLASS,        "GLASS_on_GLASS");
	ARX_SOUND_LoadCollision(MATERIAL_GLASS,  MATERIAL_CLOTH,        "GLASS_on_CLOTH");
	ARX_SOUND_LoadCollision(MATERIAL_GLASS,  MATERIAL_WOOD,         "GLASS_on_WOOD");
	ARX_SOUND_LoadCollision(MATERIAL_GLASS,  MATERIAL_EARTH,        "GLASS_on_EARTH");
	ARX_SOUND_LoadCollision(MATERIAL_GLASS,  MATERIAL_WATER,        "GLASS_on_WATER");
	ARX_SOUND_LoadCollision(MATERIAL_GLASS,  MATERIAL_ICE,          "GLASS_on_ICE");
	ARX_SOUND_LoadCollision(MATERIAL_GLASS,  MATERIAL_GRAVEL,       "GLASS_on_GRAVEL");
	ARX_SOUND_LoadCollision(MATERIAL_GLASS,  MATERIAL_STONE,        "GLASS_on_STONE");

	ARX_SOUND_LoadCollision(MATERIAL_CLOTH,  MATERIAL_CLOTH,        "CLOTH_on_CLOTH");
	ARX_SOUND_LoadCollision(MATERIAL_CLOTH,  MATERIAL_WOOD,         "CLOTH_on_WOOD");
	ARX_SOUND_LoadCollision(MATERIAL_CLOTH,  MATERIAL_EARTH,        "CLOTH_on_EARTH");
	ARX_SOUND_LoadCollision(MATERIAL_CLOTH,  MATERIAL_WATER,        "CLOTH_on_WATER");
	ARX_SOUND_LoadCollision(MATERIAL_CLOTH,  MATERIAL_ICE,          "CLOTH_on_ICE");
	ARX_SOUND_LoadCollision(MATERIAL_CLOTH,  MATERIAL_GRAVEL,       "CLOTH_on_GRAVEL");
	ARX_SOUND_LoadCollision(MATERIAL_CLOTH,  MATERIAL_STONE,        "CLOTH_on_STONE");

	ARX_SOUND_LoadCollision(MATERIAL_WOOD,   MATERIAL_WOOD,         "WOOD_on_WOOD");
	ARX_SOUND_LoadCollision(MATERIAL_WOOD,   MATERIAL_EARTH,        "WOOD_on_EARTH");
	ARX_SOUND_LoadCollision(MATERIAL_WOOD,   MATERIAL_WATER,        "WOOD_on_WATER");
	ARX_SOUND_LoadCollision(MATERIAL_WOOD,   MATERIAL_ICE,          "WOOD_on_ICE");
	ARX_SOUND_LoadCollision(MATERIAL_WOOD,   MATERIAL_GRAVEL,       "WOOD_on_GRAVEL");
	ARX_SOUND_LoadCollision(MATERIAL_WOOD,   MATERIAL_STONE,        "WOOD_on_STONE");

	ARX_SOUND_LoadCollision(MATERIAL_EARTH,  MATERIAL_EARTH,        "EARTH_on_EARTH");
	ARX_SOUND_LoadCollision(MATERIAL_EARTH,  MATERIAL_WATER,        "EARTH_on_WATER");
	ARX_SOUND_LoadCollision(MATERIAL_EARTH,  MATERIAL_ICE,          "EARTH_on_ICE");
	ARX_SOUND_LoadCollision(MATERIAL_EARTH,  MATERIAL_GRAVEL,       "EARTH_on_GRAVEL");
	ARX_SOUND_LoadCollision(MATERIAL_EARTH,  MATERIAL_STONE,        "EARTH_on_STONE");

	ARX_SOUND_LoadCollision(MATERIAL_WATER,  MATERIAL_WATER,        "WATER_on_WATER");
	ARX_SOUND_LoadCollision(MATERIAL_WATER,  MATERIAL_ICE,          "WATER_on_ICE");
	ARX_SOUND_LoadCollision(MATERIAL_WATER,  MATERIAL_GRAVEL,       "WATER_on_GRAVEL");
	ARX_SOUND_LoadCollision(MATERIAL_WATER,  MATERIAL_STONE,        "WATER_on_STONE");

	ARX_SOUND_LoadCollision(MATERIAL_ICE,    MATERIAL_ICE,          "ICE_on_ICE");
	ARX_SOUND_LoadCollision(MATERIAL_ICE,    MATERIAL_GRAVEL,       "ICE_on_GRAVEL");
	ARX_SOUND_LoadCollision(MATERIAL_ICE,    MATERIAL_STONE,        "ICE_on_STONE");

	ARX_SOUND_LoadCollision(MATERIAL_GRAVEL, MATERIAL_GRAVEL,       "GRAVEL_on_GRAVEL");
	ARX_SOUND_LoadCollision(MATERIAL_GRAVEL, MATERIAL_STONE,        "GRAVEL_on_STONE");

	ARX_SOUND_LoadCollision(MATERIAL_STONE,  MATERIAL_STONE,        "STONE_on_STONE");
}

unsigned long PresenceSectionCallback(const char * lpszText)
{
	return PARSE_INI_FILE_CONTINUE;
}

unsigned long PresenceStringCallback(const char * lpszText)
{
	unsigned long ulKeySize;
	const char * lpszValue;
	void * ptr;
	ARX_SOUND_Presence * current_presence;

	lpszValue = strchr(lpszText, '=');

	if (!lpszValue) return PARSE_INI_FILE_CONTINUE;

	ulKeySize = lpszValue - lpszText + sizeof(ARX_SOUND_FILE_EXTENSION_WAV);

	if (!strlen(++lpszValue)) return PARSE_INI_FILE_CONTINUE;

	//Allocate the new map entry
	ptr = realloc(presence_l, sizeof(ARX_SOUND_Presence) * (presence_c + 1));

	if (!ptr) return PARSE_INI_FILE_STOP;

	presence_l = (ARX_SOUND_Presence *)ptr;

	current_presence = &presence_l[presence_c];
	current_presence->name = (char *)malloc(ulKeySize);

	if (!current_presence->name)
	{
		presence_l = (ARX_SOUND_Presence *)realloc(presence_l, sizeof(ARX_SOUND_Presence) * (presence_c));
		return PARSE_INI_FILE_STOP;
	}

	memcpy(current_presence->name, lpszText, ulKeySize - sizeof(ARX_SOUND_FILE_EXTENSION_WAV));
	memcpy(&current_presence->name[ulKeySize - sizeof(ARX_SOUND_FILE_EXTENSION_WAV)], ARX_SOUND_FILE_EXTENSION_WAV, sizeof(ARX_SOUND_FILE_EXTENSION_WAV));
	current_presence->name_size = ulKeySize;
	current_presence->factor = (float)atoi(lpszValue) / 100.0F;

	presence_c++;

	return PARSE_INI_FILE_CONTINUE;
}

static void ARX_SOUND_CreatePresenceMap()
{
	char path[256];
	char * lpszFileText;
	long lFileSize;

	sprintf(path, "%s%s%s%s",
	        Project.workingdir,
	        ARX_SOUND_PATH_INI, ARX_SOUND_PRESENCE_NAME, ARX_SOUND_FILE_EXTENSION_INI);

	lpszFileText = (char *)PAK_FileLoadMallocZero(path, &lFileSize);

	if (!lpszFileText) return;

	ARX_SOUND_ParseIniFile(lpszFileText, lFileSize, PresenceSectionCallback, PresenceStringCallback);

	free(lpszFileText);
}

static void ARX_SOUND_DeletePresenceMap()
{
	for (unsigned long i(0); i < presence_c; i++)
		free(presence_l[i].name);

	free(presence_l), presence_l = NULL, presence_c = 0;
}

static float GetSamplePresenceFactor(const char * name)
{
	for (unsigned long i(0); i < presence_c; i++)
		if (!strnicmp(presence_l[i].name, name, presence_l[i].name_size)) return presence_l[i].factor;

	return 1.0F;
}
LARGE_INTEGER Sstart_chrono, Send_chrono;
unsigned long BENCH_SOUND = 0;
LPTHREAD_START_ROUTINE UpdateSoundThread(char *)
{
	bExitUpdateThread = false;

	while (!bExitUpdateThread)
	{
		Sleep(ARX_SOUND_UPDATE_INTERVAL);
		QueryPerformanceCounter(&Sstart_chrono);
		aalUpdate();
		QueryPerformanceCounter(&Send_chrono);
		BENCH_SOUND = (unsigned long)(Send_chrono.QuadPart - Sstart_chrono.QuadPart);
	}

	ExitThread(0);

	return 0;
}

static void ARX_SOUND_LaunchUpdateThread()
{
	DWORD id;

	if (hUpdateThread) return;

	hUpdateThread = (HANDLE)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)UpdateSoundThread, NULL, 0, &id);

	if (hUpdateThread) SetThreadPriority(hUpdateThread, THREAD_PRIORITY_NORMAL);//_LOWEST);
}

static void ARX_SOUND_KillUpdateThread()
{
	if (!hUpdateThread) return;

	SetThreadPriority(hUpdateThread, THREAD_PRIORITY_HIGHEST);

	bExitUpdateThread = true;

	if (WaitForSingleObject(hUpdateThread, 5000) == WAIT_TIMEOUT)
		MessageBox(NULL, "Failed while killing audio thread", "ARX Fatalis - Error", MB_OK);

	CloseHandle(hUpdateThread), hUpdateThread = NULL;
}

static bool isSection(char * _lpszText)
{
	ULONG i = 0;
	unsigned long ulTextSize = strlen(_lpszText);
	bool bFirst = false;
	bool bLast = false;

	while (i < ulTextSize)
	{
		if (_lpszText[i] == '[')
		{
			if (bFirst) return false;
			else bFirst = true;
		}
		else if (_lpszText[i] == ']')
		{
			if (!bFirst) return false;

			if (bLast) return false;
			else bLast = true;
		}
		else if (isalpha(_lpszText[i]))
		{
			if (!bFirst) return false;
			else if (bFirst && bLast) return false;
		}

		i++;
	}

	if (bFirst && bLast) return true;

	return false;
}

//-----------------------------------------------------------------------------
static bool isString(char * _lpszText)
{
	ULONG i = 0;
	unsigned long ulTextSize = strlen(_lpszText);
 
 
	bool bSpace = false;
	bool bAlpha = false;

	while (i < ulTextSize)
	{
		if (_lpszText[i] == '=')
		{
			if (bSpace) return false;
			else bSpace = true;
		}
		else if (isalpha(_lpszText[i]) && !bAlpha)
		{
			bAlpha = true;
		}

		i++;
	}

	if (bSpace && bAlpha) return true;

	return false;
}

//-----------------------------------------------------------------------------
static bool isNotEmpty(char * _lpszText)
{
	ULONG i = 0;
	unsigned long ulTextSize = strlen(_lpszText);

	while (i < ulTextSize)
	{
		if (isalpha(_lpszText[i])) return true;

		i++;
	}

	return false;
}

//-----------------------------------------------------------------------------
static char * CleanSection(const char * _lpszText)
{
	unsigned long ulTextSize = strlen(_lpszText);
	char * lpszText = (char *)malloc((ulTextSize + 1) * sizeof(char));

	ZeroMemory(lpszText, (ulTextSize + 1) * sizeof(char));

	unsigned long ulPos = 0;
	bool bFirst = false;

	for (unsigned long ul = 0; ul < ulTextSize; ul++)
	{
		if (_lpszText[ul] == '[') bFirst = true;
		else if (_lpszText[ul] == ']') break;
		else if (bFirst)
		{
			lpszText[ulPos] = _lpszText[ul];
			ulPos ++;
		}
	}

	return lpszText;
}

//-----------------------------------------------------------------------------
static char * CleanString(const char * _lpszText)
{
	unsigned long ulTextSize = strlen(_lpszText);
	char * lpszText = (char *)malloc((ulTextSize + 1) * sizeof(char));

	ZeroMemory(lpszText, (ulTextSize + 1) * sizeof(char));

	unsigned long ulPos = 0;
 
 

	for (unsigned long ul = 0; ul < ulTextSize; ul++)
		if (_lpszText[ul] == '=' || _lpszText[ul] == '_' || isalnum(_lpszText[ul]))
		{
			lpszText[ulPos] = _lpszText[ul];
			++ulPos;
		}

	while (ulPos--)
	{
		if (isalpha(lpszText[ulPos])) break;
		else if (lpszText[ulPos] == '"' || lpszText[ulPos] == ' ')
			lpszText[ulPos] = 0;
	}

	return lpszText;
}

static void ARX_SOUND_ParseIniFile(char * _lpszTextFile, const unsigned long _ulFileSize, ParseIniFileCallback lpSectionCallback, ParseIniFileCallback lpStringCallback)
{
 
	char * pFile = _lpszTextFile;

	if (!lpSectionCallback || !lpStringCallback) return;

	//-------------------------------------------------------------------------
	//clean up comments
	for (unsigned long i = 0; i < _ulFileSize; i++)
		if (pFile[i] == '/' && pFile[i + 1] == '/')
		{
			unsigned long j = i;

			while (j < _ulFileSize && pFile[j] != '\r' && pFile[j + 1] != '\n')
			{
				pFile[j] = ' ';
				j++;
			}
		}

	//-------------------------------------------------------------------------
	// get all lines into list
	list<char *> lText;
	char * pLine = strtok(pFile, "\r\n");

	while (pLine)
	{
		if (isNotEmpty(pLine)) lText.insert(lText.end(), (pLine));

		pLine = strtok(NULL, "\r\n");
	}

	list<char *>::iterator it;

	//-------------------------------------------------------------------------
	// look up for sections and associated keys
	it = lText.begin();

	while (it != lText.end())
	{
		if (isSection(*it))
		{
			unsigned long lResult;
			char * lpszSection;

			lResult = (*lpSectionCallback)(lpszSection = CleanSection(*it));
			free(lpszSection);

			if (lResult == PARSE_INI_FILE_SKIP) continue;
			else if (lResult == PARSE_INI_FILE_STOP) break;

			it++;

			while ((it != lText.end()) && (!isSection(*it)))
			{
				if (isString(*it))
				{
					char * lpszString = CleanString(*it);

					lResult = (*lpStringCallback)(lpszString);
					free(lpszString);

					if (lResult == PARSE_INI_FILE_SKIP) continue;
					else if (lResult == PARSE_INI_FILE_STOP) break;

					it++;
				}
				else break;
			}

			continue;
		}

		it++;
	}

	it = it;
}
