#!/usr/bin/env python3
"""Install the Q2 Games plugin into a *source tree*, never a live device."""
import argparse
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MARKER = "/* Q2 Games: managed launcher */"
SNIPPET = (
    MARKER + "\n"
    "#ifdef SHANLING_Q2\n"
    "q2_games.c\n"
    "#endif\n"
)
AUTOROCK = '#define AUTOROCK ROCKBOX_DIR "/rocks/games/q2_games.rock"'

def install(target: Path, games_first: bool = False):
    target = target.resolve()
    sources = target / "apps/plugins/SOURCES"
    main = target / "apps/main.c"
    config = target / "firmware/export/config/shanlingq2.h"
    if not (sources.is_file() and main.is_file() and config.is_file()):
        raise ValueError("Expected a Shanling Q2 Rockbox source checkout")
    if "#define SHANLING_Q2" not in config.read_text() and "Shanling Q2" not in config.read_text():
        raise ValueError("Target is not the Shanling Q2 fork")
    plugins = target / "apps/plugins"
    for source, destination in [
        (ROOT / "rockbox/q2_games.c", plugins / "q2_games.c"),
        (ROOT / "src/q2_games_catalog.c", plugins / "q2_games_catalog.inc"),
        (ROOT / "src/q2_games_catalog.h", plugins / "q2_games_catalog.h"),
    ]:
        destination.write_bytes(source.read_bytes())
    old = sources.read_text()
    if MARKER not in old:
        sources.write_text(SNIPPET + "\n" + old)
    if games_first:
        contents = main.read_text()
        if AUTOROCK not in contents:
            # Rockbox already has an optional AUTOROCK launcher hook.
            token = '// #define AUTOROCK /* define this to check for "autostart.rock" on boot */'
            if token not in contents:
                raise ValueError("Rockbox AUTOROCK hook changed; aborting edit")
            main.write_text(contents.replace(token, AUTOROCK + " /* Q2 Games */", 1))
    return plugins / "q2_games.c"

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rockbox", type=Path, help="DiamondBond q2-rockbox checkout")
    parser.add_argument("--games-first", action="store_true",
                        help="opt in to starting Q2 Games when Rockbox boots")
    args = parser.parse_args()
    print("Installed:", install(args.rockbox, args.games_first))
    print("Build the plugin with the matching Rockbox Q2 firmware/sysroot.")

if __name__ == "__main__":
    main()
