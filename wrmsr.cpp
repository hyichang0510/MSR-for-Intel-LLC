
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "wrmsr.h"




void wrmsr_on_cpu_0(uint32_t reg, int valcnt, uint64_t *regvals) {
    uint64_t data;
    // char msr_file_name[64];
    char *msr_file_name = "/dev/cpu/0/msr";
    int cpu = 0;

    // sprintf(msr_file_name, "/dev/cpu/%d/msr", cpu);
    static int fd = -1;

    if (fd < 0) {
        fd = open(msr_file_name, O_WRONLY);
        if (fd < 0) {
            if (errno == ENXIO) {
                fprintf(stderr, "wrmsr: No CPU %d\n", cpu);
                exit(2);
            } else if (errno == EIO) {
                fprintf(stderr, "wrmsr: CPU %d doesn't support MSRs\n", cpu);
                exit(3);
            } else {
                perror("wrmsr: open");
                exit(127);
            }
        }
    }

    while (valcnt--) {
        // data = strtoull(*regvals++, NULL, 0);
        data = *regvals++;
        // printf("writing msr 0x%08" PRIx32 " to 0x%016" PRIx64 "\n", reg,
        // data);
        if (pwrite(fd, &data, sizeof data, reg) != sizeof data) {
            if (errno == EIO) {
                fprintf(stderr,
                        "wrmsr: CPU %d cannot set MSR "
                        "0x%08" PRIx32 " to 0x%016" PRIx64 "\n",
                        cpu, reg, data);
                exit(4);
            } else {
                perror("wrmsr: pwrite");
                exit(127);
            }
        }
    }

    // close(fd);

    return;
}