

#include <latency.h>

HTIMER* gConnection;
HTIMER* gPrepare;
HTIMER* gExecute;
HTIMER* gBindCol;
HTIMER* gFetch;
HTIMER* gCommit;

int   gTotRecordCount;
int   gTotCommitCount;
int   gTotFetchCount;
FILE* gOutfileFp;

void initTimer()
{
    enum unit_type unit = gProperty.mTimerUnit;
    int begin    = gProperty.mTimerBegin;
    int interval = gProperty.mTimerInterval;
    int count    = gProperty.mTimerCount;
    gConnection = init_timer( unit, begin, interval, count );
    gPrepare    = init_timer( unit, begin, interval, count );
    gExecute    = init_timer( unit, begin, interval, count );
    gBindCol    = init_timer( unit, begin, interval, count );
    gFetch      = init_timer( unit, begin, interval, count );
    gCommit     = init_timer( unit, begin, interval, count );

    gTotRecordCount = 0;
    gTotCommitCount = 0;

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


void printLatencyData()
{

    if( gTotRecordCount == 0 )
        gTotRecordCount++;

    LOGGER( "=== Result data of latency ===\n\n" );
    LOGGER( "Repeat of Execution : %d\n\n", gProperty.mRepeat );
#ifdef __PRINT_CONN__
    LOGGER("[Connection]\n");
    elapse_timer( gConnection , gTotCommitCount );
#endif

    if( gProperty.mExecuteType == PREPARE_EXECUTE )
    {
        LOGGER("[Prepare]\n");
        elapse_timer( gPrepare , gProperty.mRepeat );
    }

    LOGGER("[Execute]\n");
    elapse_timer( gExecute , gProperty.mRepeat );

    if( gProperty.mQueryType == SQL_DIAG_SELECT_CURSOR )
    {
#ifdef __PRINT_BIND__
        LOGGER("[BindColumn]\n");
        elapse_timer( gBindCol , gProperty.mRepeat );
#endif        

#ifdef __PRINT_FETCH__
        LOGGER("[Fetch]\n");
        elapse_timer( gFetch , gTotFetchCount );
#endif
    }
    else
    {
#ifdef __PRINT_COMMIT__
        LOGGER("[Commit]\n");
        elapse_timer( gCommit , gTotCommitCount );
#endif
    }


}
