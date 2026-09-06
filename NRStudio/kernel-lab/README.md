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

## Post-block harness and cache experiment

post_bench.cpp runs the original compiled SM86 post block with a 184-byte
parameter structure, linear tensor buffers, a normalized CUDA texture object,
and a CUDA surface object. The 1440p case uses the recorded 2560x1472 tensor
dimensions, (-4,-4) offsets, and grid 321,185,1 / block 32,1,1. The source and
destination CUDA arrays are float32 RGBA; the game uses D3D12 resources and its
exact texture descriptors have not been reproduced. Weights and tensors are
synthetic. This is not a capture/replay of actual model intermediates.

An initial tensor allocation assumption was too small. Compute Sanitizer caught
23,345 errors despite finite, repeatable-looking output; those timings were
rejected. Extra edge padding alone was insufficient. The final harness uses
overprovisioned storage (16 bytes per tensor pixel for the first buffer, 32 for
the second, plus surrounding padding), and the exercised 1440p launch passes
memcheck with zero errors. The exact native tensor extents/layout are still not
established; allocating sufficient synthetic storage is not a proof of them.
The installed runtime was not modified by these failures or tests.

All output values are written and finite, repeated output is bit-identical, and
allocation guards pass. Nsight Compute full application replay on the validated
harness reports 168 registers/thread, 96 local bytes/thread, no static shared
memory, 25% theoretical occupancy, 24.95% achieved occupancy, and register
spilling. Reported duration is 6.09 ms, DRAM throughput 10.85% of peak, and
approximately 839 million executed warp instructions. These measurements suggest
instruction/dependency costs deserve investigation; they do not prove which
source-level operation dominates or what speedup a rewrite could achieve.

The cache experiment compares the default, minimum, and maximum preferred shared
memory carveouts through cuFuncSetAttribute. These values are driver preferences,
not guaranteed physical allocations. Two separate runs each use 30 rotating-order
measurements, ten launches per measurement, with identical-output and guard
checks. The first three measurements are excluded from the timing summary.

| Run | Default median | Maximum carveout median | Difference |
|---|---:|---:|---:|
| First | 5.178020 ms | 5.117625 ms | 0.060395 ms (1.17%) |
| Repeat | 5.177482 ms | 5.111897 ms | 0.065585 ms (1.27%) |

Individual samples overlap. This small standalone improvement is not an in-game
FPS result and has not been integrated into the D3D12 runtime. It is not evidence
of a substantial performance unlock. Further work should investigate the compiled
kernel's instruction/conversion paths with a stronger reference-output comparison
before attempting an arithmetic rewrite.

Build with build_post.cmd. Run `post_bench.exe path\\to\\module-0.2.sm_86.cubin`
for 1440p, append `small` for the synthetic 256x256 case, or `cache` for the
rotating cache experiment. summarize_post.py validates and summarizes both saved
cache runs. ncu-post-1440.txt contains the validated harness profile; instrumentation
inflates the separate harness timings printed during profiling. No NVIDIA binary,
game file, installer or image-quality setting was changed.

## Exact FP8 conversion building blocks

Inspection of the deployed post-block SASS found 14,504 static instructions,
including extensive packed-half arithmetic and integer manipulation. It already
uses half2 multiply-by-256 decoding and packed rounding logic. Replacing a slow
generic CUDA-header fallback is therefore not an established optimization for
this binary. analyze_post_instructions.py records counts without publishing the
proprietary disassembly. Counts alone do not identify dynamic bottlenecks.

fp8_exact.cuh supplies our source implementations of E4M3 finite-saturating,
nearest-even conversion between binary16 and FP8, including packed pairs. It
also supplies nr_quantize_e4m3_half2: a fused operation that keeps the result in
binary16 while reproducing an FP8 encode/decode round trip. Normal values round
away seven fraction bits; subnormal values round at the FP8 subnormal spacing.
Signed zeros, finite saturation, and canonical NaNs are preserved according to
the tested CUDA reference behavior.

The potential use is avoiding intermediate packing/unpacking where an FP16
arithmetic path immediately consumes quantized values. It is NOT a replacement
for FP8 tensor storage, and it cannot be substituted for arbitrary FP8 MMA
instructions without recovering their fragment layout and accumulation behavior.
Holding expanded values in registers could also increase register pressure.

fp8_exact_test.cu compares against the CUDA 13.3 FP8 header implementation, both
on the host and on the RTX 3090 compiled for SM86:

- All 65,536 binary16 encodings pass the scalar encoder comparison.
- All 256 FP8 encodings pass the scalar decoder comparison.
- All 65,536 packed FP8 pairs pass the packed decoder comparison.
- 262,144 packed binary16 pairs pass encoder and fused-quantizer comparisons.
  The four pairing patterns cover every binary16 value in each lane, with
  complements, a permutation, sign changes and NaN companions. This is not all
  2^32 possible packed binary16 pairs.
- GPU Compute Sanitizer memcheck reports zero errors for the final tests.

There are zero bit mismatches in these tests, including nonfinite inputs. This
does not prove equivalence to every existing modified-kernel instruction path
or to Blackwell hardware behavior. No complete post-block candidate has been
rebuilt or benchmarked with this fused routine, and no speedup is claimed.
The next integration work must preserve packed layouts, validate MMA operand
mapping/accumulation, compare full-block outputs, and check resource usage.

Run build_fp8_exact.cmd followed by fp8_exact_test.exe. CUDA 13.3 and MSVC 14.38
paths in the build script may need adjustment. No NVIDIA PTX, cubin, game DLL or
installer is altered by these tests.

## Full post-block compatibility prototype

fp8_mma.cuh implements two ways to map E4M3 m16n8k32 operands onto two SM86 FP16
m16n8k16 operations. One uses contiguous K halves and lane shuffles; the other
uses corresponding pair-interleaved permutations of A and B to avoid those
shuffles. The mapping follows NVIDIA's [PTX fragment documentation](https://docs.nvidia.com/cuda/parallel-thread-execution/index.html#warp-level-matrix-fragment-mma-16832)
and its [FP16 fragment documentation](https://docs.nvidia.com/cuda/parallel-thread-execution/index.html#warp-level-matrix-fragment-mma-16816-float).
Different partitions can change FP16 accumulation results despite computing the
same real-valued dot product.

mma_mapping_test.cu uses 128 matrices with signed, dense, sparse-routing and
nonzero accumulator cases. All 16,384 output values match independent CPU matrix
products exactly for both mappings. Values were chosen so sums/products are
exactly representable: this checks operand placement, not arbitrary floating-point
accumulation equivalence. The current interleaved mapping also passes memcheck.

build_post_helpers.cmd generates PTX from our conversion/MMA helpers.
build_post_prototype.py extracts only the local clean NVIDIA post-block entry and
replaces 388 FP8 encodes, 40 decodes and 256 FP8 MMA operations with helper calls.
It emits a separate local SM86 cubin. The final interleaved prototype compiles with
160 registers/thread and zero reported stack/spill bytes. PTX helper calls may be
inlined by ptxas. No NVIDIA model DLL is patched, and this compatibility baseline
does not yet use the fused quantizer from the preceding experiment.

The initial contiguous prototype matched all 262,144 output values on the uniform
256x256 synthetic case and passed memcheck, but was substantially slower (roughly
0.89 ms versus the original's roughly 0.12 ms in this small harness). Varying the
tensors and weights revealed mismatches. Switching to the interleaved partition
did not resolve them. The final prototype is **rejected for deployment**.

| Interleaved prototype test | Bit mismatches / 262,144 values | Maximum absolute difference |
|---|---:|---:|
| Varied tensors/weights, pattern 1 | 75,939 | 0.0017700195 |
| Varied tensors/weights, pattern 2 | 119,421 | 0.0014343262 |
| Vary first tensor only | 0 | 0 |
| Vary second tensor only | 0 | 0 |
| Vary weights only | 61,504 | 0.0000038147 |

These differential tests point toward an arithmetic compatibility issue but do
not prove its cause, nor do they establish the exact native tensor layout.
The nonuniform prototype run still has finite, repeatable output and passes
Compute Sanitizer memcheck. Memory safety and isolated conversion correctness
therefore do not establish whole-block output equivalence.

The next requirement is resolving compatibility with the current compiled block's
arithmetic before fusing or tuning the replacement. The signed clean PTX is not
assumed identical to the modified SM86 implementation merely because names and
parameter sizes match. No full-model or game FPS improvement is claimed.

Reproduce with build_mma_mapping.cmd and mma_mapping_test.exe, then
build_post_helpers.cmd and `python build_post_prototype.py`. The latter requires
the local extracted vendor PTX and checks expected transformation counts.
post_bench.exe now accepts an optional output filename (third argument after the
executable) and pattern number (fourth); `small` selects 256x256. Patterns 1/2
vary all tensors/weights and 3/4/5 vary individual inputs. Explicit stream
synchronization precedes host-driven pattern uploads. compare_post_prototype.py
compares local artifacts and writes only statistics/hashes for private review.
Generated vendor-derived PTX/cubins and output arrays remain local.

## Accumulation order and packed helper follow-up

The follow-up tests reverse the order of the two FP16 matrix operations for both
K partitions. Reversing the order eliminates differences in the weights-only
test, but combined tensor/weight patterns still differ. This identifies an
order-sensitive case; it does not identify a complete match for the current
kernel's arithmetic. arithmetic-variants-summary.json retains all comparisons.

New packed encode/decode routines in fp8_exact.cuh pass the same exhaustive
scalar/packed conversion checks against CUDA, with zero mismatches. Packed-helper
and scalar-helper post-block prototypes produce byte-identical outputs on the
three compared patterns. This preserves the prototype's behavior, including its
remaining differences from the deployed kernel. The packed/reversed matrix
mapping also passes all 16,384 exactly representable matrix outputs.

Disassembly of the outlined packed prototype found 684 helper calls. The new
`--inline` generator option expands helper bodies, renames registers/labels and
replaces parameter transfers and returns. Inlined output matches the outlined
prototype on the same tests. The inlined block compiles with 168 registers,
64 stack bytes, 64 spill-store bytes and 72 spill-load bytes. Its nonuniform
small-image run passes memcheck. Removing calls did not establish a speedup:

| Uniform synthetic 1440p benchmark | Default-policy median per launch |
|---|---:|
| Current compiled block | 5.210889 ms |
| Inlined packed prototype, reversed interleaved order | 7.231780 ms |

Each run contains rotating cache-policy trials with ten launches per measurement.
The table uses the nine default-policy measurements after discarding the first
three warm-up measurements overall. Runs were separate, not an in-game A/B test.
The candidate is slower and retains output mismatches, so it remains rejected.

Reproduce the packed/reversed helpers with
`build_post_helpers.cmd -DNR_MMA_REVERSE=1 -DNR_FAST_FP8=1`, then
`python build_post_prototype.py --inline`. Add `-DNR_MMA_CONTIGUOUS=1` to the
helper build for the alternative partition. Omitting the flags retains the
original scalar/default-order prototype. build_mma_mapping.cmd accepts the same
defines. Build metadata records the generated helper hash and inline setting.
The fused quantizer remains a separate validated building block; it has not been
integrated into a compatible full model. The installed app and games are unchanged.

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

## Post-block candidate: full-model validation, 2026-09-06

A new isolated candidate matches the original post-block's observed FP16 MMA
word grouping and uses direct mask/shift/half2 expansion for its MMA operands.
At a 192-register limit, two standalone 1440p runs measured about 4.03 and
4.04 ms, versus 5.21 and 5.15 ms for the original: about 22% less time for this
one block. Small varied-input checks and a varied 1440p check were bit-exact.
Compute Sanitizer reported zero errors at both resolutions.

The optional trace-probe replacement loads a separate candidate module and
substitutes only this function in copied launch descriptors. The original
model, its other kernels, the installed app and game files are unchanged.
The probe retained the original function handle for model-owned cleanup and
released its candidate after completed GPU work. A requested candidate that
fails to load or is never used fails explicitly.

The full model with its actual weights completed baseline/candidate runs at
256x256 (4 evaluations) and 2560x1440 (10 evaluations, repeated pair). Saved
final RGBA16F outputs were byte-identical. Each 1440p comparison covered
14,745,600 half values. This checks the saved final frame; intermediate frames
were checked for finite output, not saved for byte comparisons.

1440p warm medians (first frame excluded; all other samples retained):

| Run | Original block | Candidate block | Original chain total | Candidate chain total |
| --- | ---: | ---: | ---: | ---: |
| First pair | 5.254 ms | 4.584 ms | 39.998 ms | 40.234 ms |
| Repeat pair | 6.018 ms | 4.598 ms | 40.899 ms | 39.504 ms |

The per-block gain survives full-model integration. Whole-model timings have
large outliers and do not yet establish a reliable overall gain or game FPS.
Inputs are synthetic D3D12 textures, not a captured game sequence. The raw MMA
expansion intentionally matches this inspected binary (including +/-480 for
FP8 NaN byte encodings); it is not a general-purpose FP8 decoder. The ordinary
FP8 helper retains its separate canonical-NaN behavior.

Rebuild the candidate with locally available, inspected vendor PTX:

```
build_post_helpers.cmd -DNR_MMA_WORD_PARTITION=1 -DNR_MMA_STRIDED=1 -DNR_FAST_FP8=1 -DNR_MMA_RAW_DECODE=1
python build_post_prototype.py --inline --registers=192
python prepare_trace.py
build_trace.cmd
```

Run trace_probe.exe with core/model/forwarder paths, width, height and frame
count, in a fresh output directory. Set process environment variable
NRSTUDIO_TEST_CUBIN to the absolute candidate cubin path only for candidate
runs; clear it for baseline runs. candidate-status.txt must show the expected
substitution count and completed=1. compare_full_post.py compares the named
full-post-*-1440 directories and preserves all timing samples in JSON.
Vendor PTX, cubins and output textures remain local and are not redistributed.
Game integration and an actual game benchmark remain pending.
