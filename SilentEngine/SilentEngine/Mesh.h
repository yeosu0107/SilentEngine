#pragma once

#include <cstdint>
#include <DirectXMath.h>
#include <vector>
#include "D3DUtil.h"

using namespace DirectX;
using namespace std;

class Mesh
{
public:
	using uint16 = uint16_t;
	using uint32 = uint32_t;

	struct Vertex
	{
		Vertex() {}
		Vertex(
			const XMFLOAT3& p,
			const XMFLOAT3& n,
			const XMFLOAT3& t,
			const XMFLOAT2& uv) :
			xmf3Position(p),
			xmf3Normal(n),
			xmf3TangentU(t),
			xmf2TexCoord(uv) {};

		Vertex(float px, float py, float pz,
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v) :
			xmf3Position(px, py, pz),
			xmf3Normal(nx, ny, nz),
			xmf3TangentU(tx, ty, tz),
			xmf2TexCoord(u, v) {};
		
		XMFLOAT3 xmf3Position;
		XMFLOAT3 xmf3Normal;
		XMFLOAT3 xmf3TangentU;
		XMFLOAT2 xmf2TexCoord;
	};

	struct MeshData
	{
		vector<Vertex> m_pVertices;		// 정점 버퍼
		vector<uint32> m_pIndices32;	// 인덱스 버퍼

		vector<uint16>& GetIndices16()	// 정점 인덱스 메모리를 축소 시켜서 획득
		{
			if (m_pIndices16.empty())
			{
				m_pIndices16.resize(m_pIndices32.size());
				for (size_t i = 0; i < m_pIndices32.size(); ++i)
					m_pIndices16[i] = static_cast<uint16>(m_pIndices32[i]);
			}
			return m_pIndices16;
		}

	private:
		vector<uint16> m_pIndices16;
	};

	MeshData CreateBox(float fwidth, float fheight, float fdepth, uint32 numSubdivisions);

private:
	void Subdivide(MeshData& meshData);
	Vertex MidPoint(const Vertex& v0, const Vertex& v1);
};

