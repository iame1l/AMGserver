斗地主AI

```C++
//游戏状态定义
#define GS_WAIT_SETGAME					0				//等待东家设置状态
#define GS_WAIT_ARGEE					1				//等待同意设置
#define GS_SEND_CARD					20				//发牌状态
#define GS_WAIT_BACK					21				//等待扣压底牌
#define GS_PLAY_GAME					22				//游戏中状态.
#define GS_WAIT_NEXT					23				//等待下一盘开始 

#define GS_FLAG_NORMAL					0				//正常情况
#define GS_FLAG_CALL_SCORE				1				//叫分
#define GS_FLAG_ROB_NT					2				//抢地主
#define GS_FLAG_ADD_DOUBLE				3				//加棒
#define GS_FLAG_SHOW_CARD				4				//亮牌
#define GS_FLAG_PLAY_GAME               5 

#define MAX_GAME_TURNS					64				//最多局数（用于）

const  int  MAX_TASK_TYPE   = 4 ;  ///任务最大种类 
const  int  MAX_CARD_SHAPE  = 8 ;  ///牌型最大种类
const  int  MAX_CARD_TYPE   = 15 ; ///牌种类
```

