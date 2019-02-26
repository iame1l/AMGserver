#ifndef MSG_BASE
#define MSG_BASE

#include <fstream>
#include <map>
#include <vector>
#include <list>
#include <iterator>
#include <algorithm>
#include <string>
using namespace std;
#pragma pack( 1 )

struct MsgHead
{
	int		_msgID;
	int		_msgLen;
	int		_stationLen;
	__int64	_time;
	
	MsgHead( )
	{
		memset( this, 0, sizeof( MsgHead ));
	}
};

struct UserInfor
{
	char		szNickName[61];			//昵称
	char		szheadUrl[256];			//头像地址
	UINT		nLogoID;				//头像ID
	__int64		i64UserMoney;			//玩家金币
	bool		bBoy;					///性别
};

struct RecordUserInfo
{

	BYTE		bWatchDeskView;						//观看的玩家作为

	char		szDeskPassWord[20];					//桌子密码

	bool		bVIPRoom;							//是否VIP房间
	bool		bBuyRoom;							//是否可购买房间

	bool		bHasPassword;						//是否有密码

	int			nPlayCount;							//玩家数目

	UINT		nComType;							//游戏类型
	UINT		nKindID;							//游戏类型ID
	UINT		nNameID;							//游戏名称ID
	UINT		dwRoomRule;							//游戏房间规则

	int			nBasePoint;							//基础倍数

	int			iVipGameCount;						//购买桌子局数

	int			iRunGameCount;						// 当前运行局数

	UserInfor	userinfo[PLAY_COUNT];

	RecordUserInfo( )
	{
		memset( this, 0, sizeof( RecordUserInfo ));
	}
};

#pragma pack( )

/*
*\brief文件操作类, 包含打开文件，附加消息，读取消息
*/
class zNewFileOperator
{
private:
	FILE*				m_file;					//文件流
	char*				m_fileName;				//文件名
public:
	zNewFileOperator( )
	{
		m_file = NULL;
	}
	virtual ~zNewFileOperator( )
	{

	}
	/*
	*\brief 设置文件名
	*para fileName创建文件的名称
	*/
	void	setFileName( char *fileName )
	{
		if ( m_fileName != NULL)
		{
			free( m_fileName );
			m_fileName = NULL;
		}

		int iLen			= strlen( fileName );
		m_fileName		= ( char * )malloc( ( iLen + 1 ) * sizeof( char ) );
		memcpy( m_fileName , fileName, iLen * sizeof( char ));
		m_fileName[iLen]	= '\0';
	}
	/*
	*\brief穿件文件，保证文件常驻内存
	*para write 读写模式
	*/
	bool	createFile( bool write = true )
	{
		char Mode[2];
		if ( write )
		{
			sprintf( Mode, "wb" );
		}
		else
		{
			sprintf( Mode, "rb" );
		}
		m_file = fopen( m_fileName, Mode );
		return m_file != NULL;
	}
	/*
	*\brief向文件流附加消息包
	*para property 需要添加的消息包
	*/
	void	appendProperty( int nMsgId, void *buffer, int nLen, void *stationBuffer, int nStationLen )
	{
		if ( m_file == NULL )
		{
			return ;
		}
		MsgHead head;
		head._msgID			= nMsgId;
		head._time			= time( NULL );
		head._msgLen		= nLen;
		head._stationLen	= nStationLen;
		fwrite( &head, sizeof( head ), 1, m_file ); // 写1个结构
		if ( nStationLen )
		{
			fwrite( stationBuffer, nStationLen, 1, m_file ); // 写1个结构
		}
		fwrite( buffer, nLen, 1, m_file ); // 写1个结构
		//fflush( file );
	}

	void CloseFile( )
	{
		if ( m_file != NULL )
		{
			fclose( m_file );
			m_file = NULL;
		}
	}

	void RenameFile( char *_newFileName )
	{
		rename( m_fileName, _newFileName );
	}
	void RemoveFile( )
	{
		remove( m_fileName );
	}
	void CutAndReName( char* _newFileName )
	{
		RenameFile( _newFileName );
		RemoveFile( );
	}
};

#endif