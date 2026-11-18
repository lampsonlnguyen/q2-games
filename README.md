# Q2 Games — native games for Q2 Pod

**Runs directly in Q2 Pod → Library → Games. No Rockbox boot, Rockbox UI, or Rockbox runtime required.**

This project ports open-source Rockbox game logic into a native, freestanding MIPS32 card app for DiamondBond's Shanling Q2 firmware. This is **not** an emulator or a `.rock` plugin launcher.

## Status

**Native game selection with four built-in games: Pong, Snake, BrickMania and Spacerocks.** Pong is Adapted from Rockbox `apps/plugins/pong.c` (Daniel Stenberg, GPL-2.0-or-later): original ball-paddle collision behavior, offset-dependent angle reflections, ball randomization, score reset and CPU tracking. The user controls the right paddle; the left paddle is CPU-controlled. UI and device platform bridge are new native Q2 code.

Planned next: Spacerocks, then additional Rockbox games. **Four games are built into the native ZIP; these are native adaptations rather than binary-compatible Rockbox plugins.** Other games in the legacy Rockbox launcher catalog are not included. Real hardware playability is not yet confirmed.

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

### Controls

| Q2 control | Action |
| --- | --- |
| Wheel | Select game; move paddle in Pong/BrickMania; turn in Snake/Spacerocks |
| Previous / Next side buttons | Choose game in menu; move paddle in Pong; turn Snake |
| Center | Start/resume; fire in Spacerocks |
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

No arbitrary `.rock` binary compatibility. Existing Rockbox plugins depend on Rockbox's API, memory and loader; games must be ported or an authentic compatibility layer implemented. Native games are Pong, Snake, BrickMania and Spacerocks; full Rockbox plugin compatibility is not implemented. GitHub Actions builds a MIPS executable and ZIP; device performance and control mapping still require physical verification.

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

## Black screen troubleshooting (hardware diagnosis)

The ARGB framebuffer writer now forces every pixel opaque (`alpha=0xff`), matching the working `q2-arcade` renderer. The previous renderer wrote `0x00RRGGBB` colors (zero alpha), which is a strong candidate for the observed black display. The project also **draws its first frame directly to the active framebuffer page** and temporarily disables double-buffer page flips to isolate display-driver issues. A diagnostic is written to the microSD card on every app launch:

```
/.games/q2-games-diagnostic.txt
```

After launching Q2 Games once, remove/mount the microSD card on your computer and read this file. A successful launch up to the first draw reads:

```
START: Q2 Games entry reached
OK: framebuffer opened
OK: input socket bound and game state initialized
OK: first frame written to visible framebuffer
```

If there is an `ERROR:` line, it identifies a startup failure (framebuffer open/ioctl/geometry/mmap or input socket bind). If the log is **absent**, verify the installed binary was replaced, the SD card is writable and the launcher actually started; a missing log alone does not prove the executable never ran. If the log ends with the successful first-frame line yet the display remains black, investigate pixel format, framebuffer page selection or whether the firmware redraws over the game. The log is overwritten on each launch. This is a temporary diagnostic and not evidence of successful hardware rendering.

## Display quality fix

The original tiny 3×5 glyph table used incorrect bit patterns and was not a readable screen font. The renderer now reuses the anti-aliased Liberation Sans Bold 12/17px font from `q2-arcade` (in `src/font.h`), with alpha compositing and fully opaque ARGB pixels. The earlier diagnostic forced single-buffer rendering, which caused a moving tear line as the LCD scanned out an in-progress frame; the native platform now attempts double-buffered `FBIOPAN_DISPLAY` again, with a direct-draw fallback if panning isn't supported. Physical hardware validation is still needed after this change.

## Native Rockbox-inspired ports, batch 1

- **BrickMania:** 10×5 destructible brick grid, angled paddle rebounds, lives and stages. Native compact adaptation of Rockbox `apps/plugins/brickmania.c` (Ben Basha et al., GPL-2.0-or-later).
- **Spacerocks:** spacecraft with wheel rotation and projectiles, drifting/splitting rocks and waves. Native compact adaptation of Rockbox `apps/plugins/spacerocks.c` (Mat Holton, GPL-2.0-or-later). Center fires.

These are **simplified native adaptations**, not feature-complete ports of the original Rockbox plugins. Additional games require individual implementation. The confirmed working single-page framebuffer mode remains unchanged; page flipping has not been re-enabled.

### Spacerocks controls

Use the wheel to steer, **Center** to fire, **Next/right** to thrust and **Previous/left** to hyperspace to a new location. The spaceship drifts between button presses, wraps at the screen edges, takes collision damage, and has three lives. This is still a compact adaptation; enemy ships and all original Rockbox special effects are not implemented.
