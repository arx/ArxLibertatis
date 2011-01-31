
#ifndef ARX_ANIMATION_CINEMATICKEYFRAMER_H
#define ARX_ANIMATION_CINEMATICKEYFRAMER_H

#include "animation/Cinematic.h" // for CinematicLight
#include "graphics/GraphicsTypes.h" // for EERIE_3D


#define INTERP_NO -1
#define INTERP_BEZIER 0
#define INTERP_LINEAR 1


// TODO used for loading
// Mainly in CinematicKeyframer
#pragma pack(push,1)
struct C_KEY {
	
	enum Language {
		French = 0,
		German = 1,
		Spanish = 2,
		English = 3
	};
	
	int frame;
	int numbitmap;
	int fx; // associated fx
	short typeinterp, force;
	EERIE_3D pos;
	float angz;
	int color;
	int colord;
	int colorf;
	float speed;
	CinematicLight light;
	EERIE_3D posgrille;
	float angzgrille;
	float speedtrack;
	int idsound[16];	// 16 languages max.
};
#pragma pack(pop)

// TODO used for loading
#pragma pack(push,1)
struct CinematicTrack {
	int startframe;
	int endframe;
	float currframe;
	float fps;
	int nbkey;
	int pause;
	C_KEY * key;
};
#pragma pack(pop)


bool DeleteTrack();
bool AllocTrack(int sf, int ef, float fps);
bool AddKey(C_KEY * key, bool writecolor, bool writecolord, bool writecolorf);
bool AddKeyLoad(C_KEY * key);
void AddDiffKey(Cinematic * c, C_KEY * key, bool writecolor, bool writecolord, bool writecolorf);
bool GereTrack(Cinematic * c, float fpscurr);

void PlayTrack(Cinematic * c);
int GetCurrentFrame();
int GetStartFrame();
int GetEndFrame();
void SetCurrFrame(int frame);
bool GereTrackNoPlay(Cinematic * c);
float GetTrackFPS();

C_KEY * GetKey(int f, int * num);
C_KEY * SearchKey(int f, int * num);

float GetTimeKeyFramer(Cinematic * c);
void InitUndo();
void UpDateAllKeyLight();

#endif // ARX_ANIMATION_CINEMATICKEYFRAMER_H
