from pathlib import Path
p=Path(__file__).resolve().parent/'native_forwarder.cpp';s=(p.parents[2]/'fp8-ampere-candidate/native-nr-3090-v1/native_forwarder.cpp').read_text();s=s.replace('namespace {','#include "post_runtime.h"\n\nnamespace {',1)
needle='    if (!g_snip.initialised && g_snip.init) {'
assert s.count(needle)==1
s=s.replace(needle,'    NRPost::Install(device,snippetPath);\n    NRPost::Scope postScope;\n'+needle)
p.write_text(s)
