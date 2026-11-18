# Rockbox game-porting roadmap for Q2 Games

Source of truth: [DiamondBond/q2-rockbox `apps/plugins/CATEGORIES`](https://github.com/DiamondBond/q2-rockbox/blob/shanlingq2/apps/plugins/CATEGORIES) on the `shanlingq2` branch. This manifest currently labels **90 plugins as games** (45 individual game categories and 45 `sgt-` puzzle entries). This is a *source catalog*, not a claim that all build for the Shanling Q2 or can be executed by the native Q2 Pod runtime.

## Status

The Q2 Games app has **fourteen built-in native adaptations**: Pong, Snake, BrickMania, Spacerocks, Sliding Puzzle, Minesweeper, Reversi, Flipit, 2048, Maze, Codebuster, Dice, Jackpot, and Robotfindskitten. They are adapted gameplay, **not binary-compatible `.rock` plugins**, and have not all been verified on physical hardware. Build and host-side logic tests are required before each device rollout.

**The other 76 source-catalog entries are not yet implemented.** Full feature-for-feature equivalence to the originals is a separate goal from compact native versions.

## Non-SGT games (45)

| Game | Status |
|---|---|
| `2048` | Native adaptation (hardware verification pending) |
| `amaze` | Not ported |
| `blackjack` | Not ported |
| `boomshine` | Not ported |
| `brickmania` | Native adaptation (hardware verification pending) |
| `bubbles` | Not ported |
| `chessbox` | Larger implementation/engine or asset work |
| `chopper` | Not ported |
| `clix` | Not ported |
| `codebuster` | Native adaptation (hardware verification pending) |
| `dice` | Simplified native adaptation (hardware verification pending) |
| `doom` | Larger implementation/engine or asset work |
| `duke3d` | Larger implementation/engine or asset work |
| `flipit` | Native adaptation (hardware verification pending) |
| `goban` | Larger implementation/engine or asset work |
| `invadrox` | Not ported |
| `jackpot` | Simplified native adaptation (hardware verification pending) |
| `jewels` | Larger implementation/engine or asset work |
| `maze` | Native adaptation (hardware verification pending) |
| `mazezam` | Not ported |
| `minesweeper` | Native adaptation (hardware verification pending) |
| `nim` | Not ported |
| `pacbox` | Larger implementation/engine or asset work |
| `pegbox` | Not ported |
| `picross` | Not ported |
| `pixel-painter` | Larger implementation/engine or asset work |
| `pong` | Native adaptation (hardware verification pending) |
| `quake` | Larger implementation/engine or asset work |
| `reversi` | Native adaptation (hardware verification pending) |
| `robotfindskitten` | Native adaptation (hardware verification pending) |
| `rockblox` | Larger implementation/engine or asset work |
| `rockblox1d` | Not ported |
| `sliding_puzzle` | Native adaptation (hardware verification pending) |
| `snake` | Native adaptation (hardware verification pending) |
| `snake2` | Not ported |
| `sokoban` | Larger implementation/engine or asset work |
| `solitaire` | Larger implementation/engine or asset work |
| `spacerocks` | Native adaptation (hardware verification pending) |
| `star` | Not ported |
| `sudoku` | Larger implementation/engine or asset work |
| `wolf3d` | Larger implementation/engine or asset work |
| `wormlet` | Not ported |
| `xobox` | Not ported |
| `xrick` | Larger implementation/engine or asset work |
| `xworld` | Larger implementation/engine or asset work |

## Simon Tatham puzzle collection (45 category entries)

Rockbox also includes a large group of puzzles from the Simon Tatham collection, many sharing the `sgt-puzzles` infrastructure rather than being independently packaged standalone native Q2 games. They need an intentional shared engine/port instead of duplicating 45 separate game frameworks.

- `sgt-blackbox`
- `sgt-bridges`
- `sgt-cube`
- `sgt-dominosa`
- `sgt-fifteen`
- `sgt-filling`
- `sgt-flip`
- `sgt-flood`
- `sgt-galaxies`
- `sgt-group`
- `sgt-guess`
- `sgt-inertia`
- `sgt-keen`
- `sgt-lightup`
- `sgt-loopy`
- `sgt-magnets`
- `sgt-map`
- `sgt-mines`
- `sgt-mosaic`
- `sgt-net`
- `sgt-netslide`
- `sgt-palisade`
- `sgt-pattern`
- `sgt-pearl`
- `sgt-pegs`
- `sgt-puzzles`
- `sgt-range`
- `sgt-rect`
- `sgt-samegame`
- `sgt-separate`
- `sgt-signpost`
- `sgt-singles`
- `sgt-sixteen`
- `sgt-slant`
- `sgt-slide`
- `sgt-sokoban`
- `sgt-solo`
- `sgt-tents`
- `sgt-towers`
- `sgt-tracks`
- `sgt-twiddle`
- `sgt-undead`
- `sgt-unequal`
- `sgt-unruly`
- `sgt-untangle`

## Architecture and acceptance criteria

1. Preserve the known-working **single visible framebuffer page**; re-enabling framebuffer panning previously caused a black display on Q2 hardware.
2. Render offscreen and publish finished frames; the copy is not guaranteed tear-free absent validated vsync. Avoid copying at 60 FPS for static puzzle screens.
3. Maintain opaque ARGB pixels and the readable Q2 Arcade font.
4. Keep input controls accurate for the Q2 wheel, center, touch and forwarded side buttons. Test on hardware: the firmware may intercept some side buttons depending on the `q2app` audio setting.
5. Build as freestanding little-endian MIPS32 in `.games/q2-games` with `q2app`; no Rockbox UI or runtime required.
6. Require clean `make check`, MIPS build, package validation, and actual device tests before marking each game hardware-verified.
7. Preserve upstream GPL licenses and credits. Larger engine ports need memory/performance profiling and assessment of external data/assets.
8. Don't add inert game menu entries. Each entry must have meaningful control, game state, score/win/lose rules, and rendering.

## Next implementation batches

- **Batch 3:** Amaze, Pegbox, Boomshine; independent rule sets and predictable Q2 controls.
- **Batch 4:** Sokoban, Rockblox, Jewels, Sudoku, Picross; solve graphics/input and data needs with shared components.
- **Batch 5:** Investigate a shared port of the Simon Tatham puzzle runtime.
- **Complex plugin engines:** Doom, Quake, Duke3D, Wolf3D, emulators and interpreter-backed games need separate feasibility efforts.

The catalog includes copyrighted game names and third-party assets. Source licensing must be checked per game; the project should not bundle copyrighted commercial game data merely because an open-source engine exists.

## Scope decision: small games only

**In scope:** single-screen, small fixed-memory, turn-based or simple 2D arcade games using the Q2 wheel, center key and touch panel; no external game data, large asset pack, additional emulator or interpreter engine. Finish device verification on existing games before expanding aggressively.

**Out of scope (do not implement in this repository):** Doom, Duke3D, Quake, Wolf3D, Pacbox, Xrick, Xworld, Chessbox, Goban, complex emulators/interpreters, commercial engine data, and the shared 45-game Simon Tatham puzzle framework. Some other category entries may also be too large after code inspection. These are deliberately omitted rather than represented as broken placeholders in the menu.

**Candidate small-game backlog, subject to source and hardware review:** Amaze, Blackjack, Boomshine, Bubbles, Chopper, Clix, Mazezam, Nim, Pegbox, Picross, Rockblox1d, Snake2, Star, Wormlet, Xobox. Game names are not promises of feasibility; only finished, tested native adaptations are added to the installable menu.
