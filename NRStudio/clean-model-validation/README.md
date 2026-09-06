# Clean NBA model validation — 6 September 2026

User supplied `Desktop/SL 2.13/SL 2.13`, described as NBA files and a current
NVIDIA driver. All 12 supplied DLL/EXE files passed Windows Authenticode checks
with NVIDIA Corporation signer subjects.

- Clean NR DLL: 310.8.0.0, 165,840,496 bytes,
  SHA256 `e16bcf15e16e13f527491cdf7845b2fe6521a738d8f7c9c721866a8496e1fc8e`.
- Supplied driver: package setup.cfg identifies 616.64;
  SHA256 `36584e5df1dc048df5c677e9591295b96b870e7a6a0ad1461457b89de8bc1b37`.
- Streamline DLLs report 2.13.0.0; SR and FG DLLs report 310.8.0.0.

The installed model differs from the signed candidate by 16,151,336 bytes:
25 in `.text`, 16,151,311 in `.data`. The `.rsrc` section is identical.
This comparison establishes differences, not the purpose of every difference.

## Probe result on 616.56

Existing native_probe.exe, existing 575cd0b5 forwarder, 256x256, three requested
frames. NGX core initialization, capability retrieval and float slot round-trip
succeeded. Clean model initialization returned 1, feature creation returned
`bad00001`, feature pointer null, process exit 7. No evaluation occurred.
This does not prove the clean model is inherently incompatible with RTX 3090;
the current integration and driver need to be isolated as causes.

## Driver update blocked

The signed 616.64 package was extracted, its setup version/signature verified,
and the active driver was exported to `driver-transition/backup-before-61664`.
NVIDIA profile files were also backed up. A driver-only install was attempted
without requesting a clean install or automatic restart.

Installer exit: -469762016. The log reports `Previous reboot detected` and a
failed `REBOOT` constraint. The system remains on 616.56. Do not bypass the
pending-restart check or describe 616.64 as installed/tested.

## Resume after the user restarts Windows

1. Check the active driver with nvidia-smi.
2. If still 616.56, rerun `driver-transition/Install-Driver61664.ps1`; inspect
   its exit, installation-61664.json, and active driver. Handle any further
   required restart before probing.
3. Rerun the clean-model probe on the actual updated driver. Capture its output.
4. Only if creation/evaluation succeed, broaden to the existing 1440p finite
   output/GPU-completion test, then stage the app hash and game deployments.
5. Keep the signed model unchanged; the forwarder is a separate DLL.

No game DLLs, NR settings, app model pins, or installer model payloads were
changed in this test. The signed candidate remains at the user's supplied path.
