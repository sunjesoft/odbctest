
#include <common.h>
#include <log.h>
#include <property.h>

#ifdef __GOLDILOCKS__
#include <goldilocks.h>
#else
#include <sundb.h>
#endif

#ifndef _LATENCY_H_
#define _LATENCY_H_

typedef struct _latencyData{
    struct timeval mStart;
    struct timeval mEnd;
    unsigned long  mDiff;
}LatencyData;

extern LatencyData gConnection;
extern LatencyData gPrepare;
extern LatencyData gExecute;
extern LatencyData gExecuteTot;
extern LatencyData gBindCol;
extern LatencyData gBindColTot;
extern LatencyData gFetch;
extern LatencyData gFetchTot;
extern LatencyData gCommit;
extern LatencyData gCommitTot;

int   gTotRecordCount;
int   gTotCommitCount;
FILE* gOutfileFp;

void initLatencyData();
void getDiffTime( LatencyData* aLatencyData );
void printLatencyData();

#endif
