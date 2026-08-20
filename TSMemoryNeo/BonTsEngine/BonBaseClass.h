#ifndef BON_BASE_CLASS_H
#define BON_BASE_CLASS_H


#include "Exception.h"
#include "TsUtilClass.h"


class CBonBaseClass : public CBonErrorHandler
{
	CTracer *m_pTracer;
public:
	CBonBaseClass();
	virtual ~CBonBaseClass();
	virtual void SetTracer(CTracer *pTracer);
protected:
	void Trace(LPCTSTR pszOutput, ...);
};


#endif
