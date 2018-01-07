#include "superscalar.h"

bool readify(FILE* trace, int* pc, int* oprType, int* dst, int* src1, int* src2, int* mem){
   if(!feof(trace)){
      fscanf(trace, "%x %d %d %d %d %x\n", pc, oprType, dst, src1, src2, mem); 
      return true;
   }
   else
      return false;
}

void printFooter(sscPT sscP) {

   printf("CONFIGURATION\n");
   printf("superscalar bandwidth (N) = %d\n",sscP->n);
   printf("dispatch queue size (2*N) = %d\n",2*(sscP->n));
   printf("schedule queue size (S)   = %d\n",sscP->s);
   printf("RESULTS\n");
   printf("number of instructions = %d\n",sscP->instCount);
   printf("number of cycles       = %d\n",sscP->cycleCount-1);
   printf("IPC                    = %.2f\n",((float)sscP->instCount/(float)(sscP->cycleCount-1)));
}

int main (int argc, char** argv) {

   int s       = atoi(argv[1]);
   int n       = atoi(argv[2]);
   int bSize   = atoi(argv[3]); 
   int l1Size  = atoi(argv[4]);
   int l1Assoc = atoi(argv[5]);
   int l2Size  = atoi(argv[6]);
   int l2Assoc = atoi(argv[7]);
   int traceNo = 8;

   FILE* trace;

   trace = fopen(argv[traceNo], "r");

   sscPT sscP  = sscAllocate(s, n, bSize, l1Size, l1Assoc, l2Size, l2Assoc, trace, readify);

   simulate(sscP);

   printFooter(sscP);
}
