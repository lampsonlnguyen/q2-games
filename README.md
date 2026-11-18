# Q2 Games — native games for Q2 Pod

**Runs directly in Q2 Pod → Library → Games. No Rockbox boot, Rockbox UI, or Rockbox runtime required.**

This project ports open-source Rockbox game logic into a native, freestanding MIPS32 card app for DiamondBond's Shanling Q2 firmware. This is **not** an emulator or a `.rock` plugin launcher.

## Status

**First playable port: Pong.** Adapted from Rockbox `apps/plugins/pong.c` (Daniel Stenberg, GPL-2.0-or-later): original ball-paddle collision behavior, offset-dependent angle reflections, ball randomization, score reset and CPU tracking. The user controls the right paddle; the left paddle is CPU-controlled. UI and device platform bridge are new native Q2 code.

Planned next: Spacerocks, then additional Rockbox games. **Only Pong is built in the native ZIP right now.** Other games in the legacy Rockbox launcher catalog are not included. Real hardware playability is not yet confirmed.

## Build on Ubuntu / WSL

```sh
cd /mnt/c/github/q2-games
git pull --ff-only
make check
make
```

Dependencies: `clang`, `lld`, `make`, `zip`, `python3` and a host C compiler. **No stock Shanling firmware, external Rockbox checkout, or sysroot is needed.** `make` produces:

```
dist/q2-games.zip
dist/.games/q2app
dist/.games/q2-games
```

Unzip `dist/q2-games.zip` at the microSD root. It creates `/.games/q2app` and `/.games/q2-games`; Q2 Pod should discover **Q2 Games** in **Library → Games**. This does not touch the firmware image or the Rockbox installation.

### Controls (Pong)

| Q2 control | Action |
| --- | --- |
| Wheel | Move player paddle |
| Previous / Next side buttons | Move player paddle |
| Center | Start / resume |
| Play/Pause | Pause / resume |
| Back | Pause; Back again returns to title; Back from title exits |
| Touch | Tap title to start; tap playing field to position paddle; tap top edge to pause |

## Implementation

- `src/native_platform.c`: Q2 Pod native MIPS Linux syscalls, input socket, 375×320 framebuffer rotation and double-buffering. Derived from our `q2-arcade` card runtime.
- `src/native_app.c`: Pong gameplay port and Q2 input mapping.
- `src/native_render.c`: compact framebuffer renderer, no Rockbox graphics dependency.
- `src/native_app.h`: shared API.
- `tests/native_test.c`: portable logic/input checks.
- `docs/native.md`: porting notes.

## Limitations

No arbitrary `.rock` binary compatibility. Existing Rockbox plugins depend on Rockbox's API, memory and loader; games must be ported or an authentic compatibility layer implemented. The first port is Pong only. GitHub Actions builds a MIPS executable and ZIP; device performance and control mapping still require physical verification.

## Licensing

GPL-3.0-or-later project. Pong mechanics adapted from Rockbox Pong by Daniel Stenberg (2004), licensed GPL-2.0-or-later. Native Q2 bridge adapted from `q2-arcade`, GPL-3.0-or-later. Copyright/license notices are preserved.
