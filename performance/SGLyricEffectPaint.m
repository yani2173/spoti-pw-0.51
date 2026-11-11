#import "SGLyricEffects.h"
#import "SGLyricGlyphs.h"
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <objc/runtime.h>
#include <math.h>

// UIKit/Core Animation drawing for sung-word effects. Only public API is used here; the timing
// policy and the verified 0.51.0 hooks are in SGLyricEffects.m. Nothing in this file runs per
// frame: text is recolored once, and particles/motion are rendered by Core Animation.
// The particle artwork of each theme is the vector drawing in SGLyricGlyphs.c.
typedef struct {
    CGFloat top[3], middle[3], bottom[3]; // text gradient, top to bottom
    CGFloat glow[3];
    CGFloat tint[3];                      // particle color
    float greenRange;
    BOOL area;                            // spawn over the whole word instead of a thin band
    BOOL additive;
    CGFloat origin;                       // band position, 0 (top) ... 1 (bottom)
    float birthRate, lifetime, lifetimeRange, alphaSpeed;
    CGFloat velocity, velocityRange, gravity; // gravity < 0 rises
    CGFloat scale, scaleRange, scaleSpeed, spinRange;
} SGThemeLook;

static const SGThemeLook sg_looks[SGLyricThemeCount] = {
    [SGLyricThemeFire] = { // flames lick upward from the lower half of the word
        {1.00, 0.16, 0.05}, {1.00, 0.52, 0.06}, {1.00, 0.89, 0.30}, {1.00, 0.45, 0.10},
        {1.00, 0.62, 0.18}, 0.25f, NO, YES, 0.62,
        12, 0.85f, 0.30f, -1.1f, 9, 7, -75, 0.40, 0.14, -0.20, 0.25},
    [SGLyricThemeIce] = { // snowflakes drift down slowly, turning
        {1.00, 1.00, 1.00}, {0.72, 0.93, 1.00}, {0.36, 0.72, 1.00}, {0.55, 0.85, 1.00},
        {0.92, 0.97, 1.00}, 0, NO, NO, 0.20,
        8, 1.40f, 0.40f, -0.65f, 6, 4, 22, 0.36, 0.12, 0, 1.2},
    [SGLyricThemeWater] = { // drops fall
        {0.55, 0.90, 1.00}, {0.20, 0.62, 1.00}, {0.08, 0.36, 0.95}, {0.20, 0.60, 1.00},
        {0.50, 0.78, 1.00}, 0, NO, NO, 0.30,
        11, 0.60f, 0.15f, -1.5f, 2, 2, 380, 0.34, 0.10, 0, 0},
    [SGLyricThemeLove] = { // hearts float up
        {1.00, 0.62, 0.78}, {1.00, 0.30, 0.52}, {0.96, 0.12, 0.30}, {1.00, 0.25, 0.50},
        {1.00, 0.35, 0.58}, 0.12f, NO, NO, 0.40,
        6, 1.30f, 0.30f, -0.75f, 8, 6, -38, 0.36, 0.12, 0.08, 0.5},
    [SGLyricThemeShine] = { // sparkles twinkle in place over the word
        {1.00, 1.00, 0.95}, {1.00, 0.90, 0.50}, {1.00, 0.72, 0.18}, {1.00, 0.85, 0.40},
        {1.00, 0.93, 0.62}, 0, YES, YES, 0.50,
        11, 0.65f, 0.20f, -1.4f, 4, 4, 0, 0.44, 0.16, -0.55, 2.0},
    [SGLyricThemeElectric] = { // bolts snap outward and vanish
        {1.00, 1.00, 0.75}, {0.75, 0.95, 1.00}, {0.45, 0.55, 1.00}, {0.60, 0.80, 1.00},
        {0.78, 0.93, 1.00}, 0, YES, YES, 0.50,
        16, 0.28f, 0.10f, -3.0f, 95, 45, 0, 0.42, 0.14, -0.70, 0.6},
    [SGLyricThemeNight] = { // crescents hang and fade
        {0.86, 0.74, 1.00}, {0.62, 0.45, 1.00}, {0.40, 0.30, 0.90}, {0.60, 0.40, 1.00},
        {0.76, 0.64, 1.00}, 0, YES, YES, 0.50,
        6, 1.70f, 0.50f, -0.55f, 7, 5, -6, 0.34, 0.12, 0, 0.4},
    [SGLyricThemeSky] = { // small clouds rise
        {1.00, 1.00, 1.00}, {0.74, 0.90, 1.00}, {0.45, 0.74, 1.00}, {0.62, 0.84, 1.00},
        {0.95, 0.98, 1.00}, 0, NO, NO, 0.45,
        6, 1.50f, 0.40f, -0.6f, 6, 4, -24, 0.46, 0.14, 0.05, 0},
    [SGLyricThemeMoney] = { // banknotes are thrown up, then tumble down
        {1.00, 0.95, 0.55}, {0.62, 0.92, 0.42}, {0.20, 0.78, 0.36}, {0.35, 0.90, 0.40},
        {0.56, 0.90, 0.46}, 0.08f, YES, NO, 0.50,
        9, 1.10f, 0.30f, -0.9f, 30, 14, 70, 0.48, 0.12, 0, 3.0},
    [SGLyricThemeBloom] = { // blossoms fall, spinning
        {1.00, 0.80, 0.90}, {1.00, 0.52, 0.72}, {0.52, 0.86, 0.46}, {1.00, 0.55, 0.75},
        {1.00, 0.66, 0.80}, 0.12f, NO, NO, 0.25,
        7, 1.50f, 0.40f, -0.6f, 10, 6, 26, 0.40, 0.12, 0, 2.5},
    [SGLyricThemeBlood] = { // heavy drops from the foot of the word
        {1.00, 0.42, 0.36}, {0.94, 0.14, 0.14}, {0.70, 0.02, 0.08}, {0.95, 0.10, 0.12},
        {0.90, 0.08, 0.10}, 0, NO, NO, 0.70,
        7, 0.90f, 0.25f, -0.9f, 1, 1, 150, 0.36, 0.12, 0, 0},
    [SGLyricThemeParty] = { // notes jump out in mixed colors
        {1.00, 0.40, 0.85}, {0.72, 0.50, 1.00}, {0.30, 0.90, 1.00}, {0.85, 0.45, 1.00},
        {1.00, 0.55, 0.90}, 0.45f, YES, YES, 0.50,
        12, 0.80f, 0.25f, -1.2f, 45, 25, 60, 0.44, 0.14, -0.20, 0.6},
    [SGLyricThemeToxic] = { // bubbles rise and swell
        {0.95, 1.00, 0.45}, {0.62, 0.98, 0.25}, {0.25, 0.82, 0.30}, {0.55, 1.00, 0.25},
        {0.70, 1.00, 0.35}, 0.10f, NO, YES, 0.60,
        9, 1.30f, 0.40f, -0.7f, 5, 4, -30, 0.30, 0.12, 0.12, 0},
    [SGLyricThemePurple] = { // slow purple haze, swelling as it lifts
        {0.98, 0.72, 1.00}, {0.74, 0.36, 0.98}, {0.42, 0.14, 0.78}, {0.72, 0.30, 1.00},
        {0.70, 0.36, 1.00}, 0, YES, YES, 0.50,
        7, 2.00f, 0.50f, -0.45f, 5, 4, -10, 0.55, 0.20, 0.28, 0},
    [SGLyricThemeDrip] = { // cut gems flash and fall away
        {1.00, 1.00, 1.00}, {0.80, 0.95, 1.00}, {0.78, 0.70, 1.00}, {0.80, 0.92, 1.00},
        {0.86, 0.96, 1.00}, 0, YES, YES, 0.50,
        9, 0.85f, 0.25f, -1.1f, 12, 8, 40, 0.42, 0.14, -0.15, 1.0},
};

@interface SGLyricPaintRecord : NSObject
@property (nonatomic) BOOL painted;
@property (nonatomic, copy) NSArray<UILabel *> *labels;
@property (nonatomic, copy) NSArray<UIColor *> *colors;
@property (nonatomic, weak) CALayer *shadowLayer;
@property (nonatomic, strong) id shadowColor;
@property (nonatomic, weak) CAEmitterLayer *emitter;
@end
@implementation SGLyricPaintRecord
@end

static char sg_recordKey;
static NSString *const sg_motionKey = @"spotifyglass.lyricWordMotion";
static NSString *const sg_entranceKey = @"spotifyglass.lyricWordEntrance";
// At most this many words emit particles at once; further words keep color, glow and motion.
static const NSUInteger sg_emitterLimit = 3;
static NSHashTable<CAEmitterLayer *> *sg_emitters;

static NSUInteger liveEmitters(void) {
    NSUInteger live = 0;
    for (CAEmitterLayer *emitter in sg_emitters.allObjects)
        if (emitter.birthRate > 0 && emitter.superlayer) live++;
    return live;
}

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

static UIColor *solid(const CGFloat rgb[3]) {
    return [UIColor colorWithRed:rgb[0] green:rgb[1] blue:rgb[2] alpha:1];
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
        CGFloat locations[3] = {0.20, 0.55, 0.85};
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

// Fills one figure, or for an even-odd drawing the whole path at once so inner figures cut holes.
static void fillGlyph(const SGGlyphShape *shape) {
    UIBezierPath *path = [UIBezierPath bezierPath];
    path.usesEvenOddFillRule = shape->evenOdd;
    for (size_t i = 0; i < shape->count; i++) {
        const float *v = shape->ops[i].v;
        switch (shape->ops[i].kind) {
            case SGGlyphMove: [path moveToPoint:CGPointMake(v[0], v[1])]; break;
            case SGGlyphLine: [path addLineToPoint:CGPointMake(v[0], v[1])]; break;
            case SGGlyphCurve:
                [path addCurveToPoint:CGPointMake(v[4], v[5]) controlPoint1:CGPointMake(v[0], v[1])
                        controlPoint2:CGPointMake(v[2], v[3])];
                break;
            case SGGlyphClose: [path closePath]; break;
            case SGGlyphOval:
                [path appendPath:[UIBezierPath bezierPathWithOvalInRect:CGRectMake(v[0], v[1], v[2], v[3])]];
                break;
            default: return;
        }
        BOOL closed = shape->ops[i].kind == SGGlyphClose || shape->ops[i].kind == SGGlyphOval;
        if (!closed || shape->evenOdd) continue;
        [path fill];
        [path removeAllPoints];
    }
    if (shape->evenOdd) [path fill];
}

// White artwork, rendered once per drawing at the screen's scale; the emitter cell tints it.
static UIImage *particleImage(SGLyricGlyph glyph) {
    static UIImage *images[SGLyricGlyphCount];
    if ((NSUInteger)glyph >= SGLyricGlyphCount) return nil;
    if (images[glyph]) return images[glyph];
    const SGGlyphShape *shape = SGLyricGlyphOutline(glyph);
    CGFloat box = SGLyricGlyphBox;
    UIGraphicsImageRenderer *renderer = [[UIGraphicsImageRenderer alloc] initWithSize:CGSizeMake(box, box)];
    images[glyph] = [renderer imageWithActions:^(UIGraphicsImageRendererContext *context) {
        [UIColor.whiteColor setFill];
        if (shape) {
            fillGlyph(shape);
            return;
        }
        CGFloat components[8] = {1, 1, 1, 1, 1, 1, 1, 0};
        CGFloat locations[2] = {0.25, 1};
        CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
        CGGradientRef gradient = CGGradientCreateWithColorComponents(space, components, locations, 2);
        CGPoint center = CGPointMake(box / 2, box / 2);
        CGContextDrawRadialGradient(context.CGContext, gradient, center, 0, center, box / 2, 0);
        CGGradientRelease(gradient);
        CGColorSpaceRelease(space);
    }];
    return images[glyph];
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
    layer.shadowColor = solid(sg_looks[theme].glow).CGColor;
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

// Stops new particles; the ones in flight fade out, then the layer is removed.
static void retire(CAEmitterLayer *emitter) {
    if (!emitter || emitter.birthRate == 0) return;
    emitter.birthRate = 0;
    __weak CAEmitterLayer *weakEmitter = emitter;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [weakEmitter removeFromSuperlayer];
    });
}

// Small additive motion on top of the core's own transform. Loops end on a whole cycle.
static CAAnimation *motion(SGLyricTheme theme, double sung, double limit) {
    NSString *path = nil;
    NSArray<NSNumber *> *values = nil;
    CFTimeInterval duration = 0;
    float fixedRepeats = 0;
    BOOL smooth = YES;
    switch (theme) {
        case SGLyricThemeFire:
            path = @"transform.scale.y"; values = @[@0, @0.035, @0.01, @0.045, @0]; duration = 0.55; break;
        case SGLyricThemeIce:
            path = @"transform.translation.x"; values = @[@0, @-1.1, @1.1, @-0.8, @0.8, @0];
            duration = 0.32; fixedRepeats = 2; smooth = NO; break;
        case SGLyricThemeWater:
            path = @"transform.translation.y"; values = @[@0, @1.6, @0, @-1.6, @0]; duration = 1.3; break;
        case SGLyricThemeLove:
            path = @"transform.scale"; values = @[@0, @0.1, @0.02, @0.07, @0, @0]; duration = 0.85; break;
        case SGLyricThemeShine:
            path = @"transform.scale"; values = @[@0, @0.07, @0]; duration = 0.5; fixedRepeats = 1; break;
        case SGLyricThemeElectric:
            path = @"transform.translation.x"; values = @[@0, @2, @-2, @1.6, @-1.6, @0.6, @0];
            duration = 0.2; fixedRepeats = 3; smooth = NO; break;
        case SGLyricThemeSky:
            path = @"transform.translation.y"; values = @[@0, @-2.2, @0]; duration = 1.4; break;
        case SGLyricThemeMoney:
            path = @"transform.scale"; values = @[@0, @0.06, @0]; duration = 0.45; fixedRepeats = 2; break;
        case SGLyricThemeBloom:
            path = @"transform.rotation.z"; values = @[@0, @0.025, @0, @-0.025, @0]; duration = 1.6; break;
        case SGLyricThemeBlood:
            path = @"transform.scale"; values = @[@0, @0.05, @0, @0.03, @0]; duration = 1.0; break;
        case SGLyricThemeParty:
            path = @"transform.translation.y"; values = @[@0, @-3, @0, @-1.5, @0]; duration = 0.5; break;
        case SGLyricThemeToxic:
            path = @"transform.translation.x"; values = @[@0, @1.2, @0, @-1.2, @0]; duration = 0.9; break;
        case SGLyricThemePurple: // a slow, leaning sway
            path = @"transform.rotation.z"; values = @[@0, @0.03, @0, @-0.03, @0]; duration = 2.0; break;
        case SGLyricThemeDrip:
            path = @"transform.scale"; values = @[@0, @0.07, @0]; duration = 0.5; fixedRepeats = 2; break;
        default:
            return nil;
    }
    CAKeyframeAnimation *animation = [CAKeyframeAnimation animationWithKeyPath:path];
    animation.values = values;
    animation.duration = duration;
    animation.additive = YES;
    // A curve through the keyframes instead of straight segments; shivers and shocks stay sharp.
    if (smooth) animation.calculationMode = kCAAnimationCubic;
    double cycles = fixedRepeats > 0 ? fixedRepeats : floor(fmin(sung, limit) / duration);
    animation.repeatCount = (float)fmax(1, cycles);
    return animation;
}

// One short overshoot as the word begins, on every theme whose own motion is not already a pop.
static CAAnimation *entrance(SGLyricTheme theme) {
    if (theme == SGLyricThemeShine || theme == SGLyricThemeMoney || theme == SGLyricThemeDrip) return nil;
    CAKeyframeAnimation *animation = [CAKeyframeAnimation animationWithKeyPath:@"transform.scale"];
    animation.values = @[@0, @0.075, @-0.012, @0];
    animation.keyTimes = @[@0, @0.38, @0.74, @1];
    animation.duration = 0.34;
    animation.additive = YES;
    animation.calculationMode = kCAAnimationCubic;
    return animation;
}

static void quench(id word) {
    retire(recordOf(word, NO).emitter);
    if ([word isKindOfClass:UIView.class]) [((UIView *)word).layer removeAnimationForKey:sg_motionKey];
}

static void emit(id word, SGLyricTheme theme, double sung, double limit) {
    if (!themed(theme) || ![word isKindOfClass:UIView.class] || !(limit > 0)) return;
    if (UIAccessibilityIsReduceMotionEnabled()) return; // Themed color and glow only.
    UIView *view = word;
    CGRect bounds = view.bounds;
    CGFloat width = CGRectGetWidth(bounds), height = CGRectGetHeight(bounds);
    if (!(width >= 1) || !(height >= 1)) return;
    SGLyricPaintRecord *record = recordOf(word, YES);
    retire(record.emitter);
    const SGThemeLook *look = &sg_looks[theme];

    CAAnimation *pop = entrance(theme);
    if (pop) [view.layer addAnimation:pop forKey:sg_entranceKey];
    CAAnimation *animation = motion(theme, sung, limit);
    if (animation) [view.layer addAnimation:animation forKey:sg_motionKey];

    UIImage *image = particleImage(SGLyricGlyphForTheme(theme));
    if (!sg_emitters) sg_emitters = [NSHashTable weakObjectsHashTable];
    if (!image || liveEmitters() >= sg_emitterLimit) return;
    // Particles follow the lyric size, so small backing lines are not buried and large text is not sparse.
    CGFloat size = (CGFloat)fmin(1.4, fmax(0.7, height / 44));

    CAEmitterCell *cell = [CAEmitterCell emitterCell];
    cell.contents = (__bridge id)image.CGImage;
    cell.contentsScale = image.scale;
    cell.birthRate = look->birthRate * (float)fmin(2.0, fmax(0.6, width / 70));
    cell.lifetime = look->lifetime;
    cell.lifetimeRange = look->lifetimeRange;
    // Direction comes from gravity rather than an emission angle, whose meaning depends on the shape.
    cell.velocity = look->velocity;
    cell.velocityRange = look->velocityRange;
    cell.emissionRange = (CGFloat)(2 * M_PI);
    cell.yAcceleration = look->gravity;
    cell.scale = look->scale * size;
    cell.scaleRange = look->scaleRange * size;
    cell.scaleSpeed = look->scaleSpeed * size;
    cell.alphaRange = 0.25f;
    cell.spinRange = look->spinRange;
    cell.alphaSpeed = look->alphaSpeed;
    cell.greenRange = look->greenRange;
    cell.color = solid(look->tint).CGColor;

    CAEmitterLayer *emitter = [CAEmitterLayer layer];
    [CATransaction begin];
    [CATransaction setDisableActions:YES];
    emitter.frame = bounds;
    emitter.emitterShape = kCAEmitterLayerRectangle;
    emitter.emitterPosition = CGPointMake(CGRectGetMidX(bounds),
        look->area ? CGRectGetMidY(bounds) : CGRectGetMinY(bounds) + height * look->origin);
    emitter.emitterSize = CGSizeMake(width, look->area ? height * 0.7 : height * 0.2);
    emitter.renderMode = look->additive ? kCAEmitterLayerAdditive : kCAEmitterLayerUnordered;
    emitter.seed = arc4random();
    emitter.emitterCells = @[cell];
    // Without this the emitter starts as if it had already been running.
    emitter.beginTime = [view.layer convertTime:CACurrentMediaTime() fromLayer:nil];
    [view.layer addSublayer:emitter];
    [CATransaction commit];
    record.emitter = emitter;
    [sg_emitters addObject:emitter];

    // The lyrics clock may pause without another tick; never keep emitting indefinitely.
    __weak CAEmitterLayer *weakEmitter = emitter;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(limit * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        retire(weakEmitter);
    });
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
