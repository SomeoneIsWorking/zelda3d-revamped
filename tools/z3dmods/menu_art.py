"""Our own menu pieces, drawn at a texture pack's scale into a menu sheet's empty cells (hud.py's
"ui" edits): the inventory's backdrop, a dark gradient from top to bottom, which the patch stretches
over the screen (patch/oot3d/inventory_screen.cpp)."""

from __future__ import annotations

from PIL import Image

BACKDROP = ((16, 23, 32), (7, 10, 15))


def draw(edit: dict, size: tuple[int, int]) -> Image.Image:
    """The piece an edit names, `size` pixels."""
    name = edit["ui"]
    if name == "backdrop":
        top, bottom = BACKDROP
        column = Image.new("RGBA", (1, size[1]))
        for y in range(size[1]):
            t = y / max(1, size[1] - 1)
            column.putpixel((0, y), (*(round(a + (b - a) * t) for a, b in zip(top, bottom)), 255))
        return column.resize(size)
    raise ValueError(f"no menu piece {name!r}")
