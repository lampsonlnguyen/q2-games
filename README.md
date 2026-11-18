# Q2 Games — native games for Q2 Pod

**Runs directly in Q2 Pod → Library → Games. No Rockbox boot, Rockbox UI, or Rockbox runtime required.**

This project ports open-source Rockbox game logic into a native, freestanding MIPS32 card app for DiamondBond's Shanling Q2 firmware. This is **not** an emulator or a `.rock` plugin launcher.

## Status

**Native game selection with two built-in games: Pong and Snake.** Pong is Adapted from Rockbox `apps/plugins/pong.c` (Daniel Stenberg, GPL-2.0-or-later): original ball-paddle collision behavior, offset-dependent angle reflections, ball randomization, score reset and CPU tracking. The user controls the right paddle; the left paddle is CPU-controlled. UI and device platform bridge are new native Q2 code.

Planned next: Spacerocks, then additional Rockbox games. **Only Pong and Snake are built into the native ZIP.** Other games in the legacy Rockbox launcher catalog are not included. Real hardware playability is not yet confirmed.

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

### Controls (Pong and Snake)

| Q2 control | Action |
| --- | --- |
| Wheel | Select game in menu; move paddle in Pong; turn in Snake |
| Previous / Next side buttons | Choose game in menu; move paddle in Pong; turn Snake |
| Center | Start / resume |
| Play/Pause | Pause / resume |
| Back | Pause; Back again returns to title; Back from title exits |
| Touch | Tap title to start; tap playing field to position paddle; tap top edge to pause |

## Implementation

- `src/native_platform.c`: Q2 Pod native MIPS Linux syscalls, input socket, 375×320 framebuffer rotation and double-buffering. Derived from our `q2-arcade` card runtime.
- `src/native_app.c`: Pong gameplay port, Snake logic (Rockbox Snake-inspired), native game-selection menu and Q2 input mapping.
- `src/native_render.c`: compact framebuffer renderer, no Rockbox graphics dependency.
- `src/native_app.h`: shared API.
- `tests/native_test.c`: portable logic/input checks.
- `docs/native.md`: porting notes.

## Limitations

No arbitrary `.rock` binary compatibility. Existing Rockbox plugins depend on Rockbox's API, memory and loader; games must be ported or an authentic compatibility layer implemented. The two native games are Pong and Snake; full Rockbox plugin compatibility is not implemented. GitHub Actions builds a MIPS executable and ZIP; device performance and control mapping still require physical verification.

## Licensing

GPL-3.0-or-later project. Pong mechanics adapted from Rockbox Pong by Daniel Stenberg (2004), GPL-2.0-or-later. Snake gameplay is a new fixed-array implementation inspired by Rockbox Snake by Itai Shaked (2002), GPL-2.0-or-later. Native Q2 bridge adapted from `q2-arcade`, GPL-3.0-or-later. Copyright/license notices are preserved.

## Missing from the Games list?

Q2 Pod's `patch/addon.c` discovery rule is **folder `.games/` containing both `q2app` and `q2-games`**. The app is listed under `Library → Games` because `q2app` contains `game=1`. This is a card app, **not** a `.rock` plugin.

If you downloaded `q2-games-native.zip` from GitHub Actions, that is an **artifact wrapper ZIP**, containing `q2-games.zip`; extract the wrapper first, then extract `q2-games.zip` onto the SD-card **root**. Copying a ZIP file onto the card without extracting it does not install the app.

Check with:

```sh
python3 tools/check_card.py /path/to/your/sdcard
```

The checker validates the manifest, Q2 MIPS executable and locations, and counts other card apps. **Q2 Pod currently has `APPS_MAX=8` total card apps**, across Games, Library and Streaming. If you have more than eight valid card apps, some may not be discovered, depending on directory iteration order; temporarily move unused apps off the SD card to test. A successful CI build does not bypass this device limit. Restart Q2 Pod after copying files if the list does not update.
