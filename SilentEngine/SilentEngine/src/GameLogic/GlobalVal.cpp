#include "stdafx.h"
#include "GlobalVal.h"

bool GlobalVal::instanceFlag = false;
GlobalVal* GlobalVal::g_instance = nullptr;

GlobalVal* GlobalVal::getInstance() {
	if (!g_instance) {
		g_instance = new GlobalVal();
		instanceFlag = true;
	}
	return g_instance;
}

void GlobalVal::LoadModels(ModelLoader * models)
{
	m_globalModels = models;
}

void GlobalVal::LoadMaps(MapLoader * maps)
{
	m_globalMaps = maps;
}

void GlobalVal::LoadEffects(EffectLoader * effects)
{
	m_globalEffects = effects;
}

void GlobalVal::setPlayer(GameObject * object)
{
	m_player = object;
}

GameObject * GlobalVal::getPlayer()
{
	return m_player;
}