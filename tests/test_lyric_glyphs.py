"""Particle artwork data behind the sung-word lyric effects.

Compiles the portable C tables used by the add-on and checks that every theme has a
well-formed drawing. It does not rasterize with UIKit: how the drawings look as moving
particles on a phone needs a device.
"""
import ctypes as C
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
COMPILER = os.environ.get("CC", "cc")
MOVE, LINE, CURVE, CLOSE, OVAL, LAYER, LAYER_CUT, ALPHA, GLOW = range(9)
BOX = 24.0
THEMES = ["none", "fire", "ice", "water", "love", "shine", "electric", "night", "sky", "money",
          "bloom", "blood", "party", "toxic", "purple", "drip"]
(HAZE, FLAME, SNOWFLAKE, DROP, BLOOD, HEART, SPARKLE, BOLT, MOON, CLOUD, BILL, FLOWER, NOTE,
 BUBBLE, GEM, GLYPHS) = range(16)


class Op(C.Structure):
    _fields_ = [("kind", C.c_ubyte), ("v", C.c_float * 6)]


class Shape(C.Structure):
    _fields_ = [("ops", C.POINTER(Op)), ("count", C.c_size_t)]


@unittest.skipUnless(shutil.which(COMPILER), "a C compiler is required for the lyric glyph checks")
class LyricGlyphTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.folder = tempfile.TemporaryDirectory(ignore_cleanup_errors=True)
        cls.addClassCleanup(cls.folder.cleanup)
        path = Path(cls.folder.name) / ("glyphs.dylib" if sys.platform == "darwin" else "glyphs.so")
        subprocess.run([COMPILER, "-std=c11", "-O2", "-Wall", "-Wextra", "-Werror",
                        "-dynamiclib" if sys.platform == "darwin" else "-shared", "-fPIC",
                        str(ROOT / "performance/SGLyricGlyphs.c"), "-o", str(path)], check=True)
        cls.lib = C.CDLL(str(path))
        cls.lib.SGLyricGlyphForTheme.argtypes = [C.c_int]
        cls.lib.SGLyricGlyphForTheme.restype = C.c_int
        cls.lib.SGLyricGlyphOutline.argtypes = [C.c_int]
        cls.lib.SGLyricGlyphOutline.restype = C.POINTER(Shape)

    def ops(self, glyph):
        shape = self.lib.SGLyricGlyphOutline(glyph)
        self.assertTrue(shape, glyph)
        return [(shape.contents.ops[i].kind, list(shape.contents.ops[i].v))
                for i in range(shape.contents.count)]

    def test_each_theme_has_its_own_artwork(self):
        expected = {"fire": FLAME, "ice": SNOWFLAKE, "water": DROP, "love": HEART, "shine": SPARKLE,
                    "electric": BOLT, "night": MOON, "sky": CLOUD, "money": BILL, "bloom": FLOWER,
                    "blood": BLOOD, "party": NOTE, "toxic": BUBBLE, "purple": HAZE, "drip": GEM}
        self.assertEqual(set(expected), set(THEMES[1:]))
        for name, glyph in expected.items():
            self.assertEqual(self.lib.SGLyricGlyphForTheme(THEMES.index(name)), glyph, name)
        self.assertEqual(set(expected.values()), set(range(GLYPHS)), "no drawing is unused")
        for theme in (0, len(THEMES), -1, 1000):
            self.assertEqual(self.lib.SGLyricGlyphForTheme(theme), HAZE, theme)

    def test_only_the_haze_has_no_outline(self):
        for glyph in (HAZE, GLYPHS, -1, 1000):
            self.assertFalse(self.lib.SGLyricGlyphOutline(glyph), glyph)
        for glyph in range(FLAME, GLYPHS):
            self.assertTrue(self.ops(glyph), glyph)

    def test_drawings_are_painted_closed_figures_inside_the_box(self):
        used = {MOVE: 2, LINE: 2, CURVE: 6, CLOSE: 0, OVAL: 4, LAYER: 6, LAYER_CUT: 6, ALPHA: 1, GLOW: 4}
        for glyph in range(FLAME, GLYPHS):
            is_open = painted = False
            figures = 0
            low, high = [BOX, BOX], [0.0, 0.0]
            for index, (kind, values) in enumerate(self.ops(glyph)):
                self.assertIn(kind, used, glyph)
                self.assertEqual(values[used[kind]:], [0.0] * (6 - used[kind]), glyph)
                if kind in (LAYER, LAYER_CUT, ALPHA, GLOW):
                    self.assertFalse(is_open, f"{glyph}: the paint changes inside a figure")
                    self.assertTrue(all(0 <= value <= 1 for value in values), f"{glyph}: {values}")
                    painted = painted or kind in (LAYER, LAYER_CUT)
                    if kind == GLOW:
                        self.assertEqual(index, 0, f"{glyph}: the halo lies behind the drawing")
                    if kind == ALPHA:
                        self.assertGreater(values[0], 0, glyph)
                    continue
                self.assertTrue(painted, f"{glyph}: a figure before any layer has no color")
                if kind == OVAL:
                    self.assertFalse(is_open, glyph)
                    self.assertTrue(values[2] > 0 and values[3] > 0, glyph)
                    points = [values[:2], [values[0] + values[2], values[1] + values[3]]]
                    figures += 1
                elif kind == MOVE:
                    self.assertFalse(is_open, f"{glyph}: a figure was left open")
                    is_open = True
                    points = [values[:2]]
                elif kind == CLOSE:
                    self.assertTrue(is_open, glyph)
                    is_open = False
                    figures += 1
                    points = []
                else:
                    self.assertTrue(is_open, f"{glyph}: a segment without a start point")
                    points = [values[i:i + 2] for i in range(0, used[kind], 2)]
                for x, y in points:
                    self.assertTrue(0 <= x <= BOX and 0 <= y <= BOX, f"{glyph}: {(x, y)} leaves the box")
                    low = [min(low[0], x), min(low[1], y)]
                    high = [max(high[0], x), max(high[1], y)]
            self.assertFalse(is_open, f"{glyph}: the last figure is not closed")
            self.assertGreater(figures, 0, glyph)
            # A drawing uses the box, so every theme's particles have a comparable size.
            self.assertGreaterEqual(max(high[0] - low[0], high[1] - low[1]), 0.75 * BOX, glyph)

    def test_every_drawing_has_a_halo_and_shading(self):
        for glyph in range(FLAME, GLYPHS):
            ops = self.ops(glyph)
            self.assertEqual(ops[0][0], GLOW, glyph)
            layers = [values for kind, values in ops if kind in (LAYER, LAYER_CUT)]
            self.assertGreaterEqual(len(layers), 1, glyph)
            self.assertTrue(len(layers) > 1 or layers[0][:3] != layers[0][3:], f"{glyph} is one flat color")


if __name__ == "__main__":
    unittest.main()
