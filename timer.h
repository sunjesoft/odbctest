


#include <vector>
#include <algorithm>
#include <common.h>
#include <log.h>
#include <pthread.h>


#ifndef _TIMER_H_
#define _TIMER_H_

#define MIN_SECTION_COUNT   1
#define MAX_SECTION_COUNT   20
#define UNIT_DEFAULT        0

enum unit_type
{
    UNIT_MICRO    = UNIT_DEFAULT,
    UNIT_SEC,
    UNIT_MILI,
    UNIT_NANO
};

typedef struct timer_stat
{
    int             mUnitType;     /* 단위 (SEC, MILI, MICRO, NANO) */
    double          mSum;          /* 전체 시간 합계 - 단위에 따른 시간 */

    int             mBegin;        /* 통계를 보여줄 시간구간의 시작 값 */
    int             mInterval;     /* 통계를 보여줄 시간구간의 간격 */
    int             mSectionCount; /* 몇 개의 시간구간으로 보여줄 것인가 */

    struct timespec mStartTimer;
    struct timespec mEndTimer;

    double          mMax;

    char          * mTime;
} HTIMER;

HTIMER * init_timer( int, int, int, int );
int start_timer( HTIMER * );
int start_timer2( HTIMER * aTimer, struct timespec * aStart );
int end_timer( HTIMER * );
int elapse_timer( HTIMER *, int );
int final_timer( HTIMER * timer );

int get_unit_type_str( int, char * );

extern pthread_mutex_t   __gTimerStatMutex;

#endif
