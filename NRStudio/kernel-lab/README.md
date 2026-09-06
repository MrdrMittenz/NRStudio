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

## Feed-forward validation and full-model GPU timing

feedforward_bench.cpp now exercises the original SM86 plain and chained FP8
feed-forward and projection functions from module 4. The structs are 56 and 72
bytes. Launches use captured 1440p shapes: grid 10,6,2 / block 32,8,1 and grid
20,6,1 / block 32,4,1, with height 48 and width 80. Synthetic input buffers are
populated before marking input flags ready (zero); output flags start at -1.

Both synthetic seeds pass five repeats for all four variants. Chained output
matches plain output byte-for-byte across the full 16 MiB output allocation;
1,966,080 bytes change from the sentinel and each chained kernel publishes 120
completion flags. Allocation guards pass and Compute Sanitizer memcheck reports
zero errors. This is a narrow equivalence check on synthetic packed inputs, not
a proof for all activations, offsets, weights or concurrent graph scheduling.
It does not justify replacing a chained kernel with a plain one in the model.

Build with build_feedforward.cmd, then run feedforward_bench.exe with the path to
module-4.2.sm_86.cubin. An optional exact kernel name selects one function for
profiling. The unprofiled stages take roughly 0.09 and 0.05 ms per call in this
harness. They are called 15 times each per model evaluation at the traced shape.

chain_timing.h adds D3D12 GPU timestamps around each original NVAPI kernel-chain
submission in the isolated full-model probe. The chains are not split or their
kernel arguments changed. The driver also makes null/empty initialization calls;
these are forwarded without instrumentation. An initial instrumented probe
crashed before this guard was added; the guarded version passed all subsequent
captures. Results are exported only after successful completion of the probe.

The observed graph submits 156 single-kernel chains per evaluation. A three-frame
capture and a separate ten-frame capture completed with successful evaluation and
finite, nonzero outputs. The first three frame output statistics also match the
earlier probe without GPU timestamps. The ten-frame capture's median warm-frame
sum of chain spans is 39.632 ms; one 65.860 ms outlier remains in the report.
summarize_chains.py retains all frames, excludes initialization frame 0 from
ranking, and reports both means and medians of each stage's per-frame total.

| Stage | Calls/frame | Median total GPU span/frame |
|---|---:|---:|
| fused post block, swin 1h/32, FP8 | 1 | 5.083 ms |
| fused swin 8h/256/8 chained FP8 | 12 | 4.351 ms |
| fused swin 1h/32/1 chained FP8 | 4 | 3.784 ms |
| fused pre block, swin 1h/32/1 downsample FP8 | 1 | 2.983 ms |
| fused swin 4h/128/4 chained FP8 | 8 | 2.566 ms |
| feed-forward 512 chained FP8 | 15 | 1.375 ms |

These are measurements of the full modified model on synthetic textures, with
real model weights and its original dispatch. Timestamp insertion/resolution can
affect scheduling, and the spans include chain execution effects. They are not
isolated Nsight instruction counters or an in-game FPS benchmark. Medians of
individual stage totals need not sum to the median full-frame total.

The results move the next optimization target to the post block and fused
attention kernels, rather than the isolated decoder or buffer clear. The post
block uses a 184-byte struct and grid 321,185,1 with one warp per block; recovering
its texture/surface and tensor arguments is still needed for standalone testing.
No measured code optimization has been deployed from this lab.

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
