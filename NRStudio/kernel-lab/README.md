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
are not guaranteed to equal kernel-only GPU durations. Subsequent model launch
traces below record actual clear-buffer lengths and call counts.

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

## Initial decoder metadata

`cc_dec_input_upsample_1024_512` and its `_fp8` variant both take one 80-byte
parameter structure. The interface was subsequently recovered and exercised
as described below. They use 168 registers/thread, up to 384 threads
per block, and 2064 bytes of static shared memory. Reported local memory is
40 bytes for the base variant and 168 for FP8; local allocation alone does not
prove register spilling or its performance cost.

Static disassembly contains approximately 1659 versus 7215 instructions. The
FP8 variant's larger code/local allocation is an investigation lead, not proof
that it dominates NR time. The exact original model dispatch must be captured
or its parameter struct reconstructed with validated shapes before testing.
The two variants cannot be interchanged merely because their ABI sizes match.

## Decoder launch recovery and validation (2026-09-06)

trace_launches.h instruments NVAPI function creation and kernel-chain submission
inside our isolated D3D12 probe. Calls are forwarded unchanged. This is diagnostic
code, not installed into games. Each kernel's first launch is recorded with its
parameter bytes; counts cover three evaluations, not GPU durations.

The original modified model completed three successful, finite evaluations at
256x256 and 2560x1440. Both invoke the tilesync FP8 decoder once per evaluation.
The clear count is 23,296 and 147,712 respectively, once per evaluation.

| Model output | Decoder grid | Block | Input H/W | Output H/W |
|---|---|---|---|---|
| 256x256 | 4,2,4 | 32,2,1 | 8,8 | 12,12 |
| 2560x1440 | 20,6,4 | 32,2,1 | 24,40 | 48,80 |

The 80-byte parameter contains eight pointers then four int32 dimensions.
PTX access analysis suggests pointer roles: source, skip, output, half partials,
stage flags, input flags, FP8 partials, weights. Stage flags start at -1;
z partitions publish and wait for stage completion. The isolated harness uses
the non-tilesync FP8 variant, avoiding dependencies on absent upstream kernels.
This is not a replay of the entire model or its exact tilesync execution.

decoder_bench.cpp uses synthetic half inputs and small FP8 weight bytes, guarded
allocations and three reset/repeat runs. At the two captured shapes, it writes
36,864 and 983,040 finite half values, respectively, with bit-identical repeated
outputs and unchanged guards. Compute Sanitizer memcheck reports zero errors at
both shapes. This validates the exercised accesses and repeatability; it does
not establish mathematical equivalence to a reference decoder or real game data.

Nsight Compute full application replay completed for both shapes. At the 1440p
shape it reports 48.70 microseconds, 168 registers/thread, 25% theoretical and
13.27% achieved occupancy, and 20,160 local memory spilling requests. The
unprofiled CUDA-event measurements were 50.176, 48.128 and 50.176 microseconds.
Timing emitted by the harness DURING profiling includes instrumentation and must
not be used as normal performance. Profiler estimated speedups are diagnostic
heuristics, not achieved gains or game FPS predictions.

The measured standalone decoder is small compared with the previously measured
whole evaluation. It is not established as a major game bottleneck. The trace
also records 15 calls per evaluation for each of the chained FP8 feed-forward
and feed-forward projection kernels, and 14 for a projection kernel. Recovering
and timing those kernels is the next investigation; counts alone do not rank cost.
No replacement kernel, game runtime, installer or image-quality setting changed.

Build with build_decoder.cmd, then run decoder_bench.exe path\\to\\module-6.2.sm_86.cubin
(small shape) or append `1440`. Run Compute Sanitizer with `--tool memcheck
--error-exitcode 9`. Profile using `ncu --set full --replay-mode application
--launch-count 1 --kernel-name cc_dec_input_upsample_1024_512_fp8`, followed by
the harness command. build_trace.cmd compiles trace_probe.cpp; prepare_trace.py
regenerates it from the local recovery probe. Build scripts currently require
adjusting local SDK/recovery paths on another machine.

Only our harness/instrumentation source, build scripts and diagnostic text are
included in the private review. Extracted NVIDIA PTX/cubins and output textures
remain local and are not included.

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
