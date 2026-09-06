from pathlib import Path
import re
root=Path.cwd();sdk=Path(r'C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\um\d3d12.h').read_text()
s=sdk.split('typedef struct ID3D12GraphicsCommandListVtbl',1)[1].split('} ID3D12GraphicsCommandListVtbl;',1)[0]
methods=re.findall(r'(\w+)\s*\(\s*STDMETHODCALLTYPE\s*\*(\w+)\s*\)\s*\((.*?)\);',s,re.S)
assert len(methods)==60,len(methods)
lines=['// Generated from installed Windows SDK signatures; isolated probe only.']
for slot,(ret,name,args) in enumerate(methods):
 if slot<9:continue
 args=re.sub(r'/\*.*?\*/','',args,flags=re.S)
 while True:
  annotation=re.search(r'_\w+_\(',args)
  if not annotation:break
  depth=1;end=annotation.end()
  while depth:
   depth+=(args[end]=='(')-(args[end]==')');end+=1
  args=args[:annotation.start()]+args[end:]
 args=re.sub(r'\b_\w+_\b','',args);args=' '.join(args.split())
 params=args.split(',');names=[]
 for param in params:
  names.append(re.search(r'(\w+)\s*(?:\[[^]]*\])?\s*$',param)[1])
 lines += [f'using D3D_{name}={ret}(STDMETHODCALLTYPE*)({args});',f'static D3D_{name} original_{name};',f'static {ret} STDMETHODCALLTYPE Audit_{name}({args})'+'{'+f'Boundary(This,"{name}");return original_{name}('+','.join(names)+');}']
lines+=['static void AttachD3D(ID3D12GraphicsCommandList*c){auto v=*reinterpret_cast<void***>(c);']
for slot,(ret,name,args) in enumerate(methods):
 if slot<9:continue
 lines+=[f'original_{name}=reinterpret_cast<D3D_{name}>(v[{slot}]);',f'if(DetourAttach(reinterpret_cast<PVOID*>(&original_{name}),Audit_{name})!=NO_ERROR)exit(60);']
lines+=['}']
(root/'launch_boundaries.h').write_text('\n'.join(lines)+'\n')
