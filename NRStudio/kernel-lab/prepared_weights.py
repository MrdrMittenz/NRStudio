"""Isolated post-block experiment; generated vendor-derived PTX stays local.

Keep the original ABI fields and append a prepared-weight pointer at byte 184.
Each packed 32-bit word maps to two half2 words (even bytes, then odd bytes).
Only direct vector-loaded operands used by FP8 MMA are substituted.
"""
import re

MMA = (r'mma\.sync\.aligned\.m16n8k32\.row\.col\.f16\.e4m3\.e4m3\.f16\s*'
       r'\{([^}]+)\},\s*\{([^}]+)\},\s*\{([^}]+)\},\s*\{([^}]+)\};')

def prepare(body):
    wanted = set()
    for m in re.finditer(MMA, body):
        wanted.update(re.findall(r'%r\d+', m[3]))
    mapping = {}
    definitions = {}
    for line in body.splitlines():
        m = re.fullmatch(r'(mov\.u32|mul\.wide\.s32|add\.s64) (%\w+), (.*);', line.strip())
        if m:
            definitions[m[2]] = (m[1], m[3].split(', '))
    def value(reg, lane):
        if reg == '%rd6': return 0
        if reg == '%laneid': return lane
        if not reg.startswith('%'): return int(reg)
        op,args = definitions[reg]
        values = [value(r,lane) for r in args]
        if op == 'mov.u32': return values[0]
        if op == 'mul.wide.s32': return values[0]*values[1]
        if op == 'add.s64': return sum(values)
        raise ValueError(op)
    source_ends = []
    load_pattern = r'ld\.weak\.global\.ca\.v[24]\.u32\s*\{([^}]+)\},\s*\[(%rd\d+)\];'
    # Establish pointer ancestry before accepting a load. This kernel uses rd6
    # for the weight parameter; reject unfamiliar arithmetic rather than guess.
    derived = {'%rd6'}
    for line in body.splitlines():
        m = re.fullmatch(r'add\.s64 (%rd\d+), (%rd\d+), (?:%rd\d+|\d+);', line.strip())
        if m and m[2] in derived:
            derived.add(m[1])
    assert 'ld.param.b64 %rd6, [%rd11+24];' in body
    def load(m):
        regs = re.findall(r'%r\d+', m[1])
        selected = [(i,r) for i,r in enumerate(regs) if r in wanted]
        if not selected:
            return m[0]
        assert m[2] in derived, 'Unproven weight address ' + m[2]
        lines = [m[0], '{ .reg .b64 off, ptr;',
                 f'sub.u64 off, {m[2]}, %rd6;', 'shl.b64 off, off, 1;',
                 'add.u64 ptr, %nrPrepared, off;']
        for i,r in selected:
            assert r not in mapping
            pair = ('%nrW' + r[2:] + 'e', '%nrW' + r[2:] + 'o')
            mapping[r] = pair
            source_ends.extend(value(m[2],lane)+i*4+4 for lane in range(32))
            lines.append(f'ld.global.v2.u32 {{{pair[0]}, {pair[1]}}}, [ptr+{i*8}];')
        return '\n'.join(lines + ['}'])
    body = re.sub(load_pattern, load, body)
    assert len(mapping) == 104, f'Unexpected prepared operands: {len(mapping)}'
    print('prepared weight source extent:', max(source_ends), 'bytes')
    from pathlib import Path
    Path(__file__).with_name('prepared_weight_extent.h').write_text(
        '#pragma once\nstatic constexpr unsigned NR_PREPARED_SOURCE_BYTES = '+str(max(source_ends))+';\n')
    body = body.replace('_param_0[184]', '_param_0[192]', 1)
    marker = 'mov.b64 %rd11, '
    pos = body.index(marker)
    declarations = '.reg .b64 %nrPrepared;\n' + ''.join(
        f'.reg .b32 {a}, {b};\n' for a,b in mapping.values())
    declarations += 'ld.param.b64 %nrPrepared, [cc_tinlayout_fused_post_block_swin_1h_32_fp8_param_0+184];\n'
    body = body[:pos] + declarations + body[pos:]
    return body, mapping

def mma(d, a, b, c, mapping, activations=None):
    """Preserve the accepted post-word-raw accumulation and byte partition."""
    lines = ['{', '.reg .b32 a0,a1,a2,a3,b0,b1,t,s,c0,c1,scale;',
             'mov.b32 scale, 0x5c005c00;',
             f'mov.b32 c0, {c[0]};', f'mov.b32 c1, {c[1]};']
    def decode(dst, src, part):
        if activations and src in activations:
            low,high=activations[src]
            selector='0x5410' if part==0 else '0x7632'
            return [f'prmt.b32 {dst}, {low}, {high}, {selector};']
        if src in mapping:
            return [f'mov.b32 {dst}, {mapping[src][part]};']
        return [f'shr.u32 s, {src}, {part*8};',
                'and.b32 t, s, 0x007f007f;', 'shl.b32 t, t, 7;',
                'and.b32 s, s, 0x00800080;', 'shl.b32 s, s, 8;',
                f'or.b32 {dst}, t, s;', f'mul.f16x2 {dst}, {dst}, scale;']
    for part in range(2):
        for dst,src,half in [('a0',a[part*2],0),('a1',a[part*2+1],0),
                             ('a2',a[part*2],1),('a3',a[part*2+1],1),
                             ('b0',b[part],0),('b1',b[part],1)]:
            lines += decode(dst,src,half)
        lines.append('mma.sync.aligned.m16n8k16.row.col.f16.f16.f16.f16 '
                     '{c0,c1}, {a0,a1,a2,a3}, {b0,b1}, {c0,c1};')
    return '\n'.join(lines + [f'mov.b32 {d[0]}, c0;',f'mov.b32 {d[1]}, c1;','}'])
