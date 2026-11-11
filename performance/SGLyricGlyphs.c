#include "SGLyricGlyphs.h"

// Every drawing is built from a few gradient layers in a 24-unit box: a base shape, lighter
// inner shapes and white highlights, over a soft halo in the theme's color.
#define M(x, y) {SGGlyphMove, {x, y, 0, 0, 0, 0}}
#define L(x, y) {SGGlyphLine, {x, y, 0, 0, 0, 0}}
#define C(a, b, c, d, x, y) {SGGlyphCurve, {a, b, c, d, x, y}}
#define Z {SGGlyphClose, {0, 0, 0, 0, 0, 0}}
#define O(x, y, w, h) {SGGlyphOval, {x, y, w, h, 0, 0}}
#define LAYER(r, g, b, r2, g2, b2) {SGGlyphLayer, {r, g, b, r2, g2, b2}}
#define CUT(r, g, b, r2, g2, b2) {SGGlyphLayerCut, {r, g, b, r2, g2, b2}}
#define ALPHA(a) {SGGlyphAlpha, {a, 0, 0, 0, 0, 0}}
#define GLOW(r, g, b, a) {SGGlyphGlow, {r, g, b, a, 0, 0}}

// Three nested tongues: red to orange, yellow, and a white-hot core.
static const SGGlyphOp sg_flame[] = {
    GLOW(1, 0.45, 0.1, 0.5),
    LAYER(1, 0.2, 0.12, 1, 0.55, 0.08), M(12, 1.2), C(13.2, 6, 19, 8.6, 19, 14.6),
    C(19, 19.2, 15.8, 22.8, 12, 22.8), C(8.2, 22.8, 5, 19.2, 5, 14.6),
    C(5, 12, 6.2, 9.8, 7.8, 8.2), C(8, 10.4, 8.9, 11.8, 10.2, 12.2), C(9.5, 8, 10.4, 3.8, 12, 1.2),
    Z,
    LAYER(1, 0.72, 0.12, 1, 0.92, 0.35), M(12, 10.5), C(13.4, 13.2, 16, 14.8, 16, 18),
    C(16, 20.6, 14.2, 22.2, 12, 22.2), C(9.8, 22.2, 8, 20.6, 8, 18),
    C(8, 14.8, 10.6, 13.2, 12, 10.5), Z,
    LAYER(1, 0.97, 0.75, 1, 1, 1), M(12, 16.12), C(12.73, 17.52, 14.08, 18.35, 14.08, 20.02),
    C(14.08, 21.37, 13.14, 22.2, 12, 22.2), C(10.86, 22.2, 9.92, 21.37, 9.92, 20.02),
    C(9.92, 18.35, 11.27, 17.52, 12, 16.12), Z,
};

// Six tapered arms, two pairs of barbs on each, and a bright center.
static const SGGlyphOp sg_snowflake[] = {
    GLOW(0.6, 0.85, 1, 0.45),
    LAYER(1, 1, 1, 0.55, 0.84, 1), M(12.31, 11.46), L(19.76, 15.76), L(21.53, 17.5),
    L(19.14, 16.84), L(11.69, 12.54), Z, M(15.62, 13.76), L(15.85, 16.45), L(15.41, 17.09),
    L(14.85, 16.53), L(14.62, 13.84), Z, M(14.91, 13.35), L(17.35, 12.21), L(18.11, 12.41),
    L(17.78, 13.11), L(15.33, 14.25), Z, M(18.21, 15.26), L(18.37, 17.05), L(17.92, 17.69),
    L(17.37, 17.14), L(17.22, 15.34), Z, M(17.5, 14.85), L(19.14, 14.09), L(19.89, 14.29),
    L(19.56, 14.99), L(17.93, 15.75), Z, M(12.62, 12), L(12.62, 20.6), L(12, 23), L(11.38, 20.6),
    L(11.38, 12), Z, M(12.29, 16.01), L(10.08, 17.56), L(9.3, 17.49), L(9.5, 16.74),
    L(11.71, 15.19), Z, M(12.29, 15.19), L(14.5, 16.74), L(14.7, 17.49), L(13.92, 17.56),
    L(11.71, 16.01), Z, M(12.29, 19.01), L(10.81, 20.04), L(10.03, 19.98), L(10.24, 19.22),
    L(11.71, 18.19), Z, M(12.29, 18.19), L(13.76, 19.22), L(13.97, 19.98), L(13.19, 20.04),
    L(11.71, 19.01), Z, M(12.31, 12.54), L(4.86, 16.84), L(2.47, 17.5), L(4.24, 15.76),
    L(11.69, 11.46), Z, M(8.67, 14.25), L(6.22, 13.11), L(5.89, 12.41), L(6.65, 12.21),
    L(9.09, 13.35), Z, M(9.38, 13.84), L(9.15, 16.53), L(8.59, 17.09), L(8.15, 16.45),
    L(8.38, 13.76), Z, M(6.07, 15.75), L(4.44, 14.99), L(4.11, 14.29), L(4.86, 14.09),
    L(6.5, 14.85), Z, M(6.78, 15.34), L(6.63, 17.14), L(6.08, 17.69), L(5.63, 17.05),
    L(5.79, 15.26), Z, M(11.69, 12.54), L(4.24, 8.24), L(2.47, 6.5), L(4.86, 7.16),
    L(12.31, 11.46), Z, M(8.38, 10.24), L(8.15, 7.55), L(8.59, 6.91), L(9.15, 7.47),
    L(9.38, 10.16), Z, M(9.09, 10.65), L(6.65, 11.79), L(5.89, 11.59), L(6.22, 10.89),
    L(8.67, 9.75), Z, M(5.79, 8.74), L(5.63, 6.95), L(6.08, 6.31), L(6.63, 6.86), L(6.78, 8.66), Z,
    M(6.5, 9.15), L(4.86, 9.91), L(4.11, 9.71), L(4.44, 9.01), L(6.07, 8.25), Z, M(11.38, 12),
    L(11.38, 3.4), L(12, 1), L(12.62, 3.4), L(12.62, 12), Z, M(11.71, 7.99), L(13.92, 6.44),
    L(14.7, 6.51), L(14.5, 7.26), L(12.29, 8.81), Z, M(11.71, 8.81), L(9.5, 7.26), L(9.3, 6.51),
    L(10.08, 6.44), L(12.29, 7.99), Z, M(11.71, 4.99), L(13.19, 3.96), L(13.97, 4.02),
    L(13.76, 4.78), L(12.29, 5.81), Z, M(11.71, 5.81), L(10.24, 4.78), L(10.03, 4.02),
    L(10.81, 3.96), L(12.29, 4.99), Z, M(11.69, 11.46), L(19.14, 7.16), L(21.53, 6.5),
    L(19.76, 8.24), L(12.31, 12.54), Z, M(15.33, 9.75), L(17.78, 10.89), L(18.11, 11.59),
    L(17.35, 11.79), L(14.91, 10.65), Z, M(14.62, 10.16), L(14.85, 7.47), L(15.41, 6.91),
    L(15.85, 7.55), L(15.62, 10.24), Z, M(17.93, 8.25), L(19.56, 9.01), L(19.89, 9.71),
    L(19.14, 9.91), L(17.5, 9.15), Z, M(17.22, 8.66), L(17.37, 6.86), L(17.92, 6.31),
    L(18.37, 6.95), L(18.21, 8.74), Z,
    LAYER(1, 1, 1, 1, 1, 1), O(10.6, 10.6, 2.8, 2.8),
};

// A falling drop, light at the tip and deep blue at the base, with a curved highlight.
static const SGGlyphOp sg_drop[] = {
    GLOW(0.3, 0.65, 1, 0.4),
    LAYER(0.72, 0.95, 1, 0.1, 0.42, 1), M(12, 1.5), C(12, 1.5, 5, 10.2, 5, 15.4),
    C(5, 19.6, 8.2, 22.5, 12, 22.5), C(15.8, 22.5, 19, 19.6, 19, 15.4),
    C(19, 10.2, 12, 1.5, 12, 1.5), Z,
    ALPHA(0.85),
    LAYER(1, 1, 1, 1, 1, 1), M(8.6, 12.6), C(7.4, 14.6, 7, 17.2, 8.6, 19.2),
    C(8, 17, 8.2, 14.6, 9.6, 12.9), C(9.4, 12.4, 8.9, 12.3, 8.6, 12.6), Z,
};

// The same drop in red.
static const SGGlyphOp sg_blood[] = {
    GLOW(0.9, 0.05, 0.1, 0.35),
    LAYER(1, 0.36, 0.3, 0.62, 0, 0.08), M(12, 1.5), C(12, 1.5, 5, 10.2, 5, 15.4),
    C(5, 19.6, 8.2, 22.5, 12, 22.5), C(15.8, 22.5, 19, 19.6, 19, 15.4),
    C(19, 10.2, 12, 1.5, 12, 1.5), Z,
    ALPHA(0.7),
    LAYER(1, 0.8, 0.78, 1, 0.8, 0.78), M(8.6, 12.6), C(7.4, 14.6, 7, 17.2, 8.6, 19.2),
    C(8, 17, 8.2, 14.6, 9.6, 12.9), C(9.4, 12.4, 8.9, 12.3, 8.6, 12.6), Z,
};

// A glossy heart with a highlight on its left lobe.
static const SGGlyphOp sg_heart[] = {
    GLOW(1, 0.3, 0.55, 0.45),
    LAYER(1, 0.6, 0.78, 0.95, 0.08, 0.3), M(12, 21.8), C(4.2, 16.6, 1.2, 12.6, 1.2, 8),
    C(1.2, 2.2, 9.4, 0.8, 12, 6), C(14.6, 0.8, 22.8, 2.2, 22.8, 8),
    C(22.8, 12.6, 19.8, 16.6, 12, 21.8), Z,
    ALPHA(0.6),
    LAYER(1, 1, 1, 1, 1, 1), M(8.65, 5.6), C(9.16, 6.25, 8.66, 7.5, 7.52, 8.38),
    C(6.39, 9.27, 5.06, 9.45, 4.55, 8.8), C(4.04, 8.15, 4.54, 6.9, 5.68, 6.02),
    C(6.81, 5.13, 8.14, 4.95, 8.65, 5.6), Z,
};

// A four-pointed star with a white core and a small companion.
static const SGGlyphOp sg_sparkle[] = {
    GLOW(1, 0.85, 0.4, 0.5),
    LAYER(1, 1, 1, 1, 0.74, 0.2), M(10.5, 2.5), C(11.13, 8.59, 14.91, 12.37, 21, 13),
    C(14.91, 13.63, 11.13, 17.41, 10.5, 23.5), C(9.87, 17.41, 6.09, 13.63, 0, 13),
    C(6.09, 12.37, 9.87, 8.59, 10.5, 2.5), Z,
    LAYER(1, 1, 1, 1, 1, 1), M(10.5, 8.4), C(11.42, 11.07, 12.43, 12.08, 15.1, 13),
    C(12.43, 13.92, 11.42, 14.93, 10.5, 17.6), C(9.58, 14.93, 8.57, 13.92, 5.9, 13),
    C(8.57, 12.08, 9.58, 11.07, 10.5, 8.4), Z,
    LAYER(1, 1, 1, 1, 0.88, 0.5), M(19.6, 0.2), C(19.85, 2.64, 21.36, 4.15, 23.8, 4.4),
    C(21.36, 4.65, 19.85, 6.16, 19.6, 8.6), C(19.35, 6.16, 17.84, 4.65, 15.4, 4.4),
    C(17.84, 4.15, 19.35, 2.64, 19.6, 0.2), Z,
};

// A lightning bolt fading from pale yellow to blue, with a white facet.
static const SGGlyphOp sg_bolt[] = {
    GLOW(0.4, 0.7, 1, 0.6),
    LAYER(1, 1, 0.78, 0.3, 0.55, 1), M(15.6, 0.8), L(5, 13.3), L(10.5, 13.3), L(7.4, 23.2),
    L(19.4, 9.3), L(13.5, 9.3), Z,
    ALPHA(0.9),
    LAYER(1, 1, 1, 1, 1, 1), M(14.1, 3.9), L(8, 11.9), L(11.2, 11.9), Z,
};

// A crescent (two circular arcs) holding two small stars.
static const SGGlyphOp sg_moon[] = {
    GLOW(0.6, 0.45, 1, 0.5),
    LAYER(1, 0.95, 1, 0.56, 0.4, 1), M(9.8, 3.03), C(4.91, 3.44, 1.08, 7.38, 0.81, 12.28),
    C(0.55, 17.17, 3.94, 21.5, 8.76, 22.43), C(13.57, 23.35, 18.32, 20.57, 19.89, 15.93),
    C(16.32, 18.42, 11.42, 17.68, 8.74, 14.25), C(6.06, 10.83, 6.52, 5.9, 9.8, 3.03), Z,
    LAYER(1, 1, 1, 1, 1, 1), M(17.2, 4.8), C(17.4, 6.77, 18.63, 8, 20.6, 8.2),
    C(18.63, 8.4, 17.4, 9.63, 17.2, 11.6), C(17, 9.63, 15.77, 8.4, 13.8, 8.2),
    C(15.77, 8, 17, 6.77, 17.2, 4.8), Z, M(21.2, 12.7), C(21.31, 13.8, 22, 14.49, 23.1, 14.6),
    C(22, 14.71, 21.31, 15.4, 21.2, 16.5), C(21.09, 15.4, 20.4, 14.71, 19.3, 14.6),
    C(20.4, 14.49, 21.09, 13.8, 21.2, 12.7), Z,
};

// Three puffs on a flat base.
static const SGGlyphOp sg_cloud[] = {
    GLOW(0.7, 0.88, 1, 0.35),
    LAYER(1, 1, 1, 0.66, 0.84, 1), O(1.5, 11, 9, 9), O(6, 4.5, 11, 11), O(13.5, 9.5, 9, 9.5),
    M(6, 13.5), L(18, 13.5), L(18, 20), L(6, 20), Z,
};

// A banknote caught mid-air: waving edges, portrait oval and two seals.
static const SGGlyphOp sg_bill[] = {
    GLOW(0.3, 0.9, 0.4, 0.35),
    LAYER(0.62, 0.95, 0.5, 0.1, 0.62, 0.3), M(1.2, 7.6), C(7, 4.2, 16, 10, 22.8, 6), L(22.8, 16.4),
    C(16, 20.4, 7, 14.6, 1.2, 18), Z,
    ALPHA(0.9),
    LAYER(0.9, 1, 0.82, 0.9, 1, 0.82), O(9, 8.4, 6, 7.2), O(3.4, 10.7, 2.2, 2.2),
    O(18.4, 11.1, 2.2, 2.2),
    ALPHA(1),
    LAYER(0.1, 0.55, 0.28, 0.1, 0.55, 0.28), O(10.6, 10.2, 2.8, 3.6),
};

// Five overlapping petals around a golden center.
static const SGGlyphOp sg_flower[] = {
    GLOW(1, 0.5, 0.75, 0.4),
    LAYER(1, 0.82, 0.9, 1, 0.36, 0.62), M(12, 0.8), C(14.04, 0.8, 15.7, 2.86, 15.7, 5.4),
    C(15.7, 7.94, 14.04, 10, 12, 10), C(9.96, 10, 8.3, 7.94, 8.3, 5.4),
    C(8.3, 2.86, 9.96, 0.8, 12, 0.8), Z, M(22.65, 8.54),
    C(23.28, 10.48, 21.84, 12.69, 19.42, 13.48), C(17, 14.26, 14.53, 13.33, 13.9, 11.38),
    C(13.27, 9.44, 14.72, 7.23, 17.13, 6.44), C(19.55, 5.66, 22.02, 6.6, 22.65, 8.54), Z,
    M(18.58, 21.06), C(16.93, 22.26, 14.38, 21.57, 12.89, 19.51),
    C(11.39, 17.46, 11.52, 14.82, 13.18, 13.62), C(14.83, 12.42, 17.38, 13.11, 18.87, 15.16),
    C(20.37, 17.22, 20.24, 19.86, 18.58, 21.06), Z, M(5.42, 21.06),
    C(3.76, 19.86, 3.63, 17.22, 5.13, 15.16), C(6.62, 13.11, 9.17, 12.42, 10.82, 13.62),
    C(12.48, 14.82, 12.61, 17.46, 11.11, 19.51), C(9.62, 21.57, 7.07, 22.26, 5.42, 21.06), Z,
    M(1.35, 8.54), C(1.98, 6.6, 4.45, 5.66, 6.87, 6.44), C(9.28, 7.23, 10.73, 9.44, 10.1, 11.38),
    C(9.47, 13.33, 7, 14.26, 4.58, 13.48), C(2.16, 12.69, 0.72, 10.48, 1.35, 8.54), Z,
    LAYER(1, 0.95, 0.55, 1, 0.72, 0.2), O(9.3, 9.3, 5.4, 5.4),
    ALPHA(0.8),
    LAYER(1, 1, 1, 1, 1, 1), O(10.4, 10.2, 1.6, 1.4),
};

// Two beamed eighth notes with tilted heads.
static const SGGlyphOp sg_note[] = {
    GLOW(0.85, 0.4, 1, 0.45),
    LAYER(1, 0.55, 0.95, 0.5, 0.4, 1), M(9.22, 17.74), C(9.8, 19.17, 8.65, 20.99, 6.65, 21.8),
    C(4.65, 22.6, 2.56, 22.09, 1.98, 20.66), C(1.4, 19.23, 2.55, 17.41, 4.55, 16.6),
    C(6.55, 15.8, 8.64, 16.31, 9.22, 17.74), Z, M(20.62, 14.94),
    C(21.2, 16.37, 20.05, 18.19, 18.05, 19), C(16.05, 19.8, 13.96, 19.29, 13.38, 17.86),
    C(12.8, 16.43, 13.95, 14.61, 15.95, 13.8), C(17.95, 13, 20.04, 13.51, 20.62, 14.94), Z,
    M(7.9, 5.4), L(9.4, 5.4), L(9.4, 19), L(7.9, 19), Z, M(19.3, 2.6), L(20.8, 2.6), L(20.8, 16.2),
    L(19.3, 16.2), Z, M(7.9, 5.2), L(20.8, 2.2), L(20.8, 6.6), L(7.9, 9.6), Z,
};

// A translucent bubble: tinted body, bright rim, two highlights.
static const SGGlyphOp sg_bubble[] = {
    GLOW(0.6, 1, 0.3, 0.4),
    ALPHA(0.28),
    LAYER(0.8, 1, 0.5, 0.8, 1, 0.5), O(3, 3, 18, 18),
    ALPHA(1),
    CUT(0.9, 1, 0.5, 0.3, 0.85, 0.25), O(1.6, 1.6, 20.8, 20.8), O(3.6, 3.6, 16.8, 16.8),
    ALPHA(0.9),
    LAYER(1, 1, 1, 1, 1, 1), M(10.33, 6.51), C(10.8, 7.19, 10.24, 8.41, 9.06, 9.23),
    C(7.88, 10.05, 6.55, 10.17, 6.07, 9.49), C(5.6, 8.81, 6.16, 7.59, 7.34, 6.77),
    C(8.52, 5.95, 9.85, 5.83, 10.33, 6.51), Z, O(15.2, 15.6, 1.6, 1.6),
};

// A cut gem seen from the side. Each facet has its own shade; a glint sits on the corner.
static const SGGlyphOp sg_gem[] = {
    GLOW(0.7, 0.9, 1, 0.5),
    LAYER(0.72, 0.93, 1, 0.72, 0.93, 1), M(6.4, 4), L(1, 10.2), L(7.6, 10.2), Z,
    LAYER(1, 1, 1, 1, 1, 1), M(6.4, 4), L(12, 4), L(7.6, 10.2), Z,
    LAYER(0.6, 0.84, 1, 0.6, 0.84, 1), M(12, 4), L(7.6, 10.2), L(16.4, 10.2), Z,
    LAYER(0.93, 0.98, 1, 0.93, 0.98, 1), M(12, 4), L(17.6, 4), L(16.4, 10.2), Z,
    LAYER(0.5, 0.76, 1, 0.5, 0.76, 1), M(17.6, 4), L(16.4, 10.2), L(23, 10.2), Z,
    LAYER(0.5, 0.76, 1, 0.66, 0.56, 1), M(1, 10.2), L(7.6, 10.2), L(12, 23), Z,
    LAYER(0.9, 0.98, 1, 0.72, 0.8, 1), M(7.6, 10.2), L(16.4, 10.2), L(12, 23), Z,
    LAYER(0.4, 0.64, 0.98, 0.56, 0.46, 0.95), M(16.4, 10.2), L(23, 10.2), L(12, 23), Z,
    LAYER(1, 1, 1, 1, 1, 1), M(20.4, 0.3), C(20.6, 2.21, 21.79, 3.4, 23.7, 3.6),
    C(21.79, 3.8, 20.6, 4.99, 20.4, 6.9), C(20.2, 4.99, 19.01, 3.8, 17.1, 3.6),
    C(19.01, 3.4, 20.2, 2.21, 20.4, 0.3), Z,
};

#define SHAPE(table) {table, sizeof(table) / sizeof(table[0])}
static const SGGlyphShape sg_shapes[SGLyricGlyphCount] = {
    [SGLyricGlyphFlame] = SHAPE(sg_flame),
    [SGLyricGlyphSnowflake] = SHAPE(sg_snowflake),
    [SGLyricGlyphDrop] = SHAPE(sg_drop),
    [SGLyricGlyphBlood] = SHAPE(sg_blood),
    [SGLyricGlyphHeart] = SHAPE(sg_heart),
    [SGLyricGlyphSparkle] = SHAPE(sg_sparkle),
    [SGLyricGlyphBolt] = SHAPE(sg_bolt),
    [SGLyricGlyphMoon] = SHAPE(sg_moon),
    [SGLyricGlyphCloud] = SHAPE(sg_cloud),
    [SGLyricGlyphBill] = SHAPE(sg_bill),
    [SGLyricGlyphFlower] = SHAPE(sg_flower),
    [SGLyricGlyphNote] = SHAPE(sg_note),
    [SGLyricGlyphBubble] = SHAPE(sg_bubble),
    [SGLyricGlyphGem] = SHAPE(sg_gem),
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
        case SGLyricThemeBlood: return SGLyricGlyphBlood;
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
