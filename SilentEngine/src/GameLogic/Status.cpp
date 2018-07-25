#include "stdafx.h"
#include "Status.h"
#include "..\Model\StringTokenizer.h"

StatusLoader::StatusLoader(string fileName)
{
	ifstream in(fileName);

	string tmpName;
	string delim = ",";
	StringTokenizer st = StringTokenizer("");

	while (getline(in, tmpName)) {
		st = StringTokenizer(tmpName, delim);
	}
	in.close();
}

StatusLoader::~StatusLoader()
{
	m_status.clear();
}
