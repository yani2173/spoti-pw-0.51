#ifndef SG_LYRIC_GLYPHS_H
#define SG_LYRIC_GLYPHS_H

#include <stdbool.h>
#include <stddef.h>
#include "SGLyricWords.h"

// Hand-drawn particle artwork for sung-word effects, as portable vector data: one small drawing
// per theme, so a fire word sheds flames and a money word sheds banknotes. No UIKit, no allocation.
typedef enum {
    SGLyricGlyphHaze = 0, // a soft round puff; drawn as a radial fade, it has no outline
    SGLyricGlyphFlame,
    SGLyricGlyphSnowflake,
    SGLyricGlyphDrop,
    SGLyricGlyphBlood,
    SGLyricGlyphHeart,
    SGLyricGlyphSparkle,
    SGLyricGlyphBolt,
    SGLyricGlyphMoon,
    SGLyricGlyphCloud,
    SGLyricGlyphBill,
    SGLyricGlyphFlower,
    SGLyricGlyphNote,
    SGLyricGlyphBubble,
    SGLyricGlyphGem,
    SGLyricGlyphCount
} SGLyricGlyph;

// A drawing is a list of colored layers. A layer op sets the paint; the figures after it use it.
typedef enum {
    SGGlyphMove,     // v: x, y
    SGGlyphLine,     // v: x, y
    SGGlyphCurve,    // v: control 1, control 2, end point
    SGGlyphClose,
    SGGlyphOval,     // v: x, y, width, height; a closed figure by itself
    SGGlyphLayer,    // v: RGB at the top of the box, RGB at the bottom. Every closed figure is
                     // filled by itself, so overlapping figures add up whatever their winding.
    SGGlyphLayerCut, // the same paint, but the figures form one even-odd path: inner ones cut holes
    SGGlyphAlpha,    // v: opacity of the layers that follow (1 at the start)
    SGGlyphGlow      // v: RGB, opacity. A soft round halo behind the drawing, fading into the margin.
} SGGlyphOpKind;

typedef struct {
    unsigned char kind;
    float v[6];
} SGGlyphOp;

typedef struct {
    const SGGlyphOp *ops;
    size_t count;
} SGGlyphShape;

// Figures use a square of SGLyricGlyphBox units with the origin at the top left. The image
// adds SGLyricGlyphMargin on every side for the halo.
#define SGLyricGlyphBox 24
#define SGLyricGlyphMargin 4

SGLyricGlyph SGLyricGlyphForTheme(SGLyricTheme theme);
// NULL for the haze puff and for values outside the enum.
const SGGlyphShape *SGLyricGlyphOutline(SGLyricGlyph glyph);

#endif
