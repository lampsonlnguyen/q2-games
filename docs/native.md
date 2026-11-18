# Q2 Games native implementation

This is a standalone Shanling Q2 Pod card app. Rockbox DOES NOT start and the Rockbox launcher in `rockbox/q2_games.c` is legacy, not built.

First playable game: Pong, with Rockbox Pong collision, paddle-angle rebound and CPU tracking adapted from `DiamondBond/q2-rockbox/apps/plugins/pong.c` (Daniel Stenberg, 2004, GPL-2.0-or-later). Original input handling, framebuffers and card app integration are native Q2 code derived from q2-arcade.

`make` creates `dist/q2-games.zip`; unzip to the microSD root to install `/.games/q2app` and `/.games/q2-games`. Q2 Pod should discover it under Library > Games. Requires physical Q2 validation.

Wheel: move paddle; Center: start/resume; Back: pause then return to menu, another Back exits from menu; Play/Pause: pause/resume; Previous/Next: move paddle. Touch: tap to start, tap in arena to position paddle, tap top edge to pause.

Next milestone: port Spacerocks' gameplay logic using the same native framebuffer and input APIs. Arbitrary `.rock` binaries are NOT supported; each game has to be ported or a genuine Rockbox compatibility runtime implemented.
