# Isolated SM86 kernel lab

RTX 3090, driver 616.64; 6 September 2026. This experiment does not install or
replace any app/game DLL and does not establish an FPS improvement.

## Verified result

The compiled `cc_cb_clear` kernel can be called directly through the CUDA Driver
API. Driver metadata reports ONE 16-byte parameter, not two separate arguments.
Its inspected layout is a device pointer at offset 0, signed element count at
offset 8, and trailing padding. SASS stores 0xFFFFFFFF into each uint32 element
in bounds. It does not zero the buffer despite the name.

The harness validates bit-exact output and unchanged prefix/suffix guards at
12 lengths: 1, 3, 4, 31, 32, 33, 255, 256, 257, 4096, 57600 and 3686400.
It compares the original compiled kernel, a separately compiled SM86 vector
store kernel and cuMemsetD32Async. All output/guard tests passed.

Timing uses four rotating-order rounds per size with CUDA events and 100 or
1000 launches. Reported timings include effects of CPU submission gaps; they
are not guaranteed to equal kernel-only GPU durations. The model's actual
clear-buffer length, frequency and scheduling have not been captured.

At 57,600 elements median times were 4.486 / 4.451 / 4.545 microseconds for
original / vector / memset. At 3,686,400 they were 20.465 / 20.726 / 20.731.
There is no reliable improvement to deploy from this experiment.

## Profiling breakthrough

Nsight Compute successfully profiled the original kernel through this harness:
one 256-thread block, CC 8.6, eight metric-collection passes. The profiled case
was the first one-element correctness test, not a realistic NR workload.
Its 2.14 microsecond reported duration is useful to confirm profiling works,
not to diagnose occupancy or rank model bottlenecks.

Previously the D3D12 model probe yielded no kernels in Nsight Compute. We now
have a demonstrated profiling route once a kernel's launch interface is known.
No counter permissions or driver security settings were changed.

## Next target identified, not yet launched

`cc_dec_input_upsample_1024_512` and its `_fp8` variant both take one 80-byte
parameter structure. Fields and launch shape still require recovery before a
safe standalone execution. They use 168 registers/thread, up to 384 threads
per block, and 2064 bytes of static shared memory. Reported local memory is
40 bytes for the base variant and 168 for FP8; local allocation alone does not
prove register spilling or its performance cost.

Static disassembly contains approximately 1659 versus 7215 instructions. The
FP8 variant's larger code/local allocation is an investigation lead, not proof
that it dominates NR time. The exact original model dispatch must be captured
or its parameter struct reconstructed with validated shapes before testing.
The two variants cannot be interchanged merely because their ABI sizes match.

## Reproduce

Requires the local inspected module-6.2.sm_86.cubin (not included here), CUDA
13.3 tools, MSVC 14.38 and an RTX 3090. build.cmd compiles our candidate and
harness. Run:

```
clear_bench.exe path\to\module-6.2.sm_86.cubin clear_candidate.cubin
ncu --set basic --launch-count 1 --kernel-name cc_cb_clear --kill yes --log-file ncu-clear.txt clear_bench.exe path\to\module-6.2.sm_86.cubin clear_candidate.cubin
```

The profiler intentionally terminates the test after the selected kernel; run
the unprofiled harness separately for the complete correctness checks.
kernel_metadata.py queries interfaces without launching kernels. summarize.py
records static instruction families and median timing results. Scripts that
refer to the recovery workspace require adjusting that path on another machine.
