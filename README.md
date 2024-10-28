# Performance Counter for LLC intel CPU
This demo code uses Performance counter, aka model specific registers (MSRs), to record the Last Level Cache (LLC) miss or hits instead of a timing side-channel. Further work like reverse engineering hash function rely on this measurement (Hash function reverse engineering needs mathematic methods and doesn't implemented in this code).

Only Sky Lake and Kaby Lake arch are included, but scaning for other CPUs only needs some modification on MSR values and masks.

# Reference
MSRs and their configuration values [https://cdrdv2.intel.com/v1/dl/getContent/671098]