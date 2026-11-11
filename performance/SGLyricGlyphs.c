#include "SGLyricGlyphs.h"

// Every drawing is a filled silhouette in a 24-unit box; it is tinted by its theme when shown.
// Details are cut-outs (the flame's core, the banknote's portrait, the gem's facets) so they
// stay visible in one color at particle size.
#define M(x, y) {SGGlyphMove, {x, y, 0, 0, 0, 0}}
#define L(x, y) {SGGlyphLine, {x, y, 0, 0, 0, 0}}
#define C(a, b, c, d, x, y) {SGGlyphCurve, {a, b, c, d, x, y}}
#define Z {SGGlyphClose, {0, 0, 0, 0, 0, 0}}
#define O(x, y, w, h) {SGGlyphOval, {x, y, w, h, 0, 0}}

// A flame with a hollow core.
static const SGGlyphOp sg_flame[] = {
    M(12, 1.5), C(13, 6, 18.5, 8.5, 18.5, 14.5), C(18.5, 19, 15.5, 22.5, 12, 22.5),
    C(8.5, 22.5, 5.5, 19, 5.5, 14.5), C(5.5, 12, 6.5, 10, 8, 8.5),
    C(8.2, 10.5, 9, 11.8, 10.2, 12.2), C(9.6, 8, 10.5, 4, 12, 1.5), Z, M(12, 13.2),
    C(13.2, 15.2, 15, 16.3, 15, 18.4), C(15, 20.2, 13.6, 21.2, 12, 21.2),
    C(10.4, 21.2, 9, 20.2, 9, 18.4), C(9, 16.3, 10.8, 15.2, 12, 13.2), Z,
};

// Three crossed bars with two barbs on each of the six arms.
static const SGGlyphOp sg_snowflake[] = {
    M(1.2, 11.15), L(22.8, 11.15), L(22.8, 12.85), L(1.2, 12.85), Z, M(7.34, 2.22),
    L(18.14, 20.93), L(16.66, 21.78), L(5.86, 3.07), Z, M(18.14, 3.07), L(7.34, 21.78),
    L(5.86, 20.93), L(16.66, 2.22), Z, M(17.95, 11.57), L(20.66, 15.04), L(19.56, 15.9),
    L(16.85, 12.43), Z, M(16.85, 11.57), L(19.56, 8.1), L(20.66, 8.96), L(17.95, 12.43), Z,
    M(15.35, 16.94), L(13.7, 21.02), L(12.4, 20.49), L(14.05, 16.41), Z, M(14.8, 15.98),
    L(19.15, 16.6), L(18.96, 17.98), L(14.6, 17.37), Z, M(9.4, 17.37), L(5.04, 17.98),
    L(4.85, 16.6), L(9.2, 15.98), Z, M(9.95, 16.41), L(11.6, 20.49), L(10.3, 21.02),
    L(8.65, 16.94), Z, M(6.05, 12.43), L(3.34, 8.96), L(4.44, 8.1), L(7.15, 11.57), Z,
    M(7.15, 12.43), L(4.44, 15.9), L(3.34, 15.04), L(6.05, 11.57), Z, M(8.65, 7.06), L(10.3, 2.98),
    L(11.6, 3.51), L(9.95, 7.59), Z, M(9.2, 8.02), L(4.85, 7.4), L(5.04, 6.02), L(9.4, 6.63), Z,
    M(14.6, 6.63), L(18.96, 6.02), L(19.15, 7.4), L(14.8, 8.02), Z, M(14.05, 7.59), L(12.4, 3.51),
    L(13.7, 2.98), L(15.35, 7.06), Z,
};

// A falling drop with a highlight. Water and blood share it.
static const SGGlyphOp sg_drop[] = {
    M(12, 2), C(12, 2, 5.5, 10.5, 5.5, 15.5), C(5.5, 19.4, 8.4, 22, 12, 22),
    C(15.6, 22, 18.5, 19.4, 18.5, 15.5), C(18.5, 10.5, 12, 2, 12, 2), Z, O(8.1, 13.6, 2.4, 4.4),
};

static const SGGlyphOp sg_heart[] = {
    M(12, 21.5), C(4.5, 16.5, 1.5, 12.5, 1.5, 8), C(1.5, 2.5, 9.5, 1, 12, 6),
    C(14.5, 1, 22.5, 2.5, 22.5, 8), C(22.5, 12.5, 19.5, 16.5, 12, 21.5), Z,
};

// A four-pointed star with concave sides.
static const SGGlyphOp sg_sparkle[] = {
    M(12, 0.5), C(12.6, 7.5, 16.5, 11.4, 23.5, 12), C(16.5, 12.6, 12.6, 16.5, 12, 23.5),
    C(11.4, 16.5, 7.5, 12.6, 0.5, 12), C(7.5, 11.4, 11.4, 7.5, 12, 0.5), Z,
};

static const SGGlyphOp sg_bolt[] = {
    M(14.5, 1), L(4.5, 13.6), L(10.8, 13.6), L(8.2, 23), L(19.5, 9.6), L(13, 9.6), Z,
};

// A crescent: the long arc of one circle closed by the arc of a second, offset one.
static const SGGlyphOp sg_moon[] = {
    M(9.63, 2.59), C(4.69, 3.28, 1.01, 7.5, 1, 12.48), C(0.99, 17.47, 4.66, 21.7, 9.59, 22.4),
    C(14.53, 23.1, 19.23, 20.06, 20.61, 15.27), C(17.08, 17.97, 12.06, 17.46, 9.15, 14.1),
    C(6.24, 10.74, 6.45, 5.7, 9.63, 2.59), Z,
};

static const SGGlyphOp sg_cloud[] = {
    O(1.5, 11, 9, 9), O(6, 4.5, 11, 11), O(13.5, 9.5, 9, 9.5), M(6, 13.5), L(18, 13.5), L(18, 20),
    L(6, 20), Z,
};

// A banknote: rounded rectangle, portrait oval and two seals.
static const SGGlyphOp sg_bill[] = {
    M(3.2, 6), L(20.8, 6), C(21.9, 6, 23, 7.1, 23, 8.2), L(23, 15.8),
    C(23, 16.9, 21.9, 18, 20.8, 18), L(3.2, 18), C(2.1, 18, 1, 16.9, 1, 15.8), L(1, 8.2),
    C(1, 7.1, 2.1, 6, 3.2, 6), Z, O(8.8, 8.3, 6.4, 7.4), O(3.6, 10.8, 2.4, 2.4),
    O(18, 10.8, 2.4, 2.4),
};

// Five round petals around a separate center.
static const SGGlyphOp sg_flower[] = {
    O(9.5, 9.5, 5, 5), O(8, 1, 8, 8), O(14.66, 5.84, 8, 8), O(12.11, 13.66, 8, 8),
    O(3.89, 13.66, 8, 8), O(1.34, 5.84, 8, 8),
};

// An eighth note: head, stem and flag.
static const SGGlyphOp sg_note[] = {
    O(3.5, 15.6, 8.6, 6.6), M(10.4, 3), L(12.1, 3), L(12.1, 19), L(10.4, 19), Z, M(11.2, 3),
    L(12.1, 3), C(12.6, 6.6, 18.6, 7.2, 18.6, 12.6), C(18.6, 13.9, 18.3, 14.9, 17.7, 15.9),
    C(17.7, 11.7, 14.6, 10, 12.1, 9.4), L(11.2, 9.4), Z,
};

// A ring with a highlight inside.
static const SGGlyphOp sg_bubble[] = {
    O(2, 2, 20, 20), O(4, 4, 16, 16), O(7, 7.2, 4.6, 3.2),
};

// A cut gem seen from the side: five crown facets over three pavilion facets.
static const SGGlyphOp sg_gem[] = {
    M(6.24, 3.97), L(1.9, 9.18), L(7.1, 9.18), Z, M(7, 3.42), L(11.22, 3.42), L(7.84, 8.48), Z,
    M(12, 3.76), L(8.38, 9.18), L(15.62, 9.18), Z, M(12.78, 3.42), L(17, 3.42), L(16.16, 8.48), Z,
    M(17.76, 3.97), L(16.9, 9.18), L(22.1, 9.18), Z, M(1.91, 10.02), L(7.3, 10.02),
    L(10.89, 20.64), Z, M(8.19, 10.02), L(15.81, 10.02), L(12, 21.29), Z, M(16.7, 10.02),
    L(22.09, 10.02), L(13.11, 20.64), Z,
};

#define SHAPE(table, evenOdd) {table, sizeof(table) / sizeof(table[0]), evenOdd}
static const SGGlyphShape sg_shapes[SGLyricGlyphCount] = {
    [SGLyricGlyphFlame] = SHAPE(sg_flame, true),
    [SGLyricGlyphSnowflake] = SHAPE(sg_snowflake, false),
    [SGLyricGlyphDrop] = SHAPE(sg_drop, true),
    [SGLyricGlyphHeart] = SHAPE(sg_heart, false),
    [SGLyricGlyphSparkle] = SHAPE(sg_sparkle, false),
    [SGLyricGlyphBolt] = SHAPE(sg_bolt, false),
    [SGLyricGlyphMoon] = SHAPE(sg_moon, false),
    [SGLyricGlyphCloud] = SHAPE(sg_cloud, false),
    [SGLyricGlyphBill] = SHAPE(sg_bill, true),
    [SGLyricGlyphFlower] = SHAPE(sg_flower, false),
    [SGLyricGlyphNote] = SHAPE(sg_note, false),
    [SGLyricGlyphBubble] = SHAPE(sg_bubble, true),
    [SGLyricGlyphGem] = SHAPE(sg_gem, false),
};

SGLyricGlyph SGLyricGlyphForTheme(SGLyricTheme theme) {
    switch (theme) {
        case SGLyricThemeFire: return SGLyricGlyphFlame;
        case SGLyricThemeIce: return SGLyricGlyphSnowflake;
        case SGLyricThemeWater: return SGLyricGlyphDrop;
        case SGLyricThemeLove: return SGLyricGlyphHeart;
        case SGLyricThemeShine: return SGLyricGlyphSparkle;
        case SGLyricThemeElectric: return SGLyricGlyphBolt;
        case SGLyricThemeNight: return SGLyricGlyphMoon;
        case SGLyricThemeSky: return SGLyricGlyphCloud;
        case SGLyricThemeMoney: return SGLyricGlyphBill;
        case SGLyricThemeBloom: return SGLyricGlyphFlower;
        case SGLyricThemeBlood: return SGLyricGlyphDrop;
        case SGLyricThemeParty: return SGLyricGlyphNote;
        case SGLyricThemeToxic: return SGLyricGlyphBubble;
        case SGLyricThemePurple: return SGLyricGlyphHaze;
        case SGLyricThemeDrip: return SGLyricGlyphGem;
        default: return SGLyricGlyphHaze;
    }
}

const SGGlyphShape *SGLyricGlyphOutline(SGLyricGlyph glyph) {
    if ((int)glyph <= SGLyricGlyphHaze || (int)glyph >= SGLyricGlyphCount) return NULL;
    return &sg_shapes[glyph];
}
