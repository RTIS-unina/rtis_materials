#include "header.h"
#include "pmc_core.h"
//***************************PMC FUNCTIONS*******************************************************************
void pmc_setup(void* info){
PMC_CORE_DISABLE;
PMC_CORE_RESET;
PMC_CORE_SELECT_EVENT(PERFEVTSEL0, PERFEVTSEL1, PERFEVTSEL2, PERFEVTSEL3);
#ifdef NO_SMT
PMC_CORE_RESET_NOSMT;
PMC_CORE_SELECT_EVENT_NOSMT(PERFEVTSEL4, PERFEVTSEL5, PERFEVTSEL6, PERFEVTSEL7);
#endif
}

void pmc_stop(void* info){
__asm__ __volatile__("wrmsr" : : "c"(0x38F), "a"(0x00), "d"(0x07));  //stop counters
}
