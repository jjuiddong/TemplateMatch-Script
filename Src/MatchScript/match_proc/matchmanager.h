//
// 2016-06-17, jjuiddong
// �̹��� ��Ī�� �������ִ� ���α׷�
// ScreenCapture -> Image Match
// 
#pragma once


namespace cvproc { namespace imagematch {

	
	class cMatchManager : public common::cSingleton<cMatchManager>
	{
	public:
		cMatchManager();
		virtual ~cMatchManager();

		bool Init(const string &filename, const string &executeLabel);
		string CaptureAndDetect();
		void CaptureDelay(const int delayMilliSeconds);


	public:
		cMatchScript2 m_matchScript;
		string m_executeLabel;
		int m_delayCapture; // defualt = 100
	};

}}