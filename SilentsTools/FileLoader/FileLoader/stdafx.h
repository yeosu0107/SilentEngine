// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 또는 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"
#include "assimp\cimport.h"
#include "assimp\postprocess.h"
#include "assimp\scene.h"
#include <stdio.h>
#include <tchar.h>

const struct aiScene* scene = nullptr;

// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.

extern void LoadAsset(const char* path);

extern void showMesh(const struct aiScene* sc, const struct aiNode* nd);

extern void LoadBorn(const struct aiScene* sc, const struct aiNode* nd);