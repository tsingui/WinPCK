//////////////////////////////////////////////////////////////////////
// PckControlCenter.cpp: 用于解析完美世界公司的pck文件中的数据，并显示在List中
// 头文件,界面与PCK类的数据交互，控制中心
//
// 此程序由 李秋枫/stsm/liqf 编写，pck结构参考若水的pck结构.txt，并
// 参考了其易语言代码中并于读pck文件列表的部分
//
// 此代码预计将会开源，任何基于此代码的修改发布请保留原作者信息
// 
// 2012.10.10
//////////////////////////////////////////////////////////////////////

#include "PckControlCenter.h"
#include "PckClass.h"
	

CPckControlCenter::CPckControlCenter():
	m_lpClassPck(NULL),
	m_isSearchMode(FALSE),
	m_lpPckRootNode(NULL),
	m_RestoreInfomation({ 0 }),
	m_emunFileFormat(FMTPCK_UNKNOWN),
	cParams({ 0 })
{
	init();
	Reset();
}

CPckControlCenter::~CPckControlCenter()
{
	DeleteRestoreData();
}

void CPckControlCenter::setMainWnd(HWND _hWnd)
{
	m_hWndMain = _hWnd;
}

void CPckControlCenter::init()
{
	cParams.lpPckControlCenter = this;
}

void CPckControlCenter::Reset(DWORD dwUIProgressUpper)
{
	memset(&cParams.cVarParams, 0, sizeof(PCK_VARIETY_PARAMS));
	cParams.cVarParams.dwUIProgressUpper = dwUIProgressUpper;
}