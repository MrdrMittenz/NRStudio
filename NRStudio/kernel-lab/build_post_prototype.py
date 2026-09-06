"""Build an isolated compatibility prototype; proprietary PTX/cubin stay local.

This does not patch the model. Default mode retains helper calls; --inline
expands their PTX bodies with distinct registers and labels for comparison.
"""
import hashlib
import json
import re
import subprocess
import sys
from pathlib import Path

root = Path(__file__).resolve().parent
inline = '--inline' in sys.argv[1:]
helper_source = (root / 'post_helpers.ptx').read_text()
serial = 0

def inline_helper(name, arguments, output, bits):
    global serial
    serial += 1
    prefix = f'nr{serial}_'
    header = re.search(r'\.visible \.func\s*\([^)]*\)\s*' + name + r'\(', helper_source)
    if not header:
        raise ValueError('Missing helper ' + name)
    start = helper_source.index('{', header.end())
    level = 1
    end = start + 1
    while level:
        level += (helper_source[end] == '{') - (helper_source[end] == '}')
        end += 1
    text = helper_source[start + 1:end - 1]
    special = {'tid', 'ntid', 'ctaid', 'nctaid', 'laneid', 'warpid', 'warpsize'}
    text = re.sub(r'%([A-Za-z_]\w*)', lambda m: m[0] if m[1] in special else '%' + prefix + m[1], text)
    text = re.sub(r'\$L\w+', lambda m: '$' + prefix + m[0][1:], text)
    def load(m):
        destination, index = m.groups()
        return f'mov.b32 {destination}, {arguments[int(index)]};'
    text, loaded = re.subn(r'ld\.param\.u32\s+(%\w+),\s*\[' + name + r'_param_(\d+)\];', load, text)
    if loaded != len(arguments):
        raise ValueError(f'Unexpected loads in {name}: {loaded}')
    text, stored = re.subn(r'st\.param\.b' + str(bits) + r'\s+\[func_retval0\+0\],\s*(%\w+);',
                          lambda m: f'mov.b{bits} {output}, {m[1]};', text)
    if stored != 1 or 'ld.param' in text or 'st.param' in text or 'call.' in text:
        raise ValueError('Unsupported inline helper body ' + name)
    label = prefix + 'end'
    text = re.sub(r'\bret;', 'bra ' + label + ';', text)
    return '{\n' + text + '\n' + label + ':\n}'
original = root / 'module-0.1.sm_120.ptx'
source = original.read_text()
name = 'cc_tinlayout_fused_post_block_swin_1h_32_fp8'
begin = source.index('.visible .entry ' + name + '(')
end = source.find('.visible .entry ', begin + 1)
body = source[begin:end if end >= 0 else None]
counts = {'encode': 0, 'decode': 0, 'mma': 0}

def conversion(match):
    kind, dst, src = match.groups()
    encode = kind == 'rn.satfinite.e4m3x2.f16x2'
    counts['encode' if encode else 'decode'] += 1
    func = 'nrEncode' if encode else 'nrDecode'
    before = f'mov.b32 v, {src};' if encode else f'cvt.u32.u16 v, {src};'
    after = f'cvt.u16.u32 {dst}, result;' if encode else f'mov.b32 {dst}, result;'
    if inline:
        return '{\n.reg .b32 v, result;\n' + before + '\n' + inline_helper(func, ['v'], 'result', 32) + '\n' + after + '\n}'
    return ('{\n.reg .b32 v, result;\n.param .b32 arg;\n.param .b32 retv;\n'
            f'{before}\nst.param.b32 [arg], v;\ncall.uni (retv), {func}, (arg);\n'
            f'ld.param.b32 result, [retv];\n{after}\n}}')

body = re.sub(r'cvt\.(rn\.satfinite\.e4m3x2\.f16x2|rn\.f16x2\.e4m3x2)\s+(%\w+),\s*(%\w+);', conversion, body)

def mma(match):
    counts['mma'] += 1
    d, a, b, c = ([v.strip() for v in group.split(',')] for group in match.groups())
    if list(map(len, [d, a, b, c])) != [2, 4, 2, 2]:
        raise ValueError('Unexpected MMA operands')
    args = a + b + c
    if inline:
        return '{\n.reg .b64 result;\n' + inline_helper('nrMma', args, 'result', 64) + f'\nmov.b64 {{{d[0]},{d[1]}}}, result;\n}}'
    lines = ['{', '.reg .b64 result;', '.param .b64 retv;']
    for i, value in enumerate(args):
        lines += [f'.param .b32 arg{i};', f'st.param.b32 [arg{i}], {value};']
    lines += ['call.uni (retv), nrMma, (' + ','.join(f'arg{i}' for i in range(8)) + ');',
              'ld.param.b64 result, [retv];', f'mov.b64 {{{d[0]},{d[1]}}}, result;', '}']
    return '\n'.join(lines)

body = re.sub(r'mma\.sync\.aligned\.m16n8k32\.row\.col\.f16\.e4m3\.e4m3\.f16\s*'
              r'\{([^}]+)\},\s*\{([^}]+)\},\s*\{([^}]+)\},\s*\{([^}]+)\};', mma, body)
if counts != {'encode': 388, 'decode': 40, 'mma': 256}:
    raise ValueError(f'Unexpected transformation counts: {counts}')
if re.search(r'(?m)^\s*(?:cvt|mma)\.[^\n]*e4m3', body):
    raise ValueError('Untranslated FP8 arithmetic')
helpers = helper_source
helpers = helpers[helpers.index('.visible .func'):]
output = root / 'post-prototype.ptx'
output.write_text('.version 9.3\n.target sm_86\n.address_size 64\n' + helpers + '\n' + body)
command = [r'C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.3\bin\ptxas.exe',
           '-arch=sm_86', '-O3', '-v', str(output), '-o', str(root / 'post-prototype.cubin')]
result = subprocess.run(command, capture_output=True, text=True)
(root / 'post-prototype-build.txt').write_text(result.stdout + result.stderr)
(root / 'post-prototype-build.json').write_text(json.dumps(dict(
    input_sha256=hashlib.sha256(original.read_bytes()).hexdigest(),
    helper_sha256=hashlib.sha256((root / 'post_helpers.ptx').read_bytes()).hexdigest(),
    inline_helpers=inline,
    transformed=counts, exit_code=result.returncode,
    limitations='Compatibility prototype; helper expansion recorded above. Not deployed; '
    'numerical and performance acceptance remain required.'), indent=2))
print(result.stdout + result.stderr)
raise SystemExit(result.returncode)
