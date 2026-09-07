# Experimental post-block runtime integration

## Current direct-activation update, 2026-09-07

The current forwarder embeds two replacements: post (224-register limit) and
swin8 (240-register limit). Both retain FP8 rounding and the accepted Ampere
FP16 matrix operation order while avoiding selected register-only pack/unpack
round trips. Model weights, NR working resolution, evaluation frequency, and
game settings are unchanged. Prepared-weight experiments remain isolated and
are not embedded. The pre and swin1 candidates were slower and are excluded.

Build the helpers with the accepted flags documented in ../kernel-lab, then
run build_post_prototype.py --inline --registers=224 --direct-activations for
post, and --kernel=swin8 --inline --registers=240 --direct-activations for swin8.
Copy post-direct.cubin as post.cubin, swin8-direct.cubin as swin8.cubin, and
retain the previously validated post-word-raw.cubin as previous-post.cubin.
candidate.rc embeds those three local artifacts. Run build.cmd.
These generated proprietary-derived artifacts remain outside source control.

The same model hash, RTX 3090 PCI ID, enable marker and hook lifecycle checks
apply. Only singleton launches with validated parameter sizes, block shapes
and zero dynamic shared memory are replaced. The extra target uses 88 parameter
bytes and a 32x8x1 block; post uses 184 bytes and 32x1x1.

The existing PostOriginal event now selects the **previous validated runtime**:
the old optimized post plus the model's original swin8. The historical event
name and original/optimized log labels are retained for benchmark tooling.
Removing the enable marker and restarting selects the unmodified model kernels.

Validation: direct-validation/result.json compares every output frame against
the previous runtime: 60 frames at 2560x1440, 12 at 1920x1080, and 12 at
3072x1728. All SHA-256 values match, GPU fences complete, finite readbacks pass,
both candidate functions are released, and the 25 exported names/ordinals
match. The disabled-marker case and live benchmark switch also pass.
Inputs are synthetic; this does not establish equivalence in every game scene.

Sustained isolated model tests measured approximately 4-5% lower summed GPU
kernel time for the selected pair. This is not an in-game FPS measurement.
See ../kernel-lab/direct-activation-results.json. The same-scene STALKER test
measured 19.437 FPS previous versus 19.926 FPS new (+0.489 FPS, +2.51%). All
24 logged evaluations succeeded at unchanged model/guide sizes. All trials
held focus; the first two recorded input-timestamp changes. The last two were
input-idle and showed a similar +2.55% difference. This remains one scene and
two trials per mode, with presentation FPS rather than independently classified
rendered/generated frames. See game-benchmark/direct01-REPORT.md. New mode was
restored and acknowledged with subsequent candidate launches.
The local STALKER forwarder and its installation journal
were updated; the installed app's runtime and distributable installer have
not yet been updated.

rollback_direct.ps1 restores the previous validated runtime and its managed
installation hash. rollback.ps1 restores the original pre-experiment forwarder.
Both require the game to be closed and verify the deployed file before writing.

## Previous build history

This opt-in forwarder embeds the locally built post-block cubin validated in
../kernel-lab. It replaces only the inspected singleton 184-byte, 32-thread
post-block launch. All original model functions still exist. Unknown launch
shapes are forwarded unchanged. Candidate load failures retain the original.

Enable by placing nr-post-opt.enable beside nvngx.dll_dlssnr.dll before launch.
Remove that marker and restart the game to disable. Adapter PCI ID 10DE:2204
(RTX 3090) and the inspected model SHA-256 8270b350... are checked before hook
installation. Hooks are installed outside DllMain, with existing process threads
enlisted in the Detours transaction; the forwarder is pinned to prevent stale
hook code pointers. Candidate handles are released when the corresponding
original function is destroyed. The observed full-model lifecycle passes.

Build requirements: MSVC 14.38, Windows SDK resource compiler, DXGI/D3D12/BCrypt,
and the NVAPI headers and Detours library from the inspected OptiScaler checkout.
The source assumes the local workspace layout; adjust absolute build paths on
another machine. prepare.py regenerates native_forwarder.cpp from the saved
baseline with the two integration points. build.cmd compiles the forwarder.
Before building, generate the validated cubin using ../kernel-lab/README.md and
copy post-prototype.cubin here as post.cubin. Proprietary PTX and cubin data are
not included in the source review. This is not a self-contained public build.

Validation, 2026-09-06:
- All 25 export names and ordinals match the installed baseline.
- Initial integrated enabled/disabled 12-frame 1440p tests have byte-identical
  final outputs, SHA-256 7bced19533a39e49991de283162e1ef1dea83eb0a6af43abb22ad3e3d33cd7df.
- Final build adds thread enlistment and completes 120 consecutive 1440p
  evaluations with success results and finite readbacks. Runtime logs record
  120 candidate launches and completed candidate destruction.
- The independent kernel sanitizer checks and full-model comparison are in
  ../kernel-lab. They do not establish official NVIDIA support or game FPS.

Deployment is limited to the local STALKER 2 forwarder and enable marker.
The model DLL, OptiScaler settings, installed app runtime and installer remain
at their prior versions. deployment.json records hashes; game-backup holds the
previous forwarder. Run rollback.ps1 with the game closed to restore it.
Gameplay validation: see game-benchmark/REPORT.md. The optimized kernel runs
in-game; two samples measured about 19.3 FPS. Input activity prevents a
controlled comparison, and no game FPS speedup is claimed.

## Live benchmark switch

The updated forwarder creates a manual-reset, process-specific Windows event:
Local\NRStudio.PostOriginal.<PID>. Setting it selects the original post kernel;
resetting it selects the optimized kernel. NR remains fully enabled. The switch
is checked only for the matched post-block launch and does not touch other
kernels, graphics settings, game input, or model weights. The default is optimized.
The small event handle lives with the pinned forwarder until process exit.

Use `python benchmark_switch.py PID original` or `optimized`; runtime logs must
acknowledge the selected mode. `python test_switch.py` validated both modes
inside one 12-frame 1440p model run with byte-identical final output relative to
the original reference. The locally embedded cubin is unchanged.

From game-benchmark, `python run_comparison.py PID --prefix UNIQUE` runs four
30-second captures in original/optimized/optimized/original order, with five
seconds settling per trial. It checks mode acknowledgements, records focus and
input activity, preserves invalid samples, and restores optimized mode in a
finally block. The user must first load gameplay and keep the game in focus.
This switch provides measurement capability; it is not an additional speedup.
