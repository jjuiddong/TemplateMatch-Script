
#include "stdafx.h"
#include "match_result.h"

using namespace cv;
using namespace cvproc;
using namespace cvproc::imagematch;


cMatchResult::cMatchResult()
	: m_sharedData(NULL)
{
	Clear();
}

cMatchResult::~cMatchResult()
{
}


void cMatchResult::Init(cMatchScript *p, const cv::Mat &input, const string &inputName,
	sParseTree *labelTree, const bool isRegisterInput, const bool isBlockMode)
{
	Clear();

	m_script = p;
	m_isRun = true;
	m_isTerminate = false;
	m_input = input.clone();
	m_srcImage = input.clone();
	m_inputName = inputName;
	m_registerInput = isRegisterInput;
	m_removeInput = isRegisterInput;
	m_blockMode = isBlockMode;
	m_nodeLabel = labelTree;
	m_beginTime = timeGetTime();

	if (!labelTree)
		m_resultStr = "not found label";

	if (isRegisterInput || inputName.empty())
	{
		m_sharedData->SetInputImage2(input, m_inputName);
	}
	else
	{
		m_inputName = inputName;
		m_sharedData->SetInputImage(input, inputName);
	}

	m_input = m_sharedData->LoadImage(m_inputName).clone();
}


void cMatchResult::Clear()
{
	m_traverseType = 0;
	m_script = NULL;
	m_input = Mat();
 	m_srcImage = Mat();
 	m_inputName.clear();
	m_registerInput = true;
	m_blockMode = false;
	m_removeInput = false;
	m_nodeLabel = NULL;
	m_resultStr.clear();
	m_isRun = false;
	m_isTerminate = false;
	m_isEnd = false;
	m_result = 0;
	m_matchCount = 0;
	m_beginTime = 0;

	ZeroMemory(m_data, sizeof(m_data));
}


void cMatchResult::TerminateMatch()
{
	m_isTerminate = true;
}


void cMatchResult::MatchComplete()
{
	m_isRun = false;
	m_isEnd = false;
	m_isTerminate = false;
}
