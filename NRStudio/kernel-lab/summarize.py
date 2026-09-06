from pathlib import Path
import subprocess,re,json,statistics
root=Path(__file__).resolve().parent
modules=root.parents[1]/'fp8-ampere-candidate/tools/nr_recovery/modules'
tool=r'C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.3\bin\cuobjdump.exe'
names=['cc_cb_clear','cc_dec_input_upsample_1024_512','cc_dec_input_upsample_1024_512_fp8']
reports=[]
for name in names:
    result=subprocess.run([tool,'--dump-sass','--function',name,str(modules/'module-6.2.sm_86.cubin')],capture_output=True,text=True,check=True)
    # Mnemonics only; code size and instruction counts do not establish runtime cost.
    ops=re.findall(r'/\*[0-9a-f]+\*/\s+(?:@!?P\d+\s+)?([A-Z][A-Z0-9_.]*)',result.stdout)
    families={}
    for op in ops: families[op.split('.')[0]]=families.get(op.split('.')[0],0)+1
    reports.append(dict(kernel=name,instructions=len(ops),families=families))
    if name=='cc_cb_clear': (root/'clear-disassembly.txt').write_text(result.stdout)
(root/'instruction-inventory.json').write_text(json.dumps(reports,indent=2))
text=(root/'clear-results.txt').read_text(encoding='utf-8-sig')
rows=re.findall(r'timing n=(\d+) run=(\d+) variant=(\d+) mean_us=([0-9.]+)',text)
summary=[]
for n in sorted({int(x[0]) for x in rows}):
    summary.append({'elements':n,'median_us':{['original','vector','cuda_memset'][v]:statistics.median(float(t) for size,_,variant,t in rows if int(size)==n and int(variant)==v) for v in range(3)}})
(root/'timing-summary.json').write_text(json.dumps(summary,indent=2))
print(json.dumps(summary[-3:],indent=2))
print(json.dumps([{k:v for k,v in r.items() if k!='families'} for r in reports],indent=2))
