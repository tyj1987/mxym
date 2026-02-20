#ifndef __GAMERESOURCESTRUCT_H__
#define __GAMERESOURCESTRUCT_H__

#include "CommonDefine.h"
#include "CommonGameDefine.h"
#include "CommonGameStruct.h"
#include "../4DyuchiGRX_common/typedef.h"

class ITEM_MIX_INFO;
class ITEM_MIX_RES;
class ITEM_MIX_MATERIAL;

class CSkillInfo;

//#define MAX_ITEMNAME_LENGTH	30

//-----------------------------------------------------------------------
//	������ �⺻ ����Ʈ ��ġ�� �������� ������ ����ġ
//-----------------------------------------------------------------------
struct CharBasicStats
{
	WORD	wGenGol;						// �ٰ�
	WORD	wMinChub;							// ��ø
	WORD	wCheRyuk;							// ü��
	WORD	wSimMek;						// �ɸ�
	//���ο� ��ȹ �������� ������
	//BYTE	BonusPoint;						// ���ʽ� ����Ʈ general-15
};
class RECALLINFO
{
public:
	DWORD	RecallType;
	DWORD	RecallID;
	DWORD	RecallMonsterNum;
	DWORD	RecallRange;
};
class ATTACKPATTERNINFO
{
public:
	WORD	AttackPatternNum;
	WORD	* AttackPattern;		// 0->1->2->2->1->....
};
class CAttackPatternInfo
{
public:
	ATTACKPATTERNINFO * pAttackPatternInfo;
	virtual ~CAttackPatternInfo()
	{
		delete [] pAttackPatternInfo;
	}
	void Init(WORD wTotalNum)
	{
		MaxAttackNum = wTotalNum;
		pAttackPatternInfo = new ATTACKPATTERNINFO[MaxAttackNum];
		memset(pAttackPatternInfo, 0, sizeof(ATTACKPATTERNINFO)*MaxAttackNum);
	}
	void InitPattern(WORD PNum, WORD wTotalNum)
	{
		pAttackPatternInfo[PNum].AttackPatternNum = wTotalNum;
		pAttackPatternInfo[PNum].AttackPattern = new WORD[wTotalNum];
		memset(pAttackPatternInfo[PNum].AttackPattern, 0, sizeof(WORD)*wTotalNum);
	}
	void SetVal(WORD PNum, WORD PIdx, WORD PVal)
	{
		pAttackPatternInfo[PNum].AttackPattern[PIdx] = PVal;
	}
	WORD		MaxAttackNum;
};
class CRecallInfo
{
public:
	RECALLINFO * pRecallInfo;

	CRecallInfo(){ memset(this, 0, sizeof(CRecallInfo)); }
	virtual ~CRecallInfo()
	{
		if(pRecallInfo)
		delete [] pRecallInfo;
	}
	void Init(DWORD nRecall)
	{
		MaxRecallNum = nRecall;
		pRecallInfo = new RECALLINFO[MaxRecallNum];
		memset(pRecallInfo, 0, sizeof(RECALLINFO)*MaxRecallNum);
	}
	
	RECALLINFO * GetRecallInfo(DWORD dwIdx)
	{
		ASSERT(dwIdx < MaxRecallNum);
		return &pRecallInfo[dwIdx];
	}
	void SetRecallType(DWORD dwIdx, DWORD dwType)
	{
		pRecallInfo[dwIdx].RecallType = dwType;
	}
	void SetRecallID(DWORD dwIdx, DWORD dwID)
	{
		pRecallInfo[dwIdx].RecallID = dwID;
	}
	void SetRecallMonsterNum(DWORD dwIdx, DWORD dwNum)
	{
		pRecallInfo[dwIdx].RecallMonsterNum = dwNum;
	}
	void SetRecallRange(DWORD dwIdx, DWORD dwRange)
	{
		pRecallInfo[dwIdx].RecallRange = dwRange;
	}
protected:
	DWORD	MaxRecallNum;
	
};
//-----------------------------------------------------------------------
// ���� ����Ʈ ����
//-----------------------------------------------------------------------

class BASE_MONSTER_LIST
{
public:
	
	WORD		MonsterKind;			// ��ȣ
	WORD		ObjectKind;				// Object��ȣ 32 �Ϲ� 33 ���� 35 �ʵ庸�� 36 �ʵ庸������
	char		Name[MAX_MONSTER_NAME_LENGTH +1];				// �����̸�	
	char		EngName[MAX_NAME_LENGTH+1];			// �����̸�
	LEVELTYPE	Level;					// ���ͷ���	
	int			MoveDramaNum;				// �̵����� ��ȣ	
	DWORD		MotionID;
	char		ChxName[MAX_CHXNAME_LENGTH];			// ���� CHX_Index	
	float		Scale;					// ���� ũ��	
	DWORD		Gravity;				// ���� ����
	float		Weight;					// ���� ����	
	DWORD		MonsterRadius;
	DWORD		Life;					// ������	
	DWORD		Shield;					// ȣ�Ű���	
	DWORD		ExpPoint;				// ����ġ
	WORD		Tall;					// Ű

	WORD		AttackPhysicalMin;// ���� �ּҹ��� ���ݷ�	
	WORD		AttackPhysicalMax;// ���� �ִ빰�� ���ݷ�
	WORD		CriticalPercent;// ũ��Ƽ�� �ۼ�Ʈ
	WORD		PhysicalDefense;// ���� ����	
	ATTRIBUTEREGIST AttribResist;		// �Ӽ����׷�

	WORD		WalkMove;// ���Ͱȱ��̵���	
	WORD		RunMove;// ���Ͷٱ��̵���	
	WORD		RunawayMove;// ���͵����̵���

	int			Damage1DramaNum;// ���� 1 ���� ��ȣ	
	int			Die1DramaNum;// ���� 1 ���� ��ȣ	
	int			Die2DramaNum;// ���� 2 ���� ��ȣ	

	DWORD		StandTime;
	BYTE		StandRate;
	BYTE		WalkRate;
	BYTE		RunRate;
	DWORD		DomainRange;					// ���� ���� : ������
	DWORD		PursuitForgiveTime;
//	DWORD		PursuitForgiveStartTime;		//not inclue in txt list
	DWORD		PursuitForgiveDistance;	

	BOOL		bForeAttack;					// ������
	DWORD		SearchPeriodicTime;				// Ž�� �ֱ�
	WORD		TargetSelect;					// Ÿ�� ���� : FIND_CLOSE, FIND_FIRST
	DWORD		SearchRange;
	DWORD		SearchAngle;
	WORD		TargetChange;

	WORD		AttackKind;
	DWORD		AttackNum;
	DWORD *		AttackIndex;
	DWORD *		AttackRate;
	CSkillInfo ** SkillInfo;	
	
	
	WORD		InitHelpType;		// 0 : no request, 1~~
	BOOL		bHearing;
	DWORD		HearingDistance;
};

class SMONSTER_LIST : public BASE_MONSTER_LIST
{
public:
	/*
	virtual ~SMONSTER_LIST()
	{
		for(int i = 0 ; i < MaxAttackNum ; ++i )
			delete [] AttackNumPattern[i];
		delete [] AttackNumPattern;
		
		delete [] MaxAttackIndexNum;
	}
	WORD		MaxAttackNum;
	WORD		* MaxAttackIndexNum;
	WORD		** AttackNumPattern;
	*/

	CAttackPatternInfo AttackPatternClass;
	CRecallInfo RecallInfoClass;
};

class MONSTEREX_LIST : public BASE_MONSTER_LIST
{
public:
	MONEYTYPE MinDropMoney;				// �ּҵ� �׼�	
	MONEYTYPE MaxDropMoney;				// �ִ� �� �׼�	

	
	WORD DropRate[eDROPITEMKIND_MAX];
	
	// �������� ���
	// �ʱ�ȭ�� �ʿ�!!
	WORD CurDropRate[eDROPITEMKIND_MAX];


	WORD ItemListNum1;				// �ش������1����Ʈǥ��ȣ
	WORD ItemListNum2;				// �ش������2����Ʈǥ��ȣ - �߰� 
	WORD ItemListNum3;				// �ش������3����Ʈǥ��ȣ - �߰� 


	BYTE MaxDropItemNum;			// �ִ������ ����

	float fRunawayLifeRate;			// ������ Ȯ�� 0~100
	WORD RunawayRate;				// ���� Ȯ��
	WORD RunawayHelpRate;			// ���� ����Ȯ��
	WORD RunawayDistance;			// ���� �Ÿ�

	WORD Talk1;						// ��� 1
	WORD Talk2;				
	WORD Talk3;

	WORD HelpRequestCondition;
	WORD HelpRequestDistance;
	WORD HelpRequestTargetNum;

	WORD RecallCondition;
	WORD RecallObjectID;
	WORD RecallObjectNum;	
};
//////////////////////////////////////////////////////////////////////////
// ���� chx ������ ����Ʈ
//////////////////////////////////////////////////////////////////////////
typedef struct _CHXLIST
{
	StaticString FileName;
}CHXLIST;

//-----------------------------------------------------------------------
// �� ����Ʈ ����
//-----------------------------------------------------------------------
#define MAX_PET_GRADE	3
struct BASE_PET_LIST
{
	DWORD	PetKind;	//�� �ε���
	char	Name[MAX_NAME_LENGTH+1];	//�̸�
	//PetGrade;
	DWORD	MotionID;	//��ǵ�ID
	WORD	Pet2DIdx;
	char	ChxName_PerGrade[MAX_PET_GRADE][MAX_CHXNAME_LENGTH];
	float	Scale;		//������
	WORD	SummonLevel;	//��ȯ����
	WORD	Tall[MAX_PET_GRADE];	//�̸������ġ
	float	SpeedFromHero;	//���ΰ��� �ӵ� ����/��ȹ���� ����
	WORD	SkillIdx[MAX_PET_GRADE];	//��뽺ų�ε���
	CSkillInfo*	SkillInfo[MAX_PET_GRADE];//��뽺ų����
	DWORD	SkillRecharge;	//��ų�������ð�(�ӵ�)
//	DWORD	SkillGuageMax;	//��ų��������
	DWORD	StaminaDecreasePerSkill[MAX_PET_GRADE];	//�ܰ躰 ��ų ���� �Ҹ� ���¹̳�
	DWORD	StaminaMax[MAX_PET_GRADE];	//�ֽ��׹̳�(�ִ�ġ):���Ŀ����غ�ȭ
	WORD	InventoryTapNum[MAX_PET_GRADE];	//�����κ��丮�Ǽ�
	int		DieDramaNum[MAX_PET_GRADE];		//�ܰ躰 ���� ���� ��ȣ
	WORD	BuffList[MAX_PET_GRADE][3];

//	WORD	EvolutionStep;	//��ȭ�ܰ�
};

struct PET_RULE
{
	DWORD ResummonVilidTime;
	DWORD GradeUpProb1to2;
	DWORD GradeUpProb2to3;
	DWORD MaxFriendship;
	DWORD DefaultFriendship;
	DWORD RevivalFriendship;
	DWORD RevivalFriendship_Shop;
	int FriendShipPerSkill[3];
	int FriendShipPer10Sec[3];
	int FriendShipPerMasterDie;
	int FriendShipPerStaminaZero;
	int FriendShipPerTrade;
	int FriendShipPerUpgradeFail;
	int StaminaStand;
	int StaminaMove;
};

struct PET_BUFF_LIST
{
	WORD Idx;
	char BuffName[MAX_NAME_LENGTH+1];
	BYTE BuffKind;
	DWORD BuffSuccessProb;
	DWORD BuffValueData;
	DWORD BuffAdditionalData;
};

//SW070127 Ÿ��ź
//-----------------------------------------------------------------------
// Ÿ��ź ����Ʈ ����
//-----------------------------------------------------------------------
#define MAX_TITAN_GRADE	3

//#define MAX_CHXNAME_LENGTH	32
struct TITANINFO_GRADE
{
	//char	BodyChxName[MAX_CHXNAME_LENGTH];
	//char	FaceModName[MAX_CHXNAME_LENGTH];
	//char	CloakModName[MAX_CHXNAME_LENGTH];
	//char	ShoulderModName[MAX_CHXNAME_LENGTH];

	DWORD	Speed;
	WORD	KyungGongSpeed[3];
	float	Scale;
	WORD	Tall;

	DWORD	MaxFuel;			// ����, ĳ������ ü�¿� �ش�
	DWORD	MaxSpell;			// ����, ĳ������ ���¿� �ش�
	//DWORD	Endurance;		// �������� ������
	DWORD	Critical;
	float	AttackRange;

	WORD	MeleeAttackPowerMin;
	WORD	MeleeAttackPowerMax;
	WORD	RangeAttackPowerMin;
	WORD	RangeAttackPowerMax;

	//float	PhyAtk;			// ���� ����
	float	PhyDef;			// ���� ���

	ATTRIBUTEATTACK	AttAtk;			// �Ӽ� ����
	ATTRIBUTEREGIST	AttReg;			// �Ӽ� ���

	DWORD	RecallReduceTime;		//Ÿ��ź ���ȯ�ð�
	DWORD	dwRecallSpellTime;			//2007. 11. 19. CBH - Ÿ��ź ��ȯ ĳ���� �ð�
};

struct BASE_TITAN_LIST
{
	DWORD	TitanIdx;						//Ÿ��ź ��ȣ
	char	TitanName[MAX_NAME_LENGTH+1];	//Ÿ��ź �̸�
	char	TitanBaseChxName[MAX_CHXNAME_LENGTH+1];
	char	TitanfaceModName[MAX_CHXNAME_LENGTH+1];
	//DWORD	MotionID;						//��ǵ� ID
	WORD	Titan2DImage;						//
	
	TITANINFO_GRADE	GradeInfo[MAX_TITAN_GRADE];
};

struct TITAN_SKILLLINK
{
	//DWORD	linkIdx;
	DWORD	CharSkillIdx;
	DWORD	TitanSkillIdx;
#ifdef _DEBUG
	char	skillName[MAX_NAME_LENGTH+1];
#endif
};

struct TITAN_EQUIPITEM_ENDURANCE
{
	DWORD titanEquipItemIdx;
	DWORD decreaseEndurance;
};

struct TITAN_RULE
{
	DWORD	TitanRecallValidTime_ZeroFuel;	// ��Ÿ���� ���� �������� ���� �����Ǿ��� �� ���ȯ ���� �ð�.
	DWORD	TitanRecallValidTime_Seal;		// ��Ÿ���� ���ȯ ���� �ð�.
	DWORD	TitanRecallValidTime_General;		// magi82(24) - Titan(071108) �Ϲݸ� ���ȯ ���� �ð�.(�Ϲݸ��� ������ ���� ���� ���ȯ�ð� ����)
	DWORD	TitanVampValidTime;			// ���� ü�� ���� ���� �ð�.
	DWORD	TitanFuelRate_ForVamp;			// ���� ü�� ���� ����. Ÿ��ź ü��% ������ ��.
	DWORD	MasterLifeRate_AllowForTitanVamp;			// ���� ü�� ���� ����. ���� ü���� ��ü ü���� % �̻��� �� ����.
	DWORD	MasterLifeRate_FromTitanVamp;			// ���� ü�� ���� ����. ���� ��ü ü���� % ����.
	DWORD	FuelConvertRate_FromTitanVamp;			// ���� ü�� ���� �� ��ȯ�� %.
	DWORD	MasterLifeRate_forTitanCall;			// Ÿ��ź ��ȯ ������ ���� ü�� %.
	DWORD	TitanDamageShareRate;				// ���ΰ� �й��ϴ� ������ %.
	DWORD	Prob_DecreaseEndurance;				// ������ �϶� Ȯ�� %.
	DWORD	Prob_DecreaseArmorEndurance_Atk;	// ���ݽ� �� �������� �϶��� Ȯ�� %.
	DWORD	Prob_DecreaseArmorEndurance_Def;	// ���� �� �������� �϶��� Ȯ�� %.
	DWORD	TitanMaintainTime;					// magi82(25)
};

//-----------------------------------------------------------------------
// ����ȯ ��ǥ��� ����ü
//-----------------------------------------------------------------------
typedef struct _MAPCHANGE_INFO
{
	WORD Kind;
	char CurMapName[MAX_NAME_LENGTH+1];
	char ObjectName[MAX_NAME_LENGTH+1];
	WORD CurMapNum;
	WORD MoveMapNum;
	VECTOR3 CurPoint;
	VECTOR3 MovePoint;
	WORD chx_num;
	
	// ���⼳��
}MAPCHANGE_INFO;

//-----------------------------------------------------------------------
// LogIn ��ǥ��� ����ü
//-----------------------------------------------------------------------
typedef struct _LOGINPOINT_INFO
{
	WORD Kind;
	char MapName[MAX_NAME_LENGTH+1];
	WORD MapNum;
	BYTE MapLoginPointNum;
	VECTOR3 CurPoint[10];
	WORD chx_num;

	// ���⼳��
}LOGINPOINT_INFO;

//-----------------------------------------------------------------------
// NPC ����
//-----------------------------------------------------------------------
typedef struct _NPC_LIST
{
	WORD NpcKind;			// Npc ����
	char Name[MAX_NAME_LENGTH+1];			// Npc �̸�
	WORD ModelNum;			// MonsterChxList������ ��ȣ
	WORD JobKind;			// ��������������
	float Scale;			// Npc ũ��
	WORD Tall;				// Ű
	BOOL ShowJob;
}NPC_LIST;

//-----------------------------------------------------------------------
// NPC ����
//-----------------------------------------------------------------------
typedef struct _NPC_REGEN
{

	_NPC_REGEN():dwObjectID(0),NpcKind(0)
	{
		Pos.x = Pos.y = Pos.z = 0;
	}
	DWORD	dwObjectID;		// �ű� �߰� taiyo
	MAPTYPE	MapNum;			// Map��ȣ
	WORD	NpcKind;		// NpcKind
	char	Name[MAX_NAME_LENGTH+1];		// Npc �̸�
	WORD	NpcIndex;		// NpcIndex(������ȣ�� ���)
	VECTOR3 Pos;			// ���� ��ġ
	float	Angle;			// ���� ����
}NPC_REGEN;


//-----------------------------------------------------------------------
// MOD ����Ʈ 
//-----------------------------------------------------------------------
typedef struct _MOD_LIST
{
	_MOD_LIST()
	{
		MaxModFile = 0;
		ModFile = NULL;
	}
	~_MOD_LIST()
	{
		if(MaxModFile == 0)
			return;
		SAFE_DELETE_ARRAY(ModFile);
	}
	DWORD MaxModFile;
	StaticString* ModFile;
	StaticString BaseObjectFile;
	
}MOD_LIST;

//---------------<������ ���� ����Ʈ>------------------------------------------------------
struct ITEM_INFO
{
	WORD ItemIdx;			//		Item_Index	
	char ItemName[MAX_ITEMNAME_LENGTH+1];		//		Item_name
	WORD ItemTooltipIdx;	//		Item���� ���� index
	WORD Image2DNum;		//		2D_Image	
	WORD ItemKind;			//		����������	"0 : ���� 1 : ������ 2 : ���������� 3 : ��Ÿ"
	DWORD BuyPrice;			//		���԰���	
	DWORD SellPrice;		//		�ǸŰ���	
//	WORD Rarity;			//		��Ұ�ġ	
	DWORD Rarity;			//		��Ұ�ġ	
	WORD WeaponType;		//		��������	���� / ������
	WORD GenGol;			//		�ٰ�+(����)	����� ���� �����ۿ� ������
	WORD MinChub;			//		��ø+(����)	����� ���� �����ۿ� ������
	WORD CheRyuk;			//		ü��+(����)	����� ���� �����ۿ� ������
	WORD SimMek;			//		�ɸ�+(����)	����� ���� �����ۿ� ������
	DWORD Life;				//		������+(����)	����� ���� �����ۿ� ������
	DWORD Shield;			//		ȣ�Ű���+(����)	����� ���� �����ۿ� ������
	WORD NaeRyuk;			//		����+(����)	����� ���� �����ۿ� ������
	ATTRIBUTEREGIST AttrRegist;		//		ȭ �Ӽ� ���׷�+(����)	����� ���� �����ۿ� ������
							//		�� �Ӽ� ���׷�+(����)	����� ���� �����ۿ� ������
							//		�� �Ӽ� ���׷�+(����)	����� ���� �����ۿ� ������
							//		�� �Ӽ� ���׷�+(����)	����� ���� �����ۿ� ������
							//		�� �Ӽ� ���׷�+(����)	����� ���� �����ۿ� ������
	WORD LimitJob;			//		�������� ����	���� / ���������� ��� / ��¹���-ȭ��, �ظ�
	WORD LimitGender;		//		�������� ����	���� / ���������� ���
	LEVELTYPE LimitLevel;		//		�������� ����	���� / ���������� ���
	WORD LimitGenGol;		//		�������� �ٰ�	���� / ���������� ���
	WORD LimitMinChub;		//		�������� ��ø	���� / ���������� ���
	WORD LimitCheRyuk;		//		�������� ü��	���� / ���������� ���
	WORD LimitSimMek;		//		�������� �ɸ�	���� / ���������� ���

	WORD ItemGrade;			//	����������	������ ���	
	WORD RangeType;			//				�Ÿ�����	
	WORD EquipKind;			//				��������	
	WORD Part3DType;		//				3D������ȣ	
	WORD Part3DModelNum;	//				3D�����𵨹�ȣ	
	WORD MeleeAttackMin;	//				�ٰŸ� �ּҰ��ݷ�
	WORD MeleeAttackMax;	//				�ٰŸ� �ִ���ݷ�
	WORD RangeAttackMin;	//				���Ÿ� �ּҰ��ݷ�
	WORD RangeAttackMax;	//				���Ÿ� �ִ���ݷ�
	WORD CriticalPercent;	//				ũ��Ƽ�� �ۼ�Ʈ
	ATTRIBUTEATTACK AttrAttack;		//				ȭ�Ӽ����ݷ�+%
							//				���Ӽ����ݷ�+%
							//				��Ӽ����ݷ�+%
							//				�ݼӼ����ݷ�+%
							//				����ݰ��ݷ�+%
	WORD PhyDef;			//				���� ����+ 
	WORD Plus_MugongIdx;	//				Ư�� ����+ index
	WORD Plus_Value;		//				Ư�� ����+ ����
	WORD AllPlus_Kind;		//				���� ������ ����+ 
	WORD AllPlus_Value;		//				���� ������ ����+ ����

	WORD MugongNum;			//	������	���� ����  ��ȣ
	WORD MugongType;		//			��������

	WORD LifeRecover;		//	����	������+(ȸ��) ������ġ	���� ����
	float LifeRecoverRate;		//			������+(ȸ��)  	���� ����
	WORD NaeRyukRecover;	//			����+(ȸ��) ������ġ	���� ����
	float NaeRyukRecoverRate;	//			����+(ȸ��)    	���� ����

	WORD ItemType;			//	��Ÿ	������ �Ӽ�	=>����Ʈ, ��ؼ�, ���
	WORD wItemAttr;			// �����ۼӼ�(ȭ, ��, ��, ��, ��) => �Ϻ� ���ð���
	WORD wAcquireSkillIdx1;	// �ʿ��� ��ų �ε��� => �Ϻ����ð���
	WORD wAcquireSkillIdx2;	// �ʿ��� ��ų �ε��� => �Ϻ����ð���
	WORD wDeleteSkillIdx;	// ���� ��ų �ε��� => �Ϻ����ð���

	WORD wSetItemKind;		// ��Ʈ������ ����  *** 2007. 6. 7. CBH ***
};


//// 2007. 6. 8. CBH - SETITEM �ɼ� ���� ����ü �߰� ////////////////////////////////////////////
struct SET_ITEM_OPTION
{
	WORD wIndex;									// ��Ʈ������ �ɼ� �ε���
	WORD wSetItemKind;								// ��Ʈ������ ����
	char szSetItemName[MAX_NAME_LENGTH+1];			// ��Ʈ������ ���ξ� �̸�
	
	WORD wGenGol;									//	�ٰ�
	WORD wMinChub;									//	��ø
	WORD wCheRyuk;									//	ü��
	WORD wSimMek;									//	�ɸ�
	DWORD dwLife;									//	������
	DWORD dwShield;									//	ȣ�Ű���
	DWORD dwNaeRyuk;									//	����
	ATTRIBUTEREGIST AttrRegist;						//	ȭ �Ӽ� ���׷�+(����)
													//	�� �Ӽ� ���׷�+(����)
													//	�� �Ӽ� ���׷�+(����)
													//	�� �Ӽ� ���׷�+(����)
													//	�� �Ӽ� ���׷�+(����)

	LEVELTYPE wLimitLevel;							//	�������� ����
	char szEffect[100];								//	���� ����Ʈ
	WORD wLimitGenGol;								//	�������� �ٰ�
	WORD wLimitMinChub;								//	�������� ��ø
	WORD wLimitCheRyuk;								//	�������� ü��
	WORD wLimitSimMek;								//	�������� �ɸ�	
	
	WORD wMeleeAttackMin;							//	�ٰŸ� �ּҰ��ݷ�
	WORD wMeleeAttackMax;							//	�ٰŸ� �ִ���ݷ�
	WORD wRangeAttackMin;							//	���Ÿ� �ּҰ��ݷ�
	WORD wRangeAttackMax;							//	���Ÿ� �ִ���ݷ�
	WORD wCriticalPercent;							//	ũ��Ƽ�� �ۼ�Ʈ
	ATTRIBUTEATTACK AttrAttack;						//	ȭ�Ӽ����ݷ�+%
													//	���Ӽ����ݷ�+%
													//	��Ӽ����ݷ�+%
													//	�ݼӼ����ݷ�+%
													//	����ݰ��ݷ�+%
	
	WORD wPhyDef;									//	���� ����+ 
	WORD wLifeRecover;								//	������+(ȸ��) ������ġ
	float fLifeRecoverRate;							//	������+(ȸ��)
	WORD wNaeRyukRecover;							//	����+(ȸ��) ������ġ
	float fNaeRyukRecoverRate;						//	����+(ȸ��)
	WORD wSetValue;									//	��Ʈ ����
	WORD wApplicationValue;							//  ����Ʈ ���� ��Ʈ ��
};

struct SET_ITEM_INFO
{
	WORD wKind;
	WORD wCount;
};
///////////////////////////////////////////////////////////////////////////

//!//
struct PET_NAME
{
	WORD PetIdx;			//		Item_Index	
	char ItemName[MAX_PETNAME_LENGTH+1];		//		Item_name

};
/*
class ITEM_REINFORCE_INFO
{
public:
	WORD			wItemIdx;
	char			szItemName[MAX_ITEMNAME_LENGTH+1];
	LEVELTYPE		ItemLevel;
	WORD			ReinforceType;
	float			w;
};
*/


//---�ƾ��۰�ȭ
struct sITEM_REINFORCE_INFO
{
	WORD			wItemIdx;
	WORD			ReinforceType;
	float			fMaxReinforce;
	float			fw;
};
//-----------------------------------
struct sITEM_RAREREINFORCE_INFO	: public sITEM_REINFORCE_INFO
{
	WORD			wMaterialGravity;
};

//---�ƾ��ۺ���
struct sITEM_DISSOLUTIONITEM
{
	WORD			wUnitItemIdx;					//���ص� ������ �ε���
	WORD			wUnitItemNum;						//���ص� ������ ����
};

struct sITEM_DISSOLUTION_INFO
{
	WORD					wDissolutionItemIdx;	//������ ���̽������� �ε���
	WORD					wUnitKindNum;			//���ص� ������ ���� ����
	WORD					wUnitTotalNum;			//���ص� ������ �� ����
	sITEM_DISSOLUTIONITEM*	pUnitItem;				//���ؾ����� ����
};
//-----------------------------------



class ITEM_MIX_INFO
{
public:
	WORD			wItemIdx;
	char			szItemName[MAX_ITEMNAME_LENGTH+1];
	
	WORD			wResultItemNum;
	ITEM_MIX_RES	* psResultItemInfo;
};

class ITEM_MIX_RES
{
public:
	WORD		wResItemIdx;
	char		szResItemName[MAX_ITEMNAME_LENGTH+1];
	WORD		wMixKind;
	MONEYTYPE	Money;
	WORD		SuccessRatio;
	WORD		wMaterialItemNum;
	ITEM_MIX_MATERIAL * psMaterialItemInfo;
};

class ITEM_MIX_MATERIAL
{
public:
	WORD		wMatItemIdx;
	char		szMatItemName[MAX_ITEMNAME_LENGTH+1];
	WORD		wMatItemNum;
};

struct HILEVEL_ITEM_MIX_RATE_INFO	// +10�� ���� ��� �߰� �۾� by Stiner(2008/06/11)-10+ItemMix
{
	WORD	wItemLevel;
	DWORD	dwBaseRate;
	WORD	dwModulate;
};

// magi82 - Titan(070129)
struct TITAN_PARTS_KIND
{
	DWORD	dwPartsIdx;
	DWORD	dwPartsKind;
	DWORD	dwResultTitanIdx;
};

struct TITAN_STATSINFO
{
	DWORD	titanIdx;						// Ÿ��ź�ε���
	DWORD	titanType;						// Ÿ��
	DWORD	minAtt;							// �ּҰ��ݷ�
	DWORD	maxAtt;							// �ִ���ݷ�
	DWORD	woigongCri;						// �ܰ��ϰ�
	DWORD	naegongCri;						// �����ϰ�
	DWORD	minlongAtt;						// �ּҿ��Ÿ����ݷ�
	DWORD	maxlongAtt;						// �ִ���Ÿ����ݷ�
	DWORD	distance;						// �����Ÿ�
	DWORD	Life;							// ������
	DWORD	defense;						// ����
	DWORD	Mana;							// ����
	DWORD	Hosin;							// ȣ�Ű���
	WORD	AttrFire;						// ȭ
	WORD	AttrWater;						// ��
	WORD	AttrTree;						// ��
	WORD	AttrIron;						// ��
	WORD	AttrEarth;						// ��
};
//////////////////////////////////////////////////////////////////////////

// magi82 - Titan(070131)
struct TITAN_UPGRADE_MATERIAL
{
    DWORD dwIndex;
	DWORD dwCount;
};

struct TITAN_UPGRADEINFO
{
    DWORD	dwTitanIdx;
	DWORD	dwNextTitanIdx;
	DWORD	dwMoney;
	WORD	wMaterial_Num;
	TITAN_UPGRADE_MATERIAL* pTitanUpgradeMaterial;

	TITAN_UPGRADEINFO::TITAN_UPGRADEINFO()
	{
		pTitanUpgradeMaterial = NULL;
	}
	TITAN_UPGRADEINFO::~TITAN_UPGRADEINFO()
	{
		if( pTitanUpgradeMaterial )
		{
			delete [] pTitanUpgradeMaterial;
			pTitanUpgradeMaterial = NULL;
		}
	}
};
//////////////////////////////////////////////////////////////////////////

// magi82 - Titan(070205)
struct TITAN_BREAK_MATERIAL
{
	DWORD	dwMaterialIdx;
	WORD	wMaterCnt;
	WORD	wRate;
};

struct TITAN_BREAKINFO
{
	DWORD	dwIdx;
	DWORD	dwMoney;
	WORD	wTotalCnt;
	WORD	wGetCnt;
	TITAN_BREAK_MATERIAL* pTitanBreakMaterial;

	TITAN_BREAKINFO::TITAN_BREAKINFO()
	{
		dwIdx = 0;
		wTotalCnt = 0;
		wGetCnt = 0;
		pTitanBreakMaterial = NULL;
	}
	TITAN_BREAKINFO::~TITAN_BREAKINFO()
	{
		if( pTitanBreakMaterial )
		{
			delete [] pTitanBreakMaterial;
			pTitanBreakMaterial = NULL;
		}
	}
};

struct BobusangPosPerMap
{
	VECTOR3	ApprPos;
	float	ApprDir;
};
//////////////////////////////////////////////////////////////////////////

// magi82 - UniqueItem(070625)
struct UNIQUE_ITEM_OPTION_INFO
{
	DWORD	dwItemIdx;								// ����ũ ������ �ε���
	char	strManEffectName[MAX_CHXNAME_LENGTH];	// ����ũ ������ ����Ʈ ���� �̸�(����)
	char	strWomanEffectName[MAX_CHXNAME_LENGTH];	// ����ũ ������ ����Ʈ ���� �̸�(����)
	BYTE	ExchangeFlag;							// �ŷ� ���� ����
	BYTE	MixFlag;								// ���� ���� ����
	DWORD	dwCurseCancellation;					// ���������� �������� �ε���

	// HP(+/-)
	int		nHp;
	// MP(+/-)
	int		nMp;
	// ����������(+/-)
	int		nPhyDamage;
	// �ϰݵ�����(+/-)
	int		nCriDamage;
	// �ϰ�Ȯ��(+/-)
	int		nCriRate;
	// �ٰ�(+/-)
	int		nGengol;
	// ��ø(+/-)
	int		nMinChub;
	// ü��(+/-)
	int		nCheRyuk;
	// �ɸ�(+/-)
	int		nSimMek;
	// ����(+/-)
	int		nDefen;
	// �����Ÿ�(+/-)
	int		nRange;
	// �Ӽ�����(+/-)
	int		nAttR;
	// ������ ����
	int		nEnemyDefen;
	// ȣ�Ű���(+/-)
	int		nShield;
    
	UNIQUE_ITEM_OPTION_INFO::UNIQUE_ITEM_OPTION_INFO()
	{
		dwItemIdx			= 0;
		ZeroMemory(strManEffectName, sizeof(strManEffectName));
		ZeroMemory(strWomanEffectName, sizeof(strWomanEffectName));
		ExchangeFlag		= 0;
		MixFlag				= 0;

		nHp					= 0;
		nMp					= 0;
		nPhyDamage			= 0;
		nCriDamage			= 0;
		nCriRate			= 0;
		nGengol				= 0;
		nMinChub			= 0;
		nCheRyuk			= 0;
		nSimMek				= 0;
		nDefen				= 0;
		nRange				= 0;
		nAttR				= 0;
		nEnemyDefen			= 0;
		nShield				= 0;
	}
};

struct UNIQUE_ITEM_MIX_MATERIAL_INFO
{
	DWORD	dwMaterialIdx;
	WORD	wMaterialNum;
};

struct UNIQUE_ITEM_MIX_RESULT_INFO
{
	DWORD	dwResultIdx;
	WORD	wResultRate;
};

struct UNIQUE_ITEM_MIX_INFO 
{
	DWORD	dwItemIdx;
	WORD	wMaterialKindNum;
	
	UNIQUE_ITEM_MIX_MATERIAL_INFO*	sUniqueItemMixMaterial;
	UNIQUE_ITEM_MIX_RESULT_INFO		sUniqueItemMixResult[UNIQUE_ITEM_MIX_RESULT_INDEX];
};

//// 2007. 12. 6. CBH - ��Ų SETITEM �ɼ� ���� ����ü �߰� ////////////////////////////////////////////
struct SKIN_SELECT_ITEM_INFO
{
	DWORD dwIndex;							// ��Ų �ε���
	char szSkinName[MAX_ITEMNAME_LENGTH+1];	//��Ų �̸�
	DWORD dwLimitLevel;						// ���� ����
	WORD wEquipItem[SKINITEM_LIST_MAX];		//��Ų ������ ������ �ε��� �迭

	SKIN_SELECT_ITEM_INFO::SKIN_SELECT_ITEM_INFO()
	{
		dwIndex = 0;	//�ʱ� ���� -1�� ���ش�. ������ ��Ʈ�� ����Ʈ�� ��ȯ���� ���߱����� ����
		dwLimitLevel = 0;
		memset( wEquipItem, 0, sizeof(WORD)*SKINITEM_LIST_MAX );		
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------------------------------------//
//		Ŭ���̾�Ʈ�� ����ϴ� �κ�
#ifdef _CLIENT_RESOURCE_FIELD_
//-----------------------------------------------------------------------------------------------------------//

typedef struct _PRELOAD
{
	StaticString FileName;
}PRELOAD;



struct SETSHOPITEM
{
	DWORD ItemIdx;
	char Name[MAX_ITEMNAME_LENGTH+1];
};


#ifdef _TESTCLIENT_

//trustpak
struct TESTCHARACTER_INFO
{
	BYTE byGender;
	float fX;
	float fY;
	float fZ;
	float fAngle;
};

#define MAX_TEST_CHARACTER_NO 8
//


struct TESTCLIENTINFO {
	TESTCLIENTINFO()
	{
		//trustpak
		memset(this, 0, sizeof(TESTCLIENTINFO));
		//

		LightEffect = 1;
		Effect = 0;
		MugongIdx = 0;
		WeaponIdx = MIN_EQUIPMENTITEM_INDEX+1;
		Gender = GENDER_MALE;
		x = 30000.f;
		z = 30000.f;
		MonsterEffect = 1;
		MonsterNum = 1;
		MonsterKind = 13;
		Map = 17;

		DressIdx = 0;
		HatIdx = 0;
		ShoesIdx = 0;
		HairType = 0;
		FaceType = 0;
		

		CharacterNum = 0;
		BossMonsterNum = 0;
		BossMonsterKind = 1000;
		
		memset( &titaninfo, 0, sizeof(titaninfo) );
		titaninfo.TitanGrade = 1;
		titaninfo.TitanKind = 1;
		titaninfo.TitanScale = 100;
	}
	float x,z;

	BOOL bInTitan;

	WORD DressIdx;
	WORD HatIdx;
	WORD ShoesIdx;

	BYTE HairType;
	BYTE FaceType;

	WORD MonsterKind;
	BOOL bMonsterDie;

	WORD BossMonsterKind;
	WORD BossMonsterNum;
	char BossInfofile[17];

	int Effect;
	WORD MugongIdx;
	WORD WeaponIdx;
	BYTE Gender;
	int MonsterEffect;
	WORD MonsterNum;
	WORD CharacterNum;
	MAPTYPE Map;

	BOOL LightEffect;
	
	float Width;
	float Height;

	//trustpak
	DWORD				dwTestCharacterNo;
	TESTCHARACTER_INFO	aTestCharacterInfo[MAX_TEST_CHARACTER_NO];
	
	//
	TITAN_APPEARANCEINFO	titaninfo;

	int nTestEffectNum;
};
#endif

//-----------------------------------------------------------------------
// game config ���� 
//-----------------------------------------------------------------------
struct GAMEDESC_INI
{
	GAMEDESC_INI()
	{
		strcpy(DistributeServerIP,"211.233.35.36");
		DistributeServerPort = 400;

		AgentServerPort = 100;
		
		bShadow = MHSHADOW_CIRCLE;
		
		dispInfo.dwWidth = 1024;		//WINDOW SIZE
		dispInfo.dwHeight = 768;
		dispInfo.dwBPS = 4;
		dispInfo.dispType = WINDOW_WITH_BLT;
		//dispInfo.bWindowed = TRUE;
		dispInfo.dwRefreshRate = 70;

		MaxShadowTexDetail = 256;
		MaxShadowNum = 4;

		FramePerSec = 30;
		TickPerFrame = 1000/(float)FramePerSec;

		MasterVolume = 1;
		SoundVolume = 1;
		BGMVolume = 1;
		
		CameraMinAngleX = 0;
		CameraMaxAngleX = 89.f;
		CameraMinDistance = 200.f;
		CameraMaxDistance = 1000.f;

		LimitDay = 0;
		LimitID[0] = 0;
		LimitPWD[0] = 0;

		strWindowTitle[0] = 0;
	}

	float MasterVolume;
	float SoundVolume;
	float BGMVolume;

	char DistributeServerIP[32];
	WORD DistributeServerPort;
	
	WORD AgentServerPort;
	
	DISPLAY_INFO dispInfo;

	BYTE bShadow;
	DWORD MaxShadowNum;
	DWORD MaxShadowTexDetail;

	DWORD FramePerSec;
	float TickPerFrame;

	char MovePoint[64];

	float CameraMinAngleX;
	float CameraMaxAngleX;
	float CameraMinDistance;
	float CameraMaxDistance;

	DWORD LimitDay;
	char LimitID[MAX_NAME_LENGTH+1];
	char LimitPWD[MAX_NAME_LENGTH+1];

	char strWindowTitle[128];
};

struct SEVERLIST
{
	char	DistributeIP[16];
	WORD	DistributePort;
	char	ServerName[64];
	WORD	ServerNo;
	BOOL	bEnter;

	SEVERLIST()
	{
		ServerNo = 1;
		strcpy( DistributeIP, "211.233.35.36" );
		DistributePort = 400;
		strcpy( ServerName, "Test" );
		bEnter = TRUE;
	}
};


//-----------------------------------------------------------------------------------------------------------//
#endif //_CLIENT_RESOURCE_FIELD_
//-----------------------------------------------------------------------------------------------------------//


//-----------------------------------------------------------------------------------------------------------//
//		������ ����ϴ� �κ�
#ifdef _SERVER_RESOURCE_FIELD_

//-----------------------------------------------------------------------------------------------------------//
#include "ServerTable.h"
//-----------------------------------------------------------------------
// ĳ���� IN/OUT POINT ����
//-----------------------------------------------------------------------
typedef struct _CHARACTERINOUTPOINT
{
	WORD MapNum;
	VECTOR3 MapInPoint[MAX_MAP_NUM];
	VECTOR3 MapOutPoint[MAX_MAP_NUM];
}CHARACTERINOUTPOINT;


//-----------------------------------------------------------------------------------------------------------//
#endif //_SERVER_RESOURCE_FIELD_
//-----------------------------------------------------------------------------------------------------------//
#endif //__GAMERESOURCESTRUCT_H__
