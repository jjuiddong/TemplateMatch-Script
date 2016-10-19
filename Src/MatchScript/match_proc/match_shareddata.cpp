
#include "stdafx.h"
#include "match_shareddata.h"

using namespace cv;
using namespace cvproc;
using namespace imagematch;

Mat cSharedData::m_nullMat; // static 변수 선언

//-------------------------------------------------------------------------------------------------------
// sCvtKey
bool cSharedData::sCvtKey::operator<(const sCvtKey&rhs) const
{
	if (key1 < rhs.key1)
	{
		return true;
	}
	else 	if (key1 == rhs.key1)
	{
		if (key2 < rhs.key2)
			return true;
		else if (key2 == rhs.key2)
			return key3 < rhs.key3;
		else
			return false;
	}
	else
	{
		return false;
	}
}

bool cSharedData::sCvtKey::operator==(const sCvtKey&rhs) const
{
	return (key1 == rhs.key1) && (key2 == rhs.key2) && (key3 == rhs.key3);
}
//-------------------------------------------------------------------------------------------------------


cSharedData::cSharedData()
	: m_tessIdx(0)
	, m_inputImageId(0)
{
//	if (m_tess.empty())
	if (0)
	{
		for (int i = 0; i < 16; ++i)
		{
			tess::cTessWrapper *p = new tess::cTessWrapper();
			p->Init("./", "eng", "dictionary.txt");
			m_tess.push_back(p);
		}
	}

}

cSharedData::~cSharedData()
{
	Clear();
}


Mat& cSharedData::LoadImage(const string &fileName)
{
	AutoCSLock cs(m_loadImgCS);

	auto it = m_imgTable.find(fileName);
	if (m_imgTable.end() != it)
	{
		if (it->second)
			return *it->second;
	}

	Mat *img = new Mat();
	*img = imread(fileName);
	if (!img->data)
	{
		dbg::ErrLog("Error!! loadImage name=%s\n", fileName.c_str());
		return m_nullMat;
	}

	if (m_isGray)
		cvtColor(*img, *img, CV_BGR2GRAY);
	m_imgTable[fileName] = img;
	return *img;
}


cv::Mat& cSharedData::LoadScalarImage(const string &fileName, const cv::Scalar &scalar, const float scale)
{
	int imageId;
	sscanf_s(fileName.c_str(), "%*s %d", &imageId);

	const __int64 key = ((__int64)imageId << 32) |
		((__int64)scalar[0]) << 24 | ((__int64)scalar[1]) << 16 | ((__int64)scalar[2]) << 8 | ((__int64)(scale * 10));

	AutoCSLock cs(m_loadScalarImgCS);
	auto it = m_scalarImgTable.find(key);
	if (m_scalarImgTable.end() != it)
		return *it->second;

	Mat &src = LoadImage(string("color") + fileName);
	if (!src.data)
		return m_nullMat;

	Mat *mat = new Mat();
	*mat = src.clone() & scalar;
	if (m_isGray)
	{
		cvtColor(*mat, *mat, CV_BGR2GRAY);
		if (scale > 0)
			*mat *= scale;
	}

	m_scalarImgTable[key] = mat;
	return *mat;
}


cv::Mat& cSharedData::LoadCvtImageAcc(const string &fileName, sParseTree *node)
{
	RETV(!node, m_nullMat);

	int imageId;
	sscanf_s(fileName.c_str(), "%*s %d", &imageId);

	// make key
	__int64 keys[6] = { 0,0,0, 0,0,0 };
	int keyCnt = 0;
	for (int i = 0; i < 3; ++i)
	{
		if (!node->IsEmptyHsv(i))
		{
			int hsv[6];
			sscanf(node->attrs[common::format("hsv%d", i)].c_str(), "%d,%d,%d,%d,%d,%d", &hsv[0], &hsv[1], &hsv[2], &hsv[3], &hsv[4], &hsv[5]);

			const __int64 key = ((__int64)hsv[0]) << 16 | ((__int64)hsv[1]) << 8 | ((__int64)hsv[2]) |
				((__int64)hsv[3]) << 40 | ((__int64)hsv[4]) << 32 | ((__int64)hsv[5]) << 24 |
				((__int64)imageId << 48);
			keys[keyCnt++] = key;
		}
	}

	for (int i = 0; i < 3; ++i)
	{
		if (!node->IsEmptyHls(i))
		{
			int hls[6];
			sscanf(node->attrs[common::format("hls%d", i)].c_str(), "%d,%d,%d,%d,%d,%d", &hls[0], &hls[1], &hls[2], &hls[3], &hls[4], &hls[5]);

			const __int64 key = ((__int64)hls[0]) << 16 | ((__int64)hls[1]) << 8 | ((__int64)hls[2]) |
				((__int64)hls[3]) << 40 | ((__int64)hls[4]) << 32 | ((__int64)hls[5]) << 24 |
				((__int64)imageId << 48);
			keys[keyCnt++] = key;
		}
	}

	AutoCSLock cs(m_loadHsvImgCS);
	auto it = m_cvtImgTable.find({ keys[0], keys[1],keys[2] });
	if (m_cvtImgTable.end() != it)
		return *it->second;

	Mat &src = LoadImage(string("color") + fileName);
	if (!src.data)
		return m_nullMat;

	Mat *dst = NULL;

	// hsv conversion
	{
		Mat hsv;
		for (int i = 0; i < 3; ++i)
		{
			if (!node->IsEmptyHsv(i))
			{
				if (!hsv.data)
					cvtColor(src, hsv, CV_BGR2HSV);

				const string key = common::format("hsv%d", i);
				Scalar lower, upper;
				sscanf(node->attrs[key].c_str(), "%lf,%lf,%lf,%lf,%lf,%lf", &lower[0], &lower[1], &lower[2], &upper[0], &upper[1], &upper[2]);

				Mat tmp;
				inRange(hsv, lower, upper, tmp);

				if (!dst)
				{
					dst = new Mat();
					*dst = Mat::zeros(src.rows, src.cols, tmp.flags);
				}

				*dst += tmp;
			}
		}
	}

	// hsl conversion
	{
		Mat hls;
		for (int i = 0; i < 3; ++i)
		{
			if (!node->IsEmptyHls(i))
			{
				if (!hls.data)
					cvtColor(src, hls, CV_BGR2HLS);

				const string key = common::format("hls%d", i);
				Scalar lower, upper;
				sscanf(node->attrs[key].c_str(), "%lf,%lf,%lf,%lf,%lf,%lf", &lower[0], &lower[1], &lower[2], &upper[0], &upper[1], &upper[2]);

				Mat tmp;
				inRange(hls, lower, upper, tmp);

				if (!dst)
				{
					dst = new Mat();
					*dst = Mat::zeros(src.rows, src.cols, tmp.flags);
				}

				*dst += tmp;
			}
		}
	}

	if (keyCnt > 1)
	{
		threshold(*dst, *dst, 240, 255, CV_THRESH_BINARY);
		erode(*dst, *dst, Mat());
	}

	m_cvtImgTable[{ keys[0], keys[1], keys[2] }] = dst;
	return *dst;
}


void cSharedData::LoadDescriptor(const string &fileName, OUT vector<KeyPoint> **keyPoints, OUT cv::Mat **descriptor)
{
	if (fileName.empty())
	{
		*keyPoints = NULL;
		*descriptor = NULL;
		return;
	}

	{
		AutoCSLock cs(m_loadDescriptCS); // 소멸자 호출을 위한 지역변수
		auto it = m_keyPointsTable.find(fileName);
		if (m_keyPointsTable.end() != it)
		{
			*keyPoints = it->second;
			*descriptor = m_descriptorTable[fileName];
			return;
		}
	}

	const Mat &src = LoadImage(fileName);
	LoadDescriptor(fileName, src, keyPoints, descriptor);
}


void cSharedData::LoadDescriptor(const string &keyName, const cv::Mat &img,
	OUT vector<KeyPoint> **keyPoints, OUT cv::Mat **descriptor, const bool isSearch) //isSearch=true
{
	if (keyName.empty())
	{
		*keyPoints = NULL;
		*descriptor = NULL;
		return;
	}

	AutoCSLock cs(m_loadDescriptCS);

	if (isSearch)
	{
		auto it = m_keyPointsTable.find(keyName);
		if (m_keyPointsTable.end() != it)
		{
			*keyPoints = it->second;
			*descriptor = m_descriptorTable[keyName];
			return; // find, and return
		}
	}

	// not found, Compute keyPoints, Descriptor
	vector<KeyPoint> *keyPts = new vector<KeyPoint>();
	Mat *descr = new Mat();
	m_detector->detectAndCompute(img, Mat(), *keyPts, *descr);

	delete m_keyPointsTable[keyName];
	m_keyPointsTable[keyName] = keyPts;
	delete m_descriptorTable[keyName];
	m_descriptorTable[keyName] = descr;

	*keyPoints = keyPts;
	*descriptor = descr;
}


// 입력 영상을 등록한다.
// 컬러입력 영상일 경우, 흑백 영상으로 저장하고, color+name 으로 원본 영상을 저장한다.
void cSharedData::SetInputImage(const cv::Mat &img, const string &name) // name=@input
{
	const string colorName = string("color") + name;

	{
		AutoCSLock cs(m_loadImgCS); // 소멸자 호출을 위한 지역변수

		delete m_imgTable[name];
		delete m_imgTable[colorName];

		// 1 channel
		if (img.channels() == 1) // Gray
		{
			m_imgTable[name] = new Mat();
			*m_imgTable[name] = img.clone(); // register input image
		}
		else if (img.channels() >= 3) // BGR -> Gray
		{
			m_imgTable[name] = new Mat();
			m_imgTable[colorName] = new Mat();

			*m_imgTable[colorName] = img.clone(); // source color image
			cvtColor(img, *m_imgTable[name], CV_BGR2GRAY);
		}
	}

	{
		AutoCSLock cs(m_loadDescriptCS); // 소멸자 호출을 위한 지역변수

		delete m_keyPointsTable[name];
		m_keyPointsTable.erase(name);
		delete m_descriptorTable[name];
		m_descriptorTable.erase(name);

		delete m_keyPointsTable[colorName];
		m_keyPointsTable.erase(colorName);
		delete m_descriptorTable[colorName];
		m_descriptorTable.erase(colorName);
	}
}


void cSharedData::SetInputImage2(const cv::Mat &img, OUT string &name)
{
	// 중복된 input name을 쓸 경우, 비동기처리시 문제가 발생한다.
	// 쓰레드가 끝나지 않은 상황에서, 같은 input name을 바꿀 수 있다.
	// 그래서 중복되지 않는 input name을 설정해야 한다.

	++m_inputImageId;
	if (m_inputImageId > 1000000) // limit check
		m_inputImageId = 0;

	stringstream ss;
	ss << "@input " << m_inputImageId;
	name = ss.str();

	SetInputImage(img, name);
}


void cSharedData::RemoveInputImage(const string &name)
{
	{
		AutoCSLock cs(m_loadImgCS); // 소멸자 호출을 위한 지역변수

		auto it = m_imgTable.find(name);
		if (it != m_imgTable.end())
		{
			delete it->second;
			m_imgTable.erase(it);
		}

		const string colorName = string("color") + name;
		auto it2 = m_imgTable.find(colorName);
		if (it2 != m_imgTable.end())
		{
			delete it2->second;
			m_imgTable.erase(it2);
		}
	}

	{
		AutoCSLock cs(m_loadDescriptCS); // 소멸자 호출을 위한 지역 변수

		delete m_keyPointsTable[name];
		m_keyPointsTable.erase(name);

		delete m_descriptorTable[name];
		m_descriptorTable.erase(name);
	}

	int imageId;
	sscanf_s(name.c_str(), "%*s %d", &imageId); //@input + number
	RemoveScalarImage(imageId);
	RemoveCvtImage(imageId);
}


void cSharedData::RemoveScalarImage(const int imageId)
{
	AutoCSLock cs(m_loadScalarImgCS);

	auto it = m_scalarImgTable.begin();
	while (m_scalarImgTable.end() != it)
	{
		const int id = (int)(it->first >> 32);
		if (id == imageId)
		{
			delete it->second;
			it = m_scalarImgTable.erase(it++);
		}
		else
		{
			++it;
		}
	}
}


void cSharedData::RemoveCvtImage(const int imageId)
{
	AutoCSLock cs(m_loadHsvImgCS);

	auto it = m_cvtImgTable.begin();
	while (m_cvtImgTable.end() != it)
	{
		const int id = (int)(it->first.key1 >> 48);
		if (id == imageId)
		{
			delete it->second;
			m_cvtImgTable.erase(it++);
		}
		else
		{
			++it;
		}
	}

}


void cSharedData::ClearMemPool()
{
	{
		AutoCSLock cs(m_loadImgCS); // 소멸자 호출을 위한 지역변수

		for each (auto kv in m_imgTable)
			delete kv.second;
		m_imgTable.clear();
	}

	{
		AutoCSLock cs(m_loadDescriptCS); // 소멸자 호출을 위한 지역변수

		for each (auto it in m_keyPointsTable)
			delete it.second;
		m_keyPointsTable.clear();

		for each (auto it in m_descriptorTable)
			delete it.second;
		m_descriptorTable.clear();
	}

	{
		AutoCSLock cs(m_loadScalarImgCS);
		for each (auto it in m_scalarImgTable)
			delete it.second;
		m_scalarImgTable.clear();
	}

	{
		AutoCSLock cs(m_loadHsvImgCS);
		for each (auto it in m_cvtImgTable)
			delete it.second;
		m_cvtImgTable.clear();
	}
}

void cSharedData::Clear()
{
	ClearMemPool();

	for each (auto mr in m_matchResults)
		delete mr.p;
	m_matchResults.clear();

	for each (auto p in m_tess)
		delete p;
	m_tess.clear();
}


// MatchResult 를 생성해서 리턴한다.
cMatchResult* cSharedData::AllocMatchResult()
{
	for (uint i = 0; i < m_matchResults.size(); ++i)
	{
		if (!m_matchResults[i].used)
		{
			m_matchResults[i].used = true;
			m_matchResults[i].p->m_sharedData = this;
			return m_matchResults[i].p;
		}
	}

	cMatchResult *p = new cMatchResult;
	p->m_sharedData = this;
	m_matchResults.push_back({ true, p });

	dbg::Log("matchResult count = %d \n", m_matchResults.size());
	return p;
}


void cSharedData::FreeMatchResult(cMatchResult *p)
{
	for (uint i = 0; i < m_matchResults.size(); ++i)
	{
		if (m_matchResults[i].p == p)
		{
			m_matchResults[i].used = false;
			return;
		}
	}

	dbg::ErrLog("cMatchProcessor::FreeMatchResult() Not Found pointer \n");
}


tess::cTessWrapper* cSharedData::GetTesseract()
{
	AutoCSLock cs(m_tessCS);

	const int MAX_TESS = 16;
	if ((m_tessIdx >= (int)m_tess.size()) && (m_tess.size() < MAX_TESS))
	{
		tess::cTessWrapper *p = new tess::cTessWrapper();
		p->Init("./", "eng", "dictionary.txt");
		m_tess.push_back(p);
	}

	tess::cTessWrapper *p = m_tess[m_tessIdx++];
	if (MAX_TESS <= m_tessIdx)
		m_tessIdx = 0;

	return p;
}
