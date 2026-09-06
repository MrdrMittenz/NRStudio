"""Record instruction counts only; input SASS stays local and is not published."""
import collections
import json
import re
from pathlib import Path

root = Path(__file__).resolve().parent
source = (root / 'post-original.sass').read_text(encoding='utf-8-sig')
instructions = []
for line in source.splitlines():
    match = re.search(r'/\*[0-9a-f]+\*/\s+(?:@!?P\d+\s+)?([^;]+);', line)
    if match:
        instructions.append(match[1].strip())
counts = collections.Counter(line.split()[0] for line in instructions)
report = dict(kernel='cc_tinlayout_fused_post_block_swin_1h_32_fp8', architecture=86,
              instruction_count=len(instructions), opcode_counts=dict(counts.most_common()),
              half2_multiply_by_256_count=sum(line.startswith('HMUL2 ') and '256, 256' in line for line in instructions),
              limitations='Static instruction counts are not dynamic execution costs. '
              'The deployed SM86 kernel already uses packed half arithmetic and integer '
              'FP8 conversions; no slow generic CUDA-header conversion fallback was established.')
(root / 'post-instruction-summary.json').write_text(json.dumps(report, indent=2))
print('Instruction count:', len(instructions))
print('Most common:', counts.most_common(8))
