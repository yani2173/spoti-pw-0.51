#import "SGLyricEffects.h"
#import "SGPerformance.h"
#import "SGPerfABI.h"
#include <math.h>

// Sung-word effects for the verified 0.51.0 karaoke word view (see README.md).
// SGRKaraokeSweep.showTime: sends glowAt: with the song clock to every word of a shown line on
// each lyrics tick. This file only decides *when* a themed word is painted, emitting or restored;
// it needs no timer, display link or view traversal of its own. Drawing is behind SGLyricEffectPainter.
@protocol SGLyricEffectWord <NSObject>
@property (nonatomic, readonly) NSString *text;
@property (nonatomic, readonly) long long start;
@property (nonatomic, readonly) long long end;
@end

@interface SGLyricEffectState : NSObject
@property (nonatomic) SGLyricTheme theme;
@property (nonatomic) BOOL painted;
@property (nonatomic) BOOL emitting;
@property (nonatomic) BOOL emitted;
@property (nonatomic) float glow;
@end
@implementation SGLyricEffectState
@end

static SGLyricEffectPainter sg_painter;
static void (*sg_glowAt)(id, SEL, double);
static void (*sg_settle)(id, SEL);
static Ivar sg_wordIvar, sg_litIvar;
static ptrdiff_t sg_glowOffset;
static Class sg_wordClass;
static char sg_stateKey;
static SGLyricEffectState *sg_plain;
static NSHashTable *sg_active;
static BOOL sg_enabled, sg_wanted, sg_lightMode, sg_installed;
// Shown in Mod Settings: tells an unsupported app, a disabled policy and "no matching words" apart.
static NSUInteger sg_seen, sg_matched, sg_painted;

static NSString *status(void) {
    if (!sg_installed) return @"Unsupported app version";
    if (!sg_wanted) return @"Off";
    if (sg_lightMode) return @"Off \u00b7 Light mode";
    return [NSString stringWithFormat:@"On \u00b7 %lu words seen, %lu matched, %lu shown",
            (unsigned long)sg_seen, (unsigned long)sg_matched, (unsigned long)sg_painted];
}

static double coreGlow(id view) {
    double glow;
    memcpy(&glow, (const char *)(__bridge const void *)view + sg_glowOffset, sizeof(glow));
    return glow;
}

static SGLyricEffectState *stateOf(id view) {
    SGLyricEffectState *state = objc_getAssociatedObject(view, &sg_stateKey);
    if (state) return state;
    // Classified once per word view. Every unthemed word shares one immutable state.
    state = sg_plain;
    sg_seen++;
    id<SGLyricEffectWord> word = object_getIvar(view, sg_wordIvar);
    NSString *text = [word isKindOfClass:sg_wordClass] ? word.text : nil;
    if ([text isKindOfClass:NSString.class]) {
        const char *utf8 = text.UTF8String;
        SGLyricTheme theme = utf8 ? SGLyricThemeForWord(utf8, strlen(utf8)) : SGLyricThemeNone;
        if (theme != SGLyricThemeNone) {
            sg_matched++;
            state = [SGLyricEffectState new];
            state.theme = theme;
            state.glow = -1;
        }
    }
    objc_setAssociatedObject(view, &sg_stateKey, state, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    return state;
}

static void quench(id view, SGLyricEffectState *state) {
    if (!state.emitting) return;
    state.emitting = NO;
    sg_painter.quench(view);
}

static void restore(id view, SGLyricEffectState *state) {
    quench(view, state);
    if (state.painted) {
        state.painted = NO;
        state.glow = -1;
        id lit = object_getIvar(view, sg_litIvar);
        sg_painter.unpaint(view, lit);
        // Hand the glow back at the strength the core last computed and cached for this word.
        if (lit) sg_painter.glow(lit, (float)coreGlow(view));
    }
    [sg_active removeObject:view];
}

static void update(id view, SGLyricEffectState *state, double time, BOOL coreWroteGlow) {
    id<SGLyricEffectWord> word = object_getIvar(view, sg_wordIvar);
    id lit = object_getIvar(view, sg_litIvar);
    if (!lit || ![word isKindOfClass:sg_wordClass]) {
        restore(view, state);
        return;
    }
    SGLyricPhase phase = SGLyricPhaseAt(time, word.start, word.end);
    if (phase.upcoming) state.emitted = NO;
    if (!phase.painted) {
        if (state.painted || state.emitting) restore(view, state);
        return;
    }
    if (!state.painted) {
        state.painted = YES;
        sg_painted++;
        [sg_active addObject:view];
        sg_painter.paint(view, lit, state.theme);
    }
    if (!phase.emitting) {
        quench(view, state);
    } else if (!state.emitted) {
        // Once per pass over the word: a paused clock repeats the same time on every tick.
        state.emitted = state.emitting = YES;
        sg_painter.emit(view, state.theme, phase.remaining, phase.safety);
    }
    // The core writes its own opacity only when its cached value changes; write after it then,
    // and otherwise only when the themed envelope moved.
    float glow = fmaxf((float)coreGlow(view), 0.9f * phase.glow);
    if (coreWroteGlow || fabsf(glow - state.glow) > 0.004f) {
        state.glow = glow;
        sg_painter.glow(lit, glow);
    }
}

static void glowAt(id view, SEL cmd, double time) {
    SGLyricEffectState *state = (sg_enabled && NSThread.isMainThread) ? stateOf(view) : nil;
    if (!state || state == sg_plain) {
        sg_glowAt(view, cmd, time);
        return;
    }
    double before = coreGlow(view);
    sg_glowAt(view, cmd, time);
    update(view, state, time, coreGlow(view) != before);
}

static void settle(id view, SEL cmd) {
    sg_settle(view, cmd);
    if (!NSThread.isMainThread) return;
    // The line changed state or its ticks are about to stop: no new particles. Its themed text
    // stays with the line's own fade and is repainted or restored by the next tick's time.
    SGLyricEffectState *state = objc_getAssociatedObject(view, &sg_stateKey);
    if (state) quench(view, state);
}

static void applyPolicy(void) {
    BOOL enabled = sg_wanted && !sg_lightMode;
    if (enabled == sg_enabled) return;
    sg_enabled = enabled;
    if (enabled) return;
    for (id view in sg_active.allObjects) {
        SGLyricEffectState *state = objc_getAssociatedObject(view, &sg_stateKey);
        if (state) restore(view, state);
    }
}

static BOOL wanted(void) {
    NSUserDefaults *store = NSUserDefaults.standardUserDefaults;
    return ![store objectForKey:SGLyricWordEffectsKey] || [store boolForKey:SGLyricWordEffectsKey];
}

static void lightModeChanged(BOOL lightMode) {
    sg_lightMode = lightMode;
    applyPolicy();
}

static BOOL hasIvar(Class cls, const char *name, const char *type, Ivar *ivar) {
    *ivar = class_getInstanceVariable(cls, name);
    const char *actual = *ivar ? ivar_getTypeEncoding(*ivar) : NULL;
    return actual && strcmp(actual, type) == 0;
}

void SGInstallLyricEffects(const SGLyricEffectPainter *painter) {
    if (!painter || !painter->paint || !painter->unpaint || !painter->emit ||
        !painter->quench || !painter->glow) return;
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        SGPerformanceSetWordEffectsStatus(status);
        Class view = NSClassFromString(@"SGRKaraokeWordView");
        Class word = NSClassFromString(@"SGKaraokeWord");
        SEL glow = sel_registerName("glowAt:"), settled = sel_registerName("settle");
        Ivar glowIvar;
        if (!SGPerfHasMethod(view, glow, 'v', "d") || !SGPerfHasMethod(view, settled, 'v', "") ||
            !SGPerfHasMethod(word, sel_registerName("text"), '@', "") ||
            !SGPerfHasMethod(word, sel_registerName("start"), 'q', "") ||
            !SGPerfHasMethod(word, sel_registerName("end"), 'q', "") ||
            !hasIvar(view, "_word", "@\"SGKaraokeWord\"", &sg_wordIvar) ||
            !hasIvar(view, "_lit", "@\"UIView\"", &sg_litIvar) ||
            !hasIvar(view, "_glow", "d", &glowIvar)) return;
        sg_glowOffset = ivar_getOffset(glowIvar);
        sg_wordClass = word;
        sg_painter = *painter;
        sg_plain = [SGLyricEffectState new];
        sg_active = [NSHashTable weakObjectsHashTable];
        sg_wanted = wanted();
        sg_installed = YES;
        sg_glowAt = (void (*)(id, SEL, double))method_getImplementation(class_getInstanceMethod(view, glow));
        sg_settle = (void (*)(id, SEL))method_getImplementation(class_getInstanceMethod(view, settled));
        // Light mode and the saved preference are both known before a word can be painted.
        SGPerformanceSetLightModeObserver(lightModeChanged);
        [NSNotificationCenter.defaultCenter addObserverForName:NSUserDefaultsDidChangeNotification object:nil
                                                         queue:nil usingBlock:^(NSNotification *note) {
            (void)note;
            // Covers the native switch, reset and import. Queued blocks read the current value.
            void (^refresh)(void) = ^{
                sg_wanted = wanted();
                applyPolicy();
            };
            if (NSThread.isMainThread) refresh();
            else dispatch_async(dispatch_get_main_queue(), refresh);
        }];
        SGPerfReplaceMethod(view, glow, (IMP)glowAt);
        SGPerfReplaceMethod(view, settled, (IMP)settle);
    });
}
