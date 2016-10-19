
#include "stdafx.h"
#include "match_processor.h"


using namespace cv;
using namespace cvproc;
using namespace cvproc::imagematch;

cMatchProcessor::cMatchProcessor()
	: m_isMatchThreadLoop(false)
	, m_threadHandle(NULL)
	, m_isThreadTerminate(false)
	, m_isLog(true)
	, m_isLog2(true)
	, m_lastMatchResult(NULL)
{
}

cMatchProcessor::~cMatchProcessor()
{
}


bool cMatchProcessor::Match(INOUT cMatchResult &matchResult )
{
	m_lastMatchResult = &matchResult;
	if (!matchResult.m_nodeLabel)
		return false;

	matchResult.m_matchCount = 0;
	m_isMatchThreadLoop = true;
	m_isThreadTerminate = false;

	matchResult.m_isEnd = false;
	matchResult.m_result = 0;

	m_threadHandle = (HANDLE)_beginthreadex(NULL, 0, MatchThreadMain, &matchResult, 0, NULL);

	if (matchResult.m_blockMode)
	{
		// 쓰레드를 종료 확인하고, 리턴
		while (!matchResult.m_isEnd)
			Sleep(1);
		//WaitForSingleObject(m_threadHandle, INFINITE);
		m_threadHandle = NULL;
		m_isMatchThreadLoop = false;
	}

	return true;
}


// return value 0 : fail
//					    1 : success
int cMatchProcessor::executeTreeEx(INOUT sExecuteTreeArg &arg)
{
	cMatchScript &script = *arg.matchResult->m_script;
	cSharedData *sharedData = arg.matchResult->m_sharedData;
	const cv::Mat &input = arg.matchResult->m_input;
	const string &inputName = arg.matchResult->m_inputName;
	sParseTree *parent = arg.parent;
	sParseTree *node = arg.node;
	cv::Mat *out = arg.out;

	if (!node)
		return 0;
	if (!node->child)
		return 1; // terminal node, success finish
	if ('@' == node->attrs["id"][0]) // link node, excute child node
		return 1;

	int matchType = 0; // template match
	if (((script.m_matchType == 1) && (node->attrs["type"].empty())) 
		|| (node->attrs["type"] == "featurematch"))
		matchType = 1; // feature match
	else if (node->attrs["type"] == "ocrmatch")
		matchType = 2; // ocr match

	if (node->attrs["type"] == "ocrmatch")
	{
		return executeOcr(arg);
	}

	const Mat &matObj = sharedData->LoadImage(node->attrs["id"]);
	if (matObj.empty())
		return 0;

	const cv::Size csize(input.cols - matObj.cols + 1, input.rows - matObj.rows + 1);
	if ((csize.height < 0) || csize.width < 0)
		return 0;

	// roi x,y,w,h
	cv::Rect roi(0, 0, input.cols, input.rows);
	if (!node->IsEmptyRoi())
		sscanf(node->attrs["roi"].c_str(), "%d,%d,%d,%d", &roi.x, &roi.y, &roi.width, &roi.height);

	if ((atoi(node->attrs["relation"].c_str())>0) && parent)
	{
		roi.x += parent->matchLoc.x;
		roi.y += parent->matchLoc.y;
	}

	// roi limit check
	roi.x = max(0, roi.x);
	roi.y = max(0, roi.y);
	if (input.cols < roi.x + roi.width)
		roi.width = input.cols - roi.x;
	if (input.rows < roi.y + roi.height)
		roi.height = input.rows - roi.y;

	if ((roi.width < 0) || (roi.height < 0))
	{
		// too small source image
		return 0;
	}

	double min=0, max=0;
	Point left_top;
	const float mthreshold = (float)atof(node->attrs["threshold"].c_str());
	const float maxThreshold = (mthreshold <= 0) ? 0.9f : mthreshold;
	bool isDetect = false;

	if (m_isLog)
		dbg::Log("executeTree %s, matchType=%d \n", node->attrs["id"], matchType);

	const Mat *src = &input;

	// channel match
	if (!node->IsEmptyBgr())
	{
		cv::Scalar scalar;
		sscanf(node->attrs["scalar"].c_str(), "%lf,%lf,%lf", &scalar[0], &scalar[1], &scalar[2]);
		const float scale = (float)atof(node->attrs["scale"].c_str());
		src = &sharedData->LoadScalarImage(inputName, scalar, scale); // BGR
	}

	// hsv match
	if (!node->IsEmptyCvt())
	{
		src = &sharedData->LoadCvtImageAcc(inputName, node);
	}

	if (!src->data)
		return 0; // fail

	arg.matchResult->m_matchCount++;
	node->processCnt++; // count node match

	if (matchType == 0) // --> templatematch
	{
		if (((roi.width >= matObj.cols) && (roi.height >= matObj.rows))
			&& (src->channels() == matObj.channels()))
		{
			Mat matResult(csize, IPL_DEPTH_32F);
			cv::matchTemplate((*src)(roi), matObj, matResult, CV_TM_CCOEFF_NORMED);
			cv::minMaxLoc(matResult, &min, &max, NULL, &left_top);
		}

		isDetect = max > maxThreshold;
		node->max = max;

		// 매칭 결과 저장
		const Point lt = left_top + Point(roi.x, roi.y); // relative location
		arg.matchResult->m_data[node->id].max = max;
		arg.matchResult->m_data[node->id].matchRect = Rect(lt.x, lt.y, matObj.cols, matObj.rows);

		// 매칭된 장면을 복사해 리턴한다.
		if (out)
		{
			*out = (*src)(Rect(left_top.x, left_top.y, left_top.x + matObj.cols, left_top.y + matObj.rows));
		}

		if (m_isLog)
			dbg::Log("%s --- templatematch max=%f, IsDetection = %d \n", node->attrs["id"], max, isDetect);
	}
	else if (matchType == 1) // feature match
	{
		vector<KeyPoint> *objectKeyPoints, *sceneKeyPoints;
		Mat *objectDescriotor, *sceneDescriptor;
		sharedData->LoadDescriptor(node->attrs["id"], &objectKeyPoints, &objectDescriotor);
		sharedData->LoadDescriptor(inputName, &sceneKeyPoints, &sceneDescriptor);

		if (!objectKeyPoints || !sceneKeyPoints || !sceneKeyPoints || !sceneDescriptor)
			return 0; // 이미지 로딩 실패

		if (!sceneKeyPoints->empty() && sceneDescriptor->data)
		{
			cFeatureMatch match(m_isGray, script.m_isDebug);
			match.m_isLog = m_isLog;
			//isDetect = match.Match((*src)(roi), *sceneKeyPoints, *sceneDescriptor, matObj, 
			//	*objectKeyPoints, *objectDescriotor, node->str);

			// TODO: feature match roi
			isDetect = match.Match(*src, *sceneKeyPoints, *sceneDescriptor, matObj,
				*objectKeyPoints, *objectDescriotor, node->attrs["id"]);
			max = isDetect ? 1 : 0;

			// 매칭 결과 저장
			arg.matchResult->m_data[node->id].max = max;
			if (match.m_rect.m_contours.size() >= 4)
			{
				arg.matchResult->m_data[node->id].matchRect2[0] = match.m_rect.m_contours[0];
				arg.matchResult->m_data[node->id].matchRect2[1] = match.m_rect.m_contours[1];
				arg.matchResult->m_data[node->id].matchRect2[2] = match.m_rect.m_contours[2];
				arg.matchResult->m_data[node->id].matchRect2[3] = match.m_rect.m_contours[3];
			}
		}
	}

	// 성공이든, 실패든, 매칭된 위치는 저장한다.
	node->matchLoc = left_top + Point(roi.x, roi.y);

	if (isDetect)
	{
		return 1;
	}

	return 0;
}


int cMatchProcessor::executeOcr(INOUT sExecuteTreeArg &arg)
{
	cSharedData *sharedData = arg.matchResult->m_sharedData;
	const cv::Mat &input = arg.matchResult->m_input;
	const string &inputName = arg.matchResult->m_inputName;
	sParseTree *parent = arg.parent;
	sParseTree *node = arg.node;

	if (node->IsEmptyHsv(0))
		return 0;

	// roi x,y,w,h
	cv::Rect roi(0, 0, input.cols, input.rows);
	if (!node->IsEmptyRoi())
		sscanf(node->attrs["roi"].c_str(), "%d,%d,%d,%d", &roi.x, &roi.y, &roi.width, &roi.height);

	if ((atoi(node->attrs["relation"].c_str())>0) && parent)
	{
		roi.x += parent->matchLoc.x;
		roi.y += parent->matchLoc.y;
	}

	// roi limit check
	roi.x = max(0, roi.x);
	roi.y = max(0, roi.y);
	if (input.cols < roi.x + roi.width)
		roi.width = input.cols - roi.x;
	if (input.rows < roi.y + roi.height)
		roi.height = input.rows - roi.y;

	if ((roi.width < 0) || (roi.height < 0))
	{
		// too small source image
		return 0;
	}

	//-------------------------------------------------------------------------------------------------------------------------------------------
	// Convert HSV
	Mat &colorSrc = sharedData->LoadImage(string("color") + inputName);
	if (!colorSrc.data)
		return 0;

	Mat dst;
	if ((roi.width != colorSrc.cols) || (roi.height != colorSrc.rows))
		dst = colorSrc(roi).clone();
	else
		dst = colorSrc.clone();

	Scalar lower, upper;
	sscanf(node->attrs["hsv0"].c_str(), "%lf,%lf,%lf,%lf,%lf,%lf", &lower[0], &lower[1], &lower[2], &upper[0], &upper[1], &upper[2]);

	cvtColor(dst, dst, CV_BGR2HSV);
	inRange(dst, lower, upper, dst);
	//-------------------------------------------------------------------------------------------------------------------------------------------


	cDeSkew deSkew;
	const int t0_1 = timeGetTime();
	deSkew.DeSkew(dst, 0.005f, 0, true);
	const int t0_2 = timeGetTime();

	float maxFitness = -FLT_MAX;
	tess::cTessWrapper *tess = sharedData->GetTesseract();
	const int t1 = timeGetTime();
	const string srcStr = tess->Recognize(deSkew.m_tessImg);
	const int t2 = timeGetTime();
	const string result = tess->Dictionary(node->attrs["id"], srcStr, maxFitness);
	const int t3 = timeGetTime();

	dbg::Log2("log2.txt", "%s\n", trim(string(srcStr)).c_str());

	if (m_isLog2)
	{
		dbg::Log("tesseract recognition deskew = %d, recog time = %d, dictionary = %d, dict = %s, result = %s, maxFitness = %3.3f  \n", 
			t0_2 - t0_1, t2 - t1, t3 - t2, node->attrs["id"].c_str(), result.c_str(), maxFitness);
		dbg::Log("    -src = %s \n", srcStr.c_str());
	}

	if (t2 - t1 > 200)
	{// 인식하는데 오래걸리는 것은 파일로 남겨서, 테스트 해본다.
	 // 중복되지 않는 파일명으로 생성한다.
		static int imgCnt = 1;
		string fileName;
		do
		{
			std::stringstream ss;
			ss << "dbg/ocr_capture" << imgCnt++ << "_" << (t2-t1) << ".jpg";
			fileName = ss.str();
		} while (PathFileExistsA(fileName.c_str()));
		imwrite(fileName.c_str(), deSkew.m_tessImg);
	}

	bool isDetect = true;

	// 스트링 테이블이 있다면, 스트링 테이블에 포함된 문자일 때만,
	// 매칭에 성공한 것을 간주한다.
	if (!node->attrs["table"].empty())
	{
		vector<string> out;
		tokenizer2(node->attrs["table"], ",", out);
		isDetect = false;
		for each (auto &str in out)
		{
			if (str == result)
			{
				isDetect = true;
				break;
			}
		}
	}

	arg.matchResult->m_matchCount++;
	node->processCnt++; // count node match
	arg.matchResult->m_data[node->id].max = 0;

	if (isDetect)
	{
	 	arg.matchResult->m_data[node->id].max = maxFitness;

		// 인식한 문장 저장
		const int MAX_STR_LEN = sizeof(arg.matchResult->m_data[node->id].str);
		ZeroMemory(arg.matchResult->m_data[node->id].str, MAX_STR_LEN);
		memcpy(arg.matchResult->m_data[node->id].str, result.c_str(), MIN(result.length(), MAX_STR_LEN - 1));
	}

	// 성공이든, 실패든, 매칭된 위치는 저장한다.
	node->matchLoc = deSkew.m_deSkewPoint1;
	arg.matchResult->m_data[node->id].matchRect2[0] = deSkew.m_pts[0] + Point(roi.x, roi.y);
	arg.matchResult->m_data[node->id].matchRect2[1] = deSkew.m_pts[1] + Point(roi.x, roi.y);
	arg.matchResult->m_data[node->id].matchRect2[2] = deSkew.m_pts[2] + Point(roi.x, roi.y);
	arg.matchResult->m_data[node->id].matchRect2[3] = deSkew.m_pts[3] + Point(roi.x, roi.y);

	return isDetect;
}


// 스레드가 실행 중이라면, 강제 종료시킨다.
void cMatchProcessor::FinishMatchThread()
{
	if (m_threadHandle)
	{
		m_isThreadTerminate = true;
		WaitForSingleObject(m_threadHandle, INFINITE); // 5 seconds wait
		m_threadHandle = NULL;
		m_isMatchThreadLoop = false;
	}
}
