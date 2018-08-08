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
	m_fmod->createSound("res\\Sound\\main.mp3", FMOD_LOOP_NORMAL, nullptr, &m_sound[MAIN]);
	m_fmod->createSound("res\\Sound\\fantasy_1.mp3", FMOD_LOOP_NORMAL, nullptr, &m_sound[BGM_FANTASY_1]);
	m_fmod->createSound("res\\Sound\\fantasy_2.mp3", FMOD_LOOP_NORMAL, nullptr, &m_sound[BGM_FANTASY_2]);
	m_fmod->createSound("res\\Sound\\fantasy_3.mp3", FMOD_LOOP_NORMAL, nullptr, &m_sound[BGM_FANTASY_3]);
	m_fmod->createSound("res\\Sound\\scary_1.wav", FMOD_LOOP_NORMAL, nullptr, &m_sound[BGM_SCARY_1]);
	m_fmod->createSound("res\\Sound\\scary_2.wav", FMOD_LOOP_NORMAL, nullptr, &m_sound[BGM_SCARY_2]);
	m_fmod->createSound("res\\Sound\\scary_3.wav", FMOD_LOOP_NORMAL, nullptr, &m_sound[BGM_SCARY_3]);
	m_fmod->createSound("res\\Sound\\boss.mp3", FMOD_LOOP_NORMAL, nullptr, &m_sound[BOSS]);

	//FX 荤款靛 积己
	m_fmod->createSound("res\\Sound\\hit01.mp3", FMOD_LOOP_OFF, nullptr, &m_sound[HIT01]);
	m_fmod->createSound("res\\Sound\\hit02.mp3", FMOD_LOOP_OFF, nullptr, &m_sound[HIT02]);
	m_fmod->createSound("res\\Sound\\hit03.mp3", FMOD_LOOP_OFF, nullptr, &m_sound[KICK01]);
	m_fmod->createSound("res\\Sound\\hit04.mp3", FMOD_LOOP_OFF, nullptr, &m_sound[KICK02]);
	m_fmod->createSound("res\\Sound\\hit05.mp3", FMOD_LOOP_OFF, nullptr, &m_sound[PUNCH01]);

	m_fmod->createSound("res\\Sound\\room_clear.mp3", FMOD_LOOP_OFF, nullptr, &m_sound[ROOM_CLEAR]);
	m_fmod->createSound("res\\Sound\\die.mp3", FMOD_LOOP_OFF, nullptr, &m_sound[DIE]);
	m_fmod->createSound("res\\Sound\\victory.mp3", FMOD_LOOP_OFF, nullptr, &m_sound[VICTORY]);

	//Player 荤款靛 积己
	m_fmod->createSound("res\\Sound\\Attack01.mp3", FMOD_LOOP_OFF, nullptr, &m_sound[ATT01]);
	m_fmod->createSound("res\\Sound\\Attack02.mp3", FMOD_LOOP_OFF, nullptr, &m_sound[ATT02]);
	m_fmod->createSound("res\\Sound\\Attack03.mp3", FMOD_LOOP_OFF, nullptr, &m_sound[ATT03]);
	m_fmod->createSound("res\\Sound\\Skill01.mp3", FMOD_LOOP_OFF, nullptr, &m_sound[SKILL01]);
	m_fmod->createSound("res\\Sound\\Skill02.mp3", FMOD_LOOP_OFF, nullptr, &m_sound[SKILL02]);
	m_fmod->createSound("res\\Sound\\Skill03.mp3", FMOD_LOOP_OFF, nullptr, &m_sound[SKILL03]);
	m_fmod->createSound("res\\Sound\\Skill04.mp3", FMOD_LOOP_OFF, nullptr, &m_sound[SKILL04]);
	m_fmod->createSound("res\\Sound\\hurt01.mp3", FMOD_LOOP_OFF, nullptr, &m_sound[HURT01]);
	m_fmod->createSound("res\\Sound\\hurt02.mp3", FMOD_LOOP_OFF, nullptr, &m_sound[HURT02]);
	m_fmod->createSound("res\\Sound\\wind_01.mp3", FMOD_LOOP_OFF, nullptr, &m_sound[WIND01]);
	m_fmod->createSound("res\\Sound\\wind_02.mp3", FMOD_LOOP_OFF, nullptr, &m_sound[WIND02]);
	m_fmod->createSound("res\\Sound\\player_hitted.mp3", FMOD_LOOP_OFF, nullptr, &m_sound[HITTED]);
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
