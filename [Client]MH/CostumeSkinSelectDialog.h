// SkinSelectDialog.h: interface for the CSkinSelectDialog class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _COSTUMESKINSELECTDLG_H
#define _COSTUMESKINSELECTDLG_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "./interface/cDialog.h"
#include "./interface/cListDialog.h"
#include "./interface/cIconDialog.h"
#include "./interface/cPushupButton.h"
#include "ItemShow.h"
#include "GameResourceStruct.h"
#include "input/Mouse.h"

class cListDialog;

class CCostumeSkinSelectDialog : public cDialog
{	
private:	
	enum TAB_BTN
	{
		eTabBtn_Hat = 0,
		eTabBtn_Dress,
		eTabBtn_Accessory,

		eTabBtn_Max,
	};

private:
	cListDialog*	m_pCostumeSkinListDlg;
	cIconDialog*	m_pCostumeSkinIconDlg;			
	cPushupButton*	m_pCostumTabBtn[eTabBtn_Max];	
	CItemShow		m_CostumeSkinView;

	DWORD m_dwSelectIdx;
	DWORD m_dwSkinDelayTime;	//������ �� �ð� ����
	BOOL m_bSkinDelayResult;	//������������ üũ�ϴ� ����

	CYHHashTable<SKIN_SELECT_ITEM_INFO>	m_CostumeSkinHat;	//�Ӹ� ���� ������
	CYHHashTable<SKIN_SELECT_ITEM_INFO>	m_CostumeSkinDress;	//�� ���� ������
	CYHHashTable<SKIN_SELECT_ITEM_INFO>	m_CostumeSkinAccessory;	//�Ǽ��縮 ���� ������
	CYHHashTable<SKIN_SELECT_ITEM_INFO>* m_pCurrentSkinTable; //���� ���� ���̺� 

public:
	CCostumeSkinSelectDialog();
	virtual ~CCostumeSkinSelectDialog();
	void Linking();
	virtual void SetActive( BOOL val );	
	virtual DWORD ActionEvent(CMouse* mouseInfo);
	BOOL OnActionEvent(LONG lId, void * p, DWORD we);
	//virtual void Render();

	void CostumeSkinKindData();	//�� ������ ��Ų ������ �����ϴ� �Լ�
	void CostumeSkinListInfo(TAB_BTN eCostumeKind);	//������ ���� ��Ų �����͸� ����Ʈ�� Add�ϴ� �Լ�
	/*
	void InitSkinDelayTime();
	void StartSkinDelayTime();
	BOOL CheckDelay();
	*/
	SKIN_SELECT_ITEM_INFO* GetCurrentSkinInfo(DWORD dwSelectIdx);
	void SetCostumTabBtnFocus(TAB_BTN eCostumeKind);
};

#endif // _COSTUMESKINSELECTDLG_H