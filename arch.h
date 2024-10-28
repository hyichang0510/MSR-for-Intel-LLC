#ifndef SLICE_REVERSE_ARCH_H
#define SLICE_REVERSE_ARCH_H


typedef enum {
    CPU_UNKNOWN,
    INTEL_CORE,
    MAX_CLASS,
} class_t;

typedef enum {
    UARCH_UNKNOWN,
    SKYLAKE,
    KABY_LAKE,
    MAX_ARCH,
} uarch_t;

extern const char *const uarch_names[MAX_ARCH];
extern uarch_t archi;

extern class_t classes; // xeon or core
extern const char *const classes_names[MAX_CLASS];

extern unsigned long long msr_unc_perf_global_ctr;
extern unsigned long long msr_unc_cbo_perfevtsel0[8];
extern unsigned long long msr_unc_cbo_per_ctr0[8];
extern unsigned long long msr_unc_cbo_per_ctr0[8];
extern unsigned long long val_enable_ctrs;
extern unsigned long long val_disable_ctrs;
extern unsigned long long val_select_evt_core;
extern unsigned long long val_reset_ctrs;

extern int nb_cores;
extern int max_slices;
int determine_class_uarch(int cpu_model);
int setup_perf_counters(class_t classes, uarch_t archi, int nb_cores);


#endif