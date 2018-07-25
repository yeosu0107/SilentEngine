#include "stdafx.h"
#include "D3DMath.h"

const float D3DMath::Infinity = FLT_MAX;
const float D3DMath::Pi = 3.1415926535f;

float D3DMath::AngleFromXY(float x, float y)
{
	float ftheta = 0.0f;

	if (x >= 0.0f)
	{
		ftheta = atanf(y / x);

		if (ftheta < 0.0f)
			ftheta += 2.0f * Pi;
	}

	else
		ftheta = atanf(y / x) + Pi;

	return ftheta;
}

XMVECTOR D3DMath::RandUnitVec3()
{
	XMVECTOR vOneVector = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR vZoroVector = XMVectorZero();
	
	while (true)
	{
		XMVECTOR vRandVector = XMVectorSet(RandF(-1.0f, 1.0f), RandF(-1.0f, 1.0f), RandF(-1.0f, 1.0f), 0.0f);

		if (XMVector3Greater(XMVector3LengthSq(vRandVector), vOneVector))
			continue;

		return XMVector3Normalize(vRandVector);
	}
}

XMVECTOR D3DMath::RandHemisphereUnitVec3(XMVECTOR v)
{
	XMVECTOR vOneVector = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR vZoroVector = XMVectorZero();

	while (true)
	{
		XMVECTOR vRandVector = XMVectorSet(RandF(-1.0f, 1.0f),RandF(-1.0f, 1.0f), RandF(-1.0f, 1.0f), 0.0f);

		if (XMVector3Greater(XMVector3LengthSq(vRandVector), vOneVector))
			continue;

		if (XMVector3Less(XMVector3Dot(v, vRandVector), vZoroVector))
			continue;

		return XMVector3Normalize(vRandVector);
	}
}
