
#include <common.h>
#include <json-c/json.h>
#include <log.h>



#ifndef _PROPERTY_H_
#define _PROPERTY_H_

extern struct _Property gProperty;

#define MAX_DATABASE_LEN (256)
#define MAX_DSN_LEN      (256)
#define MAX_UID_LEN      (256)
#define MAX_PWD_LEN      (256)
#define MAX_QUERY_LEN    (1024*64)
#define MAX_OUTFILE_LEN  (256)

#define AUTOCOMMIT_OFF  (0)
#define AUTOCOMMIT_ON   (1)

#define DIRECT_EXECUTE  (0)
#define PREPARE_EXECUTE (1)

#define MAX_CONTROL_FILE_LEN (256)

#define MAX_KEY_LEN   (256)
#define MAX_VALUE_LEN (256)

typedef struct _Property {
    char  mDatabase[ MAX_DATABASE_LEN +1 ];
    char  mDSN[ MAX_DSN_LEN +1 ];
    char  mUID[ MAX_UID_LEN +1 ];
    char  mPWD[ MAX_PWD_LEN +1 ];
    unsigned long  mArray;
    unsigned long  mAutoCommit;
    useconds_t     mUSleep;
    int   mRepeat;
    int   mExecuteType;
    int   mQueryType;    
    int   mCommitInterval;
    char  mQuery[ MAX_QUERY_LEN +1 ];
    char  mOutfile[ MAX_OUTFILE_LEN +1 ];
}Property;

int setControlFile( const char* aControlFile );
int initProperty();


#endif
