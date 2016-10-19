//
// 2016-06-13, jjuiddong
// image match script executer
//
#pragma once

#include "match_parser.h"


namespace cvproc {
	namespace imagematch
	{

		class cMatchScript
		{
		public:
			cMatchScript();
			virtual ~cMatchScript();

			bool Read(const string &fileName);
			void Exec();
			const sParseTree* FindTreeLabel(const string &label) const;
			void Clear();


		public:
			void Build(sParseTree *parent, sParseTree *prev, sParseTree *current);
			int BuildAttributes(const sParseTree *node, const string &str, vector<string> &attributes);
			void SetTreeAttribute(sParseTree *node, vector<string> &attribs);
// 			void GetCloneMatchingArea(const cv::Mat &input, const string &inputName, const int inputImageId, sParseTree *node, 
// 				OUT cv::Mat *out);
			void ClearResultTree();


		public:
			cParser m_parser;
			map<string, sParseTree*> m_treeLabelTable;
			cvproc::imagematch::sParseTree* m_nodeTable[1024]; // key = node->id
			set<cvproc::imagematch::sParseTree*> m_nodes;
			bool m_isDebug = false;
			int m_matchType; // 0=template match, 1=feature match, default = 0
 			int m_treeId; // sParseTree id 부여
			int m_cloneLinkId; // 새로 생성된 트리 이름에 사용
		};

	}
}
