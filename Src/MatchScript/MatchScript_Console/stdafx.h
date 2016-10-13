#pragma once

#pragma warning(disable: 4819)


#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS 
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#endif

#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif



// 매크로 정의
#ifndef SAFE_DELETE
#define SAFE_DELETE(p) {if (p) { delete p; p=NULL;} }
#endif
#ifndef SAFE_DELETEA
#define SAFE_DELETEA(p) {if (p) { delete[] p; p=NULL;} }
#endif
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) {if (p) { p->Release(); p=NULL;} }
#endif
#ifndef SAFE_RELEASE2
#define SAFE_RELEASE2(p) {if (p) { p->release(); p=NULL;} }
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif

#define RET(exp)		{if((exp)) return; }			// exp가 true이면 리턴
#define RETV(exp,val)	{if((exp)) return val; }
#define ASSERT_RET(exp)		{assert(exp); RET(!(exp) ); }
#define ASSERT_RETV(exp,val)		{assert(exp); RETV(!(exp),val ); }
#define BRK(exp)		{if((exp)) break; }			// exp가 break


typedef unsigned int    u_int;


#include <iostream>
#include <windows.h>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <set>
#include <queue>
#include <thread>
#include <shlwapi.h>
#include <process.h>
#include <iterator>
#include <shlwapi.h>
#include <mmsystem.h>
#pragma comment( lib, "winmm.lib" )
#pragma comment( lib, "shlwapi.lib" )


using std::string;
using std::wstring;
using std::vector;
using std::map;
using std::list;
using std::set;
using std::queue;
using std::stringstream;
using std::wstringstream;


//------------------------------------------------------------------------------
// extra files, common files
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv/cvaux.h>
#include <opencv2/calib3d.hpp>

#include "../extra/utility.h"
using namespace common;

#include "../extra/config.h"
#include "../extra/math/Math.h"
#include "../extra/rectcontour.h"
#include "../extra/rectcontour2.h"
#include "../extra/screencapture.h"
#include "../extra/linecontour.h"
#include "../extra/RecognitionConfig.h"
#include "../extra/detectpoint.h"
#include "../extra/squarecontour.h"
#include "../extra/skewdetect.h"
#include "../extra/deskew.h"
#include "../extra/convertimage.h"
#include "../extra/featurematch.h"
#include "../extra/utility2.h"
//------------------------------------------------------------------------------


#include "../match_proc/matchproc.h"
