//#include "stdafx.h"
//#include "Stage.h"
//
//#pragma comment(lib, "assimp.lib")
//
//using namespace STAGE;
//
//int main()
//{
//	MapGenerator tmp;
//	tmp.SetMap(15,7);
//	tmp.printMap();
//    return 0;
//}

#include "stdafx.h"
#include "Stage.h"
#include "InputModel.h"

#include <time.h>

#pragma comment(lib, "SilentsTools.lib")



using namespace STAGE;

int main()
{
	clock_t begin, end;
	begin = clock();
	InputModel model;
	for(int i=0; i<1; ++i)
		model.LoadAsset("fbxsample/idle.fbx");
	end = clock();

	cout << "수행시간 : " << (end - begin) / CLOCKS_PER_SEC << endl;

	//LoadAsset("idle.fbx");
	/*MapGenerator* map;
	for (int i = 0; i < 5; ++i) {
		map = new MapGenerator(i);
		map->SetMap(7, 5);
		map->printMap();
		printf("\n");
		delete map;
	}*/

	return 0;
}
