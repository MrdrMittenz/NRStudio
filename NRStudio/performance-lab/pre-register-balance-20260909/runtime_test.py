from pathlib import Path
import subprocess,os,shutil,re,json,csv,statistics,hashlib
root=Path(__file__).resolve().parent;runtime=Path.home()/'AppData/Local/Programs/NRStudio/runtime'
assert not any(x in subprocess.check_output(['tasklist','/FO','CSV','/NH'],text=True).lower() for x in ['stalker2-win64-shipping.exe','deadisland-win64-shipping.exe'])
allruns=[]
for name,n,timing in [('alternate-quality',12,False),('alternate-timing01',200,True),('alternate-timing02',200,True),('alternate-timing03',200,True)]:
 f=root/name;f.mkdir();dll=f/'nvngx.dll_dlssnr.dll';shutil.copy2(root/'forwarder'/dll.name,dll)
 for marker in ['nr-post-opt.enable','nr-gpu-timing.enable','nr-prepared-post.enable']:(f/marker).write_text('Isolated validation\n')
 env=os.environ.copy();env.update(NRSTUDIO_RUNTIME_ALTERNATE='1',NRSTUDIO_OUTER_TIMING='1')
 env.pop('NRSTUDIO_HYBRID_INPUT',None);env.pop('NRSTUDIO_TIMING_ONLY',None)
 if timing:env['NRSTUDIO_TIMING_ONLY']='1'
 with (f/'probe.txt').open('w') as log:
  p=subprocess.run([str(root.parent/'prepared-runtime-20260908/direct_probe.exe'),r'C:\Windows\System32\DriverStore\FileRepository\nvmdi.inf_amd64_ba34f758deb2a7d2\nvngx.dll',str(runtime/'nvngx_dlssnr.dll'),str(dll),'2560','1440',str(n)],cwd=f,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=120)
 text=(f/'probe.txt').read_text();assert p.returncode==0,(name,p.returncode,text[-1200:])
 assert 'kernel=pre' in (f/'dlssnr-native.log').read_text()
 assert text.count(' invalid=0 ')==(1 if timing else n)
 rows=list(csv.DictReader(next(f.glob('nr-gpu-timings-*.csv')).open()));rows.sort(key=lambda r:int(r['evaluation']));assert len(rows)==n
 for i,r in enumerate(rows):
  assert int(r['prepared'])==1,(name,i,r)
  assert r['scope']=='pipeline' and int(r['dropped'])==0 and int(r['unavailable'])==0
  assert all(float(r[k])>=0 for k in ['total_ms','encode_ms','guides_ms','model_ms','resolve_ms','post_ms']),r
 hashes=re.findall(r'frame_sha256 frame=\d+ mode=\w+ hash=([0-9a-f]{64})',text)
 if not timing:
  baseline=json.loads((root.parent/'prepared-runtime-20260908/1440-control/result.json').read_text())['hashes'];assert hashes==baseline
 medians={m:{k:statistics.median(float(r[k]) for i,r in enumerate(rows) if i>=32 and int(i%4 in [1,2])==m) for k in ['total_ms','model_ms','post_ms']} for m in [0,1]} if timing else {}
 result={'name':name,'frames':n,'medians':medians,'hashes':hashes,'samples':rows};allruns.append(result)
 print(name,medians if timing else '12 frames exact; both modes and all pipeline phase timestamps validated',flush=True)
(root/'alternate-results.json').write_text(json.dumps({'runs':allruns,'runtime_sha256':hashlib.sha256((root/'forwarder/nvngx.dll_dlssnr.dll').read_bytes()).hexdigest(),'scope':'Coarse completion-aware GPU timestamps, no 156 per-kernel timing wrappers. Outer phase API exercised with empty surrounding passes; actual game pass costs require live capture.'},indent=2))
