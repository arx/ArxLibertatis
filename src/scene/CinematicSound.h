
#ifndef ARX_SCENE_CINEMATICSOUND_H
#define ARX_SCENE_CINEMATICSOUND_H

#include "scene/GameSound.h" // TODO for ArxSound


class Cinematic;


#define MAX_SOUND 256


struct CinematicSound {
	short active, load;
	char * dir;
	char * name;
	char * sound;
	ArxSound idhandle;
};


void DeleteAllSound();

void InitSound(Cinematic * c);
CinematicSound * GetFreeSound(int * num);
bool DeleteFreeSound(int num);
int AddSoundToList(const std::string & path);
bool PlaySoundKeyFramer(int id);
void StopSoundKeyFramer(void);

#endif // ARX_SCENE_CINEMATICSOUND_H
