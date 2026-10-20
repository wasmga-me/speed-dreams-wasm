#!/bin/sh
# Builds Speed Dreams (https://www.speed-dreams.net/, code GPL-2.0-or-later,
# art Free Art License 1.3 unless a folder's readme says otherwise) for the
# site, from upstream source at the commit below, with the changes in
# speeddreams.diff and plib.diff beside this script:
# - every module and robot (human and simplix) is linked into the game, since
#   WebAssembly has no shared libraries: their entry points take the module's
#   name (cmake/macros.cmake), wasmmodules.py writes the table that stands in
#   for dlopen/dlsym (tgf/wasmdl.h), and lib/ holds empty files where the game
#   looks for the modules;
# - OpenGL through gl4es on WebGL, with GLU from gl4es's author; the 3D view
#   is ssggraph on PLIB (osggraph needs OpenSceneGraph);
# - the event loop runs on Emscripten's main loop, and the window size and
#   full screen belong to the page (wg_resize(); the display options, the
#   full screen keys, Downloads and Quit are left out);
# - a race waits until the page has its track and cars (Module.wgDataReady);
# - no network: curl is the stand-in in curlstub/, and online races are off;
# - fixes: the generated drivers' three-letter code no longer keeps a byte of
#   the previous letter (a name like Mäkelä broke the driver's file), and two
#   simplix functions that clash with simuv5 once linked together are static.
# The data is upstream's base data plus the cars and tracks below, each from
# its own repository, split by pack.py beside this script (see there) into
# zips for R2. Left out for their licences: brondehach, petit, corkscrew,
# garage and ls1-archer-r9 (none of them are here).
#
# Needs git, cmake, make, python3 and emsdk 4.0.7 (source emsdk_env.sh first).
# Usage: sh build.sh [work dir]
# Writes speeddreams.js, speeddreams.wasm and packs.json next to this script,
# and the zips for R2 to <work dir>/r2/speeddreams/ (upload them to
# wasmgame-files under the same keys). For a newer Speed Dreams, copy this
# folder to speeddreams-<new rev>-p1 and change the commits below; a change to
# the build here takes the next -p number.
set -eu

SD_REV=4e29a83bb701f47ade617a9d73e20e736c492e82
PLIB_REV=ef2f193d0bfd8cef08250f39f9338f3f55a84e1e
GL4ES_REV=c9895df34cd466c23bc60c2bd3db3d87e98fcbe7
GLU_REV=2fed2bda2b725d2b9e32c435b48d5141cc95827f
EMSDK_VERSION=4.0.7
FORGE=https://forge.a-lec.org/speed-dreams
# Cars and tracks: name, commit, and for tracks the category folder.
CARS="sc-boxer-96:fb04fe05abed87b148ea70e1a1d2af8a7bf2fb9c
sc-kanagawa-z35gts:c27d54783399bd13ae1819c2a51c4303c5a06ab6
sc-kongei-kk8s:89aad394346e4d14ee49fd852c6b0ae840df54ab
sc-lynx-220:26363e7fae9ccf0081a44af94ae2d41609051f4b
sc-murasama-nsx:6ff4662b88f704d06f5d69fe9978de3b89dd133d
sc-spirit-300:5700ae0320ae07005c530be505cb5fb56d5ef6a6"
TRACKS="brno:33c1e32ad16860a4ee81ceb00205e139600ae035:circuit
tuna:df51b0cc55e2e25d2d6185261e130a1a16380a75:circuit
street-1:73c59aef626520b4422091e3e98d0d9f7da85323:road
pinabashi:266e39c6d6e89c2949c85059230e6a912e188900:road
dirt-2:5909c9d1ea69f48f9adb58b154642f1f082e651a:dirt
tennessee:3abbce5cf8a2118de9871fd9d3340feffc70ec3a:speedway"

here=$(cd "$(dirname "$0")" && pwd)
work=${1:-${TMPDIR:-/tmp}/speeddreams-build}
mkdir -p "$work"
jobs=${JOBS:-$(nproc)}

case "$(emcc --version | head -1)" in
  *" $EMSDK_VERSION "*) ;;
  *) echo "warning: built and tested with emsdk $EMSDK_VERSION" >&2 ;;
esac

# Emscripten fetches its ports as GitHub archives; clone them instead.
. "$here/ports.sh"
port sdl2 libsdl-org/SDL release-2.32.0 SDL-release-2.32.0 https://github.com/libsdl-org/SDL/archive/release-2.32.0.zip
port sdl2_mixer libsdl-org/SDL_mixer release-2.8.0 SDL_mixer-release-2.8.0 https://github.com/libsdl-org/SDL_mixer/archive/release-2.8.0.zip
port sdl2_ttf libsdl-org/SDL_ttf release-2.20.2 SDL_ttf-release-2.20.2 https://github.com/libsdl-org/SDL_ttf/archive/release-2.20.2.zip
port freetype freetype/freetype VER-2-13-3 freetype-VER-2-13-3 https://github.com/freetype/freetype/archive/VER-2-13-3.zip
port harfbuzz harfbuzz/harfbuzz 3.2.0 harfbuzz-3.2.0 https://github.com/harfbuzz/harfbuzz/releases/download/3.2.0/harfbuzz-3.2.0.tar.xz
port ogg emscripten-ports/ogg version_1 Ogg-version_1 https://github.com/emscripten-ports/ogg/archive/version_1.zip
port vorbis emscripten-ports/vorbis version_1 Vorbis-version_1 https://github.com/emscripten-ports/vorbis/archive/version_1.zip
port zlib madler/zlib v1.3.1 zlib-1.3.1 https://github.com/madler/zlib/archive/refs/tags/v1.3.1.tar.gz
PORTS='-sUSE_SDL=2 -sUSE_SDL_TTF=2 -sUSE_SDL_MIXER=2 -sSDL2_MIXER_FORMATS=["ogg"] -sUSE_LIBPNG=1 -sUSE_LIBJPEG=1 -sUSE_ZLIB=1 -sUSE_OGG=1 -sUSE_VORBIS=1'
# Build the ports now: cmake below is given their libraries' paths.
printf 'int main(void) { return 0; }\n' > "$work/ports.c"
emcc $PORTS "$work/ports.c" -o "$work/ports.js"

# Libraries.
p=$work/deps
src=$work/libs
mkdir -p "$p/lib" "$p/include" "$src"
clone() { # dir url rev
  [ -d "$src/$1" ] || git clone -q "$2" "$src/$1"
  git -C "$src/$1" checkout -q -f "$3"
}
if [ ! -f "$p/lib/libGL.a" ]; then
  clone gl4es https://github.com/ptitSeb/gl4es.git $GL4ES_REV
  emcmake cmake -S "$src/gl4es" -B "$src/gl4es/build" -DCMAKE_BUILD_TYPE=Release -DNOX11=ON -DNOEGL=ON -DSTATICLIB=ON >/dev/null
  make -s -j"$jobs" -C "$src/gl4es/build"
  cp "$src/gl4es/lib/libGL.a" "$p/lib/"
  mkdir -p "$p/include/GL" && cp -r "$src/gl4es/include/GL/"* "$p/include/GL/" && cp "$src/gl4es/include/gl4esinit.h" "$p/include/"
fi
if [ ! -f "$p/lib/libGLU.a" ]; then
  # Built with gl4es's names for the GL calls (glu_mangle.h), like everything else.
  clone glu https://github.com/ptitSeb/GLU.git $GLU_REV
  mkdir -p "$src/glu/o"
  for f in libutil/error libutil/glue libutil/mipmap libutil/project libutil/quad libutil/registry \
    libtess/dict libtess/geom libtess/memalloc libtess/mesh libtess/normal libtess/priorityq libtess/render libtess/sweep libtess/tess libtess/tessmono; do
    emcc -O2 -fsigned-char -DLIBRARYBUILD -std=c99 -include "$p/include/GL/glu_mangle.h" -I"$p/include" -I"$src/glu/include" -I"$src/glu/src/include" \
      -c "$src/glu/src/$f.c" -o "$src/glu/o/$(basename $f).o"
  done
  emar rcs "$p/lib/libGLU.a" "$src/glu/o/"*.o
fi
if [ ! -f "$p/lib/libenet.a" ]; then
  clone enet https://github.com/lsalzman/enet.git v1.3.18
  emcmake cmake -S "$src/enet" -B "$src/enet/build" -DCMAKE_BUILD_TYPE=Release >/dev/null
  make -s -j"$jobs" -C "$src/enet/build"
  cp "$src/enet/build/libenet.a" "$p/lib/" && cp -r "$src/enet/include/enet" "$p/include/"
fi
if [ ! -f "$p/lib/libcjson.a" ]; then
  clone cjson https://github.com/DaveGamble/cJSON.git v1.7.18
  mkdir -p "$p/include/cjson" && cp "$src/cjson/cJSON.h" "$p/include/cjson/"
  emcc -O2 -c "$src/cjson/cJSON.c" -o "$src/cjson/cJSON.o" && emar rcs "$p/lib/libcjson.a" "$src/cjson/cJSON.o"
fi
if [ ! -f "$p/lib/libutf8proc.a" ]; then
  clone utf8proc https://github.com/JuliaStrings/utf8proc.git v2.9.0
  cp "$src/utf8proc/utf8proc.h" "$p/include/"
  emcc -O2 -DUTF8PROC_STATIC -c "$src/utf8proc/utf8proc.c" -o "$src/utf8proc/u.o" && emar rcs "$p/lib/libutf8proc.a" "$src/utf8proc/u.o"
fi
if [ ! -f "$p/lib/librhash.a" ]; then
  clone rhash https://github.com/rhash/RHash.git v1.4.5
  cp "$src/rhash/librhash/rhash.h" "$src/rhash/librhash/rhash_torrent.h" "$p/include/"
  rm -f "$src/rhash/librhash/"*.o
  for f in "$src/rhash/librhash/"*.c; do
    case $f in *test_*) continue ;; esac
    emcc -O2 -DRHASH_XVERSION=0x01040500 -c "$f" -o "${f%.c}.o"
  done
  emar rcs "$p/lib/librhash.a" "$src/rhash/librhash/"*.o
fi
if [ ! -f "$p/lib/libminizip.a" ]; then
  clone zlib https://github.com/madler/zlib.git v1.3.1
  m=$src/zlib/contrib/minizip
  mkdir -p "$p/include/minizip" && cp "$m/crypt.h" "$m/ioapi.h" "$m/mztools.h" "$m/unzip.h" "$m/zip.h" "$p/include/minizip/"
  for f in ioapi unzip zip mztools; do emcc -O2 -sUSE_ZLIB=1 -c "$m/$f.c" -o "$m/$f.o"; done
  emar rcs "$p/lib/libminizip.a" "$m/ioapi.o" "$m/unzip.o" "$m/zip.o" "$m/mztools.o"
fi
if [ ! -f "$p/lib/libcurl.a" ]; then
  mkdir -p "$p/include/curl" && cp "$here/curlstub/curl/curl.h" "$p/include/curl/"
  emcc -O2 -I"$p/include" -c "$here/curlstub/curl.c" -o "$src/curl.o" && emar rcs "$p/lib/libcurl.a" "$src/curl.o"
fi
if [ ! -f "$p/lib/libplibssg.a" ]; then
  # PLIB's 3D parts, with gl4es's headers (sound and joystick come from SDL).
  clone plib $FORGE/plib.git $PLIB_REV
  git -C "$src/plib" apply "$here/plib.diff"
  emcmake cmake -S "$src/plib" -B "$src/plib/build" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-I$p/include" -DCMAKE_CXX_FLAGS="-I$p/include" \
    -DOPENGL_gl_LIBRARY="$p/lib/libGL.a" -DOPENGL_INCLUDE_DIR="$p/include" >/dev/null
  make -s -j"$jobs" -C "$src/plib/build" plibul plibsg plibssg plibssgaux
  for l in ul sg ssg ssgaux; do cp "$(find "$src/plib/build" -name "libplib$l.a" | head -1)" "$p/lib/"; done
  mkdir -p "$p/include/plib" && find "$src/plib/src" -name "*.h" -exec cp {} "$p/include/plib/" \;
fi

# The game.
sd=$work/speed-dreams-code
if [ ! -d "$sd" ]; then
  git clone -q $FORGE/speed-dreams-code.git "$sd"
fi
git -C "$sd" checkout -q -f $SD_REV
git -C "$sd" clean -q -fd src
git -C "$sd" submodule -q update --init
git -C "$sd" apply "$here/speeddreams.diff"
R=$(em-config CACHE)/sysroot
S=$R/lib/wasm32-emscripten
F="$PORTS -I$p/include -fwasm-exceptions"
b=$work/build
emcmake cmake -S "$sd" -B "$b" -Wno-dev -DCMAKE_BUILD_TYPE=Release \
  -DOPTION_OSGGRAPH=OFF -DOPTION_TRACKEDITOR=OFF -DOPTION_3RDPARTY_EXPAT=OFF -DOPTION_CLIENT_SERVER=ON -DOPTION_XRANDR=OFF \
  -DOPTION_CHECK_CONTENTS=OFF -DOPTION_UNLOAD_SSGGRAPH=OFF \
  -DCMAKE_C_FLAGS="$F" -DCMAKE_CXX_FLAGS="$F" -DOPENGL_gl_LIBRARY="$p/lib/libGL.a" -DOPENGL_INCLUDE_DIR="$p/include" \
  -DPLIB_INCLUDE_DIR="$p/include" -DPLIB_UL_LIBRARY="$p/lib/libplibul.a" -DPLIB_JS_LIBRARY="$p/lib/libplibul.a" -DPLIB_SL_LIBRARY="$p/lib/libplibul.a" \
  -DPLIB_SG_LIBRARY="$p/lib/libplibsg.a" -DPLIB_SSG_LIBRARY="$p/lib/libplibssg.a" -DPLIB_SSGAUX_LIBRARY="$p/lib/libplibssgaux.a" \
  -DSDL2_LIBRARY="$S/libSDL2.a" -DSDL2_INCLUDE_DIR="$R/include/SDL2" -DSDL2_MIXER_LIBRARY="$S/libSDL2_mixer-ogg.a" -DSDL2_MIXER_INCLUDE_DIR="$R/include/SDL2" \
  -DSDL2_TTF_LIBRARY="$S/libSDL2_ttf.a" -DSDL2_TTF_INCLUDE_DIR="$R/include/SDL2" \
  -DOGG_LIBRARY="$S/libogg.a" -DOGG_INCLUDE_DIR="$R/include" -DVORBIS_LIBRARY="$S/libvorbis.a" -DVORBIS_INCLUDE_DIR="$R/include" \
  -DVORBISFILE_LIBRARY="$S/libvorbis.a" -DVORBISFILE_INCLUDE_DIR="$R/include" \
  -DZLIB_LIBRARY="$S/libz.a" -DZLIB_INCLUDE_DIR="$R/include" -DPNG_LIBRARY="$S/libpng.a" -DPNG_PNG_INCLUDE_DIR="$R/include" \
  -DJPEG_LIBRARY="$S/libjpeg.a" -DJPEG_INCLUDE_DIR="$R/include" \
  -DCURL_LIBRARY="$p/lib/libcurl.a" -DCURL_INCLUDE_DIR="$p/include" -DENET_LIBRARY="$p/lib/libenet.a" -DENET_INCLUDE_DIR="$p/include" \
  -DCJSON_LIBRARY="$p/lib/libcjson.a" -DCJSON_INCLUDE_DIR="$p/include" -Dminizip_LIBRARIES="$p/lib/libminizip.a" -Dminizip_INCLUDE_DIRS="$p/include" \
  -Drhash_LIBRARIES="$p/lib/librhash.a" -Drhash_INCLUDE_DIRS="$p/include" -Dutf8proc_LIBRARIES="$p/lib/libutf8proc.a" -Dutf8proc_INCLUDE_DIRS="$p/include" >/dev/null
MODS="ssggraph snddefault legacymenu trackv1 simuv5 standardgame telemetryv1"
ROBOTS="human simplix"
make -s -j"$jobs" -C "$b" $MODS $ROBOTS
# The game's own link wants shared modules: only its main.cpp is used.
make -s -C "$b/src/main" main.cpp.o
python3 "$here/wasmmodules.py" "$b/wasmmodules.cpp" $MODS -- $ROBOTS
whole=""
for m in $MODS $ROBOTS; do whole="$whole $(find "$b/src" -name "$m.a")"; done
libs=$(find "$b/src/libs" "$b/freesolid" -name "*.a" | grep -v "_static.a" | tr '\n' ' ')
csnet=$(find "$b/src/modules" -name "*csnetworking.a" | head -1)
em++ -O3 -fwasm-exceptions "$b/src/main/CMakeFiles/speed-dreams-2.dir/main.cpp.o" "$b/wasmmodules.cpp" "$here/glglue.c" \
  -Wl,--whole-archive $whole -Wl,--no-whole-archive $libs $csnet \
  "$p/lib/libplibssgaux.a" "$p/lib/libplibssg.a" "$p/lib/libplibsg.a" "$p/lib/libplibul.a" "$p/lib/libGLU.a" "$p/lib/libGL.a" \
  "$p/lib/libenet.a" "$p/lib/libcjson.a" "$p/lib/libminizip.a" "$p/lib/librhash.a" "$p/lib/libutf8proc.a" "$p/lib/libcurl.a" \
  $PORTS -lopenal -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=268435456 -sSTACK_SIZE=4194304 -sFORCE_FILESYSTEM=1 \
  -sINVOKE_RUN=0 -sEXIT_RUNTIME=0 -sEXPORTED_RUNTIME_METHODS=FS,callMain,cwrap -sEXPORT_ALL=0 \
  -sMIN_WEBGL_VERSION=1 -sMAX_WEBGL_VERSION=2 -sFULL_ES2=1 -o "$here/speeddreams.js"

# The data: upstream's base data as the game installs it, plus the cars and tracks.
stage=$work/stage
rm -rf "$stage"
cmake --install "$b/speed-dreams-data" --prefix "$stage" >/dev/null
d=$stage/share/games/speed-dreams-2
mkdir -p "$work/assets"
asset() { # name rev dest
  [ -d "$work/assets/$1" ] || git clone -q "$FORGE/$1.git" "$work/assets/$1"
  git -C "$work/assets/$1" checkout -q -f "$2"
  mkdir -p "$3" && cp -r "$work/assets/$1/." "$3/" && rm -rf "$3/.git"
}
for c in $CARS; do asset "${c%%:*}" "${c#*:}" "$d/cars/models/${c%%:*}"; done
for t in $TRACKS; do
  name=${t%%:*} rest=${t#*:}
  asset "$name" "${rest%%:*}" "$d/tracks/${rest#*:}/$name"
done
rm -rf "$work/packs" "$work/r2" && mkdir -p "$work/r2/speeddreams"
python3 "$here/pack.py" "$d" "$here/seed" "$work/packs"
cp "$work/packs/packs.json" "$here/"
for z in "$work"/packs/*.zip; do
  n=$(basename "$z" .zip)
  cp "$z" "$work/r2/speeddreams/$n-$(sha256sum "$z" | cut -c1-12).zip"
done
ls -l "$work/r2/speeddreams"
