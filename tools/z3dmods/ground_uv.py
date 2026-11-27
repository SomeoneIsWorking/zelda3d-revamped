"""Floor meshes whose texture the remake's models lay as a patchwork, mapped again by position.

The remakes' floors give each triangle its own piece of a repeating texture: turned, mirrored or
scaled against the triangle beside it (in Kokiri Forest's grass, `s04_yuka_01r`, a third of the
shared edges). The game's own art is blurred toward its edges and hides the breaks; a texture
pack's sharper redraw shows every one as a line. Such a floor's texture coordinates are replaced
by its vertices' world x and z at the mesh's own texels per unit, so every shared edge reads the
same texels on both sides, and the texture repeats across the floor as the hardware wraps it.
"""

from __future__ import annotations

import math
import statistics
import struct
from collections import defaultdict

from PIL import Image, ImageChops, ImageStat

from z3dmods import cmb, pica_texture, zsi

# Share of a mesh's shared edges whose two sides run the texture differently that makes it a
# patchwork; a continuously mapped mesh has none.
PATCHWORK = 0.1
# Share of a mesh's area that must face up (its normal within FLOOR_SLOPE of up) to map it from above.
FLOOR_AREA = 0.9
FLOOR_SLOPE = math.radians(40)
# A texture repeats seamlessly when its opposite edges differ by less than this share of what
# neighbouring lines inside it do (Kokiri Forest's grass: about 0.3; a bordered carpet: over 1).
SEAMLESS = 0.6
# A texture looks alike across each axis when its sixteen bands' mean brightness spreads by less
# than this share of its texels' (ground: at most 0.41; a road strip, dirt between grass: 0.70).
UNIFORM = 0.55
# Share of texels at most half covered that marks a decal (flowers, vines) rather than a floor.
DECAL = 0.01
_DECODED = (pica_texture.ETC1, pica_texture.ETC1A4, pica_texture.RGBA4)
# How far, in texels, the two sides of a shared edge may run apart and still be one mapping.
_SAME_RUN = 1.5
_LIMITS = {"b": 127, "h": 32767, "i": (1 << 31) - 1}


def _triangles(model: cmb.Cmb, shape: cmb.Shape) -> list[tuple[int, int, int]]:
    indices = model.indices(shape)
    return [tuple(indices[i:i + 3]) for i in range(0, len(indices) - 2, 3)]


def _patchwork(positions, coordinates, triangles, texels: tuple[int, int]) -> float:
    """The share of shared edges whose two sides run the texture differently."""
    sides: dict[tuple, list[tuple]] = defaultdict(list)
    for corners in triangles:
        for a, b in ((0, 1), (1, 2), (2, 0)):
            ends = sorted((corners[a], corners[b]), key=lambda index: positions[index])
            sides[(positions[ends[0]], positions[ends[1]])].append(ends)
    shared = differ = 0
    for found in sides.values():
        if len(found) != 2:
            continue
        shared += 1
        (a0, a1), (b0, b1) = found
        runs = [tuple((coordinates[end][axis] - coordinates[start][axis]) * texels[axis] for axis in (0, 1))
                for start, end in ((a0, a1), (b0, b1))]
        if any(abs(runs[0][axis] - runs[1][axis]) > _SAME_RUN for axis in (0, 1)):
            differ += 1
    return differ / shared if shared else 0.0


def _floor(positions, triangles) -> bool:
    """Whether most of the mesh's area faces up."""
    up = total = 0.0
    for a, b, c in triangles:
        (ax, ay, az), (bx, by, bz), (cx, cy, cz) = positions[a], positions[b], positions[c]
        ux, uy, uz = bx - ax, by - ay, bz - az
        vx, vy, vz = cx - ax, cy - ay, cz - az
        normal = (uy * vz - uz * vy, uz * vx - ux * vz, ux * vy - uy * vx)
        area = math.hypot(*normal)
        total += area
        if area and abs(normal[1]) / area >= math.cos(FLOOR_SLOPE):
            up += area
    return total > 0 and up / total >= FLOOR_AREA


def _units_per_texel(positions, coordinates, triangles, texels: tuple[int, int]) -> float:
    """The mesh's median world units per texel along its triangles' edges."""
    found = []
    for corners in triangles:
        for a, b in ((0, 1), (1, 2), (2, 0)):
            i, j = corners[a], corners[b]
            run = math.hypot(*((coordinates[j][axis] - coordinates[i][axis]) * texels[axis] for axis in (0, 1)))
            if run > 1:
                found.append(math.dist((positions[i][0], positions[i][2]), (positions[j][0], positions[j][2])) / run)
    return statistics.median(found)


def _ground(model: cmb.Cmb, texture: cmb.Texture) -> bool:
    """Whether the game's image of `texture` is ground that may be laid anywhere: opaque, alike
    across both axes, and repeating without a seam across its wrap."""
    pixels = model.texture_data(texture)
    if pixels is None or texture.format not in _DECODED:
        return False
    level = pixels[:pica_texture.level_size(texture.width, texture.height, texture.format)]
    decoded = pica_texture.decode(level, texture.width, texture.height, texture.format)
    clear = decoded.getchannel("A").point(lambda alpha: 255 if alpha < 128 else 0)
    if ImageStat.Stat(clear).mean[0] / 255 > DECAL:
        return False
    image = decoded.convert("RGB")
    spread = max(ImageStat.Stat(image.convert("L")).stddev[0], 1.0)
    for turned in (image, image.transpose(Image.Transpose.TRANSPOSE)):
        width, height = turned.size
        band = max(1, width // 16)
        means = [ImageStat.Stat(turned.convert("L").crop((x, 0, x + band, height))).mean[0]
                 for x in range(0, width, band)]
        if statistics.pstdev(means) >= UNIFORM * spread:
            return False

        def apart(a: int, b: int, picture: Image.Image = turned, tall: int = height) -> float:
            lines = (picture.crop((a, 0, a + 1, tall)), picture.crop((b, 0, b + 1, tall)))
            return sum(ImageStat.Stat(ImageChops.difference(*lines)).mean) / 3

        inside = statistics.median(apart(x, x + 1) for x in range(1, width - 2, max(1, width // 32)))
        if apart(width - 1, 0) >= SEAMLESS * max(inside, 1.0):
            return False
    return True


def _shared(model: cmb.Cmb) -> set[int]:
    """Shapes whose texture coordinates another shape reads too: their scale is not theirs alone."""
    owners: dict[int, set[int]] = defaultdict(set)
    for number, shape in enumerate(model.shapes):
        if "tex0" in model.arrays(shape):
            attribute = shape.attribute("tex0")
            size = cmb.COMPONENTS[attribute.data_type][1]
            for index in set(model.indices(shape)):
                owners[attribute.start + index * 2 * size].add(number)
    return {number for found in owners.values() if len(found) > 1 for number in found}


def remap(model: bytearray, base: int) -> list[str]:
    """Map every patchwork floor of the CMB at `base` in `model` by position, in place; the names
    of the textures whose floors were mapped."""
    parsed = cmb.Cmb.parse(bytes(model[base:]))
    shared = _shared(parsed)
    mapped = []
    for mesh in parsed.meshes:
        slot = parsed.materials[mesh.material][0]
        shape = parsed.shapes[mesh.shape]
        if slot < 0 or mesh.shape in shared or not {"position", "tex0"} <= set(parsed.arrays(shape)):
            continue
        if parsed.wrap(mesh.material, 0) != (cmb.GL_REPEAT, cmb.GL_REPEAT):
            continue
        attribute = shape.attribute("tex0")
        code, size = cmb.COMPONENTS[attribute.data_type]
        if code not in _LIMITS and code != "f":
            continue
        texture = parsed.textures[slot]
        texels = (texture.width, texture.height)
        positions = parsed.vertices(shape, "position")
        coordinates = parsed.vertices(shape, "tex0")
        triangles = _triangles(parsed, shape)
        if not _floor(positions, triangles) or _patchwork(positions, coordinates, triangles, texels) < PATCHWORK:
            continue
        if not _ground(parsed, texture):
            continue
        units = _units_per_texel(positions, coordinates, triangles, texels)
        used = sorted({index for corners in triangles for index in corners})
        placed = {index: (positions[index][0] / (units * texels[0]), -positions[index][2] / (units * texels[1]))
                  for index in used}
        scale = attribute.scale
        if code != "f":
            scale = max(abs(value) for point in placed.values() for value in point) / _LIMITS[code]
            struct.pack_into("<f", model, base + parsed.attribute_at(shape, "tex0") + 4, scale)
        start = base + parsed.vatr["tex0"][0] + attribute.start
        for index in used:
            for axis, value in enumerate(placed[index]):
                raw = value / scale if code == "f" else round(value / scale)
                struct.pack_into(f"<{code}", model, start + (index * 2 + axis) * size, raw)
        mapped.append(texture.name)
    return mapped


def room(data: bytes) -> tuple[bytes, list[str]]:
    """The room file with every model's patchwork floors mapped by position, and their textures."""
    out = bytearray(data)
    mapped = [name for model in zsi.Room.parse(data).models() for name in remap(out, model)]
    return bytes(out), mapped
