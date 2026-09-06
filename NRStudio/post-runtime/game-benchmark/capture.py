"""Capture one stationary 30-second trial after a five-second settling period.
Run only with the selected NR state visually verified and gameplay in focus.
"""
from pathlib import Path
import sys, subprocess, time, json, ctypes
from PIL import ImageGrab
ROOT=Path(__file__).resolve().parent
GAME=Path(r'C:\Program Files (x86)\Steam\steamapps\common\S.T.A.L.K.E.R. 2 Heart of Chornobyl\Stalker2\Binaries\Win64')
PM=ROOT/'PresentMon-2.5.1-x64.exe'
pid=int(sys.argv[1]); name=sys.argv[2]
user=ctypes.windll.user32
user.GetForegroundWindow.restype=ctypes.c_void_p
user.GetWindowThreadProcessId.argtypes=[ctypes.c_void_p,ctypes.POINTER(ctypes.c_ulong)]
def foreground():
    result=ctypes.c_ulong(); user.GetWindowThreadProcessId(user.GetForegroundWindow(),ctypes.byref(result)); return result.value
class LastInput(ctypes.Structure):
    _fields_=[('cbSize',ctypes.c_uint),('dwTime',ctypes.c_uint)]
def last_input():
    info=LastInput();info.cbSize=ctypes.sizeof(info);user.GetLastInputInfo(ctypes.byref(info));return info.dwTime
log=GAME/'dlssnr-native.log'
metadata={'name':name,'pid':pid,'warmup_seconds':5,'requested_seconds':30,'started_utc':time.strftime('%Y-%m-%dT%H:%M:%SZ',time.gmtime()),'foreground_checks':[]}
start=log.stat().st_size
time.sleep(5)
ImageGrab.grab().save(ROOT/(name+'-before.png'))
with (ROOT/(name+'-gpu.csv')).open('w') as out:
    gpu=subprocess.Popen(['nvidia-smi','--query-gpu=timestamp,name,driver_version,memory.used,utilization.gpu,temperature.gpu,power.draw,clocks.current.graphics,clocks.current.memory','--format=csv','--loop-ms=1000'],stdout=out,stderr=subprocess.DEVNULL,creationflags=0x08000000)
    capture=subprocess.Popen([str(PM),'--process_id',str(pid),'--output_file',str(ROOT/(name+'.csv')),'--timed','30','--terminate_after_timed','--no_console_stats','--session_name','NRStudioBenchmark'],stdout=subprocess.PIPE,stderr=subprocess.STDOUT,creationflags=0x08000000)
    while capture.poll() is None:
        metadata['foreground_checks'].append({'time':time.time(),'pid':foreground(),'last_input_tick':last_input()});time.sleep(.5)
    gpu.terminate();gpu.wait(timeout=10)
metadata['returncode']=capture.returncode
metadata['console']=capture.stdout.read().decode(errors='replace')
metadata['finished_utc']=time.strftime('%Y-%m-%dT%H:%M:%SZ',time.gmtime())
metadata['foreground_valid']=all(x['pid']==pid for x in metadata['foreground_checks'])
metadata['input_idle']=len({x['last_input_tick'] for x in metadata['foreground_checks']})==1
ImageGrab.grab().save(ROOT/(name+'-after.png'))
with log.open('rb') as f: f.seek(start); (ROOT/(name+'-native.log')).write_bytes(f.read())
(ROOT/(name+'-metadata.json')).write_text(json.dumps(metadata,indent=2))
print(json.dumps({k:v for k,v in metadata.items() if k!='foreground_checks'},indent=2))
if capture.returncode or not metadata['foreground_valid'] or not metadata['input_idle']: sys.exit(1)
