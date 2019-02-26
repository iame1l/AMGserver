#ifndef __PLATLOCK_H__
#define __PLATLOCK_H__

#include <WinBase.h>

class AutoPlatLock
{
public:
    AutoPlatLock(CRITICAL_SECTION*section) :m_section(section)
    {
        if (m_section != NULL) EnterCriticalSection(m_section);
    }
    ~AutoPlatLock()
    {
        if (m_section != NULL) LeaveCriticalSection(m_section);
    }
private:
    AutoPlatLock(const AutoPlatLock&);
    AutoPlatLock& operator = (const AutoPlatLock&);
private:
    CRITICAL_SECTION*m_section;
};

#endif // __PLATLOCK_H__