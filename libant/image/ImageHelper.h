#ifndef LIBANT_IMAGEHELPER_H_
#define LIBANT_IMAGEHELPER_H_

#include <string>

/**
 * @brief 封装图片操作，如图片裁剪
 */
class ImageHelper {
public:
	/**
	 * @brief 裁剪图片
	 * @param dst 裁剪后的文件全路径（包括文件名）
	 * @param src 待裁剪文件全路径（包括文件名）
	 * @param xpos 裁剪起始点X坐标
	 * @param ypos 裁剪起始点Y坐标
	 * @param xposEnd 裁剪结束点X坐标
	 * @param xposEnd 裁剪结束点Y坐标
	 * @return 成功返回true，失败返回false
	 */
	static bool Crop(const std::string& dst, const std::string& src, size_t xpos, size_t ypos, size_t xposEnd, size_t yposEnd);
};

#endif // LIBANT_IMAGEHELPER_H_
