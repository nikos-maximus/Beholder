#ifndef BH_SOUND_H
#define BH_SOUND_H

#include <irrKlang.h>
#include <glm/glm.hpp>

class TiXmlElement;

namespace bh{

typedef irrklang::ISoundSource* soundSource;

struct AudioParams
{
	AudioParams();
	void LoadXML(TiXmlElement const* workingEl);
	void SaveXML(TiXmlElement* workingEl);
	float masterGain;
};

bool InitSound();
void ShutdownSound();
void PlaySoundSource(soundSource srcId);
void PlaySoundSource(soundSource srcId,glm::vec3 pos);
soundSource LoadSoundEffect(char const* name);
void UnloadSound(irrklang::ISoundSource* srcId);
void RemoveAllSounds();
bool IsSoundPlaying(soundSource srcId);

};

#endif
