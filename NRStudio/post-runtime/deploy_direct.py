"""Deploy the validated direct-activation update to the existing STALKER profile."""
from pathlib import Path
import hashlib,json,subprocess,shutil,os,datetime
root=Path(__file__).resolve().parent
record=json.loads((root/'deployment.json').read_text());game=Path(record['game'])
processes=subprocess.check_output(['tasklist','/FO','CSV','/NH'],text=True).lower()
assert 'stalker2-win64-shipping.exe' not in processes,'Close STALKER before deployment'
validation=json.loads((root/'direct-validation/result.json').read_text())
assert validation['every_compared_frame_exact'] and validation['exports_unchanged']
assert json.loads((root/'switch-validation-direct/result.json').read_text())['both_modes_executed']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
source=root/'nvngx.dll_dlssnr.dll';target=game/source.name;before=root/'before-direct-update'/source.name
assert sha(target)==sha(before)==record['candidate'],'Unexpected game/runtime state'
assert sha(source)==next(x['forwarder_sha256'] for x in validation['results'] if x['case']=='candidate')
assert sha(game/'nvngx_dlssnr.dll')==record['model']
assert (game/'nr-post-opt.enable').is_file()
journal_path=game/'.nr-studio/installation.json';journal=json.loads(journal_path.read_text())
assert journal['State']=='Installed'
entries=[e for e in journal['Files'] if e['Name']==source.name];assert len(entries)==1
backup=root/'before-direct-update';shutil.copy2(journal_path,backup/'installation.json')
staged=game/'.nr-studio'/f'{source.name}.new';shutil.copy2(staged,backup/'forwarder-stage.new')
def atomic(path,data):
 temp=path.with_name(path.name+'.direct-update.tmp')
 with temp.open('xb') as f:f.write(data);f.flush();os.fsync(f.fileno())
 os.replace(temp,path)
atomic(target,source.read_bytes());atomic(staged,source.read_bytes())
entries[0]['Installed']=sha(source);atomic(journal_path,json.dumps(journal).encode())
record['previous_candidate']=record['candidate'];record['candidate']=sha(source)
record['direct_update_utc']=datetime.datetime.now(datetime.timezone.utc).isoformat()
record['benchmark_baseline']='previous validated post kernel plus original swin8'
atomic(root/'deployment.json',json.dumps(record,indent=2).encode())
assert sha(target)==sha(source)
print(json.dumps({'game':str(game),'forwarder_sha256':sha(target),'journal_updated':True},indent=2))
