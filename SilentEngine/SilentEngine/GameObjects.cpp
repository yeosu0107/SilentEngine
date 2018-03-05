#include "stdafx.h"
#include "GameObjects.h"

GameObjects::GameObjects(int nMeshes = 1)
{
	m_pRenderItem = make_unique<RenderObject>();
	m_pRenderItem->m_ppMeshes = vector<MeshGeometry*>(nMeshes);

}

GameObjects::~GameObjects()
{
}

void GameObjects::SetMesh(int nIndex, MeshGeometry * pMesh)
{
	m_pRenderItem->m_ppMeshes[nIndex] = pMesh;
}
