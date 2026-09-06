"""Keep rounded activation values in half2 registers inside the same kernel.

Only direct encode/pack -> MMA use is replaced. Existing packed values remain
available for all other uses. PTXAS removes dead encoding when appropriate.
"""
import re
from collections import Counter
from prepared_weights import MMA

def analyze(body):
    encoded = {}
    for m in re.finditer(r'cvt\.rn\.satfinite\.e4m3x2\.f16x2\s+(%\w+),\s*(%\w+);',body):
        if m[1] in encoded:
            raise ValueError('Repeated encode destination')
        encoded[m[1]] = m[2]
    wanted=set()
    for m in re.finditer(MMA,body):
        wanted.update(re.findall(r'%r\d+',m[2]+','+m[3]))
    mappings={}
    required=set()
    vector_destinations=set()
    for m in re.finditer(r'(?m)^\s*[\w.]+\s*\{([^}]+)\}',body):
        vector_destinations.update(re.findall(r'%\w+',m[1]))
    for m in re.finditer(r'mov\.b32\s+(%r\d+),\s*\{(%rs\d+),\s*(%rs\d+)\};',body):
        if m[1] in wanted and m[2] in encoded and m[3] in encoded:
            # Some chained kernels merge values loaded from memory and values
            # computed locally into the same register on different branches.
            # A global alias would incorrectly replace the loaded branch too.
            if any(r in vector_destinations for r in (m[1],m[2],m[3])):
                continue
            if m[1] in mappings:
                raise ValueError('Repeated packed destination')
            mappings[m[1]]=('%nrQ'+m[2][1:],'%nrQ'+m[3][1:])
            required.update((m[2],m[3]))
    if not mappings:
        raise ValueError('No direct activation paths found')
    # Reject any additional instruction writing selected packed destinations.
    for reg in mappings:
        definitions=re.findall(r'(?m)^\s*[\w.]+\s+'+re.escape(reg)+r'\s*,',body)
        if len(definitions)!=1:
            raise ValueError('Ambiguous packed register '+reg)
    declarations=''.join(f'.reg .b32 %nrQ{r[1:]};\n' for r in sorted(required))
    start=body.index('{')+1
    return body[:start]+'\n'+declarations+body[start:],mappings,required
