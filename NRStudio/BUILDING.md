# Build NR Studio 1.3.4

Extract NRStudio-source.zip into a working directory. The archive includes the matching
C# sources, packaging script, license assets, native adapter patch, experimental shaders
and forwarder resources. It does not contain NVIDIA's proprietary model source or driver
source. Those components are pinned binary inputs from the complete local package.

## App and installer

Use Windows x64, Python 3.11 or later, .NET Framework 4.8 reference assemblies and the
Visual Studio C# compiler. The tested compiler is in Visual Studio 18 Build Tools. Run:

```powershell
python build.py --output-dir D:\NRStudio-build
```

By default this reads runtime DLLs and vendor installers from the installed NR Studio
folder under `%LOCALAPPDATA%\Programs\NRStudio`. Install version 1.3.4 first or supply
extracted package inputs explicitly:

```powershell
python build.py --output-dir D:\NRStudio-build --runtime-dir C:\Inputs\runtime --vendor-dir C:\Inputs\prerequisites --csc "C:\Path\To\Roslyn\csc.exe"
```

`--driver` can override the NVIDIA installer input path. The build verifies all pinned
runtime/vendor hashes, runs installation/profile and diagnostic/monitor checks, embeds
the payload, then verifies every embedded file against SHA256.json. Reserve approximately
8 GB of scratch disk space for payload copies and retained test fixtures. It builds the
app and installer; it does not install them or change games. Unsigned compiler outputs
can differ in PE timestamps, so rebuilding is not a claim of byte-identical EXEs.

## Native adapter and shaders

RTX 40-series kernels: in RuntimeSource/Forwarder, run build_ada_kernels.cmd
with CUDA 13.3 ptxas before build.cmd. It compiles the retained PTX to sm_89,
with register limits 224 (post) and 240 (swin8). candidate.rc embeds these as
201/202 alongside unchanged sm_86 resources 101/102 and control resource 103.
test_gpu_profile.cmd tests adapter selection, multi-GPU LUID matching and live
NVAPI detection. Execution/quality/performance tests on Ada remain required.

RuntimeSource/UPSTREAM.txt records the native repository URL and exact base commit.
Check out that commit with its submodules, apply RuntimeSource/optiscaler.patch with
`git apply`, then copy RuntimeSource/overlay/ over the checkout, preserving relative paths.
The overlay includes files that were untracked in the original native worktree.

Compile `OptiScaler/shaders/dlssnr/precompile/DlssNrResidual.hlsl` using the Windows SDK
FXC compiler with `/T cs_5_0 /E CSMain /O3`. Generate its header using the upstream
`OptiScaler/shaders/shader_tools/create_header.py` and symbol `DlssNrResidual_cso`.
The accepted Current shader is pinned separately and should not be regenerated as part
of the residual shader build. Build OptiScaler.sln, target OptiScaler, Release/x64,
PlatformToolset=v145, with PostBuildEventUseInBuild=false. Use a dedicated IntDir if needed.

RuntimeSource/Forwarder contains the matching C++ adapter, resource script and its three
compiled kernel resources. Use rc.exe on candidate.rc, then compile native_forwarder.cpp
as a C++17 DLL with UNICODE/_UNICODE, the upstream NVAPI and Detours include directories,
and d3d12.lib, dxgi.lib, bcrypt.lib plus upstream detours.lib. The included build.cmd
records the original local tool paths; adjust them to your checkout/toolchain.
These resources and the NVIDIA model are not independently recreated by the C# build.
RuntimeSource/Kernels retains available PTX and build metadata for internal review;
proprietary-derived model/kernel resources remain subject to their existing terms.

Changing native code requires validating the resulting DLL and deliberately updating its
pins in src/Core.cs and build_release.py. The release build rejects a mismatched runtime.
The private review repository is https://github.com/MrdrMittenz/NRStudio and requires
access; the bundled source snapshot is the reference for this release and requires no
repository account to inspect the app/adapter changes. It has not been published remotely
by the packaging operation.
