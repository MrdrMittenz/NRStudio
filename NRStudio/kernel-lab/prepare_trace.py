from pathlib import Path
root=Path(__file__).resolve().parent
original=root.parents[1]/'fp8-ampere-candidate/native-nr-3090-v1/native_probe.cpp'
source=original.read_text()
source=source.replace('int wmain(int argc,wchar_t**argv) {','#include "trace_launches.h"\nint wmain(int argc,wchar_t**argv) {\n    NRTrace::Install();')
source=source.replace('hr(dev->CreateCommandQueue(&qd,IID_PPV_ARGS(&queue)),"queue");', 'hr(dev->CreateCommandQueue(&qd,IID_PPV_ARGS(&queue)),"queue");\n    ChainTiming::Initialize(queue);')
source=source.replace('    return 0;\n}', '    ChainTiming::completed = true;\n    return 0;\n}')
(root/'trace_probe.cpp').write_text(source)
