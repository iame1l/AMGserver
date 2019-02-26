#include "stdafx.h"
#include "AIComFunction.h"
bool IsValidNumber(CString strCheck)
{
    const char *p = strCheck.GetBuffer();
    if(*p == '-') p++;
    bool rt = false;
    while(*p != 0)
    {
        rt = true;
        if(*p >= '0' && *p <= '9')
        {
            p++;
            continue;
        }
        rt = false;
        break;
    }
    return rt;
}

bool IsContestGame(int nRoomRule)
{
#ifdef GRR_CONTEST
    if (nRoomRule & GRR_CONTEST) return true;
#endif
#ifdef GRR_TIMINGCONTEST
    if (nRoomRule & GRR_TIMINGCONTEST) return true;
#endif
    return false;

}