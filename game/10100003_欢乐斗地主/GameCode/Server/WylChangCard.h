#ifndef _WYLCHANGECARD_H
#define _WYLCHANGECARD_H

//说明：用户换牌的公共类，wyl_20170214
//版本：v1.0（暂时没加去重、配摸牌，以后逐步完善）

class CChangeCard
{
private:
	int		m_iPlayCount;
	int		m_iHandCardCount;
	bool	m_bSwitch;

	CINIFile m_file;
public:
	CChangeCard(int	iPlayCount, int	iHandCardCount):m_iPlayCount(iPlayCount), m_iHandCardCount(iHandCardCount),m_file(CString (GetAppPath_WYL()) +_T("\\") + CString("10100003")  + _T("_s.ini"))
	{
		m_bSwitch = m_file.GetKeyVal("game", "WylDebugSwitch", 0);
	}
	char *GetAppPath_WYL()
	{
		char s_Path[1024];
		ZeroMemory(s_Path,sizeof(s_Path));
		DWORD dwLength=GetModuleFileNameA(GetModuleHandle(NULL), s_Path, sizeof(s_Path));
		char *p = strrchr(s_Path, '\\');
		if(p != NULL)	*p = '\0';
		return s_Path;
	}
	//设置庄家
	bool ChangeNT(long& byNt)
	{
		if (!m_bSwitch)
		{
			return false;
		}
		BYTE byReadCard = m_file.GetKeyVal("game", "Zhuang", 255);
		if (255 != byReadCard)
		{
			byNt = byReadCard;
		}
	}

	//设置庄家
	bool ChangeNT(BYTE& byNt)
	{
		if (!m_bSwitch)
		{
			return false;
		}
		BYTE byReadCard = m_file.GetKeyVal("game", "Zhuang", 255);
		if (255 != byReadCard)
		{
			byNt = byReadCard;
		}
	}

	//设置精牌
	bool ChangeJingCard(BYTE& byCard)
	{
		if (!m_bSwitch)
		{
			return false;
		}
		BYTE byReadCard = m_file.GetKeyVal("game", "Jing", 255);
		if (255 != byReadCard)
		{
			byCard = byReadCard;
		}
	}

	bool ChangeUserCard(BYTE pCard[] )
	{
		if (!m_bSwitch)
		{
			return false;
		}

		CString sText;

		for (int i = 0; i < m_iPlayCount; ++i)
		{
			for (int j = 0; j < m_iHandCardCount; ++j)
			{
				sText.Format("Card[%d][%d]", i, j);
				BYTE byReadCard = m_file.GetKeyVal("game",sText,255);
				if (byReadCard != 255)
				{
					pCard[i * m_iHandCardCount + j] = byReadCard;
				}
			}
		}
	}
	//设置下一张摸牌
	void SetMoPai(BYTE byStation, BYTE& byCard)
	{
		CString sText;
		for (int i = 0; i < m_iPlayCount; ++i)
		{
			if (i != byStation)
			{
				continue;
			}
			sText.Format("ZhuaCard[%d]", i);
			BYTE byReadCard = m_file.GetKeyVal("game",sText,255);
			if (255 != byReadCard)
			{
				byCard = byReadCard;
			}
		}
	}

	bool ChangeAllCard(BYTE pCard[])
	{
		for (int i = 0; i < m_iHandCardCount * m_iPlayCount; i++)
		{
			;
		}
	}
};

#endif
