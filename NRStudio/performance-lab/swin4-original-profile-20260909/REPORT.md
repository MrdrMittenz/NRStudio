# SWIN4 profiling result ? 2026-09-09

The game was confirmed closed before GPU work. All work is private; installed NR/FG files and settings were unchanged.

Original cc_tinlayout_fused_swin_4h_128_4_chained_fp8 from SM86 module 2. Captured launch: grid 41x24x1, block 32x4x1, parameter block 88 bytes, height 184, width 320, offsets -4/-4. Original uses 168 registers/thread, 56 stack bytes and 8192 shared bytes.

Validation: four final signed random seeds passed all allocation guards and completion flags, produced four different hashes and varied output bytes. Final Compute Sanitizer memcheck: zero errors. Earlier positive inputs saturated and smaller signed inputs underflowed; these were rejected as candidate-quality inputs. Final magnitude bytes span 16..48 in steps of 8 with random sign. Raw full-output hashes include unchanged initialized margins; output distributions demonstrate variation within the written region. Synthetic inputs are not captured model activations, and this is not whole-model quality validation.

Final Nsight Compute profile (swin4-final.ncu-rep, 45 replay passes, one dispatch): duration 348.54 us; DRAM throughput 6.60%; compute throughput 53.06%; issue slots busy 43.98%; achieved occupancy 23.86%; eligible warps/scheduler 0.72; 110208 local spilling requests. Spills account for 5.85% of L1 sectors and also reach L2. These support investigating instruction scheduling/register pressure rather than treating this as a DRAM-bandwidth bottleneck. Profiler estimated speedups are not predictions of achievable game gains. Historical model traces show eight chained SWIN4 calls/frame; do not multiply this synthetic dispatch timing into a measured current game cost.

The extracted module-2 fatbinary contains cubins, with no embedded PTX emitted by cuobjdump. Consequently changing a ptxas register cap is not directly available for this stage. Next implementation work must recover/reconstruct the kernel's instruction logic or locate an equivalent source representation, then validate original-versus-candidate outputs before cumulative model benchmarking. No optimization candidate has passed or been deployed, and no new FPS gain is claimed.

Artifacts: final-seed-results.json, final-seed-1..4.txt, final-memcheck.txt, swin4-final.ncu-rep, final-details.txt, original.sass, resources.txt, manifest.json and swin4_bench.cpp.
