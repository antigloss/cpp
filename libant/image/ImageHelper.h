#ifndef LIBANT_IMAGEHELPER_H_
#define LIBANT_IMAGEHELPER_H_

#include <string>

/**
 * @brief ��װͼƬ��������ͼƬ�ü�
 */
class ImageHelper {
public:
	/**
	 * @brief �ü�ͼƬ
	 * @param dst �ü�����ļ�ȫ·���������ļ�����
	 * @param src ���ü��ļ�ȫ·���������ļ�����
	 * @param xpos �ü���ʼ��X����
	 * @param ypos �ü���ʼ��Y����
	 * @param xposEnd �ü�������X����
	 * @param xposEnd �ü�������Y����
	 * @return �ɹ�����true��ʧ�ܷ���false
	 */
	static bool Crop(const std::string& dst, const std::string& src, size_t xpos, size_t ypos, size_t xposEnd, size_t yposEnd);
};

#endif // LIBANT_IMAGEHELPER_H_
