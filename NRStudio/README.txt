NR STUDIO 1.3.4 — EXPERIMENTAL NR

MODEL STATUS (2026-09-06)
The bundled third-party-sourced NR model reports version 310.8.0.0. Windows
reports Authenticode HashMismatch: its embedded NVIDIA signature does not
validate. Matching the package hash confirms the tested file, not authenticity.
Native evaluation was observed in STALKER 2 and 7 Days to Die, but full official
DLSS 5 provenance and feature parity are not established. See MODEL-AUDIT.md.
Check runtime reports signature and recent/historical evaluation separately.

Standalone Windows app and native in-game NR Studio control panel.
Tested runtime baseline: RTX 3090, NVIDIA driver 616.64, S.T.A.L.K.E.R. 2.
RTX 40-series (Ada): dedicated sm_89 kernels selected for the game's GPU.
Ada hardware validation is pending. A performance advantage over the RTX 3090
depends on the exact card and workload; no measured Ada FPS claim is available.
New games need individual validation; performance is not guaranteed across games.

ONE INSTALLER
Run NRStudio-Setup-1.3.4-Experimental.exe. No Swapper installation, manual model
import or CUDA toolkit is needed. The package contains:
 - NR Studio desktop manager and small independent uninstaller
 - Native NVIDIA NR model (validated SHA-256 8270b350...cc206)
 - Distinct native forwarder and modified OptiScaler runtime
 - NR Studio's own Insert overlay with live sliders and comparison tools
 - Original signed Visual C++ x64 redistributable installer
 - Original signed NVIDIA 616.64 driver installer, including its vendor UI

Target OS: Windows 10 version 2004 or later / Windows 11 x64 with .NET Framework
4.8. NVIDIA driver installation may require administrator approval and a restart.
Setup checks prerequisites and offers the bundled driver when the detected version
is older than the tested version or is unavailable. It does not silently replace
a newer driver. Visual C++ setup runs when the installed runtime cannot be confirmed.
--quiet installs app files only; administrators using it must handle prerequisites.

AFTER INSTALLATION
1. Add the game's actual 64-bit rendering executable. For Unreal games it is
   usually Project\Binaries\Win64\Project-Win64-Shipping.exe.
2. Close the game and click Install / update NR. Backups are made before any replacement.
3. Launch a compatible DirectX 12 game and select native DLSS in its settings.
4. Press Insert. NR STUDIO opens in-game without the desktop app needing to run.
5. Adjust detail, colour, paper white and highlight guard live. Model resolution,
   intensity and structure sliders commit when released because they rebuild
   model resources. Style/preset changes also rebuild the model.
6. Use Comparison for original/NR side-by-side or a movable wipe. Use Save profile
   to persist current settings. Insert returns control to the game.

The live panel reports whether the pass is running and its GPU cost when timing
is available. This is not a promise of 30 FPS or a displayed-FPS counter.
Advanced OptiScaler opens the underlying integration settings; Back to NR Studio
returns to the dedicated panel. Reopening with Insert starts on NR Studio.

MATCHED RESIDUAL (EXPERIMENTAL)
Composition 0 / Current remains the default. Composition 1 / Matched residual
keeps the original image and enlarges the signed NR edit when Model resolution
is reduced. Residual tone controls broad changes; Residual detail controls the
remaining finer changes. Both at 1 retain the complete edit. Both at 0 return
the original image while the model still runs. These are spatial controls,
not semantic lighting/material masks.

The HDR resize filter averages represented light before encoding it again,
preventing thin bright features from becoming unnecessarily dark in the proxy.
One clean STALKER scene capture reached 32.23 presentation FPS at scale 0.67.
Full-resolution captures in that run detected input activity, so no controlled
relative-speedup claim is made. Visual differences remain; this is not a
zero-loss preset or a universal 30+ FPS guarantee. See RELEASE-NOTES.md.
Current composition, full model resolution and after upscaling remain defaults.

WAITING FOR NATIVE DLSS FRAMES
A visible Insert menu does not mean NR is evaluating. Enable native DLSS in
the game's settings and confirm the panel says Native NR running. STALKER can
retain TSR in a separate profile even when another settings file lists DLSS;
reselect and apply DLSS in-game. DirectX 12 alone does not supply the inputs.

HELP IN THE APP
Click Help to open the built-in guide. Its tabs explain every desktop control,
the Insert menu, model settings, performance tradeoffs, saving and recovery.
Package contents still opens this README for installation and package details.

GPU / VRAM MONITOR
Click GPU / VRAM monitor beside Package contents. It shows live NVIDIA readings
for used, free and total VRAM, GPU load, temperature, power and clocks, with a
40-sample memory graph. The readings cover the whole selected GPU. Free VRAM
is not the game's Windows memory budget. Unavailable counters stay unavailable.
Select a GPU if several are installed. Saved game settings and recent/historical
native evaluation evidence appear below; use Insert for the active NR path and
NR GPU cost. This monitor does not measure game FPS.
Refresh is every 3 seconds while open and not minimized. Pause or close it for
controlled FPS tests. Refresh now takes one sample; Copy snapshot copies the
readings and their timestamp locally. Opening it does not change game settings.

NR PROCESSING ORDER
Select a game in the desktop app and choose NR processing order:
 - After upscaling (full resolution): the default, existing quality path.
 - Before upscaling (experimental): NR runs on the render-resolution image
   before DLSS enlarges it. It can be faster, but fine detail and motion may differ.
Close the game, choose the order, click Save settings, then launch the game.
The choice is saved separately for each game. The Insert menu's NR before
upscaling checkbox controls the same choice and also requires a game restart.
If a game has an older runtime, click Install / update NR first. Updates preserve
its slider values and the original backups. Other games are not updated.

Before upscaling has been exercised in STALKER 2 on this RTX 3090. It measured
25.80 FPS versus 20.03 FPS in one room, with comparison limitations; this does
not establish equal visual quality or predict performance in another game.
Unsupported input layouts use the existing post-upscale route and log the reason.

Desktop Save settings applies on the next game launch, with an INI and placement backup.
Existing game settings are preserved by installation. Fresh installations start
with NR enabled, detail 0.75, colour 0.5, full model resolution and Insert enabled.
These are starting values, not universal realism settings. 'auto' uses defaults.

COMPATIBILITY AND RECOVERY
The 7 Days to Die profile selects 7DaysToDie.exe with -force-d3d12, using the
standard client rather than the separate EAC launcher. It is an experimental
profile; native evaluation is logged, but broader validation remains incomplete. EAC-required
servers are not a supported target. Adding this executable through Add game
automatically selects the same launch profile.

This package targets RTX 3090 owners and 64-bit DirectX 12 native DLSS games.
Adding a game is not proof of support. Games with anti-cheat or restrictions on
injected mods need separate compatibility review; the app bypasses no protections.
GPU / driver setup shows GPU, driver and VRAM and offers the bundled driver if
needed. The known working driver is 616.64; this is not a measured minimum version.

Restore original files returns the exact state before NR Studio installed into
that game. If NR was already there, restoring returns to that previous NR build.
Unexpected DLL changes stop restoration. Current settings and original backups
are preserved in dated .nr-studio-restored folders. Installing/Restoring status
after an interruption can be recovered with Restore original files. An interruption
before installation.json exists changes no game files: archive that incomplete
.nr-studio directory manually before retrying.

Unknown existing dxgi.dll proxies are refused to protect other mods. Resolve the
proxy conflict first. If a protected game folder denies writes, run the manager
as administrator for that operation. No other games are auto-installed.

Uninstall through Windows Installed apps. Game installations, external settings
backups and your library remain. Restore games first to undo their deployments.
The model bundled inside the app is removed along with the app itself.

SOURCE AND VENDOR COMPONENTS
Independent project, not an NVIDIA product. NR Studio source is in
NRStudio-source.zip under GPL-3.0-or-later; COPYING.txt contains that license.
The runtime is based on OptiScaler_DLSSNR commit
433cc11d8a6b92dfe4977de4dd88ffe0afbec781, with the capture correction and
NR Studio overlay changes. Current source, patch and build instructions are in
the private review repository: https://github.com/MrdrMittenz/NRStudio
Repository access is required. Vendor software retains its own terms;
the NVIDIA driver license is included under Licenses and its installer presents
the applicable terms. The model was copied from the existing local installation;
this project has not established permission to redistribute that proprietary model
publicly. Creating this complete local package does not establish those rights.

Build: extract NRStudio-source.zip and follow BUILDING.md. Python 3.11+,
.NET Framework 4.8 and the Visual Studio C# compiler are required. The build
pins local runtime and prerequisite files, runs checks and verifies the final
installer payload. It does not install games or replace the driver. Matching
native adapter patches, shaders and forwarder resources are included for review.
NVIDIA model and driver source are not included. The private repository may
lag this local release; the bundled source snapshot identifies this build.
No account, background service, telemetry or network updater is added by NR Studio.
