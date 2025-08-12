#!/usr/bin/env python3
import re
from pathlib import Path
import sys

# Read gun definitions from source file
src = Path(__file__).resolve().parents[1] / 'src' / 'dopewars.c'
text = src.read_text()

# Extract tuples of (name, price, damage)
pattern = re.compile(r'\{N_\("([^"]+)"\),\s*(\d+),\s*\d+,\s*(\d+)\}')
guns = [(n, int(p), int(d)) for n, p, d in pattern.findall(text)]

if len(guns) < 2:
    print("Not enough gun definitions found")
    sys.exit(1)

# Sort guns by price and verify damage increases with price
sorted_guns = sorted(guns, key=lambda g: g[1])
price_damage_ok = all(sorted_guns[i][2] <= sorted_guns[i+1][2]
                      for i in range(len(sorted_guns)-1))

if not price_damage_ok:
    print("Gun damage does not scale with price", file=sys.stderr)
    sys.exit(1)

sys.exit(0)
