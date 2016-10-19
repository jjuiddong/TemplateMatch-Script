//
// 2016-07-15, jjuiddong
// ImageMatch ����� �����Ѵ�.
//
#pragma once


namespace cvproc {
	namespace imagematch {
		
		class cMatchScript;
		class cSharedData;

		class cMatchResult
		{
		public:
			cMatchResult();
			virtual ~cMatchResult();

			void Init(cMatchScript *p, const cv::Mat &input, const string &inputName,
				sParseTree *labelTree, const bool isRegisterInput, const bool isBlockMode);
			void MatchComplete();
			void TerminateMatch();
			void Clear();


		public:
			// �Է�
			int m_traverseType;	 // ��Ī ��ȸ�ϴ� ��� ����, {0 = ��Ī�Ǹ� ����, 1=��� ��Ī����, ���� ���շ��� ���� ��� ����}
			cMatchScript *m_script;
			cSharedData *m_sharedData;
			cv::Mat m_input;
			cv::Mat m_srcImage; // ���� �Է� �̹���, m_input �� ��Ī�� ��, ��鿵������ �ٲ��.
			string m_inputName; // @input+number
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
			int m_beginTime; // debug

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