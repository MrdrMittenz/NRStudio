NR STUDIO 1.1 — COMPLETE EDITION

Standalone Windows app and native in-game NR Studio control panel.
Tested runtime baseline: RTX 3090, NVIDIA driver 616.56, S.T.A.L.K.E.R. 2.
New games need individual validation; performance is not guaranteed across games.

ONE INSTALLER
Run NRStudio-Setup-1.1.0-Complete.exe. No Swapper installation, manual model
import or CUDA toolkit is needed. The package contains:
 - NR Studio desktop manager and small independent uninstaller
 - Native NVIDIA NR model (validated SHA-256 8270b350...cc206)
 - Distinct native forwarder and modified OptiScaler runtime
 - NR Studio's own Insert overlay with live sliders and comparison tools
 - Original signed Visual C++ x64 redistributable installer
 - Original signed NVIDIA 616.56 driver installer, including its vendor UI

Target OS: Windows 10 version 2004 or later / Windows 11 x64 with .NET Framework
4.8. NVIDIA driver installation may require administrator approval and a restart.
Setup checks prerequisites and offers the bundled driver when the detected version
is older than the tested version or is unavailable. It does not silently replace
a newer driver. Visual C++ setup runs when the installed runtime cannot be confirmed.
--quiet installs app files only; administrators using it must handle prerequisites.

AFTER INSTALLATION
1. Add the game's actual 64-bit rendering executable. For Unreal games it is
   usually Project\Binaries\Win64\Project-Win64-Shipping.exe.
2. Close the game and click Install NR. Backups are made before any replacement.
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

Desktop Save settings applies on the next game launch, with an INI backup.
Existing game settings are preserved by installation. Fresh installations start
with NR enabled, detail 0.75, colour 0.5, full model resolution and Insert enabled.
These are starting values, not universal realism settings. 'auto' uses defaults.

COMPATIBILITY AND RECOVERY
The 7 Days to Die profile selects 7DaysToDie.exe with -force-d3d12, using the
standard client rather than the separate EAC launcher. It is an experimental
profile; NR rendering has not yet been validated in this game. EAC-required
servers are not a supported target. Adding this executable through Add game
automatically selects the same launch profile.

This package targets RTX 3090 owners and 64-bit DirectX 12 native DLSS games.
Adding a game is not proof of support. Games with anti-cheat or restrictions on
injected mods need separate compatibility review; the app bypasses no protections.
GPU / driver setup shows GPU, driver and VRAM and offers the bundled driver if
needed. The known working driver is 616.56; this is not a measured minimum version.

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
NR Studio overlay changes. Corresponding source and pinned dependencies are in
NRStudio-1.1.0-corresponding-source.zip beside the installer. Keep that source
available with copies of the GPL runtime. Vendor software retains its own terms;
the NVIDIA driver license is included under Licenses and its installer presents
the applicable terms. The model was copied from the existing local installation;
this project has not established permission to redistribute that proprietary model
publicly. Creating this complete local package does not establish those rights.

Build: Python 3, Visual Studio C# compiler and the included upstream MSVC sources.
build.py documents source and prerequisite locations. No account, background
service, telemetry or network updater is added by NR Studio.
