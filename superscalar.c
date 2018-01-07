#include "superscalar.h"

//fakeRetire
// Remove instructions from the head of fake ROB until an instruction 
// is reached that is not in the WB state

//execute
// From the executeList, check for instructions that are finishing
// execution this cycle, and:
// 1) Remove the instruction from the executeList.
// 2) Transition from EX state to WB state.
// 3) Update the register file state (e.g., ready flag) and wakeup
//    dependent instructions (set their operand ready flags)

// Remove all EX and transition in program order
void execute (sscPT sscP) {


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
   while( listNodeCount(sscP->executeList) < sscP->n ) {
      instructPT instructP  = peekNth(sscP->issueList, count);
      if(instructP == NULL) return;
      if(instructP->readyS1 && instructP->readyS2) {
         popNth(sscP->issueList, count);
         instructP->state   = EX;
         instructP->enterEX = sscP->cycleCount;
         instructP->exitIS  = sscP->cycleCount;
         pushTail(sscP->executeList, instructP);
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
      if(instructP->state != ID) break;
      popHead(sscP->dispatchList);
      instructP->state = IS;
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
      if(instructP->dstReg != NO_REG) {
         sscP->registerFile[instructP->dstReg].ready   = 0;
         sscP->registerFile[instructP->dstReg].tag     = instructP->tagD;
      }
      pushTail(sscP->issueList, instructP);
   }
   forEach (sscP->dispatchList, NULL, trans);
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
void fetch (sscPT sscP)  {
   int inst                = sscP->n;
   while(--inst && listNodeCount(sscP->dispatchList) >= 2*(sscP->n)) {
      int pc, oprType, dst, src1, src2, mem;
      bool cond            = readTrace(&pc, &oprType, &dst, &src1, &src2, &mem);
      if (!cond) return; 
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

      pushTail(sscP->fakeRob, instructP);
      pushTail(sscP->dispatchList, instructP);
   }
}

sscPT sscAllocate (int s, int n) {

   sscPT sscP                       = (sscPT)calloc(1, sizeof(sscT));
   sscP->s                          = s;
   sscP->n                          = n;
   sscP->dispatchList               = createList();
   sscP->issueList                  = createList();
   sscP->executeList                = createList();
   sscP->fakeRob                    = createList();

   for(int i = 0; i < N_REGS; i++) {
      sscP->registerFile[i].ready   = 1;
   }
   return sscP;
}
