#pragma once

//�̱��� Ŭ����
class GlobalVal {
private:
	GlobalVal() {}

	static bool			instanceFlag;
	static GlobalVal* g_instance;
private:
	ModelLoader*		m_globalModels = nullptr;
	MapLoader*		m_globalMaps = nullptr;
	EffectLoader*		m_globalEffects = nullptr;
	FirePositionLoader*  m_globalFirePos = nullptr;

	bool			m_bFogEnable = false;
	GameObject*		m_player = nullptr;
	GameObject**	m_Enemys = nullptr;
	void*					m_projecttile = nullptr;
	void*					m_hitPaticle = nullptr;

	UINT					m_nEnemy;
	int						m_remainEnemy;
public:
	static GlobalVal* getInstance();

	virtual ~GlobalVal() {
		instanceFlag = false;
		delete m_globalModels;
		delete m_globalMaps;
		delete m_globalEffects;
		delete g_instance;
		delete m_globalFirePos;

		m_globalModels = nullptr;
		m_globalMaps = nullptr;
		m_globalEffects = nullptr;
		m_globalFirePos = nullptr;
		m_player = nullptr;
		g_instance = nullptr;
	}

	void LoadModels(ModelLoader* models);
	void LoadMaps(MapLoader* maps);
	void LoadEffects(EffectLoader* effects);
	void LoadFirePos(FirePositionLoader* firepos);

	void setPlayer(GameObject* object);
	void setEnemy(GameObject** objects);
	void setPorjectile(void* pContext);
	void setHitPaticle(void* pContext);
	void setFogEnable(bool bEnabled);

	ModelLoader* getModelLoader() { return m_globalModels; }
	MapLoader* getMapLoader() { return m_globalMaps; }
	EffectLoader* getEffectLoader() { return m_globalEffects; }
	FirePositionLoader* getFirePos() { return m_globalFirePos; }

	GameObject* getPlayer();
	GameObject** getEnemy();
	void* getProjectile() { return m_projecttile; }
	void* getHitPaticle() { return m_hitPaticle; }
	bool getFogEnable() { return m_bFogEnable; }
	UINT* getNumEnemy() { return &m_nEnemy; }
	int* getRemainEnemy() { return &m_remainEnemy; }
};