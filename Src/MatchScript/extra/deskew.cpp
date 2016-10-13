
#include "stdafx.h"
#include "deskew.h"

using namespace cvproc;
using namespace cv;


cDeSkew::cDeSkew()  : m_pts(4)
{
}

cDeSkew::~cDeSkew()
{
}


//
// canny�� ������ �����, ���� �� ������ ã��, 
// �� ���򼱿� ������ �� ��° ������ ã��, �� ���� ��������
//  ��ü ������ ȸ����Ű��, �μ����� ������� �ڽ��� �����ϰ�,  �����Ѵ�.
// ȸ���� ������ src�� ����ǰ�, �ڽ��� m_tessImg�� ����ȴ�.
//
bool cDeSkew::DeSkew( 
	INOUT Mat &src,
	const double arcAlpha, //= 0.005f,
	const int deSkewDebug, // = 0
	const bool isTesseractOcr) // = false
{
	if (!src.data)
		return false;

	Mat &dst = src;
	Mat tmp = dst.clone();
	Mat tmp2 = dst.clone();

	if (tmp2.data && (tmp2.channels() >= 3))
		cvtColor(tmp2, tmp2, CV_BGR2GRAY);
	threshold(tmp2, tmp2, 20, 255, CV_THRESH_BINARY_INV);
	cv::Canny(tmp2, tmp2, 0, 100, 5);

	vector<vector<cv::Point>> contours;
	cv::findContours(dst, contours, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

	// �¿�� ����� �ٰ����� ã�´�.
	int maxLenIdx = -1;
	int maxLength = 0;
	std::vector<cv::Point> maxLine;
	std::vector<cv::Point> approx;
	for (u_int i = 0; i < contours.size(); i++)
	{
		cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*arcAlpha, true);
		if (approx.empty())
			continue;

		int minX = 10000;
		int maxX = 0;
		for each (auto pos in approx)
		{
			if (minX > pos.x)
				minX = pos.x;
			if (maxX < pos.x)
				maxX = pos.x;
		}

		const int len = abs(maxX - minX);
		if (maxLength < len)
		{
			maxLength = len;
			maxLenIdx = i;
		}
	}

	if (maxLenIdx == -1)
		return false; // error occur


	// �¿�� ���� �� �ٰ����� ������ ������ ����ؼ�, ���� �̹����� ������ �ǰ� ȸ����Ų��.
	cv::approxPolyDP(cv::Mat(contours[maxLenIdx]), approx, cv::arcLength(cv::Mat(contours[maxLenIdx]), true)*arcAlpha, true);

	double maxV = 0, maxV2 = 0;
	uint idx1 = 0, idx2 = 0;
	// ���� �� ������ ���Ѵ�. ������ circular line �� ����
	for (u_int i = 0; i < approx.size() - 1; ++i)
	{
		const int i2 = (i + 1) % approx.size(); // circular rect
		const double n = cv::norm(approx[i] - approx[i2]);
		if (maxV < n)
		{
			maxV = n;
			idx1 = i;
		}
	}

	// �� ��°�� �� ������ ���Ѵ�. ���� �� ���ΰ� �����̾�� �Ѵ�.

	// ���� �� ������ ���͸� ���Ѵ�.
	vector<Vector3> lineDirs(approx.size());
	for (u_int i = 0; i < approx.size(); ++i)
	{
		const int i2 = (i + 1) % approx.size(); // circular rect
		const Vector3 p1 = Vector3((float)approx[i].x, (float)approx[i].y, 0);
		const Vector3 p2 = Vector3((float)approx[i2].x, (float)approx[i2].y, 0);
		Vector3 v = p2 - p1;
		v.Normalize();
		lineDirs[i] = v;
	}

	struct sLine
	{
		double angle;
		uint i1; // approx index1
		uint i2; // approx index1
		double len; // length
	};
	//typedef pair<double, int> LineType; // angle, index
	vector<sLine> lines;
	lines.reserve(approx.size());

	// ���� �پ��ִ� ������ ���, �����Ѵ�. (y  ������ �Ǵ��Ѵ�.)
	const int minimumLength = 20;
	for (uint i = 0; i < approx.size(); ++i)
	{
		if (i == idx1)
			continue; // max �� ����

		const uint i2 = (i + 1) % approx.size(); // circular rect
		const double len = cv::norm(approx[i] - approx[i2]);
		const float angle = abs(lineDirs[idx1].DotProduct(lineDirs[i]));
		if (angle > 0.8f)
			lines.push_back({ angle, i, i2, len });
	}

	// ���� �� ���� �߰�
	lines.push_back({ 1, idx1, idx1 + 1, maxV });

	// �� ���γ���, �����̸鼭, �Ÿ��� �� ���� ���Ѵ�.
	struct sLinePair {
		float angle;
		int idx1;
		int idx2;
		double len;
	};

	vector<sLinePair> linePairs;
	linePairs.reserve(lines.size() * lines.size());

	for (uint i = 0; i < lines.size() - 1; ++i)
	{
		for (uint k = i + 1; k < lines.size(); ++k)
		{
			// y ���̰� ����ũ�� �̻��̾�� �Ѵ�.
			const int i1 = lines[i].i1;
			const int i12 = lines[i].i2;
			const int i2 = lines[k].i1;
			const int i22 = lines[k].i2;
			const int left1 = (approx[i1].x < approx[i12].x) ? i1 : i12;
			const int left2 = (approx[i2].x < approx[i22].x) ? i2 : i22;
			if (abs(approx[left1].y - approx[left2].y) < minimumLength)
				continue;

			sLinePair type;
			type.angle = abs(lineDirs[lines[i].i1].DotProduct(lineDirs[lines[k].i1]));
			type.idx1 = lines[i].i1;
			type.idx2 = lines[k].i1;
			type.len = lines[i].len + lines[k].len;
			linePairs.push_back(type);
		}
	}

	// ��鼭, ���� ������ �� ������ ���Ѵ�.
	std::sort(linePairs.begin(), linePairs.end(),
		[](const sLinePair &a, sLinePair &b)
	{
		return a.angle*a.len > b.angle*b.len;
	});

	if (!linePairs.empty())
	{
		idx1 = linePairs.front().idx1;
		idx2 = linePairs.front().idx2;
	}


	const int i1 = idx1;
	const int i12 = (idx1 + 1) % approx.size();
	const int i2 = idx2;
	const int i22 = (idx2 + 1) % approx.size();

	// debug display
	if (deSkewDebug)
	{
		cvtColor(dst, dst, CV_GRAY2BGR);
		for (u_int i = 0; i < approx.size() - 1; ++i)
			cv::line(dst, approx[i], approx[i + 1], Scalar(255, 255, 255), 2);
		cv::line(dst, approx[approx.size() - 1], approx[0], Scalar(255, 255, 255), 2);
		cv::line(dst, approx[i1], approx[i12], Scalar(255, 0, 0), 3);
		cv::line(dst, approx[i2], approx[i22], Scalar(0, 0, 255), 3);
	}


	//
	// p1 -------- p2
	// |                 |
	// |                 |
	// p3 -------- p4
	//
	const Vector3 p1 = (approx[i1].x < approx[i12].x) ? Vector3((float)approx[i1].x, (float)approx[i1].y, 0) : Vector3((float)approx[i12].x, (float)approx[i12].y, 0);
	const Vector3 p2 = (approx[i1].x > approx[i12].x) ? Vector3((float)approx[i1].x, (float)approx[i1].y, 0) : Vector3((float)approx[i12].x, (float)approx[i12].y, 0);
	const Vector3 p3 = (approx[i2].x < approx[i22].x) ? Vector3((float)approx[i2].x, (float)approx[i2].y, 0) : Vector3((float)approx[i22].x, (float)approx[i22].y, 0);
	const Vector3 p4 = (approx[i2].x > approx[i22].x) ? Vector3((float)approx[i2].x, (float)approx[i2].y, 0) : Vector3((float)approx[i22].x, (float)approx[i22].y, 0);
	const Vector3 _p1 = (p1.y < p3.y) ? p1 : p3;
	const Vector3 _p2 = (p1.y < p3.y) ? p2 : p4;
	const Vector3 _p3 = (p1.y < p3.y) ? p3 : p1;
	const Vector3 _p4 = (p1.y < p3.y) ? p4 : p2;

	Vector3 v = p2 - p1;
	v.Normalize();
	double angle = RAD2ANGLE(acos(v.DotProduct(Vector3(1, 0, 0))));
	if (v.y < 0)
		angle = -angle;

	// �� ���� ���̰� ���� ���ٸ�, 4���� �������� skew�� �Ѵ�.
	const int diffLen = abs((p1 - p2).Length() - (p3 - p4).Length());
	if (diffLen < 20)
	{
		m_pts[0] = Point((int)_p1.x, (int)_p1.y);
		m_pts[1] = Point((int)_p2.x, (int)_p2.y);
		m_pts[2] = Point((int)_p4.x, (int)_p4.y);
		m_pts[3] = Point((int)_p3.x, (int)_p3.y);
	}
	else
	{
		// �簢���� ����� skew �Ѵ�.
		m_pts[0] = Point((int)_p1.x, (int)_p1.y);
		m_pts[1] = Point((int)_p2.x, (int)_p2.y);
		m_pts[2] = Point((int)_p2.x, (int)_p4.y);
		m_pts[3] = Point((int)_p1.x, (int)_p3.y);
	}

	m_deSkewPoint1 = Point((int)p1.x, (int)p1.y);
	m_deSkewPoint2 = Point((int)p3.x, (int)p3.y);
	m_deSkewPoint3 = (p2.x > p4.x) ? Point((int)p2.x, (int)p2.y) : Point((int)p4.x, (int)p4.y);

	// P1,P2,P3,P4 ������ deskew �Ѵ�.data
	int left = 10000, top = 10000, right = 0, bottom = 0;
	for each (auto &pos in m_pts)
	{
		if (pos.x < left)
			left = pos.x;
		if (pos.y < top)
			top = pos.y;
		if (pos.x > right)
			right = pos.x;
		if (pos.y > bottom)
			bottom = pos.y;
	}

	if (isTesseractOcr)
	{
		const int height = (int)(abs(p3.y - p1.y) * cos(ANGLE2RAD(angle)));
		cRectContour rect(m_pts);
		cSkewDetect skewDetect;
		skewDetect.Init(rect, 1.f, rect.Width(), height);
		m_tessImg = skewDetect.Transform(tmp);

		// �� �Ʒ� ����� �����Ѵ�.
		const int culTop = CullingTop(m_tessImg);
		const int culBottom = CullingBottom(m_tessImg);
		const int culT = (culTop < m_tessImg.rows / 2) ? culTop : 0;
		const int culB = (culBottom > m_tessImg.rows / 2) ? culBottom : m_tessImg.rows;
		// ������ ������ŭ, �̹����� ¥����. ����� �ʹ�ũ��, �����ν��� ���� �ʴ´�.
		if (culB - culT != m_tessImg.rows) // ũ���� ��ȭ�� ���� ����
			m_tessImg = m_tessImg(Rect(0, culT, m_tessImg.cols, culB-culT));
	}

	return true;
}


// return culling top position
int cDeSkew::CullingTop(const Mat &src)
{
	int culTop = 0;
	int culRight = 0;
	int culRightMinY = 0;
	for (int i = 0; i < src.rows; ++i)
	{
		bool checkNextRow = false;
		const uchar *p = src.ptr(i) + 100; // 100 pixel ������ �����ؼ� �˻�, ���ʿ� ���η� ���� �׾��� ��찡 ����.
		for (int k = 100; k < src.cols; ++k, ++p)
		{
			if (*p < 200)
			{
				// ����� �߻��� ��ġ�� �����Ѵ�. ���� �� ��ġ ����
				if (k > culRight)
				{
					culRight = k;
					culRightMinY = culTop;
				}

				checkNextRow = true;
				break; // check next row
			}
		}

		if (!checkNextRow)
			break;

		++culTop;
		if (src.rows / 2 <= culTop)
			break;
	}

	// ������ üũ, culY�� ũ�ٸ�, ����� ������ ��ġ���� �ø��Ѵ�.
	// ��������: ������ ��ġ�� �ʺ��� 20% ���� �϶�.
	if ((culTop >= src.rows / 2)
		&& (((float)abs(src.cols - culRight) / (float)src.cols) < 0.2f))
		culTop = culRightMinY;

	return culTop;
}


// return culling bottom position
int cDeSkew::CullingBottom(const Mat &src)
{
	int culBottom = src.rows;
	int culRight = 0;
	int culRightMinY = 0;

	// �Ʒ����� ���� �˻��Ѵ�.
	for (int i = src.rows-1; i >= 0; --i)
	{
		bool checkNextRow = false;
		const uchar *p = src.ptr(i) + 10; // 10 pixel ������ �����ؼ� �˻�, ���ʿ� ���η� ���� �׾��� ��찡 ����.
		for (int k = 10; k < src.cols; ++k, ++p)
		{
			if (*p < 200)
			{
				// ����� �߻��� ��ġ�� �����Ѵ�. ���� �� ��ġ ����
				if (k > culRight)
				{
					culRight = k;
					culRightMinY = culBottom;
				}

				checkNextRow = true;
				break; // check next row
			}
		}

		if (!checkNextRow)
			break;

		--culBottom;
		if (src.rows / 2 >= culBottom)
			break;
	}

	// ������ üũ, culY�� ũ�ٸ�, ����� ������ ��ġ���� �ø��Ѵ�.
	// ��������: ������ ��ġ�� �ʺ��� 20% ���� �϶�.
	if ((culBottom >= src.rows / 2)
		&& (((float)abs(src.cols - culRight) / (float)src.cols) < 0.2f))
		culBottom = culRightMinY;

	return culBottom;
}
