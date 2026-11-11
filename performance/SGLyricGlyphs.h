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

typedef enum {
    SGGlyphMove,  // v: x, y
    SGGlyphLine,  // v: x, y
    SGGlyphCurve, // v: control 1, control 2, end point
    SGGlyphClose,
    SGGlyphOval   // v: x, y, width, height; a closed figure by itself
} SGGlyphOpKind;

typedef struct {
    unsigned char kind;
    float v[6];
} SGGlyphOp;

// Coordinates are in a square of SGLyricGlyphBox units with the origin at the top left.
// evenOdd: all figures form one path and inner figures cut holes. Otherwise every closed
// figure is filled by itself and they add up, whatever their winding direction.
typedef struct {
    const SGGlyphOp *ops;
    size_t count;
    bool evenOdd;
} SGGlyphShape;

#define SGLyricGlyphBox 24

SGLyricGlyph SGLyricGlyphForTheme(SGLyricTheme theme);
// NULL for the haze puff and for values outside the enum.
const SGGlyphShape *SGLyricGlyphOutline(SGLyricGlyph glyph);

#endif
