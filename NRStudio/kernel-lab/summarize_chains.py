"""Summarize measured D3D12 timestamp spans; never infer FPS from these."""
import csv
import json
import statistics
from collections import defaultdict
from pathlib import Path

root = Path(__file__).resolve().parent
rows = list(csv.DictReader((root / 'chain-timings.tsv').open(), delimiter='\t'))
frame = -1
groups = defaultdict(list)
per_frame = defaultdict(lambda: defaultdict(float))
frame_totals = defaultdict(float)
for row in rows:
    if row['first'] == 'cc_cb_clear':
        frame += 1
    us = float(row['gpu_us'])
    if int(row['end_tick']) < int(row['start_tick']):
        raise ValueError('Non-monotonic timestamp span')
    frame_totals[frame] += us
    if frame > 0:
        key = (row['first'], row['last'], int(row['kernel_count']))
        groups[key].append(us)
        per_frame[key][frame] += us
warm_frames = len([f for f in frame_totals if f > 0])
ranking = []
for (first, last, count), times in groups.items():
    ranking.append(dict(first=first, last=last, kernels_per_chain=count,
                        calls_per_frame=len(times) / warm_frames,
                        mean_us=sum(times) / len(times),
                        total_us_per_frame=sum(times) / warm_frames,
                        median_total_us_per_frame=statistics.median(per_frame[(first, last, count)].values())))
ranking.sort(key=lambda row: row['median_total_us_per_frame'], reverse=True)
result = dict(frame_chain_totals_us=frame_totals, warm_frames=warm_frames,
              median_warm_frame_us=statistics.median(v for k, v in frame_totals.items() if k > 0),
              ranking=ranking,
              limitations='Synthetic full-model inputs. Timestamp spans include '
              'chain scheduling and instrumentation effects. No in-game FPS measurement.')
(root / 'chain-summary.json').write_text(json.dumps(result, indent=2))
print('Chain totals per frame (us):', dict(frame_totals))
for row in ranking[:15]:
    print(f"{row['median_total_us_per_frame']:9.2f} median us/frame {row['calls_per_frame']:4g} calls "
          f"{row['first']}" + (f" .. {row['last']}" if row['first'] != row['last'] else ''))
