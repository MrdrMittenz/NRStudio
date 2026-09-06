from pathlib import Path
import csv,json,math,collections,re
import numpy as np
ROOT=Path(__file__).resolve().parent
reports=[]
for p in sorted(ROOT.glob('*.csv')):
    if p.stem.endswith('-gpu') or p.stem.startswith('pilot'):continue
    rows=list(csv.DictReader(p.open()))
    if not rows:continue
    counts=collections.Counter(r['SwapChainAddress'] for r in rows)
    swap=counts.most_common(1)[0][0]
    frames=[r for r in rows if r['SwapChainAddress']==swap]
    intervals=np.array([float(r['MsBetweenPresents']) for r in frames[1:] if r['MsBetweenPresents'] not in ('NA','') and float(r['MsBetweenPresents'])>0])
    meta=json.loads(p.with_name(p.stem+'-metadata.json').read_text())
    log=p.with_name(p.stem+'-native.log').read_text()
    evals=re.findall(r'evaluate frame=(\d+) result=([0-9A-Fa-f]+) size=(\d+x\d+) guides=(\d+x\d+)',log)
    report={'trial':p.stem,'controlled':meta.get('input_idle',False) and meta.get('foreground_valid',False) and not meta.get('invalid_reason'),
      'invalid_reason':meta.get('invalid_reason'),'rows':len(frames),'swapchains':dict(counts),'frame_intervals':len(intervals),
      'mean_fps':1000/float(intervals.mean()),'one_percent_low_fps':1000/float(np.sort(intervals)[-math.ceil(.01*len(intervals)):].mean()),
      'p99_frame_ms':float(np.percentile(intervals,99)), 'mean_frame_ms':float(intervals.mean()),
      'duration_seconds':(float(frames[-1]['TimeInMs'])-float(frames[0]['TimeInMs']))/1000,
      'logged_eval_samples':len(evals),'model_sizes':sorted({e[2] for e in evals}),'guide_sizes':sorted({e[3] for e in evals}),
      'native_failures':sum(e[1]!='00000001' for e in evals)}
    gpu=list(csv.DictReader(p.with_name(p.stem+'-gpu.csv').open()))
    if gpu:
        memory=[float(next(v for k,v in r.items() if k.strip().startswith('memory.used')).split()[0]) for r in gpu]
        report['gpu_total_used_mib_peak']=max(memory)
    reports.append(report)
(ROOT/'results.json').write_text(json.dumps(reports,indent=2))
print(json.dumps(reports,indent=2))
