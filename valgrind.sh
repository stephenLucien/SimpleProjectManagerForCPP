#!/bin/bash

#
export TARGET_HOST=${TARGET_HOST=host_gprof}

./compile.sh refresh_and_rebuild
if test $? -ne 0; then
    exit 1
fi

export BUILD_DIR=${BUILD_DIR=build_${TARGET_HOST}}

EXEC=${BUILD_DIR}/exec.strip

# memory error detector
# ​​Memory Leaks​​: Identifies unreleased heap blocks (e.g., missing free/delete), categorizing leaks as "definitely lost," "indirectly lost," or "still reachable" .
# ​​Invalid Memory Access​​: Flags out-of-bounds array accesses, use-after-free errors, and stack/heap boundary violations .
# ​​Uninitialized Values​​: Detects usage of uninitialized variables or heap memory .
# ​​Mismatched Allocation/Deallocation​​: Warns about mismatched APIs (e.g., malloc + delete or new[] + free) .
# ​​Overlapping Memory Copies​​: Checks for invalid memcpy operations with overlapping source/destination pointers
val_mem() {
    valgrind --tool=memcheck \
        --leak-check=full \
        --show-leak-kinds=all \
        --track-origins=yes \
        --error-exitcode=1 \
        ${EXEC}
}

# performance profiler
# ​​Function Call Graphs​​: Generates detailed call hierarchies to identify hot functions .
# ​​CPU Instruction Counting​​: Tracks instructions executed per function, helping optimize computational hotspots .
# ​​Cache Simulation​​: Simulates L1/L2 cache behavior to pinpoint cache misses impacting performance .
# ​​Thread-Specific Profiling​​: With --separate-threads=yes, profiles multi-threaded workloads per thread .
val_call() {
    valgrind --tool=callgrind \
        --separate-threads=yes \
        --callgrind-out-file=${EXEC}.callgrind.out \
        ${EXEC}

    # visualize results
    kcachegrind ${EXEC}.callgrind.out
}

# Thread Error Detector
# ​​Data Races​​: Detects concurrent access to shared variables without proper locking .
# ​​Lock Ordering Problems​​: Identifies potential deadlocks caused by inconsistent lock acquisition sequences .
# ​​POSIX API Misuse​​: Flags incorrect usage of pthread APIs (e.g., unlocking a non-owned mutex) .
# ​​Atomicity Violations​​: Warns about non-atomic operations on shared data .
val_hel() {
    valgrind --tool=helgrind \
        ${EXEC}
}

case $1 in
mem)
    val_mem
    ;;
call)
    val_call
    ;;
hel)
    val_hel
    ;;
*)
    val_mem
    ;;
esac
