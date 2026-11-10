#import "SGLyricEffects.h"
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <objc/runtime.h>
#include <math.h>

// UIKit/Core Animation drawing for sung-word effects. Only public API is used here; the timing
// policy and the verified 0.51.0 hooks are in SGLyricEffects.m. Nothing in this file runs per
// frame: text is recolored once, and particles/motion are rendered by Core Animation.
typedef NS_ENUM(NSInteger, SGParticleShape) {
    SGParticleDot, SGParticleSpark, SGParticleHeart, SGParticleDrop, SGParticleShapeCount
};

typedef struct {
    CGFloat top[3], middle[3], bottom[3]; // text gradient, top to bottom
    CGFloat glow[3];
    CGFloat tint[3];                      // particle color
    float greenRange;
    SGParticleShape shape;
    BOOL area;                            // spawn over the whole word instead of a thin band
    BOOL additive;
    CGFloat origin;                       // band position, 0 (top) ... 1 (bottom)
    float birthRate, lifetime, lifetimeRange, alphaSpeed;
    CGFloat velocity, velocityRange, gravity; // gravity < 0 rises
    CGFloat scale, scaleRange, scaleSpeed, spinRange;
} SGThemeLook;

static const SGThemeLook sg_looks[SGLyricThemeCount] = {
    [SGLyricThemeFire] = {
        {1.00, 0.16, 0.05}, {1.00, 0.52, 0.06}, {1.00, 0.89, 0.30}, {1.00, 0.45, 0.10},
        {1.00, 0.60, 0.15}, 0.25f, SGParticleDot, NO, YES, 0.62,
        20, 0.85f, 0.30f, -1.1f, 9, 7, -75, 0.20, 0.09, -0.12, 0},
    [SGLyricThemeIce] = {
        {1.00, 1.00, 1.00}, {0.72, 0.93, 1.00}, {0.36, 0.72, 1.00}, {0.55, 0.85, 1.00},
        {0.92, 0.97, 1.00}, 0, SGParticleDot, NO, NO, 0.20,
        11, 1.30f, 0.40f, -0.7f, 6, 4, 22, 0.16, 0.07, 0, 0},
    [SGLyricThemeWater] = {
        {0.55, 0.90, 1.00}, {0.20, 0.62, 1.00}, {0.08, 0.36, 0.95}, {0.20, 0.60, 1.00},
        {0.50, 0.78, 1.00}, 0, SGParticleDrop, NO, NO, 0.30,
        13, 0.60f, 0.15f, -1.5f, 2, 2, 380, 0.42, 0.12, 0, 0},
    [SGLyricThemeLove] = {
        {1.00, 0.62, 0.78}, {1.00, 0.30, 0.52}, {0.96, 0.12, 0.30}, {1.00, 0.25, 0.50},
        {1.00, 0.35, 0.58}, 0.12f, SGParticleHeart, NO, NO, 0.40,
        6, 1.30f, 0.30f, -0.75f, 8, 6, -38, 0.36, 0.12, 0.08, 0.5},
    [SGLyricThemeShine] = {
        {1.00, 1.00, 0.95}, {1.00, 0.90, 0.50}, {1.00, 0.72, 0.18}, {1.00, 0.85, 0.40},
        {1.00, 0.93, 0.62}, 0, SGParticleSpark, YES, YES, 0.50,
        11, 0.65f, 0.20f, -1.4f, 4, 4, 0, 0.42, 0.16, -0.55, 2.0},
    [SGLyricThemeElectric] = {
        {1.00, 1.00, 0.75}, {0.75, 0.95, 1.00}, {0.45, 0.55, 1.00}, {0.60, 0.80, 1.00},
        {0.78, 0.93, 1.00}, 0, SGParticleSpark, YES, YES, 0.50,
        24, 0.26f, 0.10f, -3.2f, 95, 45, 0, 0.26, 0.10, -0.60, 6.0},
    [SGLyricThemeNight] = {
        {0.86, 0.74, 1.00}, {0.62, 0.45, 1.00}, {0.40, 0.30, 0.90}, {0.60, 0.40, 1.00},
        {0.72, 0.58, 1.00}, 0, SGParticleDot, YES, YES, 0.50,
        8, 1.70f, 0.50f, -0.55f, 7, 5, -6, 0.26, 0.10, 0, 0},
    [SGLyricThemeSky] = {
        {1.00, 1.00, 1.00}, {0.74, 0.90, 1.00}, {0.45, 0.74, 1.00}, {0.62, 0.84, 1.00},
        {0.95, 0.98, 1.00}, 0, SGParticleDot, NO, NO, 0.45,
        9, 1.50f, 0.40f, -0.6f, 6, 4, -24, 0.24, 0.10, 0.05, 0},
    [SGLyricThemeMoney] = {
        {1.00, 0.95, 0.55}, {0.62, 0.92, 0.42}, {0.20, 0.78, 0.36}, {0.35, 0.90, 0.40},
        {1.00, 0.88, 0.40}, 0.10f, SGParticleSpark, YES, YES, 0.50,
        12, 0.75f, 0.25f, -1.2f, 10, 8, -30, 0.36, 0.14, -0.40, 3.0},
    [SGLyricThemeBloom] = {
        {1.00, 0.80, 0.90}, {1.00, 0.52, 0.72}, {0.52, 0.86, 0.46}, {1.00, 0.55, 0.75},
        {1.00, 0.66, 0.80}, 0.12f, SGParticleDrop, NO, NO, 0.25,
        8, 1.50f, 0.40f, -0.6f, 10, 6, 26, 0.34, 0.12, 0, 2.5},
    [SGLyricThemeBlood] = {
        {1.00, 0.42, 0.36}, {0.94, 0.14, 0.14}, {0.70, 0.02, 0.08}, {0.95, 0.10, 0.12},
        {0.90, 0.08, 0.10}, 0, SGParticleDrop, NO, NO, 0.70,
        7, 0.90f, 0.25f, -0.9f, 1, 1, 150, 0.36, 0.12, 0, 0},
    [SGLyricThemeParty] = {
        {1.00, 0.40, 0.85}, {0.72, 0.50, 1.00}, {0.30, 0.90, 1.00}, {0.85, 0.45, 1.00},
        {1.00, 0.55, 0.90}, 0.45f, SGParticleSpark, YES, YES, 0.50,
        18, 0.70f, 0.25f, -1.3f, 45, 25, 60, 0.30, 0.12, -0.30, 5.0},
    [SGLyricThemeToxic] = {
        {0.95, 1.00, 0.45}, {0.62, 0.98, 0.25}, {0.25, 0.82, 0.30}, {0.55, 1.00, 0.25},
        {0.70, 1.00, 0.35}, 0.10f, SGParticleDot, NO, YES, 0.60,
        10, 1.30f, 0.40f, -0.7f, 5, 4, -30, 0.26, 0.12, 0.10, 0},
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

static UIImage *particleImage(SGParticleShape shape) {
    static UIImage *images[SGParticleShapeCount];
    if (images[shape]) return images[shape];
    UIGraphicsImageRenderer *renderer = [[UIGraphicsImageRenderer alloc] initWithSize:CGSizeMake(24, 24)];
    images[shape] = [renderer imageWithActions:^(UIGraphicsImageRendererContext *context) {
        [UIColor.whiteColor setFill];
        if (shape == SGParticleDot) {
            CGFloat components[8] = {1, 1, 1, 1, 1, 1, 1, 0};
            CGFloat locations[2] = {0.25, 1};
            CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
            CGGradientRef gradient = CGGradientCreateWithColorComponents(space, components, locations, 2);
            CGContextDrawRadialGradient(context.CGContext, gradient, CGPointMake(12, 12), 0,
                                        CGPointMake(12, 12), 12, 0);
            CGGradientRelease(gradient);
            CGColorSpaceRelease(space);
        } else if (shape == SGParticleSpark) {
            static const CGFloat points[8][2] = {
                {12, 0}, {14.4, 9.6}, {24, 12}, {14.4, 14.4}, {12, 24}, {9.6, 14.4}, {0, 12}, {9.6, 9.6}};
            UIBezierPath *path = [UIBezierPath bezierPath];
            [path moveToPoint:CGPointMake(points[0][0], points[0][1])];
            for (int i = 1; i < 8; i++) [path addLineToPoint:CGPointMake(points[i][0], points[i][1])];
            [path closePath];
            [path fill];
        } else if (shape == SGParticleHeart) {
            UIBezierPath *path = [UIBezierPath bezierPath];
            [path moveToPoint:CGPointMake(12, 21.5)];
            [path addCurveToPoint:CGPointMake(1.5, 8) controlPoint1:CGPointMake(4.5, 16.5) controlPoint2:CGPointMake(1.5, 12.5)];
            [path addCurveToPoint:CGPointMake(12, 6) controlPoint1:CGPointMake(1.5, 2.5) controlPoint2:CGPointMake(9.5, 1)];
            [path addCurveToPoint:CGPointMake(22.5, 8) controlPoint1:CGPointMake(14.5, 1) controlPoint2:CGPointMake(22.5, 2.5)];
            [path addCurveToPoint:CGPointMake(12, 21.5) controlPoint1:CGPointMake(22.5, 12.5) controlPoint2:CGPointMake(19.5, 16.5)];
            [path closePath];
            [path fill];
        } else {
            [[UIBezierPath bezierPathWithOvalInRect:CGRectMake(9.5, 3, 5, 18)] fill];
        }
    }];
    return images[shape];
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
    switch (theme) {
        case SGLyricThemeFire:
            path = @"transform.scale.y"; values = @[@0, @0.035, @0.01, @0.045, @0]; duration = 0.55; break;
        case SGLyricThemeIce:
            path = @"transform.translation.x"; values = @[@0, @-1.1, @1.1, @-0.8, @0.8, @0];
            duration = 0.32; fixedRepeats = 2; break;
        case SGLyricThemeWater:
            path = @"transform.translation.y"; values = @[@0, @1.6, @0, @-1.6, @0]; duration = 1.3; break;
        case SGLyricThemeLove:
            path = @"transform.scale"; values = @[@0, @0.1, @0.02, @0.07, @0, @0]; duration = 0.85; break;
        case SGLyricThemeShine:
            path = @"transform.scale"; values = @[@0, @0.07, @0]; duration = 0.5; fixedRepeats = 1; break;
        case SGLyricThemeElectric:
            path = @"transform.translation.x"; values = @[@0, @2, @-2, @1.6, @-1.6, @0.6, @0];
            duration = 0.2; fixedRepeats = 3; break;
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
        default:
            return nil;
    }
    CAKeyframeAnimation *animation = [CAKeyframeAnimation animationWithKeyPath:path];
    animation.values = values;
    animation.duration = duration;
    animation.additive = YES;
    double cycles = fixedRepeats > 0 ? fixedRepeats : floor(fmin(sung, limit) / duration);
    animation.repeatCount = (float)fmax(1, cycles);
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
    UIImage *image = particleImage(look->shape);
    if (!image) return;

    CAEmitterCell *cell = [CAEmitterCell emitterCell];
    cell.contents = (__bridge id)image.CGImage;
    cell.contentsScale = image.scale;
    cell.birthRate = look->birthRate * (float)fmin(2.5, fmax(0.6, width / 70));
    cell.lifetime = look->lifetime;
    cell.lifetimeRange = look->lifetimeRange;
    // Direction comes from gravity rather than an emission angle, whose meaning depends on the shape.
    cell.velocity = look->velocity;
    cell.velocityRange = look->velocityRange;
    cell.emissionRange = (CGFloat)(2 * M_PI);
    cell.yAcceleration = look->gravity;
    cell.scale = look->scale;
    cell.scaleRange = look->scaleRange;
    cell.scaleSpeed = look->scaleSpeed;
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

    CAAnimation *animation = motion(theme, sung, limit);
    if (animation) [view.layer addAnimation:animation forKey:sg_motionKey];

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
