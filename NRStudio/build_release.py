"""Build NR Studio 1.3.4 from pinned local runtime/vendor inputs; never deploy games."""
from pathlib import Path
import argparse
import hashlib
import json
import os
import shutil
import subprocess
import zipfile

ROOT = Path(__file__).resolve().parent
VERSION = '1.3.4'
INSTALLED = Path.home() / 'AppData/Local/Programs/NRStudio'
EXPECTED = {
    'dxgi.dll': '116df4a09899236f90862919475b0c4756fae0381a2d2378c48f047701354c5e',
    'nvngx.dll_dlssnr.dll': 'fd5ded22a0435303c60df26ce130411afe6c2fe2d99c6990a6c9ed0d70bc0950',
    'nvngx_dlssnr.dll': '8270b350cd82de5ce89806872cdd6b6a9249b80836b91bbeb3573470744cc206',
}
VENDOR = {
    'NVIDIA-616.64.exe': '36584e5df1dc048df5c677e9591295b96b870e7a6a0ad1461457b89de8bc1b37',
    'vc_redist.x64.exe': '90e48ade404e4576d023abfa374f323555f233982a8805ea9ac63dca9491a16b',
}

def sha(path):
    with Path(path).open('rb') as f: return hashlib.file_digest(f, 'sha256').hexdigest()

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output-dir', type=Path, default=ROOT / ('build-release-' + VERSION))
    p.add_argument('--runtime-dir', type=Path, default=INSTALLED / 'runtime')
    p.add_argument('--vendor-dir', type=Path, default=INSTALLED / 'prerequisites')
    p.add_argument('--driver', type=Path)
    p.add_argument('--csc', type=Path, default=Path('C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/MSBuild/Current/Bin/Roslyn/csc.exe'))
    args = p.parse_args()
    stage = args.output_dir.resolve(); app = stage / 'NRStudio'
    app.mkdir(parents=True, exist_ok=True)
    (app / 'runtime').mkdir(exist_ok=True); (app / 'prerequisites').mkdir(exist_ok=True)
    for name, expected in EXPECTED.items():
        source = args.runtime_dir / name
        if sha(source) != expected: raise RuntimeError('Unverified runtime input: ' + str(source))
        shutil.copy2(source, app / 'runtime' / name)
    shutil.copy2(args.runtime_dir / 'OptiScaler.ini', app / 'runtime/OptiScaler.ini')
    for name, expected in VENDOR.items():
        source = args.driver if name.startswith('NVIDIA-') and args.driver else args.vendor_dir / name
        if sha(source) != expected: raise RuntimeError('Unverified vendor input: ' + str(source))
        shutil.copy2(source, app / 'prerequisites' / name)
    for name in ['README.txt', 'MODEL-AUDIT.md', 'RELEASE-NOTES.md', 'BUILDING.md']:
        shutil.copy2(ROOT / name, app / name)
    shutil.copy2(ROOT / 'assets/COPYING.txt', app / 'COPYING.txt')
    shutil.copytree(ROOT / 'assets/Licenses', app / 'Licenses', dirs_exist_ok=True)

    def compile(output, files, console=False, resource=None):
        command = [str(args.csc), '/nologo', '/optimize+', '/platform:x64',
            '/target:' + ('exe' if console else 'winexe'), '/out:' + str(output),
            '/win32manifest:' + str(ROOT / 'src/app.manifest')]
        command += ['/r:' + n for n in ['System.Windows.Forms.dll', 'System.Drawing.dll',
            'System.Web.Extensions.dll', 'System.IO.Compression.dll', 'System.IO.Compression.FileSystem.dll', 'Microsoft.CSharp.dll']]
        if resource: command += ['/resource:' + str(resource) + ',app.zip']
        subprocess.run(command + [str(ROOT / 'src' / n) for n in ['AssemblyInfo.cs'] + files], check=True)

    compile(app / 'NRStudio.exe', ['Core.cs','Prerequisites.cs','RuntimeDiagnostics.cs','GpuMonitor.cs','Performance.cs','Help.cs','App.cs'])
    compile(app / 'Uninstall.exe', ['Core.cs','Prerequisites.cs','Setup.cs'])
    compile(stage / 'Tests.exe', ['Core.cs','Tests.cs'], True)
    compile(stage / 'FeederTests.exe', ['Core.cs','FeederTests.cs'], True)
    compile(stage / 'DiagnosticsTests.exe', ['RuntimeDiagnostics.cs','DiagnosticsTests.cs'], True)
    compile(stage / 'GpuMonitorTests.exe', ['GpuMonitor.cs','GpuMonitorTests.cs'], True)
    temp = stage / 'test-fixtures'; temp.mkdir(exist_ok=True)
    env = dict(os.environ, TEMP=str(temp), TMP=str(temp))
    subprocess.run([str(stage / 'Tests.exe'), str(app / 'runtime'), str(app), str(app / 'Uninstall.exe')], check=True, env=env)
    subprocess.run([str(stage / 'FeederTests.exe'), str(app / 'runtime'), str(app / 'Uninstall.exe')], check=True, env=env)
    subprocess.run([str(stage / 'DiagnosticsTests.exe')], check=True)
    subprocess.run([str(stage / 'GpuMonitorTests.exe')], check=True)

    with zipfile.ZipFile(app / 'NRStudio-source.zip', 'w', zipfile.ZIP_DEFLATED) as z:
        for folder in ['src','assets','RuntimeSource']:
            for path in sorted((ROOT / folder).rglob('*')):
                if path.is_file(): z.write(path, path.relative_to(ROOT).as_posix())
        for name in ['build.py','build_release.py','README.txt','MODEL-AUDIT.md','RELEASE-NOTES.md','BUILDING.md']:
            z.write(ROOT / name, name)
    manifest = {path.relative_to(app).as_posix(): sha(path) for path in sorted(app.rglob('*')) if path.is_file() and path.name != 'SHA256.json'}
    (app / 'SHA256.json').write_text(json.dumps(manifest, indent=2), encoding='utf-8')
    payload = stage / 'app.zip'
    with zipfile.ZipFile(payload, 'w', zipfile.ZIP_DEFLATED, compresslevel=3) as z:
        for path in sorted(app.rglob('*')):
            if path.is_file():
                rel = path.relative_to(app).as_posix()
                z.write(path, rel, compress_type=zipfile.ZIP_STORED if rel.startswith('prerequisites/') else zipfile.ZIP_DEFLATED)
    installer = stage / ('NRStudio-Setup-' + VERSION + '-Experimental.exe')
    compile(installer, ['Core.cs','Prerequisites.cs','Setup.cs'], resource=payload)
    with zipfile.ZipFile(installer) as z:
        hashes = json.loads(z.read('SHA256.json'))
        names = {e.filename for e in z.infolist() if not e.is_dir()}
        if names != set(hashes) | {'SHA256.json'}: raise RuntimeError('Payload manifest membership mismatch')
        for name, expected in hashes.items():
            with z.open(name) as f:
                if hashlib.file_digest(f, 'sha256').hexdigest() != expected: raise RuntimeError('Payload mismatch: ' + name)
        for name, expected in EXPECTED.items():
            if hashes['runtime/' + name] != expected: raise RuntimeError('Packaged runtime mismatch')
        if 'prerequisites/NVIDIA-616.56.exe' in names: raise RuntimeError('Obsolete driver in payload')
    result = {'version':VERSION, 'installer':str(installer), 'sha256':sha(installer),
        'bytes':installer.stat().st_size, 'payload_files_verified':len(manifest), 'runtime':EXPECTED,
        'vendor':VENDOR, 'checks':{'app':35,'diagnostics':7,'monitor':12},
        'quality_default':'Current composition, model resolution 1.0, after upscaling',
        'residual_quality_status':'Experimental; full-resolution visual equivalence unproven'}
    (stage / 'release.json').write_text(json.dumps(result, indent=2), encoding='utf-8')
    print(json.dumps(result, indent=2), flush=True)

if __name__ == '__main__': main()
