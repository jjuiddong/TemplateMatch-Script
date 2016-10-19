
#include "stdafx.h"
#include "match_manager.h"
#include "match_processor.h"

using namespace cvproc;
using namespace cvproc::imagematch;
using namespace cv;

cMatchManager::cMatchManager() 
	: m_delayCapture(100)
{
}

cMatchManager::~cMatchManager()
{
}


bool cMatchManager::Init(const string &matchScriptFilename, const string &executeLabel)
{
	dbg::RemoveLog();
	dbg::RemoveErrLog();

	m_executeLabel = executeLabel;
	return m_matchScript.Read(matchScriptFilename);
}


string cMatchManager::CaptureAndDetect()
{
	cScreenCapture capture;
	capture.m_prtScrSleepTime = m_delayCapture;
	Mat img = capture.ScreenCapture(true);
	
	cMatchResult *result = m_sharedData.AllocMatchResult();
	if (!result)
		return "";

	result->Init(&m_matchScript, img, "@input", 
		(sParseTree*)m_matchScript.FindTreeLabel(m_executeLabel), true, true);
	cMatchProcessor::Get()->Match(*result);
	const string str = result->m_resultStr;
	m_sharedData.FreeMatchResult(result);
	return str;
}


void cMatchManager::CaptureDelay(const int delayMilliSeconds)
{
	m_delayCapture = delayMilliSeconds;
}


void cMatchManager::Match(INOUT cMatchResult &mresult)
{
	cMatchProcessor::Get()->Match(mresult);
}
