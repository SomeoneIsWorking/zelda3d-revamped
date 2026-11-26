"""Our own menu pieces, drawn at a texture pack's scale into a menu sheet's empty cells (hud.py's
"ui" edits): the inventory's panel cut for nine-slicing and its rounded item
cell (patch/oot3d/inventory_screen.cpp)."""

from __future__ import annotations

from PIL import Image, ImageDraw

# The panel and the cells in the game's slate blue, edged lighter.
SLATE = (18, 28, 37, 245)
SLATE_EDGE = (96, 120, 136, 255)
CELL = (30, 46, 58, 255)
CELL_EDGE = (78, 100, 116, 255)
CELL_LIGHT = (120, 146, 162, 110)
# Corner radii in the menu's pixels.
PANEL_RADIUS = 10
CELL_RADIUS = 7


def _rounded(size: tuple[int, int], box: tuple[int, int, int, int], radius: int,
             fill: tuple[int, ...] | None, outline: tuple[int, ...] | None, width: int) -> Image.Image:
    layer = Image.new("RGBA", size, (0, 0, 0, 0))
    ImageDraw.Draw(layer).rounded_rectangle(box, radius, fill=fill, outline=outline, width=width)
    return layer


def draw(edit: dict, size: tuple[int, int], scale: int) -> Image.Image:
    """The piece an edit names, `size` pixels, the sheet's pixels being `scale` of the menu's."""
    name = edit["ui"]
    if name == "panel":
        return _rounded(size, (0, 0, size[0] - 1, size[1] - 1), PANEL_RADIUS * scale, SLATE, SLATE_EDGE, scale)
    if name == "cell":
        box = (scale, scale, size[0] - scale, size[1] - scale)
        cell = _rounded(size, box, CELL_RADIUS * scale, CELL, CELL_EDGE, scale)
        # A lit inner edge along the top, as the game's stone catches the light.
        lit = _rounded(size, (box[0] + scale, box[1] + scale, box[2] - scale, box[3] - scale),
                       (CELL_RADIUS - 1) * scale, None, CELL_LIGHT, scale)
        lit.paste((0, 0, 0, 0), (0, size[1] // 2, size[0], size[1]))
        cell.alpha_composite(lit)
        return cell
    raise ValueError(f"no menu piece {name!r}")
