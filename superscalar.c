#include "superscalar.h"

//fakeRetire
// Remove instructions from the head of fake ROB until an instruction 
// is reached that is not in the WB state
void fakeRetire (sscPT sscP) {
   while(1) {
      //printf("fakeRetire\n");
      instructPT iP = peekHead (sscP->fakeRob);
      if(iP == NULL || iP->state != WB) return;
      popHead (sscP->fakeRob);
      printf("%d fu{%d} src{%d,%d} dst{%d} IF{%d,%d} ID{%d,%d} IS{%d,%d} EX{%d,%d} WB{%d,%d}\n",
            iP->tagD, iP->oprType, iP->src1Reg, iP->src2Reg, iP->destReg, 
            iP->enterIF, (iP->exitIF-iP->enterIF), 
            iP->enterID, (iP->exitID-iP->enterID),
            iP->enterIS, (iP->exitIS-iP->enterIS),
            iP->enterEX, (iP->exitEX-iP->enterEX),
            iP->enterWB, (iP->exitWB-iP->enterWB));
   }
}

//execute
// From the executeList, check for instructions that are finishing
// execution this cycle, and:
// 1) Remove the instruction from the executeList.
// 2) Transition from EX state to WB state.
// 3) Update the register file state (e.g., ready flag) and wakeup
//    dependent instructions (set their operand ready flags)

void wakeUp (void* data, void* userData)
{
   instructPT instructISP   = data;
   instructPT instructEXP   = userData;
   if(instructEXP->destReg != NO_REG) {
      if(instructISP->src1Reg != NO_REG && instructEXP->tagD == instructISP->src1New)
         instructISP->readyS1  = 1;
      if(instructISP->src2Reg != NO_REG && instructEXP->tagD == instructISP->src2New)
         instructISP->readyS2 = 1; 
   }
}

// Remove all EX and transition in program order
void execute (sscPT sscP) {
   int count = 0;
   while(1) {
      //printf("execute\n");
      instructPT instructP  = peekNth(sscP->executeList, count);
      if(instructP == NULL) return;
      if((sscP->cycleCount - instructP->enterEX) >= instructP->exDelay) {
         popNth(sscP->executeList, count);
         if(instructP->destReg != NO_REG && sscP->registerFile[instructP->destReg].tag == instructP->tagD)
            sscP->registerFile[instructP->destReg].ready = 1;
         forEach (sscP->issueList, instructP, wakeUp);
         instructP->state   = WB;
         instructP->enterWB = sscP->cycleCount;
         instructP->exitWB  = instructP->enterWB+1; //hardcoded delay to model 1 cycle in WB
         instructP->exitEX  = sscP->cycleCount;
      }
      else
         count++;
   }
}

//issue
// From the issueList, construct a temp list of instructions whose
// operands are ready – these are the READY instructions. Scan the READY
// instructions in ascending order of tags and issue up to N of them.
// To issue an instruction:
// 1) Remove the instruction from the issueList and add it to the
//    executeList.
// 2) Transition from the IS state to the EX state.
// 3) Free up the scheduling queue entry (e.g., decrement a count
//    of the number of instructions in the scheduling queue)
// 4) Set a timer in the instruction’s data structure that will allow
//    you to model the execution latency
// Clear the old list


/*!proto*/
void issue (sscPT sscP) 
/*!endproto*/
{
   int count = 0;
   int n     = 0;
   while( n < sscP->n ) {
      instructPT instructP  = peekNth(sscP->issueList, count);
      //printf("issue, %d %d %d %p\n", sscP->n, sscP->s, listNodeCount(sscP->issueList), instructP);
      if(instructP == NULL) return;
      if(instructP->readyS1 && instructP->readyS2) {
         popNth(sscP->issueList, count);
         instructP->state   = EX;
         instructP->enterEX = sscP->cycleCount;
         instructP->exitIS  = sscP->cycleCount;
         pushTail(sscP->executeList, instructP);
         n++;
         if(instructP->oprType == 2 && sscP->cachel1 != NULL) {
            if(!read(sscP->cachel1, instructP->memAddr)) 
               instructP->exDelay = (sscP->cachel2 != NULL) ? (read(sscP->cachel2, instructP->memAddr) ? 10 : 20) : 20;
            else
               instructP->exDelay = 5;
         }
      }
      else
         count++;
   }
}

//dispatch
// From the dispatchList, construct a temp list of instructions in the ID
// state (don’t include those in the IF state – you must model the
// 1 cycle fetch latency). Scan the temp list in ascending order of
// tags and, if the scheduling queue is not full, then:
// 1) Remove the instruction from the dispatchList and add it to the
//    issueList. Reserve a schedule queue entry (e.g. increment a
//    count of the number of instructions in the scheduling
//    queue) and free a dispatch queue entry (e.g. decrement a count of
//    the number of instructions in the dispatch queue).
// 2) Transition from the ID state to the IS state.
// 3) Rename source operands by looking up state in the register
//    file; rename destination operands by updating state in
//    the register file.
// For instructions in the dispatchList that are in the IF
// state, unconditionally transition to the ID state (models the 1 cycle
// latency for instruction fetch)

void trans (void* data, void* userData)
{
   instructPT instructP  = data;
   sscPT sscP            = userData;
   if( instructP->state == IF ){
      instructP->state   = ID;
      instructP->exitIF  = sscP->cycleCount;
      instructP->enterID = sscP->cycleCount;
   }
}

/*!proto*/
void dispatch (sscPT sscP) 
/*!endproto*/
{
   while(listNodeCount(sscP->issueList) < (sscP->s)) {
      instructPT instructP = peekHead(sscP->dispatchList);
      if(instructP == NULL || instructP->state != ID) break;
      popHead(sscP->dispatchList);
      instructP->state   = IS;
      instructP->enterIS = sscP->cycleCount;
      instructP->exitID  = sscP->cycleCount;
      //check for Dst and Src operands
      if(instructP->src1Reg != NO_REG) {
         instructP->readyS1 = sscP->registerFile[instructP->src1Reg].ready;
         instructP->src1New = sscP->registerFile[instructP->src1Reg].tag;
      }
      if(instructP->src2Reg != NO_REG) {
         instructP->readyS2 = sscP->registerFile[instructP->src2Reg].ready;
         instructP->src2New = sscP->registerFile[instructP->src2Reg].tag;
      }
      if(instructP->destReg != NO_REG) {
         sscP->registerFile[instructP->destReg].ready   = 0;
         sscP->registerFile[instructP->destReg].tag     = instructP->tagD;
      }
      pushTail(sscP->issueList, instructP);
      //printf("dispatch %p\n", instructP);
   }
   forEach (sscP->dispatchList, sscP, trans);
}


// Read new instructions from the trace as long as 
// 1) you have not reached the end-of-file, 
// 2) the fetch bandwidth is not exceeded,
// 3) the dispatch queue is not full.
//
// Then, for each incoming instruction:
// 1) Push the new instruction onto the fake-ROB. Initialize the
//    instruction’s data structure, including setting its state to IF.
// 2) Add the instruction to the dispatchList and reserve a
//    dispatch queue entry (e.g., increment a count of the number
//    of instructions in the dispatch queue)
bool fetch (sscPT sscP)  {
   int inst                = sscP->n;
   while(inst-- && listNodeCount(sscP->dispatchList) < 2*(sscP->n)) {
      int pc, oprType, dst, src1, src2, mem;
      bool cond            = (*sscP->readTrace)(sscP->trace, &pc, &oprType, &dst, &src1, &src2, &mem);
      if (!cond) return false; 
      instructPT instructP = (instructPT)calloc(1, sizeof(instructT));
      instructP->state     = IF;
      instructP->oprType   = oprType;
      instructP->destReg   = dst;
      instructP->src1Reg   = src1;
      instructP->src2Reg   = src2;
      instructP->pc        = pc;
      instructP->tagD      = sscP->instCount++;
      instructP->readyS1   = 1;
      instructP->readyS2   = 1;
      instructP->enterIF   = sscP->cycleCount;
      instructP->memAddr   = mem;

      switch(oprType) {
         case 0:  instructP->exDelay = 1;
                  break;
         case 1:  instructP->exDelay = 2;
                  break;
         default: instructP->exDelay = 5;
                  break;
      }

      pushTail(sscP->fakeRob, instructP);
      pushTail(sscP->dispatchList, instructP);
   }
   return true;
}

/*!proto*/
sscPT sscAllocate (int s, int n, int bSize, int l1Size, int l1Assoc, int l2Size, int l2Assoc, FILE* trace, 
                   bool (*readTrace)(FILE* , int* , int* , int* , int* , int* , int* )) 
/*!endproto*/
{

   sscPT sscP                       = (sscPT)calloc(1, sizeof(sscT));
   sscP->cachel1                    = cacheAllocate(l1Size, bSize, l1Assoc, WBWA, LRU);
   sscP->cachel2                    = cacheAllocate(l2Size, bSize, l2Assoc, WBWA, LRU);
   sscP->s                          = s;
   sscP->n                          = n;
   sscP->trace                      = trace;
   sscP->readTrace                  = readTrace;
   sscP->dispatchList               = createList();
   sscP->issueList                  = createList();
   sscP->executeList                = createList();
   sscP->fakeRob                    = createList();

   for(int i = 0; i < N_REGS; i++) {
      sscP->registerFile[i].ready   = 1;
   }
   return sscP;
}

/*!proto*/
void simulate (sscPT sscP) 
/*!endproto*/
{
   bool endOfTrace = false;
   do {
      fakeRetire(sscP);
      execute(sscP);
      issue(sscP);
      dispatch(sscP);
      endOfTrace = !fetch(sscP);
      sscP->cycleCount++;
   }while(listNodeCount(sscP->fakeRob) != 0 || !endOfTrace);
}
