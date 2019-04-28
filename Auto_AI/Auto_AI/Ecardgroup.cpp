#include "StdAfx.h"
#include "Ecardgroup.h"


Ecardgroup::Ecardgroup()
{
}

Ecardgroup::Ecardgroup(Type t, int v)
: type(t)
, value(v)
, count(0)
{

}
Ecardgroup& Ecardgroup::operator=(Ecardgroup &cg)
{
	this->group = cg.group;
	this->cards = cg.cards;
	this->type = cg.type;
	this->value = cg.value;
	this->count = cg.count;
	return *this;
}
void Ecardgroup::Clear(void)
{
	group.clear();
	cards.clear();
	type = cardgroupisUnkown;
	value = 0;
	count = 0;
	return;
}

//添加0-53表示的牌
void Ecardgroup::AddNumber(int num)
{
	++count;
	cards.insert(num);
	++group[Translate(num)];
}
//去掉一张牌
void Ecardgroup::DeleteNumber(int num)
{
	if (cards.find(num) == cards.end())//确定要去掉的牌在结构内
		return;
	--count;
	cards.erase(num);
	if (--group[Translate(num)] == 0)
		group.erase(Translate(num));
}
//把0-53转换成3-17权值，其中A（14）、2（15）、小王（16）、大王（17）
int Ecardgroup::Translate(int num)
{
	if (num < 52)
		return num / 4 + 3;
	else
		return num - 36;
}
