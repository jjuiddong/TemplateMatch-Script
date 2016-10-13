//
// 2016-06-26, jjuiddong
//
// ���ڿ� �ɼ����� �̹����� �������ؼ� �����Ѵ�.
// option
//     - bgr=num1,num2,num3
//         - Mat &= Scalar(num1,num2,num3)
//     - scale=num
//         - Mat *= num
//     - gray=0/1
//         - gray convert
//		- invert=150
//			- threshold()
//     - hsv=num1,num2,num3,num4,num5,num6
//         - hsv converting, inRange( Scalar(num1,num2,num3), Scalar(num4,num5,num6) )
//
#pragma once

namespace cvproc
{

	class cConvertImage
	{
	public:
		cConvertImage();
		virtual ~cConvertImage();
		void Convert(const cv::Mat &src, OUT cv::Mat &dst, const string &option);
	};

}
