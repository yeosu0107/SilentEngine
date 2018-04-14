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

	GameObject*		m_player;
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

	GameObject* getPlayer();
	ModelLoader* getModelLoader() { return m_globalModels; }
	MapLoader* getMapLoader() { return m_globalMaps; }
	EffectLoader* getEffectLoader() { return m_globalEffects; }
};