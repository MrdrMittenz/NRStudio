"""Record prototype comparisons and timing limitations; arrays remain local."""
from pathlib import Path
import json
import re
import statistics
import numpy as np

root = Path(__file__).resolve().parent
comparisons = []
for variant in ['reverse-interleaved', 'reverse-contiguous', 'packed-reverse', 'inline-packed-reverse']:
    for pattern in (1, 2, 5):
        baseline = np.fromfile(root / f'post-original-small-{pattern}.f32', dtype=np.float32)
        candidate = np.fromfile(root / f'post-{variant}-{pattern}.f32', dtype=np.float32)
        if baseline.shape != candidate.shape or not np.isfinite(candidate).all():
            raise ValueError('Invalid candidate output')
        comparisons.append(dict(variant=variant, pattern=pattern,
            mismatches=int(np.count_nonzero(baseline.view(np.uint32) != candidate.view(np.uint32))),
            max_abs=float(np.max(np.abs(baseline.astype(np.float64) - candidate))),
            values=int(baseline.size)))
timings = {}
for filename in ['post-current-batched-1440.txt', 'post-inline-batched-1440.txt']:
    samples = []
    for line in (root / filename).read_text(encoding='utf-8-sig').splitlines():
        match = re.search(r'run=(\d+) carveout=(-?\d+) gpu_ms=([\d.]+)', line)
        if match and int(match[1]) >= 3 and int(match[2]) == -1:
            if 'unwritten=0 invalid=0' not in line or 'repeat=pass guards=pass' not in line:
                raise ValueError('Failed timing-run output check')
            samples.append(float(match[3]))
    timings[filename] = dict(samples=len(samples), median_ms=statistics.median(samples),
                            min_ms=min(samples), max_ms=max(samples))
result = dict(status='rejected_for_deployment', comparisons=comparisons, timings=timings,
              limitations='Synthetic tensors/weights. Small-output numerical comparisons '
              'and separate uniform 1440p timing runs. No in-game FPS improvement. '
              'Reversing accumulation fixes the weights-only case but not combined inputs.')
(root / 'arithmetic-variants-summary.json').write_text(json.dumps(result, indent=2))
print('Saved arithmetic-variants-summary.json')
