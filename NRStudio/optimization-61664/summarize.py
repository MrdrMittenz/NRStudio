from pathlib import Path
import re,json,statistics
root=Path(__file__).resolve().parent
result={}
for name in ['shader-results.txt','shader-results-half.txt']:
    data=(root/name).read_bytes()
    text=data.decode('utf-16' if data.startswith(b'\xff\xfe') else 'utf-8')
    rows=re.findall(r'timing run=(\d+) variant=(\w+) gpu_ms=([0-9.]+)',text)
    result[name]={variant:{'mean_ms':statistics.mean(float(ms) for _,v,ms in rows if v==variant),'median_ms':statistics.median(float(ms) for _,v,ms in rows if v==variant)} for variant in ['baseline','candidate']}
    (root/name).write_bytes(text.encode('utf-8'))
(root/'shader-summary.json').write_text(json.dumps(result,indent=2))
print(json.dumps(result,indent=2))
