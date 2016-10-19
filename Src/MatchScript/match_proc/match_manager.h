//
// 2016-06-17, jjuiddong
// 이미지 매칭을 쉽게해주는 프로그램
// ScreenCapture -> Image Match
// 
#pragma once


namespace cvproc { namespace imagematch {


	class cMatchManager : public common::cSingleton<cMatchManager>
	{
	public:
		cMatchManager();
		virtual ~cMatchManager();

		bool Init(const string &matchScriptFilename, const string &executeLabel);
		string CaptureAndDetect();
		void CaptureDelay(const int delayMilliSeconds);
		void Match(INOUT cMatchResult &mresult);


	public:
		cMatchScript m_matchScript;
		cSharedData m_sharedData;
		string m_executeLabel;
		int m_delayCapture; // defualt = 100
	};

}}
