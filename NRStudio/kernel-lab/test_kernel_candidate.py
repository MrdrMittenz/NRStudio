"""Compare an isolated post-kernel candidate against the accepted post kernel."""
from pathlib import Path
import argparse,hashlib,json,re,statistics,subprocess
root=Path(__file__).resolve().parent
p=argparse.ArgumentParser()
p.add_argument('prefix');p.add_argument('cubin');p.add_argument('--control',default='post-word-raw.cubin')
args=p.parse_args()
if not re.fullmatch(r'[a-z0-9-]+',args.prefix):raise ValueError('Invalid prefix')
processes=subprocess.check_output(['tasklist','/FO','CSV','/NH'],text=True).lower()
if any(game in processes for game in ('stalker2-win64-shipping.exe','7daystodie.exe')):
    raise SystemExit('Close the game before isolated tests')
results=[]
def run(label,cubin,mode,pattern):
    output=root/(label+'.f32')
    if output.exists():raise FileExistsError(output)
    proc=subprocess.run([str(root/'post_bench.exe'),cubin,mode,str(output),str(pattern)],
                        cwd=root,capture_output=True,text=True,timeout=60)
    (root/(label+'.txt')).write_text(proc.stdout+proc.stderr)
    if proc.returncode:raise RuntimeError(f'{label} failed: {proc.returncode}')
    times=[float(t) for t in re.findall(r'gpu_ms=([\d.]+)',proc.stdout)]
    row=dict(label=label,cubin=cubin,mode=mode,pattern=pattern,gpu_ms=times,
             warm_median_ms=statistics.median(times[1:]),
             output_sha256=hashlib.sha256(output.read_bytes()).hexdigest())
    results.append(row);print(label,row['warm_median_ms'],flush=True)
    return row
for pattern in (1,2,3,4,5):
    row=run(f'{args.prefix}-small-{pattern}',args.cubin,'small',pattern)
    expected=hashlib.sha256((root/f'post-original-small-{pattern}.f32').read_bytes()).hexdigest()
    if row['output_sha256']!=expected:raise RuntimeError('Small output mismatch')
for i,cubin in enumerate((args.control,args.cubin,args.cubin,args.control)):
    run(f'{args.prefix}-abba-{i}',cubin,'normal',1)
if len({r['output_sha256'] for r in results if r['mode']=='normal'})!=1:
    raise RuntimeError('1440p output mismatch')
(root/(args.prefix+'-comparison.json')).write_text(json.dumps(dict(tests=results,outputs_exact=True,
 scope='Synthetic post kernel only, not a game FPS measurement.'),indent=2))
