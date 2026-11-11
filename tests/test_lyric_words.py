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


class Phase(C.Structure):
    _fields_ = [("upcoming", C.c_bool), ("painted", C.c_bool), ("emitting", C.c_bool),
                ("glow", C.c_float), ("remaining", C.c_double), ("safety", C.c_double)]


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
                   str(ROOT / "performance/SGLyricWords.c"), "-o", str(path)]
        if sys.platform != "win32":
            command.insert(-2, "-lm")
        subprocess.run(command, check=True)
        cls.lib = C.CDLL(str(path))
        cls.lib.SGLyricThemeForWord.argtypes = [C.c_char_p, C.c_size_t]
        cls.lib.SGLyricThemeForWord.restype = C.c_int
        cls.lib.SGLyricPhaseAt.argtypes = [C.c_double, C.c_int64, C.c_int64]
        cls.lib.SGLyricPhaseAt.restype = Phase

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

    def test_look_alikes_of_keywords_stay_plain(self):
        for word in ("staring", "stared", "stares", "number", "numbers", "sober", "mister", "liter",
                     "icing", "huge", "crowd", "gravel", "surgery", "bowl", "tray", "camp", "little",
                     "sweet", "current", "honest", "remain", "coin", "swing", "sting", "boring"):
            self.assertEqual(self.theme(word), NONE, word)
        self.assertEqual(self.theme("devil"), FIRE, "an exact keyword wins over a shorter one inside it")
        self.assertEqual(self.theme("star"), SHINE)

    def test_hip_hop_vocabulary(self):
        cases = {"purple": PURPLE, "swag": DRIP, "sizzurp": PURPLE, "codeine": PURPLE, "hazy": PURPLE,
                 "trill": PURPLE, "trippy": PURPLE, "wavy": PURPLE, "kush": PURPLE,
                 "fashion": DRIP, "killa": DRIP, "designer": DRIP, "dior": DRIP, "prada": DRIP,
                 "raf": DRIP, "margiela": DRIP, "pretty": DRIP, "flacko": DRIP, "rocky": DRIP,
                 "jiggy": DRIP, "sundress": DRIP, "babushka": DRIP, "diamonds": DRIP, "chains": DRIP,
                 "vvs": DRIP, "freshest": DRIP, "styling": DRIP, "models": DRIP, "hermes": DRIP,
                 "praise": SHINE, "lord": SHINE, "pray": SHINE, "prayer": SHINE, "blessed": SHINE,
                 "blessings": SHINE, "goldie": SHINE, "god": SHINE,
                 "peso": MONEY, "pesos": MONEY, "bands": MONEY, "racks": MONEY, "guap": MONEY,
                 "hustler": MONEY, "hustlin'": MONEY,
                 "riot": BLOOD, "glock": BLOOD, "choppa": BLOOD, "shooters": BLOOD, "opps": BLOOD,
                 "molly": TOXIC, "xans": TOXIC, "blunts": TOXIC, "henny": TOXIC, "faded": TOXIC,
                 "turnt": PARTY, "moshing": PARTY, "vibes": PARTY, "wildin'": PARTY, "hyped": PARTY,
                 "sandman": NIGHT, "casket": NIGHT, "skyline": SKY, "freezer": ICE}
        for word, theme in cases.items():
            self.assertEqual(self.theme(word), theme, word)
        # An exact keyword still wins over a new, shorter base.
        self.assertEqual(self.theme("hyper"), ELECTRIC)
        # Plural-only slang does not theme its ordinary singular.
        for word in ("band", "rack", "stack", "comma", "stone", "fade", "miles", "lean", "rap",
                     "raped", "design", "swagless", "saucer", "saucers", "bluntly", "prey"):
            self.assertEqual(self.theme(word), NONE, word)

    def test_dollar_sign_spellings(self):
        self.assertEqual(self.theme("A$AP"), DRIP)
        self.assertEqual(self.theme("L$D"), PURPLE)
        self.assertEqual(self.theme("ca$h"), MONEY)
        self.assertEqual(self.theme("FA$HION"), DRIP)
        self.assertEqual(self.theme("$tar"), NONE, "a leading sign is a separator")
        self.assertEqual(self.theme("love$"), LOVE, "so is a trailing one")
        self.assertEqual(self.theme("fire$5"), FIRE)
        for text in ("$", "$$$", "$100", "a$", "ki$$"):
            self.assertEqual(self.theme(text), NONE, text)

    def test_doubled_final_consonants(self):
        cases = {"drippin'": WATER, "dripping": WATER, "dripped": WATER, "swagger": DRIP,
                 "swaggin": DRIP, "clubbing": PARTY, "drummer": PARTY, "clapped": PARTY,
                 "gunned": BLOOD, "warring": BLOOD, "jetting": SKY, "sobbing": WATER,
                 "wetter": WATER, "littest": FIRE, "dimmer": NIGHT, "fuelled": FIRE}
        for word, theme in cases.items():
            self.assertEqual(self.theme(word), theme, word)
        for word in ("litter", "littered", "winning", "shinning", "planning", "summer", "dinner",
                     "better", "letter", "running", "getting", "seeing", "freeing", "herring",
                     "scarred", "slimmer", "butter", "inner", "telling", "calling"):
            self.assertEqual(self.theme(word), NONE, word)

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
