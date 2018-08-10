
#include <odbc.h>

SQLHENV gEnv;
SQLHDBC gDbc;

int  checkLatency();
void PrintDiagnosticRecord( SQLSMALLINT aHandleType, SQLHANDLE aHandle );
int  initODBC();
int  allocRowStatus( RecordData* aRecordData );
int  setFetchArray( SQLHSTMT aStmt , RecordData* aRecordData );
int  setParamArray( SQLHSTMT aStmt , RecordData* aRecordData );
int  prepareSQL( SQLHSTMT aStmt , SQLCHAR* aQuery );
int  executeSQL( SQLHSTMT aStmt );
void printExecutedQueryType( SQLLEN aQueryType );
int  getExecutedQueryType( SQLHSTMT aStmt );
int  getResultSetMeta( SQLHSTMT aStmt , RecordInfo* aRecordInfo );
int  doFetch( SQLHSTMT aStmt ,  RecordInfo* aRecordInfo, RecordData* aRecordData );
void freeRecordInfo( RecordInfo* aRecordInfo );
void freeRecordData( RecordData* aRecordData );
int  allocColumnData( RecordInfo* aRecordInfo , RecordData* aRecordData );
int  bindColumns( SQLHSTMT aStmt, RecordInfo* aRecordInfo , RecordData* aRecordData );
int  printColumnData( RecordInfo* aRecordInfo , RecordData* aRecordData , SQLULEN aFetchedCount );


void PrintDiagnosticRecord( SQLSMALLINT aHandleType, SQLHANDLE aHandle )
{
    SQLCHAR       sSQLState[6];
    SQLINTEGER    sNaiveError;
    SQLSMALLINT   sTextLength;
    SQLCHAR       sMessageText[SQL_MAX_MESSAGE_LENGTH];
    SQLSMALLINT   sRecNumber = 1;
    SQLRETURN     sReturn;

    
    while( 1 )
    {
        sReturn = SQLGetDiagRec( aHandleType,
                                 aHandle,
                                 sRecNumber,
                                 sSQLState,
                                 &sNaiveError,
                                 sMessageText,
                                 SQL_MAX_MESSAGE_LENGTH,
                                 &sTextLength );

        if( sReturn == SQL_NO_DATA )
        {
            break;
        }

        STL_TRY( sReturn == SQL_SUCCESS );

        SET_ERROR("=============================================" );
        SET_ERROR("SQL_DIAG_SQLSTATE     : %s", sSQLState );
        SET_ERROR("SQL_DIAG_NATIVE       : %d", sNaiveError );
        SET_ERROR("SQL_DIAG_MESSAGE_TEXT : %s", sMessageText );
        SET_ERROR("=============================================" );

        sRecNumber++;
    }
    return;

    STL_FINISH;

    SET_ERROR("SQLGetDiagRec() failure.\n" );

    return;
}


int initODBC()
{
    STL_TRY_THROW( SQLAllocHandle( SQL_HANDLE_ENV,
                                   NULL,
                                   &gEnv ) == SQL_SUCCESS,
                   RAMP_ERR_ENV );

    STL_TRY_THROW( SQLSetEnvAttr( gEnv,
                                  SQL_ATTR_ODBC_VERSION,
                                  (SQLPOINTER)SQL_OV_ODBC3,
                                  0 ) == SQL_SUCCESS,
                   RAMP_ERR_ENV );

    STL_TRY_THROW( SQLAllocHandle( SQL_HANDLE_DBC,
                                   gEnv,
                                   &gDbc ) == SQL_SUCCESS,
                   RAMP_ERR_ENV );

    
#ifdef __PRINT_CONN__
    gettimeofday( & gConnection.mStart, NULL );
#endif
    STL_TRY_THROW( SQLConnect( gDbc,
                               (SQLCHAR*)gProperty.mDSN,
                               SQL_NTS,
                               (SQLCHAR*)gProperty.mUID,
                               SQL_NTS,
                               (SQLCHAR*)gProperty.mPWD,
                               SQL_NTS) == SQL_SUCCESS,
                   RAMP_ERR_DBC );
#ifdef __PRINT_CONN__
    gettimeofday( & gConnection.mEnd, NULL );

    getDiffTime( & gConnection );
#endif

    STL_TRY_THROW( SQLSetConnectAttr( gDbc,
                                      SQL_ATTR_AUTOCOMMIT,
                                      (SQLPOINTER)gProperty.mAutoCommit,
                                      SQL_IS_UINTEGER) == SQL_SUCCESS,
                   RAMP_ERR_DBC );

    

    return SUCCESS;

    STL_CATCH( RAMP_ERR_ENV )
    {
        PrintDiagnosticRecord( SQL_HANDLE_ENV, gEnv );
    }

    STL_CATCH( RAMP_ERR_DBC )
    {
        PrintDiagnosticRecord( SQL_HANDLE_DBC, gDbc );
    }

    STL_FINISH;

    SET_ERROR( "initialize ODBC, fail." );
    
    return FAIL;

}

int allocRowStatus( RecordData* aRecordData )
{
    aRecordData->mRowStatus = calloc( gProperty.mArray, sizeof( SQLUSMALLINT ) );

    STL_TRY( aRecordData->mRowStatus != NULL );

    return SUCCESS;

    STL_FINISH;

    SET_ERROR( "row_status allocate failed. %s", strerror( errno ) );

    return FAIL;
}

int setFetchArray( SQLHSTMT aStmt , RecordData* aRecordData )
{
    STL_TRY_THROW( SQLSetStmtAttr( aStmt,
                                   SQL_ATTR_ROWS_FETCHED_PTR,
                                   (SQLPOINTER) & aRecordData->mCurrentCount,
                                   0 ) == SQL_SUCCESS,
                   RAMP_ERR_STMT );

    STL_TRY_THROW( SQLSetStmtAttr( aStmt,
                                   SQL_ATTR_ROW_ARRAY_SIZE,
                                   (SQLPOINTER) gProperty.mArray,
                                   0 ) == SQL_SUCCESS,
                   RAMP_ERR_STMT );

    STL_TRY_THROW( SQLSetStmtAttr( aStmt,
                                   SQL_ATTR_ROW_STATUS_PTR,
                                   (SQLPOINTER) aRecordData->mRowStatus,
                                   0 ) == SQL_SUCCESS,
                   RAMP_ERR_STMT );

    return SUCCESS;

    STL_CATCH( RAMP_ERR_STMT )
    {
        PrintDiagnosticRecord( SQL_HANDLE_STMT, aStmt );
    }

    STL_FINISH;

    return FAIL;
}

int setParamArray( SQLHSTMT aStmt , RecordData* aRecordData )
{          
    STL_TRY_THROW( SQLSetStmtAttr( aStmt,
                                   SQL_ATTR_PARAM_BIND_TYPE,
                                   (SQLPOINTER)SQL_PARAM_BIND_BY_COLUMN,
                                   0 ) == SQL_SUCCESS,
                   RAMP_ERR_STMT );

    STL_TRY_THROW( SQLSetStmtAttr( aStmt,
                                   SQL_ATTR_PARAMSET_SIZE,
                                   (SQLPOINTER) gProperty.mArray,
                                   0 ) == SQL_SUCCESS,
                   RAMP_ERR_STMT );

    return SUCCESS;

    STL_CATCH( RAMP_ERR_STMT )
    {
        PrintDiagnosticRecord( SQL_HANDLE_STMT, aStmt );
    }

    STL_FINISH;

    return FAIL;
}


int prepareSQL( SQLHSTMT aStmt , SQLCHAR* aQuery )
{
    if( gProperty.mExecuteType == PREPARE_EXECUTE )
    {
        gettimeofday( &gPrepare.mStart, NULL );
        STL_TRY_THROW( SQLPrepare( aStmt,
                                   (SQLCHAR*)gProperty.mQuery,
                                   strlen( gProperty.mQuery ) ) == SQL_SUCCESS,
                       RAMP_ERR_STMT );

        gettimeofday( &gPrepare.mEnd, NULL );

        getDiffTime( & gPrepare );
    }
    else
    {
        /* do nothing */
    }

    return SUCCESS;

    STL_CATCH( RAMP_ERR_STMT )
    {
        PrintDiagnosticRecord( SQL_HANDLE_STMT, aStmt );
    }

    STL_FINISH;

    return FAIL;
}

int executeSQL( SQLHSTMT aStmt )
{
    if( gProperty.mExecuteType == DIRECT_EXECUTE )
    {
        gettimeofday( &gExecute.mStart, NULL );
        STL_TRY_THROW( SQLExecDirect( aStmt,
                                      (SQLCHAR*)gProperty.mQuery,
                                      strlen( gProperty.mQuery ) ) == SQL_SUCCESS,
                       RAMP_ERR_STMT );       
        gettimeofday( &gExecute.mEnd, NULL );
    }
    else if( gProperty.mExecuteType == PREPARE_EXECUTE )
    {
        gettimeofday( &gExecute.mStart, NULL );
        STL_TRY_THROW( SQLExecute( aStmt ) == SQL_SUCCESS,
                       RAMP_ERR_STMT );
        gettimeofday( &gExecute.mEnd, NULL );
    }
    else
    {
        SET_ERROR( "invalid execute type." );
        STL_THROW( STL_FINISH_LABEL );
    }

    getDiffTime( & gExecute );
    

    return SUCCESS;

    STL_CATCH( RAMP_ERR_STMT )
    {
        PrintDiagnosticRecord( SQL_HANDLE_STMT, aStmt );
    }

    STL_FINISH;

    return FAIL;
}

void printExecutedQueryType( SQLLEN aQueryType )
{
    switch( aQueryType )
    {
        case SQL_DIAG_DELETE_WHERE:
            LOGGER( "Query Type : DLETE" );
            break;
            
        case SQL_DIAG_INSERT:
            LOGGER( "Query Type : INSERT" );
            break;
            
        case SQL_DIAG_SELECT_CURSOR:
            LOGGER( "Query Type : SELECT" );
            break;
            
        case SQL_DIAG_UPDATE_WHERE:
            LOGGER( "Query Type : UPDATE" );
            break;
            
        case SQL_DIAG_UNKNOWN_STATEMENT:
        default:
            LOGGER( "Query Type : unknown [%d]", aQueryType );
            break;
            
    }

    return;
}

int getExecutedQueryType( SQLHSTMT aStmt )
{
    SQLLEN sDiagInfo;

    
    STL_TRY_THROW( SQLGetDiagField( SQL_HANDLE_STMT,
                                    aStmt,
                                    0,
                                    SQL_DIAG_DYNAMIC_FUNCTION_CODE,
                                    & sDiagInfo,
                                    0,
                                    0 ) == SQL_SUCCESS,
                   RAMP_ERR_STMT );

    return sDiagInfo;

    STL_CATCH( RAMP_ERR_STMT )
    {
        PrintDiagnosticRecord( SQL_HANDLE_STMT, aStmt );
    }

    STL_FINISH;

    return FAIL;
                                    
                                    
}

int getResultSetMeta( SQLHSTMT aStmt , RecordInfo* aRecordInfo )
{
    SQLSMALLINT sColumnCount = 0;
    SQLRETURN   sRet         = SQL_ERROR;
    SQLHDESC    sDesc        = NULL;
    int         sIndex       = 0;

    if( aRecordInfo->mColumn != NULL )
    {
        freeRecordInfo( aRecordInfo );       
    }

    memset( (void*)aRecordInfo, 0x00, sizeof( RecordInfo ) );

    STL_TRY_THROW( SQLGetStmtAttr( aStmt,
                                   SQL_ATTR_IMP_ROW_DESC,
                                   & sDesc,
                                   SQL_IS_POINTER,
                                   NULL ) == SQL_SUCCESS,
                   RAMP_ERR_STMT );

    STL_TRY_THROW( SQLNumResultCols( aStmt,
                                     & sColumnCount ) == SQL_SUCCESS,
                   RAMP_ERR_STMT );

    aRecordInfo->mColumnCount = sColumnCount;
    aRecordInfo->mColumn = calloc( sColumnCount, sizeof( ColumnInfo ) );

    for( sIndex = 0; sIndex < sColumnCount; sIndex++ )
    {
        sRet = SQLDescribeCol( aStmt,
                               sIndex+1,
                               aRecordInfo->mColumn[ sIndex ].mColName,
                               (SQLSMALLINT)BUF_LEN,
                               & aRecordInfo->mColumn[ sIndex ].mColNameLen,
                               & aRecordInfo->mColumn[ sIndex ].mColType,
                               & aRecordInfo->mColumn[ sIndex ].mColSize,
                               & aRecordInfo->mColumn[ sIndex ].mColDecimalDigits,
                               & aRecordInfo->mColumn[ sIndex ].mColNullablePtr );
        STL_TRY_THROW( sRet == SQL_SUCCESS,
                       RAMP_ERR_STMT );

        sRet = SQLGetDescField( sDesc,
                                sIndex+1,
                                SQL_DESC_DISPLAY_SIZE,
                                (SQLPOINTER)& aRecordInfo->mColumn[ sIndex].mColDisplaySize,
                                SQL_IS_POINTER,
                                NULL );
        STL_TRY_THROW( sRet == SQL_SUCCESS,
                       RAMP_ERR_STMT );
    }

    return SUCCESS;

    STL_CATCH( RAMP_ERR_STMT )
    {
        PrintDiagnosticRecord( SQL_HANDLE_STMT, aStmt );
    }
    
    STL_FINISH;

    if( aRecordInfo->mColumn != NULL )
    {
        free( aRecordInfo->mColumn );
    }

    return FAIL;
}

int doFetch( SQLHSTMT aStmt , RecordInfo* aRecordInfo , RecordData* aRecordData )
{
    SQLRETURN sRet = SQL_ERROR;
    LatencyData sTempData;

    memset( & sTempData, 0x00, sizeof( sTempData ) );

    while( 1 )
    {
#ifdef __PRINT_FETCH__
        gettimeofday( &gFetch.mStart, NULL );
#endif
        sRet = SQLFetch( aStmt );
#ifdef __PRINT_FETCH__
        gettimeofday( &gFetch.mEnd, NULL );
#endif
        if( sRet == SQL_SUCCESS )
        {
            printColumnData( aRecordInfo , aRecordData , aRecordData->mCurrentCount );
            aRecordData->mTotalCount += aRecordData->mCurrentCount;
        }
        else if( sRet == SQL_NO_DATA )
        {
            break;
        }
        else if( sRet == SQL_SUCCESS_WITH_INFO )
        {
            PrintDiagnosticRecord( SQL_HANDLE_STMT, aStmt );
            break;
        }
        else
        {
            PrintDiagnosticRecord( SQL_HANDLE_STMT, aStmt );
            STL_THROW( STL_FINISH_LABEL );
        }

#ifdef __PRINT_FETCH__
        getDiffTime( & gFetch );
        sTempData.mDiff += gFetch.mDiff;
#endif
    }

#ifdef __PRINT_FETCH__
    gFetch.mDiff = sTempData.mDiff;
#endif

    if( gOutfileFp != NULL )
    {
        fclose( gOutfileFp );
    }

    return SUCCESS;

    STL_FINISH;

    return FAIL;
}

void freeRecordInfo( RecordInfo* aRecordInfo )
{
    if( aRecordInfo->mColumn != NULL )
    {
        free( aRecordInfo->mColumn );
    }
   
    memset( (void*)aRecordInfo, 0x00 , sizeof( RecordInfo ) );
}

void freeRecordData( RecordData* aRecordData )
{
    int sIndex = 0;

    for( sIndex = 0; sIndex < aRecordData->mColDataCount; sIndex++ )
    {
        if( aRecordData->mColData[ sIndex ].mData != NULL )
        {
            free( aRecordData->mColData[ sIndex ].mData );
            aRecordData->mColData[ sIndex ].mData = NULL;
        }

        if( aRecordData->mColData[ sIndex ].mDataInd != NULL )
        {
            free( aRecordData->mColData[ sIndex ].mDataInd );
            aRecordData->mColData[ sIndex ].mDataInd = NULL;
        }
        
    }

    if( aRecordData->mRowStatus != NULL )
    {
        free( aRecordData->mRowStatus );
        aRecordData->mRowStatus = NULL;
    }

    memset( (void*)aRecordData, 0x00 , sizeof( RecordData ) );
}

int allocColumnData( RecordInfo* aRecordInfo , RecordData* aRecordData )
{
    int sIndex = 0; 

    if( aRecordData->mColData != NULL )
    {
        freeRecordData( aRecordData );       
    }

    memset( (void*)aRecordData, 0x00, sizeof( RecordData ) );
    
    aRecordData->mColData = calloc( aRecordInfo->mColumnCount , sizeof( ColumnData ) );

    STL_TRY( aRecordData->mColData != NULL );

    for( sIndex = 0; sIndex < aRecordInfo->mColumnCount; sIndex++ )
    {
        aRecordData->mColData[ sIndex ].mData =
            calloc( (aRecordInfo->mColumn[ sIndex ].mColDisplaySize+1) , gProperty.mArray );

        STL_TRY( aRecordData->mColData[ sIndex ].mData != NULL );

		aRecordData->mColData[ sIndex ].mDataInd =
			calloc( sizeof( SQLLEN ), gProperty.mArray );

        STL_TRY( aRecordData->mColData[ sIndex ].mDataInd != NULL );
    }

    aRecordData->mRowStatus = calloc( gProperty.mArray, sizeof( SQLUSMALLINT ) );

    STL_TRY( aRecordData->mRowStatus != NULL );

    return SUCCESS;

    STL_FINISH;

    for( sIndex = 0; sIndex < aRecordInfo->mColumnCount; sIndex++ )
    {
        if( aRecordData->mColData[ sIndex ].mData != NULL )
        {
            free( aRecordData->mColData[ sIndex ].mData );
            aRecordData->mColData[ sIndex ].mData = NULL;
        }
    }

    SET_ERROR( "memory allocation failed. %s", strerror( errno ) );

    return FAIL;
}

int bindColumns( SQLHSTMT aStmt, RecordInfo* aRecordInfo , RecordData* aRecordData )
{
    int sIndex = 0;
    SQLRETURN sRet = SQL_ERROR;

#ifdef __PRINT_BIND__
    gettimeofday( & gBindCol.mStart, NULL );
#endif
    for( sIndex = 0; sIndex < aRecordInfo->mColumnCount; sIndex++ )
    {
        sRet = SQLBindCol( aStmt,
                           sIndex+1,
                           SQL_C_CHAR,
                           aRecordData->mColData[ sIndex ].mData,
                           aRecordInfo->mColumn[ sIndex].mColDisplaySize+1,
                           aRecordData->mColData[ sIndex ].mDataInd );
        STL_TRY_THROW( sRet == SQL_SUCCESS,
                       RAMP_ERR_STMT );
    }
#ifdef __PRINT_BIND__
    gettimeofday( & gBindCol.mEnd, NULL );

    getDiffTime( & gBindCol );
#endif

    return SUCCESS;

    STL_CATCH( RAMP_ERR_STMT )
    {
        PrintDiagnosticRecord( SQL_HANDLE_STMT, aStmt );
    }

    STL_FINISH;

    return FAIL;
}

int checkLatency()
{
    SQLHSTMT sStmt = NULL;
    RecordInfo sRecordInfo;
    RecordData sRecordData;
    int sIndex = 0;
	int sCount = 1;

    memset( &sRecordInfo, 0x00, sizeof( sRecordInfo ));
    memset( &sRecordData, 0x00, sizeof( sRecordData ));

    STL_TRY_THROW( SQLAllocHandle( SQL_HANDLE_STMT,
                                   gDbc,
                                   & sStmt ) == SQL_SUCCESS,
                   RAMP_ERR_DBC );

    STL_TRY( prepareSQL( sStmt , (SQLCHAR*)gProperty.mQuery ) == SUCCESS );

    for( sIndex = 0; sIndex < gProperty.mRepeat; sIndex++ )
    {
        gExecute.mDiff          = 0;
        gBindCol.mDiff          = 0;
        gFetch.mDiff            = 0;
        sRecordData.mTotalCount = 0;
        
        STL_TRY( executeSQL( sStmt ) == SUCCESS );
        
        gProperty.mQueryType = getExecutedQueryType( sStmt );

        if( gProperty.mQueryType == SQL_DIAG_SELECT_CURSOR )
        {
            STL_TRY( getResultSetMeta( sStmt ,
                                       & sRecordInfo ) == SUCCESS );

            STL_TRY( allocColumnData( & sRecordInfo,
                                      & sRecordData ) == SUCCESS );

            STL_TRY( setFetchArray( sStmt,
                                    & sRecordData ) == SUCCESS );

            STL_TRY( bindColumns( sStmt,
                                  & sRecordInfo,
                                  & sRecordData ) == SUCCESS );

            STL_TRY( doFetch( sStmt , 
                              & sRecordInfo,
                              & sRecordData ) == SUCCESS );

			SQLCloseCursor( sStmt );
        }
        else if( gProperty.mQueryType == SQL_DIAG_INSERT       ||
                 gProperty.mQueryType == SQL_DIAG_UPDATE_WHERE ||
                 gProperty.mQueryType == SQL_DIAG_DELETE_WHERE )
        {
            // array 및 멀티 execute 고려하지 않음.. 사실 파라미터를 지원한기 때문에 의미 없음 
            gProperty.mArray = 1;

            STL_TRY_THROW( SQLRowCount( sStmt,
                                        (SQLLEN*)&sRecordData.mCurrentCount ) == SQL_SUCCESS,
                           RAMP_ERR_STMT );

            sRecordData.mTotalCount += sRecordData.mCurrentCount;

			if( (sCount % gProperty.mCommitInterval) == 0 )
			{
				gettimeofday( & gCommit.mStart, NULL );
				SQLEndTran( SQL_HANDLE_DBC, gDbc, SQL_COMMIT );
				gettimeofday( & gCommit.mEnd, NULL );
				getDiffTime( & gCommit );
				gTotCommitCount++;
			}

            if( gProperty.mUSleep > 0 )
            {
                usleep( gProperty.mUSleep );
            }
            
        }
	else if( gProperty.mQueryType == SQL_DIAG_CALL )
	{
            gProperty.mArray = 1;

            sRecordData.mTotalCount += 1;

			if( (sCount % gProperty.mCommitInterval) == 0 )
			{
				gettimeofday( & gCommit.mStart, NULL );
				SQLEndTran( SQL_HANDLE_DBC, gDbc, SQL_COMMIT );
				gettimeofday( & gCommit.mEnd, NULL );
				getDiffTime( & gCommit );
				gTotCommitCount++;
			}

            if( gProperty.mUSleep > 0 )
            {
                usleep( gProperty.mUSleep );
            }

	}
        else
        {
            SET_ERROR( "not supported query type. %d", gProperty.mQueryType );
        }

        gExecuteTot.mDiff += gExecute.mDiff;

#ifdef __PRINT_BIND__
        gBindColTot.mDiff += gBindCol.mDiff;
#endif

#ifdef __PRINT_FETCH__
        gFetchTot.mDiff   += gFetch.mDiff;
#endif

#ifdef __PRINT_COMMIT__
        gCommitTot.mDiff  += gCommit.mDiff;
#endif



        gTotRecordCount += sRecordData.mTotalCount;

        if( gProperty.mQueryType == SQL_DIAG_SELECT_CURSOR )
        {
            freeRecordData( & sRecordData );
            freeRecordInfo( & sRecordInfo );
        }


    }

    printExecutedQueryType( gProperty.mQueryType );

    if( gProperty.mQueryType == SQL_DIAG_SELECT_CURSOR )
    {
        LOGGER( "Fetched Record Count : %lu", gTotRecordCount );
        LOGGER( "" );
    }
    else if( gProperty.mQueryType == SQL_DIAG_INSERT ||
             gProperty.mQueryType == SQL_DIAG_UPDATE_WHERE ||
             gProperty.mQueryType == SQL_DIAG_DELETE_WHERE )
    {
        LOGGER( "Affected Record Count : %lu", gTotRecordCount );
        LOGGER( "" );
    }

    if( sStmt != NULL )
    {
        SQLFreeHandle( SQL_HANDLE_STMT, sStmt );
    }

    if( gDbc != NULL )
    {
        SQLEndTran( SQL_HANDLE_DBC, gDbc, SQL_COMMIT );
        SQLDisconnect( gDbc );
        SQLFreeHandle( SQL_HANDLE_DBC, gDbc );
    }

    if( gEnv != NULL )
    {
        SQLFreeHandle( SQL_HANDLE_ENV, gEnv );
    }

    printLatencyData();

    return SUCCESS;

    STL_CATCH( RAMP_ERR_DBC )
    {
        PrintDiagnosticRecord( SQL_HANDLE_DBC, gDbc );
    }

    STL_CATCH( RAMP_ERR_STMT )
    {
        PrintDiagnosticRecord( SQL_HANDLE_STMT, sStmt );
    }

    STL_FINISH;

    if( sStmt != NULL )
    {
        SQLFreeHandle( SQL_HANDLE_STMT, sStmt );
    }

    if( gDbc != NULL )
    {
        SQLEndTran( SQL_HANDLE_DBC, gDbc, SQL_ROLLBACK );
        SQLDisconnect( gDbc );
        SQLFreeHandle( SQL_HANDLE_DBC, gDbc );
    }

    if( gEnv != NULL )
    {
        SQLFreeHandle( SQL_HANDLE_ENV, gEnv );
    }

    return FAIL;
                                   
}


int printColumnData( RecordInfo* aRecordInfo , RecordData* aRecordData, SQLULEN aFetchedCount )
{
    int sRecordIndex;
    int sFetchedCount = aFetchedCount;

    int sColumnSize;
    int sColumnIndex;
    int sColumnCount = aRecordInfo->mColumnCount;

    for( sRecordIndex = 0; sRecordIndex < sFetchedCount; sRecordIndex++ )
    {
        for( sColumnIndex = 0; sColumnIndex < sColumnCount; sColumnIndex++ )
        {
            sColumnSize = aRecordInfo->mColumn[ sColumnIndex ].mColDisplaySize+1;
            fwrite( aRecordData->mColData[ sColumnIndex ].mData + (sColumnSize * sRecordIndex ),
                    aRecordData->mColData[ sColumnIndex ].mDataInd[ sRecordIndex ],
                    1,
                    gOutfileFp );

            if( sColumnIndex == sColumnCount-1 )
            {
                continue;
            }
            else
            {
                fprintf( gOutfileFp, "," );
            }
        }
        fprintf( gOutfileFp , "\n" );

    }

    return SUCCESS;
}
