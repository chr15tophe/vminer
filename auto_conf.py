#!/usr/bin/env python3

# This script extracts the previous_block_hash, message, and threshold, given
# the source code of www.activitiesvenona.org/VENONA2021/VENONACOIN/block.

import re
import sys

if len(sys.argv) < 3:
    print(f"usage: {sys.argv[0]} <(input) html source code of /VENONACOIN/block> <(output) .conf to write to>")
    exit()

previous_block_hash = None
message = None
threshold = None

with open(sys.argv[1], "r") as fh:
    for ln in fh.readlines():
        ln = ln.strip()

        if "PREV HASH:" in ln:
            previous_block_hash = re.search("[0-9A-F]{64}", ln).group(0)
        elif "MESSAGE:" in ln:
            ln = ln.replace("&nbsp;", " ")
            message = ln.split("</b>")[-1].strip()
        elif "THRESHOLD" in ln and "BUY" not in ln:
            threshold = re.search("[0-9A-F]{64}", ln).group(0)

# Sanity checks.
assert previous_block_hash is not None
assert message is not None
assert threshold is not None
assert all(c in "0123456789ABCDEF" for c in previous_block_hash)
assert 64 == len(previous_block_hash)
assert 64 >= len(message)
assert all(c in "0123456789ABCDEF" for c in threshold)
assert 64 == len(threshold)

config = f"""\
PREVIOUS_BLOCK_HASH = "{previous_block_hash}"
MESSAGE = "{message}"
THRESHOLD = "{threshold}"
"""

print(f"Extracted the following information from {sys.argv[1]}.")
print(f"")
print(config)

x = input(f"Confirm write to {sys.argv[2]}? (y/N): ")
if len(x) > 0 and x.lower()[0] == "y":
    with open(sys.argv[2], "w") as fh:
        fh.write(config)
else:
    print(f"Aborted!")
