//
// Point�� �ν��ϴµ� �ʿ��� �Ķ���͵��� �����Ѵ�.
// �ַ� cv::SimpleBlobDetector::Params �������� �����Ѵ�.
//
#pragma once


struct sRecognitionConfigAttribute
{
	int detectPointThreshold;
	float detectPointMinArea;
	float detectPointMaxArea;
	float detectPointCircularity;
	float detectPointMinConvexity;
	float detectPointMinInertia;
	bool printDetectPoint;
	float detectPointMinLineDist;
	float detectPointMaxLineDist;
	int detectPointLoopCount;

	float detectLineMaxDot;
	float detectLineMinAngle;
	float detectLineMinOldAngle; // �� �� ���ΰ� �ּ� ���� ��
	float detectLineMinDiffDistance; // ���� ���ΰ��� �Ÿ��� �����̻� �־����� ���� ����ó��

	// ThresholdFinder
	int thresholdFinderOffset;

};


class cRecognitionConfig : public common::cConfig
{
public:
	virtual void InitDefault() override;
	virtual void UpdateParseData() override;
	bool Write(const string &fileName);

	cRecognitionConfig& operator = (const cRecognitionConfig &rhs);


public:
	sRecognitionConfigAttribute m_attr;
};

