# Architecture and compatibility notes

## Plugin ABI

Rockbox's `apps/plugin.h` declares `PLUGIN_MAGIC`, `PLUGIN_API_VERSION`, and a large API vector. The real `apps/plugin.c` supplies that vector and loads `.rock` files. Rebuilding this functionality separately risks ABI, memory, rendering, file and input incompatibilities. Instead, this first version is a Rockbox **plugin that launches other Rockbox plugins**.

## Q2 fork facts

- `firmware/export/config/shanlingq2.h`: 375x320 display, `IPOD_4G_PAD` input, `PLUGIN_BUFFER_SIZE 0x200000`.
- Rockbox `apps/main.c`: optional `AUTOROCK` entry point, disabled by default.
- Q2 port `apps/plugins/SOURCES`: explicitly omits `bubbles`, `jewels`, `invadrox`, `rockblox`, `wormlet` pending layout work.
- Q2's Rockbox build must include both the launcher and its game plugins. Installing only a single `.rock` compiled for some other model is unsupported.

## Handoff and launch

The launcher calls `rb->plugin_open(path, NULL)` and returns `PLUGIN_GOTO_PLUGIN`, which delegates the actual load to Rockbox core. It does not exec a `.rock` ELF or interpret plugin bytes. The launcher hides games that do not exist under `/.rockbox/rocks/games/`.

## Q2 Pod Games integration (future)

Q2 Pod executes MIPS card apps in a separate process; Rockbox game plugins are **not** Linux programs. Integrating them into Q2 Pod directly requires a correct transition that releases Q2 Pod's display/audio/key ownership before Rockbox starts, and an agreed return path. A new native loader is not part of this first milestone.

## Testing boundaries

Host catalog and installer tests verify selection, source editing, and idempotence. They do not test the Rockbox plugin ABI or validate the compiled Q2 binary. This needs a real cross-build using the matching Rockbox fork and sysroot, followed by device testing.
