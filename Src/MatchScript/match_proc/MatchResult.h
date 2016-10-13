//
// 2016-07-15, jjuiddong
// ImageMatch ����� �����Ѵ�.
//
#pragma once


namespace cvproc {
	namespace imagematch {
		
		class cMatchScript2;

		class cMatchResult
		{
		public:
			cMatchResult();
			virtual ~cMatchResult();

			void Init(cMatchScript2 *p, const cv::Mat &input, const string &inputName, 
				const int inputImageId, sParseTree *labelTree, const bool isRegisterInput, 
				const bool isBlockMode);
			
			void MatchComplete();

			void TerminateMatch();

			void Clear();


		public:
			// �Է�
			int m_traverseType;	 // ��Ī ��ȸ�ϴ� ��� ����, {0 = ��Ī�Ǹ� ����, 1=��� ��Ī����, ���� ���շ��� ���� ��� ����}
			cMatchScript2 *m_script;
			cv::Mat m_input;
			cv::Mat m_srcImage; // ���� �Է� �̹���, m_input �� ��Ī�� ��, ��鿵������ �ٲ��.
			string m_inputName;
			int m_inputImageId;
			bool m_registerInput;
			bool m_blockMode;
			bool m_removeInput;
			sParseTree *m_nodeLabel;
			bool m_isTerminate; // ��Ī�� ������ �� true �� �Ǿ�� �Ѵ�.

			// ���
			string m_resultStr;
			bool m_isRun;	// ��Ī�� �õ��ߴٸ� true ���ȴ�.
			bool m_isEnd;
			int m_result;
			int m_matchCount; // debug
			int m_beginTime;

			struct sNodeResult
			{
				bool nodesRun; // ������� �����ϱ� ������, ����ȭ�� ����, �ӽ� ���۷� ������ ���¸� �Ǵ��Ѵ�. key = node->id
				cv::Rect matchRect; // ��Ī�� ��ġ�� �����Ѵ�. TemplateMatch
				cv::Point matchRect2[4]; // ��Ī�� ��ġ�� �����Ѵ�. FeatureMatch
				double max; // ���ø� ��ġ �ִ� ���� �����Ѵ�.
				char str[128]; // OCR �ν� ���ڿ�
				int result; // ��Ī ���
			};

			sNodeResult m_data[1024];
		};

} }