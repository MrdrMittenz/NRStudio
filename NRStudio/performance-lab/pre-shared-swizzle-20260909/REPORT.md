# Preprocessing shared-memory swizzle experiment

Selected as an untried targeted experiment because prior preprocessing profiling reported two-way shared-load bank conflicts and the current source preserves that addressing. This choice is a reasoned priority, not a quantified probability of success.

Changed only shared-memory byte addressing: offset -> offset XOR ((offset >> 1) AND 64). Exhaustively verified bijection over 2048 bytes and contiguity/alignment of every 16-byte vector. Rewrote both vector stores and all 16 scalar loads (18 sites). The recovered lane pattern maps to 32 distinct banks instead of 16 by algebra; no hardware-counter confirmation was collected for this candidate. No arithmetic, synchronization, model weights, image resolution or evaluation cadence changes.

Candidate compiled at 224 registers, 24 spill-load/store bytes and 2048 shared bytes. Private forwarder retains all installed cumulative optimizations and swaps only preprocessing against a copy of the validated 224-register control. Four standalone input seeds matched exactly with guards passing. Twelve alternating actual-model output hashes matched the validated baseline, with valid pipeline timestamps. No claim of exhaustive input equivalence.

Three 200-frame alternating timing runs, first 32 frames excluded (see runtime_test.py), all produced higher candidate median model time. See decision.json for exact results. Absolute timing varied between runs, so only within-run comparisons are used. The mathematically improved bank mapping did not produce a measured full-model benefit; added addressing work and compiler changes are possible causes, not proven diagnoses.

Decision: reject this candidate. No game/runtime/package files modified. Existing performance gains remain installed. All candidate code, hashes, logs and measurements retained for audit. No new gameplay FPS or latency gain claimed.
