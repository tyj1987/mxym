// GridSystem.h: interface for the CGridSystem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GRIDSYSTEM_H__C000AFC9_8D29_4467_9118_DF283166EDA1__INCLUDED_)
#define AFX_GRIDSYSTEM_H__C000AFC9_8D29_4467_9118_DF283166EDA1__INCLUDED_
#include "..\[CC]Header\CommonStruct.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CObject;
class CGridTable;
class CPackedData;
class CBattle;
struct GridPosition;


enum 
{
	FIND_CLOSE=0, 
	FIND_FIRST,
};
class CGridSystem
{
//	int m_nMaxLength;
	CYHHashTable<CObject> m_TempAddList;
//	CGridTable* m_pGridTable;

	CYHHashTable<CGridTable> m_GridTable;

	void RealAddObject(CObject* pObject,VECTOR3* pPos);

public:
	CGridTable* GetGridTable(CObject * pObject);

	GETINSTANCE(CGridSystem)

	CGridSystem();
	virtual ~CGridSystem();

	void Init();
	void Release();
	
	void CreateGridTable(DWORD GridID, BYTE Kind, int MaxWidth, int MaxHeight, MAPTYPE MapNum);
	void AddGridTable(DWORD ID, BYTE Kind);
	void DeleteGridTable(DWORD GridID);

	void CalcGridPosition(float x,float z,GridPosition* pOutGridPos);

	void GridProcess();		// ӽ Ʈ س ༮ ˻ Init ༮鸸 Addش.

	void ChangeGrid(CObject* pObject,VECTOR3* pPos);

	void AddObject(CObject* pObject,VECTOR3* pPos);
	void AddObject(CObject* pObject,VECTOR3* pPos, DWORD GridID);
	void RemoveObject(CObject* pObject);

	MAPTYPE GetGridMapNum(DWORD GridID);

	
	// FALSE ȯϸ ׸尡  
	// TRUE ȯϸ dwChangeTimeĿ ̵ ׸  ־ Ѵ.
	BOOL GetGridChangeTime(CObject* pObject,VECTOR3* pStartPos,VECTOR3* pTargetPos,float MoveSpeed,DWORD& dwChangeTime);


	//////////////////////////////////////////////////////////////////////////
	// Ÿ ۿ
	void AddGridToPackedData(GridPosition* pGridPosition,CPackedData* pPackedData, CObject* pObject);
	void AddGridToPackedDataWithoutOne(GridPosition* pGridPosition,DWORD dwID,CPackedData* pPackedData, CObject* pObject);

	void AddGridToPackedDataExceptLastGrid(GridPosition* pGridPosition,DWORD ExceptID,CPackedData* pPackedData, CObject* pObject);
	void AddLastGridToPackedDataExceptCurrentGrid(GridPosition* pGridPosition,DWORD ExceptID,CPackedData* pPackedData, CObject* pObject);	



	//////////////////////////////////////////////////////////////////////////
	// AI ֺ  ִ ȿ    ÷̾ ã
	CObject* FindPlayerInRange(VECTOR3* pPos,float Range, CObject* pObject,BYTE flag=FIND_CLOSE);
	CObject* FindMonsterInRange(VECTOR3* pPos,float Range, CObject* pObject,BYTE flag=FIND_CLOSE);

	//////////////////////////////////////////////////////////////////////////
	/// 06. 08. 2  - ̿
	/// ȿ׸峻  Ÿ 
	CObject* FindPlayerRandom(CObject* pObject);
	//////////////////////////////////////////////////////////////////////////

};

#endif // !defined(AFX_GRIDSYSTEM_H__C000AFC9_8D29_4467_9118_DF283166EDA1__INCLUDED_)
