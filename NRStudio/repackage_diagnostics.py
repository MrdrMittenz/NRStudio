"""Refresh app/docs in the existing local installer, preserving runtime bytes."""
from pathlib import Path
import subprocess, zipfile, hashlib, json, io

root=Path(__file__).resolve().parent
review=root.parent/'NRStudio-private-review'
csc=r'C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\Roslyn\csc.exe'
subprocess.run(['python',str(root/'prepare_private_review.py')],check=True)
subprocess.run(['python',str(review/'build_review.py'),'--csc',csc],check=True)
stage=root/'build-diagnostics-1.1.1'
stage.mkdir(exist_ok=True)
payload=stage/'app.zip'
original=root/'dist/NRStudio-Setup-1.1.0-Complete.exe'
source=io.BytesIO()
with zipfile.ZipFile(source,'w',zipfile.ZIP_DEFLATED) as z:
    for p in (root/'src').glob('*'): z.write(p,'src/'+p.name)
    for name in ['build.py','README.txt','MODEL-AUDIT.md','repackage_diagnostics.py']:
        z.write(root/name,name)
replacements={
    'NRStudio.exe':(review/'build/NRStudio.exe').read_bytes(),
    'Uninstall.exe':(review/'build/Uninstall.exe').read_bytes(),
    'README.txt':(root/'README.txt').read_bytes(),
    'MODEL-AUDIT.md':(root/'MODEL-AUDIT.md').read_bytes(),
    'NRStudio-source.zip':source.getvalue(),
}
manifest={}
with zipfile.ZipFile(original) as old, zipfile.ZipFile(payload,'w',zipfile.ZIP_DEFLATED,compresslevel=3) as new:
    for entry in old.infolist():
        if entry.filename in replacements or entry.filename=='SHA256.json': continue
        digest=hashlib.sha256()
        dest=zipfile.ZipInfo(entry.filename)
        dest.compress_type=zipfile.ZIP_STORED if entry.filename.startswith('prerequisites/') else zipfile.ZIP_DEFLATED
        with old.open(entry) as src, new.open(dest,'w',force_zip64=True) as sink:
            while chunk:=src.read(4*1024*1024): sink.write(chunk);digest.update(chunk)
        manifest[entry.filename]=digest.hexdigest()
    for name,data in replacements.items():
        new.writestr(name,data);manifest[name]=hashlib.sha256(data).hexdigest()
    new.writestr('SHA256.json',json.dumps(manifest,indent=2))
subprocess.run(['python',str(review/'build_review.py'),'--csc',csc,'--payload',str(payload)],check=True)
target=root/'dist/NRStudio-Setup-1.1.1-Experimental.exe'
target.write_bytes((review/'build/NRStudio-Setup.exe').read_bytes())
print('Built',target,flush=True)
print('SHA256',hashlib.sha256(target.read_bytes()).hexdigest(),flush=True)
