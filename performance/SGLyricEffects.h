#import <Foundation/Foundation.h>
#import "SGLyricWords.h"

// Default on. Uses the prefix already covered by the mod's settings backup and reset.
#define SGLyricWordEffectsKey @"spotifyglass.performance.lyricWordEffects"

// Drawing callbacks, used on the main thread only. `word` is a verified SGRKaraokeWordView
// and `lit` its sung-text view. Each one is idempotent; the timing policy stays in SGLyricEffects.m.
typedef struct {
    void (*paint)(id word, id lit, SGLyricTheme theme);          // themed text and glow color
    void (*unpaint)(id word, id lit);                            // restore the original colors
    // Particles and motion for a word with `sung` seconds left; must stop by itself after `limit`.
    void (*emit)(id word, SGLyricTheme theme, double sung, double limit);
    void (*quench)(id word);                                     // no new particles or motion
    void (*glow)(id lit, float opacity);                         // glow strength while painted
} SGLyricEffectPainter;

// Installs once, only when the pinned 0.51.0 word view and word model have the verified ABI.
void SGInstallLyricEffects(const SGLyricEffectPainter *painter);
