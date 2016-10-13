#pragma once

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv/cvaux.h>
#include <opencv2/calib3d.hpp>


#include "tesseract/tesswrapper.h"
#include "imagematch.h"
#include "matchscript.h"
#include "matchthread.h"
#include "matchprocessor.h"
#include "matchmanager.h"
#include "MatchResult.h"



#ifdef _DEBUG
	#pragma comment(lib, "opencv_core310d.lib")
	#pragma comment(lib, "opencv_imgcodecs310d.lib")
	#pragma comment(lib, "opencv_highgui310d.lib")
	#pragma comment(lib, "opencv_imgproc310d.lib")
	#pragma comment(lib, "opencv_features2d310d.lib")
	#pragma comment(lib, "opencv_xfeatures2d310d.lib")
	#pragma comment(lib, "opencv_calib3d310d.lib")
	#pragma comment(lib, "opencv_videoio310d.lib")
	#pragma comment(lib, "opencv_flann310d.lib")
#else
	#pragma comment(lib, "opencv_core310.lib")
	#pragma comment(lib, "opencv_imgcodecs310.lib")
	#pragma comment(lib, "opencv_highgui310.lib")
	#pragma comment(lib, "opencv_imgproc310.lib")
#endif

#pragma comment( lib, "d3d9.lib" )
#pragma comment( lib, "d3dx9.lib" )
