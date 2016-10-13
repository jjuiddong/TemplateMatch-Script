
#include "stdafx.h"
#include "utility.h"


using namespace common;
using namespace dbg;



void common::replaceAll(std::string& str, const std::string& from, const std::string& to) {
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

// "  skdfjskdjf  "
// "$$$skdfjskdjf$$$"
// "skdfjskdjf"
string& common::trim(string &str)
{
	// 앞에서부터 검색
	for (int i = 0; i < (int)str.length(); ++i)
	{
		if ((str[i] == '\n') || (str[i] == '\t') || (str[i] == '\r') || (str[i] == ' '))
		{
			rotatepopvector(str, i);
			//			str[ i] = '$';
			--i;
		}
		else
			break;
	}

	// 뒤에서부터 검색
	for (int i = (int)str.length() - 1; i >= 0; --i)
	{
		if ((str[i] == '\n') || (str[i] == '\t') || (str[i] == '\r') || (str[i] == ' '))
		{
			rotatepopvector(str, i);
			//			str[ i] = '$';
		}
		else
			break;
	}

	//	replaceAll(str, "$", "");
	return str;
}


// "  skdfjskdjf  "
// "$$$skdfjskdjf$$$"
// "skdfjskdjf"
void common::trimw(wstring &str)
{
	str = str2wstr(trim(wstr2str(str)));
}


//------------------------------------------------------------------------
// 유니코드를 멀티바이트 문자로 변환
//------------------------------------------------------------------------
std::string common::wstr2str(const std::wstring &wstr)
{
	const int slength = (int)wstr.length() + 1;
	const int len = ::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), slength, 0, 0, NULL, FALSE);
	char* buf = new char[len];
	::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), slength, buf, len, NULL, FALSE);
	std::string r(buf);
	delete[] buf;
	return r;
}


//------------------------------------------------------------------------
// 멀티바이트 문자를 유니코드로 변환
//------------------------------------------------------------------------
std::wstring common::str2wstr(const std::string &str)
{
	int len;
	int slength = (int)str.length() + 1;
	len = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	::MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}


//------------------------------------------------------------------------
// 스트링포맷
//------------------------------------------------------------------------
std::string common::format(const char* fmt, ...)
{
	char textString[256] = { '\0' };
	va_list args;
	va_start(args, fmt);
	vsnprintf_s(textString, sizeof(textString), _TRUNCATE, fmt, args);
	va_end(args);
	return textString;
}


//------------------------------------------------------------------------
// 스트링포맷 wstring 용
//------------------------------------------------------------------------
std::wstring common::formatw(const char* fmt, ...)
{
	char textString[256] = { '\0' };
	va_list args;
	va_start(args, fmt);
	vsnprintf_s(textString, sizeof(textString), _TRUNCATE, fmt, args);
	va_end(args);
	return str2wstr(textString);
}


// 넘어온 인자 str 을 소문자로 바꿔서 리턴한다.
string& common::lowerCase(string &str)
{
	std::transform(str.begin(), str.end(), str.begin(), tolower);
	return str;
}

// 넘어온 인자 str 을 대문자로 바꿔서 리턴한다.
string& common::upperCase(string &str)
{
	std::transform(str.begin(), str.end(), str.begin(), toupper);
	return str;
}


wstring& common::lowerCasew(wstring &str)
{
	std::transform(str.begin(), str.end(), str.begin(), tolower);
	return str;
}

wstring& common::upperCasew(wstring &str)
{
	std::transform(str.begin(), str.end(), str.begin(), toupper);
	return str;
}


// 원본 문자열 str에서 구분자 delimeter 로 분해해서 out 에 저장해서 리턴한다.
// delimeter 는 저장되지 않는다.
void common::tokenizer(const string &str, const string &delimeter, const string &ignoreStr, OUT vector<string> &out)
{
	string tmp = str;
	int offset = 0;
	int first = 0;

	while (!tmp.empty())
	{
		const int pos = (int)tmp.find(delimeter, offset);
		if (string::npos == pos)
		{
			out.push_back(tmp.substr(first));
			break;
		}
		else
		{
			const string tok = tmp.substr(offset, pos - offset);
			offset += (int)tok.length() + (int)delimeter.length();
			if (tok != ignoreStr)
			{
				out.push_back(tmp.substr(first, pos - first));
				first = offset;
			}

		}
	}
}


// 원본 문자열 str에서 구분자 delimeter 로 분해해서 out 에 저장해서 리턴한다.
// delimeter 는 저장되지 않는다.
void common::wtokenizer(const wstring &str, const wstring &delimeter, const wstring &ignoreStr, OUT vector<wstring> &out)
{
	wstring tmp = str;
	int offset = 0;
	int first = 0;

	while (!tmp.empty())
	{
		const int pos = (int)tmp.find(delimeter, offset);
		if (wstring::npos == pos)
		{
			out.push_back(tmp.substr(first));
			break;
		}
		else
		{
			const wstring tok = tmp.substr(offset, pos - offset);
			offset += (int)tok.length() + (int)delimeter.length();
			if (tok != ignoreStr)
			{
				out.push_back(tmp.substr(first, pos - first));
				first = offset;
			}

		}
	}
}


// 원본 문자열 str에서 구분자 delimeters 로 분해해서 out 에 저장해서 리턴한다.
// 이 때 구분 단위는 delimeters 의 한 문자를 기준으로 한다.
// delimeters 는 저장되지 않는다.
//
// ex) tokenizer2( "aaa; bbbb cccc ddd; eee", ", ", out)
//		out-> aaa bbb ccc ddd eee
void common::tokenizer2(const string &str, const string &delimeters, OUT vector<string> &out)
{
	string tmp = str;
	int offset = 0;

	while (tmp[offset])
	{
		string tok;
		tok.reserve(32);
		while (tmp[offset])
		{
			if (strchr(delimeters.c_str(), tmp[offset]))
			{
				offset++;
				break;
			}
			tok += tmp[offset++];
		}

		if (!tok.empty())
			out.push_back(tok);
	}
}


//------------------------------------------------------------------------
// 출력창에 스트링을 출력한다.
//------------------------------------------------------------------------
void dbg::Print(const std::string &str)
{
	OutputDebugStringA(str.c_str());
	OutputDebugStringA("\n");
}


//------------------------------------------------------------------------
// 출력창에 스트링을 출력한다. (가변인자 출력)
//------------------------------------------------------------------------
void dbg::Print(const char* fmt, ...)
{
	char textString[256] = { '\0' };
	va_list args;
	va_start(args, fmt);
	vsnprintf_s(textString, sizeof(textString), _TRUNCATE, fmt, args);
	va_end(args);
	OutputDebugStringA(textString);
	//OutputDebugStringA("\n");
}


void dbg::Log(const char* fmt, ...)
{
	char textString[256];
	ZeroMemory(textString, sizeof(textString));

	va_list args;
	va_start(args, fmt);
	vsnprintf_s(textString, sizeof(textString) - 1, _TRUNCATE, fmt, args);
	va_end(args);

	std::ofstream ofs("log.txt", std::ios::app);
	if (ofs.is_open())
		ofs << textString;

	// 	FILE *fp = fopen("log.txt", "a+");
	// 	if (fp)
	// 	{
	// 		fputs(textString, fp);
	// 		fclose(fp);
	// 	}
}


// fileName 의 파일에 로그를 남긴다.
void dbg::Log2(const char *fileName, const char* fmt, ...)
{
	char textString[256] = { '\0' };
	va_list args;
	va_start(args, fmt);
	vsnprintf_s(textString, sizeof(textString), _TRUNCATE, fmt, args);
	va_end(args);

	FILE *fp = fopen(fileName, "a+");
	if (fp)
	{
		fputs(textString, fp);
		fclose(fp);
	}
}


void dbg::ErrLog(const char* fmt, ...)
{
	char textString[256] = { '\0' };
	va_list args;
	va_start(args, fmt);
	vsnprintf_s(textString, sizeof(textString), _TRUNCATE, fmt, args);
	va_end(args);

	FILE *fp = fopen("errlog.txt", "a+");
	if (fp)
	{
		fputs(textString, fp);
		fclose(fp);
	}

	// 로그파일에도 에러 메세지를 저장한다.
	Log("Error : %s", textString);
}


void dbg::RemoveErrLog()
{
	FILE *fp = fopen("errlog.txt", "w");
	if (fp)
	{
		fputs("", fp);
		fclose(fp);
	}
}


void dbg::RemoveLog2(const char *fileName)
{
	FILE *fp = fopen(fileName, "w");
	if (fp)
	{
		fputs("", fp);
		fclose(fp);
	}
}


void dbg::RemoveLog()
{
	FILE *fp = fopen("log.txt", "w");
	if (fp)
	{
		fputs("", fp);
		fclose(fp);
	}
}

