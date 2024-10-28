

#define clflush(p) asm volatile("clflush (%0)" ::"r"(p));
int is_intel();
int get_cpu_architecture();
int get_cpu_model();


unsigned long threads_per_core();
unsigned long threads_per_package();
unsigned long cores_per_package();
void print_bin(uint64_t val);

void quicksort(int a[], int l, int r);
uintptr_t read_pagemap(char *path_buf, uintptr_t virt_addr);