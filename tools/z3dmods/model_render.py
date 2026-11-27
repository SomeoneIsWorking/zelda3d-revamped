"""A room model's textured mesh drawn from above, from the model and an image alone.

Each triangle is drawn in an orthographic view down the y axis, its texture coordinates mapped
straight (exact for that view), the texture repeating, with no lighting, filtering past the image's
own, or emulator. A line in the render is in the model's coordinates and the image; one that shows
in play and not here is drawn by something else.
"""

from __future__ import annotations

import math
from dataclasses import dataclass

from PIL import Image, ImageDraw

from z3dmods import cmb


@dataclass(frozen=True)
class View:
    """The world's x, z box drawn, and pixels per world unit."""

    left: float
    top: float
    right: float
    bottom: float
    scale: float

    @property
    def size(self) -> tuple[int, int]:
        return round((self.right - self.left) * self.scale), round((self.bottom - self.top) * self.scale)

    def pixel(self, x: float, z: float) -> tuple[float, float]:
        return (x - self.left) * self.scale, (z - self.top) * self.scale


def _solve(points: list[tuple[float, float]], values: list[float]) -> tuple[float, float, float]:
    """a, b, c with a*x + b*y + c = value at the three points."""
    (x0, y0), (x1, y1), (x2, y2) = points
    det = (x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0)
    v0, v1, v2 = values
    a = ((v1 - v0) * (y2 - y0) - (v2 - v0) * (y1 - y0)) / det
    b = ((x1 - x0) * (v2 - v0) - (x2 - x0) * (v1 - v0)) / det
    return a, b, v0 - a * x0 - b * y0


def render(model: cmb.Cmb, mesh: cmb.Mesh, image: Image.Image, view: View,
           coordinates: list[tuple[float, ...]] | None = None) -> Image.Image:
    """`mesh` of `model` textured with `image`, its own texture coordinates unless `coordinates`
    (by vertex) stand in for them."""
    shape = model.shapes[mesh.shape]
    positions = model.vertices(shape, "position")
    uv = coordinates or model.vertices(shape, "tex0")
    indices = model.indices(shape)
    width, height = image.size
    source = image.convert("RGBA")
    tilings: dict[int, Image.Image] = {}

    def tiled(count: int) -> Image.Image:
        """The image repeated `count` times each way, framed by one more copy on every side."""
        if count not in tilings:
            sheet = Image.new("RGBA", (width * (count + 2), height * (count + 2)))
            for ty in range(count + 2):
                for tx in range(count + 2):
                    sheet.paste(source, (tx * width, ty * height))
            tilings[count] = sheet
        return tilings[count]

    out = Image.new("RGBA", view.size, (255, 0, 255, 255))
    for first in range(0, len(indices) - 2, 3):
        corners = indices[first:first + 3]
        points = [view.pixel(positions[i][0], positions[i][2]) for i in corners]
        if abs((points[1][0] - points[0][0]) * (points[2][1] - points[0][1])
               - (points[2][0] - points[0][0]) * (points[1][1] - points[0][1])) < 1e-6:
            continue
        xs = [p[0] for p in points]
        ys = [p[1] for p in points]
        if max(xs) < 0 or max(ys) < 0 or min(xs) > view.size[0] or min(ys) > view.size[1]:
            continue
        # The texture repeats: a whole number off a triangle's coordinates draws the same.
        shift_u = math.floor(min(uv[i][0] for i in corners))
        shift_v = math.floor(min(uv[i][1] for i in corners))
        count = math.ceil(max(max(uv[i][axis] for i in corners) - shift for axis, shift in ((0, shift_u), (1, shift_v))))
        count = max(count, 1)
        # Texture rows run top-down in the image; v runs bottom-up.
        us = [(uv[i][0] - shift_u + 1) * width for i in corners]
        vs = [(count + 1 - (uv[i][1] - shift_v)) * height for i in corners]
        a, b, c = _solve(points, us)
        d, e, f = _solve(points, vs)
        left, top = max(0, int(min(xs))), max(0, int(min(ys)))
        right, bottom = min(view.size[0], int(max(xs)) + 2), min(view.size[1], int(max(ys)) + 2)
        box = (right - left, bottom - top)
        drawn = tiled(count).transform(box, Image.Transform.AFFINE,
                                (a, b, a * left + b * top + c, d, e, d * left + e * top + f),
                                Image.Resampling.BILINEAR)
        mask = Image.new("L", box)
        ImageDraw.Draw(mask).polygon([(x - left, y - top) for x, y in points], fill=255)
        out.paste(drawn, (left, top), mask)
    return out
