#ifndef LIBANT_CHAR_ENCODING_H_
#define LIBANT_CHAR_ENCODING_H_

#include <string>

// UTF8和GBK相互转换
std::string Utf8ToGbk(const std::string& input);
std::string Utf8ToGbk(const void* input, int inputLen);
std::string GbkToUtf8(const std::string& input);
std::string GbkToUtf8(const void* input, int inputLen);
// UTF8和Unicode相互转换
std::wstring Utf8ToUnicode(const std::string& input);
std::wstring Utf8ToUnicode(const void* input, int inputLen);
std::string UnicodeToUtf8(const std::wstring& input);
std::string UnicodeToUtf8(const wchar_t* input, int inputLen);

#endif // LIBANT_CHAR_ENCODING_H_
