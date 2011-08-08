
#ifndef ARX_SCENE_CINEMATICSOUND_H
#define ARX_SCENE_CINEMATICSOUND_H

#include "scene/GameSound.h"
#include "io/FilePath.h"

class Cinematic;

#define MAX_SOUND 256

struct CinematicSound {
	
	CinematicSound();
	
	short active;
	fs::path file;
	ArxSound handle;
	
};

void DeleteAllSound();

CinematicSound * GetFreeSound(int * num);
bool DeleteFreeSound(int num);
int AddSoundToList(const fs::path & path);
bool PlaySoundKeyFramer(int id);
void StopSoundKeyFramer(void);

#endif // ARX_SCENE_CINEMATICSOUND_H
