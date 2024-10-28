#include <cpuid.h>
#include <getopt.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "util.h"
#include "arch.h"
#include "monitoring.h"

#define HUGETLB_std     "/mnt/huge/buff"

extern const char *const uarch_names[MAX_ARCH];
extern uarch_t archi;
extern class_t classes; 
extern const char *const classes_names[MAX_CLASS];
int main(int argc, char **argv){

    if(!is_intel()){
        fprintf(stderr, "This program only works on Intel CPUs\n"); 
        exit(EXIT_FAILURE);
    }

    int cpu_mask = 0;
    cpu_set_t my_set;
    CPU_ZERO(&my_set);
    CPU_SET(cpu_mask, &my_set);
    sched_setaffinity(0, sizeof(cpu_set_t), &my_set);

    nb_cores = cores_per_package();
    int cpu_model  = get_cpu_model();

    if (determine_class_uarch(cpu_model) < 0) {
        exit(EXIT_FAILURE);
    }

    if (setup_perf_counters(classes, archi, nb_cores) < 0) {
        exit(EXIT_FAILURE);
    }

    printf("Micro-architecture: %s\n", uarch_names[archi]);
    printf("Number of cores: %d\n", nb_cores);

    unsigned long long i;
    unsigned long long const stride = 64;
    unsigned long long const nb_loops = 100;

    // For 4kB pages
    int mem_fd;
    if((mem_fd = open(HUGETLB_std, O_CREAT | O_RDWR, 0755)) == -1){
        perror("open");
        exit(EXIT_FAILURE);
    }
    char *mem = (char *)mmap(NULL, nb_loops * stride, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == NULL) {
        printf("Malloc has failed \n");
        return -1;
    } else {
        printf("[+] Allocated memory\n");
    }

    for (i = 0; i < nb_loops * stride; i++) {
        mem[i] = 12;
    }

    //scan
    for (i = 0; i < nb_loops; i++) {
        // printf("monitoring core\n");
        monitor_single_address_core((uintptr_t)mem + (i * stride), 1);
    }

    munmap(mem, (4<<10));
}