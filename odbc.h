

#include <common.h>
#include <log.h>
#include <property.h>
#include <latency.h>

#ifdef __GOLDILOCKS__
#include <goldilocks.h>
#else
#include <sundb.h>
#endif

#ifndef _ODBC_H_
#define _ODBC_H_

#define BUF_LEN (256)

int initODBC();
int checkLatency();

typedef struct _ColumnStruct{
    SQLCHAR      mColName[BUF_LEN];
    SQLSMALLINT  mColNameLen;
    SQLSMALLINT  mColType;
    SQLULEN      mColSize;
    SQLSMALLINT  mColDecimalDigits;
    SQLSMALLINT  mColNullablePtr;
    SQLLEN       mColOctetLength;
    SQLINTEGER   mColLeadingPrecision;
    SQLLEN       mColDisplaySize;
   
}ColumnInfo;

typedef struct _RecordStruct{
    ColumnInfo* mColumn;
    int         mColumnCount;
}RecordInfo;

typedef struct _ColumnDataStruct{
    SQLLEN*     mDataInd;
    SQLCHAR*    mData;
}ColumnData;

typedef struct _RecordDataStruct{
    ColumnData*   mColData;
    int           mColDataCount;
    SQLULEN       mCurrentCount;
    SQLULEN       mTotalCount;
    SQLUSMALLINT* mRowStatus;     
}RecordData;


#endif
