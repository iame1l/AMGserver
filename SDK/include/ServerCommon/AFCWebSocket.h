#ifndef __HN_AFC_WEB_SOCKET_H_
#define __HN_AFC_WEB_SOCKET_H_
#include "AFCSocket.h"

class CTCPSocketManageForWeb;

//TCP SOCKET 类
class AFX_EXT_CLASS CTCPSocketForWeb
{
    friend class CTCPSocketManageForWeb;

    //结构定义
    struct OverLappedStruct		//SOCKET 重叠 IO 结构
    {
        //变量定义
        OVERLAPPED				OverLapped;							//重叠结构
        WSABUF					WSABuffer;							//数据缓冲
        UINT					uOperationType;						//操作类型
        DWORD					dwHandleID;							//处理 ID 号码
    };

    enum HN_SOCKET_TYPE
    {
        e_normal_socket,
        e_web_socket
    };

    //变量定义
protected:
    UINT						m_uIndex;							//SOCKET 索引
    DWORD						m_dwHandleID;						//处理标识
    char						m_szSendBuf[SED_SIZE];				//发送数据缓冲区
    char						m_szRecvBuf[RCV_SIZE];				//数据接收缓冲区
    char                        m_szRecvMsg[MAX_SEND_SIZE];
    int                         m_szRecvMsgPos;
    long int					m_lBeginTime;						//连接时间
    SOCKET						m_hSocket;							//SOCKET 句柄
    CTCPSocketManageForWeb		* m_pManage;						//SOCKET 管理类指针
    struct sockaddr_in			m_SocketAddr;						//地址信息

    HN_SOCKET_TYPE m_socketType;
    bool m_bHandShaked;
    bool HandShake();

    //内部数据
private:
    bool						m_bSending;							//是否发送中
    DWORD						m_dwSendBuffLen;					//发送缓冲区长度
    DWORD						m_dwRecvBuffLen;					//接收缓冲区长度
    CAFCSignedLock				m_csSendLock;						//读取同步锁
    CAFCSignedLock				m_csRecvLock;						//接受同步锁
    OverLappedStruct			m_SendOverData;						//发送数据重叠结构
    OverLappedStruct			m_RecvOverData;						//接收数据重叠结构

    //函数定义
public:
    //构造函数
    CTCPSocketForWeb(void);
    //析够函数
    virtual ~CTCPSocketForWeb(void);

    //功能函数
public:
    //获取处理标志
    inline DWORD GetHandleID() { return m_dwHandleID; }
    //合并 SOCKET
    inline bool Attach(SOCKET hSocket, sockaddr_in & Addr, DWORD dwHandleID);
    //发送数据函数
    inline int SendData(const char * pData, UINT uBufLen);
    //发送数据函数
    inline int SendData(BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode, DWORD dwHandleID);
    //发送数据函数
    inline int SendData(void * pData, UINT uBufLen, BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode, DWORD dwHandleID);
    //功能函数

private:
    //发送数据函数
    inline int SendDataForWeb(const char * pData, UINT uBufLen);
    //发送数据函数
    inline int SendDataForWeb(BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode, DWORD dwHandleID);
    //发送数据函数
    inline int SendDataForWeb(void * pData, UINT uBufLen, BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode, DWORD dwHandleID);

    //发送数据函数
    inline int SendDataForNormal(const char * pData, UINT uBufLen);
    //发送数据函数
    inline int SendDataForNormal(BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode, DWORD dwHandleID);
    //发送数据函数
    inline int SendDataForNormal(void * pData, UINT uBufLen, BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode, DWORD dwHandleID);

public:
    //判断是否连接
    inline bool IsConnect() { return (m_hSocket!=INVALID_SOCKET); }
    //获取连接时间
    inline long int GetBeginTime() { return m_lBeginTime; };
    //获取连接信息
    inline ULONG GetConnectData() { return m_SocketAddr.sin_addr.S_un.S_addr; }
    //获取接收锁
    inline  CAFCSignedLock * GetRecvLock() { return &m_csRecvLock; }
    //获取发送锁锁
    inline  CAFCSignedLock * GetSendLock() { return &m_csSendLock; }
    //判断是否连接
    inline bool IsConnectID(DWORD dwHandleID);

    //辅助函数
protected:
    //设置 ID 索引号码
    inline void SetIndex(UINT uIndex) { m_uIndex=uIndex; };
    //设置管理类指针
    inline void SetSocketManagePtr(CTCPSocketManageForWeb * pManage) { m_pManage=pManage; };
    //发送完成函数
    inline bool OnSendCompleted(DWORD dwThancferred, DWORD dwHandleID);
    //接收完成函数
    inline bool OnRecvCompleted(DWORD dwHandleID);
    //检测发送数据函数
    inline bool CheckSendData();
    //投递接收 IO
    inline bool RecvData();
    //关闭 SOCKET
    inline bool CloseSocket();
};

//TCP 网络 SOCKET 管理类
class AFX_EXT_CLASS CTCPSocketManageForWeb
{
    ///int m_CurConnectCount;
    //结构定义
    struct BatchSendLineStruct	//批量发送线程队列结构
    {
        //变量定义
        NetMessageHead			NetHead;						//数据包头
        char					szSendData[MAX_SEND_SIZE_OUT];		//发送缓存
    };

    struct UnionSendLineStruct	//批量发送线程队列结构
    {
        //变量定义
        NetMessageHead			NetHead;							//数据包头
        int						iUnionID;
        char					szSendData[MAX_SEND_SIZE_OUT];		//发送缓存
    };

    struct ThreadStartStruct	//线程启动参数结构
    {
        //变量定义
        HANDLE					hCompletionPort;				//完成端口
        HANDLE					hThreadEvent;					//线程事件
        CTCPSocketManageForWeb		* pSocketManage;				//SOCKET 管理指针
    };

    //内核变量
protected:
    bool						m_bInit;						//初始化标志
    volatile bool				m_bRun;							//工作标志
    CListenSocket				m_ListenSocket;					//监听 SOCKET
    BYTE						* m_pKeep;						//保持连接数据
    IServerSocketServiceForWeb		* m_pService;					//服务接口
    CAFCPtrArray<CTCPSocketForWeb>	m_SocketPtrArray;				//TCP SOCKET 数组
    //设置参数
protected:
    bool						m_bCheckConnect;				//是否检测断线
    BYTE						m_bMaxVer;						//最新版本
    BYTE						m_bLessVer;						//最低版本
    UINT						m_uCheckTime;					//检测断线间隔
    UINT						m_uListenPort;					//监听断口

public:
    UINT						m_iSecretKey;                   //加密校验码的密钥

    //批发数据
protected:
    UINT						m_uBatchCerrenPos;				//批量发送点
    UINT						m_uBatchLineCount;				//发送队列数目
    UINT						m_uUnionCerrenPos;				//组播发送点
    UINT						m_uUnionLineCount;				//组播发送队列数目
    CAFCSignedLock				m_csBacthSend;					//批发同步锁
    CAFCSignedLock				m_csUnionSend;					//组发同步锁
    BatchSendLineStruct			m_SendLine[uBatchLineSize];		//批量发送队列
    UnionSendLineStruct			m_UnionSendLine[uBatchLineSize];//组播发送队列

    map<int,vector<int>>		m_UnionData;					//组播信息
    //完成端口变量
private:
    UINT						m_uRSThreadCount;				//读写线程数目
    HANDLE						m_hEventThread;					//批量发送事件
    HANDLE						m_hCompletionPortBatch;			//批量完成端口
    HANDLE						m_hCompletionPortRS;			//读写完成端口
    HANDLE						m_hThreadKeep;					//SOCKET 维护线程
    HANDLE						m_hThreadAccept;				//应答线程
    HANDLE						m_hCompletionPortUnion;			//组播发送完成端口

    //函数定义
public:
    //构造函数
    CTCPSocketManageForWeb();
    CTCPSocketManageForWeb(CTCPSocketManageForWeb&);
    CTCPSocketManageForWeb& operator=(CTCPSocketManageForWeb&); 
    //析构函数
    virtual ~CTCPSocketManageForWeb();

    //服务函数
public:
    //初始化
    virtual bool Init(UINT uMaxCount, UINT uListenPort, BYTE bMaxVer, BYTE bLessVer, int iSecretKey, IServerSocketServiceForWeb * pService);
    //取消初始化
    virtual bool UnInit();
    //开始服务
    virtual bool Start(UINT uAcceptThreadCount, UINT uSocketThradCount);
    //停止服务
    virtual bool Stop();
    //消息处理函数
    virtual bool OnNetMessage(CTCPSocketForWeb * pSocket, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uIndex, DWORD dwHandleID);

    //功能函数
public:
    //发送数据函数
    inline int SendData(UINT uIndex, const char * pData, UINT uBufLen);
    //发送数据函数
    inline int SendData(UINT uIndex, BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode, DWORD dwHandleID);
    //发送数据函数
    inline int SendData(UINT uIndex, void * pData, UINT uBufLen, BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode, DWORD dwHandleID);
    //发送数据函数
    inline int SendData(CTCPSocket * pSocket, void * pData, UINT uBufLen, BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode, DWORD dwHandleID);
    //批量发送函数
    inline int SendDataBatch(void * pData, UINT uSize, BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode);
    //组播发送函数
    inline int SendDataUnion(UINT uUnionID, void * pData, UINT uSize, BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode);
    //关闭连接
    inline bool CloseSocket(UINT uIndex, DWORD dwHandleID);
    //关闭连接
    inline bool CloseSocket(CTCPSocketForWeb * pSocket, DWORD dwHandleID);
    //判断是否连接
    inline bool IsConnectID(UINT uIndex, DWORD dwHandleID);
    //数值变 IP 字符
    static TCHAR * ULongToIP(ULONG dwIP, TCHAR * szIP);
    static ULONG IPToULong(TCHAR * szIP);
    //获取本机 IP 地址
    inline static bool GetLoaclIPAddr(TCHAR szAddrBuffer[], UINT uStation);
    //获取指定index的连接信息
    string GetServerInfo(UINT uIndex);
    //获取连接的端口号
    inline UINT GetListenPort(){return m_uListenPort;}
    //内核函数
private:
    //生成空闲 SOCKET
    inline CTCPSocketForWeb * TCPSocketNew();
    //释放活动 SOCKET
    inline bool TCPSocketDelete(CTCPSocketForWeb * pSocket);
    //SOCKET 连接应答线程
    inline static unsigned __stdcall ThreadAccept(LPVOID pThreadData);
    //SOCKET 数据接收线程
    inline static unsigned __stdcall ThreadRSSocket(LPVOID pThreadData);
    //SOCKET 批量发送数据线程
    inline static unsigned __stdcall ThreadBatchSend(LPVOID pThreadData);
    //SOCKET 组播发送数据线程
    inline static unsigned __stdcall ThreadUnionSend(LPVOID pThreadData);
    //SOCKET 维护线程
    inline static unsigned __stdcall ThreadKeepActive(LPVOID pThreadData);
    //获取时间函数
    inline static unsigned __int64 AFCGetTimer();
    //获取处理 ID 号码
    inline static DWORD GetHandleID();

public:
    //添加用户到群组
    inline int UnionAddUser(UINT uUnionID, UINT uSocketID);
    //从群组删除用户
    inline int UnionRemoveUser(UINT uUnionID, UINT uSocketID);
    //移除整个群组
    inline int UnionRemoveAll(UINT uUnionID);
    //用户下线移除用户所有群组
    inline int UnionRemovUserAll(UINT uSocketID);
};

#endif 
