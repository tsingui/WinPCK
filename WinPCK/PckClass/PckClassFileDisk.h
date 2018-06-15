#pragma once

typedef unsigned __int64	QWORD;
#include "PckStructs.h"
#include "PckClassLog.h"
#include "MapViewFile.h"


class CPckClassFileDisk
{
public:
	CPckClassFileDisk();
	~CPckClassFileDisk();

protected:
	BOOL		EnumAllFilesByPathList(const vector<tstring> &lpszFilePath, DWORD &_out_FileCount, QWORD &_out_TotalFileSize, vector<FILES_TO_COMPRESS> *lpFileLinkList);
	//压缩重建、压缩写入空间不够时扩展空间
	BOOL		IsNeedExpandWritingFile(
		CMapViewFileWrite *lpWrite,
		QWORD dwWritingAddressPointer,
		QWORD dwFileSizeToWrite,
		QWORD &dwExpectedTotalCompressedFileSize);

	//重建pck时需要的磁盘空间（目标文件名，待重建的PCK文件大小）
	QWORD	GetPckFilesizeRebuild(LPCTSTR lpszFilename, QWORD qwPckFilesize);

	//重命名时需要的文件的大小
	QWORD	GetPckFilesizeRename(LPCTSTR lpszFilename, QWORD qwCurrentPckFilesize);

	//添加和新建文件时，写入的pck文件大小
	QWORD	GetPckFilesizeByCompressed(LPCSTR lpszFilename, QWORD qwToCompressFilesize, QWORD qwCurrentPckFilesize);
	QWORD	GetPckFilesizeByCompressed(LPCWSTR lpszFilename, QWORD qwToCompressFilesize, QWORD qwCurrentPckFilesize);

private:

	//qwCurrentPckFilesize为已经存在的文件大小，qwToAddSpace是需要扩大的大小，返回值为（qwCurrentPckFilesize + 可以再扩大的最大大小）
	QWORD	GetPckFilesizeByCompressed(QWORD qwDiskFreeSpace, QWORD qwToAddSpace, QWORD qwCurrentPckFilesize);
	//遍历文件夹
	VOID	EnumFile(LPSTR szFilename, BOOL IsPatition, DWORD &dwFileCount, vector<FILES_TO_COMPRESS> *lpFileLinkList, QWORD &qwTotalFileSize, size_t nLen);

};

