#!/usr/bin/env python3
"""XPD v1 reference packer + inspector. Python 3.9+, standard library only.

Package ONLY user-supplied matching English Pokemon Red/Blue ROMs and
binary/PNG source art/maps from the provided pokered source archive.
This is an asset container, NOT a complete native-game data translation.
"""
from __future__ import annotations
import argparse, collections, hashlib, io, json, os, struct, sys, zipfile, zlib
from pathlib import Path

MAGIC = b"XPD1"
VERSION = 1
HEADER_SIZE = 96
HEADER_FMT = "<4sHH7I32s"
ENTRY_FMT = "<7I"
ENTRY_SIZE = struct.calcsize(ENTRY_FMT)
BANK_SIZE = 16 * 1024
ROM_SIZE = 1 * 1024 * 1024
KNOWN = {
    "red": ("POKEMON RED", "ea9bcae617fdf159b045185467ae58b2e4a48b9a", 1),
    "blue": ("POKEMON BLUE", "d7037c83e1ae5b39bde3c30787637ba1d4c48ce2", 2),
}
TYPES = {
    "rom": b"ROM ", "bank": b"BANK", "png": b"PNG ",
    "map": b"MAP ", "blockset": b"BSET", "tilemap": b"TILE",
    "rle": b"RLE ", "metadata": b"JSON",
}
ALIAS_FLAG = 1
def crc(data): return zlib.crc32(data) & 0xFFFFFFFF
def align(n, k=16): return (n + k - 1) // k * k
def fourcc(code): return int.from_bytes(TYPES[code], "little")
def hash_rom(data, game):
    want_title, want_sha, _ = KNOWN[game]
    if len(data) != ROM_SIZE:
        raise ValueError("%s: expected exactly 1 MiB, found %d bytes" % (game, len(data)))
    title = data[0x134:0x144].split(b"\0")[0].decode("ascii", "replace")
    sha1 = hashlib.sha1(data).hexdigest()
    if title != want_title or sha1 != want_sha:
        raise ValueError("%s: ROM revision/hash mismatch: title=%r sha1=%s (expected %s)" %
                         (game, title, sha1, want_sha))
    if data[0x147] != 0x13 or data[0x148] != 0x05 or data[0x149] != 0x03:
        raise ValueError("%s: unexpected cartridge header" % game)
    check = 0
    for b in data[0x134:0x14D]: check = (check - b - 1) & 0xFF
    if check != data[0x14D]:
        raise ValueError("%s: invalid ROM header checksum" % game)
    return {"title": title, "sha1": sha1, "sha256": hashlib.sha256(data).hexdigest(),
            "bytes": len(data), "banks": len(data)//BANK_SIZE}

def get_roms(args):
    roms = {}
    if args.roms_zip:
        with zipfile.ZipFile(args.roms_zip) as z:
            for zi in z.infolist():
                if zi.is_dir() or not zi.filename.lower().endswith((".gb", ".gbc")): continue
                data = z.read(zi)
                for game in KNOWN:
                    if hashlib.sha1(data).hexdigest() == KNOWN[game][1]:
                        if game in roms: raise ValueError("duplicate %s ROM" % game)
                        roms[game] = data
    for game in KNOWN:
        path = getattr(args, "rom_" + game, None)
        if path: roms[game] = Path(path).read_bytes()
    games = list(KNOWN) if args.game == "both" else [args.game]
    for game in games:
        if game not in roms: raise ValueError("missing verified %s ROM" % game)
        hash_rom(roms[game], game)
    return {g: roms[g] for g in games}

def include_source_resource(path, game):
    path = path.lower()
    if not (path.startswith("gfx/") or path.startswith("maps/")): return None
    if path.startswith("gfx/pokemon/front_rg/") or path.startswith("gfx/tilesets/tilesets_rg/"):
        return None # Red/Green variants not used by the target Red/Blue build
    # These are version-specific replacements, not generic assets.
    exclusive = (
        ("gfx/intro/red_nidorino", "red"),
        ("gfx/intro/blue_jigglypuff", "blue"),
        ("gfx/sgb/red_border", "red"),
        ("gfx/sgb/blue_border", "blue"),
        ("gfx/slots/red_slots", "red"),
        ("gfx/slots/blue_slots", "blue"),
        ("gfx/title/red_version", "red"),
        ("gfx/title/blue_version", "blue"),
    )
    for prefix, which in exclusive:
        if path.startswith(prefix) and game != which: return None
    suffix = path.rsplit(".", 1)[-1]
    if path.startswith("maps/") and suffix == "blk": return "map"
    if path.startswith("gfx/"):
        return {"png": "png", "bst": "blockset", "tilemap": "tilemap",
                "rle": "rle"}.get(suffix)
    return None

def source_entries(zip_path, game):
    with zipfile.ZipFile(zip_path) as z:
        found = []
        for zi in z.infolist():
            if zi.is_dir(): continue
            parts = zi.filename.split("/", 1)
            if len(parts) != 2: continue
            rel = parts[1].replace("\\", "/")
            kind = include_source_resource(rel, game)
            if kind:
                if ".." in rel.split("/") or rel.startswith("/"):
                    raise ValueError("unsafe source file path " + rel)
                found.append((rel, kind, z.read(zi)))
    found.sort(key=lambda t: t[0])
    return found

def build_pack(game, rom, sources, dest):
    meta = hash_rom(rom, game)
    # (path, kind, bytes | None, alias start within ROM) ; aliases require zero copies.
    entries = [("rom/full.gb", "rom", rom, None)]
    for i in range(ROM_SIZE // BANK_SIZE):
        entries.append(("rom/bank/%02d.bin" % i, "bank", None, i * BANK_SIZE))
    for path, kind, data in sources:
        entries.append((path, kind, data, None))
    manifest = {
        "format": "XPD1", "version": VERSION, "game": game,
        "game_code": KNOWN[game][2], "source_rom": meta,
        "source_binary_resource_types": ["png", "map", "blockset", "tilemap", "rle"],
        "rom_banks_are_views": True,
        "not_yet_translated": ["map events / NPCs", "dialogue and story scripts",
                               "battle logic", "encounter rules", "music/SFX sequences",
                               "Pokemon stats", "save semantics"],
        "note": "No emulator or Game Boy CPU is provided. ASM sources are NOT packed as runtime game data."
    }
    entries.append(("meta/build.json", "metadata",
                    (json.dumps(manifest, indent=2, sort_keys=True)+"\n").encode(), None))
    entries.sort(key=lambda x: x[0])
    folded = [e[0].lower() for e in entries]
    if len(set(folded)) != len(folded): raise ValueError("duplicate case-insensitive resource")
    names = b""
    name_offsets = {}
    for path, *_ in entries:
        raw = path.encode("utf-8")
        if not raw or b"\0" in raw: raise ValueError("invalid name")
        name_offsets[path] = len(names)
        names += raw + b"\0"
    table_off = HEADER_SIZE
    names_off = table_off + len(entries) * ENTRY_SIZE
    data_off = align(names_off + len(names), 16)
    payloads = []
    # Store rom first to support zero-copy bank views.
    cur = data_off
    rom_offset = cur
    payloads.append((cur, rom))
    cur = align(cur + len(rom), 16)
    records = []
    for path, kind, data, alias in entries:
        if alias is not None:
            pos, sz, flags, digest = rom_offset + alias, BANK_SIZE, ALIAS_FLAG, crc(rom[alias:alias+BANK_SIZE])
        elif path == "rom/full.gb":
            pos, sz, flags, digest = rom_offset, len(rom), 0, crc(rom)
        else:
            pos, sz, flags, digest = cur, len(data), 0, crc(data)
            payloads.append((cur, data))
            cur = align(cur + sz, 16)
        records.append((name_offsets[path], fourcc(kind), flags, pos, sz, sz, digest))
    if cur >= 2**32: raise ValueError("XPD v1 file too large")
    header = struct.pack(HEADER_FMT, MAGIC, VERSION, HEADER_SIZE, KNOWN[game][2],
                         len(records), table_off, names_off, len(names),
                         data_off, crc(rom), bytes.fromhex(meta["sha256"]))
    header += bytes(HEADER_SIZE-len(header))
    dest.parent.mkdir(parents=True, exist_ok=True)
    with dest.open("wb") as f:
        f.write(header)
        for rec in records: f.write(struct.pack(ENTRY_FMT,*rec))
        f.write(names)
        f.write(bytes(data_off-f.tell()))
        for pos, data in payloads:
            if pos < f.tell(): raise AssertionError("overlapping payloads")
            if pos > f.tell(): f.write(bytes(pos-f.tell()))
            f.write(data)
    return {"game":game,"path":str(dest), "size":dest.stat().st_size,
            "entries":len(entries),"kinds":dict(collections.Counter(x[1] for x in entries)),
            "rom_sha1":meta["sha1"],"xpd_sha256":hashlib.sha256(dest.read_bytes()).hexdigest()}

def read_archive(path, deep=False):
    with open(path,"rb") as f:
        f.seek(0,os.SEEK_END)
        total = f.tell()
        f.seek(0)
        if total < HEADER_SIZE: raise ValueError("truncated header")
        h = f.read(HEADER_SIZE)
        values = struct.unpack_from(HEADER_FMT,h)
        magic,version,hsize, game_code,n,idx_off,names_off,names_len,data_off,romcrc,romsha=values
        if magic != MAGIC or version != VERSION or hsize != HEADER_SIZE: raise ValueError("invalid XPD1 header")
        if game_code not in (1,2): raise ValueError("unsupported game code")
        if n <= 0 or n > 100000: raise ValueError("bad entry count")
        if idx_off != HEADER_SIZE or names_off != idx_off+n*ENTRY_SIZE:
            raise ValueError("bad index offsets")
        if names_len > 64*1024*1024 or names_off+names_len > total or data_off > total:
            raise ValueError("bad string offsets")
        if data_off < names_off+names_len: raise ValueError("index overlaps payload")
        f.seek(idx_off)
        raw_index = f.read(n*ENTRY_SIZE)
        f.seek(names_off)
        names = f.read(names_len)
        records = []
        existing = set()
        for i in range(n):
            no,code,flags,offset,packed,raw,checksum=struct.unpack_from(ENTRY_FMT,raw_index,i*ENTRY_SIZE)
            if no >= len(names): raise ValueError("name offset out of bounds")
            end = names.find(b"\0",no)
            if end < 0: raise ValueError("unterminated resource name")
            name = names[no:end].decode("utf-8")
            if name in existing or name.startswith("/") or ".." in name.split("/") or "\\" in name:
                raise ValueError("duplicate/unsafe resource name")
            existing.add(name)
            if flags & ~ALIAS_FLAG: raise ValueError("unknown record flags")
            if packed != raw: raise ValueError("compression is unsupported in v1")
            if offset < data_off or offset+packed > total: raise ValueError("resource out of bounds")
            if deep:
                f.seek(offset)
                if crc(f.read(packed)) != checksum: raise ValueError("CRC mismatch: " + name)
            records.append({"name":name,"type":code.to_bytes(4,"little").decode("ascii"),
                            "offset":offset,"size":packed,"alias":bool(flags & ALIAS_FLAG),"crc32":checksum})
        game = "red" if game_code==1 else "blue"
        if deep:
            rom = next((x for x in records if x["name"]=="rom/full.gb"),None)
            if not rom: raise ValueError("missing full ROM")
            f.seek(rom["offset"])
            data = f.read(rom["size"])
            if crc(data) != romcrc or hashlib.sha256(data).digest() != romsha:
                raise ValueError("ROM digest mismatch")
            hash_rom(data,game)
            for i in range(64):
                bank=next((x for x in records if x["name"]=="rom/bank/%02d.bin"%i),None)
                if not bank or not bank["alias"] or bank["offset"]!=rom["offset"]+i*BANK_SIZE:
                    raise ValueError("bank view mismatch: %d"%i)
        return {"game":game,"size":total,"count":n,
                "types":dict(collections.Counter(x["type"].strip() for x in records)),
                "rom_sha256":romsha.hex(),"records":records}

def run(args):
    if args.command=="build":
        roms=get_roms(args)
        summary=[]
        for game,rom in roms.items():
            source=source_entries(args.pokered_zip,game) if args.pokered_zip else []
            filename=Path(args.out_dir)/(game+".xpd")
            s=build_pack(game,rom,source,filename)
            read_archive(filename,deep=True)
            summary.append(s)
            print(json.dumps(s,indent=2))
        if args.report:
            Path(args.report).write_text(json.dumps(summary,indent=2)+"\n",encoding="utf-8")
    elif args.command in ("inspect","verify"):
        info=read_archive(args.archive,deep=args.command=="verify")
        print("XPD %s: %d resources, %d bytes" % (info["game"],info["count"],info["size"]))
        for kind,n in sorted(info["types"].items()): print("  %-6s %d"%(kind,n))
        print("Embedded ROM SHA-256:",info["rom_sha256"])
        if args.command=="verify": print("All CRC32 checks, bank views, and ROM hashes verified.")
        if args.list: print("\n".join(x["name"] for x in info["records"]))
    elif args.command=="extract":
        info=read_archive(args.archive,deep=True)
        entry=next((x for x in info["records"] if x["name"]==args.resource),None)
        if entry is None: raise ValueError("resource not found")
        with open(args.archive,"rb") as f:
            f.seek(entry["offset"])
            raw=f.read(entry["size"])
        Path(args.out).write_bytes(raw)
        print("Extracted %s (%d bytes) to %s"%(args.resource,len(raw),args.out))

def main():
    ap=argparse.ArgumentParser(description="XPD1 Xbox Pokemon Data packer / verifier")
    sub=ap.add_subparsers(dest="command",required=True)
    b=sub.add_parser("build")
    b.add_argument("--roms-zip",help="ZIP holding user-supplied Red and Blue ROMs")
    b.add_argument("--rom-red",help="Red ROM file")
    b.add_argument("--rom-blue",help="Blue ROM file")
    b.add_argument("--pokered-zip",help="pokered-master.zip for PNG, blockset, map-block resources")
    b.add_argument("--game",choices=["red","blue","both"],default="both")
    b.add_argument("--out-dir",default="out")
    b.add_argument("--report",help="write JSON build report")
    for cmd in ("inspect","verify"):
        p=sub.add_parser(cmd)
        p.add_argument("archive")
        p.add_argument("--list",action="store_true")
    e=sub.add_parser("extract")
    e.add_argument("archive")
    e.add_argument("resource")
    e.add_argument("--out",required=True)
    args=ap.parse_args()
    try: run(args)
    except (ValueError, OSError, zipfile.BadZipFile) as err:
        print("XPD error: "+str(err),file=sys.stderr)
        sys.exit(1)
if __name__=="__main__":main()
