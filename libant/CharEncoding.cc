#include "CharEncoding.h"

using namespace std;

#ifdef _WIN32

#include <Windows.h>

inline string convertEncoding(const char* input, int inputLen, int fromEncoding, int toEncoding)
{
	// 先获取转换后的长度
	int len = MultiByteToWideChar(fromEncoding, 0, input, inputLen, 0, 0);
	// 转换为宽字符
	wstring wcharBuf(len, 0);
	MultiByteToWideChar(fromEncoding, 0, input, inputLen, &(wcharBuf[0]), len);
	// 获取目标编码格式的字符串长度
	int len2 = WideCharToMultiByte(toEncoding, 0, wcharBuf.data(), len, 0, 0, 0, 0);
	// 转换到目标编码
	string charBuf(len2, 0);
	WideCharToMultiByte(toEncoding, 0, wcharBuf.data(), len, &(charBuf[0]), len2, 0, 0);
	return charBuf;
}

#else

inline string convertEncoding(const char* input, int inputLen, int fromEncoding, int toEncoding)
{
	return "Not yet implemented!";
}

#endif

#ifdef _WIN32

string Utf8ToGbk(const string& input)
{
	return convertEncoding(input.c_str(), static_cast<int>(input.size()), CP_UTF8, CP_ACP);
}

string Utf8ToGbk(const void* input, int inputLen)
{
	return convertEncoding(reinterpret_cast<const char*>(input), inputLen, CP_UTF8, CP_ACP);
}

string GbkToUtf8(const string& input)
{
	return convertEncoding(input.c_str(), static_cast<int>(input.size()), CP_ACP, CP_UTF8);
}

string GbkToUtf8(const void* input, int inputLen)
{
	return convertEncoding(reinterpret_cast<const char*>(input), inputLen, CP_ACP, CP_UTF8);
}

std::wstring Utf8ToUnicode(const std::string& input)
{
	return Utf8ToUnicode(input.c_str(), static_cast<int>(input.size()));
}

std::wstring Utf8ToUnicode(const void* input, int inputLen)
{
	// 转换为宽字符
	wstring wcharBuf;
	wcharBuf.resize(inputLen);
	auto len = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(input), inputLen, &(wcharBuf[0]), inputLen);
	wcharBuf.resize(len);
	return wcharBuf;
}

std::string UnicodeToUtf8(const std::wstring& input)
{
	return UnicodeToUtf8(input.c_str(), static_cast<int>(input.size()));
}

std::string UnicodeToUtf8(const wchar_t* input, int inputLen)
{
	auto outLen = inputLen * 3;
	// 转换到目标编码
	string charBuf;
	charBuf.resize(outLen);
	outLen = WideCharToMultiByte(CP_UTF8, 0, input, inputLen, &(charBuf[0]), outLen, 0, 0);
	charBuf.resize(outLen);
	return charBuf;
}

#endif