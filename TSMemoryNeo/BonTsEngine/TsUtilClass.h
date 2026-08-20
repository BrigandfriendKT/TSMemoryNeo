// TsUtilClass.h: TSユーティリティークラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// ダイナミックリファレンス管理ベースクラス
/////////////////////////////////////////////////////////////////////////////

class CDynamicReferenceable
{
public:
	CDynamicReferenceable();
	virtual ~CDynamicReferenceable();

	void AddRef(void);
	void ReleaseRef(void);
	DWORD GetRefCount(void) const;

private:
	DWORD m_dwRefCount;
};


/////////////////////////////////////////////////////////////////////////////
// クリティカルセクションラッパークラス
/////////////////////////////////////////////////////////////////////////////

class CCriticalLock
{
public:
	CCriticalLock();
	virtual ~CCriticalLock();

	void Lock(void);
	void Unlock(void);
	bool TryLock(DWORD TimeOut=0);
private:
	CRITICAL_SECTION m_CriticalSection;
};


/////////////////////////////////////////////////////////////////////////////
// ブロックスコープロッククラス
/////////////////////////////////////////////////////////////////////////////

class CBlockLock
{
public:
	CBlockLock(CCriticalLock *pCriticalLock);
	virtual ~CBlockLock();
		
private:
	CCriticalLock *m_pCriticalLock;
};

class CTryBlockLock
{
public:
	CTryBlockLock(CCriticalLock *pCriticalLock);
	bool TryLock(DWORD TimeOut=0);
	~CTryBlockLock();
private:
	CCriticalLock *m_pCriticalLock;
	bool m_bLocked;
};


/////////////////////////////////////////////////////////////////////////////
// イベントクラス
/////////////////////////////////////////////////////////////////////////////

class CLocalEvent
{
	HANDLE m_hEvent;
public:
	CLocalEvent();
	~CLocalEvent();
	bool Create(bool bManual = false, bool bInitialState = false);
	bool IsCreated() const;
	void Close();
	bool Set();
	bool Reset();
	DWORD Wait(DWORD Timeout = INFINITE);
	bool IsSignaled();
};


/////////////////////////////////////////////////////////////////////////////
// トレースクラス
/////////////////////////////////////////////////////////////////////////////

class CTracer
{
	TCHAR m_szBuffer[256];
public:
	virtual ~CTracer() {}
	void Trace(LPCTSTR pszOutput, ...);
	void TraceV(LPCTSTR pszOutput,va_list Args);
protected:
	virtual void OnTrace(LPCTSTR pszOutput)=0;
};


/////////////////////////////////////////////////////////////////////////////
// CRC計算クラス
/////////////////////////////////////////////////////////////////////////////

class CCrcCalculator
{
public:
	static WORD CalcCrc16(const BYTE *pData, DWORD DataSize, WORD wCurCrc = 0xFFFF);
	static DWORD CalcCrc32(const BYTE *pData, DWORD DataSize, DWORD dwCurCrc = 0xFFFFFFFFUL);
};
