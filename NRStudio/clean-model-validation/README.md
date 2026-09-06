# Clean NBA model validation — 6 September 2026

## Completed after restart

NVIDIA 616.64 installed successfully (installer exit 0). Both Windows driver
inventory (32.0.16.1664, oem86.inf) and nvidia-smi report the updated driver.

The clean NVIDIA-signed model still fails feature creation with `bad00001`.
Read-only extraction of the seven registered CUDA buffers followed by NVIDIA
cuobjdump inspection reports SM120 ELF and PTX in every module. Direct CUDA
module-load tests on the RTX 3090 return `CUDA_ERROR_NO_BINARY_FOR_GPU` (209)
for all seven. This clean build has no loadable GPU code for this machine;
changing DLL names, signatures or UI labels cannot fix that architecture gap.

The existing modified 8270b350 model passed a regression test on 616.64 with
the existing forwarder: successful creation, GPU initialization completion,
three successful 2560x1440 evaluations, zero non-finite outputs, and nonzero
output changes. Warm evaluation plus readback was 38.657 and 38.232 ms; the
first frame was 255.046 ms. This is a short synthetic test, not a game FPS
benchmark or proof of full official feature parity.

The existing model's registered modules were previously inspected as SM86.
The binary comparison shows the entire 147,697,152-byte `.rsrc` section is
identical to the signed NBA model. The 25 code-byte and 16,151,311 data-byte
differences explain why the modified copy does not retain a valid signature,
but their exact semantics and numerical equivalence remain to be audited.

Decision: keep the current working model and forwarder in the app and games.
The supplied signed file is the comparison baseline, not a working drop-in
replacement on RTX 3090. No NVIDIA DLL was changed or re-signed in this audit.
The existing 1.1.1 installer still bundles driver 616.56; the host is on 616.64.

Evidence: probe-61664.txt, cuda-module-validation.json,
clean-module-architectures.json, modified-model-regression-61664.txt,
section-comparison.json and signatures.json. Extracted GPU code and output
textures remain local and are not included in the private source repository.

The earlier steps below record the initial run and resolved restart blocker.

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
