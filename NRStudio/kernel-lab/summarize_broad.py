"""Keep all samples for broader kernel candidates; do not infer game FPS."""
from pathlib import Path
import csv,statistics,json,hashlib
root=Path(__file__).resolve().parent
names=['swin1-base-1440','swin1-test-1440','swin1-r224-1440','pre-test-1440','pre-r224-1440','swin8-test-1440','swin8-r168-1440','broad-base-repeat-1440','swin8-repeat-1440']
kernels={'swin1':'cc_tinlayout_fused_swin_1h_32_1_chained_fp8','pre':'cc_tinlayout_fused_pre_block_swin_1h_32_1_ds_fp8','swin8':'cc_tinlayout_fused_swin_8h_256_8_chained_fp8'}
reference=(root/'swin1-base-1440/native-output-2560x1440.rgba16f').read_bytes()
report={}
for name in names:
    case=root/name;rows=list(csv.DictReader((case/'chain-timings.tsv').open(),delimiter='\t'));frames=[]
    for row in rows:
        if int(row['end_tick'])<int(row['start_tick']):raise ValueError('Bad timestamp')
        if row['first']=='cc_cb_clear':frames.append({})
        k=row['first'];frames[-1][k]=frames[-1].get(k,0)+float(row['gpu_us'])/1000
    run=json.loads((case/'run.json').read_text())
    if run['candidate']:run['cubin_sha256']=hashlib.sha256((root/run['candidate']).read_bytes()).hexdigest()
    report[name]=dict(run=run,final_output_byte_exact=(case/'native-output-2560x1440.rgba16f').read_bytes()==reference,frame_totals_ms=[sum(f.values()) for f in frames],median_warm_total_ms=statistics.median(sum(f.values()) for f in frames[1:]),blocks={label:dict(samples_ms=[f[k] for f in frames],median_warm_ms=statistics.median(f[k] for f in frames[1:])) for label,k in kernels.items()})
(root/'broad-kernel-comparison.json').write_text(json.dumps(report,indent=2)+'\n')
assert all(row['final_output_byte_exact'] for row in report.values())
for name,r in report.items():print(name,'total',round(r['median_warm_total_ms'],3),{k:round(v['median_warm_ms'],3) for k,v in r['blocks'].items()})
