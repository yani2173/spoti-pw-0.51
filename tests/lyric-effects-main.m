#import <Foundation/Foundation.h>
#import <objc/runtime.h>
#import "../performance/SGLyricEffects.h"
#import "../performance/SGPerformance.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// Stand-ins with the ivar and method ABI verified in the pinned 0.51.0 core. The recording
// painter replaces UIKit drawing: these tests cover timing, policy and restoration only.
static void check(BOOL condition, const char *message) {
    if (!condition) { fprintf(stderr, "FAIL: %s\n", message); exit(1); }
}

@interface UIView : NSObject
@end
@implementation UIView
@end

@interface SGKaraokeWord : NSObject
@property (nonatomic, copy) NSString *text;
@property (nonatomic) long long start;
@property (nonatomic) long long end;
@end
@implementation SGKaraokeWord
@end

@interface SGTestWordModel : NSObject {
    SGKaraokeWord *_word;
    UIView *_lit;
    double _glow;
}
@property (nonatomic) NSInteger nativeGlows, nativeSettles;
@property (nonatomic) double coreGlow;
- (instancetype)initWithText:(NSString *)text start:(long long)start end:(long long)end;
- (void)glowAt:(double)time;
- (void)settle;
- (UIView *)litView;
@end
@implementation SGTestWordModel
- (instancetype)initWithText:(NSString *)text start:(long long)start end:(long long)end {
    if ((self = [super init])) {
        _word = [SGKaraokeWord new];
        _word.text = text;
        _word.start = start;
        _word.end = end;
        _lit = [UIView new];
    }
    return self;
}
- (void)glowAt:(double)time {
    (void)time;
    self.nativeGlows++;
    _glow = self.coreGlow; // Like the core, cache the strength it last applied.
}
- (void)settle { self.nativeSettles++; }
- (UIView *)litView { return _lit; }
@end
@interface SGRKaraokeWordView : SGTestWordModel
@end
@implementation SGRKaraokeWordView
@end

static NSInteger paints, unpaints, emits, quenches, glows;
static SGLyricTheme paintedTheme;
static double emittedSung, emittedLimit;
static float lastGlow;
static __unsafe_unretained id lastLit;

static void paint(id word, id lit, SGLyricTheme theme) { (void)word; paints++; lastLit = lit; paintedTheme = theme; }
static void unpaint(id word, id lit) { (void)word; unpaints++; lastLit = lit; }
static void emit(id word, SGLyricTheme theme, double sung, double limit) {
    (void)word; (void)theme;
    emits++; emittedSung = sung; emittedLimit = limit;
}
static void quench(id word) { (void)word; quenches++; }
static void glow(id lit, float opacity) { (void)lit; glows++; lastGlow = opacity; }
static const SGLyricEffectPainter painter = {paint, unpaint, emit, quench, glow};

static void (*lightObserver)(BOOL);
static BOOL startInLightMode;
void SGPerformanceSetLightModeObserver(void (*observer)(BOOL lightMode)) {
    lightObserver = observer;
    if (observer) observer(startInLightMode);
}

static NSString *(*statusProvider)(void);
void SGPerformanceSetWordEffectsStatus(NSString *(*provider)(void)) { statusProvider = provider; }
static BOOL statusIs(NSString *expected) { return statusProvider && [statusProvider() isEqualToString:expected]; }

static BOOL near(double a, double b) { return fabs(a - b) < 0.0001; }
static void setWanted(id value) {
    NSUserDefaults *store = NSUserDefaults.standardUserDefaults;
    if (value) [store setObject:value forKey:SGLyricWordEffectsKey];
    else [store removeObjectForKey:SGLyricWordEffectsKey];
    [NSNotificationCenter.defaultCenter postNotificationName:NSUserDefaultsDidChangeNotification object:nil];
}

int main(int argc, const char **argv) {
    @autoreleasepool {
        NSString *mode = argc > 1 ? @(argv[1]) : @"";
        [NSUserDefaults.standardUserDefaults removeObjectForKey:SGLyricWordEffectsKey];
        if ([mode isEqual:@"unsupported"]) {
            SEL selector = @selector(glowAt:);
            Method method = class_getInstanceMethod(SGRKaraokeWordView.class, selector);
            class_addMethod(SGRKaraokeWordView.class, selector, method_getImplementation(method), "v20@0:8f16");
            SGInstallLyricEffects(&painter);
            SGRKaraokeWordView *view = [[SGRKaraokeWordView alloc] initWithText:@"fire" start:1000 end:2000];
            [view glowAt:1500];
            check(view.nativeGlows == 1 && !paints && !emits && !lightObserver, "an unsupported word-view ABI is left untouched");
            check(statusIs(@"Unsupported app version"), "the status reports an unsupported ABI");
            puts("PASS: unsupported word effect ABI");
            return 0;
        }
        if ([mode isEqual:@"disabled"]) {
            [NSUserDefaults.standardUserDefaults setBool:NO forKey:SGLyricWordEffectsKey];
            SGInstallLyricEffects(&painter);
            SGRKaraokeWordView *view = [[SGRKaraokeWordView alloc] initWithText:@"fire" start:1000 end:2000];
            [view glowAt:1500];
            check(view.nativeGlows == 1 && !paints && !emits && !glows, "a saved off preference applies from startup");
            check(statusIs(@"Off"), "the status reports the saved preference");
            setWanted(nil);
            [view glowAt:1500];
            check(paints == 1 && emits == 1, "resetting the preference restores the default-on effects live");
            puts("PASS: saved word effect preference");
            return 0;
        }
        if ([mode isEqual:@"light-mode"]) {
            startInLightMode = YES;
            SGInstallLyricEffects(&painter);
            SGRKaraokeWordView *view = [[SGRKaraokeWordView alloc] initWithText:@"fire" start:1000 end:2000];
            [view glowAt:1500];
            check(view.nativeGlows == 1 && !paints && !emits, "Light mode at startup keeps lyrics plain");
            check(statusIs(@"Off \u00b7 Light mode"), "the status names Light mode as the reason");
            lightObserver(NO);
            [view glowAt:1500];
            check(paints == 1 && emits == 1, "leaving Light mode enables the effects on the next tick");
            puts("PASS: word effects with Light mode at startup");
            return 0;
        }

        SGInstallLyricEffects(NULL);
        SGInstallLyricEffects(&painter);
        SGInstallLyricEffects(&painter);
        check(lightObserver != NULL, "the module follows the effective Light mode");

        SGRKaraokeWordView *plain = [[SGRKaraokeWordView alloc] initWithText:@"the" start:1000 end:2000];
        for (double time = 0; time < 3000; time += 100) [plain glowAt:time];
        [plain settle];
        check(plain.nativeGlows == 30 && plain.nativeSettles == 1 && !paints && !emits && !glows && !quenches,
              "unthemed words only reach the original methods");
        check(statusIs(@"On \u00b7 1 words seen, 0 matched, 0 shown"), "the status counts words that reached the hook");

        SGRKaraokeWordView *view = [[SGRKaraokeWordView alloc] initWithText:@"Burns," start:1000 end:2000];
        [view glowAt:500];
        check(view.nativeGlows == 1 && !paints && !glows, "nothing changes before a themed word is due");
        [view glowAt:850];
        check(paints == 1 && paintedTheme == SGLyricThemeFire && lastLit == view.litView && !emits,
              "the themed text is prepared just before the word, on its sung-text view");
        check(glows == 1 && lastGlow == 0, "the glow starts from the core's strength");
        check(statusIs(@"On \u00b7 2 words seen, 1 matched, 1 shown"), "the status counts matched and shown words");
        [view glowAt:1500];
        check(emits == 1 && near(emittedSung, 0.5) && near(emittedLimit, 1.5), "particles start once with a bounded lifetime");
        check(near(lastGlow, 0.9), "the themed glow follows the word");
        NSInteger glowWrites = glows;
        for (int i = 0; i < 200; i++) [view glowAt:1500];
        check(paints == 1 && emits == 1 && glows == glowWrites && view.nativeGlows == 203,
              "a paused clock repeats no drawing work");
        view.coreGlow = 0.3;
        [view glowAt:1500];
        check(glows == glowWrites + 1 && near(lastGlow, 0.9), "the themed glow is reapplied after the core writes its own");
        view.coreGlow = 0.95;
        [view glowAt:1500];
        check(near(lastGlow, 0.95), "a stronger core glow is kept");
        view.coreGlow = 0.3;
        [view glowAt:2000];
        check(quenches == 1 && paints == 1 && !unpaints, "particles stop at the word's end while its color stays");
        [view glowAt:2000];
        check(quenches == 1, "stopping is not repeated");
        [view glowAt:3000];
        check(near(lastGlow, 0.3) && !unpaints, "afterwards the glow is the core's own strength");
        [view glowAt:500];
        check(unpaints == 1 && lastLit == view.litView && near(lastGlow, 0.3), "seeking before the word restores its text and glow");
        [view glowAt:500];
        check(unpaints == 1, "restoring is not repeated");
        [view glowAt:1200];
        check(paints == 2 && emits == 2, "the word plays again on another pass");
        [view settle];
        check(view.nativeSettles == 1 && quenches == 2 && unpaints == 1, "a settled line stops particles and keeps its color for the fade");
        [view glowAt:1300];
        check(emits == 2, "particles are not restarted within the same pass");
        [view glowAt:900];
        [view glowAt:1300];
        check(emits == 3, "a seek back to just before the word allows another pass");

        setWanted(@NO);
        check(unpaints == 2 && quenches == 3, "turning the setting off restores shown words immediately");
        NSInteger before = view.nativeGlows;
        [view glowAt:1400];
        check(view.nativeGlows == before + 1 && paints == 2 && emits == 3, "disabled effects pass through");
        setWanted(@YES);
        [view glowAt:1400];
        check(paints == 3 && emits == 3, "re-enabling repaints on the next tick without replaying particles mid-word");

        lightObserver(YES);
        check(unpaints == 3, "Light mode restores shown words immediately");
        [view glowAt:1450];
        check(paints == 3, "Light mode keeps lyrics plain");
        lightObserver(NO);
        [view glowAt:1450];
        check(paints == 4, "leaving Light mode resumes on the next tick");

        dispatch_semaphore_t done = dispatch_semaphore_create(0);
        dispatch_async(dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0), ^{
            SGRKaraokeWordView *worker = [[SGRKaraokeWordView alloc] initWithText:@"fire" start:1000 end:2000];
            [worker glowAt:1500];
            [worker settle];
            dispatch_semaphore_signal(done);
        });
        dispatch_semaphore_wait(done, DISPATCH_TIME_FOREVER);
        check(paints == 4 && emits == 3, "off-main calls pass through");

        SGTestWordModel *other = [[SGTestWordModel alloc] initWithText:@"fire" start:1000 end:2000];
        [other glowAt:1500];
        check(other.nativeGlows == 1 && paints == 4, "inherited methods are overridden only on the mod class");

        SGRKaraokeWordView *unsynced = [[SGRKaraokeWordView alloc] initWithText:@"love" start:5000 end:5000];
        [unsynced glowAt:5100];
        check(paints == 5 && paintedTheme == SGLyricThemeLove && emits == 4, "a word without a usable span still gets a short effect");

        __weak SGRKaraokeWordView *released;
        @autoreleasepool {
            SGRKaraokeWordView *temporary = [[SGRKaraokeWordView alloc] initWithText:@"star" start:0 end:1000];
            [temporary glowAt:500];
            released = temporary;
        }
        check(!released, "painted words are not retained");
        setWanted(@NO);
        setWanted(nil);
        check(unpaints == 5, "a released word is skipped when the others are restored");
        puts("PASS: word effect timing, replays, glow hand-off, settings, Light mode, threads, ABI and ownership");
    }
    return 0;
}
