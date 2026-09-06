# NR performance investigation — RTX 3090, driver 616.64

The working app, game DLLs and settings were not changed. The candidate below
was evaluated in a standalone D3D12 harness and **not deployed** because it
did not demonstrate a reliable speedup.

## Current bottleneck evidence

- Prior full-resolution game sample: 18.93 FPS / 52.84 ms average presentation
  interval, modified NR model at 2560x1440; one input event means the sample
  was not a fully controlled stationary benchmark.
- Isolated native model test on this driver: 38.657 and 38.232 ms for the two
  warm evaluations including GPU completion and output readback. This is not
  a per-kernel profile or an exact measure of model-only GPU time.
- Nsight Compute basic collection against the isolated D3D12 probe reported
  **No kernels were profiled**. It cannot establish a CUDA-kernel bottleneck
  for this execution path. No hardware counters or driver settings were changed.
- The model already contains compiled SM86 GPU code. Do not describe this as
  recompiling/translating the entire model on every frame, or assume the cost
  of FP8 conversion without measuring it.

## Tested shader simplification

The composition shader calls HueOkLab(model * ratio, model). Because the ratio
is nonnegative, ideal OkLab scaling preserves chroma direction; the candidate
replaces that hue correction with ClampAp1(model * ratio). Floating-point
round trips differ, so numerical comparison is required.

The harness compares the deployed shader bytecode with the candidate at 1440p
over eight SDR/HDR, brightness, strength and comparison configurations, using
random colours, neutrals, zeros and saturated red. It checks 117,964,800 scalar
output values per format, then times six alternating-order pairs of 100 GPU
dispatches with timestamps and UAV ordering barriers. This is an isolated
composition-shader benchmark, not an end-to-end FPS test.

- FP32 maximum absolute difference: 0.0000476837; RMS: 0.000001989.
- FP16 maximum absolute difference: 0.0078125 at HDR magnitudes; maximum error
  scaled by max(1,abs(reference)): 0.00097371. This is not bit-identical output.
- Both formats: zero non-finite candidate values and zero alpha differences.
- FP16 baseline composition cost is approximately 0.14 ms. Candidate timings
  overlap and are sometimes worse. See shader-summary.json and raw logs.

Even eliminating a 0.14 ms pass would save only about 0.3% of the observed
52.84 ms frame. This candidate is therefore rejected for the working runtime.

## Changes that could materially help

1. Obtain the source/build procedure for the modified SM86 kernels and a GPU
   trace capable of observing this D3D12 execution path. Optimize measured
   hot kernels (conversion, matrix operations, memory traffic or synchronization)
   with numerical and temporal validation. No particular kernel is proven hot
   by the current data, and no speedup is promised.
2. Process fewer model pixels. Existing WorkingScale 0.75 means 56.25% as many
   model pixels while retaining full-size game output. It changes the detail
   the NR model sees; it is a quality/performance tradeoff, not a quality-neutral
   code optimization. It has not been selected or benchmarked in this task.
3. Investigate a pre-upscale NR integration as a separate experimental path.
   Correct colour, motion, temporal history and upscaler order would need
   validation. It is not equivalent to optimizing full-resolution inference.

Reproduction requires the local original shader header, candidate.hlsl, Windows
SDK fxc, x64 Visual Studio C++ tools and D3D12. prepare_shader_test.py builds the
candidate and extracts the baseline bytecode; build_shader_test.cmd compiles
the harness. Run shader_bench.exe baseline.cso candidate.cso with optional
fourth argument `half` for FP16 textures. Nothing in these scripts deploys DLLs.
