#ifndef MONEY_DATAMANAGE_H
#define MONEY_DATAMANAGE_H

class MoneyManage
{
public:
	MoneyManage(unsigned int id,__int64 money):deskNO(id),iMoney(money){};

	unsigned int deskNO;
	__int64 iMoney;
};
struct stuless
{
	bool operator()(const MoneyManage &first,  const MoneyManage &second)
	{
		return first.iMoney > second.iMoney;
	}

};

#endif // MONEY_DATAMANAGE_H