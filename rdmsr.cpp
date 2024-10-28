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

#include "rdmsr.h"

uint64_t rdmsr_on_cpu_0(uint32_t reg) {
    uint64_t data;
    int cpu = 0;

    // char *pat;
    // int width;
    char *msr_file_name = "/dev/cpu/0/msr";
    /*	unsigned int bits;*/

    // sprintf(msr_file_name, "/dev/cpu/%d/msr", cpu);

    static int fd = -1;

    if (fd < 0) {
        fd = open(msr_file_name, O_RDONLY);
        if (fd < 0) {
            if (errno == ENXIO) {
                fprintf(stderr, "rdmsr: No CPU %d\n", cpu);
                exit(2);
            } else if (errno == EIO) {
                fprintf(stderr, "rdmsr: CPU %d doesn't support MSRs\n", cpu);
                exit(3);
            } else {
                perror("rdmsr: open");
                exit(127);
            }
        }
    }

    if (pread(fd, &data, sizeof data, reg) != sizeof data) {
        if (errno == EIO) {
            fprintf(stderr,
                    "rdmsr: CPU %d cannot read "
                    "MSR 0x%08" PRIx32 "\n",
                    cpu, reg);
            exit(4);
        } else {
            perror("rdmsr: pread");
            exit(127);
        }
    }

    // close(fd);

    return data;
}
