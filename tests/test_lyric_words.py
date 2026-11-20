"""Word classification and timing policy behind the sung-word lyric effects.

Compiles the portable C used by the add-on. It does not render anything: colors,
particles and motion are UIKit/Core Animation work that needs a device.
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
(NONE, FIRE, ICE, WATER, LOVE, SHINE, ELECTRIC, NIGHT, SKY, MONEY, BLOOM, BLOOD, PARTY, TOXIC,
 PURPLE, DRIP) = range(16)
COUNT = 16


class Phase(C.Structure):
    _fields_ = [("upcoming", C.c_bool), ("painted", C.c_bool), ("emitting", C.c_bool),
                ("glow", C.c_float), ("remaining", C.c_double), ("safety", C.c_double)]


POINT = C.CFUNCTYPE(None, C.c_void_p, C.c_double, C.c_double)
CURVE = C.CFUNCTYPE(None, C.c_void_p, *[C.c_double] * 6)
CLOSE = C.CFUNCTYPE(None, C.c_void_p)


class Sink(C.Structure):
    _fields_ = [("move", POINT), ("line", POINT), ("curve", CURVE), ("close", CLOSE)]


@unittest.skipUnless(shutil.which(COMPILER), "a C compiler is required for the lyric word checks")
class LyricWordTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # A loaded library stays locked on Windows; leave that copy to the system's cleanup.
        cls.folder = tempfile.TemporaryDirectory(ignore_cleanup_errors=True)
        cls.addClassCleanup(cls.folder.cleanup)
        path = Path(cls.folder.name) / ("words.dylib" if sys.platform == "darwin" else "words.so")
        command = [COMPILER, "-std=c11", "-O2", "-Wall", "-Wextra", "-Werror",
                   "-dynamiclib" if sys.platform == "darwin" else "-shared", "-fPIC",
                   str(ROOT / "performance/SGLyricWords.c"), str(ROOT / "performance/SGLyricGlyphs.c"),
                   "-o", str(path)]
        if sys.platform != "win32":
            command.insert(-2, "-lm")
        subprocess.run(command, check=True)
        cls.lib = C.CDLL(str(path))
        cls.lib.SGLyricThemeForWord.argtypes = [C.c_char_p, C.c_size_t]
        cls.lib.SGLyricThemeForWord.restype = C.c_int
        cls.lib.SGLyricPhaseAt.argtypes = [C.c_double, C.c_int64, C.c_int64]
        cls.lib.SGLyricPhaseAt.restype = Phase
        cls.lib.SGLyricGlyphOutline.argtypes = [C.c_int]
        cls.lib.SGLyricGlyphOutline.restype = C.c_char_p
        cls.lib.SGLyricGlyphWalk.argtypes = [C.c_char_p, C.POINTER(Sink), C.c_void_p]
        cls.lib.SGLyricGlyphWalk.restype = C.c_bool

    def theme(self, text):
        data = text.encode("utf-8")
        return self.lib.SGLyricThemeForWord(data, len(data))

    def test_each_theme_has_keywords(self):
        expected = {"fire": FIRE, "ice": ICE, "rain": WATER, "love": LOVE, "star": SHINE,
                    "thunder": ELECTRIC, "midnight": NIGHT, "sky": SKY, "money": MONEY, "flower": BLOOM,
                    "blood": BLOOD, "dance": PARTY, "poison": TOXIC, "purple": PURPLE, "fashion": DRIP}
        for word, theme in expected.items():
            self.assertEqual(self.theme(word), theme, word)

    def test_case_punctuation_and_possessives_are_ignored(self):
        for text in ("burns", "Burns", "BURNS,", " burns ", "(burns)", "burns...", "burns…",
                     "“burns”", "burns!?", "‘burns’"):
            self.assertEqual(self.theme(text), FIRE, repr(text))
        self.assertEqual(self.theme("heart's"), LOVE)
        self.assertEqual(self.theme("heart’s"), LOVE)
        self.assertEqual(self.theme("burnin'"), FIRE)
        self.assertEqual(self.theme("lovin’"), LOVE)
        self.assertEqual(self.theme("'cause"), NONE)
        for text in ("die\u00a0", "\u00a0die", "die\u200b", "\ufeffdie", "\u00abdie\u00bb", "die\u2009",
                     "\u00a1die!", "die\u2060"):
            self.assertEqual(self.theme(text), BLOOD, repr(text))

    def test_common_inflections(self):
        cases = {"burned": FIRE, "burning": FIRE, "flames": FIRE, "blazing": FIRE, "ashes": FIRE,
                 "hotter": FIRE, "fired": FIRE, "colder": ICE, "coldest": ICE, "freezing": ICE,
                 "iced": ICE, "raining": WATER, "oceans": WATER, "crying": WATER, "cried": WATER,
                 "drowning": WATER, "loved": LOVE, "loving": LOVE, "lovely": LOVE, "kisses": LOVE,
                 "hearts": LOVE, "stars": SHINE, "shining": SHINE, "brightest": SHINE,
                 "sparkling": SHINE, "glowing": SHINE, "shocked": ELECTRIC, "sparks": ELECTRIC,
                 "nights": NIGHT, "darker": NIGHT, "shadows": NIGHT, "haunted": NIGHT}
        for word, theme in cases.items():
            self.assertEqual(self.theme(word), theme, word)

    def test_extended_vocabulary(self):
        cases = {"volcano": FIRE, "lit": FIRE, "exploded": FIRE, "rockets": FIRE, "dragon": FIRE,
                 "cool": ICE, "cooler": ICE, "numb": ICE, "avalanche": ICE, "december": ICE,
                 "blue": WATER, "deeper": WATER, "swimming": WATER, "diving": WATER, "beaches": WATER,
                 "misty": WATER, "sinking": WATER, "hugging": LOVE, "passion": LOVE, "desired": LOVE,
                 "honey": LOVE, "heartbroken": LOVE, "married": LOVE, "sunset": SHINE, "dawn": SHINE,
                 "candles": SHINE, "glorious": SHINE, "crown": SHINE, "universe": SHINE, "rays": SHINE,
                 "charged": ELECTRIC, "wired": ELECTRIC, "pulsing": ELECTRIC, "bolts": ELECTRIC,
                 "glitches": ELECTRIC, "magnetic": ELECTRIC, "dreaming": NIGHT, "dreams": NIGHT,
                 "asleep": NIGHT, "wolves": NIGHT, "cursed": NIGHT, "witches": NIGHT, "black": NIGHT,
                 "evil": NIGHT, "howling": NIGHT, "zombies": NIGHT}
        for word, theme in cases.items():
            self.assertEqual(self.theme(word), theme, word)

    def test_added_themes(self):
        cases = {"roses": BLOOM, "flying": SKY, "skies": SKY, "higher": SKY, "wings": SKY, "clouds": SKY, "winds": SKY,
                 "cash": MONEY, "richest": MONEY, "dollars": MONEY, "millions": MONEY, "paid": MONEY,
                 "rose": BLOOM, "flowers": BLOOM, "blooming": BLOOM, "leaves": BLOOM, "growing": BLOOM,
                 "greener": BLOOM, "bleeding": BLOOD, "killed": BLOOD, "hated": BLOOD, "raging": BLOOD,
                 "dying": BLOOD, "died": BLOOD, "wars": BLOOD, "dancing": PARTY, "singing": PARTY,
                 "singer": PARTY, "parties": PARTY, "louder": PARTY, "beats": PARTY, "rocking": PARTY,
                 "pills": TOXIC, "drugs": TOXIC, "sicker": TOXIC, "poisoned": TOXIC, "snakes": TOXIC}
        for word, theme in cases.items():
            self.assertEqual(self.theme(word), theme, word)
        # Exact keywords of another theme are not swallowed by a shorter base.
        self.assertEqual(self.theme("rocket"), FIRE)
        self.assertEqual(self.theme("raven"), NIGHT)
        self.assertEqual(self.theme("planet"), SHINE)
        for word in ("warning", "winding", "scared", "scary", "skill", "spill", "pillow", "paint",
                     "painting", "single", "hair", "fair", "warm", "diet", "win", "swing", "roses'x"):
            self.assertEqual(self.theme(word), NONE, word)

    def test_hip_hop_vocabulary(self):
        cases = {"purple": PURPLE, "lean": PURPLE, "leaning": PURPLE, "codeine": PURPLE, "sippin'": PURPLE,
                 "sipping": PURPLE, "trill": PURPLE, "wavy": PURPLE, "trippin’": PURPLE, "hazy": PURPLE,
                 "Dior": DRIP, "Prada": DRIP, "Raf": DRIP, "Margiela": DRIP, "fashion": DRIP,
                 "dripping": DRIP, "drip": DRIP, "swag": DRIP, "pretty": DRIP, "Flacko": DRIP,
                 "Harlem": DRIP, "Rocky": DRIP, "chains": DRIP, "grills": DRIP, "styled": DRIP,
                 "freshest": DRIP, "models": DRIP, "babushka": DRIP, "sundress": DRIP,
                 "peso": MONEY, "racks": MONEY, "bands": MONEY, "stacking": MONEY, "Maybach": MONEY,
                 "goldie": SHINE, "Lord": SHINE, "praising": SHINE, "blessed": SHINE,
                 "riots": BLOOD, "gang": BLOOD, "mob": BLOOD, "shooters": BLOOD, "hijacked": BLOOD,
                 "wilder": PARTY, "jukebox": PARTY, "molly": TOXIC, "blunts": TOXIC,
                 "sandman": NIGHT, "phoenix": FIRE, "distorted": ELECTRIC}
        for word, theme in cases.items():
            self.assertEqual(self.theme(word), theme, word)
        self.assertEqual(self.theme("rocks"), PARTY, "the stage name does not take over the verb")
        for word in ("miles", "band", "clean", "learn", "least", "strip", "grip", "script", "further",
                     "furry", "racing", "racket", "bluntly", "mobile", "opposite", "freshman",
                     "wilderness", "amend", "beefy", "prettier", "cup", "world", "ripped"):
            self.assertEqual(self.theme(word), NONE, word)

    def test_a_dollar_sign_written_for_an_s(self):
        cases = {"A$AP": DRIP, "a$ap": DRIP, "FA$HION": DRIP, "L$D": PURPLE, "PE$O": MONEY,
                 "$wag": DRIP, "ca$h": MONEY, "rack$": MONEY, "$ky": SKY}
        for word, theme in cases.items():
            self.assertEqual(self.theme(word), theme, word)
        for word in ("$", "$100", "100$", "$$$", "A$", "$ $", "a$ap$$x" * 4):
            self.assertEqual(self.theme(word), NONE, word)
        self.assertEqual(self.theme("$100-cash"), MONEY)

    def walk(self, outline):
        """Returns the outline's subpaths as lists of points, or None if it is rejected."""
        paths = []
        sink = Sink(POINT(lambda _, x, y: paths.append([(x, y)])),
                    POINT(lambda _, x, y: paths[-1].append((x, y))),
                    CURVE(lambda _, a, b, c, d, x, y: paths[-1].extend([(a, b), (c, d), (x, y)])),
                    CLOSE(lambda _: paths[-1].append("closed")))
        return paths if self.lib.SGLyricGlyphWalk(outline, C.byref(sink), None) else None

    def test_every_theme_has_its_own_drawing(self):
        outlines = [self.lib.SGLyricGlyphOutline(theme) for theme in range(1, COUNT)]
        self.assertEqual(len(set(outlines)), COUNT - 1)
        for theme, outline in enumerate(outlines, 1):
            paths = self.walk(outline)
            self.assertTrue(paths, theme)
            points = [point for path in paths for point in path if point != "closed"]
            for path in paths:
                self.assertEqual(path[-1], "closed", theme)
                self.assertGreaterEqual(len(path), 4, theme)
            xs, ys = [x for x, _ in points], [y for _, y in points]
            # Room for the halo on every side, and large enough to read as a particle.
            self.assertTrue(2 <= min(xs) and max(xs) <= 38 and 2 <= min(ys) and max(ys) <= 38, theme)
            self.assertTrue(max(xs) - min(xs) >= 18 and max(ys) - min(ys) >= 18, theme)
        for theme in (NONE, COUNT, -1, 99):
            self.assertIsNone(self.lib.SGLyricGlyphOutline(theme))

    def test_malformed_outlines_are_rejected(self):
        self.assertEqual(self.walk(b"M1 2 L3 4 C5 6 7 8 9 10 Z"),
                         [[(1, 2), (3, 4), (5, 6), (7, 8), (9, 10), "closed"]])
        self.assertEqual(len(self.walk(b" M1 1 L2 2 L3 1 Z  M5 5 L6 6 L7 5 Z ")), 2)
        for outline in (b"", b"   ", b"L1 2 Z", b"M1 2 L3 4", b"M1 2 M3 4 Z", b"M1 2 Z Z", b"M1 Z",
                        b"M1 2 L3 Z", b"M1 2 C1 2 3 4 5 Z", b"M-1 2 L3 4 Z", b"M1 2 L41 4 Z",
                        b"M1 2 Lnan 4 Z", b"M1 2 Linf 4 Z", b"M1 2 Q3 4 5 6 Z", b"m1 2 l3 4 z",
                        b"M1 2 L3 4 Z x", b"M1,2 L3,4 Z", b"Z"):
            self.assertIsNone(self.walk(outline), outline)
        self.assertFalse(self.lib.SGLyricGlyphWalk(None, C.byref(Sink()), None))
        self.assertFalse(self.lib.SGLyricGlyphWalk(b"M1 2 L3 4 Z", None, None))
        self.assertFalse(self.lib.SGLyricGlyphWalk(b"M1 2 L3 4 Z", C.byref(Sink()), None))

    def test_look_alikes_of_keywords_stay_plain(self):
        for word in ("staring", "stared", "stares", "number", "numbers", "sober", "mister", "liter",
                     "icing", "huge", "crowd", "gravel", "surgery", "bowl", "pray", "camp", "little",
                     "sweet", "current", "honest", "remain", "coin", "swing", "sting", "boring"):
            self.assertEqual(self.theme(word), NONE, word)
        self.assertEqual(self.theme("devil"), FIRE, "an exact keyword wins over a shorter one inside it")
        self.assertEqual(self.theme("star"), SHINE)

    def test_only_whole_words_match(self):
        for word in ("start", "started", "nice", "hotel", "sunday", "season", "search", "seen",
                     "icon", "colder's'x", "fireman", "the", "a", "", "burnable", "lightweight",
                     "don't", "i'm", "kissable", "starve", "raindeer", "king", "thing", "red",
                     "skin", "west", "only", "hello", "shell", "glove", "clover", "beheart"):
            self.assertEqual(self.theme(word), NONE, word)

    def test_compound_tokens_use_their_first_themed_part(self):
        self.assertEqual(self.theme("ice-cold"), ICE)
        self.assertEqual(self.theme("red-hot"), FIRE)
        self.assertEqual(self.theme("star/moon"), SHINE)
        self.assertEqual(self.theme("oh—love"), LOVE)

    def test_other_scripts_and_unusual_input_are_left_alone(self):
        for text in ("café", "огън", "火", "fire\U0001F525", "éstar",
                     "x" * 64, "fire" * 12, "\xe2", "☃"):
            self.assertEqual(self.theme(text), NONE, repr(text))
        self.assertEqual(self.lib.SGLyricThemeForWord(None, 0), NONE)
        self.assertEqual(self.lib.SGLyricThemeForWord(b"fire\xe2\x80", 6), NONE)
        self.assertEqual(self.lib.SGLyricThemeForWord(b"fireXXXX", 4), FIRE, "length, not a terminator, bounds the word")

    def test_phase_follows_the_word_span(self):
        phase = self.lib.SGLyricPhaseAt
        early = phase(700, 1000, 2000)
        self.assertTrue(early.upcoming)
        self.assertFalse(early.painted or early.emitting)
        self.assertEqual(early.glow, 0)
        ready = phase(850, 1000, 2000)
        self.assertTrue(ready.upcoming and ready.painted)
        self.assertFalse(ready.emitting)
        sung = phase(1500, 1000, 2000)
        self.assertFalse(sung.upcoming)
        self.assertTrue(sung.painted and sung.emitting)
        self.assertAlmostEqual(sung.glow, 1.0, places=6)
        self.assertAlmostEqual(sung.remaining, 0.5, places=6)
        self.assertAlmostEqual(sung.safety, 1.5, places=6)
        done = phase(2000, 1000, 2000)
        self.assertTrue(done.painted)
        self.assertFalse(done.emitting)
        self.assertEqual((done.remaining, done.safety), (0, 0))
        self.assertAlmostEqual(done.glow, 1.0, places=6)
        self.assertAlmostEqual(phase(2225, 1000, 2000).glow, 0.5, places=6)
        late = phase(9000, 1000, 2000)
        self.assertTrue(late.painted)
        self.assertFalse(late.emitting or late.upcoming)
        self.assertEqual(late.glow, 0)

    def test_glow_rises_and_falls_monotonically(self):
        values = [self.lib.SGLyricPhaseAt(t, 1000, 2000).glow for t in range(900, 2600, 5)]
        peak = values.index(max(values))
        self.assertEqual(values[:peak + 1], sorted(values[:peak + 1]))
        self.assertEqual(values[peak:], sorted(values[peak:], reverse=True))
        self.assertTrue(all(0 <= value <= 1 for value in values))

    def test_safety_limit_is_bounded(self):
        phase = self.lib.SGLyricPhaseAt
        self.assertAlmostEqual(phase(1990, 1000, 2000).safety, 0.8, places=6)
        self.assertAlmostEqual(phase(0, 0, 3_600_000).safety, 8.0, places=6)

    def test_unusable_spans_and_times(self):
        phase = self.lib.SGLyricPhaseAt
        for end in (1000, 0, -5):
            placeholder = phase(1200, 1000, end)
            self.assertTrue(placeholder.emitting, end)
            self.assertFalse(phase(2600, 1000, end).emitting, end)
        for time in (float("nan"), float("inf"), float("-inf")):
            unknown = phase(time, 1000, 2000)
            self.assertTrue(unknown.upcoming)
            self.assertFalse(unknown.painted or unknown.emitting)
            self.assertEqual(unknown.glow, 0)
        extreme = phase(0, -2**63, 2**63 - 1)
        self.assertTrue(extreme.emitting)
        self.assertAlmostEqual(extreme.safety, 8.0, places=6)


if __name__ == "__main__":
    unittest.main()
