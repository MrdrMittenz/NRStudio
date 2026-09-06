from pathlib import Path
root=Path.cwd();s=(root/'launch_audit_probe.cpp').read_text().replace('launch_audit.h','launch_batch.h').replace('LaunchAudit::','LaunchBatch::').replace('LaunchBatch::Begin(frame);','LaunchBatch::Begin(frame,cmd);')
s=s.replace('#include "launch_batch.h"','#include "launch_batch.h"\n#include "chain_timing.h"')
s=s.replace('hr(dev->CreateCommandQueue(&qd,IID_PPV_ARGS(&queue)),"queue");','hr(dev->CreateCommandQueue(&qd,IID_PPV_ARGS(&queue)),"queue");\n    ChainTiming::Initialize(queue);')
s=s.replace('        LaunchBatch::Begin(frame,cmd);','        unsigned timing=ChainTiming::Begin(cmd,1,"model","model");\n        LaunchBatch::Begin(frame,cmd);')
s=s.replace('        LaunchBatch::End();','        LaunchBatch::End();\n        ChainTiming::End(cmd,timing);')
s=s.replace('    return 0;\n}', '    ChainTiming::completed=true;ChainTiming::Finish();\n    return 0;\n}')
(root/'launch_batch_probe.cpp').write_text(s)
(root/'build_launch_batch.cmd').write_text((root/'build_launch_audit.cmd').read_text().replace('launch_audit_probe','launch_batch_probe'))
