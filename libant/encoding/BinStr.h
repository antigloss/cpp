/**
 *============================================================
 *  @file      BinStr.h
 *  @brief     用于把数值转换成字符串形式，或者把字符串形式的数字转换成数值。
 *
 *  note       代码注释里面写的是'\\0'并不是写错了，而是为了doxygen生成文档时能够正确显示。
 *
 *============================================================
 */

#ifndef LIBCOMMON_HEX_STR_H_
#define LIBCOMMON_HEX_STR_H_

#include <cctype>
#include <cstdint>
#include <cstring>

/**
 * @brief 把字符first和second合并成十六进制数值。假设first=='A'，second=='B'，则char2hex的返回值为171，即十六进制的0xAB。
 *
 * @param first
 * @param second
 *
 * @return first和second合并成的十六进制数值。
 */
inline uint8_t HexCharToByte(char first, char second)
{
	first = toupper(first);
	second = toupper(second);

	uint8_t digit = (first >= 'A' ? (first - 'A') + 10 : (first - '0')) * 16;
	digit += (second >= 'A' ? (second - 'A') + 10 : (second - '0'));

	return digit;
}

/**
 * @brief 把十六进制数值hex转换成字符形式。假设hex==0xCD，则((char*)deststring)[0]=='C'，((char*)deststring)[1]=='D'。
 *
 * @param hex 十六进制数值
 * @param deststring hex转换成字符后保存到deststring里。deststring指向的空间不能小于两个字节。
 *
 * @return void
 */
inline void ByteToHexStr(uint8_t hex, void* deststring)
{
	static const char hex_map[] = "0123456789ABCDEF";

	char* dest = (char*)deststring;

	dest[0] = hex_map[hex / 16];
	dest[1] = hex_map[hex % 16];
}

/**
 * @brief 把十六进制的数值数组srchex转换成字符串形式。假设srchex[2]=={ 0xAB, 0xCD }，
 *        则转换成功后deststring里面保存的字符串为"ABCD"。
 *
 * @param srchex 十六进制的数值数组
 * @param srclen 数组长度
 * @param deststring srchex转换成字符串后保存到deststring里。deststring指向的空间不能小于（srclen * 2 + 1）个字节。
 *                   转换结束后，deststring以'\\0'结尾。
 *
 * @return void
 */
inline void BytesToHexStr(const void* srchex, size_t srclen, void* deststring)
{
	const uint8_t* hex = (const uint8_t*)srchex;
	char* dest = (char*)deststring;

	size_t i;
	for (i = 0; i != srclen; ++i) {
		ByteToHexStr(hex[i], &dest[i * 2]);
	}
	dest[srclen * 2] = '\0';
}

/**
 * @brief 把十六进制的数值数组srchex转换成字符串形式。假设srchex[2]=={ 0xAB, 0xCD }，
 *        则转换成功后deststring里面保存的字符串为"ABCD"。如果指定了分隔符，则B和C之间会有分隔符隔开。
 *
 * @param srchex 十六进制的数值数组
 * @param srclen 数组长度
 * @param deststring srchex转换成字符串后保存到deststring里。deststring指向的空间不能小于
 *          （srclen * (2 + strlen(seperator)) + 1）个字节。转换结束后，deststring以'\\0'结尾。
 * @param seperator 分隔符。假设分隔符为竖线"|"，则deststring里保存"AB|CD"。
 *
 * @return void
 */
inline void BytesToHexStrWithSeperator(const void* srchex, size_t srclen, void* deststring, const void* seperator)
{
	const uint8_t* hex = (const uint8_t*)srchex;
	char* dest = reinterpret_cast<char*>(deststring);
	const char* sep = reinterpret_cast<const char*>(seperator);

	size_t incr = strlen(sep) + 2;
	size_t i;
	for (i = 0; i != srclen; ++i) {
		ByteToHexStr(hex[i], &dest[i * incr]);
		if (i != (srclen - 1)) {
			strcpy(&dest[i * incr + 2], sep);
		}
	}
	dest[srclen * incr] = '\0';
}

/**
 * @brief 把字符串形式的十六进制数srcstring转换成数值形式。假设srcstring=="ABCD"，
 *        则((uint8_t*)desthex)[0]==0xAB，((uint8_t*)desthex)[1]==0xCD。
 *
 * @param srcstring 字符串形式的十六进制数。它的长度必须是偶数。
 * @param srclen srcstring的长度，必须是偶数。
 * @param desthex srcstring转换成数值形式后保存到desthex里。desthex指向的空间不能小于(srclen / 2)个字节。
 *
 * @return void
 */
inline void HexStrToBytes(const void* srcstring, size_t srclen, void* desthex)
{
	const char* src = (const char*)srcstring;
	uint8_t* dest = (uint8_t*)desthex;

	size_t i;
	for (i = 0; i != srclen; i += 2) {
		dest[i / 2] = HexCharToByte(src[i], src[i + 1]);
	}
}

#endif // end of LIBCOMMON_HEX_STR_H_
