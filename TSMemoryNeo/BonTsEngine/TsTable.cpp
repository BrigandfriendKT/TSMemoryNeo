// TsTable.cpp: TSテーブルラッパークラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TsTable.h"
#include "TsEncode.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CPsiTableクラスの構築/消滅
//////////////////////////////////////////////////////////////////////
/*
CPsiTable::CPsiTable()
{
	Reset();
}

CPsiTable::CPsiTable(const CPsiTable &Operand)
{
	*this = Operand;
}

CPsiTable & CPsiTable::operator = (const CPsiTable &Operand)
{
	m_TableArray = Operand.m_TableArray;

	return *this;
}

const bool CPsiTable::StoreSection(const CPsiSection *pSection, bool *pbUpdate)
{
	bool bUpdate = false;

	// セクションのサイズをチェック
	if(!pSection->GetPayloadSize()){
		if(pbUpdate)*pbUpdate = bUpdate;
		return false;
		}

	// カレントネクストインジケータチェック(本来は別バンクに貯めるべき？)
	if(!pSection->IsCurrentNext()){
		if(pbUpdate)*pbUpdate = bUpdate;
		return true;
		}

	// テーブルID拡張を検索する
	DWORD dwIndex;

	for(dwIndex = 0UL ; dwIndex < m_TableArray.size() ; dwIndex++){
		if(m_TableArray[dwIndex].wTableIdExtension == pSection->GetTableIdExtension())break;
		}

	if(dwIndex >= m_TableArray.size()){
		// テーブルID拡張が見つからない、テーブル追加
		m_TableArray.resize(dwIndex + 1);
		m_TableArray[dwIndex].wTableIdExtension = pSection->GetTableIdExtension();
		m_TableArray[dwIndex].wSectionNum = (WORD)pSection->GetLastSectionNumber() + 1U;
		m_TableArray[dwIndex].byVersionNo = pSection->GetVersionNo();
		m_TableArray[dwIndex].SectionArray.resize(m_TableArray[dwIndex].wSectionNum);
		bUpdate = true;
		}
	else if(m_TableArray[dwIndex].byVersionNo != pSection->GetVersionNo()){
		// バージョンが不一致、テーブルが更新された
		m_TableArray[dwIndex].wSectionNum = (WORD)pSection->GetLastSectionNumber() + 1U;
		m_TableArray[dwIndex].byVersionNo = pSection->GetVersionNo();
		m_TableArray[dwIndex].SectionArray.clear();
		m_TableArray[dwIndex].SectionArray.resize(m_TableArray[dwIndex].wSectionNum);
		bUpdate = true;
		}
	else if(m_TableArray[dwIndex].wSectionNum != ((WORD)pSection->GetLastSectionNumber() + 1U)){
		// セクション数が変化した(例外？)
		m_TableArray[dwIndex].wSectionNum = (WORD)pSection->GetLastSectionNumber() + 1U;
		m_TableArray[dwIndex].SectionArray.resize(m_TableArray[dwIndex].wSectionNum);
		}

	// セクションデータを更新する
	m_TableArray[dwIndex].SectionArray[pSection->GetSectionNumber()].SetData(pSection->GetPayloadData(), pSection->GetPayloadSize());

	// 更新情報設定
	if(pbUpdate)*pbUpdate = bUpdate;

	return true;
}

const WORD CPsiTable::GetExtensionNum(void) const
{
	// テーブルの数を返す
	return m_TableArray.size();
}

const bool CPsiTable::GetExtension(const WORD wIndex, WORD *pwExtension) const
{
	if(wIndex >= GetExtensionNum())return false;

	// テーブルID拡張を返す
	*pwExtension = m_TableArray[wIndex].wTableIdExtension;

	return true;
}

const bool CPsiTable::GetSectionNum(const WORD wIndex, WORD *pwSectionNum) const
{
	if(wIndex >= GetExtensionNum())return false;

	// テーブルID拡張を返す
	*pwSectionNum = m_TableArray[wIndex].wSectionNum;

	return true;
}

const CMediaData * CPsiTable::GetSectionData(const WORD wIndex, const BYTE bySectionNo) const
{
	if(wIndex >= GetExtensionNum())return NULL;
	if(bySectionNo > m_TableArray[wIndex].wSectionNum)return NULL;

	// セクションデータを返す
	return &m_TableArray[wIndex].SectionArray[bySectionNo];
}

void CPsiTable::Reset(void)
{
	// 全てのテーブルを削除する
	m_TableArray.clear();
}
*/


/////////////////////////////////////////////////////////////////////////////
// PSIシングルテーブル抽象化クラス
/////////////////////////////////////////////////////////////////////////////

CPsiSingleTable::CPsiSingleTable(const bool bTargetSectionExt)
	: CTsPidMapTarget()
	, m_PsiSectionParser(this, bTargetSectionExt)
	, m_bTableUpdated(false)
{

}

CPsiSingleTable::CPsiSingleTable(const CPsiSingleTable &Operand)
	: CTsPidMapTarget()
	, m_PsiSectionParser(this)
{
	// コピーコンストラクタ
	*this = Operand;
}

CPsiSingleTable::~CPsiSingleTable()
{

}

CPsiSingleTable & CPsiSingleTable::operator = (const CPsiSingleTable &Operand)
{
	// インスタンスのコピー
	m_CurSection = Operand.m_CurSection;
	m_PsiSectionParser = Operand.m_PsiSectionParser;
	m_bTableUpdated = Operand.m_bTableUpdated;

	return *this;
}

const bool CPsiSingleTable::StorePacket(const CTsPacket *pPacket)
{
	if(!pPacket)return false;

	m_bTableUpdated = false;

	// パケットストア
	m_PsiSectionParser.StorePacket(pPacket);
	
	return m_bTableUpdated;
}

void CPsiSingleTable::OnPidUnmapped(const WORD wPID)
{
	// インスタンスを開放する
	delete this;
}

void CPsiSingleTable::Reset(void)
{
	// 状態初期化
	m_PsiSectionParser.Reset();
	m_CurSection.Reset();

	m_bTableUpdated = false;
}

const DWORD CPsiSingleTable::GetCrcErrorCount(void) const
{
	// CRCエラーカウントを取得する
	return m_PsiSectionParser.GetCrcErrorCount();
}

const bool CPsiSingleTable::OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection)
{
	// デフォルトの実装では何もしない
	return true;
}

void CPsiSingleTable::OnPsiSection(const CPsiSectionParser *pPsiSectionParser, const CPsiSection *pSection)
{
	// セクションのフィルタリングを行う場合は派生クラスでオーバーライドする
	// デフォルトの実装ではセクションペイロード更新時に仮想関数に通知する
	if(!(*pSection == m_CurSection)){
		// セクションが更新された
		if(OnTableUpdate(pSection, &m_CurSection)){
			// セクションストア
			m_CurSection = *pSection;
			m_bTableUpdated = true;
			}
		}
}


/////////////////////////////////////////////////////////////////////////////
// PSIシングルテーブル抽象化クラス
/////////////////////////////////////////////////////////////////////////////

CPsiNullTable::CPsiNullTable()
	: CTsPidMapTarget()
{

}

CPsiNullTable::CPsiNullTable(const CPsiNullTable &Operand)
	: CTsPidMapTarget()
{
	// コピーコンストラクタ
	*this = Operand;
}

CPsiNullTable::~CPsiNullTable()
{

}

CPsiNullTable & CPsiNullTable::operator = (const CPsiNullTable &Operand)
{
	// インスタンスのコピー

	return *this;
}

/* 純粋仮想関数として実装
const bool CPsiNullTable::StorePacket(const CTsPacket *pPacket)
{
	if(!pPacket)return false;
	return true;
}
*/

void CPsiNullTable::OnPidUnmapped(const WORD wPID)
{
	// インスタンスを開放する
	delete this;
}


/////////////////////////////////////////////////////////////////////////////
// PSIテーブルセット抽象化クラス
/////////////////////////////////////////////////////////////////////////////
/*
CPsiTableSuite::CPsiTableSuite()
	: m_bTargetSectionExt(true)
{
	m_PsiSectionParser.SetRecvCallback(m_bTargetSectionExt, CPsiTableSuite::StoreSection, this);

	Reset();
}

CPsiTableSuite::CPsiTableSuite(const CPsiTableSuite &Operand)
{
	*this = Operand;
}

CPsiTableSuite & CPsiTableSuite::operator = (const CPsiTableSuite &Operand)
{
	m_TableSet = Operand.m_TableSet;
	m_bTargetSectionExt = Operand.m_bTargetSectionExt;
	m_bTableUpdated = Operand.m_bTableUpdated;
	m_PsiSectionParser = Operand.m_PsiSectionParser;
	
	m_PsiSectionParser.SetRecvCallback(m_bTargetSectionExt, CPsiTableSuite::StoreSection, this);

	return *this;
}

const bool CPsiTableSuite::StorePacket(const CTsPacket *pPacket)
{
	m_bTableUpdated = false;
	
	// PSIセクションをテーブルに追加する
	m_PsiSectionParser.StorePacket(pPacket);

	return m_bTableUpdated;
}

void CPsiTableSuite::SetTargetSectionExt(const bool bTargetExt)
{
	// 処理対象とするセクションの種類を設定する(拡張 or 標準)
	m_bTargetSectionExt = bTargetExt;
	m_PsiSectionParser.SetRecvCallback(bTargetExt, CPsiTableSuite::StoreSection, this);
}

const bool CPsiTableSuite::AddTable(const BYTE byTableID)
{
	const WORD wNum = m_TableSet.size();

	// 既存のIDをチェック
	if(GetIndexByID(byTableID) < wNum)return false;

	// 最後尾に要素を追加
	m_TableSet.resize(wNum + 1U);

	// IDをセット
	m_TableSet[wNum].byTableID = byTableID;

	return true;
}

const WORD CPsiTableSuite::GetIndexByID(const BYTE byTableID)
{
	// 既存のテーブルIDを検索する
	for(WORD wIndex = 0U ; wIndex < m_TableSet.size() ; wIndex++){
		if(m_TableSet[wIndex].byTableID == byTableID)return wIndex;
		}

	// エラー時は常にテーブルの最大数より大きいインデックスを返す
	return 0x0100U;
}

const CPsiTable * CPsiTableSuite::GetTable(const WORD wIndex) const
{
	// テーブルを返す
	return (wIndex < m_TableSet.size())? &m_TableSet[wIndex].PsiTable : NULL;
}

const CMediaData * CPsiTableSuite::GetSectionData(const WORD wIndex, const WORD wSubIndex, const BYTE bySectionNo) const
{
	// セクションデータを返す
	const CPsiTable *pPsiTable = GetTable(wIndex);

	// テーブルが見つからない
	if(!pPsiTable)return NULL;

	// データを返す
	return pPsiTable->GetSectionData(wSubIndex, bySectionNo);
}

void CPsiTableSuite::Reset(void)
{
	m_bTableUpdated = false;

	m_PsiSectionParser.Reset();
	m_TableSet.clear();
}

const DWORD CPsiTableSuite::GetCrcErrorCount(void) const
{
	return m_PsiSectionParser.GetCrcErrorCount();
}

void CALLBACK CPsiTableSuite::StoreSection(const CPsiSection *pSection, const PVOID pParam)
{
	CPsiTableSuite *pThis = static_cast<CPsiTableSuite *>(pParam);

	// 対象外のセクションは処理しない
	if(pSection->IsExtendedSection() != pThis->m_bTargetSectionExt)return;

	// テーブルIDを検索
	const WORD wIndex = pThis->GetIndexByID(pSection->GetTableID());

	// テーブルIDが見つからない
	if(wIndex >= pThis->m_TableSet.size())return;

	// テーブルにストアする
	bool bUpdate = false;
	pThis->m_TableSet[wIndex].PsiTable.StoreSection(pSection, &bUpdate);

	if(bUpdate)pThis->m_bTableUpdated = true;
}
*/


/////////////////////////////////////////////////////////////////////////////
// PATテーブル抽象化クラス
/////////////////////////////////////////////////////////////////////////////

CPatTable::CPatTable()
	: CPsiSingleTable()
{
	Reset();
}

CPatTable::CPatTable(const CPatTable &Operand)
{
	*this = Operand;
}

CPatTable & CPatTable::operator = (const CPatTable &Operand)
{
	if (&Operand != this) {
		CPsiSingleTable::operator = (Operand);

		m_NitPIDArray = Operand.m_NitPIDArray;
		m_PmtPIDArray = Operand.m_PmtPIDArray;
	}

	return *this;
}

void CPatTable::Reset(void)
{
	// 状態をクリアする
	CPsiSingleTable::Reset();

	m_NitPIDArray.clear();
	m_PmtPIDArray.clear();
}

const WORD CPatTable::GetTransportStreamID(void) const
{
	return m_CurSection.GetTableIdExtension();
}

const WORD CPatTable::GetNitPID(const WORD wIndex) const
{
	// NITのPIDを返す
	return (wIndex < m_NitPIDArray.size())? m_NitPIDArray[wIndex] : 0xFFFFU;	// 0xFFFFは未定義のPID
}

const WORD CPatTable::GetNitNum(void) const
{
	// NITの数を返す
	return (WORD)m_NitPIDArray.size();
}

const WORD CPatTable::GetPmtPID(const WORD wIndex) const
{
	// PMTのPIDを返す
	return (wIndex < m_PmtPIDArray.size())? m_PmtPIDArray[wIndex].wPID : 0xFFFFU;	// 0xFFFFは未定義のPID
}

const WORD CPatTable::GetProgramID(const WORD wIndex) const
{
	// Program Number IDを返す
	return (wIndex < m_PmtPIDArray.size())? m_PmtPIDArray[wIndex].wProgramID : 0x0000U;	// 0xFFFFは未定義のPID
}

const WORD CPatTable::GetProgramNum(void) const
{
	// PMTの数を返す
	return (WORD)m_PmtPIDArray.size();
}

const bool CPatTable::IsPmtTablePID(const WORD wPID) const
{
	// PMTのPIDかどうかを返す
	for (WORD wIndex = 0U ; wIndex < m_PmtPIDArray.size() ; wIndex++) {
		if (wPID == m_PmtPIDArray[wIndex].wPID)
			return true;
	}

	return false;
}

const bool CPatTable::OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection)
{
	const WORD wDataSize = pCurSection->GetPayloadSize();
	const BYTE *pHexData = pCurSection->GetPayloadData();

	if(wDataSize % 4U)return false;						// テーブルのサイズが不正
	if(pCurSection->GetTableID() != 0x00U)return false;	// テーブルIDが不正

	// PIDをクリアする
	m_NitPIDArray.clear();
	m_PmtPIDArray.clear();

	// テーブルを解析する

	TRACE(TEXT("\n------- PAT Table -------\nTS ID = %04X\n"), pCurSection->GetTableIdExtension());

	for (WORD wPos = 0 ; wPos < wDataSize ; wPos += 4, pHexData += 4) {
		TAG_PATITEM PatItem;

		PatItem.wProgramID	= ((WORD)pHexData[0] << 8) | (WORD)pHexData[1];				// +1,2
		PatItem.wPID		= ((WORD)(pHexData[2] & 0x1FU) << 8) | (WORD)pHexData[3];	// +3,4

		if (PatItem.wProgramID == 0) {
			// NITのPID
			TRACE(TEXT("NIT #%u [ID:%04X][PID:%04X]\n"), m_NitPIDArray.size(), PatItem.wProgramID, PatItem.wPID);
			m_NitPIDArray.push_back(PatItem.wPID);
		} else {
			// PMTのPID
			TRACE(TEXT("PMT #%u [ID:%04X][PID:%04X]\n"), m_PmtPIDArray.size(), PatItem.wProgramID, PatItem.wPID);
			m_PmtPIDArray.push_back(PatItem);
		}
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// CATテーブル抽象化クラス
/////////////////////////////////////////////////////////////////////////////

CCatTable::CCatTable()
	: CPsiSingleTable()
{
	Reset();
}

CCatTable::~CCatTable(void)
{
}

CCatTable  & CCatTable::operator = (const CCatTable &Operand)
{
	if (this != &Operand) {
		CPsiSingleTable::operator=(Operand);
		m_DescBlock = Operand.m_DescBlock;
	}
	return *this;
}

void CCatTable::Reset(void)
{
	// 状態をクリアする
	m_DescBlock.Reset();

	CPsiSingleTable::Reset();
}

const CDescBlock * CCatTable::GetCatDesc(void) const
{
	// 記述子領域を返す
	return &m_DescBlock;
}

const bool CCatTable::OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection)
{
	if (pCurSection->GetTableID() != 0x01U)
		return false;	// テーブルIDが不正

	TRACE(TEXT("\n------- CAT Table -------\n"));

	// テーブルを解析する
	m_DescBlock.ParseBlock(pCurSection->GetPayloadData(), pCurSection->GetPayloadSize());

#ifdef DEBUG
	for (WORD i = 0 ; i < m_DescBlock.GetDescNum() ; i++) {
		const CBaseDesc *pBaseDesc = m_DescBlock.GetDescByIndex(i);
		TRACE(TEXT("[%lu] TAG = 0x%02X LEN = %lu\n"), i, pBaseDesc->GetTag(), pBaseDesc->GetLength());
	}
#endif

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// PMTテーブル抽象化クラス
/////////////////////////////////////////////////////////////////////////////

CPmtTable::CPmtTable()
	: CPsiSingleTable()
{
	Reset();
}

CPmtTable::CPmtTable(const CPmtTable &Operand)
{
	*this = Operand;
}

CPmtTable & CPmtTable::operator = (const CPmtTable &Operand)
{
	if (this != &Operand) {
		CPsiSingleTable::operator=(Operand);
		m_wPcrPID = Operand.m_wPcrPID;
		m_TableDescBlock = Operand.m_TableDescBlock;
		m_EsInfoArray = Operand.m_EsInfoArray;
	}

	return *this;
}

void CPmtTable::Reset(void)
{
	// 状態をクリアする
	CPsiSingleTable::Reset();

	m_wPcrPID = 0xFFFFU;
	m_TableDescBlock.Reset();
	m_EsInfoArray.clear();
}

const WORD CPmtTable::GetProgramNumberID(void) const
{
	return m_CurSection.GetTableIdExtension();
}

const WORD CPmtTable::GetPcrPID(void) const
{
	// PCR_PID を返す
	return m_wPcrPID;
}

const CDescBlock * CPmtTable::GetTableDesc(void) const
{
	// テーブルの記述子ブロックを返す
	return &m_TableDescBlock;
}

const WORD CPmtTable::GetEcmPID(void) const
{
	// ECMのPIDを返す
	const CCaMethodDesc *pCaMethodDesc = dynamic_cast<const CCaMethodDesc *>(m_TableDescBlock.GetDescByTag(CCaMethodDesc::DESC_TAG));

	return (pCaMethodDesc)? pCaMethodDesc->GetCaPID() : 0xFFFFU;
}

const WORD CPmtTable::GetEsInfoNum(void) const
{
	// ES情報の数を返す
	return (WORD)m_EsInfoArray.size();
}

const BYTE CPmtTable::GetStreamTypeID(const WORD wIndex) const
{
	// Stream Type ID を返す
	return (wIndex < m_EsInfoArray.size())? m_EsInfoArray[wIndex].byStreamTypeID : 0x00U;	// 0x00は未定義のID
}

const WORD CPmtTable::GetEsPID(const WORD wIndex) const
{
	// Elementary Stream PID を返す
	return (wIndex < m_EsInfoArray.size())? m_EsInfoArray[wIndex].wEsPID : 0xFFFFU;			// 0xFFFFは未定義のPID
}

const CDescBlock * CPmtTable::GetItemDesc(const WORD wIndex) const
{
	// アイテムの記述子ブロックを返す
	return (wIndex < m_EsInfoArray.size())? &m_EsInfoArray[wIndex].DescBlock : NULL;
}

const bool CPmtTable::OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection)
{
	const WORD wDataSize = pCurSection->GetPayloadSize();
	const BYTE *pHexData = pCurSection->GetPayloadData();

	if (wDataSize < 4)
		return false;

	if(pCurSection->GetTableID() != 0x02U)return false;	// テーブルIDが不正

	// 状態をクリアする
	m_wPcrPID = 0xFFFFU;
	m_EsInfoArray.clear();

	// テーブルを解析する
	m_wPcrPID = ((WORD)(pHexData[0] & 0x1FU) << 8) | (WORD)pHexData[1];				// +0,1
	WORD wDescLen = ((WORD)(pHexData[2] & 0x0FU) << 8) | (WORD)pHexData[3];
	if (4 + wDescLen > wDataSize)
		return false;

	// 記述子ブロック
	m_TableDescBlock.ParseBlock(&pHexData[4], wDescLen);

	TRACE(TEXT("\n------- PMT Table -------\nProgram Number ID = %04X(%d)\nPCR PID = %04X\nECM PID = %04X\n"),
		pCurSection->GetTableIdExtension(), pCurSection->GetTableIdExtension(), m_wPcrPID , (m_TableDescBlock.GetDescByTag(CCaMethodDesc::DESC_TAG))? dynamic_cast<const CCaMethodDesc *>(m_TableDescBlock.GetDescByTag(CCaMethodDesc::DESC_TAG))->GetCaPID() : 0xFFFFU);

	// ストリーム情報を解析
	for (WORD wPos = wDescLen + 4 ; wPos + 5 <= wDataSize ; wPos += 5 + wDescLen) {
		TAG_PMTITEM PmtItem;

		PmtItem.byStreamTypeID = pHexData[wPos + 0];													// +0
		PmtItem.wEsPID = ((WORD)(pHexData[wPos + 1] & 0x1FU) << 8) | (WORD)pHexData[wPos + 2];			// +1,2	
		wDescLen = ((WORD)(pHexData[wPos + 3] & 0x0FU) << 8) | (WORD)pHexData[wPos + 4];				// +3,4
		if (wPos + 5 + wDescLen > wDataSize)
			break;

		// 記述子ブロック
		PmtItem.DescBlock.ParseBlock(&pHexData[wPos + 5], wDescLen);

		TRACE(TEXT("[%u] Stream Type ID = %02X  PID = %04X\n"), m_EsInfoArray.size(), PmtItem.byStreamTypeID, PmtItem.wEsPID);

		// テーブルに追加する
		m_EsInfoArray.push_back(PmtItem);
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// SDTテーブル抽象化クラス
/////////////////////////////////////////////////////////////////////////////

CSdtTable::CSdtTable()
	: CPsiSingleTable()
{

}

CSdtTable::CSdtTable(const CSdtTable &Operand)
{
	*this = Operand;
}

CSdtTable & CSdtTable::operator = (const CSdtTable &Operand)
{
	if (this != &Operand) {
		CPsiSingleTable::operator = (Operand);
		m_ServiceInfoArray = Operand.m_ServiceInfoArray;
	}

	return *this;
}

void CSdtTable::Reset(void)
{
	// 状態をクリアする
	CPsiSingleTable::Reset();

	m_ServiceInfoArray.clear();
}

const WORD CSdtTable::GetServiceNum(void) const
{
	// サービス数を返す
	return (WORD)m_ServiceInfoArray.size();
}

const WORD CSdtTable::GetServiceIndexByID(const WORD wServiceID)
{
	// サービスIDからインデックスを返す
	for (WORD wIndex = 0U ; wIndex < GetServiceNum() ; wIndex++) {
		if (m_ServiceInfoArray[wIndex].wServiceID == wServiceID) {
			return wIndex;
		}
	}

	// サービスIDが見つからない
	return 0xFFFFU;
}

const WORD CSdtTable::GetServiceID(const WORD wIndex) const
{
	// 	サービスIDを返す
	return (wIndex < GetServiceNum())? m_ServiceInfoArray[wIndex].wServiceID : 0xFFFFU;
}

const BYTE CSdtTable::GetRunningStatus(const WORD wIndex) const
{
	// Running Statusを返す
	return (wIndex < GetServiceNum())? m_ServiceInfoArray[wIndex].byRunningStatus : 0xFFU;
}

const bool CSdtTable::GetFreeCaMode(const WORD wIndex) const
{
	// Free CA Modeを返す
	return (wIndex < GetServiceNum())? m_ServiceInfoArray[wIndex].bFreeCaMode : false;
}

const CDescBlock * CSdtTable::GetItemDesc(const WORD wIndex) const
{
	// アイテムの記述子ブロックを返す
	return (wIndex < m_ServiceInfoArray.size())? &m_ServiceInfoArray[wIndex].DescBlock : NULL;
}

void CSdtTable::OnPsiSection(const CPsiSectionParser *pPsiSectionParser, const CPsiSection *pSection)
{
	if(pSection->GetTableID() != 0x42U)return;

	// Actual Streamだけを処理する
	CPsiSingleTable::OnPsiSection(pPsiSectionParser, pSection);
}

const bool CSdtTable::OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection)
{
	const WORD wDataSize = pCurSection->GetPayloadSize();
	const BYTE *pHexData = pCurSection->GetPayloadData();

	// 状態をクリアする
	m_ServiceInfoArray.clear();

	TRACE(TEXT("\n------- SDT Table -------\nTransport Stream ID = %04X\nOriginal Network ID = %02X%02X\n"), pCurSection->GetTableIdExtension(), pHexData[0], pHexData[1]);

	// テーブルを解析する
	WORD wDescLen;
	for (WORD wPos = 3 ; wPos + 5 <= wDataSize ; wPos += 5 + wDescLen) {
		TAG_SDTITEM SdtItem;

		SdtItem.wServiceID		= ((WORD)pHexData[wPos + 0] << 8) | (WORD)pHexData[wPos + 1];
		SdtItem.byRunningStatus	= pHexData[wPos + 3] >> 5;
		SdtItem.bFreeCaMode		= (pHexData[wPos + 3] & 0x10U)? true : false;

		// Service Descriptor
		wDescLen = ((WORD)(pHexData[wPos + 3] & 0x0FU) << 8) | (WORD)pHexData[wPos + 4];

		if (wPos + 5 + wDescLen > wDataSize)
			break;

		// 記述子ブロック
		SdtItem.DescBlock.ParseBlock(&pHexData[wPos + 5], wDescLen);

		// テーブル追加
		m_ServiceInfoArray.push_back(SdtItem);

#ifdef _DEBUG
		// デバッグ用ここから
		const CServiceDesc *pServiceDesc = dynamic_cast<const CServiceDesc *>(SdtItem.DescBlock.GetDescByTag(CServiceDesc::DESC_TAG));
		if (pServiceDesc) {
			TCHAR szServiceName[1024] = {TEXT('\0')};
			pServiceDesc->GetServiceName(szServiceName,1024);
			TRACE(TEXT("[%u] Service ID = %04X  Running Status = %01X  Free CA Mode = %u  Service Type = %02X  Service Name = %s\n"), m_ServiceInfoArray.size(), SdtItem.wServiceID, SdtItem.byRunningStatus, SdtItem.bFreeCaMode, pServiceDesc->GetServiceType(), szServiceName);
		} else {
			TRACE(TEXT("[%u] Service ID = %04X  Running Status = %01X  Free CA Mode = %u  ※サービス記述子なし\n"), m_ServiceInfoArray.size(), SdtItem.wServiceID, SdtItem.byRunningStatus, SdtItem.bFreeCaMode);
		}
		// ここまで
#endif
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// NITテーブル抽象化クラス
/////////////////////////////////////////////////////////////////////////////

CNitTable::CNitTable()
	: CPsiSingleTable()
{
	Reset();
}

CNitTable::CNitTable(const CNitTable &Operand)
{
	*this = Operand;
}

CNitTable & CNitTable::operator = (const CNitTable &Operand)
{
	if (this != &Operand) {
		CPsiSingleTable::operator = (Operand);
		m_wNetworkID = Operand.m_wNetworkID;
		m_NetworkDescBlock = Operand.m_NetworkDescBlock;
		m_TransportStreamArray = Operand.m_TransportStreamArray;
	}

	return *this;
}

void CNitTable::Reset(void)
{
	CPsiSingleTable::Reset();
	m_wNetworkID = 0xFFFF;
	m_NetworkDescBlock.Reset();
	m_TransportStreamArray.clear();
}

const WORD CNitTable::GetNetworkID(void) const
{
	return m_wNetworkID;
}

const CDescBlock * CNitTable::GetNetworkDesc(void) const
{
	return &m_NetworkDescBlock;
}

const WORD CNitTable::GetTransportStreamNum(void) const
{
	return (WORD)m_TransportStreamArray.size();
}

const WORD CNitTable::GetTransportStreamID(const WORD wIndex) const
{
	if (wIndex >= GetTransportStreamNum())
		return 0xFFFF;
	return m_TransportStreamArray[wIndex].wTransportStreamID;
}

const WORD CNitTable::GetOriginalNetworkID(const WORD wIndex) const
{
	if (wIndex >= GetTransportStreamNum())
		return 0xFFFF;
	return m_TransportStreamArray[wIndex].wOriginalNetworkID;
}

const CDescBlock * CNitTable::GetItemDesc(const WORD wIndex) const
{
	if (wIndex >= GetTransportStreamNum())
		return NULL;
	return &m_TransportStreamArray[wIndex].DescBlock;
}

const bool CNitTable::OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection)
{
	const WORD wDataSize = pCurSection->GetPayloadSize();
	const BYTE *pHexData = pCurSection->GetPayloadData();

	if (wDataSize < 2)
		return false;
	if (pCurSection->GetTableID() != 0x40)
		return false;

	m_TransportStreamArray.clear();

	m_wNetworkID = pCurSection->GetTableIdExtension();

	WORD DescLength = ((WORD)(pHexData[0] & 0x0F) << 8) | pHexData[1];
	WORD Pos = 2;
	if (Pos + DescLength > wDataSize)
		return false;
	m_NetworkDescBlock.ParseBlock(&pHexData[Pos], DescLength);
	Pos += DescLength;

	WORD StreamLoopLength = ((WORD)(pHexData[Pos] & 0x0F) << 8) | pHexData[Pos + 1];
	Pos += 2;
	if (Pos + StreamLoopLength > wDataSize)
		return false;
	for (WORD i = 0 ; i < StreamLoopLength ; i += 6 + DescLength) {
		TAG_NITITEM NitItem;

		if (Pos + 6 > wDataSize)
			return false;
		NitItem.wTransportStreamID = ((WORD)pHexData[Pos + 0] << 8) | pHexData[Pos + 1];
		NitItem.wOriginalNetworkID = ((WORD)pHexData[Pos + 2] << 8) | pHexData[Pos + 3];
		DescLength = ((WORD)(pHexData[Pos + 4] & 0x0F) << 8) | pHexData[Pos + 5];
		Pos += 6;
		if (Pos + DescLength > wDataSize)
			return false;
		NitItem.DescBlock.ParseBlock(&pHexData[Pos], DescLength);
		Pos += DescLength;
		m_TransportStreamArray.push_back(NitItem);
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// H-EIT[p/f]テーブル抽象化クラス
/////////////////////////////////////////////////////////////////////////////

CHEitTable::CHEitTable()
	: CPsiSingleTable()
{

}

CHEitTable::CHEitTable(const CHEitTable &Operand)
{
	*this = Operand;
}

CHEitTable & CHEitTable::operator = (const CHEitTable &Operand)
{
	if (this != &Operand) {
		CPsiSingleTable::operator = (Operand);
		m_EitArray = Operand.m_EitArray;
	}

	return *this;
}

void CHEitTable::Reset(void)
{
	// 状態をクリアする
	CPsiSingleTable::Reset();

	m_EitArray.clear();
}

const DWORD CHEitTable::GetServiceNum(void) const
{
	// サービス数を返す
	return (WORD)m_EitArray.size();
}

const int CHEitTable::GetServiceIndexByID(WORD ServiceID) const
{
	// サービスIDからインデックスを返す
	for (DWORD Index = 0 ; Index < GetServiceNum() ; Index++) {
		if (m_EitArray[Index].ServiceID == ServiceID) {
			return Index;
		}
	}

	// サービスIDが見つからない
	return -1;
}

const WORD CHEitTable::GetServiceID(DWORD Index) const
{
	// サービスIDを返す
	if (Index >= GetServiceNum())
		return 0xFFFF;
	return m_EitArray[Index].ServiceID;
}

const WORD CHEitTable::GetTransportStreamID(DWORD Index) const
{
	if (Index >= GetServiceNum())
		return 0;
	return m_EitArray[Index].TransportStreamID;
}

const WORD CHEitTable::GetOriginalNetworkID(DWORD Index) const
{
	if (Index >= GetServiceNum())
		return 0;
	return m_EitArray[Index].OriginalNetworkID;
}

const WORD CHEitTable::GetEventID(DWORD Index,DWORD EventIndex) const
{
	if (Index >= GetServiceNum() || EventIndex>1)
		return 0;
	return m_EitArray[Index].EventList[EventIndex].EventID;
}

const SYSTEMTIME *CHEitTable::GetStartTime(DWORD Index,DWORD EventIndex) const
{
	if (Index >= GetServiceNum() || EventIndex>1)
		return NULL;
	return &m_EitArray[Index].EventList[EventIndex].StartTime;
}

const DWORD CHEitTable::GetDuration(DWORD Index,DWORD EventIndex) const
{
	if (Index >= GetServiceNum() || EventIndex>1)
		return 0;
	return m_EitArray[Index].EventList[EventIndex].Duration;
}

const BYTE CHEitTable::GetRunningStatus(DWORD Index,DWORD EventIndex) const
{
	if (Index >= GetServiceNum() || EventIndex>1)
		return 0xFF;
	return m_EitArray[Index].EventList[EventIndex].RunningStatus;
}

const bool CHEitTable::GetFreeCaMode(DWORD Index,DWORD EventIndex) const
{
	// Free CA Modeを返す
	if (Index >= GetServiceNum() || EventIndex>1)
		return false;
	return m_EitArray[Index].EventList[EventIndex].FreeCaMode;
}

const CDescBlock * CHEitTable::GetItemDesc(DWORD Index,DWORD EventIndex) const
{
	// アイテムの記述子ブロックを返す
	if (Index >= GetServiceNum() || EventIndex>1)
		return NULL;
	return &m_EitArray[Index].EventList[EventIndex].DescBlock;
}

const bool CHEitTable::OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection)
{
	if (pCurSection->GetTableID()!=0x4E
			|| pCurSection->GetSectionNumber()>1
			|| pCurSection->GetLastSectionNumber()!=0x01)
		return false;

	const WORD wDataSize = pCurSection->GetPayloadSize();
	const BYTE *pHexData = pCurSection->GetPayloadData();

	if (wDataSize < 18)
		return false;

	WORD ServiceID, TransportStreamID, OriginalNetworkID;
	ServiceID = pCurSection->GetTableIdExtension();
	TransportStreamID = ((WORD)pHexData[0] << 8) | pHexData[1];
	OriginalNetworkID = ((WORD)pHexData[2] << 8) | pHexData[3];

	//TRACE(TEXT("------- H-EIT Table -------\nSID = %04X / TSID = %04X / ONID = %04X\n"),ServiceID,TransportStreamID,OriginalNetworkID);

	size_t Index = GetServiceIndexByID(ServiceID);
	if (Index < 0) {
		HEitInfo EitInfo;

		EitInfo.ServiceID = ServiceID;
		EitInfo.TransportStreamID = TransportStreamID;
		EitInfo.OriginalNetworkID = OriginalNetworkID;
		m_EitArray.push_back(EitInfo);
		Index=m_EitArray.size() - 1;
	}

	if (pHexData[4] != 0x01)	// segment_last_section_number
		return false;
	if (pHexData[5] != 0x4E)	// last_table_id
		return false;

	EventInfo &Info = m_EitArray[Index].EventList[pCurSection->GetSectionNumber()];

	Info.EventID = (pHexData[6] << 8) | pHexData[7];
	if (!CAribTime::AribToSystemTime(&pHexData[8], &Info.StartTime))
		::ZeroMemory(&Info.StartTime, sizeof(SYSTEMTIME));
	Info.Duration = CAribTime::AribBcdToSecond(&pHexData[13]);
	Info.RunningStatus = pHexData[16] >> 5;
	Info.FreeCaMode = (pHexData[16] & 0x10) != 0;
	WORD DescLength = (((WORD)pHexData[16] & 0x0F) << 8) | pHexData[17];
	if (DescLength > 0 && DescLength <= wDataSize - 18)
		Info.DescBlock.ParseBlock(&pHexData[18], DescLength);
	else
		Info.DescBlock.Reset();

	/*
	TRACE(TEXT("EventID = %02X / %04d/%d/%d %d:%02d %lu sec\n"),
		  Info.EventID,Info.StartTime.wYear,Info.StartTime.wMonth,Info.StartTime.wDay,Info.StartTime.wHour,Info.StartTime.wMinute,Info.Duration);
	*/

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// PCR抽象化クラス
/////////////////////////////////////////////////////////////////////////////

CPcrTable::CPcrTable(WORD wServiceIndex)
	: CPsiNullTable()
	, m_ui64_Pcr(0)
{
	m_ServiceIndex.push_back(wServiceIndex);
}

CPcrTable::CPcrTable(const CPcrTable &Operand)
{
	*this = Operand;
}

CPcrTable & CPcrTable::operator = (const CPcrTable &Operand)
{
	if (this != &Operand) {
		CPsiNullTable::operator = (Operand);
		m_ServiceIndex = Operand.m_ServiceIndex;
		m_ui64_Pcr = Operand.m_ui64_Pcr;
	}

	return *this;
}

const bool CPcrTable::StorePacket(const CTsPacket *pPacket)
{
	if(!pPacket)return false;
	
	if(pPacket->m_AdaptationField.bPcrFlag){
		m_ui64_Pcr = ((unsigned __int64)pPacket->m_AdaptationField.pOptionData[0] << 25 ) |
			((unsigned __int64)pPacket->m_AdaptationField.pOptionData[1] << 17 )|
			((unsigned __int64)pPacket->m_AdaptationField.pOptionData[2] << 9 )|
			((unsigned __int64)pPacket->m_AdaptationField.pOptionData[3] << 1 )|
			((unsigned __int64)pPacket->m_AdaptationField.pOptionData[4] >> 7 );
		}

	return true;
}

void CPcrTable::AddServiceIndex(WORD wServiceIndex)
{
	m_ServiceIndex.push_back(wServiceIndex);
}

const WORD CPcrTable::GetServiceIndex(WORD *pwServiceIndex, WORD wIndex)
{
	if (wIndex < m_ServiceIndex.size() && pwServiceIndex) {
		*pwServiceIndex = m_ServiceIndex[wIndex];
		return true;
	}
	return false;
}

const unsigned __int64 CPcrTable::GetPcrTimeStamp()
{
	return m_ui64_Pcr;
}
