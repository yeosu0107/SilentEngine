#include "stdafx.h"
#include "StringTokenizer.h"

StringTokenizer::StringTokenizer(string str) {
	mStr = str;
	mDelims = "";
	mReturnDelims = false;
	tokenizer();
}
StringTokenizer::StringTokenizer(string str, string delims) {
	mStr = str;
	mDelims = delims;
	mReturnDelims = false;
	tokenizer();
}
StringTokenizer::StringTokenizer(string str, string delims,bool returnDelims) {
	mStr = str;
	mDelims = delims;
	mReturnDelims = returnDelims;
	tokenizer();
}

void StringTokenizer::tokenizer() {
	string token = "";

	for(unsigned int i=0;i<mStr.size();i++) {
		if(isDelims(mStr[i])) {
			if(token != "")
				tokens.push(token);
			if(mReturnDelims) {
				string str;
				str = mStr[i];
				tokens.push(str);
			}
			token = "";
		} else {
			token += mStr[i];
		}
	}

	if(token != "")
		tokens.push(token);
}

bool StringTokenizer::isDelims(char c) {
	if(mDelims == "") {
		return isspace(c);
	} else {
		for(unsigned int i=0;i<mDelims.size();i++)
			if(mDelims[i] == c) return true;
	}
	return false;
}

int StringTokenizer::countTokens() {
	return tokens.size();
}

bool StringTokenizer::hasMoreTokens() {
	return !tokens.empty();
}

string StringTokenizer::nextToken() {
	string ret;
	ret = tokens.front();
	tokens.pop();

	return ret;
}