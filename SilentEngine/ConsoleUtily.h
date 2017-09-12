#pragma once
/*
	�ֱ� ���� : ��ǿ�
	���� �ð� : 2017.09.13 02:27
	���� ���� : XMFLOAT3, XMFLOAT4, XMMATIRX, XMFLOAT4X4 ���� ����ϴ� �Ϻ� Ŭ���� �ܼ� ��ƿ��Ƽ �Լ� �߰�
*/
#include "stdafx.h"

using std::ostream;

// XMFLOAT3�� Cout���� �ٷ� ����� �� �ְ� ���ش�.
ostream& XM_CALLCONV operator<<(ostream& os, const XMFLOAT3& v)
{
	os << "(" << v.x << ", " << v.y << ", " << v.z << ") ";
	return os;
}

// XMFLOAT4�� Cout���� �ٷ� ����� �� �ְ� ���ش�.
ostream& XM_CALLCONV operator<<(ostream& os, const XMFLOAT4& v)
{
	os << "(" << v.x << ", " << v.y << ", " << v.z << "," << v.w << ") ";
	return os;
}


// XMMATRIX�� Cout���� �ٷ� ����� �� �ְ� ���ش�.
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

// XMFLOAT4X4�� Cout���� �ٷ� ����� �� �ְ� ���ش�.
ostream& XM_CALLCONV operator<<(ostream& os, const XMFLOAT4X4& m)
{
	XMFLOAT3 pfloat3[4];
	XMMATRIX mmatrix = XMLoadFloat4x4(&m);

	os << mmatrix;

	return os;
}