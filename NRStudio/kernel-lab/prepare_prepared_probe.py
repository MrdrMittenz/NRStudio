from pathlib import Path
root=Path(__file__).resolve().parent
source=(root/'trace_launches.h').read_text()
marker='static Chain chain=nullptr;static ChainEx chainEx=nullptr;'
assert source.count(marker)==1
source=source.replace(marker,marker+'\n#include "prepared_launch.h"')
source=source.replace('n?names[k[0].hFunction]:"empty",n?names[k[n-1].hFunction]:"empty"',
 '(n==1&&k[0].hFunction==originalFunction)?(UsePrepared()?"post-prepared":"post-current"):(n?names[k[0].hFunction]:"empty"),n?names[k[n-1].hFunction]:"empty"')
for fn in ('chain','chainEx'):
    old=f'{fn}(c,Substitute(k,n,copy),n)'
    assert source.count(old)==1
    source=source.replace(old,f'LaunchPrepared({fn},c,k,n)')
source=source.replace('if(ChainTiming::completed&&testDevice){','FinishPrepared();\n if(ChainTiming::completed&&testDevice){')
(root/'prepared_trace_launches.h').write_text(source)
probe=(root/'trace_probe.cpp').read_text().replace('#include "trace_launches.h"','#include "prepared_trace_launches.h"')
(root/'prepared_probe.cpp').write_text(probe)
build=(root/'build_trace.cmd').read_text().replace('trace_probe','prepared_probe')
build=build.replace('cl /nologo', '"C:\\Program Files\\NVIDIA GPU Computing Toolkit\\CUDA\\v13.3\\bin\\nvcc.exe" -cubin -arch=sm_86 -O3 prepare_weights.cu -o prepare_weights.cubin\nif errorlevel 1 exit /b %errorlevel%\ncl /nologo')
(root/'build_prepared_probe.cmd').write_text(build)
