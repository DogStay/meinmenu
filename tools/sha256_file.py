from pathlib import Path
import hashlib,sys
p=Path(sys.argv[1])
print(hashlib.sha256(p.read_bytes()).hexdigest())
