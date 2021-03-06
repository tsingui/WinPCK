//////////////////////////////////////////////////////////////////////
// PckClassRebuildFilter.cpp: 用于解析过滤脚本
//
// 此程序由 李秋枫/stsm/liqf 编写，部分代码改写自RapidCRC
//
// 此代码开源，任何基于此代码的修改发布请保留原作者信息
// 
// 2018.5.15
//////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include "CharsCodeConv.h"
#include "PckClassWriteOperator.h"
//chkfile
//protect
//delete
//rmdir

#define MAX_LINE_LENGTH (MAX_PATH + 10)
#define MAX_SCRIPT_SIZE	(10*1024*1024)

void CPckClassWriteOperator::SetBufPointer(SCRIPTBUFFER *sfvBuf, int pos)
{
	sfvBuf->dwPos += pos;
	sfvBuf->bufpos += pos;
}

BOOL CPckClassWriteOperator::ReadFromBuffer(void *_dstbuf, size_t size, SCRIPTBUFFER *sfvBuf)
{
	if((sfvBuf->dwPos + size) > sfvBuf->dwSize)
		return FALSE;

	memcpy(_dstbuf, sfvBuf->bufpos, size);
	SetBufPointer(sfvBuf, size);
	return TRUE;
}

VOID CPckClassWriteOperator::GetNextLine(SCRIPTBUFFER *sfvBuf, CHAR * szLineAnsi, CONST UINT uiLengthLine,
	UINT * puiStringLength, BOOL * pbErrorOccured, BOOL * pbEndOfFile)
{
	CHAR myChar;

	UINT uiCount;
	BOOL bSuccess;

	// 至少需要一个字节存放 \0
	if(uiLengthLine <= 1) {
		(*pbErrorOccured) = TRUE;
		return;
	}

	ZeroMemory(szLineAnsi, uiLengthLine);
	uiCount = 0;

	while(TRUE) {
		//在下次循环中写入szLineAnsi[uiCount]并在 szLineAnsi[uiCount+1] 中写入\0
		if(uiCount >= uiLengthLine - 1) {
			(*pbErrorOccured) = TRUE;
			return;
		}
		bSuccess = ReadFromBuffer(&myChar, sizeof(myChar), sfvBuf);

		if(!bSuccess) {
			(*pbErrorOccured) = FALSE;
			(*puiStringLength) = uiCount;
			(*pbEndOfFile) = TRUE;
			return;
		}

		if((myChar == '\n') || (myChar == '\r')) {
			do {
				bSuccess = ReadFromBuffer(&myChar, sizeof(myChar), sfvBuf);

			} while((bSuccess) && ((myChar == '\n') || (myChar == '\r')));

			if(bSuccess && !((myChar == '\n') || (myChar == '\r')))
				SetBufPointer(sfvBuf, -1);

			(*pbErrorOccured) = FALSE;
			(*puiStringLength) = uiCount;
			(*pbEndOfFile) = FALSE;
			return;
		} else {
			szLineAnsi[uiCount] = myChar;
			uiCount += 1;
		}
	}
	return;
}

BOOL CPckClassWriteOperator::ParseOneLine(FILEOP * pFileOp, char * lpszLine, TCHAR *lpszFileName)
{

	char szOperator[16] = { 0 };
	//首先检查16个字符内有没有空格或tab
	char *lpszCell = lpszLine, *lpszSearch = lpszLine;
	size_t count = 0;

	char *lpszOperator = szOperator;
	BOOL isValid = FALSE;

	while((16 > (++count)) &&
		(*lpszSearch)) {

		if((' ' == *lpszSearch) || ('\t' == *lpszSearch)) {
			isValid = TRUE;
			break;
		}
		*lpszOperator++ = *lpszSearch++;
	}

	if(!isValid)
		return FALSE;

	//解析szOperator
	const char *lpszOpPos = strstr(szOperators, szOperator);
	if(NULL == lpszOpPos)
		return FALSE;

	pFileOp->op = SCRIPTOP((lpszOpPos - szOperators) / 8);

	//过滤掉剩下的空格和tab
	while((' ' == *lpszSearch) || ('\t' == *lpszSearch))
		lpszSearch++;

	isValid = FALSE;
	count = 0;

	if((MAX_PATH <= strlen(lpszSearch)) || (0 == *lpszSearch))
		return FALSE;

	CAnsi2Ucs cA2U;
	cA2U.GetString(lpszSearch, pFileOp->szFilename, sizeof(pFileOp->szFilename) / sizeof(TCHAR));
	wcscpy(pFileOp->szFilenameBuffer, pFileOp->szFilename);

	//检查文件名是否正确
	if(OP_CheckFile == pFileOp->op) {
#ifdef UNICODE
		if(0 == _tcsicmp(lpszFileName, pFileOp->szFilename))
#else
		CAnsi2Ucs cA2U;
		if(0 == wcsicmp(cA2U.GetString(lpszFileName), pFileOp->szFilename))
#endif
			return TRUE;
		else
			return FALSE;
	}

	return TRUE;
}

//分解脚本中的目录
void CPckClassWriteOperator::SepratePaths(FILEOP * pFileOp)
{
	pFileOp->lpszSepratedPaths[0] = pFileOp->szFilenameBuffer;

	wchar_t *lpszSearch = pFileOp->szFilenameBuffer;
	int nPathDepthCount = 1;


	while(*lpszSearch) {

		wchar_t *test = wcschr(lpszSearch, L'\\');
		//这里没有考虑存在\\双斜杠的情况
		if((L'\\' == *lpszSearch) || (L'/' == *lpszSearch)) {
			*lpszSearch = 0;
			++lpszSearch;
			pFileOp->lpszSepratedPaths[nPathDepthCount] = lpszSearch;
			++nPathDepthCount;
		}
		++lpszSearch;
	}

}

LPPCK_PATH_NODE CPckClassWriteOperator::LocationFileIndex(LPWSTR *lpszPaths, LPPCK_PATH_NODE lpNode)
{
	LPCWSTR lpszSearchDir = *lpszPaths;

	if((NULL == lpszSearchDir) || (NULL == lpNode))
		return NULL;

	//直接跳过..目录
	LPPCK_PATH_NODE lpNodeSearch = lpNode->next;

	while(1) {

		if(NULL == lpNodeSearch)
			return NULL;

		if(0 == wcsicmp(lpszSearchDir, lpNodeSearch->szName)) {

			//是否已经匹配完
			if(NULL == *(lpszPaths + 1)) {

				return lpNodeSearch;
			} else {
				return LocationFileIndex(lpszPaths + 1, lpNodeSearch->child);
			}
		}
		lpNodeSearch = lpNodeSearch->next;
	}

	return NULL;
}

void CPckClassWriteOperator::MarkFilterFlagToFileIndex(LPPCKINDEXTABLE	lpPckIndexTable, SCRIPTOP op)
{
	switch(op) {

	case OP_Protect:
		lpPckIndexTable->isProtected = TRUE;
		break;

	case OP_Delete:
	case OP_Rmdir:
		if((!lpPckIndexTable->isInvalid) && (!lpPckIndexTable->isProtected)) {

			lpPckIndexTable->isToDeDelete = TRUE;
			lpPckIndexTable->isInvalid = TRUE;
		}
		break;
	}
}

//将一行脚本内容应用到查找到的文件列表中
void CPckClassWriteOperator::MarkFilterFlagToNode(LPPCK_PATH_NODE lpNode, SCRIPTOP op)
{
	lpNode = lpNode->child->next;

	while(NULL != lpNode) {
		if(NULL == lpNode->child) {
			MarkFilterFlagToFileIndex(lpNode->lpPckIndexTable, op);
		} else {
			MarkFilterFlagToNode(lpNode, op);
		}

		lpNode = lpNode->next;
	}

}


#pragma region ApplyScript2IndexList,将脚本内容应用到文件列表中

//将脚本内容应用到文件列表中
BOOL CPckClassWriteOperator::ApplyScript2IndexList(VOID *pfirstFileOp, PCK_ALL_INFOS _DstPckAllInfo)
{

	vector<FILEOP> * lpfirstFileOp = (vector<FILEOP> *)pfirstFileOp;
	//FILEOP *m_firstFileOp = (FILEOP *)pfirstFileOp;
	//FILEOP * pFileOp = m_firstFileOp;

	//解析过程是否发生了错误
	BOOL bHasErrorHappend = FALSE;

	size_t nListCount = lpfirstFileOp->size();

	//while(NULL != pFileOp) {
	for(size_t i = 0;i<nListCount;i++){

		FILEOP * pFileOp = &(*lpfirstFileOp)[i];

		if(OP_CheckFile != pFileOp->op) {

			//分解脚本中的目录
			SepratePaths(pFileOp);

			//定位文件索引
			LPPCK_PATH_NODE lpFoundNode = LocationFileIndex(pFileOp->lpszSepratedPaths, _DstPckAllInfo.cRootNode.child);
			if(NULL == lpFoundNode) {

				m_PckLog.PrintLogW(TEXT("已解析脚本失败在: %s, 跳过..."), pFileOp->szFilename);
				bHasErrorHappend = TRUE;

			} else {
				if(NULL != lpFoundNode->child) {

					MarkFilterFlagToNode(lpFoundNode, pFileOp->op);
				} else {
					MarkFilterFlagToFileIndex(lpFoundNode->lpPckIndexTable, pFileOp->op);
				}
			}

		}
		//pFileOp = pFileOp->next;
	}

	return (!bHasErrorHappend);
}

#pragma endregion

BOOL CPckClassWriteOperator::ParseScript(LPCTSTR lpszScriptFile, PCK_ALL_INFOS &_DstPckAllInfo)
{

	CHAR	szLineAnsi[MAX_LINE_LENGTH];
	char	szLine[MAX_LINE_LENGTH];
	UINT	uiStringLength;
	BOOL	bErrorOccured, bEndOfFile;
	SCRIPTBUFFER sfvBuf = { 0 };
	FILEOP * pFileOp;

	BYTE   * lpBufferToRead;
	CMapViewFileRead	cFileRead;
	//LinkList<FILEOP> m_firstFileOp;
	vector<FILEOP> cFirstFileOp(1);

	m_PckLog.PrintLogI("开始解析脚本...");

	//读取文件所有字符
	if(NULL == (lpBufferToRead = cFileRead.OpenMappingViewAllRead(lpszScriptFile))) {

		return FALSE;
	}

	sfvBuf.dwSize = (DWORD)cFileRead.GetFileSize();
	//如果大于10MB，退出
	if(MAX_SCRIPT_SIZE < sfvBuf.dwSize)
		return FALSE;

	if(NULL == (sfvBuf.buffer = (BYTE*)malloc(sfvBuf.dwSize)))
		return FALSE;

	memcpy(sfvBuf.buffer, lpBufferToRead, sfvBuf.dwSize);

	//m_firstFileOp.insertNext();
	pFileOp = &cFirstFileOp.back();

	//处理读取的内容
	sfvBuf.bufpos = sfvBuf.buffer;
	GetNextLine(&sfvBuf, szLineAnsi, MAX_LINE_LENGTH, &uiStringLength, &bErrorOccured, &bEndOfFile);

	if(bErrorOccured) {
		m_PckLog.PrintLogEL("无法读取 SFV 文件", __FILE__, __FUNCTION__, __LINE__);
		free(sfvBuf.buffer);
		return FALSE;
	}

	//记录已经读取到第多少行了
	int iReadLineNumber = 1;
	//解析过程是否发生了错误
	BOOL bHasErrorHappend = FALSE;

	while(!(bEndOfFile && uiStringLength == 0)) {

		if(uiStringLength > 5) {
			strcpy_s(szLine, MAX_LINE_LENGTH, szLineAnsi);

			//删除尾部多余的空格
			while((szLine[uiStringLength - 1] == TEXT(' ')) && (uiStringLength > 8)) {
				szLine[uiStringLength - 1] = NULL;
				uiStringLength--;
			}

			//排队注释行和空行
			if((szLine[0] != TEXT(';')) && (szLine[0] != TEXT('\0'))) {

				//一行脚本分为两部分，操作和文件名
				if(ParseOneLine(pFileOp, szLine, _DstPckAllInfo.szFileTitle)) {

					cFirstFileOp.push_back(FILEOP{ 0 });
					pFileOp = &cFirstFileOp.back();
					//m_firstFileOp.insertNext();
					//pFileOp = m_firstFileOp.last();

				} else {

					m_PckLog.PrintLogW("脚本解析失败在行%d: %s, 跳过...", iReadLineNumber, szLine);
					bHasErrorHappend = TRUE;
				}
			}
		}

		GetNextLine(&sfvBuf, szLineAnsi, MAX_LINE_LENGTH, &uiStringLength, &bErrorOccured, &bEndOfFile);
		if(bErrorOccured) {
			m_PckLog.PrintLogEL("无法读取 SFV 文件", __FILE__, __FUNCTION__, __LINE__);
			free(sfvBuf.buffer);
			return FALSE;
		}

		++iReadLineNumber;

	}
	free(sfvBuf.buffer);


	BOOL rtn = FALSE;

	//将数据应用于tree中
	if(!bHasErrorHappend)
		rtn = ApplyScript2IndexList(&cFirstFileOp, _DstPckAllInfo);

	if(!rtn) {
		ResetRebuildFilterInIndexList(_DstPckAllInfo);
		m_PckLog.PrintLogI("解析脚本失败");
	} else {
		m_PckLog.PrintLogI("解析脚本成功");
	}

	return rtn;
}


//清除掉重建包时所需要读取的过滤信息
void CPckClassWriteOperator::ResetRebuildFilterInIndexList(PCK_ALL_INFOS &_DstPckAllInfo)
{
	LPPCKINDEXTABLE lpPckIndexTable = _DstPckAllInfo.lpPckIndexTable;

	for(DWORD i = 0;i < _DstPckAllInfo.dwFileCount;++i) {

		if(lpPckIndexTable->isToDeDelete) {
			lpPckIndexTable->isInvalid = FALSE;
		}
		lpPckIndexTable->isProtected = lpPckIndexTable->isToDeDelete = FALSE;
		++lpPckIndexTable;
	}
}

BOOL CPckClassWriteOperator::ParseScript(LPCTSTR lpszScriptFile)
{
	return ParseScript(lpszScriptFile, m_PckAllInfo);
}