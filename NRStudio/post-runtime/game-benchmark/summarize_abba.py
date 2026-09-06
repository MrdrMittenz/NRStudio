"""Summarize saved ABBA measurements without treating individual frames as independent trials."""
from pathlib import Path
import json,csv,statistics
root=Path(__file__).resolve().parent
rows=[r for r in json.loads((root/'results.json').read_text()) if r['trial'].startswith('abba01-')]
for r in rows:
    m=json.loads((root/(r['trial']+'-metadata.json')).read_text());checks=m['foreground_checks']
    r['input_change_seconds']=[round(b['time']-checks[0]['time'],2) for a,b in zip(checks,checks[1:]) if a['last_input_tick']!=b['last_input_tick']]
    r['kernel_mode']=m['kernel_mode'];r['mode_acknowledged_before']=m['mode_acknowledged_before'];r['mode_acknowledged_after']=m['mode_acknowledged_after']
    gpu=[{k.strip():v.strip() for k,v in row.items()} for row in csv.DictReader((root/(r['trial']+'-gpu.csv')).open())]
    r['gpu_temperature_range_c']=[min(float(g['temperature.gpu']) for g in gpu),max(float(g['temperature.gpu']) for g in gpu)]
    r['mean_gpu_clock_mhz']=statistics.mean(float(g['clocks.current.graphics [MHz]'].split()[0]) for g in gpu)
summary={mode:statistics.mean(r['mean_fps'] for r in rows if r['kernel_mode']==mode) for mode in ['original','optimized']}
summary['observed_difference_fps']=summary['optimized']-summary['original'];summary['observed_difference_percent']=(summary['optimized']/summary['original']-1)*100
summary['reliable_speedup_established']=False
summary['limitations']='Input activity in all four captures; presentation intervals not independently classified for frame generation; two samples per mode.'
report={'trials':rows,'summary':summary}
(root/'abba01-comparison.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
