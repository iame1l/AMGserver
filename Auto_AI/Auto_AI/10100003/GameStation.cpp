#include "stdafx.h"
#include "GameStation.h"
#include "def.h"

#define MOV_ATTRIBUTE( name, value )	\
	m_##name = (value)

void CGameStation::Clear()
{
	MOV_ATTRIBUTE( OutCardPeople, INVALID_INDEX );
	MOV_ATTRIBUTE( NtPeople, INVALID_INDEX );
	MOV_ATTRIBUTE( BigOutPeople, INVALID_INDEX );
}
