"""Summarize isolated scheduling tests, preserving failed runs and all timings."""
from pathlib import Path
import csv,json,statistics,hashlib
root=Path(__file__).resolve().parent
reference=(root/'launch-batch-off-1440/native-output-2560x1440.rgba16f').read_bytes()
report={}
for name in ['off','on','on-repeat','off-repeat','on-limit4','off-limit4-control']:
    case=root/f'launch-batch-{name}-1440'
    output=case/'native-output-2560x1440.rgba16f'
    if not output.exists():
        report[name]={'completed':False,'failure':'Probe reported device after evaluation HRESULT=887a0006 on frame 0','deployed':False}
        continue
    rows=list(csv.DictReader((case/'chain-timings.tsv').open(),delimiter='\t'))
    times=[float(r['gpu_us'])/1000 for r in rows]
    assert all(int(r['end_tick'])>=int(r['start_tick']) for r in rows)
    events=list(csv.DictReader((case/'batch-events.tsv').open(),delimiter='\t'))
    frames=[r for r in events if r['event']=='frame_launches']
    data=output.read_bytes()
    report[name]=dict(completed=True,model_gpu_ms=times,median_warm_ms=statistics.median(times[1:]),final_output_byte_exact=data==reference,sha256=hashlib.sha256(data).hexdigest(),frame_launches=[int(r['value']) for r in frames],frame_groups=[int(r['status_or_groups']) for r in frames],deployed=False)
assert all(r['final_output_byte_exact'] for r in report.values() if r['completed'])
(root/'launch-batch-comparison.json').write_text(json.dumps(report,indent=2)+'\n')
for name,row in report.items():print(name,row.get('median_warm_ms',row.get('failure')))
