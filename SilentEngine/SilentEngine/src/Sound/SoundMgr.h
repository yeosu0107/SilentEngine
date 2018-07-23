#pragma once

#include "fmod.hpp"
#include "fmod_errors.h"

#pragma comment(lib, "lib/fmod64_vc.lib")

using namespace FMOD;



enum SOUND {
	BGM_FANTASY_1 = 0, BGM_FANTASY_2 = 1, BGM_FANTASY_3,
	BGM_SCARY_1, BGM_SCARY_2, BGM_SCARY_3,
	HIT01, KICK01, KICK02, PUNCH01, HIT02,
	ATT01, ATT02, ATT03, SKILL01, SKILL02, SKILL03, SKILL04
};

inline SOUND SOUNDEFFECT(UINT index) {
	//index를 넣으면 BGM 다음 인덱스를 반환
	return static_cast<SOUND>(6 + index);
}

enum CHANNEL {
	BGM = 0, FX = 1, PLAYER = 2
};

class SoundMgr
{
private:
	static SoundMgr*	instance;

	System*					m_fmod;
	Channel*				m_ch[10];
	Sound*					m_sound[20];
public:
	static SoundMgr * getInstance();

	void init();
	void play(SOUND index, CHANNEL chNum);
	void ch_stop(CHANNEL chNum);
	void changeSound(SOUND index, CHANNEL chNum);
};