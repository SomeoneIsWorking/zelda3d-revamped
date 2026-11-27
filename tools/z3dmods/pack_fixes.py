"""Texture pack images whose art is off the game's layout of the texture, moved back onto it.

A pack image replaces a game texture by name, and the game samples it at the game's own
coordinates: art the pack drew elsewhere shows on the wrong faces. A set's texture entry names a
table (`fixes`, under `sets/pack-fixes/`) of such images, each by its path in the pack (`file`),
with knots in the game's texels, `[pack, game]`: the pack's column (`columns`) or row (a `band`'s
`rows`, for the columns from `from` rightward, after the columns) at the first lies at the second.
Between knots the pack's art is stretched straight. The game texture's size is the file name's
(`tex1_<w>x<h>_...`). Each image is rebuilt from the pack's own file on every install, keeping its
alpha; the install leaves the pack's copy out (Azahar keeps one texture per hash)."""

from __future__ import annotations

import io
import re
import tomllib
from pathlib import Path

from PIL import Image

_SIZE = re.compile(r"tex1_(\d+)x(\d+)_")


def _moved(image: Image.Image, knots: list[list[float]], scale: float) -> Image.Image:
    """`image` with its columns moved from each knot's pack column to its game column."""
    out = Image.new(image.mode, image.size)
    for (pack0, game0), (pack1, game1) in zip(knots, knots[1:]):
        width = round(game1 * scale) - round(game0 * scale)
        piece = image.crop((round(pack0 * scale), 0, round(pack1 * scale), image.height))
        out.paste(piece.resize((width, image.height), Image.Resampling.LANCZOS), (round(game0 * scale), 0))
    return out


def _fixed(image: Image.Image, entry: dict, game_width: int) -> Image.Image:
    scale = image.width / game_width
    fixed = _moved(image, entry["columns"], scale) if "columns" in entry else image.copy()
    for band in entry.get("band", []):
        left = round(band["from"] * scale)
        turned = fixed.crop((left, 0, fixed.width, fixed.height)).transpose(Image.Transpose.TRANSPOSE)
        fixed.paste(_moved(turned, band["rows"], scale).transpose(Image.Transpose.TRANSPOSE), (left, 0))
    return fixed


def build(texture_root: Path, table: Path) -> dict[str, bytes]:
    """File name -> PNG: each image of `table`, rebuilt from the pack under `texture_root`."""
    out = {}
    for entry in tomllib.loads(table.read_text())["image"]:
        path = texture_root / entry["file"]
        size = _SIZE.search(path.name)
        if size is None:
            raise RuntimeError(f"{entry['file']}: the name has no texture size")
        with Image.open(path) as image:
            fixed = _fixed(image.convert("RGBA"), entry, int(size.group(1)))
        buffer = io.BytesIO()
        fixed.save(buffer, "PNG")
        out[path.name] = buffer.getvalue()
    return out
