#pragma once




//一组牌的可能类型
enum Type {
	cardgroupisUnkown,//未知
	cardgroupisSingle,//单张
	cardgroupisDouble,//对子
	cardgroupisThree,//三条
	cardgroupisSingleSeq,//单顺
	cardgroupisDoubleSeq,//双顺
	cardgroupisThreeSeq,//三顺
	cardgroupisThreePlus,//三带一（一张或一对）
	cardgroupisAirplane,//飞机
	cardgroupisFourSeq,//四带二（两张或两对）
	cardgroupisBomb,//炸弹、王炸
};

class Ecardgroup
{
public:
	Ecardgroup();
	Ecardgroup(Type t, int v);
	Ecardgroup& operator=(Ecardgroup &cg);
	void AddNumber(int num);//添加0-53表示的牌元素
	void DeleteNumber(int num);//去掉一张牌
	void Clear(void);//重置此结构
	//把0-53转换成3-17权值，其中A（14）、2（15）、小王（16）、大王（17）
	static int Translate(int num);


private:
public:
	std::map<int, int> group;//3-17权值集合
	std::set<int> cards;//0-53组成的集合，主要用于方便画面显示
	Type type;//牌型类型（单牌、对子等等）
	int value;//权值
	int count;//此结构元素数量（牌数量
};



