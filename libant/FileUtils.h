#ifndef LIBANT_FILEUTILS_H_
#define LIBANT_FILEUTILS_H_

#include <cstdint>
#include <string>

struct FileInfo {
	FileInfo()
	{
		FileSize = 0;
		LastModifiedTime = 0;
	}

	void Swap(FileInfo& info)
	{
		std::swap(FileSize, info.FileSize);
		std::swap(LastModifiedTime, info.LastModifiedTime);
		CheckSum.swap(info.CheckSum);
	}

	int64_t		FileSize;
	time_t		LastModifiedTime;
	std::string	CheckSum;
};

// 检查filePath和fileInfo是否同一个文件，相同返回true，不同返回false，并把fileInfo更新成filePath的信息
bool IsSameFileUpdate(const std::string& filePath, FileInfo& fileInfo);

#endif // LIBANT_FILEUTILS_H_
