#include "StdAfx.h"
#include "LogModule.h"

CLogModule CLogModule::m_pLogModule; 
CLogModule::CLogModule( void )
{

}

CLogModule::~CLogModule( void )
{
   
}

/// »’÷æ
void CLogModule::WriteLog( const char *pPutFromat, ... )
{
	if( pPutFromat == NULL )
	{
		return;
	}
	char szFilename[256];
	CString strPath = CINIFile::GetAppPath( ) +"\\GameLog\\";
	SHCreateDirectoryEx( NULL, strPath, NULL );
	CTime time = CTime::GetCurrentTime( );
	sprintf( szFilename, "%s%d_%d%02d%02d_UserLog.csv", strPath , NAME_ID, time.GetYear( ), time.GetMonth( ), time.GetDay( ) );
	FILE *fp = fopen( szFilename, "a" );
	if ( NULL == fp )
	{
		return;
	}
	char s[1024];
	va_list arg_ptr;
	va_start( arg_ptr, pPutFromat );
	_vsnprintf( s, sizeof( s )-1, pPutFromat, arg_ptr );
	fprintf( fp, "%s", s );
	fclose( fp );
}

void CLogModule::LogCharToChar( BYTE src[], string & dst, int len )
{
	for( int j=0;j<len;j++ )
	{
		if( src[j]!=255 )
		{
			char tem[10];
			sprintf( tem, "%d ", src[j] );
			dst += string( tem );
		}   
	}
}

void CLogModule::DebugPrintf( const char *p, ... )
{
	CTime time = CTime::GetCurrentTime( );
	char szFilename[256];
	sprintf( szFilename, "%d-%d%d%d-log.txt", NAME_ID, time.GetYear( ), time.GetMonth( ), time.GetDay( ) );
	FILE *fp = fopen( szFilename, "a" );
	if ( NULL == fp )
	{
		return;
	}
	va_list arg;
	va_start( arg, p );
	vfprintf( fp, p, arg );
	fprintf( fp, "\n" );

	fclose( fp );
}