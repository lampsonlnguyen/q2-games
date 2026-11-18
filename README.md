# Q2 Games — Rockbox plugin launcher for Shanling Q2

**Experimental source integration, not a standalone Rockbox emulator.**

This project develops a reusable game launcher against [DiamondBond's Shanling Q2 Rockbox port](https://github.com/DiamondBond/q2-rockbox/tree/shanlingq2). It uses Rockbox's **real** plugin loader and API rather than pretending that existing `.rock` files are Linux executables. The goal is to minimize per-game ports. The Q2 Rockbox fork already supplies graphics, input, a 2 MiB plugin buffer, and the plugin loader.

## Current scope

- Rockbox-native `q2_games.rock` launcher source, with button/wheel navigation and display-safe margins.
- An availability-filtered catalog of games supported in the Shanling Q2 fork.
- Direct handoff through `rb->plugin_open()` and `PLUGIN_GOTO_PLUGIN`.
- Deterministic installation script into a **separate clone** of the Rockbox Q2 fork.
- Optional, explicit game-first boot source patch using Rockbox's existing `AUTOROCK` hook.
- Host tests and CI for catalog/install integration.

**Not implemented:** a standalone `.rock` plugin loader inside Q2 Pod, direct launching from Q2 Pod's Library > Games menu, prebuilt MIPS binary, automatic firmware patching, support for arbitrary other-device `.rock` binaries. Don't copy `q2_games.c` directly to your SD card; it is source code.

## Why reuse Rockbox?

The hosted Shanling Q2 Rockbox fork has `PLUGIN_BUFFER_SIZE=0x200000`, native framebuffer, iPod-style wheel/button keymapping and a versioned plugin ABI. Existing `.rock` games must be built against **the matching Rockbox binary**; third-party `.rock` files cannot be assumed compatible. The current fork intentionally excludes `bubbles`, `invadrox`, `jewels`, `rockblox`, and others due to 375x320 layouts; we do not claim those currently work.

## One-command Q2 cross-build (Ubuntu/WSL)

After cloning both repositories side by side and building the compatible sysroot with the **stock Shanling V1.32 ZIP**, run:

```sh
cd /mnt/c/github/q2-games
make check
make
```

Default `make` now installs the launcher source into `../q2-rockbox`, configures a Rockbox Q2 build if one is not configured, compiles Rockbox and plugins, and creates `~/q2-rockbox-build/rockbox.zip`. It does **not** use the host compiler for `.rock` games.

Override paths when needed:

```sh
make ROCKBOX_ROOT=/mnt/c/github/q2-rockbox \
  Q2_SYSROOT="$HOME/q2-rockbox-sysroot" \
  Q2_BUILD_DIR="$HOME/q2-rockbox-build" JOBS=4
```

Use `make check` for local unit tests only. To reinstall updated launcher source without cross-building, use `make integrate`.

Existing Rockbox build directories will be reused: delete the build directory and reconfigure if its target or sysroot changed.

## Integrate into the Rockbox Q2 source tree

```sh
git clone --branch shanlingq2 https://github.com/DiamondBond/q2-rockbox.git rockbox
git clone https://github.com/lampsonlnguyen/q2-games.git
python3 q2-games/tools/install_into_rockbox.py rockbox
```

The script copies source files to `rockbox/apps/plugins/` and adds `q2_games.c` under `#ifdef SHANLING_Q2` in `apps/plugins/SOURCES`. It is idempotent and does **not** alter the stock firmware, build configurations, or the original fork. Re-run after updating `q2-games`.

Build Rockbox using the Q2 fork's own **documented toolchain and firmware sysroot**, and run its usual plugin packaging. A resulting `q2_games.rock` would be installed under `/.rockbox/rocks/games/` alongside the matching Rockbox binary and other games. **This repo cannot build the actual MIPS plugin without the external Q2 build environment.**

### Optional games-first Rockbox startup

To make the launcher run when **Rockbox** starts, apply the separate, opt-in source edit:

```sh
python3 q2-games/tools/install_into_rockbox.py rockbox --games-first
```

This enables the *existing* `AUTOROCK` hook to launch `/.rockbox/rocks/games/q2_games.rock` after Rockbox initializes. It does **not** change Q2 Pod's boot selection or launch from Library > Games. Keep a normal Rockbox build available in case this experimental startup path misbehaves.

## Verification

```sh
python3 -m unittest discover -s tests -v
```

CI verifies catalog availability, selected games, installer idempotence and an opt-in startup patch against a representative Rockbox source fixture. Real Q2 cross-compilation and hardware validation remain outstanding.

## Roadmap

1. Cross-compile and validate `q2_games.rock` within a pinned DiamondBond Rockbox Q2 build.
2. Validate Pong, Spacerocks, Chopper and selected puzzle plugins on actual Q2 hardware.
3. Fix layouts and controls for excluded Invadrox and Bubbles upstream or as small target patches.
4. Integrate a safe, coordinated Q2 Pod-to-Rockbox handoff to reach Games directly from Q2 Pod.
5. Investigate a games-only Rockbox runtime package without including a second incompatible plugin loader.

## Attribution & license

Original Q2 Games launcher and installer source: GPL-3.0-or-later. Rockbox is GPLv2-or-later, with per-file licensing; preserve notices for every copied/modified file. No proprietary ROMs, artwork or firmware are distributed.
