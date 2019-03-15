#ifndef __UTIL_H__
#define __UTIL_H__
/*

#define SHOW_ERROR_MSG(hWnd, errorID) {\
string str = GetErrorString( errorID ); \
char s[1024] = {0}; \	
sprintf_s( s, sizeof(s), "类：[%s] 函数：[%s] 行：[%d]", typeid(*this).name(), __FUNCDNAME__, __LINE__ ); \
str += s; \
MessageBox( hWnd, str, NULL, 0 ); }
*/

// #define SHOW_ERROR_MSG(hWnd, errorID) {string str = GetErrorString( errorID ); char s[1024] = {0}; sprintf_s( s, sizeof(s), "函数：[%s] 行：[%d] 描述：[%s]", __FUNCTION__, __LINE__, str.c_str() ); MessageBox( hWnd, s, NULL, 0 );}
#define SHOW_ERROR_MSG(hWnd, errorID) \
	{ \
		if (errorID > em_Msg_Error_ID_Client) \
		{ \
		    errorID = (em_Msg_Error_ID)((int)errorID - (int)em_Msg_Error_ID_Client); \
			string str = GetErrorString( errorID );  \
			char s[1024] = {0}; \
			sprintf_s( s, sizeof(s), "[客户端验证错误]函数：[%s] 行：[%d] 描述：[%s]", __FUNCTION__, __LINE__, str.c_str() ); \
			MessageBox( hWnd, s, NULL, 0 ); \
		} \
		else \
		{ \
			string str = GetErrorString( errorID );  \
			char s[1024] = {0}; \
			sprintf_s( s, sizeof(s), "函数：[%s] 行：[%d] 描述：[%s]", __FUNCTION__, __LINE__, str.c_str() ); \
			MessageBox( hWnd, s, NULL, 0 ); \
		}\
	} 

enum em_Msg_Error_ID
{
	em_Msg_Error_ID_Right = 0,
	em_Msg_Error_ID_NULL,                   // 空指针
	em_Msg_Error_ID_Size,                   // 包大小错误
	em_Msg_Error_ID_Watch,                  // 旁观者
	em_Msg_Error_ID_SlopOver,               // 越界

	em_Msg_Error_ID_Client = 1000,          // 客户端错误

	em_Msg_Error_ID_Max
};

// 	TMSG_ADMIN_CONTROL_CLEAR_STORE_REQ *pReq = NULL;
//  em_Msg_Error_ID errorID = em_Msg_Error_ID_Right;
//  if (!IsValidityMsg(pData, size, &pReq, errorID))    goto MsgError;
template<typename T>
bool IsValidityMsg( void *pBuffer, int len, T **p, em_Msg_Error_ID &errorID )
{
	if (len != sizeof(T)/*sizeof(*(*p))*/)    
	{
		errorID = em_Msg_Error_ID_Size;
		return false;
	}
	*p = static_cast<T*>(pBuffer);
	if (*p == NULL)    
	{
		errorID = em_Msg_Error_ID_NULL;
		return false;
	}

	return true;
}

// 该函数用户 客户端 收到服务器 回复时的处理。 消息体内 必须有 WORD errorID 成员, 且作为消息包第一个成员
// int baseLen = 0 - 表示 如果有父类， 必须告诉父类结构的大小
template<typename T>
bool IsValidityMsgRsp( void *pBuffer, int len, T **p, em_Msg_Error_ID &errorID, int baseLen = 0 )
{
	if (!IsValidityMsg(pBuffer, len, p, errorID))
	{
		errorID = (em_Msg_Error_ID)((int)errorID + (int)em_Msg_Error_ID_Client);   // 标识 客户端 收到的数据错误， 不是服务器 返回了 错误码
		return false;
	}
	
	BYTE *pTemp = (BYTE*)*p;

	WORD i = *((WORD*)(pTemp+baseLen));
	errorID = (em_Msg_Error_ID)i;
	
	if (errorID != em_Msg_Error_ID_Right)    return false;

	return true;
}

bool IsNull( void *p, em_Msg_Error_ID &errorID )
{
	if (p == NULL)    
	{
		errorID = em_Msg_Error_ID_NULL;
		return false;
	}

	return true;
}

bool IsSlopOver( int min, int max, int value, em_Msg_Error_ID &errorID )
{
	if (value < min || value >= max)    
	{
		errorID = em_Msg_Error_ID_SlopOver;
		return false;
	}

	return true;
}

const string GetErrorString( em_Msg_Error_ID errorID )
{
	if (errorID < em_Msg_Error_ID_Right || errorID >= em_Msg_Error_ID_Max )
	{
		return "Error";
	}

	string str[em_Msg_Error_ID_Max];
	str[em_Msg_Error_ID_NULL] = TEXT("空指针错误");
	str[em_Msg_Error_ID_Size] = TEXT("大小错误");

	return str[errorID];
}

#endif