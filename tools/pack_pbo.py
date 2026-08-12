from pathlib import Path
import hashlib, struct, sys

def cstring(s):
    return s.encode("utf-8") + b"\0"

def pack(src, dst, prefix):
    src = src.resolve()
    files = [p for p in src.rglob("*") if p.is_file()]
    files.sort(key=lambda p: (0 if p.name=="$PBOPREFIX$" and p.parent==src else 1,
                              str(p.relative_to(src)).lower()))
    h = bytearray()
    h += b"\0" + struct.pack("<5I", 0x56657273,0,0,0,0)
    h += cstring("product")+cstring("dayz ugc")+cstring("prefix")+cstring(prefix)+b"\0"
    payloads = []
    for p in files:
        data = p.read_bytes()
        rel = str(p.relative_to(src)).replace("/", "\\")
        ts = int(p.stat().st_mtime) & 0xffffffff
        h += cstring(rel)
        h += struct.pack("<5I",0,0,0,ts,len(data))
        payloads.append(data)
    h += b"\0" + struct.pack("<5I",0,0,0,0,0)
    unsigned = bytes(h) + b"".join(payloads)
    final = unsigned + b"\0" + hashlib.sha1(unsigned).digest()
    dst.parent.mkdir(parents=True, exist_ok=True)
    dst.write_bytes(final)
    print("Created:", dst)
    print("Entries:", len(files))
    print("SHA256:", hashlib.sha256(final).hexdigest())

if len(sys.argv)!=4:
    print("Usage: python pack_pbo.py <source_dir> <output.pbo> <prefix>")
    raise SystemExit(1)

pack(Path(sys.argv[1]), Path(sys.argv[2]), sys.argv[3])
