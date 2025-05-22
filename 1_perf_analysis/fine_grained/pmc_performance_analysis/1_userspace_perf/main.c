#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <stdint.h>
#define CACHE_LINE_SIZE 64
//#define MICRO_STATE_CACHED 1
int *g_mem_ptr; 
int g_mem_size=CACHE_LINE_SIZE*1000;
static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                            int cpu, int group_fd, unsigned long flags) {
    return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

int main() {
    int sum=0;
    struct perf_event_attr pe_cycles, pe_instructions, pe_cache_refs, pe_cache_misses;
    long long count_cycles, count_instructions, count_cache_refs, count_cache_misses;
    int fd_cycles, fd_instructions, fd_cache_refs, fd_cache_misses;

    memset(&pe_cycles, 0, sizeof(struct perf_event_attr));
    pe_cycles.type = PERF_TYPE_HARDWARE;
    pe_cycles.size = sizeof(struct perf_event_attr);
    pe_cycles.config = PERF_COUNT_HW_CPU_CYCLES;
    pe_cycles.disabled = 1;
    pe_cycles.exclude_kernel = 1;
    pe_cycles.exclude_hv = 1;

    memset(&pe_instructions, 0, sizeof(struct perf_event_attr));
    pe_instructions = pe_cycles;
    pe_instructions.config = PERF_COUNT_HW_INSTRUCTIONS;

    memset(&pe_cache_refs, 0, sizeof(struct perf_event_attr));
    pe_cache_refs = pe_cycles;
    pe_cache_refs.config = PERF_COUNT_HW_CACHE_REFERENCES;

    memset(&pe_cache_misses, 0, sizeof(struct perf_event_attr));
    pe_cache_misses = pe_cycles;
    pe_cache_misses.config = PERF_COUNT_HW_CACHE_MISSES;

    fd_cycles = perf_event_open(&pe_cycles, 0, -1, -1, 0);
    fd_instructions = perf_event_open(&pe_instructions, 0, -1, -1, 0);
    fd_cache_refs = perf_event_open(&pe_cache_refs, 0, -1, -1, 0);
    fd_cache_misses = perf_event_open(&pe_cache_misses, 0, -1, -1, 0);

    if (fd_cycles == -1 || fd_instructions == -1 || fd_cache_refs == -1 || fd_cache_misses == -1) {
        perror("perf_event_open");
        exit(EXIT_FAILURE);
    }

    ioctl(fd_cycles, PERF_EVENT_IOC_RESET, 0);
    ioctl(fd_instructions, PERF_EVENT_IOC_RESET, 0);
    ioctl(fd_cache_refs, PERF_EVENT_IOC_RESET, 0);
    ioctl(fd_cache_misses, PERF_EVENT_IOC_RESET, 0);
    //MICRO STATE
    g_mem_ptr = (int *)malloc(g_mem_size);
    for (int i=0;i<(g_mem_size/sizeof(int));i=i+CACHE_LINE_SIZE/sizeof(int)) g_mem_ptr[i]=1;
    __asm__ __volatile__("mfence" ::: "memory");
    #ifndef MICRO_STATE_CACHED
    for (int i=0;i<(g_mem_size/sizeof(int));i=i+CACHE_LINE_SIZE/sizeof(int)) __asm__ __volatile__ ("clflush (%0)" :: "r"(&g_mem_ptr[i]));
    __asm__ __volatile__("mfence" ::: "memory");
    #endif
    ioctl(fd_cycles, PERF_EVENT_IOC_ENABLE, 0);
    ioctl(fd_instructions, PERF_EVENT_IOC_ENABLE, 0);
    ioctl(fd_cache_refs, PERF_EVENT_IOC_ENABLE, 0);
    ioctl(fd_cache_misses, PERF_EVENT_IOC_ENABLE, 0);
    
    // Codice da misurare
    //Workload
    for (int i=0;i<(g_mem_size/sizeof(int));i=i+CACHE_LINE_SIZE/sizeof(int)) sum=sum+g_mem_ptr[i];
    __asm__ __volatile__("mfence" ::: "memory");

    ioctl(fd_cycles, PERF_EVENT_IOC_DISABLE, 0);
    ioctl(fd_instructions, PERF_EVENT_IOC_DISABLE, 0);
    ioctl(fd_cache_refs, PERF_EVENT_IOC_DISABLE, 0);
    ioctl(fd_cache_misses, PERF_EVENT_IOC_DISABLE, 0);

    read(fd_cycles, &count_cycles, sizeof(long long));
    read(fd_instructions, &count_instructions, sizeof(long long));
    read(fd_cache_refs, &count_cache_refs, sizeof(long long));
    read(fd_cache_misses, &count_cache_misses, sizeof(long long));

    printf("Cycles: %lld\n", count_cycles);
    printf("Instructions: %lld\n", count_instructions);
    printf("Cache References: %lld\n", count_cache_refs);
    printf("Cache Misses: %lld\n", count_cache_misses);

    if (count_instructions > 0)
        printf("CPI: %.3f\n", (double)count_cycles / count_instructions);
    else
        printf("CPI: N/A\n");

    if (count_cache_refs > 0)
        printf("Cache Miss Rate: %.3f%%\n", 100.0 * count_cache_misses / count_cache_refs);
    else
        printf("Cache Miss Rate: N/A\n");

    close(fd_cycles);
    close(fd_instructions);
    close(fd_cache_refs);
    close(fd_cache_misses);

    return 0;
}
