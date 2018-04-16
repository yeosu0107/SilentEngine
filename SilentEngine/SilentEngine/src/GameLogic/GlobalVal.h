#pragma once

//ΩÃ±€≈Ê ≈¨∑°Ω∫
class GlobalVal {
private:
	GlobalVal() {}

	static bool			instanceFlag;
	static GlobalVal* g_instance;
private:
	ModelLoader*		m_globalModels = nullptr;
	MapLoader*		m_globalMaps = nullptr;
	EffectLoader*		m_globalEffects = nullptr;

	GameObject*		m_player = nullptr;
	GameObject**	m_Enemys = nullptr;
	void*					m_projecttile = nullptr;

	UINT					m_nEnemy;
public:
	static GlobalVal* getInstance();

	virtual ~GlobalVal() {
		instanceFlag = false;
		delete m_globalModels;
		delete m_globalMaps;
		delete m_globalEffects;
		delete g_instance;

		m_globalModels = nullptr;
		m_globalMaps = nullptr;
		m_globalEffects = nullptr;
		m_player = nullptr;
		g_instance = nullptr;
	}

	void LoadModels(ModelLoader* models);
	void LoadMaps(MapLoader* maps);
	void LoadEffects(EffectLoader* effects);
	void setPlayer(GameObject* object);
	void setEnemy(GameObject** objects);
	void setPorjectile(void* pContext);

	ModelLoader* getModelLoader() { return m_globalModels; }
	MapLoader* getMapLoader() { return m_globalMaps; }
	EffectLoader* getEffectLoader() { return m_globalEffects; }

	GameObject* getPlayer();
	GameObject** getEnemy();
	void* getProjectile() { return m_projecttile; }

	UINT* getNumEnemy() { return &m_nEnemy; }
};