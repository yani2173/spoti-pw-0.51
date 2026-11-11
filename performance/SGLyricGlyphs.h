#ifndef SG_LYRIC_GLYPHS_H
#define SG_LYRIC_GLYPHS_H

#include <stdbool.h>
#include "SGLyricWords.h"

// The mod's own particle drawings, one per theme, as outlines in a 40 x 40 box with y pointing
// down. Portable: no UIKit, no allocation. The outline is filled with the even-odd rule, so a
// shape inside another one is a cut-out (a highlight, a facet gap).
#define SG_LYRIC_GLYPH_BOX 40.0

// An outline in a small subset of the SVG path syntax: absolute M, L, C and Z only.
// NULL for SGLyricThemeNone and anything outside the enum.
const char *SGLyricGlyphOutline(SGLyricTheme theme);

typedef struct {
    void (*move)(void *context, double x, double y);
    void (*line)(void *context, double x, double y);
    void (*curve)(void *context, double x1, double y1, double x2, double y2, double x, double y);
    void (*close)(void *context);
} SGLyricGlyphSink;

// Replays an outline into `sink`. Returns false, and the caller discards what it received, unless
// the whole text is well formed: every subpath starts with M and ends with Z, and every
// coordinate is inside the box.
bool SGLyricGlyphWalk(const char *outline, const SGLyricGlyphSink *sink, void *context);

#endif
