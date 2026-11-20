#include "SGLyricGlyphs.h"
#include <math.h>
#include <stdlib.h>

// Drawn for this mod; see README.md. Each is a silhouette that still reads at about ten points,
// with cut-outs no thinner than two units so they survive being scaled down.
static const char *const sg_outlines[SGLyricThemeCount] = {
    // A flame with a curl on its left and a hollow heart.
    [SGLyricThemeFire] =
        "M21 3 C22 10 31 14.5 31 24 C31 31.5 26 37 20 37 C14 37 9 31.5 9 24.5 C9 20.5 10.8 17.3 "
        "13.2 15 C13.6 18 14.8 19.8 16.6 20.8 C15.6 14 17.6 7.6 21 3 Z M20.2 22.5 C22.6 25.2 24.6 "
        "27.4 24.6 30 C24.6 32.6 22.6 34.2 20.2 34.2 C17.8 34.2 15.8 32.6 15.8 30 C15.8 27.4 17.8 "
        "25.2 20.2 22.5 Z",
    // A snow crystal: six long and six short blades around a hexagon.
    [SGLyricThemeIce] =
        "M20 15.2 L22.1 9.5 L20 3 L17.9 9.5 Z M23.3 14.3 L26 12.6 L26.4 8.9 L23.4 11.1 Z M24.2 "
        "17.6 L30.1 16.6 L34.7 11.5 L28 12.9 Z M26.6 20 L29.4 21.5 L32.8 20 L29.4 18.5 Z M24.2 "
        "22.4 L28 27.1 L34.7 28.5 L30.1 23.4 Z M23.3 25.7 L23.4 28.9 L26.4 31.1 L26 27.4 Z M20 "
        "24.8 L17.9 30.5 L20 37 L22.1 30.5 Z M16.7 25.7 L14 27.4 L13.6 31.1 L16.6 28.9 Z M15.8 "
        "22.4 L9.9 23.4 L5.3 28.5 L12 27.1 Z M13.4 20 L10.6 18.5 L7.2 20 L10.6 21.5 Z M15.8 17.6 "
        "L12 12.9 L5.3 11.5 L9.9 16.6 Z M16.7 14.3 L16.6 11.1 L13.6 8.9 L14 12.6 Z M20 16.6 L22.9 "
        "18.3 L22.9 21.7 L20 23.4 L17.1 21.7 L17.1 18.3 Z",
    // A drop with a crescent highlight.
    [SGLyricThemeWater] =
        "M20 3 C24 10 31 17 31 25 C31 31.6 26 36.5 20 36.5 C14 36.5 9 31.6 9 25 C9 17 16 10 20 3 "
        "Z M13.4 24 C13.4 28.6 16 31.8 19.6 32.6 C19.2 33.6 17.8 33.6 16.6 33 C13.4 31.4 11.6 28 "
        "12 24.4 C12.2 23.4 13.4 23.2 13.4 24 Z",
    // A heart with a highlight on its left lobe.
    [SGLyricThemeLove] =
        "M20 35.5 C16 32 5 25 5 15.5 C5 10 9 6 13.5 6 C16.3 6 18.6 7.4 20 9.8 C21.4 7.4 23.7 6 "
        "26.5 6 C31 6 35 10 35 15.5 C35 25 24 32 20 35.5 Z M12.8 9.6 C10.2 10 8.4 12.4 8.4 15.2 "
        "C8.4 16.2 9.8 16.2 10 15.2 C10.3 13.2 11.4 11.8 13.2 11.2 C14.2 10.8 13.8 9.4 12.8 9.6 Z",
    // A four-point glint and a smaller one beside it.
    [SGLyricThemeShine] =
        "M18 6 C18 18 21 21 33 21 C21 21 18 24 18 36 C18 24 15 21 3 21 C15 21 18 18 18 6 Z M31.5 "
        "2.5 C31.5 7.3 32.7 8.5 37.5 8.5 C32.7 8.5 31.5 9.7 31.5 14.5 C31.5 9.7 30.3 8.5 25.5 8.5 "
        "C30.3 8.5 31.5 7.3 31.5 2.5 Z",
    // A lightning bolt.
    [SGLyricThemeElectric] =
        "M24 3 L9 22.5 L17.6 22.5 L14.5 37 L31 15.6 L21.6 15.6 Z",
    // A crescent moon holding a star.
    [SGLyricThemeNight] =
        "M24.5 4.5 C16 5.5 9.5 12.5 9.5 21 C9.5 30 16.8 36.5 25.5 36.5 C29 36.5 32 35.5 34.5 33.5 "
        "C26 34 19.5 27.5 19.5 19.5 C19.5 13.5 21.5 8 24.5 4.5 Z M29.5 9 C29.5 13.3 30.7 14.5 35 "
        "14.5 C30.7 14.5 29.5 15.7 29.5 20 C29.5 15.7 28.3 14.5 24 14.5 C28.3 14.5 29.5 13.3 29.5 "
        "9 Z",
    // A cloud.
    [SGLyricThemeSky] =
        "M11.5 30.5 C7 30.5 3.5 27.2 3.5 23 C3.5 19.2 6.3 16.2 10 15.6 C11 11.2 14.8 8 19.5 8 "
        "C24.4 8 28.4 11.4 29.2 16 C33.4 16.2 36.5 19.4 36.5 23.3 C36.5 27.3 33.2 30.5 29 30.5 Z",
    // A banknote with a portrait window and two marks.
    [SGLyricThemeMoney] =
        "M6.5 10.5 L33.5 10.5 C35.4 10.5 37 12.1 37 14 L37 26 C37 27.9 35.4 29.5 33.5 29.5 L6.5 "
        "29.5 C4.6 29.5 3 27.9 3 26 L3 14 C3 12.1 4.6 10.5 6.5 10.5 Z M20 14.8 C22.9 14.8 25.2 "
        "17.1 25.2 20 C25.2 22.9 22.9 25.2 20 25.2 C17.1 25.2 14.8 22.9 14.8 20 C14.8 17.1 17.1 "
        "14.8 20 14.8 Z M9.4 18.1 C10.4 18.1 11.3 19 11.3 20 C11.3 21 10.4 21.9 9.4 21.9 C8.4 "
        "21.9 7.5 21 7.5 20 C7.5 19 8.4 18.1 9.4 18.1 Z M30.6 18.1 C31.6 18.1 32.5 19 32.5 20 "
        "C32.5 21 31.6 21.9 30.6 21.9 C29.6 21.9 28.7 21 28.7 20 C28.7 19 29.6 18.1 30.6 18.1 Z",
    // A five-petal blossom.
    [SGLyricThemeBloom] =
        "M20 17 C25.6 13.4 26.2 6.6 20 3.2 C13.8 6.6 14.4 13.4 20 17 Z M22.9 19.1 C28 23.3 34.7 "
        "21.8 36 14.8 C30.8 10 24.5 12.6 22.9 19.1 Z M21.8 22.4 C19.3 28.6 22.9 34.5 29.9 33.6 "
        "C32.9 27.2 28.4 22 21.8 22.4 Z M18.2 22.4 C11.6 22 7.1 27.2 10.1 33.6 C17.1 34.5 20.7 "
        "28.6 18.2 22.4 Z M17.1 19.1 C15.5 12.6 9.2 10 4 14.8 C5.3 21.8 12 23.3 17.1 19.1 Z M20 "
        "17.4 C21.4 17.4 22.6 18.6 22.6 20 C22.6 21.4 21.4 22.6 20 22.6 C18.6 22.6 17.4 21.4 17.4 "
        "20 C17.4 18.6 18.6 17.4 20 17.4 Z",
    // A sharp drop and a smaller one falling beside it.
    [SGLyricThemeBlood] =
        "M17 3 C19.5 11 27 17.5 27 26 C27 32 22.5 36.5 17 36.5 C11.5 36.5 7 32 7 26 C7 17.5 14.5 "
        "11 17 3 Z M31.5 9 C32.6 12.6 35.5 15 35.5 18.4 C35.5 20.8 33.7 22.5 31.5 22.5 C29.3 22.5 "
        "27.5 20.8 27.5 18.4 C27.5 15 30.4 12.6 31.5 9 Z",
    // A pair of beamed notes.
    [SGLyricThemeParty] =
        "M14 8.5 L34 4 L34 27.5 C34 30.6 31.2 33 27.8 33 C24.6 33 22 31 22 28.4 C22 25.6 24.8 "
        "23.4 28.2 23.4 C29.2 23.4 30.2 23.6 31 24 L31 12.4 L17 15.6 L17 31 C17 34.1 14.2 36.5 "
        "10.8 36.5 C7.6 36.5 5 34.5 5 31.9 C5 29.1 7.8 26.9 11.2 26.9 C12.2 26.9 13.2 27.1 14 "
        "27.5 Z",
    // Bubbles: two hollow, one with a highlight, and a solid bead.
    [SGLyricThemeToxic] =
        "M16 11 C22.6 11 28 16.4 28 23 C28 29.6 22.6 35 16 35 C9.4 35 4 29.6 4 23 C4 16.4 9.4 11 "
        "16 11 Z M16 14.2 C20.9 14.2 24.8 18.1 24.8 23 C24.8 27.9 20.9 31.8 16 31.8 C11.1 31.8 "
        "7.2 27.9 7.2 23 C7.2 18.1 11.1 14.2 16 14.2 Z M10.6 19.4 C11.6 17.4 13.4 16 15.6 15.6 "
        "C16.8 15.4 17 17 15.9 17.3 C14.3 17.7 13 18.7 12.2 20.2 C11.6 21.2 10.1 20.5 10.6 19.4 Z "
        "M31 5 C34 5 36.5 7.5 36.5 10.5 C36.5 13.5 34 16 31 16 C28 16 25.5 13.5 25.5 10.5 C25.5 "
        "7.5 28 5 31 5 Z M31 7.4 C32.7 7.4 34.1 8.8 34.1 10.5 C34.1 12.2 32.7 13.6 31 13.6 C29.3 "
        "13.6 27.9 12.2 27.9 10.5 C27.9 8.8 29.3 7.4 31 7.4 Z M33.5 23.5 C35.2 23.5 36.5 24.8 "
        "36.5 26.5 C36.5 28.2 35.2 29.5 33.5 29.5 C31.8 29.5 30.5 28.2 30.5 26.5 C30.5 24.8 31.8 "
        "23.5 33.5 23.5 Z",
    // A double cup: straw, lid, the outer cup's lip and the cup.
    [SGLyricThemePurple] =
        "M21.5 2.5 L24.5 2.5 L23.2 9 L20.2 9 Z M9.5 10.5 L30.5 10.5 C31.6 10.5 32.5 11.4 32.5 "
        "12.5 L32.5 13.5 C32.5 14.6 31.6 15.5 30.5 15.5 L9.5 15.5 C8.4 15.5 7.5 14.6 7.5 13.5 "
        "L7.5 12.5 C7.5 11.4 8.4 10.5 9.5 10.5 Z M9.6 17.5 L30.4 17.5 L29.5 23.5 L10.5 23.5 Z "
        "M10.8 25.5 L29.2 25.5 L27.9 35 C27.7 36.2 26.8 37 25.6 37 L14.4 37 C13.2 37 12.3 36.2 "
        "12.1 35 Z",
    // A brilliant-cut stone: three crown facets over three pavilion facets.
    [SGLyricThemeDrip] =
        "M11 7 L17.2 7 L14.4 15 L4 15 Z M19.4 7 L20.6 7 L23.4 15 L16.6 15 Z M22.8 7 L29 7 L36 15 "
        "L25.6 15 Z M4.4 17.2 L14.2 17.2 L18.6 34 Z M16.6 17.2 L23.4 17.2 L20 31.5 Z M25.8 17.2 "
        "L35.6 17.2 L21.4 34 Z",
};

const char *SGLyricGlyphOutline(SGLyricTheme theme) {
    return theme > SGLyricThemeNone && theme < SGLyricThemeCount ? sg_outlines[theme] : NULL;
}

// Reads `count` numbers inside the box; strtod skips the spaces between them.
static bool numbers(const char **cursor, double *values, int count) {
    for (int i = 0; i < count; i++) {
        char *end;
        values[i] = strtod(*cursor, &end);
        if (end == *cursor || !isfinite(values[i]) || values[i] < 0.0 || values[i] > SG_LYRIC_GLYPH_BOX)
            return false;
        *cursor = end;
    }
    return true;
}

bool SGLyricGlyphWalk(const char *outline, const SGLyricGlyphSink *sink, void *context) {
    if (!outline || !sink || !sink->move || !sink->line || !sink->curve || !sink->close) return false;
    bool open = false, any = false;
    double v[6];
    for (const char *cursor = outline; ; ) {
        while (*cursor == ' ') cursor++;
        char command = *cursor;
        if (!command) return any && !open;
        cursor++;
        if (command == 'M') {
            if (open || !numbers(&cursor, v, 2)) return false;
            sink->move(context, v[0], v[1]);
            open = any = true;
        } else if (command == 'L') {
            if (!open || !numbers(&cursor, v, 2)) return false;
            sink->line(context, v[0], v[1]);
        } else if (command == 'C') {
            if (!open || !numbers(&cursor, v, 6)) return false;
            sink->curve(context, v[0], v[1], v[2], v[3], v[4], v[5]);
        } else if (command == 'Z') {
            if (!open) return false;
            sink->close(context);
            open = false;
        } else {
            return false;
        }
    }
}
