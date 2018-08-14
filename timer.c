


// not define SHOW_LAT_SECTION : tps 결과와 함께 latency 를 99%, 99.9% , avg 정도로 보여준다.
// define SHOW_LAT_SECTION     : latency 를 구간별 통계로 보여준다.

//#define SHOW_LAT_SECTION

#include "timer.h"

pthread_mutex_t   __gTimerStatMutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * aBegin 값을 시작으로 aInterval 만큼의 간격으로
 * aSecCount 만큼의 구간 갯수에 대한 통계를 보여달라.
 * 단위는 aUnit 이다.
 * 만약 ( UNIT_MICRO, 1, 2, 10 ) 라는 argument 라면
 * 1 micro 초부터 2 micro 간격으로 10 개의 구간으로 통계를
 * 쌓으라는 말이다.
 */
HTIMER * init_timer( int  aUnit,
        int  aBegin,
        int  aInterval,
        int  aSecCount )
{
    HTIMER * sTimer;


    if( aSecCount < MIN_SECTION_COUNT || aSecCount > MAX_SECTION_COUNT )
    {
        LOGGER("[%s] Invalid section count[%d]. It must be > %d and < %d.\n",
                __func__, aSecCount, MIN_SECTION_COUNT, MAX_SECTION_COUNT );
        return NULL;
    }

    sTimer = (HTIMER *)malloc(sizeof(HTIMER));
    if( sTimer == NULL )
    {
        LOGGER("lack of memory : %s\n", strerror(errno));
        return NULL;
    }
    memset( sTimer, 0x00, sizeof(HTIMER) );

    sTimer->mTime = (char *)malloc(aSecCount * sizeof(int));
    if( sTimer->mTime == NULL )
    {
        LOGGER("lack of memory : %s\n", strerror(errno));
        return NULL;
    }
    memset( sTimer->mTime, 0x00, aSecCount * sizeof(int) );

    sTimer->mSum      = 0;
    sTimer->mUnitType = aUnit;
    sTimer->mBegin    = aBegin;
    sTimer->mInterval = aInterval;
    sTimer->mSectionCount  = aSecCount;
    sTimer->mMax = 0;

    return sTimer;
}


/*
 * 시간을 재기 시작하는 시점에 call 한다.
 */
int start_timer( HTIMER *  aTimer )
{
    int   sRet;

    if( aTimer == NULL )
    {
        LOGGER("timer is null.\n");
        return -1;
    }
    if( aTimer->mTime == NULL )
    {
        LOGGER("timer->mTime is null.\n");
        return -1;
    }

    sRet = clock_gettime( CLOCK_REALTIME, &aTimer->mStartTimer );
    if( sRet != 0 )
    {
        LOGGER("clock_gettime error : %s\n", strerror(errno));
        return -1;
    }

    return 0;
}


/*
 * start 지점을 알고 있으며 이를 그대로 mStartTimer 에 복사.
 * 주로 단방향 성능 시험에서 패킷에 담긴 시간을 사용할 때 이용된다.
 */
int start_timer2( HTIMER *            aTimer,
        struct timespec  * aStart )
{
    memcpy(&aTimer->mStartTimer, aStart, sizeof(struct timespec) );

    return 0;
}


/*
 * 시간 측정의 끝부분에서 call 한다.
 * start_timer 에서 측정했던 시간과의 차이를 통해 간격을 구한다.
 */
int end_timer( HTIMER *  aTimer )
{
    int       i;
    int       sMultiplyVal;
    int     * sTimeCount;
    double    sTemp;

    if( aTimer == NULL )
    {
        LOGGER("timer is null.\n");
        return -1;
    }
    if( aTimer->mTime == NULL )
    {
        LOGGER("timer->mTime is null.\n");
        return -1;
    }

    clock_gettime( CLOCK_REALTIME, &aTimer->mEndTimer );

    switch( aTimer->mUnitType )
    {
        case UNIT_SEC   :  sMultiplyVal = 1;          break;
        case UNIT_MILI  :  sMultiplyVal = 1000;       break;
        case UNIT_MICRO :  sMultiplyVal = 1000000;    break;
        case UNIT_NANO  :  sMultiplyVal = 1000000000; break;
        default :
                           LOGGER("invalid unit type[%d]\n", aTimer->mUnitType );
                           return -1;
    }

    /* 각 시간 단위로 값을 구한다. */
    sTemp = ((aTimer->mEndTimer.tv_sec + aTimer->mEndTimer.tv_nsec/1000000000.0)
            - (aTimer->mStartTimer.tv_sec + aTimer->mStartTimer.tv_nsec/1000000000.0))
        * sMultiplyVal;
    if( sTemp < 0 )
    {
        LOGGER("invalid timer value[%.9f]\n", sTemp );
        return -1;
    }

    /* sTemp 값을 검사하여 맞는 section 구간의 count 값을 증가시킨다. */
    for( i = 0; i < aTimer->mSectionCount; i++ )
    {
        sTimeCount = (int *)aTimer->mTime + i;

        if( i == 0 )
        {

            if( sTemp < aTimer->mBegin )
            {
                *sTimeCount = *sTimeCount + 1;
                break;
            }
            else
            {
                continue;
            }
        }
        else if( i > 0 && i < (aTimer->mSectionCount - 1) )
        {
            if( sTemp >= (aTimer->mBegin + ((i - 1) * aTimer->mInterval))
                    && sTemp < (aTimer->mBegin + (i * aTimer->mInterval)) )
            {
                *sTimeCount = *sTimeCount + 1;
                break;
            }
            else
            {
                continue;
            }
        }
        else
        {
            *sTimeCount = *sTimeCount + 1;
        }
    }

    aTimer->mSum = aTimer->mSum + sTemp;
    if( sTemp > aTimer->mMax )
    {
        aTimer->mMax = sTemp;
    }

    return 0;
}


/*
 * 누적된 측정 정보들을 화면에 보여준다.
 */
int elapse_timer( HTIMER *      aTimer,
        int          aIterCnt )
{
    int      i;
    int      sRet;
    char     sUnitType[128];
    int    * sTimeCount;

    if( aTimer == NULL )
    {
        LOGGER("timer is null.\n");
        return -1;
    }
    if( aTimer->mTime == NULL )
    {
        LOGGER("timer->mTime is null.\n");
        return -1;
    }

    sRet = get_unit_type_str( aTimer->mUnitType, sUnitType );
    if( sRet != 0 )
    {
        LOGGER("invalid unit type[%d]\n", aTimer->mUnitType );
        return -1;
    }

    pthread_mutex_lock( &__gTimerStatMutex );

    LOGGER("=========================================================\n");
    LOGGER("[Unit : %s]\n", sUnitType );
    LOGGER("Total = [%.9f] ,  Avg = [%.9f]\n", aTimer->mSum, (double)(aTimer->mSum/aIterCnt));
    LOGGER("Max = [%.9f]\n", aTimer->mMax);
    LOGGER("=========================================================\n");
    LOGGER(" Cnt Time Interval    Percent     Count\n");
    LOGGER("=========================================================\n");

    aTimer->mSum = 0;
    for( i = 0; i < aTimer->mSectionCount; i++ )
    {
        sTimeCount = (int *)aTimer->mTime + i;
        aTimer->mSum = aTimer->mSum + *sTimeCount;
    }

    for( i = 0; i < aTimer->mSectionCount; i++ )
    {
        sTimeCount = (int *)aTimer->mTime + i;

        if( i == 0 )
        {
            LOGGER("[%02d] 0 <  time < %d :  %.3f   :  %d\n",
                    i,
                    aTimer->mBegin,
                    (double)(*sTimeCount/aTimer->mSum)*100.0,
                    *sTimeCount);
        }
        else if( i == (aTimer->mSectionCount - 1) )
        {
            LOGGER("[%02d] %d <= time < @ :  %.3f   :  %d\n",
                    i,
                    aTimer->mBegin + (i - 1) * aTimer->mInterval,
                    (double)(*sTimeCount/aTimer->mSum)*100.0,
                    *sTimeCount);
        }
        else
        {
            LOGGER("[%02d] %d <= time < %d :  %.3f   :  %d\n",
                    i,
                    aTimer->mBegin + ((i -1) * aTimer->mInterval),
                    aTimer->mBegin + (i * aTimer->mInterval),
                    (double)(*sTimeCount/aTimer->mSum)*100.0, *sTimeCount);
        }
    }
    LOGGER("=========================================================\n");

    pthread_mutex_unlock( &__gTimerStatMutex );

    free( aTimer );

    return 0;
}


int final_timer( HTIMER * timer )
{
    if( timer != NULL )
    {
        free( timer );
    }

    return 0;
}


int get_unit_type_str( int    aUnitType,
        char * aUnitTypeStr )
{
    switch( aUnitType )
    {
        case UNIT_SEC   : strcpy(aUnitTypeStr, "Second");       break;
        case UNIT_MILI  : strcpy(aUnitTypeStr, "Mili Second");  break;
        case UNIT_MICRO : strcpy(aUnitTypeStr, "Micro Second"); break;
        case UNIT_NANO  : strcpy(aUnitTypeStr, "Nano Second");  break;
        default : return -1;
    }

    return 0;
}

int getTimeDiff ( const struct timeval& aStart,
        const struct timeval& aEnd ) {

    return (aEnd.tv_sec - aStart.tv_sec) * 1000000  +
        (aEnd.tv_usec - aStart.tv_usec ) ;
}



