"""Bounded isolated prepared-weight acceptance tests; no game deployment."""
from pathlib import Path
import hashlib
import json
import re
import statistics
import subprocess

root = Path(__file__).resolve().parent
processes = subprocess.check_output(['tasklist','/FO','CSV','/NH'], text=True).lower()
if 'stalker2-win64-shipping.exe' in processes or '7daystodie.exe' in processes:
    raise SystemExit('Close the game before isolated GPU tests')
results = []
def run(label, cubin, mode, pattern):
    output = root / (label + '.f32')
    proc = subprocess.run([str(root/'post_bench.exe'), cubin, mode, str(output), str(pattern)],
                          cwd=root, capture_output=True, text=True, timeout=60)
    (root/(label+'.txt')).write_text(proc.stdout+proc.stderr)
    if proc.returncode:
        raise RuntimeError(f'{label} failed: {proc.returncode}')
    times = [float(t) for t in re.findall(r'gpu_ms=([\d.]+)',proc.stdout)]
    result = dict(label=label,cubin=cubin,mode=mode,pattern=pattern,
                  gpu_ms=times,warm_median_ms=statistics.median(times[1:]),
                  output_sha256=hashlib.sha256(output.read_bytes()).hexdigest())
    results.append(result)
    print(label, result['warm_median_ms'], flush=True)
    return result

for pattern in (1,2,3,4,5):
    candidate=run(f'prepared-r224-small-{pattern}','post-prepared.cubin','small',pattern)
    expected=hashlib.sha256((root/f'post-original-small-{pattern}.f32').read_bytes()).hexdigest()
    if candidate['output_sha256'] != expected:
        raise RuntimeError('Small output mismatch')
for i,cubin in enumerate(('post-word-raw.cubin','post-prepared-r192.cubin','post-prepared.cubin',
                          'post-prepared.cubin','post-prepared-r192.cubin','post-word-raw.cubin')):
    run(f'prepared-compare-{i}',cubin,'normal',1)
hashes={r['output_sha256'] for r in results if r['mode']=='normal'}
if len(hashes)!=1:
    raise RuntimeError('1440p output mismatch')
(root/'prepared-weight-comparison.json').write_text(json.dumps(dict(
    tests=results, outputs_exact=True,
    scope='Synthetic post block only. Preparation excluded; no full-model or game gain established.'),indent=2))
