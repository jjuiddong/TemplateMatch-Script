
#include "stdafx.h"

using namespace std;
using namespace cv;
using namespace cvproc;
using namespace cvproc::imagematch;

void main()
{
	cMatchManager mng;
	mng.Init("match-script.txt", "@test");

	cMatchResult *result = mng.m_sharedData.AllocMatchResult();

	list<string> exts;
	exts.push_back("jpg");
	exts.push_back("png");
	list <string> out;
	CollectFiles(exts, "./image/", out);
	out.sort();
	for each (auto &str in out)
	{
		Mat img = imread(str);
		result->Init(&mng.m_matchScript, img, "", 
			(sParseTree*)mng.m_matchScript.FindTreeLabel("@test"), true, true);
		mng.Match(*result);

		cout << "image=" << str << ", result = " << result->m_resultStr << endl;
	}

	mng.m_sharedData.FreeMatchResult(result);

	cout << endl << endl << "press key down to exit program" << endl;

	getchar();
}

