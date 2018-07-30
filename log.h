



#include <common.h>

#ifndef _LOG_H_
#define _LOG_H_

#define MAX_ERROR_MESSAGE_LEN   64*1024 

extern FILE*    gLogStream;
extern char     gErrorMessage[ MAX_ERROR_MESSAGE_LEN ];

void logPrint( int aLineNo, const char* aFormat, ... );
void getStringDateTime( char* aBuffer, int aLength );
int setLogger( char* aLogFile );


#define PRINT( ... ) Print(__VA_ARGS__)
#define LOGGER( ... ) logPrint( __LINE__, __VA_ARGS__ )
#define SET_ERROR( ... ) do {                                       \
   snprintf( gErrorMessage , MAX_ERROR_MESSAGE_LEN , __VA_ARGS__ ); \
   logPrint( __LINE__, gErrorMessage );                             \
                            }while(0)



#endif
