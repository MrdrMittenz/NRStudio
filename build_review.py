"""Compile review binaries without installing anything or bundling vendor software."""
from pathlib import Path
import argparse, subprocess
root = Path(__file__).resolve().parent
parser=argparse.ArgumentParser()
parser.add_argument('--csc',required=True,help='Path to the Visual Studio Roslyn csc.exe')
parser.add_argument('--payload',type=Path,help='Optional prepared app.zip payload for the installer')
args=parser.parse_args()
out=root/'build'
out.mkdir(exist_ok=True)
def compile(name,sources,console=False,extra=()):
    command=[args.csc,'/nologo','/optimize+','/platform:x64','/target:'+('exe' if console else 'winexe'),'/out:'+str(out/name),'/win32manifest:'+str(root/'NRStudio/src/app.manifest')]
    command += ['/r:'+name for name in ['System.Windows.Forms.dll','System.Drawing.dll','System.Web.Extensions.dll','System.IO.Compression.dll','System.IO.Compression.FileSystem.dll','Microsoft.CSharp.dll']]
    subprocess.run(command+list(extra)+[str(root/'NRStudio/src'/s) for s in sources],check=True)
compile('NRStudio.exe',['Core.cs','Prerequisites.cs','RuntimeDiagnostics.cs','App.cs'])
compile('Uninstall.exe',['Core.cs','Prerequisites.cs','Setup.cs'])
compile('Tests.exe',['Core.cs','Tests.cs'],True)
compile('DiagnosticsTests.exe',['RuntimeDiagnostics.cs','DiagnosticsTests.cs'],True)
if args.payload:
    payload=args.payload.resolve(strict=True)
    compile('NRStudio-Setup.exe',['Core.cs','Prerequisites.cs','Setup.cs'],extra=['/resource:'+str(payload)+',app.zip'])
print('Review binaries:',out)
