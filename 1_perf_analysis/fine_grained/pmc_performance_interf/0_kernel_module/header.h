
#include <linux/module.h>      
#include <linux/init.h>        
#include <linux/kernel.h>      
#include <asm/msr.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/pci.h>
#include <asm/smp.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include <linux/cpu.h>

//**********general parameters************************************************
// Events independent from the arch. SDM Intel	Event Select (SEL)	Umask
// INSTRUCTIONS_RETIRED	0xC0	0x00
// UNHALTED_CORE_CYCLES	0x3C	0x00
// UNHALTED_REFERENCE_CYCLES	0x3C	0x01
// LLC_REFERENCES	0x2E	0x4F
// LLC_MISSES	0x2E	0x41
// BRANCH_INSTRUCTIONS_RETIRED	0xC4	0x00
// BRANCH_MISSES_RETIRED	0xC5	0x00
// BUS_CYCLES	0x0C	0x00
// BRANCH_INSTRUCTIONS (perf)	0xC4	0x00
// BRANCH_MISSES (perf)	0xC5	0x00

#define PMC_CORE
#define FINE_GRAINED_EVENTS


#ifdef FINE_GRAINED_EVENTS
#define OS 1
#define USR 1
#define EDGE 0
#define EVT_SEL_PMC_CORE_0 0x3C //Cycles 
#define U_MASK_PMC_CORE_0  0x01//
#define PERFEVTSEL0 EVT_SEL_PMC_CORE_0|U_MASK_PMC_CORE_0<<8|USR<<16|OS<<17|EDGE<<18|1<<22  //
#define EVT_SEL_PMC_CORE_1 0xC0 // Instr. retired
#define U_MASK_PMC_CORE_1 0x00
#define PERFEVTSEL1  EVT_SEL_PMC_CORE_1|U_MASK_PMC_CORE_1<<8|USR<<16|OS<<17|EDGE<<18|1<<22
#define EVT_SEL_PMC_CORE_2  0x2E//LLC References 
#define U_MASK_PMC_CORE_2 0x4F//
#define PERFEVTSEL2  EVT_SEL_PMC_CORE_2|U_MASK_PMC_CORE_2<<8|USR<<16|OS<<17|EDGE<<18|1<<22
#define EVT_SEL_PMC_CORE_3 0x2E//LLC Misses
#define U_MASK_PMC_CORE_3 0x41//
#define PERFEVTSEL3  EVT_SEL_PMC_CORE_3|U_MASK_PMC_CORE_3<<8|USR<<16|OS<<17|EDGE<<18|1<<22

#if defined(NO_SMT)
#define EN_PMC4 1
#define EVT_SEL_PMC_CORE_4 0x2E //llc misses
#define U_MASK_PMC_CORE_4 0x41
#define PERFEVTSEL4  EVT_SEL_PMC_CORE_4|U_MASK_PMC_CORE_4<<8|USR<<16|OS<<17|EDGE<<18|1<<22
#define EN_PMC5 1
#define EVT_SEL_PMC_CORE_5 0x2E //llc references 
#define U_MASK_PMC_CORE_5 0x4F
#define PERFEVTSEL5 EVT_SEL_PMC_CORE_5|U_MASK_PMC_CORE_5<<8|USR<<16|OS<<17|EDGE<<18|1<<22
#define EN_PMC6 1
#define EVT_SEL_PMC_CORE_6 0x24 //L2 References
#define U_MASK_PMC_CORE_6 0xFF
#define PERFEVTSEL6  EVT_SEL_PMC_CORE_6|U_MASK_PMC_CORE_6<<8|USR<<16|OS<<17|EDGE<<18|EN_PMC6<<22
#define EN_PMC7 1
#define EVT_SEL_PMC_CORE_7 0xC0 //instr retired
#define U_MASK_PMC_CORE_7 0x00
#define PERFEVTSEL7  EVT_SEL_PMC_CORE_7|U_MASK_PMC_CORE_7<<8|USR<<16|OS<<17|EDGE<<18|1<<22
#endif
#endif


#ifdef COARSE_GRAINED_EVENTS
#define OS 1
#define USR 1
#define EDGE 0
#define EVT_SEL_PMC_CORE_0 0xD0//MEM LOAD INSTR RETIRED 
#define U_MASK_PMC_CORE_0  0x81
#define PERFEVTSEL0  EVT_SEL_PMC_CORE_0|U_MASK_PMC_CORE_0<<8|0<<16|1<<17|EDGE<<18|1<<22
#define EVT_SEL_PMC_CORE_1 0xD0	//MEM LOAD INSTR RETIRED
#define U_MASK_PMC_CORE_1 0x81
#define PERFEVTSEL1  EVT_SEL_PMC_CORE_1|U_MASK_PMC_CORE_1<<8|1<<16|0<<17|EDGE<<18|1<<22
#define EVT_SEL_PMC_CORE_2  0xD1  //MEM LOAD INSTR RETIRED with miss in L3
#define U_MASK_PMC_CORE_2 0x20
#define PERFEVTSEL2  EVT_SEL_PMC_CORE_2|U_MASK_PMC_CORE_2<<8|0<<16|1<<17|EDGE<<18|1<<22
#define EVT_SEL_PMC_CORE_3 0xD1	//MEM LOAD INSTR RETIRED with miss in L3
#define U_MASK_PMC_CORE_3 0x20
#define PERFEVTSEL3  EVT_SEL_PMC_CORE_3|U_MASK_PMC_CORE_3<<8|1<<16|0<<17|EDGE<<18|1<<22
#if defined(NO_SMT)
#define EN_PMC4 1
#define EVT_SEL_PMC_CORE_4 0x24 //llc references (usr space)
#define U_MASK_PMC_CORE_4 0xFF
#define PERFEVTSEL4  EVT_SEL_PMC_CORE_4|U_MASK_PMC_CORE_4<<8|1<<16|0<<17|EDGE<<18|EN_PMC4<<22
#define EN_PMC5 1
#define EVT_SEL_PMC_CORE_5 0x2E //llc misses (usrspace)
#define U_MASK_PMC_CORE_5 0x41
#define PERFEVTSEL5 EVT_SEL_PMC_CORE_5|U_MASK_PMC_CORE_5<<8|1<<16|0<<17|EDGE<<18|EN_PMC5<<22
#define EN_PMC6 1
#define EVT_SEL_PMC_CORE_6 0xC0 //instr retired
#define U_MASK_PMC_CORE_6 0x00
#define PERFEVTSEL6  EVT_SEL_PMC_CORE_6|U_MASK_PMC_CORE_6<<8|0<<16|1<<17|EDGE<<18|EN_PMC6<<22
#define EN_PMC7 1
#define EVT_SEL_PMC_CORE_7 0xC0 //instr retired
#define U_MASK_PMC_CORE_7 0x00
#define PERFEVTSEL7  EVT_SEL_PMC_CORE_7|U_MASK_PMC_CORE_7<<8|1<<16|0<<17|EDGE<<18|EN_PMC7<<22
#endif
#endif

void pmc_setup(void*);
void pmc_stop(void*);



