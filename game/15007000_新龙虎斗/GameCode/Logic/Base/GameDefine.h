#ifndef GAME_DEFINE_H
#define GAME_DEFINE_H

typedef unsigned char uval;
typedef unsigned int  uint;
typedef unsigned char uchar;
typedef unsigned int UID;

#define  ONE_HAND_CARD_COUNT   2

#define  BYTE_ERR						255				//无符号字符类型初始化

const size_t MIN_COUNT_OF_SEQ    = 5;
const size_t MIN_COUNT_OF_SEQ2	 = 2;
const size_t MIN_COUNT_OF_SEQ3	 = 2;
const size_t MIN_COUNT_OF_BOMB   = 4;
const size_t MIN_COUNT_OF_JOKERS = 4;

//比较结果
enum em_CmpResult
{
	e_cmp_big	= 0,
	e_cmp_equal = 1,
	e_cmp_little= 2,
	e_cmp_error = 0xff
};

enum em_Assemble_Err_NO
{
	e_assemble_err_succ     = 0, //符合
	e_assemble_err_has_done = 1, //已经组牌过
	e_assemble_err_rule     = 2, //不符合配牌规则	
};

enum E_CALL_STATE
{
	E_CALL_NIL		= BYTE_ERR,
	E_CALL_CALL		= 0,
	E_CALL_NOT_CALL = 1,
};

enum E_PLAYCARD_STATE
{
	E_PLAYCARD_NIL	= 0,
	E_PLAYCARD_PASS = 1,
	E_PLAYCARD_DO	= 2,
};

enum E_RECORD
{
	E_RESULT_DRAW = 0,
	E_RESULT_WIN  = 1,
	E_RESULT_LOST = 2,
	E_RESULT_NIL  = 0xff,
};

enum E_RESULT_TYPE
{
	E_RESULT_TYPE_NIL = 255,
	E_RESULT_TYPE_NOMAL			= 1,	//普通
	E_RESULT_TYPE_DOUBLE_KILL	= 2,	//独食A3
	//E_RESULT_TYPE_CHALLENGE		= 3,	//叫牌
};

#endif //GAME_DEFINE_H