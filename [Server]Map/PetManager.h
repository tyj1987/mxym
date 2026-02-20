#pragma once
#include "..\[CC]Header\CommonStruct.h"

#define PET_SKILLCHARGE_CHECKTIME	1000
#define PET_MAX_SKILL_CHARGE	10000
#define PET_MAX_LEVEL		3
#define PET_GRADEUP_PROB_1TO2	80
#define PET_GRADEUP_PROB_2TO3	80

#define PET_RESUMMON_VALID_TIME	30000

#define CRISTMAS_EVENTPET 8
#define CRISTMAS_EVENTPET_SUMMONNING_TIME 60000*30

enum eFncOptPetTotalInfo{ eWithNULLOBJ, eWithNULL };
enum ePetUpgradeResult{eUpgradeSucess=0, eUpgradeFailforProb=1, eUpgradeFailforEtc=2, eUpgradeFailfor3rdUp=3, eUpgradeFailforSamePetSummoned=4,};
enum ePetFeedResult{ePFR_Sucess=0, ePFR_Unsummoned, ePFR_StaminaFull};

//����
enum ePetBuffKind
{ePB_None,
ePB_Demage_Percent=1,		//���� Ȯ���� ������ ����
ePB_Dodge=2,				//���� Ȯ���� ȸ��
ePB_MasterAllStatUp=3,		//����ĳ���� ���� ���
ePB_Item_DoubleChance=4,	//���� Ȯ���� ������ ȹ�� 2��
ePB_NoForeAtkMonster=5,		//���������� �񼱰� ȿ�� //�Ϲݸ�
ePB_ReduceCriticalDmg=6,	//�޴��ϰݵ���������
ePB_MasterAllStatRound=7,	//����ĳ���� ���� �ݿø�
ePB_Item_RareProbUp=8,		//����ȹ��Ȯ�� ����
ePB_MussangTimeIncrease=9,	//���ֹߵ��ð� ����
ePB_Kind_Max};
//���ȯ����
enum PetSummonning{ePSS_ReleaseSummon,ePSS_SaveSummon};

//������
enum ePetKind{ePK_None, ePK_CommonPet=1, ePK_ShopItemPet=2, ePK_EventPet=4,};

struct BuffData
{
	BuffData():Prob(0),BuffValueData(0),BuffAdditionalData(0) {};
	DWORD	Prob;
	DWORD	BuffValueData;
	DWORD	BuffAdditionalData;
};

enum ePetEquipItemIdx
{
	ePEII_FriendshipIncrease50perc = 55800,
	ePEII_StaminaReductionDecrease50perc = 55801,
};

struct PETEQUIP_ITEMOPTION
{
	PETEQUIP_ITEMOPTION():iPetStaminaReductionDecrease(0),
		iPetStaminaMaxIncreaseAmount(0),
		iPetStaminaRecoverateIncrease(0),
		iPetStaminaRecoverateAmount(0),
		iPetFriendshipIncrease(0),
		fPetFriendshipProtectionRate(0){};
	int	iPetStaminaReductionDecrease;	//�� ���¹̳� �Һ� ���ҷ�(%)
	int	iPetStaminaMaxIncreaseAmount;	//�� ���¹̳� �ִ�ġ ������(val)
	int	iPetStaminaRecoverateIncrease;	//�� ���¹̳� ȸ�� ������(%)
	int	iPetStaminaRecoverateAmount;	//�� ���¹̳� ȸ�� �߰���(val)
	int	iPetFriendshipIncrease;	//�� ģ�е� �߰� ȹ�淮(%)
	//SW070531 �� ģ�е� ��ȣ���
	float fPetFriendshipProtectionRate;	//���� ��Ȱ �� �� ģ�е� �϶� ��ȣ��(%)
};

// 前向声明（Pet.h会提供完整定义）
class CPlayer;
class CPet;

class CPetManager
{
//	static CIndexGenerator			m_PetIDGenerator;
	static DWORD					m_dwPetObjectID;

	CPlayer*						m_pPlayer;
	CPet*							m_pCurSummonPet;

	PETEQUIP_ITEMOPTION				m_PetEquipOption;

	DWORD							m_dwSkillRechargeCheckTime;
	DWORD							m_dwSkillRechargeAmount;
	BOOL							m_bSkillGuageFull;

	BOOL							m_bPetStaminaZero;

	DWORD							m_dwReleaseDelayTime;

	DWORD							m_dwResummonDelayTime;

	//SW070531 �� ģ�е� ��ȣ���
	int								m_iFriendshipReduceAmount;

	//SW061211 ũ���������̺�Ʈ
	DWORD							m_dwEventPetSummonRemainTime;	//���ε� �ð� ->��ȯ�ð� + 30Min
	DWORD							m_dwEventPetCheckTime;			//���μ��� üŷ

	WORD							m_wPetKind;

	DWORD		m_dwPetValidDistPosCheckTime;

	int								m_BuffFlag;	//�� ���� ���� ����
	BuffData						m_BuffData[ePB_Kind_Max];

	//CYHHashTable<CPet>				m_PetTable;
	CYHHashTable<PET_TOTALINFO>		m_PetInfoList;

public:
	CPetManager(void);
	virtual ~CPetManager(void);

	void		Init(CPlayer* pPlayer);
	void		Release();

	void		AddPet(CPet* pPet,DWORD	ItemDBIdx);
//	void		RemovePet(DWORD ItemDBIdx);
	void		AddPetTotalInfo(PET_TOTALINFO* pPetInfo,int flagSendMsgTo = eServerOnly);
	void		RemovePetTotalInfo(DWORD dwSummonItemDBIdx);	//�� ��ȯ��
	void		DeletePet(DWORD ItemDBIdx);	//�� ������
//	void		DeletePet(DWORD dwPetID);
	void		ReleaseCurPetMove();
	void		UpGradeSummonPet();
	BOOL		UpGradePet(DWORD dwSummonItemDBIdx, BOOL bCheckProb = TRUE);
	BOOL		RevivalPet(DWORD dwSummonItemDBIdx, int iGrade);
	void		RemovePet();

	void		AddPetInfoList(PET_TOTALINFO* pPetInfo);
	void		RemovePetInfoList(DWORD dwSummonItemDBIdx);
	WORD		GetPetInfoList(PET_TOTALINFO* RtInfo);

	CPet*		GetPet(DWORD ItemDBIdx);
	PET_TOTALINFO* GetPetTotalInfo(DWORD dwItemDBIdx, int flg = eWithNULLOBJ);
	BOOL		CheckPetDefaultFriendship(DWORD dwItemDBIdx);
	CPet*		GetPetFromSummonItemDBIdx(DWORD dwItemDBIdx);
	CPet*		GetCurSummonPet() {	return m_pCurSummonPet;	}
	void		SetCurSummonPetNull()	{	m_pCurSummonPet = NULL;	}

//	void		SummonPet(DWORD dwPetID);
	void		SummonPet(DWORD dwItemDBIdx, BOOL bSummonFromUser = TRUE);
	//SW061211 ũ���������̺�Ʈ
	BOOL		CheckBeforeSummonPet(DWORD dwItemDBIdx);
	void		SummonEventPet();
	void		SetCurSummonPetKind(WORD kind)	{ m_wPetKind = kind;	}
	BOOL		CheckCurSummonPetKindIs(int kind);
	void		CheckEventPetSummonRemainTime();

	void		SealPet();

	void		ExchangePet(DWORD dwItemDBIdx, CPlayer* pNewMaster, BOOL bReduceFriendship = TRUE );
	BOOL		IsCurPetSummonItem(DWORD dwItemDBIdx);
	void		FeedUpPet(DWORD dwFeedAmount);
	
	void		PetProcess();	//�Լ�,�����̸��� �ڼ��� �Ǵ� �����Ҽ� �ְ�!
	void		CheckPosValidDistWithMaster();

	void		CheckStaminaZero();

	void		CalcPetSkillRecharge();
	BOOL		IsSkillGuageFull()	{	return m_bSkillGuageFull;	}

	void		ReleaseMoveWithDelay(DWORD delayTime);
	void		CheckDelayRelease();

	void		NetworkMsgParse(BYTE Protocol, void* pMsg);

	void		UpdateCurPetInfo();
	void		UpdateLogoutToDB();

	void		SetSommonPetStamina(BYTE bFlag);
	int			SetSommonPetFriendship(DWORD dwFriendship);
	void		SetSommonPetSkillReady();

	void		SetPetBuffInfo();
	void		GetPetBuffResultRt(int BuffKind, void* Data);
	void		RefleshPetMaintainBuff();

	void		SetPetEquipOption(DWORD ItemIdx, BOOL bAddOpt);
	PETEQUIP_ITEMOPTION*	GetPetEquipOption()	{	return &m_PetEquipOption;	};

	void		SetPetSummonning(int flag);	//���̵��� �� ��ȯ���� DB����
	void		CheckSummonningPet();

	void		SetResummonCheckTime();
	BOOL		CheckResummonAvailable();

	void		SendPetInfo(PET_TOTALINFO* pPetInfo);

	//SW070531 �� ģ�е� ��ȣ���
	void		SetReduceAmountPetFriendship(CObject* pAttacker);
	void		ReducePetFriendshipWithMastersRevive();

	/*CPet�� �־�����.
	void		CalcStamina();
	void		CalcFriendship();
	void		CalcSkillRecharge();
	*/

};
