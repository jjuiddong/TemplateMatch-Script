//
// 2016-07-13, jjuiddong
// 이미지 매칭을 전담하는 싱글톤 클래스
// 쓰레드로 동작하며, MatchScript Parse 트리를 순회하며,
// 이미지 매칭을 시도한다.
//
// 2016-10-14
//		- 쓰레드 공유 데이타 분리
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
				sParseTree *parent; // 매칭할 부모 노드
				sParseTree *node; // 매칭할 노드, 템플릿 이미지 경로를 저장하고 있다.
				cv::Mat *out; // 결과 정보를 저장해서 리턴
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
			bool m_isThreadTerminate; // false 일 때만, 스레드가 동작한다. 스레드를 중간에 멈추는 용도로 씀.
			HANDLE m_threadHandle;
		};

} }
