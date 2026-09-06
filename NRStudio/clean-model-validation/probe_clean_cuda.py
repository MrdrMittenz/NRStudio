"""Validate native Ampere modules on the installed driver without launching kernels."""
import ctypes as c
import json
from pathlib import Path
import subprocess
import sys
ROOT = Path(__file__).resolve().parent


def child():
    cuda = c.WinDLL('nvcuda.dll')
    def function(name, types):
        f = getattr(cuda, name)
        f.argtypes = types
        f.restype = c.c_int
        return f
    init = function('cuInit', [c.c_uint])
    device_get = function('cuDeviceGet', [c.POINTER(c.c_int), c.c_int])
    retain = function('cuDevicePrimaryCtxRetain', [c.POINTER(c.c_void_p), c.c_int])
    current = function('cuCtxSetCurrent', [c.c_void_p])
    load = function('cuModuleLoadData', [c.POINTER(c.c_void_p), c.c_void_p])
    unload = function('cuModuleUnload', [c.c_void_p])
    error = function('cuGetErrorName', [c.c_int, c.POINTER(c.c_char_p)])
    def check(code):
        if code:
            name = c.c_char_p()
            error(code, c.byref(name))
            raise RuntimeError(f'{code}: {name.value}')
    check(init(0))
    device = c.c_int()
    check(device_get(c.byref(device), 0))
    ctx = c.c_void_p()
    check(retain(c.byref(ctx), device))
    check(current(ctx))
    results = []
    for path in sorted(list((ROOT / 'modules').glob('*.fatbin')) + list((ROOT / 'modules').glob('*.sm_86.cubin'))):
        # The driver's fatbin selector should choose the native SM86 image.
        module = c.c_void_p()
        data = c.create_string_buffer(path.read_bytes())
        code = load(c.byref(module), data)
        name = c.c_char_p()
        error(code, c.byref(name))
        item = dict(file=path.name, result=code, result_name=name.value.decode())
        if not code:
            check(unload(module))
        results.append(item)
    (ROOT / 'cuda-module-validation.json').write_text(json.dumps(results, indent=2))
    print(json.dumps(results, indent=2))
    if any(x['result'] for x in results):
        raise RuntimeError('One or more modules failed to load')


if __name__ == '__main__':
    if '--child' in sys.argv:
        child()
    else:
        result = subprocess.run([sys.executable, str(Path(__file__).resolve()), '--child'],
                                capture_output=True, text=True, timeout=45)
        print(result.stdout, end='')
        print(result.stderr, end='', file=sys.stderr)
        sys.exit(result.returncode)
