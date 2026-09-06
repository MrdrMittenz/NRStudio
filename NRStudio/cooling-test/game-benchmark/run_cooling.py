"""Compare automatic fans with 80%; restore automatic control even on failure."""
from pathlib import Path
import argparse,json,subprocess,sys,time
root=Path(__file__).resolve().parent
sys.path.insert(0,str(root.parents[1]/'post-runtime'))
from benchmark_switch import select
p=argparse.ArgumentParser();p.add_argument('pid',type=int);p.add_argument('--prefix',default='cooling01');args=p.parse_args()
fan=root.parent/'fanctl.exe'
log=Path(r'C:\Program Files (x86)\Steam\steamapps\common\S.T.A.L.K.E.R. 2 Heart of Chornobyl\Stalker2\Binaries\Win64\dlssnr-native.log')
records=[]
manual_requested=False
def control(*values):
    result=subprocess.run([str(fan),*values],capture_output=True,text=True,timeout=12)
    records.append(dict(time=time.time(),args=values,returncode=result.returncode,output=result.stdout+result.stderr))
    if result.returncode:raise RuntimeError(records[-1])
    return result.stdout
try:
    before=control('inspect')
    if 'fan_flags=1 ' not in before:raise RuntimeError('Expected original automatic fan mode')
    select(args.pid,'optimized')
    for mode in ['auto','fan80']:
        name=f'{args.prefix}-{mode}'
        if(root/(name+'.csv')).exists():raise RuntimeError('Choose a fresh prefix')
        if mode=='fan80':
            manual_requested=True
            control('set','80')
            print('80% fan requested. Cooling for 60 seconds before capture.',flush=True)
            with (root/(name+'-settle-gpu.csv')).open('w') as out:
                telemetry=subprocess.Popen(['nvidia-smi','--query-gpu=timestamp,temperature.gpu,fan.speed,clocks.current.graphics,clocks_event_reasons.sw_thermal_slowdown','--format=csv','--loop-ms=1000'],stdout=out,creationflags=0x08000000)
                try:
                    for step in range(2):time.sleep(30);print(f'Cooling elapsed {(step+1)*30}/60 seconds',flush=True)
                finally:telemetry.terminate();telemetry.wait(timeout=5)
            state=control('inspect')
            if 'fan_percent=80 fan_flags=0 ' not in state:raise RuntimeError('Manual 80% setting not retained')
        print(f'Capturing {mode}: five seconds settling, then 30 seconds gameplay',flush=True)
        result=subprocess.run([sys.executable,str(root/'capture.py'),str(args.pid),name],cwd=root,timeout=50)
        meta=json.loads((root/(name+'-metadata.json')).read_text());meta['fan_test_mode']=mode
        if not meta['input_idle']:meta['invalid_reason']='Input activity during capture'
        if not meta['foreground_valid']:meta['invalid_reason']='Game lost foreground'
        (root/(name+'-metadata.json')).write_text(json.dumps(meta,indent=2))
        if not meta['foreground_valid'] or meta['returncode']:raise RuntimeError('Invalid capture; stopping comparison')
finally:
    try:
        if manual_requested:control('auto')
        state=control('inspect')
        if 'fan_flags=1 ' not in state:raise RuntimeError('Automatic fan restoration not confirmed')
        print('Automatic fan control restored and verified',flush=True)
    finally:(root/(args.prefix+'-fan-control.json')).write_text(json.dumps(records,indent=2)+'\n')
subprocess.run([sys.executable,str(root/'analyze.py')],cwd=root,check=True)
