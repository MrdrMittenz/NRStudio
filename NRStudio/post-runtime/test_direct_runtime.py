"""Validate the embedded runtime, every output frame, exports and cleanup."""
from pathlib import Path
import subprocess,hashlib,json,re,shutil,os
import pefile
root=Path(__file__).resolve().parent;lab=root.parent/'kernel-lab'
processes=subprocess.check_output(['tasklist','/FO','CSV','/NH'],text=True).lower()
assert not any(g in processes for g in ('stalker2-win64-shipping.exe','7daystodie.exe'))
source=(lab/'direct_probe.cpp').read_text()
assert source.count('NRDirectTrace::Install();')==1
source=source.replace('NRDirectTrace::Install();','if(BCryptOpenAlgorithmProvider(&NRDirectTrace::sha,BCRYPT_SHA256_ALGORITHM,nullptr,0)<0)return 97;')
source=source.replace('ChainTiming::Initialize(queue);','')
(lab/'integrated_probe.cpp').write_text(source)
build=(lab/'build_direct_probe.cmd').read_text().replace('direct_probe','integrated_probe')
(lab/'build_integrated_probe.cmd').write_text(build)
subprocess.run([str(lab/'build_integrated_probe.cmd')],cwd=lab,check=True)
baseline=root/'before-direct-update/nvngx.dll_dlssnr.dll';candidate=root/'nvngx.dll_dlssnr.dll'
def exports(path):
 pe=pefile.PE(str(path));return [(e.ordinal,e.name.decode() if e.name else None) for e in pe.DIRECTORY_ENTRY_EXPORT.symbols]
assert exports(baseline)==exports(candidate),'Exports changed'
case=root/'direct-validation';case.mkdir(exist_ok=False)
runtime=Path.home()/'AppData/Local/Programs/NRStudio/runtime'
core=r'C:\Windows\System32\DriverStore\FileRepository\nvmdi.inf_amd64_ba34f758deb2a7d2\nvngx.dll'
env=os.environ.copy();env.pop('NRSTUDIO_TIMING_ONLY',None)
rows=[]
for name,forward,enabled,frames,width,height in (
 ('baseline',baseline,True,60,2560,1440),('candidate',candidate,True,60,2560,1440),
 ('disabled',candidate,False,12,2560,1440),
 ('baseline-1080',baseline,True,12,1920,1080),('candidate-1080',candidate,True,12,1920,1080),
 ('baseline-1728',baseline,True,12,3072,1728),('candidate-1728',candidate,True,12,3072,1728)):
 folder=case/name;folder.mkdir();dll=folder/'nvngx.dll_dlssnr.dll';shutil.copy2(forward,dll)
 if enabled:(folder/'nr-post-opt.enable').write_text('Enabled for isolated validation\n')
 with (folder/'probe.txt').open('w') as log:
  proc=subprocess.run([str(lab/'integrated_probe.exe'),core,str(runtime/'nvngx_dlssnr.dll'),str(dll),str(width),str(height),str(frames)],cwd=folder,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=120)
 assert proc.returncode==0,(name,proc.returncode)
 text=(folder/'probe.txt').read_text();assert text.count(' invalid=0 ')==frames
 hashes=re.findall(r'frame_sha256 frame=\d+ mode=\w+ hash=([0-9a-f]{64})',text);assert len(hashes)==frames
 log=(folder/'dlssnr-native.log').read_text()
 if enabled and name.startswith('candidate'):
  for key in ('post','swin8'):
   assert 'post-opt ready: kernel='+key in log
   assert 'post-opt released: kernel='+key in log
 elif not enabled:assert 'post-opt disabled: no enable marker' in log
 row=dict(case=name,width=width,height=height,frames=frames,frame_hashes=hashes,
          forwarder_sha256=hashlib.sha256(dll.read_bytes()).hexdigest())
 rows.append(row);print(name,'passed',flush=True)
by_name={r['case']:r for r in rows}
for suffix in ('','-1080','-1728'):
 assert by_name['baseline'+suffix]['frame_hashes']==by_name['candidate'+suffix]['frame_hashes'],'Frame mismatch '+suffix
assert by_name['disabled']['frame_hashes']==by_name['baseline']['frame_hashes'][:12]
report=dict(exports_unchanged=True,every_compared_frame_exact=True,results=rows,
 limitations='Synthetic model inputs. Does not measure game FPS or establish all-scene equivalence.')
(case/'result.json').write_text(json.dumps(report,indent=2));print('Runtime validation passed')
