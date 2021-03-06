//////////////////////////////////////////////////////////////////////
// ZupClass.h: 用于解析完美世界公司的zup文件中的数据，并显示在List中
// 头文件
//
// 此程序由 李秋枫/stsm/liqf 编写
//
// 此代码预计将会开源，任何基于此代码的修改发布请保留原作者信息
// 
// 2012.5.23
//////////////////////////////////////////////////////////////////////

#include "PckClass.h"
#include "ZupHeader.h"
#include "DictHash.h"
#include "PckClassLog.h"

#if !defined(_ZUPCLASS_H_)
#define _ZUPCLASS_H_

class CZupClass : public CPckClass
{
public:

	//CZupClass();
	CZupClass(LPPCK_RUNTIME_PARAMS inout);
	virtual ~CZupClass();

	BOOL	Init(LPCTSTR	szFile);

	CONST	LPPCKINDEXTABLE		GetPckIndexTable();
	//CONST	LPPCK_PATH_NODE		GetPckPathNode();

	//新建pck文件
	//virtual BOOL	CreatePckFile(LPTSTR szPckFile, LPTSTR szPath) { m_PckLog.PrintLogE(TEXT_NOTSUPPORT);return FALSE; }

	//重建pck文件
	virtual BOOL	RebuildPckFile(LPCTSTR szRebuildPckFile, BOOL bUseRecompress) { m_PckLog.PrintLogE(TEXT_NOTSUPPORT);return FALSE; }

	//新建、更新pck文件
	virtual BOOL	UpdatePckFile(LPCTSTR szPckFile, const vector<tstring> &lpszFilePath, const LPPCK_PATH_NODE lpNodeToInsert) { m_PckLog.PrintLogE(TEXT_NOTSUPPORT);return FALSE; }

	//重命名文件
	virtual BOOL	RenameFilename() { m_PckLog.PrintLogE(TEXT_NOTSUPPORT);return FALSE; }

	//删除一个节点
	virtual VOID	DeleteNode(LPPCK_PATH_NODE lpNode) { m_PckLog.PrintLogE(TEXT_NOTSUPPORT);}

	//重命名一个节点
	virtual BOOL	RenameNode(LPPCK_PATH_NODE lpNode, char* lpszReplaceString){m_PckLog.PrintLogE(TEXT_NOTSUPPORT);return FALSE;}

	virtual BOOL	RenameIndex(LPPCK_PATH_NODE lpNode, char* lpszReplaceString) { m_PckLog.PrintLogE(TEXT_NOTSUPPORT); return FALSE;}
	virtual BOOL	RenameIndex(LPPCKINDEXTABLE lpIndex, char* lpszReplaceString) { m_PckLog.PrintLogE(TEXT_NOTSUPPORT); return FALSE;}

	//预览文件
	BOOL	GetSingleFileData(LPVOID lpvoidFileRead, LPPCKINDEXTABLE lpZupFileIndexTable, char *buffer, size_t sizeOfBuffer = 0);


private:

	//LPPCK_PATH_NODE				m_lpRootNodeZup;
	LPPCKINDEXTABLE				m_lpZupIndexTable;

	CDictHash					*m_lpDictHash;

private:

	void	BuildDirTree();
	BOOL	BuildZupBaseDict();
	void	DecodeFilename(char *_dst, wchar_t *_wdst, char *_src);

	_inline void	DecodeDict(LPZUP_FILENAME_DICT lpZupDict);
	VOID	AddDict(char *&lpszStringToAdd);

	LPPCKINDEXTABLE GetBaseFileIndex(LPPCKINDEXTABLE lpIndex, LPPCKINDEXTABLE lpZeroBaseIndex);

};

#endif