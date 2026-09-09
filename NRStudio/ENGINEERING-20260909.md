# Engineering update ? 9 September 2026

Current installed version: 1.3.4 Experimental. Exact preprocessing, prepared post weights and prior SWIN8 improvements are cumulative. This source update does not deploy a new runtime or publish an installer.

The preprocessing release subsequently showed small positive gameplay differences in accepted stationary blocks: 0.569%/0.805% in one session and 0.183%/0.280% in another. These are scene-specific results, not a guaranteed or additive FPS claim. See performance-lab/autonomous-20260909 reports for exclusions and limits.

An external, separately installed dlssg_for_sm86 proxy was tested on RTX 3090 in STALKER 2 with NR enabled. A clean stationary off/on pair observed 21.14 to 39.76 displayed FPS, and instrumented frame-start-to-display latency 57.1 to 59.1 ms. This is not hardware click-to-photon latency or proof of Reflex sleep correctness. Generated frame classification was unavailable. User reported acceptable motion/HUD appearance; no objective motion-quality equivalence claim. It is not integrated into NRStudio and its DLL/assets are not included here. Upstream: https://github.com/sdli1995/dlssg_for_sm86 (tested commit a4760d4a49d6c791bba88f24378a56c0dd1c57b0).

SWIN4 original-kernel profiling completed with guarded, varied synthetic inputs and zero memcheck errors. It suggests instruction/register pressure; the module has no embedded PTX. Synthetic timing is not game timing.

Rejected work: additional preprocessing register limits, SWIN1 reconstruction/cache/scheduling variants, and preprocessing shared-memory swizzle. The latest swizzle matched tested outputs exactly but increased full-model median time by 0.44%, 0.59%, 0.04% across three runs. It was not deployed.

Next queued experiment: compiler scheduling on the current preprocessing implementation. It has not been run; no result is claimed. All retained experiment documents are evidence snapshots, not release build inputs. Vendor binaries, raw output buffers, screenshots and game settings are excluded.

The current app source and NativeForwarder snapshot supersede older copies under post-runtime and kernel-lab, which remain historical experiment artifacts. Full release packaging requires the additional local package assets described in BUILDING.md; build_review.py provides the source-only app/test build.

Source update validation: app and test targets compiled; 35 installation/runtime tests, 7 diagnostics tests, 12 GPU-monitor tests and feeder checks passed using temporary fixtures. Updated patch applies to the pinned upstream base. Native runtime was not rebuilt for this source publication.
