#include "stdafx.h"
#include "BonBaseClass.h"




CBonBaseClass::CBonBaseClass()
	: m_pTracer(NULL)
{
}


CBonBaseClass::~CBonBaseClass()
{
}


void CBonBaseClass::SetTracer(CTracer *pTracer)
{
	m_pTracer=pTracer;
}


void CBonBaseClass::Trace(LPCTSTR pszOutput, ...)
{
	va_list Args;

	va_start(Args,pszOutput);
	if (m_pTracer!=NULL)
		m_pTracer->TraceV(pszOutput,Args);
	va_end(Args);
}
