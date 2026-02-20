// ChannelDialog.h: interface for the CChannelDialog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHANNELDIALOG_H__2799AAF3_2A8B_402D_86D4_EB2547D1888B__INCLUDED_)
#define AFX_CHANNELDIALOG_H__2799AAF3_2A8B_402D_86D4_EB2547D1888B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "./interface/cDialog.h"
#include ".\interface\cWindowHeader.h"


class cListCtrl;
class CChannelDialog  : public cDialog
{
	int m_BaseChannelIndex;
	cListCtrl * m_pChannelLCtrl;
	BOOL m_bInit;
	int m_SelectRowIdx;
	//2008. 5. 9. CBH - 移动到频道时的地图编号保存
	WORD m_wMoveMapNum;
	//2008. 5. 9. CBH - 移动到频道时的状态对象保存
	DWORD m_dwChangeMapState;

public:
	CChannelDialog();
	virtual ~CChannelDialog();

	virtual DWORD ActionEvent(CMouse * mouseInfo);
	void Linking();
	void SetChannelList(MSG_CHANNEL_INFO* pInfo);
	virtual void SetActive(BOOL val);

	void SelectChannel(int rowidx);
	void OnConnect();

	//2008. 5. 8. CBH - 新移动频道系统需要添加的函数
	void SendMapChannelInfoSYN(WORD wMapNum, DWORD dwState = eMapChange_General);	//移动到指定频道的请求函数
	void MapChange();
};

#endif // !defined(AFX_CHANNELDIALOG_H__2799AAF3_2A8B_402D_86D4_EB2547D1888B__INCLUDED_)
