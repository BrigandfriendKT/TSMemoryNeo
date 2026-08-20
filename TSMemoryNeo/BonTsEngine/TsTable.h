// TsTable.h: TSテーブルラッパークラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include <vector>
#include "MediaData.h"
#include "TsStream.h"
#include "TsDescriptor.h"


using std::vector;


/////////////////////////////////////////////////////////////////////////////
// PSIテーブル抽象化クラス
/////////////////////////////////////////////////////////////////////////////
/*
class CPsiTable		// 保留： 実際のユースケースを判断した上で仕様を決める必要あり
{
public:
	CPsiTable();
	CPsiTable(const CPsiTable &Operand);

	CPsiTable & operator = (const CPsiTable &Operand);

	const bool StoreSection(const CPsiSection *pSection, bool *pbUpdate = NULL);

	const WORD GetExtensionNum(void) const;
	const bool GetExtension(const WORD wIndex, WORD *pwExtension) const;
	const bool GetSectionNum(const WORD wIndex, WORD *pwSectionNum) const;
	const CMediaData * GetSectionData(const WORD wIndex = 0U, const BYTE bySectionNo = 0U) const;

	void Reset(void);

protected:
	struct TAG_TABLEITEM
	{
		WORD wTableIdExtension;				// テーブルID拡張
		WORD wSectionNum;					// セクション数
		BYTE byVersionNo;					// バージョン番号
		vector<CMediaData> SectionArray;	// セクションデータ
	};

	vector<TAG_TABLEITEM> m_TableArray;		// テーブル
};
*/

/////////////////////////////////////////////////////////////////////////////
// PSIシングルテーブル抽象化クラス
/////////////////////////////////////////////////////////////////////////////

class CPsiSingleTable :	public CTsPidMapTarget,
						public CPsiSectionParser::IPsiSectionHandler
{
public:
	CPsiSingleTable(const bool bTargetSectionExt = true);
	CPsiSingleTable(const CPsiSingleTable &Operand);
	virtual ~CPsiSingleTable();

	CPsiSingleTable & operator = (const CPsiSingleTable &Operand);

// CTsPidMapTarget
	virtual const bool StorePacket(const CTsPacket *pPacket);
	virtual void OnPidUnmapped(const WORD wPID);

// CPsiSingleTable
	virtual void Reset(void);

	const DWORD GetCrcErrorCount(void) const;

	CPsiSection m_CurSection;

protected:
	virtual void OnPsiSection(const CPsiSectionParser *pPsiSectionParser, const CPsiSection *pSection);
	virtual const bool OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection);

private:
	CPsiSectionParser m_PsiSectionParser;
	bool m_bTableUpdated;
};

/////////////////////////////////////////////////////////////////////////////
// 何も処理がないテーブル抽象化クラス(主にAdaptationField処理用)
// PSIテーブルとして処理するべきではないかもしれないが、流れ上ここに記述
/////////////////////////////////////////////////////////////////////////////
class CPsiNullTable :	public CTsPidMapTarget
{
public:
	CPsiNullTable();
	CPsiNullTable(const CPsiNullTable &Operand);
	virtual ~CPsiNullTable();

	CPsiNullTable & operator = (const CPsiNullTable &Operand);	

// CTsPidMapTarget
	virtual const bool StorePacket(const CTsPacket *pPacket) = 0;
	virtual void OnPidUnmapped(const WORD wPID);

// CPsiNullTable

};

/////////////////////////////////////////////////////////////////////////////
// PSIテーブルセット抽象化クラス
/////////////////////////////////////////////////////////////////////////////
/*
class CPsiTableSuite	// 保留： 実際のユースケースを判断した上で仕様を決める必要あり
{
public:
	CPsiTableSuite();
	CPsiTableSuite(const CPsiTableSuite &Operand);

	CPsiTableSuite & operator = (const CPsiTableSuite &Operand);

	const bool StorePacket(const CTsPacket *pPacket);

	void SetTargetSectionExt(const bool bTargetExt);
	const bool AddTable(const BYTE byTableID);

	const WORD GetIndexByID(const BYTE byTableID);
	const CPsiTable * GetTable(const WORD wIndex = 0U) const;
	const CMediaData * GetSectionData(const WORD wIndex = 0U, const WORD wSubIndex = 0U, const BYTE bySectionNo = 0U) const;

	const DWORD GetCrcErrorCount(void) const;
	void Reset(void);

protected:
	static void CALLBACK StoreSection(const CPsiSection *pSection, const PVOID pParam);

	struct TAG_TABLESET
	{
		BYTE byTableID;						// テーブルID
		CPsiTable PsiTable;					// テーブル
	};

	vector<TAG_TABLESET> m_TableSet;		// テーブルセット

	bool m_bTargetSectionExt;
	bool m_bTableUpdated;

private:
	CPsiSectionParser m_PsiSectionParser;
};
*/

/////////////////////////////////////////////////////////////////////////////
// PATテーブル抽象化クラス
/////////////////////////////////////////////////////////////////////////////

class CPatTable : public CPsiSingleTable
{
public:
	CPatTable();
	CPatTable(const CPatTable &Operand);

	CPatTable & operator = (const CPatTable &Operand);

// CPsiSingleTable
	virtual void Reset(void);

// CPatTable
	const WORD GetTransportStreamID(void) const;

	const WORD GetNitPID(const WORD wIndex = 0U) const;
	const WORD GetNitNum(void) const;

	const WORD GetPmtPID(const WORD wIndex = 0U) const;
	const WORD GetProgramID(const WORD wIndex = 0U) const;
	const WORD GetProgramNum(void) const;

	const bool IsPmtTablePID(const WORD wPID) const;

protected:
	virtual const bool OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection);

	struct TAG_PATITEM
	{
		WORD wProgramID;	// 放送番組番号ID
		WORD wPID;			// PMTのPID
	};

	vector<WORD> m_NitPIDArray;
	vector<TAG_PATITEM> m_PmtPIDArray;
};


/////////////////////////////////////////////////////////////////////////////
// CATテーブル抽象化クラス
/////////////////////////////////////////////////////////////////////////////

class CCatTable : public CPsiSingleTable
{
public:
	CCatTable();
	virtual ~CCatTable(void);
	CCatTable(const CCatTable &Operand);

	CCatTable  & operator = (const CCatTable &Operand);

// CPsiSingleTable
	virtual void Reset(void);

// CCatTable
	const CDescBlock * GetCatDesc() const;

protected:
	virtual const bool OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection);

	CDescBlock m_DescBlock;
};


/////////////////////////////////////////////////////////////////////////////
// PMTテーブル抽象化クラス
/////////////////////////////////////////////////////////////////////////////

class CPmtTable : public CPsiSingleTable
{
public:
	CPmtTable();
	CPmtTable(const CPmtTable &Operand);

	CPmtTable & operator = (const CPmtTable &Operand);

// CPsiSingleTable
	virtual void Reset(void);

// CPmtTable
	const WORD GetProgramNumberID(void) const;

	const WORD GetPcrPID(void) const;
	const CDescBlock * GetTableDesc(void) const;
	const WORD GetEcmPID(void) const;

	const WORD GetEsInfoNum(void) const;
	const BYTE GetStreamTypeID(const WORD wIndex) const;
	const WORD GetEsPID(const WORD wIndex) const;
	const CDescBlock * GetItemDesc(const WORD wIndex) const;

protected:
	virtual const bool OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection);

	struct TAG_PMTITEM
	{
		BYTE byStreamTypeID;			// Stream Type ID
		WORD wEsPID;					// Elementary Stream PID
		CDescBlock DescBlock;			// Stream ID Descriptor 他
	};

	vector<TAG_PMTITEM> m_EsInfoArray;

	WORD m_wPcrPID;						// PCR_PID
	CDescBlock m_TableDescBlock;		// Conditional Access Method Descriptor 他
};


/////////////////////////////////////////////////////////////////////////////
// SDTテーブル抽象化クラス
/////////////////////////////////////////////////////////////////////////////

class CSdtTable : public CPsiSingleTable
{
public:
	CSdtTable();
	CSdtTable(const CSdtTable &Operand);

	CSdtTable & operator = (const CSdtTable &Operand);

// CPsiSingleTable
	virtual void Reset(void);

// CSdtTable
	const WORD GetServiceNum(void) const;
	const WORD GetServiceIndexByID(const WORD wServiceID);
	const WORD GetServiceID(const WORD wIndex) const;
	const BYTE GetRunningStatus(const WORD wIndex) const;
	const bool GetFreeCaMode(const WORD wIndex) const;
	const CDescBlock * GetItemDesc(const WORD wIndex) const;

protected:
	virtual void OnPsiSection(const CPsiSectionParser *pPsiSectionParser, const CPsiSection *pSection);
	virtual const bool OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection);

	struct TAG_SDTITEM
	{
		WORD wServiceID;				// Service ID
		BYTE byRunningStatus;			// Running Status
		bool bFreeCaMode;				// Free CA Mode(true: CA / false: Free)
		CDescBlock DescBlock;			// Service Descriptor 他
	};

	vector<TAG_SDTITEM> m_ServiceInfoArray;
};


/////////////////////////////////////////////////////////////////////////////
// NITテーブル抽象化クラス
/////////////////////////////////////////////////////////////////////////////

class CNitTable : public CPsiSingleTable
{
public:
	CNitTable();
	CNitTable(const CNitTable &Operand);

	CNitTable & operator = (const CNitTable &Operand);

// CPsiSingleTable
	virtual void Reset(void);

// CNitTable
	const WORD GetNetworkID(void) const;
	const CDescBlock * GetNetworkDesc(void) const;
	const WORD GetTransportStreamNum(void) const;
	const WORD GetTransportStreamID(const WORD wIndex) const;
	const WORD GetOriginalNetworkID(const WORD wIndex) const;
	const CDescBlock * GetItemDesc(const WORD wIndex) const;

protected:
	virtual const bool OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection);

	struct TAG_NITITEM {
		WORD wTransportStreamID;
		WORD wOriginalNetworkID;
		CDescBlock DescBlock;
	};

	WORD m_wNetworkID;				// Network ID
	CDescBlock m_NetworkDescBlock;	// Network descriptor
	vector<TAG_NITITEM> m_TransportStreamArray;
};


/////////////////////////////////////////////////////////////////////////////
// H-EIT[p/f]テーブル抽象化クラス
/////////////////////////////////////////////////////////////////////////////

class CHEitTable : public CPsiSingleTable
{
public:
	CHEitTable();
	CHEitTable(const CHEitTable &Operand);

	CHEitTable & operator = (const CHEitTable &Operand);

// CPsiSingleTable
	virtual void Reset(void);

// CHEitTable
	const DWORD GetServiceNum(void) const;
	const int GetServiceIndexByID(WORD ServiceID) const;
	const WORD GetServiceID(DWORD Index) const;
	const WORD GetTransportStreamID(DWORD Index) const;
	const WORD GetOriginalNetworkID(DWORD Index) const;
	const WORD GetEventID(DWORD Index,DWORD EventIndex) const;
	const SYSTEMTIME *GetStartTime(DWORD Index,DWORD EventIndex) const;
	const DWORD GetDuration(DWORD Index,DWORD EventIndex) const;
	const BYTE GetRunningStatus(DWORD Index,DWORD EventIndex) const;
	const bool GetFreeCaMode(DWORD Index,DWORD EventIndex) const;
	const CDescBlock * GetItemDesc(DWORD Index,DWORD EventIndex) const;

protected:
	virtual const bool OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection);

	struct EventInfo {
		WORD EventID;
		SYSTEMTIME StartTime;
		DWORD Duration;
		BYTE RunningStatus;
		bool FreeCaMode;
		CDescBlock DescBlock;
	};

	struct HEitInfo {
		WORD ServiceID;
		WORD TransportStreamID;
		WORD OriginalNetworkID;
		EventInfo EventList[2];
	};

	vector<HEitInfo> m_EitArray;
};


/////////////////////////////////////////////////////////////////////////////
// PCR抽象化クラス
// 元々Demuxの箇所にあったものだが使ってないようだったので、Table側に移動
// 現時点で使えるものとは言い難い
/////////////////////////////////////////////////////////////////////////////

class CPcrTable : public CPsiNullTable
{
public:
	CPcrTable(WORD wServiceIndex);
	CPcrTable(const CPcrTable &Operand);

	CPcrTable & operator = (const CPcrTable &Operand);

// CPsiNullTable
	virtual const bool StorePacket(const CTsPacket *pPacket);

// CPcrTable
	void AddServiceIndex(WORD wServiceIndex);
	const WORD GetServiceIndex(WORD *pwServiceIndex, WORD wIndex=0);
	const unsigned __int64 GetPcrTimeStamp();

protected:
	vector<WORD> m_ServiceIndex;
	unsigned __int64 m_ui64_Pcr;
};
