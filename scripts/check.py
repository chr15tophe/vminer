#!/usr/bin/env python3
# This little script will quickly and easily verify a nonce.

import hashlib
import sys

if len(sys.argv) != 4:
    print(f"usage: {sys.argv[0]} <username> <vminer.conf> <nonce>")
    exit()

with open(sys.argv[2], "r") as fh:
    exec(fh.read()) # Don't do this, this is dumb.

MAGIC_BYTES = b"ISVENONACOINBEST"
MINER = sys.argv[1].encode().ljust(16, b' ')
PREVIOUS_BLOCK_HASH = PREVIOUS_BLOCK_HASH.encode()
MESSAGE = MESSAGE.encode().ljust(64, b' ')
THRESHOLD = int(THRESHOLD, 16)
NONCE = sys.argv[3].encode()

# Calculate the block signature.

block = hashlib.sha256(
    MAGIC_BYTES + PREVIOUS_BLOCK_HASH + MESSAGE.ljust(64, b" ")
).digest()

signature = hashlib.sha256(block + MINER + NONCE).hexdigest()
if int(signature, 16) < THRESHOLD:
    print(f"Nonce '{sys.argv[3]}' is valid!")

print(f"Signature is {signature}")

