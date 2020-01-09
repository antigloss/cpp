/**************************************************************
 *  Filename:    FTPClient.h
 *
 *  Description: 封装FTP操作
 *
 **************************************************************/

#ifndef LIBANT_FTPCLIENT_H_
#define LIBANT_FTPCLIENT_H_

#include <string>
#include <curl/curl.h>

/**
 * @brief 封装FTP操作
 */
class FTPClient {
public:
	FTPClient();

	~FTPClient()
	{
		if (curl_) {
			curl_easy_cleanup(curl_);
		}
	}

	/**
	 * @brief 上传文件
	 * @param srcFile 需要上传的文件
	 * @param url 文件上传到这个路径下
	 * @param user 用户名
	 * @param passwd 密码
	 * @return 返回上传文件的大小，小于0表示失败
	 */
	int64_t Upload(const std::string& srcFile, const std::string& url, const std::string* user = nullptr, const std::string* passwd = nullptr);
	/**
	 * @brief 下载文件
	 * @param destFile 文件保存路径
	 * @param url 文件在服务器上的地址
	 * @param user 用户名
	 * @param passwd 密码
	 * @return 1成功，0文件不存在，-1失败
	 */
	int Download(const std::string& destFile, const std::string& url, const std::string* user = nullptr, const std::string* passwd = nullptr);

	const std::string& LastErrMsg() const
	{
		return errMsg_;
	}

private:
	static size_t uploadHandler(void* buffer, size_t size, size_t nmemb, void* userData);
	static size_t downloadHandler(void* buffer, size_t size, size_t nmemb, void* userData);

private:
	CURL*		curl_;
	std::string	processIDStr_;
	std::string	errMsg_;
};

#endif // LIBANT_FTPCLIENT_H_
