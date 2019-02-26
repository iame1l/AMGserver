#pragma once
#include "ExtensionObject.h"

#define DECLARE_ATTRIBUTE_METHOD( type, name )	\
	protected:	\
	type m_##name;	\
	public:	\
		type Get##name()	{ return m_##name; }	\
		void Set##name( type arg ) { m_##name = arg; }


class CGameStation
	: public Extension
{
public:
	CGameStation()
	{
		Clear();
	}
public:
	DECLARE_ATTRIBUTE_METHOD( int, OutCardPeople );
	DECLARE_ATTRIBUTE_METHOD( int, NtPeople );
	DECLARE_ATTRIBUTE_METHOD( int, BigOutPeople );
public:
	void Clear();
};
