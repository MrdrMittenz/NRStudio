"""Compare the saved final output and all timestamp samples of isolated runs."""
from pathlib import Path
import csv
import hashlib
import json
import statistics
import numpy as np

root = Path(__file__).resolve().parent
reference = np.fromfile(root / 'full-post-baseline-1440/native-output-2560x1440.rgba16f', dtype=np.uint16)
report = {}
for path in sorted(root.glob('full-post-*-1440')):
    rows = list(csv.DictReader((path / 'chain-timings.tsv').open(), delimiter='\t'))
    frames, post = [], []
    for row in rows:
        if int(row['end_tick']) < int(row['start_tick']):
            raise ValueError('Non-monotonic GPU timestamp')
        if row['first'] == 'cc_cb_clear':
            frames.append(0.0)
        frames[-1] += float(row['gpu_us']) / 1000
        if row['first'] == 'cc_tinlayout_fused_post_block_swin_1h_32_fp8':
            post.append(float(row['gpu_us']) / 1000)
    output = path / 'native-output-2560x1440.rgba16f'
    actual = np.fromfile(output, dtype=np.uint16)
    if actual.shape != reference.shape:
        raise ValueError('Output shape mismatch')
    report[path.name] = dict(frames_ms=frames, median_warm_chain_ms=statistics.median(frames[1:]),
        post_ms=post, median_warm_post_ms=statistics.median(post[1:]),
        output_sha256=hashlib.sha256(output.read_bytes()).hexdigest(),
        mismatches=int(np.count_nonzero(reference != actual)), values=int(reference.size))
(root / 'full-post-comparison.json').write_text(json.dumps(report, indent=2) + '\n')
print(json.dumps(report, indent=2))
if any(row['mismatches'] for row in report.values()):
    raise SystemExit(1)
