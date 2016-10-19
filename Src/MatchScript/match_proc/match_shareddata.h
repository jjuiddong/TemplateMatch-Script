//
// 2016-10-15, jjuiddong
// match process 쓰레드 내에서 서로 공유하는 데이타를 관리한다.
//
//
#pragma once


namespace cvproc {
	namespace imagematch {

		class cSharedData
		{
		public:
			cSharedData();
			virtual ~cSharedData();

			cv::Mat& LoadImage(const string &fileName);
			cv::Mat& LoadScalarImage(const string &fileName, const cv::Scalar &scalar, const float scale);
			cv::Mat& LoadCvtImageAcc(const string &fileName, sParseTree *node);
			void LoadDescriptor(const string &fileName, OUT vector<cv::KeyPoint> **keyPoints, OUT cv::Mat **descriptor);
			void LoadDescriptor(const string &keyName, const cv::Mat &img, OUT vector<cv::KeyPoint> **keyPoints, OUT cv::Mat **descriptor, const bool isSearch = true);
			tess::cTessWrapper* GetTesseract();

			void SetInputImage(const cv::Mat &img, const string &name = "@input");
			void SetInputImage2(const cv::Mat &img, OUT string &name);
			void RemoveInputImage(const string &name);
			void RemoveScalarImage(const int imageId);
			void RemoveCvtImage(const int imageId);
			cMatchResult* AllocMatchResult();
			void FreeMatchResult(cMatchResult *p);
			void ClearMemPool();
			void Clear();


		public:
			map<string, cv::Mat* > m_imgTable;
			map<string, vector<cv::KeyPoint>* > m_keyPointsTable; // use feature detection
			map<string, cv::Mat* > m_descriptorTable; // use feature detection
			map<__int64, cv::Mat* > m_scalarImgTable;	// source image & scalar  table
			cv::Ptr<cv::xfeatures2d::SURF> m_detector;
			bool m_isGray = true;

			struct sCvtKey
			{
				__int64 key1; // hsv, hsl, scalar
				__int64 key2;
				__int64 key3;
				bool operator<(const sCvtKey&rhs) const;
				bool operator==(const sCvtKey&rhs) const;
			};
			map<sCvtKey, cv::Mat* > m_cvtImgTable;	// hsv, hls convert image table

			// match result pool
			struct sMRInfo
			{
				bool used;
				cMatchResult *p;
			};
			vector<sMRInfo> m_matchResults;

			// tesseract pool
			vector<tess::cTessWrapper*> m_tess;
			int m_tessIdx;
			CriticalSection m_tessCS;

			CriticalSection m_loadImgCS;
			CriticalSection m_loadDescriptCS;
			CriticalSection m_loadScalarImgCS;
			CriticalSection m_loadHsvImgCS;
			int m_inputImageId; // input image counting
			bool m_isMatchThreadLoop;
			bool m_isThreadTerminate; // false 일 때만, 스레드가 동작한다. 스레드를 중간에 멈추는 용도로 씀.
			HANDLE m_threadHandle;
			static cv::Mat m_nullMat; // empty Mat (only use null Mat return)
		};

	}
}
