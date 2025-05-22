/**
 * @file hello.c
 * @author Giorgio Farina (giorgio.fari96@gmail.com)
 * @brief 

 * Parameters of Core
    * EVT_SEL_PMC_CORE_0
    * U_MASK_PMC_CORE_0
    * EVT_SEL_PMC_CORE_1
    * U_MASK_PMC_CORE_1
    * EVT_SEL_PMC_CORE_2
    * U_MASK_PMC_CORE_2
    * EVT_SEL_PMC_CORE_3
    * U_MASK_PMC_CORE_3
 * Extention to 8 pmc on core if SMT is disabled
    * EVT_SEL_PMC_CORE_4
    * U_MASK_PMC_CORE_4
    * EVT_SEL_PMC_CORE_5
    * U_MASK_PMC_CORE_5
    * EVT_SEL_PMC_CORE_6
    * U_MASK_PMC_CORE_6
    * EVT_SEL_PMC_CORE_7
    * U_MASK_PMC_CORE_7
    * 
 * @version 0.1
 * @date 2021-04-28
 * 
 * @copyright Copyright (c) 2021
 * 
 
 * 
 */
#include "header.h"
#include "pmc_core.h"
#include <linux/types.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Giorgio Farina");
MODULE_DESCRIPTION("Example of precise performance evaluation with PMC.Workload:sum op, Micro-state: Cached, Micro-state:No-Cached");

#define CACHE_LINE_SIZE 64

//#define MICRO_STATE_CACHED 1

int *g_mem_ptr; 
int g_mem_size=CACHE_LINE_SIZE*1000;

static int __init msrdrv_init(void)
{
int i=0;
int sum=0;
unsigned long long  pmc1_start=0;
unsigned long long  pmc2_start=0;
unsigned long long  pmc3_start=0;
unsigned long long  pmc4_start=0;
unsigned long long  pmc1_end=0;
unsigned long long  pmc2_end=0;
unsigned long long  pmc3_end=0;
unsigned long long  pmc4_end=0;
uint64_t cpi_x1000=0;

int cpu = smp_processor_id();
printk("CPU: %d\n", cpu);

//MICRO STATE
g_mem_ptr = (int *)kmalloc(g_mem_size, GFP_KERNEL);
for (i=0;i<(g_mem_size/sizeof(int));i=i+CACHE_LINE_SIZE/sizeof(int)) g_mem_ptr[i]=1;
__asm__ __volatile__("mfence" ::: "memory");
#ifndef MICRO_STATE_CACHED
for (i=0;i<(g_mem_size/sizeof(int));i=i+CACHE_LINE_SIZE/sizeof(int)) __asm__ __volatile__ ("clflush (%0)" :: "r"(&g_mem_ptr[i]));
__asm__ __volatile__("mfence" ::: "memory");
#endif

pmc_setup(NULL);
PMC_CORE_ENABLE;
PMC1_CORE_READ(pmc1_start); 
PMC2_CORE_READ(pmc2_start);
PMC3_CORE_READ(pmc3_start); 
PMC4_CORE_READ(pmc4_start);

//Workload
for (i=0;i<(g_mem_size/sizeof(int));i=i+CACHE_LINE_SIZE/sizeof(int)) sum=sum+g_mem_ptr[i];
__asm__ __volatile__("mfence" ::: "memory");

PMC1_CORE_READ(pmc1_end); 
PMC2_CORE_READ(pmc2_end);
PMC3_CORE_READ(pmc3_end);
PMC4_CORE_READ(pmc4_end);
cpi_x1000 = ((pmc1_end-pmc1_start)*1000/(pmc2_end-pmc2_start));
printk("Expected Mem Accesses %d - Cycles: %llu, Instr: %llu, LLC Ref: %llu,  LLC Miss: %llu \n", sum, pmc1_end-pmc1_start, pmc2_end-pmc2_start, pmc3_end-pmc3_start, pmc4_end-pmc4_start);
printk("CPI: ~ %llu.%03llu\n",
       cpi_x1000 / 1000,
       cpi_x1000 % 1000);
return 0;
} 


static void __exit msrdrv_exit(void)
{  
pmc_stop(NULL);
kfree(g_mem_ptr);
return;
}


module_init(msrdrv_init);
module_exit(msrdrv_exit);




