//////////////////////////////////////////////
// StringTokenizer v0.1						//
// copyright : arer							//
// homepage : http://arer.tistroy.com		//
// �� �ڵ��� ���۱��� tistory�� arer���� �ְ� //
// ���� ���� �� ������ �����մϴ�.			//
// ���� ��ó�� �����ֽñ� �ٶ��ϴ�.			//
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