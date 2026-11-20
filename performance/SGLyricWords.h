#ifndef SG_LYRIC_WORDS_H
#define SG_LYRIC_WORDS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Portable word classification and timing policy for sung-word effects. No UIKit, no allocation.
typedef enum {
    SGLyricThemeNone = 0,
    SGLyricThemeFire,
    SGLyricThemeIce,
    SGLyricThemeWater,
    SGLyricThemeLove,
    SGLyricThemeShine,
    SGLyricThemeElectric,
    SGLyricThemeNight,
    SGLyricThemeSky,
    SGLyricThemeMoney,
    SGLyricThemeBloom,
    SGLyricThemeBlood,
    SGLyricThemeParty,
    SGLyricThemeToxic,
    SGLyricThemePurple, // lean, haze and the slowed-down Houston sound
    SGLyricThemeDrip,   // fashion houses, jewelry and style
    SGLyricThemeCount
} SGLyricTheme;

// One lyric token as UTF-8. Case, surrounding punctuation, a possessive and common English
// inflections are ignored. A "$" written for an "s" is read as one: "A$AP", "L$D", "PE$O". Matching is on whole words only: "star" never matches "start".
SGLyricTheme SGLyricThemeForWord(const char *utf8, size_t length);

typedef struct {
    bool upcoming;   // before the word starts; a later pass may emit again
    bool painted;    // themed text is shown (from 200 ms before the start, masked by the sweep)
    bool emitting;   // particles and motion run only while the word is sung
    float glow;      // 0...1 envelope: 150 ms rise from the start, 450 ms fall after the end
    double remaining; // seconds until the word ends at normal speed; 0 unless emitting
    double safety;    // seconds after which emission must stop without another tick (paused clock)
} SGLyricPhase;

// Times are the core's milliseconds. A word without a usable span is treated as 1.5 s long.
SGLyricPhase SGLyricPhaseAt(double now, int64_t start, int64_t end);

#endif
