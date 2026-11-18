#!/usr/bin/env python3
"""Check Q2 Pod's native card-game discovery contract on a built package or SD card."""
from pathlib import Path
import sys

def main() -> int:
    if len(sys.argv) != 2:
        print("Usage: python3 tools/check_card.py <SD-card-root-or-dist>")
        return 2
    root = Path(sys.argv[1]).expanduser().resolve()
    print(f"Scanning: {root}")
    if not root.is_dir():
        print("ERROR: directory does not exist")
        return 2
    card = root / ".games"
    manifest = card / "q2app"
    binary = card / "q2-games"
    if not manifest.is_file() or not binary.is_file():
        print("NOT INSTALLED: expected both:")
        print(f"  {manifest}")
        print(f"  {binary}")
        print("Extract dist/q2-games.zip DIRECTLY to the SD-card root.")
        print("If downloaded from GitHub Actions, first unzip the artifact to obtain q2-games.zip.")
        return 1
    body = manifest.read_text(encoding="utf-8", errors="replace")
    fields = dict(line.split("=", 1) for line in body.splitlines() if "=" in line)
    if fields.get("game") != "1" or not fields.get("name"):
        print("ERROR: invalid game manifest (must include name= and game=1)")
        return 1
    with binary.open("rb") as f:
        signature = f.read(20)
    if len(signature) != 20 or signature[:4] != bytes((127, 69, 76, 70)) or signature[4] != 1 or signature[5] != 1 or int.from_bytes(signature[18:20], "little") != 8:
        print("ERROR: q2-games is not a 32-bit little-endian MIPS ELF executable")
        return 1
    print("OK: valid q2app and Q2 MIPS executable at expected paths")
    print(f"  Games menu title: {fields['name']}")
    apps = []
    for folder in root.iterdir():
        if folder.is_dir() and folder.name.startswith(".") and (folder / "q2app").is_file():
            appid = folder.name[1:]
            if appid and len(appid) <= 15 and all(c in "abcdefghijklmnopqrstuvwxyz0123456789-" for c in appid) and (folder / ("q2-" + appid)).is_file():
                apps.append(folder.name)
    print(f"Detected native card apps: {len(apps)}")
    for app in sorted(apps):
        print(f"  {app}")
    if len(apps) > 8:
        print("WARNING: Q2 Pod's addon scanner supports only 8 card apps total (APPS_MAX=8).")
        print("Which 8 appear depends on SD-card directory iteration order, not app name.")
        print("Remove or temporarily move some unused apps, or use firmware with a higher limit.")
    print("If the Q2 still cannot find it, restart Q2 Pod and verify the card is mounted.")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
