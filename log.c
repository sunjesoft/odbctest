

#include <log.h>
#include <time.h>
#include <stdarg.h>

FILE* gLogStream;
char  gErrorMessage[ MAX_ERROR_MESSAGE_LEN ];

void logPrint( int aLineNo, const char* aFormat, ... )
{
    va_list sArgPtr;
    char    sDateTime[ 50 ];

    getStringDateTime( sDateTime , 50 );

    fprintf( gLogStream, "[%s]: ", sDateTime );

    va_start( sArgPtr, aFormat );
    vfprintf( gLogStream, aFormat, sArgPtr );
    va_end( sArgPtr );

    fputc( '\n', gLogStream );
    fflush( gLogStream );
}

void Print( const char* aFormat, ... )
{
    va_list sArgPtr;
    
    va_start( sArgPtr, aFormat );
    vfprintf( gLogStream, aFormat, sArgPtr );
    va_end( sArgPtr );

    fputc( '\n', gLogStream );
    fflush( gLogStream );
}

void getStringDateTime( char* aBuffer, int aLength )
{
    time_t  sNow;
    struct tm* sTime;

    sNow  = time((time_t*)NULL);
    sTime = localtime(&sNow);

    snprintf( aBuffer , aLength , "%.4d-%.2d-%.2d %.2d:%.2d:%.2d",
            sTime->tm_year+1900, sTime->tm_mon+1, sTime->tm_mday,
            sTime->tm_hour , sTime->tm_min , sTime->tm_sec );
}

int setLogger( char* aLogFile )
{
    if( strcasecmp( aLogFile , "STDOUT" ) == 0 )
    {
        gLogStream = stdout;
    }
    else
    {
        gLogStream = fopen( aLogFile , "w" );
        if( gLogStream == NULL )
        {
            fprintf( stderr, "log file open failed(%s): %s\n", aLogFile , strerror( errno ) );
            return FAIL;
        }
    }

    return SUCCESS;
}
