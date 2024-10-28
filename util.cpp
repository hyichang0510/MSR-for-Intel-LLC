#include "cpuid.h"
#include <assert.h>
#include <errno.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

#define PAGEMAP_ENTRY 8
#define GET_BIT(X, Y) (X & ((uint64_t)1 << Y)) >> Y

#define GET_PFN(X) (X & 0x7FFFFFFFFFFFFF)
#define cpuid(ina, inc, a, b, c, d)                                            \
    asm("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(ina), "c"(inc));


int is_intel() {
    // Output registers
    unsigned long eax;

    // Input EAX = 0x0 for vendor identification string
    int eax_in = 0;
    int ecx_in = 0;
    int name[4] = {0, 0, 0, 0};

    cpuid(eax_in, ecx_in, eax, name[0], name[2], name[1]);

    if (strcmp((char *)name, "GenuineIntel") != 0) {
        return 0;
    }

    return 1;
}



int get_cpu_architecture() {
    unsigned int model;
    int name[4] = {0, 0, 0, 0};
    __cpuid(0, model, name[0], name[2], name[1]);
    if (strcmp((char *)name, "GenuineIntel") != 0)
        return -1;
    return model;
}

int get_cpu_model() { // TODO
    // Output registers
    unsigned long eax, ebx, ecx, edx;

    // Input EAX = 0x1 for Type, Family, Model, and Stepping ID
    int eax_in = 1;
    int ecx_in = 0;

    cpuid(eax_in, ecx_in, eax, ebx, ecx, edx);

    int extended_model = (eax & ~(~0 << 4) << 16) >> 16; // EAX, bits 19-16
    int model = (eax & ~(~0 << 4) << 4) >> 4;            // EAX, bits 7-4
    int cpu_model = (extended_model << 4) + model;

    return cpu_model;
}



unsigned long threads_per_core() {
    // Output registers
    unsigned long eax, ebx, ecx, edx;

    // Input EAX = 0xB for 2xAPIC information leaf, ECX for sub-leaves
    int eax_in = 11;
    int ecx_in = 0;

    cpuid(eax_in, ecx_in, eax, ebx, ecx, edx);

    return ebx;
}

unsigned long threads_per_package() {
    // Output registers
    unsigned long eax, ebx, ecx, edx;

    // Input EAX = 0xB for 2xAPIC information leaf, ECX for sub-leaves
    int eax_in = 11;
    int ecx_in = 1;

    cpuid(eax_in, ecx_in, eax, ebx, ecx, edx);
    return ebx;
}

unsigned long cores_per_package() {
    return threads_per_package() / threads_per_core();
}



void print_bin(uint64_t val) {
    int i;
    for (i = 63; i >= 0; i--) {
        if ((val >> i) & 1) {
            printf("1");
        } else
            printf("0");
    }
}

int partition(int a[], int l, int r) {
    int pivot, i, j, t;
    pivot = a[l];
    i = l;
    j = r + 1;

    while (1) {
        do
            ++i;
        while (a[i] <= pivot && i <= r);
        do
            --j;
        while (a[j] > pivot);
        if (i >= j)
            break;
        t = a[i];
        a[i] = a[j];
        a[j] = t;
    }

    t = a[l];
    a[l] = a[j];
    a[j] = t;
    return j;
}

void quicksort(int a[], int l, int r) {
    int j;
    if (l < r) {
        // divide and conquer
        j = partition(a, l, r);
        quicksort(a, l, j - 1);
        quicksort(a, j + 1, r);
    }
}

const int __endian_bit = 1;
#define is_bigendian() ((*(char *)&__endian_bit) == 0)

uintptr_t read_pagemap(char *path_buf, uintptr_t virt_addr) {
    int i, c, status;
    uint64_t file_offset;
    uintptr_t read_val;
    FILE *f;

    f = fopen(path_buf, "rb");
    if (!f) {
        printf("Error! Cannot open %s\n", path_buf);
        return -1;
    }

    // Shifting by virt-addr-offset number of bytes
    // and multiplying by the size of an address (the size of an entry in
    // pagemap file)
    file_offset = virt_addr / getpagesize() * PAGEMAP_ENTRY;
    status = fseek(f, file_offset, SEEK_SET);
    if (status) {
        perror("Failed to do fseek!");
        return -1;
    }
    errno = 0;
    read_val = 0;
    unsigned char c_buf[PAGEMAP_ENTRY];
    for (i = 0; i < PAGEMAP_ENTRY; i++) {
        c = getc(f);
        if (c == EOF) {
            printf("\nReached end of the file\n");
            return 0;
        }
        if (is_bigendian())
            c_buf[i] = c;
        else
            c_buf[PAGEMAP_ENTRY - i - 1] = c;
    }
    for (i = 0; i < PAGEMAP_ENTRY; i++) {
        read_val = (read_val << 8) + c_buf[i];
    }
    if (GET_BIT(read_val, 63)) {
        // printf("PFN: 0x%llx\n",(unsigned long long) GET_PFN(read_val));
    } else {
        // printf("Page not present\n");
        return 0;
    }
    if (GET_BIT(read_val, 62)) {
        printf("Page swapped\n");
        return 0;
    }
    fclose(f);

    uintptr_t phys_addr;
    phys_addr = GET_PFN(read_val) << 12 | (virt_addr & 0xFFF);

    return phys_addr;
}