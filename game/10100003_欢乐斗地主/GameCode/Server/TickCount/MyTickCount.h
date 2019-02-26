#ifndef _ZZMJ_MY_TICKCOUNT_H_
#define _ZZMJ_MY_TICKCOUNT_H_

class CMyGetTickCount
{
public:
	CMyGetTickCount(void);
	~CMyGetTickCount(void);

	static void Init();

	static long long GetTime();	// ∫¡√Îº∂
};

#endif