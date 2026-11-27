"""The game art the app's play HUD draws (app/src/hud.hpp): item icons cut from the game's own icon
sheet, or the texture pack's copy of it, by the game's table of an icon frame per item id, and
named pieces (the rupee, the key, the hearts) cut from the HUD's sheet the same way.

`patch/<game>/hud.toml` names them under `[icons]`: `sheet` (a romfs CTXB, or `<archive>.gar/<file>`),
`frame` (a frame's size in the sheet's own pixels), `columns` and `icons` (how many frames are
icons; the sheet holds other art after them). A game with a table of an icon frame per item id
names it: `frames` (the decomp's global), `count` and `none` (its length and its "no icon" value).
Without one, an item's frame is its id.

`[art]` names the HUD's sheet (`sheet`) and each piece's rectangle in the sheet's own pixels
(`<name> = [x, y, w, h]`).

A piece or icon from the pack's copy keeps only the pack's ink joined to the game's own art in that
cell: the pack's upscaled art spills a few pixels over the game's cell edges, and a cut taking a
neighbour's spill in draws it as a line down the HUD.

`[words]` names the sheet of the A button's words: cells of `cell` pixels from `first`, `rows` to
a column, counted down each column; an action's number is its word's cell. `count` is how many
cells are words and `none` the number that shows no word (the decomp's defines). `{language}` in
`sheet` is each of `languages` (system language -> the sheet's folder); a word is cut once a
language, as `word_<the CFG module's language number>_<action>`."""

from __future__ import annotations

import io
import tomllib
from pathlib import Path

from z3dmods import ctr, ctxb, gar, symbols, system_config
from z3dmods.texture_pack import TexturePack

from PIL import Image, ImageDraw, ImageFilter

# Alpha above this is the pack's ink when telling a cell's art from a neighbour's spill, and how
# far (an odd square, in the pack's pixels) ink reaches to join the ink near it and keep its fainter edge.
SPILL_INK = 8
SOFT_EDGE = 5
_FILLED, _JOINED = 255, 128
# Where the loader maps the code image.
IMAGE_BASE = 0x00100000
# An icon's longest side as installed; a HUD button is never drawn larger.
LARGEST = 256


class _Sheet:
    """A romfs sheet: the game's own and the pack's copy when it has one."""

    def __init__(self, romfs: ctr.Romfs, pack: TexturePack | None, name: str) -> None:
        archive, gar_mark, inside = name.partition(".gar/")
        file = gar.files(romfs.read(archive + ".gar"))[inside] if gar_mark else romfs.read(archive)
        vanilla, gl_format, data = ctxb.image(file)
        self.vanilla = vanilla.convert("RGBA")
        self.image = ((pack.replacement(vanilla, data, gl_format) if pack else None) or vanilla).convert("RGBA")
        # How many of the image's pixels one of the game's is.
        self.scale = self.image.width / vanilla.width

    def cut(self, left: int, top: int, width: int, height: int) -> Image.Image:
        """A cell of the game's pixels, from the image."""
        return self.image.crop(tuple(round(side * self.scale) for side in (left, top, left + width, top + height)))

    def art(self, left: int, top: int, width: int, height: int) -> Image.Image:
        """A cell with only the ink joined to the game's own art in it, and that ink's soft edge."""
        cell = self.cut(left, top, width, height)
        if self.scale == 1:
            return cell
        ink = (cell.getchannel("A").point(lambda alpha: _FILLED if alpha > SPILL_INK else 0)
               .filter(ImageFilter.MaxFilter(SOFT_EDGE)))
        game = self.vanilla.crop((left, top, left + width, top + height)).getchannel("A")
        for y in range(height):
            for x in range(width):
                at = (min(int((x + 0.5) * self.scale), cell.width - 1), min(int((y + 0.5) * self.scale), cell.height - 1))
                if game.getpixel((x, y)) and ink.getpixel(at) == _FILLED:
                    ImageDraw.floodfill(ink, at, _JOINED)
        joined = ink.point(lambda value: 255 if value == _JOINED else 0)
        return Image.composite(cell, Image.new("RGBA", cell.size, (0, 0, 0, 0)), joined)


def _png(sheet: _Sheet, left: int, top: int, width: int, height: int) -> bytes:
    icon = sheet.art(left, top, width, height)
    icon.thumbnail((LARGEST, LARGEST), Image.Resampling.LANCZOS)
    buffer = io.BytesIO()
    icon.save(buffer, "PNG")
    return buffer.getvalue()


def build(repo: Path, game: str, romfs: ctr.Romfs, pack: TexturePack | None, code: bytes) -> dict[str, bytes]:
    """File stem -> PNG: an item's id or a piece's name; nothing for a game without a hud.toml."""
    path = repo / "patch" / game / "hud.toml"
    tables = tomllib.loads(path.read_text()) if path.exists() else {}
    out = {}
    if (art := tables.get("art")) is not None:
        sheet = _Sheet(romfs, pack, art["sheet"])
        out.update({name: _png(sheet, *box) for name, box in art.items() if name != "sheet"})
    if (table := tables.get("icons")) is not None:
        out.update(_items(repo, game, romfs, pack, code, table))
    if (table := tables.get("words")) is not None:
        decomp = symbols.load(repo, game)
        for language, folder in table["languages"].items():
            sheet = table["sheet"].replace("{language}", folder)
            out.update(_words(decomp, romfs, pack, table, sheet, f"word_{system_config.LANGUAGES[language]}_"))
    return out


# Alpha above this is a word's ink.
INK = 32


def _words(decomp: symbols.Decomp, romfs: ctr.Romfs, pack: TexturePack | None, table: dict, name: str,
           prefix: str) -> dict[str, bytes]:
    """`<prefix><number>`: each action word of the sheet `name`, its cell cut to the ink's width at the
    cell's height."""
    sheet = _Sheet(romfs, pack, name)
    (left, top), (width, height) = table["first"], table["cell"]
    none = decomp.define(table["none"])
    words = {}
    for number in range(decomp.define(table["count"])):
        x, y = left + number // table["rows"] * width, top + number % table["rows"] * height
        cell = sheet.cut(x, y, width, height)
        ink = cell.getchannel("A").point(lambda alpha: 255 if alpha > INK else 0).getbbox()
        if number == none or ink is None:
            continue
        word = cell.crop((ink[0], 0, ink[2], cell.height))
        word.thumbnail((LARGEST, LARGEST), Image.Resampling.LANCZOS)
        buffer = io.BytesIO()
        word.save(buffer, "PNG")
        words[f"{prefix}{number}"] = buffer.getvalue()
    return words


def _items(repo: Path, game: str, romfs: ctr.Romfs, pack: TexturePack | None, code: bytes,
           table: dict) -> dict[str, bytes]:
    decomp = symbols.load(repo, game)
    sheet = _Sheet(romfs, pack, table["sheet"])
    width, height = table["frame"]
    if "frames" in table:
        start = symbols.global_(decomp, table["frames"]) - IMAGE_BASE
        frames = code[start:start + decomp.define(table["count"])]
        none = decomp.define(table["none"])
    else:
        frames = range(table["icons"])
        none = None
    icons = {}
    for item, frame in enumerate(frames):
        if frame == none or frame >= table["icons"]:
            continue
        icons[str(item)] = _png(sheet, frame % table["columns"] * width, frame // table["columns"] * height,
                                width, height)
    return icons
