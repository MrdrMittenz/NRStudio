"""Compare local synthetic output artifacts; publish statistics, not tensors."""
import hashlib
import json
from pathlib import Path
import numpy as np

root = Path(__file__).resolve().parent
rows = []
for pattern in range(1, 6):
    baseline_path = root / f'post-original-small-{pattern}.f32'
    candidate_path = root / f'post-prototype-interleaved-{pattern}.f32'
    baseline = np.fromfile(baseline_path, dtype=np.float32)
    candidate = np.fromfile(candidate_path, dtype=np.float32)
    if baseline.shape != candidate.shape or not baseline.size:
        raise ValueError('Output size mismatch')
    if not np.isfinite(baseline).all() or not np.isfinite(candidate).all():
        raise ValueError('Nonfinite output')
    delta = baseline.astype(np.float64) - candidate.astype(np.float64)
    rows.append(dict(pattern=pattern, values=int(baseline.size),
                     bit_mismatches=int(np.count_nonzero(baseline.view(np.uint32) != candidate.view(np.uint32))),
                     max_abs=float(np.max(np.abs(delta))), rms=float(np.sqrt(np.mean(delta * delta))),
                     baseline_sha256=hashlib.sha256(baseline_path.read_bytes()).hexdigest(),
                     candidate_sha256=hashlib.sha256(candidate_path.read_bytes()).hexdigest()))
result = dict(status='rejected_for_deployment', variant='pair-interleaved K partition',
              tests=rows, limitations='Synthetic 256x256 test. Patterns 1/2 vary tensors and '
              'weights; 3 varies tensor 0 only, 4 tensor 1 only, 5 weights only. '
              'Not a full-model/game quality assessment; fused quantizer not integrated.')
(root / 'post-prototype-comparison.json').write_text(json.dumps(result, indent=2))
print(json.dumps(result, indent=2))
