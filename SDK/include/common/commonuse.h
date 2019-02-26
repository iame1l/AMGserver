/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#pragma once
#define MAX_STRING_SIZE 128
#include <map>
#include "cfgEngine.h"
#include "Encrypt.h"
#include <algorithm>
#include <string>
#include <direct.h>
#include <Windows.h>

using namespace std;
//#include "DecryptFile.h"

///定义输出
#ifndef EXT_CLASS
#	ifdef  AFC_FACE_DLL				
#		define EXT_CLASS _declspec(dllexport)
#	elif defined(_AFC_COM_DLL_EX)	/// 服务器平台类库AfcComDll.dll
#		define EXT_CLASS _declspec(dllexport)
#	elif defined(THIS_IS_DLL)		/// 游戏框架DllForGame.dll
#		define EXT_CLASS _declspec(dllexport)
#	else
#		define EXT_CLASS _declspec(dllimport)
#	endif
#endif

/// 新建的CBcfFile类封装，每个文件只读取一次，以后各次都直接从内存中取值
/// 使用者在进程结束时须显式调用CBcfFile::ClearMap()函数来释放，否则MFC会报内存泄漏

/// 因为类中使用到了静态成员，须在工程中包含有本头文件的cpp中定义下面一行代码
/// std::map<CString, CBcfFileNameItem*> CBcfFile::mapFile;

/// 我们的游戏模式用到了动态库，在动态库和EXE中都用到了CBcfFile，因此，把这个类放到动态库中导出
/// 上述静态成员应在动态库中定义，即AFCComDllEx.dll，DllForGame.dll

///
/// 节点基类
class CBcfListItem
{
public:
	std::map<CString, CBcfListItem*> m_ChildMap;
	CBcfListItem()
	{
		m_ChildMap.clear();
	}
	virtual ~CBcfListItem()
	{
		std::map<CString, CBcfListItem*>::iterator iter = m_ChildMap.begin();
		while (iter != m_ChildMap.end())
		{
			CBcfListItem* pListItem = iter->second;
			if (pListItem != NULL)
			{
				delete pListItem;
			}
			++iter;
		}
		m_ChildMap.clear();
	}
	virtual void SetData(void* pData,UINT uSize)=0;
};
/// 键值map
class CBcfKeyItem : public CBcfListItem
{
	CString m_strKey;
	CString m_strValue;
	CString m_strMemo;
	CBcfKeyItem();///默认构造函数是不允许执行的
public:
	CBcfKeyItem(CString szKey, CString szValue, CString szMemo) : m_strKey(szKey), m_strValue(szValue), m_strMemo(szMemo){}
	virtual void SetData(void* pData,UINT uSize){};
	CString GetValue(){return m_strValue;}
	CString GetMemo(){return m_strMemo;}
};
/// 段map
class CBcfSectionItem : public CBcfListItem
{
	CString m_strSection;	/// 用来保存这个段的名字
public:
	CBcfSectionItem(CString szSection) : m_strSection(szSection){}
	virtual void SetData(void* pData,UINT uSize){};
};
/// 文件map
class CBcfFileNameItem : public CBcfListItem
{
	CString m_strFileName;	/// 用来保存这个文件名字
public:
	CBcfFileNameItem(CString szFileName) : m_strFileName(szFileName){}
	virtual void SetData(void* pData,UINT uSize){};
};
/// 供使用者调用的类
class EXT_CLASS CBcfFile
{
	static std::map<CString, CBcfFileNameItem*> mapFile;
	CBcfFileNameItem *m_pFile;
	bool m_bIsFileExist;
public:

	// 文件是否存在
	bool IsFileExist()
	{
		return m_bIsFileExist;
	}

	static __int64 my_atoi(const char *str)
	{
		__int64 result = 0;
		int signal = 1;
		/* 默认为正数 */
		if((*str>='0'&&*str<='9')||*str=='-'||*str=='+')
		{
			if(*str=='-'||*str=='+')
		 {
			 if(*str=='-')
				 signal = -1; /* 输入负数 */
			 str++;
		 }
		}
		else return 0;/* 开始转换 */
		while(*str>='0'&&*str<='9')
			result = result*10+(*str++ -'0');
		return signal*result;
	}
	static void ClearMap()
	{
		std::map<CString, CBcfFileNameItem*>::iterator iter = mapFile.begin();
		while (iter != mapFile.end())
		{
			CBcfFileNameItem* pListItem = iter->second;
			if (pListItem != NULL)
			{
				delete pListItem;
			}
			++iter;
		}
		mapFile.clear();
	}
	static CString GetWinSysPath()
	{
		char lpBuffer[MAX_STRING_SIZE];
		GetSystemDirectory( lpBuffer, MAX_STRING_SIZE);
		CString ss=lpBuffer;
		ss += "\\";
		return ss;

	}
	static bool IsFileExist(CString fileName)
	{
		WIN32_FIND_DATA  d;
		HANDLE  hd= FindFirstFile(fileName,&d);
		bool bRet = (hd != INVALID_HANDLE_VALUE);
		FindClose(hd);
		return bRet;
	}
	static CString GetAppPath(bool bFource=true)
	{
		bFource = true;
		if(!bFource)
		{
			char lpBuffer[MAX_STRING_SIZE];
			ZeroMemory(lpBuffer,sizeof(lpBuffer));
			GetCurrentDirectory(MAX_STRING_SIZE,lpBuffer);
			CString ss=lpBuffer;
			ss += "\\";
			return ss;
		}
		else
		{
			TCHAR szModuleName[MAX_PATH];
			ZeroMemory(szModuleName,sizeof(szModuleName));
			DWORD dwLength=GetModuleFileName(AfxGetInstanceHandle(),szModuleName,sizeof(szModuleName));
			CString filepath=szModuleName;
			CString path;
			int nSlash = filepath.ReverseFind(_T('/'));
			if (nSlash == -1)
				nSlash = filepath.ReverseFind(_T('\\'));
			if (nSlash != -1 && filepath.GetLength() > 1)
			{
				path = filepath.Left(nSlash+1);
				SetCurrentDirectory(path);
				return path;
			}
			else
			{
				char lpBuffer[MAX_STRING_SIZE];
				ZeroMemory(lpBuffer,sizeof(lpBuffer));
				GetCurrentDirectory(MAX_STRING_SIZE,lpBuffer);
				CString ss=lpBuffer;
				ss += "\\";
				return ss;
			}

		}
	}

	/// 构造函数，创建或者给成员赋值
	CBcfFile(CString szFileName)
	{
		if (!IsFileExist(szFileName))
		{
			m_bIsFileExist = false;
			return;
		}
		m_bIsFileExist = false;
		/// 如果不存在，就创建之
		m_pFile = mapFile[szFileName];
		if (NULL == m_pFile)
		{
			bool bRet = ReadBcfFile(szFileName);
			if (bRet)	/// 如果文件存在，则赋值
			{
				m_bIsFileExist = true;
				mapFile[szFileName] = m_pFile;
			}
		}
		else
		{
			m_bIsFileExist = true;
		}
	}
	/// 析构函数，其中不能释放指针，否则会出现野指针
	~CBcfFile()
	{
		m_pFile = NULL;
	}
	/// 设置键值，内存读取时无效
	void SetKeyValString(CString secName,CString keyName,CString Val){}
	/// 关闭文件，不需要关闭
	void CloseFile(){}
	/// 取整数值
	int GetKeyVal(CString secName,CString keyName,int lpDefault)
	{
		if (!m_bIsFileExist)
		{
			return lpDefault;
		}
		TCHAR szSec[MAX_PATH];
		TCHAR szKey[MAX_PATH];
		UpperString(szSec, secName.GetBuffer());
		UpperString(szKey, keyName.GetBuffer());
		CBcfSectionItem *pSectionItem = (CBcfSectionItem *)m_pFile->m_ChildMap[szSec];
		if (NULL != pSectionItem)
		{
			CBcfKeyItem *pKeyItem = (CBcfKeyItem *)pSectionItem->m_ChildMap[szKey];
			if (NULL != pKeyItem)
			{
				return atoi(pKeyItem->GetValue());
			}
		}
		return lpDefault;
	}

	__int64 GetKeyVal(CString secName,CString keyName,__int64 lpDefault)
	{

		char str[255];
		sprintf_s(str, "%I64d", lpDefault); 

		CString __int64Str=GetKeyVal(secName,keyName,str);
		return my_atoi(__int64Str);
		
	}
	/// 取字符串
	CString GetKeyVal(CString secName,CString keyName,LPCTSTR lpDefault)
	{
		if (!m_bIsFileExist)
		{
			return lpDefault;
		}
		TCHAR szSec[MAX_PATH];
		TCHAR szKey[MAX_PATH];
		UpperString(szSec, secName.GetBuffer());
		UpperString(szKey, keyName.GetBuffer());
		CBcfSectionItem *pSectionItem = (CBcfSectionItem *)m_pFile->m_ChildMap[szSec];
		if (NULL != pSectionItem)
		{
			CBcfKeyItem *pKeyItem = (CBcfKeyItem *)pSectionItem->m_ChildMap[szKey];
			if (NULL != pKeyItem)
			{
				return pKeyItem->GetValue();
			}
		}
		return lpDefault;
	}
	/// 取注释
	CString GetKeyMemo(CString secName,CString keyName,LPCTSTR lpDefault)
	{
		if (!m_bIsFileExist)
		{
			return lpDefault;
		}
		TCHAR szSec[MAX_PATH];
		TCHAR szKey[MAX_PATH];
		UpperString(szSec, secName.GetBuffer());
		UpperString(szKey, keyName.GetBuffer());
		CBcfSectionItem *pSectionItem = (CBcfSectionItem *)m_pFile->m_ChildMap[szSec];
		if (NULL != pSectionItem)
		{
			CBcfKeyItem *pKeyItem = (CBcfKeyItem *)pSectionItem->m_ChildMap[szKey];
			if (NULL != pKeyItem)
			{
				return pKeyItem->GetMemo();
			}
		}
		return lpDefault;
	}
private:
	/// 转换成大写字符
	TCHAR *UpperString(TCHAR *szDes, LPCTSTR szSrc)
	{
		if ((szDes == NULL) || (NULL == szSrc))
		{
			return NULL;
		}
		int nLen = _tcslen(szSrc);
		//_tcscpy_s(szDes, nLen+1, szSrc);
		_tcscpy(szDes, szSrc);
		for (int i=0; i<nLen+1; ++i)
		{
			if (szDes[i]>='a' && szDes[i]<='z')
			{
				szDes[i] = szDes[i]-0x20;
			}
		}
		return szDes;
	}
	/// 只能在构造函数中当文件还没有读取时调用
	bool ReadBcfFile(CString szFileName)
	{
		DWORD hFileHandle = cfgOpenFile(szFileName);
		if(hFileHandle<0x10)
			return false;
		/// 打开文件成功，进行读取过程
		POSITION posSection = cfgFindFirstSectionPosition(hFileHandle);
		if (NULL==posSection)
		{
			return false;
		}

		m_pFile = new CBcfFileNameItem(szFileName);
		/// 循环外面声明循环里用到的变量
		LPCTSTR szAnyCaseSection,szAnyCaseKeyName,szKeyValue,szKeyMemo;
		TCHAR szSection[MAX_PATH], szKeyName[MAX_PATH];
		int		nKeyAttrib;
		while (NULL != posSection)
		{
			szAnyCaseSection = cfgFindNextSection(hFileHandle, posSection);	/// 读一次就会改变一次posSection，直到尾部时，变成NULL
			/// 把字符串转换成大写
			UpperString(szSection, szAnyCaseSection);
			/// 创建一个段
			CBcfSectionItem *pSectionItem = new CBcfSectionItem(szSection);
			/// 放到文件map中
			m_pFile->m_ChildMap[szSection] = pSectionItem;
			POSITION posKey = cfgFindFistKeyPosition(hFileHandle, szSection);
			while (NULL != posKey)
			{
				cfgFindNextKey(hFileHandle, szSection, posKey, szAnyCaseKeyName, szKeyValue, nKeyAttrib);
				UpperString(szKeyName, szAnyCaseKeyName);
				szKeyMemo = cfgGetKeyMemo(hFileHandle, szSection, szKeyName);
				CBcfKeyItem *pKeyItem = new CBcfKeyItem(szKeyName, szKeyValue, szKeyMemo);
				pSectionItem->m_ChildMap[szKeyName] = pKeyItem;
			}
		}
		cfgClose(hFileHandle);
		return true;
	}
};

/// Modified by zxd 20090810 }}
///


class CINIFile  
{
  CString lpFileName ;
  DWORD mMaxSize ;
public:

	static __int64 my_atoi(const char *str)
	{
		__int64 result = 0;
		int signal = 1;
		/* 默认为正数 */
		if((*str>='0'&&*str<='9')||*str=='-'||*str=='+')
		{
			if(*str=='-'||*str=='+')
		 {
			 if(*str=='-')
				 signal = -1; /* 输入负数 */
			 str++;
		 }
		}
		else return 0;/* 开始转换 */
		while(*str>='0'&&*str<='9')
			result = result*10+(*str++ -'0');
		return signal*result;
	}
	static bool IsFileExist(CString fileName)
	{
		WIN32_FIND_DATA  d;
		HANDLE  hd= FindFirstFile(fileName,&d);
		bool bRet = (hd != INVALID_HANDLE_VALUE);
		FindClose(hd);
		return bRet;
	}
	static void CopyFileTo(CString destFileName,CString srcFileName)
	{
		bool bb=IsFileExist(srcFileName);
		if(!bb)return;
		CopyFile( srcFileName, destFileName, FALSE);

	}
		
	static CString GetWinSysPath()
	{
		char lpBuffer[MAX_STRING_SIZE];
		GetSystemDirectory( lpBuffer, MAX_STRING_SIZE);
		CString ss=lpBuffer;
		ss += "\\";
		return ss;

	}
		
	static CString GetAppPath(bool bFource=true)
	 {
         // 固定使用第二种方式获取程序路径，
         // 第一种方式在某些情况下会出现问题。
         bFource = true;

		if(!bFource)
		{
			char lpBuffer[MAX_STRING_SIZE];
			ZeroMemory(lpBuffer,sizeof(lpBuffer));
			GetCurrentDirectory(MAX_STRING_SIZE,lpBuffer);
			CString ss=lpBuffer;
			ss += "\\";
			return ss;
		}
		else
		{
			TCHAR szModuleName[MAX_PATH];
			ZeroMemory(szModuleName,sizeof(szModuleName));
			HINSTANCE test = AfxGetInstanceHandle();
			DWORD dwLength=GetModuleFileName(AfxGetInstanceHandle(),szModuleName,sizeof(szModuleName));
			CString filepath=szModuleName;
			CString path;
			int nSlash = filepath.ReverseFind(_T('/'));
			if (nSlash == -1)
				nSlash = filepath.ReverseFind(_T('\\'));
			if (nSlash != -1 && filepath.GetLength() > 1)
			{
				path = filepath.Left(nSlash+1);
				SetCurrentDirectory(path);
				return path;
			}
			else
			{
				char lpBuffer[MAX_STRING_SIZE];
				ZeroMemory(lpBuffer,sizeof(lpBuffer));
				GetCurrentDirectory(MAX_STRING_SIZE,lpBuffer);
				CString ss=lpBuffer;
				ss += "\\";
				return ss;
			}

		}
	}

	int GetKeyVal(CString secName,CString keyName,int lpDefault)
	{
		return (int) GetPrivateProfileInt(
					secName,/// points to section name
					keyName,/// points to key name
					lpDefault,       /// return value if key name not found
					lpFileName///LPCTSTR lpFileName  /// initialization file name
					);

	}

	UINT GetKeyVal(CString secName,CString keyName,UINT lpDefault)
	{
		return (UINT) GetPrivateProfileInt(
					secName,/// points to section name
					keyName,/// points to key name
					lpDefault,       /// return value if key name not found
					lpFileName///LPCTSTR lpFileName  /// initialization file name
					);

	}

	__int64 GetKeyVal(CString secName,CString keyName,__int64 lpDefault)
	{
		
		char str[255];
		sprintf_s(str, 255, "%I64d", lpDefault); 
	    CString __int64Str=GetKeyVal(secName,keyName,str);
		return my_atoi(__int64Str);
	}
	

	CString GetKeyVal(CString secName,CString keyName,LPCTSTR lpDefault)
	{ 
		char* re = new char[mMaxSize + 1];
		ZeroMemory(re,mMaxSize);
		GetPrivateProfileString(
								secName,/// points to section name
								keyName,/// points to key name
								lpDefault,/// points to default string
								re,/// points to destination buffer
								mMaxSize,/// size of destination buffer
								lpFileName /// points to initialization filename
							);
		CString ss=re;
		delete []re;
		return ss;
	}
		
	void SetKeyValString(CString secName,CString keyName,CString Val)
	{ 

		WritePrivateProfileString(
								secName,/// pointer to section name
								keyName,/// pointer to key name
								Val,/// pointer to string to add
								lpFileName/// pointer to initialization filename
								);

	}

		
	CINIFile(CString FileName,int maxsize=MAX_STRING_SIZE)
	{
		lpFileName=FileName;
		mMaxSize = maxsize;

	}

	~CINIFile()	{}
	void SetINIFileName(CString fileName){lpFileName=fileName;}
	
};

template<class T>
static void CopyPtrList(CPtrList &dest,CPtrList &src)
{
   for(POSITION pos=src.GetHeadPosition ();pos !=NULL;)
	{
		T *pi=(T *)src.GetNext (pos);
		if(pi)
		{
			T *pii=new T(); *pii=*pi;
			dest.AddTail (pii);
		}
	}
}

template<class T> 
static void EmptyPtrList(CPtrList &src)
{
	 while(!src.IsEmpty ())
	{
		T *pi=(T *)src.RemoveHead  ();
		delete pi;
	}
	src.RemoveAll ();
}


template<class T>
static void AddToPtrListToTail(CPtrList &dest,T &add)
{
  T *pii=new T(); *pii=add;
  dest.AddTail (pii);
}

template<class T>
static void AddToPtrListToHead(CPtrList &dest,T &add)
{
  T *pii=new T();*pii=add;
  dest.AddHead (pii);
}

#define MAX_PASSWORD_LENGTH    255
struct EXT_CLASS FileItem
{
	void * _pBufFile;
	ULONG  _nFileLen;
	unsigned char _digest[MAX_PASSWORD_LENGTH];
	FileItem()
	{
		_pBufFile=NULL;
		_nFileLen=0;
		ZeroMemory(_digest,MAX_PASSWORD_LENGTH);
	}
	~FileItem()
	{
		if(_pBufFile!=NULL)
		{
			delete []_pBufFile;
			_pBufFile=NULL;
		}
	}
};


class CMemIO
{
public:
	CMemIO(unsigned long ulSize)
	{
		m_pBuffer = new char[ulSize];
		m_ulSize = ulSize;
		m_ulSeek = NULL;
		m_bNeedDel = true;
	}
	CMemIO(const char *pBuffer, unsigned long ulSize)
	{
		m_pBuffer = (char*)pBuffer;
		m_ulSize = ulSize;
		m_ulSeek = NULL;
		m_bNeedDel = false;
	}
	~CMemIO()
	{
		if (m_bNeedDel)
			delete m_pBuffer;
	}
private:
	CMemIO(const CMemIO &other);
	CMemIO &operator = (const CMemIO &other);

public:
	int Write(char *pbuf, unsigned long ulSize)
	{
		if ((m_ulSize-m_ulSeek)<ulSize)
		{
			return 0;
		}
		memcpy(m_pBuffer+m_ulSeek, pbuf, ulSize);
		m_ulSeek += ulSize;
		return ulSize;
	}
	const char *GetMemBlock(unsigned long ulSize, unsigned long *ulSizeGetted)
	{
		if ((m_ulSize-m_ulSeek)<ulSize)
		{
			return NULL;
		}
		*ulSizeGetted = ulSize;
		char *pRet = m_pBuffer+m_ulSeek;
		m_ulSeek += ulSize;
		return pRet;
	}
	unsigned long Read(char *pDst, unsigned long ulSize)
	{
		if ((m_ulSize-m_ulSeek)<ulSize)
		{
			return 0;
		}        
		memcpy(pDst, m_pBuffer+m_ulSeek, ulSize);
		//pDst = m_pBuffer+m_ulSeek;
		m_ulSeek += ulSize;
		return ulSize;
	}
	unsigned long Read(char **pDst, unsigned long ulSize)
	{
		if ((m_ulSize-m_ulSeek)<ulSize)
		{
			return 0;
		}                
		*pDst = m_pBuffer+m_ulSeek;
		m_ulSeek += ulSize;
		return ulSize;
	}
	void SeekToBegin()
	{
		m_ulSeek = 0;
	}

private:
	char            *m_pBuffer;
	unsigned long   m_ulSize;
	unsigned long   m_ulSeek;
	bool            m_bNeedDel;

};
/**	@brief 该类用于将一个文件夹的内容写成一个文件，并带还原功能
*/
struct FileInfo
{
	char          szFileName[MAX_PATH];
	unsigned long ulStartPos;
	unsigned long ulLen;       
};

struct FilePos
{
	unsigned long ulStartPos;
	unsigned long ulLen;       
};

typedef std::map<std::string, FilePos> PackPosInfo;

struct Zipmen
{
	void *pBuffer;
	unsigned long uLen;
	PackPosInfo posInfo;
	Zipmen()
	{
		pBuffer = NULL;
		uLen = 0;
	}
};

/**
* @brief 得到打内存的PosInfo
*/
static void GetPosInfo(const char *pPacked, const unsigned long ulMemLen, PackPosInfo &posInfo)
{
	CMemIO mem(pPacked, ulMemLen);    
	mem.SeekToBegin();

	int nSize = 0;
	mem.Read((char*)&nSize, sizeof(nSize));

	FileInfo *pPos = new FileInfo[nSize];
	if (mem.Read((char**)&pPos, sizeof(FileInfo)*nSize)<sizeof(FileInfo)*nSize)
	{
		return ;
	}

	for (int i=0; i<nSize; ++i)
	{
		FilePos pos;
		pos.ulLen = pPos[i].ulLen;
		pos.ulStartPos = pPos[i].ulStartPos;
		std::string strFileName = pPos[i].szFileName;      
		transform(strFileName.begin(), strFileName.end(), strFileName.begin(), tolower);
		posInfo[strFileName] = pos;
	}
};
