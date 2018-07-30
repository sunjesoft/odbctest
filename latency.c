

#include <latency.h>

LatencyData gConnection;
LatencyData gPrepare;
LatencyData gExecute;
LatencyData gExecuteTot;
LatencyData gBindCol;
LatencyData gBindColTot;
LatencyData gFetch;
LatencyData gFetchTot;
LatencyData gCommit;
LatencyData gCommitTot;

extern int gTotRecordCount;

void initLatencyData()
{
    memset( & gConnection, 0x00 , sizeof( LatencyData ) );
    memset( & gPrepare   , 0x00 , sizeof( LatencyData ) );
    memset( & gExecute   , 0x00 , sizeof( LatencyData ) );
    memset( & gExecuteTot, 0x00 , sizeof( LatencyData ) );
    memset( & gBindCol   , 0x00 , sizeof( LatencyData ) );
    memset( & gBindColTot, 0x00 , sizeof( LatencyData ) );
    memset( & gFetch     , 0x00 , sizeof( LatencyData ) );
    memset( & gFetchTot  , 0x00 , sizeof( LatencyData ) );

    gTotRecordCount = 0;

    if( strlen( gProperty.mOutfile ) > 0 )
    {
        gOutfileFp = fopen( gProperty.mOutfile, "w" );
        if( gOutfileFp == NULL )
        {
            SET_ERROR( "outfile : %s\n", strerror( errno ) );
        }
    }
    else
    {
        gOutfileFp = NULL;
    }
}

void getDiffTime( LatencyData* aLatencyData )
{
    aLatencyData->mDiff = (unsigned long) (aLatencyData->mEnd.tv_sec - aLatencyData->mStart.tv_sec) * 1000000  +
        (aLatencyData->mEnd.tv_usec - aLatencyData->mStart.tv_usec);
}


void printLatencyData()
{

    if( gTotRecordCount == 0 )
        gTotRecordCount++;

    LOGGER( "=== Result data of latency ===" );
    LOGGER( "" );
    LOGGER( "Repeat of Execution : %d", gProperty.mRepeat );
    LOGGER( "" );
#ifdef __PRINT_CONN__
    LOGGER( "   %25s : %lu usec",
            "[Connection Latency]",
            gConnection.mDiff);
#endif

    if( gProperty.mExecuteType == PREPARE_EXECUTE )
    {
        LOGGER( "   %25s : %lu usec",
                "[Prepare Latency]",
                gPrepare.mDiff);
    }

    LOGGER( "   %25s : %lu usec",
            "[Execute Avg Latency]",
            gExecuteTot.mDiff / gProperty.mRepeat);

    if( gProperty.mQueryType == SQL_DIAG_SELECT_CURSOR )
    {
#ifdef __PRINT_BIND__
        LOGGER( "   %25s : %lu usec",
                "[BindCol Avg Latency]",
                gBindColTot.mDiff / gProperty.mRepeat);
#endif        

#ifdef __PRINT_FETCH__
        LOGGER( "   %25s : %lu usec",
                "[Fetch Avg Latency]",
                gFetchTot.mDiff / gTotRecordCount); 
#endif
    }

    LOGGER( "   %25s : %lu usec",
            "[Execute Tot Latency]",
            gExecuteTot.mDiff);

    if( gProperty.mQueryType == SQL_DIAG_SELECT_CURSOR )
    {

#ifdef __PRINT_BIND__
        LOGGER( "   %25s : %lu usec",
                "[BindCol Tot Latency]",
                gBindColTot.mDiff);
#endif

#ifdef __PRINT_FETCH__
        LOGGER( "   %25s : %lu usec",
                "[Fetch Tot Latency]",
                gFetchTot.mDiff);
#endif
    }

}
