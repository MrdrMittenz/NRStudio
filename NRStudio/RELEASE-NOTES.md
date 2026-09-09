# NR Studio 1.3.4 Experimental

RTX 3090 preprocessing now uses compact exact quantization. Existing exact
Swin8, exact post, prepared post and completion-aware resource handling remain
active. Default-mode validation matched all 56 output frames; another 12
alternating frames matched with timing enabled. Previous candidate sanitizer
checks passed. Ada kernels and behavior are unchanged and remain hardware-untested.

Five monitored candidate runs reduced whole-model GPU time by 0.70–1.00%.
After correcting comparison logging, three release-build checks showed a smaller,
variable 0.04–1.21% reduction. These are isolated model tests, not measured game
FPS improvements. Live STALKER comparison is pending.

On RTX 3090, the comparison event selects previous versus new preprocessing;
prepared post and exact Swin8 remain enabled in both modes. Unsupported launch
layouts retain their original dispatch. Runtime update retains managed rollback
files and game settings. Detailed GPU timing remains opt-in.

## NR Studio 1.3.3 Experimental

Prepared post weights now use a bounded GPU buffer pool with completion fences.
The exact post/Swin8 improvements remain the fallback and comparison reference.
Two interleaved isolated runs reduced model time by roughly 0.6–0.9%; this is not
a measured game FPS uplift. All tested frame comparisons matched.

Optional completion-aware telemetry separates encode, guide preparation, model
and resolve. It is disabled by default; nr-gpu-timing.enable enables local CSV
capture on the next launch. Slots with uncertain ownership fall back or remain
quarantined; the runtime does not wait on the CPU for each frame. Prepared
resources remain bounded and retained for the process lifetime. Ada kernels
are unchanged. Live STALKER pipeline validation remains pending.

## NR Studio 1.3.2 Experimental

RTX 3090 cumulative update: retain direct activation reuse and add exact packed
conversion to both post and Swin8. Separate validation matched every tested frame.
The post experiment reduced its stage time by about 10%; the incremental Swin8
experiment reduced its stage time by about 1.7%. These are isolated measurements,
not additive FPS percentages or a new in-game performance guarantee. The previous
packaged pair remains embedded for comparison. Ada binaries are unchanged.

## Previous changes

RTX 40-series support: the optimized NR runtime now identifies the game's Ada
GPU through its adapter LUID and selects native sm_89 post/swin8 kernels. RTX
3090 continues to use the identical sm_86 kernel payloads. Unknown architectures
retain the base model path; a failed replacement-kernel load also falls back.
Ada code is compiled and GPU selection is tested, but no RTX 40-series hardware
was available for numerical, gameplay or performance validation. No FPS uplift
over RTX 3090 is claimed. Compare the same scene, resolution and NR settings;
lower-tier and laptop 40-series cards are not guaranteed to beat a 3090.

Dead Island 2 feeder fix (2026-09-08): NR now obtains its graphics device from
the submitted command list for both feature creation and composition. ReShade
can return a wrapped device from a resource while the feeder submits a native
command list; mixing their descriptor heaps caused a first-frame access violation.
When the standard feeder and RenoDX NR add-ons coexist, NR Studio disables the
duplicate RenoDX NR add-on in ReShade settings, preserving both add-on files and
recording the original settings for Restore. ReShade and the feeder stay enabled.
Live validation recorded successful native NR evaluations in Dead Island 2 at
3840x2160. This is activation evidence, not a visual-quality or long-session claim.

Local compatibility fix (2026-09-08): installation recognizes a ReShade x64
dxgi.dll, preserves it as ReShade64.dll and enables OptiScaler's ReShade loader.
Its add-ons, configuration and swapped nvngx_dlss.dll are preserved. Restore
recovers the original DLL layout; conflicting ReShade64.dll files and unknown
proxy DLLs remain protected. This enables installation alongside ReShade; it
does not establish DLSS or NR compatibility in games without an upscaler input.

This release adds matched-residual composition to the desktop app and Insert menu. It
keeps the original image and enlarges the model's signed edit when the model runs at a
smaller working size. Tone and detail controls adjust broad and finer spatial changes.
The revised HDR resizing filter averages represented light before encoding it again,
avoiding an unnecessary brightness drop for thin lights in reduced model inputs.

Current composition, working scale 1.0 and after-upscaling placement remain the defaults.
Existing game profiles and original backups are retained during runtime updates. The
desktop Help explains the new controls and the missing-DLSS-input status.

The release bundles the tested NVIDIA 616.64 driver installer and the Visual C++ x64
redistributable. Setup offers prerequisites through their vendor installers; quiet setup
only installs app files. An upgrade removes obsolete package files only if they still
match their old manifest hashes. It preserves modified obsolete files.

Validation of the unchanged native runtime includes 71 GPU shader checks on the RTX 3090,
with the D3D12 debug layer unavailable. One clean STALKER room capture at residual scale
0.67 measured 32.23 presentation FPS. Both accompanying full-resolution captures detected
input and remain provisional. There is no controlled relative-speedup claim for that run.
Earlier 28.32 FPS measurements were from another build/run and are not evidence of this
filter's speedup. The revised resize pass alone measured approximately 0.11-0.12 ms.

Reduced scale still changes texture, local contrast and lighting. Faces, camera movement
and other scenes require further validation. No universal 30+ FPS or zero-visual-loss
claim is made. Temporal residual reuse, dirty-tile execution, automatic GPU budgets and a
trained Student rendering mode are not included.

The bundled model and forwarder are unchanged. Model provenance/signature limitations
remain documented in MODEL-AUDIT.md. This is an independent experimental integration;
loading the menu alone does not establish native NR evaluation or official DLSS 5 parity.
