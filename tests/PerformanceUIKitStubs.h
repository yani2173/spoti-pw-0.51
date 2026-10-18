#import <Foundation/Foundation.h>

// Recent macOS versions also have CADisplayLink. Give the stand-in a distinct runtime class name.
#define CADisplayLink SGTestDisplayLink
#define NSProcessInfo SGTestProcessInfo

@interface SGTestProcessInfo : NSObject
+ (instancetype)processInfo;
@property (nonatomic, getter=isLowPowerModeEnabled) BOOL lowPowerModeEnabled;
@property (nonatomic) NSProcessInfoThermalState thermalState;
@end

typedef NS_ENUM(NSInteger, UIApplicationState) {
    UIApplicationStateActive, UIApplicationStateInactive, UIApplicationStateBackground
};
extern NSRunLoopMode const UITrackingRunLoopMode;
extern NSNotificationName const UIApplicationWillResignActiveNotification;
extern NSNotificationName const UIApplicationDidBecomeActiveNotification;
@interface UIApplication : NSObject
+ (instancetype)sharedApplication;
@property (nonatomic) UIApplicationState applicationState;
- (void)openURL:(NSURL *)url options:(NSDictionary *)options completionHandler:(void (^)(BOOL success))completion;
@end
@interface CADisplayLink : NSObject
+ (instancetype)displayLinkWithTarget:(id)target selector:(SEL)selector;
@property (nonatomic, getter=isPaused) BOOL paused;
@property (nonatomic) NSInteger preferredFramesPerSecond;
@property (nonatomic, copy) void (^pauseObserver)(BOOL paused);
@property (nonatomic, readonly) id target;
@property (nonatomic, readonly) BOOL invalidated;
- (void)invalidate;
- (void)fire;
@end

@interface UIView : NSObject
@property(nonatomic, strong) id window;
@end
@interface UIViewController : NSObject
@property(nonatomic, strong) UIView *viewIfLoaded;
@property(nonatomic, strong) UIViewController *presentedViewController;
- (void)presentViewController:(UIViewController *)controller animated:(BOOL)animated completion:(void (^)(void))completion;
- (void)dismissViewControllerAnimated:(BOOL)animated completion:(void (^)(void))completion;
@end
typedef NS_ENUM(NSInteger, UIAlertControllerStyle) { UIAlertControllerStyleAlert };
typedef NS_ENUM(NSInteger, UIAlertActionStyle) { UIAlertActionStyleDefault };
@interface UIAlertAction : NSObject
@property(nonatomic, copy) void (^handler)(UIAlertAction *);
+ (instancetype)actionWithTitle:(NSString *)title style:(UIAlertActionStyle)style handler:(void (^)(UIAlertAction *))handler;
@end
@interface UIAlertController : UIViewController
@property(nonatomic, strong) NSMutableArray<UIAlertAction *> *actions;
+ (instancetype)alertControllerWithTitle:(NSString *)title message:(NSString *)message preferredStyle:(UIAlertControllerStyle)style;
- (void)addAction:(UIAlertAction *)action;
@end
