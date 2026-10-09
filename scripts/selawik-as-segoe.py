#!/usr/bin/env python3
"""Make "Segoe UI" fonts from Selawik for prefixes without Windows fonts.

Adobe's UI library asks DirectWrite for the family "Segoe UI". Wine's DirectWrite only
lists families registered under HKLM\\...\\CurrentVersion\\Fonts by their own name and
ignores GDI FontSubstitutes, so a substitute "Segoe UI" -> "Selawik" does not help it.
Selawik is Microsoft's open, metric-compatible fallback for Segoe UI (SIL OFL 1.1); its
licence reserves the name "Selawik" for unmodified copies, so a renamed copy is what it
asks for. This rewrites the family names in each file's 'name' table and writes the
result under the Windows file names; nothing else in the fonts changes.

Usage: selawik-as-segoe.py <output folder> <selawik .ttf files...>
"""
import os
import struct
import sys

WINDOWS_NAMES = {  # Selawik release file -> Segoe UI file name on Windows
    "selawk.ttf": "segoeui.ttf",
    "selawkb.ttf": "segoeuib.ttf",
    "selawkl.ttf": "segoeuil.ttf",
    "selawksl.ttf": "segoeuisl.ttf",
    "selawksb.ttf": "seguisb.ttf",
}
FAMILY_IDS = {1, 3, 4, 16, 18}  # family, unique id, full name, typographic family, compatible full
POSTSCRIPT_ID = 6               # no spaces allowed


def checksum(data):
    data += b"\0" * (-len(data) % 4)
    return sum(struct.unpack(">%dI" % (len(data) // 4), data)) & 0xFFFFFFFF


def rename(name_table):
    fmt, count, _ = struct.unpack(">HHH", name_table[:6])
    records, strings, seen = [], bytearray(), {}
    for i in range(count):
        platform, encoding, language, name_id, length, offset = struct.unpack(
            ">HHHHHH", name_table[6 + 12 * i:18 + 12 * i])
        start = struct.unpack(">H", name_table[4:6])[0] + offset
        raw = name_table[start:start + length]
        if name_id in FAMILY_IDS or name_id == POSTSCRIPT_ID:
            codec = "utf-16-be" if platform in (0, 3) else "latin-1"
            new = "SegoeUI" if name_id == POSTSCRIPT_ID else "Segoe UI"
            raw = raw.decode(codec).replace("Selawik", new).encode(codec)
        if raw not in seen:
            seen[raw] = len(strings)
            strings += raw
        records.append(struct.pack(">HHHHHH", platform, encoding, language, name_id,
                                   len(raw), seen[raw]))
    return struct.pack(">HHH", fmt, count, 6 + 12 * count) + b"".join(records) + bytes(strings)


def convert(data):
    count = struct.unpack(">H", data[4:6])[0]
    tables = {}
    for i in range(count):
        tag, _, offset, length = struct.unpack(">4sIII", data[12 + 16 * i:28 + 16 * i])
        tables[tag] = data[offset:offset + length]
    tables[b"name"] = rename(tables[b"name"])
    tables.pop(b"DSIG", None)  # the signature no longer matches
    tags = sorted(tables)
    selector = max(0, len(tags).bit_length() - 1)
    search = (1 << selector) * 16
    out = bytearray(data[:4] + struct.pack(">HHHH", len(tags), search, selector,
                                           len(tags) * 16 - search))
    offset = 12 + 16 * len(tags)
    body = bytearray()
    for tag in tags:
        table = tables[tag]
        out += tag + struct.pack(">III", checksum(table), offset, len(table))
        if tag == b"head":
            head_at = offset
        body += table + b"\0" * (-len(table) % 4)
        offset += len(table) + (-len(table) % 4)
    out += body
    out[head_at + 8:head_at + 12] = b"\0\0\0\0"  # checkSumAdjustment over the whole file
    out[head_at + 8:head_at + 12] = struct.pack(">I", (0xB1B0AFBA - checksum(bytes(out))) & 0xFFFFFFFF)
    return bytes(out)


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__.strip().splitlines()[-1])
    made = 0
    for src in sys.argv[2:]:
        target = WINDOWS_NAMES.get(os.path.basename(src).lower())
        if not target:
            continue
        with open(src, "rb") as f:
            data = convert(f.read())
        with open(os.path.join(sys.argv[1], target), "wb") as f:
            f.write(data)
        made += 1
    print(made)
    return 0 if made else 1


if __name__ == "__main__":
    sys.exit(main())
