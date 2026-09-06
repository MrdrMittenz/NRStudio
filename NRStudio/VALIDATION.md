NR Studio 1.0 validation — 2026-09-06

- Release app, test harness and standalone installer compiled with Roslyn for Windows x64.
- 17 deployment assertions passed in an isolated temporary game folder. Covered original configuration preservation, native model hash, duplicate installation rejection, unrelated INI keys, external DLL conflict protection, byte-for-byte restoration, settings archive, interrupted deployment recovery, invalid journal paths, unknown proxy rejection, duplicate INI keys and Insert menu defaults.
- Fixture: `C:\Users\ReviOSGaming\AppData\Local\Temp\NRStudio-tests-d3cc9ffeee444935b207af158933300f`.
- Per-user installation exited 0. Uninstall removed application executable, uninstaller, Start menu shortcut and uninstall registry entry. User model and library remained.
- Reinstallation exited 0. Eight installed package files matched SHA256.json with zero mismatches.
- App launched successfully; existing S.T.A.L.K.E.R. 2 entry and model were loaded. High-DPI layout visually inspected after correcting clipping. Settings list scrolls; all seven action buttons are visible.
- No new game compatibility or FPS measurement was performed. The runtime is the pinned build previously tested in S.T.A.L.K.E.R. 2. This packaging task did not deploy runtime files or save settings into the real game.
- NVIDIA model is imported from a local user-supplied copy, not included in the installer. Corresponding runtime source is provided as a separate archive beside the installer.
