// OptionManager.h: interface for the COptionManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPTIONMANAGER_H__16984CD0_B39F_40C4_AA38_3BCF9894A020__INCLUDED_)
#define AFX_OPTIONMANAGER_H__16984CD0_B39F_40C4_AA38_3BCF9894A020__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define OPTIONMGR USINGTON(COptionManager)

class COptionDialog;
class cGuageBar;

struct sGAMEOPTION
{
	BOOL bNoDeal;	//...
	BOOL bNoParty;	//...
	BOOL bNoFriend;	//...
	BOOL bNoVimu;	//...

	BOOL bNameMunpa;
	BOOL bNameParty;
	BOOL bNameOthers;
	BOOL bNoMemberDamage;
	BOOL bNoGameTip;
	BOOL bMunpaIntro;	// magi82 - �α��νø� ���ļҰ�â �ɼ����� 070108

	int nMacroMode;

	BOOL bNoWhisper;	//...
	BOOL bNoChatting;
	BOOL bNoBalloon;
	BOOL bAutoHide;
	BOOL bNoShoutChat;
	BOOL bNoGuildChat;
	BOOL bNoAllianceChat;
	BOOL bNoSystemMsg;
	BOOL bNoExpMsg;
	BOOL bNoItemMsg;

	int nGamma;
	int nSightDistance;

	BOOL bGraphicCursor;		//

	BOOL bShadowHero;
	BOOL bShadowMonster;
	BOOL bShadowOthers;
	//SW050822
#ifndef _JAPAN_LOCAL_	

	BOOL bAutoCtrl;
	int nLODMode;
	int nEffectMode;
	int nEffectSnow; // 2005.12.28�� ��On/Off �߰� 

#endif

	BOOL bSoundBGM;				//
	BOOL bSoundEnvironment;		//
	int	nVolumnBGM;				//
	int nVolumnEnvironment;		//

	//SW060904 ĳ��ȯ��ݻ簪 ���� �ɼ� �߰�
	BOOL bAmbientMax;

//	WORD wKyungGongIdx;
//	BOOL bIsKyungGongMode;
	
//	BOOL bShadowMap;
//	int nShadowDetail;
//	BOOL bHideGXObject;
//	int nInterfaceAlpha;
//	BOOL bNameHero;
//	BOOL bNameMonster;
//	BOOL bCameraAction;
//	BOOL bSoundInterface;
//	BOOL bSoundAttack;
//	BOOL bSoundCharater;
//	BOOL bSoundMonster;

	// 登录难度设置 - 0:普通 1:极难
	int nLoginCombo;

	BOOL bIntroFlag;	// magi82 - Intro(070713)
	BOOL bNoAvatarView;
};


class COptionManager  
{
protected:

	sGAMEOPTION		m_GameOption;

	BOOL			m_bBGMSoundChanged;
	WORD			m_wSendOption;

	//SW050804 �ɼ�â: ���� ������ ���� �߰�
	BOOL			m_bShowMenberDamage;

	//SW050825 �׷��ȿɼ�Tab: GraphicAutoCtrlProcess()
	COptionDialog*	m_pOptionDlg;
	cGuageBar*		m_pCammaGB;		//���� ������ �ȵ�
	cGuageBar*		m_pSightGB;
	LONG			m_SigntGBMinValue;
	LONG			m_SigntGBMaxValue;
	DWORD			m_dwShdowOption;
	enum {LOWEST=1,LOW,MID,HIGH,STANDARD_FRAME=15,NICE_FRAME=20};
	DWORD			m_dwLastCheckTime;
	DWORD			m_dwCheck;
	DWORD			m_dwFrameAverage;
	DWORD			m_dwCountFrame;
	int				m_nCurOptionState;
	
protected:

	BOOL LoadGameOption();
	WORD MakeOptionFlag();

public:

	//MAKESINGLETON(COptionManager);
	
	COptionManager();
	virtual ~COptionManager();

	void Init();

	void SaveGameOption();
	void SetDefaultOption();
	void ApplySettings();
	void CancelSettings();

	void SendOptionMsg();

	sGAMEOPTION* GetGameOption() { return &m_GameOption; }
	void SetGameOption( sGAMEOPTION* pOption );
	
//	void SetKyungGongIdx(WORD KyungGondIdx);
//	WORD GetKyungGongIdx()	{ return m_GameOption.wKyungGongIdx;	}
//	BOOL GetKyungGongMode();
//	void SetKyungGongMode(BOOL bMode);
//	void ClrKyungGong();

//parsing
	void NetworkMsgParse( BYTE Protocol, void* pMsg );

	void SetMemberDemageActive(BOOL bDo) {m_bShowMenberDamage = bDo;}
	BOOL IsShowMemberDamage() {return m_bShowMenberDamage;}	

	void GraphicAutoCtrlProcess();	
	void ApplyGraphicSetting();
	void InitForGameIn();

};


EXTERNGLOBALTON(COptionManager);

#endif // !defined(AFX_OPTIONMANAGER_H__16984CD0_B39F_40C4_AA38_3BCF9894A020__INCLUDED_)

