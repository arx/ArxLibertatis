# Arx Libertatis Blender Tools

Import and export Arx Fatalis assets: models (`.ftl`), animations (`.tea`) and
whole levels (`.fts` + `.dlf` + `.llf`).

Arx stores a pile of derived shit that Blender has no equivalent for. Bone
parenting, room membership, navmesh clearance: none of it survives a dumb round
trip and none of it throws an error when it goes missing. The engine swallows the
broken file and quietly does the wrong thing, which is why most of this document
is about what the format actually means instead of which button to press.

---

## Foreword

I am 29. I am a stock controller for Cornetts IGA up here in Far North
Queensland, which is a chain of about thirty independent supermarkets running
from Oakey to Cooktown and out west to Longreach, and if you have not heard of us
that is fine, nobody has.

My job is to turn up on time and count cans of beans. That is genuinely most of
it. The other part is deciphering the Maggi and Gravox meal base sachets, because
apparently not one person in this building can put a sachet on a shelf in the
right spot. They are floppy, they are all the same size, the facings are a
nightmare, and every single week I am standing in aisle four re-sorting Stroganoff
from Chicken Casserole like it is a bloody puzzle game. Twelve years of school and an undergraduate in IT for
this.

So you might reasonably ask why the arse a bean counter wrote a technical manual
for a Blender addon.

Because I lost years. Actual years. I had a character go invisible in game and I
tore apart the animation exporter for months , rewrote the quaternion handling,
verified twenty three stock animations round tripping clean, and it was none of
that. At one point I was living on a couch in atherton deciphering this shit.
It was the level exporter renumbering rooms and forgetting to take the
portal lists with it, so the poor bastard was standing in a room the engine could
not open and therefore refused to draw. Then the same level had every navmesh
anchor written with a positive height, which makes the whole graph unusable by
every creature alive, so NPCs steered blindly at their targets and the cutscene
just sat there. Nothing in the tooling said a word about any of it. No error. No
warning. The engine ate the broken file and shrugged.

Every one of those nights cost me a pack of Double Happiness, which I am aware are
illegal, along with roughly four out of every five cigarettes smoked in this
country. They do not come in plain packaging because they never went past customs.
I know what is probably in them. I keep buying them anyway, because a legal pack
costs more than I spend on food.

I drive a 2008 Lancer with a CVT that is on the way out and cannot be replaced for
anything close to what I have. Last week I took the catalytic converter off with a
hacksaw blade wrapped in a sock. It sounds like a wounded animal now and I have made my peace with
that.

I have had sex once, seven years ago, and it was awful.

That is the whole picture. I am not a programmer. I am a bloke who counts tins for
a living and does this at night because the alternative is thinking about the
first part of this paragraph. Everything in this document was learned the stupid
way, by breaking it, staring at it, and reading the engine source until the reason
turned up. It is written down so the next poor bugger does not have to lose the
same fortnight I did.

Read the format sections. They are the point. The button clicking is the easy bit.

---

## TL;DR for artists

Put the pipe down for five minutes. This bit is the whole job.

Once, before you start:

1. Blender 4.4 or newer.
2. Symlink `arx_addon` into your Blender addons folder (section 2) and tick it on
   in Preferences. Symlink is probably too complicated so just copy the addon shit into the right folder.
3. Point **Arx assets root directory** at your unpacked game data. You will have to figure out how to run arxunpak
4. Leave "Allow use of fallback io library" unticked. If the word "fallback" turns
   up in the console, something is fucked. Go get a programmer.

Editing a character model:

1. File > Import > Arx Fatalis Model (.ftl).
2. Do your thing. Triangles only. No quads, no ngons.
3. Press N in the viewport, go to the **Arx** tab, **Arx Model Setup**.
4. Work down the list until every line is green. Most have a fix button. Do not
   skip one because it looks unimportant, they are all there because someone
   shipped a broken model once.
5. File > Export > Arx Fatalis Model (.ftl).

Editing an animation:

1. New blend file. Import the **model** first (.ftl). That is where the skeleton
   comes from.
2. Click the **mesh**,(object mode) not the skeleton.
3. File > Import > Tea animation (.tea).
4. Set your scene to **24 fps**. Not 25, not 30.
5. Animate. Walking motion goes on the **whole object**, not on a bone.
6. Click the mesh again, File > Export > Tea animation (.tea).

Editing a level:

1. Properties > Scene > Arx Libertatis Levels > Reload Area List.
2. Pick your level, Import Selected Area.
3. Untick both lighting checkboxes (section 7.9). They ship ticked and they will
   wreck your lighting.
4. Do your thing.
5. Changed any geometry? Assign room numbers to the new faces (Arx tab > Arx Face
   Attributes) and hit **Generate Anchors (Navmesh)**. The lighting is getting
   re-baked whether you want it or not.
6. Export FTS / LLF / DLF, or Export All Area Data.
7. Run `python3 repair_fts.py <your fast.fts>`. If it whinges, run it again with
   `--fix`.

Rules that stop you wrecking a level:

- Bone scale stays at 1.0. Scale a bone in a character animation and that body
  part turns into a beach ball in game for a laugh. 
- 24 fps. Always.
- New level geometry needs a room number, or things standing on it can vanish.
- New level geometry needs anchors regenerated, or NPCs walk into walls and
  cutscenes hang. 
- Do not rename paths or zones. The game's scripts call them by name.
- Both lighting checkboxes off 

Broken in game? Section 10 has a symptom table. Nine times out of ten it is one of
the six above.

---

## Contents

1. [Requirements](#1-requirements)
2. [Installing](#2-installing)
3. [Data directory layout](#3-data-directory-layout)
4. [Shared conventions](#4-shared-conventions)
5. [Path A: models (`.ftl`)](#5-path-a-models-ftl)
6. [Path B: animations (`.tea`)](#6-path-b-animations-tea)
7. [Path C: levels and areas (`.fts`, `.dlf`, `.llf`)](#7-path-c-levels-and-areas-fts-dlf-llf)
8. [Verification and repair tools](#8-verification-and-repair-tools)
9. [Known limitations](#9-known-limitations)
10. [Troubleshooting by symptom](#10-troubleshooting-by-symptom)

---

## 1. Requirements

Blender 4.4.0 or newer, per `bl_info["blender"] = (4, 4, 0)`. Developed against
4.4.3.

libArxIO. Not optional. It is the native library that does the PKWare implode and
explode, and without it you are stuffed.

Unpacked game data, as an actual directory tree. The addon does not read `.pak`
files, so run arxunpak first.

Python is whatever Blender ships with. The standalone scripts in this folder need
nothing past the standard library plus libArxIO, except the ones that drive
Blender.

### 1.1 Where libArxIO goes

`lib.py` searches `<arx_addon>/libArxIO.so.0`, then `<arx_addon>/libArxIO.so`,
then the bare name `libArxIO.so.0` left to the system dynamic loader via
`LD_LIBRARY_PATH` or `ldconfig`.

The first two are relative to `lib.py`, so the bloody thing belongs in the
`arx_addon` package directory. That is where the repo already keeps
`libArxIO.so`, `libArxIO.so.0` and `libArxIO.so.1.2.9999.9999`. A CMake build
drops its own copy in the build directory, so symlink or copy that one in if you
are tracking your own builds.

It does not look in the asset root. Chucking it in there and then wondering why
sod all works is a well trodden path.

On Windows the name is `ArxIO.dll`, same package directory. `checkDll` reads the
PE header and tells you to get stuffed if the machine type does not match the
running Blender, so an x64 addon in a 32 bit Blender goes nowhere.

### 1.2 The fallback

The "Allow use of fallback io library" preference exists so a failed library load
degrades instead of throwing. What you get is this and not one thing more:

```python
class ArxIOFallback:
    def unpack(self, data):
        return decompress_ftl(data)   # naivePkware, pure Python
```

Decompression only. There is no `pack`, so anything that has to compress on the
way out is cactus. Level import goes through the same `unpack` and is useless in
practice. And it is slow as a wet week: on a 5 MB `fast.fts`, libArxIO returns
instantly while naivePkware had not finished after two bloody minutes.

If the console says "Failed to load native io library, using slow fallback", stop
right there and fix the library path. Do not work around it, you are only making
more work for yourself.

---

## 2. Installing

Symlink the addon package instead of copying it, so edits to the repo take effect
next time Blender starts:

```sh
ln -s /path/to/ArxLibertatis/plugins/blender/arx_addon \
      ~/.config/blender/4.4/scripts/addons/arx_addon
```

Then in Blender: Edit > Preferences > Add-ons, enable Arx Libertatis Tools, expand
it and set the assets root per section 3. Leave the fallback checkbox alone.

Changing either preference calls `arxAddonReload()`, which rebuilds the managers
and rescans the data directory.

Restart Blender if you edit addon source while it is running. The module level
`importlib.reload` calls in `main.py` and `managers.py` cover some modules and not
others, and half reloaded state will have you chasing bugs that do not exist for
an hour before you twig.

---

## 3. Data directory layout

`files.py` does the scanning. It takes the two layouts the game itself takes, plus
a few case variations of each:

```
<root>/graph/levels/levelN/          level1.dlf, level1.llf, fast.fts
<root>/game/graph/levels/levelN/     (legacy nesting, also searched)
<root>/graph/obj3d/textures/         *.bmp, *.jpg
<root>/graph/obj3d/anims/            **/*.tea
<root>/graph/obj3d/interactive/      **/*.ftl and the .asl beside them
```

An area only appears in the list if the scan found files for it.
`Levels.update()` reports which of DLF / FTS / LLF are missing per area. An area
missing all three still gets listed but cannot be imported, so do not spit the
dummy when bugger all happens.

Texture lookup on model export resolves against `<root>/graph/obj3d/textures`.
Nothing else reads that directory.

---

## 4. Shared conventions

### 4.1 Axes

Arx is Y down, Blender is Z up. The permutation lives in `arx_io_util.py`:

```python
arx_pos_to_blender_for_model(pos) -> (x,  z, -y)
blender_pos_to_arx(pos)           -> (x, -z,  y)
```

It preserves handedness, so it is valid on normals as well as positions.

Worked example, because this one catches everyone. An Arx root translation of
`(0, 0, -147.30)` lands in Blender as `(0, -147.30, 0)`. The walk direction ends
up on Blender's Y. Go looking on Z, find a zero, and you have found the axis
mapping and not a bug, so calm down.

### 4.2 Scale

`scale_factor` is 0.1 everywhere. One Blender unit is ten Arx units. It is exposed
on the TEA exporter and applied by `arx_transform_to_blender` and
`blender_to_arx_transform`.

Do not apply it a second time by hand. That is how you get a walk cycle a tenth
the length it should be and spend an afternoon swearing at the engine.

### 4.3 Rotations

Quaternions get conjugated by the change of basis that matches the position
permutation:

```
import:  q_blender = T · q_arx · T⁻¹        T = [[1,0,0],[0,0,1],[0,-1,0]]
export:  q_arx     = T⁻¹ · q_blender · T
```

Use `T` where the export wants `T⁻¹` and the round trip conjugates twice, spinning
every bone 180 degrees about X. The fingerprint is y and z negated on every
quaternion in the file, which is bloody handy when you are diffing a dodgy export
against its original and cannot work out what is going on.

`q` and `-q` are the same rotation. The exporter can pick either sign and the
shipped files use both, so any comparison you write has to be up to sign or you
will chase phantom differences all arvo.

### 4.4 Scale is an offset from 1

Everywhere Arx stores a per bone scale, meaning TEA `zoom` and the
`EERIE_GROUP::zoom` it turns into, the engine does this:

```cpp
bone.anim.scale = (bone.init.scale + Vec3f(1.f)) * parent.anim.scale;
```

Zero means no scaling. Every stock animation leaves it at zero for a bone nobody
scaled. The addon converts on the way in (`+1`) and out (`-1`), so 1.0 in Blender
is neutral like a sane person would expect.

Write a raw 1.0 into the file yourself and congratulations, you have doubled that
bone and everything hanging off it.

### 4.5 Cylinder heights are negative

Anchor clearance, entity collision cylinders and the pathfinder all use negative
heights, because Y is down. `EERIE_COLLISION_Cylinder_Create` clamps a human to
height `-165`, radius 40 or under. The cylinder system is a piece of shit. Good luck. 

A positive height is not "really tall". It is unusable by every creature in the
game, full stop.

### 4.6 Two workflows, independent of each other

Single asset work is scene independent. File > Import / Export for `.ftl` and
`.tea`, in an empty blend file if you like. No area list, no level import, no
`Area_NN` scene, no fixed scene name, no collections to keep tidy. Sections 5.3
and 6.4.

Area work is not. Properties > Scene > Arx Libertatis Levels builds an `Area_NN`
scene full of collections whose names the exporter hunts by. Section 7.

Keep them in separate blend files unless you have a reason not to.
If you import all the models first, they should show up in a scene imported afterwards. 

---

## 5. Path A: models (`.ftl`)

### 5.1 What FTL actually stores

A vertex list, a triangle list, texture container names, and vertex groups. Each
group has a name, an origin vertex index, and its member vertices.

What it does not store is the bone hierarchy. The engine rebuilds parenting in
`getFatherIndex` by asking, for each group, which earlier group's vertex list holds
this group's origin vertex. `EERIE_CreateCedricData` in `src/scene/Object.cpp`
does the same at load time.

It also does not store an object transform. Your object's location, rotation and
scale in Blender go absolutely nowhere.

Action points, or attach points, are a name plus a vertex index. The engine works
out the driving bone with `getGroupForVertex`, which scans groups in reverse and
therefore uses the highest numbered group holding that vertex.

Because parenting is derived rather than stored, the armature you pose in Blender
belongs to us and not to the format. Its one job at export time is to tell the
exporter which bone parents which, so it can shove each child's origin vertex into
the parent's vertex list and make the engine's rebuild come out the way you meant. This is a pile of shit. 

### 5.2 What the asset root is and is not needed for

FTL import and export take a file path from a browser. They do not care where the
file lives and they do not need a level imported.

The asset root gets used for exactly two things here: the "Textures exist" check,
which lists `<root>/graph/obj3d/textures`, and Import all models, which walks the
whole tree. Everything else works fine on a model sitting on your desktop.

### 5.3 Single model workflow

1. New blend file, or any existing one. No area scene needed.
2. File > Import > Arx Fatalis Model (.ftl).
3. Edit the mesh. Keep it triangles.
4. View3D > sidebar (N) > Arx tab > Arx Model Setup, and work down the checklist
   until every line is green. Section 5.4.
5. File > Export > Arx Fatalis Model (.ftl).

There is one import option, Import Tweaks, off by default. It also loads the
`tweaks/` variants beside the model, meaning alternate heads, armour and so on.
Tweaks are probably fucked up, haven't worked out how to save them separete. 

Import builds the mesh, the `grp:NN:name` vertex groups, the `origin:NN:name`
empties (vertex parented), the attach point empties, and an armature reconstructed
with the same `getFatherIndex` rule the engine uses.

### 5.4 The setup checklist

View3D > sidebar (N) > Arx tab > Arx Model Setup.

Every reason the exporter would knock the mesh back or quietly mangle it, with a
fix button next to the mechanical ones, in the order they are worth fixing.

Transform applied. FTL stores no object transform, so a moved, rotated or scaled
object exports in the wrong bloody spot. Fix button: Apply Transform.

Face data layers. `arx_facetype` (int) and `arx_transval` (float) face attributes
carry polygon type flags and transparency. No layers, nothing to write. Fix
button: Add Face Data.

Triangles only. FTL has no quads. Not one. Fix button: Triangulate.

Material names end in `-mat`. The exporter chops the last four characters and
builds `GRAPH\OBJ3D\TEXTURES\<stem>.<STEM>`. This check walks every object in the
file, not just the active one, because `validate_mesh` does. Fix button: Fix
Material Names.

Textures exist. Nothing verifies the texture is actually in the data directory.
The model loads, renders untextured, and says not a word. Renaming a material to
satisfy the `-mat` rule is how most people end up here. No fix button, sort the
name out or add the file.

Bone vertex groups. Groups must be named `grp:NN:whatever`. `NN` is the group
number and it is what TEA files address. No fix button.

Bone origins on vertices. Each `grp:NN:` needs an `origin:NN:` empty, vertex
parented. An Arx bone is a vertex index and sod all else. Fix button: Snap Origins
To Vertices.

Armature. Needs exactly one root. Groups that do not nest derive a flat skeleton
where every bone is a root, and that animates like a bag of cats. Fix button:
Build Armature From Groups.

Attach points assigned. Every non origin child empty needs an `arx_action_group`
custom property naming the bone that drives it. Fix button: Assign.

Four of those are worth more than a line.

Snapping origins is not just "nearest vertex". Weight painting makes groups
overlap, and once they do, the choice of origin decides the exported hierarchy.
`getFatherIndex` walks back from a bone and stops at the first group holding its
origin, so the origin must not be held by any group numbered between the bone and
its intended parent. The operator enforces that and will not reuse a vertex
another origin has already claimed.

Empty positions are not `empty.location`. Once an empty is vertex parented that
value is an offset from the parent vertex, usually near enough to zero. Read it as
a position and every "nearest vertex" search shoots off to the model origin,
quietly rewiring the whole rig behind your back. The addon uses
`obj.matrix_world.inverted() @ empty.matrix_world.translation`, and so should any
script of your own.

Build Armature From Groups has two modes. From this mesh applies the engine's own
rule, the deepest earlier group holding this bone's origin, which is right for
models whose groups nest, which is all the shipped ones. On disjoint groups it
gives you all roots instead of guessing, which is honest and bloody useless. From
an .ftl file copies parenting and attach point bone assignments from a model whose
group list matches by number, and that is what you want when your mesh is a
re-skin of an existing one. Either way, rebuilding removes the old armature first,
so you do not end up with a graveyard of `foo-amt.001` orphans.

Attach point assignment is a guess. It uses the nearest vertex's deepest group,
which is right for anything held in a hand and wrong for anything hanging off a
bone that carries no geometry. Check the bastard.

### 5.5 Bulk and test operators

Under Properties > Scene > Arx Libertatis Models. Test model export runs the
current scene's objects through the serializer without writing anything. Import
all models bulk loads the data directory, slow as, mostly good for sweeping the
whole set for import regressions.

---

## 6. Path B: animations (`.tea`)

### 6.1 What TEA stores

A header, then one record per keyframe. Each keyframe carries:

`num_frame`, its position on a 24 fps timeline. This is the only timing the engine
uses. `Animation.cpp` puts keyframe i at `num_frame / 24` seconds and the animation
runs `nb_frames / 24`.

`time_frame`, never read by the engine and zero in most shipped files. Ignore it.

`flag_frame`, either `-1` or `9` for a footstep. The engine asserts it is one of
those two and nothing else.

`key_move`, `key_orient` and `key_morph`, saying whether a root translation, root
rotation and morph block follow.

`master_key_frame` and `key_frame`, never read by the engine.

One `THEO_GROUPANIM` per group, holding `key_group` (also never read), a
quaternion, a translation and a `zoom`.

An optional sample record, gated by the field before it, then a `num_sfx` field
the engine skips.

### 6.2 Five ways to wreck a TEA round trip

Zoom written as a factor. See section 4.4. A bone at Blender scale 1.0 goes out as
0 or you have doubled it.

Root translation treated as incremental. `key_move` is a position on the
animation's own timeline. The last frame of a walk cycle carries the whole
distance covered, not a step. `CalcTranslation` interpolates linearly between
consecutive keyframes and `StoreEntityMovement` turns that into the NPC's actual
movement, so a wrong value changes how far the character walks and buggers any
script waiting for them to turn up. It is a right bastard to diagnose in game
because the animation itself looks perfectly fine while the poor sod under-shoots
his mark.

Metadata indexed positionally. Per keyframe metadata is stored against
`num_frame`, not against an index. An export bakes every frame, so the frame it is
writing is usually not one of the keyframes the original had. The importer stashes
`arx_frame_numbers` on the action alongside `arx_frame_flags`,
`arx_frame_durations`, `arx_frame_info_strings`, `arx_keyframe_flags`,
`arx_step_sound_frames` and `arx_sound_effects`, and the exporter looks them up by
timeline position. Index into those lists positionally and every single one lands
on the wrong frame.

A sample record written without its flag. `num_sample` of `-1` means no sample.
Anything else means a 260 byte `THEA_SAMPLE` follows. Write `-1` and the record
anyway and you desync every keyframe after it, because the engine's parser and the
addon's read it the same way. The file is garbage from that point on. `sample_size`
can be 0, since the engine loads the sound by name.

Interpolation weights computed sensibly. `GetTimeBetweenKeyFrames` sums
`frames[kk].time` over `(start, end]`, and `time` is the keyframe's absolute
position on the timeline, so it sums absolute positions. Yes it is a stupid
formula. No, you do not get to fix it, because it is what the engine plays.
`dataTea.py` reproduces it exactly. Sum durations instead and your curve will not
match what the game shows.

### 6.3 Void groups

Quaternions are the complicated bullshit replacement of euler angles. 
The engine decides a bone carries no animation by comparing its quaternion to
identity exactly, and its translation and zoom to zero:

```cpp
if(groups[g].quat != quat_identity() || translate != Vec3f(0.f) || zoom != Vec3f(0.f))
```

A void group is left to whatever lower animation layer is running. That is how
combat and casting overlays sit on top of a walk cycle.

Two consequences. A bone that comes back from Blender as `0.99999994` is the same
rotation but no longer compares equal, so it stops being void and pinches the bone
off the layer underneath. `dataTea.py` snaps near identity back to exactly identity
on export to stop that. And sometimes an author wants a bone kept out of the void
set on purpose so it holds its own bone instead of handing it over. The trick is
the negated identity `(-1, 0, 0, 0)`: same rotation, not equal to identity. Set the
custom property `arx_never_void` on a pose bone and the exporter writes it for you.

### 6.4 Single animation workflow

TEA editing is completely scene independent. You need one thing in the scene: a
mesh with an armature modifier. That is it.

1. New blend file.
2. File > Import > Arx Fatalis Model (.ftl) for the model the animation is meant
   for. That is where the rig comes from.
3. Select the mesh, not the armature.
4. File > Import > Tea animation (.tea).
5. Set the scene to 24 fps.
6. Edit.
7. Mesh still selected, File > Export > Tea animation (.tea).

TEA files are model agnostic. They address bones by group number and the engine
takes `min(bones, nb_groups)`. Any rig whose `grp:NN:` numbering lines up will do,
which is exactly how one animation set drives every human in the game. Build a new
character on the human skeleton, copy the group numbering group for group, and you
inherit the whole stock animation library without exporting a single `.tea`.

Numbering that does not line up gives you an animation that loads fine and poses
the wrong bloody limbs. Nothing checks it. Count your groups.

### 6.5 What import does to the scene

It creates an action, keys every animated bone, and stores the metadata from
section 6.2 on the action as custom properties. Keyframes land on Blender frame
`num_frame + 1`, since Blender counts from one, with LINEAR interpolation. Void
groups are left unkeyed so they stay posed by whatever else is running.

Keyframes are deliberately unevenly spaced. Lay them out end to end at one frame
each and you compress the animation and flatten its rhythm. `human_normal_walk`
comes out 2.6 times too fast that way, which looks ridiculous.

### 6.6 Editing rules

Work at 24 fps. The exporter warns if the scene fps differs, because playback in
Blender will not match the game.

Root motion lives on the mesh object's location and rotation, not on a bone.

Bone scale is neutral at 1.0. Check it before you export. A stray scale on a neck
or spine bone propagates to the whole subtree through
`bone.anim.scale = (init.scale + 1) * parent.anim.scale`, and then you have a
character wandering round with a head like a beach ball.

### 6.7 Export options

File > Export > Tea animation (.tea), mesh active. Action Name defaults to the
active action. Frame Rate is 24.0 and anything else desyncs from the engine, so
leave it alone. Scale Factor is 0.1 and has to match what the import used. TEA
Version is 2015; 2014 drops the 256 byte `info_frame`, the engine takes both, and
it rejects anything below 2014 outright.

The export bakes every frame in the action's range and writes `key_move` and
`key_orient` on all of them, so nothing is left to the engine's own fill in.

### 6.8 Verifying

```sh
./run_tea_roundtrip_test.sh <original.tea> <model.ftl>
```

Imports the animation onto the model, exports it again, compares. It pairs frames
by `num_frame`, since counts legitimately differ after a re-bake, compares
quaternions up to sign, and treats an absent root translation as an explicit zero.
A clean result prints `Compare: Ok`.

`BLENDER_PATH` and `GAME_DIR` are hardcoded at the top of the script, so edit the
bastard for your machine. Paths you pass in have to live under `GAME_DIR` or the
model importer knocks the name back as too long.

---

## 7. Path C: levels and areas (`.fts`, `.dlf`, `.llf`)

### 7.1 The three files

`fast.fts` holds background geometry as a tile grid, textures, the anchor navmesh,
portals, rooms, and the room distance matrix. `levelN.dlf` holds entity
placements, paths, zones, fogs and the scene header. `levelN.llf` holds baked
vertex lighting and the light list.

FTS and DLF bodies are PKWare imploded. The FTS header's `uncompressedsize` field
gates decompression: `FastSceneLoad` only calls `blast` when it is non zero, so a
hand written FTS can be stored uncompressed by setting that field to 0. The DLF
body is compressed from version 1.44 onward.

### 7.2 Importing

Properties > Scene > Arx Libertatis Levels. Reload Area List rescans the data
directory. Pick an area, then Import Selected Area.

You get a scene called `Area_NN` holding `<scene>-background` (the level geometry,
one mesh), `<scene>-anchors` (the navmesh, one vertex per anchor and one edge per
link), `<scene>-portals` (one mesh object per portal quad), `<scene>-lights` and
`<scene>-dlf-lights`, `<scene>-entities` (one sub collection per entity id),
`<scene>-paths`, `<scene>-zones` (each with an editable `zone_outline:` child mesh)
and `<scene>-fogs`.

Import All Levels does the lot. Go make a cuppa, it takes bloody ages.

### 7.3 Face attributes on the background mesh

| Attribute | Type | Meaning |
|---|---|---|
| `arx_polytype` | int, per face | Polygon type flags (`POLY_QUAD`, `POLY_WATER`, `POLY_TRANS` and so on). |
| `arx_transval` | float, per face | Transparency value. |
| `arx_room` | int, per face | Room membership. Rooms number from 1. `0` is the engine's unused first slot, `-1` means the polygon belongs to no room. |
| `arx_facetype` | int, per face | Model side face flags, shared with the FTL path. |

Edit them in View3D > Arx tab > Arx Face Attributes: set a room number, then
Assign Room To Selected, Select Faces In Room, List Rooms. Works in both Object
and Edit mode. In Edit mode it reads the live bmesh, because the mesh attribute
arrays are stale there and reading those would just feed you rubbish.

### 7.4 Rooms and portals

Room ids are positions in an array, not names. The engine sizes its room array
from the highest id it sees and indexes straight in, so one room numbered 420
costs you 421 room entries and a 421 by 421 distance matrix. Past
`MAX_ROOMS = 255`, `ComputePortalVertexBuffer` gives up before building a single
vertex buffer and the whole level renders black.

So the exporter compacts room ids into a contiguous range on the way out, in
`_buildRoomMap`. Four things are indexed by room id and all four have to move
together: polygon `arx_room` attributes, portal `room_1` and `room_2`, each room's
own portal index list and polygon references, and the room distance matrix.

Portal index lists need no adjustment in themselves, since they point into the
portal array and that keeps its order, but they have to travel with their room.

Here is what happens when they do not all move together, because this one cost a
week. A room ends up holding the portal list of whichever room used to sit at that
index. Rooms that replaced an empty one get no portals at all.
`ARX_PORTALS_Frustrum_ComputeRoom` can then never open them, the frustum list
stays empty, and `ARX_SCENE_PORTAL_ClipIO` answers an empty frustum list by hiding
every entity standing in that room. The room geometry still draws. The people in
it do not, and you will swear blind the character model is broken when the model
is perfectly bloody fine.

Portals are objects in a collection whose name contains `portals`. They carry
`arx_room_1` and `arx_room_2` for the two rooms they join, and `arx_useportal`,
which is 1 to take part in traversal. Object > Add Portal Properties
(`arx.portal_init`) adds them.

### 7.5 The room distance matrix

`SP_GetRoomDist` returns:

```cpp
fdist(from, dist.startpos) + dist.distance + fdist(dist.endpos, to)
```

`startpos` and `endpos` have to be real waypoints on the route, the centres of the
first and last doorway along it, with `distance` the run between them. A distance
of 0 reads as "no route" and the engine falls back to straight line distance,
which is safe.

A big distance with the waypoints left on the world origin is not safe. Every
cross room measurement comes out at roughly the level's distance from the origin,
somewhere around a million units. `PrepareIOTreatZone` compares that against
`TREATZONE_LIMIT`, a few thousand, and drops the entity out of the treat zone. No
physics, no drawing, sod all, until the player walks into the same room. Every
entity in the level goes at once and it looks for all the world like a rendering
bug.

The exporter derives the matrix from the portal graph, in
`compute_room_distances`, for any entry it cannot preserve.

### 7.6 Anchors

NPCs cannot move without a navmesh, and geometry built in Blender turns up without
one.

Properties > Scene > Arx Libertatis Levels > Generate Anchors (Navmesh) rebuilds
it from the background geometry. Export also generates one when the level has none
at all, but it will not overwrite an existing graph, on the grounds that a hand
edited one is the author's and not the exporter's to stuff about with.

The graph is a mesh: one vertex per anchor, one edge per link. Per vertex
attributes carry each anchor's cylinder:

| Attribute | Default | Notes |
|---|---|---|
| `arx_anchor_radius` | 45.0 | DANAE's own anchors run 40 to 50. |
| `arx_anchor_height` | -165.0 | Negative. Section 4.5. |
| `arx_anchor_flags` | 0 | |

`PathFinder::move` skips any node where `height > m_height || radius < m_radius`,
with `m_height` the creature's cylinder height, which is negative. A positive
anchor height therefore fails against every creature alive and leaves you with a
navmesh nothing can use. NPCs fall back to steering straight at their target,
which looks like working pathfinding right up until something has to arrive at an
exact spot, and then your cutscene sits there doing bugger all while you wonder
what the hell happened.

Editing the anchor mesh directly is fine. Move vertices to move anchors, add or
remove edges to change links. Keep the per vertex attributes intact: missing or
wrong length and the exporter substitutes defaults, which is exactly how you get
the mess above.

### 7.7 Zones and paths

A zone and a path are the same record. `arx_zone_height` is the only thing telling
them apart, and a height of 0 makes it a path rather than a volume. The panel warns
you about this, and it warns you for a reason.

Properties > Object > Arx Zone shows up for any object with `arx_zone_height`.
Height is `arx_zone_height`, 0 meaning path. Ambiance is `arx_zone_flags` bit 1
(value 2) and plays `arx_zone_ambiance` while the player is inside, with volume
from `arx_zone_amb_max_vol`. Colour is bit 2 (value 4) and tints the view via
`arx_zone_rgb`. Far Clip is bit 3 (value 8) and overrides view distance via
`arx_zone_farclip`.

The zone's footprint is an editable `zone_outline:` child mesh. Only the footprint
counts, since the engine tests X and Z, and fewer than three points cannot enclose
a bloody thing. Levels imported before outline meshes existed have
`zone_waypoint:` empties instead, so re-import to convert.

Path pathways carry `arx_pathway_flag` and `arx_pathway_time` attributes. Camera
cutscenes drive cameras along these by name, `SET_PATH PATH_FIRST` and mates in
the camera's `.asl`, so rename a path and you have silently broken every script
that references it. Nothing will tell you. Nothing.

### 7.8 Exporting

Properties > Scene > Arx Libertatis Levels, area selected. Export FTS (Geometry)
writes geometry, rooms, portals, anchors and the distance matrix. Export LLF
(Lighting) writes lighting only. Export DLF (Entities/Zones) writes entities,
paths, zones and fogs, skipping all the expensive FTS processing, so it is the
quick one for shuffling props about. Export All Area Data does all three.

What an FTS export does, in order, and the order matters. Portals are read back
from the `portals` collection if there is one, and no collection means the
original portal data is kept. Anchors are read back from the `anchors` collection
if there is one, and none at all means a navmesh gets generated. Rooms are
renumbered in `_buildRoomMap`, with polygons, portals, room structures and the
distance matrix all moving onto the new ids together. Room polygon references are
rebuilt, but only if geometry changed or portals were added, otherwise the
originals are kept.

Change geometry and two things follow whether you like it or not. The lighting
bake is forced to rebuild, because keeping a bake for a mesh that no longer
matches throws vector bounds errors. And you assign the room ids yourself: new
faces default to room 0, the engine's dead slot, where they will not be room
culled but do not belong anywhere either.

### 7.9 Lighting and the two checkboxes

Properties > Scene, the lighting panel. Two boxes, one checkbox each, both ticked
out of the box. Untick both. The reason is different for each one.

`import_original_lighting`, in the Import Settings box, does nothing. It is
declared, drawn, and then never read anywhere in the addon. Untick it for tidiness
if you like, but it changes sod all behaviour and it is not the cause of a bad
bake.

`regenerate_lighting`, in the Export Lighting box, is the one that matters. On,
and an LLF export calls `updateLlfFile` and rewrites the level's baked vertex
lighting with whichever `lighting_method` is selected. Off, and the exporter skips
it and leaves your original bake the hell alone.

With it off the exporter reports why it skipped, `regenerate_lighting=False`,
which is the message you want to see. Left on, every export re-bakes the whole
level from the Blender side, and unless the lights, normals and ambient are set up
exactly as DANAE had them the result will not match what shipped. Turn it on when
you are actually re-lighting, not as a default.

Setting `lighting_method` to `SKIP` does the same job by another route, and the
export says `method=SKIP`.

The nested DANAE Bake Settings, `danae_use_normals`, `danae_raylaunch` and
`danae_ambient`, only appear while `regenerate_lighting` is on and only apply to
the `DANAE` method. Those defaults reproduce the original editor, so leave them be
if you are baking. Note the panel's own warning: the bake uses the Arx light
properties, not Blender wattage, so cranking a Blender light does absolutely sod
all.

There is also an internal flag, `_preserve_original_lighting`, that the UI does
not expose. LLF export sets it False so vertex lighting is recalculated. FTS only
export sets it True to skip an expensive calculation. Changed geometry forces it
False regardless, per section 7.8.

---

## 8. Verification and repair tools

Standalone scripts in this folder. The ones that need Blender say so.

`repair_fts.py <fast.fts> [--fix]` checks a round tripped FTS for stale room to
portal references, rooms with geometry but no portals, and anchors with a non
negative height. `--fix` rebuilds the room section from the portals, recomputes
the distance matrix from the portal graph, and resets crook anchor heights to
`-165`. It keeps the original as `.bak` and will not clobber one that already
exists. Writes the payload uncompressed.

`run_tea_roundtrip_test.sh <tea> [ftl]` needs Blender. Import, export, compare an
animation. Section 6.8. `run_ftl_roundtrip_test.sh <ftl>` is the same for models,
also needs Blender. `test_tea_roundtrip.py` is the script the TEA runner drives.

`test_tea_timing.py` does keyframe placement and duration checks.
`tea_diff_tool.py <a> <b>` structurally diffs two TEA files. `tea_debug.py` dumps
a TEA file's frames and groups. `compare_ftl.py` structurally diffs two FTL files.

`checkAssets.py` sweeps the data directory for assets that fail to load, and
`checkCinematics.py` does the same for `.cin` files. `validateSceneData.py` runs
consistency checks over a level's scene data. `test_fts_rooms.py` checks room and
portal structure.

`level_lighting_dump.py` dumps a level's lighting for a squiz.
`test_danae_lighting.py` checks the lighting model against `danae_lighting.py`.
`bake_check.py` verifies a lighting bake.

`make_test_level.py` builds a synthetic level from scratch, and doubles as the
reference for what a hand authored FTS actually has to contain. `catalog.py`
inventories the data directory.

`anchor_generation.py` has no Blender imports on purpose, so it serves both the
addon and the standalone level generator, and you can run it directly for a self
check.

---

## 9. Known limitations

Embedded TEA audio is not preserved. The shipped `human_death.tea` carries 14 KB
of PCM inline. Import chucks it and export writes `sample_size = 0`. The engine
loads the sample by name from the sfx directory so the sound still plays, but the
bytes are gone from the file.

Compound Euler rotations do not survive a DLF round trip. Entities with rotation
on more than one axis can come back 90 degrees out on two of them, seen on
`HELMET_PLATE_AC`. Single axis rotations are fine, and angles differing by a
multiple of 360 are equivalent as far as the engine cares.

An animation can have more groups than the model has bones. The engine takes
`min(bones, nb_groups)` and ignores the rest, so exporting such an animation
against a smaller rig silently drops the extras. Check your group counts before
you go blaming the exporter.

Some bulk import operators wipe collections. A few of the animation test operators
remove every collection in the file before importing, so do not run them in a file
with unsaved work in it unless you fancy redoing your whole afternoon.

`key_group`, `master_key_frame` and `key_frame` are never read by the engine. They
are preserved best effort and will tell you bugger all when you are debugging.

---

## 10. Troubleshooting by symptom

| Symptom | Likely cause | Where to look |
|---|---|---|
| Character invisible, geometry round them fine | Room culling. Their room has an empty frustum list, or its portal list belongs to some other room. | `repair_fts.py <fast.fts>` |
| Everything in one area invisible and frozen | Room distance matrix has big distances with zeroed waypoints, so entities fall out of `PrepareIOTreatZone`. | 7.5, `repair_fts.py` |
| NPCs walk but never arrive, scripts waiting on arrival never fire | Anchor heights positive, navmesh unusable, NPCs steering straight at targets. | 7.6, `repair_fts.py` |
| Character scrambled or inside out after an animation export | Rotation conjugated twice. Check whether every quaternion has y and z negated against the original. | 4.3 |
| One body part enormous | A bone scale that is not 1.0 in Blender, or `zoom` written as an absolute scale. | 4.4 |
| Walk cycle covers the wrong distance | Root translation treated as incremental instead of absolute, or scaled twice. | 6.2 |
| Footstep sounds on nearly every frame | `flag_frame` guessed instead of taken from `arx_step_sound_frames`. | 6.2 |
| Exported TEA unreadable past the first sound | `num_sample` written as `-1` with the sample record present anyway. | 6.2 |
| Animation poses the wrong limbs | Group numbering does not line up between the animation and the rig. | 6.4 |
| Model renders untextured | Material name does not resolve to a file under `graph/obj3d/textures`. | 5.4 |
| Model animates like a bag of cats | Flat skeleton, every bone a root because the groups do not nest. | 5.4 |
| Lighting looks nothing like it did before an export | `regenerate_lighting` left on, so the export re-baked the level. | 7.9 |
| Vector bounds errors during export | Geometry changed, so the bake was forced to regenerate against a mesh that no longer matches. | 7.9 |
| Level renders black | More than 255 rooms, `ComputePortalVertexBuffer` bails out. | 7.4 |
| Level import does sod all | `libArxIO` did not load and the fallback is on. It only handles FTL. | 1.1, 1.2 |
