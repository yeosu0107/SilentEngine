#include "stdafx.h"
#include "SoundMgr.h"

SoundMgr* SoundMgr::instance = nullptr;

SoundMgr * SoundMgr::getInstance()
{
	if (instance == nullptr) {
		instance = new SoundMgr();
	}
	return instance;
}

void SoundMgr::init()
{
	//fmod 按眉 积己
	System_Create(&m_fmod);
	m_fmod->init(10, FMOD_INIT_NORMAL, nullptr);

	//BGM 荤款靛 积己
	m_fmod->createSound("res\\Sound\\fantasy_1.wav", FMOD_LOOP_NORMAL, nullptr, &m_sound[BGM_FANTASY_1]);
	m_fmod->createSound("res\\Sound\\fantasy_2.wav", FMOD_LOOP_NORMAL, nullptr, &m_sound[BGM_FANTASY_2]);
	m_fmod->createSound("res\\Sound\\fantasy_3.wav", FMOD_LOOP_NORMAL, nullptr, &m_sound[BGM_FANTASY_3]);
	m_fmod->createSound("res\\Sound\\scary_1.wav", FMOD_LOOP_NORMAL, nullptr, &m_sound[BGM_SCARY_1]);
	m_fmod->createSound("res\\Sound\\scary_2.wav", FMOD_LOOP_NORMAL, nullptr, &m_sound[BGM_SCARY_2]);
	m_fmod->createSound("res\\Sound\\scary_3.wav", FMOD_LOOP_NORMAL, nullptr, &m_sound[BGM_SCARY_3]);
}

void SoundMgr::play(SOUND index, CHANNEL chNum)
{
	m_fmod->update();
	m_fmod->playSound(m_sound[index], nullptr, false, &m_ch[chNum]);
}

void SoundMgr::ch_stop(CHANNEL chNum)
{
	m_ch[chNum]->stop();
}

void SoundMgr::changeSound(SOUND index, CHANNEL chNum)
{
	ch_stop(chNum);
	play(index, chNum);
}
