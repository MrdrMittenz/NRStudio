from pathlib import Path
import re,subprocess
root=Path(__file__).resolve().parent
ref=root.parents[1]/'optiscaler-nr-reference/OptiScaler/shaders/dlssnr/precompile'
source=(ref/'dlssnr.hlsl').read_text()
old='HueOkLab(model * ratio, model)'
assert source.count(old)==1
(root/'candidate.hlsl').write_text(source.replace(old,'ClampAp1(model * ratio)'))
header=(ref/'DlssNr_Shader.h').read_text()
(root/'baseline.cso').write_bytes(bytes(int(v,16) for v in re.findall(r'0x([0-9a-fA-F]{2})',header)))
fxc=r'C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\fxc.exe'
subprocess.run([fxc,'/nologo','/T','cs_5_0','/E','CSMain','/O3','/Fo',str(root/'candidate.cso'),str(root/'candidate.hlsl')],check=True)
