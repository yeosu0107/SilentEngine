#pragma once
/*
	최근 수정 : 김건우
	수정 시간 : 2017.09.13 02:27
	수정 내용 : XMFLOAT3, XMFLOAT4, XMMATIRX, XMFLOAT4X4 같은 사용하는 일부 클래스 콘솔 유틸리티 함수 추가
*/
#include "stdafx.h"

using std::ostream;

// XMFLOAT3를 Cout으로 바로 출력할 수 있게 해준다.
ostream& XM_CALLCONV operator<<(ostream& os, const XMFLOAT3& v)
{
	os << "(" << v.x << ", " << v.y << ", " << v.z << ") ";
	return os;
}

// XMFLOAT4를 Cout으로 바로 출력할 수 있게 해준다.
ostream& XM_CALLCONV operator<<(ostream& os, const XMFLOAT4& v)
{
	os << "(" << v.x << ", " << v.y << ", " << v.z << "," << v.w << ") ";
	return os;
}


// XMMATRIX를 Cout으로 바로 출력할 수 있게 해준다.
ostream& XM_CALLCONV operator<<(ostream& os, const XMMATRIX& m)
{
	XMFLOAT4 pfloat4;

	for (int i = 0; i < 4; i++)
	{
		XMStoreFloat4(&pfloat4, m.r[i]);
		os << pfloat4 << "\n";
	}

	return os;
}

// XMFLOAT4X4를 Cout으로 바로 출력할 수 있게 해준다.
ostream& XM_CALLCONV operator<<(ostream& os, const XMFLOAT4X4& m)
{
	XMFLOAT3 pfloat3[4];
	XMMATRIX mmatrix = XMLoadFloat4x4(&m);

	os << mmatrix;

	return os;
}