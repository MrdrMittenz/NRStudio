# Experimental post-block runtime integration

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
Game performance validation is pending the user's loaded gameplay scene.
