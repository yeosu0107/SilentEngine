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

void GlobalVal::LoadFirePos(FirePositionLoader * firepos)
{
	m_globalFirePos = firepos;
}

void GlobalVal::setPlayer(GameObject * object)
{
	m_player = object;
}

void GlobalVal::setEnemy(GameObject ** objects)
{
	m_Enemys = objects;
}

void GlobalVal::setPorjectile(void * pContext)
{
	m_projecttile = pContext;
}
//void GlobalVal::setHitPaticle(void * pContext)
//{
//	m_hitPaticle = pContext;
//}
void GlobalVal::setFogEnable(bool bEnabled)
{
	m_bFogEnable = bEnabled;
}
void GlobalVal::setCamera(Camera * camera)
{
	m_sceneCamera = camera;
}
void GlobalVal::setPaticle(int type, XMFLOAT3 * pos)
{
	paticleType = type;
	m_hitPoint[0] = pos[0];
	m_hitPoint[1] = pos[1];
	draw_paticle = true;
}
GameObject * GlobalVal::getPlayer()
{
	return m_player;
}

GameObject ** GlobalVal::getEnemy()
{
	return m_Enemys;
}

bool GlobalVal::isDrawPaticle(int & type, XMFLOAT3* pos)
{
	if (draw_paticle) {
		type = paticleType;
		pos[0] = m_hitPoint[0];
		pos[1] = m_hitPoint[1];
		draw_paticle = false;
		return true;
	}
	return false;
}
