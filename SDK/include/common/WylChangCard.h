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
public:
	CChangeCard(int	iPlayCount, int	iHandCardCount):m_iPlayCount(iPlayCount), m_iHandCardCount(iHandCardCount)
	{
		CString strPath(GetAppPath_WYL());
		CINIFile f(strPath +_T("\\") + SKIN_FOLDER  + _T("_s.ini"));
		m_bSwitch = f.GetKeyVal("game", "WylDebugSwitch", 0);
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
	bool ChangeUserCard(BYTE pCard[] )
	{
		if (!m_bSwitch)
		{
			return false;
		}
		CString strPath(GetAppPath_WYL());
		CINIFile f(strPath +_T("\\") + SKIN_FOLDER  + _T("_s.ini"));
		CString sText;

		for (int i = 0; i < m_iPlayCount; ++i)
		{
			for (int j = 0; j < m_iHandCardCount; ++j)
			{
				sText.Format("Card[%d][%d]", i, j);
				BYTE byReadCard = f.GetKeyVal("game",sText,255);
				if (byReadCard != 255)
				{
					pCard[i * m_iHandCardCount + j] = byReadCard;
				}
			}
		}
	}


	//CChangeCard changeCard(PLAY_COUNT, HAND_CARD_NUM);
	//changeCard.ChangeAllCard((BYTE*)pDesk->sUserData.m_MenPai.byMenPai, index, num);
	bool ChangeAllCard(BYTE* pSource, int iSize, int iBeginIndex)
	{
		if (!m_bSwitch)
		{
			return false;
		}
		BYTE byCard[4][14];
		memset(byCard, 255, sizeof(byCard));

		CString strPath(GetAppPath_WYL());
		CINIFile f(strPath +_T("\\") + SKIN_FOLDER  + _T("_s.ini"));
		CString sText;

		for (int i = 0; i < m_iPlayCount; ++i)
		{
			for (int j = 0; j < m_iHandCardCount; ++j)
			{
				sText.Format("Card[%d][%d]", i, j);
				BYTE byReadCard = f.GetKeyVal("game",sText,255);
				if (byReadCard != 255)
				{
					byCard[i][j] = byReadCard;
				}
			}
		}
		//备份
		int iIndex = iBeginIndex;
		BYTE byTempSource[300];
		memset(byTempSource, 255, sizeof(byTempSource));
		memcpy_s(byTempSource, iSize, pSource, iSize);

		//1.复制配置的手牌
		memset(pSource, 255, iSize);
		for (int i = 0; i < m_iPlayCount; ++i)
		{
			for (int j = 0; j < m_iHandCardCount; ++j)
			{
				if (iIndex >= iSize)
				{
					iIndex = 0;
				}
				//1.1 赋值
				pSource[iIndex++] = byCard[i][j];
				//1.2 删除找到的配牌
				for (int k = 0; k < iSize; k++)
				{
					if (byTempSource[k] == byCard[i][j])
					{
						byTempSource[k] = 255;
						break;
					}
				}

			}
		}
		//2.拷贝剩余牌
		for (int i = 0; i < iSize; ++i)
		{
			if (pSource[i] != 255)
			{
				continue;
			}
			for (int j = 0; j < iSize; ++j)
			{
				if (byTempSource[j] != 255)
				{
					pSource[i] = byTempSource[j];
					byTempSource[j] = 255;
					break;
				}
			}
		}
	}
};

#endif
