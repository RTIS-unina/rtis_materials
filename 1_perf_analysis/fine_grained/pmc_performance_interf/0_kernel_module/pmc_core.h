



#define PMC_CORE_DISABLE __asm__ __volatile__("wrmsr" : : "c"(0x38F), "a"(0x00), "d"(0x00)) // ia32_perf_global_ctrl: disable 4 PMCs & 3 FFCs 
#define PMC_CORE_ENABLE __asm__ __volatile__("wrmsr" : : "c"(0x38F), "a"(0x0f), "d"(0x00)) // ia32_perf_global_ctrl: enable 4 PMCs
#define PMC_CORE_ENABLE_NOSMT __asm__ __volatile__("wrmsr" : : "c"(0x38F), "a"(0xff), "d"(0x00)) // ia32_perf_global_ctrl: enable 8 PMCs 


#define PMC_CORE_RESET do{\
__asm__ __volatile__("wrmsr" : : "c"(0xc1), "a"(0x00), "d"(0x00));  \
__asm__ __volatile__("wrmsr" : : "c"(0xc2), "a"(0x00), "d"(0x00));  \
__asm__ __volatile__("wrmsr" : : "c"(0xc3), "a"(0x00), "d"(0x00));  \
__asm__ __volatile__("wrmsr" : : "c"(0xc4), "a"(0x00), "d"(0x00));  \
}while(0)


#define PMC_CORE_RESET_NOSMT do{\
__asm__ __volatile__("wrmsr" : : "c"(0xc5), "a"(0x00), "d"(0x00));  \
__asm__ __volatile__("wrmsr" : : "c"(0xc6), "a"(0x00), "d"(0x00)); \
__asm__ __volatile__("wrmsr" : : "c"(0xc7), "a"(0x00), "d"(0x00));  \
__asm__ __volatile__("wrmsr" : : "c"(0xc8), "a"(0x00), "d"(0x00)); \
}while(0)

#define PMC_CORE_SELECT_EVENT(sel_evnt1, sel_evnt2, sel_evnt3, sel_evnt4)do{\
__asm__ __volatile__("wrmsr" : : "c"(0x186), "a"(sel_evnt1), "d"(0x00)); \
__asm__ __volatile__("wrmsr" : : "c"(0x187), "a"(sel_evnt2), "d"(0x00));\
__asm__ __volatile__("wrmsr" : : "c"(0x188), "a"(sel_evnt3), "d"(0x00)); \
__asm__ __volatile__("wrmsr" : : "c"(0x189), "a"(sel_evnt4), "d"(0x00));\
}while(0)

#define PMC_CORE_SELECT_EVENT_NOSMT(sel_evnt5, sel_evnt6, sel_evnt7, sel_evnt8) do{\
__asm__ __volatile__("wrmsr" : : "c"(0x18a), "a"(sel_evnt5), "d"(0x00)); \
__asm__ __volatile__("wrmsr" : : "c"(0x18b), "a"(sel_evnt6), "d"(0x00)); \
__asm__ __volatile__("wrmsr" : : "c"(0x18c), "a"(sel_evnt7), "d"(0x00)); \
__asm__ __volatile__("wrmsr" : : "c"(0x18d), "a"(sel_evnt8), "d"(0x00));\
}while(0)


/****************************READ PMC**********************************************************/

#define PMC1_CORE_READ(value_pmc1) do{\
int edx, eax;\
edx = 0;\
eax = 0;\
__asm__ __volatile__("rdmsr" : "=a"(eax), "=d"(edx) : "c"(0xc1));\
value_pmc1 = (eax | (unsigned long long)edx << 0x20);\
}while(0)

#define PMC2_CORE_READ(value_pmc2) do{\
int edx, eax;\
edx = 0;\
eax = 0;\
__asm__ __volatile__("rdmsr" : "=a"(eax), "=d"(edx) : "c"(0xc2));\
value_pmc2 = (eax | (unsigned long long)edx << 0x20);\
}while(0)

#define PMC3_CORE_READ(value_pmc3) do{\
int edx, eax;\
edx = 0;\
eax = 0;\
__asm__ __volatile__("rdmsr" : "=a"(eax), "=d"(edx) : "c"(0xc3));\
value_pmc3 = (eax | (unsigned long long)edx << 0x20);\
}while(0)

#define PMC4_CORE_READ(value_pmc4) do{\
int edx, eax;\
edx = 0;\
eax = 0;\
__asm__ __volatile__("rdmsr" : "=a"(eax), "=d"(edx) : "c"(0xc4));\
value_pmc4 = (eax | (unsigned long long)edx << 0x20);\
}while(0)



#define PMC5_CORE_READ(value_pmc5) do{\
int edx, eax;\
edx = 0;\
eax = 0;\
__asm__ __volatile__("rdmsr" : "=a"(eax), "=d"(edx) : "c"(0xc5));\
value_pmc5 = (eax | (unsigned long long)edx << 0x20);\
}while(0)


#define PMC6_CORE_READ(value_pmc6) do{\
int edx, eax;\
edx = 0;\
eax = 0;\
__asm__ __volatile__("rdmsr" : "=a"(eax), "=d"(edx) : "c"(0xc6));\
value_pmc6 = (eax | (unsigned long long)edx << 0x20);\
}while(0)


#define PMC7_CORE_READ(value_pmc7) do{\
int edx, eax;\
edx = 0;\
eax = 0;\
__asm__ __volatile__("rdmsr" : "=a"(eax), "=d"(edx) : "c"(0xc7));\
value_pmc7 = (eax | (unsigned long long)edx << 0x20);\
}while(0)

 
    
#define PMC8_CORE_READ(value_pmc8) do{\
int edx, eax;\
edx = 0;\
eax = 0;\
__asm__ __volatile__("rdmsr" : "=a"(eax), "=d"(edx) : "c"(0xc8));\
value_pmc8 = (eax | (unsigned long long)edx << 0x20);\
}while(0)
  

#define READ_TSC(value) do {\
unsigned long   cycles_low, cycles_high;\
__asm__ __volatile__("RDTSC;" : "=d"(cycles_high), "=a"(cycles_low):"d"(0), "a"(0): );\
value = ( ((uint64_t)cycles_high << 32) | cycles_low ); \
}while(0)
