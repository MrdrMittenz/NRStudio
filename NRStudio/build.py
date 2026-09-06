from pathlib import Path
import subprocess, shutil, zipfile, hashlib, json

ROOT = Path(__file__).resolve().parent
REF = ROOT.parent / 'optiscaler-nr-reference'
LAB = ROOT.parent / 'fp8-ampere-candidate'
CSC = r'C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\Roslyn\csc.exe'
DIST = ROOT / 'dist'
APP = DIST / 'NRStudio'
APP.mkdir(parents=True, exist_ok=True)
(APP / 'runtime').mkdir(exist_ok=True)

def compile(name, sources, target='winexe', extra=()):
    command = [CSC, '/nologo', '/optimize+', '/platform:x64', '/target:'+target,
               '/out:'+str(name), '/r:System.Windows.Forms.dll', '/r:System.Drawing.dll',
               '/r:System.Web.Extensions.dll', '/r:System.IO.Compression.dll',
               '/r:System.IO.Compression.FileSystem.dll', '/r:Microsoft.CSharp.dll',
               '/win32manifest:'+str(ROOT/'src/app.manifest')]
    subprocess.run(command + list(extra) + [str(ROOT/'src'/s) for s in sources], check=True)

compile(APP/'NRStudio.exe', ['Core.cs','Prerequisites.cs','App.cs'])
compile(APP/'Uninstall.exe', ['Core.cs','Prerequisites.cs','Setup.cs'])
compile(DIST/'Tests.exe', ['Core.cs','Tests.cs'], 'exe')
shutil.copy2(REF/'x64/Release/OptiScaler.dll', APP/'runtime/dxgi.dll')
shutil.copy2(LAB/'native-nr-3090-v1/nvngx_dlssnr.dll', APP/'runtime/nvngx_dlssnr.dll')
shutil.copy2(LAB/'native-nr-3090-v1/nvngx.dll_dlssnr.dll', APP/'runtime/nvngx.dll_dlssnr.dll')
shutil.copy2(REF/'OptiScaler.ini', APP/'runtime/OptiScaler.ini')
shutil.copy2(ROOT/'README.txt',APP/'README.txt')
shutil.copy2(REF/'LICENSE',APP/'COPYING.txt')
shutil.copytree(REF/'Licenses',APP/'Licenses',dirs_exist_ok=True)
(APP/'prerequisites').mkdir(exist_ok=True)
shutil.copy2(Path.home()/'Downloads/616.56-desktop-win10-win11-64bit-international-dch-whql.exe', APP/'prerequisites/NVIDIA-616.56.exe')
shutil.copy2(r'C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Redist\MSVC\14.51.36231\vc_redist.x64.exe', APP/'prerequisites/vc_redist.x64.exe')
shutil.copy2(ROOT.parent/'driver-transition/61656/EULA.txt', APP/'Licenses/NVIDIA-driver-EULA.txt')
sourcezip=DIST/'NRStudio-1.1.0-corresponding-source.zip'
if (APP/'Source.zip').exists():
    shutil.move(str(APP/'Source.zip'), str(sourcezip))
if not sourcezip.exists():
    print('Archiving corresponding OptiScaler source and pinned dependencies...', flush=True)
    paths=subprocess.check_output(['git','ls-files','--recurse-submodules','-z'],cwd=REF).decode().split('\0')
    with zipfile.ZipFile(sourcezip,'w',zipfile.ZIP_DEFLATED,compresslevel=3) as z:
        for relative in paths:
            p=REF/relative
            if p.is_file(): z.write(p,'OptiScaler/'+relative)
        for name in ['native_forwarder.cpp','prepare_native_forwarder.py','build_native_probe.cmd','native_probe.cpp']:
            z.write(LAB/'native-nr-3090-v1'/name,'NativeForwarder/'+name)
        for name in ['build_capture_fix.cmd','capture_readback_test.cpp','test_capture_fix.cmd']:
            z.write(REF/name,'OptiScaler/'+name)
with zipfile.ZipFile(APP/'NRStudio-source.zip','w',zipfile.ZIP_DEFLATED) as z:
    for p in (ROOT/'src').glob('*'): z.write(p,'src/'+p.name)
    for name in ['build.py','README.txt']: z.write(ROOT/name,name)
manifest={str(p.relative_to(APP)).replace('\\','/'):hashlib.sha256(p.read_bytes()).hexdigest() for p in APP.rglob('*') if p.is_file() and p.name!='SHA256.json'}
(APP/'SHA256.json').write_text(json.dumps(manifest,indent=2))
portable=DIST/'NRStudio-1.1.0-complete-portable.zip'
with zipfile.ZipFile(portable,'w',zipfile.ZIP_DEFLATED,compresslevel=3) as z:
    for p in APP.rglob('*'):
        if p.is_file(): z.write(p,p.relative_to(APP),compress_type=zipfile.ZIP_STORED if p.parent.name=='prerequisites' else zipfile.ZIP_DEFLATED)
compile(DIST/'NRStudio-Setup-1.1.0-Complete.exe',['Core.cs','Prerequisites.cs','Setup.cs'],extra=['/resource:'+str(portable)+',app.zip'])
print('Built installer:', DIST/'NRStudio-Setup-1.1.0-Complete.exe',flush=True)
