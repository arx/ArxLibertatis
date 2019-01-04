/*
 * Copyright 2014-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "game/magic/SpellRecognition.h"

#include <map>
#include <string>

#include <boost/lexical_cast.hpp>

#include "core/Config.h"
#include "game/Equipment.h"
#include "game/Player.h"
#include "game/magic/Spell.h"
#include "game/spell/Cheat.h"
#include "graphics/Math.h"
#include "graphics/particle/ParticleEffects.h"
#include "math/GtxFunctions.h"
#include "scene/GameSound.h"
#include "input/Input.h"
#include "io/log/Logger.h"

static std::vector<Vec2f> plist;
bool bPrecastSpell = false;

static void handleRuneDetection(Rune rune);

typedef struct RunePattern{
	Rune runeId;
	CheatRune cheatId;
	std::string dirs;
} RunePattern;

const RunePattern patternData[] = {
	{RUNE_AAM,         CheatRune_AAM,         "6"},
	{RUNE_CETRIUS,     CheatRune_None,        "386"},
	{RUNE_COMUNICATUM, CheatRune_COMUNICATUM, "62426"},
	{RUNE_COSUM,       CheatRune_None,        "6248"},
	{RUNE_FOLGORA,     CheatRune_None,        "93"},
	{RUNE_FRIDD,       CheatRune_None,        "862"},
	{RUNE_KAOM,        CheatRune_KAOM,        "41236"},
	{RUNE_KAOM,        CheatRune_KAOM,       "1236"},
	{RUNE_MEGA,        CheatRune_MEGA,        "8"},
	{RUNE_MORTE,       CheatRune_None,        "62"},
	{RUNE_MOVIS,       CheatRune_None,        "616"},
	{RUNE_NHI,         CheatRune_None,        "4"},
	{RUNE_RHAA,        CheatRune_None,        "2"},
	{RUNE_SPACIUM,     CheatRune_SPACIUM,     "4268"},
	{RUNE_STREGUM,     CheatRune_STREGUM,     "838"},
	{RUNE_TAAR,        CheatRune_None,        "626"},
	{RUNE_TEMPUS,      CheatRune_None,        "862686"},
	{RUNE_TERA,        CheatRune_None,        "926"},
	{RUNE_VISTA,       CheatRune_None,        "31"},
	{RUNE_VITAE,       CheatRune_None,        "68"},
	{RUNE_YOK,         CheatRune_None,        "268"},
	// Cheat runes
	{RUNE_NONE,        CheatRune_O,          "9317"},
	{RUNE_NONE,        CheatRune_M,          "8392"},
	{RUNE_NONE,        CheatRune_A,          "934"},
	{RUNE_NONE,        CheatRune_Passwall,   "626262"},
	{RUNE_NONE,        CheatRune_S,          "161"},
	{RUNE_NONE,        CheatRune_F,          "86"},
	{RUNE_NONE,        CheatRune_R,          "8313"},
	{RUNE_NONE,        CheatRune_U,          "238"},
	{RUNE_NONE,        CheatRune_W,          "2829"},
	{RUNE_NONE,        CheatRune_W,          "2929"},
	{RUNE_NONE,        CheatRune_P,          "831"},
	{RUNE_NONE,        CheatRune_X,          "349"},
	{RUNE_NONE,        CheatRune_ChangeSkin, "828282"},
	{RUNE_NONE,        CheatRune_26,         "26"}
};

class RuneRecognitionAlt {
	
	static const size_t s_requiredPointCount = 20;
	static const size_t s_directionCount = s_requiredPointCount - 1;
	static const size_t s_patternCount = ARRAY_SIZE(patternData);
	static const int s_maxTolerance = 2;
	
	int m_dirs[s_directionCount];
	std::vector<Vec2f> m_points;
	std::vector<size_t> m_indices;
	
	int findMatchingPattern();
	void resampleInput(const std::vector<Vec2f> &in);
	void inputToDirs();
	void findKeyPoints(std::vector<Vec2f> &in);
	void setFailedSeq();
	static void callRuneHandlers(int index);
	static char angle2arx(int dir);
	static int arx2angle(char dir);
	static int quantizeAngleToDir(float angle);
	static int angleDiff(int angle1, int angle2);
	static float angleVectorX(Vec2f v);
	
public:
	
	void analyze();
	
};

/*!
 * Return the smallest difference between angle1 and angle2
 */
int RuneRecognitionAlt::angleDiff(int angle1, int angle2) {
	return (4 - glm::abs(glm::abs(angle1 - angle2) - 4));
}

/*!
 * Call appropriate handlers for a rune at patternData's index
 */
void RuneRecognitionAlt::callRuneHandlers(int index) {
	if(patternData[index].runeId != RUNE_NONE) {
		handleRuneDetection(patternData[index].runeId);
	}
	if(patternData[index].cheatId != CheatRune_None) {
		handleCheatRuneDetection(patternData[index].cheatId);
	}
}

/*!
 * Resample the input while keeping key points contained in m_indices
 * Based on the resample function of $1 Unistroke Recognizer
 * https://depts.washington.edu/aimgroup/proj/dollar/
 */
void RuneRecognitionAlt::resampleInput(const std::vector<Vec2f> &in) {
	
	// Find the total length
	float totalLen = 0.0;
	for(size_t i = 1; i < in.size(); i++) {
		totalLen += glm::distance(in[i - 1], in[i]);
	}
	
	m_points.push_back(in[0]);
	
	size_t segmentCount = m_indices.size() - 1;
	int pointsAdded = 0;
	float segRemains = 0.0;
	
	for(size_t segment = 0; segment < segmentCount; segment++) {
		
		// Distance along curve from key point 1 to key point 2
		float segLen = 0.0;
		for(size_t index = m_indices[segment]; index < m_indices[segment + 1]; index++) {
			segLen += glm::distance(in[index], in[index + 1]);
		}
		
		float pointsToAddFloat = (segLen / totalLen) * (s_requiredPointCount - 1) + segRemains;
		int pointsToAdd = int(glm::round(pointsToAddFloat));
		segRemains = pointsToAddFloat - float(pointsToAdd);

		if(segment != segmentCount - 1) {
			pointsAdded += pointsToAdd;
		} else {
			pointsToAdd = s_requiredPointCount - 1 - pointsAdded;
		}

		if(pointsToAdd == 0) {
			continue;
		}

		arx_assert(pointsToAdd > 0);
		
		float interval = segLen / pointsToAdd;
		float remains = 0.0;
		
		bool newPointAdded = false; // Was a new point added?
		bool endOfSegment = false; // At the end of segment
		
		size_t index = m_indices[segment] + 1;
		int reallyAdded = 0;
		size_t endIndex = m_indices[segment + 1];
		
		Vec2f prevPoint = in[m_indices[segment]];
		Vec2f thisPoint(0.f);
		
		// Add points in between key points while retaining curvature
		while(!endOfSegment || newPointAdded) {
			if(!newPointAdded) {
				if(!endOfSegment) {
					thisPoint = in[index];
					if(index == endIndex) {
						endOfSegment = true;
					} else {
						index++;
					}
				}
			} else {
				newPointAdded = false;
			}
			
			float dist = glm::distance(prevPoint, thisPoint);
			float coeff = (interval - remains) / dist;
			if((remains + dist) >= interval) {
				Vec2f p = prevPoint + coeff * (thisPoint - prevPoint);
				m_points.push_back(p);
				reallyAdded++;
				remains = 0.0;
				prevPoint = p;
				newPointAdded = true;
				continue;
			} else {
				remains += dist;
			}
			
			prevPoint = thisPoint;
		}
		if(reallyAdded == pointsToAdd - 1) {
			// Fell short of one point due to rounding error
			m_points.push_back(in[endIndex]);
		}
	}
}

/*!
 * Round the angle to the nearest direction
 */
int RuneRecognitionAlt::quantizeAngleToDir(float angle) {
	return int(glm::round(angle / (glm::pi<float>() / 4.0f))) % 8;
}

/*!
* Return the angle between a vector and the X axis in radians
* The angle is always positive
*/
float RuneRecognitionAlt::angleVectorX(Vec2f v) {
	float angle = arx::orientedAngle(glm::normalize(v), Vec2f(1, 0));
	
	if(angle < 0) {
		angle += 2 * glm::pi<float>();
	}
	return angle;
}

/*!
 * Convert a vector of input points to a sequence of directions
 */
void RuneRecognitionAlt::inputToDirs() {
	for(size_t i = 1; i < m_points.size(); i++) {
		int dir = quantizeAngleToDir(angleVectorX(m_points[i] - m_points[i - 1]));
		arx_assert(dir >= 0 && dir < 8);
		m_dirs[i - 1] = dir;
	}
}

/*!
 * Find all key points in the input (angles greater than minAngle).
 * The output will always be at least two points, the start and the end.
 * Inserts indices pointing to the in vector into m_indices.
 */
void RuneRecognitionAlt::findKeyPoints(std::vector<Vec2f> &in) {
	const float TOLERANCE = 0.30f;
	size_t inputSize = in.size();
	
	// Calculate tolerance based on the overall size of the drawing
	Vec2f max = in[0];
	Vec2f min = in[0];
	for(size_t i = 1; i < inputSize; i++) {
		max = glm::max(max, in[i]);
		min = glm::min(min, in[i]);
	}
	float currTolerance = (max.x - min.x + max.y - min.y) / 2.0f * TOLERANCE;
	
	m_indices.push_back(0);
	Vec2f lastImp = in.front();
	
	float minAngle = (2.0f / 3.0f) * glm::pi<float>();
	
	for(size_t i = 2; i < inputSize; i++) {
		Vec2f thisPoint = in[i - 1];
		Vec2f nextPoint = in[i];
		
		float distance = glm::length(lastImp - thisPoint);
		
		if(distance > currTolerance) {
			float angle2 = arx::angle(glm::normalize(lastImp - thisPoint), glm::normalize(nextPoint - thisPoint));
			if(angle2 < minAngle) {
				lastImp = thisPoint;
				m_indices.push_back(i - 1);
			}
		}
	}
	m_indices.push_back(in.size() - 1);
}

/*!
 * Convert angle based direction numbering to ARX's default
 */
char RuneRecognitionAlt::angle2arx(int dir) {
	switch(dir) {
		case 0: return '6';
		case 1: return '9';
		case 2: return '8';
		case 3: return '7';
		case 4: return '4';
		case 5: return '1';
		case 6: return '2';
		case 7: return '3';
		default: arx_unreachable();
	}
}

/*!
 * Convert ARX's default direction numbering to angle based directions
 */
int RuneRecognitionAlt::arx2angle(char dir) {
	switch(dir) {
		case '6': return 0;
		case '9': return 1;
		case '8': return 2;
		case '7': return 3;
		case '4': return 4;
		case '1': return 5;
		case '2': return 6;
		case '3': return 7;
		default: arx_unreachable();
	}
}

void RuneRecognitionAlt::setFailedSeq(){
	LAST_FAILED_SEQUENCE.clear();
	int lastDir = -1;
	for(size_t i = 0; i < s_directionCount; i++) {
		if(m_dirs[i] != lastDir) {
			LAST_FAILED_SEQUENCE.push_back(angle2arx(m_dirs[i]));
			lastDir = m_dirs[i];
		}
	}
}

/*! Compare the input directions (m_dirs) with each rune pattern
 *  Returns an index to patternData, -1 if no match is found
 */
int RuneRecognitionAlt::findMatchingPattern(){
	int index = -1;
	int min = std::numeric_limits<int>::max();
	for(size_t rune = 0; rune < s_patternCount; rune++) {
		bool refuse = false;
		int errors = 0;
		size_t patternIndex = 0, inputIndex = 0;
		size_t patternSize = patternData[rune].dirs.size();

		int curPatternDir = arx2angle(patternData[rune].dirs[0]);
		int curInputDir = m_dirs[0];
		int nextPatternDir = (patternIndex < patternSize - 1) ? arx2angle(patternData[rune].dirs[1]) : curPatternDir;
		int nextInputDir = (inputIndex < s_directionCount - 1) ? m_dirs[1] : curInputDir;
		
		while(inputIndex < s_directionCount) {
			int diff = angleDiff(curPatternDir, curInputDir);
			errors += diff;
			
			if(diff > 1 || errors > s_maxTolerance) {
				refuse = true;
				break;
			}
			
			if(patternIndex < patternSize - 1 && nextInputDir >= 0 && nextInputDir != curInputDir) {
				// If the pattern deviates, move to the next pattern dir only if the difference is smaller
				if(nextPatternDir >= 0  && (nextPatternDir == nextInputDir
					|| angleDiff(nextInputDir, curPatternDir) > angleDiff(nextInputDir, nextPatternDir))) {
					patternIndex++;
					curPatternDir = nextPatternDir;
					nextPatternDir = (patternIndex < patternSize - 1) ? arx2angle(patternData[rune].dirs[patternIndex + 1]) : curPatternDir;
				}
			}
			curInputDir = nextInputDir;
			nextInputDir = (inputIndex < s_directionCount - 1) ? m_dirs[inputIndex + 1] : curInputDir;
			inputIndex++;
		}
		
		if(patternIndex < patternSize - 1 || refuse) {
			continue;
		}
		
		if(errors < min) {
			min = errors;
			index = rune;
		}
	}
	
	if(min <= s_maxTolerance) {
		return index;
	} else {
		setFailedSeq();
		return -1;
	}
}

void RuneRecognitionAlt::analyze() {
	
	if(plist.size() < 2) {
		plist.clear();
		return;
	}
	
	m_points.clear();
	m_indices.clear();
	
	findKeyPoints(plist);
	if(m_indices.size() > s_requiredPointCount) {
		// Too deformed
		return;
	}
	resampleInput(plist);
	inputToDirs();
	
	int index = findMatchingPattern();
	if(index >= 0) {
		callRuneHandlers(index);
	} else {
		return;
	}
	
	bPrecastSpell = false;
	
	if(GInput->actionPressed(CONTROLS_CUST_STEALTHMODE)) {
		bPrecastSpell = true;
	}
	
}

RuneRecognitionAlt g_altRecognizer;

void ARX_SPELLS_Analyse_Alt() {
	g_altRecognizer.analyze();
}

static const size_t MAX_POINTS(200);

Rune SpellSymbol[MAX_SPELL_SYMBOLS];

size_t CurrSpellSymbol = 0;
std::string SpellMoves; // Rune directions encoded as numpad keys

std::string LAST_FAILED_SEQUENCE = "none";

struct SpellDefinition {
	SpellDefinition * next[RUNE_COUNT];
	SpellType spell;
	SpellDefinition() : spell(SPELL_NONE) {
		for(size_t i = 0; i < RUNE_COUNT; i++) {
			next[i] = NULL;
		}
	}

	~SpellDefinition() {
		for(size_t i = 0; i < RUNE_COUNT; i++) {
			delete next[i];
		}
	}
};
static SpellDefinition definedSpells;

typedef std::map<std::string, SpellType> SpellNames;
static SpellNames spellNames;

struct RawSpellDefinition {
	SpellType spell;
	std::string name;
	Rune symbols[MAX_SPELL_SYMBOLS];
};

// TODO move to external file
static const RawSpellDefinition allSpells[] = {
	{SPELL_CURSE,                 "curse",                 {RUNE_RHAA,    RUNE_STREGUM,     RUNE_VITAE,   RUNE_NONE}}, // level 4
	{SPELL_FREEZE_TIME,           "freeze_time",           {RUNE_RHAA,    RUNE_TEMPUS,      RUNE_NONE}}, // level 10
	{SPELL_LOWER_ARMOR,           "lower_armor",           {RUNE_RHAA,    RUNE_KAOM,        RUNE_NONE}}, // level 2
	{SPELL_SLOW_DOWN,             "slowdown",              {RUNE_RHAA,    RUNE_MOVIS,       RUNE_NONE}}, // level 6
	{SPELL_HARM,                  "harm",                  {RUNE_RHAA,    RUNE_VITAE,       RUNE_NONE}}, // level 2
	{SPELL_CONFUSE,               "confuse",               {RUNE_RHAA,    RUNE_VISTA,       RUNE_NONE}}, // level 7
	{SPELL_MASS_PARALYSE,         "mass_paralyse",         {RUNE_MEGA,    RUNE_NHI,         RUNE_MOVIS,   RUNE_NONE}}, // level 9
	{SPELL_ARMOR,                 "armor",                 {RUNE_MEGA,    RUNE_KAOM,        RUNE_NONE}}, // level 2
	{SPELL_MAGIC_SIGHT,           "magic_sight",           {RUNE_MEGA,    RUNE_VISTA,       RUNE_NONE}}, // level 1
	{SPELL_HEAL,                  "heal",                  {RUNE_MEGA,    RUNE_VITAE,       RUNE_NONE}}, // level 2
	{SPELL_SPEED,                 "speed",                 {RUNE_MEGA,    RUNE_MOVIS,       RUNE_NONE}}, // level 3
	{SPELL_BLESS,                 "bless",                 {RUNE_MEGA,    RUNE_STREGUM,     RUNE_VITAE,   RUNE_NONE}}, // level 4
	{SPELL_ENCHANT_WEAPON,        "enchant_weapon",        {RUNE_MEGA,    RUNE_STREGUM,     RUNE_COSUM,   RUNE_NONE}}, // level 8
	{SPELL_MASS_INCINERATE,       "mass_incinerate",       {RUNE_MEGA,    RUNE_AAM,         RUNE_MEGA,    RUNE_YOK, RUNE_NONE}}, // level 10
	{SPELL_ACTIVATE_PORTAL,       "activate_portal",       {RUNE_MEGA,    RUNE_SPACIUM,     RUNE_NONE}}, // level ?
	{SPELL_LEVITATE,              "levitate",              {RUNE_MEGA,    RUNE_SPACIUM,     RUNE_MOVIS,   RUNE_NONE}}, // level 5
	{SPELL_PARALYSE,              "paralyse",              {RUNE_NHI,     RUNE_MOVIS,       RUNE_NONE}}, // level 6
	{SPELL_CURE_POISON,           "cure_poison",           {RUNE_NHI,     RUNE_CETRIUS,     RUNE_NONE}}, // level 5
	{SPELL_DOUSE,                 "douse",                 {RUNE_NHI,     RUNE_YOK,         RUNE_NONE}}, // level 1
	{SPELL_DISPELL_ILLUSION,      "dispell_illusion",      {RUNE_NHI,     RUNE_STREGUM,     RUNE_VISTA,   RUNE_NONE}}, // level 3
	{SPELL_NEGATE_MAGIC,          "negate_magic",          {RUNE_NHI,     RUNE_STREGUM,     RUNE_SPACIUM, RUNE_NONE}}, // level 9
	{SPELL_DISPELL_FIELD,         "dispell_field",         {RUNE_NHI,     RUNE_SPACIUM,     RUNE_NONE}}, // level 4
	{SPELL_DISARM_TRAP,           "disarm_trap",           {RUNE_NHI,     RUNE_MORTE,       RUNE_COSUM,   RUNE_NONE}}, // level 6
	{SPELL_INVISIBILITY,          "invisibility",          {RUNE_NHI,     RUNE_VISTA,       RUNE_NONE}}, // level ?
	{SPELL_FLYING_EYE,            "flying_eye",            {RUNE_VISTA,   RUNE_MOVIS,       RUNE_NONE}}, // level 7
	{SPELL_REPEL_UNDEAD,          "repel_undead",          {RUNE_MORTE,   RUNE_KAOM,        RUNE_NONE}}, // level 5
	{SPELL_DETECT_TRAP,           "detect_trap",           {RUNE_MORTE,   RUNE_COSUM,       RUNE_VISTA,   RUNE_NONE}}, // level 2
	{SPELL_CONTROL_TARGET,        "control",               {RUNE_MOVIS,   RUNE_COMUNICATUM, RUNE_NONE}}, // level 10
	{SPELL_MANA_DRAIN,            "mana_drain",            {RUNE_STREGUM, RUNE_MOVIS,       RUNE_NONE}}, // level 8
	{SPELL_INCINERATE,            "incinerate",            {RUNE_AAM,     RUNE_MEGA,        RUNE_YOK,     RUNE_NONE}}, // level 9
	{SPELL_EXPLOSION,             "explosion",             {RUNE_AAM,     RUNE_MEGA,        RUNE_MORTE,   RUNE_NONE}}, // level 8
	{SPELL_CREATE_FIELD,          "create_field",          {RUNE_AAM,     RUNE_KAOM,        RUNE_SPACIUM, RUNE_NONE}}, // level 6
	{SPELL_RISE_DEAD,             "raise_dead",            {RUNE_AAM,     RUNE_MORTE,       RUNE_VITAE,   RUNE_NONE}}, // level 6
	{SPELL_RUNE_OF_GUARDING,      "rune_of_guarding",      {RUNE_AAM,     RUNE_MORTE,       RUNE_COSUM,   RUNE_NONE}}, // level 5
	{SPELL_SUMMON_CREATURE,       "summon_creature",       {RUNE_AAM,     RUNE_VITAE,       RUNE_TERA,    RUNE_NONE}}, // level 9
	{SPELL_CREATE_FOOD,           "create_food",           {RUNE_AAM,     RUNE_VITAE,       RUNE_COSUM,   RUNE_NONE}}, // level 3
	{SPELL_LIGHTNING_STRIKE,      "lightning_strike",      {RUNE_AAM,     RUNE_FOLGORA,     RUNE_TAAR,    RUNE_NONE}}, // level 7
	{SPELL_MASS_LIGHTNING_STRIKE, "mass_lightning_strike", {RUNE_AAM,     RUNE_FOLGORA,     RUNE_SPACIUM, RUNE_NONE}}, // level 10
	{SPELL_IGNIT,                 "ignit",                 {RUNE_AAM,     RUNE_YOK,         RUNE_NONE}}, // level 1
	{SPELL_FIRE_FIELD,            "fire_field",            {RUNE_AAM,     RUNE_YOK,         RUNE_SPACIUM, RUNE_NONE}}, // level 7
	{SPELL_FIREBALL,              "fireball",              {RUNE_AAM,     RUNE_YOK,         RUNE_TAAR,    RUNE_NONE}}, // level 3
	{SPELL_ICE_FIELD,             "ice_field",             {RUNE_AAM,     RUNE_FRIDD,       RUNE_SPACIUM, RUNE_NONE}}, // level 7
	{SPELL_ICE_PROJECTILE,        "ice_projectile",        {RUNE_AAM,     RUNE_FRIDD,       RUNE_TAAR,    RUNE_NONE}}, // level 3
	{SPELL_POISON_PROJECTILE,     "poison_projectile",     {RUNE_AAM,     RUNE_CETRIUS,     RUNE_TAAR,    RUNE_NONE}}, // level 5
	{SPELL_MAGIC_MISSILE,         "magic_missile",         {RUNE_AAM,     RUNE_TAAR,        RUNE_NONE}}, // level 1
	{SPELL_FIRE_PROTECTION,       "fire_protection",       {RUNE_YOK,     RUNE_KAOM,        RUNE_NONE}}, // level 4
	{SPELL_COLD_PROTECTION,       "cold_protection",       {RUNE_FRIDD,   RUNE_KAOM,        RUNE_NONE}}, // level 4
	{SPELL_LIFE_DRAIN,            "life_drain",            {RUNE_VITAE,   RUNE_MOVIS,       RUNE_NONE}}, // level 8
	{SPELL_TELEKINESIS,           "telekinesis",           {RUNE_SPACIUM, RUNE_COMUNICATUM, RUNE_NONE}}, // level 4
	{SPELL_FAKE_SUMMON,           "fake_summon",           {RUNE_NONE}}
};

static void addSpell(const Rune symbols[MAX_SPELL_SYMBOLS], SpellType spell, const std::string & name) {
	
	typedef std::pair<SpellNames::const_iterator, bool> Res;
	Res res = spellNames.insert(std::make_pair(name, spell));
	if(!res.second) {
		LogWarning << "Duplicate spell name: " + name;
	}
	
	if(symbols[0] == RUNE_NONE) {
		return;
	}
	
	SpellDefinition * def = &definedSpells;
	
	for(size_t i = 0; i < MAX_SPELL_SYMBOLS; i++) {
		if(symbols[i] == RUNE_NONE) {
			break;
		}
		arx_assert(symbols[i] >= 0 && (size_t)symbols[i] < RUNE_COUNT);
		if(def->next[symbols[i]] == NULL) {
			def->next[symbols[i]] = new SpellDefinition();
		}
		def = def->next[symbols[i]];
	}
	
	arx_assert(def->spell == SPELL_NONE);
	
	def->spell = spell;
}

void spellRecognitionInit() {
	
	for(size_t i = 0; i < ARRAY_SIZE(allSpells); i++) {
		addSpell(allSpells[i].symbols, allSpells[i].spell, allSpells[i].name);
	}

	plist.reserve(MAX_POINTS);
}

//-----------------------------------------------------------------------------
// Resets Spell Recognition
void ARX_SPELLS_ResetRecognition() {
	
	for(size_t i = 0; i < MAX_SPELL_SYMBOLS; i++) {
		SpellSymbol[i] = RUNE_NONE;
	}
	
	for(size_t i = 0; i < 6; i++) {
		player.SpellToMemorize.iSpellSymbols[i] = RUNE_NONE;
	}
	
	CurrSpellSymbol = 0;
}


void spellRecognitionPointsReset() {
	plist.clear();
}

// Adds a 2D point to currently drawn spell symbol
void ARX_SPELLS_AddPoint(const Vec2s & pos) {
	
	if(!plist.empty() && Vec2f(pos) == plist.back()) {
		return;
	}
	
	if(plist.size() == MAX_POINTS) {
		plist.pop_back();
	}
	
	plist.push_back(Vec2f(pos));
}




SpellType GetSpellId(const std::string & spell) {
	
	SpellNames::const_iterator it = spellNames.find(spell);
	
	return (it == spellNames.end()) ? SPELL_NONE : it->second;
}

enum ARX_SPELLS_RuneDirection
{
	AUP,
	AUPRIGHT,
	ARIGHT,
	ADOWNRIGHT,
	ADOWN,
	ADOWNLEFT,
	ALEFT,
	AUPLEFT
};





static SpellType getSpell(const Rune symbols[MAX_SPELL_SYMBOLS]) {
	
	const SpellDefinition * def = &definedSpells;
	
	for(size_t i = 0; i < MAX_SPELL_SYMBOLS; i++) {
		if(symbols[i] == RUNE_NONE) {
			break;
		}
		arx_assert(symbols[i] >= 0 && (size_t)symbols[i] < RUNE_COUNT);
		if(def->next[symbols[i]] == NULL) {
			return SPELL_NONE;
		}
		def = def->next[symbols[i]];
	}
	
	return def->spell;
}



void ARX_SPELLS_Analyse() {
	
	unsigned char dirs[MAX_POINTS];
	unsigned char lastdir = 255;
	long cdir = 0;

	for(size_t i = 1; i < plist.size() ; i++) {
		
		Vec2f d = plist[i - 1] - plist[i];
		
		if(arx::length2(d) > 100) {
			
			float a = std::abs(d.x);
			float b = std::abs(d.y);
			
			if(b != 0.f && a / b > 0.4f && a / b < 2.5f) {
				// Diagonal movemement.
				
				if(d.x < 0 && d.y < 0) {
					if(lastdir != ADOWNRIGHT) {
						lastdir = dirs[cdir++] = ADOWNRIGHT;
					}
				} else if(d.x > 0 && d.y < 0) {
					if(lastdir != ADOWNLEFT) {
						lastdir = dirs[cdir++] = ADOWNLEFT;
					}
				} else if(d.x < 0 && d.y > 0) {
					if(lastdir != AUPRIGHT) {
						lastdir = dirs[cdir++] = AUPRIGHT;
					}
				} else if(d.x > 0 && d.y > 0) {
					if(lastdir != AUPLEFT) {
						lastdir = dirs[cdir++] = AUPLEFT;
					}
				}
				
			} else if(a > b) {
				// Horizontal movement.
				
				if(d.x < 0) {
					if(lastdir != ARIGHT) {
						lastdir = dirs[cdir++] = ARIGHT;
					}
				} else {
					if(lastdir != ALEFT) {
						lastdir = dirs[cdir++] = ALEFT;
					}
				}
				
			} else {
				// Vertical movement.
				
				if(d.y < 0) {
					if(lastdir != ADOWN) {
						lastdir = dirs[cdir++] = ADOWN;
					}
				} else {
					if(lastdir != AUP) {
						lastdir = dirs[cdir++] = AUP;
					}
				}
			}
		}
	}

	SpellMoves.clear();
	
	if ( cdir > 0 )
	{

		for (long i = 0 ; i < cdir ; i++ )
		{
			switch ( dirs[i] )
			{
				case AUP:
					SpellMoves += "8";
					break;

				case ADOWN:
					SpellMoves += "2";
					break;

				case ALEFT:
					SpellMoves += "4";
					break;

				case ARIGHT:
					SpellMoves += "6";
					break;

				case AUPRIGHT:
					SpellMoves += "9";
					break;

				case ADOWNRIGHT:
					SpellMoves += "3";
					break;

				case AUPLEFT:
					SpellMoves += "7";
					break;

				case ADOWNLEFT:
					SpellMoves += "1";
					break;
			}
		}
	}
}

static void handleRuneDetection(Rune rune) {
	SpellSymbol[CurrSpellSymbol++] = rune;

	if(CurrSpellSymbol >= MAX_SPELL_SYMBOLS) {
		CurrSpellSymbol = MAX_SPELL_SYMBOLS - 1;
	}
	
	ARX_SOUND_PlaySFX(g_snd.SYMB[rune]);
}

static void unrecognizedRune() {
	
	if(SpellMoves.length() >= 127) {
		SpellMoves.resize(127);
	}
	
	LAST_FAILED_SEQUENCE = SpellMoves;
	LogDebug("Unknown Symbol - " + SpellMoves);
	
}

void ARX_SPELLS_AnalyseSYMBOL() {
	
	long sm = 0;
	try {
		sm = boost::lexical_cast<long>(SpellMoves);
	} catch(...) {
		LogDebug("bad spell moves: " << SpellMoves);
	}
	
	switch(sm) {
		
		// COSUM
		case 62148:
		case 632148:
		case 62498:
		case 62748:
		case 6248:
			handleRuneDetection(RUNE_COSUM);
			break;
		// COMUNICATUM
		case 632426:
		case 627426:
		case 634236:
		case 624326:
		case 62426:
			handleCheatRuneDetection(CheatRune_COMUNICATUM);
			handleRuneDetection(RUNE_COMUNICATUM);
			break;
		// FOLGORA
		case 9823:
		case 9232:
		case 983:
		case 963:
		case 923:
		case 932:
		case 93:
			handleRuneDetection(RUNE_FOLGORA);
			break;
		// SPACIUM
		case 42368:
		case 42678:
		case 42698:
		case 4268:
			handleCheatRuneDetection(CheatRune_SPACIUM);
			handleRuneDetection(RUNE_SPACIUM);
			break;
		// TERA
		case 9826:
		case 92126:
		case 9264:
		case 9296:
		case 926:
			handleRuneDetection(RUNE_TERA);
			break;
		// CETRIUS
		case 286:
		case 3286:
		case 23836:
		case 38636:
		case 2986:
		case 2386:
		case 386:
			handleRuneDetection(RUNE_CETRIUS);
			break;
		// RHAA
		case 28:
		case 2:
			handleRuneDetection(RUNE_RHAA);
			break;
		// FRIDD
		case 98362:
		case 8362:
		case 8632:
		case 8962:
		case 862:
			handleRuneDetection(RUNE_FRIDD);
			break;
		// KAOM
		case 41236:
		case 23:
		case 236:
		case 2369:
		case 136:
		case 12369:
		case 1236:
			handleCheatRuneDetection(CheatRune_KAOM);
			handleRuneDetection(RUNE_KAOM);
			break;
		// STREGUM
		case 82328:
		case 8328:
		case 2328:
		case 8938:
		case 8238:
		case 838:
			handleCheatRuneDetection(CheatRune_STREGUM);
			handleRuneDetection(RUNE_STREGUM);
			break;
		// MORTE
		case 628:
		case 621:
		case 62:
			handleRuneDetection(RUNE_MORTE);
			break;
		// TEMPUS
		case 962686:
		case 862686:
		case 8626862:
			handleRuneDetection(RUNE_TEMPUS);
			break;
		// MOVIS
		case 6316:
		case 61236:
		case 6146:
		case 61216:
		case 6216:
		case 6416:
		case 62126:
		case 61264:
		case 6126:
		case 6136:
		case 616:
			handleRuneDetection(RUNE_MOVIS);
			break;
		// NHI
		case 46:
		case 4:
			handleRuneDetection(RUNE_NHI);
			break;
		// AAM
		case 64:
		case 6:
			handleCheatRuneDetection(CheatRune_AAM);
			handleRuneDetection(RUNE_AAM);
			break;
		// YOK
		case 412369:
		case 2687:
		case 2698:
		case 2638:
		case 26386:
		case 2368:
		case 2689:
		case 268:
			handleRuneDetection(RUNE_YOK);
			break;
		// TAAR
		case 6236:
		case 6264:
		case 626:
			handleRuneDetection(RUNE_TAAR);
			break;
		// MEGA
		case 82:
		case 8:
			handleCheatRuneDetection(CheatRune_MEGA);
			handleRuneDetection(RUNE_MEGA);
			break;
		// VISTA
		case 3614:
		case 361:
		case 341:
		case 3212:
		case 3214:
		case 312:
		case 314:
		case 321:
		case 31:
			handleRuneDetection(RUNE_VISTA);
			break;
		// VITAE
		case 698:
		case 68:
			handleRuneDetection(RUNE_VITAE);
			break;
		//-----------------------------------------------
		// Cheat spells
		// Special UW mode
		case 238:
		case 2398:
		case 23898:
		case 236987:
		case 23698:
			handleCheatRuneDetection(CheatRune_U);
			unrecognizedRune();
			break;
		case 2382398:
		case 2829:
		case 23982398:
		case 39892398:
		case 2398938:
		case 28239898:
		case 238982398:
		case 238923898:
		case 28982398:
		case 3923989:
		case 292398:
		case 398329:
		case 38923898:
		case 2398289:
		case 289823898:
		case 2989238:
		case 29829:
		case 2393239:
		case 38239:
		case 239829:
		case 2898239:
		case 28982898:
		case 389389:
		case 3892389:
		case 289289:
		case 289239:
		case 239289:
		case 2989298:
		case 2392398:
		case 238929:
		case 28923898:
		case 2929:
		case 2398298:
		case 239823898:
		case 28238:
		case 2892398:
		case 28298:
		case 298289:
		case 38929:
		case 289298989:
		case 23892398:
		case 238239:
		case 29298:
		case 2329298:
		case 232389829:
		case 2389829:
		case 239239:
		case 282398:
		case 2389239:
		case 2929898:
		case 3292398:
		case 23923298:
		case 23898239:
		case 3232929:
		case 2982398:
		case 238298:
		case 3939:
			handleCheatRuneDetection(CheatRune_W);
			unrecognizedRune();
			break;
		case 161:
		case 1621:
		case 1261:
			handleCheatRuneDetection(CheatRune_S);
			unrecognizedRune();
			break;
		case 83614:
		case 8361:
		case 8341:
		case 83212:
		case 83214:
		case 8312:
		case 8314:
		case 8321:
		case 831:
		case 82341:
		case 834:
		case 823:
		case 8234:
		case 8231:
			handleCheatRuneDetection(CheatRune_P);
			unrecognizedRune();
			break;
		case 83692:
		case 823982:
		case 83982:
		case 82369892:
		case 82392:
		case 83892:
		case 823282:
		case 8392:
			handleCheatRuneDetection(CheatRune_M);
			unrecognizedRune();
			break;
		case 98324:
		case 92324:
		case 89324:
		case 9324:
		case 9892324:
		case 9234:
		case 934:
			handleCheatRuneDetection(CheatRune_A);
			unrecognizedRune();
			break;
		case 3249:
		case 2349:
		case 323489:
		case 23249:
		case 3489:
		case 32498:
		case 349:
			handleCheatRuneDetection(CheatRune_X);
			unrecognizedRune();
			break;
		case 26:
			handleCheatRuneDetection(CheatRune_26);
			unrecognizedRune();
			break;
		case 9232187:
		case 93187:
		case 9234187:
		case 831878:
		case 923187:
		case 932187:
		case 93217:
		case 9317:
			handleCheatRuneDetection(CheatRune_O);
			unrecognizedRune();
			break;
		case 82313:
		case 8343:
		case 82343:
		case 83413:
		case 8313:
			handleCheatRuneDetection(CheatRune_R);
			unrecognizedRune();
			break;
		case 86:
			handleCheatRuneDetection(CheatRune_F);
			unrecognizedRune();
			break;
		case 626262:
			handleCheatRuneDetection(CheatRune_Passwall);
			break;
		case 828282:
			handleCheatRuneDetection(CheatRune_ChangeSkin);
			unrecognizedRune();
			break;
		default: {
			unrecognizedRune();
		}
	}
	
	bPrecastSpell = GInput->actionPressed(CONTROLS_CUST_STEALTHMODE);
	
}

bool ARX_SPELLS_AnalyseSPELL() {
	
	SpellcastFlags flags = 0;
	
	if(GInput->actionPressed(CONTROLS_CUST_STEALTHMODE) || bPrecastSpell) {
		flags |= SPELLCAST_FLAG_PRECAST;
	}
	
	bPrecastSpell = false;
	
	SpellType spell;
	
	if(SpellSymbol[0] == RUNE_MEGA && SpellSymbol[1] == RUNE_MEGA
	   && SpellSymbol[2] == RUNE_MEGA && SpellSymbol[3] == RUNE_AAM
	   && SpellSymbol[4] == RUNE_VITAE && SpellSymbol[5] == RUNE_TERA) {
		cur_mega = 10;
		spell = SPELL_SUMMON_CREATURE;
	} else {
		spell = getSpell(SpellSymbol);
	}
	
	if(spell == SPELL_NONE) {
		ARX_SOUND_PlaySFX(g_snd.MAGIC_FIZZLE);
		
		if(player.SpellToMemorize.bSpell) {
			CurrSpellSymbol = 0;
			player.SpellToMemorize.bSpell = false;
		}
		
		return false;
	}
	
	return ARX_SPELLS_Launch(spell,
	                         EntityHandle_Player,
	                         flags,
	                         -1,
	                         EntityHandle(),
	                         GameDuration::ofRaw(-1));
	
}
