#!/usr/bin/env python3
# This little script will quickly and easily verify a nonce.

import hashlib

NONCES = b"""
INFO: Starting attack at 867Ig0LVTS08QUX5.
SUCCESS: Found nonce 867Ig0LW6rowQUX5.
INFO: Currently averaging ~2447057.303475 h/s...
SUCCESS: Found nonce 867Ig0LWISAmQUX5.
INFO: Currently averaging ~2474921.006709 h/s...
SUCCESS: Found nonce 867Ig0LXB85aQUX5.
SUCCESS: Found nonce 867Ig0LXBG1oQUX5.
INFO: Currently averaging ~2484438.100846 h/s...
""".split(b"\n")

BLOCK_HASH = bytes.fromhex("D32DD2A70420219F42DAE0EB9E091CA8606D47B1A3EBB7DDB5A35B9220ACB169")
MINER = (b"hugh").ljust(16, b" ")
THRESHOLD = 0x0000020000000000000000000000000000000000000000000000000000000000

for NONCE in NONCES:
    if b"SUCCESS: Found nonce" not in NONCE:
        continue

    NONCE = NONCE.split(b" ")[3][:-1]

    signature = hashlib.sha256(BLOCK_HASH + MINER + NONCE).hexdigest()
    if int(signature, 16) < THRESHOLD:
        print(f"Nonce '{NONCE.decode()}' is valid!")
    else:
        print(f"Nonce '{NONCE.decode()}' is valid!")
