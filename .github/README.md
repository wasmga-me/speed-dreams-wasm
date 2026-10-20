# Speed Dreams in the browser

This fork is the web build of [Speed Dreams](https://www.speed-dreams.net/) ([source](https://forge.a-lec.org/speed-dreams/speed-dreams-code)) that runs on wasmga.me. Play it at **https://wasmga.me/games/speed-dreams**.

It is Speed Dreams built with Emscripten. WebAssembly has no shared libraries, so every module and the human and simplix robots are linked into one `.wasm`, with a table standing in for `dlopen`. OpenGL goes through [gl4es](https://github.com/ptitSeb/gl4es) on WebGL and the 3D view is ssggraph on PLIB. There is no network: online races and downloads are off.

## What's here

- Branch `wasmgame` (this one) is upstream commit [`4e29a83`](https://forge.a-lec.org/speed-dreams/speed-dreams-code/src/commit/4e29a83bb701f47ade617a9d73e20e736c492e82) with our changes on top as commits, so `git log` and the diff against `4e29a83` show everything we changed.
- [`wasmgame/`](../wasmgame) holds the build script and the same changes as `.diff` files (`speeddreams.diff` for this repository, `plib.diff` for PLIB). `build.sh` starts from a fresh upstream checkout, applies them and builds, and its header comment lists each change. `pack.py` splits the data into the zips the page downloads, and `seed/` is the first-run Quick Race.

## Building

Needs git, cmake, make, python3 and emsdk 4.0.7 (source `emsdk_env.sh` first).

```sh
sh wasmgame/build.sh /tmp/sd-build
```

It builds the libraries (gl4es, GLU, PLIB, enet, cJSON, utf8proc, RHash, minizip and a curl stand-in), then the game, and writes `speeddreams.js`, `speeddreams.wasm` and `packs.json` next to the script. The base data plus the cars and tracks pinned in `build.sh` are split by `pack.py` into zips under `<work dir>/r2/speeddreams/`.

## Running it

The page runs `main()` itself after writing the first zip to `/sd`, with `--datadir /sd/data/ --libdir /sd/lib/ --localdir /home/web_user/sd/`, and fetches the other zips while the game runs. Before each race the game asks `Module.wgDataReady` for the track's and cars' folders and waits until they are in. The page owns the window size (`wg_resize()`) and full screen.

## Licence

Same as upstream: the code is GPL-2.0-or-later, the art Free Art License 1.3 unless a folder's readme says otherwise. See upstream's own licence files in this repository. Our changes are under the same licence as the files they change.
