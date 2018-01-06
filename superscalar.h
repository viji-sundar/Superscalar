#include<stdio.h>
#include<stdlib.h>
#include<malloc.h>
#include "ll.h"

#define IF 0
#define ID 1
#define IS 2
#define EX 3
#define WB 4
#define N_REGS 128

typedef struct _instructT*    instructPT;
typedef struct _sscT*         sscPT;

typedef struct _instructT{
   int          state;
   int          oprType;
   int          destReg;
   int          src1Reg;
   int          src2Reg;
   int          tag;
   int          pc;
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

   int          cycleCount;
   int          instCount;

}sscT;
