/**************************************************************
 *  Filename:    .cc
 *
 *  Description: 
 *
 **************************************************************/

#include <cstring>
#include <fstream>
#include <stdexcept>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "FTPClient.h"

using namespace std;

FTPClient::FTPClient()
{
	curl_ = nullptr;

#ifdef _WIN32
	processIDStr_ = to_string(GetCurrentProcessId());
#else
	processIDStr_ = to_string(getpid());
#endif
}

int64_t FTPClient::Upload(const string& srcFile, const string& url, const string* user, const string* passwd)
{
	ifstream fin(srcFile, ifstream::binary);
	if (!fin) {
		errMsg_.clear();
		errMsg_.append("Failed to open ").append(srcFile);
		return -1;
	}

	// 获取文件大小
	fin.seekg(0, fstream::end);
	curl_off_t fileSize = fin.tellg();
	fin.seekg(0);

	if (curl_) {
		curl_easy_cleanup(curl_);
	}
	curl_ = curl_easy_init();
	if (!curl_) {
		errMsg_.clear();
		errMsg_.append("curl_easy_init failed!");
		return -1;
	}

	auto ret = curl_easy_setopt(curl_, CURLOPT_READFUNCTION, uploadHandler);
	if (ret != CURLE_OK) {
		errMsg_.clear();
		errMsg_.append(curl_easy_strerror(ret));
		return -1;
	}
	
	ret = curl_easy_setopt(curl_, CURLOPT_READDATA, &fin);
	if (ret != CURLE_OK) {
		errMsg_.clear();
		errMsg_.append(curl_easy_strerror(ret));
		return -1;
	}

	ret = curl_easy_setopt(curl_, CURLOPT_INFILESIZE_LARGE, fileSize);
	if (ret != CURLE_OK) {
		errMsg_.clear();
		errMsg_.append(curl_easy_strerror(ret));
		return -1;
	}

	ret = curl_easy_setopt(curl_, CURLOPT_UPLOAD, 1L);
	if (ret != CURLE_OK) {
		errMsg_.clear();
		errMsg_.append(curl_easy_strerror(ret));
		return -1;
	}

	ret = curl_easy_setopt(curl_, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);
	if (ret != CURLE_OK) {
		errMsg_.clear();
		errMsg_.append(curl_easy_strerror(ret));
		return -1;
	}

	ret = curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
	if (ret != CURLE_OK) {
		errMsg_.clear();
		errMsg_.append(curl_easy_strerror(ret));
		return -1;
	}

	if (user) {
		if (passwd) {
			ret = curl_easy_setopt(curl_, CURLOPT_USERPWD, (*user + ':' + *passwd).c_str());
		} else {
			ret = curl_easy_setopt(curl_, CURLOPT_USERNAME, user->c_str());
		}
	} else if (passwd) {
		ret = curl_easy_setopt(curl_, CURLOPT_PASSWORD, passwd->c_str());
	}
	if (ret != CURLE_OK) {
		errMsg_.clear();
		errMsg_.append(curl_easy_strerror(ret));
		return -1;
	}

	// 文件上传未完成时，其他程序想上传或者下载同名文件，都会失败。
	// 文件正在被下载时，其他程序可以同时下载，但是无法上传。
	// 同名文件重复上传会覆盖老文件。故此无需先传临时文件，然后再改名字。
	ret = curl_easy_perform(curl_);
	if (ret != CURLE_OK) {
		errMsg_.clear();
		errMsg_.append(curl_easy_strerror(ret));
		return -1;
	}

	return fileSize;
}

int FTPClient::Download(const std::string& destFile, const std::string& url, const std::string* user, const std::string* passwd)
{
	string escURL(url); // 把所有空格都替换成%20
	for (auto idx = escURL.find(' '); idx != string::npos; idx = escURL.find(' ', idx + 3)) {
		escURL.replace(idx, 1, "%20");
	}

	if (curl_) {
		curl_easy_cleanup(curl_);
	}
	curl_ = curl_easy_init();
	if (!curl_) {
		errMsg_.clear();
		errMsg_.append("curl_easy_init failed!");
		return -1;
	}

	auto ret = curl_easy_setopt(curl_, CURLOPT_URL, escURL.c_str());
	if (ret != CURLE_OK) {
		errMsg_.clear();
		errMsg_.append(curl_easy_strerror(ret));
		return -1;
	}

	if (user) {
		if (passwd) {
			ret = curl_easy_setopt(curl_, CURLOPT_USERPWD, (*user + ':' + *passwd).c_str());
		} else {
			ret = curl_easy_setopt(curl_, CURLOPT_USERNAME, user->c_str());
		}
	} else if (passwd) {
		ret = curl_easy_setopt(curl_, CURLOPT_PASSWORD, passwd->c_str());
	}
	if (ret != CURLE_OK) {
		errMsg_.clear();
		errMsg_.append(curl_easy_strerror(ret));
		return -1;
	}
	
	// Define our callback to get called when there's data to be written
	ret = curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, downloadHandler);
	if (ret != CURLE_OK) {
		errMsg_.clear();
		errMsg_.append(curl_easy_strerror(ret));
		return -1;
	}

	// http返回400以上的状态码则认为是错误
	ret = curl_easy_setopt(curl_, CURLOPT_FAILONERROR, 1L);
	if (ret != CURLE_OK) {
		errMsg_.clear();
		errMsg_.append(curl_easy_strerror(ret));
		return -1;
	}

	// 临时文件加入进程号，避免名字冲突
	string tmpFile = destFile;
	tmpFile.append(".").append(processIDStr_).append(".tmp");
	ofstream fout(tmpFile, ofstream::binary | ofstream::trunc);
	if (!fout) {
		errMsg_.clear();
		errMsg_.append("Failed to create ").append(destFile);
		return -1;
	}

	ret = curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &fout);
	if (ret != CURLE_OK) {
		fout.close(); // Windows下，如果不关闭文件，是无法删除的
		remove(tmpFile.c_str());

		errMsg_.clear();
		errMsg_.append(curl_easy_strerror(ret));
		return -1;
	}

	ret = curl_easy_perform(curl_);
	fout.close(); // 关闭文件，确保缓存数据写入文件
	if (ret != CURLE_OK) {
		errMsg_.clear();
		errMsg_.append(curl_easy_strerror(ret));

		remove(tmpFile.c_str());
		return (ret != CURLE_REMOTE_FILE_NOT_FOUND) ? -1 : 0;
	}

#ifdef _WIN32
	if (!MoveFileEx(tmpFile.c_str(), destFile.c_str(), MOVEFILE_REPLACE_EXISTING)) {
		errMsg_.clear();
		errMsg_.append("Download succ, rename failed! errCode=").append(to_string(GetLastError()));
#else
	if (rename(tmpFile.c_str(), destFile.c_str()) != 0) {
		errMsg_.clear();
		errMsg_.append("Download succ, rename failed! ").append(strerror(errno));
#endif
		remove(tmpFile.c_str());
		return -1;
	}

	return 1;
}

size_t FTPClient::uploadHandler(void* buffer, size_t size, size_t nmemb, void* userData)
{
	ifstream* file = reinterpret_cast<ifstream*>(userData);
	file->read(static_cast<char*>(buffer), nmemb);
	return static_cast<size_t>(file->gcount());
}

size_t FTPClient::downloadHandler(void* buffer, size_t size, size_t nmemb, void* userData)
{
	ofstream* file = reinterpret_cast<ofstream*>(userData);
	file->write(static_cast<char*>(buffer), nmemb);
	if (!file->fail()) {
		return nmemb;
	}
	return 0;
}
