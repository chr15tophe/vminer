import sys
import hashlib

BLOCK_HASH = bytes.fromhex("D32DD2A70420219F42DAE0EB9E091CA8606D47B1A3EBB7DDB5A35B9220ACB169")
USERNAME = (b"hugh").ljust(16, b' ')
THRESHOLD = 0x0000020000000000000000000000000000000000000000000000000000000000

for ln in sys.stdin.readlines():
    if "SUCCESS" in ln:
        nonce = ln[21:21+16]

        signature = hashlib.sha256(BLOCK_HASH + USERNAME + nonce.encode()).hexdigest()
        if int(signature, 16) < THRESHOLD:
            print(f"Nonce '{nonce}' is valid!")
        else:
            print(f"Nonce '{nonce}' is NOT valid!")
