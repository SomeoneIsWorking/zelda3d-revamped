"""Check a texture pack's images against the game's layout of the textures they replace.

A run's texture dump (`z3dmods run ... --harness --no-textures --dump-textures`) holds each game
texture as the game uploads it. Each is paired with the installed textures folder's image of the
same name (`pica_texture.pack_name`), the pack's image is cut down to the game's size, and both are
reduced to their edges. Each block of `BLOCK` texels is matched within `REACH` texels: a block whose
edges line up better moved than in place by more than the noise is off the game's layout by that
move. Textures are listed worst first by the share of their blocks that are off, and the worst
get an overlay in the output folder: the game's edges red, the pack's green, yellow where they meet.
"""

from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path

from PIL import Image, ImageChops, ImageFilter, ImageOps, ImageStat

from z3dmods import pica_texture
from z3dmods.texture_pack import TexturePack

BLOCK = 16
REACH = 3
# How much better, in mean edge difference, a moved block must match to count as off.
MARGIN = 3.0
_DUMPED = re.compile(r"tex1_(\d+)x(\d+)_[0-9A-F]+_(\d+)_mip0\.png$")
_FORMATS = {number: gl_format for gl_format, number in pica_texture.PACK_FORMATS.items()}


@dataclass
class Checked:
    dumped: Path
    pack: Path
    blocks: int
    off: list[tuple[int, int, int, int]]  # x, y, dx, dy in game texels

    @property
    def share(self) -> float:
        return len(self.off) / self.blocks if self.blocks else 0.0


def _edges(image: Image.Image) -> Image.Image:
    return image.convert("L").filter(ImageFilter.FIND_EDGES).filter(ImageFilter.GaussianBlur(0.7))


def _check(dumped: Path, pack_path: Path) -> Checked:
    with Image.open(dumped) as game_image, Image.open(pack_path) as pack_image:
        game = _edges(game_image)
        pack = _edges(pack_image.convert("RGBA").resize(game_image.size, Image.Resampling.BOX))
    width, height = game.size
    blocks, off = 0, []
    for y in range(0, height - BLOCK + 1, BLOCK):
        for x in range(0, width - BLOCK + 1, BLOCK):
            target = game.crop((x, y, x + BLOCK, y + BLOCK))
            if ImageStat.Stat(target).stddev[0] < 8:
                continue
            blocks += 1
            scores = {}
            for dy in range(-REACH, REACH + 1):
                for dx in range(-REACH, REACH + 1):
                    if 0 <= x + dx and 0 <= y + dy and x + dx + BLOCK <= width and y + dy + BLOCK <= height:
                        moved = pack.crop((x + dx, y + dy, x + dx + BLOCK, y + dy + BLOCK))
                        scores[(dx, dy)] = ImageStat.Stat(ImageChops.difference(moved, target)).mean[0]
            (dx, dy), best = min(scores.items(), key=lambda item: item[1])
            if scores.get((0, 0), best) - best > MARGIN:
                off.append((x, y, dx, dy))
    return Checked(dumped, pack_path, blocks, off)


def overlay(checked: Checked, scale: int = 4) -> Image.Image:
    """The game's edges red and the pack's green over the game's size times `scale`."""
    with Image.open(checked.dumped) as game_image, Image.open(checked.pack) as pack_image:
        size = (game_image.width * scale, game_image.height * scale)
        game = game_image.convert("RGBA").resize(size, Image.Resampling.BICUBIC)
        pack = pack_image.convert("RGBA").resize(size, Image.Resampling.LANCZOS)

    def lines(image: Image.Image) -> Image.Image:
        found = image.convert("L").filter(ImageFilter.GaussianBlur(scale / 3)).filter(ImageFilter.FIND_EDGES)
        return ImageOps.autocontrast(found, cutoff=2).point(lambda value: 255 if value > 90 else 0)

    return Image.merge("RGB", (lines(game), lines(pack), Image.new("L", size)))


def run(dump: Path, textures: Path, out: Path, worst: int) -> list[Checked]:
    pack = TexturePack(textures)
    found = {path.name: path for path in textures.rglob("tex1_*_mip0.png")}
    checked = []
    for dumped in sorted(dump.rglob("tex1_*_mip0.png")):
        match = _DUMPED.search(dumped.name)
        if match is None or int(match.group(3)) not in _FORMATS:
            continue
        # A dump without a pack is named by the new hash: a new-hash pack's name already.
        name = dumped.name
        if not pack.new_hash:
            with Image.open(dumped) as image:
                name = pack.name(image.convert("RGBA"), b"", _FORMATS[int(match.group(3))])
        if name in found:
            checked.append(_check(dumped, found[name]))
    checked.sort(key=lambda item: (-item.share, -len(item.off)))
    out.mkdir(parents=True, exist_ok=True)
    for rank, item in enumerate(checked[:worst]):
        overlay(item).save(out / f"{rank:02d}-{item.pack.stem}.png")
    return checked
