#import "SGLyricEffects.h"
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <objc/runtime.h>
#include <math.h>

// UIKit/Core Animation drawing for sung-word effects. Only public API is used here; the timing
// policy and the verified 0.51.0 hooks are in SGLyricEffects.m. Nothing in this file runs per
// frame: text is recolored once, and motes/motion are rendered by Core Animation.
//
// The look is deliberately quiet. A word keeps the lyric's white at its top and takes a soft
// tint toward its base, lifts once on a critically damped spring while it is sung, settles the
// same way, and gives off a few small motes of light. There are no shakes, bounces, loops or
// hand-drawn particles. A theme is carried by hue, by which way its particles drift, and by one
// small glyph from Apple's own SF Symbols set (a heart, a snowflake, a drop, a sparkle), flat and
// used sparingly, so the particles share the system's drawing instead of imitating it.

typedef struct {
    CGFloat top[3], middle[3], bottom[3]; // text gradient, top to bottom
    CGFloat glow[3];
    CGFloat mote[3];
    CGFloat drift;                        // mote acceleration in points/s^2; negative rises
    float rate;                           // motes per second for a word about 70 points wide
    float lifetime;
    CGFloat response;                     // spring response in seconds
    const char *symbol;                   // SF Symbol name; NULL for a plain mote of light
    CGFloat spin;                         // how far a particle may turn, in radians per second
} SGThemeLook;

static const SGThemeLook sg_looks[SGLyricThemeCount] = {
    [SGLyricThemeFire] = {{1.00, 0.96, 0.86}, {1.00, 0.74, 0.42}, {1.00, 0.48, 0.24},
                          {1.00, 0.52, 0.22}, {1.00, 0.78, 0.50}, -34, 9, 1.2f, 0.36,
                          "flame.fill", 0.2},
    [SGLyricThemeIce] = {{1.00, 1.00, 1.00}, {0.86, 0.95, 1.00}, {0.62, 0.84, 1.00},
                         {0.62, 0.86, 1.00}, {0.92, 0.97, 1.00}, 14, 6, 1.7f, 0.46,
                          "snowflake", 0.6},
    [SGLyricThemeWater] = {{0.90, 0.97, 1.00}, {0.60, 0.84, 1.00}, {0.32, 0.64, 1.00},
                           {0.30, 0.62, 1.00}, {0.72, 0.88, 1.00}, 30, 6, 1.3f, 0.46,
                          "drop.fill", 0},
    [SGLyricThemeLove] = {{1.00, 0.93, 0.95}, {1.00, 0.66, 0.76}, {1.00, 0.38, 0.54},
                          {1.00, 0.38, 0.56}, {1.00, 0.74, 0.82}, -18, 5, 1.6f, 0.42,
                          "heart.fill", 0.2},
    [SGLyricThemeShine] = {{1.00, 1.00, 0.97}, {1.00, 0.93, 0.68}, {1.00, 0.80, 0.38},
                           {1.00, 0.84, 0.45}, {1.00, 0.95, 0.78}, 0, 7, 1.3f, 0.40,
                          "sparkle", 0.5},
    [SGLyricThemeElectric] = {{1.00, 1.00, 1.00}, {0.80, 0.92, 1.00}, {0.50, 0.66, 1.00},
                              {0.52, 0.72, 1.00}, {0.86, 0.94, 1.00}, 0, 7, 0.9f, 0.26,
                          "bolt.fill", 0.2},
    [SGLyricThemeNight] = {{0.95, 0.92, 1.00}, {0.76, 0.68, 1.00}, {0.55, 0.46, 0.98},
                           {0.60, 0.46, 1.00}, {0.82, 0.76, 1.00}, -4, 5, 1.9f, 0.56,
                          "moon.fill", 0.15},
    [SGLyricThemeSky] = {{1.00, 1.00, 1.00}, {0.84, 0.94, 1.00}, {0.60, 0.82, 1.00},
                         {0.62, 0.84, 1.00}, {0.95, 0.98, 1.00}, -20, 5, 1.8f, 0.50,
                          "cloud.fill", 0},
    [SGLyricThemeMoney] = {{1.00, 0.99, 0.86}, {0.80, 0.94, 0.62}, {0.42, 0.82, 0.50},
                           {0.45, 0.85, 0.50}, {0.88, 0.96, 0.70}, -10, 6, 1.4f, 0.38,
                          "dollarsign", 0.2},
    [SGLyricThemeBloom] = {{1.00, 0.96, 1.00}, {0.96, 0.78, 0.98}, {0.84, 0.56, 0.92},
                           {0.86, 0.56, 0.94}, {0.98, 0.84, 1.00}, 12, 5, 1.8f, 0.50,
                          "leaf.fill", 1.0},
    [SGLyricThemeBlood] = {{1.00, 0.90, 0.88}, {1.00, 0.52, 0.48}, {0.90, 0.20, 0.22},
                           {0.92, 0.20, 0.22}, {1.00, 0.60, 0.56}, 20, 4, 1.5f, 0.44,
                          "drop.fill", 0},
    [SGLyricThemeParty] = {{1.00, 0.94, 0.90}, {1.00, 0.66, 0.72}, {0.92, 0.42, 0.86},
                           {0.96, 0.46, 0.80}, {1.00, 0.80, 0.88}, -12, 8, 1.2f, 0.32,
                          "music.note", 0.25},
    [SGLyricThemeToxic] = {{0.98, 1.00, 0.86}, {0.84, 0.98, 0.50}, {0.60, 0.90, 0.30},
                           {0.60, 0.92, 0.30}, {0.90, 1.00, 0.66}, -16, 5, 1.6f, 0.44,
                          "circle", 0},
};

// How far a sung word rises: 4% larger and about a point up, the same on every theme.
static const CGFloat sg_riseScale = 0.04;
static const CGFloat sg_risePoints = 1.2;
// At most this many words give off motes at once; further words keep color, glow and motion.
static const NSUInteger sg_emitterLimit = 3;

@interface SGLyricPaintRecord : NSObject
@property (nonatomic) BOOL painted;
@property (nonatomic, copy) NSArray<UILabel *> *labels;
@property (nonatomic, copy) NSArray<UIColor *> *colors;
@property (nonatomic, weak) CALayer *shadowLayer;
@property (nonatomic, strong) id shadowColor;
@property (nonatomic, weak) CAEmitterLayer *emitter;
@property (nonatomic) BOOL risen;
@property (nonatomic) CGFloat lift;
@property (nonatomic) CGFloat response;
@property (nonatomic) NSUInteger motion; // generation: a delayed cleanup never touches a newer pass
@end
@implementation SGLyricPaintRecord
@end

static char sg_recordKey;
static NSHashTable<CAEmitterLayer *> *sg_emitters;
static NSString *const sg_riseScaleKey = @"spotifyglass.lyricWord.riseScale";
static NSString *const sg_riseLiftKey = @"spotifyglass.lyricWord.riseLift";
static NSString *const sg_fallScaleKey = @"spotifyglass.lyricWord.fallScale";
static NSString *const sg_fallLiftKey = @"spotifyglass.lyricWord.fallLift";

static SGLyricPaintRecord *recordOf(id word, BOOL create) {
    SGLyricPaintRecord *record = objc_getAssociatedObject(word, &sg_recordKey);
    if (!record && create) {
        record = [SGLyricPaintRecord new];
        objc_setAssociatedObject(word, &sg_recordKey, record, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }
    return record;
}

static BOOL themed(SGLyricTheme theme) {
    return theme != SGLyricThemeNone && theme < SGLyricThemeCount;
}

static UIColor *solid(const CGFloat rgb[3], CGFloat alpha) {
    return [UIColor colorWithRed:rgb[0] green:rgb[1] blue:rgb[2] alpha:alpha];
}

// A vertical gradient tile as tall as the label, so it repeats only sideways. A label draws
// its own glyphs with it, which keeps the core's per-letter wave and lift transforms intact.
static UIColor *themeFill(SGLyricTheme theme, CGFloat height) {
    if (!(height >= 1) || height > 512) return nil;
    static NSMutableDictionary<NSNumber *, UIColor *> *fills;
    if (!fills) fills = [NSMutableDictionary dictionary];
    NSInteger rows = (NSInteger)ceil(height);
    NSNumber *key = @((NSInteger)theme * 1024 + rows);
    UIColor *fill = fills[key];
    if (fill) return fill;
    const SGThemeLook *look = &sg_looks[theme];
    UIGraphicsImageRenderer *renderer = [[UIGraphicsImageRenderer alloc] initWithSize:CGSizeMake(2, rows)];
    UIImage *image = [renderer imageWithActions:^(UIGraphicsImageRendererContext *context) {
        CGFloat components[12] = {
            look->top[0], look->top[1], look->top[2], 1,
            look->middle[0], look->middle[1], look->middle[2], 1,
            look->bottom[0], look->bottom[1], look->bottom[2], 1};
        // The lyric's own white holds through the upper part of the glyphs.
        CGFloat locations[3] = {0.30, 0.60, 0.86};
        CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
        CGGradientRef gradient = CGGradientCreateWithColorComponents(space, components, locations, 3);
        CGContextDrawLinearGradient(context.CGContext, gradient, CGPointZero, CGPointMake(0, rows),
            kCGGradientDrawsBeforeStartLocation | kCGGradientDrawsAfterEndLocation);
        CGGradientRelease(gradient);
        CGColorSpaceRelease(space);
    }];
    if (!image) return nil;
    fill = [UIColor colorWithPatternImage:image];
    if (fills.count >= 32) [fills removeAllObjects]; // Font sizes are few; this only bounds the cache.
    fills[key] = fill;
    return fill;
}

// Particle images, drawn once, cached and tinted by the emitter. A theme's glyph is the system's
// SF Symbol, rendered flat in white with a faint halo; nothing here draws a shape by hand. If a
// symbol is missing on this iOS version the theme falls back to the plain mote of light.
static const CGFloat sg_canvas = 40;

static UIImage *particleImage(const char *symbol) {
    static NSMutableDictionary<NSString *, UIImage *> *images;
    if (!images) images = [NSMutableDictionary dictionary];
    NSString *name = symbol ? @(symbol) : @"";
    UIImage *cached = images[name];
    if (cached) return cached;
    UIImage *glyph = nil;
    if (name.length) {
        UIImageSymbolConfiguration *style =
            [UIImageSymbolConfiguration configurationWithPointSize:26 weight:UIImageSymbolWeightSemibold];
        glyph = [[UIImage systemImageNamed:name withConfiguration:style]
                 imageWithTintColor:UIColor.whiteColor renderingMode:UIImageRenderingModeAlwaysOriginal];
    }
    UIGraphicsImageRenderer *renderer =
        [[UIGraphicsImageRenderer alloc] initWithSize:CGSizeMake(sg_canvas, sg_canvas)];
    UIImage *image = [renderer imageWithActions:^(UIGraphicsImageRendererContext *context) {
        CGContextRef canvas = context.CGContext;
        CGSize size = glyph.size;
        if (!glyph || !(size.width >= 1) || !(size.height >= 1)) {
            // A bright core falling off to nothing, with no visible edge.
            CGFloat components[12] = {1, 1, 1, 1, 1, 1, 1, 0.35, 1, 1, 1, 0};
            CGFloat locations[3] = {0, 0.35, 1};
            CGPoint center = CGPointMake(sg_canvas / 2, sg_canvas / 2);
            CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
            CGGradientRef gradient = CGGradientCreateWithColorComponents(space, components, locations, 3);
            CGContextDrawRadialGradient(canvas, gradient, center, 0, center, sg_canvas / 2, 0);
            CGGradientRelease(gradient);
            CGColorSpaceRelease(space);
            return;
        }
        // The symbol keeps its own proportions inside a 30-point box, leaving room for the halo.
        CGFloat fit = (CGFloat)fmin(30 / size.width, 30 / size.height);
        CGSize drawn = CGSizeMake(size.width * fit, size.height * fit);
        CGContextSetShadowWithColor(canvas, CGSizeZero, 3, [UIColor colorWithWhite:1 alpha:0.55].CGColor);
        [glyph drawInRect:CGRectMake((sg_canvas - drawn.width) / 2, (sg_canvas - drawn.height) / 2,
                                     drawn.width, drawn.height)];
    }];
    if (image) images[name] = image;
    return image;
}

static void paint(id word, id lit, SGLyricTheme theme) {
    if (!themed(theme) || ![lit isKindOfClass:UIView.class]) return;
    SGLyricPaintRecord *record = recordOf(word, YES);
    if (record.painted) return;
    UIView *view = lit;
    // Verified in the core: `_lit` is one label, or a container of per-letter labels.
    NSArray *candidates = [view isKindOfClass:UILabel.class] ? @[view] : view.subviews;
    NSMutableArray<UILabel *> *labels = [NSMutableArray array];
    NSMutableArray<UIColor *> *colors = [NSMutableArray array];
    for (UILabel *label in candidates) {
        if (![label isKindOfClass:UILabel.class]) continue;
        UIColor *fill = themeFill(theme, CGRectGetHeight(label.bounds));
        if (!fill) continue;
        [labels addObject:label];
        [colors addObject:label.textColor ?: UIColor.whiteColor];
        label.textColor = fill;
    }
    CALayer *layer = view.layer;
    record.labels = labels;
    record.colors = colors;
    record.shadowLayer = layer;
    record.shadowColor = (__bridge id)layer.shadowColor;
    layer.shadowColor = solid(sg_looks[theme].glow, 1).CGColor;
    record.painted = YES;
}

static void unpaint(id word, id lit) {
    (void)lit;
    SGLyricPaintRecord *record = recordOf(word, NO);
    if (!record.painted) return;
    record.painted = NO;
    NSArray<UIColor *> *colors = record.colors;
    [record.labels enumerateObjectsUsingBlock:^(UILabel *label, NSUInteger index, BOOL *stop) {
        (void)stop;
        label.textColor = colors[index];
    }];
    record.shadowLayer.shadowColor = (__bridge CGColorRef)record.shadowColor;
    record.labels = nil;
    record.colors = nil;
    record.shadowColor = nil;
}

// Stops new motes; the ones in flight fade out, then the layer is removed.
static void retire(CAEmitterLayer *emitter) {
    if (!emitter || emitter.birthRate == 0) return;
    emitter.birthRate = 0;
    __weak CAEmitterLayer *weakEmitter = emitter;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [weakEmitter removeFromSuperlayer];
    });
}

// A critically damped spring (damping ratio 1): it arrives without overshoot. It is additive,
// so it adds to whatever transform the core sets and two of them blend without a jump.
static CASpringAnimation *spring(NSString *path, CGFloat change, CGFloat response) {
    CASpringAnimation *animation = [CASpringAnimation animationWithKeyPath:path];
    animation.mass = 1;
    animation.stiffness = (CGFloat)pow(2 * M_PI / response, 2);
    animation.damping = (CGFloat)(4 * M_PI / response);
    animation.fromValue = @0;
    animation.toValue = @(change);
    animation.additive = YES;
    animation.duration = animation.settlingDuration;
    animation.fillMode = kCAFillModeForwards;
    animation.removedOnCompletion = NO;
    return animation;
}

static void clearMotion(CALayer *layer) {
    for (NSString *key in @[sg_riseScaleKey, sg_riseLiftKey, sg_fallScaleKey, sg_fallLiftKey])
        [layer removeAnimationForKey:key];
}

static void rise(UIView *view, SGLyricPaintRecord *record, CGFloat lift, CGFloat response) {
    if (record.risen) return;
    CALayer *layer = view.layer;
    clearMotion(layer);
    record.risen = YES;
    record.motion++;
    record.lift = lift;
    record.response = response;
    [layer addAnimation:spring(@"transform.scale", sg_riseScale, response) forKey:sg_riseScaleKey];
    [layer addAnimation:spring(@"transform.translation.y", -lift, response) forKey:sg_riseLiftKey];
}

// The opposite springs are added on top of the held rise, so settling starts from wherever the
// word is, even mid-rise. Once both have come to rest their sum is zero and all four are removed.
static void settleMotion(UIView *view, SGLyricPaintRecord *record) {
    if (!record.risen) return;
    record.risen = NO;
    CALayer *layer = view.layer;
    // The core clears a word's animations when its line is activated; then there is nothing to undo.
    if (![layer animationForKey:sg_riseScaleKey]) {
        clearMotion(layer);
        return;
    }
    CASpringAnimation *scale = spring(@"transform.scale", -sg_riseScale, record.response * 1.25);
    [layer addAnimation:scale forKey:sg_fallScaleKey];
    [layer addAnimation:spring(@"transform.translation.y", record.lift, record.response * 1.25) forKey:sg_fallLiftKey];
    NSUInteger generation = record.motion;
    __weak UIView *weakView = view;
    __weak SGLyricPaintRecord *weakRecord = record;
    CFTimeInterval wait = fmin(3.0, scale.duration) + 0.05;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(wait * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        SGLyricPaintRecord *current = weakRecord;
        if (current && current.motion == generation && !current.risen) clearMotion(weakView.layer);
    });
}

static void quench(id word) {
    SGLyricPaintRecord *record = recordOf(word, NO);
    retire(record.emitter);
    if (record && [word isKindOfClass:UIView.class]) settleMotion(word, record);
}

static NSUInteger liveEmitters(void) {
    NSUInteger live = 0;
    for (CAEmitterLayer *emitter in sg_emitters.allObjects)
        if (emitter.birthRate > 0 && emitter.superlayer) live++;
    return live;
}

static void emit(id word, SGLyricTheme theme, double sung, double limit) {
    (void)sung;
    if (!themed(theme) || ![word isKindOfClass:UIView.class] || !(limit > 0)) return;
    if (UIAccessibilityIsReduceMotionEnabled()) return; // Themed color and glow only.
    UIView *view = word;
    CGRect bounds = view.bounds;
    CGFloat width = CGRectGetWidth(bounds), height = CGRectGetHeight(bounds);
    if (!(width >= 1) || !(height >= 1)) return;
    SGLyricPaintRecord *record = recordOf(word, YES);
    const SGThemeLook *look = &sg_looks[theme];
    // Everything follows the lyric size, so backing lines stay delicate and large text is not sparse.
    CGFloat size = (CGFloat)fmin(1.4, fmax(0.7, height / 44));
    rise(view, record, sg_risePoints * size, look->response);

    // The lyrics clock may pause without another tick; never hold the rise or keep emitting.
    NSUInteger generation = record.motion;
    __weak UIView *weakView = view;
    __weak SGLyricPaintRecord *weakRecord = record;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(limit * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        SGLyricPaintRecord *current = weakRecord;
        UIView *owner = weakView;
        if (!current || !owner || current.motion != generation) return;
        retire(current.emitter);
        settleMotion(owner, current);
    });

    retire(record.emitter);
    BOOL mote = look->symbol == NULL;
    UIImage *image = particleImage(look->symbol);
    if (!sg_emitters) sg_emitters = [NSHashTable weakObjectsHashTable];
    if (!image || liveEmitters() >= sg_emitterLimit) return;

    // A particle appears small and faint, grows a little and fades: no hard birth, no hard death.
    // Shapes are fewer and a little larger than motes so they read without crowding the word.
    CAEmitterCell *cell = [CAEmitterCell emitterCell];
    cell.contents = (__bridge id)image.CGImage;
    cell.contentsScale = image.scale;
    cell.birthRate = look->rate * (mote ? 1.0f : 0.6f) * (float)fmin(2.0, fmax(0.6, width / 70));
    // The bitmap holds more detail than a particle shows; mipmaps keep it clean when scaled down.
    cell.minificationFilter = kCAFilterTrilinear;
    cell.lifetime = look->lifetime;
    cell.lifetimeRange = look->lifetime * 0.3f;
    // Direction comes from the drift rather than an emission angle, whose meaning depends on the shape.
    cell.velocity = 5;
    cell.velocityRange = 4;
    cell.emissionRange = (CGFloat)(2 * M_PI);
    cell.yAcceleration = look->drift;
    // On-screen points for normal lyric text: where a particle starts, how much that varies,
    // and how much it grows per second.
    cell.scale = (mote ? 1.7 : 8.0) / sg_canvas * size;
    cell.scaleRange = (mote ? 0.8 : 1.6) / sg_canvas * size;
    cell.scaleSpeed = (mote ? 1.4 : 2.2) / sg_canvas * size;
    // Leaves and snow turn slowly as they fall; a heart or a note only tilts.
    cell.spinRange = look->spin;
    cell.color = solid(look->mote, mote ? 0.7 : 0.85).CGColor;
    cell.alphaRange = 0.2f;
    cell.alphaSpeed = -0.7f / look->lifetime;

    CAEmitterLayer *emitter = [CAEmitterLayer layer];
    [CATransaction begin];
    [CATransaction setDisableActions:YES];
    emitter.frame = bounds;
    emitter.emitterShape = kCAEmitterLayerRectangle;
    emitter.emitterPosition = CGPointMake(CGRectGetMidX(bounds), CGRectGetMidY(bounds));
    emitter.emitterSize = CGSizeMake(width, height * 0.5);
    emitter.renderMode = kCAEmitterLayerAdditive;
    emitter.seed = arc4random();
    emitter.emitterCells = @[cell];
    // Without this the emitter starts as if it had already been running.
    emitter.beginTime = [view.layer convertTime:CACurrentMediaTime() fromLayer:nil];
    [view.layer addSublayer:emitter];
    [CATransaction commit];
    record.emitter = emitter;
    [sg_emitters addObject:emitter];
}

static void glow(id lit, float opacity) {
    if ([lit isKindOfClass:UIView.class]) ((UIView *)lit).layer.shadowOpacity = opacity;
}

__attribute__((constructor)) static void installLyricEffects(void) {
    static const SGLyricEffectPainter painter = {paint, unpaint, emit, quench, glow};
    @autoreleasepool {
        if (NSThread.isMainThread) SGInstallLyricEffects(&painter);
        else dispatch_async(dispatch_get_main_queue(), ^{ SGInstallLyricEffects(&painter); });
    }
}
