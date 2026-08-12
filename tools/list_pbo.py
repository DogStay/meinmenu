from pathlib import Path
import struct, sys

def cstr(data,pos):
    end=data.find(b"\0",pos)
    if end<0: raise ValueError("bad pbo")
    return data[pos:end].decode("utf-8",errors="replace"), end+1

if len(sys.argv)!=2:
    print("Usage: python list_pbo.py <file.pbo>")
    raise SystemExit(1)

data=Path(sys.argv[1]).read_bytes()
pos=0
name,pos=cstr(data,pos)

if name=="":
    _,_,_,_,_=struct.unpack_from("<5I",data,pos); pos+=20
    while True:
        k,pos=cstr(data,pos)
        if k=="": break
        v,pos=cstr(data,pos)
        print("PROPERTY",k,"=",v)

entries=[]
while True:
    name,pos=cstr(data,pos)
    vals=struct.unpack_from("<5I",data,pos); pos+=20
    if name=="": break
    entries.append((name, vals[4]))

print("FILES:",len(entries))
for n,s in entries:
    print(f"{s:10d}  {n}")
