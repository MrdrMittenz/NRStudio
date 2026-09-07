"""Summarize one complete four-trial comparison without mixing older runs."""
from pathlib import Path
import argparse,collections,csv,json,math,re,statistics
root=Path(__file__).resolve().parent
p=argparse.ArgumentParser();p.add_argument('prefix');args=p.parse_args()
assert re.fullmatch(r'[a-zA-Z0-9_-]+',args.prefix)
sequence=json.loads((root/(args.prefix+'-sequence.json')).read_text())
assert len(sequence)==4 and [x['kernel_mode'] for x in sequence]==['original','optimized','optimized','original']
trials=[];pooled={'original':[],'optimized':[]}
def metrics(values):
 worst=sorted(values)[-math.ceil(len(values)*.01):]
 return dict(average_fps=1000/statistics.mean(values),one_percent_low_fps=1000/statistics.mean(worst),
             mean_frame_ms=statistics.mean(values),frame_intervals=len(values))
for item in sequence:
 name=item['trial'];mode=item['kernel_mode']
 meta=json.loads((root/(name+'-metadata.json')).read_text())
 rows=list(csv.DictReader((root/(name+'.csv')).open()))
 swaps=collections.Counter(r['SwapChainAddress'] for r in rows);swap=swaps.most_common(1)[0][0]
 rows=[r for r in rows if r['SwapChainAddress']==swap]
 intervals=[float(r['MsBetweenPresents']) for r in rows[1:] if r['MsBetweenPresents'] not in ('NA','') and float(r['MsBetweenPresents'])>0]
 assert intervals
 pooled[mode].extend(intervals)
 checks=meta['foreground_checks'];changes=sum(a['last_input_tick']!=b['last_input_tick'] for a,b in zip(checks,checks[1:]))
 native=(root/(name+'-native.log')).read_text()
 evals=re.findall(r'evaluate frame=(\d+) result=([0-9A-Fa-f]+) size=(\d+x\d+) guides=(\d+x\d+)',native)
 gpu=[{k.strip():v.strip() for k,v in r.items()} for r in csv.DictReader((root/(name+'-gpu.csv')).open())]
 telemetry={}
 for column in ('temperature.gpu','clocks.current.graphics [MHz]','power.draw [W]'):
  values=[float(r[column].split()[0]) for r in gpu if column in r and r[column]!='[N/A]']
  if values:telemetry[column]=dict(min=min(values),max=max(values),mean=statistics.mean(values))
 valid=bool(meta['returncode']==0 and meta['foreground_valid'] and meta['input_idle'] and
            meta['mode_acknowledged_before'] and meta['mode_acknowledged_after'] and evals and
            all(e[1]=='00000001' for e in evals))
 trials.append(dict(trial=name,mode=mode,**metrics(intervals),controlled=valid,
   foreground_valid=meta['foreground_valid'],input_timestamp_changes=changes,
   mode_acknowledged_before=meta['mode_acknowledged_before'],mode_acknowledged_after=meta['mode_acknowledged_after'],
   native_eval_samples=len(evals),native_failures=sum(e[1]!='00000001' for e in evals),
   model_sizes=sorted({e[2] for e in evals}),guide_sizes=sorted({e[3] for e in evals}),gpu=telemetry))
summary={mode:metrics(values) for mode,values in pooled.items()}
old=summary['original']['average_fps'];new=summary['optimized']['average_fps']
result=dict(prefix=args.prefix,pid=meta['pid'],trials=trials,pooled=summary,
 observed_fps_difference=new-old,observed_percent_difference=100*(new/old-1),
 all_trials_controlled=all(x['controlled'] for x in trials),
 baseline='Previous validated optimized post kernel and original swin8; NR remains enabled in both modes.',
 limitations='Presentation intervals; generated frames not independently classified. Two trials per mode. Input timestamp changes cannot be attributed to a device or person by this monitor.')
controlled={mode:[r for r in trials if r['controlled'] and r['mode']==mode] for mode in pooled}
if all(len(v)==1 for v in controlled.values()):
 a=controlled['original'][0]['average_fps'];b=controlled['optimized'][0]['average_fps']
 result['input_idle_pair']=dict(previous_fps=a,new_fps=b,observed_percent_difference=100*(b/a-1),
   limitation='One input-idle sample per mode; not an independent repeated benchmark.')
game_log=Path(r'C:\Program Files (x86)\Steam\steamapps\common\S.T.A.L.K.E.R. 2 Heart of Chornobyl\Stalker2\Binaries\Win64\dlssnr-native.log')
lines=[line for line in game_log.read_text().splitlines() if f'pid={meta["pid"]} ' in line]
ack=[line for line in lines if 'post-opt benchmark mode=' in line]
result['new_mode_restored_verified']=bool(ack and ack[-1].endswith('mode=optimized') and
 any('post-opt launch=' in line for line in lines[lines.index(ack[-1])+1:]))
(root/(args.prefix+'-comparison.json')).write_text(json.dumps(result,indent=2)+'\n')
lines=['# STALKER 2 direct-activation kernel comparison','',result['baseline'],'',
 '| Trial | Kernels | Average FPS | 1% low FPS | Input timestamp changes |',
 '| --- | --- | ---: | ---: | ---: |']
for index,row in enumerate(trials,1):
 lines.append(f"| {index} | {'Previous' if row['mode']=='original' else 'New'} | {row['average_fps']:.3f} | {row['one_percent_low_fps']:.3f} | {row['input_timestamp_changes']} |")
lines+=['',f'Pooled previous: {old:.3f} FPS. Pooled new: {new:.3f} FPS. Observed difference: {new-old:+.3f} FPS ({100*(new/old-1):+.2f}%).',
 '',f'All trials controlled: {result["all_trials_controlled"]}.', '',result['limitations'],
 '',f'Native evaluation samples: {sum(x["native_eval_samples"] for x in trials)}. Logged failures: {sum(x["native_failures"] for x in trials)}.',
 '', f'New kernels restored and acknowledged, with subsequent candidate launches: {result["new_mode_restored_verified"]}.',
 'Raw gameplay images remain local. This comparison alone does not establish a gain across other scenes or games.']
if 'input_idle_pair' in result:
 pair=result['input_idle_pair']
 lines+=['',f'The final input-idle pair measured {pair["previous_fps"]:.3f} FPS previous versus {pair["new_fps"]:.3f} FPS new ({pair["observed_percent_difference"]:+.2f}%). There is only one input-idle sample per mode.']
(root/(args.prefix+'-REPORT.md')).write_text('\n'.join(lines)+'\n')
print(json.dumps(result,indent=2))
