#include "stdafx.h"
#include "Cfg.h"

void CCfg::SetValue(LPCTSTR lpszKey, int nValue)
{
	MSTR_INT::_Pairib ib = m_msi.insert( MSTR_INT::value_type(lpszKey, nValue) );
	if( !ib.second )
		ib.first->second = nValue;
}

int CCfg::GetValue(LPCTSTR lpszKey, int nDefault)
{
	int nResult = nDefault;

	MSTR_INT::iterator it = m_msi.find(lpszKey);
	if( m_msi.end() != it )
		nResult = it->second;

	return nResult;
}