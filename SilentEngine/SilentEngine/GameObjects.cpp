#include "stdafx.h"
#include "GameObjects.h"

GameObjects::GameObjects(int nMeshes = 1)
{
	m_pRenderItem = make_unique<RenderItem>();
	m_pRenderItem->m_ppMeshes = vector<MeshGeometry*>(nMeshes);

}

GameObjects::~GameObjects()
{
}
