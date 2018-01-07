#include<stdio.h>
#include<stdlib.h>
#include<malloc.h>
#include "ll.h"
#include "cache.h"

#define IF 0
#define ID 1
#define IS 2
#define EX 3
#define WB 4
#define N_REGS 128
#define NO_REG -1

typedef struct _instructT*    instructPT;
typedef struct _sscT*         sscPT;

typedef struct _instructT{
   int          state;
   int          oprType;
   int          destReg;
   int          src1Reg;
   int          readyS1;
   int          src1New;
   int          src2Reg;
   int          readyS2;
   int          src2New;
   int          tagD;
   int          pc;
   int          memAddr;

   int          exDelay;

   int          enterIF;
   int          exitIF;
   int          enterID;
   int          exitID;
   int          enterIS;
   int          exitIS;
   int          enterEX;
   int          exitEX;
   int          enterWB;
   int          exitWB;
}instructT;

typedef struct _rfT{
   int          ready;
   int          tag;
}rfT;

typedef struct _sscT{
   listPT       dispatchList;
   listPT       issueList;
   listPT       executeList;
   listPT       fakeRob;
   int          n;
   int          s;
   
   rfT          registerFile[N_REGS];
   cachePT      cachel1;
   cachePT      cachel2;

   int          cycleCount;
   int          instCount;
   FILE*        trace;
   bool         (*readTrace)(FILE*, int*, int*, int*, int*, int*, int*); // readTracePT readTraceP;

}sscT;

#include "build/superscalar_proto.h"
