"""Splits the installed Speed Dreams data into the zips the play page downloads.

start: everything the menus need (config, menu art, fonts, sounds, robots'
settings) and every car's and track's description and preview pictures, so
all of them are listed from the start, plus lib/ (empty stand-ins for the
modules, which are linked into the game but looked for as files) and seed/
(first-run settings: a Quick Race on Brno against four AI drivers, from
beside this script, and steering with the arrow keys); music: the menu and
race music; car-<id> and track-<id>: each car's and track's models and
textures. A race waits until its track and cars are in
(Module.wgDataReady in speeddreams.diff).
Leaves out: the robots that aren't linked, the race modes that are online or
need cars and tracks not here (the Supercars championship runs on the tracks
here instead), and source
files (.xcf, .psd, .blend, .bz2 and the cars' unused .ac models). Lists
only the kept files in user-files. Makes ssggraph the 3D module (upstream's default, osggraph, isn't built).
Writes <out>/<name>.zip and <out>/packs.json (which folders each zip holds).
Usage: python3 pack.py <data dir> <seed dir> <out dir>
"""
import json, os, re, sys, zipfile

data, seed, out = sys.argv[1], sys.argv[2], sys.argv[3]
os.makedirs(out, exist_ok=True)

MODULES = ["graphic/ssggraph", "sound/snddefault", "userinterface/legacymenu", "track/trackv1",
           "simu/simuv5", "racing/standardgame", "telemetry/telemetryv1"]
ROBOTS = ["human", "simplix"]
SKIP_DRIVERS = {"axiom", "dandroid", "shadow", "urbanski", "usr"}
# Race modes kept: the others are online or need cars and tracks not here.
RACES = {"quickrace.xml", "practice.xml", "championship-supercars.xml"}
SOURCES = (".xcf", ".psd", ".blend", ".bz2")
# What the menus show of a car or track before its zip is in.
LIGHT = re.compile(r"(\.xml|preview\.jpg|^outline\.png|^readme\.txt|^[^/]+\.jpg)$")

packs = {"start": [], "music": []}
zips = {"start": {}, "music": {}}

def add(pack, name, path=None, text=None):
    zips.setdefault(pack, {})[name] = (path, text)

def used_ac(folder):
    xml = "".join(open(os.path.join(folder, f), encoding="utf-8", errors="replace").read()
                  for f in os.listdir(folder) if f.endswith(".xml"))
    return set(re.findall(r'val="([^"]+\.ac)"', xml))

TRACKS = sorted((c, t) for c in os.listdir(os.path.join(data, "tracks")) if c != "categories"
                for t in os.listdir(os.path.join(data, "tracks", c)))

def championship(text):
    """The Supercars championship, on the tracks here."""
    start = text.index('<section name="Tracks">')
    end = text.index('<section name="Races">', start)
    events = "".join(f"""    <section name="{i}">
      <attstr name="name" val="{t}"/>
      <attstr name="category" val="{c}"/>
    </section>

""" for i, (c, t) in enumerate(TRACKS, 1))
    return text[:start] + f"""<section name="Tracks">
    <attnum name="maximum number" val="20"/>
{events}  </section>

  """ + text[end:]

for root, dirs, files in os.walk(data):
    dirs[:] = sorted(d for d in dirs if d != ".git")
    rel = os.path.relpath(root, data).replace(os.sep, "/")
    parts = [] if rel == "." else rel.split("/")
    if parts[:1] == ["drivers"] and len(parts) > 1 and parts[1] in SKIP_DRIVERS:
        continue
    car = parts[2] if parts[:2] == ["cars", "models"] and len(parts) > 2 else None
    track = parts[2] if parts[:1] == ["tracks"] and len(parts) > 2 and parts[1] != "categories" else None
    keep_ac = used_ac(root) if car and len(parts) == 3 else None
    for f in sorted(files):
        name = f"data/{rel}/{f}" if parts else f"data/{f}"
        path = os.path.join(root, f)
        if f.lower().endswith(SOURCES) or f in ("CMakeLists.txt", ".gitignore"):
            continue
        if parts == ["config", "raceman"] and f not in RACES:
            continue
        if parts == ["config", "raceman"] and f == "championship-supercars.xml":
            add("start", name, text=championship(open(path, encoding="utf-8").read()))
            continue
        if keep_ac is not None and f.endswith(".ac") and f not in keep_ac:
            continue
        if parts == ["config"] and f == "raceengine.xml":
            text = open(path, encoding="utf-8").read()
            text, n = re.subn(r'(<attstr name="graphic" val=")[os]sggraph(")', r"\1ssggraph\2", text)
            assert n == 1, "raceengine.xml: graphic module not found"
            add("start", name, text=text)
            continue
        item = f"car-{car}" if car else f"track-{track}" if track else None
        if item:
            folder = "/".join(parts[:3])
            packs[item] = [folder]
            add(item, name, path)
            sub = "/".join(parts[3:] + [f])
            if LIGHT.search(sub) and len(parts) == 3:
                add("start", name, path)
        elif parts[:2] == ["data", "music"]:
            add("music", name, path)
        else:
            add("start", name, path)

# The settings copied to the player's folder on first run: the game stops at
# the first one it can't find, so only those kept here.
kept = [l for l in open(os.path.join(data, "user-files"), encoding="utf-8").read().splitlines()
        if l and f"data/{l}" in zips["start"]]
add("start", "data/user-files", text="\n".join(kept) + "\n")

for m in MODULES:
    add("start", f"lib/modules/{m}.so", text="")
for r in ROBOTS:
    add("start", f"lib/drivers/{r}/{r}.so", text="")
# The player steers with the arrow keys (upstream's default is the mouse).
prefs = open(os.path.join(data, "drivers/human/preferences.xml"), encoding="utf-8").read()
for side in ("LEFT", "RIGHT"):
    prefs, n = re.subn(f'val="MOUSE_{side}"', f'val="{side.title()} Arrow"', prefs, count=1)
    assert n == 1, f"preferences.xml: MOUSE_{side} not found"
add("start", "seed/drivers/human/preferences.xml", text=prefs)
for root, _, files in os.walk(seed):
    for f in files:
        p = os.path.join(root, f)
        add("start", "seed/" + os.path.relpath(p, seed).replace(os.sep, "/"), p)

STORED = (".png", ".jpg", ".ogg", ".ttf", ".otf")
for pack, files in zips.items():
    with zipfile.ZipFile(os.path.join(out, pack + ".zip"), "w") as z:
        for name in sorted(files):
            path, text = files[name]
            info = zipfile.ZipInfo(name, (2020, 1, 1, 0, 0, 0))
            comp = zipfile.ZIP_STORED if name.lower().endswith(STORED) else zipfile.ZIP_DEFLATED
            body = text.encode() if text is not None else open(path, "rb").read()
            z.writestr(info, body, compress_type=comp, compresslevel=9)
    print(pack, len(files), os.path.getsize(os.path.join(out, pack + ".zip")))
json.dump({k: v for k, v in packs.items() if k not in ("start", "music")}, open(os.path.join(out, "packs.json"), "w"), indent=1)
