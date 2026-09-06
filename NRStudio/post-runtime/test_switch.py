from pathlib import Path
import subprocess,time,hashlib,json
from benchmark_switch import select
root=Path(__file__).resolve().parent
probe=root.parents[1]/'fp8-ampere-candidate/native-nr-3090-v1/native_probe.exe'
runtime=Path.home()/'AppData/Local/Programs/NRStudio/runtime'
core=r'C:\Windows\System32\DriverStore\FileRepository\nvmdi.inf_amd64_ba34f758deb2a7d2\nvngx.dll'
case=root/'switch-validation';case.mkdir(exist_ok=True)
with (case/'probe.txt').open('w') as output:
    proc=subprocess.Popen([str(probe),core,str(runtime/'nvngx_dlssnr.dll'),str(root/'nvngx.dll_dlssnr.dll'),'2560','1440','12'],cwd=case,stdout=output,stderr=subprocess.STDOUT,creationflags=0x08000000)
    try:
        deadline=time.monotonic()+15
        while True:
            try:select(proc.pid,'original');break
            except OSError:
                if proc.poll() is not None or time.monotonic()>deadline:raise
                time.sleep(.05)
        time.sleep(1.0)
        select(proc.pid,'optimized')
        result=proc.wait(timeout=20)
    finally:
        if proc.poll() is None:proc.terminate();proc.wait(timeout=5)
assert result==0,result
log=(root/'dlssnr-native.log').read_text()
lines=[x for x in log.splitlines() if f'pid={proc.pid} ' in x]
assert any('benchmark mode=original' in x for x in lines)
assert any('benchmark mode=optimized' in x for x in lines)
assert any('post-opt original launch=' in x for x in lines)
assert any('post-opt launch=' in x for x in lines)
text=(case/'probe.txt').read_text();assert text.count(' invalid=0 ')==12
actual=case/'native-output-2560x1440.rgba16f';reference=root/'disabled/native-output-2560x1440.rgba16f'
assert actual.read_bytes()==reference.read_bytes(),'Output mismatch with original 12-frame reference'
(case/'runtime.log').write_text('\n'.join(lines)+'\n')
report=dict(pid=proc.pid,frames=12,both_modes_executed=True,final_output_byte_exact=True,sha256=hashlib.sha256(actual.read_bytes()).hexdigest())
(case/'result.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
