#ifndef _ABILITYHEADER_
#define _ABILITYHEADER_

#include "..\[CC]Header\CommonGameDefine.h"
#include "..\[CC]Header\CommonGameStruct.h"  // 添加此行以获取ATTRIBUTE_VAL模板定义

WORD GetAbilityIndex_WithKindPosition(BYTE Kind,BYTE Position);
void GetKindPosition_WithAbilityIndex(WORD AbilityIndex,BYTE* pOutKind,BYTE* pOutPosition);
BYTE GetAbilityLevel_WithDBValue(BYTE DBValue);
// 06. 01 �̿��� - ����â ����
// �� �Լ��� ������� �ʴ´�
//WORD GetAbilityQuickPosition(BYTE DBValue);

#define ABILITYUPDATE_CHARACTERLEVEL_CHANGED			0x00000001
#define ABILITYUPDATE_ABILITYUPITEM_USE					0x00000002
#define ABILITYUPDATE_ABILITYLEVEL_CHANGED				0x00000004
#define ABILITYUPDATE_ABILITYEXP_CHANGED				0x00000008
#define ABILITYUPDATE_ABILITYQUICK_CHANGED				0x00000010
#define ABILITYUPDATE_ABILITYINITED						0x00000020
#define ABILITYUPDATE_ABILITYLEVEL_CHANGEDBYSHOPITEM	0x00000100
#define ABILITYUPDATE_CHARACTERSTAGE_CHANGED			0x00000200

enum eAbilityGradeUpMethod
{
	eAGM_OnOff,		//0: 1ȸ�� ������ : �ѹ� �����Ͽ� ������ ȿ���� ������ �Ǹ� �� �̻��� ȿ�������� ����. (On/Off��)
	eAGM_GradeUp,	//1: ��� ������ : Ư��ġ�� ����Ͽ� ����ؼ� ������ ������ �ִ� ����
	eAGM_Restrict,	//2: ���� ������ : Ư�� Ư�Ⱑ �ش� ������ �����ϱ� ������ Ư�⸦ �����ų�� ���� ����
};

enum eAbilityUseType
{
	eAUT_Use,		//0: 1ȸ�� Ÿ��
	eAUT_Passive,	//1: �нú� Ÿ��
	eAUT_Active,	//2: ON/OFF
};

enum eAbilityGroupKind
{
	eAGK_Battle,
	eAGK_KyungGong,
	eAGK_Society,
	eAGK_Job,

	eAGK_Max,
};

#define ABILITYLEVEL_DB_KEY		'A'

#define ABILITYQUICKPOSITION_NULL	' '
#define ABILITYQUICKPOSITION_KEY	'A'

#define ABILITY_MAX_LEVEL		30

#define MAX_TOTAL_ABILITY	(MAX_ABILITY_NUM_PER_GROUP*eAGK_Max)
#define MAX_ABILITY_NUM_PER_GROUP	40
#define MAX_ABILITY_LEVEL 30

#define ABILITY_STARTINDEX_BATTLE		100
#define ABILITY_STARTINDEX_KYUNGGONG	200
#define ABILITY_STARTINDEX_SOCIETY		300
#define ABILITY_STARTINDEX_JOB			400
#define ABILITY_STARTINDEX_INTERVAL		100


struct ABILITY_STATS
{
	DWORD PhyAttackUp[WP_MAX];
	ATTRIBUTE_VAL<float> AttAttackUp;
	ATTRIBUTE_VAL<float> AttRegistUp;
	DWORD DefenceUp;

	DWORD LifeUp;
	DWORD NaeRyukUp;
	DWORD ShieldUp;
	
	DWORD UngiUpVal;

	// Change Stage
	DWORD StatGen;	// ��
	DWORD StatMin;	// ��
	DWORD StatChe;	// ü
	DWORD StatSim;	// ��
	float Kyunggong;	// ��Ź�
	WORD  KyunggongLevel;
	float fNoAttrib;	// �����
	DWORD SkillDamage;	// ����������
	DWORD CriticalDamage;	// �ϰݵ�����
	DWORD dwTitanRidingPlusTime;	//2007. 11. 6. CBH - Ÿ��ź ž�� �ð�

	DWORD GetPhyAttackUp(WORD WeaponKind)
	{
		return PhyAttackUp[WeaponKind-1];
	}
	void SetPhyAttackUp(WORD WeaponKind,DWORD UpVal)
	{
		PhyAttackUp[WeaponKind-1] = UpVal;
	}
	void Clear()
	{
		memset(this,0,sizeof(ABILITY_STATS));
	}
};

struct ABILITY_CALC
{
	DWORD	dwPhyAttack;	// ���� ����
	float	fAttribAttack;	// �Ӽ� ����
	float	fAttribRegist;	// �Ӽ� ����
	DWORD	dwLife;			// ������
	DWORD	dwDeffence;		// ����
	DWORD	dwNearyuk;		// ����
	DWORD	dwShield;		// ȣ��
	DWORD	dwUngi;			// ���
	DWORD	dwStat;			// ��, ��, ü, ��
	float	fKyunggong;		// ������ǵ�
	float	fNoAttrib;		// �����
	DWORD	dwSkillDamage;	// ����������
	DWORD	dwCriticalDamage;	// �ϰݵ�����
	DWORD	dwTitanRidingPlusTime;	//2007. 11. 6. CBH - Ÿ��ź ž�� �ð�
};

enum eABILITY_USE_KIND
{
	eAUK_KYUNGGONG = 1,
	eAUK_JOB,

	eAUK_PHYATTACK,
	eAUK_ATTRIB,

	eAUK_MAXLIFE,
	eAUK_DEFENCE,
	eAUK_MAXNEARYUK,
	eAUK_MAXSHIELD,
	eAUK_UNGISPEED,

	eAUK_SOCIETY = 10,	//�̰� 10�̶�� ���ߴ�.
	eAUK_PET	= 11,	//11 abilitybaseinfo.bin -> effect_kind
};

enum eABILITY_USE_KIND_STAGE	// ������ ȿ���� 11~100
{
	eAUK_STAT1				= 11,	// ��
	eAUK_STAT2				= 12,	// ��
	eAUK_STAT3				= 13,	// ü
	eAUK_STAT4				= 14,	// ��
	eAUK_KYUNGGONG_SPEED	= 15,	// ��Ź�
	eAUK_JUSOOL				= 16,	// �ּ�
	eAUK_NOATTRIB			= 17,	// �����
	eAUK_SKILL_DAMAGE		= 18,	// ����������
	eAUK_CRITICAL_DAMAGE	= 19,	// �ϰݵ�����
};

enum eABILITY_USE_JOB
{
	eAUKJOB_Ungijosik = 101,	// �������
	eAUKJOB_Vimu = 102,			// �񹫽�û
	eAUKJOB_Party = 103,		// ����â��
	eAUKJOB_Guild = 104,		// ����â��
	eAUKJOB_Deal = 105,			// �ŷ�
	eAUKJOB_StreetStall = 106,  // �����󰳼�

	eAUKJOB_Restraint = 108,	// ����
	eAUKJOB_Attack = 109,		// ����

	eAUKJOB_StallFind = 110,	// ������ �˻� by Stiner(8)
	eAUKJOB_AutoNote = 111,	// �����Ʈ
	eAUKJOB_PartyFind = 112,	//2008. 6. 4. CBH - ���� ã�� �߰�

	eAUKJOB_TitanExpert = 120,	//Ÿ��ź ����

	eAUKJOB_Upgrade = 201,		//��޾�
	eAUKJOB_Mix = 202,			//����
	eAUKJOB_Reinforce = 203,	//��ȭ
	eAUKJOB_Dissolve = 204,		//����
};

enum eABILITY_USE_PET
{
	eAUKPET_State	= 150,
	eAUKPET_Inven	= 151,
	eAUKPET_Skill	= 152,
	eAUKPET_Rest	= 153,
	eAUKPET_Seal	= 154,
};

enum eABILITY_USE_SOCIETY
{
	eAUKJOB_Bow		= 301,		// �λ�(����)
	eAUKJOB_Happy	= 302,		// ���
	eAUKJOB_Sad		= 303,		// ����
	eAUKJOB_Yes		= 304,		// ����
	eAUKJOB_No		= 305,		// ����
	eAUKJOB_Sit		= 306,		// �ɱ�/�Ͼ��
//	eAUKJOB_Stand	= 307,		// �Ͼ��
	eAUKJOB_BigBow	= 307,		// �ݽ��λ�
};

enum eAbilityIconState
{
	// 06. 01. �����Ƽâ �������̽� ���� - �̿���
	// ���̴� ���� ����
	eAIS_NotUsable,
	eAIS_NotAvailable,
	eAIS_OnlyVisible,
	eAIS_Usable,
};


enum AbilityAcquireKind
{
	eAAK_Item,				//�������� ����Ͽ� Ư�⸦ ����
	eAAK_Quest,				//����Ʈ�� ���Ͽ� Ư�⸦ ����
	eAAK_OtherAbility,		//Ư�� Ư�Ⱑ ���� ��� �̻��� �����ϸ� ��´�.
	eAAK_CharacterLevel,	//ĳ������ ������ Ư���ܰ迡 �����ϸ� ����
	eAAK_CharacterStage,	//����������� ����
	
	eAAK_Max,
};


#define ABILITYINDEX_UNGI	401	//�������
#define ABILITYINDEX_VIMU	402	//�񹫽�û
#define ABILITYINDEX_PARTY	403	//����â��
#define ABILITYINDEX_GUILD	404	//����â��
#define ABILITYINDEX_EXCHANGE	405	//�ŷ�
#define ABILITYINDEX_STALL	406	//�����󰳼�


#define ABILITYINDEX_ITEMUPGRADE	407	//�����۵�޾�
#define ABILITYINDEX_ITEMMIX		408	//����������
#define ABILITYINDEX_ITEMREINFORCE	409	//�����۰�ȭ
#define ABILITYINDEX_ITEMDISSOLVE	410	//�����ۺ���
#define ABILITYINDEX_LEADERSHIP		411	//������
#define	ABILITYINDEX_RESTRAINT		412	// ����

/*DB �۾���
#define ABILITYINDEX_PETSTATE		501	//���������
#define ABILITYINDEX_PETINVEN		502	//���κ��丮
#define ABILITYINDEX_PETSKILL		503	//�꽺ų
#define ABILITYINDEX_PETREST		504	//���޽Ļ���
#define ABILITYINDEX_PETSEAL		505	//�����
*/
//�ӽ�
#define ABILITYINDEX_PETSTATE		450	//���������
#define ABILITYINDEX_PETINVEN		451	//���κ��丮
#define ABILITYINDEX_PETSKILL		452	//�꽺ų
#define ABILITYINDEX_PETREST		453	//���޽Ļ���
#define ABILITYINDEX_PETSEAL		454	//�����

#define ABILITYINDEX_TACTIC_HWA		122	//����-����(�ּ�)
#define ABILITYINDEX_TACTIC_GEUK	128	//����-����(�ּ�)
#define ABILITYINDEX_TACTIC_MUSSANG	129 //����


#endif
