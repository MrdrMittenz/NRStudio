# NR Studio — private engineering review

Source snapshot of the local NR Studio 1.1.1 work, prepared 6 September 2026.
This repository contains the desktop manager, installer/uninstaller, native NR
forwarder, and the patch implementing the custom Insert overlay and capture fix.
It includes compiled-code source; it does not contain the proprietary NVIDIA NR
model, driver installers, redistributables, game files, or compiled release files.
The NVIDIA model is a binary dependency and cannot be rebuilt from this repository.
The current model fails Windows signature validation (HashMismatch). See
[the audit](NRStudio/MODEL-AUDIT.md); a matching package hash does not establish
official NVIDIA authenticity.

The runtime is based on [OptiScaler_DLSSNR](https://github.com/Dagherbou/OptiScaler_DLSSNR)
at commit `433cc11d8a6b92dfe4977de4dd88ffe0afbec781`. Our complete tracked changes
are in `patches/optiscaler.patch`. `UPSTREAM.txt` records dependency commits.
NR Studio is independent of NVIDIA and the upstream maintainers.

## Build the review code

Use Windows x64, Python 3, Git, Visual Studio Build Tools with C++ tools, Windows
SDK, Roslyn C# compiler and .NET Framework 4.8 development support. The local
OptiScaler build used MSVC v145; the forwarder used MSVC 14.38 (v143).

1. Clone this private repository. Run `python prepare_upstream.py` from its root.
   This obtains the pinned upstream and submodules and applies our patch.
2. In an x64 Visual Studio developer command prompt with v145 available, run:

   ```bat
   cd optiscaler-nr-reference
   msbuild OptiScaler.sln /t:OptiScaler /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v145 /p:PostBuildEventUseInBuild=false /m:4
   cd ..
   ```

   Output: `optiscaler-nr-reference/x64/Release/OptiScaler.dll`, deployed as `dxgi.dll`.
3. Compile the app and tests, supplying your installed Roslyn compiler path:

   ```bat
   python build_review.py --csc "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\Roslyn\csc.exe"
   ```

4. In an x64 developer prompt configured for MSVC 14.38, compile the saved
   forwarder directly (do not regenerate it from an older upstream template):

   ```bat
   cd build
   cl /nologo /EHsc /O2 /LD ..\NativeForwarder\native_forwarder.cpp /Fe:nvngx.dll_dlssnr.dll /link d3d12.lib
   cd ..
   ```

The output `nvngx.dll_dlssnr.dll` is the forwarder. The separately obtained
`nvngx_dlssnr.dll` is the original model. They are different files.
Builds do not install into games. Compiler differences may change binary hashes.

The existing deployment tests require the original release runtime directory,
including the proprietary model and exact pinned DLL hashes. They are not
source-only tests. With that runtime available, run:

```bat
build\Tests.exe "C:\path\to\NRStudio\runtime" unused "build\NRStudio.exe"
```

The second argument is a retained, unused placeholder. Tests create temporary
game fixtures; they do not modify installed games. A source-only checkout can
compile the tests but cannot execute them successfully without those binaries.

`build\DiagnosticsTests.exe` runs seven additional source-only tests covering
fresh versus historical native log evidence and failures. The app's Check
runtime also queries Windows Authenticode separately from file hash matching.

## Installer packaging

`NRStudio/build.py` is the original full packaging recipe, retained for inspection.
It has workstation-specific paths for the compiler, sibling OptiScaler and native
runtime folders, driver, driver EULA, and VC redistributable. Adjust those paths
before using it on another machine. It also produces archives in `dist`.
It requires the separately available vendor binaries; the review build does not.

Alternatively, `build_review.py --csc "..." --payload "path\to\app.zip"`
compiles the same setup source with an existing prepared payload embedded.
That ZIP must contain the app and uninstaller at its root, `runtime` with the
three DLLs and OptiScaler.ini, `prerequisites` with NVIDIA-616.56.exe and
vc_redist.x64.exe, and the source/license/readme/hash files described by the
original packaging recipe. An arbitrary ZIP is not a complete installer payload.

## Review focus and evidence limits

Review game-file backups and restoration in Core.cs, prerequisite checks in
Prerequisites.cs, extraction/uninstall paths in Setup.cs, the overlay lifecycle,
and native NGX resource and parameter handling. Existing validation notes in
NRStudio describe earlier checks; they are not a new validation of every rebuild.

Native NR execution was observed on RTX 3090 with driver 616.56 in STALKER 2.
A measured full-resolution 1440p scene ran about 19 FPS with NR and 69 FPS without;
performance varies. This is not proof of official product parity or universal
DX12 compatibility. The 7 Days to Die log now shows historical native evaluation
success; broader visual and compatibility validation remain incomplete.

GPL license text is in LICENSE; upstream dependencies retain their own licenses.
Private repository access must be granted to the reviewing developer through
GitHub's collaborator settings before they can open the repository link.
