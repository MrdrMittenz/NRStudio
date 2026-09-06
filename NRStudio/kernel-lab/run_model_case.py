"""Run one isolated full-model case with the unchanged installed forwarder."""
from pathlib import Path
import argparse,subprocess,os,json,hashlib
root=Path(__file__).resolve().parent
p=argparse.ArgumentParser();p.add_argument('case');p.add_argument('--kernel',choices=['swin1','swin8','pre','post']);p.add_argument('--cubin');p.add_argument('--width',type=int,default=256);p.add_argument('--height',type=int,default=256);p.add_argument('--frames',type=int,default=10);args=p.parse_args()
assert bool(args.kernel)==bool(args.cubin)
processes=subprocess.check_output(['tasklist','/fo','csv'],text=True)
assert 'Stalker2-Win64-Shipping.exe' not in processes, 'Close the game before isolated GPU measurements'
case=root/args.case;case.mkdir(exist_ok=False)
env=os.environ.copy()
for key in ['NRSTUDIO_TEST_CUBIN','NRSTUDIO_TEST_KERNEL']:env.pop(key,None)
if args.cubin:
 env['NRSTUDIO_TEST_CUBIN']=str((root/args.cubin).resolve())
 env['NRSTUDIO_TEST_KERNEL']={'swin8':'cc_tinlayout_fused_swin_8h_256_8_chained_fp8','swin1':'cc_tinlayout_fused_swin_1h_32_1_chained_fp8','pre':'cc_tinlayout_fused_pre_block_swin_1h_32_1_ds_fp8','post':'cc_tinlayout_fused_post_block_swin_1h_32_fp8'}[args.kernel]
runtime=Path.home()/'AppData/Local/Programs/NRStudio/runtime'
with (case/'probe.txt').open('w') as log:
 result=subprocess.run([str(root/'trace_probe.exe'),r'C:\Windows\System32\DriverStore\FileRepository\nvmdi.inf_amd64_ba34f758deb2a7d2\nvngx.dll',str(runtime/'nvngx_dlssnr.dll'),str(runtime/'nvngx.dll_dlssnr.dll'),str(args.width),str(args.height),str(args.frames)],cwd=case,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=60)
assert result.returncode==0,result.returncode
text=(case/'probe.txt').read_text();assert text.count(' invalid=0 ')==args.frames
out=case/f'native-output-{args.width}x{args.height}.rgba16f'
report=dict(case=args.case,frames=args.frames,sha256=hashlib.sha256(out.read_bytes()).hexdigest(),candidate=args.cubin)
(case/'run.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
