from pathlib import Path
root=Path(__file__).resolve().parent
source=(root.parents[1]/'fp8-ampere-candidate/native-nr-3090-v1/native_probe.cpp').read_text()
source=source.replace('int wmain(int argc,wchar_t**argv) {','#include "launch_audit.h"\nint wmain(int argc,wchar_t**argv) {')
needle='hr(dev->CreateCommandList(0,D3D12_COMMAND_LIST_TYPE_DIRECT,alloc,nullptr,IID_PPV_ARGS(&cmd)),"command list");'
assert source.count(needle)==1
source=source.replace(needle,needle+'\n    LaunchAudit::Install(cmd);')
source=source.replace('        rc=evaluate(', '        LaunchAudit::Begin(frame);\n        rc=evaluate(')
source=source.replace('        printf("frame %d evaluate=', '        LaunchAudit::End();\n        printf("frame %d evaluate=')
(root/'launch_audit_probe.cpp').write_text(source)
build=(root/'build_trace.cmd').read_text().replace('trace_probe.cpp','launch_audit_probe.cpp').replace('trace_probe.exe','launch_audit_probe.exe')
(root/'build_launch_audit.cmd').write_text(build)
