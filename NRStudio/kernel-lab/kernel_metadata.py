"""Inspect function metadata only; this script launches no GPU kernels."""
from pathlib import Path
import ctypes as c,json
root=Path(__file__).resolve().parent
cuda=c.WinDLL('nvcuda.dll')
def function(name,args):
    f=getattr(cuda,name);f.argtypes=args;f.restype=c.c_int;return f
def check(code):
    if code:raise RuntimeError('CUDA result '+str(code))
check(function('cuInit',[c.c_uint])(0))
dev=c.c_int();check(function('cuDeviceGet',[c.POINTER(c.c_int),c.c_int])(c.byref(dev),0))
ctx=c.c_void_p();check(function('cuDevicePrimaryCtxRetain',[c.POINTER(c.c_void_p),c.c_int])(c.byref(ctx),dev))
check(function('cuCtxSetCurrent',[c.c_void_p])(ctx))
module=c.c_void_p()
path=root.parents[1]/'fp8-ampere-candidate/tools/nr_recovery/modules/module-6.2.sm_86.cubin'
check(function('cuModuleLoad',[c.POINTER(c.c_void_p),c.c_char_p])(c.byref(module),str(path).encode()))
get=function('cuModuleGetFunction',[c.POINTER(c.c_void_p),c.c_void_p,c.c_char_p])
param=function('cuFuncGetParamInfo',[c.c_void_p,c.c_size_t,c.POINTER(c.c_size_t),c.POINTER(c.c_size_t)])
attr=function('cuFuncGetAttribute',[c.POINTER(c.c_int),c.c_int,c.c_void_p])
results=[]
for name in ['cc_cb_clear','cc_dec_input_upsample_1024_512','cc_dec_input_upsample_1024_512_fp8']:
    f=c.c_void_p();check(get(c.byref(f),module,name.encode()));row={'kernel':name,'parameters':[],'attributes':{}}
    for index in range(32):
        offset,size=c.c_size_t(),c.c_size_t();code=param(f,index,c.byref(offset),c.byref(size))
        if code==1:break
        check(code);row['parameters'].append(dict(index=index,offset=offset.value,bytes=size.value))
    for code,key in [(0,'max_threads'),(1,'static_shared_bytes'),(3,'local_bytes'),(4,'registers_per_thread')]:
        value=c.c_int();check(attr(c.byref(value),code,f));row['attributes'][key]=value.value
    results.append(row)
check(function('cuModuleUnload',[c.c_void_p])(module))
check(function('cuDevicePrimaryCtxRelease',[c.c_int])(dev))
(root/'kernel-metadata.json').write_text(json.dumps(results,indent=2))
print(json.dumps(results,indent=2))
