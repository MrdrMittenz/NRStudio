"""Run an original/optimized/optimized/original comparison; restore optimized."""
from pathlib import Path
import argparse,subprocess,sys,time,json
sys.path.insert(0,str(Path(__file__).resolve().parent.parent))
from benchmark_switch import select
root=Path(__file__).resolve().parent
parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('pid',type=int);parser.add_argument('--prefix',default='abba01');args=parser.parse_args()
log=Path(r'C:\Program Files (x86)\Steam\steamapps\common\S.T.A.L.K.E.R. 2 Heart of Chornobyl\Stalker2\Binaries\Win64\dlssnr-native.log')
report=[]
def acknowledged(mode):
    lines=[line for line in log.read_text().splitlines() if f'pid={args.pid} ' in line and 'post-opt benchmark mode=' in line]
    return bool(lines and lines[-1].endswith('mode='+mode))
try:
    for index,mode in enumerate(['original','optimized','optimized','original'],1):
        name=f'{args.prefix}-{index}-{mode}'
        if (root/(name+'.csv')).exists():raise RuntimeError('Use a new prefix to preserve previous results')
        select(args.pid,mode)
        deadline=time.monotonic()+5
        while not acknowledged(mode):
            if time.monotonic()>deadline:raise RuntimeError('Runtime did not acknowledge kernel mode')
            time.sleep(.1)
        print(f'Trial {index}/4: {mode}; 5-second settling, then 30-second capture',flush=True)
        result=subprocess.run([sys.executable,str(root/'capture.py'),str(args.pid),name],cwd=root)
        meta=json.loads((root/(name+'-metadata.json')).read_text())
        meta['kernel_mode']=mode;meta['mode_acknowledged_before']=True;meta['mode_acknowledged_after']=acknowledged(mode)
        meta['invalid_reason']=None
        if not meta['input_idle']:meta['invalid_reason']='Input activity during capture'
        if not meta['foreground_valid']:meta['invalid_reason']='Game lost foreground during capture'
        if not meta['mode_acknowledged_after']:raise RuntimeError('Kernel mode changed unexpectedly')
        (root/(name+'-metadata.json')).write_text(json.dumps(meta,indent=2))
        report.append(dict(trial=name,kernel_mode=mode,returncode=result.returncode,input_idle=meta['input_idle'],foreground_valid=meta['foreground_valid']))
        if not meta['foreground_valid']:raise RuntimeError('Stopping: game lost focus')
finally:
    select(args.pid,'optimized')
    (root/(args.prefix+'-sequence.json')).write_text(json.dumps(report,indent=2)+'\n')
    print('Optimized kernel requested for normal play',flush=True)
subprocess.run([sys.executable,str(root/'analyze.py')],cwd=root,check=True)
