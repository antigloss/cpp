#include <ctime>
#include <fstream>
#include <vector>
#include <openssl/md5.h>
#include <boost/filesystem.hpp>

#ifndef _WIN32
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

#include "BinStr.h"
#include "FileUtils.h"

using namespace std;
namespace fs = boost::filesystem;

bool IsSameFileUpdate(const std::string& filePath, FileInfo& fileInfo)
{
	boost::system::error_code ec;

	fs::path fp(filePath);
	int64_t fileSz = fs::file_size(fp, ec);
	if (ec) {
		return false;
	}

	auto lastModTime = fs::last_write_time(fp, ec);
	if (ec) {
		return false;
	}

	if (fileInfo.FileSize == fileSz && fileInfo.LastModifiedTime == lastModTime) {
		return true;
	}

	fileInfo.FileSize = fileSz;
	fileInfo.LastModifiedTime = lastModTime;
	if (fileInfo.FileSize <= 0) {
		fileInfo.CheckSum.clear();
		return false;
	}
	
	ifstream fin(filePath, ifstream::binary);
	if (!fin) {
		fileInfo.CheckSum.clear();
		return false;
	}

	vector<char> fileBuf(fileInfo.FileSize);
	fin.read(fileBuf.data(), fileBuf.size());

	unsigned char md5buf[MD5_DIGEST_LENGTH];
	MD5(reinterpret_cast<unsigned char*>(fileBuf.data()), fileBuf.size(), md5buf);

	char md5hexbuf[MD5_DIGEST_LENGTH * 2 + 1];
	BytesToHexStr(md5buf, MD5_DIGEST_LENGTH, md5hexbuf);

	string md5hex(md5hexbuf);
	if (fileInfo.CheckSum == md5hex) {
		return true;
	}

	fileInfo.CheckSum.swap(md5hex);
	return false;
}
