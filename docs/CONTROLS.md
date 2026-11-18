# Q2 Games: input and display conventions

## Source implementations reviewed

- [Q2 Arcade `src/app.c`](https://github.com/lampsonlnguyen/q2-arcade/blob/main/src/app.c): joystick-free menu selection, Q2 wheel travel, button navigation, touch hitboxes, pause and game-specific mappings.
- [Q2 Flappybird `src/app.c`](https://github.com/lampsonlnguyen/q2-flappybird/blob/main/src/app.c): act on Center **down**, reject repeated key-downs, do not repeat on matching up; tap on **touch-down**; top-level restart/pause behavior.
- [Q2 Pod `patch/addon.c`](https://github.com/DiamondBond/q2-pod/blob/main/patch/addon.c): firmware forwards datagrams to card-app input socket; some media keys are not forwarded to audio-disabled games.

## Protocol

- `w <signed-raw-wheel-travel>`: raw travel, **negative means clockwise** on this firmware.
- `k d <code>` / `k u <code>`: button press/release. Key codes seen in upstream: Return/Back 170, Play 171, Previous 172, Next 173, Center 218, Left 222, Right 223.
- `t d <x> <y>` / `t u <x> <y>`: touch in landscape 375×320 game coordinates.

## Rules used by Q2 Games

1. **Menu:** wheel movement accumulates in a signed remainder, 28 raw units per game-selection step (default Q2 Arcade 50% wheel speed). Fast flicks may advance multiple entries with a cap of 8 steps per message. Previous/Next and Left/Right are optional alternative navigation.
2. **Center:** act on press, ignore press auto-repeat, consume the matching release. Still accept a release-only Center event as a fallback for older firmware.
3. **Touch:** act on touch-down when present and consume its matching release. Still accept touch-up-only events as fallback. Each game owns distinct on-screen hitboxes.
4. **Pause/exit:** Back while playing pauses; Back while paused returns to selection; Back in selection exits. Center resumes or starts. This avoids relying on media keys.
5. **State transitions:** clear the wheel remainder when starting a game or returning to the selection menu. Do not carry menu scroll into a game's steering.
6. **Framebuffer:** render to an opaque ARGB 375×320 software buffer and copy the completed frame to the **known-visible** framebuffer page. **Do not re-enable `FBIOPAN_DISPLAY`**: it caused black screens on the physical Q2. The software copy mitigates (but cannot guarantee eliminating) tearing. Static puzzle games do not redraw continuously.
7. **Hardware:** the firmware can intercept Play/Previous/Next when card app `audio=0`; every essential game action must remain possible via Center, wheel and touch without those optional buttons.

## Adding the next small game

- Provide a fixed-memory state and a deterministic reset.
- Implement update only if animation requires it; otherwise event-driven render.
- Map wheel to selection/rotation/movement and Center to its primary action. Provide touch control where sensible.
- Add regression tests for initialization, navigation, game rules, termination, and opaque frame output.
- Update the menu, Makefile, README, and the Rockbox game roadmap. The game must be functional; no placeholder titles.
- Require successful host tests, freestanding MIPS compilation and package validation. Physical Q2 testing is a separate required confirmation.

The full original Rockbox plugin API and `.rock` runtime are **not** implemented. These games are independent native gameplay adaptations.
