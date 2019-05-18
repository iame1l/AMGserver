
#include <map>
#include <string>
#include "StdAfx.h"
#include "../Server/ServerManage.h"

// 玩家已经抽了多少张卡
std::map<long int, int> UserDrawnCardCount;

unsigned char GetCardValue(const CString &cardListStr, int cnt, unsigned char oriCard)
{
	unsigned char ret = oriCard;
	if(cardListStr.GetLength() > 0)
	{
		int foundPos = 0;
		int lastPos = 0;
		int i = 0;
		while(i <= cnt && foundPos >= 0)
		{
			++i;
			foundPos = cardListStr.Find(',', foundPos);
			if(i <= cnt && foundPos >= 0)
			{
				foundPos += 1;
				lastPos = foundPos;
			}
		}

		if(i > cnt)
		{
			// 存在第i张牌
			if(foundPos < 0)
			{
				// 第i张牌是最后一张(没有以逗号结束的)
				foundPos = cardListStr.GetLength();
			}
			ret = _ttoi(cardListStr.Mid(lastPos, foundPos - lastPos));
		}
	}
	return ret;
}

// 修改指定玩家的牌//mark
unsigned char GetSpecifiedCard(long int userID, unsigned char oriCard, unsigned char toBeReplace[], int length, bool reset, bool enabled)
{
	if(!enabled) return oriCard;

	CString s = CINIFile::GetAppPath ();/////本地路径
	CINIFile f(s + SKIN_FOLDER  + _T("_s.ini"));

	CString key;
	key = TEXT("SuperSet");
	char nameBuf[256];
	sprintf(nameBuf, "Card_%d", userID);
	CString cardListStr = f.GetKeyVal(key,nameBuf,"");

	unsigned char ret = oriCard;
	if(cardListStr.GetLength() > 0)
	{
		if(reset)
		{
			UserDrawnCardCount[userID] = 0;
		}
		int cnt = UserDrawnCardCount[userID];
		UserDrawnCardCount[userID] = cnt + 1;

		ret = GetCardValue(cardListStr, cnt, oriCard);
		if(ret != oriCard)
		{
			for(int i = 0; i < length; ++i)
			{
				if(toBeReplace[i] == ret)
				{
					toBeReplace[i] = oriCard;
					break;
				}
			}
		}
	}

	return ret;
}

// 修改鸟（/马）牌
unsigned char GetSpecifiedBirdCard(unsigned char oriCard, int cnt, bool enabled)
{
	if(!enabled) return oriCard;

	CString s = CINIFile::GetAppPath ();/////本地路径
	CINIFile f(s + SKIN_FOLDER  + _T("_s.ini"));

	CString key;
	key = TEXT("SuperSet");
	CString cardListStr = f.GetKeyVal(key,"Bird","");
	return GetCardValue(cardListStr, cnt, oriCard);
}



