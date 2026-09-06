from pathlib import Path
root=Path(__file__).resolve().parent
source=(root.parents[1]/'fp8-ampere-candidate/native-nr-3090-v1/native_probe.cpp').read_text()
changes={
 'int wmain(int argc,wchar_t**argv) {':'#include "direct_trace.h"\nint wmain(int argc,wchar_t**argv) {\n NRDirectTrace::Install();',
 'hr(dev->CreateCommandQueue(&qd,IID_PPV_ARGS(&queue)),"queue");':'hr(dev->CreateCommandQueue(&qd,IID_PPV_ARGS(&queue)),"queue");\n ChainTiming::Initialize(queue);',
 'for(int frame=0;frame<frames;frame++){':'for(int frame=0;frame<frames;frame++){\n NRDirectTrace::frame=frame;',
 '        void* p=nullptr;hr(readback->Map':'        if(NRDirectTrace::timingOnly && frame+1<frames)continue;\n        void* p=nullptr;hr(readback->Map',
 'if(frame==frames-1){FILE* file':'NRDirectTrace::HashFrame(p,W,H,footprints[3].Footprint.RowPitch);\n if(frame==frames-1){FILE* file',
 '    return 0;\n}':'    ChainTiming::completed=true;\n    return 0;\n}',
}
for old,new in changes.items():
 assert source.count(old)==1,old
 source=source.replace(old,new)
(root/'direct_probe.cpp').write_text(source)
build=(root/'build_trace.cmd').read_text().replace('trace_probe','direct_probe').replace('d3d12.lib dxgi.lib','d3d12.lib dxgi.lib bcrypt.lib')
(root/'build_direct_probe.cmd').write_text(build)
