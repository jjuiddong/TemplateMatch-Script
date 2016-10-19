//
// 2016-07-13, jjuiddong
// �̹��� ��Ī�� �����ϴ� �̱��� Ŭ����
// ������� �����ϸ�, MatchScript Parse Ʈ���� ��ȸ�ϸ�,
// �̹��� ��Ī�� �õ��Ѵ�.
//
// 2016-10-14
//		- ������ ���� ����Ÿ �и�
//
// 
#pragma once


namespace cvproc {
	namespace imagematch {

		class cMatchResult;

		class cMatchProcessor : public common::cSingleton<cMatchProcessor>
		{
		public:
			cMatchProcessor();
			virtual ~cMatchProcessor();

			bool Match(INOUT cMatchResult &matchResult );

			struct sExecuteTreeArg 
			{
				bool isUsed;
				bool isEnd;
				sParseTree *parent; // ��Ī�� �θ� ���
				sParseTree *node; // ��Ī�� ���, ���ø� �̹��� ��θ� �����ϰ� �ִ�.
				cv::Mat *out; // ��� ������ �����ؼ� ����
				int result;
				cMatchResult *matchResult;
			};
			int executeTreeEx(INOUT sExecuteTreeArg &arg);
			int executeOcr(INOUT sExecuteTreeArg &arg);
			void FinishMatchThread();


		public:
			cMatchResult *m_lastMatchResult;
			bool m_isGray = true;
 			bool m_isLog; // default = true, log message print to log.txt
			bool m_isLog2; // default = true, ocr log

			bool m_isMatchThreadLoop;
			bool m_isThreadTerminate; // false �� ����, �����尡 �����Ѵ�. �����带 �߰��� ���ߴ� �뵵�� ��.
			HANDLE m_threadHandle;
		};

} }
