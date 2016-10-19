#include "stdafx.h"
#include "match_script.h"
#include "match_processor.h"

using namespace cv;
using namespace cvproc;
using namespace cvproc::imagematch;
using namespace std;


cMatchScript::cMatchScript()
 	: m_matchType(0)
 	, m_treeId(0)
	, m_cloneLinkId(0)
{
}

cMatchScript::~cMatchScript()
{
	Clear();
}


void cMatchScript::Build(sParseTree *parent, sParseTree *prev, sParseTree *current)
{
	RET(!current);

	// check tree label node link
	// only link node,  paren==null is head node
	if ((current->type == 0) && parent)
	{
		if (const sParseTree *linkNode = FindTreeLabel(current->attrs["id"])) // 링크노드 일경우.
		{
			// 링크노드의 클론을 만들어 추가한다.
			sParseTree *node = linkNode->clone(false);
			node->next = current->next;
			SAFE_DELETE(current);
			if (prev)
				prev->next = node;
			else
				parent->child = node;

			Build(parent, node, node->next);
			return;
		}
	}

	current->id = m_treeId++;

	Build(current, NULL, current->child);
	Build(parent, current, current->next);
}


// tok1, tok2, tok3, tok4, tok5 ...  분리 
// node->str = tok1
// tok2~5 : roi x,y,w,h
// { tok = tok }
// return value : 0 = none
//						   1,2,3... = add symbol table, symbolCode
int cMatchScript::BuildAttributes(const sParseTree *node, const string &str, vector<string> &attributes)
{
	int reval = 0;

 	m_parser.m_lineStr = (char*)str.c_str();
	while (1)
	{
		const char *pid = m_parser.id();
		const char *pnum = (!*pid) ? m_parser.number() : NULL;

		if (*pid)
		{
			const string symb = m_parser.GetSymbol(pid);
			const int symbType = m_parser.GetSymbolType(pid);
			if (symbType == 0) // string type
			{
				if (symb.empty())
				{
					attributes.push_back(pid);
				}
				else
				{
					char *old = m_parser.m_lineStr;
					reval += BuildAttributes(node, symb.c_str(), attributes);
					m_parser.m_lineStr = old;
				}
			}
			else if (symbType == 1)
			{
				// string table type,
				// setTreeAttribute() 에서 처리한다.
				attributes.push_back(pid);
			}
		}
		else if (*pnum)
		{
			attributes.push_back(pnum);
		}
		else if (m_parser.m_lineStr[0] == '{')
		{
			// parsing, { id = id, id = id } 
			if (m_parser.assigned_list(node))
			{
				// 심볼테이블 트리 추가
				reval += 1;
			}
		}
		else
		{
			dbg::ErrLog("line {%d} error!! tree attribute fail [%s]\n", node->lineNum, str.c_str());
			m_parser.m_isError = true;
			break;
		}

		// comma check
		m_parser.m_lineStr = m_parser.passBlank(m_parser.m_lineStr);
		if (!m_parser.m_lineStr)
			break;
		if (*m_parser.m_lineStr == ',')
			++m_parser.m_lineStr;
	}

	return reval;
}


// threshold_0.9 
// templatematch, featurematch
void cMatchScript::SetTreeAttribute(sParseTree *node, vector<string> &attribs)
{
	node->id = m_treeId++;

	if (attribs.empty())
		return;

	vector<int> rmIdices; // remove attribute index array
	int hsvCnt = 0;
	int hlsCnt = 0;
}


// node 가 매칭에 성공했다면, 성공한 위치(matchLoc)의 이미지를 복사해 리턴한다.
// void cMatchScript::GetCloneMatchingArea(const Mat &input, const string &inputName, const int inputImageId, 
// 	sParseTree *node, OUT cv::Mat *out)
// {
// 	const Mat &matScene = cMatchProcessor::Get()->LoadImage(inputName);
// 	if (matScene.empty())
// 		return;
// 
// 	const Mat &matObj = cMatchProcessor::Get()->LoadImage(node->attrs["id"]);
// 	if (matObj.empty())
// 		return;
// 
// 	const cv::Size csize(input.cols - matObj.cols + 1, input.rows - matObj.rows + 1);
// 	if ((csize.height < 0) || csize.width < 0)
// 		return;
// 
// 	const Mat *src = &matScene;
// 
// 	// channel match
// 	if (!node->IsEmptyBgr())
// 	{
// 		cv::Scalar scalar;
// 		sscanf(node->attrs["scalar"].c_str(), "%lf,%lf,%lf", &scalar[0], &scalar[1], &scalar[2]);
// 		const float scale = (float)atof(node->attrs["scale"].c_str());
// 		src = &cMatchProcessor::Get()->LoadScalarImage(inputName, inputImageId, scalar, scale); // BGR
// 	}
// 
// 	// hsv match
// 	if (!node->IsEmptyCvt())
// 	{
// 		src = &cMatchProcessor::Get()->LoadCvtImageAcc(inputName, inputImageId, node);
// 	}
// 
// 	if (!src->data)
// 		return;// fail
// 
// 	const bool isFeatureMatch = ((m_matchType == 1) && (node->attrs["type"].empty())) || (node->attrs["type"] == "featurematch");
// 	if (!isFeatureMatch) // --> templatematch
// 	{
// 		Point left_top = node->matchLoc;
// 		*out = (*src)(Rect(left_top.x, left_top.y, matObj.cols, matObj.rows));
// 	}
// }


// 스크립트의 exec 명령어를 실행한다.
void cMatchScript::Exec()
{
	sParseTree *node = m_parser.m_execRoot;
	while (node)
	{
		stringstream ss(node->attrs["id"]);
		string label, file;
		ss >> label >> file;
		if (label.empty() || file.empty())
			break;

		Mat img = imread(file.c_str());
		if (!img.data)
			break;

		const int t1 = timeGetTime();
		//ExecuteEx(label, file, img, true);
		const int t2 = timeGetTime();
		//cout << "exec(" << label << ") << " << file << " = " << m_threadArg.resultStr << ", " << t2-t1 << endl;

		node = node->next;
	}
}


const sParseTree* cMatchScript::FindTreeLabel(const string &label) const
{
	auto it = m_treeLabelTable.find(label);
	return (m_treeLabelTable.end() != it)? it->second : NULL;
}


// 스크립트를 읽고, 파스트리를 생성한다.
bool cMatchScript::Read(const string &fileName)
{
	Clear();

	if (!m_parser.Read(fileName))
		return false;

	// head node들의 next 링크를 모두 제거한다. 독립적으로 작동하는 트리를 만들기 위해서다.
	vector<sParseTree*> headNodes; // 트리 헤드노드들을 순서대로 처리하기 위해서 만듬
	sParseTree *node = (m_parser.m_treeRoot)? m_parser.m_treeRoot->clone() : NULL; // tree copy
	while (node)
	{
		if ('@' != node->attrs["id"][0])
		{
			MessageBoxA(NULL, "Error!! ImageMatchScript LabelNode Name must start \'@\' ", "Error", MB_OK);
			return false;
		}

		m_treeLabelTable[node->attrs["id"]] = node;
		headNodes.push_back(node);
		sParseTree *next = node->next;
		node->next = NULL; // tree label node에는 자식만 연결된 상태로 등록된다. next는 없다.
		node = next;
	}

	m_treeId = 0;
	m_cloneLinkId = 0;
	Build(NULL, NULL, m_parser.m_execRoot);
	for each (auto it in headNodes)
		Build(NULL, NULL, it);

  	m_isDebug = atoi(m_parser.GetSymbol("debug").c_str()) ? true : false;
  	m_matchType = atoi(m_parser.GetSymbol("matchtype").c_str());

	// update node table
	m_nodes.clear();
	for each (auto it in m_treeLabelTable)
		m_parser.collectTree(it.second, m_nodes);

	ZeroMemory(m_nodeTable, sizeof(m_nodeTable));
	for each (auto it in m_nodes)
		m_nodeTable[it->id] = it;

	return true;
}


void cMatchScript::Clear()
{
	m_parser.Clear();

	for each (auto kv in m_nodes)
		delete kv;
	m_nodes.clear();
	m_treeLabelTable.clear();
}


// sParsetree 의 result 값을 초기화 한다.
// 함수가 재귀적으로 호출될 때, 중복 연산을 막기위해 쓰인다.
void cMatchScript::ClearResultTree() 
{
	for each (auto it in m_nodes)
	{
		it->result = -1;
		it->processCnt = 0;
	}
}
