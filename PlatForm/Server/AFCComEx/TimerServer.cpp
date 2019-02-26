

#include "stdafx.h"
/*#include "TimerServer.h"

#define GRANULARITY 10 //10ms
#define WHEEL_BITS1 8
#define WHEEL_BITS2 6
#define WHEEL_SIZE1 (1 << WHEEL_BITS1) //256
#define WHEEL_SIZE2 (1 << WHEEL_BITS2) //64
#define WHEEL_MASK1 (WHEEL_SIZE1 - 1)
#define WHEEL_MASK2 (WHEEL_SIZE2 - 1)
#define WHEEL_NUM 5


typedef struct stNodeLink {
	stNodeLink *prev;
	stNodeLink *next;
	stNodeLink() {prev = next = this;} //circle
}SNodeLink;
typedef struct stTimerNode {
	SNodeLink link;
	__int64 dead_time;
	CThreadTimer *timer;
	stTimerNode(CThreadTimer *t, __int64 dt) :  dead_time(dt), timer(t) {}
}STimerNode;
typedef struct stWheel {
	SNodeLink *spokes;
	uint32_t size;
	uint32_t spokeindex;
	stWheel(uint32_t n) : size(n), spokeindex(0){ 
		spokes = new SNodeLink[n];
	}
	~stWheel() { 
		/ ** clean ** /
	}
}SWheel;

SWheel *wheels_[WHEEL_NUM];

void CTimerManager::AddTimerNode(uint32_t milseconds, STimerNode *node) {
	SNodeLink *spoke = NULL;
	uint32_t interval = milseconds / GRANULARITY;
	uint32_t threshold1 = WHEEL_SIZE1;
	uint32_t threshold2 = 1 << (WHEEL_BITS1 + WHEEL_BITS2);
	uint32_t threshold3 = 1 << (WHEEL_BITS1 + 2 * WHEEL_BITS2);
	uint32_t threshold4 = 1 << (WHEEL_BITS1 + 3 * WHEEL_BITS2);

	if (interval < threshold1) {
		uint32_t index = (interval + wheels_[0]->spokeindex) & WHEEL_MASK1;
		spoke = wheels_[0]->spokes + index;
	} else if (interval < threshold2) {
		uint32_t index = ((interval - threshold1 + wheels_[1]->spokeindex * threshold1) >> WHEEL_BITS1) & WHEEL_MASK2;
		spoke = wheels_[1]->spokes + index;
	} else if (interval < threshold3) {
		uint32_t index = ((interval - threshold2 + wheels_[2]->spokeindex * threshold2) >> (WHEEL_BITS1 + WHEEL_BITS2)) & WHEEL_MASK2;
		spoke = wheels_[2]->spokes + index;
	} else if (interval < threshold4) {
		uint32_t index = ((interval - threshold3 + wheels_[3]->spokeindex * threshold3) >> (WHEEL_BITS1 + 2 * WHEEL_BITS2)) & WHEEL_MASK2;
		spoke = wheels_[3]->spokes + index;
	} else {
		uint32_t index = ((interval - threshold4 + wheels_[4]->spokeindex * threshold4) >> (WHEEL_BITS1 + 3 * WHEEL_BITS2)) & WHEEL_MASK2;
		spoke = wheels_[4]->spokes + index;
	}
	SNodeLink *nodelink = &(node->link);
	nodelink->prev = spoke->prev;
	spoke->prev->next = nodelink;
	nodelink->next = spoke;
	spoke->prev = nodelink;
}

void CTimerManager::RemoveTimer(STimerNode* node) {
	SNodeLink *nodelink = &(node->link);
	if (nodelink->prev) {
		nodelink->prev->next = nodelink->next;
	}
	if (nodelink->next) {
		nodelink->next->prev = nodelink->prev;
	}
	nodelink->prev = nodelink->next = NULL;

	delete node;
}

void CTimerManager::DetectTimerList() {
	uint64_t now = GetCurrentMillisec();
	uint32_t loopnum = now > checktime_ ? (now - checktime_) / GRANULARITY : 0;

	SWheel *wheel =  wheels_[0];
	for (uint32_t i = 0; i < loopnum; ++i) {
		SNodeLink *spoke = wheel->spokes + wheel->spokeindex;
		SNodeLink *link = spoke->next;
		while (link != spoke) {
			STimerNode *node = (STimerNode *)link;
			link->prev->next = link->next;
			link->next->prev = link->prev;
			link = node->link.next;
			AddToReadyNode(node);
		}
		if (++(wheel->spokeindex) >= wheel->size) {
			wheel->spokeindex = 0;
			Cascade(1);
		}
		checktime_ += GRANULARITY;
	}
	DoTimeOutCallBack();
}

uint32_t CTimerManager::Cascade(uint32_t wheelindex) {
	if (wheelindex < 1 || wheelindex >= WHEEL_NUM) {
		return 0;
	}
	SWheel *wheel =  wheels_[wheelindex];
	int casnum = 0;
	uint64_t now = GetCurrentMillisec();
	SNodeLink *spoke = wheel->spokes + (wheel->spokeindex++);
	SNodeLink *link = spoke->next;
	spoke->next = spoke->prev = spoke;
	while (link != spoke) {
		STimerNode *node = (STimerNode *)link;
		link = node->link.next;
		if (node->dead_time <= now) {
			AddToReadyNode(node);
		} else {
			uint32_t milseconds = node->dead_time - now;
			AddTimerNode(milseconds, node);
			++casnum;
		}

	}

	if (wheel->spokeindex >= wheel->size) {
		wheel->spokeindex = 0;
		casnum += Cascade(++wheelindex);
	}
	return casnum;
}*/