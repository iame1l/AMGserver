#pragma once
#include <map>
#include "HNString.h"
#include "ExtensionObject.h"

class CCfg
	: public Extension
{
	typedef std::map<tstring, int>		MSTR_INT;
public:
	CCfg() {}
public:
	void SetValue(LPCTSTR lpszKey, int nValue);
	int GetValue(LPCTSTR lpszKey, int nDefault = 0);
	void Clear()
	{
		m_msi.clear();
	}
public:
	MSTR_INT	m_msi;
};