"""Extract the seven exact module buffers registered by CCNetwork::init_kernels."""
import hashlib
import json
from pathlib import Path
import struct
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_64, x86_const as x
DLL = Path.home() / 'Desktop/SL 2.13/SL 2.13/nvngx_dlssnr.dll'
ROOT = Path(__file__).resolve().parent

raw = DLL.read_bytes()
assert hashlib.sha256(raw).hexdigest() == 'e16bcf15e16e13f527491cdf7845b2fe6521a738d8f7c9c721866a8496e1fc8e'
pe = pefile.PE(data=raw)
base = pe.OPTIONAL_HEADER.ImageBase
md = Cs(CS_ARCH_X86, CS_MODE_64)
md.detail = True
entries = []
size = None
for ins in md.disasm(pe.get_data(0x3d8c3, 0x3d97d - 0x3d8c3), base + 0x3d8c3):
    ops = ins.operands
    if len(ops) != 2 or ops[1].type != x.X86_OP_MEM or ops[1].mem.base != x.X86_REG_RIP:
        continue
    rva = ins.address + ins.size + ops[1].mem.disp - base
    if ins.mnemonic == 'mov' and ops[0].reg == x.X86_REG_R9:
        size = struct.unpack('<Q', pe.get_data(rva, 8))[0]
    if ins.mnemonic == 'lea' and ops[0].reg == x.X86_REG_R8:
        assert size and 0 < size < 32 * 1024 * 1024
        data = pe.get_data(rva, size)
        assert len(data) == size
        folder = ROOT / 'modules'
        folder.mkdir(exist_ok=True)
        name = f'module-{len(entries)}.bin'
        (folder / name).write_bytes(data)
        magic, version, header_size, payload_size = struct.unpack_from('<IHHQ', data)
        assert magic == 0xba55ed50 and version == 1 and header_size == 16
        fatbin_size = header_size + payload_size
        assert fatbin_size <= len(data)
        (folder / name.replace('.bin', '.fatbin')).write_bytes(data[:fatbin_size])
        entries.append(dict(file=name, rva=hex(rva), size=size,
                            fatbin_size=fatbin_size, trailing_bytes=len(data)-fatbin_size,
                            sha256=hashlib.sha256(data).hexdigest(),
                            header=data[:64].hex(), elf_offset=data.find(b'\x7fELF')))
        size = None
assert len(entries) == 7
(folder / 'manifest.json').write_text(json.dumps(entries, indent=2))
print(json.dumps(entries, indent=2))
