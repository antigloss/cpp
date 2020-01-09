/**
 *============================================================
 *  @file      BinStr.h
 *  @brief     ���ڰ���ֵת�����ַ�����ʽ�����߰��ַ�����ʽ������ת������ֵ��
 *
 *  note       ����ע������д����'\\0'������д���ˣ�����Ϊ��doxygen�����ĵ�ʱ�ܹ���ȷ��ʾ��
 *
 *============================================================
 */

#ifndef LIBCOMMON_HEX_STR_H_
#define LIBCOMMON_HEX_STR_H_

#include <cctype>
#include <cstdint>
#include <cstring>

/**
 * @brief ���ַ�first��second�ϲ���ʮ��������ֵ������first=='A'��second=='B'����char2hex�ķ���ֵΪ171����ʮ�����Ƶ�0xAB��
 *
 * @param first
 * @param second
 *
 * @return first��second�ϲ��ɵ�ʮ��������ֵ��
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
 * @brief ��ʮ��������ֵhexת�����ַ���ʽ������hex==0xCD����((char*)deststring)[0]=='C'��((char*)deststring)[1]=='D'��
 *
 * @param hex ʮ��������ֵ
 * @param deststring hexת�����ַ��󱣴浽deststring�deststringָ��Ŀռ䲻��С�������ֽڡ�
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
 * @brief ��ʮ�����Ƶ���ֵ����srchexת�����ַ�����ʽ������srchex[2]=={ 0xAB, 0xCD }��
 *        ��ת���ɹ���deststring���汣����ַ���Ϊ"ABCD"��
 *
 * @param srchex ʮ�����Ƶ���ֵ����
 * @param srclen ���鳤��
 * @param deststring srchexת�����ַ����󱣴浽deststring�deststringָ��Ŀռ䲻��С�ڣ�srclen * 2 + 1�����ֽڡ�
 *                   ת��������deststring��'\\0'��β��
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
 * @brief ��ʮ�����Ƶ���ֵ����srchexת�����ַ�����ʽ������srchex[2]=={ 0xAB, 0xCD }��
 *        ��ת���ɹ���deststring���汣����ַ���Ϊ"ABCD"�����ָ���˷ָ�������B��C֮����зָ���������
 *
 * @param srchex ʮ�����Ƶ���ֵ����
 * @param srclen ���鳤��
 * @param deststring srchexת�����ַ����󱣴浽deststring�deststringָ��Ŀռ䲻��С��
 *          ��srclen * (2 + strlen(seperator)) + 1�����ֽڡ�ת��������deststring��'\\0'��β��
 * @param seperator �ָ���������ָ���Ϊ����"|"����deststring�ﱣ��"AB|CD"��
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
 * @brief ���ַ�����ʽ��ʮ��������srcstringת������ֵ��ʽ������srcstring=="ABCD"��
 *        ��((uint8_t*)desthex)[0]==0xAB��((uint8_t*)desthex)[1]==0xCD��
 *
 * @param srcstring �ַ�����ʽ��ʮ�������������ĳ��ȱ�����ż����
 * @param srclen srcstring�ĳ��ȣ�������ż����
 * @param desthex srcstringת������ֵ��ʽ�󱣴浽desthex�desthexָ��Ŀռ䲻��С��(srclen / 2)���ֽڡ�
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
