
#include <common.h>
#include <log.h>
#include <property.h>
#include <timer.h>

#ifdef __GOLDILOCKS__
#include <goldilocks.h>
#else
#include <sundb.h>
#endif

#ifndef _LATENCY_H_
#define _LATENCY_H_

extern HTIMER* gConnection;
extern HTIMER* gPrepare;
extern HTIMER* gExecute;
extern HTIMER* gExecuteTot;
extern HTIMER* gBindCol;
extern HTIMER* gBindColTot;
extern HTIMER* gFetch;
extern HTIMER* gFetchTot;
extern HTIMER* gCommit;
extern HTIMER* gCommitTot;

extern int   gTotRecordCount;
extern int   gTotCommitCount;
extern int   gTotFetchCount;
extern FILE* gOutfileFp;

void initTimer();
void printLatencyData();

#endif
