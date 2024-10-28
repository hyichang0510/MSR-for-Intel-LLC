#include <stdio.h>
#include <string.h>

#include "arch.h"

uarch_t archi = UARCH_UNKNOWN;
class_t classes = CPU_UNKNOWN; // xeon or core

int nb_cores;
int max_slices;

const char *const classes_names[MAX_CLASS] = {"Unknown Class", "core"};
const char *const uarch_names[MAX_ARCH] = {"Unkown uarch",  "Skylake",  "Kaby Lake"};

// Core MSRs and values
unsigned long long msr_unc_perf_global_ctr = -1;
unsigned long long msr_unc_cbo_perfevtsel0[8] = {0};
unsigned long long msr_unc_cbo_per_ctr0[8] = {0};
unsigned long long val_enable_ctrs = -1;
unsigned long long val_disable_ctrs = -1;
unsigned long long val_select_evt_core = -1;
unsigned long long val_reset_ctrs = -1;


int determine_class_uarch(int cpu_model) {

    // CPU class: Xeon or Core
    switch (cpu_model) {
    case 42:
    case 58:
    case 60:
    case 69:
    case 70:
    case 61:
    case 71:
    case 78:
    case 94:
    case 142:
    case 158:
        classes = INTEL_CORE;
        break;
    default:
        classes = CPU_UNKNOWN;
        printf("CPU is undefined\n");
        return -1;
    }

    // CPU micro-architecture
    switch (cpu_model) {

    case 78:
    case 94:
        archi = SKYLAKE; // Skylake (core)
        break;

    case 142:
    case 158:
        archi = KABY_LAKE; // Kaby Lake or Coffee Lake
        break;
    default:
        archi = UARCH_UNKNOWN;
        printf("Micro-architecture is undefined\n");
        return -1;
    }

    return 0;
}


int setup_perf_counters(class_t classes, uarch_t archi, int nb_cores) {
    if(classes == INTEL_CORE && (archi == KABY_LAKE || archi == SKYLAKE)){
        msr_unc_perf_global_ctr = 0xe01;
        val_enable_ctrs = 0x20000000;
        max_slices = 7;

        static unsigned long long _msr_unc_cbo_perfevtsel0[] = {0x700, 0x710, 0x720, 0x730, 0x740, 0x750, 0x760};

        memcpy(msr_unc_cbo_perfevtsel0,_msr_unc_cbo_perfevtsel0, max_slices * sizeof (unsigned long long));

        static unsigned long long _msr_unc_cbo_per_ctr0[] = {0x706, 0x716, 0x726, 0x736, 0x746, 0x756, 0x766};
        memcpy(msr_unc_cbo_per_ctr0,_msr_unc_cbo_per_ctr0, max_slices * sizeof (unsigned long long));
        val_disable_ctrs = 0x0;
        val_select_evt_core = 0x408f34;
        val_reset_ctrs = 0x0;
        
        return 0;
    }
    else return 1;

}
