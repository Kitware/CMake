# this file simulates an execution of cuda-memcheck

set(LOG_FILE "$ENV{PSEUDO_LOGFILE}")
message("LOG_FILE=[${LOG_FILE}]")

# clear the log file
file(REMOVE "${LOG_FILE}")

# create an error of each type of sanitizer tool and failure

# initcheck
file(APPEND "${LOG_FILE}"
"========= CUDA-MEMCHECK
========= Uninitialized __global__ memory read of size 4
=========     at 0x00000020 in test(int*, int*)
=========     by thread (0,0,0) in block (0,0,0)
=========     Address 0x1303d80000
=========     Saved host backtrace up to driver entry point
=========     Host Frame:/lib64/libcuda.so.1 (cuLaunchKernel + 0x346) [0x297db6]
=========     Host Frame:./uninit-read [0x101d9]
=========     Host Frame:./uninit-read [0x10267]
=========     Host Frame:./uninit-read [0x465b5]
=========     Host Frame:./uninit-read [0x3342]
=========     Host Frame:./uninit-read [0x3143]
=========     Host Frame:/lib64/libc.so.6 (__libc_start_main + 0xf5) [0x22505]
=========     Host Frame:./uninit-read [0x31e2]
=========
========= Unused memory in allocation 0x1303d80000 of size 16 bytes
=========     Not written any memory.
=========     100.00% of allocation were unused.
=========     Saved host backtrace up to driver entry point
=========     Host Frame:/lib64/libcuda.so.1 (cuMemAlloc_v2 + 0x1b7) [0x26ec97]
=========     Host Frame:./uninit-read [0x2bbd3]
=========     Host Frame:./uninit-read [0x71ab]
=========     Host Frame:./uninit-read [0x3c84f]
=========     Host Frame:./uninit-read [0x3111]
=========     Host Frame:/lib64/libc.so.6 (__libc_start_main + 0xf5) [0x22505]
=========     Host Frame:./uninit-read [0x31e2]
=========
========= Host API memory access error at host access to 0x1303fd1400 of size 25600 bytes
=========     Uninitialized access at 0x1303fd4600 on access by cudaMemcopy source.
=========     Saved host backtrace up to driver entry point at error
=========     Host Frame:/usr/lib/x86_64-linux-gnu/libcuda.so.1 (cuMemcpyDtoH_v2 + 0x1ec) [0x29200c]
=========     Host Frame:/usr/local/cuda/targets/x86_64-linux/lib/libcudart.so.10.1 [0x38aaa]
=========     Host Frame:/usr/local/cuda/targets/x86_64-linux/lib/libcudart.so.10.1 [0x18946]
=========     Host Frame:/usr/local/cuda/targets/x86_64-linux/lib/libcudart.so.10.1 (cudaMemcpy + 0x1a2) [0x3b8c2]
=========     Host Frame:/something/somewhere [0xcafe]
=========
========= ERROR SUMMARY: 2 errors
")


# synccheck
file(APPEND "${LOG_FILE}"
"========= CUDA-MEMCHECK
========= Barrier error detected. Divergent thread(s) in warp
=========     at 0x00000058 in test(int*, int*)
=========     by thread (1,0,0) in block (0,0,0)
=========     Device Frame:test(int*, int*) (test(int*, int*) : 0x60)
=========     Saved host backtrace up to driver entry point at kernel launch time
=========     Host Frame:/lib64/libcuda.so.1 (cuLaunchKernel + 0x346) [0x297db6]
=========     Host Frame:./sync [0x101d9]
=========     Host Frame:./sync [0x10267]
=========     Host Frame:./sync [0x465b5]
=========     Host Frame:./sync [0x3342]
=========     Host Frame:./sync [0x314a]
=========     Host Frame:/lib64/libc.so.6 (__libc_start_main + 0xf5) [0x22505]
=========     Host Frame:./sync [0x31e2]
=========
========= Barrier error detected. Divergent thread(s) in warp
=========     at 0x00000058 in test(int*, int*)
=========     by thread (0,0,0) in block (0,0,0)
=========     Device Frame:test(int*, int*) (test(int*, int*) : 0x60)
=========     Saved host backtrace up to driver entry point at kernel launch time
=========     Host Frame:/lib64/libcuda.so.1 (cuLaunchKernel + 0x346) [0x297db6]
=========     Host Frame:./sync [0x101d9]
=========     Host Frame:./sync [0x10267]
=========     Host Frame:./sync [0x465b5]
=========     Host Frame:./sync [0x3342]
=========     Host Frame:./sync [0x314a]
=========     Host Frame:/lib64/libc.so.6 (__libc_start_main + 0xf5) [0x22505]
=========     Host Frame:./sync [0x31e2]
=========
========= ERROR SUMMARY: 2 errors
")

# memcheck
file(APPEND "${LOG_FILE}"
"========= CUDA-MEMCHECK
========= Invalid __global__ read of size 4
=========     at 0x00000020 in test(int*, int*)
=========     by thread (0,0,0) in block (0,0,0)
=========     Address 0x00000000 is out of bounds
=========     Saved host backtrace up to driver entry point at kernel launch time
=========     Host Frame:/lib64/libcuda.so.1 (cuLaunchKernel + 0x346) [0x297db6]
=========     Host Frame:./invalid-read [0x101d9]
=========     Host Frame:./invalid-read [0x10267]
=========     Host Frame:./invalid-read [0x465b5]
=========     Host Frame:./invalid-read [0x3342]
=========     Host Frame:./invalid-read [0x3142]
=========     Host Frame:/lib64/libc.so.6 (__libc_start_main + 0xf5) [0x22505]
=========     Host Frame:./invalid-read [0x31e2]
=========
========= Program hit cudaErrorLaunchFailure (error 719) due to \"unspecified launch failure\" on CUDA API call to cudaDeviceSynchronize.
=========     Saved host backtrace up to driver entry point at error
=========     Host Frame:/lib64/libcuda.so.1 [0x3ac5a3]
=========     Host Frame:./invalid-read [0x2e576]
=========     Host Frame:./invalid-read [0x3147]
=========     Host Frame:/lib64/libc.so.6 (__libc_start_main + 0xf5) [0x22505]
=========     Host Frame:./invalid-read [0x31e2]
=========
========= Program hit cudaErrorLaunchFailure (error 719) due to \"unspecified launch failure\" on CUDA API call to cudaFree.
=========     Saved host backtrace up to driver entry point at error
=========     Host Frame:/lib64/libcuda.so.1 [0x3ac5a3]
=========     Host Frame:./invalid-read [0x3c106]
=========     Host Frame:./invalid-read [0x3150]
=========     Host Frame:/lib64/libc.so.6 (__libc_start_main + 0xf5) [0x22505]
=========     Host Frame:./invalid-read [0x31e2]
=========
========= Fatal UVM GPU fault of type invalid pde due to invalid address
=========     during atomic access to address 0x20be00000
=========
========= Fatal UVM CPU fault due to invalid operation
=========     during read access to address 0x1357c92000
=========
========= LEAK SUMMARY: 0 bytes leaked in 0 allocations
========= ERROR SUMMARY: 3 errors
")

# memcheck with leak-check full
file(APPEND "${LOG_FILE}"
"========= CUDA-MEMCHECK
========= Leaked 10 bytes at 0x1303d80000
=========     Saved host backtrace up to driver entry point at cudaMalloc time
=========     Host Frame:/lib64/libcuda.so.1 (cuMemAlloc_v2 + 0x1b7) [0x26ec97]
=========     Host Frame:./leak [0x2bab3]
=========     Host Frame:./leak [0x708b]
=========     Host Frame:./leak [0x3c72f]
=========     Host Frame:./leak [0x3113]
=========     Host Frame:/lib64/libc.so.6 (__libc_start_main + 0xf5) [0x22505]
=========     Host Frame:./leak [0x3174]
=========
========= LEAK SUMMARY: 10 bytes leaked in 1 allocations
========= ERROR SUMMARY: 1 error
")

# racecheck with racecheck-report all
file(APPEND "${LOG_FILE}"
"========= CUDA-MEMCHECK
========= WARN:(Warp Level Programming) Potential WAR hazard detected at __shared__ 0x3 in block (0, 0, 0) :
=========     Read Thread (31, 0, 0) at 0x00000170 in ./race.cu:4:test(int*, int*)
=========     Write Thread (0, 0, 0) at 0x000001a8 in ./race.cu:4:test(int*, int*)
=========     Current Value : 0, Incoming Value : 0
=========     Saved host backtrace up to driver entry point at kernel launch time
=========     Host Frame:/lib64/libcuda.so.1 (cuLaunchKernel + 0x346) [0x297db6]
=========     Host Frame:./race [0x101d9]
=========     Host Frame:./race [0x10267]
=========     Host Frame:./race [0x465b5]
=========     Host Frame:./race [0x3342]
=========     Host Frame:./race [0x314a]
=========     Host Frame:/lib64/libc.so.6 (__libc_start_main + 0xf5) [0x22505]
=========     Host Frame:./race [0x31e2]
=========
========= WARN:(Warp Level Programming) Potential WAR hazard detected at __shared__ 0x2 in block (0, 0, 0) :
=========     Read Thread (31, 0, 0) at 0x00000170 in ./race.cu:4:test(int*, int*)
=========     Write Thread (0, 0, 0) at 0x000001a8 in ./race.cu:4:test(int*, int*)
=========     Current Value : 0, Incoming Value : 0
=========     Saved host backtrace up to driver entry point at kernel launch time
=========     Host Frame:/lib64/libcuda.so.1 (cuLaunchKernel + 0x346) [0x297db6]
=========     Host Frame:./race [0x101d9]
=========     Host Frame:./race [0x10267]
=========     Host Frame:./race [0x465b5]
=========     Host Frame:./race [0x3342]
=========     Host Frame:./race [0x314a]
=========     Host Frame:/lib64/libc.so.6 (__libc_start_main + 0xf5) [0x22505]
=========     Host Frame:./race [0x31e2]
=========
========= WARN:(Warp Level Programming) Potential WAR hazard detected at __shared__ 0x1 in block (0, 0, 0) :
=========     Read Thread (31, 0, 0) at 0x00000170 in ./race.cu:4:test(int*, int*)
=========     Write Thread (0, 0, 0) at 0x000001a8 in ./race.cu:4:test(int*, int*)
=========     Current Value : 0, Incoming Value : 0
=========     Saved host backtrace up to driver entry point at kernel launch time
=========     Host Frame:/lib64/libcuda.so.1 (cuLaunchKernel + 0x346) [0x297db6]
=========     Host Frame:./race [0x101d9]
=========     Host Frame:./race [0x10267]
=========     Host Frame:./race [0x465b5]
=========     Host Frame:./race [0x3342]
=========     Host Frame:./race [0x314a]
=========     Host Frame:/lib64/libc.so.6 (__libc_start_main + 0xf5) [0x22505]
=========     Host Frame:./race [0x31e2]
=========
========= WARN:(Warp Level Programming) Potential WAR hazard detected at __shared__ 0x0 in block (0, 0, 0) :
=========     Read Thread (31, 0, 0) at 0x00000170 in ./race.cu:4:test(int*, int*)
=========     Write Thread (0, 0, 0) at 0x000001a8 in ./race.cu:4:test(int*, int*)
=========     Current Value : 0, Incoming Value : 1
=========     Saved host backtrace up to driver entry point at kernel launch time
=========     Host Frame:/lib64/libcuda.so.1 (cuLaunchKernel + 0x346) [0x297db6]
=========     Host Frame:./race [0x101d9]
=========     Host Frame:./race [0x10267]
=========     Host Frame:./race [0x465b5]
=========     Host Frame:./race [0x3342]
=========     Host Frame:./race [0x314a]
=========     Host Frame:/lib64/libc.so.6 (__libc_start_main + 0xf5) [0x22505]
=========     Host Frame:./race [0x31e2]
=========
========= WARN:(Warp Level Programming) Potential RAW hazard detected at __shared__ 0x3 in block (0, 0, 0) :
=========     Write Thread (31, 0, 0) at 0x00000148 in ./race.cu:3:test(int*, int*)
=========     Read Thread (0, 0, 0) at 0x00000170 in ./race.cu:4:test(int*, int*)
=========     Current Value : 0
=========     Saved host backtrace up to driver entry point at kernel launch time
=========     Host Frame:/lib64/libcuda.so.1 (cuLaunchKernel + 0x346) [0x297db6]
=========     Host Frame:./race [0x101d9]
=========     Host Frame:./race [0x10267]
=========     Host Frame:./race [0x465b5]
=========     Host Frame:./race [0x3342]
=========     Host Frame:./race [0x314a]
=========     Host Frame:/lib64/libc.so.6 (__libc_start_main + 0xf5) [0x22505]
=========     Host Frame:./race [0x31e2]
=========
========= WARN:(Warp Level Programming) Potential RAW hazard detected at __shared__ 0x2 in block (0, 0, 0) :
=========     Write Thread (31, 0, 0) at 0x00000148 in ./race.cu:3:test(int*, int*)
=========     Read Thread (0, 0, 0) at 0x00000170 in ./race.cu:4:test(int*, int*)
=========     Current Value : 0
=========     Saved host backtrace up to driver entry point at kernel launch time
=========     Host Frame:/lib64/libcuda.so.1 (cuLaunchKernel + 0x346) [0x297db6]
=========     Host Frame:./race [0x101d9]
=========     Host Frame:./race [0x10267]
=========     Host Frame:./race [0x465b5]
=========     Host Frame:./race [0x3342]
=========     Host Frame:./race [0x314a]
=========     Host Frame:/lib64/libc.so.6 (__libc_start_main + 0xf5) [0x22505]
=========     Host Frame:./race [0x31e2]
=========
========= WARN:(Warp Level Programming) Potential RAW hazard detected at __shared__ 0x1 in block (0, 0, 0) :
=========     Write Thread (31, 0, 0) at 0x00000148 in ./race.cu:3:test(int*, int*)
=========     Read Thread (0, 0, 0) at 0x00000170 in ./race.cu:4:test(int*, int*)
=========     Current Value : 0
=========     Saved host backtrace up to driver entry point at kernel launch time
=========     Host Frame:/lib64/libcuda.so.1 (cuLaunchKernel + 0x346) [0x297db6]
=========     Host Frame:./race [0x101d9]
=========     Host Frame:./race [0x10267]
=========     Host Frame:./race [0x465b5]
=========     Host Frame:./race [0x3342]
=========     Host Frame:./race [0x314a]
=========     Host Frame:/lib64/libc.so.6 (__libc_start_main + 0xf5) [0x22505]
=========     Host Frame:./race [0x31e2]
=========
========= WARN:(Warp Level Programming) Potential RAW hazard detected at __shared__ 0x0 in block (0, 0, 0) :
=========     Write Thread (31, 0, 0) at 0x00000148 in ./race.cu:3:test(int*, int*)
=========     Read Thread (0, 0, 0) at 0x00000170 in ./race.cu:4:test(int*, int*)
=========     Current Value : 0
=========     Saved host backtrace up to driver entry point at kernel launch time
=========     Host Frame:/lib64/libcuda.so.1 (cuLaunchKernel + 0x346) [0x297db6]
=========     Host Frame:./race [0x101d9]
=========     Host Frame:./race [0x10267]
=========     Host Frame:./race [0x465b5]
=========     Host Frame:./race [0x3342]
=========     Host Frame:./race [0x314a]
=========     Host Frame:/lib64/libc.so.6 (__libc_start_main + 0xf5) [0x22505]
=========     Host Frame:./race [0x31e2]
=========
========= WARN: Race reported between Read access at 0x00000170 in ./race.cu:4:test(int*, int*)
=========     and Write access at 0x00000148 in ./race.cu:3:test(int*, int*) [4 hazards]
=========     and Write access at 0x000001a8 in ./race.cu:4:test(int*, int*) [4 hazards]
=========
========= WARN: Race reported between Write access at 0x00000148 in ./race.cu:3:test(int*, int*)
=========     and Write access at 0x00000148 in ./race.cu:3:test(int*, int*) [124 hazards]
=========     and Read access at 0x00000170 in ./race.cu:4:test(int*, int*) [4 hazards]
=========
========= WARN: Race reported between Write access at 0x000001a8 in ./race.cu:4:test(int*, int*)
=========     and Write access at 0x000001a8 in ./race.cu:4:test(int*, int*) [124 hazards]
=========     and Read access at 0x00000170 in ./race.cu:4:test(int*, int*) [4 hazards]
=========
========= WARN: Race reported between Write access at 0x00000148 in ./race.cu:3:test(int*, int*)
=========     and Write access at 0x00000148 in ./race.cu:3:test(int*, int*) [124 hazards]
=========     and Read access at 0x00000170 in ./race.cu:4:test(int*, int*) [4 hazards]
=========
========= RACECHECK SUMMARY: 12 hazards displayed (0 errors, 12 warnings)
")

# false-positives
file(APPEND "${LOG_FILE}"
"========= COMPUTE-SANITIZER
========= Error: Target application terminated before first instrumented API call
========= Tracking kernels launched by child processes requires the --target-processes all option.
========= Error: No attachable process found. compute-sanitizer timed-out.
========= Default timeout can be adjusted with --launch-timeout. Awaiting target completion.
")
