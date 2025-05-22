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




struct state_cpu{
   unsigned long long  period;
   unsigned long long  pmc1;
   unsigned long long  pmc2;
   unsigned long long  pmc3;
   unsigned long long  pmc4;
   unsigned long long  sum;
};

int g_mem_size=CACHE_LINE_SIZE*10000;
cpumask_var_t cpu_mask;
int n_core_interf = 15;
struct state_cpu* global_point_state_cpu;
static atomic_t ready_count;
static volatile int start_flag;
//Kernel Module Inputs, you can change it
module_param(n_core_interf,int,0660);

void benchmark(void* info){
   struct state_cpu* state_core;
   unsigned long flags;
   int *g_mem_ptr; 
   int i,sum=0;
   uint64_t start, end, middle;
   int64_t pmc1_end, pmc1_start;
   int64_t pmc2_end, pmc2_start;
   int64_t pmc3_end, pmc3_start;
   int64_t pmc4_end, pmc4_start;
   int cpu = smp_processor_id();
   state_core=per_cpu_ptr(global_point_state_cpu,smp_processor_id());
   /*****************MICRO STATE SETUP********************/
   g_mem_ptr = (int *)kmalloc(g_mem_size, GFP_KERNEL);
   for (i=0;i<(g_mem_size/sizeof(int));i=i+CACHE_LINE_SIZE/sizeof(int)) g_mem_ptr[i]=1;
   #ifndef MICRO_STATE_CACHED
   for (i=0;i<(g_mem_size/sizeof(int));i=i+CACHE_LINE_SIZE/sizeof(int)) __asm__ __volatile__ ("clflush (%0)" :: "r"(&g_mem_ptr[i]));
   #endif
   preempt_disable();
   raw_local_irq_save(flags);
   if(cpu!=0){
      atomic_inc(&ready_count);
      while (!READ_ONCE(start_flag))cpu_relax();
   }else{
      while (atomic_read(&ready_count) < n_core_interf)cpu_relax();
      WRITE_ONCE(start_flag, 1);
   }
   //printk("%d Ready_count %d\n",cpu,atomic_read(&ready_count));
   
   __asm__ __volatile__("mfence" ::: "memory"); //serialization instruction
   /*****************READ PMC AND TSC EVENTS********************/
   PMC_CORE_ENABLE;
   PMC1_CORE_READ(pmc1_start);
   PMC2_CORE_READ(pmc2_start);
   PMC3_CORE_READ(pmc3_start);
   PMC4_CORE_READ(pmc4_start);
   READ_TSC(start);
   __asm__ __volatile__("mfence" ::: "memory");
   READ_TSC(middle);
   /*****************START BENCHMARK********************/
   for (i=0;i<(g_mem_size/sizeof(int));i=i+CACHE_LINE_SIZE/sizeof(int)) sum=sum+g_mem_ptr[i];
   /*****************END BENCHMARK********************/
   __asm__ __volatile__("mfence" ::: "memory");
   READ_TSC(end);
   PMC1_CORE_READ(pmc1_end);
   PMC2_CORE_READ(pmc2_end);
   PMC3_CORE_READ(pmc3_end);
   PMC4_CORE_READ(pmc4_end);
   __asm__ __volatile__ ("mfence" ::: "memory");
   /*****************SAVE DIFFERENCES IN GLOBAL VARIABLES********************/
   
   state_core->period = (end-middle)-(middle-start);
   state_core->pmc1= pmc1_end-pmc1_start;
   state_core->pmc2= pmc2_end-pmc2_start;
   state_core->pmc3= pmc3_end-pmc3_start;
   state_core->pmc4= pmc4_end-pmc4_start;
   state_core->sum= sum;
   __asm__ __volatile__("lfence" ::: "memory");
   raw_local_irq_restore(flags);
   preempt_enable(); 
    /*****************RE SETUP OF ENVIRONMENT********************/
    kfree(g_mem_ptr);
}


static int __init msrdrv_init(void)
{
   
   
   for (int c_int=0;c_int<15+1;c_int++){
      int avg_lat_llcmiss_x1000=0;
      n_core_interf=c_int;
      for (int k=0;k<10;k++){
         int id=0; 
         int i=0;
         int idcpu;
         uint64_t cpi_x1000=0;
         uint64_t lat_llcmiss=0;
         int debug=0;
         zalloc_cpumask_var(&cpu_mask,GFP_NOWAIT);
         cpumask_set_cpu(0,cpu_mask);
         atomic_set(&ready_count, 0);
         start_flag=0;
         
         for(i=1;i<n_core_interf+1;i++){
            cpumask_set_cpu(i,cpu_mask); 
         }
         if(debug) printk("[perf-analysis] Number of interfering cores: %d\n",n_core_interf );
         global_point_state_cpu= alloc_percpu(struct state_cpu); /*allocation of multi core status*/

         if(debug) printk("[perf-analysis] Setting the per-cpu state.\n");
         cpus_read_lock();
         for_each_cpu(id, cpu_mask) {
            struct state_cpu* state_core=per_cpu_ptr(global_point_state_cpu,id);
            memset(state_core, 0, sizeof(struct state_cpu));
         }
         cpus_read_unlock();
         if(debug) printk("[perf-analysis] Setting the per-cpu performance counters.\n");
         on_each_cpu_mask(cpu_mask, pmc_setup, NULL, 1);
         if(debug) printk("[perf-analysis] Run the benchmark on each cpu.\n");
         
         on_each_cpu_mask(cpu_mask, benchmark, NULL, 1);
         for_each_cpu(idcpu, cpu_mask){
            struct state_cpu* state_core=per_cpu_ptr(global_point_state_cpu,idcpu);
            cpi_x1000 = ((state_core->period)*1000/(state_core->pmc2));
            lat_llcmiss = ((state_core->period)/(state_core->pmc4));
            if(debug) printk(" CPU %d -Expected accesses %llu, TSC: %llu, Cycles: %llu, Instr: %llu, LLC Ref: %llu,  LLC Miss: %llu \n",idcpu, state_core->sum,state_core->period, state_core->pmc1, state_core->pmc2, state_core->pmc3, state_core->pmc4);
            if(idcpu==0){
               if(debug) printk("CPI: ~ %llu.%03llu\n", 
               cpi_x1000 / 1000,
               cpi_x1000 % 1000);
            }
            
         }
         avg_lat_llcmiss_x1000=avg_lat_llcmiss_x1000+(lat_llcmiss);

      }
      printk("# Interf cores: %d, Avg LLC miss lat %d\n",n_core_interf, avg_lat_llcmiss_x1000/10);
   }
   
   
   return 0;
}

static void __exit msrdrv_exit(void)
{

   printk("[perf-analysis] Resetting the per-cpu state and per-core pmc.\n");
   on_each_cpu_mask(cpu_mask, pmc_stop, NULL, 1);
   free_cpumask_var(cpu_mask);
   free_percpu(global_point_state_cpu);
   return;
}


module_init(msrdrv_init);
module_exit(msrdrv_exit);




