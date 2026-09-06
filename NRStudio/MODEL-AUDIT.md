# NR model audit — 6 September 2026

The bundled file is an experimental third-party-sourced NVIDIA NR runtime.
Native evaluation has been observed, but authenticity is not established.
Do not describe this as a verified full official DLSS 5 implementation.

## Observed identity

- Source: the existing local DLSS 5 Swapper installation; its update metadata
  names `rakanki911/DLSS5-Swapper`. This identifies the local copy source,
  not the original publisher's distribution chain.
- Filename: `nvngx_dlssnr.dll`; size: 165,840,496 bytes.
- File version: 310.8.0.0; description: NVIDIA DLSSNR - DVS PRODUCTION.
- SHA-256: `8270b350cd82de5ce89806872cdd6b6a9249b80836b91bbeb3573470744cc206`.
- Windows Authenticode: **HashMismatch**, reproduced against both the Swapper
  source and installed NR Studio model. The embedded signer names NVIDIA
  Corporation, but that certificate does not authenticate this altered digest.

Metadata and the pinned hash identify our tested bytes, not an unchanged
official release. The cause and extent of the mismatch are not established.
No signature bypass or attempt to re-sign the model was made in this audit.

## Execution evidence

Existing logs contain successful native evaluate calls (`result=00000001`):

- STALKER 2: PID 18284, frame 49680, model 2560x1440, guides 1972x1108.
- 7 Days to Die: PID 14568, frame 9120, model 3072x1728, guides 3199x1799.

These are historical observations, not confirmation of a currently running
game or equivalence to an official integration. The prior STALKER benchmark
measured approximately 19 FPS with full-resolution NR and 69 FPS without.

## References and unresolved comparison

[NVIDIA's research overview](https://research.nvidia.com/labs/adlr/DLSS5/)
describes DLSS 5 conditioning on rendered frames, engine motion, temporal
state and artistic controls. This does not authenticate our binary.
The [NVIDIA DLSS SDK repository](https://github.com/NVIDIA/DLSS) tree checked
in this audit did not provide an NR DLL for direct comparison.

A [signature repair utility's author](https://github.com/kayle2203/dlssnr-signature-repair)
documents a different 310.8.0.0 hash,
`e16bcf15e16e13f527491cdf7845b2fe6521a738d8f7c9c721866a8496e1fc8e`,
and requires a valid NVIDIA signature from an existing game. This is a
comparison lead, not an independently verified replacement available here.

Next: obtain a clean model from an official installed game or NVIDIA package,
verify its signature, compare it with the current DLL, and test it in an
isolated native probe before changing the pinned model or deploying it.
No replacement was located in the checked Steam common directory, Downloads,
local program installations, or project workspace.

## Diagnostic fix

Check runtime distinguishes package hashes, signature validity and execution.
Recent evidence requires the exact running game executable, a matching PID,
the latest complete evaluate entry, a log written in that process session,
and both log write time and evaluation tick within ten seconds. Otherwise
evidence is labelled historical. This is not continuous GPU monitoring.

Seven tests cover recency, PID mismatch, a closed game, reboot tick mismatch,
latest-call failure and creation-only logs. Rendering DLLs and game settings
remain unchanged while the diagnostics are corrected.
