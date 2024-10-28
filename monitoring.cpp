#include <errno.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "arch.h"
#include "global_variables.h"
#include "monitoring.h"
#include "poke.h"
#include "rdmsr.h"
#include "util.h"
#include "wrmsr.h"


int monitor_single_address_core(uintptr_t addr, int print) {

    int i;

    // Disable counters
    uint64_t val[] = {val_disable_ctrs};
    wrmsr_on_cpu_0(msr_unc_perf_global_ctr, 1, val);

    // Reset counters
    val[0] = val_reset_ctrs;
    for (i = 0; i < nb_cores; i++) {
        wrmsr_on_cpu_0(msr_unc_cbo_per_ctr0[i], 1, val);
    }

    // Select event to monitor
    val[0] = val_select_evt_core;
    for (i = 0; i < nb_cores; i++) {
        wrmsr_on_cpu_0(msr_unc_cbo_perfevtsel0[i], 1, val);
    }

    // Enable counting
    val[0] = val_enable_ctrs;
    // val[0] = val_enable_ctrs;
    wrmsr_on_cpu_0(msr_unc_perf_global_ctr, 1, val);

    uintptr_t paddr;
    paddr = poke(addr);



    // Read counter
    int *cboxes = (int *)calloc(max_slices, sizeof(int));
    int *cboxes_tri = (int *)calloc(max_slices, sizeof(int));
    int res_temp;
    for (i = 0; i < nb_cores; i++) {
        res_temp = rdmsr_on_cpu_0(msr_unc_cbo_per_ctr0[i]);
        cboxes[i] = ((res_temp - nb_pokes) < 0) ? 0 : res_temp - nb_pokes;
        cboxes_tri[i] = ((res_temp - nb_pokes) < 0) ? 0 : res_temp - nb_pokes;
    }

    int slice = 0;
    int first = 0;
    int second = 0;
    float percent;


    for (i = 0; i < max_slices; i++) {
        if (cboxes[i] > cboxes[slice]) {
            slice = i;
        }
    }
    // Calculate the ratio between the first and the second result
    // to estimate the error
    quicksort(cboxes_tri, 0, max_slices - 1);
    first = cboxes_tri[max_slices - 1];
    second = cboxes_tri[max_slices - 2];
    percent = ((float)second) / ((float)first) * 100;

    // Pretty print
    if (print) {
        print_bin(paddr);
        printf(" %d %6.2f", slice, percent);
        for (i = 0; i < nb_cores; i++) {
            printf(" % 6d", cboxes[i]);
        }
        printf("\n");
    }

    free(cboxes);
    free(cboxes_tri);

    return slice;
}