"""Summarize cache experiments without projecting an in-game FPS gain."""
import collections
import json
import re
import statistics
from pathlib import Path

root = Path(__file__).resolve().parent
results = {}
for filename in ['post-cache-results.txt', 'post-cache-repeat.txt']:
    groups = collections.defaultdict(list)
    for line in (root / filename).read_text(encoding='utf-8-sig').splitlines():
        match = re.search(r'run=(\d+) carveout=(-?\d+) gpu_ms=([\d.]+)', line)
        if match:
            if 'unwritten=0 invalid=0' not in line or 'repeat=pass guards=pass' not in line:
                raise ValueError('Output validation failed: ' + line)
            if int(match[1]) >= 3:
                groups[int(match[2])].append(float(match[3]))
    if set(groups) != {-1, 0, 100}:
        raise ValueError('Missing cache policy')
    rows = {str(mode): dict(samples=len(values), median_ms=statistics.median(values),
                           min_ms=min(values), max_ms=max(values))
            for mode, values in sorted(groups.items())}
    saved = rows['-1']['median_ms'] - rows['100']['median_ms']
    results[filename] = dict(policies=rows, median_saved_ms=saved,
                            percent_saved=100 * saved / rows['-1']['median_ms'])
result = dict(experiments=results,
              limitations='Synthetic, overprovisioned tensors and float32 CUDA textures; '
              'not a full-model replay. Carveout is a driver preference. Ten launches '
              'per timing sample include submission gaps. No game FPS gain established '
              'and no runtime changes deployed.')
(root / 'post-summary.json').write_text(json.dumps(result, indent=2))
print(json.dumps(result, indent=2))
