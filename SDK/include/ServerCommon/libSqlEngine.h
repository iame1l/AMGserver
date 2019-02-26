// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 LIBSQLENGINE_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// LIBSQLENGINE_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifndef __libSqlEngine_h__
#define __libSqlEngine_h__

#ifdef LIBSQLENGINE_EXPORTS
#define LIBSQLENGINE_API __declspec(dllexport)
#else
#define LIBSQLENGINE_API __declspec(dllimport)
#endif

#include <windows.h>
#include <ATLComTime.h>
#include <tchar.h>
	
//////////////////////////////////////////////////////////////////////////
static const GUID IID_IDBEngineSink={0xb7914ba4, 0x6d57, 0x4b13, 0xb2, 0x3c, 0xf3, 0xbd, 0xf3, 0x78, 0x46, 0x36};

class IDBEngineSink
{
public:
	virtual ~IDBEngineSink() {}

public:
	virtual void closeRecord() = 0;

public:
	virtual bool adoBOF() = 0;
	virtual bool adoEndOfFile() = 0;
	virtual bool moveFirst() = 0;
	virtual bool moveNext() = 0;
	virtual bool moveLast() = 0;

public:
	// 获取记录数
	virtual long getRecordCount() = 0;
	// 获取字段数
	virtual long getFieldCount() = 0;

	// 执行函数，存储过程
public:
	// renturn   0	成功 
	// 			-1	参数错误
	// 			-2	连接断开
	// 			-3	指令集初始化失败
	// 			-4	脚本执行异常
	virtual int execSQL(LPCTSTR sql, bool returnValue) = 0;
	virtual int setStoredProc(LPCTSTR storedProc, bool returnValue) = 0;
	virtual int execStoredProc() = 0;

	// 设置存储过程参数 入参
public:
	// renturn   0	成功 
	// 			-1	参数字段为空
	// 			-2	创建存储过程参数失败

	virtual int addInputParameter(LPCTSTR fieldName, bool fieldValue) = 0;
	virtual int addInputParameter(LPCTSTR fieldName, char fieldValue) = 0;
	virtual int addInputParameter(LPCTSTR fieldName, unsigned char fieldValue) = 0;
	virtual int addInputParameter(LPCTSTR fieldName, short fieldValue) = 0;
	virtual int addInputParameter(LPCTSTR fieldName, unsigned short fieldValue) = 0;
	virtual int addInputParameter(LPCTSTR fieldName, int fieldValue) = 0;
	virtual int addInputParameter(LPCTSTR fieldName, unsigned int fieldValue) = 0;
	virtual int addInputParameter(LPCTSTR fieldName, long long fieldValue) = 0;
	virtual int addInputParameter(LPCTSTR fieldName, unsigned long long fieldValue) = 0;
	virtual int addInputParameter(LPCTSTR fieldName, float fieldValue) = 0;
	virtual int addInputParameter(LPCTSTR fieldName, double fieldValue) = 0;
	virtual int addInputParameter(LPCTSTR fieldName, unsigned int fieldSize, LPCTSTR fieldValue) = 0;
	virtual int addInputParameter(LPCTSTR fieldName, unsigned int fieldSize, BYTE* fieldValue) = 0;

	// 设置存储过程参数 出参
public:
	// renturn   0	成功 
	// 			-1	参数字段为空
	// 			-2	创建存储过程参数失败
	virtual int addOutParameter(LPCTSTR fieldName, int fieldValue) = 0;
	virtual int addOutParameter(LPCTSTR fieldName, unsigned int fieldValue) = 0;
	virtual int addOutParameter(LPCTSTR fieldName, long long fieldValue) = 0;
	virtual int addOutParameter(LPCTSTR fieldName, unsigned long long fieldValue) = 0;
	virtual int addOutParameter(LPCTSTR fieldName, double fieldValue) = 0;
	virtual int addOutParameter(LPCTSTR fieldName, unsigned int fieldSize, LPCTSTR fieldValue) = 0;

	//////////////////////////////////////////////////////////////////////////
	// 获取存储过程 返回值
public:
	virtual long getReturnValue() = 0;

	// 获取存储过程 出参
public:
	// renturn   0	成功 
	// 			-1	参数字段为空
	// 			-2	出参字段为空
	// 			-2	目标缓冲区太小
	virtual int getOutParameter(LPCTSTR fieldName, int* fieldValue) = 0;
	virtual int getOutParameter(LPCTSTR fieldName, unsigned int* fieldValue) = 0;
	virtual int getOutParameter(LPCTSTR fieldName, long long* fieldValue) = 0;
	virtual int getOutParameter(LPCTSTR fieldName, unsigned long long* fieldValue) = 0;
	virtual int getOutParameter(LPCTSTR fieldName, double* fieldValue) = 0;
	virtual int getOutParameter(LPCTSTR fieldName, LPTSTR dstField, unsigned int dstFieldLength) = 0;

	// 获取存储过程 集合
public:
	// renturn   0	成功 
	// 			-1	参数错误
	// 			-2	记录集字段值为空
	// 			-4	目标缓冲区太小
	virtual int getValue(LPCTSTR fieldName, bool* fieldValue) = 0;
	virtual int getValue(LPCTSTR fieldName, char* fieldValue) = 0;
	virtual int getValue(LPCTSTR fieldName, unsigned char* fieldValue) = 0;
	virtual int getValue(LPCTSTR fieldName, short* fieldValue) = 0;
	virtual int getValue(LPCTSTR fieldName, unsigned short* fieldValue) = 0;
	virtual int getValue(LPCTSTR fieldName, int* fieldValue) = 0;
	virtual int getValue(LPCTSTR fieldName, unsigned int* fieldValue) = 0;
	virtual int getValue(LPCTSTR fieldName, long* fieldValue) = 0;
	virtual int getValue(LPCTSTR fieldName, unsigned long* fieldValue) = 0;
	virtual int getValue(LPCTSTR fieldName, long long* fieldValue) = 0;
	virtual int getValue(LPCTSTR fieldName, unsigned long long* fieldValue) = 0;
	virtual int getValue(LPCTSTR fieldName, float* fieldValue) = 0;
	virtual int getValue(LPCTSTR fieldName, double* fieldValue) = 0;
	virtual int getValue(LPCTSTR fieldName, COleDateTime* fieldValue) = 0;
	virtual int getValue(LPCTSTR fieldName, GUID* fieldValue) = 0;
	virtual int getValue(LPCTSTR fieldName, LPTSTR dstField, unsigned int dstFieldLength) = 0;
	virtual int getValue(LPCTSTR fieldName, BYTE* dstField, unsigned int dstFieldLength) = 0;
};

typedef struct DB_CREATE_PARAM
{
	TCHAR server[64];
	TCHAR db[64];
	TCHAR uid[64];
	TCHAR pwd[64];
	long connectTimeOut;  // default 10秒
	long commandTimeOut;  // default 5秒

	DB_CREATE_PARAM() : connectTimeOut(0), commandTimeOut(0)
	{
		memset(server, 0x0, sizeof(server));
		memset(db, 0x0, sizeof(db));
		memset(uid, 0x0, sizeof(uid));
		memset(pwd, 0x0, sizeof(pwd));
	}
} DBCreateParam;


#ifdef __cplusplus
extern "C" {
#endif // _cplusplus

	LIBSQLENGINE_API void dklSQLDebugString(const TCHAR* format, ...);

	//************************************
	// Method:    dklSQLEngineMaxConnectCount
	// Returns:   void
	// Qualifier: 数据库最大连接数
	//************************************
	LIBSQLENGINE_API unsigned short dklSQLEngineMaxConnectCount();

	//************************************
	// Method:    dklSQLEngineConnectCount
	// Returns:   void
	// Qualifier: 数据库当前连接数
	//************************************
	LIBSQLENGINE_API unsigned short dklSQLEngineConnectCount();

	//************************************
	// Method:    dklSQLEngineCreate
	// Returns:   LIBSQLENGINE_API int	(0 成功,	-1 参数错误,	-2 连接数据库失败,	-3 连接数据库异常,	-4 超出最大连接数)
	// Qualifier: 创建数据库连接
	// Parameter: const DBCreateParam & param
	// Parameter: IDBEngineSink *  * pEngineSink
	//************************************
	LIBSQLENGINE_API int dklSQLEngineCreate(const DBCreateParam& param, IDBEngineSink * * pEngineSink);

	//************************************
	// Method:    dklSQLEngineClose
	// Returns:   void
	// Qualifier: 关闭数据库
	// Parameter: void *  & pEngine
	//************************************
	LIBSQLENGINE_API void dklSQLEngineDelete(IDBEngineSink * & pEngineSink);

#ifdef __cplusplus
};
#endif //_cplusplus


#endif // __libSqlEngine_h__
