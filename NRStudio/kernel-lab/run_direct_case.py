from pathlib import Path
import argparse,subprocess,os,json,hashlib,re
root=Path(__file__).resolve().parent
p=argparse.ArgumentParser();p.add_argument('case');p.add_argument('--mode',choices=['candidate','control','alternate'],required=True)
p.add_argument('--keys',default='post,swin1,pre,swin8');p.add_argument('--frames',type=int,default=60);p.add_argument('--timing-only',action='store_true')
p.add_argument('--width',type=int,default=2560);p.add_argument('--height',type=int,default=1440);args=p.parse_args()
if not re.fullmatch(r'[a-z0-9-]+',args.case):raise ValueError('Invalid case')
assert set(args.keys.split(','))<={'post','swin1','pre','swin8'}
assert 1<=args.frames<=100
processes=subprocess.check_output(['tasklist','/FO','CSV','/NH'],text=True).lower()
assert not any(g in processes for g in ('stalker2-win64-shipping.exe','7daystodie.exe')),'Close game before testing'
case=root/args.case;case.mkdir(exist_ok=False)
env=os.environ.copy();env.update(NRSTUDIO_DIRECT_DIR=str(root),NRSTUDIO_DIRECT_MODE=args.mode,NRSTUDIO_DIRECT_KEYS=args.keys)
env.pop('NRSTUDIO_TIMING_ONLY',None)
if args.timing_only:env['NRSTUDIO_TIMING_ONLY']='1'
runtime=Path.home()/'AppData/Local/Programs/NRStudio/runtime'
with (case/'probe.txt').open('w') as log:
 result=subprocess.run([str(root/'direct_probe.exe'),r'C:\Windows\System32\DriverStore\FileRepository\nvmdi.inf_amd64_ba34f758deb2a7d2\nvngx.dll',str(runtime/'nvngx_dlssnr.dll'),str(runtime/'nvngx.dll_dlssnr.dll'),str(args.width),str(args.height),str(args.frames)],cwd=case,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=120)
assert result.returncode==0,result.returncode
text=(case/'probe.txt').read_text();expected_checks=1 if args.timing_only else args.frames
assert text.count(' invalid=0 ')==expected_checks
hashes=re.findall(r'frame_sha256 frame=(\d+) mode=(\w+) hash=([0-9a-f]{64})',text)
assert len(hashes)==expected_checks
report=dict(case=args.case,mode=args.mode,keys=args.keys,frames=args.frames,timing_only=args.timing_only,frame_hashes=hashes,
 final_sha256=hashlib.sha256((case/f'native-output-{args.width}x{args.height}.rgba16f').read_bytes()).hexdigest())
(case/'run.json').write_text(json.dumps(report,indent=2));print(json.dumps({k:v for k,v in report.items() if k!='frame_hashes'}))
