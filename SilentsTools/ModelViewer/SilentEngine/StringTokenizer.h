//////////////////////////////////////////////
// StringTokenizer v0.1						//
// copyright : arer							//
// homepage : http://arer.tistroy.com		//
// 본 코드의 저작권은 tistory의 arer에게 있고 //
// 무단 수정 및 배포를 금지합니다.			//
// 사용시 출처를 밝혀주시기 바랍니다.			//
//////////////////////////////////////////////

#include <string>
#include <queue>
#include <cctype>
using namespace std;

class StringTokenizer {
public:
	StringTokenizer(string str);
	StringTokenizer(string str,string delims);
	StringTokenizer(string str,string delims,bool returnDelims);

	int countTokens();
	string nextToken();
	//string nextToken(string delims);
	bool hasMoreTokens();

private:
	string mStr;
	string mDelims;
	bool mReturnDelims;
	queue<string> tokens;

	void tokenizer();
	bool isDelims(char c);
};